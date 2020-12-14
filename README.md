# Melang
Melang is a Co-routine Programming Language. It's just a baby girl now.

It is noly support UNIX/Linux, and actually I don't know what it will become or where it will be used.

Melang has already supported MySQL 8.0, but the newest MySQL C client library is unstable. So if trying to connect an unreachable address, a buffer-overflow will happened even though program may not crash. I have submitted bug report to MySQL community, no more news now.



### Installation

At first, you should confirm that your device can visit GitHub. Then execute these shell commands below:

```
git clone https://github.com/Water-Melon/Melang.git
cd Melang
sudo ./install [--prefix=INSTALL_PATH] [--libprefix=Lib_INSTALL_PATH]
```

*prefix* is the path where melang will be placed after shell *install* executed. Default is */usr/bin/*.

*libprefix* is the install-path of library that melang relied on. The library will be installed at */usr/local/melon_for_melang* by default.

So you have to confirm that you have *w* to these two installation directories.

After installed, you can execute these commands below:

```
melang -v //show version
melang -h //help information
melang a.mln b.mln ... //execute melang files
```



### Syntax

#### 1.data types

Melang is a implicit-type script language. But it has some data types:

1. integer
2. real
3. boolean
4. nil
5. array
6. set
7. object

#### 2.operators

Listed by priority from low to high.

1. ,
2. =  +=  -=  >>=  <<=  *=  /=  |=  &=  ^=  %=
3. &&  ||
4. &  |  ^
5. ==  !=
6. \>  >=  <  <=
7. \>>  <<
8. \+  -
9. \*  /  %
10. ++  --  (suffix)
11. []  .
12. \-  ~  !  &  $  ()  ++  -- (prefix)

#### 3.name rules

Token name must start with alphabet or *_* . And Token name only consist of alphabet, *_* or digit.

```
_    _name    abc123      //correct
123  123abc   //incorrect
```

#### 4.statement

Every statement must be separated by  *;* . Any statement must be ended by  *;*  . There are two exceptions — the definitions of function and set.

```
a = 1;
b = 2;
@c() {
  return 1;
}
@d {
  p = 1;
  @q () {
    return 2;
  }
}
```

#### 5.expression

Expression is the basic unit of statement.

Usually, one statement only has one expression. But if one statement has many expressions, every expression must be separated by  *,*  .

```
a = 1
a = 1, b = 2
```

#### 6.if-else

```
if (a == 1) {
  a = 2;
} fi

if (a == 1)
  a = 2;
fi

if (a == 1) {
  a = 2;
} else {
  a = 3;
}

if (a == 1)
  a = 2;
else
  a = 3;

if (a == 1) {
  a = 2;
} else if (a == 10) {
  a = 8;
} fi

if (a == 1) {
  a = 2;
} else if (a == 10) {
  a = 8;
} else {
  a = 1;
}

if (a == 1)
  a = 2;
else if (a == 10)
  a = 8;
fi

if (a == 1)
  a = 2;
else if (a == 10)
  a = 8;
else
  a = 1;
```

#### 7.for

```
array = [1, 2, 'a':3, 4];
array2 = [];
for (i = 0; i < 4; i++)
  array2[] = array[i];
array3 = [];
for (i = 0; i < 4; i++) {
  if (array[i] == array['a']) continue; fi
  array3[] = array[i];
}
```

#### 8.switch

```
i = 'hello';
switch (i) {
  case 1:
    i = 100;
    break;
  case 'hello':
    i = 'world';
  default:
    i += ' hello';
    break;
}
```

#### 9.while

```
i = 0;
while (i < 10) {
  i++;
}
```

#### 10.set

```
human {
  name;
  age;
  gender;
  score;
  @init (name, age, gender, score)
  {
    this.name = name;
    this.age = age;
    this.gender = gender;
    this.score = score;
  }
  @getScore ()
  {
    return this.score;
  }
  @setScore (score)
  {
    this.score = score;
  }
}

child = $human; //child is an instance of set human.
child.init('John', 8, 'male', 98);
```

#### 11.goto

```
@foo ()
{
  a = 10;
again:
  a++;
  if (a < 100)
    goto again;
  fi
  return a;
}
```

#### 12.break

```
for (i = 0; i < 1000; i++) {
  if (i > 10) break; fi
}

i = 0;
while (i < 1000) {
  i++;
  if (i > 10) break; fi
}

switch (i) {
  case 'aaa':
    i += 'bbb';
    break;
  default:
    i = true;
    break;
}
```

#### 13.continue

```
j = 0;
for (i = 0; i < 1000; i++) {
  if (i % 2) continue; fi
  ++j;
}
```

#### 14.return

```
@foo1 ()
{
  return 1;
}

@foo2 ()
{
  a = 2;
  return a + 1;
}

@foo3 ()
{
  @bar ()
  {
    return 100;
  }
  return bar;
}

@foo4 ()
{
  test {
    a;
    @b ()
    {
      this.a = 10;
    }
  }
  return $test;
}

test {
  a;
  @b() {}
  @c() {return this.b;}
}
a = $test;
a.c()();
```

#### 15.function

There are two ways to call function:

  1> foo();

  2> @foo();

The difference between these two is:

    The first case is usually used to call a function where is in some functions' scope.
    
    The second case is usually to call some global functions, such as *mln_print*, especially in some other functions.

```
@foo ()
{
  a = 1;
  return a;
}

@foo ()
{
  @b ()
  {
    return 'aaa';
  }
  return b;
}

test
{
  var1;
  var2;
  @foo1() {return this.var1;}
  @foo2() {return this.var2;}
}

@foo (&a)
{
  a = 'aaa';
}
a = 1;
foo(a);
@mln_dump(a); // a = 'aaa'

//variable arguments
@foo ()
{
  @mln_print(args); //args is an internal variable.
}
@foo('Hello', 1, ['world', 2]); //output: [hello, 1, [world, 2, ], ]
```

#### 16.reflection

```
@foo ()
{
  return 'hello';
}
a = 'foo';
b = 'a';
b();
```

#### 17.injection

```
set {
  a;
}
inst = $set;
inst.b = 10;
@mln_dump(inst.b); // 10
ret = @mln_setProperty(inst, 'c', 'abc');
@mln_print(ret); // 'abc'
@mln_print(inst.c); // 'abc'
@mln_print(@mln_getProperty(inst, 'c')); // 'abc'
```

#### 18.reactive programming

```
@handler (newValue, userData)
{
  @mln_dump(newValue);
  @mln_dump(userData);
}
var = 10;
userData = 'hello';
@mln_watch(var, handler, userData);
var = 11; //then handler() will be called.

@handler (&newValue, &userData)
{
  userData = 'world';
}
var = 10;
userData = 'hello';
@mln_watch(var, handler, userData);
var = 11; //then handler() will be called.
@mln_dump(userData); // userData = 'world'
```

**19.co-routine**

​		<1>Run co-routine in command line

```
/*the tasks those behind command name are running in a same thread.*/
$ ./melang a.mln b.mln ...
```

​		<2>Run co-routine via eval function

​		Let's see a very simple http server implemented by Melang.

```
/* filename: server.mln */
listenfd = @mln_tcpListen('127.0.0.1', '1234');
while (1) {
    fd = @mln_tcpAccept(listenfd);
    @mln_print(fd);
    @mln_eval('processor.mln', fd);
}
```

```
/* filename: processor.mln */
fd = EVAL_DATA;
ret = @mln_tcpRecv(fd);
if (ret) {
    @mln_tcpSend(fd, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na\r\n\r\n");
}fi
@mln_tcpClose(fd);
@mln_print('quit');
```

​		Now, you can use ab (apache bench) to make a test.

**20.co-routine pool**

​		web server example

```
/* filename: server.mln */
listenfd = @mln_tcpListen('127.0.0.1', '1234');
for (i = 0; i < 100; ++i) {
    @mln_eval('processor.mln', i);
}
while (1) {
    fd = @mln_tcpAccept(listenfd);
    @mln_msgQueueSend('test', fd);
}
```

```
/* filename: processor.mln */
@mln_print(EVAL_DATA);
while (1) {
    fd = @mln_msgQueueRecv('test');
    ret = @mln_tcpRecv(fd);
    if (ret) {
        @mln_tcpSend(fd, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na\r\n\r\n");
    }fi
    @mln_tcpClose(fd);
}
```



### Pre-process

#### 1.macro

```
#define NAME 'John'
#NAME
#undef NAME
```

#### 2.condition

```
#if NAME
#endif

#if NAME
#else
#endif
```

#### 3.include

```
#include '~/lib.mln'
```



### Functions

For more details, see https://github.com/Water-Melon/Melon/blob/master/doc/Melon%20Developer%20Guide.txt  section *Internal Function*.
