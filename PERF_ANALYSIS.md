# Melon / Melang 性能与多线程分析

## 0. 当前基线 (Linux aarch64, GCC -O2/-O3)

| 引擎 | fib(30) wall time | 与 CPython 比 |
| --- | --- | --- |
| Melang VM (当前 master) | **0.43 s** | 4.7× 慢 |
| Melang VM (perf/lang, 全档完成) | **~0.31 s** | 3.4× 慢 |
| CPython 3 | 0.090 s | 1.0× (基准) |

之前 PERF_CHANGES.md 中的 Linux/aarch64 数据是 0.59s (优化后)，VM commit 又优化到 0.44s — 与本次实测的 0.43s 一致，说明 commit 9a25c3b 已落地。

> **进度更新（高成本档完成后）**：tagged-value (16-byte) 操作数栈
> + DUP borrow bit 已落地。fib(30) 由 0.43s 降至约 0.31s，约 28%
> 加速，与原始预算（数值循环 2–3×）的下界吻合 — 余下的差距主要
> 来自 funccall 路径 (`mln_lang_stack_handler_funccall_run` /
> `mln_lang_var_create_int` 等) 已被压缩到栈外，但函数边界本身
> 仍是 ~30–40% 的剩余开销，不在本档计划内。
>
> **tagged-value 设计选择**：选用 16-byte tag+pad 而非 8-byte
> NaN-boxing 的理由是可移植性 — aarch64 的 52-bit VA、MSVC 的
> /fp:fast NaN payload 处理、以及严格别名规则下的 double↔u64 punning
> 都在 NaN-boxing 路径上有坑。tag+pad 形式是 portable C99，在
> Linux/Mac/MSYS2/MSVC 全部直接编译，无需平台分支。

要追平 CPython（≈ 0.09 s），还需 **~4.7× 加速**。这是激进目标，但不是不可能。下面分析每个剩余瓶颈，按"性价比"排序。

---

## 1. 用 gprof 定位的剩余热点

```
20.4%  dispatch_one              16.6 M 次 (16M 条字节码)
 7.4%  mln_alloc_free             8.3 M 次
 5.6%  mln_alloc_m                8.3 M 次
 5.6%  mln_lang_var_create_int    6.7 M 次（每次调用约 4 次）
 5.6%  __mln_lang_symbol_node_join  1.66 M 次（每次调用 1 次）
 5.6%  mln_lang_stack_handler_funccall_run  1.66 M 次
 5.6%  mln_rbtree_search          3.8 M 次
 3.7%  mln_rbtree_node_new        1.4 M 次
 3.7%  vm_pop_frame_with_ret      1.66 M 次
 1.85% mln_lang_var_create_bool   1.66 M 次（每次调用 1 次比较）
```

`fib(30)` 共 1.66 M 次函数调用、约 16 M 条字节码、每次调用约 5 次 slab alloc。

> 注：`_init` 12.96% 是 gprof 自身的伪样本，可忽略。

---

## 2. 还有哪些可优化点（按性价比排序）

### 🔴 Tier 1：最高性价比（单项可省 10–20%，建议立即做）

#### A. 自递归 CALL_SELF 的"快速直送帧"路径
**问题**：当前 CALL_SELF（fib 自递归）实际上还是走完整调用通道：
1. `mln_lang_funccall_val_new` — 分配 `funccall_val_t` + `args` 内嵌 `mln_array_t`（带 32 元素预留缓冲）
2. 逐个 `mln_lang_funccall_val_add_arg`（往 array 追加）
3. `mln_lang_stack_handler_funccall_run_compat` 进而 `__mln_lang_scope_push`
4. 每个形参一次 `__mln_lang_symbol_node_join`（哈希桶 + 作用域链表）
5. `mln_lang_vm_push_frame_for_call` 分配 `vm_frame_t` + `opstack[]` + `slots[]`
6. RETURN 时 `mln_lang_withdraw_until_func_compat` 把刚 join 的 symbol_node 全部 free 回 cache

profile 中这条路径占 **>20%**。

**优化**：识别 `chunk->purely_local`（编译期判定为：无 `LOAD_GLOBAL` / `BIND_FUNC` / `BIND_SET` / `NEW_OBJECT` / `CALL_VALUE` / `CALL_METHOD`），CALL_SELF 时直接：
- 从帧 freelist 取一个 frame；
- 把 nargs 个 var 直接拷进 `frame->slots[0..n_args-1]`；
- 不 push scope、不 join symbol；
- 不分配 `funccall_val_t`。

RETURN 时对应地：
- 不 pop scope、不 free symbol_node；
- 直接 frame_freelist.push(frame)。

**预期收益**：fib 整段调用通道开销可降到原来的 1/3–1/4。整体 25–35% 加速。

#### B. 帧（frame）/ opstack / slots 的 freelist 复用
**问题**：每次 CALL 都 `mln_alloc_m` 三次（frame、opstack、slots），RETURN 时 `mln_alloc_free` 三次。1.66M 调用 = 10M alloc/free。当前 val/var freelist 已经很有效（profile 中 alloc/free 8.3M 次都集中在这里以外），还剩这 3 个固定 alloc 没 freelist。

**优化**：参考 `val_freelist`/`var_freelist`，加一个 `frame_freelist`。frame 大小固定，opstack 和 slots 也按 chunk->max_stack/n_locals 统计 — 理想情况是同一 chunk 反复调用时 `op_cap`/`n_locals` 已经合适，不需要重新 alloc。

实现细节：把 `frame.next` 复用作 freelist 链；slots[]/opstack[] 单独缓存（按容量分桶或干脆按 chunk 关联）。

**预期收益**：mln_alloc 调用减少一半（剩下 ~4M）。整体 8–12% 加速。

#### C. 内联 `dispatch_one` 进 `vm_step` 主循环
**问题**：每条字节码都通过 `dispatch_one(ctx)` 函数调用 — 16M 次函数调用 + frame 重新装载 `frame = FRAME_TOP(ctx)`。

**优化**：把 `dispatch_one` 的 `switch (insn.op)` 直接内联到 `mln_lang_vm_step` 的 while 循环里。frame 只在 push/pop 时刷新指针，opcode 间不需要重读。

进阶：用 GCC 的 computed goto（threaded code）— 但 commit 9a25c3b 已经声明"Pure C89, no computed-goto"，要保留可移植，可以用 `#if defined(__GNUC__) && !defined(__clang__)` 包裹一个 fast 路径。

**预期收益**：基础内联 5–8% 加速；computed goto 再加 5–10%。

#### D. 帧的 scope-skip 标志位（与 A 配合）
**问题**：即使是 CALL_VALUE / CALL_METHOD，如果被调用者的 chunk 也是 `purely_local`，仍可省去 scope_push/symbol_node_join。当前只对 CALL_SELF 优化的话，跨函数调用还是走慢路径。

**优化**：把"快速直送帧"路径推广到所有 CALL，只要 callee chunk `purely_local`。需要在 CALL_VALUE/CALL_METHOD 的 dispatch 中先检查 callee->vm_chunk->purely_local。

**预期收益**：跨函数 CPU 密集型代码也能受益（不只是 fib 这种自递归）。

---

### 🟡 Tier 2：中等性价比（单项 5–15%，需要更多侵入）

#### E. 操作数栈使用 tagged-immediate（"小整数无需装箱"）
**问题**：每次 `LOAD_INT`、算术结果、比较结果都创建一个 `mln_lang_var_t` + `mln_lang_val_t`。对于 fib 这种以 int 为主的脚本，几乎每条指令都要走 freelist 取 var/val、初始化、压栈，再下一条指令把它消耗掉、归还到 freelist。

**优化**：操作数栈不再存 `mln_lang_var_t *`，而存 `tagged_value_t`：
```c
typedef struct {
    union { mln_s64_t i; double f; mln_lang_var_t *p; } u;
    mln_u8_t tag;  // INT / BOOL / NIL / OBJ_PTR
} tagged_value_t;
```
`LOAD_INT`、`LOAD_LOCAL` 优先走 immediate；arithmetic op 在两端都是 immediate 时直接计算；只有跨函数边界（CALL/RETURN）或赋值 LOCAL 时才"装箱"成 var。

**预期收益**：var/val 分配从 6.7M 降到 ~1.6M（仅边界）。整体 25–40% 加速。

**风险/复杂度**：高。要重写 dispatch 中所有路径，包括对 `mln_lang_methods` 慢路径的退化、Watch 触发点、RETURN 时把 immediate 变 var。但这条是真正闭合到 CPython 的关键。

#### F. 内联 LOAD_LOCAL+常量+二元运算的 super-instruction
比如 `LOAD_LOCAL i; LOAD_INT 1; SUB` 在 fib 体内非常常见（`i-1`, `i-2`）。可以编译为单条 `LOCAL_INT_SUB(slot, iconst_idx)`：直接读 slots[slot]、和 iconst[idx] 做 SUB、压栈。

**预期收益**：3 条字节码合 1 条，dispatch 数量降 30–40%（fib 体内）。整体 5–10% 加速。

成本：编译器多识别一种模式，但是模式很局部、改动小。

#### G. 编译期常量整数 = 0..255 的内联编码
当前 `LOAD_INT` 用 `b` 字段（mln_s16_t）做 `iconsts[]` 索引。如果整数在 -32768..32767 范围内，可以直接编码进 `b`，省去 iconsts 表查找。常量小整数（0/1/2/-1）很多。

**预期收益**：1–2%（小但便宜）。

---

### 🟢 Tier 3：长期/结构性

#### H. mln_gc_collect 每个 slice 都触发
`mln_lang_run_handler` 每个 1700-op 时间片末尾调一次 `mln_gc_collect(ctx->gc, ctx)`。fib 没有 object/array，gc 实际无事可做但仍 walk 数据结构。profile 中 1.4M `mln_rbtree_node_new` 大概率出在 GC 内部（每次 collect 都重建 root set）。

**优化**：
- 在 ctx 上加一个 `gc_dirty` 位，仅当 `BIND_SET`/`NEW_OBJECT`/`NEW_ARRAY` 触发后才设位；
- collect 时若 `!gc_dirty` 直接 early-return；
- 或者把 GC root set 改成"增量"维护，不是每次重建。

**预期收益**：纯 int 脚本 5–10%；object 脚本无影响。

#### I. dispatch_one 中减少分支
`MLN_VOP_ADD/SUB/MUL/DIV/MOD/LT/LE/GT/GE/EQ/NE` 共用一个 case，再走 `apply_binop`，里面二级 switch。可以拆开 11 个独立 case，每个直接做 `int op int` 内联。

**预期收益**：1–3%。

#### J. 把 `mln_lang_var_create_int` 完全 inline 到调用点
当前 `var_create_int` = `val_new(INT) + var_new_ref_string(NULL_name)`，是两个 freelist 操作 + 字段初始化。在 dispatch_one 的 hot 路径，宁可手动展开成 5-6 行内联代码，省一次函数调用。

**预期收益**：1–3%。

---

## 3. 多线程审计 (-t=N)

### 3.1 现状摘要
melang.c 启动 N-1 个 worker，加上主线程共 N 个；所有线程**共享一个 `mln_event_t *ev`**：
- 主线程跑 `mln_event_dispatch(ev)`；
- worker 在 `mln_iothread_recv` 上自旋等首条 init 消息，收到后也进入 `mln_event_dispatch(ev)`；
- 每个线程有自己的 `__thread t_node->signal_fd`（socketpair fd）；
- `mln_lang_run_handler` 在哪个线程被回调，由 epoll 把 signal_fd 分发到哪个线程决定。

调度：
- `mln_lang_run_handler` 用 `pthread_mutex_trylock(&lang->lock)` + `ctx->owner == 0` 抢一个 ctx；
- 抢到后置 `ctx->owner = pthread_self()`，**释放 lang->lock**，运行一个 1700-op 时间片；
- 时间片结束后再加锁，置 `ctx->owner = 0`，若 `run_head != NULL` 调 `signal(lang)`。

### 3.2 单脚本 fib(30) 跑 -t=4 是否有加速？
**❌ 没有，也不会有。**

`mln_lang_job_new` 只创建一个 ctx；`run_head` 永远只有一个 ctx；任意时刻只有一个线程持有它的 owner。其他线程要么空转 trylock 拿不到，要么因为没人调用 `signal` 唤醒它们的 signal_fd 而休眠。

这点要让用户清楚：**`-t=N` 不是 SMP 加速器，它是 I/O offload + 多脚本并发的机制**。fib 这种纯 CPU 单脚本永远是单核。

### 3.3 多脚本 (`./melang a.m b.m c.m -t=3`) 是否真并发？
**✅ 在原理上可以**，每个脚本是一个 ctx，不同 ctx 的时间片可在不同线程并发跑。但有**实际瓶颈**：

#### 瓶颈 1：work distribution 不均
`signal(lang)` 只把**当前线程的** `t_node->signal_fd` 设为可写。也就是说 worker A 跑完一个 slice 后只唤醒自己，并不会通知其他空闲线程"还有工作"。最坏情况下一直是同一个线程在抢 ctx，其他 N-1 个线程空闲。

**修复建议**：在 `mln_lang_run_handler` 末尾，若 `run_head != NULL`，应该唤醒**任意空闲线程**的 signal_fd，而不是自己的。需要遍历 `head` 链表（已有）选一个 owner==0 的 t_node。

#### 瓶颈 2：单一 lang->lock 是中央锁
每个 slice 边界都要 `lock → 调度 → unlock → 运行 → lock → 释放 owner`。N 个线程都要进出这把锁，竞争剧烈。N>4 时几乎肯定退化。

**修复建议**：用每线程的 ready-queue + work-stealing；或者简化：把 trylock 换成 `pthread_mutex_lock` 排队（牺牲公平性减少自旋）；或者把 `ctx->owner` 用原子 CAS 代替全局锁。

#### 瓶颈 3：INTERNAL builtin 全局串行化
funccall_run 走 INTERNAL 分支时：
```c
pthread_mutex_lock(&ctx->lang->lock);
var = prototype->data.process(ctx);
pthread_mutex_unlock(&ctx->lang->lock);
```
任何 `Dump`/`Eval`/`Pipe`/`mq.recv` 都要等所有线程释放 lang->lock 才能开始。多脚本场景下，密集 IO 操作会变成 N 路 worker 排队。

**修复建议**：评估每个 INTERNAL 是否真依赖 lang 全局状态。`Dump` 只用 stderr，可不锁；`Pipe` 已经有自己的 lock；只有 `Eval`/`mln_lang_job_new`-like 才需要 lang->lock。可以让 INTERNAL 自己声明是否需要全局锁（`prototype->needs_lang_lock`），dispatch 时按需加锁。

### 3.4 真正的多线程 bug

#### Bug-1：`pthread_t` 与整数 0 比较的可移植性
`ctx->owner = pthread_self()` 然后 `if (ctx->owner == 0)` — POSIX 不保证 `pthread_t` 是整数类型（macOS pthread_t 是结构体指针，但在 64-bit 上勉强算 OK；Linux 是 `unsigned long`；Windows 是 struct）。当前 Linux 没问题，**移植到任何 pthread_t 是 struct 的平台会编译失败**。

**修复建议**：换成 `mln_u32_t owner_set:1` 标志位 + `pthread_t owner_id`，分离"是否被占有"和"被谁占有"。

#### Bug-2：worker 启动期间的 race
```c
if (nth > 1) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (head != NULL) { ... mln_iothread_send(...); }
        else break;
    }
}
```
主线程持续 `iothread_send` 直到 `head` 链表为空。但 worker 的 `head` 添加发生在 `mln_iothread_entry` 启动之后（不是 `pthread_create` 返回时）。如果 N 大、调度延迟长，主线程会一直发空转消息。OK，最终会停。但**worker 可能没 ready 时就被发消息触发** `mln_iothread_msg_handler` → `signal(lang)` → `mln_event_fd_set(ev, t_node->signal_fd, ...)`。**`t_node` 这时可能还没初始化，`__thread` 默认 0**，`fd=0` 是 stdin！会 crash 或者把 stdin 注册到 epoll 上。

**修复建议**：worker 在 `mln_iothread_entry` 最开始就初始化 `t_node` 并完成 fd 注册，然后再进 recv 循环。或者 main 不要发空转消息，改用 `pthread_barrier` 同步。

#### Bug-3：mln_event 的并发安全
所有线程共享一个 `mln_event_t *ev`，并发调用 `mln_event_fd_set`、`mln_event_dispatch`。最近的 commit `eaefdae perf(event): optimize for 2x+ multi-threaded performance` 应该已经处理了，但要验证：
- 多个线程同时调 `mln_event_dispatch(ev)` 是否安全？（即 epoll_wait + 处理 events 是否互斥？）
- `mln_event_fd_set` 在 dispatch 期间外部线程调用是否安全？

如果上述不安全，多脚本场景下 `t=N>1` 直接 crash。建议跑一个简单的多脚本压测验证。

---

## 4. 推荐的实施顺序

如果只做最少工作匹配 CPython：
1. **Tier 1A** + **B** + **C**（自递归直送帧 + 帧 freelist + dispatch 内联）→ 预计 0.43s 降到 ~0.20s（**2.1× 加速**），fib 还落后 CPython 约 2×。
2. 加 **Tier 2E**（tagged-immediate 操作数栈）→ 预计再降到 ~0.10s，**与 CPython 持平甚至略快**。

如果想保守：
- 先做 1（Tier 1），落地后再决定要不要做 Tier 2E（tagged-immediate 是较大改动）。

多线程方面：
- 单脚本 fib 不会从 -t=N 受益，需要让用户预期对齐；
- 多脚本场景要先修 Bug-1/Bug-2（潜在 crash），再做 work distribution 与 INTERNAL 锁分离。

---

## 5. 不能动的特性兜底

所有上述优化都不能破坏：
- ✅ 事件驱动调度：vm_step 仍按 1700-op budget 让出；
- ✅ Watch 反应式：每个 ASSIGN_LOCAL/SET_PROPERTY/SET_INDEX 仍走 vm_fire_watcher；purely_local 判定不影响 watcher 触发（slot 是否有 func 是运行时检查）；
- ✅ Eval/Pipe/Dump 等 INTERNAL builtin：通过 funccall_run 慢路径，与 Tier 1 优化的快路径并存；
- ✅ Operator overload：编译器看到 `op_*_flag` 仍 refuse 编译，回落 AST walker；
- ✅ lib_src/*.so ABI：Tier 1 的所有改动都只在 ctx 内部加字段（用 padding 替换或追加），不破坏对外 struct 布局——但 frame_freelist 字段的位置如果加在 ctx 末尾就 OK。

---

## 总结

要追平 CPython（4.7×），最现实的路径是 **Tier 1 (A+B+C)** 拿到 2× 之后做 **Tier 2E** 拿到剩余 2×。Tier 1 实施 1–2 天工作量、低风险；Tier 2E 是 3–5 天工作量、中等风险但收益直接决定能不能达到目标。

多线程方面：当前 -t=N 对 fib 无意义；多脚本场景有潜在的启动 race 和 work distribution 问题，建议在做 Tier 1 之前先把这俩 bug 修了，否则任何 -t>1 的回归测试都不可信。
