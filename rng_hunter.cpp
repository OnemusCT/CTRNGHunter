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

#include "seed_parser.h"
#include "msvc_rand_wrapper.h"
#include "rng_sim.h"

constexpr time_t CHECK_INTERVAL = 1000;

// A thread safe class to track current statistics for RNG hunting
class HunterStatistics {
  public:
    // total is the total number of seeds being processed. Used for calculating percentages.
    HunterStatistics(time_t total) :
        total_seeds_found_(0),
        seeds_processed_(0), total_(total), last_percentage_(0) {}

    size_t total_seeds_found() {
        return total_seeds_found_;
    }

    size_t seeds_processed() {
        return seeds_processed_;
    }

    size_t add_seeds_found(size_t seeds) {
        // fetch_add returns the value prior to the addition
        return total_seeds_found_.fetch_add(seeds) + seeds;
    }

    size_t add_seeds_processed(size_t seeds) {
        // fetch_add returns the value prior to the addition
        return seeds_processed_.fetch_add(seeds) + seeds;
    }

    void maybe_print_progress() {
        size_t processed = seeds_processed_.load();
        size_t current_percentage = (processed * 100) / total_;
        current_percentage = (current_percentage / 10) * 10;

        if (current_percentage > last_percentage_ && current_percentage <= 100) {
            std::lock_guard<std::mutex> lock(print_mutex_);
            if (current_percentage > last_percentage_) {
                last_percentage_ = current_percentage;
                std::cout << current_percentage << "% - " << total_seeds_found_.load()
                    << " seeds found" << std::endl;
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
    std::cout << "Loading input file: " << filename << std::endl;
    if (!std::filesystem::exists(filename)) {
        std::cerr << "Error: File not found at path: " << filename << std::endl;
        return false;
    }
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
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
                functions_[i].emplace_back([this, i](bool log) {
                    return rng_sim_pool_[i]->load(log);
                });
            }
        }
        else if (funcName == "room") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](bool log) {
                    return rng_sim_pool_[i]->room(log);
                });
            }
        }
        else if (funcName == "battle") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](bool log) {
                    return rng_sim_pool_[i]->battle(log);
                });
            }
        }
        else if (funcName == "new_game") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](bool log) {
                    return rng_sim_pool_[i]->new_game(log);
                });
            }
        }
        else if (funcName == "portal") {
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i](bool log) {
                    return rng_sim_pool_[i]->portal(log);
                });
            }
        }
        else if (funcName == "heal") {
            int heal_num = 1;
            iss >> heal_num;
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, heal_num](bool log) {
                    return rng_sim_pool_[i]->heal(heal_num, log);
                });
            }
        }
        else if (funcName == "battle_with_rng") {
            std::string rng_str;

            if (!(iss >> rng_str)) {
                std::cerr << "Error: battle_with_rng requires 1 parameter" << std::endl;
                return false;
            }
            std::vector<int> rng_vals;
            std::stringstream rng_stream(rng_str);
            std::string rng_token;
            while (std::getline(rng_stream, rng_token, ',')) {
                try {
                    rng_vals.push_back(std::stoi(rng_token, nullptr, 16));
                }
                catch (const std::exception&) {
                    std::cerr << "Error: Invalid rng value: " << rng_token << std::endl;
                    return false;
                }
            }
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, &rng_vals](bool log) {
                    return rng_sim_pool_[i]->battle_with_rng(rng_vals, log);
                });
            }
        }
        else if (funcName == "battle_with_crits") {
            std::string thresholds_str;
            int min_crits, max_turns;
            if (!(iss >> thresholds_str >> min_crits >> max_turns)) {
                std::cerr << "Error: battle_with_crits requires 3 parameters (thresholds min_crits max_turns)" << std::endl;
                return false;
            }

            std::vector<int> thresholds;
            std::stringstream thresh_stream(thresholds_str);
            std::string thresh_token;
            while (std::getline(thresh_stream, thresh_token, ',')) {
                try {
                    thresholds.push_back(std::stoi(thresh_token));
                }
                catch (const std::exception&) {
                    std::cerr << "Error: Invalid threshold value: " << thresh_token << std::endl;
                    return false;
                }
            }
            for (size_t i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].emplace_back([this, i, &thresholds, min_crits, max_turns](bool log) {
                    return rng_sim_pool_[i]->battle_with_crits(thresholds, min_crits, max_turns, log);
                });
            }
        }
        else {
            std::cerr << "Unknown function: " << funcName << std::endl;
            return false;
        }
    }
    file.close();
    std::cout << "Done loading" << std::endl;
    return true;
}

void RNGHunter::logSeed(time_t seed) {
    rng_sim_pool_[0]->init(seed);
    std::cout << "Seed: " << seed_to_string(seed) << " (" << seed << ")" << std::endl;
    for (const auto& func : functions_[0]) {
        std::ignore = func(/*log=*/true);
    }
}

void RNGHunter::extendSeed(time_t seed, int max_rolls) {
    rng_sim_pool_[0]->init(seed);
    for(size_t i = 0; i < functions_[0].size() - 1; i++) {
        std::ignore = functions_[0][i](/*log=*/true);
    }
    auto func = functions_[0][functions_[0].size()-1];
    for(int i = 0; i < max_rolls; i++) {
        if (func(false)) {
            std::cout << std::format("Rolls: {}, ({} rooms, {} heals)", i+1, (i/66)*2, i%66) << std::endl;
        }
    }
}

void RNGHunter::logSeedFromFunctions(time_t seed, const std::vector<std::function<bool(bool)>>& functions) {
    // We don't know which sim was used to generate the functions, so just seed them all
    for (const auto & sim : rng_sim_pool_) {
        sim->init(seed);
    }
    std::cout << "Seed: " << seed_to_string(seed) << " (" << seed << ")" << std::endl;
    for (const auto& func : functions) {
        std::ignore = func(/*log=*/true);
    }
}

void RNGHunter::clear() {
    functions_.clear();
}

std::unordered_map<time_t, std::vector<std::function<bool(bool)>>> RNGHunter::findSeeds(time_t start, time_t end, int allowable_heals, int allowable_room_pairs) {
    auto start_time = std::chrono::steady_clock::now();
    std::cout << "Finding seeds between " << start << " and " << end << std::endl;
    size_t num_threads = rng_sim_pool_.size();
    std::vector<std::thread> threads;
    std::vector<std::unordered_map<time_t, std::vector<std::function<bool(bool)>>>> thread_results(num_threads);
    std::mutex print_mutex;

    time_t total = end - start + 1;
    HunterStatistics statistics(total);
    time_t chunk_size = total / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        time_t thread_start = start + i * chunk_size;
        time_t thread_end = (i == num_threads - 1) ? end : thread_start + chunk_size - 1;

        threads.emplace_back([this, i, thread_start, thread_end, allowable_heals, allowable_room_pairs, &thread_results, &statistics] {

                size_t local_seeds_found = 0;
                size_t local_processed = 0;


                for (time_t seed = thread_start; seed <= thread_end; ++seed) {
                    bool debug = debug_seeds_.find(seed) != debug_seeds_.end();
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

                    rng_sim_pool_[i]->init(seed);
                    int curr_allowable_heals = allowable_heals;
                    int curr_allowable_room_pairs = allowable_room_pairs;
                    std::vector<std::function<bool(bool)>> curr_results;
                    curr_results.reserve(functions_[i].size() + 50);
                    bool all_pass = true;
                    for (const auto& func : functions_[i]) {
                        if (!func(/*log=*/debug)) {
                            if (curr_allowable_heals == 0 && curr_allowable_room_pairs == 0) {
                                all_pass = false;
                                break;
                            }
                            if (debug) std::cout << "Trying to extend" << std::endl;
                            std::stack<std::function<bool(bool)>> extra_funcs;
                            rng_sim_pool_[i]->roll_back_last_rng();
                            // Heals are more expensive than room transitions due to the time required to open the tech menu
                            // so prioritize finding options that use room pairs instead.
                            bool passed = false;
                            for (int heals = 0; heals <= curr_allowable_heals; heals++) {
                                for (int rooms = 1; rooms <= curr_allowable_room_pairs; rooms++) {
                                    if (debug) std::cout << "Adding " << ((rooms+1)*2) << " rooms" << std::endl;
                                    std::function room_func = [this, i](bool log) {
                                        return rng_sim_pool_[i]->room(log);
                                    };
                                    std::ignore = room_func(debug);
                                    std::ignore = room_func(debug);
                                    extra_funcs.push(room_func);
                                    extra_funcs.push(room_func);
                                    if (func(/*log=*/debug)) {
                                        if (debug) std::cout << "Found extension!" << std::endl;
                                        passed = true;
                                        curr_allowable_room_pairs -= rooms;
                                        curr_allowable_heals -= heals;
                                        break;
                                    }
                                    if (debug) std::cout << "Rolling back to try again" << std::endl;
                                    rng_sim_pool_[i]->roll_back_last_rng();
                                }
                                if (passed) {
                                    while (!extra_funcs.empty()) {
                                        curr_results.push_back(std::move(extra_funcs.top()));
                                        extra_funcs.pop();
                                    }
                                    break;
                                }
                                // TODO: Implement heals
                            }
                            if (!passed) {
                                all_pass = false;
                                break;
                            }
                        }
                        curr_results.push_back(func);
                    }
                    if (all_pass) {
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

    std::unordered_map<time_t, std::vector<std::function<bool(bool)>>> seeds;
    for (auto& results : thread_results) {
        seeds.merge(results);
    }
    std::cout << "Done! " << seeds.size() << " seeds found!" << std::endl;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
    return seeds;
}
