# Interview Strategy & Common Topics

> A meta-guide for embedded systems interviews — what to expect, how to prepare, and the most frequently tested topics across companies.

---

## What Interviewers Are Looking For

### Technical competency tiers

| Level | What they test | Example questions |
|---|---|---|
| Junior | C basics, pointer arithmetic, memory layout | "What's `sizeof` this struct?", "What does `volatile` do?" |
| Mid | Debug methodology, peripheral drivers, RTOS basics | "How would you debug a UART sending garbage?", "Explain priority inversion" |
| Senior | System design, tradeoffs, safety/reliability patterns | "Design the firmware architecture for a battery-powered sensor node" |

### Beyond coding

- **Debugging ability** — Can you systematically narrow down a bug?
- **Hardware-software boundary** — Do you understand what the compiler and CPU actually do?
- **Communication** — Can you explain your reasoning? Whiteboard embedded diagrams?
- **Safety mindset** — Do you think about what happens when things go wrong?

---

## Most Frequently Asked Topics

### Tier 1: Almost guaranteed

- `volatile` — when, why, what it prevents (optimization)
- `const` vs `volatile` vs `const volatile`
- Pointer arithmetic and array decay
- `sizeof` struct with padding
- Stack vs heap
- Interrupt handling — ISR best practices
- Signed vs unsigned traps (`-1 > 1U`)
- Endianness — byte order in memory

### Tier 2: Very common

- Bit manipulation — set, clear, toggle, check
- UART/SPI/I2C differences and when to use each
- Ring buffer implementation
- Memory-mapped I/O — register access patterns
- RTOS basics — mutex vs semaphore, priority inversion
- Linked list operations (insert, delete, reverse)
- State machine design
- DMA — what, why, when

### Tier 3: Senior / specialized

- Linker scripts — sections, memory regions, placement
- Boot process — reset vector, startup code, `.bss` init
- Watchdog strategies and non-blocking architectures
- Cache coherence (Cortex-M7, multi-core)
- Power management — sleep modes, wake sources
- Safety standards awareness (MISRA, DO-178, IEC 62304)
- Real-time guarantees — WCET, determinism

---

## How to Approach Whiteboard / Live Coding

### Pattern for C questions

1. **Clarify the question** — Ask about platform, word size, endianness if relevant
2. **State any assumptions** — "I'll assume 32-bit ARM, little-endian"
3. **Walk through the code** — Talk out loud as you trace execution
4. **Identify the trap** — Most questions have one. State it explicitly
5. **Reference the standard** — "The C standard says this is undefined behavior because..."

### Pattern for debugging questions

1. **Reproduce** — "First, I'd try to reproduce it consistently"
2. **Isolate** — "I'd narrow it down to this subsystem by..."
3. **Hypothesize** — "Given the symptoms, I suspect..."
4. **Verify** — "I'd confirm by using [tool/technique]"
5. **Fix and validate** — "The fix would be X. I'd verify by..."

### Pattern for design questions

1. **Requirements** — "What are the constraints? Power? Memory? Real-time?"
2. **Architecture** — Draw the block diagram: tasks, ISRs, data flow
3. **Key tradeoffs** — "I'd choose X over Y because..."
4. **Error handling** — "If this fails, the system would..."
5. **Testing** — "I'd validate this by..."

---

## Common Mistakes in Interviews

### Technical mistakes

| Mistake | Better approach |
|---|---|
| Saying "it depends" without elaborating | Always follow up with the specific factors |
| Guessing at UB output | Say "this is undefined behavior" and explain WHY |
| Ignoring edge cases | State them: "What if the pointer is NULL?", "What if size is 0?" |
| Not considering embedded constraints | Always mention: stack size, ISR context, no OS, no printf |

### Communication mistakes

| Mistake | Better approach |
|---|---|
| Jumping to code immediately | Restate the problem, discuss approach first |
| Going silent while thinking | Narrate your thought process |
| Not asking clarifying questions | "What MCU? What RTOS? Any memory constraints?" |
| Over-engineering the solution | Start simple, optimize only if asked |

---

## Study Checklist

### C Core

- [ ] Pointer arithmetic — all operators (`*`, `&`, `->`, `[]`)
- [ ] Array vs pointer — sizeof, decay, function parameters
- [ ] String handling — `char[]` vs `char*`, `strlen` vs `sizeof`
- [ ] Structs — padding, alignment, `packed`, `offsetof`
- [ ] Unions — size, type punning, endianness inspection
- [ ] Bit fields — portability issues, signed trap
- [ ] Macros — parenthesization, `do/while(0)`, stringification
- [ ] Integer promotions and signed/unsigned conversions
- [ ] `volatile`, `const`, `static`, `extern`, `register`
- [ ] Undefined behavior — top 5 cases
- [ ] `malloc`/`free` — double free, memory leaks, fragmentation

### Embedded

- [ ] Memory-mapped I/O — `*(volatile uint32_t *)addr`
- [ ] Interrupt handling — NVIC, ISR naming, flag clearing
- [ ] Communication protocols — UART, SPI, I2C, CAN basics
- [ ] DMA — setup, cache coherence, transfer complete handling
- [ ] Watchdog — configuration, kicking, timeout sizing
- [ ] Boot process — vector table, startup code
- [ ] Linker script sections — `.text`, `.data`, `.bss`, `.rodata`
- [ ] Power modes — sleep, stop, standby, wake sources

### RTOS

- [ ] Task states — ready, running, blocked, suspended
- [ ] Mutex vs semaphore — ownership, priority inheritance
- [ ] Priority inversion — cause and fix
- [ ] Deadlock — four conditions, prevention strategies
- [ ] Inter-task communication — queues, notifications, event groups
- [ ] Stack sizing — watermark checking, overflow detection

### Data Structures

- [ ] Linked list — insert, delete, reverse, detect cycle
- [ ] Circular buffer — enqueue, dequeue, full/empty detection
- [ ] Stack and queue — implementation, use cases
- [ ] Sorting — quicksort, mergesort, complexity comparison
- [ ] Binary search — iterative and recursive, off-by-one errors

---

## Recommended Practice Order

1. **Start with C tricky questions** — They overlap with everything else
2. **Work through embedded concepts** — volatile, registers, ISRs
3. **Build a ring buffer from scratch** — Combines C, pointers, and embedded patterns
4. **Implement a simple UART driver** — Register manipulation, ISR, buffering
5. **Practice debugging cases** — Narrate your approach to each scenario
6. **Run through MCQs** — Timed, 30 questions in 30 minutes
7. **Mock design question** — "Design the firmware for [X]" — practice on a whiteboard

---

## Quick Reference: Interview Tip Sheet

| Situation | Do this |
|---|---|
| Don't know the answer | Say what you DO know, reason through it |
| Question has a trap | Name the trap explicitly: "This is UB because..." |
| Asked about a protocol you haven't used | Explain the fundamentals, compare with one you know |
| Asked to optimize | First get it correct, THEN optimize |
| Debugging question | Follow the pattern: reproduce → isolate → hypothesize → verify |
| Design question | Start with constraints, draw a block diagram |
| Code won't compile (whiteboard) | Stay calm, trace through, fix syntax logically |
