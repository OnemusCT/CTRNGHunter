#pragma once

#include <vector>
#include <functional>
#include <string>
#include <unordered_map>
#include <set>

#include "rng_sim.h"
#include "walkthrough_gen/walkthrough_gen.h"

enum class ActionType {
    LOAD,
    ROOM,
    EXTRA_ROOMS,
    BATTLE,
    BATTLE_WITH_RNG,
    BATTLE_WITH_CRITS,
    NEW_GAME,
    PORTAL,
    HEAL,
    EXTRA_HEAL,
    BURN,
    DISABLE_EXTRA_ROOMS,
    ENABLE_EXTRA_ROOMS,
    ENABLE_EXTRA_HEALS,
    DISABLE_EXTRA_HEALS
};

class RNGHunter;

// Represents a single RNG manipulation step from the parsed route file.
struct Action {
    ActionType type;
    int num = 0;              // Used by room, heal, burn
    int min_crits = 0;        // Used by battle_with_crits
    int max_turns = 0;        // Used by battle_with_crits
    int name_idx = -1;        // Index in RNGHunter::action_names_
    int values_idx = -1;      // Index in RNGHunter::action_values_

    // Executes this action on the given simulator
    bool execute(RNGSim& sim, RNGSim::LogLevel log_level, const RNGHunter& hunter) const;
};

/**
 * Searches a range of Unix timestamp seeds for ones that produce desired RNG manipulation
 * outcomes in a Chrono Trigger speedrun route.
 *
 * The route is defined by an input file containing a sequence of game actions (load, room,
 * battle_with_rng, etc.). RNGHunter parses these into Action sequences, then evaluates
 * each candidate seed by replaying the sequence through an RNGSim. When an action fails
 * (e.g. battle RNG doesn't match), the hunter attempts to fix it by inserting extra room
 * transitions or heals before the failing action.
 * Supports multi-threaded search via a pool of independent RNGSim instances.
 */
class RNGHunter {
    public:
        // max_seeds  Stop searching after finding this many valid seeds.
        // pool_size  Number of RNGSim instances (and threads) for parallel search.
        explicit RNGHunter(int max_seeds, int pool_size = 1) : max_seeds_(max_seeds) {
            for (int i = 0; i < pool_size; i++) {
                rng_sim_pool_.push_back(RNGSim::Create());
            }
        }

        // Marks a seed for verbose debug logging during findSeeds().
        void addDebugSeed(time_t seed);

        std::string_view getActionName(int idx) const {
            if (idx >= 0) return action_names_[idx];
            return "";
        }

        const std::vector<int>& getActionValues(int idx) const {
            return action_values_[idx];
        }

        // Parses an input file defining the route's RNG action sequence.
        // Supports commands: load, room, battle, battle_with_rng, battle_with_crits,
        // new_game, portal, heal, burn, enable/disable_extra_rooms/heals, and import.
        // Returns false on parse error.
        bool parseFile(const std::string& filename);

        // Replays the route for `seed` with generous allowances, then logs the result.
        void logSeed(time_t seed, RNGSim::LogLevel log_level);

        // Replays a specific function sequence for `seed` and logs each step.
        void logSeedFromFunctions(time_t seed, const std::vector<Action>& functions, RNGSim::LogLevel log_level);

        // Finds how many extra RNG rolls are needed for the last action in the route to succeed.
        void extendSeed(time_t seed, int max_rolls);

        // Replays the route for `seed` and writes a markdown walkthrough to `out`.
        void generateWalkthrough(WalkthroughType type, time_t seed, std::ostream& out);

        // Clears the parsed function sequences.
        void clear();

        // Searches all seeds in [`start`, `end`] for valid routes, allowing up to
        // `allowable_heals` extra heals and `allowable_room_pairs` extra room transition
        // pairs per failing action. Returns a map of valid seed -> function sequence.
        std::unordered_map<time_t, std::vector<Action>> findSeeds(time_t start, time_t end, int allowable_heals,
                                                                      int allowable_room_pairs,
                                                                      RNGSim::LogLevel log_level);

    private:
        // Tests a single seed, inserting extra rooms/heals as needed to make all actions pass.
        // Returns the (possibly extended) function sequence, or empty if the seed is invalid.
        std::vector<Action> findSeedHelper(int sim_index, time_t seed, int allowable_heals,
                                           int allowable_room_pairs, RNGSim::LogLevel log_level);

        // Uses extra heuristics (rooms/heals) to attempt to salvage a failing action.
        bool tryExtendWithRoomsAndHeals(int sim_index, const Action& func, int& curr_allowable_room_pairs,
                                        int allowable_heals, RNGSim::LogLevel log_level,
                                        std::vector<Action>& curr_results);

        size_t max_seeds_;
        std::vector<Action> functions_;
        std::vector<std::unique_ptr<RNGSim>> rng_sim_pool_;
        std::set<time_t> debug_seeds_;
        std::vector<std::string> action_names_;
        std::vector<std::vector<int>> action_values_;
};
