# Undefined Behavior (UB)

> Undefined behavior means the C standard imposes **no requirements** — anything can happen. The compiler can assume UB never occurs and optimize accordingly.

---

## What is Undefined Behavior?

When your code triggers UB, the compiler is free to:
- Produce "expected" results (by coincidence)
- Produce garbage values
- Crash (segfault)
- Silently corrupt data
- **Optimize away entire code paths** that lead to UB

**The most dangerous part:** UB can appear to "work" in debug builds and break only in optimized release builds.

---

## Sequence Points & Unsequenced Modifications

### The Rule (C11 §6.5/2)

> If a side effect on a scalar object is unsequenced relative to either a different side effect on the same object or a value computation using the value of the same object, the behavior is undefined.

**In plain English:** Don't read and modify (or modify twice) the same variable between two sequence points.

### Sequence points occur at:
- End of a full expression (the `;`)
- `&&`, `||`, `?:`, `,` operators (between left and right operand)
- Function call (after arguments are evaluated, before the call)

---

## Classic UB Examples

### Example 1: `i = i++ + ++i`

```c
int i = 0;
i = i++ + ++i;  // UNDEFINED BEHAVIOR
```

**Why:** `i` is modified three times (`i++`, `++i`, and `i = ...`) between two sequence points. The standard says nothing about which modification happens first.

Different compilers, or even different optimization levels, may produce different results.

### Example 2: printf with multiple side effects

```c
int x = 10;
printf("%d %d %d", x, x++, ++x);  // UNDEFINED BEHAVIOR
```

**Why:** The order in which function arguments are evaluated is **unspecified**. The compiler can evaluate `x`, `x++`, and `++x` in any order. Combined with multiple modifications to `x`, this is UB.

### Example 3: Multiple modifications

```c
int a = 5;
a = a++ + a++;  // UNDEFINED BEHAVIOR
// Could be 10, 11, 12, or anything
```

### Example 4: Array subscript

```c
int i = 0;
int arr[5] = {10, 20, 30, 40, 50};
arr[i] = i++;   // UNDEFINED BEHAVIOR
// Is it arr[0] = 0 or arr[1] = 0? Unspecified.
```

---

## Other Common Sources of UB

### Signed Integer Overflow

```c
int x = INT_MAX;
x = x + 1;  // UNDEFINED BEHAVIOR (signed overflow)
```

**Note:** Unsigned overflow is well-defined — it wraps around (modulo 2^N).

```c
unsigned int u = UINT_MAX;
u = u + 1;  // Well-defined: u becomes 0
```

**Embedded impact:** Compilers may optimize based on the assumption that signed overflow never occurs:

```c
// Compiler may optimize this to always true:
if (x + 1 > x) { ... }  // "obviously true" if overflow can't happen
```

### Dereferencing NULL or Invalid Pointers

```c
int *p = NULL;
*p = 10;  // UB — typically segfault, but not guaranteed
```

### Use After Free

```c
int *p = malloc(sizeof(int));
free(p);
*p = 42;  // UB — dangling pointer access
```

### Buffer Overflow

```c
int arr[5];
arr[5] = 10;   // UB — writing past array bounds
arr[-1] = 10;  // UB — writing before array start
```

### Shifting Issues

```c
int x = 1;
x << 32;     // UB if int is 32 bits (shift >= width of type)
x << -1;     // UB — negative shift count

int y = -1;
y << 1;      // UB in C99/C11 (shifting negative values)
```

### Division by Zero

```c
int x = 10 / 0;   // UB
int y = 10 % 0;   // UB
```

### Accessing Uninitialized Variables

```c
int x;
printf("%d\n", x);  // UB — x has indeterminate value
```

Note: Static/global variables are zero-initialized by default. Local (auto) variables are not.

### Misaligned Access

```c
char buf[8];
int *p = (int *)(buf + 1);  // Potentially misaligned
*p = 42;  // UB on architectures requiring alignment (ARM Cortex-M0)
```

### Modifying String Literals

```c
char *s = "hello";
s[0] = 'H';  // UB — string literals are in read-only memory
```

See [strings-and-literals.md](strings-and-literals.md) for details.

### Violating strict aliasing

```c
float f = 3.14;
int *ip = (int *)&f;
int x = *ip;  // UB — strict aliasing violation
```

**Safe alternative (type-punning via union or memcpy):**

```c
float f = 3.14;
int x;
memcpy(&x, &f, sizeof(x));  // Well-defined
```

---

## Unspecified vs Implementation-Defined vs Undefined

| Category | Meaning | Example |
|---|---|---|
| **Undefined** | Anything can happen, no guarantees | `i = i++` |
| **Unspecified** | Must pick one of several valid behaviors, no need to document | Order of function argument evaluation |
| **Implementation-defined** | Must pick one behavior AND document it | Size of `int`, right-shift of negative numbers |

---

## How to Catch UB

| Tool | What it catches |
|---|---|
| `-Wall -Wextra -Wpedantic` | Many common warnings |
| `-fsanitize=undefined` (UBSan) | Runtime UB detection |
| `-fsanitize=address` (ASan) | Buffer overflows, use-after-free |
| `valgrind` | Memory errors, uninitialized reads |
| Static analyzers (cppcheck, clang-tidy) | Many patterns at compile time |

```sh
# Compile with UBSan
gcc -fsanitize=undefined -g program.c -o program
./program
```

---

## Quick Reference — "Is This UB?"

| Code | UB? | Why |
|---|---|---|
| `i = i++ + ++i;` | Yes | Multiple unsequenced modifications |
| `printf("%d %d", x++, x++);` | Yes | Unsequenced modifications in args |
| `a[i] = i++;` | Yes | Unsequenced read/write of `i` |
| `f(a++, a++);` | Yes | Unsequenced modifications |
| `x = x + 1;` (x is INT_MAX) | Yes | Signed overflow |
| `u = u + 1;` (u is UINT_MAX) | No | Unsigned wraps to 0 |
| `*NULL` | Yes | Null pointer dereference |
| `arr[10]` (size 10) | Yes | Out-of-bounds access |
| `1 << 32` (32-bit int) | Yes | Shift ≥ type width |
