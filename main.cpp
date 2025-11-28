#include <iostream>
#include <stdlib.h>
#include <vector>
#include <format>
#include <time.h>

#include "CLI11.hpp"
#include "rng_table.h"
#include "seed_parser.h"
#include "rng_hunter.h"

void print_rng_values(time_t seed, int num_output) {
    srand(seed);
    for (int i = 0; i < num_output; i++) {
        int r = rand();
        int rng_index = (r % 0xFF) + 1;
        std::cout << std::format("{:4}: 0x{:04X}, 0x{:02X}\n", i, r, rng_index);
    }
}

void print_crit_values(int threshold) {
    for (int i = 0; i < 256; i++) {
        std::cout << std::format("0x{:02X} (0x{:02X}): {:5}", i, rng_table(i), crit_table(i, threshold)) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    CLI::App app("CT RNG Hunter");
    CLI::App* list_rng = app.add_subcommand("list_rng", "List the RNG values for a given seed");

    time_t seed = 1600000000;
    list_rng->add_option("-s,--seed", seed, "Unix time for the starting seed")->capture_default_str();
    int num_output=20;
    list_rng->add_option("-n,--num", num_output, "The number of RNG values to print for the seed")->capture_default_str();

    list_rng->callback([&]() {
        print_rng_values(seed, num_output);
    });

    CLI::App* list_crits = app.add_subcommand("list_crits", "Lists the Crit values for a given threshold");
    int threshold = 10;
    list_crits->add_option("-t,--threshold", threshold, "The threshold for a crit (or % chance of crit, for example by default Crono's value would be 10 for a 10% crit")->capture_default_str();

    list_crits->callback([&]() {
        print_crit_values(threshold);
    });

    CLI::App* convert_to_seed = app.add_subcommand("convert_to_seed", "Converts a CTManip Time to a unix timestamp seed");
    std::string timestamp;
    convert_to_seed->add_option("-t,--timestamp", timestamp, "The timestamp to convert")->required();
    convert_to_seed->callback([&]() {
        std::cout << string_to_seed(timestamp) << std::endl;
    });

    CLI::App* find_seeds = app.add_subcommand("find_seeds", "Finds seeds that match the requirements from the input file.");
    std::string filename;
    time_t start = 1700000000;
    time_t end = 1800000000;
    int max_seeds = 10;
    int pool = 8;
    find_seeds->add_option("-f,--filename", filename, "The file")->required();
    find_seeds->add_option("-s,--start", start, "Unix time to start looking for seeds")->capture_default_str();
    find_seeds->add_option("-e,--end", end, "Unix time to end looking for seeds")->capture_default_str();
    find_seeds->add_option("-m,--max_seeds", max_seeds, "Maximum number of seeds to find.")->capture_default_str();
    find_seeds->add_option("-p,--pool",pool,"Pool size for RNG hunters")->capture_default_str();
    find_seeds->callback([&]() {
        RNGHunter hunter(max_seeds, 8);
        if (!hunter.parseFile(filename)) {
            std::cerr << "Unable to load file" << std::endl;
        }
        else {
            std::vector<time_t> valid_seeds = hunter.findSeeds(start, end);
            for (time_t t : valid_seeds) {
                hunter.logSeed(t);
            }
        }
    });

    CLI::App* log_seed = app.add_subcommand("log_seed", "Executes and logs the results of the given seed");
    log_seed->add_option("-f,--filename", filename, "The file to execute")->required();
    log_seed->add_option("-s,--seed", seed, "The seed to run");
    log_seed->callback([&]() {
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
    extend_seed->callback([&]() {
        RNGHunter hunter(1);
        if (!hunter.parseFile(filename)) {
            std::cerr << "Unable to load file" << std::endl;
        }
        hunter.extendSeed(seed, max_rolls);
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}