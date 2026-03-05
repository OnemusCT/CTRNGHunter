#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <string>

#include "msvc_rand_wrapper.h"

/**
 * Simulates Chrono Trigger's RNG consumption during a speedrun route.
 *
 * Each method corresponds to a game action that advances the RNG by a known number
 * of steps (e.g. loading a save consumes 42 RNG calls, entering a room consumes 33).
 * The simulator tracks per-encounter statistics (extra rooms, battle RNG values, extra
 * heals) for generating a walkthrough of the route.
 * Created via the RNGSim::Create() factory method.
 */
class RNGSim {
    public:

        RNGSim() = default;

        enum LogLevel {
            NONE = 0,
            // No logging
            PARTIAL = 1,
            // Log the manipulated parts (extra rooms, heals, fight RNG)
            FULL = 2 // Log everything
        };

        // Seeds the internal RNG and resets all state (extra rooms, heals, encounter maps).
        void init(time_t seed);

        // Simulates loading a save file (advances RNG by 42 steps).
        bool load(LogLevel log_level);

        // Simulates traversing `num` room transitions (advances RNG by 33 * num steps).
        bool room(int num, LogLevel log_level);

        // Adds an extra pair of room transitions for RNG manipulation (advances RNG by 66 steps).
        // Returns false if extra rooms are currently disabled.
        bool extra_rooms(LogLevel log_level);

        // Simulates a battle encounter (advances RNG by 1 step), computing the battle RNG value.
        bool battle(std::string_view name, LogLevel log_level);

        // Simulates a battle and checks if the resulting RNG value matches any value in `rng_vals`.
        // On match, records the encounter data under `name`. Returns true if a match is found.
        bool battle_with_rng(const std::vector<int>& rng_vals, std::string_view name, LogLevel log_level);

        // Simulates a battle and checks if enough critical hits occur.
        // Uses `threshold` (crit chances per turn, cycled), counts crits over `max_turns`,
        // and returns true if crits >= `min_crits`. Records encounter data under `name` on success.
        bool battle_with_crits(const std::vector<int>& threshold, int min_crits, int max_turns, std::string_view name,
                               LogLevel log_level);

        // Simulates starting a new game (advances RNG by 35 steps).
        bool new_game(LogLevel log_level);

        // Simulates a portal/end-of-time/special room transition (advances RNG by 1 step).
        bool portal(LogLevel log_level);

        // Simulates `num` healing actions (advances RNG by `num` steps).
        bool heal(int num, LogLevel log_level);

        // Adds an extra heal action for RNG manipulation (advances RNG by 1 step).
        // Returns false if extra heals are currently disabled.
        bool extra_heal(LogLevel log_level);

        // Disables extra room transitions. extra_rooms() will return false until re-enabled.
        bool disable_extra_rooms(LogLevel log_level);

        // Enables extra room transitions (enabled by default after init).
        bool enable_extra_rooms(LogLevel log_level);

        // Enables extra healing actions (disabled by default after init).
        bool enable_extra_heals(LogLevel log_level);

        // Disables extra healing actions.
        bool disable_extra_heals(LogLevel log_level);

        // Reverses the RNG state by `steps` individual steps.
        void roll_back_rng(int steps);

        // Reverses the RNG state by the number of steps consumed by the last operation.
        void roll_back_last_rng();

        // Burns (discards) `num` RNG values, advancing the state without game effect.
        void burn(int num, LogLevel log_level);

        struct EncounterStats {
            int extra_rooms = 0;
            int battle_rng = 0;
            int extra_heals = 0;
        };

        // Returns a map of encounter name -> encounter statistics.
        std::unordered_map<std::string, EncounterStats> get_encounter_stats();

        // Factory method. Returns a new RNGSim instance.
        static std::unique_ptr<RNGSim> Create();
        
    private:
        // Advances the RNG by `n` steps. In FULL log mode, prints each individual value;
        // otherwise uses the multi-step fast path.
        void roll_rng(int n, std::string_view type, LogLevel log_level);

        MSVCRandWrapper rng_;

        int last_steps = 0; // Steps consumed by the most recent operation (for rollback)
        bool extra_rooms_enabled_ = true; // Whether extra_rooms() is allowed
        bool extra_heals_enabled_ = false; // Whether extra_heal() is allowed
        int extra_room_count_ = 0; // Running count of extra rooms since last encounter
        int extra_heals_count_ = 0; // Running count of extra heals since last encounter
        int last_battle_rng_ = 0; // Battle RNG value from the most recent battle
        std::unordered_map<std::string, EncounterStats> stats_map_; // encounter name -> stats
};
