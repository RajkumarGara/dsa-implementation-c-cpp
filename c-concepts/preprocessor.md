# Preprocessor

> The C preprocessor runs **before** compilation. It's a text-substitution engine — powerful but dangerous if misused. Essential knowledge for embedded firmware.

---

## Include Guards

Prevent double-inclusion of header files.

```c
// Classic include guard
#ifndef MY_HEADER_H
#define MY_HEADER_H

// declarations here

#endif // MY_HEADER_H
```

```c
// Pragma alternative (non-standard but widely supported)
#pragma once
```

| Method | Portable? | Notes |
|---|---|---|
| `#ifndef` guard | Yes (standard) | Requires unique macro name |
| `#pragma once` | No (but GCC, Clang, MSVC all support it) | Simpler, no name collision risk |

---

## Macros vs Functions

### Object-like macros (constants)

```c
#define MAX_SIZE 100
#define PI 3.14159
```

### Function-like macros

```c
#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
```

### Why all the parentheses?

```c
#define SQUARE_BAD(x) x * x

SQUARE_BAD(2 + 3);   // expands to: 2 + 3 * 2 + 3 = 2 + 6 + 3 = 11 (wrong!)
SQUARE(2 + 3);        // expands to: ((2 + 3) * (2 + 3)) = 25 (correct)
```

**Rule:** Always wrap macro parameters AND the entire expression in parentheses.

### Double-evaluation problem

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int x = 5, y = 3;
int z = MAX(x++, y);
// Expands to: ((x++) > (y) ? (x++) : (y))
// x gets incremented TWICE if x > y!
```

**Fix:** Use `static inline` functions (C99+) or GCC's `typeof` extension:

```c
// C99 inline function — no double evaluation
static inline int max_int(int a, int b) {
    return a > b ? a : b;
}

// GCC extension — works with any type
#define MAX_SAFE(a, b) ({       \
    __typeof__(a) _a = (a);     \
    __typeof__(b) _b = (b);     \
    _a > _b ? _a : _b;          \
})
```

---

## Macros vs inline Functions

| Feature | Macro | `static inline` |
|---|---|---|
| Type checking | None | Yes |
| Double evaluation | Yes (danger!) | No |
| Debugging | Hard (expands before compile) | Normal |
| Side effects | Dangerous | Safe |
| Type-generic | Yes | No (need one per type, or C11 `_Generic`) |
| Works in C89 | Yes | No (`inline` is C99) |

**Preference:** Use `static inline` for function-like behavior. Use macros for constants, conditional compilation, and truly type-generic operations.

---

## Stringification (`#`)

Converts a macro argument to a string literal.

```c
#define TO_STRING(x) #x
#define PRINT_VAR(var) printf(#var " = %d\n", var)

TO_STRING(hello);       // "hello"
PRINT_VAR(count);       // printf("count" " = %d\n", count);
                        // → printf("count = %d\n", count);
```

### Stringify after expansion

```c
#define STRINGIFY(x) #x
#define EXPAND_STRINGIFY(x) STRINGIFY(x)

#define VERSION 42

STRINGIFY(VERSION);         // "VERSION" — stringifies the macro name
EXPAND_STRINGIFY(VERSION);  // "42" — expands first, then stringifies
```

---

## Token Pasting (`##`)

Concatenates two tokens into one.

```c
#define CONCAT(a, b) a##b

CONCAT(foo, bar);    // foobar — a single token
CONCAT(my_, var);    // my_var

int CONCAT(value_, 1) = 10;  // int value_1 = 10;
```

**Embedded use case:** Generate register definitions:

```c
#define GPIO_REG(port, reg) GPIO##port->reg

GPIO_REG(A, ODR) = 0xFF;   // GPIOA->ODR = 0xFF;
GPIO_REG(B, IDR);           // GPIOB->IDR
```

---

## Variadic Macros (C99)

```c
#define DEBUG_LOG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

DEBUG_LOG("value = %d", x);      // printf("[DEBUG] value = %d\n", x);
DEBUG_LOG("starting");            // printf("[DEBUG] starting\n");
```

`##__VA_ARGS__` — the `##` removes the trailing comma when no extra arguments are passed (GCC/Clang extension; C23 standardizes `__VA_OPT__`).

```c
// C23 standard way:
#define DEBUG_LOG(fmt, ...) printf("[DEBUG] " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
```

---

## Conditional Compilation

```c
#ifdef DEBUG
    printf("Debug mode\n");
#endif

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 256
#endif

#if defined(PLATFORM_ARM) && (CHIP_FAMILY == STM32)
    #include "stm32_hal.h"
#elif defined(PLATFORM_AVR)
    #include "avr_hal.h"
#else
    #error "Unsupported platform"
#endif
```

### Common patterns in embedded

```c
// Enable/disable features at compile time
#define FEATURE_UART_ENABLED  1
#define FEATURE_SPI_ENABLED   0

#if FEATURE_UART_ENABLED
void uart_init(void) { /* ... */ }
#endif

// Platform-specific code
#if defined(__ARM_ARCH)
    #define DISABLE_INTERRUPTS() __disable_irq()
#elif defined(__AVR__)
    #define DISABLE_INTERRUPTS() cli()
#endif
```

---

## Predefined Macros

| Macro | Value | Use |
|---|---|---|
| `__FILE__` | Current filename (string) | Error messages, logging |
| `__LINE__` | Current line number (int) | Error messages, logging |
| `__func__` | Current function name (C99) | Logging |
| `__DATE__` | Compilation date | Firmware version |
| `__TIME__` | Compilation time | Firmware version |
| `__STDC__` | 1 if standard C | Portability checks |
| `__STDC_VERSION__` | C standard version (e.g., 201112L for C11) | Feature detection |

```c
#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("ASSERT FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
        while(1); /* halt in embedded */ \
    } \
} while(0)
```

---

## Multi-Line Macros — Use `do { } while(0)`

```c
// BAD — breaks if used in an if/else:
#define SWAP(a, b) { int t = a; a = b; b = t; }

if (x > y)
    SWAP(x, y);    // expands to { ... };
else               // ERROR: else without matching if (semicolon problem)
    other();

// GOOD — do-while(0) idiom:
#define SWAP(a, b) do { int t = (a); (a) = (b); (b) = t; } while(0)

if (x > y)
    SWAP(x, y);    // works correctly
else
    other();
```

---

## `#pragma` Directives

```c
#pragma once                    // include guard

#pragma pack(push, 1)           // pack structs (no padding)
struct packed { char a; int b; };
#pragma pack(pop)               // restore default packing

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
int unused_var;
#pragma GCC diagnostic pop

#pragma weak symbol_name        // weak linkage (overridable)
```

---

## `#error` and `#warning`

```c
#if BUFFER_SIZE < 64
    #error "BUFFER_SIZE must be at least 64"
#endif

#if BUFFER_SIZE < 256
    #warning "BUFFER_SIZE is small — consider increasing"
#endif
```

---

## Quick Reference

| Technique | Syntax | Watch Out For |
|---|---|---|
| Object macro | `#define NAME value` | No semicolons, no type checking |
| Function macro | `#define F(x) ((x) * (x))` | Parenthesize everything |
| Stringification | `#x` | Won't expand macros — use two-level trick |
| Token pasting | `a##b` | Result must be a valid token |
| Variadic | `__VA_ARGS__` | Trailing comma — use `##__VA_ARGS__` |
| Multi-line | `do { } while(0)` | Avoid bare `{ }` blocks |
| Include guard | `#ifndef X_H` | Use unique names |
