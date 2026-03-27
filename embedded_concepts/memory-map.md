# Memory Map

> A memory map describes how an embedded system's address space is divided among Flash, RAM, peripherals, boot ROM, external memory, and special regions. Good firmware work depends on understanding where every access goes.

This file explains the MCU hardware address-space view. For C runtime sections such as `.text`, `.data`, `.bss`, heap, and stack, see [memory-layout.md](../c_core/memory-layout.md).

---

## What a Memory Map Is

A memory map is the address-level view of the system.

Example layout:

```text
0x00000000  Boot alias / vector mapping
0x08000000  Internal Flash
0x20000000  SRAM
0x40000000  Peripheral registers
0x60000000  External memory / device region
0xE0000000  System control space
```

The exact values depend on the MCU family.

---

## Why It Matters

You use the memory map when you:
- place code and data with the linker
- access memory-mapped peripherals
- debug faults at raw addresses
- size stack, heap, and buffers
- design bootloader and application partitions
- configure MPU or cache policy

---

## Typical Regions

| Region | Typical contents |
|---|---|
| Flash / ROM | code, constants, vector table, stored initializers |
| SRAM | stack, heap, `.data`, `.bss`, DMA buffers |
| Peripheral space | UART, GPIO, SPI, timers, ADC, watchdog registers |
| System space | NVIC, SysTick, debug blocks, fault status registers |
| External memory | QSPI Flash, SDRAM, SRAM, FPGA windows |

---

## Flash Region

Flash is non-volatile and usually slower to write than RAM.

Typical uses:
- firmware image
- bootloader
- calibration tables
- stored configuration defaults

Typical constraints:
- erase by sector/page, not by byte
- limited program/erase cycles
- alignment requirements for erase and program operations

---

## SRAM Region

SRAM is volatile working memory.

Typical contents:
- stack
- heap
- global writable variables
- RTOS task stacks
- communication buffers
- DMA descriptors and payloads

Firmware often fails because SRAM use is underestimated rather than because Flash fills first.

---

## Peripheral Region

Peripherals appear as memory addresses.

```c
#define GPIOA_BASE 0x48000000U
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
```

These are not normal RAM locations.
Reading or writing them can:
- change hardware state
- acknowledge events
- clear flags
- trigger transfers

---

## System Control Space

On Cortex-M devices, high addresses include core control blocks.

Examples:
- NVIC interrupt controller
- SysTick timer
- SCB fault and configuration registers
- DWT trace/debug support

This region matters heavily during fault analysis.

---

## Address Aliasing

Some systems expose the same physical memory at multiple addresses.

Examples:
- boot alias at `0x00000000`
- Flash remapped to vector space
- cached and uncached views of the same memory

This can confuse debugging if you do not realize two addresses refer to the same underlying storage.

---

## Memory-Mapped Peripheral Blocks

Peripherals usually occupy register blocks with documented offsets.

```c
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0x48000000U)
```

The reference manual defines the layout. Your code mirrors it.

---

## Memory Protection and Attributes

Some MCUs provide an MPU or cache configuration per region.

Region attributes may define:
- executable or non-executable
- privileged or unprivileged access
- cached or non-cached
- bufferable or non-bufferable
- read-only or read-write

This matters for:
- protecting stacks and guard regions
- isolating tasks
- DMA buffers that must bypass cache
- blocking accidental code execution from RAM

---

## DMA and Memory Regions

DMA engines may not reach every address equally.

Common issues:
- DMA cannot access tightly coupled memory
- cacheable RAM causes stale data unless cleaned or invalidated
- buffer alignment is required
- descriptor tables must be in specific SRAM banks

The memory map is part of DMA correctness.

---

## Bootloader and Application Partitioning

Many embedded products split Flash into regions.

Example:

| Region | Address range |
|---|---|
| Bootloader | `0x08000000 - 0x08007FFF` |
| Application | `0x08008000 - 0x0806FFFF` |
| Settings | `0x08070000 - 0x08071FFF` |
| Backup image | `0x08072000 - ...` |

This partition must be reflected consistently in:
- linker scripts
- update logic
- vector relocation
- documentation

---

## Fault Debugging with the Memory Map

When you see a faulting address, classify the region first.

Examples:
- address in peripheral space: maybe invalid register access
- address near stack top: likely stack overflow or bad pointer
- address near `0x00000000`: null pointer or wrong vector/branch
- address in Flash during write: perhaps illegal execution or Flash routine issue

The first question is often not "what line failed" but "what region is this address in".

---

## Common Questions

**Q: Why are peripheral registers part of the memory map?**

Because the CPU accesses them using normal load and store instructions to fixed addresses.

**Q: Why is the memory map important for linker scripts?**

The linker must place sections only into valid physical regions with correct runtime behavior.

**Q: Why might a DMA buffer need special placement?**

Because not all RAM regions are equally reachable or cache-safe for DMA hardware.

---

## Quick Revision

- The memory map is the system's address-space blueprint.
- Flash, SRAM, peripherals, and system control each occupy distinct regions.
- Peripheral addresses are not ordinary memory.
- Bootloader partitioning and DMA behavior depend on region layout.
- Fault debugging starts by identifying the address region.
