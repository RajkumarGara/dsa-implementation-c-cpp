# Boot Process

> A firmware image does not start at `main()`. The processor resets into a fixed entry path that sets up memory, runtime state, clocks, and interrupt vectors before application code can safely run.

---

## High-Level Reset Flow

```text
Power-on / Reset
      |
      v
Boot ROM or hardware reset logic
      |
      v
Vector table lookup
      |
      v
Reset_Handler
      |
      +-- Set stack pointer
      +-- Copy .data from Flash to RAM
      +-- Zero .bss
      +-- Optionally init C/C++ runtime
      +-- SystemInit() / clock setup
      +-- Call main()
```

The exact path depends on the MCU family, but the structure is broadly the same.

---

## Vector Table

Most MCUs begin execution by reading a vector table at a fixed address.

Typical Cortex-M layout:

| Offset | Meaning |
|---|---|
| `0x00` | Initial stack pointer value |
| `0x04` | Reset handler address |
| `0x08` | NMI handler |
| `0x0C` | HardFault handler |
| `...` | Other exception and IRQ handlers |

```c
__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    (void *)0x20010000,   // initial SP
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
};
```

If the vector table is wrong, the device may fault before any debug output is possible.

---

## Reset Handler Responsibilities

The reset handler is the first real firmware code.

```c
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void) {
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst++ = *src++;
    }

    for (dst = &_sbss; dst < &_ebss; ++dst) {
        *dst = 0;
    }

    SystemInit();
    main();

    while (1) {
    }
}
```

Key tasks:
- initialize RAM sections
- make global and static variables valid
- configure low-level hardware needed before `main()`
- branch into the application

---

## Why `.data` Must Be Copied

Initialized globals live in RAM at runtime, but their initial values are stored in Flash in the firmware image.

```c
int baud_rate = 115200;  // lives in RAM at runtime
```

At boot:
- the initial value `115200` exists in Flash
- startup code copies it into the RAM address for `baud_rate`

If this copy is skipped, initialized globals contain garbage.

---

## Why `.bss` Must Be Cleared

Uninitialized globals and statics are expected to start as zero.

```c
static uint8_t rx_count;
static uint32_t error_flags;
```

Startup code explicitly zeroes the `.bss` section. Without this step, code depending on zero initialization fails immediately and unpredictably.

---

## Boot ROM, Bootloader, and Application

### Boot ROM

Code burned by the chip vendor. It may:
- select a boot source
- support firmware download over UART/USB/CAN
- verify secure boot state

### Bootloader

User or vendor firmware that runs before the application. It may:
- validate the application image
- support field updates
- choose between multiple images
- jump to the application entry point

### Application

The main firmware image with your product logic.

```text
Reset -> Boot ROM -> Bootloader -> Application Reset_Handler -> main()
```

---

## Jumping From Bootloader to Application

A bootloader must transfer control cleanly.

Typical requirements:
- disable interrupts
- stop peripherals that would interfere
- set MSP or stack pointer to the application's initial SP
- relocate vector table if required
- jump to the application's reset handler

```c
typedef void (*entry_fn_t)(void);

void jump_to_app(uint32_t app_base) {
    uint32_t app_sp = *(uint32_t *)app_base;
    uint32_t app_reset = *(uint32_t *)(app_base + 4U);

    __disable_irq();
    __set_MSP(app_sp);
    SCB->VTOR = app_base;

    ((entry_fn_t)app_reset)();
}
```

If vector relocation or stack setup is wrong, interrupts and faults will land in the wrong image.

---

## C++ Runtime Considerations

C++ adds startup work beyond plain C.

Typical extras:
- run global constructors
- initialize static objects with nontrivial constructors
- optionally register destructors

This is why some embedded projects restrict or carefully control global objects in C++.

---

## Common Early-Boot Failures

| Problem | Typical Cause |
|---|---|
| HardFault before `main()` | bad vector table, invalid stack pointer, broken startup code |
| globals have wrong values | `.data` copy bug |
| static buffers not zero | `.bss` clear bug |
| interrupts go to wrong handlers | VTOR not set correctly |
| app works under debugger only | debugger initialized clocks or RAM differently |

---

## Interview Questions

**Q: Why does execution not start at `main()`?**

Because RAM, stack pointer, and runtime sections are not valid yet. Startup code must prepare them first.

**Q: What is the difference between bootloader and reset handler?**

The bootloader is a separate image that may select or validate firmware. The reset handler is the application entry point inside a specific image.

**Q: Why do embedded debugs often fail before `main()`?**

Because errors in vector tables, clock init, stack pointer setup, or memory initialization happen before normal application logic starts.

---

## Quick Revision

- Reset enters the vector table, not `main()`.
- Startup code copies `.data` and clears `.bss`.
- Boot ROM, bootloader, and application are distinct layers.
- Boot handoff must set stack pointer and vectors correctly.
- Many catastrophic firmware failures happen before the first line of application code.
