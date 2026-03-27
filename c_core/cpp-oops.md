# C++ OOP

> C++ keeps object-oriented features close to the machine. In embedded work, you need to understand both the language model and the runtime cost of each feature.

---

## The Four OOP Pillars

| Pillar | Meaning | C++ Mechanism |
|---|---|---|
| Encapsulation | Bind data and behavior together | `class`, `private`, `public`, `protected` |
| Abstraction | Expose only what the user needs | interfaces, pure virtual functions |
| Inheritance | Build a derived type from a base type | `class Derived : public Base` |
| Polymorphism | One interface, many implementations | virtual dispatch, overriding |

---

## Class vs Struct

In C++, `class` and `struct` are almost the same.

```cpp
struct RegisterConfig {
    int address;
    int mask;
};

class Uart {
private:
    int baudRate;

public:
    void init();
};
```

**Difference:**
- `struct` defaults to `public`
- `class` defaults to `private`

Use `struct` for passive data holders and `class` when you want invariants and controlled APIs.

---

## Encapsulation

```cpp
class RingBuffer {
private:
    int data[8]{};
    int head{0};
    int tail{0};
    int count{0};

public:
    bool push(int value) {
        if (count == 8) {
            return false;
        }
        data[head] = value;
        head = (head + 1) % 8;
        ++count;
        return true;
    }

    bool pop(int &value) {
        if (count == 0) {
            return false;
        }
        value = data[tail];
        tail = (tail + 1) % 8;
        --count;
        return true;
    }
};
```

The object protects its own state. Callers cannot corrupt `head`, `tail`, or `count` directly.

---

## Abstraction with Interfaces

```cpp
class UartDriver {
public:
    virtual void init() = 0;
    virtual void writeByte(char byte) = 0;
    virtual ~UartDriver() = default;
};

class MockUartDriver : public UartDriver {
public:
    void init() override {
    }

    void writeByte(char byte) override {
        (void)byte;
    }
};
```

A pure virtual function makes the class abstract. This is useful when you want to swap a hardware driver with a mock in tests.

---

## Inheritance

```cpp
class Peripheral {
public:
    void enableClock() {
    }
};

class SpiPeripheral : public Peripheral {
public:
    void configureMode(int mode) {
        (void)mode;
    }
};
```

**Inheritance types:**
- `public`: public base members stay public in the derived class
- `protected`: public and protected base members become protected
- `private`: public and protected base members become private

Prefer `public` inheritance only when the derived type is truly a kind of the base type.

---

## Polymorphism

### Compile-time polymorphism

```cpp
class Adc {
public:
    void sample(int channel) {
        (void)channel;
    }

    void sample(int channel, int averaging) {
        (void)channel;
        (void)averaging;
    }
};
```

This is function overloading. The compiler chooses the overload.

### Run-time polymorphism

```cpp
class Logger {
public:
    virtual void write(const char *msg) = 0;
    virtual ~Logger() = default;
};

class SerialLogger : public Logger {
public:
    void write(const char *msg) override {
        (void)msg;
    }
};

void logBootMessage(Logger &logger) {
    logger.write("boot ok");
}
```

This uses a vtable. It adds flexibility but costs some memory and an indirect call.

---

## Constructors and Destructors

```cpp
class GpioPin {
public:
    GpioPin(int port, int pin)
        : port_(port), pin_(pin) {
    }

    ~GpioPin() {
    }

private:
    int port_;
    int pin_;
};
```

**Key points:**
- Constructor initializes the object
- Destructor runs when the object leaves scope
- Prefer member initializer lists instead of assignment inside the constructor body

---

## Virtual Destructors

If a class is used polymorphically, its destructor should usually be virtual.

```cpp
class Device {
public:
    virtual ~Device() = default;
};

class Sensor : public Device {
public:
    ~Sensor() override = default;
};
```

Without a virtual destructor, deleting a derived object through a base pointer is undefined behavior.

---

## Access Specifiers

| Specifier | Accessible inside class | Accessible in derived class | Accessible outside |
|---|---|---|---|
| `private` | Yes | No | No |
| `protected` | Yes | Yes | No |
| `public` | Yes | Yes | Yes |

Prefer `private` by default. Use `protected` sparingly.

---

## `this` Pointer

Every non-static member function gets an implicit `this` pointer.

```cpp
class Counter {
public:
    void increment() {
        this->value++;
    }

private:
    int value{0};
};
```

`this` points to the current object.

---

## Static Members

```cpp
class Watchdog {
public:
    static int timeoutMs;
};

int Watchdog::timeoutMs = 1000;
```

Static data is shared by all objects. Static member functions do not get a `this` pointer.

---

## Operator Overloading

```cpp
class FixedPoint {
public:
    explicit FixedPoint(int raw) : raw_(raw) {
    }

    FixedPoint operator+(const FixedPoint &other) const {
        return FixedPoint(raw_ + other.raw_);
    }

private:
    int raw_;
};
```

Use operator overloading only when it makes code clearer and preserves expected semantics.

---

## Object Slicing

```cpp
class Base {
public:
    virtual void info() {
    }
};

class Derived : public Base {
public:
    void info() override {
    }
};

Derived derived;
Base base = derived;   // slicing: derived part is discarded
```

Pass polymorphic objects by reference or pointer to avoid slicing.

---

## Composition vs Inheritance

Prefer composition when one object uses another instead of being a specialized version of it.

```cpp
class SpiBus {
public:
    void transfer() {
    }
};

class ExternalFlash {
public:
    explicit ExternalFlash(SpiBus &bus) : bus_(bus) {
    }

    void read() {
        bus_.transfer();
    }

private:
    SpiBus &bus_;
};
```

**Rule of thumb:**
- inheritance for "is-a"
- composition for "has-a"

Composition is usually easier to test and maintain.

---

## Embedded C++ Notes

- Avoid dynamic allocation unless the project explicitly allows it.
- Be careful with virtual functions in very small microcontrollers because of ROM and RAM overhead.
- Use RAII where possible for lock guards, register restore helpers, and deterministic cleanup.
- Prefer stack allocation and fixed-size containers when resource limits are strict.

---

## Common Questions

**Q: Why use a class instead of a struct?**

Use a class when the type must preserve invariants and hide implementation details.

**Q: What is a vtable?**

A compiler-generated table of virtual function addresses used for run-time polymorphism.

**Q: Why is a virtual destructor important?**

It ensures the correct derived destructor runs when deleting through a base pointer.

**Q: What is object slicing?**

Copying a derived object into a base object by value discards the derived portion.

---

## Quick Revision

- `class` is `private` by default, `struct` is `public` by default.
- Encapsulation protects state.
- Abstraction exposes behavior through a clean interface.
- Inheritance models type relationships.
- Polymorphism can be compile-time or run-time.
- Prefer composition unless inheritance is clearly justified.
