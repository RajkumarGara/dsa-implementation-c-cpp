# Data Structures & OS MCQs

> Multiple-choice questions on data structures, algorithms, and operating system concepts as they apply to embedded and systems programming.

---

## Data Structures

### Q1. What is the time complexity of accessing an element in an array by index?

A) O(n)  
B) O(log n)  
C) **O(1)** ✓  
D) O(n²)

**Explanation:** Array elements are stored in contiguous memory. The address is computed directly: `base + index * sizeof(element)`. No traversal needed.

---

### Q2. What data structure is best for implementing a UART receive buffer?

A) Stack  
B) Binary tree  
C) **Circular (ring) buffer** ✓  
D) Hash table

**Explanation:** A circular buffer provides O(1) enqueue/dequeue, fixed memory footprint, and works perfectly for producer-consumer patterns (ISR produces, main loop consumes).

---

### Q3. What is the time complexity of insertion at the head of a singly linked list?

A) O(n)  
B) O(log n)  
C) **O(1)** ✓  
D) O(n²)

**Explanation:** Insert at head: create new node, point its `next` to current head, update head. No traversal needed. Insertion at the tail is O(n) without a tail pointer.

---

### Q4. What is a stack overflow in the context of the stack data structure?

A) Writing past array bounds  
B) Memory leak  
C) **Pushing onto a full stack** ✓  
D) Popping from an empty stack

**Explanation:** Stack overflow = push when full. Stack underflow = pop when empty. In embedded context, this also refers to the call stack growing beyond its allocated region.

---

### Q5. What is the main advantage of a hash table over a sorted array for lookups?

A) Less memory usage  
B) Maintains order  
C) **O(1) average-case lookup vs O(log n)** ✓  
D) Simpler implementation

**Explanation:** Hash tables provide constant-time average lookup. Sorted arrays need binary search (O(log n)). Trade-off: hash tables use more memory and have O(n) worst case.

---

### Q6. In which traversal does a binary search tree produce sorted output?

A) Pre-order  
B) Post-order  
C) **In-order** ✓  
D) Level-order

**Explanation:** In-order traversal (left → root → right) visits BST nodes in ascending order. This is a defining property of BSTs.

---

### Q7. What is the worst-case time complexity of quicksort?

A) O(n)  
B) O(n log n)  
C) **O(n²)** ✓  
D) O(log n)

**Explanation:** Worst case occurs when the pivot is always the smallest or largest element (already sorted input with naive pivot). Average case is O(n log n).

---

### Q8. Which data structure is used to implement function call tracking in a program?

A) Queue  
B) **Stack (call stack)** ✓  
C) Linked list  
D) Tree

**Explanation:** Function calls push a stack frame (return address, locals, saved registers). Returns pop the frame. LIFO order matches nested function calls.

---

### Q9. What is the space complexity of a circular buffer with capacity N?

A) O(1)  
B) O(log n)  
C) **O(N) — fixed at compile time** ✓  
D) O(n²)

**Explanation:** A circular buffer uses a fixed array of size N. No dynamic allocation needed — ideal for embedded systems with deterministic memory requirements.

---

### Q10. What is the primary use of a priority queue in embedded systems?

A) Sorting data  
B) UART buffering  
C) **Task scheduling (run highest priority task next)** ✓  
D) Memory allocation

**Explanation:** RTOS schedulers use priority queues to determine which ready task to execute. The ready list is essentially a priority queue of task control blocks.

---

## Operating Systems — Processes & Threads

### Q11. What is the key difference between a process and a thread?

A) Threads are faster  
B) Processes share memory  
C) **Processes have separate address spaces; threads share the same address space** ✓  
D) Threads can't be scheduled

**Explanation:** Threads within a process share code, data, and heap. They have separate stacks and register contexts. Processes have fully isolated address spaces.

---

### Q12. What is a context switch?

A) Changing the CPU clock speed  
B) Switching between programs  
C) **Saving the current task's state and restoring another task's state** ✓  
D) Resetting the processor

**Explanation:** A context switch saves registers (PC, SP, general-purpose), status flags, and possibly the stack pointer to the outgoing task's TCB, then loads the incoming task's state.

---

### Q13. What is the difference between preemptive and cooperative scheduling?

A) Preemptive is simpler  
B) Cooperative is more responsive  
C) **Preemptive: kernel forces task switches. Cooperative: tasks must yield voluntarily** ✓  
D) No practical difference

**Explanation:** Preemptive scheduling ensures high-priority tasks run promptly. Cooperative scheduling relies on tasks calling `yield()` — a misbehaving task can starve others.

---

### Q14. What is a race condition?

A) Two CPUs running at different speeds  
B) **Outcome depends on the non-deterministic timing of concurrent operations** ✓  
C) Running too many processes  
D) CPU cache miss

**Explanation:** A race condition occurs when multiple threads/tasks access shared data without proper synchronization, leading to results that depend on execution ordering.

---

### Q15. What is the difference between a deadlock and a livelock?

A) No difference  
B) Deadlock is temporary  
C) **Deadlock: tasks blocked forever. Livelock: tasks actively change state but make no progress** ✓  
D) Livelock only happens in hardware

**Explanation:** In deadlock, tasks are permanently stuck waiting. In livelock, tasks repeatedly respond to each other without making forward progress (like two people stepping aside into each other's path).

---

## Operating Systems — Synchronization

### Q16. What are the four necessary conditions for deadlock?

A) Race, starvation, livelock, priority inversion  
B) **Mutual exclusion, hold and wait, no preemption, circular wait** ✓  
C) Lock, unlock, wait, signal  
D) FIFO, LIFO, priority, round-robin

**Explanation:** All four conditions (Coffman conditions) must hold simultaneously for deadlock. Breaking any one prevents deadlock. Most common fix: impose a lock ordering (breaks circular wait).

---

### Q17. What is a semaphore?

A) A type of memory  
B) **A signaling mechanism with a counter for controlling access to shared resources** ✓  
C) A scheduling algorithm  
D) A hardware interrupt

**Explanation:** A counting semaphore has an integer count. `wait()` decrements (blocks if 0). `signal()` increments. Binary semaphore: count is 0 or 1. Used for synchronization and resource counting.

---

### Q18. What is a spinlock best suited for?

A) Long critical sections  
B) Userspace applications  
C) **Very short critical sections on multiprocessor systems** ✓  
D) Replacing mutex in single-core RTOS

**Explanation:** Spinlocks busy-wait instead of sleeping. They're efficient for very short critical sections on multi-core systems where the cost of sleeping/waking exceeds the spin time. Wasteful on single-core.

---

### Q19. What is the producer-consumer problem?

A) Manufacturing optimization  
B) **Synchronizing tasks where one produces data and another consumes it** ✓  
C) Memory allocation pattern  
D) CPU scheduling problem

**Explanation:** Classic synchronization problem: a producer fills a buffer, a consumer empties it. Requires synchronization to prevent overflow (full buffer), underflow (empty buffer), and data corruption.

---

### Q20. What is a condition variable used for?

A) Boolean flag checking  
B) **Allowing a thread to wait until a particular condition is true** ✓  
C) Managing priority inheritance  
D) Interrupt handling

**Explanation:** A condition variable lets a thread atomically release a mutex and sleep until signaled. On wakeup, it reacquires the mutex. Used with `pthread_cond_wait()` / `pthread_cond_signal()`.

---

## Operating Systems — Memory Management

### Q21. What is virtual memory?

A) RAM extension using ROM  
B) Cache memory  
C) **An abstraction that gives each process its own address space, mapped to physical memory** ✓  
D) Memory used by VMs

**Explanation:** Virtual memory provides isolation (each process sees its own flat address space), enables paging (swap to disk), and allows memory protection (read/write/execute permissions per page).

---

### Q22. What is a page fault?

A) Memory corruption  
B) **Access to a virtual page not currently in physical memory** ✓  
C) Segmentation fault  
D) Stack overflow

**Explanation:** A page fault triggers the OS to load the required page from disk (or allocate it). A minor page fault needs no disk I/O; a major fault requires loading from swap.

---

### Q23. Why is dynamic memory allocation generally avoided in safety-critical embedded systems?

A) It's too slow  
B) C doesn't support it  
C) **Non-deterministic timing and risk of fragmentation/exhaustion** ✓  
D) It uses too much power

**Explanation:** `malloc()`/`free()` have non-deterministic execution time, can cause heap fragmentation (eventually failing to allocate), and are hard to verify for worst-case analysis.

---

### Q24. What is a memory pool (fixed-block allocator)?

A) Virtual memory region  
B) **Pre-allocated set of fixed-size blocks for deterministic allocation** ✓  
C) Garbage collection area  
D) Stack region

**Explanation:** Memory pools allocate blocks of uniform size from a pre-allocated array. O(1) allocation/deallocation with no fragmentation. Widely used in RTOS and embedded.

---

### Q25. What is the difference between internal and external fragmentation?

A) **Internal: wasted space within an allocated block. External: wasted space between blocks** ✓  
B) Internal: inside a function. External: between functions  
C) Internal: RAM. External: ROM  
D) No practical difference

**Explanation:** Internal fragmentation: allocated block is larger than needed (e.g., 128-byte block for 100-byte request). External: free memory exists but is scattered — no single contiguous block is large enough.

---

## Quick Reference

| Topic | Key fact |
|---|---|
| Circular buffer | O(1) enqueue/dequeue, fixed memory — ideal for embedded |
| Array access | O(1) by index via pointer arithmetic |
| Linked list insert at head | O(1), tail is O(n) without tail pointer |
| Hash table lookup | O(1) average, O(n) worst case |
| Context switch | Save/restore registers, PC, SP between tasks |
| Deadlock conditions | Mutual exclusion + hold & wait + no preemption + circular wait |
| Mutex vs semaphore | Mutex: ownership + priority inheritance. Semaphore: counting + signaling |
| Virtual memory | Per-process address space → physical mapping via page tables |
| Memory pools | Fixed-block, O(1), no fragmentation — preferred in embedded |
| Race condition | Unsynchronized concurrent access to shared data |
