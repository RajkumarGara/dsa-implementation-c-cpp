# Bitwise Operations

> Bit manipulation is the most essential embedded C skill — controlling hardware registers, flags, protocol fields, and optimizing for speed and memory.

---

## Basic Operators

| Operator | Symbol | Example | Result |
|---|---|---|---|
| AND | `&` | `0b1100 & 0b1010` | `0b1000` |
| OR | `\|` | `0b1100 \| 0b1010` | `0b1110` |
| XOR | `^` | `0b1100 ^ 0b1010` | `0b0110` |
| NOT | `~` | `~0b1100` | `0b...0011` (inverts all bits) |
| Left Shift | `<<` | `0b0001 << 2` | `0b0100` |
| Right Shift | `>>` | `0b1000 >> 2` | `0b0010` |

---

## Set, Clear, Toggle, Check a Bit

The four most common operations in embedded C:

```c
#define BIT(n) (1U << (n))

uint32_t reg = 0;

// SET bit n
reg |= BIT(3);        // reg = 0b00001000

// CLEAR bit n
reg &= ~BIT(3);       // reg = 0b00000000

// TOGGLE bit n
reg ^= BIT(3);        // flips bit 3

// CHECK if bit n is set
if (reg & BIT(3)) {
    // bit 3 is set
}
```

### Multi-bit mask operations

```c
#define MASK  0x0F   // lower nibble: bits 3:0

// Set bits in mask
reg |= MASK;

// Clear bits in mask
reg &= ~MASK;

// Read only masked bits
uint8_t nibble = reg & MASK;

// Write a value to a field (read-modify-write)
reg = (reg & ~MASK) | (new_value & MASK);
```

### Read-Modify-Write for hardware registers

```c
// Set bits 7:4 to 0x5, leave everything else unchanged
#define FIELD_MASK   (0xFU << 4)
#define FIELD_VAL(x) (((x) & 0xFU) << 4)

volatile uint32_t *REG = (volatile uint32_t *)0x40000000;
*REG = (*REG & ~FIELD_MASK) | FIELD_VAL(0x5);
```

---

## Shift Operators

### Left shift `<<`

```c
1 << 0   // 1   (0b00000001)
1 << 1   // 2   (0b00000010)
1 << 7   // 128 (0b10000000)

x << n   // equivalent to x * 2^n (for non-negative values without overflow)
```

### Right shift `>>`

```c
8 >> 1   // 4   (0b00000100)
8 >> 2   // 2   (0b00000010)

x >> n   // equivalent to x / 2^n (for unsigned or non-negative values)
```

### Shift traps

```c
1 << 32;     // UB if int is 32-bit (shift >= type width)
1 << -1;     // UB (negative shift)
-1 << 1;     // UB in C99/C11 (shifting negative signed values)
-1 >> 1;     // Implementation-defined (arithmetic vs logical shift)
```

**Rule:** Use **unsigned** types for bitwise operations to avoid surprises.

```c
1U << 31;    // OK — unsigned literal
(uint32_t)1 << 31;  // explicit — always safe
```

---

## Swap Without Temporary Variable

```c
// XOR swap
a ^= b;
b ^= a;
a ^= b;
// a and b are swapped
```

**Caveat:** Fails if `a` and `b` are the same variable (`a ^= a` = 0). Also, a regular temp swap is usually faster on modern CPUs.

```c
// Better in practice:
int temp = a;
a = b;
b = temp;
```

---

## Power of 2 Checks

```c
// Check if n is a power of 2
int is_power_of_2(unsigned int n) {
    return n && !(n & (n - 1));
}

// 8  = 0b1000, 7  = 0b0111 → 8 & 7 = 0 ✓
// 6  = 0b0110, 5  = 0b0101 → 6 & 5 = 4 ✗
```

### Round up to next power of 2

```c
uint32_t next_power_of_2(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
```

---

## Count Set Bits (Population Count)

```c
// Brian Kernighan's algorithm — O(number of set bits)
int count_bits(unsigned int n) {
    int count = 0;
    while (n) {
        n &= (n - 1);  // clears the lowest set bit
        count++;
    }
    return count;
}

// GCC builtin (uses hardware instruction if available)
int count = __builtin_popcount(n);      // for unsigned int
int count = __builtin_popcountl(n);     // for unsigned long
```

---

## Isolate, Extract, and Insert Bits

```c
// Isolate lowest set bit
int lowest_set = n & (-n);
// Example: n = 0b10110100 → lowest_set = 0b00000100

// Clear lowest set bit
n &= (n - 1);
// Example: n = 0b10110100 → n = 0b10110000

// Extract bits [high:low] from value
#define EXTRACT_BITS(val, high, low) \
    (((val) >> (low)) & ((1U << ((high) - (low) + 1)) - 1))

uint32_t field = EXTRACT_BITS(0xABCD1234, 15, 8);  // extracts bits 15:8 = 0x12

// Insert value into bits [high:low]
#define INSERT_BITS(reg, val, high, low) \
    (((reg) & ~(((1U << ((high) - (low) + 1)) - 1) << (low))) | \
     (((val) & ((1U << ((high) - (low) + 1)) - 1)) << (low)))
```

---

## Byte Extraction (Endianness Handling)

```c
uint32_t val = 0x12345678;

uint8_t byte0 = (val >> 0)  & 0xFF;   // 0x78 (LSB)
uint8_t byte1 = (val >> 8)  & 0xFF;   // 0x56
uint8_t byte2 = (val >> 16) & 0xFF;   // 0x34
uint8_t byte3 = (val >> 24) & 0xFF;   // 0x12 (MSB)

// Reconstruct from bytes
uint32_t rebuilt = ((uint32_t)byte3 << 24) |
                   ((uint32_t)byte2 << 16) |
                   ((uint32_t)byte1 << 8)  |
                   ((uint32_t)byte0);
```

---

## Embedded Register Patterns

```c
// Typical GPIO configuration register
#define GPIO_MODE_INPUT    0x0U
#define GPIO_MODE_OUTPUT   0x1U
#define GPIO_MODE_ALT      0x2U
#define GPIO_MODE_ANALOG   0x3U

#define GPIO_MODE_MASK(pin)    (0x3U << ((pin) * 2))
#define GPIO_MODE_SET(pin, m)  (((m) & 0x3U) << ((pin) * 2))

// Set pin 5 to output mode
volatile uint32_t *GPIOA_MODER = (volatile uint32_t *)0x48000000;
*GPIOA_MODER = (*GPIOA_MODER & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, GPIO_MODE_OUTPUT);
```

---

## Arithmetic vs Bitwise

| Arithmetic | Bitwise Equivalent |
|---|---|
| `x * 2` | `x << 1` |
| `x / 2` | `x >> 1` (unsigned only) |
| `x * 8` | `x << 3` |
| `x % 8` | `x & 7` (only for powers of 2) |
| `x % 2 == 0` | `(x & 1) == 0` |

**Note:** Modern compilers do these optimizations automatically. Prefer readable code — write `x / 2`, not `x >> 1`, unless the bitwise intent is meaningful.

---

## Quick Reference

| Operation | Code | Notes |
|---|---|---|
| Set bit n | `reg \|= (1U << n)` | |
| Clear bit n | `reg &= ~(1U << n)` | |
| Toggle bit n | `reg ^= (1U << n)` | |
| Check bit n | `reg & (1U << n)` | Nonzero if set |
| Lowest set bit | `n & (-n)` | Isolates it |
| Clear lowest set | `n & (n-1)` | Kernighan's trick |
| Power of 2? | `n && !(n & (n-1))` | |
| Swap via XOR | `a^=b; b^=a; a^=b;` | Don't use if a==b |
| Modulo power of 2 | `x & (n-1)` | Only when n is power of 2 |
