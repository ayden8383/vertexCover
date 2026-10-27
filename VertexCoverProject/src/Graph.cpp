#include "vertex_cover/Graph.hpp"

#include <stdexcept>
#include <utility>

namespace vc {

Graph::Graph(int vertex_count) : vertex_count_(vertex_count) {
    if (vertex_count < 0) {
        throw std::invalid_argument("Vertex count must be nonnegative");
    }
}

int Graph::vertexCount() const noexcept { return vertex_count_; }

const std::vector<Edge>& Graph::edges() const noexcept { return edges_; }

void Graph::addEdge(int u, int v) {
    if (u < 0 || v < 0 || u >= vertex_count_ || v >= vertex_count_) {
        throw std::out_of_range("Edge vertex ID is out of range");
    }
    if (u == v) {
        throw std::invalid_argument("Self-loops are not supported");
    }
    if (u > v) {
        std::swap(u, v);
    }
    edges_.push_back({u, v});
}

} // namespace vc
