#include "vertex_cover/Approximation.hpp"

namespace vc {

ApproximationResult solveApproximation(const Graph& graph) {
    const int n = graph.vertexCount();
    std::vector<char> matched(static_cast<std::size_t>(n), false);
    ApproximationResult result;

    for (const Edge& edge : graph.edges()) {
        if (!matched[edge.u] && !matched[edge.v]) {
            matched[edge.u] = true;
            matched[edge.v] = true;
            result.matching.push_back(edge);
        }
    }

    // Every matched endpoint belongs to the vertex cover.
    for (int v = 0; v < n; ++v) {
        if (matched[v]) {
            result.cover.push_back(v);
        }
    }
    return result;
}

} // namespace vc
