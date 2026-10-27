#pragma once

#include <vector>

namespace vc {

struct Edge {
    int u;
    int v;
};

// Simple, undirected graph with zero-based vertex IDs 0 .. n-1.
// Generators ensure no duplicate edges; addEdge() checks bounds and self-loops.
class Graph {
public:
    explicit Graph(int vertex_count);

    int vertexCount() const noexcept;
    const std::vector<Edge>& edges() const noexcept;
    void addEdge(int u, int v);

private:
    int vertex_count_;
    std::vector<Edge> edges_;
};

} // namespace vc
