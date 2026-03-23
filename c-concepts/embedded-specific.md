# Embedded-Specific C Concepts

> Patterns, idioms, and pitfalls specific to bare-metal and RTOS-based embedded software development.

---

## Memory-Mapped I/O

Hardware peripherals are accessed as memory addresses. The standard C idiom:

```c
#define REG (*(volatile uint32_t *)0x40000000)

REG = 0xFF;              // write to the register
uint32_t val = REG;      // read from the register
```

### Breaking it down

```
(volatile uint32_t *)0x40000000
│         │              │
│         │              └── Hardware address (integer)
│         └── Cast to pointer to uint32_t
└── volatile: don't optimize away reads/writes

*(...)
└── Dereference: access the value at that address
```

### Struct overlay for register blocks

```c
typedef struct {
    volatile uint32_t CR;     // Control Register    (offset 0x00)
    volatile uint32_t SR;     // Status Register     (offset 0x04)
    volatile uint32_t DR;     // Data Register       (offset 0x08)
    volatile uint32_t BRR;    // Baud Rate Register  (offset 0x0C)
} UART_TypeDef;

#define UART1 ((UART_TypeDef *)0x40011000)

UART1->CR = 0x01;          // enable UART
while (!(UART1->SR & 0x80)); // wait for TX empty
UART1->DR = 'A';           // send character
```

**Warning:** Use `#pragma pack` or `__attribute__((packed))` if your struct has padding that doesn't match the hardware layout. But verify alignment requirements first.

---

## volatile — When and Why

### Required for:

| Scenario | Why volatile |
|---|---|
| Hardware registers | Value changes due to hardware, not your code |
| ISR-shared variables | ISR modifies value asynchronously |
| DMA buffers | DMA engine writes to memory behind compiler's back |
| Memory-mapped peripherals | Reads have side effects (e.g., clearing flags) |

### Not sufficient for:

| Scenario | Also need |
|---|---|
| Atomic access | Disable interrupts or use atomic types |
| Multi-core coherence | Memory barriers (`__DMB()`, `__DSB()`) |
| Ordering guarantees | Compiler barriers (`asm volatile("" ::: "memory")`) |

### Compiler barrier vs memory barrier

```c
// Compiler barrier — prevents compiler reordering
asm volatile("" ::: "memory");

// ARM memory barriers — prevents CPU reordering
__DSB();  // Data Synchronization Barrier
__DMB();  // Data Memory Barrier
__ISB();  // Instruction Synchronization Barrier
```

---

## Interrupt Service Routines (ISR)

### ISR best practices

```c
// Keep ISRs SHORT
volatile uint8_t uart_rx_flag = 0;
volatile uint8_t uart_rx_data = 0;

void UART1_IRQHandler(void) {
    uart_rx_data = UART1->DR;  // read data (often clears interrupt flag)
    uart_rx_flag = 1;          // set flag for main loop
}

// Main loop processes the data
int main(void) {
    while (1) {
        if (uart_rx_flag) {
            uart_rx_flag = 0;
            process_data(uart_rx_data);
        }
    }
}
```

### ISR rules

| Do ✓ | Don't ✗ |
|---|---|
| Set flags | `printf` / any I/O |
| Copy data to buffer | `malloc` / `free` |
| Clear interrupt source | Long computations |
| Toggle GPIO for debug | Call non-reentrant functions |
| Increment counters | Blocking waits / delays |

### Shared variable protection

```c
// Critical section — disable interrupts
static inline uint32_t enter_critical(void) {
    uint32_t primask;
    asm volatile("MRS %0, PRIMASK\n"
                 "CPSID I" : "=r"(primask));
    return primask;
}

static inline void exit_critical(uint32_t primask) {
    asm volatile("MSR PRIMASK, %0" :: "r"(primask));
}

// Usage
uint32_t saved = enter_critical();
shared_counter++;  // safe — interrupts disabled
exit_critical(saved);
```

---

## Startup Code & Reset Handler

What happens before `main()`:

```
Power On / Reset
      │
      ▼
Reset_Handler (startup code)
      │
      ├── Initialize stack pointer (from linker script)
      ├── Copy .data from Flash to RAM
      ├── Zero-fill .bss
      ├── Initialize C library (optional)
      ├── Call SystemInit() (clock config)
      └── Call main()
```

### Linker script basics

```ld
MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS {
    .text : {           /* Code → Flash */
        *(.isr_vector)
        *(.text*)
        *(.rodata*)
    } > FLASH

    .data : {           /* Initialized data → RAM (loaded from Flash) */
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {            /* Uninitialized data → RAM (zeroed at startup) */
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
```

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
// Typical pattern
void watchdog_init(uint32_t timeout_ms) {
    WDG->TIMEOUT = timeout_ms;
    WDG->CR |= WDG_ENABLE;
}

void watchdog_feed(void) {
    WDG->RELOAD = WDG_RELOAD_KEY;  // "kick" the watchdog
}

int main(void) {
    watchdog_init(1000);  // 1 second timeout
    while (1) {
        do_work();
        watchdog_feed();  // must call before timeout expires
    }
}
```

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

**Warning on ARM:** Unaligned access to packed structs can cause a HardFault on Cortex-M0/M0+. Cortex-M3/M4/M7 handle unaligned access in hardware (with performance penalty).

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
| Memory-mapped register | `#define REG (*(volatile uint32_t *)ADDR)` |
| Register struct overlay | `#define PERIPH ((TypeDef *)BASE_ADDR)` |
| ISR communication | `volatile` flag + main loop polling |
| Critical section | Disable/restore interrupts |
| Ring buffer | Power-of-2 size + `& (SIZE-1)` modulo |
| Fixed-point math | Q16.16 with shift-based multiply/divide |
| Weak functions | `__attribute__((weak))` for defaults |
| Packed structs | `#pragma pack(1)` or `__attribute__((packed))` |
| Watchdog | Init + periodic feed in main loop |
