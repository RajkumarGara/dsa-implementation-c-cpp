# Embedded Systems MCQs

> Multiple-choice questions covering embedded architecture, peripherals, RTOS concepts, and hardware-software interaction. Each question includes a detailed explanation.

---

## Memory & Architecture

### Q1. Which memory section stores uninitialized global variables?

A) `.text`  
B) `.data`  
C) **`.bss`** ✓  
D) `.rodata`

**Explanation:** Uninitialized global and static variables go in `.bss` (Block Started by Symbol). The startup code zeroes this region. `.data` stores initialized non-zero globals.

---

### Q2. What happens on a Harvard architecture when you try to execute data from RAM?

A) It works normally  
B) **It may not be possible — separate instruction and data buses** ✓  
C) Compilation error  
D) The OS prevents it

**Explanation:** Harvard architecture has separate memory spaces for instructions and data. Modified Harvard (like most ARM MCUs) may allow execution from RAM but pure Harvard cannot.

---

### Q3. What is the typical boot sequence of an ARM Cortex-M MCU?

A) main() → startup → interrupt init  
B) Bootloader → OS → main()  
C) **Reset vector → Stack pointer init → startup code → main()** ✓  
D) BIOS → bootloader → kernel → main()

**Explanation:** On reset, Cortex-M loads the initial stack pointer from address 0x00, then the reset vector from address 0x04, then executes startup code (zero `.bss`, copy `.data`), then calls `main()`.

---

### Q4. What is the purpose of the linker script in embedded development?

A) Compile source files  
B) **Define memory layout — where code and data are placed** ✓  
C) Configure the debugger  
D) Generate header files

**Explanation:** The linker script specifies memory regions (FLASH, RAM), section placement (.text, .data, .bss), stack/heap sizes, and the entry point.

---

### Q5. What is memory-mapped I/O?

A) Mapping files to memory  
B) Virtual memory paging  
C) **Accessing hardware registers through memory addresses** ✓  
D) DMA configuration

**Explanation:** Peripheral registers are assigned addresses in the memory map. Reading/writing these addresses controls hardware. `*(volatile uint32_t *)0x40000000 = 0xFF;`

---

## Volatile & const

### Q6. What does `volatile` tell the compiler?

A) The variable is constant  
B) The variable is thread-safe  
C) **The variable may change outside the program's control — don't optimize** ✓  
D) The variable is stored in registers

**Explanation:** `volatile` prevents the compiler from caching the value in a register or optimizing away reads/writes. Required for hardware registers and ISR-shared variables.

---

### Q7. What does `const volatile uint32_t *reg` mean?

A) Contradiction — can't be both  
B) Read-only, can't change  
C) **Read-only in code, but hardware can change it** ✓  
D) Mutable, cached in register

**Explanation:** `const` means code can't write to it. `volatile` means the value may change externally (e.g., a read-only status register that changes when hardware events occur).

---

### Q8. Which scenario does NOT require `volatile`?

A) Hardware status register  
B) Variable modified in an ISR  
C) DMA destination buffer  
D) **Local loop counter** ✓

**Explanation:** A local counter only accessed within one function scope has no external visibility. The compiler can safely optimize it.

---

## Interrupts

### Q9. What should you avoid doing inside an ISR?

A) Reading a register  
B) Setting a flag  
C) **Calling `printf()` or `malloc()`** ✓  
D) Clearing an interrupt flag

**Explanation:** `printf()` and `malloc()` are not reentrant, may use locks, and take unpredictable time. ISRs should be fast, deterministic, and avoid non-reentrant library calls.

---

### Q10. What happens if you don't clear the interrupt flag in an ISR?

A) Nothing — hardware clears it  
B) The interrupt fires once more then stops  
C) **The ISR is called repeatedly (infinite re-entry)** ✓  
D) The MCU resets

**Explanation:** Most peripherals keep the interrupt line asserted until the flag is explicitly cleared. The ISR will re-enter immediately after returning, starving the main loop.

---

### Q11. What is interrupt latency?

A) Time between two interrupts  
B) **Time from interrupt assertion to ISR execution** ✓  
C) Duration of the ISR  
D) Time to return from ISR

**Explanation:** Latency includes hardware recognition, context saving (stacking registers), and vector fetch. On Cortex-M, this is typically 12 cycles.

---

### Q12. What is the purpose of the NVIC on ARM Cortex-M?

A) Memory management  
B) Clock configuration  
C) **Nested interrupt priority management and enabling** ✓  
D) Bus arbitration

**Explanation:** The Nested Vectored Interrupt Controller manages interrupt priorities, enables/disables individual interrupts, and handles preemption based on priority levels.

---

### Q13. On ARM Cortex-M, which priority number is higher priority?

A) 15  
B) 7  
C) **0** ✓  
D) 255

**Explanation:** Lower numerical value = higher priority on ARM Cortex-M. Priority 0 is the highest (most urgent). This is opposite to some other architectures.

---

## Communication Protocols

### Q14. What is the key difference between SPI and I2C?

A) SPI is wireless, I2C is wired  
B) I2C is faster than SPI  
C) **SPI uses separate lines (MOSI/MISO/CLK/CS), I2C uses 2 lines (SDA/SCL)** ✓  
D) SPI supports only one device

**Explanation:** SPI is full-duplex, faster, uses 4+ lines (extra CS per slave). I2C is half-duplex, slower, uses 2 lines with addressing (multiple devices on same bus).

---

### Q15. What determines the SPI communication mode?

A) Baud rate and frame size  
B) **Clock polarity (CPOL) and clock phase (CPHA)** ✓  
C) Number of slave devices  
D) Data width only

**Explanation:** SPI has 4 modes (0–3) defined by CPOL (idle clock level) and CPHA (data sampling edge). Both master and slave must use the same mode.

---

### Q16. Why do I2C lines need pull-up resistors?

A) To increase speed  
B) To filter noise  
C) **Because I2C uses open-drain outputs — pull-ups provide the high state** ✓  
D) To set the I2C address

**Explanation:** I2C devices can only pull the line LOW (open-drain). The pull-up resistor pulls the line HIGH when no device is driving it low. This enables the wired-AND bus topology.

---

### Q17. What is the purpose of the UART start bit?

A) Error detection  
B) Address selection  
C) **Synchronize the receiver to the beginning of a data frame** ✓  
D) Set baud rate

**Explanation:** UART is asynchronous — no shared clock. The start bit (always low) tells the receiver that a data frame is beginning, allowing it to time subsequent bit samples.

---

## RTOS Concepts

### Q18. What is the difference between a mutex and a binary semaphore?

A) No difference  
B) Mutex is faster  
C) **Mutex has ownership and priority inheritance; binary semaphore doesn't** ✓  
D) Semaphore can only be used in ISRs

**Explanation:** A mutex can only be released by the task that acquired it (ownership). It supports priority inheritance to prevent priority inversion. A binary semaphore has no ownership concept.

---

### Q19. What is priority inversion?

A) Changing task priorities at runtime  
B) Running tasks in reverse order  
C) **A high-priority task blocked by a lower-priority task** ✓  
D) Stack overflow in a high-priority task

**Explanation:** A high-priority task waits for a resource held by a low-priority task, which is preempted by a medium-priority task. The fix is priority inheritance or priority ceiling.

---

### Q20. What does preemptive scheduling mean?

A) Tasks must explicitly yield the CPU  
B) Tasks run in round-robin order only  
C) **The scheduler can interrupt a running task to run a higher-priority one** ✓  
D) Only one task can run at a time

**Explanation:** In preemptive scheduling, the kernel can stop a lower-priority task at any point when a higher-priority task becomes ready, ensuring deadline-critical tasks run promptly.

---

### Q21. What is a critical section in RTOS context?

A) A section of code that crashes  
B) The main() function  
C) **Code that must execute atomically without interruption** ✓  
D) Code that runs in an ISR

**Explanation:** Critical sections protect shared resources. Implemented by disabling interrupts or suspending the scheduler. Should be as short as possible.

---

### Q22. What is the purpose of a message queue in an RTOS?

A) Task scheduling  
B) Memory allocation  
C) **Inter-task communication with buffering** ✓  
D) Interrupt handling

**Explanation:** Message queues let tasks send and receive typed messages. They provide buffering (multiple messages can be queued) and synchronization (tasks can block waiting for messages).

---

## Miscellaneous Embedded

### Q23. What is a watchdog timer?

A) A profiling tool  
B) A real-time clock  
C) **A timer that resets the MCU if not periodically serviced** ✓  
D) A debug breakpoint

**Explanation:** The watchdog detects software hangs. The application must periodically "kick" (reload) the watchdog. If software locks up and fails to kick, the watchdog resets the system.

---

### Q24. What is debouncing in the context of embedded systems?

A) Reducing power consumption  
B) **Filtering out rapid signal transitions from mechanical switches** ✓  
C) Calibrating ADC readings  
D) Managing interrupt priorities

**Explanation:** Mechanical switches bounce — producing multiple rapid transitions when pressed. Debouncing (in hardware or software) filters these to produce a single clean edge.

---

### Q25. What is the purpose of DMA?

A) Debug memory access  
B) Dynamic memory allocation  
C) Digital-to-analog conversion  
D) **Transferring data between memory and peripherals without CPU intervention** ✓

**Explanation:** DMA (Direct Memory Access) offloads data transfer from the CPU. The CPU sets up the transfer (source, destination, count) and the DMA controller handles it independently.

---

### Q26. What is the endianness of ARM Cortex-M by default?

A) Big-endian  
B) **Little-endian** ✓  
C) Mixed-endian  
D) Configurable only at design time

**Explanation:** ARM Cortex-M defaults to little-endian (least significant byte at lowest address). Some ARM cores support both (bi-endian), but Cortex-M typically only supports little-endian.

---

### Q27. What is a brownout reset?

A) Reset caused by software  
B) Reset caused by watchdog timeout  
C) **Reset triggered when supply voltage drops below a threshold** ✓  
D) Reset caused by overclocking

**Explanation:** The brownout detector monitors VDD. If voltage drops too low for reliable operation, it resets the MCU to prevent data corruption from running at insufficient voltage.

---

### Q28. What is the purpose of `__attribute__((weak))` in embedded C?

A) Makes the function run faster  
B) **Allows the function to be overridden by a non-weak definition** ✓  
C) Reduces code size  
D) Prevents optimization

**Explanation:** Weak symbols can be overridden by strong (normal) definitions. Used in startup code and HAL to provide default handlers that users can replace without modifying library code.

---

## Quick Reference: Key Facts

| Topic | Key fact |
|---|---|
| `.bss` | Uninitialized globals, zeroed at startup |
| `volatile` | Prevents compiler optimization of reads/writes |
| `const volatile` | Read-only in code, changeable by hardware |
| ISR rules | Fast, no blocking calls, clear flags |
| NVIC priority | Lower number = higher priority (Cortex-M) |
| SPI modes | Defined by CPOL and CPHA (4 combinations) |
| I2C pull-ups | Required because of open-drain outputs |
| Mutex vs semaphore | Mutex has ownership + priority inheritance |
| DMA | CPU-free data transfer |
| Watchdog | Resets system if not periodically kicked |
| Weak symbols | Default implementation, overridable |
