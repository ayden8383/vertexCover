#include "vertex_cover/BranchAndBound.hpp"
#include "vertex_cover/Generators.hpp"
#include "vertex_cover/Verification.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
int main(int argc, char** argv) {
    int n=40, seed=0; double p=0.2, timeout=30;
    std::string mode="full";
    try {
        for(int i=1;i<argc;++i){
            std::string a=argv[i];
            if(a=="--n" && i+1<argc) n=std::stoi(argv[++i]);
            else if(a=="--p" && i+1<argc) p=std::stod(argv[++i]);
            else if(a=="--seed" && i+1<argc) seed=std::stoi(argv[++i]);
            else if(a=="--timeout" && i+1<argc) timeout=std::stod(argv[++i]);
            else if(a=="--disable-reductions") mode="no_reductions";
            else if(a=="--disable-bounds") mode="no_bounds";
            else if(a=="--disable-greedy") mode="no_greedy";
            else if(a=="--mode" && i+1<argc) mode=argv[++i];
            else throw std::runtime_error("Unknown or missing command option: " + a);
        }
        vc::ExactOptions opt;
        opt.timeLimitSeconds = timeout;
        if(mode=="no_reductions") {
            opt.degreeOneRule=false; opt.triangleRule=false;
            opt.highDegreeRule=false; opt.pathsAndCycles=false;
        } else if(mode=="no_bounds") {
            opt.matchingBound=false; opt.cliqueCoverBound=false;
        } else if(mode=="no_greedy") {
            opt.greedyStart=false;
        } else if(mode!="full") throw std::runtime_error("Unknown mode: "+mode);
        auto graph=vc::makeRandomGraph(n,p,static_cast<std::uint32_t>(seed));
        auto t1=std::chrono::steady_clock::now();
        auto res=vc::solveBranchAndBound(graph,opt);
        auto t2=std::chrono::steady_clock::now();
        double elapsed_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
        bool valid=vc::isVertexCover(graph,res.cover);
        std::cout << std::setprecision(12)
          << "{\"n\":"<<n<<",\"m\":"<<graph.edges().size()
          <<",\"p\":"<<p<<",\"seed\":"<<seed
          <<",\"mode\":\""<<mode<<"\",\"cover_size\":"<<res.cover.size()
          <<",\"optimal\":"<<(res.optimal?"true":"false")
          <<",\"valid\":"<<(valid?"true":"false")
          <<",\"nodes_explored\":"<<res.nodes_explored
          <<",\"pruned\":"<<res.pruned
          <<",\"degree_one_applied\":"<<res.degree_one_applied
          <<",\"triangle_applied\":"<<res.triangle_applied
          <<",\"high_degree_applied\":"<<res.high_degree_applied
          <<",\"initial_upper_bound\":"<<res.initial_upper_bound
          <<",\"root_matching_bound\":"<<res.root_matching_bound
          <<",\"root_clique_bound\":"<<res.root_clique_bound
          <<",\"runtime_ms\":"<<elapsed_ms<<"}\n";
        return valid?0:3;
    } catch(const std::exception& ex){ std::cerr<<ex.what()<<"\n"; return 2; }
}
