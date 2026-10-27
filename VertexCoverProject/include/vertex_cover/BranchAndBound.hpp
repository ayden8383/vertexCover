#pragma once

#include "vertex_cover/Graph.hpp"
#include <cstdint>
#include <vector>

namespace vc {

struct ExactResult {
    std::vector<int> cover;
    std::uint64_t nodes_explored = 0;
    bool optimal = false;
};

// TODO (group member): implement branch-and-bound here.
// Contract: when optimal == true, cover must be a minimum vertex cover.
// Input uses zero-based vertex IDs and is read-only.
ExactResult solveBranchAndBound(const Graph& graph);

} // namespace vc
