# Type Conversion & Undefined Behavior Traps

> For type conversion fundamentals, see [type-casting.md](../../c_core/notes/type-casting.md). For undefined behavior concepts, see [undefined-behavior.md](../../c_core/undefined_behavior/undefined-behavior.md). This file focuses on **tricky interview output questions** that test these concepts.

---

## Implicit Type Conversions

> See [type-casting.md](../../c_core/notes/type-casting.md) for signed/unsigned rules, integer promotion, and conversion rank.

### Question 1: Signed vs unsigned comparison — quick check

`int a = -1; unsigned int b = 1; if (a < b)` — which branch executes?

**Answer:** `else` branch. `-1` is converted to `UINT_MAX` (4294967295). See [type-casting.md](../../c_core/notes/type-casting.md) for full explanation.

### Question 2: Integer promotion in assignment

```c
uint8_t a = 0xFF, b = 0x01;
printf("%d\n", a + b);   // ?
uint8_t c = a + b;
printf("%d\n", c);       // ?
```

**Answer:** `256` (promoted to `int` for arithmetic), then `0` (truncated back to `uint8_t` on assignment).

### Question 3: size_t is unsigned

```c
for (size_t i = 10; i >= 0; i--) {
    printf("%zu\n", i);
}
// INFINITE LOOP — size_t is unsigned, i >= 0 is always true
```

**Fix:**

```c
for (size_t i = 10; i < 11; i--) { ... }
// Or better:
for (size_t i = 11; i-- > 0; ) { ... }
```

---

## Undefined Behavior Classics

> See [undefined-behavior.md](../../c_core/undefined_behavior/undefined-behavior.md) for the full UB catalog. See [strings-and-literals.md](../../c_core/notes/strings-and-literals.md) for string literal rules.

### Question 4: Accessing uninitialized variables

```c
int x;
printf("%d\n", x);  // UNDEFINED BEHAVIOR
// Could print 0, garbage, or the compiler can assume it never happens
```

**Note:** In some embedded toolchains, `.bss` is zeroed at startup, making uninitialized globals appear safe. But local variables (on the stack) contain whatever was there before.

---

## Tricky Output Questions

### Question 5: Post-increment in expressions

```c
int x = 5;
int y = x++ + ++x;
// UNDEFINED BEHAVIOR — x modified twice without sequence point
```

**Many interviewers expect a specific answer.** Tell them it's UB and explain why. If pressed, different compilers give different results — that's the point.

### Question 6: Return value and side effects

```c
int x = 0;
int y = (x = 5, x + 3);
printf("%d %d\n", x, y);  // 5 8
```

**Why:** The comma operator evaluates left-to-right. `x = 5` runs first, then `x + 3` (which is 8) becomes the value of the expression.

### Question 7: sizeof is not evaluated

```c
int x = 5;
int y = sizeof(x++);
printf("%d %d\n", x, y);  // 5 4
```

**Why:** The operand of `sizeof` is **not evaluated** (except for VLAs). `x++` never executes. `sizeof` resolves at compile time.

### Question 8: Short-circuit evaluation

```c
int a = 0, b = 0;
if (a++ && b++) {
    printf("inside\n");
}
printf("a=%d b=%d\n", a, b);  // a=1 b=0
```

**Why:** `a++` evaluates to 0 (false), so `&&` short-circuits — `b++` is never reached. `a` was post-incremented to 1.

---

## Volatile and Optimization Traps

### Question 9: Optimized-away loop

```c
int flag = 0;

// In another thread or ISR: flag = 1;

while (!flag) {
    // Compiler may optimize to: while(1) — infinite loop
    // Because it sees flag is never modified in this scope
}
```

**Fix:** Declare `volatile int flag = 0;`

### Question 10: Multiple volatile reads

```c
volatile uint32_t *status = (volatile uint32_t *)0x40000004;

uint32_t val1 = *status;  // Read 1: might clear a flag
uint32_t val2 = *status;  // Read 2: different value possible
// val1 != val2 is possible and expected
```

**Embedded impact:** Some status registers clear-on-read. Reading twice means losing the first value. Always read volatile once into a local variable:

```c
uint32_t snapshot = *status;
if (snapshot & FLAG_A) { ... }
if (snapshot & FLAG_B) { ... }
```

---

## Tricky Initialization

### Question 11: Static and global defaults

```c
int global_var;           // initialized to 0 (in .bss)
static int static_var;    // initialized to 0 (in .bss)

void foo(void) {
    int local_var;        // UNINITIALIZED (garbage on stack)
    static int s = 0;     // initialized once, retains value across calls
}
```

### Question 12: Designated initializers

```c
int arr[5] = {[2] = 30, [4] = 50};
// arr = {0, 0, 30, 0, 50}

struct Point { int x, y, z; };
struct Point p = {.z = 3, .x = 1};
// p = {1, 0, 3}
```

**Note:** Unspecified members are zero-initialized. This is well-defined and useful for partial init.

---

## Quick Reference

| Trap | What goes wrong | Prevention |
|---|---|---|
| Signed/unsigned comparison | `-1 > 1U` is true | Enable `-Wsign-compare` — see [type-casting.md](../../c_core/notes/type-casting.md) |
| Integer promotion | `uint8_t + uint8_t` is `int` | Be aware of widening in expressions |
| Unsigned underflow | `5U - 10U` wraps to huge value | Check before subtracting |
| `size_t` loop decrement | `i >= 0` always true | Use `i-- > 0` or signed counter |
| String literal modification | Crash or silent corruption | Use `char[]` — see [strings-and-literals.md](../../c_core/notes/strings-and-literals.md) |
| Missing volatile | Compiler optimizes out reads | Use volatile for ISR/HW variables |
| Clear-on-read registers | Double read loses data | Read volatile once into local |
| sizeof with side effects | Side effects never execute | sizeof is compile-time |
| Comma operator confusion | Only last expression is result | Use for loop headers or avoid |
