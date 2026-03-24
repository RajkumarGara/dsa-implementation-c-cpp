# RTOS & Concurrency Debugging

> Concurrency bugs are the most insidious class of embedded problems — they're intermittent, timing-dependent, and often impossible to reproduce under debug. These cases build your instincts.

---

## Case 1: Priority Inversion

### Symptoms
- High-priority task starves unexpectedly
- System responsiveness degrades under load
- Low-priority task appears to block a critical task

### The Bug

```
Task A (High priority)   — needs mutex M
Task B (Medium priority) — CPU-intensive, no shared resources
Task C (Low priority)    — holds mutex M

Timeline:
1. Task C locks mutex M
2. Task A preempts, tries to lock M → blocks (waiting on C)
3. Task B preempts Task C (higher priority)
4. Task B runs for a long time
5. Task C can't run → can't release M
6. Task A is stuck — blocked by a LOWER priority task via B
```

**The inversion:** High-priority Task A is effectively blocked by medium-priority Task B, which has nothing to do with the shared resource.

### How to Diagnose

- **RTOS trace tools** (Tracealyzer, SystemView) show task scheduling timeline
- Look for high-priority tasks blocked longer than expected
- Monitor mutex ownership and wait times

### The Fix

**Priority inheritance:** Most RTOSes support it per-mutex:

```c
// FreeRTOS: use xSemaphoreCreateMutex() — has priority inheritance
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

// NOT xSemaphoreCreateBinary() — no priority inheritance!
```

With priority inheritance, when Task A blocks on mutex M held by Task C, Task C temporarily inherits Task A's priority → runs before Task B → releases M quickly.

**Priority ceiling:** Set the mutex's ceiling priority to the highest priority of any task that uses it.

---

## Case 2: Deadlock

### Symptoms
- Two or more tasks stop executing permanently
- System appears frozen but ISRs still fire
- Watchdog triggers because tasks stop kicking it

### The Bug

```c
// Task 1:
xSemaphoreTake(mutex_A, portMAX_DELAY);   // lock A
// ... some work ...
xSemaphoreTake(mutex_B, portMAX_DELAY);   // try to lock B → BLOCKED
xSemaphoreGive(mutex_B);
xSemaphoreGive(mutex_A);

// Task 2:
xSemaphoreTake(mutex_B, portMAX_DELAY);   // lock B
// ... some work ...
xSemaphoreTake(mutex_A, portMAX_DELAY);   // try to lock A → BLOCKED
xSemaphoreGive(mutex_A);
xSemaphoreGive(mutex_B);
```

**Classic circular wait:** Task 1 holds A, waits for B. Task 2 holds B, waits for A. Neither can proceed.

### How to Diagnose

- In debugger, check each task's stack trace — both stuck in `xSemaphoreTake`
- Use `uxTaskGetSystemState()` to dump task states
- Check which mutexes each task holds and waits on

### The Fix

**Lock ordering:** Always acquire mutexes in the same global order:

```c
// RULE: Always lock A before B (alphabetical, or by address)
// Task 1:
xSemaphoreTake(mutex_A, portMAX_DELAY);
xSemaphoreTake(mutex_B, portMAX_DELAY);
// ... work ...
xSemaphoreGive(mutex_B);
xSemaphoreGive(mutex_A);

// Task 2: SAME ORDER
xSemaphoreTake(mutex_A, portMAX_DELAY);
xSemaphoreTake(mutex_B, portMAX_DELAY);
// ... work ...
xSemaphoreGive(mutex_B);
xSemaphoreGive(mutex_A);
```

**Timeout-based recovery:**

```c
if (xSemaphoreTake(mutex_B, pdMS_TO_TICKS(100)) == pdFALSE) {
    xSemaphoreGive(mutex_A);  // release what we hold
    // retry or report error
}
```

---

## Case 3: Shared Variable Without Protection

### Symptoms
- Data occasionally corrupted (1 in 1000 reads)
- Bug appears under high interrupt rate or high task-switching frequency
- Adding debug prints changes timing and hides the bug

### The Bug

```c
// Global shared between two tasks
uint32_t sensor_data;

// Task 1 (Producer):
void sensor_task(void *arg) {
    while (1) {
        sensor_data = read_adc();  // 32-bit write, may not be atomic
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task 2 (Consumer):
void display_task(void *arg) {
    while (1) {
        uint32_t val = sensor_data;  // torn read possible
        update_display(val);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

**Why it breaks:** Even on 32-bit ARM where word access is atomic, the compiler can optimize, reorder, or split the access. Without synchronization, there's no guarantee of consistency.

### The Fix

**Option A: Mutex**

```c
void sensor_task(void *arg) {
    while (1) {
        xSemaphoreTake(data_mutex, portMAX_DELAY);
        sensor_data = read_adc();
        xSemaphoreGive(data_mutex);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

**Option B: Queue (preferred for producer-consumer)**

```c
QueueHandle_t sensor_queue = xQueueCreate(5, sizeof(uint32_t));

void sensor_task(void *arg) {
    while (1) {
        uint32_t val = read_adc();
        xQueueSend(sensor_queue, &val, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void display_task(void *arg) {
    while (1) {
        uint32_t val;
        if (xQueueReceive(sensor_queue, &val, pdMS_TO_TICKS(100))) {
            update_display(val);
        }
    }
}
```

**Option C: Critical section (for ISR-task sharing)**

```c
taskENTER_CRITICAL();
sensor_data = read_adc();
taskEXIT_CRITICAL();
```

---

## Case 4: Stack Overflow in RTOS Task

### Symptoms
- One task corrupts another task's stack
- Random HardFaults that change location
- Corruption pattern: a task's local variables contain another task's data

### The Bug

```c
// Task with insufficient stack
void logging_task(void *arg) {
    char log_buffer[512];     // 512 bytes on this task's stack
    sprintf(log_buffer, ...); // sprintf can use MORE stack internally
    // Total stack usage exceeds allocated stack
}

xTaskCreate(logging_task, "Log", 128 /* words = 512 bytes */, ...);
// 512 bytes total, 512 for buffer alone — NO room for function overhead
```

### How to Diagnose

```c
// FreeRTOS stack high-water mark
UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
printf("Stack remaining: %lu words\n", remaining);
// If this approaches 0, stack overflow is imminent
```

Enable detection:

```c
// In FreeRTOSConfig.h
#define configCHECK_FOR_STACK_OVERFLOW 2  // method 2: pattern check

// Implement the hook:
void vApplicationStackOverflowHook(TaskHandle_t task, char *name) {
    printf("STACK OVERFLOW in task: %s\n", name);
    while (1);  // halt for debugging
}
```

### The Fix

- Increase task stack size
- Move large buffers to static/global scope or heap
- Profile stack usage: measure high-water mark under worst-case conditions
- Use `__attribute__((no_instrument_function))` to exclude functions from stack profiling

---

## Case 5: ISR Takes Too Long

### Symptoms
- Missed interrupts (events lost)
- Other ISRs delayed (high-latency response)
- Main loop / RTOS tasks starved
- System timing goes out of specification

### The Bug

```c
void UART_RX_ISR(void) {
    uint8_t byte = UART->DR;
    parse_packet(byte);         // complex parsing in ISR!
    if (packet_complete) {
        process_packet();        // heavy processing in ISR!
        send_response();         // blocking TX in ISR!
    }
    clear_rx_flag();
}
```

### The Fix: Defer Work to Task

```c
// ISR: minimal work only
void UART_RX_ISR(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t byte = UART->DR;

    xQueueSendFromISR(rx_queue, &byte, &xHigherPriorityTaskWoken);
    clear_rx_flag();

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Task: does the heavy work
void uart_processing_task(void *arg) {
    uint8_t byte;
    while (1) {
        if (xQueueReceive(rx_queue, &byte, portMAX_DELAY)) {
            parse_packet(byte);
            if (packet_complete) {
                process_packet();
                send_response();
            }
        }
    }
}
```

**Rule for ISRs:**
- Read peripheral data register
- Clear interrupt flag
- Signal a task (queue, semaphore, notification)
- Return ASAP

---

## Quick Reference

| Bug class | Key symptom | Prevention |
|---|---|---|
| Priority inversion | High-prio task delays unexpectedly | Use mutexes with priority inheritance |
| Deadlock | Multiple tasks permanently blocked | Consistent lock ordering |
| Data race | Intermittent corruption | Protect shared data (mutex/queue/critical) |
| RTOS stack overflow | Random corruption across tasks | High-water mark monitoring |
| Long ISR | Missed events, timing failure | Defer work to task via queue/semaphore |
| Starvation | Low-prio task never runs | Ensure high-prio tasks yield/block |
| Missed notification | Event lost | Use counting semaphore, not binary |
