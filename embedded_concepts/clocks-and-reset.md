# Clocks And Reset

> A microcontroller does not run from “one clock”. Real systems have clock sources, PLLs, bus dividers, peripheral enables, and reset paths. Many bring-up failures are really clock-tree or reset-state mistakes.

---

## Why Clocks Matter

Clocks determine:
- CPU speed
- bus timing
- UART baud generation
- timer tick rates
- ADC sampling timing
- SPI and I2C rates
- power consumption

If the clock tree is wrong, many peripherals appear broken even when the driver code is otherwise correct.

---

## Common Clock Sources

| Source | Typical use |
|---|---|
| Internal RC oscillator | fast startup, low cost, lower accuracy |
| External crystal or resonator | better accuracy, common for communication timing |
| PLL | multiplies a reference clock to a higher frequency |
| Low-speed RC or crystal | RTC, watchdog, low-power timing |

Tradeoffs are usually between startup time, accuracy, cost, and power.

---

## Basic Clock Tree Idea

```text
Clock source -> PLL / divider -> system clock -> bus clocks -> peripheral clocks
```

Typical branches:
- core clock
- AHB or system bus clock
- APB peripheral bus clocks
- dedicated peripheral kernels

Each stage can divide or multiply frequencies.

---

## Peripheral Clock Enable

A peripheral often needs its bus clock enabled before any register write works.

```c
RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
```

If this step is missing:
- register writes may do nothing
- reads may return reset values forever
- the peripheral appears “dead” even though the code compiles cleanly

---

## PLL Basics

A PLL takes a reference clock and generates a higher-frequency clock.

You generally configure:
- input source
- pre-divider
- multiplication factor
- output dividers

PLL use is common for reaching the final system clock. Misconfiguration can cause unstable behavior, boot failures, or peripherals running at unexpected speeds.

---

## Bus Clocks And Prescalers

Even if the CPU runs at one speed, peripheral buses may run slower.

Example idea:
- system clock = 120 MHz
- AHB = 120 MHz
- APB1 = 60 MHz
- APB2 = 120 MHz

Consequences:
- UART, SPI, I2C, and timers may derive timing from different buses
- a wrong assumed bus frequency causes wrong baud rates and timer periods

---

## Reset Sources

Systems can reset for different reasons.

Common sources:
- power-on reset
- external reset pin
- watchdog reset
- brown-out reset
- software reset
- lockup or fault-triggered reset on some systems

Knowing the reset source is often crucial in debugging field failures.

---

## Brown-Out And Supply Monitoring

A brown-out reset protects the MCU when supply voltage drops too low.

Without it:
- code may execute unpredictably
- Flash writes may corrupt data
- peripherals may misbehave in partial-power conditions

This is both a hardware reliability topic and a firmware debugging topic.

---

## Typical Clock Init Flow

```text
Reset state clock source
      |
      v
Enable desired oscillator
      |
      v
Wait until ready flag is set
      |
      v
Configure PLL and prescalers
      |
      v
Switch system clock source
      |
      v
Update flash wait states and dependent timing
```

The order matters. For example, switching to a faster clock without proper Flash wait states can crash the system.

---

## UART And Clock Accuracy

UART is a classic interview example.

If the peripheral clock is wrong, the baud rate is wrong.

Possible symptoms:
- framing errors
- random garbage characters
- communication works only at one temperature or board variant
- debug UART works under one build but not another after clock changes

This is why clock-tree understanding directly affects driver debugging.

---

## Timer Frequency Calculations

Typical timer rate depends on:
- timer input clock
- prescaler
- auto-reload value

Simple idea:

$$
\text{update frequency} = \frac{f_{timer}}{(PSC + 1)(ARR + 1)}
$$

If the timer bus clock assumption is wrong, all derived timing is wrong too.

---

## Low-Power Interaction

Clock configuration often changes in low-power modes.

Examples:
- PLL disabled in deep low-power states
- system wakes on a fallback oscillator first
- peripheral clocks need reinitialization after wake
- RTC stays alive on a low-speed domain

Low-power bugs are often really clock restoration bugs.

---

## Common Clock Bugs

| Problem | Cause |
|---|---|
| peripheral registers seem ignored | clock enable not set |
| wrong UART baud | wrong peripheral clock assumption |
| timer period off by 2x or 4x | bus prescaler misunderstanding |
| system unstable at higher speed | Flash wait states or voltage scaling wrong |
| device hangs during startup | waiting forever for oscillator ready |
| works on debugger, not on cold boot | clock init sequence differs from reset state |

---

## Interview Questions

**Q: Why do you need to enable a peripheral clock before using a peripheral?**

Because the peripheral logic is often held unclocked to save power, so register accesses alone are not enough.

**Q: Why can a UART driver fail even if its register settings look correct?**

Because the baud divisor depends on the actual peripheral clock, which may differ from the assumed one.

**Q: Why are clock tree questions important in embedded interviews?**

Because they connect software configuration directly to timing correctness, power, and bring-up reliability.

---

## Quick Revision

- Clock trees affect nearly every peripheral.
- CPU clock and peripheral clock are not always the same.
- Clock enables are a mandatory bring-up step.
- Reset source information is valuable in debugging.
- Many “driver bugs” are actually timing or clock configuration bugs.
