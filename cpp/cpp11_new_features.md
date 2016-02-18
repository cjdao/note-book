## C++ 11 New Features

### Important Minor Syntax Cleanups
* Spaces in Template Expressions
```cpp
vector<list<int> >; // OK in each C++ version
vector<list<int>>;  // OK since C++11
```

* nullptr and std::nullptr_t
```cpp
void f(int);
void f(void*);
f(0);         // calls f(int)
f(NULL);      // calls f(int) if NULL is 0, ambiguous otherwise
f(nullptr);   // calls f(void*)
```
### Automatic Type Deduction with auto
* declare a variable or an object without specifying its specific type by using auto.
```cpp
auto i = 42;  // i has type int
double f();
auto d = f(); // d has type double
```

* an initialization is required
```cpp
auto i; // ERROR: can’t dedulce the type of i
```

* additional qualifiers are allowed
```cpp
static auto vat = 0.19;
```

* Using auto is especially useful where the type is a pretty long and/or complicated expression
```cpp
vector<string> v;
auto pos = v.begin();          // pos has type vector<string>::iterator

auto l = [] (int x) -> bool {  // l has the type of a lambda 
                               // taking an int and returning a bool
//...
};
``` 


