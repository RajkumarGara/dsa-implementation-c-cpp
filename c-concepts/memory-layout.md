# Memory Layout

> Understanding memory layout is fundamental for embedded developers — you need to know where every byte lives, how much space it takes, and how the hardware sees it.

---

## Program Memory Segments

```
High Address
┌─────────────────────┐
│    Command-line     │
│    args & env vars  │
├─────────────────────┤
│       Stack         │  ↓ grows downward
│  (local variables,  │
│   return addresses, │
│   function params)  │
├── ── ── ── ── ── ── ┤
│                     │
│    (free space)     │
│                     │
├── ── ── ── ── ── ── ┤
│       Heap          │  ↑ grows upward
│  (malloc, calloc)   │
├─────────────────────┤
│       BSS           │  Uninitialized globals/statics (zero-filled)
├─────────────────────┤
│       Data          │  Initialized globals/statics
├─────────────────────┤
│       Text (Code)   │  Machine instructions (read-only)
├─────────────────────┤
│    (Reserved/NULL)  │
└─────────────────────┘
Low Address
```

---

## Segment Details

### Text (Code) Segment
- Contains compiled machine instructions
- **Read-only** and **executable**
- Shared across processes running the same program
- In embedded: typically stored in **Flash/ROM**

### Data Segment (Initialized Data)
- Global and static variables with explicit initial values
- **Read-write** (mutable)
- In embedded: initial values stored in Flash, copied to RAM at startup by the C runtime

```c
int global_var = 42;           // Data segment
static int static_var = 100;   // Data segment
```

### BSS Segment (Block Started by Symbol)
- Global and static variables that are **uninitialized** or **zero-initialized**
- Does NOT occupy space in the binary on disk — just a size entry
- Zeroed out at program startup by the C runtime

```c
int global_uninit;             // BSS — initialized to 0
static int static_zero = 0;   // BSS — explicitly 0
```

**Embedded tip:** BSS is significant because it determines how much RAM is needed at startup. The startup code (`Reset_Handler` / `crt0`) zeros this region.

### Heap
- Dynamic memory (`malloc`, `calloc`, `realloc`, `free`)
- Grows **upward** toward higher addresses
- Managed by the programmer (or allocator)

**Embedded note:** Many embedded systems avoid heap entirely — use static allocation or memory pools instead due to fragmentation and determinism concerns.

### Stack
- Local variables, function parameters, return addresses
- Grows **downward** toward lower addresses
- **LIFO** — Last In, First Out
- Fixed size (typically 1-8 KB in embedded, 1-8 MB on desktop)

```c
void foo(void) {
    int local = 10;       // Stack
    int arr[100];          // Stack — 400 bytes!
    char buf[1024];        // Stack — watch out for overflow in embedded!
}
```

---

## Where Do Variables Live?

```c
#include <stdio.h>
#include <stdlib.h>

int global_init = 10;          // Data segment
int global_uninit;             // BSS segment
static int static_init = 20;  // Data segment
static int static_uninit;     // BSS segment
const int global_const = 30;  // Text/rodata segment

void foo(int param) {          // param: Stack
    int local = 5;             // Stack
    static int s_local = 7;   // Data segment (persists across calls)
    int *heap_ptr = malloc(4); // heap_ptr: Stack, *heap_ptr: Heap
    const int c = 10;          // Stack (const is just a qualifier)
    free(heap_ptr);
}
```

| Variable | Segment | Lifetime |
|---|---|---|
| `global_init` | Data | Entire program |
| `global_uninit` | BSS | Entire program |
| `static_init` | Data | Entire program |
| `static_uninit` | BSS | Entire program |
| `global_const` | Text/rodata | Entire program |
| `param` | Stack | During function call |
| `local` | Stack | During function call |
| `s_local` | Data | Entire program |
| `heap_ptr` (the pointer) | Stack | During function call |
| `*heap_ptr` (the data) | Heap | Until `free()` |

---

## sizeof Pitfalls

### Pointer size is not data size

```c
int arr[10];
int *p = arr;

sizeof(arr);    // 40 — total array size
sizeof(p);      // 8 — just the pointer (64-bit system)
```

### Arrays decay to pointers when passed to functions

```c
void foo(int arr[10]) {
    sizeof(arr);  // 8! — arr is actually a pointer here
}
```

### sizeof does not evaluate its operand

```c
int x = 5;
sizeof(x++);  // x is still 5 — expression is NOT evaluated
```

### Struct size ≠ sum of members (padding)

```c
struct example {
    char a;     // 1 byte
    int b;      // 4 bytes
    char c;     // 1 byte
};
// sizeof(struct example) = 12, not 6! (padding for alignment)
```

See [structs-and-unions.md](structs-and-unions.md) for details.

### Empty struct

```c
struct empty {};
sizeof(struct empty);  // 0 in C (undefined in standard), 1 in C++
```

---

## Stack Overflow

When the stack grows beyond its allocated size:

```c
// Recursive function without proper base case
void infinite(void) {
    int buf[1024];  // 4 KB per call
    infinite();     // stack overflow!
}

// Large local arrays
void risky(void) {
    int huge[100000];  // 400 KB on stack — almost certainly overflow in embedded
}
```

**Embedded mitigation:**
- Use static analysis to compute worst-case stack depth
- Enable stack canaries (`-fstack-protector`)
- Place a sentinel pattern at the stack boundary and check periodically
- Use RTOS stack overflow detection hooks

---

## Endianness

How multi-byte values are stored in memory.

```
Value: 0x12345678

Big Endian (MSB first):          Little Endian (LSB first):
Addr:  [0]  [1]  [2]  [3]       Addr:  [0]  [1]  [2]  [3]
Data:  0x12 0x34 0x56 0x78       Data:  0x78 0x56 0x34 0x12
```

| Architecture | Endianness |
|---|---|
| x86 / x86_64 | Little Endian |
| ARM Cortex-M | Little Endian (default, bi-endian capable) |
| ARM Cortex-A | Configurable |
| PowerPC | Big Endian |
| Network byte order | Big Endian |

### Detecting endianness at runtime

```c
int is_little_endian(void) {
    uint16_t val = 0x0001;
    uint8_t *byte = (uint8_t *)&val;
    return byte[0] == 0x01;
}
```

### Converting between endianness

```c
#include <arpa/inet.h>   // POSIX

uint32_t host_val = 0x12345678;
uint32_t net_val = htonl(host_val);   // host to network (big endian)
uint32_t back = ntohl(net_val);       // network to host
```

**Embedded tip:** When parsing communication protocols (UART, SPI, I2C), always be explicit about byte order. Use byte-level access instead of casting:

```c
// Safe — works on any endianness
uint32_t read_be32(const uint8_t *buf) {
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8)  |
           ((uint32_t)buf[3]);
}
```

---

## Alignment

- Most CPUs require (or prefer) variables to be aligned to their natural size
- `int` (4 bytes) should start at an address divisible by 4
- Misaligned access: **UB on strict architectures** (ARM Cortex-M0), **performance penalty** on others

```c
// Forcing alignment
int x __attribute__((aligned(16)));  // GCC: align to 16 bytes

// Checking alignment
#include <stdalign.h>       // C11
alignof(int);  // typically 4
```

---

## Quick Reference

| Question | Answer |
|---|---|
| Where are initialized globals? | Data segment |
| Where are uninitialized globals? | BSS segment (zeroed at startup) |
| Where are local variables? | Stack |
| Where does `malloc` go? | Heap |
| Where are string literals? | Text/rodata (read-only) |
| Does BSS take space in binary? | No — just records the size needed |
| Stack grows? | Downward (high → low addresses) |
| Heap grows? | Upward (low → high addresses) |
| `sizeof(array)` in function? | Gives pointer size, not array size! |
