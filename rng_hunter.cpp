#include "rng_hunter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "rng_util.h"
#include "seed_parser.h"
#include "msvc_rand_wrapper.h"
#include "rng_sim.h"

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
            functions_.push_back(std::bind(&RNGSim::load, &rng_sim_pool_[0], std::placeholders::_1));
        }
        else if (funcName == "room") {
            functions_.push_back(std::bind(&RNGSim::room, &rng_sim_pool_[0], std::placeholders::_1));
        }
        else if (funcName == "battle") {
            functions_.push_back(std::bind(&RNGSim::battle, &rng_sim_pool_[0], std::placeholders::_1));
        }
        else if (funcName == "new_game") {
            functions_.push_back(std::bind(&RNGSim::new_game, &rng_sim_pool_[0], std::placeholders::_1));
        }
        else if (funcName == "portal") {
            functions_.push_back(std::bind(&RNGSim::portal, &rng_sim_pool_[0], std::placeholders::_1));
        }
        else if (funcName == "heal") {
            int heal_num = 1;
            iss >> heal_num;
            functions_.push_back(std::bind(&RNGSim::heal, &rng_sim_pool_[0], heal_num, std::placeholders::_1));
        }
        else if (funcName == "battle_with_rng") {
            int rng_val;
            if (!(iss >> std::hex >> rng_val >> std::dec)) {
                std::cerr << "Error: battle_with_rng requires 1 parameter" << std::endl;
                return false;
            }
            functions_.push_back(std::bind(&RNGSim::battle_with_rng, &rng_sim_pool_[0], rng_val, std::placeholders::_1));
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
            functions_.push_back(std::bind(&RNGSim::battle_with_crits, &rng_sim_pool_[0], thresholds, min_crits, max_turns, std::placeholders::_1));
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
    rng_sim_pool_[0].init(seed);
    std::cout << "Seed: " << seed_to_string(seed) << " (" << seed << ")" << std::endl;
    for (const auto& func : functions_) {
        std::ignore = func(/*log=*/true);
    }
}

void RNGHunter::clear() {
    functions_.clear();
}

std::vector<time_t> RNGHunter::findSeeds(time_t start, time_t end) {
    std::cout << "Finding seeds between " << start << " and " << end << std::endl;
    time_t total = end - start;
    time_t ten_percent = total / 10;
    int percentage = 0;
    std::vector<time_t> seeds;
    for (time_t seed = start; seed <= end && seeds.size() < max_seeds_; ++seed) {
        if ((end - seed) % ten_percent == 0) {
            std::cout << percentage << "% - " << seeds.size() << " seeds found" << std::endl;
            percentage += 10;
        }
        rng_sim_pool_[0].init(seed);
        bool all_pass = true;
        for (const auto& func : functions_) {
            if (!func(/*log=*/false)) {
                all_pass = false;
                break;
            }
        }
        if (all_pass) {
            seeds.push_back(seed);
        }
    }
    std::cout << "Done! " << seeds.size() << " seeds found!" << std::endl;
    return seeds;
}