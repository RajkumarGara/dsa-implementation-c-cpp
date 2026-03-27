# const, volatile & restrict Qualifiers

> These type qualifiers are critical in embedded C — `const` protects data, `volatile` prevents dangerous optimizations, and `restrict` enables them.

---

## const — Read-Only Qualifier

### Pointer to const data: `const int *p` (or `int const *p`)

You **cannot** change the value through the pointer, but you **can** change where the pointer points.

```c
int x = 10, y = 20;
const int *p = &x;

*p = 30;     // ERROR: cannot modify the value
p = &y;      // OK: can change the pointer itself
```

**Use case:** Function parameters that should not modify caller's data.

```c
void print_array(const int *arr, int size);  // promises not to modify arr
```

### const pointer: `int * const p`

You **cannot** change where the pointer points, but you **can** change the value.

```c
int x = 10;
int * const p = &x;

*p = 30;     // OK: can modify the value
p = &y;      // ERROR: cannot change the pointer
```

**Use case:** Hardware register pointers — the address is fixed, but you read/write the value.

### const pointer to const data: `const int * const p`

Neither the pointer nor the value can be changed.

```c
const int * const p = &x;

*p = 30;     // ERROR
p = &y;      // ERROR
```

### Reading Rule (Right-to-Left / Clockwise-Spiral)

Read the declaration from the **variable name** outward:

| Declaration | Read as |
|---|---|
| `const int *p` | p is a pointer to a const int (read-only data) |
| `int * const p` | p is a const pointer to an int (fixed address) |
| `const int * const p` | p is a const pointer to a const int (both fixed) |

### const with arrays

```c
const int arr[] = {1, 2, 3};
arr[0] = 10;  // ERROR: array elements are const
```

### const pitfall: casting away const

```c
const int x = 10;
int *p = (int *)&x;  // compiles, but...
*p = 20;              // UNDEFINED BEHAVIOR
```

**Never cast away const** unless you are absolutely sure the original object is not truly const.

---

## volatile — Prevent Compiler Optimization

Tells the compiler: **"This value can change at any time — do not optimize reads/writes away."**

```c
volatile int *status_reg = (volatile int *)0x40000000;

// Compiler MUST read from the actual address every time
while (*status_reg == 0) {
    // wait for hardware to set the flag
}
```

### Without volatile (dangerous!)

```c
int *status_reg = (int *)0x40000000;

// Compiler may optimize this to a single read:
//   temp = *status_reg;
//   while (temp == 0) {} // infinite loop!
while (*status_reg == 0) { }
```

### When to use volatile

| Scenario | Example |
|---|---|
| Memory-mapped I/O registers | `volatile uint32_t *GPIO = (volatile uint32_t *)0x48000000;` |
| Variables modified by ISR | `volatile int flag = 0;` shared between ISR and main |
| Variables modified by another thread | `volatile sig_atomic_t stop = 0;` |
| Variables modified by DMA | Buffer that DMA writes into |

### volatile does NOT provide atomicity

```c
volatile int counter = 0;

// ISR:
counter++;   // NOT atomic — still needs protection (disable interrupts, mutex, etc.)
```

### const volatile — yes, this is valid!

```c
const volatile uint32_t *status = (const volatile uint32_t *)0x40000004;
```

- `volatile` → value can change unexpectedly (hardware can update it)
- `const` → **your code** must not write to it

**Use case:** Read-only hardware status registers.

---

## restrict (C99) — Optimization Hint

Tells the compiler: **"This pointer is the only way to access this memory."** Enables aggressive optimizations.

```c
void copy(int * restrict dest, const int * restrict src, int n) {
    for (int i = 0; i < n; i++)
        dest[i] = src[i];
}
```

- Without `restrict`: compiler assumes `dest` and `src` might overlap → conservative code
- With `restrict`: compiler knows they don't overlap → can vectorize, reorder, etc.

**Rule:** If you lie (the regions actually overlap), the behavior is **undefined**.

**Standard library examples:** `memcpy` uses `restrict` (no overlap allowed), `memmove` does not (handles overlap).

---

## Common Questions

**Q: What's the difference between `const int *p` and `int * const p`?**

| | `const int *p` | `int * const p` |
|---|---|---|
| Change value (`*p = ...`) | No | Yes |
| Change pointer (`p = ...`) | Yes | No |

**Q: Can a variable be both const and volatile?**

Yes. `const volatile uint32_t *reg` — hardware can change the value, but your code cannot write to it. Common for status registers.

**Q: What happens if you don't use `volatile` for a hardware register?**

The compiler may optimize away repeated reads, cache the value in a register, or reorder accesses — causing your code to miss hardware state changes.

**Q: Does `volatile` make access atomic?**

No. `volatile` only prevents the compiler from optimizing away or reordering accesses. For atomicity, you need `sig_atomic_t`, disable interrupts, or use atomic builtins.

---

## Quick Reference

| Qualifier | Meaning | Embedded Use Case |
|---|---|---|
| `const int *p` | Read-only data | Function params that shouldn't be modified |
| `int * const p` | Fixed pointer | Hardware register at known address |
| `const int * const p` | Both fixed | ROM lookup table pointer |
| `volatile` | Don't optimize away | Hardware registers, ISR-shared variables |
| `const volatile` | HW can change, code can't write | Read-only status registers |
| `restrict` | No aliasing | `memcpy`-style functions for optimization |
