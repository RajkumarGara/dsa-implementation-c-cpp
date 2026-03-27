# embedded-systems-prep

Comprehensive embedded systems interview preparation — C core concepts, data structures & algorithms, operating systems, embedded drivers, system design, and more.

The `c_core/` section now keeps its reference docs directly in one folder for faster browsing, including C fundamentals plus focused C++ interview notes.

## Repository Structure

```
embedded-systems-prep/
├── c_core/                     # Flat reference docs for C fundamentals and C++ concepts/OOP
├── data_structures_algorithms/ # DSA implementations in C
├── operating_systems/          # Processes, threads, synchronization, scheduling
├── embedded_drivers/           # UART, I2C, SPI, CAN, GPIO, ADC, PWM, timers, interrupts
├── embedded_concepts/          # Boot flow, memory maps, registers, endianness, volatile/const, linker scripts
├── interview_prep/             # Tricky C questions, MCQs, debugging cases
└── README.md
```

## Build

Run all commands from the repository root:

```sh
make        # build
make run    # run the default program
make clean  # clean build artifacts
```

Run a specific program:

```sh
make run-one PROGRAM=data_structures_algorithms/data_structures/array_adt
```
