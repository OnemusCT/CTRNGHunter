#include <iostream>
#include <vector>
#include <format>

#include "CLI11.hpp"
#include "rng_table.h"
#include "seed_parser.h"
#include "rng_hunter.h"
#include "msvc_rand_wrapper.h"

void print_rng_values(time_t seed, int num_output) {
    MSVCRandWrapper rand = {};
    rand.srand(seed);
    for (int i = 0; i < num_output; i++) {
        int r = rand.rand();
        int rng_index = (r % 0xFF) + 1;
        std::cout << std::format("{:4}: 0x{:04X}, 0x{:02X}\n", i, r, rng_index);
    }
}

void print_crit_values(int threshold) {
    for (int i = 0; i < 256; i++) {
        std::cout << std::format("0x{:02X} (0x{:02X}): {:5}", i, rng_table(i), crit_table(i, threshold)) << std::endl;
    }
}

void print_init_table(int players = 3, int enemies = 1) {
    std::cout << std::format("RNG Post initialization for {} characters and {} enemies", players, enemies) << std::endl;
    std::cout << "\t";
    for (int i = 0; i < 16; i++) {
        std::cout << std::format("x{:1X}\t", i);
    }
    std::cout << std::endl;
    for (int i = 0; i < 256; i++) {
        if (i % 16 == 0) {
            std::cout << std::format("{:1X}x", i/16) << "\t";
        }
        std::set<int> seen;
        int t = 10;
        for (int j = 3; j > players; j--) {
            seen.insert(t--);
        }
        for (int j = i; j != i - 1; j++) {
            if(j == 256) j = 0;
            //std::cout << std::format("j: {:02X} ({:02X})", j, rng_table(j));
            seen.insert(rng_table(j)%11);
            if (seen.size() == 11) {
                j = (j+1)%256;
                while (rng_table(j) % 8 >= enemies) j = (j + 1) % 256;
                j = (j + 1) % 256;
                std::cout << std::format("{:02X}\t", j);
                break;
            }
        }
        if (i % 16 == 15) std::cout << std::endl;
    }
}

void print_init_order(int rng, int players, int enemies) {
    std::set<int> exist;
    std::vector<int> order;
    std::vector<int> entities;
    for (int i = 0; i < players; i++) {
        entities.push_back(i);
        exist.insert(i);
    }
    for (int i = 0; i < 8; i++) {
        entities.push_back(10-i);
    }
    for (int i = 0; i < enemies; i++) {
        exist.insert(3+i);
    }
    while(entities.size() < 11 ) {
        entities.push_back(0xFF);
    }
    while (order.size() < 11 - (3-players)) {
        int c = rng_table(rng)%11;
        if (entities[c] != 0xFF) {
            order.push_back(entities[c]);
            entities[c] = 0xFF;
        }
        ++rng;
    }
    std::cout << "[ ";
    for (int e : order) {
        if (exist.find(e) != exist.end())
            std::cout << std::format("**0x{:1X}** ", e);
        else
            std::cout << std::format("0x{:1X} ", e);
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char* argv[]) {
    CLI::App app("CT RNG Hunter");
    CLI::App* list_rng = app.add_subcommand("list_rng", "List the RNG values for a given seed");

    time_t seed = 1600000000;
    list_rng->add_option("-s,--seed", seed, "Unix time for the starting seed")->capture_default_str();
    int num_output=20;
    list_rng->add_option("-n,--num", num_output, "The number of RNG values to print for the seed")->capture_default_str();

    list_rng->callback([&] {
        print_rng_values(seed, num_output);
    });

    CLI::App* list_crits = app.add_subcommand("list_crits", "Lists the Crit values for a given threshold");
    int threshold = 10;
    list_crits->add_option("-t,--threshold", threshold, "The threshold for a crit (or % chance of crit, for example by default Crono's value would be 10 for a 10% crit")->capture_default_str();

    list_crits->callback([&] {
        print_crit_values(threshold);
    });

    CLI::App* convert_to_seed = app.add_subcommand("convert_to_seed", "Converts a CTManip Time to a unix timestamp seed");
    std::string timestamp;
    convert_to_seed->add_option("-t,--timestamp", timestamp, "The timestamp to convert")->required();
    convert_to_seed->callback([&] {
        std::cout << string_to_seed(timestamp) << std::endl;
    });

    CLI::App* find_seeds = app.add_subcommand("find_seeds", "Finds seeds that match the requirements from the input file.");
    std::string filename;
    time_t start = 315550800;
    time_t end = 2147403600;
    int max_seeds = 10;
    int pool = 8;
    int max_rooms = 0;
    int min_rooms = 0;
    find_seeds->add_option("-f,--filename", filename, "The file")->required();
    find_seeds->add_option("-s,--start", start, "Unix time to start looking for seeds")->capture_default_str();
    find_seeds->add_option("-e,--end", end, "Unix time to end looking for seeds")->capture_default_str();
    find_seeds->add_option("-m,--max_seeds", max_seeds, "Maximum number of seeds to find.")->capture_default_str();
    find_seeds->add_option("-p,--pool",pool,"Pool size for RNG hunters")->capture_default_str();
    find_seeds->add_option("-r,--rooms", max_rooms, "Maximum number of extra room transition pairs")->capture_default_str();
    find_seeds->add_option("--rooms_min", min_rooms, "Minimum number of extra room transition pairs")->capture_default_str();
    find_seeds->callback([&] {
        RNGHunter hunter(max_seeds, pool);
        if (!hunter.parseFile(filename)) {
            std::cerr << "Unable to load file" << std::endl;
            return;
        }
        //hunter.addDebugSeed(1097631540);
        for(int i = min_rooms; i <= max_rooms; i++) {
            std::unordered_map<time_t, std::vector<std::function<bool(bool)>>> valid_seeds = hunter.findSeeds(start, end, 0, i);
            if(!valid_seeds.empty()) {
                for (const auto& [curr_seed, functions] : valid_seeds) {
                    hunter.logSeedFromFunctions(curr_seed, functions);
                }
                break;
            }
        }
    });

    CLI::App* log_seed = app.add_subcommand("log_seed", "Executes and logs the results of the given seed");
    log_seed->add_option("-f,--filename", filename, "The file to execute")->required();
    log_seed->add_option("-s,--seed", seed, "The seed to run");
    log_seed->callback([&] {
        RNGHunter hunter(10);
        if (!hunter.parseFile(filename)) {
            std::cerr << "Unable to load file" << std::endl;
        }
        hunter.logSeed(seed);
    });

    CLI::App* convert_to_timestamp = app.add_subcommand("convert_to_timestamp", "Converts a seed to a CTManip timestamp");
    convert_to_timestamp->add_option("-s,--seed", seed, "The seed to convert")->required();
    convert_to_timestamp->callback([&]() {
        std::cout << seed_to_string(seed) << std::endl;
    });

    int max_rolls = 660;
    CLI::App* extend_seed = app.add_subcommand("extend_seed", "Finds the number of extra RNG rolls necessary for the final command in the seed to succeed");
    extend_seed->add_option("-s,--seed", seed, "The seed to extend")->required();
    extend_seed->add_option("-f,--filename", filename, "The filename to execute")->required();
    extend_seed->add_option("-m,--max_rolls", max_rolls, "The maximum number of rolls to extend the seed");
    extend_seed->callback([&] {
        RNGHunter hunter(1);
        if (!hunter.parseFile(filename)) {
            std::cerr << "Unable to load file" << std::endl;
        }
        hunter.extendSeed(seed, max_rolls);
    });

    int players = 3;
    int enemies = 1;
    CLI::App* print_init = app.add_subcommand("print_init_table", "Prints the post initialization RNG seed for each possible starting RNG seed for a number of players and enemies");
    print_init->add_option("-p,--players", players, "The number of player characters in the party");
    print_init->add_option("-e,--enemies", enemies, "The number of enemies in the fight");
    print_init->callback([&] {
        print_init_table(players, enemies);
    });

    std::string rng = "0";
    CLI::App* print_turn_order = app.add_subcommand("print_turn_order", "Prints the processing order for enemy and player turns in battle for a given RNG seed");
    print_turn_order->add_option("-p,--players", players, "The number of player characters in the party");
    print_turn_order->add_option("-e,--enemies", enemies, "The number of enemies in the fight");
    print_turn_order->add_option("-r,--rng", rng, "The RNG value that the battle starts on");
    print_turn_order->callback([&] {
        int rng_val = 0;
        try {
            rng_val = std::stoi(rng, 0, 16);
        }
        catch (const std::exception&) {
            std::cerr << "Invalid rng value, must be a hex value: " << rng << std::endl;
            return;
        }
        print_init_order(rng_val, players, enemies);
    });

    CLI11_PARSE(app, argc, argv);
    if (argc == 1) {
        std::cout << app.help() << std::endl;
    }
    return 0;
}