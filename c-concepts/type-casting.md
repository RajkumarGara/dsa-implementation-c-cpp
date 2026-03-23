# Type Casting & Conversions

> Implicit conversions and integer promotion rules cause some of the most subtle bugs in C — especially when mixing signed and unsigned types, which is extremely common in embedded code.

---

## Implicit Conversions (Automatic)

The compiler automatically converts types in expressions. The rules follow the **usual arithmetic conversions** (also called "integer promotions" + "balancing").

### Integer Promotion

Values smaller than `int` are promoted to `int` (or `unsigned int`) before any operation:

```c
char a = 10, b = 20;
// a + b: both promoted to int, result is int
int result = a + b;  // operations happen in int

uint8_t x = 200, y = 100;
uint8_t z = x + y;  // x and y promoted to int: 200 + 100 = 300
                     // 300 truncated to uint8_t: 300 - 256 = 44
```

### Usual Arithmetic Conversions (Balancing)

When operands have different types, the "smaller" one is converted to the "larger" one:

```
long double  ←  (highest rank)
double
float
unsigned long long
long long
unsigned long
long
unsigned int
int           ←  (everything below is promoted to at least int)
```

---

## The Signed/Unsigned Trap

**The #1 embedded C bug category.**

```c
unsigned int u = 1;
int s = -1;

if (s < u) {
    printf("expected\n");   // you expect this...
} else {
    printf("surprise!\n");  // ...but THIS runs!
}
```

**Why:** When comparing `int` vs `unsigned int`, the `int` is converted to `unsigned int`. `-1` becomes `4294967295` (on 32-bit), which is > 1.

### More examples

```c
// Subtraction trap
unsigned int a = 5;
unsigned int b = 10;
if (a - b < 0) {
    // NEVER TRUE — unsigned subtraction wraps to a large positive number
}
// a - b = 5 - 10 = 4294967291 (not -5)

// Size_t trap (size_t is unsigned)
int len = -1;
if (len < sizeof(int)) {
    // FALSE — len is converted to unsigned, becomes huge
}

// Array indexing
for (unsigned int i = 10; i >= 0; i--) {
    // INFINITE LOOP — unsigned can never be < 0
    // When i is 0 and decremented, it wraps to UINT_MAX
}
```

### Fix: Be explicit about types

```c
// Cast to matching types
if ((int)(a - b) < 0) { ... }

// Or use signed types for comparisons
int a = 5, b = 10;
if (a - b < 0) { ... }  // works as expected

// For loops counting down to 0, use signed or check differently
for (int i = 10; i >= 0; i--) { ... }  // use signed int
```

---

## Explicit Casts

### Integer casts

```c
int x = 300;
uint8_t y = (uint8_t)x;  // truncates: y = 44 (300 & 0xFF)

float f = 3.7;
int n = (int)f;           // truncates toward zero: n = 3 (not rounded)

int negative = -1;
unsigned int u = (unsigned int)negative;  // u = 4294967295 (0xFFFFFFFF)
```

### Pointer casts

```c
// void* to typed pointer (common with malloc)
int *p = (int *)malloc(sizeof(int));

// Between pointer types (use with caution — alignment, strict aliasing)
uint32_t val = 0x12345678;
uint8_t *bytes = (uint8_t *)&val;  // access individual bytes

// Integer to pointer (memory-mapped I/O)
volatile uint32_t *reg = (volatile uint32_t *)0x40000000;

// Pointer to integer (for debug/logging)
#include <stdint.h>
uintptr_t addr = (uintptr_t)ptr;  // portable pointer-to-integer
```

---

## Strict Aliasing Rule

Accessing an object through a pointer of a different type is UB (with exceptions):

```c
float f = 3.14;
int *ip = (int *)&f;
int x = *ip;           // UNDEFINED BEHAVIOR — strict aliasing violation
```

### Exceptions (allowed casts):

| You can access through... | Example |
|---|---|
| The object's actual type | `float *fp = &f;` |
| `char *` / `unsigned char *` | `unsigned char *p = (unsigned char *)&f;` — always OK |
| A compatible type | `const float *p = &f;` |

### Safe type-punning alternatives

```c
// Method 1: memcpy (always safe, compiler optimizes it away)
float f = 3.14;
uint32_t bits;
memcpy(&bits, &f, sizeof(bits));

// Method 2: union (implementation-defined in C99, well-defined in C11)
union { float f; uint32_t u; } pun;
pun.f = 3.14;
uint32_t bits = pun.u;
```

---

## Integer Promotion Pitfalls

### Bitwise NOT on small types

```c
uint8_t mask = 0x0F;
uint8_t result = ~mask;
// ~mask is computed as int: ~0x0000000F = 0xFFFFFFF0
// truncated back to uint8_t: 0xF0 — correct in this case

// But watch out with comparisons:
if (~mask == 0xF0) {
    // FALSE! ~mask is 0xFFFFFFF0 (int), not 0xF0
}
if ((uint8_t)~mask == 0xF0) {
    // TRUE — explicit cast
}
```

### Left shift of uint8_t

```c
uint8_t x = 1;
uint32_t result = x << 24;  // x is promoted to int (32-bit), shift works
// But:
uint8_t y = 0xFF;
uint32_t r = y << 24;       // 0xFF promoted to int = 255
                             // 255 << 24 = 0xFF000000 — but this sets the sign bit!
                             // Result is implementation-defined when assigned to uint32_t

// Fix: cast before shifting
uint32_t r = (uint32_t)y << 24;  // safe
```

---

## Casting in Embedded Contexts

### Register access

```c
// Always use volatile + explicit width
#define REG32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))
#define REG16(addr) (*(volatile uint16_t *)(uintptr_t)(addr))
#define REG8(addr)  (*(volatile uint8_t  *)(uintptr_t)(addr))
```

### Casting function pointers

```c
// Allowed but dangerous — signature must match exactly
typedef void (*generic_fp)(void);
typedef int (*math_fp)(int, int);

generic_fp gfp = (generic_fp)some_math_func;
int result = ((math_fp)gfp)(3, 4);  // cast back before calling
```

### uintptr_t — portable pointer-integer conversion

```c
#include <stdint.h>

void *ptr = malloc(100);
uintptr_t addr = (uintptr_t)ptr;   // pointer → integer (portable)
void *ptr2 = (void *)addr;          // integer → pointer
```

---

## Common Interview Questions

**Q: What's the output?**

```c
unsigned int x = 0;
x = x - 1;
printf("%u\n", x);  // 4294967295 (UINT_MAX) — unsigned underflow wraps
```

**Q: What happens here?**

```c
char c = 128;  // Implementation-defined! char may be signed (-128 to 127)
               // On most systems with signed char: c = -128 (wraps)
```

**Q: Why is `sizeof('A')` different in C and C++?**

```c
sizeof('A');  // C: 4 (character constant has type int)
              // C++: 1 (character constant has type char)
```

**Q: Is this safe?**

```c
int arr[10];
void *vp = arr;
short *sp = (short *)vp;
sp[0] = 42;  // Strict aliasing violation — UB
```

---

## Quick Reference

| Conversion | What Happens | Watch Out |
|---|---|---|
| `char` → `int` | Integer promotion (automatic) | Sign extension if `char` is signed |
| `int` → `unsigned` | Bit pattern reinterpreted | Negative becomes large positive |
| `unsigned` → `int` | Implementation-defined if value doesn't fit | May become negative |
| `float` → `int` | Truncates toward zero | Not rounded; UB if value out of range |
| `int` → `float` | May lose precision | `int` has 32 bits, `float` has 24 bits mantissa |
| `void *` → `T *` | Allowed in C without cast | Requires cast in C++ |
| `T *` → `U *` | Strict aliasing risk | OK for `char *`/`unsigned char *` |
| Signed vs unsigned compare | Signed converted to unsigned | `-1 > 1U` is true! |
