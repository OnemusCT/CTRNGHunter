#include "battle_sim.h"

#include "rng_table.h"

#include <iostream>
#include <format>
#include <set>
#include <vector>
#include <string>

constexpr int kNumPartyMembers = 3;
constexpr int kMaxEntities = 11;
constexpr int kPlayerTargetRNGLimit = 99;
constexpr int kPlayerTargetDivisor = 33;
constexpr int kCounterRollDivisor = 25;
constexpr int kShadowCounterIndex = 3;

// Helper function: Select a target that isn't dead
RollResult select_live_target(RNGTable& rng, const std::set<int>& dead_pcs) {
    auto get_target = [](int value) { return (value % kPlayerTargetRNGLimit) / kPlayerTargetDivisor; };

    RollResult result = rng.roll_result();
    while (dead_pcs.find(get_target(result.value)) != dead_pcs.end()) {
        result = rng.roll_result();
        std::cout << std::format("Target: {} - Dead ({}), reroll - ", get_target(result.value), result.to_string());
    }
    result.value = get_target(result.value);
    return result;
}

// Helper function: Print a sequence of rolls with a label
void print_roll_sequence(RNGTable& rng, const std::string& label, int count) {
    std::cout << label;
    for (int i = 0; i < count; i++) {
        std::cout << std::format(" {:02X}", rng.get_seed());
        rng.roll();
    }
}

void berserk_attack(RNGTable& rng) {
    std::cout << std::format("Berserk attack - Start RNG {:02X}", rng.get_seed()) << std::endl;

    // Roll until we find a target (value == 0)
    auto target = rng.roll_until([](int val) { return val == 0; }, 8);
    auto hit = rng.roll_result();
    auto damage = rng.roll_result();

    std::cout << std::format("\tTarget found at {}, hit {}, damage {}",
        target.to_string(), hit.to_string(), damage.to_string()) << std::endl;
}

void wander_random_target(RNGTable& rng, const std::set<int>& dead_pcs) {
    std::cout << "Wander" << std::endl;
    std::cout << "\t";

    RollResult target = select_live_target(rng, dead_pcs);

    std::cout << std::format("Target: {} - {}", target.value, target.seed) << std::endl;
}

void basic_attack(RNGTable& rng) {
    auto rolls = rng.roll_multiple(2);
    std::cout << std::format("Attack - Hit {:02X}, damage {:02X}",
        rolls[0].seed, rolls[1].seed) << std::endl;
}

void plasma_attack(RNGTable& rng) {
    auto rolls = rng.roll_multiple(3);
    std::cout << std::format("Plasma Attack - Hit {:02X}, damage {:02X}, status {:02X}",
        rolls[0].seed, rolls[1].seed, rolls[2].seed) << std::endl;
}

void geyser(RNGTable& rng, int players_alive = kNumPartyMembers) {
    print_roll_sequence(rng, "Geyser - Hits", players_alive);
    print_roll_sequence(rng, ", damage", players_alive);
    print_roll_sequence(rng, ", status", players_alive);
    std::cout << std::endl;
}

void enemy_attack(RNGTable& rng, const std::set<int>& dead_pcs) {
    constexpr int kPlayerTargetRNGLimit = 99;
    constexpr int kPlayerTargetDivisor = 33;

    std::cout << "Enemy attack" << std::endl;
    std::cout << "\t";

    RollResult target = select_live_target(rng, dead_pcs);
    auto rolls = rng.roll_multiple(2);  // hit and damage

    std::cout << std::format("Target: {} - {:02X}, hit {}, damage {}",
        target.value, target.seed,
        rolls[0].to_string(), rolls[1].to_string()) << std::endl;
}

void fire_sword(RNGTable& rng) {
    print_roll_sequence(rng, "Fire Sword -", 3);
    std::cout << std::endl;
}

void magus_counter(RNGTable& rng, int players_alive = kNumPartyMembers) {
    static const std::vector<std::string> counter_types = { "Lightning", "Water", "Fire", "Shadow" };

    std::cout << "Magus Counter" << std::endl;

    auto counter_roll = rng.roll_result();
    int counter_type = (counter_roll.value % 100) / kCounterRollDivisor;

    std::cout << std::format("\t{}", counter_types[counter_type]);

    if (counter_type == kShadowCounterIndex) {
        // Shadow counter uses 1 roll
        std::cout << std::format(" - {:02X}", rng.get_seed()) << std::endl;
        rng.roll();
    }
    else {
        // Other counters use 1 roll per living player
        print_roll_sequence(rng, " -", players_alive);
        std::cout << std::endl;
    }
}

void initialize(RNGTable& rng, int players, int enemies) {
    std::cout << std::format("Initializing battle for {} players, {} enemies, at {:02X} RNG",
        players, enemies, rng.get_seed()) << std::endl;

    std::set<int> exist;
    std::vector<int> order;
    std::vector<int> entities;

    // Build entity list
    for (int i = 0; i < players; i++) {
        entities.push_back(i);
        exist.insert(i);
    }
    for (int i = 0; i < 8; i++) {
        entities.push_back(10 - i);
    }
    for (int i = 0; i < enemies; i++) {
        exist.insert(kNumPartyMembers + i);
    }
    while (entities.size() < kMaxEntities) {
        entities.push_back(0xFF);
    }

    // Randomize turn order
    size_t expected_turns = kMaxEntities - (kNumPartyMembers - players);
    while (order.size() < expected_turns) {
        int candidate = rng_table(rng.get_seed()) % kMaxEntities;
        if (entities[candidate] != 0xFF) {
            order.push_back(entities[candidate]);
            entities[candidate] = 0xFF;
        }
        rng.roll();
    }

    // Print turn order
    std::cout << "\t[ ";
    for (int entity : order) {
        if (exist.find(entity) != exist.end()) {
            std::cout << std::format("**0x{:1X}** ", entity);
        }
        else {
            std::cout << std::format("0x{:1X} ", entity);
        }
    }

    // Roll until we get a valid enemy index
    while (rng_table(rng.get_seed()) % 8 >= enemies) {
        rng.roll();
    }

    std::cout << "]" << std::format(" final RNG - {:02X}", rng.get_seed()) << std::endl;
    rng.roll();
}

void sim_magus() {
    RNGTable rng(0x0D);
    initialize(rng, kNumPartyMembers, 1);
    berserk_attack(rng);
    wander_random_target(rng, {});
    plasma_attack(rng);
    magus_counter(rng);
    wander_random_target(rng, {});
    berserk_attack(rng);
    geyser(rng);
    wander_random_target(rng, {});
    fire_sword(rng);
    magus_counter(rng);
    wander_random_target(rng, {1});
    enemy_attack(rng, {1});
    berserk_attack(rng);
    fire_sword(rng);
    magus_counter(rng);
    geyser(rng);
    berserk_attack(rng);
    wander_random_target(rng, {});
    plasma_attack(rng);
    magus_counter(rng);
    enemy_attack(rng, {1});
    wander_random_target(rng, {});
    geyser(rng, 2);
    fire_sword(rng);
    magus_counter(rng, 2);
    wander_random_target(rng, {1});
    enemy_attack(rng, {1});
    berserk_attack(rng);
    fire_sword(rng);
}