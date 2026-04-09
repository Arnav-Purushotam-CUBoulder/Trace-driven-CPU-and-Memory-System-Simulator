# Trace-Driven CPU and Memory System Simulator

A trace-driven superscalar CPU and memory-system simulator in C++20 for studying microarchitectural tradeoffs across benchmark traces. The simulator models a simplified out-of-order core with a configurable pipeline width, reorder buffer size, branch predictor, and unified L1 data cache.

## Features

- Configurable superscalar width used for fetch, issue, and commit.
- Configurable reorder buffer (ROB) size to study memory-level parallelism and window pressure.
- Simple branch prediction with `always_taken`, `always_not_taken`, `one_bit`, and `two_bit` predictors.
- Direct-mapped or set-associative cache modeling with configurable capacity, line size, associativity, hit latency, and miss penalty.
- Detailed statistics for IPC, branch accuracy, miss rate, frontend stalls, ROB stalls, issue stalls, and memory-induced commit stalls.
- Synthetic sample traces plus a Python sweep script for generating CSV summaries and optional plots.

## Simplified Microarchitecture Model

This project is intentionally compact and educational rather than industrial-strength or cycle-accurate:

- Instructions are fetched from an execution trace rather than from a binary.
- The frontend can fetch up to `width` instructions per cycle unless it is blocked by a branch misprediction penalty or a full ROB.
- The backend issues up to `width` instructions per cycle out of the ROB.
- Loads and stores access a unified data cache when issued.
- Cache hits complete after `cache_hit_latency`; misses incur `cache_hit_latency + cache_miss_penalty`.
- Branches are predicted at fetch time and update the predictor when they complete.
- Commits happen in order from the head of the ROB, which makes long memory misses visible as commit stalls.

This is enough to study the impact of width, ROB depth, control-flow behavior, and cache conflicts on performance metrics such as IPC and stall cycles.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Quick Start

Run the simulator on one of the included traces:

```bash
./build/td_sim --trace traces/mixed.trace
./build/td_sim --trace traces/branchy.trace --width 2 --rob 32 --predictor two_bit --cache-assoc 4 --json
```

Sample output from `traces/mixed.trace` with `width=4`, `rob=64`, a two-bit branch predictor, and a 4-way 16KB cache:

```text
Trace: mixed.trace
Config:
  width=4, rob=64, predictor=two_bit, cache=16KB 4-way set-associative, line=64B, sets=64
Performance:
  cycles=878, committed=768, IPC=0.8747
Branching:
  branches=149, mispredictions=55, accuracy=63.0872%
Cache:
  accesses=405, hits=31, misses=374, miss_rate=92.3457%
Stalls:
  branch_frontend=330, rob_frontend=267, issue=647, commit=652, head_memory=651
```

Run a small experiment sweep:

```bash
python3 scripts/run_experiments.py --plot
```

The sweep writes:

- `results/experiments.csv`
- `results/summary.md`
- optional PNG charts when `matplotlib` is installed

## Trace Format

Each non-comment line is one instruction:

```text
ALU    <pc>
LOAD   <pc> <address>
STORE  <pc> <address>
BRANCH <pc> <T|N>
```

Examples:

```text
ALU 0x400000
LOAD 0x400004 0x100000
STORE 0x400008 0x100040
BRANCH 0x40000c T
```

Blank lines and lines beginning with `#` are ignored.

## CLI Options

```text
--trace <path>
--width <n>
--rob <n>
--cache-size <bytes>
--cache-line-size <bytes>
--cache-assoc <n>
--cache-hit-latency <cycles>
--cache-miss-penalty <cycles>
--predictor <always_taken|always_not_taken|one_bit|two_bit>
--bp-entries <n>
--mispredict-penalty <cycles>
--format <text|json>
--json
```

## Repository Layout

```text
include/                 public simulator interfaces
src/                     simulator implementation
tests/                   unit tests
traces/                  sample deterministic traces
scripts/generate_sample_traces.py
scripts/run_experiments.py
.github/workflows/ci.yml
```

## Resume Framing

This repo is structured to support the following resume bullets:

- Built a trace-driven CPU and memory-system simulator in C++ modeling a simplified out-of-order core with configurable pipeline width, reorder buffer size, and cache parameters.
- Implemented branch prediction and cache modeling to study the impact of control flow and memory behavior on overall performance.
- Evaluated design tradeoffs across benchmark traces by measuring IPC, miss rates, and stall behavior under different microarchitectural configurations.

## Notes

- The simulator does not model register dependencies, load/store ordering hazards, multi-level caches, or wrong-path execution.
- Those simplifications keep the code approachable while still making width, ROB depth, branch behavior, and cache associativity visible in the reported metrics.
