# CPUSim - A Pipelined CPU & Memory Hierarchy Simulator

## Overview
This program simulates a CPU task scheduler combined with a 3-level cache + RAM memory hierarchy.

## Scheduler
**Algorithm:** Round Robin (quantum = 3 cycles)

**Justification:** Round Robin is simple, fair, and ensures no task starves — every task gets a fixed CPU slice before moving to the back of the queue. It's easy to reason about and visualize cycle-by-cycle, which fits the project's clarity requirement.

## Memory Hierarchy
| Level | Capacity | Latency |
|-------|----------|---------|
| L1    | 32 slots | 4 cycles |
| L2    | 128 slots | 12 cycles |
| L3    | 512 slots | 40 cycles |
| RAM   | unlimited | 200 cycles |

On a memory request:
1. Check L1 → L2 → L3 → RAM in order
2. On hit, return the block (pay that level's latency)
3. On miss at all levels, fetch from RAM and promote the block into L1
4. Eviction policy: FIFO (oldest entry evicted when a level is full)

## How It Works
- Each task has a burst time and a list of memory blocks it needs
- One memory block is accessed per CPU cycle the task runs
- At every cycle, the program prints the current state of L1, L2, and L3
- At the end, it prints total cycles, tasks completed, scheduler used, and total RAM accesses

## How to Build & Run
```bash
g++ -std=c++17 task2.cpp -o task2
./task2
```

Make sure `input.txt` is in the same directory.

## Input Format
Each line in `input.txt`:

Example:

TASK T1 BURST 5 MEM M1 M4 M7
TASK T2 BURST 3 MEM M2 M1

- `<ID>` — unique task identifier (e.g., T1, T2)
- `<burst_time>` — number of CPU cycles the task needs
- `MEM <block1> <block2> ...` — list of memory blocks the task will access (cycled through, one per CPU cycle)

## Code Structure
All logic is contained in `taskk2.cpp`:
- **`Task` struct** — stores task ID, burst time, remaining burst, memory block list, and current memory access index
- **`CacheLevel` struct** — represents one cache level (L1/L2/L3) with its block queue, capacity, and latency
- **`isInCache`** — checks whether a block exists in a given cache level
- **`removeFromCache`** — removes a block from a cache level (used during promotion)
- **`addToCache`** — adds a block to a cache level, evicting the oldest (FIFO) if full
- **`dump`** — returns a string representation of a cache level's current contents
- **`readTasks`** — parses `input.txt` into a list of `Task` objects
- **`main`** — runs the Round Robin scheduling loop, performs cache lookups/promotions/evictions for each memory access, prints cycle-by-cycle output, and prints final results

## Sample Output
Cycle 1 - Running: T1 Requesting: M1 L1: [] >> MISS L2: [] >> MISS L3: [] >> MISS

Fetching from RAM L1: [M1] L2: [] L3: []

Cycle 2 - Running: T1 Requesting: M4 L1: [M1] >> MISS L2: [] >> MISS L3: [] >> MISS

Fetching from RAM L1: [M4, M1] L2: [] L3: []
...

Cycle 5 - Running: T2 Requesting: M1 L1: [M2, M7, M4, M1] -> HIT L2: [] L3: []
...

=== Final Results ===
Total Cycles: 72
Tasks Completed: 15
Scheduler: Round Robin (quantum = 3)
RAM Accesses: 15