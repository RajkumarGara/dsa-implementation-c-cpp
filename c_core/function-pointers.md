# Function Pointers

> Function pointers enable callbacks, jump tables, and state machines — core patterns in embedded firmware for ISR registration, command dispatching, and plugin architectures.

---

## Basic Syntax

```c
// Declare a function pointer
int (*fp)(int, int);

// Assign a function to it
int add(int a, int b) { return a + b; }
fp = add;        // or: fp = &add; (both are valid)

// Call through the pointer
int result = fp(3, 4);      // 7
int result = (*fp)(3, 4);   // also 7 — explicit dereference style
```

---

## typedef for Readability

Function pointer syntax is notoriously hard to read. Use `typedef`:

```c
// Without typedef
int (*operation)(int, int);

// With typedef
typedef int (*operation_t)(int, int);
operation_t op = add;
int result = op(3, 4);
```

### More complex example

```c
// Function that returns a function pointer (painful without typedef)
int (*get_operation(char op))(int, int);

// With typedef — much clearer
typedef int (*math_func_t)(int, int);
math_func_t get_operation(char op);
```

---

## Callbacks

Pass a function as an argument to another function.

```c
typedef void (*callback_t)(int);

void process_data(int *data, int len, callback_t on_complete) {
    // process data...
    int result = 0;
    for (int i = 0; i < len; i++) result += data[i];
    on_complete(result);  // invoke the callback
}

void print_result(int val) {
    printf("Result: %d\n", val);
}

void log_result(int val) {
    printf("[LOG] Computed: %d\n", val);
}

int data[] = {1, 2, 3, 4, 5};
process_data(data, 5, print_result);  // "Result: 15"
process_data(data, 5, log_result);    // "[LOG] Computed: 15"
```

### Embedded use: ISR registration

```c
typedef void (*isr_handler_t)(void);

static isr_handler_t isr_table[16] = {NULL};

void register_isr(int irq_num, isr_handler_t handler) {
    if (irq_num < 16) {
        isr_table[irq_num] = handler;
    }
}

void dispatch_isr(int irq_num) {
    if (irq_num < 16 && isr_table[irq_num] != NULL) {
        isr_table[irq_num]();
    }
}
```

---

## Jump Tables (Dispatch Tables)

Replace large `switch` statements with a table of function pointers.

```c
typedef int (*math_op_t)(int, int);

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }
int divide(int a, int b) { return b != 0 ? a / b : 0; }

// Jump table
math_op_t operations[] = { add, sub, mul, divide };

enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV };

int result = operations[OP_MUL](6, 7);  // 42
```

**Advantages over switch:**
- O(1) dispatch — no branch chain
- Easy to extend — just add to the array
- Common in protocol handlers, command parsers, state machines

### Embedded command parser

```c
typedef void (*cmd_handler_t)(const uint8_t *payload, uint8_t len);

typedef struct {
    uint8_t cmd_id;
    cmd_handler_t handler;
} cmd_entry_t;

void handle_read(const uint8_t *payload, uint8_t len)  { /* ... */ }
void handle_write(const uint8_t *payload, uint8_t len) { /* ... */ }
void handle_reset(const uint8_t *payload, uint8_t len) { /* ... */ }

const cmd_entry_t cmd_table[] = {
    { 0x01, handle_read },
    { 0x02, handle_write },
    { 0xFF, handle_reset },
};

void process_command(uint8_t cmd_id, const uint8_t *payload, uint8_t len) {
    for (size_t i = 0; i < sizeof(cmd_table) / sizeof(cmd_table[0]); i++) {
        if (cmd_table[i].cmd_id == cmd_id) {
            cmd_table[i].handler(payload, len);
            return;
        }
    }
    // unknown command
}
```

---

## State Machines with Function Pointers

```c
typedef void (*state_handler_t)(void);

void state_idle(void);
void state_running(void);
void state_error(void);

state_handler_t current_state = state_idle;

void state_idle(void) {
    printf("IDLE\n");
    // transition
    if (/* start condition */) {
        current_state = state_running;
    }
}

void state_running(void) {
    printf("RUNNING\n");
    if (/* error condition */) {
        current_state = state_error;
    }
}

void state_error(void) {
    printf("ERROR\n");
    current_state = state_idle;
}

// Main loop
while (1) {
    current_state();  // call current state handler
}
```

---

## Array of Function Pointers

```c
void func_a(void) { printf("A\n"); }
void func_b(void) { printf("B\n"); }
void func_c(void) { printf("C\n"); }

void (*func_table[])(void) = { func_a, func_b, func_c };
int num_funcs = sizeof(func_table) / sizeof(func_table[0]);

for (int i = 0; i < num_funcs; i++) {
    func_table[i]();
}
```

---

## Function Pointer as Struct Member

```c
typedef struct {
    const char *name;
    void (*init)(void);
    int (*read)(uint8_t *buf, int len);
    int (*write)(const uint8_t *buf, int len);
    void (*deinit)(void);
} driver_t;

// Implement for UART
void uart_init(void) { /* ... */ }
int uart_read(uint8_t *buf, int len) { /* ... */ return len; }
int uart_write(const uint8_t *buf, int len) { /* ... */ return len; }
void uart_deinit(void) { /* ... */ }

const driver_t uart_driver = {
    .name = "UART",
    .init = uart_init,
    .read = uart_read,
    .write = uart_write,
    .deinit = uart_deinit,
};

// Use generically
void use_driver(const driver_t *drv) {
    drv->init();
    uint8_t buf[32];
    drv->read(buf, sizeof(buf));
    drv->deinit();
}
```

This is the **C equivalent of polymorphism / interfaces** — very common in embedded HALs and driver frameworks.

---

## Tricky Declarations

```c
// Pointer to function
int (*fp)(int);

// Array of pointers to functions
int (*fp_array[10])(int);

// Function returning pointer to function
int (*(*fp_returning)(void))(int);

// Pointer to function taking pointer to function
void (*fp_taking_fp)(void (*)(int));
```

**Tip:** Use `cdecl.org` or the right-left rule to decode complex declarations. Better yet, use `typedef`.

---

## qsort — Standard Library Callback

```c
#include <stdlib.h>

int compare_int(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

int arr[] = {5, 2, 8, 1, 9};
qsort(arr, 5, sizeof(int), compare_int);
// arr is now {1, 2, 5, 8, 9}
```

---

## Quick Reference

| Pattern | Example | Use Case |
|---|---|---|
| Basic function pointer | `int (*fp)(int)` | Store/call functions dynamically |
| typedef | `typedef int (*fp_t)(int)` | Readability |
| Callback | `void process(data, callback)` | Decouple logic, event handling |
| Jump table | `fp_t table[] = {f1, f2, f3}` | Replace switch, O(1) dispatch |
| State machine | `state_handler_t current = idle` | Embedded state management |
| Driver interface | `struct { .init, .read, .write }` | HAL abstraction, polymorphism in C |
| ISR registration | `isr_table[n] = handler` | Interrupt handling |
