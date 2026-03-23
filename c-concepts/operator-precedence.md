# Operator Precedence & Associativity

> Getting precedence wrong causes subtle bugs that compile cleanly. These patterns show up constantly in embedded C interviews.

---

## The Classic Trio: `*ptr++` vs `(*ptr)++` vs `*++ptr`

Given:

```c
int arr[] = {10, 20, 30, 40};
int *ptr = arr;
```

### `*ptr++` — Dereference, then advance pointer

**Precedence:** `++` (postfix) binds tighter than `*` (dereference).

Equivalent to: `*(ptr++)`

```c
int val = *ptr++;
// val = 10 (dereferences original ptr = &arr[0])
// ptr now points to arr[1]
```

**Use case:** Walking through arrays — `while (*src++ = *dst++)` in `strcpy`.

### `(*ptr)++` — Dereference, then increment the value

```c
int val = (*ptr)++;
// val = 10 (returns current value before increment)
// arr[0] is now 11
// ptr still points to arr[0]
```

### `*++ptr` — Advance pointer first, then dereference

**Precedence:** `++` (prefix) and `*` have same precedence, right-to-left associativity.

Equivalent to: `*(++ptr)`

```c
int val = *++ptr;
// ptr advances to arr[1] first
// val = 20
```

### Summary Table

| Expression | Pointer after | Value returned | Array modified? |
|---|---|---|---|
| `*ptr++` | Moves forward | Old location's value | No |
| `(*ptr)++` | Unchanged | Old value (pre-inc) | Yes (value incremented) |
| `*++ptr` | Moves forward | New location's value | No |
| `++*ptr` | Unchanged | New value (post-inc) | Yes (value incremented) |

---

## Precedence Table (Most Common Operators)

From **highest** to **lowest** precedence:

| Precedence | Operators | Associativity | Description |
|---|---|---|---|
| 1 | `()` `[]` `->` `.` `++`(post) `--`(post) | Left→Right | Postfix |
| 2 | `++`(pre) `--`(pre) `+`(unary) `-`(unary) `!` `~` `*`(deref) `&`(addr) `sizeof` `(type)` | Right→Left | Unary / Prefix |
| 3 | `*` `/` `%` | Left→Right | Multiplicative |
| 4 | `+` `-` | Left→Right | Additive |
| 5 | `<<` `>>` | Left→Right | Bitwise shift |
| 6 | `<` `<=` `>` `>=` | Left→Right | Relational |
| 7 | `==` `!=` | Left→Right | Equality |
| 8 | `&` | Left→Right | Bitwise AND |
| 9 | `^` | Left→Right | Bitwise XOR |
| 10 | `\|` | Left→Right | Bitwise OR |
| 11 | `&&` | Left→Right | Logical AND |
| 12 | `\|\|` | Left→Right | Logical OR |
| 13 | `?:` | Right→Left | Ternary |
| 14 | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `\|=` | Right→Left | Assignment |
| 15 | `,` | Left→Right | Comma |

---

## Common Precedence Traps

### Trap 1: Bitwise vs Comparison

```c
if (x & 0x01 == 0)    // WRONG: parsed as x & (0x01 == 0) → x & 0
if ((x & 0x01) == 0)  // CORRECT
```

`==` has **higher** precedence than `&`. This is a very common embedded bug.

**Rule of thumb:** Always parenthesize bitwise operations in conditions.

### Trap 2: Bitwise vs Logical

```c
if (a & b && c)   // parsed as: a & (b && c) — probably not intended
if ((a & b) && c) // what you likely meant
```

### Trap 3: Shift vs Addition

```c
int x = 1 << 2 + 3;   // parsed as: 1 << (2 + 3) = 1 << 5 = 32
int x = (1 << 2) + 3;  // = 4 + 3 = 7
```

`+` has higher precedence than `<<`.

### Trap 4: Ternary vs Assignment

```c
int x = a > b ? a : b = 10;   // ERROR: parsed as (a > b ? a : b) = 10
int x = a > b ? a : (b = 10); // OK but confusing — use if/else
```

### Trap 5: Comma Operator

```c
int x = (1, 2, 3);  // x = 3 — comma evaluates left-to-right, returns last
int y = 1, 2, 3;    // ERROR: comma here is a separator, not an operator
```

```c
// In for loops — legitimate use
for (i = 0, j = n - 1; i < j; i++, j--) { }
```

### Trap 6: Arrow vs Increment

```c
struct node *p;
*p->data;     // parsed as *(p->data) — dereferences the data field
// NOT (*p)->data
```

`->` has higher precedence than `*`.

---

## Sizeof Traps

```c
int arr[10];

sizeof(arr);       // 40 (total array size)
sizeof(arr[0]);    // 4 (size of one element)
sizeof(arr) / sizeof(arr[0]);  // 10 (number of elements)

// But when passed to a function:
void foo(int arr[]) {
    sizeof(arr);   // 8! (pointer size, not array size)
}
```

```c
int x = 5;
printf("%zu\n", sizeof(x++));  // prints 4 — x is NOT incremented!
// sizeof does not evaluate its operand (except VLAs)
```

---

## Associativity Matters

### Right-to-left assignment chaining

```c
int a, b, c;
a = b = c = 10;   // Right-to-left: c=10, then b=c, then a=b
```

### Ternary chaining

```c
// Right-to-left associativity
int x = a ? b : c ? d : e;
// Parsed as: a ? b : (c ? d : e)
```

---

## Quick Reference — "When in Doubt, Parenthesize"

```c
// Always parenthesize these:
if ((x & MASK) == VALUE)     // bitwise in conditions
if ((x >> 4) + offset)       // shifts with arithmetic
val = (condition) ? a : b;   // ternary — for clarity
#define SQUARE(x) ((x) * (x))  // macro arguments
```
