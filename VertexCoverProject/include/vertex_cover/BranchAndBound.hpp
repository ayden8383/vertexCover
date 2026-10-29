#pragma once

#include "vertex_cover/Graph.hpp"
#include <cstdint>
#include <vector>

namespace vc {

// Each pruning technique can be switched off, so experiments can measure what
// each one contributes (report section 4.3). Everything is on by default.
struct ExactOptions {
    bool degreeOneRule = true;     // deg(u) = 1               -> take u's neighbour
    bool triangleRule = true;      // deg(u) = 2, nbrs adjacent -> take both neighbours
    bool highDegreeRule = true;    // deg(v) > remaining budget -> v must be taken
    bool matchingBound = true;     // lower bound: size of a maximal matching
    bool cliqueCoverBound = true;  // lower bound: sum over a clique partition of (size - 1)
    bool greedyStart = true;       // start from a greedy cover instead of "every non-isolated vertex"
    bool pathsAndCycles = true;    // max degree <= 2 -> solve the paths and cycles directly, no branching
    double timeLimitSeconds = 0;   // 0 = no limit
};

struct ExactResult {
    std::vector<int> cover;            // Zero-based vertex IDs, in increasing order
    std::uint64_t nodes_explored = 0;  // search-tree nodes visited
    bool optimal = false;              // false only if the time limit stopped the search early

    std::uint64_t pruned = 0;  // nodes cut off by a bound instead of being branched on
    std::uint64_t degree_one_applied = 0;
    std::uint64_t triangle_applied = 0;
    std::uint64_t high_degree_applied = 0;
    int initial_upper_bound = 0;  // size of the starting cover
    int root_matching_bound = 0;  // matching lower bound on the whole input graph
    int root_clique_bound = 0;    // clique-cover lower bound on the whole input graph
    double seconds = 0.0;
};

// Exact minimum vertex cover by branch-and-bound.
// Contract: when optimal == true, cover must be a minimum vertex cover.
// Input uses zero-based vertex IDs and is read-only.
ExactResult solveBranchAndBound(const Graph& graph, const ExactOptions& options = {});

} // namespace vc
