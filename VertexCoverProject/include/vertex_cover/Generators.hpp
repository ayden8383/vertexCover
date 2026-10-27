#pragma once

#include "vertex_cover/Graph.hpp"
#include <cstdint>

namespace vc {

Graph makeRandomGraph(int n, double edge_probability, std::uint32_t seed);
Graph makePathGraph(int n);
Graph makeStarGraph(int n);
Graph makeCycleGraph(int n);
Graph makeGridGraph(int rows, int cols);
Graph makeCompleteBipartiteGraph(int left_size, int right_size);

} // namespace vc
