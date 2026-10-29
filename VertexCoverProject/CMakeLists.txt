cmake_minimum_required(VERSION 3.20)

project(VertexCoverProject VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared graph and algorithm implementations
add_library(vertex_cover_lib
    src/Graph.cpp
    src/Generators.cpp
    src/Approximation.cpp
    src/BranchAndBound.cpp
    src/Verification.cpp
)

target_include_directories(vertex_cover_lib PUBLIC include)

if(MSVC)
    target_compile_options(vertex_cover_lib PRIVATE /W4)
else()
    target_compile_options(vertex_cover_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Main demonstration program
add_executable(
    vertex_cover_demo
    src/main.cpp
)

target_link_libraries(
    vertex_cover_demo
    PRIVATE vertex_cover_lib
)

# Exact branch-and-bound runtime benchmarking
add_executable(
    vertex_cover_exact_benchmark
    tests/ExactBenchmark.cpp
)

target_link_libraries(
    vertex_cover_exact_benchmark
    PRIVATE vertex_cover_lib
)
# Correctness tests
add_executable(
    vertex_cover_tests
    tests/SmokeTests.cpp
)

target_link_libraries(
    vertex_cover_tests
    PRIVATE vertex_cover_lib
)

# Edge-order sensitivity experiment
add_executable(
    vertex_cover_edge_order
    tests/EdgeOrderExperiment.cpp
)

target_link_libraries(
    vertex_cover_edge_order
    PRIVATE vertex_cover_lib
)

# Runtime benchmarking
add_executable(
    vertex_cover_benchmark
    tests/Benchmark.cpp
)

target_link_libraries(
    vertex_cover_benchmark
    PRIVATE vertex_cover_lib
)

# Enable automated correctness tests
enable_testing()

add_test(
    NAME VertexCoverSmokeTests
    COMMAND vertex_cover_tests
)