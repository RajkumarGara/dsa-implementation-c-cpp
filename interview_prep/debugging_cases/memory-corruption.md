# Memory Corruption Debugging

> Memory corruption bugs are the hardest class of embedded bugs — symptoms appear far from the root cause. These cases train you to recognize patterns and narrow down the source.

---

## Case 1: Stack Overflow

### Symptoms
- Random crashes, especially in deeply nested function calls
- Local variables appear corrupted
- Return address overwritten → jumps to garbage address
- HardFault on ARM Cortex-M

### The Bug

```c
void process_data(uint8_t *data, size_t len) {
    char buffer[64];   // 64 bytes on stack
    char temp[1024];   // 1024 bytes on stack — too large for embedded!

    memcpy(temp, data, len);  // if len > 1024, stack overflow
    parse(buffer, temp, len);
}
```

### How to Diagnose

1. **Fill stack with known pattern** at startup:

```c
// In startup code, fill stack region with 0xDEADBEEF
extern uint32_t _estack, _sstack;
uint32_t *p = &_sstack;
while (p < &_estack) {
    *p++ = 0xDEADBEEF;
}
```

2. **Check stack watermark** — find where the pattern ends:

```c
uint32_t stack_used(void) {
    extern uint32_t _sstack, _estack;
    uint32_t *p = &_sstack;
    while (*p == 0xDEADBEEF && p < &_estack) p++;
    return (uint32_t)(&_estack - p) * sizeof(uint32_t);
}
```

3. **Enable stack overflow detection** in RTOS (FreeRTOS: `configCHECK_FOR_STACK_OVERFLOW`)

### The Fix

- Move large buffers to static or heap allocation
- Reduce recursion depth
- Increase stack size in linker script or RTOS task config
- Use `-fstack-usage` (GCC) to analyze stack consumption per function

---

## Case 2: Buffer Overflow Corrupting Adjacent Variables

### Symptoms
- A variable changes value "on its own"
- Bug appears/disappears when adding/removing unrelated variables
- Printf debugging "fixes" the bug (changes memory layout)

### The Bug

```c
struct {
    char name[8];
    uint32_t status;
} device;

strcpy(device.name, "TEMPERATURE");  // 12 bytes into 8-byte buffer!
// "TURE" + null terminator overwrites device.status
```

### How to Diagnose

1. **Use a watchpoint (data breakpoint)** on the corrupted variable:

```
# GDB
(gdb) watch device.status
(gdb) continue
# Breaks when status is written — shows the offending code
```

2. **Check neighboring memory** in `.map` file — what's adjacent?

3. **Add canary values** around suspected variables:

```c
uint32_t canary1 = 0xCAFEBABE;
char buffer[64];
uint32_t canary2 = 0xCAFEBABE;

// After operations, check:
assert(canary1 == 0xCAFEBABE);
assert(canary2 == 0xCAFEBABE);
```

### The Fix

- Use `strncpy` / `snprintf` with explicit size limits
- Add bounds checking at system boundaries
- Use struct padding awareness to predict which members get corrupted

---

## Case 3: Use-After-Free

### Symptoms
- Data reads return correct values sometimes, garbage other times
- Bug depends on allocation patterns (appears under load)
- Different behavior in debug vs release builds

### The Bug

```c
struct Sensor *sensor = malloc(sizeof(struct Sensor));
sensor->id = 1;
sensor->value = 25.0f;

process_sensor(sensor);
free(sensor);

// ... later, in another part of the code ...
printf("Sensor %d: %.1f\n", sensor->id, sensor->value);  // USE-AFTER-FREE
// Memory may have been reallocated for something else
```

### How to Diagnose

1. **Poison freed memory:**

```c
void safe_free(void **ptr, size_t size) {
    if (*ptr) {
        memset(*ptr, 0xDD, size);  // fill with known pattern
        free(*ptr);
        *ptr = NULL;
    }
}
```

2. **On desktop:** Use Valgrind or AddressSanitizer:

```bash
gcc -fsanitize=address -g program.c -o program
./program
```

3. **On embedded:** Track allocations with wrapper functions that log caller address:

```c
void *debug_malloc(size_t size, const char *file, int line) {
    void *p = malloc(size);
    log_alloc(p, size, file, line);
    return p;
}
#define malloc(s) debug_malloc(s, __FILE__, __LINE__)
```

### The Fix

- Set pointers to NULL after free
- Use RAII-like patterns (allocate and free at the same scope level)
- Avoid dynamic allocation entirely on embedded (use static pools)

---

## Case 4: Unaligned Memory Access

### Symptoms
- HardFault on ARM Cortex-M0/M0+
- Works on Cortex-M3/M4 (supports unaligned), fails on M0
- Crash occurs only with packed structs or odd buffer offsets

### The Bug

```c
uint8_t rx_buffer[100];
// ... fill buffer from UART ...

// Cast to struct at arbitrary offset
struct __attribute__((packed)) Header {
    uint8_t type;
    uint32_t length;  // offset 1 — NOT 4-byte aligned!
};

struct Header *hdr = (struct Header *)&rx_buffer[3];
uint32_t len = hdr->length;  // UNALIGNED ACCESS — HardFault on M0
```

### How to Diagnose

1. **Check fault registers** on ARM:

```c
void HardFault_Handler(void) {
    uint32_t cfsr = SCB->CFSR;
    if (cfsr & SCB_CFSR_UNALIGNED_Msk) {
        // Unaligned access fault
        uint32_t pc = /* read stacked PC */;
        // pc tells you which instruction faulted
    }
}
```

2. **Enable unaligned access traps** even on Cortex-M3/M4 during testing:

```c
SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
```

### The Fix

- Use `memcpy` to safely copy from unaligned sources:

```c
uint32_t len;
memcpy(&len, &rx_buffer[4], sizeof(len));  // safe on all platforms
```

- Use byte-by-byte access for protocol parsing:

```c
uint32_t read_u32_le(const uint8_t *buf) {
    return (uint32_t)buf[0]       |
           (uint32_t)buf[1] << 8  |
           (uint32_t)buf[2] << 16 |
           (uint32_t)buf[3] << 24;
}
```

---

## Case 5: ISR / Main Context Race Condition

### Symptoms
- Intermittent data corruption
- Bug appears under high interrupt rate
- Adding delays or disabling optimizations "fixes" the bug

### The Bug

```c
volatile uint32_t adc_value;
volatile uint8_t data_ready;

void ADC_ISR(void) {
    adc_value = ADC->DR;  // write 32-bit value
    data_ready = 1;
}

void main_loop(void) {
    if (data_ready) {
        uint32_t val = adc_value;  // might read TORN value
        data_ready = 0;
        process(val);
    }
}
```

**Why it breaks:** On 8-bit or 16-bit MCUs, a 32-bit write is not atomic. The ISR can interrupt the main loop between reading the high and low bytes — you get half old, half new data.

### How to Diagnose

- Check if the corrupted values look like "mixed" data (top half old, bottom half new)
- Verify whether the data type exceeds the MCU's native word size

### The Fix

```c
void main_loop(void) {
    if (data_ready) {
        __disable_irq();
        uint32_t val = adc_value;
        data_ready = 0;
        __enable_irq();
        process(val);
    }
}
```

Or use a double-buffer / sequence counter pattern:

```c
volatile uint32_t adc_buf[2];
volatile uint8_t adc_idx;

void ADC_ISR(void) {
    adc_buf[!adc_idx] = ADC->DR;
    adc_idx = !adc_idx;
}

void main_loop(void) {
    uint32_t val = adc_buf[adc_idx];  // always reads consistent value
}
```

---

## Quick Reference: Debugging Techniques

| Bug class | Key symptom | Primary tool |
|---|---|---|
| Stack overflow | Crash in deep calls, corrupted locals | Stack watermark / canary fill |
| Buffer overflow | Adjacent variable corruption | Data watchpoint (GDB) |
| Use-after-free | Intermittent garbage reads | Valgrind / ASan / poison-on-free |
| Unaligned access | HardFault on strict-alignment MCU | CFSR register, `memcpy` fix |
| ISR race condition | Intermittent tearing, timing-dependent | Disable IRQ / atomic access |
| Heap fragmentation | Allocation failure after long uptime | Static pools, track high-water mark |
