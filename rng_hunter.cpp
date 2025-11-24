#include "rng_hunter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "rng_util.h"
#include "seed_parser.h"

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
        // Skip empty lines
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string funcName;
        iss >> funcName;

        if (funcName == "load") {
            functions_.push_back(std::bind(load, std::placeholders::_1));
        }
        else if (funcName == "room") {
            functions_.push_back(std::bind(room, std::placeholders::_1));
        }
        else if (funcName == "battle") {
            functions_.push_back(std::bind(battle, std::placeholders::_1));
        }
        else if (funcName == "new_game") {
            functions_.push_back(std::bind(new_game, std::placeholders::_1));
        }
        else if (funcName == "battle_with_rng") {
            int rng_val;
            if (!(iss >> std::hex >> rng_val >> std::dec)) {
                std::cerr << "Error: battle_with_rng requires 1 parameter" << std::endl;
                return false;
            }
            functions_.push_back(std::bind(battle_with_rng, rng_val, std::placeholders::_1));
        }
        else if (funcName == "battle_with_crits") {
            int threshold, min_crits, max_turns;
            if (!(iss >> threshold >> min_crits >> max_turns)) {
                std::cerr << "Error: battle_with_crits requires 3 parameters" << std::endl;
                return false;
            }
            functions_.push_back(std::bind(battle_with_crits, threshold, min_crits, max_turns, std::placeholders::_1));
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
    init(seed);
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
    time_t total = end-start;
    time_t ten_percent = total/10;
    int percentage = 0;
    std::vector<time_t> seeds;
    for (time_t seed = start; seed <= end && seeds.size() < max_seeds_; ++seed) {
        if ((end - seed) % ten_percent == 0) {
            std::cout << percentage << "% - " << seeds.size() << " seeds found" << std::endl;
            percentage += 10;
        }
        init(seed);
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

