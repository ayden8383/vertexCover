#pragma once

#include "vertex_cover/Graph.hpp"
#include <vector>

namespace vc {

struct ApproximationResult {
    std::vector<int> cover; // Zero-based vertex IDs
    std::vector<Edge> matching;
};

// Greedy maximal matching; take both endpoints of every selected edge.
// Time: O(n + m). Additional space (excluding input): O(n).
ApproximationResult solveApproximation(const Graph& graph);

} // namespace vc
