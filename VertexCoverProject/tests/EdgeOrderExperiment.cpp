#include "vertex_cover/Approximation.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// Run the approximation algorithm with a specified edge order.
void runOrder(
    const vc::Graph& original,
    const std::vector<vc::Edge>& orderedEdges,
    const std::string& label,
    int optimum,
    std::set<int>& observedSizes
) {
    // Rebuild the same graph using a different edge order.
    vc::Graph reordered(original.vertexCount());

    for (const auto& edge : orderedEdges) {
        reordered.addEdge(edge.u, edge.v);
    }

    // Run the 2-Approximation algorithm.
    const auto result = vc::solveApproximation(reordered);

    const int coverSize = static_cast<int>(result.cover.size());

    // Verify correctness.
    const bool valid =
        vc::isVertexCover(reordered, result.cover);

    const bool guaranteed =
        coverSize >= optimum && coverSize <= 2 * optimum;

    if (!valid || !guaranteed) {
        throw std::runtime_error(
            "Invalid approximation result: " + label
        );
    }

    // Record the observed cover size.
    observedSizes.insert(coverSize);

    // Display the result.
    std::cout << std::left << std::setw(18) << label
        << std::right << std::setw(8) << coverSize
        << std::setw(8) << optimum
        << std::setw(10) << std::fixed
        << std::setprecision(2)
        << static_cast<double>(coverSize) / optimum
        << std::setw(9) << "PASS"
        << '\n';
}

int main() {
    try {
        std::cout << "EDGE-ORDER SENSITIVITY EXPERIMENT\n";
        std::cout << "Order changes; vertex and edge sets stay the same.\n\n";

        // Print column headers.
        std::cout << std::left << std::setw(18) << "Edge order"
            << std::right << std::setw(8) << "Cover"
            << std::setw(8) << "OPT"
            << std::setw(10) << "Ratio"
            << std::setw(9) << "Valid"
            << '\n';

        std::cout << std::string(53, '-') << '\n';

        // ---------------------------------------------
        // EXPERIMENT 1: Path graph with four vertices.
        // ---------------------------------------------

        const auto small = vc::makePathGraph(4);

        const int smallOpt =
            vc::bruteForceMinimumCoverSize(small);

        std::set<int> smallSizes;

        std::cout << "\nPath P4:\n";

        // Process the middle edge first.
        runOrder(
            small,
            { {1, 2}, {0, 1}, {2, 3} },
            "Middle first",
            smallOpt,
            smallSizes
        );

        // Process the outer edges first.
        runOrder(
            small,
            { {0, 1}, {2, 3}, {1, 2} },
            "Outer first",
            smallOpt,
            smallSizes
        );

        // ---------------------------------------------
        // EXPERIMENT 2: Path graph with ten vertices.
        // ---------------------------------------------

        const auto graph = vc::makePathGraph(10);

        const int optimum =
            vc::bruteForceMinimumCoverSize(graph);

        const auto edges = graph.edges();

        std::set<int> observedSizes;

        std::cout << "\nPath P10:\n";

        // Original edge ordering.
        runOrder(
            graph,
            edges,
            "Original",
            optimum,
            observedSizes
        );

        // Repeat with ten seeded edge permutations.
        for (unsigned int seed = 0; seed < 10; ++seed) {

            // Copy the original edge list.
            auto shuffled = edges;

            // Create a reproducible random generator.
            std::mt19937 rng(seed);

            // Shuffle the edge order.
            std::shuffle(
                shuffled.begin(),
                shuffled.end(),
                rng
            );

            // Run the approximation algorithm.
            runOrder(
                graph,
                shuffled,
                "Seed " + std::to_string(seed),
                optimum,
                observedSizes
            );
        }

        // ---------------------------------------------
        // EXPERIMENT SUMMARY
        // ---------------------------------------------

        std::cout << "\nObserved P10 cover sizes: ";

        for (int size : observedSizes) {
            std::cout << size << ' ';
        }

        std::cout << "\nAll edge-order checks passed.\n";

        return 0;
    }
    catch (const std::exception& e) {

        std::cerr << "Experiment FAILED: "
            << e.what() << '\n';

        return 1;
    }
}