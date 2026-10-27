#include "vertex_cover/Verification.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace vc {

bool isVertexCover(const Graph& graph, const std::vector<int>& cover) {
    std::vector<char> chosen(static_cast<std::size_t>(graph.vertexCount()), false);
    for (int v : cover) {
        if (v < 0 || v >= graph.vertexCount()) {
            return false;
        }
        chosen[v] = true;
    }
    for (const Edge& edge : graph.edges()) {
        if (!chosen[edge.u] && !chosen[edge.v]) {
            return false;
        }
    }
    return true;
}

int bruteForceMinimumCoverSize(const Graph& graph) {
    const int n = graph.vertexCount();
    if (n > 16) {
        throw std::invalid_argument("Brute-force verification supports at most 16 vertices");
    }
    int best = n;
    const std::uint64_t limit = std::uint64_t{1} << n;

    for (std::uint64_t mask = 0; mask < limit; ++mask) {
        int size = 0;
        for (int v = 0; v < n; ++v) {
            if ((mask & (std::uint64_t{1} << v)) != 0) {
                ++size;
            }
        }
        if (size >= best) continue;

        bool valid = true;
        for (const Edge& edge : graph.edges()) {
            const bool u_selected = (mask & (std::uint64_t{1} << edge.u)) != 0;
            const bool v_selected = (mask & (std::uint64_t{1} << edge.v)) != 0;
            if (!u_selected && !v_selected) {
                valid = false;
                break;
            }
        }
        if (valid) best = size;
    }
    return best;
}

} // namespace vc
