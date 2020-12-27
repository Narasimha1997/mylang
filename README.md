# MyLang

[![Build Status](https://dev.azure.com/vkvaltchev/MyLang/_apis/build/status/vvaltchev.mylang?branchName=master)](https://dev.azure.com/vkvaltchev/MyLang/_build/latest?definitionId=5&branchName=master)
[![codecov](https://codecov.io/gh/vvaltchev/mylang/branch/master/graph/badge.svg?token=B3L5Z6T6PR)](https://codecov.io/gh/vvaltchev/mylang)

## What is MyLang?

MyLang is a simple educational programming language inspired by `Python`, `JavaScript`,
and `C`, written as a personal challange in a short time, mostly to have fun writing a
*recursive descent parser* and explore the world of interpreters. Don't expect a full-blown
scripting language with libraries and frameworks ready for production use. However, `MyLang`
has a minimal set of builtins and, it *could* be used for practical purposes as well.

## Syntax

The shortest way to describe `MyLang` is: a dynamic *python-ish* language looking
like `C`. Probably, the fastest way to learn this language is to check out the
scripts in the `samples/` directory, while reading the short documentation
below.

### Core concepts

`MyLang` is a dynamic duck-typing language, like `Python`. If you know `Python` and
you're willing to use `{ }` braces, you'll be automatically able to use it. No surprises.
Strings are immutable like in `Python`, arrays can be defined using `[ ]` like in `Python`,
and dictionaries can be defined using `{ }`, as well. This language even supports
array-slices using the same `[start:end]` syntax used by `Python`.

Said that, `MyLang` differs from `Python` and other scripting languages in several
aspects:

  - There's support for parse-time constants declared using `const`.

  - All variables must be declared using `var`.

  - Variables have a scope like in C. Shadowing is supported when a variable
    is explicitly re-declared using `var`.

  - All expression statements must end with `;` like in `C`, `C++`, and `Java`.

  - The keywords `true` and `false` exist, but there's no `boolean` type. Like in `C`,
    `0` is false, everything else is `true`. However, strings, arrays and dictionaries
    have a boolean value, exactly like in `Python`. The `true` builtin is just an
    alias for the integer `1`.

  - The assignment operator `=` can be used like in `C`, inside expressions, but
    there's no such thing as the comma operator (see below).

  - MyLang supports both the classic `for` loop and an explicit `foreach` loop.

### Declaring variables

Variables are always declared with `var` and live in the scope they've been declared
(and its sub-scopes). For example:

```C#
    # Variable declared in the global scope
    var a = 42;

    {
        var b = 12;
        # Here we can see both `a` and `b`
        print(a, b);
    }

    # But here we cannot see `b`.
```

It's possible to declare multiple variables using the following familiar syntax:

```C#
    var a,b,c;
```

But there's a caveat: initializing variables doesn't work like in C. Consider the
following statement:

```C#
    var a,b,c = 42;
```

In this case, instead of just declaring `a` and `b` and initializing to `c` to 42,
we're initializing *all* the three variables to the value 42. In order to initialize
each variable to different value, use the array-expansion syntax:

```C#
    var a,b,c = [1,2,3];
```

### Declaring constants

Constants can be declared the same way variables *but* they cannot be shadowed in
nested scopes. For example:

```C#
    const c = 42;

    {
        # That's not allowed
        const c = 1;
    }
```

In `MyLang` constants are evaluated at *parse-time*, in a similar fashion to in `C++`'s
constexpr declarations. While initializing a `const`, any kind of literal can be used
in addition to the whole set of *const* builtins. For example:

```C#
    const val = sum([1,2,3]);
    const x = "hello" + " world" + " " + join(["a","b","c"], ",");
```

To understand how exactly a constant has been evaluated, run the interpreter
with the `-s` option, to dump the *abstract syntax tree* before running the script.
For the example above:

```
$ cat > t
    const val = sum([1,2,3]);
    const x = "hello" + " world" + " " + join(["a","b","c"], ",");
$ ./build/mylang t
$ ./build/mylang -s t
Syntax tree
--------------------------
Block(
)
--------------------------
```

Surprised? Well, constants other than arrays and dictionaries are not even instatiated
as variables. They just don't exist at *runtime*. Let's add a statement using `x`:

```
$ cat >> t
    print(x);
$ cat t
    const val = sum([1,2,3]);
    const x = "hello" + " world" + " " + join(["a","b","c"], ",");
    print(x);
$ ./build/mylang -s t
Syntax tree
--------------------------
Block(
  CallExpr(
    Id("print")
    ExprList(
      "hello world a,b,c"
    )
  )
)
--------------------------
hello world a,b,c
```

Now, everything should make sense. Almost the same thing happens with arrays
and dictionaries with the exception that they are instanted as at runtime as
well, in order to avoid having potentially huge literals everywhere.
Consider the following example:

```
$ ./build/mylang -s -e 'const ar=range(4); const s=ar[2:]; print(ar, s);'
Syntax tree
--------------------------
Block(
  VarDecl(
    Id("ar")
    Op '='
    LiteralArray(
      Int(0)
      Int(1)
      Int(2)
      Int(3)
    )
  )
  VarDecl(
    Id("s")
    Op '='
    LiteralArray(
      Int(2)
      Int(3)
    )
  )
  CallExpr(
    Id("print")
    ExprList(
      Id("ar")
      Id("s")
    )
  )
)
--------------------------
[0, 1, 2, 3] [2, 3]
```

As you can see, the *slice* operation has been evaluated at *parse* time while
initializing the constant `s`, but both the arrays exist at runtime as well.
Note: subscript operations on const expressions, instead, are converted to
literals:

```
$ ./build/mylang -s -e 'const ar=["a","b","c"]; print(ar[1]);'
Syntax tree
--------------------------
Block(
  VarDecl(
    Id("ar")
    Op '='
    LiteralArray(
      "a"
      "b"
      "c"
    )
  )
  CallExpr(
    Id("print")
    ExprList(
      "b"
    )
  )
)
--------------------------
b
```

That looks like a good trade-off for performance: small values like integers, floats
and strings are converted to literals during the *const evaluation*, while arrays and
dictionaries (potentially big) are left as read-only symbols at runtime, while some
operations on them (like `[index]` and `len(arr)`) are still const-evaluated.

### Conditional statements

Conditional statements work exactly like in `C`. The syntax is:

```C#
if (condition) {
    # Then block
} else {
    # Else block
}
```

And the `{ }` braces can be omitted like in C, in the case of single-statement
blocks. `condition` can be any expression, for example: `(a=3)+b >= c && !d`.

### Classic loops

`MyLang` support the classic `while` and `for` loops, exactly like in `C`.

```C#
while (condition) {
    # body
}

for (var i = 0; i < 10; i += 1) {

    # body

    if (something)
        break;

    if (something_else)
        continue;
}
```

Probably, there are only a few difference from `C` worth pointing out:

  - The `++` and `--` operators do not exist in `MyLang`, at the moment.

  - To declare multiple variables, use the syntax: `var a, b = [3,4];` or
    just `var a,b,c,d = 0;` if you want all the variables to have the same
    initial value.

  - To increase the value of multiple variables use the syntax:
    `a, b += [1, 2]`. In the extremely rare and complex cases when in the
    *increment* statement of the for-loop we need to assign to each variable
    a new variable using different expressions, take advantage of the
    expansion syntax in assignment: `i, j = [i+2, j = next(i, j) * 2]`.

### The foreach loop

### Functions and lambdas

### Exceptions

## Builtin types and functions

### Integers

### Floating-point numbers

### Strings

### Arrays

### Dictionaries
