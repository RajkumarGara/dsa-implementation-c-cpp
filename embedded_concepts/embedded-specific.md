# Embedded Patterns And Pitfalls

> Quick-reference patterns that are useful in real firmware work but should not replace the dedicated core concept docs.

Use the dedicated topic docs for foundations: [memory-map.md](memory-map.md), [registers.md](registers.md), [const-volatile.md](const-volatile.md), [boot-process.md](boot-process.md), [linker-scripts.md](linker-scripts.md), [interrupts.md](interrupts.md), [dma.md](dma.md), and [low-power.md](low-power.md).

---

## Fixed-Point Arithmetic

When you don't have an FPU (or it's too slow):

```c
// Q16.16 fixed-point: 16 integer bits + 16 fractional bits
typedef int32_t fixed_t;
#define FIXED_SHIFT 16

#define INT_TO_FIXED(x)   ((fixed_t)(x) << FIXED_SHIFT)
#define FLOAT_TO_FIXED(x) ((fixed_t)((x) * (1 << FIXED_SHIFT)))
#define FIXED_TO_INT(x)   ((x) >> FIXED_SHIFT)
#define FIXED_TO_FLOAT(x) ((float)(x) / (1 << FIXED_SHIFT))

#define FIXED_MUL(a, b)   ((fixed_t)(((int64_t)(a) * (b)) >> FIXED_SHIFT))
#define FIXED_DIV(a, b)   ((fixed_t)(((int64_t)(a) << FIXED_SHIFT) / (b)))

fixed_t a = FLOAT_TO_FIXED(3.14);
fixed_t b = FLOAT_TO_FIXED(2.0);
fixed_t c = FIXED_MUL(a, b);         // ~6.28
printf("%.2f\n", FIXED_TO_FLOAT(c));  // 6.28
```

---

## Watchdog Timer

Resets the MCU if software hangs:

```c
void watchdog_init(uint32_t timeout_ms) {
    WDG->TIMEOUT = timeout_ms;
    WDG->CR |= WDG_ENABLE;
}

void watchdog_feed(void) {
    WDG->RELOAD = WDG_RELOAD_KEY;
}
```

Pattern: initialize once, then feed it from a healthy main loop or monitored task path. For interview-style explanation and reset-debug context, see [embedded-systems-mcqs.md](../interview_prep/mcqs/embedded-systems-mcqs.md) and [peripheral-timing-bugs.md](../interview_prep/debugging_cases/peripheral-timing-bugs.md).

---

## Circular Buffer (Ring Buffer)

The go-to data structure for ISR-to-main communication:

```c
#define BUF_SIZE 64  // must be power of 2 for mask trick

typedef struct {
    volatile uint8_t data[BUF_SIZE];
    volatile uint16_t head;  // write index (ISR writes)
    volatile uint16_t tail;  // read index (main reads)
} ring_buf_t;

static inline int ring_buf_put(ring_buf_t *rb, uint8_t byte) {
    uint16_t next = (rb->head + 1) & (BUF_SIZE - 1);
    if (next == rb->tail) return -1;  // full
    rb->data[rb->head] = byte;
    rb->head = next;
    return 0;
}

static inline int ring_buf_get(ring_buf_t *rb, uint8_t *byte) {
    if (rb->head == rb->tail) return -1;  // empty
    *byte = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) & (BUF_SIZE - 1);
    return 0;
}
```

**Why power of 2?** `& (SIZE - 1)` is a fast modulo operation — no division needed.

---

## Pragma Pack & Alignment

```c
// Remove struct padding for protocol/register mapping
#pragma pack(push, 1)
typedef struct {
    uint8_t  cmd;      // 1 byte
    uint16_t length;   // 2 bytes
    uint32_t payload;  // 4 bytes
} __attribute__((packed)) packet_t;  // GCC attribute (alternative)
#pragma pack(pop)
// sizeof(packet_t) = 7, not 8 or 12
```

Use this only when a binary layout must match exactly. For the interview-focused packed-struct failure case, see [struct-union-memory-tricks.md](../interview_prep/c_tricky_questions/struct-union-memory-tricks.md).

---

## Weak Functions

Allow default implementations that can be overridden:

```c
// Default (weak) implementation
__attribute__((weak)) void Error_Handler(void) {
    while (1);  // default: just hang
}

// User override in another file — linker picks this one
void Error_Handler(void) {
    log_error("Fatal error!");
    system_reset();
}
```

**Use case:** HAL libraries provide weak default handlers; users override only what they need.

---

## Placing Variables at Specific Addresses

```c
// GCC
volatile uint32_t config __attribute__((section(".noinit")));
// Survives soft resets (not zeroed by startup code)

// Place at specific address (via linker script)
volatile uint32_t boot_flag __attribute__((section(".boot_flags")));
```

---

## Atomic Operations (C11)

```c
#include <stdatomic.h>

atomic_int counter = 0;

// In ISR or thread:
atomic_fetch_add(&counter, 1);   // atomic increment

// In main:
int val = atomic_load(&counter); // atomic read
```

**Note:** Not all embedded compilers support C11 atomics. Alternative: GCC builtins:

```c
__atomic_add_fetch(&counter, 1, __ATOMIC_SEQ_CST);
```

---

## Quick Reference

| Topic | Key Pattern |
|---|---|
| Ring buffer | Power-of-2 size + `& (SIZE-1)` modulo |
| Fixed-point math | Q16.16 with shift-based multiply/divide |
| Weak functions | `__attribute__((weak))` for defaults |
| Packed structs | `#pragma pack(1)` or `__attribute__((packed))` |
| Persistent section | `.noinit` or custom linker section |
| Atomic counter | C11 atomics or compiler builtins |
| Watchdog | Init + periodic feed in main loop |
