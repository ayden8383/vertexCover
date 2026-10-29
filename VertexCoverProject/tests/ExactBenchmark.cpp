#include "vertex_cover/BranchAndBound.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

    using Clock = std::chrono::steady_clock;

    // Helps prevent benchmark results from being discarded.
    volatile std::uint64_t resultSink = 0;

    // Stores a graph and its experimental configuration.
    struct GraphCase {
        std::string family;
        std::string name;
        vc::Graph graph;

        // -1 means not applicable.
        double probability;
        int seed;
    };

    // Stores summary statistics for a graph family.
    struct Summary {
        int count = 0;
        double totalMs = 0.0;
        double minMs = std::numeric_limits<double>::infinity();
        double maxMs = 0.0;
    };

    std::vector<GraphCase> generateCases() {

        std::vector<GraphCase> cases;

        const std::array<int, 6> sizes = {
            10, 20, 40, 60, 80, 100
        };

        // ----------------------------------------
        // 1. RANDOM GRAPHS: G(n,p)
        // ----------------------------------------

        for (int n : sizes) {

            for (double p : {0.1, 0.2, 0.5}) {

                for (int seed = 0; seed < 10; ++seed) {

                    const std::string name =
                        "G" + std::to_string(n) +
                        "_p" + std::to_string(
                            static_cast<int>(p * 10 + 0.5)
                        );

                    cases.push_back({
                        "Random",
                        name,
                        vc::makeRandomGraph(
                            n,
                            p,
                            static_cast<std::uint32_t>(seed)
                        ),
                        p,
                        seed
                        });
                }
            }
        }

        // ----------------------------------------
        // 2. PATH, STAR AND CYCLE GRAPHS
        // ----------------------------------------

        for (int n : sizes) {

            cases.push_back({
                "Path",
                "P" + std::to_string(n),
                vc::makePathGraph(n),
                -1.0,
                -1
                });

            cases.push_back({
                "Star",
                "S" + std::to_string(n),
                vc::makeStarGraph(n),
                -1.0,
                -1
                });

            cases.push_back({
                "Cycle",
                "C" + std::to_string(n),
                vc::makeCycleGraph(n),
                -1.0,
                -1
                });
        }

        // ----------------------------------------
        // 3. SQUARE GRID GRAPHS
        // ----------------------------------------

        for (int side : {4, 6, 8, 10}) {

            cases.push_back({
                "Grid",
                std::to_string(side) + "x" +
                    std::to_string(side),
                vc::makeGridGraph(side, side),
                -1.0,
                -1
                });
        }

        // ----------------------------------------
        // 4. COMPLETE BIPARTITE GRAPHS
        // ----------------------------------------

        for (int side : {5, 10, 20, 30, 40}) {

            cases.push_back({
                "Bipartite",
                "K(" + std::to_string(side) +
                    "," + std::to_string(side) + ")",
                vc::makeCompleteBipartiteGraph(
                    side,
                    side
                ),
                -1.0,
                -1
                });
        }

        return cases;
    }

    // Measure a batch of exact branch-and-bound executions.
//
// Returns the total elapsed time in seconds.

    double timeBatch(
        const vc::Graph& graph,
        std::uint64_t iterations
    ) {
        std::uint64_t checksum = 0;

        const auto start = Clock::now();

        for (std::uint64_t i = 0; i < iterations; ++i) {

            const auto result =
                vc::solveBranchAndBound(graph);

            checksum += result.cover.size();
        }

        const auto finish = Clock::now();

        // Consume the result outside the timed region.
        resultSink += checksum;

        return std::chrono::duration<double>(
            finish - start
        ).count();
    }


    // Stores benchmarking measurements.

    struct TimingResult {
        // Median execution time per algorithm call.
        double medianMilliseconds;

        // Number of algorithm executions per batch.
        std::uint64_t iterationsPerMeasurement;

        // The five individual timing measurements.
        std::array<double, 5> measurementsMs;
    };


    // Run five timed measurements and return the median.

    TimingResult benchmark(const vc::Graph& graph) {

        // 1. Warm-up execution.
        const auto warmup =
            vc::solveBranchAndBound(graph);

        resultSink += warmup.cover.size();

        // 2. Determine how many executions to batch.
        constexpr std::uint64_t pilotIterations = 200;
        constexpr double targetBatchSeconds = 0.010;

        const double pilotSeconds =
            timeBatch(graph, pilotIterations);

        const double secondsPerCall = std::max(
            pilotSeconds /
            static_cast<double>(pilotIterations),
            1e-9
        );

        const auto iterations =
            static_cast<std::uint64_t>(
                std::clamp(
                    std::ceil(
                        targetBatchSeconds / secondsPerCall
                    ),
                    1.0,
                    100000.0
                )
                );

        // 3. Collect five timed measurements.
        std::array<double, 5> samplesMs{};

        for (double& sample : samplesMs) {

            const double seconds =
                timeBatch(graph, iterations);

            // Average execution time per algorithm call.
            sample =
                (seconds * 1000.0) /
                static_cast<double>(iterations);
        }

        // 4. Calculate the median.
        // 4. Preserve the original measurement order.
        const auto measurements = samplesMs;

        // 5. Sort a copy to calculate the median.
        std::sort(
            samplesMs.begin(),
            samplesMs.end()
        );

        const double median = samplesMs[2];

        // 6. Return the median and all five measurements.
        return {
            median,
            iterations,
            measurements
        };
    }

    } // namespace

    int main() {

        try {

            // Generate all graphs before measuring execution time.
            const auto cases = generateCases();

            // Save the CSV in the program's working directory.
            const auto csvPath =
                std::filesystem::current_path() /
                "exact_benchmark.csv";

            std::ofstream csv(csvPath);

            if (!csv) {
                throw std::runtime_error(
                    "Unable to open CSV file for writing"
                );
            }

            // CSV column headings.
            csv
                << "family,instance,n,m,p,seed,"
                << "cover_size,optimal,nodes_explored,pruned,"
                << "degree_one_applied,triangle_applied,high_degree_applied,"
                << "initial_upper_bound,root_matching_bound,root_clique_bound,"
                << "median_ms,iterations_per_measurement,"
                << "measurements,"
                << "run1_ms,run2_ms,run3_ms,run4_ms,run5_ms\n";

            std::map<std::string, Summary> summaries;

            std::cout
                << "EXACT BRANCH-AND-BOUND RUNTIME BENCHMARK\n"
                << "========================================\n"
                << "Graph instances: " << cases.size() << "\n"
                << "Timed measurements per instance: 5\n"
                << "Statistic: median milliseconds "
                "per algorithm call\n\n";

            int completed = 0;

            // ----------------------------------------
            // RUN ALL BENCHMARKS
            // ----------------------------------------

            for (const auto& graphCase : cases) {

                // Verify correctness BEFORE benchmarking.
                const auto solution =
                    vc::solveBranchAndBound(graphCase.graph);

                const bool valid =
                    vc::isVertexCover(
                        graphCase.graph,
                        solution.cover
                    );

                // No time limit is set, so the search must finish.
                if (!valid || !solution.optimal) {
                    throw std::runtime_error(
                        "Invalid or non-optimal cover: " + graphCase.name
                    );
                }

                // Measure execution time.
                const auto timing =
                    benchmark(graphCase.graph);

                // Update family statistics.
                auto& summary =
                    summaries[graphCase.family];

                ++summary.count;

                summary.totalMs +=
                    timing.medianMilliseconds;

                summary.minMs = std::min(
                    summary.minMs,
                    timing.medianMilliseconds
                );

                summary.maxMs = std::max(
                    summary.maxMs,
                    timing.medianMilliseconds
                );

                // ----------------------------------------
                // EXPORT RESULT TO CSV
                // ----------------------------------------

                csv
                    << graphCase.family << ','
                    << '"' << graphCase.name << '"' << ','
                    << graphCase.graph.vertexCount() << ','
                    << graphCase.graph.edges().size() << ',';

                // Probability is only applicable to random graphs.
                if (graphCase.probability >= 0.0) {
                    csv << std::fixed
                        << std::setprecision(1)
                        << graphCase.probability;
                }

                csv << ',';

                // Seed is only applicable to random graphs.
                if (graphCase.seed >= 0) {
                    csv << graphCase.seed;
                }

                csv
                    << ','
                    << solution.cover.size()
                    << ','
                    << solution.optimal
                    << ','
                    << solution.nodes_explored
                    << ','
                    << solution.pruned
                    << ','
                    << solution.degree_one_applied
                    << ','
                    << solution.triangle_applied
                    << ','
                    << solution.high_degree_applied
                    << ','
                    << solution.initial_upper_bound
                    << ','
                    << solution.root_matching_bound
                    << ','
                    << solution.root_clique_bound
                    << ','
                    << std::fixed
                    << std::setprecision(9)
                    << timing.medianMilliseconds
                    << ','
                    << timing.iterationsPerMeasurement
                    << ",5";

                // Export all five execution-time measurements.
                for (double measurement : timing.measurementsMs) {
                    csv << ',' << measurement;
                }

                csv << '\n';

                // Display progress.
                ++completed;

                if (
                    completed % 30 == 0 ||
                    completed == static_cast<int>(cases.size())
                    ) {
                    std::cout
                        << "Completed "
                        << completed << "/"
                        << cases.size()
                        << " instances\n";
                }
            }

            csv.close();

            if (!csv) {
                throw std::runtime_error(
                    "Failed while writing CSV"
                );
            }

            // ----------------------------------------
            // CONSOLE SUMMARY
            // ----------------------------------------

            std::cout
                << "\nSUMMARY BY GRAPH FAMILY\n"
                << std::left << std::setw(15) << "Family"
                << std::right << std::setw(10) << "Graphs"
                << std::setw(16) << "Mean median ms"
                << std::setw(15) << "Min ms"
                << std::setw(15) << "Max ms"
                << '\n'
                << std::string(71, '-')
                << '\n';

            for (const auto& [family, summary] : summaries) {

                std::cout
                    << std::left
                    << std::setw(15) << family

                    << std::right
                    << std::setw(10) << summary.count

                    << std::fixed
                    << std::setprecision(6)

                    << std::setw(16)
                    << summary.totalMs / summary.count

                    << std::setw(15)
                    << summary.minMs

                    << std::setw(15)
                    << summary.maxMs

                    << '\n';
            }

            std::cout
                << "\nAll benchmark instances passed "
                "cover-validity and optimality checks.\n"
                << "CSV saved to: "
                << csvPath.string()
                << '\n';

            return 0;
        }
        catch (const std::exception& e) {

            std::cerr
                << "BENCHMARK FAILED: "
                << e.what()
                << '\n';

            return 1;
        }
    }