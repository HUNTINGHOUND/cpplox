
# cpplox
An interpreter of Lox programming language written in c++. Uses a byte code compiler and virtual machine.


## Lox:
Lox is a dynamically typed high level language that supports OOP operations and designs. Lox automatically manages memory a garbage collector so users don't have to deal with

### Data types:
* Boolean values. Just normal boolean values for logics. As expected, there are only two boolean values.

```
true;
false;
```

* Numbers. So far Lox supports only double precision floating points numbers. I plan to add hexadecimal, binary, and scientific notation in the near future.

```
12; //an integer
1.2; //a decimal
```

* String. A string literal just like any other string literal. Concatenate will be supported. 
```
""; //empty string
"abc"; //a normal string
```
* Nul. Very similar to java null. Just a simple place holder that contains no value.
```
nul; //a nul! how exciting
```
* Collection. This contains a collection of values. These values can be primitive types or classes
```
var a = collection(1,2,3); //{1,2,3}
```

### Expressions:
* Arithematics. Your standard everyday `+-*/` operators. Can only be used on numbers, except for `+` which can also be used for string concatenation and collection concatenation.
```
a + b; \\plus
a - b; \\minus
a / b; \\divide
a * b; \\multiply
```

* Comparison and equality. Your normal compare operators. Note that comparing custom classes will result in comparison of memory address rather than content . Note that equality operator will always return false if the two types are different.

```
less < more;
more > less;
moreorequal >= less;
lessorequal <= more;

2 == 2; \\true
2 == 3; \\false
  
2 != 2; \\false
2 != 3; \\true

2 == "2"; \\false, "2" is a string not a number

```
* Logical operators. The not operator will return `true` if operand is false and vice versa.
```

!true; \\false
!false; \\true

```
The `and` and `or` expression should be pretty intuitive
```

true and true; \\true
true and false; \\false

  

true or false; \\true
false or false; \\false

```
* Grouping. User can use `(` and `)` as grouping to counteract precedence. The precedence of the operators are the same as c++.
  
### Variables

Variables are declared using `var`. If the user omit the initializer, the variable will default to null.

```
var myVar = "this is a variable";
var thisisnul;
```

### Control Flow
Lox have normal `if` and `else` statements. Same as c++, it has for and while loops.
```
if(condition) {
    print "yes";
} else {
    print "no";
}
```
Here is a while loop.
```
var a = 0;
while (a < 10) {
    print a;
    a = a + 1;
}
```
And here is a for loop.
```
for (var a = 1; a < 10; a = a + 1) {
print a;
}
```
You also have.
```
var test a = true ? 1 : 2; //a = 1
```
### Functions
Functions are considered first class in Lox, this means you can get reference, store, and pass them around. Functions works almost exactly the same as c++.

You use keyword `fun` to declare functions. 

  

Here is a function call
```
foo(a,b,c);
```
Here is a function definition
```
fun foo(a,b,c) {
    print a + b + c;
}

  

fun boo(a) {
    return a + 1;
}
```
If a function reach its end without hitting a return statement, it will return `nul`. As mentioned above, functions are considered classes. Thus, we can do things like this.
```
fun add(a,b) {
    return a + b;
}

fun identity(a) {
    return a;
}

print identity(add)(1,2); //prints 3
```
Since `fun` is a statement, we can define local functions.
```

fun out() {
    fun in() {
        print "I am a local function";
    }
    in();
}
```
Lox also suppors closures, creating interesting situations like this.
```

fun returnFunction() {
    var outside = "outside";
        fun inner() {
            print outside;
        }
    return inner;
}

var fn = returnFunction();
fn(); //"outside"

```
### Classes

Very similar to high level programming langugage classes. However, there is no access modifiers. Here is how you declare a class and its methods (notice we don't declare methods with `fun`).
```
class Test {
    foo() {
        print "foo";
    }
    boo() {
        print "boo";
    }
}
```
We treat classes as first class too, similar to functions. Therefore, we can do things like this.
```

//store it into a variable
var someVariable = Test;

//pass it to some function
someFunction(someVariable);

  

//creating an instance
var someInstance = Test();
someInstance.foo(); //prints foo

```
Like most dynamically typed languages, you can add fields whenever you want. If a field did not exist it will be created.
```
someInstance.a = 1
someInstance.b = 2;
```

Ofcourse, it is up to the user to ensure that the instance is in the correct state if its methods are using fields. To make this easier, Lox supports constructors. Lox use `this` to access fields.

```
class Test {
    //constructor
    init(a, b) {
        this.a = a;
        this.b = b;
    }

    add() {
        return this.a + this.b;
    }
}

var testField = Test(1,2);
print testField.add(); //print 3
```

OOP would not be very useful without inheritence, so Lox supports it. To take inspiration from Ruby, `<` will be used. To access a method of superclass, use `super` like java. Constructors are also inherited but you probably want to define the constructor anyways for good coding style.
```
class Test2 < Test {
    init(a,b,c) {
        super.init(a,b);
        this.c = c;
    }
    multi() {
        return this.a + this.b + this.c;
    }
}
 
var testing = Test2(1,2,3);
print testing.add(); //print 3
print testing.multi(); //print 6
```
