# Interrupts

> Interrupts let hardware get CPU attention immediately without constant polling. They are central to embedded responsiveness, but they also create the most common concurrency and timing bugs in firmware.

---

## What An Interrupt Is

An interrupt is a hardware or software event that temporarily pauses normal execution and transfers control to an interrupt service routine, or ISR.

```text
Main code running
      |
      v
Interrupt event occurs
      |
      v
CPU saves context
      |
      v
ISR runs
      |
      v
Context restored
      |
      v
Main code resumes
```

This gives low-latency response without wasting CPU time on constant checks.

---

## Common Sources Of Interrupts

- timer overflow or compare match
- UART RX/TX events
- GPIO edge detection
- ADC conversion complete
- DMA transfer complete or error
- watchdog early warning
- software-triggered interrupts

---

## Interrupt Life Cycle

An interrupt usually follows this sequence:

```text
Event occurs
      -> source sets a flag
      -> source and NVIC are enabled
      -> interrupt becomes pending
      -> CPU accepts it when masking and priority allow
      -> ISR runs
      -> ISR acknowledges or clears the source
      -> CPU returns to previous code
```

This is important because an interrupt can be pending without running yet.

---

## Cortex-M NVIC Basics

On Cortex-M systems, the NVIC manages interrupts.

Typical concepts:
- enable or disable a given IRQ
- set interrupt priority
- mark an interrupt as pending
- observe whether an interrupt is active
- support preemption based on priority rules

```c
NVIC_EnableIRQ(USART1_IRQn);
NVIC_SetPriority(USART1_IRQn, 3);
```

The exact API varies by vendor library, but the model is broadly the same.

---

## Interrupt Priority

Priority decides which interrupt can preempt another.

Important distinctions:
- lower numeric value may mean higher priority on many MCUs
- some bits are implemented, some are ignored
- preemption priority and subpriority may be split by configuration

Avoid treating this as a generic rule without tying it to the MCU architecture.

### Priority mistakes that cause real bugs

- assigning every IRQ the same priority so urgent work cannot preempt background work
- putting a slow ISR above a timing-critical ISR
- assuming all priority bits are implemented when the MCU only honors a subset
- confusing subpriority ordering with true preemption capability

Priority is a system design decision, not just a register setting.

---

## Pending vs Active vs Enabled

These are different states.

| State | Meaning |
|---|---|
| Enabled | the interrupt is allowed to be serviced |
| Pending | service has been requested but ISR has not started yet |
| Active | the ISR is currently executing |

Example:
- an interrupt can become pending while global interrupts are masked
- when interrupts are unmasked, it may then run
- once entered, it is active

This distinction shows up often in debugging.

---

## Latency

Interrupt latency is the time from event occurrence to first ISR instruction.

It depends on:
- current CPU state
- disabled interrupt windows
- higher-priority ISR activity
- stacking overhead
- memory wait states
- cache and bus behavior on more complex systems

Low latency is one reason interrupts exist, but it is never zero.

---

## Nesting And Preemption

A higher-priority interrupt may interrupt a lower-priority ISR.

```text
Main -> ISR A starts -> ISR B arrives with higher priority -> ISR B runs -> ISR A resumes
```

Implications:
- shared state between ISRs can still race
- stack usage increases with nesting depth
- long low-priority ISRs can still hurt system response

Some MCUs also reduce interrupt overhead with hardware features such as tail-chaining. You do not need deep architecture-specific detail to discuss this, but it is good to know that ISR entry and exit overhead is not always identical for every interrupt sequence.

---

## Masking Interrupts

Interrupts can be blocked at multiple levels.

### Global masking

```c
__disable_irq();
critical_update();
__enable_irq();
```

### Per-IRQ masking

```c
NVIC_DisableIRQ(USART1_IRQn);
```

### Peripheral-local masking

A peripheral often has its own interrupt enable bit in addition to NVIC control.

Important point:
- disabling one layer may stop ISR entry
- but the source flag may still get set and remain pending

This is why “interrupt disabled” does not always mean “event disappeared”.

---

## ISR Design Rules

Good ISR behavior:
- acknowledge or clear the interrupt source
- capture minimal data needed
- set a flag or queue work
- exit quickly

Avoid in ISRs:
- `printf` and blocking I/O
- dynamic allocation unless the system explicitly supports it safely
- long loops and heavy parsing
- taking locks that can deadlock
- anything that depends on non-reentrant code unless guaranteed safe

### Top half and bottom half pattern

Even without RTOS terminology, this pattern matters:
- ISR is the top half: acknowledge quickly and capture minimal state
- main loop or worker task is the bottom half: do the expensive processing

This is one of the best general answers to “how should an interrupt-driven design be structured?”.

---

## Typical ISR Pattern

```c
volatile uint8_t rx_ready = 0;
volatile uint8_t rx_byte = 0;

void USART1_IRQHandler(void) {
    rx_byte = USART1->DR;
    rx_ready = 1;
}

int main(void) {
    while (1) {
        if (rx_ready) {
            rx_ready = 0;
            process_byte(rx_byte);
        }
    }
}
```

This keeps the ISR short and pushes real work into the main loop or a task context.

For repeated data streams, a single flag is often too small a design. Use a ring buffer, queue, or DMA handoff to avoid overwriting data before the consumer catches up.

---

## Polling vs Interrupts

### Polling

```c
while ((UART1->SR & RXNE_FLAG) == 0U) {
}
byte = UART1->DR;
```

Good when:
- event frequency is high and predictable
- startup code is simple
- latency constraints are mild
- no other important work must run concurrently

### Interrupts

Good when:
- events are sporadic
- CPU should sleep or do other work
- response time matters
- multiple peripherals must coexist efficiently

The explanation should include tradeoffs, not just “interrupts are better”.

---

## Critical Sections

Interrupts create concurrency between main code and ISR context.

```c
uint32_t saved = enter_critical();
shared_counter++;
exit_critical(saved);
```

Use critical sections when an operation must not be interrupted. But keep them short, because they directly increase interrupt latency.

Good follow-up rule:
- protect only the smallest necessary region
- do not disable interrupts around heavy work
- prefer designs that minimize shared mutable state

---

## Volatile Is Necessary But Not Sufficient

```c
volatile uint32_t counter;
```

`volatile` ensures the compiler performs accesses, but it does not:
- make a multi-step operation atomic
- prevent races between ISR and main
- guarantee ordering across hardware domains by itself

Use atomics, critical sections, or hardware-specific synchronization where needed.

### Classic atomicity trap

```c
volatile uint32_t count;
count++;
```

`count++` is read, modify, write. It is not automatically atomic just because `count` is volatile.

---

## Shared Data Width And Atomicity

Whether access is atomic depends on:
- CPU word size
- alignment
- bus behavior
- the exact instruction sequence

Typical rule of thumb:
- naturally aligned single-word loads/stores are often atomic on simple MCUs
- multiword objects are not
- read-modify-write operations are not atomic without protection

Avoid absolute statements like “32-bit access is always atomic”.

---

## Common Interrupt Bugs

| Problem | Cause |
|---|---|
| ISR fires continuously | interrupt flag not cleared correctly |
| missed events | priority too low, masking too long, or bad flag handling |
| corrupted shared data | non-atomic access between ISR and main |
| stack overflow | nesting or oversized local ISR buffers |
| timing jitter | ISR too long or too many competing interrupts |
| strange deadlocks | ISR calling code that assumes task-only context |
| interrupt never fires | source enabled in one place but masked in another |
| event handled only once | source flag not acknowledged in the required sequence |
| duplicate servicing | pending condition remains set when ISR exits |

---

## Measuring ISR Timing

Useful techniques:
- toggle a GPIO at ISR entry and exit
- use timer capture
- use DWT cycle counter on Cortex-M where available
- inspect trace support if the MCU has it

A GPIO pulse on a scope is one of the fastest ways to measure ISR duration and jitter.

---

## Common Questions

**Q: What is the difference between pending and active?**

Pending means service has been requested. Active means the ISR is currently executing.

**Q: Why can an interrupt keep firing forever?**

Because the source flag was not cleared properly, or the triggering condition remains true.

**Q: Why is `volatile` not enough for ISR-shared data?**

Because it does not make read-modify-write operations atomic or eliminate races.

**Q: When would polling be better than interrupts?**

When the system is simple, timing is predictable, event rate is manageable, and interrupt overhead is not justified.

**Q: Why keep ISRs short?**

Because long ISRs increase latency, jitter, and the risk of missing higher-priority time-critical events.

**Q: What is the difference between polling and interrupts?**

Polling repeatedly checks status in software. Interrupts let hardware asynchronously notify the CPU when attention is needed.

## Quick Revision

- Interrupts improve responsiveness and CPU efficiency.
- Enabled, pending, and active are different interrupt states.
- NVIC priority and masking behavior matter as much as the ISR body.
- Keep ISRs short and defer work.
- Use critical sections or atomics for shared state.
- `volatile` does not make read-modify-write operations safe.
- Interrupt bugs are often concurrency bugs, not syntax bugs.
