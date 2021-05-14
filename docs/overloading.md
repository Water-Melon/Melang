Operator Overloading



Except three operators (`$`, `||`, `&&`), the rest operators can be overloaded.

Overloading depends on a group of hook functions, their naming convention is:

```
__<type>_<optype>_operator__
```

`type` includes:

- array
- bool
- func
- int
- nil
- obj
- real
- str

`optype` includes:

- assign `=`
- pluseq `+=`
- subeq `-=`
- lmoveqi `<<=`
- rmoveq `>>=`
- muleq `*=`
- diveq `/=`
- oreq `|=`
- andeq `&=`
- xoreq `^=`
- modeq `%=`
- cor `|`
- cand `&`
- cxor `^`
- equal `==`
- nonequal `!=`
- lt `<`
- le `<=`
- gt `>`
- ge `>=`
- lmov `<<`
- rmov `>>`
- plus `+`
- sub `-`
- mul `*`
- div `/`
- mod `%`
- sdec `--`  e.g. `i--`
- sinc `++` e.g. `i++`
- index `[]`
- property `.`
- negative `-`
- reverse `~`
- not `!`
- pinc `++` e.g. `++i`
- pdec `--` e.g. `--i`



Example:

```
@__int_cand_operator__(left, right)
{
    @mln_print(left);
    @mln_print(right);
    return true;
}
@mln_print((1 & 2));
```

This example will overload `int` operator  `&` . The output is:

```
1
2
true
```

Which means the expression value is the return value of `__int_cand_operator__`.

Let's change something:

```
@__int_cand_operator__(left, right)
{
    @mln_print(left);
    @mln_print(right);
    return left & right;
}
@mln_print((1 & 2));
```

Now, the output is:

```
1
2
0
```

`&` in its hook functions will not be overloaded.



Operator overloading on object is a little bit different. Besides the way that shown above, the hook functions can be existed in `Set` definition.

Example:

```
S {
  attr;
  @__obj_cand_operator__(&left, &right)
  {
      @mln_print(left);
      @mln_print(right);
      return true;
  }
}

a = $S;
b = $S;
@mln_print((a & b));
```

The output is:

```
OBJECT
OBJECT
true
```

In this example, hook function `__obj_cand_operator__` is given in a Set definition.



Another Example:

```
S {
  attr;
  @__obj_cand_operator__(&left, &right)
  {
      @mln_print(left);
      @mln_print(right);
      return true;
  }
}

@__obj_cand_operator__(left, right)
{
    @mln_print(left);
    @mln_print(right);
    return false;
}
O {
  attr;
}

a = $S;
b = $S;
c = $O;
d = $O;
@mln_print((a & b));
@mln_print((c & d));
```

The output is:

```
OBJECT
OBJECT
true
OBJECT
OBJECT
false
```

In this example, `&` between the objects of Set `S` will call the hook function defined in `S`, and Set `O` objects will triggered global function `__obj_cand_operator__`.