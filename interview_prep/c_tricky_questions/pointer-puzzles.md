# Pointer Puzzles

> For pointer fundamentals (arithmetic, double pointers, void pointers), see [pointers.md](../../c_core/pointers.md). For function pointer patterns, see [function-pointers.md](../../c_core/function-pointers.md). This file focuses on **tricky interview questions** that go beyond the concept explanations.

---

## Pointer Arithmetic with Different Types

### Question 1: What does this print?

```c
int arr[] = {10, 20, 30, 40, 50};
int *p = arr;
printf("%d\n", *(p + 3));
printf("%d\n", *(arr + 3));
printf("%d\n", 3[arr]);
```

**Answer:** All three print `40`.

- `*(p + 3)` — pointer arithmetic: moves 3 * sizeof(int) bytes forward
- `*(arr + 3)` — array name decays to pointer, same thing
- `3[arr]` — C defines `a[b]` as `*(a + b)`, so `3[arr]` is `*(3 + arr)` = `*(arr + 3)`

### Question 2: Arithmetic on cast pointers

```c
int x = 0x12345678;
char *cp = (char *)&x;
printf("0x%x\n", *(cp + 1));
```

**Answer:** Depends on endianness.
- **Little-endian** (x86, most ARM): memory is `78 56 34 12` → prints `0x56`
- **Big-endian**: memory is `12 34 56 78` → prints `0x34`

**Embedded relevance:** This is how you inspect byte order of multi-byte registers.

### Question 3: Pointer difference

```c
int arr[10];
int *p1 = &arr[2];
int *p2 = &arr[7];
printf("%ld\n", p2 - p1);
printf("%ld\n", (char *)p2 - (char *)p1);
```

**Answer:**
- `p2 - p1` = `5` (difference in elements)
- `(char *)p2 - (char *)p1` = `20` (difference in bytes, assuming sizeof(int) = 4)

**Rule:** Pointer subtraction yields the number of elements of the pointed-to type, not bytes.

---

## Array Decay and sizeof

> See [pointers.md — Array Name vs Pointer](../../c_core/pointers.md) for the concept explanation.

### Question 4: Quick check

What does `sizeof(arr)` return inside `void foo(int arr[])` on a 64-bit system?

**Answer:** `8` — the pointer size. Arrays decay to pointers in function parameters.

### Question 5: sizeof with string literals

```c
char s1[] = "hello";
char *s2 = "hello";
```

What are `sizeof(s1)`, `sizeof(s2)`, and `strlen(s1)`?

**Answer:** `6` (includes `'\0'`), `8` (pointer size), `5` (excludes `'\0'`).

---

## Pointer to Array vs Array of Pointers

### Question 6: Declarations

```c
int *p[10];    // Array of 10 int pointers
int (*p)[10];  // Pointer to an array of 10 ints
int (*p)(int); // Pointer to a function taking int, returning int
```

**Trick:** Read declarations using the **right-left rule** — start at the variable name, go right, then left, alternating.

### Question 7: What's the difference?

```c
int arr[3][4];
int (*p)[4] = arr;      // pointer to array of 4 ints
int *q = arr[0];         // pointer to int (first element)

printf("%zu\n", sizeof(*p));  // 16 — size of int[4]
printf("%zu\n", sizeof(*q));  // 4  — size of int
printf("%d\n", p[1][2]);     // same as arr[1][2]
```

---

## Dangling, Wild, and NULL Pointers

> See [pointers.md — Dangling Pointer](../../c_core/pointers.md) for the concept explanation.

### Question 8: Spot the dangling pointer

```c
int *foo(void) {
    int x = 42;
    return &x;  // returning address of local variable
}
int *p = foo();
*p;  // UB — what's wrong?
```

**Answer:** `x` lives on `foo()`'s stack frame, which is reclaimed on return. `p` is a dangling pointer.

### Question 9: Use-after-free

```c
int *p = malloc(sizeof(int));
*p = 10;
free(p);
*p;  // UB — what's wrong?
```

**Answer:** `p` is dangling after `free()`. Always set pointers to NULL after free.

### Question 10: NULL pointer dereference

```c
int *p = NULL;
*p = 5;  // UNDEFINED BEHAVIOR — segfault on most systems
```

**Embedded impact:** On bare-metal systems without an MMU, writing to address 0 might silently corrupt the interrupt vector table.

---

## const and Pointers

### Question 11: Which is which?

```c
const int *p;         // pointer to const int — can't change *p
int const *p;         // same as above
int *const p;         // const pointer to int — can't change p
const int *const p;   // const pointer to const int — can't change either
```

**Memory trick:** Read the `*` as a divider. `const` to the left of `*` = data is const, `const` to the right of `*` = pointer is const.

### Question 12: Tricky const correctness

```c
const int x = 10;
int *p = (int *)&x;
*p = 20;
printf("%d\n", x);  // UNDEFINED BEHAVIOR
```

**Why:** Modifying a `const`-qualified object through a cast is UB. The compiler may have placed `x` in read-only memory, or it may still read the old cached value.

---

## void Pointer Tricks

### Question 13: Arithmetic on void pointers

```c
void *vp = malloc(10);
vp++;       // ILLEGAL in standard C (sizeof(void) is undefined)
vp = vp + 1; // ILLEGAL in standard C
```

**Note:** GCC allows `void *` arithmetic as an extension (treats size as 1 byte), but this is **not portable**.

### Question 14: Generic swap using void pointers

```c
void swap(void *a, void *b, size_t size) {
    char temp[size];  // VLA — careful in embedded (stack usage)
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

int x = 10, y = 20;
swap(&x, &y, sizeof(int));
// x = 20, y = 10
```

---

## Function Pointers

### Question 15: Callback pattern

```c
typedef void (*callback_t)(int);

void on_event(int code) {
    printf("Event: %d\n", code);
}

void register_callback(callback_t cb) {
    cb(42);  // call the function through the pointer
}

register_callback(on_event);  // prints "Event: 42"
```

**Embedded use case:** Hardware abstraction layers (HAL) use function pointers for interrupt callbacks, driver interfaces, and state machine transitions.

### Question 16: Array of function pointers

```c
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }

int (*ops[])(int, int) = {add, sub, mul};

printf("%d\n", ops[0](10, 5));  // 15
printf("%d\n", ops[1](10, 5));  // 5
printf("%d\n", ops[2](10, 5));  // 50
```

**Embedded use case:** Command dispatch tables, opcode handlers, state machine action tables.

---

## Quick Reference

| Trap | Symptom | Fix |
|---|---|---|
| Array decay in function params | `sizeof` returns pointer size | Pass size explicitly |
| Dangling pointer after return | Reading garbage / crash | Never return address of local |
| Use-after-free | Random corruption | Set pointer to NULL after free |
| `void *` arithmetic | Compilation error (or GCC silent) | Cast to `char *` first |
| Modifying `const` via cast | UB — cached stale value | Don't circumvent const |
| `int *p[10]` vs `int (*p)[10]` | Wrong type, wrong offset | Read declarations right-to-left |
| Pointer subtraction units | Off-by-sizeof factor | Results are in elements, not bytes |
