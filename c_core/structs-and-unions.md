# Structs & Unions

> Struct padding, alignment, and `memcmp` pitfalls matter constantly in embedded firmware. Union tricks are also common in low-level code.

---

## Struct Padding & Alignment

The compiler inserts **padding bytes** to align struct members to their natural boundaries.

### Example: Wasteful layout

```c
struct bad {
    char a;     // 1 byte
                // 3 bytes padding (align next int to 4-byte boundary)
    int b;      // 4 bytes
    char c;     // 1 byte
                // 3 bytes padding (struct size must be multiple of largest alignment)
};
// sizeof(struct bad) = 12
```

### Example: Optimized layout

```c
struct good {
    int b;      // 4 bytes
    char a;     // 1 byte
    char c;     // 1 byte
                // 2 bytes padding
};
// sizeof(struct good) = 8
```

**Rule of thumb:** Order members from largest to smallest type to minimize padding.

### Visualizing padding

```
struct bad layout:
Offset: 0    1    2    3    4    5    6    7    8    9    10   11
        [a]  [pad] [pad] [pad] [-------- b --------]  [c]  [pad] [pad] [pad]

struct good layout:
Offset: 0    1    2    3    4    5    6    7
        [-------- b --------]  [a]  [c]  [pad] [pad]
```

---

## Alignment Rules

- Each member is aligned to a boundary equal to its own size (or a platform-specific maximum)
- The struct's total size is a multiple of the **largest member's alignment**

| Type | Typical Alignment | Size |
|---|---|---|
| `char` | 1 | 1 |
| `short` | 2 | 2 |
| `int` | 4 | 4 |
| `long` (64-bit) | 8 | 8 |
| `float` | 4 | 4 |
| `double` | 8 | 8 |
| pointer | 4 or 8 | 4 or 8 |

---

## Packing Structs (Removing Padding)

```c
// GCC / Clang
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
};
// sizeof = 6 (no padding)

// MSVC
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    char c;
};
#pragma pack(pop)
```

**Warning:** Packed structs can cause **misaligned access** — UB on strict-alignment architectures (ARM Cortex-M0). Use with caution and only where needed (protocol parsing, hardware register maps).

---

## The memcmp Pitfall

```c
struct A {
    char x;
    int y;
};

struct A a = {1, 2};
struct A b = {1, 2};

if (memcmp(&a, &b, sizeof(struct A)) == 0) {
    // May be FALSE even though x and y are equal!
}
```

**Why:** Padding bytes contain **garbage** (uninitialized data). `memcmp` compares **all** bytes including padding.

### Fix: Compare member by member

```c
int structs_equal(const struct A *a, const struct A *b) {
    return a->x == b->x && a->y == b->y;
}
```

### Fix: Zero the struct before use

```c
struct A a;
memset(&a, 0, sizeof(a));  // zeroes padding too
a.x = 1;
a.y = 2;

struct A b = {0};  // zero-initializes all bytes including padding
b.x = 1;
b.y = 2;

memcmp(&a, &b, sizeof(struct A));  // now safe (both paddings are 0)
```

---

## Bit-Fields

Pack multiple values into specific bit widths within a struct.

```c
struct flags {
    unsigned int ready    : 1;   // 1 bit
    unsigned int error    : 1;   // 1 bit
    unsigned int mode     : 3;   // 3 bits (values 0-7)
    unsigned int reserved : 27;  // remaining bits
};
// sizeof = 4 (fits in one uint32_t)
```

### Bit-field pitfalls

- **Layout is implementation-defined** — bit order (MSB-first vs LSB-first) varies by compiler/platform
- Cannot take the address of a bit-field: `&flags.ready` is illegal
- Bit-fields are NOT portable for hardware register mapping — use explicit bit masks instead
- Only `int`, `unsigned int`, `signed int`, and `_Bool` are guaranteed to work

```c
// Portable alternative for hardware registers:
#define STATUS_READY   (1U << 0)
#define STATUS_ERROR   (1U << 1)
#define STATUS_MODE(x) (((x) & 0x7) << 2)

uint32_t reg = STATUS_READY | STATUS_MODE(3);
```

---

## Unions

All members share the **same memory** — size equals the **largest member**.

```c
union data {
    int i;
    float f;
    char c;
};
// sizeof(union data) = 4 (size of largest member: int or float)
```

Only one member is "active" at a time. Reading a different member than the one last written is **implementation-defined** in C (but commonly used for type-punning).

### Type-punning with unions

```c
union float_bits {
    float f;
    uint32_t u;
};

union float_bits fb;
fb.f = 3.14f;
printf("Bit pattern: 0x%08X\n", fb.u);  // inspect float bits
```

**Note:** This is technically implementation-defined in C99, but well-defined in C11 and widely supported by compilers.

### Embedded use: Protocol parsing

```c
union packet {
    uint8_t raw[8];
    struct {
        uint8_t id;
        uint8_t length;
        uint16_t crc;
        uint32_t data;
    } fields;
};

union packet pkt;
uart_read(pkt.raw, 8);
printf("ID: %d, Data: 0x%X\n", pkt.fields.id, pkt.fields.data);
```

**Caution:** Endianness and padding make this non-portable. Use byte-level parsing for truly portable code.

### Tagged union (discriminated union)

```c
enum type { TYPE_INT, TYPE_FLOAT, TYPE_STRING };

struct variant {
    enum type tag;
    union {
        int i;
        float f;
        char s[32];
    } value;
};

struct variant v;
v.tag = TYPE_INT;
v.value.i = 42;
```

---

## Flexible Array Members (C99)

A struct with an incomplete array as its last member:

```c
struct message {
    uint16_t length;
    uint8_t data[];  // flexible array member — no size
};

// Allocate dynamically
struct message *msg = malloc(sizeof(struct message) + 100);
msg->length = 100;
msg->data[0] = 0x01;  // OK — within allocated space
```

**Rules:**
- Must be the last member
- Struct must have at least one other member
- `sizeof(struct message)` does **not** include the flexible array
- Cannot declare arrays/local variables of this struct type

---

## Struct Assignment & Comparison

```c
struct point { int x, y; };

struct point a = {1, 2};
struct point b = a;           // OK — memberwise copy
b = a;                        // OK — assignment copies all members

if (a == b) { }               // ERROR — cannot compare structs with ==
if (memcmp(&a, &b, sizeof(a)) == 0) { }  // risky — padding!
```

---

## offsetof Macro

Returns the byte offset of a member within a struct.

```c
#include <stddef.h>

struct example {
    char a;
    int b;
    short c;
};

offsetof(struct example, a);  // 0
offsetof(struct example, b);  // 4 (after 3 bytes padding)
offsetof(struct example, c);  // 8
```

**Embedded use:** Calculate addresses of fields in memory-mapped structures.

---

## Quick Reference

| Topic | Key Point |
|---|---|
| Padding | Compiler adds bytes for alignment — order members large→small |
| `memcmp` on structs | Unsafe — padding bytes may differ |
| Packed structs | Remove padding but risk misaligned access |
| Bit-fields | Non-portable layout — prefer bit masks for HW registers |
| Unions | All members share memory — size = largest member |
| Flexible arrays | Incomplete array as last struct member (C99) |
| `offsetof` | Byte offset of member in struct |
| Struct `==` | Not allowed — compare member-by-member |
