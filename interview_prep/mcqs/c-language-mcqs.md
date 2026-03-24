# C Language MCQs

> Multiple-choice questions covering C core concepts — pointers, memory, types, operators, and language semantics. Each question includes a detailed explanation.

---

## Pointers & Arrays

### Q1. What is the output?

```c
int arr[] = {1, 2, 3, 4, 5};
int *p = arr;
printf("%d", *(p + 3));
```

A) 1  
B) 3  
C) **4** ✓  
D) 5

**Explanation:** `p + 3` advances 3 elements from the start. `*(p + 3)` is equivalent to `arr[3]` which is `4`.

---

### Q2. What does `sizeof(arr)` return inside a function receiving `int arr[]`?

A) Total array size in bytes  
B) Number of elements  
C) **Size of a pointer** ✓  
D) Compilation error

**Explanation:** Arrays decay to pointers when passed to functions. `sizeof(arr)` gives the pointer size (8 on 64-bit).

---

### Q3. Which is a pointer to an array of 10 ints?

A) `int *p[10];`  
B) **`int (*p)[10];`** ✓  
C) `int *(p[10]);`  
D) `int p[10];`

**Explanation:** `int (*p)[10]` — parentheses bind `*` to `p` first, making it a pointer to `int[10]`. Option A is an array of 10 pointers.

---

### Q4. What is the output?

```c
char *s = "hello";
printf("%c", *(s + 1));
```

A) h  
B) **e** ✓  
C) l  
D) Compilation error

**Explanation:** `s + 1` points to the second character. `*(s + 1)` is 'e'.

---

### Q5. What happens?

```c
int *p;
printf("%d", *p);
```

A) Prints 0  
B) Prints garbage  
C) **Undefined Behavior** ✓  
D) Compilation error

**Explanation:** `p` is uninitialized (wild pointer). Dereferencing it is UB — it may crash, print garbage, or do anything.

---

## Memory & Storage

### Q6. Where is a global `static int x;` stored?

A) Stack  
B) Heap  
C) **BSS segment** ✓  
D) Code/Text segment

**Explanation:** Uninitialized (or zero-initialized) global/static variables go in `.bss`. Initialized non-zero globals go in `.data`.

---

### Q7. What is the output?

```c
void foo(void) {
    static int count = 0;
    count++;
    printf("%d ", count);
}
// Called 3 times from main
foo(); foo(); foo();
```

A) 1 1 1  
B) 0 0 0  
C) **1 2 3** ✓  
D) Undefined

**Explanation:** `static` local variables persist across function calls. `count` is initialized once and incremented on each call.

---

### Q8. What does `malloc(0)` return?

A) Always NULL  
B) **Implementation-defined (NULL or unique pointer)** ✓  
C) Pointer to 1 byte  
D) Compilation error

**Explanation:** The C standard says `malloc(0)` returns either NULL or a unique pointer that can be passed to `free()`. Behavior varies by implementation.

---

### Q9. What is the consequence of `free(ptr); free(ptr);`?

A) No effect  
B) Memory leak  
C) **Undefined Behavior (double free)** ✓  
D) Compilation error

**Explanation:** Double-free is UB. It can corrupt the heap metadata, leading to crashes or security vulnerabilities.

---

### Q10. What segment stores string literals like `"hello"`?

A) Stack  
B) Heap  
C) BSS  
D) **Read-only data (.rodata or .text)** ✓

**Explanation:** String literals are stored in read-only memory. Modifying them is UB.

---

## Types & Conversions

### Q11. What is the output?

```c
printf("%d", sizeof(char));
```

A) 0  
B) **1** ✓  
C) 2  
D) Implementation-defined

**Explanation:** `sizeof(char)` is always 1 by definition in the C standard, regardless of whether `char` is 8 bits.

---

### Q12. What is the result of `-1 > 1U`?

A) false (0)  
B) **true (1)** ✓  
C) Compilation error  
D) Undefined

**Explanation:** When comparing `int` with `unsigned int`, the `int` is converted to `unsigned`. `-1` becomes `UINT_MAX`, which is greater than `1U`.

---

### Q13. What is the output?

```c
uint8_t a = 200;
uint8_t b = 100;
uint8_t c = a + b;
printf("%d", c);
```

A) 300  
B) **44** ✓  
C) 0  
D) Undefined

**Explanation:** `a + b = 300`, but `uint8_t` can hold 0–255. `300 % 256 = 44`. Unsigned overflow wraps around.

---

### Q14. What does `sizeof(int)` return on a 32-bit ARM Cortex-M microcontroller?

A) 2  
B) **4** ✓  
C) 8  
D) Implementation-defined

**Explanation:** On 32-bit ARM, `int` is 4 bytes. While technically implementation-defined, all standard ARM compilers use 4.

---

### Q15. What is the type of `'A'` in C?

A) `char`  
B) **`int`** ✓  
C) `unsigned char`  
D) `short`

**Explanation:** In C, character constants have type `int` (not `char`). In C++, they have type `char`. This is a classic C vs C++ distinction.

---

## Operators & Expressions

### Q16. What is the output?

```c
int x = 5;
printf("%d", x >> 1);
```

A) 10  
B) **2** ✓  
C) 1  
D) Undefined

**Explanation:** Right shift by 1 divides by 2 (for non-negative values). `5 >> 1 = 2` (integer division, truncated).

---

### Q17. What is `0x0F & 0xF0`?

A) 0xFF  
B) **0x00** ✓  
C) 0x0F  
D) 0xF0

**Explanation:** AND: `0000 1111 & 1111 0000 = 0000 0000`. No bits overlap.

---

### Q18. What is the output?

```c
int a = 1, b = 2, c = 3;
printf("%d", a | b & c);
```

A) 0  
B) 1  
C) **3** ✓  
D) 7

**Explanation:** `&` has higher precedence than `|`. So: `b & c = 2 & 3 = 2`, then `a | 2 = 1 | 2 = 3`.

---

### Q19. What does `!!x` produce for non-zero `x`?

A) `x`  
B) `-x`  
C) **1** ✓  
D) 0

**Explanation:** `!x` converts non-zero to 0, zero to 1. `!!x` converts any non-zero to 1. It's a boolean normalization idiom.

---

### Q20. What is the output?

```c
printf("%d", 2 << 3);
```

A) 6  
B) 8  
C) **16** ✓  
D) 3

**Explanation:** `2 << 3 = 2 * 2^3 = 16`. Left shift by N multiplies by 2^N.

---

## Preprocessor & Macros

### Q21. What is the output?

```c
#define MUL(a, b) a * b
printf("%d", MUL(2 + 3, 4 + 5));
```

A) 45  
B) **19** ✓  
C) 35  
D) Compilation error

**Explanation:** Expands to `2 + 3 * 4 + 5 = 2 + 12 + 5 = 19`. Macro args are not parenthesized. Fix: `#define MUL(a, b) ((a) * (b))`.

---

### Q22. What is `#x` in a macro?

A) Concatenation  
B) **Stringification (converts to string literal)** ✓  
C) Comment  
D) Preprocessor directive

**Explanation:** `#x` converts the macro argument `x` to a string. `#define STR(x) #x` — `STR(hello)` becomes `"hello"`.

---

### Q23. What does `##` do in a macro?

A) Stringification  
B) Double hash comment  
C) **Token pasting (concatenation)** ✓  
D) Logical AND

**Explanation:** `##` joins two tokens. `#define CONCAT(a,b) a##b` — `CONCAT(var, 1)` becomes `var1`.

---

### Q24. Which is a correct multi-statement macro?

A) `#define M() { a(); b(); }`  
B) `#define M() a(); b();`  
C) **`#define M() do { a(); b(); } while(0)`** ✓  
D) All are equally correct

**Explanation:** `do { ... } while(0)` works safely in all contexts (after `if`, with semicolons). Options A and B break in if-else chains.

---

### Q25. What is the value of `X` after this?

```c
#define X 3 + 2
int y = X * 2;
```

A) 10  
B) **7** ✓  
C) 12  
D) Compilation error

**Explanation:** Expands to `3 + 2 * 2 = 3 + 4 = 7`. Macro expansion is textual — no implicit parentheses.

---

## Structs & Unions

### Q26. What is `sizeof` this struct (32-bit system, natural alignment)?

```c
struct S { char c; int i; char d; };
```

A) 6  
B) 8  
C) **12** ✓  
D) 9

**Explanation:** `char(1) + pad(3) + int(4) + char(1) + pad(3) = 12`. The struct is padded to align `int` and for array alignment.

---

### Q27. What is the size of a union?

```c
union U { int i; double d; char c; };
```

A) 4  
B) 1  
C) 13  
D) **8** ✓

**Explanation:** Union size equals the size of its largest member. `double` is 8 bytes.

---

### Q28. What does `offsetof(struct S, member)` return?

A) Size of the member  
B) **Byte offset from the start of the struct** ✓  
C) Address of the member  
D) Alignment of the member

**Explanation:** `offsetof` returns the number of bytes from the beginning of the struct to the start of the specified member, accounting for padding.

---

### Q29. What value can a 1-bit `signed int` bit field hold?

A) 0 and 1  
B) **0 and -1** ✓  
C) -1 and 1  
D) Only 0

**Explanation:** A 1-bit signed int uses two's complement. Bit 0 = value 0, bit 1 = value -1. It cannot represent +1. Always use `unsigned int` for bit fields.

---

### Q30. Which is true about flexible array members?

A) They contribute to `sizeof` the struct  
B) They can appear anywhere in the struct  
C) The struct can have no other members  
D) **They must be the last member and don't affect `sizeof`** ✓

**Explanation:** `int data[];` must be the last member, the struct must have at least one other member, and `sizeof` doesn't include the flexible array.

---

## Quick Reference: Key Rules

| Topic | Rule |
|---|---|
| Array decay | Arrays decay to pointers in function args — `sizeof` gives pointer size |
| Signed/unsigned compare | Signed converts to unsigned — negative values become large |
| Integer promotion | Types < int are promoted to int in expressions |
| String literals | Stored in read-only memory — modification is UB |
| Pointer arithmetic | Moves in units of the pointed-to type, not bytes |
| Macro safety | Always parenthesize arguments and the full expression |
| Struct padding | Members aligned to natural boundaries — reorder to minimize |
| Bit fields | Ordering is implementation-defined — prefer explicit masks |
| `sizeof(char)` | Always 1, guaranteed by the standard |
| `static` locals | Persist across calls — initialized once |
