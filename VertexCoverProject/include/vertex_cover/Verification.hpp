#pragma once

#include "vertex_cover/Graph.hpp"
#include <vector>

namespace vc {

bool isVertexCover(const Graph& graph, const std::vector<int>& cover);

// Independent reference for small graphs ONLY. Returns the minimum cover size.
// Deliberately limited to <= 16 vertices to avoid excessive brute-force work.
int bruteForceMinimumCoverSize(const Graph& graph);

} // namespace vc
