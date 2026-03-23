# dsa-implementation-c-cpp
A clean collection of data structures and algorithms implemented from scratch in C and C++

## C Concepts Cheatsheets

The [c-concepts/](c-concepts/) folder contains markdown reference docs covering tricky C/C++ topics for embedded software interview prep and day-to-day work — pointers, const/volatile, operator precedence, undefined behavior, memory layout, bitwise operations, and more.

## Build

Run all commands from the repository root:

```sh
cd dsa-implementation-c-cpp
```

Build the current sample program with:

```sh
make
```

Run the default program with:

```sh
make run
```

Run one program directly by calling its binary:

```sh
./arrays/array_adt
```

Run one specific program through make:

```sh
make run-one PROGRAM=arrays/array_adt
```

Clean generated files with:

```sh
make clean
```
