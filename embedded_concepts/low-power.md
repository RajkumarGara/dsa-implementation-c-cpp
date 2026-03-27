# Low Power

> Power management is not a side topic in embedded systems. In battery-powered or thermally constrained products, low-power design is part of core firmware behavior, not an optimization after the fact.

---

## Why Low Power Matters

Low power affects:
- battery life
- thermal limits
- standby lifetime
- wakeup latency
- average current budget
- product viability in the field

A system that functions correctly but misses its power budget is still a failed design.

---

## Typical Power Modes

Names vary by MCU, but the pattern is similar.

| Mode | Typical state |
|---|---|
| Run | CPU and most clocks active |
| Sleep | CPU halted, many clocks still running |
| Stop / Deep sleep | most clocks off, RAM often retained |
| Standby / Shutdown | deepest power savings, more state lost |

Each deeper mode usually saves more power but costs more wakeup time and state restoration work.

---

## Sleep Vs Deeper Modes

### Sleep
- fast wakeup
- easy to enter and exit
- useful when events are frequent

### Stop or deep sleep
- more clock domains disabled
- PLL and fast oscillators may stop
- often needs clock reconfiguration after wake
- useful when idle time is longer

### Standby or shutdown
- highest savings
- often loses most runtime context
- usually wakes like a partial reboot

---

## Wakeup Sources

Common wakeup sources:
- external GPIO pin
- RTC alarm or wakeup timer
- watchdog or low-power timer
- communication peripheral activity
- comparator or analog threshold event

The wake source must remain powered and clocked in the chosen low-power mode.

---

## Firmware Pattern

```text
Finish outstanding work
Prepare wake sources
Quiesce peripherals
Save required context
Enter low-power mode
Wake on event
Restore clocks/peripherals/context
Resume work
```

If restore logic is incomplete, the system may wake but behave incorrectly.

---

## Clock Restoration After Wake

A very common bug pattern:
- MCU enters deep sleep
- PLL turns off
- system wakes on a low-speed fallback clock
- code resumes assuming the old clock tree still exists

Symptoms:
- UART baud incorrect after wake
- timers run at wrong rates
- communication only fails after sleep transitions

Low-power bugs often look like peripheral bugs because clocks changed underneath them.

---

## Retained Vs Lost State

Important question for every mode: what survives?

Possible retained state:
- some or all RAM
- backup registers
- RTC domain
- selected GPIO configuration

Possible lost state:
- PLL and clock tree setup
- peripheral enable state
- pending transfers
- CPU context in deepest modes

The reference manual, not intuition, decides this.

---

## Power-Saving Techniques Beyond Sleep Modes

- reduce clock frequency when full speed is unnecessary
- gate unused peripheral clocks
- disable unused GPIO pull-ups and analog blocks
- batch work to create longer idle windows
- use DMA instead of frequent CPU wakeups where appropriate
- avoid busy-wait loops when an interrupt can wake the core

Low-power design is both architecture and implementation.

---

## Polling And Power

Busy polling wastes power.

```c
while ((status_reg & READY_BIT) == 0U) {
}
```

An interrupt-driven wait or event-driven sleep usually saves much more power, especially in systems that are idle most of the time.

---

## Common Low-Power Bugs

| Problem | Cause |
|---|---|
| system does not wake | wake source not enabled or not valid in that mode |
| wakes immediately | pending flag not cleared before sleeping |
| communication broken after wake | clocks or peripherals not restored |
| current draw too high | some clock domain or peripheral never actually disabled |
| random state loss | firmware assumed RAM retention that the mode does not provide |

---

## Interview Questions

**Q: Why not always use the deepest power mode?**

Because deeper modes increase wake latency, restore complexity, and often lose more state.

**Q: Why can a system fail only after sleep/wake cycles?**

Because clocks, peripheral state, or retained memory assumptions may be wrong after wake.

**Q: How do interrupts help low-power design?**

They let the CPU sleep until real work arrives instead of burning power in polling loops.

---

## Quick Revision

- Low power is a core embedded design topic, not a polish step.
- Deeper modes save more power but cost more recovery work.
- Wake sources and retained state depend on the specific mode.
- Clock restoration after wake is a common failure point.
- Good low-power design combines sleep modes with event-driven firmware.
