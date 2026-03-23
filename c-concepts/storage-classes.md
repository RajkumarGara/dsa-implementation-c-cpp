# Storage Classes

> Storage classes control **lifetime**, **scope**, and **linkage** of variables. Interview favorites: `static` inside functions, `extern` across files, and the difference between scope and lifetime.

---

## The Four Storage Classes

| Keyword | Scope | Lifetime | Linkage | Default Init |
|---|---|---|---|---|
| `auto` | Block | Block (stack) | None | Garbage |
| `register` | Block | Block (stack) | None | Garbage |
| `static` | Block or File | Entire program | Internal (file-scope) / None (block-scope) | Zero |
| `extern` | File (or wider) | Entire program | External | Zero (if defined) |

---

## `auto` (Default for Local Variables)

Every local variable is implicitly `auto`. Almost never written explicitly.

```c
void foo(void) {
    auto int x = 5;   // same as: int x = 5;
    // lifetime: created on entry, destroyed on exit
    // scope: only within this function
}
```

**Note:** In C++11, `auto` means type inference. In C, it means automatic storage — completely different.

---

## `register`

Suggests that the variable be kept in a CPU register for fast access.

```c
void count(void) {
    register int i;
    for (i = 0; i < 1000000; i++) {
        // compiler may keep i in a register
    }
}
```

**Rules:**
- Cannot take the address: `&i` is illegal
- Compiler is free to ignore the hint (modern compilers optimize better without it)
- Mostly obsolete — rarely used today

---

## `static` — Two Different Meanings

### 1. Static local variable — persistent across calls

```c
void counter(void) {
    static int count = 0;  // initialized ONCE, persists across calls
    count++;
    printf("Called %d times\n", count);
}

counter();  // Called 1 times
counter();  // Called 2 times
counter();  // Called 3 times
```

- **Scope:** Only inside the function (block scope)
- **Lifetime:** Entire program (stored in Data/BSS segment, not stack)
- **Initialized:** Once, before `main()` starts (or on first call in C++)

**Embedded use:** Retaining state between ISR invocations or function calls without using globals.

### 2. Static at file scope — internal linkage

```c
// file: utils.c
static int helper_count = 0;       // only visible in utils.c
static void helper_func(void) {    // only visible in utils.c
    helper_count++;
}

void public_func(void) {           // visible to other files
    helper_func();
}
```

- **Scope:** Entire file (file scope)
- **Linkage:** Internal — cannot be accessed from other translation units
- **Purpose:** Encapsulation / information hiding in C (like `private` in OOP)

---

## `extern` — External Linkage

Declares that a variable or function is **defined elsewhere** (in another file).

### globals.h
```c
extern int shared_counter;  // declaration — no memory allocated
extern void init_system(void);
```

### globals.c
```c
#include "globals.h"
int shared_counter = 0;     // definition — memory allocated here
void init_system(void) { /* ... */ }
```

### main.c
```c
#include "globals.h"
shared_counter++;           // uses the variable defined in globals.c
init_system();
```

**Rules:**
- `extern` is a **declaration**, not a **definition**
- There must be exactly **one definition** across all files (One Definition Rule)
- Functions are implicitly `extern` (unless declared `static`)

### extern with const

```c
// constants.h
extern const int MAX_RETRIES;  // declaration

// constants.c
const int MAX_RETRIES = 3;     // definition
```

---

## Scope vs Lifetime vs Linkage

### Scope — Where you can **use the name**

| Scope | Visible in |
|---|---|
| Block | Within `{ }` braces |
| Function | Within the function (labels only) |
| File | From declaration to end of file |
| Program | Across multiple files (via `extern`) |

### Lifetime — How long the **memory exists**

| Lifetime | Duration | Storage |
|---|---|---|
| Automatic | Entry to exit of block | Stack |
| Static | Entire program | Data/BSS segment |
| Dynamic | `malloc` to `free` | Heap |

### Linkage — Can other files **see this name**?

| Linkage | Meaning |
|---|---|
| None | Only visible in its scope (local variables) |
| Internal | Visible within the current file (`static` at file scope) |
| External | Visible across files (`extern`, default for functions/global vars) |

---

## Tricky Examples

### Static in a header file (common mistake)

```c
// utils.h
static int counter = 0;  // Each .c file that includes this gets its OWN copy!
```

If `utils.h` is included by `a.c` and `b.c`, they each have a **separate** `counter`. This is almost never what you want.

### Extern in a function (valid but unusual)

```c
void foo(void) {
    extern int global_var;  // refers to global_var defined elsewhere
    global_var++;
}
```

### Static function in header (intentional)

```c
// Sometimes done for inline-like behavior in C89:
static int max(int a, int b) {
    return a > b ? a : b;
}
// Each .c file gets its own copy — increases code size
```

---

## Default Initialization

```c
// Global and static variables are zero-initialized by default
int global;                    // 0
static int file_static;       // 0
void foo(void) {
    static int local_static;  // 0
}

// Local (auto) variables are NOT initialized
void bar(void) {
    int local;    // GARBAGE — indeterminate value
    int *ptr;     // GARBAGE — wild pointer
}
```

---

## Quick Reference

| Scenario | Use |
|---|---|
| Variable persists across function calls | `static` local variable |
| Hide function/variable from other files | `static` at file scope |
| Share variable across files | `extern` declaration + one definition |
| Keep variable in CPU register | `register` (compiler may ignore) |
| Default local variable | `auto` (implicit, never write it) |
| Global constant shared across files | `extern const` in header, define in one `.c` |
