#include "rng_hunter.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>
#include <stack>
#include <climits>
#include <print>
#include <functional>

#include "seed_parser.h"
#include "rng_sim.h"
#include "walkthrough_gen/walkthrough_gen.h"
#include "hunter_statistics.h"

// How often (in seeds processed) each thread checks global progress and early-exit conditions.
constexpr time_t CHECK_INTERVAL = 1000;


void RNGHunter::addDebugSeed(time_t seed) {
    debug_seeds_.insert(seed);
}

bool Action::execute(RNGSim& sim, RNGSim::LogLevel log_level, const RNGHunter& hunter) const {
    switch (type) {
        case ActionType::LOAD: return sim.load(log_level);
        case ActionType::ROOM: return sim.room(num, log_level);
        case ActionType::EXTRA_ROOMS: return sim.extra_rooms(log_level);
        case ActionType::BATTLE: return sim.battle(hunter.getActionName(name_idx), log_level);
        case ActionType::BATTLE_WITH_RNG: return sim.battle_with_rng(hunter.getActionValues(values_idx), hunter.getActionName(name_idx), log_level);
        case ActionType::BATTLE_WITH_CRITS: return sim.battle_with_crits(hunter.getActionValues(values_idx), min_crits, max_turns, hunter.getActionName(name_idx), log_level);
        case ActionType::NEW_GAME: return sim.new_game(log_level);
        case ActionType::PORTAL: return sim.portal(log_level);
        case ActionType::HEAL: return sim.heal(num, log_level);
        case ActionType::EXTRA_HEAL: return sim.extra_heal(log_level);
        case ActionType::BURN: sim.burn(num, log_level); return true;
        case ActionType::DISABLE_EXTRA_ROOMS: return sim.disable_extra_rooms(log_level);
        case ActionType::ENABLE_EXTRA_ROOMS: return sim.enable_extra_rooms(log_level);
        case ActionType::ENABLE_EXTRA_HEALS: return sim.enable_extra_heals(log_level);
        case ActionType::DISABLE_EXTRA_HEALS: return sim.disable_extra_heals(log_level);
        default: return false;
    }
}

bool RNGHunter::parseFile(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::println(stderr, "Error: File not found at path: {}", filename);
        return false;
    }
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(stderr, "Error opening file: {}", filename);
        return false;
    }
    auto parsers = std::unordered_map<std::string, std::function<bool(std::istringstream&)>> {
        {"load", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::LOAD});
            return true;
        }},
        {"import", [this, &filename](std::istringstream& iss) {
            std::string importRaw;
            if (iss >> importRaw) {
                std::filesystem::path currentPath(filename);
                std::filesystem::path importPath = currentPath.parent_path() / importRaw;
                if (!this->parseFile(importPath.string())) {
                    std::println(stderr, "Error: Failed to import file: {}", importPath.string());
                    return false;
                }
                return true;
            }
            std::println(stderr, "Error: 'import' command requires a filename argument.");
            return false;
        }},
        {"room", [this](std::istringstream& iss) {
            int rooms_num = 1;
            iss >> rooms_num;
            if (rooms_num == 0) rooms_num = 1;
            this->functions_.push_back(Action{ActionType::ROOM, rooms_num});
            return true;
        }},
        {"battle", [this](std::istringstream& iss) {
            std::string battle_name;
            int name_idx = -1;
            if (iss >> battle_name && !battle_name.starts_with("#")) {
                name_idx = static_cast<int>(this->action_names_.size());
                this->action_names_.push_back(battle_name);
            }
            this->functions_.push_back(Action{ActionType::BATTLE, 0, 0, 0, name_idx});
            return true;
        }},
        {"new_game", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::NEW_GAME});
            return true;
        }},
        {"portal", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::PORTAL});
            return true;
        }},
        {"eot", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::PORTAL});
            return true;
        }},
        {"special_room", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::PORTAL});
            return true;
        }},
        {"heal", [this](std::istringstream& iss) {
            int heal_num = 1;
            iss >> heal_num;
            if (heal_num == 0) heal_num = 1;
            this->functions_.push_back(Action{ActionType::HEAL, heal_num});
            return true;
        }},
        {"battle_with_rng", [this](std::istringstream& iss) {
            std::string rng_str;
            if (!(iss >> rng_str)) {
                std::println(stderr, "Error: battle_with_rng requires at least 1 parameter");
                return false;
            }
            std::string battle_name;
            int name_idx = -1;
            if (iss >> battle_name && !battle_name.starts_with("#")) {
                name_idx = static_cast<int>(this->action_names_.size());
                this->action_names_.push_back(battle_name);
            }
            std::vector<int> rng_vals;
            std::stringstream rng_stream(rng_str);
            std::string rng_token;
            while (std::getline(rng_stream, rng_token, ',')) {
                try {
                    rng_vals.push_back(std::stoi(rng_token, nullptr, 16));
                } catch (const std::exception&) {
                    std::println(stderr, "Error: Invalid rng value: {}", rng_token);
                    return false;
                }
            }
            int values_idx = static_cast<int>(this->action_values_.size());
            this->action_values_.push_back(std::move(rng_vals));
            this->functions_.push_back(Action{ActionType::BATTLE_WITH_RNG, 0, 0, 0, name_idx, values_idx});
            return true;
        }},
        {"battle_with_crits", [this](std::istringstream& iss) {
            std::string thresholds_str;
            int min_crits, max_turns;
            if (!(iss >> thresholds_str >> min_crits >> max_turns)) {
                std::println(stderr, "Error: battle_with_crits requires 3 parameters (thresholds min_crits max_turns)");
                return false;
            }
            std::string battle_name;
            int name_idx = -1;
            if (iss >> battle_name && !battle_name.starts_with("#")) {
                name_idx = static_cast<int>(this->action_names_.size());
                this->action_names_.push_back(battle_name);
            }
            std::vector<int> thresholds;
            std::stringstream thresh_stream(thresholds_str);
            std::string thresh_token;
            while (std::getline(thresh_stream, thresh_token, ',')) {
                try {
                    thresholds.push_back(std::stoi(thresh_token));
                } catch (const std::exception&) {
                    std::println(stderr, "Error: Invalid threshold value: {}", thresh_token);
                    return false;
                }
            }
            int values_idx = static_cast<int>(this->action_values_.size());
            this->action_values_.push_back(std::move(thresholds));
            this->functions_.push_back(Action{ActionType::BATTLE_WITH_CRITS, 0, min_crits, max_turns, name_idx, values_idx});
            return true;
        }},
        {"disable_extra_rooms", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::DISABLE_EXTRA_ROOMS});
            return true;
        }},
        {"enable_extra_rooms", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::ENABLE_EXTRA_ROOMS});
            return true;
        }},
        {"disable_extra_heals", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::DISABLE_EXTRA_HEALS});
            return true;
        }},
        {"enable_extra_heals", [this](std::istringstream&) {
            this->functions_.push_back(Action{ActionType::ENABLE_EXTRA_HEALS});
            return true;
        }}
    };

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.starts_with("#")) continue;

        std::istringstream iss(line);
        std::string funcName;
        iss >> funcName;

        if (auto it = parsers.find(funcName); it != parsers.end()) {
            if (!it->second(iss)) {
                return false;
            }
        } else {
            std::println(stderr, "Unknown function: {}", funcName);
            return false;
        }
    }
    file.close();
    return true;
}

void RNGHunter::logSeed(time_t seed, RNGSim::LogLevel log_level) {
    auto result = findSeedHelper(0, seed, 32, INT_MAX - 1, RNGSim::LogLevel::NONE);
    logSeedFromFunctions(seed, result, log_level);
}

void RNGHunter::generateWalkthrough(WalkthroughType type, time_t seed, std::ostream& out) {
    std::println("Seed: {} ({})", seed_to_string(seed), seed);
    findSeedHelper(0, seed, 32, INT_MAX - 1, RNGSim::LogLevel::NONE);
    generate_walkthrough(type, seed, rng_sim_pool_[0]->get_encounter_stats(), out);
}

void RNGHunter::extendSeed(time_t seed, int max_rolls) {
    rng_sim_pool_[0]->init(seed);
    for (size_t i = 0; i < functions_.size() - 1; i++) {
        std::ignore = functions_[i].execute(*rng_sim_pool_[0], RNGSim::LogLevel::FULL, *this);
    }
    auto func = functions_[functions_.size() - 1];
    for (int i = 0; i < max_rolls; i++) {
        if (func.execute(*rng_sim_pool_[0], RNGSim::LogLevel::NONE, *this)) {
            std::println("Rolls: {}, ({} rooms, {} heals)", i + 1, (i / 66) * 2, i % 66);
        }
    }
}

void RNGHunter::logSeedFromFunctions(time_t seed, const std::vector<Action>& functions,
                                     RNGSim::LogLevel log_level) {
    // We don't know which sim was used to generate the functions, so just seed them all
    for (const auto& sim: rng_sim_pool_) {
        sim->init(seed);
    }
    std::println("Seed: {} ({})", seed_to_string(seed), seed);
    for (const auto& func: functions) {
        std::ignore = func.execute(*rng_sim_pool_[0], log_level, *this);
    }
    std::print("\n\n");
}

void RNGHunter::clear() {
    functions_.clear();
}

std::vector<Action> RNGHunter::findSeedHelper(int sim_index, time_t seed, int allowable_heals,
                                              int allowable_room_pairs, RNGSim::LogLevel log_level) {
    rng_sim_pool_[sim_index]->init(seed);
    int curr_allowable_heals = allowable_heals;
    int curr_allowable_room_pairs = allowable_room_pairs;
    std::vector<Action> curr_results;
    curr_results.reserve(functions_.size() + 50);
    bool all_pass = true;
    for (const auto& func: functions_) {
        if (!func.execute(*rng_sim_pool_[sim_index], log_level, *this)) {
            if (curr_allowable_heals == 0 && curr_allowable_room_pairs == 0) {
                all_pass = false;
                break;
            }
            if (!tryExtendWithRoomsAndHeals(sim_index, func, curr_allowable_room_pairs,
                                            allowable_heals, log_level, curr_results)) {
                all_pass = false;
                break;
            }
        }
        curr_results.push_back(func);
    }
    if (!all_pass) {
        return {};
    }
    return curr_results;
}

bool RNGHunter::tryExtendWithRoomsAndHeals(int sim_index, const Action& func,
                                           int& curr_allowable_room_pairs,
                                           int allowable_heals, RNGSim::LogLevel log_level,
                                           std::vector<Action>& curr_results) {
    if (log_level == RNGSim::LogLevel::FULL) {
        std::println("Trying to extend");
    }
    std::vector<Action> extra_funcs;
    rng_sim_pool_[sim_index]->roll_back_last_rng();
    bool passed = false;
    for (int rooms = 0; rooms <= curr_allowable_room_pairs; rooms++) {
        if (rooms > 0) {
            if (log_level == RNGSim::LogLevel::FULL) {
                std::println("Adding {} rooms", (rooms * 2));
            }
            Action extra_rooms_action{ActionType::EXTRA_ROOMS};
            // If we can't add extra rooms right now, break
            if (!extra_rooms_action.execute(*rng_sim_pool_[sim_index], log_level, *this)) break;

            extra_funcs.push_back(extra_rooms_action);
        }
        std::vector<Action> extra_heal_funcs;
        bool added_heals = false;
        for (int heals = 0; heals <= allowable_heals; heals++) {
            if (heals > 0) {
                if (log_level == RNGSim::LogLevel::FULL) {
                    std::println("Adding {} heals", heals);
                }
                Action extra_heal_action{ActionType::EXTRA_HEAL};
                if (!extra_heal_action.execute(*rng_sim_pool_[sim_index], log_level, *this)) break;
                added_heals = true;
                extra_heal_funcs.push_back(extra_heal_action);
            }
            if (func.execute(*rng_sim_pool_[sim_index], log_level, *this)) {
                if (log_level == RNGSim::LogLevel::FULL) {
                    std::println("Found extension!");
                }
                passed = true;
                curr_allowable_room_pairs -= rooms;
                break;
            } else {
                rng_sim_pool_[sim_index]->roll_back_last_rng();
            }
        }
        if (passed) {
            for (const auto& a : extra_heal_funcs) {
                curr_results.push_back(a);
            }
            break;
        }
        if (log_level == RNGSim::LogLevel::FULL) {
            std::println("Rolling back to try again");
        }
        if (added_heals) {
            rng_sim_pool_[sim_index]->roll_back_rng(allowable_heals);
            // Cycle allowing extra heals to clear out the count
            rng_sim_pool_[sim_index]->disable_extra_heals(RNGSim::LogLevel::NONE);
            rng_sim_pool_[sim_index]->enable_extra_heals(RNGSim::LogLevel::NONE);
        }
    }
    if (passed) {
        for (const auto& a : extra_funcs) {
            curr_results.push_back(a);
        }
    }
    return passed;
}

std::unordered_map<time_t, std::vector<Action>> RNGHunter::findSeeds(
    time_t start, time_t end, int allowable_heals, int allowable_room_pairs, RNGSim::LogLevel log_level) {
    auto start_time = std::chrono::steady_clock::now();
    std::println("Finding seeds between {} and {}", start, end);
    size_t num_threads = rng_sim_pool_.size();
    std::vector<std::thread> threads;
    std::vector<std::unordered_map<time_t, std::vector<Action>>> thread_results(num_threads);

    time_t total = end - start + 1;
    HunterStatistics statistics(total);
    size_t chunk_size = total / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        time_t thread_start = start + i * chunk_size;
        time_t thread_end = (i == num_threads - 1) ? end : thread_start + chunk_size - 1;

        threads.emplace_back(
            [this, i, thread_start, thread_end, allowable_heals, allowable_room_pairs, &thread_results, &statistics,
                log_level] {
                size_t local_seeds_found = 0;
                size_t local_processed = 0;

                for (time_t seed = thread_start; seed <= thread_end; ++seed) {
                    RNGSim::LogLevel debug = debug_seeds_.contains(seed) ? RNGSim::LogLevel::FULL : log_level;
                    if (local_processed % CHECK_INTERVAL == 0) {
                        size_t total_seeds_found = statistics.add_seeds_found(local_seeds_found);
                        local_seeds_found = 0;
                        if (total_seeds_found > max_seeds_) {
                            break;
                        }
                        std::ignore = statistics.add_seeds_processed(local_processed);
                        local_processed = 0;

                        statistics.maybe_print_progress();
                    }

                    std::vector<Action> curr_results = findSeedHelper(
                        static_cast<int>(i), seed, allowable_heals, allowable_room_pairs, debug);
                    if (!curr_results.empty()) {
                        thread_results[i][seed] = std::move(curr_results);
                        ++local_seeds_found;
                    }
                    if (local_seeds_found >= max_seeds_) {
                        break;
                    }

                    ++local_processed;
                }

                statistics.add_seeds_found(local_seeds_found);
                statistics.add_seeds_processed(local_processed);
            });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    std::unordered_map<time_t, std::vector<Action>> seeds;
    for (auto& results: thread_results) {
        seeds.merge(results);
    }
    std::cout << "Done! " << seeds.size() << " seeds found!" << std::endl;

    std::println("Done! {} seeds found!", seeds.size());

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::println("Execution time: {} ms", duration.count());
    return seeds;
}
