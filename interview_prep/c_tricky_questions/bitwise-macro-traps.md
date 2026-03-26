# Bitwise & Macro Traps

> Bitwise operations and macros are fertile ground for interview trick questions. A single missing parenthesis or unsigned/signed mismatch can silently break embedded firmware.

> For macro fundamentals (parenthesization, `do/while(0)`, stringification, token pasting), see [preprocessor.md](../../c_core/preprocessor.md). For bit manipulation basics (set/clear/toggle, power-of-2, popcount), see [bitwise-operations.md](../../c_core/bitwise-operations.md). This file focuses on **tricky interview gotchas** that go beyond the concept explanations.

---

## Macro Pitfalls

### Question 1: Side effects in macros

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int x = 5, y = 3;
int z = MAX(x++, y++);
// Expands to: ((x++) > (y++) ? (x++) : (y++))
// x is incremented TWICE if it's larger
```

**Answer:** `z = 6`, `x = 7`, `y = 4`

**Why:** `x++` evaluates to 5 (post-increment), `y++` evaluates to 3. Since 5 > 3, the true branch executes `x++` again (now 6 → 7). `y` was only incremented once.

**Rule:** Never pass expressions with side effects to macros. Use inline functions or GCC's `typeof` extension:

```c
#define MAX(a, b) ({          \
    typeof(a) _a = (a);       \
    typeof(b) _b = (b);       \
    _a > _b ? _a : _b;        \
})
```

---

## Bitwise Tricky Questions

### Question 2: Sign extension trap

```c
uint8_t sensor = 0xF0;  // 240 in decimal
int16_t result = (int16_t)sensor;
printf("0x%04X\n", result);  // 0x00F0 — correct, zero-extended
```

```c
int8_t sensor = 0xF0;   // -16 in decimal (signed!)
int16_t result = (int16_t)sensor;
printf("0x%04X\n", result);  // 0xFFF0 — sign-extended!
```

**Embedded impact:** When reading sensor data into wider types, signed vs unsigned matters enormously. Always use `uint8_t` for raw data.

### Question 3: Shift operator traps

```c
// Shifting by >= bit width is UNDEFINED BEHAVIOR
uint32_t x = 1;
x << 32;       // UB: shift amount >= width of type

// Shifting negative numbers left is UB
int y = -1;
y << 1;        // UNDEFINED BEHAVIOR (C11 §6.5.7)

// Right-shifting negative numbers is IMPLEMENTATION-DEFINED
int z = -4;
z >> 1;        // Could be -2 (arithmetic) or large positive (logical)
```

**Rule:** Always use unsigned types for bitwise operations in embedded code.

### Question 4: XOR swap

```c
void xor_swap(int *a, int *b) {
    if (a == b) return;  // MUST check — XOR swap fails on same address
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
```

**Why the check matters:** If `a == b`, the first XOR zeros both. The pattern is:
```
a = a ^ b → 0
b = 0 ^ b → b (still b, but a is 0 now)
a = 0 ^ b → b (both are now b, original a is lost)
```

Wait — that actually works. But let's trace with `a == b`:
```
*a ^= *b → *a = 0  (same memory!)
*b ^= *a → *b = 0  (it's the same memory, now 0 ^ 0 = 0)
*a ^= *b → *a = 0  (both are gone)
```

### Question 5: Extract and insert bit fields

```c
// Extract bits [high:low] from value
uint32_t extract_field(uint32_t val, int high, int low) {
    uint32_t mask = ((1U << (high - low + 1)) - 1) << low;
    return (val & mask) >> low;
}

// Insert value into bits [high:low]
uint32_t insert_field(uint32_t reg, uint32_t val, int high, int low) {
    uint32_t mask = ((1U << (high - low + 1)) - 1) << low;
    return (reg & ~mask) | ((val << low) & mask);
}

// Example: extract bits [7:4] from 0xAB
uint32_t result = extract_field(0xAB, 7, 4);  // result = 0xA
```

---

## Tricky Preprocessor Behavior

### Question 6: Token pasting

```c
#define CONCAT(a, b) a##b
#define REG(n) CONCAT(GPIO, n)

REG(A)->ODR = 0xFF;  // Expands to GPIOA->ODR = 0xFF;
```

**Embedded use case:** Generate register names, ISR handler names, and peripheral instances from a macro parameter.

### Question 7: Conditional compilation traps

```c
#define FOO 0

#if FOO     // FOO is 0, so this is #if 0 — FALSE
    // skipped
#endif

#ifdef FOO  // FOO is DEFINED (even though it's 0) — TRUE
    // included
#endif
```

**Watch out:** `#if` checks the **value**. `#ifdef` checks **existence**. A macro defined as 0 is still defined.

### Question 8: `_Static_assert` for compile-time checks

```c
// Ensure struct fits expected register layout
_Static_assert(sizeof(UART_Config) == 16,
               "UART_Config size mismatch with hardware");

// Ensure bit field assumptions hold
_Static_assert(sizeof(int) >= 4,
               "int must be at least 32 bits on this platform");
```

---

## Quick Reference

| Trap | Example | What goes wrong |
|---|---|---|
| Unparenthesized macro args | `SQUARE(x+1)` → `x+1*x+1` | Wrong result due to precedence |
| Side effects in macros | `MAX(x++, y)` | Argument evaluated twice |
| Multi-statement macro | `#define M() a(); b()` | Only first statement in `if` body |
| Shifting by type width | `1 << 32` on 32-bit int | Undefined behavior |
| Signed shift | `(-1) << 1` | Undefined behavior |
| Sign extension | `(int16_t)(int8_t)0xF0` | `0xFFF0` not `0x00F0` |
| `#if` vs `#ifdef` | `#define X 0` | `#if X` is false, `#ifdef X` is true |
| XOR swap same address | `xor_swap(&x, &x)` | Value becomes 0 |
| Missing `1U` in shift | `1 << 31` | Signed overflow (UB) |
