# Endianness

> Endianness defines the byte order used to store multi-byte values in memory. It matters whenever firmware touches raw bytes, communication protocols, DMA buffers, or register fields wider than one byte.

---

## What Endianness Means

A 32-bit value such as `0x12345678` occupies four bytes.

| Byte order | Memory layout |
|---|---|
| Big-endian | `12 34 56 78` |
| Little-endian | `78 56 34 12` |

The value is the same. Only the in-memory byte ordering changes.

---

## Big-Endian vs Little-Endian

```text
Value: 0x12345678

Big-endian
Address:  +0   +1   +2   +3
Data:     12   34   56   78

Little-endian
Address:  +0   +1   +2   +3
Data:     78   56   34   12
```

Typical expectations:
- x86 and most Cortex-M systems are little-endian
- network byte order is big-endian
- some architectures support both

---

## Why Embedded Developers Care

Endianness bugs appear in:
- UART, SPI, I2C, CAN, and Ethernet protocol parsing
- sensor data reconstruction from byte streams
- Flash image headers and checksums
- DMA descriptors shared with peripherals
- memory dumps and debugger inspection

A bug may look like valid data with the wrong magnitude rather than an obvious crash.

---

## Detecting Endianness

```c
#include <stdint.h>

int is_little_endian(void) {
    uint16_t value = 0x0001;
    return *((uint8_t *)&value) == 0x01;
}
```

This is fine for learning and debugging. Production protocol code should usually not branch on host endianness if a deterministic byte-by-byte conversion can be used instead.

---

## Safe Byte Assembly

Prefer explicit assembly over casting byte buffers to wider pointer types.

```c
uint16_t read_be16(const uint8_t *buf) {
    return ((uint16_t)buf[0] << 8) |
           ((uint16_t)buf[1]);
}

uint32_t read_le32(const uint8_t *buf) {
    return ((uint32_t)buf[0]) |
           ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}
```

This is portable and avoids alignment issues.

---

## Unsafe Pattern: Pointer Reinterpretation

```c
uint32_t value = *(const uint32_t *)buf;
```

Problems:
- depends on host endianness
- may fault on unaligned addresses
- may violate strict aliasing assumptions

Use byte-wise parsing or `memcpy` when portability matters.

---

## Endianness in Register Access

For most memory-mapped registers on a given MCU, you use the native CPU endianness and let the hardware define field positions.

Example:

```c
volatile uint32_t *reg = (volatile uint32_t *)0x40001000;
uint32_t value = *reg;
```

You usually care more about bit positions than byte order here. Endianness becomes more visible when:
- reading register bytes individually
- inspecting memory in a debugger
- interfacing with peripherals that define serialized byte order

---

## Network Order

Protocols often define a fixed order independent of CPU architecture.

- Ethernet/IP/TCP/UDP typically use big-endian fields
- many sensor or MCU vendor protocols use little-endian fields
- file formats also define their own byte order

Always read the protocol spec and convert explicitly.

---

## Common Interview Traps

### Byte pointer inspection

```c
uint32_t value = 0x12345678;
uint8_t *p = (uint8_t *)&value;
printf("0x%02X\n", p[0]);
```

On a little-endian system, this prints `0x78`.

### Multi-byte reconstruction bug

```c
uint16_t x = (buf[0]) | (buf[1] << 8);
```

This decodes little-endian order. It is wrong if the protocol defines big-endian bytes.

### Sign extension confusion

```c
int8_t high = (int8_t)buf[0];
int16_t combined = (high << 8) | buf[1];
```

Signed types can introduce unwanted sign extension. Use unsigned types for raw byte assembly.

---

## Debugging Endianness Problems

Symptoms:
- `0x1234` becomes `0x3412`
- sensor values are plausible but scaled wildly wrong
- CRC or checksum matches only after swapping bytes
- protocol headers look reversed in dumps

Good checks:
- inspect raw bytes in the debugger
- compare against protocol examples
- print bytes before reconstructing words
- confirm whether the peripheral sends MSB-first or LSB-first on the wire

---

## Interview Questions

**Q: Is endianness about bit order or byte order?**

Byte order of multi-byte values in memory.

**Q: Why is network byte order important?**

Because systems with different CPU endianness must still exchange data in one agreed format.

**Q: Why avoid casting a `uint8_t *` buffer to `uint32_t *`?**

It can break due to endianness assumptions, alignment faults, and aliasing issues.

---

## Quick Revision

- Endianness is byte order, not value meaning.
- Protocols define their own order; never assume host order matches.
- Assemble integers explicitly from bytes.
- Use unsigned raw byte types for parsing.
- Debug raw bytes first when values look reversed.
