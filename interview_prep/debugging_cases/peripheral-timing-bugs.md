# Peripheral & Timing Debugging

> Embedded peripheral bugs are notoriously hard — the code compiles, runs, and does nothing. These cases cover the most common ways GPIO, UART, timers, interrupts, and watchdogs misbehave.

---

## Case 1: GPIO Not Toggling

### Symptoms
- Pin stays low (or high)
- Logic analyzer shows no output
- Code "looks correct" when stepped in debugger

### Common Causes

**Cause A: Clock not enabled**

```c
// WRONG — forgot to enable GPIOA clock
GPIOA->MODER |= (1 << 10);  // Set PA5 as output
GPIOA->ODR |= (1 << 5);     // Set PA5 high
// Nothing happens because GPIOA peripheral is not clocked
```

```c
// FIX — enable clock first
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
GPIOA->MODER |= (1 << 10);
GPIOA->ODR |= (1 << 5);
```

**Cause B: Wrong pin mode**

```c
// WRONG — MODER left as input (default 00)
GPIOA->ODR |= (1 << 5);  // writing to output register, but pin is input
```

**Cause C: Alternate function override**

```c
// Pin is configured as SPI_MOSI or UART_TX by alternate function
// Even though you set ODR, the AF takes priority
```

**Cause D: Read-Modify-Write on wrong register**

```c
GPIOA->MODER |= (1 << 10);  // Trying to set PA5 to output
// But MODER[11:10] is a 2-bit field
// If MODER[11] was already set, you now have 0b11 = Analog mode!
```

```c
// FIX — clear the field first
GPIOA->MODER &= ~(3 << 10);   // clear both bits
GPIOA->MODER |= (1 << 10);    // then set to 01 (output)
```

### Debugging Checklist

1. Is the peripheral clock enabled?
2. Is the pin in the correct mode (input/output/AF/analog)?
3. Is an alternate function overriding your GPIO control?
4. Are you clearing multi-bit fields before setting?
5. Is the pin physically connected and not held by external hardware?

---

## Case 2: UART Sends Garbage

### Symptoms
- Terminal shows wrong characters or garbled output
- Some characters are correct, others are wrong
- Works at some baud rates but not others

### Common Causes

**Cause A: Wrong baud rate calculation**

```c
// BRR = f_CLK / baud_rate
// Common mistake: using wrong clock frequency
USART1->BRR = 84000000 / 115200;  // wrong if APB2 is actually 42 MHz
```

```c
// Debug: verify the actual clock
uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
USART1->BRR = pclk2 / 115200;
```

**Cause B: Clock source changed after PLL config**

```c
// After changing system clock from HSI (16 MHz) to PLL (84 MHz),
// the UART baud rate register still has the old value
// FIX: reconfigure UART after every clock change
```

**Cause C: Wrong word length or stop bits**

```c
// MCU configured for 9-bit or 2 stop bits
// Terminal configured for 8N1
// Result: framing errors, garbled data
```

**Cause D: TX/RX pins swapped**

```c
// Physical wiring: MCU TX → Device TX (wrong!)
// Correct: MCU TX → Device RX and MCU RX → Device TX
```

### Debugging Checklist

1. Verify clock frequency with oscilloscope on MCO pin
2. Calculate expected baud rate: measure time for one bit on logic analyzer
3. Confirm 8N1 settings match on both ends
4. Check TX/RX crossover in wiring
5. Send `0x55` ('U') — creates alternating bit pattern, easy to verify timing

---

## Case 3: Interrupt Never Fires

### Symptoms
- Interrupt handler never executes
- Polling the flag shows it gets set, but ISR doesn't trigger
- Works in one project, fails in another

### Common Causes

**Cause A: Global interrupts disabled**

```c
// Forgot to enable global interrupts
// On ARM: __enable_irq();
// On AVR: sei();
```

**Cause B: NVIC not configured**

```c
// Peripheral interrupt enabled, but NVIC entry is missing
USART1->CR1 |= USART_CR1_RXNEIE;  // enable RX interrupt in peripheral
// MISSING:
NVIC_EnableIRQ(USART1_IRQn);        // enable in NVIC
NVIC_SetPriority(USART1_IRQn, 5);   // set priority
```

**Cause C: Wrong ISR name**

```c
// WRONG — typo in handler name
void USART1_IRQHandler(void) { ... }   // correct for STM32
void USART1_IRQ_Handler(void) { ... }  // WRONG — extra underscore
// The default weak handler (infinite loop) runs instead
```

**Cause D: Flag not cleared in ISR**

```c
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        // process timer event
        // MISSING: TIM2->SR &= ~TIM_SR_UIF;
        // ISR fires continuously, starving main loop
    }
}
```

**Cause E: Priority and preemption**

```c
// Higher-priority ISR running with same or higher priority
// blocks this interrupt from preempting
// On ARM Cortex-M: lower number = higher priority
NVIC_SetPriority(SysTick_IRQn, 0);  // highest priority
NVIC_SetPriority(USART1_IRQn, 0);   // same priority — can't preempt
```

### Debugging Checklist

1. Are global interrupts enabled?
2. Is the interrupt enabled in the peripheral AND the NVIC?
3. Does the ISR function name match the vector table entry exactly?
4. Is the interrupt flag cleared inside the ISR?
5. Is the interrupt priority correct? (not masked by a higher-priority ISR)
6. Is `__attribute__((weak))` default handler being used instead?

---

## Case 4: Watchdog Reset Loop

### Symptoms
- System continuously resets
- Appears to boot and immediately restart
- Adding breakpoints "fixes" the bug (watchdog pauses in debug)

### Common Causes

**Cause A: Watchdog timeout too short**

```c
// Watchdog set to 10ms, but init takes 50ms
IWDG->KR = 0xCCCC;   // start watchdog
// ... long init: clock config, peripheral init, self-test ...
// Watchdog expires before first kick
```

**Cause B: Blocking call prevents kicking**

```c
while (1) {
    IWDG->KR = 0xAAAA;          // kick watchdog
    data = wait_for_sensor();    // blocks for 5 seconds!
    // Watchdog timeout is 1 second — reset occurs
    process(data);
}
```

**Cause C: Debugging masks the issue**

```c
// Many MCUs freeze the watchdog during debug halts (DBGMCU config)
// Bug only appears in release / production
// Enable watchdog during debug to catch this:
DBGMCU->APB1FZ &= ~DBGMCU_APB1_FZ_DBG_IWDG_STOP;
```

### The Fix

```c
// Use a non-blocking architecture
typedef enum { STATE_WAIT, STATE_PROCESS } State;
State state = STATE_WAIT;

while (1) {
    IWDG->KR = 0xAAAA;  // kick every loop iteration

    switch (state) {
    case STATE_WAIT:
        if (sensor_data_ready()) {
            state = STATE_PROCESS;
        }
        break;
    case STATE_PROCESS:
        process(get_sensor_data());
        state = STATE_WAIT;
        break;
    }
}
```

---

## Case 5: DMA Transfer Produces Wrong Data

### Symptoms
- First transfer works, subsequent transfers don't
- Data is shifted or offset
- Peripheral data register reads correct, but DMA destination is wrong

### Common Causes

**Cause A: DMA not reconfigured after transfer complete**

```c
// Many DMA controllers auto-disable after transfer
// Must reconfigure or re-enable before next transfer
DMA1_Stream0->CR &= ~DMA_SxCR_EN;     // disable stream
while (DMA1_Stream0->CR & DMA_SxCR_EN); // wait for disable
DMA1_Stream0->NDTR = BUFFER_SIZE;       // reset count
DMA1->LIFCR = DMA_LIFCR_CTCIF0;        // clear transfer complete flag
DMA1_Stream0->CR |= DMA_SxCR_EN;       // re-enable
```

**Cause B: Cache coherence (Cortex-M7)**

```c
// CPU writes data to buffer, DMA reads stale cached version
uint8_t tx_buf[64];
memcpy(tx_buf, data, 64);
// MISSING: SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, 64);
start_dma_transfer(tx_buf, 64);
```

```c
// DMA writes to buffer, CPU reads stale cached version
start_dma_rx(rx_buf, 64);
wait_for_dma_complete();
// MISSING: SCB_InvalidateDCache_by_Addr((uint32_t *)rx_buf, 64);
process(rx_buf);
```

**Cause C: Wrong data width**

```c
// ADC produces 16-bit data, but DMA is configured for 32-bit transfers
// Result: every other sample is garbage
```

### Debugging Checklist

1. Is DMA clock enabled?
2. Is the correct DMA stream/channel mapped to the peripheral?
3. Is the data width (byte/halfword/word) correct for both source and destination?
4. Is the transfer direction correct (peripheral-to-memory or memory-to-peripheral)?
5. Is memory increment mode enabled? (otherwise DMA writes to same address)
6. Are cache coherence operations needed? (Cortex-M7, A-series)

---

## Quick Reference

| Problem | First check | Key debug technique |
|---|---|---|
| GPIO not toggling | Peripheral clock enabled? | Toggle in debugger, check register view |
| UART garbage | Baud rate calculation | Send 0x55, measure bit width on scope |
| Interrupt not firing | NVIC enabled? ISR name correct? | Set breakpoint in default handler |
| Watchdog reset loop | Timeout vs init time | Disable watchdog in debug config |
| DMA wrong data | Stream/channel mapping | Compare DMA registers vs reference manual |
| SPI wrong data | Clock polarity/phase (CPOL/CPHA) | Capture on logic analyzer |
| I2C NAK | Pull-up resistors present? | Check with scope for proper voltage levels |
