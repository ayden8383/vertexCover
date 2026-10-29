// BranchAndBound.cpp - exact Minimum Vertex Cover by branch-and-bound.
// Primary author: Hoang Hung Nguyen
//
// Branching. Pick a vertex v of highest degree. Every vertex cover either
//   (A) contains v, or
//   (B) does not contain v - and then it must contain ALL of v's neighbours,
//       otherwise some edge (v, u) would be left uncovered.
// Searching both cases covers every possibility, so the answer is exact.
// Choosing v of high degree makes branch (B) take many vertices at once, so
// that side of the tree runs out of budget and gets pruned quickly.
//
// Keeping the search tree small:
//   1. Reduction rules take vertices that some minimum cover is known to
//      contain, without branching (see reduce()).
//   2. Lower bounds: if (vertices taken so far) + (a lower bound on what the
//      remaining graph still needs) >= (size of the best cover found so far),
//      this branch cannot produce a smaller cover, so it is cut off.
//   3. A greedy cover computed up front is the first "best so far", so the
//      bounds can cut from the very first node.
//   4. Once every remaining vertex has degree <= 2 the graph is just paths and
//      cycles, which are solved directly instead of by branching.
//
// Running time. Branching only happens on vertices of degree >= 3, so the
// search tree obeys T(k) <= T(k-1) + T(k-3) for a cover of size k, i.e. at
// most O(1.4656^k) nodes, each costing O(n + m). Memory is O(n + m).
//
// Graph state. vc::Graph only stores an edge list, so adjacency lists are
// built once up front; after that the graph is never copied. Taking a vertex
// into the cover deletes it in place and pushes it onto `cover`, so `cover`
// doubles as an undo stack: when a branch returns, vertices are popped and
// restored in reverse order, which puts the graph back exactly as it was.
// Taking or restoring v costs O(deg(v)).
#include "vertex_cover/BranchAndBound.hpp"

#include <algorithm>
#include <chrono>
#include <set>
#include <utility>

namespace vc {

namespace {

using Clock = std::chrono::steady_clock;

struct AdjacencyGraph {
    int n = 0;
    int edgeCount = 0;
    std::vector<std::vector<int>> adj;  // adj[v] = neighbours of v
};

// Neighbour lists in edge-list order. Duplicate edges are dropped: they would
// inflate the degrees that the reduction rules and branch pruning rely on.
AdjacencyGraph buildAdjacency(const Graph& graph) {
    AdjacencyGraph g;
    g.n = graph.vertexCount();
    g.adj.assign(g.n, {});
    std::set<std::pair<int, int>> seen;
    for (const Edge& edge : graph.edges()) {
        if (!seen.insert({std::min(edge.u, edge.v), std::max(edge.u, edge.v)}).second) continue;
        g.adj[edge.u].push_back(edge.v);
        g.adj[edge.v].push_back(edge.u);
        ++g.edgeCount;
    }
    return g;
}

class BranchAndBound {
public:
    BranchAndBound(const Graph& graph, const ExactOptions& opt)
        : g(buildAdjacency(graph)), opt(opt), alive(g.n, 1), degree(g.n), edgesLeft(g.edgeCount),
          matched(g.n), cliqueOf(g.n), cliqueSize(g.n), hits(g.n, 0) {
        for (int v = 0; v < g.n; ++v) degree[v] = (int)g.adj[v].size();
    }

    ExactResult run() {
        start = Clock::now();
        stats.root_matching_bound = matchingBound();
        stats.root_clique_bound = cliqueCoverBound();

        if (opt.greedyStart) {
            greedyCover();
        } else {
            for (int v = 0; v < g.n; ++v)
                if (degree[v] > 0) bestCover.push_back(v);
        }
        stats.initial_upper_bound = (int)bestCover.size();

        search();

        ExactResult result = stats;
        result.cover = bestCover;
        std::sort(result.cover.begin(), result.cover.end());
        result.optimal = !timedOut;
        result.seconds = elapsedSeconds();
        return result;
    }

private:
    const AdjacencyGraph g;
    const ExactOptions opt;

    // The current graph is the input graph minus the vertices in `cover`.
    std::vector<char> alive;  // alive[v] == 0 once v has been taken
    std::vector<int> degree;  // for alive v: number of alive neighbours
    int edgesLeft;            // edges with both endpoints alive (= not yet covered)

    std::vector<int> cover;      // vertices taken on the path from the root to this node
    std::vector<int> bestCover;  // smallest complete cover found so far

    std::vector<int> worklist;  // vertices to re-check because their degree dropped to 1 or 2

    // Scratch space for the lower bounds, allocated once.
    std::vector<char> matched;
    std::vector<int> cliqueOf, cliqueSize, hits, touched;

    ExactResult stats;  // counters and root bounds; run() fills in the rest
    Clock::time_point start;
    bool timedOut = false;

    // ---- graph updates --------------------------------------------------

    // Puts v in the cover and deletes it from the graph (its edges are now covered).
    void take(int v) {
        alive[v] = 0;
        for (int u : g.adj[v]) {
            if (!alive[u]) continue;
            --degree[u];
            --edgesLeft;
            if (degree[u] == 1 || degree[u] == 2) worklist.push_back(u);
        }
        cover.push_back(v);
    }

    // Restores the most recently taken vertices until `cover` has `mark` entries.
    void undoTo(std::size_t mark) {
        while (cover.size() > mark) {
            int v = cover.back();
            cover.pop_back();
            alive[v] = 1;
            for (int u : g.adj[v]) {
                if (!alive[u]) continue;
                ++degree[u];
                ++edgesLeft;
            }
        }
    }

    // ---- search ---------------------------------------------------------

    // Searches every cover of the current graph that extends `cover`, and
    // records it in bestCover if it is smaller. Leaves the graph unchanged.
    void search() {
        if (timeUp()) return;
        ++stats.nodes_explored;
        const std::size_t mark = cover.size();

        if (!reduce()) {  // the high-degree rule ran out of budget
            ++stats.pruned;
            undoTo(mark);
            return;
        }
        if (edgesLeft == 0) {  // every edge is covered: `cover` is a complete cover
            if (cover.size() < bestCover.size()) bestCover = cover;
            undoTo(mark);
            return;
        }
        if ((int)cover.size() + lowerBound() >= (int)bestCover.size()) {
            ++stats.pruned;
            undoTo(mark);
            return;
        }

        const int v = maxDegreeVertex();
        if (degree[v] <= 2 && opt.pathsAndCycles) {
            coverPathsAndCycles();
            if (cover.size() < bestCover.size()) bestCover = cover;
            undoTo(mark);
            return;
        }

        const std::size_t reduced = cover.size();

        // Branch A: v is in the cover.
        take(v);
        search();
        undoTo(reduced);

        // Branch B: v is not in the cover, so all of its neighbours are.
        // Skipped when that many vertices could not beat the best cover anyway.
        if (cover.size() + degree[v] < bestCover.size()) {
            for (int u : g.adj[v])
                if (alive[u]) take(u);
            search();
        }
        undoTo(mark);
    }

    // Applies the reduction rules until none applies. Every vertex taken here
    // is in some minimum cover of the current graph (degree-one and triangle
    // rules) or in every cover smaller than bestCover (high-degree rule), so no
    // better solution is lost. Returns false if this node cannot beat bestCover.
    bool reduce() {
        worklist.clear();
        for (int v = 0; v < g.n; ++v)
            if (alive[v] && (degree[v] == 1 || degree[v] == 2)) worklist.push_back(v);

        while (true) {
            while (!worklist.empty()) {
                int u = worklist.back();
                worklist.pop_back();
                if (!alive[u]) continue;
                if (degree[u] == 1 && opt.degreeOneRule) {
                    // The edge (u, w) needs u or w. w covers that edge and
                    // possibly more, so some minimum cover contains w.
                    take(aliveNeighbour(u, 0));
                    ++stats.degree_one_applied;
                } else if (degree[u] == 2 && opt.triangleRule) {
                    // If u's neighbours a and b are adjacent, u-a-b is a triangle
                    // and any cover needs two of the three. A cover that uses u can
                    // swap it for whichever of a, b it lacks without growing, so
                    // some minimum cover contains both a and b.
                    int a = aliveNeighbour(u, 0), b = aliveNeighbour(u, 1);
                    if (adjacent(a, b)) {
                        take(a);
                        take(b);
                        ++stats.triangle_applied;
                    }
                }
            }
            if (!opt.highDegreeRule) return true;

            // A cover smaller than bestCover may take at most `budget` more
            // vertices. Leaving v out would mean taking all deg(v) of its
            // neighbours, so deg(v) > budget forces v into the cover.
            int budget = (int)bestCover.size() - 1 - (int)cover.size();
            if (budget < 0) return false;
            bool tookAny = false;
            for (int v = 0; v < g.n; ++v) {
                if (alive[v] && degree[v] > budget) {
                    take(v);
                    ++stats.high_degree_applied;
                    tookAny = true;
                    if (--budget < 0) return false;
                }
            }
            if (!tookAny) return true;  // otherwise degrees changed: run the rules again
        }
    }

    // Base case: no vertex has degree > 2, so the remaining graph is disjoint
    // paths and cycles. A path on L vertices needs floor(L/2) cover vertices,
    // which repeatedly taking the neighbour of an endpoint achieves; a cycle on
    // L vertices needs ceil(L/2), and since a cycle looks the same from every
    // vertex, taking any one vertex of it is safe and leaves a path.
    // Branching here instead would be exponential in the number of cycles: on
    // many disjoint odd cycles both branches are equally good and the bounds
    // cannot tell them apart.
    void coverPathsAndCycles() {
        worklist.clear();
        for (int u = 0; u < g.n; ++u)
            if (alive[u] && degree[u] == 1) worklist.push_back(u);
        int s = 0;
        while (true) {
            while (!worklist.empty()) {
                int u = worklist.back();
                worklist.pop_back();
                if (alive[u] && degree[u] == 1) take(aliveNeighbour(u, 0));
            }
            while (s < g.n && !(alive[s] && degree[s] == 2)) ++s;  // degrees only fall, so s never needs to move back
            if (s == g.n) return;
            take(s);  // s lies on a cycle: cut it open into a path
        }
    }

    // Starting upper bound: take neighbours of degree-one vertices when there
    // are any, otherwise a vertex of highest degree. Leaves the graph unchanged.
    void greedyCover() {
        worklist.clear();
        for (int u = 0; u < g.n; ++u)
            if (degree[u] == 1) worklist.push_back(u);
        while (true) {
            while (!worklist.empty()) {
                int u = worklist.back();
                worklist.pop_back();
                if (alive[u] && degree[u] == 1) take(aliveNeighbour(u, 0));
            }
            if (edgesLeft == 0) break;
            take(maxDegreeVertex());
        }
        bestCover = cover;
        undoTo(0);
    }

    // ---- lower bounds on the cover size the current graph still needs ---

    int lowerBound() {
        int lb = 0;
        if (opt.matchingBound) lb = std::max(lb, matchingBound());
        if (opt.cliqueCoverBound) lb = std::max(lb, cliqueCoverBound());
        return lb;
    }

    // Greedy maximal matching. Matched edges share no endpoint, so a cover
    // needs a different vertex for each one: OPT >= |M|. (The 2-approximation
    // builds the same kind of matching and takes both endpoints, 2|M|; here
    // we use |M| as a lower bound instead.)
    int matchingBound() {
        std::fill(matched.begin(), matched.end(), 0);
        int size = 0;
        for (int v = 0; v < g.n; ++v) {
            if (!alive[v] || matched[v]) continue;
            for (int u : g.adj[v]) {
                if (alive[u] && !matched[u]) {
                    matched[u] = matched[v] = 1;
                    ++size;
                    break;
                }
            }
        }
        return size;
    }

    // Greedily partition the remaining vertices into cliques. A cover can leave
    // out at most one vertex of a clique (two left-out vertices would leave the
    // edge between them uncovered), so OPT >= sum over cliques of (size - 1).
    // Much stronger than the matching bound on dense graphs.
    int cliqueCoverBound() {
        std::fill(cliqueOf.begin(), cliqueOf.end(), -1);
        int numCliques = 0, bound = 0;
        for (int v = 0; v < g.n; ++v) {
            if (!alive[v] || degree[v] == 0) continue;
            // hits[c] = number of v's neighbours in clique c. v can join c when
            // it is adjacent to every member, i.e. hits[c] == cliqueSize[c].
            touched.clear();
            for (int u : g.adj[v]) {
                if (!alive[u] || cliqueOf[u] < 0) continue;
                if (hits[cliqueOf[u]]++ == 0) touched.push_back(cliqueOf[u]);
            }
            int join = -1;
            for (int c : touched) {
                if (hits[c] == cliqueSize[c] && (join < 0 || cliqueSize[c] > cliqueSize[join])) join = c;
                hits[c] = 0;
            }
            if (join >= 0) {
                cliqueOf[v] = join;
                ++cliqueSize[join];
                ++bound;  // each vertex beyond a clique's first adds 1 to sum (size - 1)
            } else {
                cliqueOf[v] = numCliques;
                cliqueSize[numCliques++] = 1;
            }
        }
        return bound;
    }

    // ---- helpers ----------------------------------------------------------

    int maxDegreeVertex() const {
        int best = -1;
        for (int v = 0; v < g.n; ++v)
            if (alive[v] && (best < 0 || degree[v] > degree[best])) best = v;
        return best;
    }

    // The k-th alive neighbour of u (k = 0 is the first).
    int aliveNeighbour(int u, int k) const {
        for (int w : g.adj[u])
            if (alive[w] && k-- == 0) return w;
        return -1;  // not reached while k < degree[u]
    }

    bool adjacent(int a, int b) const {
        if (g.adj[a].size() > g.adj[b].size()) std::swap(a, b);
        for (int w : g.adj[a])
            if (w == b) return true;
        return false;
    }

    double elapsedSeconds() const { return std::chrono::duration<double>(Clock::now() - start).count(); }

    bool timeUp() {
        if (!timedOut && opt.timeLimitSeconds > 0 && elapsedSeconds() > opt.timeLimitSeconds) timedOut = true;
        return timedOut;
    }
};

} // namespace

ExactResult solveBranchAndBound(const Graph& graph, const ExactOptions& options) {
    BranchAndBound solver(graph, options);
    return solver.run();
}

} // namespace vc
