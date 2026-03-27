# Registers

> Registers are the software-facing control surface of hardware peripherals. Embedded programming is often the discipline of reading the right register, setting the right bits, and not disturbing anything else.

---

## What a Register Is

A register is a memory-mapped hardware location used to:
- configure a peripheral
- observe status
- provide data
- clear or acknowledge events

```c
#define UART_SR (*(volatile uint32_t *)0x40011000U)
#define UART_DR (*(volatile uint32_t *)0x40011004U)
```

These accesses go to hardware, not normal RAM.

---

## Common Register Categories

| Type | Typical purpose |
|---|---|
| Control register | enable features, select modes |
| Status register | report flags and state |
| Data register | receive or transmit values |
| Configuration register | timing, widths, polarity, routing |
| Interrupt register | mask, status, clear, priority |

A peripheral usually exposes a block of related registers.

---

## Register Access Styles

### Macro style

```c
#define GPIO_ODR (*(volatile uint32_t *)0x48000014U)
GPIO_ODR |= (1U << 5);
```

### Struct overlay style

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
GPIOA->ODR |= (1U << 5);
```

The struct form is easier to read when the layout exactly matches the reference manual.

---

## Bit Manipulation Basics

```c
#define BIT(n) (1U << (n))

reg |= BIT(3);        // set bit 3
reg &= ~BIT(3);       // clear bit 3
reg ^= BIT(3);        // toggle bit 3
if (reg & BIT(3)) {   // test bit 3
}
```

Most register programming reduces to these operations plus field masks.

---

## Field Write Pattern

For multi-bit fields, clear then set.

```c
#define UART_MODE_MASK   (0x3U << 4)
#define UART_MODE_SHIFT  4U

reg = (reg & ~UART_MODE_MASK) | (2U << UART_MODE_SHIFT);
```

Why this matters:
- preserves unrelated bits
- updates only the intended field
- avoids accidental mode corruption

---

## Read-Modify-Write Hazards

A common pattern is:

```c
reg = (reg & ~MASK) | VALUE;
```

This is safe only if the register supports normal read-modify-write behavior.

Hazards:
- status bits may clear on read
- hardware may change bits between read and write
- write-one-to-clear bits may be affected incorrectly
- another context may modify the register concurrently

Always check the reference manual before assuming RMW is safe.

---

## Special Register Behaviors

### Read-only

Writes are ignored or illegal.

### Write-only

Reading may return zero, garbage, or trigger faults.

### Write-one-to-clear

```c
STATUS = ERROR_FLAG;
```

Writing a `1` clears the flag. Writing back the entire register value can accidentally clear multiple flags.

### Clear-on-read

Reading the register itself clears some event bits.

### Self-clearing bits

A start bit may return to zero automatically after hardware accepts the command.

These details are where many firmware bugs come from.

---

## Reserved Bits

Reference manuals often mark bits as reserved.

Rules of thumb:
- preserve reserved bits unless the vendor says otherwise
- write documented reset values when required
- avoid writing literal full-register constants unless you control every bit

Safer pattern:

```c
reg = (reg & ~KNOWN_MASK) | new_bits;
```

---

## Atomic Set/Clear Registers

Many MCUs provide dedicated set and clear registers to avoid read-modify-write races.

Example idea:

```c
GPIOA_BSRR = (1U << 5);        // set pin 5
GPIOA_BSRR = (1U << (5 + 16)); // clear pin 5
```

Benefits:
- atomic bit updates
- no need to read ODR first
- safer in ISR or concurrent contexts

These hardware features are preferable when available.

---

## Volatile and Register Access

Registers should normally be declared `volatile`.

```c
volatile uint32_t *timer_sr = (volatile uint32_t *)0x40000010U;
```

Without `volatile`, the compiler may:
- cache the value
- remove repeated reads
- reorder operations in unsafe ways

But `volatile` does not make access atomic or synchronized across contexts.

---

## Register Programming Sequence

Many peripherals require ordered setup.

Typical sequence:
1. enable peripheral clock
2. configure pins or routing
3. write mode and timing registers
4. clear stale status flags
5. enable the peripheral
6. enable interrupts if used

Programming registers in the wrong order can produce behavior that looks random.

---

## Example: Polling a UART Flag

```c
#define UART_SR_TXE BIT(7)

while ((UART1->SR & UART_SR_TXE) == 0U) {
}
UART1->DR = 'A';
```

If `SR` is not volatile or if TXE is clear-on-read in some design, the code must be written differently. The manual decides the correct pattern.

---

## Interview Questions

**Q: Why are reserved bits important?**

Because undocumented values may cause undefined hardware behavior now or on future silicon revisions.

**Q: Why is read-modify-write sometimes unsafe?**

Because a read can have side effects or hardware may change bits between the read and the write.

**Q: Why use dedicated set/clear registers when available?**

They avoid races and preserve unrelated bits without needing a read of the base register.

---

## Quick Revision

- Registers are memory-mapped hardware control points.
- Use masks and shifts, not magic numbers.
- Never assume read-modify-write is safe without checking the manual.
- Reserved bits and side effects matter.
- `volatile` is necessary for registers but not sufficient for concurrency safety.
