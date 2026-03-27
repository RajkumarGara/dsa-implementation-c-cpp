# C++ Concepts Beyond C

> C++ is not just "C with classes". Many of its key features solve ownership, generic programming, and safer abstraction problems that plain C leaves to discipline.

---

## References

A reference is an alias for an existing object.

```cpp
int value = 10;
int &ref = value;
ref = 20;
```

- Must be initialized
- Cannot be reseated to another object
- Safer than pointers when null is not meaningful

### Lvalue and rvalue references

```cpp
void takeLvalue(int &value) {
    (void)value;
}

void takeRvalue(int &&value) {
    (void)value;
}
```

`T&&` is the basis of move semantics.

---

## RAII

Resource Acquisition Is Initialization means a resource is tied to object lifetime.

```cpp
class InterruptLockGuard {
public:
    InterruptLockGuard() {
        disableInterrupts();
    }

    ~InterruptLockGuard() {
        enableInterrupts();
    }

private:
    void disableInterrupts() {
    }

    void enableInterrupts() {
    }
};
```

When the object goes out of scope, cleanup happens automatically, even on early return.

---

## Constructors: Copy and Move

```cpp
class Buffer {
public:
    Buffer() = default;
    Buffer(const Buffer &other) = default;
    Buffer(Buffer &&other) noexcept = default;
    Buffer &operator=(const Buffer &other) = default;
    Buffer &operator=(Buffer &&other) noexcept = default;
    ~Buffer() = default;
};
```

This connects to the rule of three, five, and zero.

### Rule of three

If you manually define one of these, you often need all three:
- destructor
- copy constructor
- copy assignment operator

### Rule of five

In modern C++, also consider:
- move constructor
- move assignment operator

### Rule of zero

Prefer designing types so the compiler-generated operations are correct and enough.

---

## Move Semantics

Move semantics transfer ownership instead of copying resources.

```cpp
#include <utility>

class Message {
public:
    Message() = default;
    Message(Message &&other) noexcept
        : length_(other.length_) {
        other.length_ = 0;
    }

private:
    int length_{0};
};

Message createMessage() {
    Message msg;
    return msg;
}
```

Moves reduce copies of buffers, containers, and other expensive objects.

---

## Smart Pointers

### `std::unique_ptr`

Single owner.

```cpp
#include <memory>

auto ptr = std::make_unique<int>(42);
```

### `std::shared_ptr`

Reference-counted shared ownership.

```cpp
#include <memory>

auto shared = std::make_shared<int>(42);
```

### `std::weak_ptr`

Non-owning observer used to break cycles.

**Embedded note:** many embedded codebases avoid heap allocation entirely, so know the concept even if the production code does not use it.

---

## Templates

Templates enable generic programming.

```cpp
template <typename T>
T maxValue(T a, T b) {
    return (a > b) ? a : b;
}
```

Templates are instantiated at compile time.

### Class template

```cpp
template <typename T, int Size>
class FixedBuffer {
public:
    bool push(const T &value) {
        if (count_ == Size) {
            return false;
        }
        data_[count_++] = value;
        return true;
    }

private:
    T data_[Size]{};
    int count_{0};
};
```

This is common in embedded systems because `Size` is fixed at compile time.

---

## Namespaces

Namespaces avoid symbol collisions.

```cpp
namespace drivers {
class Spi {
public:
    void init() {
    }
};
}
```

Use them to organize modules without relying on global prefixes everywhere.

---

## Function Overloading and Default Arguments

```cpp
void configure(int baudRate) {
    (void)baudRate;
}

void configure(int baudRate, bool parityEnabled) {
    (void)baudRate;
    (void)parityEnabled;
}
```

Overloading is compile-time polymorphism.

```cpp
void delayMs(int ms, bool lowPower = false) {
    (void)ms;
    (void)lowPower;
}
```

Default arguments are also resolved at compile time.

---

## `constexpr`

`constexpr` lets values and functions be evaluated at compile time when possible.

```cpp
constexpr int bufferSize(int channels) {
    return channels * 16;
}

int rxBuffer[bufferSize(4)];
```

Useful for array sizes, masks, lookup tables, and zero-cost abstractions.

---

## `enum class`

Stronger typed enums than C-style enums.

```cpp
enum class UartState {
    Idle,
    Busy,
    Error
};
```

Benefits:
- names stay scoped
- no implicit conversion to `int`
- fewer collisions

---

## Exception Handling

```cpp
try {
    // risky operation
} catch (...) {
    // handle error
}
```

C++ supports exceptions, but many embedded projects disable them for code size and predictability reasons.

Know the feature, but do not assume it is allowed in firmware.

---

## STL Containers and Algorithms

```cpp
#include <array>
#include <algorithm>

std::array<int, 4> values{4, 1, 3, 2};
std::sort(values.begin(), values.end());
```

Common containers:
- `std::array` for fixed-size stack storage
- `std::vector` for dynamic arrays
- `std::deque`, `std::list`, `std::map`, `std::unordered_map`

In embedded work, `std::array` is usually the safest standard container to discuss.

---

## Lambdas

```cpp
auto isEven = [](int value) {
    return (value % 2) == 0;
};
```

Useful for callbacks, local predicates, and algorithm customization.

Embedded caveat: captures can add hidden state, so understand what gets stored.

---

## `auto`

```cpp
auto count = 10;
auto *ptr = &count;
```

`auto` asks the compiler to deduce the type. Use it when the type is obvious from the right-hand side, not to hide important ownership or narrowing details.

---

## Casts in C++

Prefer C++ casts over C-style casts.

- `static_cast` for well-defined compile-time conversions
- `const_cast` for adding or removing constness
- `reinterpret_cast` for low-level bit reinterpretation and address conversions
- `dynamic_cast` for safe downcasting in polymorphic hierarchies

```cpp
int value = 7;
double scaled = static_cast<double>(value);
```

---

## Differences from C That Matter Most

| Topic | C | C++ |
|---|---|---|
| Strings | null-terminated arrays | `std::string` plus C strings |
| Memory management | `malloc` and `free` | `new` and `delete`, smart pointers, RAII |
| Generic code | macros, `void *` | templates |
| Error handling | return codes | return codes plus exceptions |
| Namespacing | manual prefixes | `namespace` |
| Abstraction | structs and function pointers | classes, templates, overloads |

---

## Embedded C++ Guidance

- Prefer deterministic patterns: RAII, `std::array`, fixed-capacity containers.
- Avoid exceptions, RTTI, and heap-heavy patterns when the target does not allow them.
- Use templates carefully; they can remove runtime overhead but increase compile time and code size.
- Understand generated code, not just syntax.

---

## Common Questions

**Q: What problem does RAII solve?**

It guarantees cleanup when an object leaves scope, which reduces leaks and half-cleaned state.

**Q: Why use templates instead of macros?**

Templates are type-safe, scoped, and checked by the compiler.

**Q: What is move semantics?**

It transfers ownership of resources from one object to another without a deep copy.

**Q: Why prefer `enum class`?**

It provides stronger typing and avoids accidental implicit conversions and global enumerator pollution.

---

## Quick Revision

- References are aliases, pointers are objects holding addresses.
- RAII ties resource cleanup to scope.
- Move semantics avoid unnecessary copies.
- Templates provide zero-overhead generic code when used well.
- `constexpr` pushes work to compile time.
- `enum class` and namespaces reduce accidental misuse.
