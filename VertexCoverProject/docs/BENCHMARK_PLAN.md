# Benchmark protocol (planned, not yet implemented)

This file records the proposed methodology in Section 3 of the report. Update it to reflect
what you actually run, **before** presenting measured results.

## Input families
- Random G(n,p): n = 10, 20, 40, 60, 80, 100; p = 0.1, 0.2, 0.5; seeds 0..9.
- Paths, stars, cycles: n = 10, 20, 40, 60, 80, 100.
- Square grids: 4x4, 6x6, 8x8, 10x10.
- Complete bipartite: K(5,5), K(10,10), K(20,20), K(30,30), K(40,40).
- Edge ordering experiment: 10 reproducible shuffles of the *same* path graph's edge list.

## Hardware (primary benchmark machine)
Windows 11 Pro; Intel Core i7-10700 @ 2.90 GHz, 8 cores/16 threads; 32 GB RAM.
Visual Studio / MSVC Release x64, preferably /O2. Record the exact MSVC compiler version.

## Proposed timing protocol
- Use std::chrono::steady_clock around **algorithm computation only**.
- Warm up once (untimed) and measure 5 independent repetitions of each graph and algorithm.
- Report median elapsed time in milliseconds.
- Very fast approximation runs may need a repeated-call timing batch; divide measured
  batch time by the number of calls and document the batch size.
- When the exact algorithm is available, implement a real 30-second timeout; it cannot
  merely check time *after* returning. Consider cooperative cancellation or an isolated
  child process. Label incomplete exact runs `timeout`, not `optimal`.
- Exclude graph generation, file I/O and CSV writing from time measurements.
- When exact search completes, ratio = approximation_cover_size / exact_cover_size.
  If OPT == 0, label ratio N/A rather than dividing by zero.
- Record the number of search nodes explored by exact branch-and-bound.
- Do not aggregate times from different machines in one runtime curve.

## CSV columns (planned)
family,n,m,parameter,seed,edge_order_seed,algorithm,repeat,time_ms,cover_size,
valid_cover,optimal,timeout,nodes_explored

## Reproducibility
- Document compiler version and exact CMake/Visual Studio configuration.
- Store generators, experiments, and recorded seeds in version control.
- Both implementations must receive the exact same graph and edge ordering on each
  paired benchmark, without cross-run mutation.
- The current project contains the graph generators, approximation algorithm,
  and smoke tests only. Exact solver and full benchmarking are intentionally TODO.
