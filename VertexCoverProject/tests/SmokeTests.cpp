#include "vertex_cover/Approximation.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void verifySmallGraph(const vc::Graph& graph, const std::string& label) {
    const vc::ApproximationResult result = vc::solveApproximation(graph);
    require(vc::isVertexCover(graph, result.cover), label + ": returned cover is invalid");
    require(result.cover.size() == 2 * result.matching.size(),
            label + ": selected vertices do not match matching endpoints");
    const int optimum = vc::bruteForceMinimumCoverSize(graph);
    require(static_cast<int>(result.cover.size()) >= optimum,
            label + ": cover size smaller than known optimum");
    require(static_cast<int>(result.cover.size()) <= 2 * optimum,
            label + ": violates the factor-2 guarantee");
}

} // namespace

int main() {
    try {
        verifySmallGraph(vc::Graph(6), "Empty graph");
        vc::Graph single_edge(2);
        single_edge.addEdge(0, 1);
        verifySmallGraph(single_edge, "Single edge");
        require(vc::bruteForceMinimumCoverSize(vc::makePathGraph(6)) == 3,
                "Path P6 should have OPT = 3");
        require(vc::bruteForceMinimumCoverSize(vc::makeStarGraph(6)) == 1,
                "Star S6 should have OPT = 1");
        require(vc::bruteForceMinimumCoverSize(vc::makeCycleGraph(6)) == 3,
                "Cycle C6 should have OPT = 3");
        require(vc::bruteForceMinimumCoverSize(vc::makeGridGraph(4, 4)) == 8,
                "Grid 4x4 should have OPT = 8");
        require(vc::bruteForceMinimumCoverSize(vc::makeCompleteBipartiteGraph(4, 4)) == 4,
                "K4,4 should have OPT = 4");
        verifySmallGraph(vc::makePathGraph(6), "Path");
        verifySmallGraph(vc::makeStarGraph(6), "Star");
        verifySmallGraph(vc::makeCycleGraph(6), "Cycle");
        verifySmallGraph(vc::makeGridGraph(4, 4), "Grid");
        verifySmallGraph(vc::makeCompleteBipartiteGraph(4, 4), "Complete bipartite");
        for (std::uint32_t seed = 0; seed < 10; ++seed) {
            verifySmallGraph(vc::makeRandomGraph(10, 0.2, seed), "Random graph");
        }

        const auto result = vc::solveApproximation(vc::makeCompleteBipartiteGraph(4, 4));
        require(result.cover.size() == 8, "K4,4 should attain the worst-case ratio 2");

        std::cout << "PASS: approximation validity, guarantee, and known-optimum checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
