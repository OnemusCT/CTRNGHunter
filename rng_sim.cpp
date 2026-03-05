#include "rng_sim.h"

#include <format>
#include <print>
#include <string_view>

#include "rng_table.h"
#include "msvc_rand_wrapper.h"

// Log labels for each action type, printed during FULL logging.
constexpr std::string_view kLoad = "load";
constexpr std::string_view kRoom = "room";
constexpr std::string_view kPortal = "portal";
constexpr std::string_view kNewGame = "new_game";
constexpr std::string_view kHeal = "heal";
constexpr std::string_view kExtraHeal = "extra_heal";
constexpr std::string_view kBurn = "burn";
constexpr std::string_view kExtraRooms = "extra_rooms";


void RNGSim::init(time_t seed) {
    rng_.srand(seed);
    extra_rooms_enabled_ = true;
    extra_room_count_ = 0;
    extra_heals_enabled_ = false;
    stats_map_.clear();
}

std::unordered_map<std::string, RNGSim::EncounterStats> RNGSim::get_encounter_stats() {
    return stats_map_;
}

void RNGSim::roll_rng(int n, std::string_view type, LogLevel log_level) {
    last_steps = n;
    if (log_level == LogLevel::FULL) {
        std::print("\t{}: (", type);
        for (int i = 0; i < n; i++) {
            if (i != 0 && i % 33 == 0) std::print("\n\t\t");
            std::print("{:04X} ", rng_.rand());
        }
        std::println(")");
    } else {
        rng_.rand(n);
    }
}

bool RNGSim::load(LogLevel log_level) {
    roll_rng(42, kLoad, log_level);
    return true;
}

bool RNGSim::room(int num, LogLevel log_level) {
    if (log_level == LogLevel::FULL) {
        roll_rng(33 * num, std::format("{} x{}", kRoom, num), log_level);
    } else {
        roll_rng(33 * num, kRoom, log_level);
    }
    return true;
}

bool RNGSim::extra_rooms(LogLevel log_level) {
    if (!extra_rooms_enabled_) return false;
    if (log_level == LogLevel::FULL) {
        roll_rng(66, kExtraRooms, log_level);
    } else if (log_level == LogLevel::PARTIAL) {
        roll_rng(66, kExtraRooms, LogLevel::NONE);
        std::println("\textra rooms");
    } else {
        roll_rng(66, kExtraRooms, LogLevel::NONE);
    }
    ++extra_room_count_;
    return true;
}

bool RNGSim::portal(LogLevel log_level) {
    roll_rng(1, kPortal, log_level);
    return true;
}

bool RNGSim::battle(std::string_view name, LogLevel log_level) {
    last_steps = 1;
    int r = rng_.rand();
    int val = (r % 0xFF) + 1;
    if (!name.empty()) {
        stats_map_[std::string(name)].battle_rng = val;
    }
    last_battle_rng_ = val;
    if (log_level == LogLevel::FULL) std::println("\tbattle: {} {:02X} ({:04X})", name, val, r);
    return true;
}

bool RNGSim::battle_with_rng(const std::vector<int>& rng_vals, std::string_view name, LogLevel log_level) {
    last_steps = 1;
    int r = rng_.rand();
    int val = (r % 0xFF) + 1;
    for (int rng_val: rng_vals) {
        if (val == rng_val) {
            last_battle_rng_ = val;
            stats_map_[std::string(name)] = {extra_room_count_, val, extra_heals_count_};
            extra_room_count_ = 0;
            extra_heals_count_ = 0;
            if (log_level >= LogLevel::PARTIAL) std::println("\tbattle rng {}: {:02X} ({:04X})", name, rng_val, r);
            return true;
        }
    }
    last_battle_rng_ = val;
    if (log_level >= LogLevel::PARTIAL) std::println("\tbattle rng {}: {:02X} DOES NOT MATCH! ({:04X})", name, val, r);
    return false;
}

bool RNGSim::battle_with_crits(const std::vector<int>& threshold, int min_crits, int max_turns, std::string_view name,
                                   LogLevel log_level) {
    last_steps = 1;
    int crits = 0;
    int r = rng_.rand();
    int val = (r % 0xFF) + 1;
    int initial_val = val;
    size_t t_index = 0;
    for (int i = 0; i < max_turns; i++) {
        if (crit_table(val, threshold[t_index])) {
            ++crits;
        }
        val = (val + 1) % 0xFF;
        t_index = (t_index + 1) % threshold.size();
    }
    last_battle_rng_ = val;
    if (log_level >= LogLevel::PARTIAL) std::println("\tbattle crits {}: {} in {} turns from {:02X} ({:04X})", name,
                                                     crits, max_turns, initial_val, r);
    if (crits >= min_crits) {
        stats_map_[std::string(name)] = {extra_room_count_, val, extra_heals_count_};
        extra_room_count_ = 0;
        extra_heals_count_ = 0;
        return true;
    }
    return false;
}

bool RNGSim::new_game(LogLevel log_level) {
    roll_rng(35, kNewGame, log_level);
    return true;
}

bool RNGSim::heal(int num, LogLevel log_level) {
    roll_rng(num, kHeal, log_level);
    return true;
}

bool RNGSim::extra_heal(LogLevel log_level) {
    if (extra_heals_enabled_) {
        ++extra_heals_count_;
        if (log_level == LogLevel::FULL) {
            roll_rng(1, kExtraHeal, log_level);
        } else if (log_level == LogLevel::PARTIAL) {
            roll_rng(1, kExtraHeal, LogLevel::NONE);
            std::println("\textra heal");
        } else {
            roll_rng(1, kExtraHeal, LogLevel::NONE);
        }
        return true;
    }

    return false;
}

void RNGSim::roll_back_rng(int steps) {
    for (int i = 0; i < steps; i++) {
        rng_.unrand();
    }
}

void RNGSim::roll_back_last_rng() {
    roll_back_rng(last_steps);
    last_steps = 0;
}

void RNGSim::burn(int num, LogLevel log_level) {
    roll_rng(num, kBurn, log_level);
}

bool RNGSim::disable_extra_rooms(LogLevel) {
    extra_rooms_enabled_ = false;
    return true;
}

bool RNGSim::enable_extra_rooms(LogLevel) {
    extra_rooms_enabled_ = true;
    return true;
}

bool RNGSim::enable_extra_heals(LogLevel) {
    extra_heals_enabled_ = true;
    extra_heals_count_ = 0;
    return true;
}

bool RNGSim::disable_extra_heals(LogLevel) {
    extra_heals_enabled_ = false;
    extra_heals_count_ = 0;
    return true;
}

std::unique_ptr<RNGSim> RNGSim::Create() {
    return std::make_unique<RNGSim>();
}
