#include "rng_hunter.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>

#include "rng_util.h"
#include "seed_parser.h"
#include "msvc_rand_wrapper.h"
#include "rng_sim.h"

constexpr time_t CHECK_INTERVAL = 1000;

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
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::load, rng_sim_pool_[i].get(), std::placeholders::_1));
            }
        }
        else if (funcName == "room") {
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::room, rng_sim_pool_[i].get(), std::placeholders::_1));
            }
        }
        else if (funcName == "battle") {
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::battle, rng_sim_pool_[i].get(), std::placeholders::_1));
            }
        }
        else if (funcName == "new_game") {
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::new_game, rng_sim_pool_[i].get(), std::placeholders::_1));
            }
        }
        else if (funcName == "portal") {
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::portal, rng_sim_pool_[i].get(), std::placeholders::_1));
            }
        }
        else if (funcName == "heal") {
            int heal_num = 1;
            iss >> heal_num;
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::heal, rng_sim_pool_[i].get(), heal_num, std::placeholders::_1));
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
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::battle_with_rng, rng_sim_pool_[i].get(), rng_vals, std::placeholders::_1));
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
            for (int i = 0; i < rng_sim_pool_.size(); i++) {
                functions_[i].push_back(std::bind(&RNGSim::battle_with_crits, rng_sim_pool_[i].get(), thresholds, min_crits, max_turns, std::placeholders::_1));
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
    for(int i = 0; i < functions_[0].size() - 1; i++) {
        std::ignore = functions_[0][i](/*log=*/true);
    }
    auto func = functions_[0][functions_[0].size()-1];
    for(int i = 0; i < max_rolls; i++) {
        if (func(false)) {
            std::cout << std::format("Rolls: {}, ({} rooms, {} heals)", i+1, (i/66)*2, i%66) << std::endl;
        }
    }
}

void RNGHunter::clear() {
    functions_.clear();
}

std::vector<time_t> RNGHunter::findSeeds(time_t start, time_t end) {
    auto start_time = std::chrono::steady_clock::now();
    std::cout << "Finding seeds between " << start << " and " << end << std::endl;
    size_t num_threads = rng_sim_pool_.size();
    std::vector<std::thread> threads;
    std::vector<std::vector<time_t>> thread_results(num_threads);
    std::atomic<size_t> total_seeds_found(0);
    std::atomic<size_t> seeds_processed(0);
    std::mutex print_mutex;
    int last_percentage = 0;

    time_t total = end - start + 1;
    time_t chunk_size = total / num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        time_t thread_start = start + i * chunk_size;
        time_t thread_end = (i == num_threads - 1) ? end : thread_start + chunk_size - 1;

        threads.emplace_back([this, i, thread_start, thread_end, &thread_results, &total_seeds_found,
            &seeds_processed, &print_mutex, &last_percentage, num_threads, total]() {
                
                size_t local_seeds_found = 0;
                size_t local_processed = 0;

                thread_results[i].reserve(max_seeds_);

                for (time_t seed = thread_start; seed <= thread_end; ++seed) {
                    if (local_processed % CHECK_INTERVAL == 0) {
                        total_seeds_found += local_seeds_found;
                        local_seeds_found = 0;
                        if (total_seeds_found > max_seeds_) {
                            break;
                        }
                        size_t processed = seeds_processed.fetch_add(local_processed) + local_processed;
                        local_processed = 0;

                        int current_percentage = (processed * 100) / total;
                        current_percentage = (current_percentage / 10) * 10;

                        if (current_percentage > last_percentage && current_percentage <= 100) {
                            std::lock_guard<std::mutex> lock(print_mutex);
                            if (current_percentage > last_percentage) {
                                last_percentage = current_percentage;
                                std::cout << current_percentage << "% - " << total_seeds_found.load()
                                    << " seeds found" << std::endl;
                            }
                        }
                    }

                    rng_sim_pool_[i]->init(seed);
                    bool all_pass = true;
                    for (const auto& func : functions_[i]) {
                        if (!func(/*log=*/false)) {
                            all_pass = false;
                            break;
                        }
                    }
                    if (all_pass) {
                        thread_results[i].push_back(seed);
                        ++local_seeds_found;
                    }
                    if (local_seeds_found >= max_seeds_) {
                        break;
                    }

                    ++local_processed;
                }

                total_seeds_found.fetch_add(local_seeds_found);
                seeds_processed.fetch_add(local_processed);
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<time_t> seeds;
    for (const auto& thread_result : thread_results) {
        seeds.insert(seeds.end(), thread_result.begin(), thread_result.end());
    }
    std::cout << "Done! " << seeds.size() << " seeds found!" << std::endl;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
    return seeds;
}
