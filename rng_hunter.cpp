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

#include "seed_parser.h"
#include "rng_sim.h"
#include "walkthrough_gen/walkthrough_gen.h"

// How often (in seeds processed) each thread checks global progress and early-exit conditions.
constexpr time_t CHECK_INTERVAL = 1000;

// Thread-safe progress tracker for multi-threaded seed searching.
// Aggregates per-thread counters and prints progress at 10% intervals.
class HunterStatistics {
  public:
    // total - Total number of seeds to process (used for percentage calculation).
    explicit HunterStatistics(time_t total) :
        total_seeds_found_(0),
        seeds_processed_(0), total_(total), last_percentage_(0) {}

    size_t total_seeds_found() {
        return total_seeds_found_;
    }

    size_t seeds_processed() {
        return seeds_processed_;
    }

    // Atomically adds to the found count and returns the new total.
    size_t add_seeds_found(size_t seeds) {
        return total_seeds_found_.fetch_add(seeds) + seeds;
    }

    // Atomically adds to the processed count and returns the new total.
    size_t add_seeds_processed(size_t seeds) {
        return seeds_processed_.fetch_add(seeds) + seeds;
    }

    // Prints progress if a new 10% milestone has been reached.
    void maybe_print_progress() {
        size_t processed = seeds_processed_.load();
        size_t current_percentage = (processed * 100) / total_;
        current_percentage = (current_percentage / 10) * 10;

        if (current_percentage > last_percentage_ && current_percentage <= 100) {
            std::lock_guard lock(print_mutex_);
            if (current_percentage > last_percentage_) {
                last_percentage_ = current_percentage;
                std::println("{}% - {} seeds found", current_percentage, total_seeds_found_.load());
            }
        }
    }

   private:
    std::atomic<size_t> total_seeds_found_;
    std::atomic<size_t> seeds_processed_;
    std::mutex print_mutex_;

    const time_t total_;
    std::atomic<size_t> last_percentage_;
};

void RNGHunter::addDebugSeed(time_t seed) {
    debug_seeds_.insert(seed);
}

bool RNGHunter::parseFile(const std::string& filename) {
    std::println("Loading input file: {}", filename);
    if (!std::filesystem::exists(filename)) {
        std::println(stderr, "Error: File not found at path: {}", filename);
        return false;
    }
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(stderr, "Error opening file: {}", filename);
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty()) continue;
        if (line.starts_with("#")) continue;

        std::istringstream iss(line);
        std::string funcName;
        iss >> funcName;

        if (funcName == "load") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->load(log_level);
                });
            }
        }
        else if (funcName == "import") {
            std::string importFilename;
            std::string importRaw;
            if (iss >> importRaw) {
                // Construct path relative to the current file's location
                std::filesystem::path currentPath(filename);
                std::filesystem::path importPath = currentPath.parent_path() / importRaw;

                if (!parseFile(importPath.string())) {
                    std::println(stderr, "Error: Failed to import file: {}", importPath.string());
                    return false;
                }
            }
            else {
                std::println(stderr, "Error: 'import' command requires a filename argument.");
                return false;
            }
        }
        else if (funcName == "room") {
            int rooms_num = 1;
            iss >> rooms_num;
            if (rooms_num == 0) rooms_num = 1;
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, rooms_num](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->room(rooms_num, log_level);
                 });
            }
        }
        else if (funcName == "battle") {
            std::string battle_name;
            if (iss >> battle_name && battle_name.starts_with("#")) {
                battle_name = "";
            }
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, battle_name](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->battle(battle_name, log_level);
                });
            }
        }
        else if (funcName == "new_game") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->new_game(log_level);
                });
            }
        }
        else if (funcName == "portal" || funcName=="eot" || funcName=="special_room") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->portal(log_level);
                });
            }
        }
        else if (funcName == "heal") {
            int heal_num = 1;
            iss >> heal_num;
            if (heal_num == 0) heal_num = 1;
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, heal_num](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->heal(heal_num, log_level);
                });
            }
        }
        else if (funcName == "battle_with_rng") {
            std::string rng_str;

            if (!(iss >> rng_str)) {
                std::println(stderr, "Error: battle_with_rng requires at least 1 parameter");
                return false;
            }
            std::string battle_name;
            if (iss >> battle_name && battle_name.starts_with("#")) {
                battle_name = "";
            }
            std::vector<int> rng_vals;
            std::stringstream rng_stream(rng_str);
            std::string rng_token;
            while (std::getline(rng_stream, rng_token, ',')) {
                try {
                    rng_vals.push_back(std::stoi(rng_token, nullptr, 16));
                }
                catch (const std::exception&) {
                    std::println(stderr, "Error: Invalid rng value: {}", rng_token);
                    return false;
                }
            }
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, rng_vals, battle_name](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->battle_with_rng(rng_vals, battle_name, log_level);
                });
            }
        }
        else if (funcName == "battle_with_crits") {
            std::string thresholds_str;
            int min_crits, max_turns;
            if (!(iss >> thresholds_str >> min_crits >> max_turns)) {
                std::println(stderr, "Error: battle_with_crits requires 3 parameters (thresholds min_crits max_turns)");
                return false;
            }
            std::string battle_name;
            if (iss >> battle_name && battle_name.starts_with("#")) {
                battle_name = "";
            }
            std::vector<int> thresholds;
            std::stringstream thresh_stream(thresholds_str);
            std::string thresh_token;
            while (std::getline(thresh_stream, thresh_token, ',')) {
                try {
                    thresholds.push_back(std::stoi(thresh_token));
                }
                catch (const std::exception&) {
                    std::println(stderr, "Error: Invalid threshold value: {}", thresh_token);
                    return false;
                }
            }
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, thresholds, min_crits, max_turns, battle_name](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->battle_with_crits(thresholds, min_crits, max_turns, battle_name, log_level);
                });
            }
        }
        else if (funcName == "disable_extra_rooms") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->disable_extra_rooms(log_level);
                });
            }
        }
        else if (funcName == "enable_extra_rooms") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->enable_extra_rooms(log_level);
                });
            }
        }
        else if (funcName == "disable_extra_heals") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->disable_extra_heals(log_level);
                });
            }
        }
        else if (funcName == "enable_extra_heals") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](RNGSim::LogLevel log_level) {
                    return rng_sim_pool_[i]->enable_extra_heals(log_level);
                });
            }
        }
        else {
            std::println(stderr, "Unknown function: {}", funcName);
            return false;
        }
    }
    file.close();
    return true;
}

void RNGHunter::logSeed(time_t seed, RNGSim::LogLevel log_level) {
    auto result = findSeedHelper(0, seed, 32, INT_MAX-1, RNGSim::LogLevel::NONE);
    logSeedFromFunctions(seed, result, log_level);
}

void RNGHunter::generateWalkthrough(time_t seed, std::ostream& out) {
    std::println("Seed: {} ({})", seed_to_string(seed), seed);
    findSeedHelper(0, seed, 32, INT_MAX - 1, RNGSim::LogLevel::NONE);
    generate_walkthrough(seed, rng_sim_pool_[0]->get_battle_rng_per_encounter(), rng_sim_pool_[0]->get_extra_rooms_per_encounter(), rng_sim_pool_[0]->get_extra_heals_per_encounter(), out);
}

void RNGHunter::extendSeed(time_t seed, int max_rolls) {
    rng_sim_pool_[0]->init(seed);
    for(size_t i = 0; i < functions_[0].size() - 1; i++) {
        std::ignore = functions_[0][i](RNGSim::LogLevel::FULL);
    }
    auto func = functions_[0][functions_[0].size()-1];
    for(int i = 0; i < max_rolls; i++) {
        if (func(RNGSim::LogLevel::NONE)) {
            std::println("Rolls: {}, ({} rooms, {} heals)", i+1, (i/66)*2, i%66);
        }
    }
}

void RNGHunter::logSeedFromFunctions(time_t seed, const std::vector<RNGSimFunc>& functions, RNGSim::LogLevel log_level) {
    // We don't know which sim was used to generate the functions, so just seed them all
    for (const auto & sim : rng_sim_pool_) {
        sim->init(seed);
    }
    std::println("Seed: {} ({})", seed_to_string(seed), seed);
    for (const auto& func : functions) {
        std::ignore = func(log_level);
    }
    std::print("\n\n");
}

void RNGHunter::clear() {
    functions_.clear();
}

std::vector<RNGSimFunc> RNGHunter::findSeedHelper(int sim_index, time_t seed, int allowable_heals, int allowable_room_pairs, RNGSim::LogLevel log_level) {
    rng_sim_pool_[sim_index]->init(seed);
    int curr_allowable_heals = allowable_heals;
    int curr_allowable_room_pairs = allowable_room_pairs;
    std::vector<RNGSimFunc> curr_results;
    curr_results.reserve(functions_[sim_index].size() + 50);
    bool all_pass = true;
    for (const auto& func : functions_[sim_index]) {
        if (!func(log_level)) {
            if (curr_allowable_heals == 0 && curr_allowable_room_pairs == 0) {
                all_pass = false;
                break;
            }
            if (log_level == RNGSim::LogLevel::FULL) {
                std::println("Trying to extend");
            }
            std::stack<RNGSimFunc> extra_funcs;
            rng_sim_pool_[sim_index]->roll_back_last_rng();
            bool passed = false;
            for (int rooms = 0; rooms <= curr_allowable_room_pairs; rooms++) {
                if (rooms > 0) {
                    if (log_level == RNGSim::LogLevel::FULL) {
                        std::println("Adding {} rooms", (rooms * 2));
                    }
                    std::function extra_rooms_func = [this, sim_index](RNGSim::LogLevel log_level) {
                        return rng_sim_pool_[sim_index]->extra_rooms(log_level);
                        };
                    // If we can't add extra rooms right now, break
                    if (!extra_rooms_func(log_level)) break;

                    extra_funcs.push(extra_rooms_func);
                }
                std::stack<RNGSimFunc> extra_heal_funcs;
                bool added_heals = false;
                for (int heals = 0; heals <= allowable_heals; heals++) {
                    if (heals > 0) {
                        if (log_level == RNGSim::LogLevel::FULL) {
                            std::println("Adding {} heals", heals);
                        }
                        std::function extra_heal_func = [this, sim_index](RNGSim::LogLevel log_level) {
                            return rng_sim_pool_[sim_index]->extra_heal(log_level);
                        };
                        if (!extra_heal_func(log_level)) break;
                        added_heals = true;
                        extra_heal_funcs.push(extra_heal_func);
                    }
                    if (func(log_level)) {
                        if (log_level == RNGSim::LogLevel::FULL) {
                            std::println("Found extension!");
                        }
                        passed = true;
                        curr_allowable_room_pairs -= rooms;
                        //curr_allowable_heals -= heals;
                        break;
                    }
                    else {
                        rng_sim_pool_[sim_index]->roll_back_last_rng();
                    }
                }
                if (passed) {
                    while (!extra_heal_funcs.empty()) {
                        curr_results.push_back(std::move(extra_heal_funcs.top()));
                        extra_heal_funcs.pop();
                    }
                    break;
                }
                if (log_level == RNGSim::LogLevel::FULL) {
                    std::println("Rolling back to try again");
                }
                // rng_sim_pool_[sim_index]->roll_back_last_rng();
                if(added_heals) {
                    rng_sim_pool_[sim_index]->roll_back_rng(allowable_heals);
                    // Cycle allowing extra heals to clear out the count
                    rng_sim_pool_[sim_index]->disable_extra_heals(RNGSim::LogLevel::NONE);
                    rng_sim_pool_[sim_index]->enable_extra_heals(RNGSim::LogLevel::NONE);
                }
            }
            if (passed) {
                while (!extra_funcs.empty()) {
                    curr_results.push_back(std::move(extra_funcs.top()));
                    extra_funcs.pop();
                }
            } else {
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

std::unordered_map<time_t, std::vector<RNGSimFunc>> RNGHunter::findSeeds(time_t start, time_t end, int allowable_heals, int allowable_room_pairs, RNGSim::LogLevel log_level) {
    auto start_time = std::chrono::steady_clock::now();
    std::println("Finding seeds between {} and {}", start, end);
    size_t num_threads = rng_sim_pool_.size();
    std::vector<std::thread> threads;
    std::vector<std::unordered_map<time_t, std::vector<RNGSimFunc>>> thread_results(num_threads);

    time_t total = end - start + 1;
    HunterStatistics statistics(total);
    size_t chunk_size = total / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        time_t thread_start = start + i * chunk_size;
        time_t thread_end = (i == num_threads - 1) ? end : thread_start + chunk_size - 1;

        threads.emplace_back([this, i, thread_start, thread_end, allowable_heals, allowable_room_pairs, &thread_results, &statistics, log_level] {

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

                    std::vector<RNGSimFunc> curr_results = findSeedHelper(i, seed, allowable_heals, allowable_room_pairs, debug);
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

    for (auto& thread : threads) {
        thread.join();
    }

    std::unordered_map<time_t, std::vector<RNGSimFunc>> seeds;
    for (auto& results : thread_results) {
        seeds.merge(results);
    }
    std::cout << "Done! " << seeds.size() << " seeds found!" << std::endl;

    std::println("Done! {} seeds found!", seeds.size());

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::println("Execution time: {} ms", duration.count());
    return seeds;
}
