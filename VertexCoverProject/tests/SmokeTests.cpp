#include "vertex_cover/Approximation.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

    int passed = 0;

    // Check whether a test condition is satisfied.
    void require(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    // Print the table heading.
    void printHeader() {
        std::cout << "\n";
        std::cout << "MINIMUM VERTEX COVER - 2-APPROXIMATION TESTS\n";
        std::cout << "============================================\n\n";

        std::cout << std::left << std::setw(23) << "Graph"
            << std::right << std::setw(5) << "n"
            << std::setw(6) << "m"
            << std::setw(9) << "Cover"
            << std::setw(7) << "OPT"
            << std::setw(9) << "Ratio"
            << std::setw(10) << "Result"
            << '\n';

        std::cout << std::string(69, '-') << '\n';
    }

    // Calculate the approximation ratio.
    // OPT = 0 is handled separately to avoid division by zero.
    std::string formatRatio(int coverSize, int optimum) {
        if (optimum == 0) {
            return "N/A";
        }

        std::ostringstream output;

        output << std::fixed << std::setprecision(2)
            << static_cast<double>(coverSize) / optimum;

        return output.str();
    }

    // Run correctness checks on a graph and print the result.
    void verifySmallGraph(
        const vc::Graph& graph,
        const std::string& label,
        int expectedOptimum = -1
    ) {
        // Run the 2-Approximation algorithm.
        const vc::ApproximationResult result =
            vc::solveApproximation(graph);

        const int coverSize =
            static_cast<int>(result.cover.size());

        // Independently calculate OPT using brute force.
        const int optimum =
            vc::bruteForceMinimumCoverSize(graph);

        // Test 1: Every edge must be covered.
        require(
            vc::isVertexCover(graph, result.cover),
            label + ": Invalid vertex cover"
        );

        // Test 2: Each matching edge contributes two vertices.
        require(
            result.cover.size() == 2 * result.matching.size(),
            label + ": Incorrect matching endpoints"
        );

        // Test 3: The cover cannot be smaller than OPT.
        require(
            coverSize >= optimum,
            label + ": Cover size is smaller than OPT"
        );

        // Test 4: Verify the 2-approximation guarantee.
        require(
            coverSize <= 2 * optimum,
            label + ": Approximation guarantee violated"
        );

        // Test 5: Check known optimal values when provided.
        if (expectedOptimum >= 0) {
            require(
                optimum == expectedOptimum,
                label + ": Incorrect known optimum"
            );
        }

        // The graph passed all correctness checks.
        ++passed;

        // Print this graph's results.
        std::cout << std::left << std::setw(23) << label
            << std::right << std::setw(5) << graph.vertexCount()
            << std::setw(6) << graph.edges().size()
            << std::setw(9) << coverSize
            << std::setw(7) << optimum
            << std::setw(9) << formatRatio(coverSize, optimum)
            << std::setw(10) << "PASS"
            << '\n';
    }

} // namespace

int main() {
    printHeader();

    try {
        // TEST 1: Empty graph
        verifySmallGraph(
            vc::Graph(6),
            "Empty graph",
            0
        );

        // TEST 2: Single edge
        vc::Graph singleEdge(2);
        singleEdge.addEdge(0, 1);

        verifySmallGraph(
            singleEdge,
            "Single edge",
            1
        );

        // TEST 3: Path graph
        verifySmallGraph(
            vc::makePathGraph(6),
            "Path P6",
            3
        );

        // TEST 4: Star graph
        verifySmallGraph(
            vc::makeStarGraph(6),
            "Star S6",
            1
        );

        // TEST 5: Cycle graph
        verifySmallGraph(
            vc::makeCycleGraph(6),
            "Cycle C6",
            3
        );

        // TEST 6: Grid graph
        verifySmallGraph(
            vc::makeGridGraph(4, 4),
            "Grid 4x4",
            8
        );

        // TEST 7: Complete bipartite graph
        verifySmallGraph(
            vc::makeCompleteBipartiteGraph(4, 4),
            "Bipartite K4,4",
            4
        );

        // TESTS 8-17: Random graphs using ten seeds
        for (std::uint32_t seed = 0; seed < 10; ++seed) {
            verifySmallGraph(
                vc::makeRandomGraph(10, 0.2, seed),
                "Random (seed " + std::to_string(seed) + ")"
            );
        }

        // Additional check:
        // K4,4 should reach the worst-case approximation ratio of 2.
        const auto worstCase = vc::solveApproximation(
            vc::makeCompleteBipartiteGraph(4, 4)
        );

        require(
            worstCase.cover.size() == 8,
            "K4,4: Expected a cover size of 8"
        );

        // Final summary
        std::cout << std::string(69, '-') << '\n';

        std::cout << "\nSUMMARY\n";
        std::cout << "-------\n";
        std::cout << "Graphs passed: " << passed
            << "/17\n";

        std::cout << "Vertex cover validity: PASS\n";
        std::cout << "2-Approximation guarantee: PASS\n";
        std::cout << "Known optimum checks: PASS\n";
        std::cout << "K4,4 worst-case check: PASS\n";

        std::cout << "\nALL TESTS PASSED!\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: "
            << e.what() << '\n';

        std::cerr << "Graphs passed before failure: "
            << passed << '\n';

        return 1;
    }
}