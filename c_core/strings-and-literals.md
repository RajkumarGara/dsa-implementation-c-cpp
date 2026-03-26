# Strings & String Literals

> Understanding where strings live in memory is essential for embedded C — writing to read-only memory is a classic crash.

---

## String Literals Live in Read-Only Memory

```c
char *s = "hello";
s[0] = 'H';  // UNDEFINED BEHAVIOR — segfault on most systems
```

**Why:** `"hello"` is a string literal stored in the `.rodata` (read-only data) section. The pointer `s` points to that read-only memory. Attempting to modify it is UB.

### Fix: Use a char array

```c
char s[] = "hello";  // copies the literal into a stack-allocated array
s[0] = 'H';          // OK — modifying your own copy
```

---

## `char *s` vs `char s[]`

| Feature | `char *s = "hello";` | `char s[] = "hello";` |
|---|---|---|
| Storage of string | `.rodata` (read-only) | Stack (writable copy) |
| `s` is | A pointer variable | An array name |
| `sizeof(s)` | Size of pointer (4 or 8) | 6 (5 chars + `'\0'`) |
| Modifiable? | No (UB) | Yes |
| Reassignable? | Yes (`s = "world"`) | No (`s = "world"` is error) |

```c
char *p = "hello";
char a[] = "hello";

sizeof(p);  // 8 (pointer size on 64-bit)
sizeof(a);  // 6 (5 chars + null terminator)
```

---

## String Literal Concatenation

Adjacent string literals are concatenated at compile time:

```c
char *msg = "Hello, "
            "world!";
// Same as: char *msg = "Hello, world!";
```

Useful for long strings and multi-line macros:

```c
#define ERROR_MSG "Error: something went wrong " \
                  "at line " __FILE__
```

---

## String Literal Deduplication

The compiler **may** (but is not required to) merge identical string literals:

```c
char *a = "hello";
char *b = "hello";
// a == b may be true (same address) — compiler optimization
// But don't rely on it!
```

---

## Losing a Pointer to a String Literal is NOT a Memory Leak

```c
char *s = "hello";
s = "world";  // "hello" is still in .rodata — no memory leaked
```

String literals live in `.rodata`, not the heap. They are freed automatically when the program exits — there is nothing to `free`.

---

## Common String Pitfalls

### Missing null terminator

```c
char buf[5] = {'h', 'e', 'l', 'l', 'o'};  // NO null terminator!
printf("%s\n", buf);  // UB — reads past buffer looking for '\0'

char buf2[5] = "hello";  // Also NO room for '\0'! 5 chars fill up the array.
char buf3[6] = "hello";  // OK — room for '\0'
char buf4[] = "hello";   // OK — compiler allocates 6 bytes
```

### Buffer overflow with strcpy/strcat

```c
char buf[10];
strcpy(buf, "this string is way too long");  // buffer overflow — UB
```

**Safe alternatives:**

```c
strncpy(buf, src, sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';  // strncpy may not null-terminate!

// Or better — snprintf:
snprintf(buf, sizeof(buf), "%s", src);  // always null-terminates
```

### strncpy does NOT always null-terminate

```c
char buf[5];
strncpy(buf, "hello world", 5);
// buf = {'h', 'e', 'l', 'l', 'o'} — NO null terminator!
```

Always add: `buf[sizeof(buf) - 1] = '\0';`

### strcmp returns 0 for equal

```c
if (strcmp(a, b) == 0) {
    // strings are equal
}

// Common mistake:
if (strcmp(a, b)) {  // true when strings are DIFFERENT! }
```

### String length vs buffer size

```c
char s[] = "hello";
strlen(s);   // 5 — does NOT count '\0'
sizeof(s);   // 6 — DOES count '\0'
```

---

## Strings in Embedded Systems

### Storing strings in Flash (ROM)

On microcontrollers with limited RAM, strings go in Flash:

```c
// AVR (GCC)
#include <avr/pgmspace.h>
const char msg[] PROGMEM = "Hello from Flash";

// ARM Cortex-M — const strings are in Flash by default
const char *msg = "Hello from Flash";  // linker places in .rodata (Flash)
```

### Fixed-size string buffers

In embedded, avoid `malloc`. Use fixed buffers:

```c
#define MAX_MSG_LEN 64
char uart_rx_buf[MAX_MSG_LEN];
```

---

## Character vs String

```c
char c = 'A';        // single character — integer value 65
char *s = "A";       // string — 2 bytes: 'A' + '\0'

sizeof('A');          // 4 in C (int!), 1 in C++
sizeof("A");         // 2 (char + null terminator)
```

**Watch out:** In C, character constants like `'A'` have type `int`, not `char`.

---

## Quick Reference

| Pattern | Safe? | Notes |
|---|---|---|
| `char *s = "hello"; s[0] = 'H';` | No (UB) | Literal is in read-only memory |
| `char s[] = "hello"; s[0] = 'H';` | Yes | Array is a writable copy |
| `strcpy(buf, long_str);` | Dangerous | Use `snprintf` instead |
| `strncpy(buf, src, n);` | Needs care | May not null-terminate |
| `snprintf(buf, size, "%s", src);` | Safe | Always null-terminates |
| `if (strcmp(a, b))` | Bug | True when strings are **different** |
| `strlen(s)` vs `sizeof(s)` | Know the difference | `strlen` excludes `\0`, `sizeof` includes it |
