# DMA

> DMA moves data between memory and peripherals without making the CPU touch every byte. It improves throughput and reduces CPU load, but it introduces ownership, coherency, and synchronization problems that normal polling code does not have.

---

## What DMA Is

DMA stands for Direct Memory Access.

Instead of the CPU doing:
- read peripheral register
- store to memory
- repeat for every element

DMA hardware performs the transfer on its own after setup.

```text
Peripheral <-> DMA controller <-> Memory
```

The CPU configures the transfer, then handles completion or errors.

---

## Why DMA Is Useful

DMA helps when:
- data volumes are large
- transfer timing is repetitive
- CPU should do other work or sleep
- peripherals stream continuously

Common use cases:
- UART RX/TX
- SPI transfers
- ADC sampling buffers
- DAC waveform output
- memory-to-memory copies on supported MCUs

---

## Basic DMA Flow

```text
Configure source address
Configure destination address
Configure transfer size and mode
Enable DMA request in peripheral
Start DMA channel/stream
Wait for completion interrupt or status flag
Process the finished buffer
```

DMA setup is usually more complex than polling, but better for sustained throughput.

---

## Peripheral-To-Memory Example

```c
// Conceptual setup only
DMAx->SRC = (uint32_t)&USART1->DR;
DMAx->DST = (uint32_t)rx_buffer;
DMAx->COUNT = RX_LEN;
DMAx->CTRL = PERIPH_TO_MEM | MINC_ENABLE | TCIE_ENABLE;
```

Typical configuration choices:
- direction
- memory increment on or off
- peripheral increment usually off
- transfer width
- circular or normal mode
- interrupt enables

---

## Normal Mode Vs Circular Mode

### Normal mode

Stops after a fixed transfer count.

Good for:
- packet-based transfers
- one-shot ADC capture
- finite SPI exchange

### Circular mode

Restarts automatically when the end of the buffer is reached.

Good for:
- continuous ADC sampling
- endless UART capture rings
- audio streams

Circular mode is powerful, but buffer ownership becomes more subtle.

---

## Buffer Ownership

The biggest DMA design question is often: who owns the buffer right now?

Possible owners:
- DMA engine writing into it
- CPU reading from it
- peripheral feeding DMA indirectly

Rules:
- do not let CPU modify a buffer region while DMA owns it
- use completion flags, half-transfer flags, or ping-pong buffers
- document ownership transitions explicitly

---

## Double Buffering

Double buffering reduces contention.

```text
DMA fills buffer A while CPU processes buffer B
then swap roles
```

This is common in:
- ADC acquisition
- audio systems
- high-rate communication streams

It gives predictable handoff points.

---

## DMA And Interrupts

DMA often works with interrupts rather than replacing them entirely.

Common DMA interrupts:
- transfer complete
- half transfer
- transfer error
- FIFO error on more advanced systems

Typical pattern:
- DMA moves the data
- ISR signals that a region is ready
- main loop or task processes it

---

## Cache And Coherency

On systems with caches, DMA can create stale-data bugs.

Possible issues:
- CPU cache still holds old buffer contents after DMA wrote new data
- CPU writes are still in cache and DMA reads stale RAM

Mitigations depend on platform:
- clean cache before DMA reads CPU-produced data
- invalidate cache after DMA writes memory for CPU use
- place DMA buffers in non-cacheable memory
- align buffers to cache-line boundaries when required

This is an advanced but very practical day-to-day firmware issue.

---

## Alignment And Reachability

DMA may have placement constraints.

Examples:
- buffer must be word-aligned
- descriptor table must be in a specific SRAM bank
- some memories are inaccessible to DMA
- transfer width must match peripheral requirements

The memory map matters directly here.

---

## Polling Vs Interrupt Vs DMA

### Polling
- simplest
- CPU cost highest
- okay for small or rare transfers

### Interrupt-driven byte handling
- better responsiveness
- still CPU-heavy at high data rates
- useful for moderate throughput

### DMA
- best for sustained throughput
- lowest per-byte CPU load
- highest setup and synchronization complexity

This comparison is a standard interview topic.

---

## Common DMA Bugs

| Problem | Cause |
|---|---|
| buffer contains old data | cache not invalidated or handoff bug |
| only first byte transfers | increment setting wrong |
| transfer never starts | peripheral DMA request not enabled |
| hard fault or bus fault | bad address, inaccessible memory, alignment issue |
| corrupted processing results | CPU and DMA used same buffer region simultaneously |
| wrong sample packing | transfer width mismatched to peripheral data width |

---

## Interview Questions

**Q: Why use DMA instead of interrupts?**

Because DMA removes per-element CPU servicing overhead and is better for sustained or high-throughput transfers.

**Q: Does DMA eliminate the need for interrupts?**

Usually no. Interrupts are still commonly used for transfer completion, half-transfer notifications, and error handling.

**Q: What is the hardest practical part of DMA?**

Usually synchronization and buffer ownership, not the basic register setup.

---

## Quick Revision

- DMA moves data without CPU touching each element.
- It is best for sustained or repeated transfers.
- Buffer ownership and coherency matter as much as register configuration.
- Circular and double-buffered modes are common in streaming use cases.
- DMA plus interrupts is a standard embedded pattern.
