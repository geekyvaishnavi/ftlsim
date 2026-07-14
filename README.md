# Design and Simulation of a Flash Translation Layer for SSD Storage Systems

A simulator of an SSD's Flash Translation Layer (FTL) — modeling address mapping, garbage collection, and wear leveling — built in **C++**.

## Motivation

NAND flash cannot be overwritten in place. Every write goes to a fresh physical page, old data is invalidated rather than erased immediately, and space is reclaimed later through garbage collection, which erases whole blocks at a time. This reclamation process causes extra physical writes beyond what the host requested — write amplification — and repeated erasing wears out flash blocks over time. This project simulates that full lifecycle to study how different garbage collection and wear-leveling policies affect SSD performance and longevity.

## Objectives

- Simulate core SSD/FTL behavior: logical-to-physical address mapping, out-of-place writes, and page invalidation
- Implement and compare multiple garbage collection victim-selection policies (greedy vs cost-benefit)
- Implement and evaluate wear-leveling strategies (dynamic and static)
- Quantify write amplification under different workload patterns (sequential, random, hotspot)
- Visualize NAND state transitions and FTL operations

## Architecture

```
Host Write/Read
      |
      v
 +----------+       +------------------+
 |   FTL    | <-->  | Mapping Table    |
 | (L2P)    |       | (logical->phys)  |
 +----------+       +------------------+
      |
      v
 +----------+       +------------------+
 |   GC     | <-->  | Wear Leveling    |
 +----------+       +------------------+
      |
      v
 +----------------------------+
 |   NAND (Blocks x Pages)     |
 +----------------------------+
```

## Modules

- **NAND model** — blocks composed of pages; each page is free, valid, or invalid; each block tracks its erase count
- **FTL mapping layer** — logical-to-physical page mapping table, read/write entrypoints
- **Garbage collection engine** — two selectable policies:
  - *Greedy*: reclaims the block with the most invalid pages
  - *Cost-benefit*: weighs invalid-page count against block erase age
- **Wear-leveling engine** — dynamic (biases free-page allocation toward less-worn blocks) and static (migrates cold data out of low-erase-count blocks)
- **Workload generator** — synthetic write patterns: sequential, random, and hotspot (skewed access)
- **Metrics** — write amplification factor (WAF), erase count distribution, GC frequency
- **Visualization** — dashboard/scrubber showing NAND block/page state over time and GC/wear-leveling events

## Features

- Page-level address mapping (logical -> physical)
- Out-of-place writes with page invalidation
- Configurable GC policy (greedy / cost-benefit)
- Dynamic and static wear leveling
- Write amplification factor (WAF) and erase-count distribution metrics
- Synthetic workload generator (random, sequential, hotspot) + trace file support
- Per-timestep JSON state dump for visualization of block/page states and operations over time

## Requirements

- C++17 or later
- CMake 3.16+
- A C++ compiler (g++, clang++, or MSVC)

## Getting Started

```bash
git clone https://github.com/<your-username>/ftlsim.git
cd ftlsim

cmake -S . -B build
cmake --build build

./build/ftlsim --workload random --pages 10000 --blocks 128 --gc-policy greedy
```

## Sample Output

```
Total writes: 10000
Total erases: 42
Write Amplification Factor: 1.34
Max erase count: 12 | Min erase count: 9 (wear leveled)
```

## Visualization

The simulator dumps NAND state per timestep as JSON/JSONL. A static web frontend (`web/`) loads that trace and replays it with a scrubber:

- Blocks and pages rendered as a grid, colored by state (free / valid / invalid)
- Pages flip as writes land and out-of-place writes invalidate old pages
- Garbage collection events show valid-page copy-out and block erase
- Per-block erase counts surface wear-leveling effectiveness at a glance
- Live metrics panel (WAF, writes vs erases, GC count, erase-count spread)

Because the frontend consumes a recorded trace, it runs independently of the simulator — no live server required.

```bash
./build/ftlsim --workload hotspot --pages 10000 --blocks 128 --gc-policy cost-benefit --dump trace.jsonl
# then open web/index.html and load trace.jsonl
```

## Evaluation

Experiments are run across sequential, random, and hotspot workloads for each GC policy, comparing:

- Write amplification factor (WAF)
- Erase count distribution across blocks (wear-leveling effectiveness)
- GC trigger frequency

Full results and analysis are documented in the project report (see `/docs`).

## Project Structure

```
ftlsim/
├── src/
│   ├── nand/
│   │   ├── page.cpp
│   │   ├── block.cpp
│   │   └── nand.cpp
│   ├── ftl/
│   │   ├── mapping.cpp
│   │   ├── ftl.cpp
│   │   ├── gc.cpp
│   │   └── wearlevel.cpp
│   ├── metrics/
│   │   └── metrics.cpp
│   ├── workload/
│   │   ├── generator.cpp
│   │   └── trace.cpp
│   └── main.cpp
├── include/
│   └── ftlsim/
│       ├── nand/
│       │   ├── page.hpp
│       │   ├── block.hpp
│       │   └── nand.hpp
│       ├── ftl/
│       │   ├── mapping.hpp
│       │   ├── ftl.hpp
│       │   ├── gc.hpp
│       │   └── wearlevel.hpp
│       ├── metrics/
│       │   └── metrics.hpp
│       └── workload/
│           ├── generator.hpp
│           └── trace.hpp
├── web/                    # visualization dashboard
├── docs/                   # project report, diagrams
├── testdata/
│   └── workloads/
├── tests/
├── CMakeLists.txt
├── README.md
├── LICENSE
└── .gitignore
```

## Design Decisions

- **Victim selection**: supports both greedy (most invalid pages) and cost-benefit (invalid-page count weighted against erase age) policies for direct comparison. Policies sit behind a common interface so new strategies slot in without touching GC internals.
- **Wear leveling**: dynamic leveling biases allocation toward low erase-count blocks; static leveling periodically migrates cold data to ensure all blocks accumulate wear evenly over time.
- **Mapping granularity**: page-level, not hybrid/block-level, for simplicity and clearer invariants.
- **Sim / visualization split**: the simulator writes a recorded trace and the web dashboard replays it, keeping the C++ core free of any web/server concerns and making runs reproducible.

## Limitations

- Single-threaded, no concurrency modeling
- Synthetic/trace-based workloads only, no real block-device interface
- No power-loss/crash-consistency simulation

## Related Work

For comparison, see MQSim and FEMU, research-grade SSD simulators/emulators with far greater scope (multi-queue modeling, full-system NVMe emulation). ftlsim is a lightweight educational simulator focused on FTL mapping, GC policy comparison, and wear-leveling mechanics.

## License

MIT
