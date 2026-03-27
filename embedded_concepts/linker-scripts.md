# Linker Scripts

> The linker script decides where code and data live in memory. In embedded systems, that means it directly controls startup behavior, RAM usage, interrupt vector placement, and whether the image can even boot.

---

## What a Linker Script Does

A linker script tells the linker:
- what memory regions exist
- where each section should be placed
- which symbols should mark section boundaries
- how the final image maps to Flash and RAM

In hosted systems this is often hidden. In firmware it is critical.

---

## MEMORY Block

The `MEMORY` block describes available regions.

```ld
MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}
```

| Field | Meaning |
|---|---|
| `ORIGIN` | start address of the region |
| `LENGTH` | size of the region |
| `rx` / `rwx` | intended permissions |

Typical mapping:
- Flash for code and constants
- RAM for writable data, stack, and heap

---

## SECTIONS Block

The `SECTIONS` block places output sections into memory.

```ld
SECTIONS {
    .text : {
        *(.isr_vector)
        *(.text*)
        *(.rodata*)
    } > FLASH

    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
```

This means:
- `.text` and read-only data execute from Flash
- `.data` lives in RAM but is stored in Flash in the image
- `.bss` lives in RAM and is zeroed at startup

---

## Important Symbols

The linker can define symbols used by startup code.

```ld
_sidata = LOADADDR(.data);
_sdata = ADDR(.data);
_edata = ADDR(.data) + SIZEOF(.data);
_sbss = ADDR(.bss);
_ebss = ADDR(.bss) + SIZEOF(.bss);
```

Startup code uses these to:
- copy `.data` from load address to runtime address
- clear `.bss`

If these symbols are wrong, startup breaks before `main()`.

---

## Load Address vs Run Address

Embedded firmware often stores data in one place and runs it in another.

Example:
- `.data` initial bytes are stored in Flash
- `.data` variables are used from RAM after reset

That is why `.data` often appears as:

```ld
.data : { *(.data*) } > RAM AT > FLASH
```

`AT > FLASH` gives the load address.
`> RAM` gives the runtime address.

---

## Vector Table Placement

The interrupt vector table usually must be placed at a specific address or section.

```ld
.text : {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
} > FLASH
```

Why `KEEP` matters:
- link-time garbage collection may otherwise discard an apparently unused vector table
- the reset path depends on it existing exactly where expected

---

## Stack and Heap Layout

Some linker scripts explicitly reserve them.

```ld
_estack = ORIGIN(RAM) + LENGTH(RAM);

.heap (NOLOAD) : {
    __heap_start__ = .;
    . = . + 0x1000;
    __heap_end__ = .;
} > RAM

.stack (NOLOAD) : {
    __stack_limit__ = .;
    . = . + 0x1000;
    __stack_top__ = .;
} > RAM
```

Many projects instead define only `_estack` and let the runtime manage the rest.

---

## Custom Sections

Firmware often places specific objects into dedicated memory.

```c
__attribute__((section(".noinit")))
uint8_t retained_buffer[256];
```

```ld
.noinit (NOLOAD) : {
    *(.noinit*)
} > RAM
```

Uses:
- retained RAM across soft reset
- DMA buffers with placement constraints
- boot metadata
- calibration tables

---

## Alignment

Linker scripts can enforce section alignment.

```ld
.data : {
    . = ALIGN(4);
    *(.data*)
    . = ALIGN(4);
} > RAM AT > FLASH
```

Alignment matters for:
- CPU access rules
- DMA engines
- Flash programming granularity
- vector table requirements

---

## Common Embedded Mistakes

| Problem | Cause |
|---|---|
| HardFault before `main()` | vector table or stack placement wrong |
| initialized globals corrupted | `.data` load/run addresses mismatched |
| RAM overflow | sections larger than physical RAM |
| bootloader overwrites app | overlapping Flash regions |
| retained data unexpectedly zero | section not marked `NOLOAD` |

---

## Bootloader Context

With bootloaders, linker scripts often offset the application image.

```ld
FLASH (rx) : ORIGIN = 0x08008000, LENGTH = 480K
```

This means:
- bootloader occupies lower Flash
- application vectors and code start later
- VTOR or equivalent relocation may be required

---

## Inspecting the Result

Useful outputs:
- `.map` file for final placement and sizes
- `objdump -h` for section headers
- `nm` for symbol addresses

These are the first places to check when memory usage or startup behavior looks wrong.

---

## Interview Questions

**Q: Why is `.data` placed in RAM `AT > FLASH`?**

Because it must be writable at runtime but its initial values come from the Flash image.

**Q: Why use `KEEP(*(.isr_vector))`?**

To prevent the linker from discarding the vector table during section garbage collection.

**Q: What is the difference between load and execution address?**

Load address is where bytes are stored in the image. Execution address is where the section is used at runtime.

---

## Quick Revision

- Linker scripts define memory layout, not just build metadata.
- `MEMORY` defines regions; `SECTIONS` places content.
- Startup code depends on linker-defined symbols.
- `.data` often loads from Flash and runs from RAM.
- Wrong linker placement can break firmware before any application code runs.
