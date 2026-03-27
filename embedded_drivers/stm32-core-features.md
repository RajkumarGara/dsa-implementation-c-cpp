# STM32 Core Features For Driver Development

> Short reference for the STM32 concepts you need before writing register-level drivers such as UART, ADC, CAN, SPI, I2C, timers, and GPIO.

---

## 1. Memory-Mapped Peripherals

STM32 peripherals are controlled through fixed memory addresses.

```c
#define USART1_BASE 0x40011000U
#define RCC_BASE    0x40023800U
```

You either:
- use register macros, or
- use a register struct overlay

```c
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)USART1_BASE)
```

---

## 2. RCC: Clock Enable Comes First

Most STM32 peripherals do nothing until their clock is enabled in RCC.

```c
RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
```

Typical rule:
- GPIO clock on AHB bus
- USART/SPI/I2C/TIM clock on APB bus
- exact register depends on STM32 family

If the clock is not enabled:
- writes appear ignored
- reads stay at reset values
- the peripheral looks dead

---

## 3. AHB / APB Bus Split

STM32 uses different buses for different peripheral groups.

Common pattern:
- AHB: GPIO, DMA, core high-speed blocks
- APB1: slower peripherals
- APB2: faster peripherals

Why you care:
- clock enable bits live in different RCC registers
- peripheral input clock affects baud rate and timer timing
- `PCLK1` and `PCLK2` may differ

---

## 4. GPIO Alternate Function Setup

Most communication peripherals need GPIO pins configured for alternate function mode.

Typical steps:
1. enable GPIO port clock
2. select alternate function mode
3. choose AF number in AFR register
4. set output type, speed, pull-up or pull-down if needed

For UART, this is required before TX/RX works.

---

## 5. NVIC And Interrupt Routing

Peripheral interrupt generation is usually two-step:
- enable interrupt inside the peripheral
- enable the IRQ in NVIC

```c
USART1->CR1 |= USART_CR1_RXNEIE;
NVIC_EnableIRQ(USART1_IRQn);
```

If only one side is enabled, the interrupt will not behave as expected.

---

## 6. Status Flags Matter

STM32 drivers are built around status flags.

For UART examples:
- `TXE`: transmit data register empty
- `TC`: transmission complete
- `RXNE`: receive data register not empty
- `ORE`, `FE`, `PE`: error flags

Polling drivers wait on these bits.
Interrupt-driven drivers enable interrupts tied to these bits.

---

## 7. Clear Flags Correctly

Some flags clear by:
- reading a register
- reading then writing another register
- writing 1 to a clear register
- writing 0 to a status bit

Do not assume all flags clear the same way. The reference manual decides this.

This is one of the most common register-level driver mistakes.

---

## 8. Baud Rate And Peripheral Clock

UART baud rate depends on the peripheral clock feeding that USART.

```text
baud setting = function(peripheral clock, desired baud)
```

If your clock-tree assumption is wrong, your UART driver will be wrong even if the code looks correct.

Before writing UART:
- know whether the peripheral is on APB1 or APB2
- know the actual peripheral clock frequency
- know which BRR formula the MCU family uses

---

## 9. Polling vs Interrupt-Driven Design

### Polling
- simpler
- good for first driver version
- CPU waits on status bits

### Interrupt-driven
- better CPU usage
- needed for asynchronous receive paths
- requires NVIC setup, ISR design, and shared-buffer handling

Recommended order:
1. blocking polling driver
2. interrupt-driven RX/TX
3. DMA if needed

---

## 10. Common STM32 Driver Bring-Up Order

For most peripherals:
1. enable peripheral clock in RCC
2. enable and configure GPIO port
3. configure alternate function pins if required
4. reset or configure peripheral registers
5. program timing or mode registers
6. clear stale flags
7. enable peripheral
8. enable interrupts if using interrupt mode

This order is more important than people think.

---

## 11. CMSIS vs HAL vs Bare Register Style

For this repo, prefer CMSIS-style register definitions and direct register access.

Why:
- close to hardware
- good for interviews
- easier to understand flag flow and register ownership
- less vendor-library noise

That means:
- use register structs and bit masks
- avoid hiding everything behind HAL calls
- keep APIs small and explicit

---

## 12. What You Need Before Writing Each Driver

| Peripheral | Minimum STM32 knowledge |
|---|---|
| UART | RCC clock, GPIO AF, BRR, TXE/RXNE/TC flags, NVIC |
| SPI | RCC clock, GPIO AF, CR1/CR2, TXE/RXNE/BSY flags |
| I2C | RCC clock, GPIO AF open-drain, timing register, START/STOP/ADDR flags |
| ADC | RCC clock, GPIO analog mode, channel selection, conversion complete flag |
| CAN | RCC clock, GPIO AF, mailbox/FIFO model, filters, TX/RX status |
| Timers | RCC clock, prescaler, ARR/CCR, update flag, NVIC |

---

## 13. Common Mistakes

- forgetting RCC clock enable
- configuring USART but not GPIO alternate function
- using wrong APB clock for baud calculation
- enabling peripheral interrupt but not NVIC, or vice versa
- clearing flags incorrectly
- ignoring error flags
- using blocking polling inside code paths that must stay responsive

---

## Quick Revision

- STM32 drivers are register and flag driven.
- RCC clock enable is the first check when hardware does nothing.
- GPIO alternate function setup is required for most serial peripherals.
- APB clock source matters for timing and baud calculations.
- Start with polling drivers, then add interrupts, then DMA.
