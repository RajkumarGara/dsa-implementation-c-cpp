# C Concepts — Embedded Developer Cheatsheets

Quick-reference markdown docs covering tricky C/C++ concepts, pitfalls, and patterns for **embedded software** interview prep and day-to-day work.

## Topics

| # | File | What's Inside |
|---|------|---------------|
| 1 | [pointers.md](pointers.md) | Pointer arithmetic, pointer-to-pointer, void pointers, dangling/wild/null pointers |
| 2 | [const-volatile.md](const-volatile.md) | `const` / `volatile` / `restrict` qualifiers, const-correctness patterns |
| 3 | [operator-precedence.md](operator-precedence.md) | Precedence & associativity table, `*ptr++` vs `(*ptr)++` vs `*++ptr`, comma operator |
| 4 | [undefined-behavior.md](undefined-behavior.md) | `i = i++ + ++i`, printf argument order, sequence points, signed overflow |
| 5 | [strings-and-literals.md](strings-and-literals.md) | String literals in read-only memory, `char[]` vs `char*`, common pitfalls |
| 6 | [memory-layout.md](memory-layout.md) | Stack vs heap vs BSS vs data vs text, `sizeof` pitfalls, endianness |
| 7 | [structs-and-unions.md](structs-and-unions.md) | Struct padding/alignment, bit-fields, unions for type-punning, `memcmp` pitfalls |
| 8 | [preprocessor.md](preprocessor.md) | Macros vs inline, stringification, token pasting, include guards, variadic macros |
| 9 | [storage-classes.md](storage-classes.md) | `static`, `extern`, `register`, `auto`, linkage rules, scope vs lifetime |
| 10 | [bitwise-operations.md](bitwise-operations.md) | Set/clear/toggle/check bits, masks, swap without temp, power-of-2 tricks |
| 11 | [function-pointers.md](function-pointers.md) | Callbacks, jump tables, ISR registration, typedef for function pointers |
| 12 | [embedded-specific.md](embedded-specific.md) | Memory-mapped I/O, ISR best practices, `volatile` usage, `#pragma pack`, linker scripts |
| 13 | [type-casting.md](type-casting.md) | Implicit conversions, integer promotion, signed/unsigned pitfalls, pointer casts |
