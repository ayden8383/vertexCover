#include "vertex_cover/Approximation.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"

#include <iostream>

int main() {
    // A six-vertex path, using the same family as Section 3.1.2.
    const vc::Graph graph = vc::makePathGraph(6);
    const vc::ApproximationResult result = vc::solveApproximation(graph);
    const int optimum = vc::bruteForceMinimumCoverSize(graph);

    std::cout << "Minimum Vertex Cover -- starter project\n";
    std::cout << "Graph: path with " << graph.vertexCount() << " vertices and "
              << graph.edges().size() << " edges\n";
    std::cout << "2-approximation cover size: " << result.cover.size() << "\n";
    std::cout << "Verified minimum size (brute force): " << optimum << "\n";
    std::cout << "Cover is valid: "
              << (vc::isVertexCover(graph, result.cover) ? "yes" : "NO") << "\n";
    std::cout << "Selected vertices (zero-based IDs): ";
    for (int v : result.cover) std::cout << v << ' ';
    std::cout << '\n';
    std::cout << "\nNext: integrate branch-and-bound, then create the unified benchmark.\n";
}
