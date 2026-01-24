#pragma once

#include <vector>
#include <functional>
#include <string>
#include <unordered_map>
#include <set>

#include "rng_sim.h"

class RNGHunter {
  public:
    explicit RNGHunter(int max_seeds, int pool_size=1) : max_seeds_(max_seeds), functions_(pool_size) {
        for (int i = 0; i < pool_size; i++) {
            rng_sim_pool_.push_back(RNGSim::Create());
        }
    }
    void addDebugSeed(time_t seed);
    bool parseFile(const std::string& filename);
    void logSeed(time_t seed);
    void logSeedFromFunctions(time_t seed, const std::vector<std::function<bool(bool)>>& functions);
    void extendSeed(time_t seed, int max_rolls);
    void generateWalkthrough(time_t seed, std::ostream& out);

    void clear();

    std::unordered_map<time_t, std::vector<std::function<bool(bool)>>> findSeeds(time_t start, time_t end, int allowable_heals = 0, int allowable_room_pairs = 0);

  private:
    std::vector<std::function<bool(bool)>> findSeedHelper(int sim_index, int seed, int allowable_heals, int allowable_room_pairs, bool debug);
    size_t max_seeds_;
    std::vector<std::vector<std::function<bool(bool)>>> functions_;
    std::vector<std::unique_ptr<RNGSim>> rng_sim_pool_;
    std::set<time_t> debug_seeds_;
};
