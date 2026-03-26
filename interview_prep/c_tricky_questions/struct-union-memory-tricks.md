# Struct, Union, and Memory Layout Tricks

> For struct/union fundamentals (padding, packing, unions, bit fields), see [structs-and-unions.md](../../c_core/structs-and-unions.md). This file focuses on **tricky interview questions** that go beyond the concept explanations.

---

## Struct Padding and Alignment

> See [structs-and-unions.md](../../c_core/structs-and-unions.md) for padding rules and visualizations.

### Question 1: What is sizeof this struct?

```c
struct A { char c; int i; char d; };
```

**Answer:** `12` (not 6). Padding aligns `int` to 4-byte boundary + tail padding. Reordering to `{ int i; char c; char d; }` gives `8`.

### Question 2: Packed struct danger

```c
struct __attribute__((packed)) C { char c; int i; char d; };
int *p = &obj.i;  // What's the risk?
```

**Answer:** `sizeof` = 6, but `p` points to an unaligned address. Dereferencing causes a **HardFault on ARM Cortex-M0/M0+**. Use packed only for wire protocols.

### Question 3: Struct tail padding

```c
struct D {
    int i;       // 4 bytes
    char c;      // 1 byte + 3 bytes tail padding
};
// sizeof(struct D) = 8

struct E {
    struct D d;  // 8 bytes (includes D's tail padding)
    char x;      // 1 byte + 3 bytes tail padding
};
// sizeof(struct E) = 12
```

**Why tail padding:** The struct must be sized so that arrays of it maintain alignment: `sizeof(struct D) * n` must keep `i` aligned for element `n`.

---

## Unions

> See [structs-and-unions.md — Unions](../../c_core/structs-and-unions.md) for union basics and type punning.

### Question 4: Union with bit fields

```c
typedef union {
    struct {
        uint8_t mode : 2;
        uint8_t enable : 1;
        uint8_t irq : 1;
        uint8_t reserved : 4;
    } bits;
    uint8_t reg;
} ControlReg;

ControlReg cr;
cr.reg = 0;
cr.bits.mode = 0x03;
cr.bits.enable = 1;
printf("0x%02X\n", cr.reg);  // 0x07 (on little-endian)
```

**Warning:** Bit field ordering (MSB-first vs LSB-first) is **implementation-defined**. Always verify with your target compiler and document assumptions.

---

## Bit Fields

### Question 5: Portability traps

```c
struct Flags {
    unsigned int a : 1;
    unsigned int b : 3;
    unsigned int c : 4;
};
```

**Non-portable aspects:**
- Bit ordering within a byte (MSB or LSB first) — implementation-defined
- Whether bit fields can cross storage unit boundaries — implementation-defined
- Whether `int` bit fields are signed or unsigned — implementation-defined
- Padding between bit fields — implementation-defined

**Rule:** If bit-level layout must match hardware registers, use explicit masks and shifts instead of bit fields.

### Question 6: Bit field sign trap

```c
struct {
    int flag : 1;  // Can hold 0 or -1 (NOT 0 or 1!)
} s;

s.flag = 1;
if (s.flag == 1) {
    printf("equal\n");   // May NOT print!
}
```

**Why:** A 1-bit signed int can represent -1 (bit set) and 0 (bit clear). Setting it to 1 stores the bit pattern 1, which is interpreted as -1 in two's complement.

**Fix:** Always use `unsigned int` for bit fields:

```c
struct {
    unsigned int flag : 1;  // Can hold 0 or 1
} s;
```

---

## Flexible Array Members (C99)

### Question 7: Variable-length structs

```c
struct Packet {
    uint16_t length;
    uint8_t type;
    uint8_t data[];  // Flexible array member — must be last
};

// sizeof(struct Packet) = 4 (does NOT include data[])

struct Packet *pkt = malloc(sizeof(struct Packet) + payload_size);
pkt->length = payload_size;
memcpy(pkt->data, payload, payload_size);
```

**Rules:**
- Must be the last member
- Struct must have at least one other member
- `sizeof` does not include the flexible array
- Cannot declare arrays of such structs or nest them

**Embedded use case:** Variable-length protocol frames, log entries, message queues.

---

## offsetof and Container Tricks

### Question 8: offsetof macro

```c
#include <stddef.h>

struct Example {
    char a;
    int b;
    short c;
};

printf("%zu\n", offsetof(struct Example, a));  // 0
printf("%zu\n", offsetof(struct Example, b));  // 4 (after padding)
printf("%zu\n", offsetof(struct Example, c));  // 8
```

### Question 9: container_of (Linux kernel pattern)

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct Device {
    int id;
    struct ListNode node;  // embedded in a linked list
    char name[32];
};

// Given a pointer to `node`, get back to `Device`
struct Device *dev = container_of(node_ptr, struct Device, node);
```

**Embedded use case:** Intrusive linked lists (no separate allocation for list bookkeeping), widely used in RTOS kernels and the Linux kernel.

---

## Quick Reference

| Trap | What happens | Fix |
|---|---|---|
| Struct padding | sizeof larger than sum of members | Reorder members largest-first |
| Packed + unaligned access | Fault on strict-alignment CPUs | Only pack for wire formats |
| Bit field ordering | Layout differs across compilers | Use explicit masks/shifts |
| 1-bit signed int | Holds -1 and 0, not 1 and 0 | Use `unsigned int` |
| Flexible array sizeof | Doesn't include array size | Add payload size to malloc |
| Union type punning | UB in C++, OK in C99+ | Be explicit about target standard |
| Bit field portability | 6 implementation-defined behaviors | Avoid for hardware register mapping |
