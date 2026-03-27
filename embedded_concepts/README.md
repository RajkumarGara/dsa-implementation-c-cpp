# Embedded Concepts

Core embedded systems notes for interview prep first, with enough depth to stay useful later in day-to-day firmware work.

## Foundations

| # | File | What's Inside |
|---|---|---|
| 1 | [memory-map.md](memory-map.md) | Flash/RAM/peripheral regions, address-space reasoning, DMA and MPU context |
| 2 | [registers.md](registers.md) | Register types, field updates, read-modify-write hazards, side effects, reserved bits |
| 3 | [const-volatile.md](const-volatile.md) | `const`, `volatile`, `restrict`, register access, ISR-shared state |
| 4 | [endianness.md](endianness.md) | Byte order, parsing bugs, protocol conversions, safe byte assembly |

## Startup And Image Layout

| # | File | What's Inside |
|---|---|---|
| 5 | [boot-process.md](boot-process.md) | Reset flow, vector table, startup code, C/C++ runtime init, bootloader handoff |
| 6 | [linker-scripts.md](linker-scripts.md) | MEMORY and SECTIONS, placement, symbols, stack/heap layout, `KEEP`, custom sections |

## Runtime System Behavior

| # | File | What's Inside |
|---|---|---|
| 7 | [interrupts.md](interrupts.md) | NVIC basics, priorities, latency, nesting, masking, ISR design rules |
| 8 | [clocks-and-reset.md](clocks-and-reset.md) | Oscillators, PLL, bus clocks, peripheral enables, reset sources, clock tree bugs |
| 9 | [dma.md](dma.md) | DMA flow, buffer ownership, circular mode, cache/coherency, ISR interaction |
| 10 | [low-power.md](low-power.md) | Sleep/stop/standby modes, wakeup sources, power-performance tradeoffs |

## Cross-Cutting Patterns

| # | File | What's Inside |
|---|---|---|
| 11 | [embedded-specific.md](embedded-specific.md) | Ring buffers, watchdog use, fixed-point arithmetic, weak symbols, noinit sections, atomics |

## Recommended Order

- Start with memory map, registers, and const/volatile.
- Read boot process and linker scripts together because startup depends on linker-defined addresses and symbols.
- Add interrupts and clocks next because most MCU bring-up problems happen there.
- Read DMA and low power after the basics because both depend on memory, concurrency, and peripheral behavior.
- Use the patterns page as a quick reference, not as the main source for core concepts.
