# Minimum Vertex Cover — C++ Project

Starter repository for the university comparison of **2-Approximation via Maximal
Matching** and **Exact Branch-and-Bound**.

## Status

- Working: graph representation; six graph-family generators; greedy maximal-matching
  2-approximation; a small independent brute-force verifier; smoke tests.
- Not yet implemented: branch-and-bound (placeholder in `src/BranchAndBound.cpp`);
  unified benchmarking framework and CSV output.
- The project is set up for C++17 and CMake. It can be opened directly in Visual Studio.

## Open in Microsoft Visual Studio

1. Install Visual Studio with the **Desktop development with C++** workload.
   Include MSVC, a Windows SDK, and the CMake tools.
2. Extract the ZIP, e.g. into `Documents\\VertexCoverProject`.
3. Open Visual Studio > **File > Open > Folder** and select the folder containing
   `CMakeLists.txt`. Visual Studio will configure the CMake project.
4. Select the **x64-Release** configuration (if available) and build the project.
5. Select `vertex_cover_demo` as the startup target and run it (Ctrl+F5).
6. To run smoke tests, select and run `vertex_cover_tests`, or run CTest in the
   project's build directory.
7. MSVC `/O2` in Release can be checked under the CMake settings/build output.
   For benchmark results, confirm the exact compiler flags and version (`cl /Bv`).

If Visual Studio does not recognise the project, verify that the **Desktop development
with C++** and CMake components are installed. You can also configure and build with
CMake from a Visual Studio Developer Command Prompt.


## Individual Work

- Ayden Tran: `src/Approximation.cpp`, `include/vertex_cover/Approximation.hpp`.
- Hoang Hung Nguyen: `src/BranchAndBound.cpp`, `include/vertex_cover/BranchAndBound.hpp`.
- Shared: `Graph`, `Generators`, `Verification`, `tests`, and later benchmarks.
- Use separate branches and pull requests before merging to `main`.
- Never commit `out/`, `.vs/`, binaries, large input datasets, or personal files.

## Example build outside Visual Studio

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## File layout

```text
VertexCoverProject/
  CMakeLists.txt
  .gitignore
  README.md
  docs/BENCHMARK_PLAN.md
  include/vertex_cover/Graph.hpp
  include/vertex_cover/Generators.hpp
  include/vertex_cover/Approximation.hpp
  include/vertex_cover/BranchAndBound.hpp
  include/vertex_cover/Verification.hpp
  src/Graph.cpp
  src/Generators.cpp
  src/Approximation.cpp
  src/BranchAndBound.cpp    
  src/Verification.cpp
  src/main.cpp
  tests/SmokeTests.cpp
  results/.gitkeep
```
