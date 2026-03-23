# Pointers

> Pointers are the backbone of C and embedded programming — mastering them is non-negotiable.

---

## Pointer Basics

```c
int x = 42;
int *p = &x;   // p stores the address of x
printf("%d\n", *p);  // 42 — dereferencing
```

- `&x` → address-of operator
- `*p` → dereference (value-at-address) operator

---

## Pointer Arithmetic

Pointer arithmetic moves in **units of the pointed-to type**, not bytes.

```c
int arr[] = {10, 20, 30, 40};
int *p = arr;

printf("%d\n", *(p + 2));  // 30 — moves 2 * sizeof(int) bytes forward
printf("%ld\n", &arr[3] - &arr[0]);  // 3 — pointer difference is in elements
```

**Key rules:**
- `p + n` moves `n * sizeof(*p)` bytes forward
- `p1 - p2` gives the number of **elements** between two pointers (must point into the same array)
- Adding two pointers is **illegal**

---

## Array Name vs Pointer

```c
int arr[5] = {1, 2, 3, 4, 5};
int *p = arr;

// These are equivalent
arr[2]    *(arr + 2)    p[2]    *(p + 2)    2[arr]

// But arr is NOT a pointer variable
sizeof(arr);  // 20 (5 * sizeof(int)) — gives total array size
sizeof(p);    // 8 (on 64-bit) — gives pointer size
```

**Watch out:** `arr` decays to a pointer in most expressions, but **not** with `sizeof`, `&`, or as a string literal initializer.

---

## Pointer to Pointer (Double Pointer)

```c
int x = 10;
int *p = &x;
int **pp = &p;

printf("%d\n", **pp);  // 10
```

**Common use cases:**
- Modifying a pointer inside a function (pass `int **`)
- Dynamic 2D arrays
- Linked list head modification

```c
void allocate(int **ptr) {
    *ptr = malloc(sizeof(int));
    **ptr = 42;
}

int *p = NULL;
allocate(&p);
printf("%d\n", *p);  // 42
free(p);
```

---

## Void Pointer (`void *`)

A generic pointer that can hold the address of any data type.

```c
int x = 10;
float f = 3.14;
void *vp;

vp = &x;
printf("%d\n", *(int *)vp);    // must cast before dereferencing

vp = &f;
printf("%.2f\n", *(float *)vp);
```

**Rules:**
- Cannot be dereferenced directly — must cast first
- Cannot do pointer arithmetic on `void *` (GNU extension allows it, treating as `char *`)
- `malloc()` returns `void *`

---

## NULL Pointer

```c
int *p = NULL;  // or int *p = 0;

if (p == NULL) {
    // safe — p doesn't point to valid memory
}
```

- Dereferencing NULL is **undefined behavior** (usually segfault)
- Always initialize pointers to `NULL` if you don't have a valid address yet
- In embedded: NULL is typically address `0x00000000`

---

## Dangling Pointer

A pointer that references memory that has been freed or gone out of scope.

```c
// Case 1: freed memory
int *p = malloc(sizeof(int));
*p = 42;
free(p);
// p is now dangling — still holds the old address
*p = 10;   // UNDEFINED BEHAVIOR

// Fix: set to NULL after free
free(p);
p = NULL;
```

```c
// Case 2: local variable out of scope
int *foo(void) {
    int x = 10;
    return &x;  // WARNING: x is destroyed after return
}
```

---

## Wild Pointer

An uninitialized pointer — points to a random/garbage address.

```c
int *p;       // wild pointer — uninitialized
*p = 10;      // UNDEFINED BEHAVIOR — writing to unknown memory
```

**Fix:** Always initialize pointers.

```c
int *p = NULL;  // safe
```

---

## Pointer to Array vs Array of Pointers

```c
int arr[5] = {1, 2, 3, 4, 5};

int (*p)[5] = &arr;     // pointer to an array of 5 ints
int *q[5];               // array of 5 int pointers
```

| Declaration | Meaning |
|---|---|
| `int (*p)[5]` | Pointer to an array of 5 ints |
| `int *p[5]` | Array of 5 pointers to int |

**Tip:** Read declarations using the *right-left rule* or use `cdecl.org`.

---

## Pointer to Function vs Function Returning Pointer

```c
int (*fp)(int, int);     // pointer to a function taking 2 ints, returning int
int *func(int, int);     // function taking 2 ints, returning pointer to int
```

See [function-pointers.md](function-pointers.md) for detailed coverage.

---

## Common Interview Questions

**Q: What is the output?**

```c
int arr[] = {10, 20, 30, 40, 50};
int *p = arr;
printf("%d\n", *p);        // 10
printf("%d\n", *(p + 3));  // 40
printf("%d\n", p[2]);      // 30
printf("%d\n", 2[p]);      // 30 — same as *(2 + p)
```

**Q: Difference between `char *s` and `char s[]`?**

See [strings-and-literals.md](strings-and-literals.md).

**Q: Can you do arithmetic on a `void *`?**

Not in standard C. GCC allows it as an extension (treats like `char *`). Always cast first for portability.

---

## Quick Reference

| Concept | Syntax | Notes |
|---|---|---|
| Declare pointer | `int *p;` | Always initialize |
| Address-of | `&x` | Get address of variable |
| Dereference | `*p` | Get value at address |
| Pointer to pointer | `int **pp;` | Used to modify pointer in function |
| Void pointer | `void *vp;` | Generic — cast before use |
| NULL pointer | `int *p = NULL;` | Safe sentinel value |
| Pointer arithmetic | `p + n` | Moves by `n * sizeof(*p)` bytes |
