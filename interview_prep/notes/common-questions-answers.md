# Commonly Asked Interview Questions & Answers

> A curated set of the most frequently asked embedded C interview questions with concise, interview-ready answers. Practice saying these answers out loud.

---

## C Language Questions

### Q: What is the difference between `const` and `#define`?

**Answer:** `const` creates a typed, scoped variable with an address — it's visible to the debugger and respects scope rules. `#define` is a textual substitution by the preprocessor — it has no type, no scope, and no address. In embedded code, prefer `const` when possible because the compiler can type-check it and you avoid macro pitfalls (no parenthesization bugs).

Exception: `#define` is still needed for compile-time integer constants in `case` labels and array sizes (in C89).

---

### Q: What does `static` do in C? Give all three uses.

**Answer:**

1. **Static local variable** — retains its value across function calls (lifetime = program, scope = function)
2. **Static global variable** — limits visibility to the current translation unit (file scope, internal linkage)
3. **Static function** — same as above: function is only visible within the file it's defined in

```c
static int count = 0;         // file-scoped global
static void helper(void) { }  // file-scoped function

void foo(void) {
    static int calls = 0;     // persists across calls to foo
    calls++;
}
```

---

### Q: Explain the difference between `malloc`, `calloc`, and `realloc`.

**Answer:**

| Function | Behavior |
|---|---|
| `malloc(size)` | Allocates `size` bytes, uninitialized |
| `calloc(n, size)` | Allocates `n * size` bytes, zero-initialized |
| `realloc(ptr, size)` | Resizes existing allocation, may move data |

- `calloc` checks for overflow in `n * size` (safer than `malloc(n * size)`)
- `realloc(NULL, size)` behaves like `malloc(size)`
- `realloc(ptr, 0)` behavior is implementation-defined (may free or return NULL)
- On embedded systems, dynamic allocation is often avoided entirely — use static pools instead

---

### Q: What is a memory leak? How do you detect it?

**Answer:** A memory leak occurs when dynamically allocated memory is no longer referenced but not freed. The heap grows until allocation fails.

Detection:
- **On desktop:** Valgrind (`valgrind --leak-check=full ./program`), AddressSanitizer
- **On embedded:** Track allocations with wrapper functions that log and count alloc/free pairs. If counts don't match at a checkpoint, you have a leak
- **Prevention:** Avoid dynamic allocation in embedded. If used, follow RAII-like patterns: allocate and free at the same abstraction level

---

### Q: What is the difference between `++i` and `i++`?

**Answer:** Both increment `i` by 1. The difference is the expression value:

- `++i` (pre-increment): increments first, then returns the new value
- `i++` (post-increment): returns the current value, then increments

```c
int i = 5;
int a = ++i;  // a = 6, i = 6
int b = i++;  // b = 6, i = 7
```

In a standalone statement (`i++;` vs `++i;`), they're identical. For iterators/pointers in loops, prefer `++i` — in C++ it can avoid a copy.

---

## Embedded Systems Questions

### Q: What is volatile and when do you use it?

**Answer:** `volatile` tells the compiler that a variable's value can change at any time outside the program's normal flow. The compiler must:
- Re-read the variable from memory on every access (no caching in register)
- Not reorder, combine, or remove reads/writes

**Use it for:**
1. Hardware registers (memory-mapped I/O)
2. Variables modified in ISRs and read in the main loop
3. Variables modified by DMA
4. Shared variables in multi-threaded code (though volatile alone is not sufficient for thread safety)

**Not sufficient for:** atomicity (need disable interrupts or atomic types), memory barriers (need `__DMB()`/`__DSB()` on multi-core).

---

### Q: Explain the boot process of an ARM Cortex-M microcontroller.

**Answer:**

1. **Power-on / Reset** — CPU fetches initial stack pointer from address `0x00000000`
2. **Reset vector** — CPU loads the reset handler address from `0x00000004` and jumps to it
3. **Startup code** — typically written in assembly or provided by the vendor:
   - Copies `.data` section from Flash to RAM
   - Zeros the `.bss` section
   - Initializes the C runtime (sets up static constructors in C++)
4. **System init** — configures clocks (PLL, prescalers), disables watchdog temporarily
5. **`main()`** — application code begins

The vector table at address 0 contains: initial SP, then handler addresses for Reset, NMI, HardFault, and all peripheral interrupts.

---

### Q: How would you debug a system that randomly resets?

**Answer:** Systematic approach:

1. **Classify the reset** — Is it watchdog, brownout, HardFault, or power?
   - Read reset source register (RCC_CSR on STM32) at startup
   - Log it to persistent storage (Flash, EEPROM, or backup SRAM)

2. **If watchdog:** The main loop is hanging
   - Check for infinite loops, blocking calls, or deadlocks
   - Add heartbeat LED toggle and watchdog kick logging

3. **If HardFault:** Software bug
   - Implement a HardFault handler that captures PC, LR, and CFSR
   - The stacked PC tells you which instruction faulted

4. **If brownout:** Power supply issue
   - Check with oscilloscope for voltage dips
   - Add bulk capacitance, check regulator specs

5. **If random with no pattern:** Likely stack overflow or memory corruption
   - Fill stack with canary pattern, check periodically
   - Enable MPU stack guard region

---

### Q: What is the difference between UART, SPI, and I2C? When would you choose each?

**Answer:**

| Feature | UART | SPI | I2C |
|---|---|---|---|
| Lines | 2 (TX, RX) | 4+ (MOSI, MISO, CLK, CS) | 2 (SDA, SCL) |
| Duplex | Full | Full | Half |
| Speed | Up to ~1 Mbps | Up to 50+ MHz | Up to 3.4 MHz |
| Addressing | None (point-to-point) | CS per device | 7/10-bit address |
| Distance | Long (RS-232/485) | Short (board-level) | Short (~1m) |

**Choose UART** for: debug console, GPS modules, Bluetooth modules, inter-board communication  
**Choose SPI** for: high-speed devices (Flash, display, ADC), when pins are abundant  
**Choose I2C** for: many slow devices on shared bus (sensors, EEPROM), when pins are scarce

---

### Q: Explain DMA and when you would use it.

**Answer:** DMA (Direct Memory Access) transfers data between memory and peripherals without CPU intervention.

**How it works:**
1. CPU configures DMA: source address, destination address, transfer count, data width
2. DMA controller performs transfers triggered by peripheral requests or software
3. DMA signals completion via interrupt
4. CPU is free to execute code during the transfer

**Use DMA for:**
- High-throughput ADC sampling (continuous conversion to buffer)
- UART/SPI bulk transfers
- Memory-to-memory copies (faster than `memcpy` on some architectures)
- Audio streaming, display framebuffer updates

**Pitfalls:**
- Cache coherence on Cortex-M7 (clean/invalidate D-cache)
- Peripheral and DMA must agree on data width
- DMA stream/channel assignment is fixed per peripheral on many MCUs

---

## RTOS Questions

### Q: Mutex vs semaphore — what's the difference?

**Answer:**

| Aspect | Mutex | Binary Semaphore |
|---|---|---|
| Purpose | Mutual exclusion — protect resources | Signaling — synchronize tasks |
| Ownership | Yes — only the owner can release | No — any task can signal |
| Priority inheritance | Yes (in most RTOS) | No |
| Typical use | Protecting shared data structures | ISR-to-task signaling, event notification |

**Interview key point:** A mutex is like a key — only the holder can unlock. A semaphore is like a traffic signal — anyone can change it. Priority inheritance prevents priority inversion with mutexes, but not with semaphores.

---

### Q: What is a real-time system? Difference between hard and soft real-time?

**Answer:**

- **Real-time** = correctness depends on both the logical result AND the time at which it's produced
- **Hard real-time** = missing a deadline is a system failure (airbag deployment, pacemaker, flight controller)
- **Soft real-time** = missing a deadline degrades quality but isn't catastrophic (video streaming, audio playback)
- **Firm real-time** = late result has no value but doesn't cause failure (sensor reading for display update)

RTOS provides deterministic scheduling, but it doesn't make a system real-time by itself. You need worst-case execution time (WCET) analysis and proper task design.

---

### Q: How do you choose task stack sizes?

**Answer:**

1. **Use static analysis** — GCC's `-fstack-usage` reports per-function stack usage
2. **Calculate worst-case call depth** — trace the deepest call chain in each task
3. **Add margin** — typically 20–50% above measured worst case
4. **Monitor at runtime** — check high-water marks (`uxTaskGetStackHighWaterMark()` in FreeRTOS)
5. **Account for ISR stacking** — on Cortex-M, ISRs use the current task's stack (in some RTOS configs, a separate ISR stack is used)

**Rule of thumb:** Start with 512–1024 bytes for simple tasks, 2048+ for tasks using printf or complex parsing. Always measure — don't guess.

---

## Quick Reference: One-Line Answers

| Question | Answer |
|---|---|
| `volatile` | Prevents compiler from caching/optimizing — re-reads from memory every time |
| `static` local | Persists across calls, initialized once |
| `const volatile` | Code can't write it, but hardware can change it |
| Big vs little endian | Big: MSB first. Little: LSB first. Network = big, ARM = little |
| Stack vs heap | Stack: auto/local, LIFO, fast. Heap: dynamic, fragmentation risk |
| Mutex vs semaphore | Mutex: ownership + priority inheritance. Semaphore: signaling |
| ISR best practice | Read data, clear flag, signal task, return fast |
| Watchdog purpose | Reset system if software hangs |
| DMA purpose | CPU-free data transfer between memory and peripherals |
| Priority inversion | High-prio task indirectly blocked by low-prio task |
