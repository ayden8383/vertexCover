#include "vertex_cover/Generators.hpp"

#include <random>
#include <stdexcept>

namespace vc {

Graph makeRandomGraph(int n, double p, std::uint32_t seed) {
    if (!(p >= 0.0 && p <= 1.0)) {
        throw std::invalid_argument("Edge probability must be between 0 and 1");
    }
    Graph graph(n);
    std::mt19937 rng(seed);
    std::bernoulli_distribution include_edge(p);
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (include_edge(rng)) {
                graph.addEdge(u, v);
            }
        }
    }
    return graph;
}

Graph makePathGraph(int n) {
    Graph graph(n);
    for (int i = 1; i < n; ++i) {
        graph.addEdge(i - 1, i);
    }
    return graph;
}

Graph makeStarGraph(int n) {
    Graph graph(n);
    for (int i = 1; i < n; ++i) {
        graph.addEdge(0, i);
    }
    return graph;
}

Graph makeCycleGraph(int n) {
    if (n < 3) {
        throw std::invalid_argument("A simple cycle needs at least 3 vertices");
    }
    Graph graph = makePathGraph(n);
    graph.addEdge(0, n - 1);
    return graph;
}

Graph makeGridGraph(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    Graph graph(rows * cols);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int u = row * cols + col;
            if (col + 1 < cols) graph.addEdge(u, u + 1);
            if (row + 1 < rows) graph.addEdge(u, u + cols);
        }
    }
    return graph;
}

Graph makeCompleteBipartiteGraph(int left_size, int right_size) {
    if (left_size < 0 || right_size < 0) {
        throw std::invalid_argument("Partition sizes must be nonnegative");
    }
    Graph graph(left_size + right_size);
    for (int u = 0; u < left_size; ++u) {
        for (int j = 0; j < right_size; ++j) {
            graph.addEdge(u, left_size + j);
        }
    }
    return graph;
}

} // namespace vc
