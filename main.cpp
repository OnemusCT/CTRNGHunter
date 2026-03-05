#include <format>
#include <iostream>
#include <print>
#include <vector>

#include "CLI11.hpp"
#include "rng_table.h"
#include "seed_parser.h"
#include "rng_hunter.h"
#include "msvc_rand_wrapper.h"
#include "guardian_sim.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    CLI::App app("CT RNG Hunter");
    CLI::App* list_rng = app.add_subcommand("list_rng", "List the RNG values for a given seed");

    time_t seed = 1600000000;
    list_rng->add_option("-s,--seed", seed, "Unix time for the starting seed")->capture_default_str();
    int num_output = 20;
    list_rng->add_option("-n,--num", num_output, "The number of RNG values to print for the seed")->
              capture_default_str();

    list_rng->callback([&] {
        print_rng_values(seed, num_output);
    });

    CLI::App* list_crits = app.add_subcommand("list_crits", "Lists the Crit values for a given threshold");
    int threshold = 10;
    list_crits->add_option("-t,--threshold", threshold,
                           "The threshold for a crit (or % chance of crit, for example by default Crono's value would be 10 for a 10% crit")
              ->capture_default_str();

    list_crits->callback([&] {
        print_crit_values(threshold);
    });

    CLI::App* convert_to_seed = app.add_subcommand("convert_to_seed",
                                                   "Converts a CTManip Time to a unix timestamp seed");
    std::string timestamp;
    convert_to_seed->add_option("-t,--timestamp", timestamp, "The timestamp to convert")->required();
    convert_to_seed->callback([&] {
        std::println("{}", string_to_seed(timestamp));
    });

    CLI::App* find_seeds = app.add_subcommand("find_seeds",
                                              "Finds seeds that match the requirements from the input file.");
    std::string filename;
    time_t start = 315550800;
    time_t end = 2147403600;
    int max_seeds = 10;
    int pool = 8;
    int max_rooms = 0;
    int min_rooms = 0;
    int max_heals = 0;
    bool verbose = false;
    find_seeds->add_option("-f,--filename", filename, "The file")->required();
    find_seeds->add_option("-s,--start", start, "Unix time to start looking for seeds")->capture_default_str();
    find_seeds->add_option("-e,--end", end, "Unix time to end looking for seeds")->capture_default_str();
    find_seeds->add_option("-m,--max_seeds", max_seeds, "Maximum number of seeds to find.")->capture_default_str();
    find_seeds->add_option("-p,--pool", pool, "Pool size for RNG hunters")->capture_default_str();
    find_seeds->add_option("-v,--verbose", verbose, "Verbose logging");
    find_seeds->add_option("--min_rooms", min_rooms, "Minimum number of extra room transition pairs")->
                capture_default_str();
    find_seeds->add_option("--max_rooms", max_rooms, "Maximum number of extra room transition pairs")->
                capture_default_str();
    find_seeds->add_option("--max_heals", max_heals, "Maximum number of extra heals")->capture_default_str();
    find_seeds->callback([&] {
        if (max_rooms < min_rooms) max_rooms = min_rooms;
        RNGHunter hunter(max_seeds, pool);
        if (!hunter.parseFile(filename)) {
            std::println(stderr, "Unable to load file");
            return;
        }
        //hunter.addDebugSeed(1097631540);
        try {
            for (int i = min_rooms; i <= max_rooms; i++) {
                std::unordered_map<time_t, std::vector<RNGSimFunc>> valid_seeds = hunter.findSeeds(
                    start, end, max_heals, i, RNGSim::LogLevel::NONE);
                if (!valid_seeds.empty()) {
                    for (const auto& [curr_seed, functions]: valid_seeds) {
                        hunter.logSeedFromFunctions(curr_seed, functions,
                                                    verbose ? RNGSim::LogLevel::FULL : RNGSim::LogLevel::PARTIAL);
                    }
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::println("{}", e.what());
        }
    });

    CLI::App* log_seed = app.add_subcommand("log_seed", "Executes and logs the results of the given seed");
    log_seed->add_option("-f,--filename", filename, "The file to execute")->required();
    log_seed->add_option("-s,--seed", seed, "The seed to run");
    log_seed->add_flag("-v,--verbose", verbose, "Verbose logging");
    log_seed->callback([&] {
        RNGHunter hunter(1);
        if (!hunter.parseFile(filename)) {
            std::println(stderr, "Unable to load file");
        }
        hunter.logSeed(seed, verbose ? RNGSim::LogLevel::FULL : RNGSim::LogLevel::PARTIAL);
    });

    WalkthroughType walkthrough_type{WalkthroughType::FULL};
    std::map<std::string, WalkthroughType> map{{"full", WalkthroughType::FULL}, {"simple", WalkthroughType::SIMPLE}};
    std::string out;
    CLI::App* generate_walkthrough = app.add_subcommand("generate_walkthrough",
                                                        "Executes and logs the results of the given seed");
    generate_walkthrough->add_option("-f,--filename", filename, "The file to execute")->required();
    generate_walkthrough->add_option("-s,--seed", seed, "The seed to run");
    generate_walkthrough->add_option("-o,--out", out, "The output file to write")->required();
    generate_walkthrough->add_option("-t,--type", walkthrough_type, "The type of the walkthrough")
        ->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
    generate_walkthrough->callback([&] {
        RNGHunter hunter(1);
        if (!hunter.parseFile(filename)) {
            std::println(stderr, "Unable to load file");
        }
        std::ofstream out_file(out);
        if (!out_file) {
            std::println(stderr, "Error opening file {}", out);
        } else {
            hunter.generateWalkthrough(walkthrough_type, seed, out_file);
            out_file.close();
        }
    });

    CLI::App* convert_to_timestamp = app.add_subcommand("convert_to_timestamp",
                                                        "Converts a seed to a CTManip timestamp");
    convert_to_timestamp->add_option("-s,--seed", seed, "The seed to convert")->required();
    convert_to_timestamp->callback([&] {
        std::println("{}", seed_to_string(seed));
    });

    int max_rolls = 660;
    CLI::App* extend_seed = app.add_subcommand("extend_seed",
                                               "Finds the number of extra RNG rolls necessary for the final command in the seed to succeed");
    extend_seed->add_option("-s,--seed", seed, "The seed to extend")->required();
    extend_seed->add_option("-f,--filename", filename, "The filename to execute")->required();
    extend_seed->add_option("-m,--max_rolls", max_rolls, "The maximum number of rolls to extend the seed");
    extend_seed->callback([&] {
        RNGHunter hunter(1);
        if (!hunter.parseFile(filename)) {
            std::println(stderr, "Unable to load file");
        }
        hunter.extendSeed(seed, max_rolls);
    });

    int players = 3;
    std::set<int> enemy_slots = {0};
    CLI::App* print_init = app.add_subcommand("print_init_table",
                                              "Prints the post initialization RNG seed for each possible starting RNG seed for a number of players and enemies");
    print_init->add_option("-p,--players", players, "The number of player characters in the party");
    print_init->add_option("-e,--enemies", enemy_slots, "The indices of the enemies in the fight");
    print_init->callback([&] {
        print_init_table(players, enemy_slots);
    });

    int enemies = 0;
    std::string rng = "0";
    CLI::App* print_turn_order = app.add_subcommand("print_turn_order",
                                                    "Prints the processing order for enemy and player turns in battle for a given RNG seed");
    print_turn_order->add_option("-p,--players", players, "The number of player characters in the party");
    print_turn_order->add_option("-e,--enemies", enemies, "The number of enemies in the fight");
    print_turn_order->add_option("-r,--rng", rng, "The RNG value that the battle starts on");
    print_turn_order->callback([&] {
        int rng_val = 0;
        try {
            rng_val = std::stoi(rng, nullptr, 16);
        } catch (const std::exception&) {
            std::println(stderr, "Invalid rng value {}, must be a hex value", rng);
            return;
        }
        print_init_order(rng_val, players, enemies);
    });

    CLI11_PARSE(app, argc, argv);
    if (argc == 1) {
        std::println("{}", app.help());
    }
    return 0;
}
