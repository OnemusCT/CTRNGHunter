#include "utils.h"
#include "rng_table.h"
#include "msvc_rand_wrapper.h"

#include <print>
#include <vector>

void print_rng_values(time_t seed, int num_output) {
    MSVCRandWrapper rand = {};
    rand.srand(seed);
    for (int i = 0; i < num_output; i++) {
        int r = rand.rand();
        int rng_index = (r % 0xFF) + 1;
        std::print("{:4}: 0x{:04X}, 0x{:02X}\n", i, r, rng_index);
    }
}

void print_crit_values(int threshold) {
    for (int i = 0; i < 256; i++) {
        std::println("0x{:02X} (0x{:02X}): {:5}", i, rng_table(i), crit_table(i, threshold));
    }
}

int turn_order(int rng, int players) {
    std::set<int> seen;
    int t = 10;
    for (int i = 3; i > players; i--) {
        seen.insert(t--);
    }
    for (int i = rng; i != rng - 1; i++) {
        if (i == 256) i = 0;
        seen.insert(rng_table(i) % 11);
        if (seen.size() == 11) {
            return (i + 1) % 256;
        }
    }
    return -1;
}

int enemy_order(int rng, const std::set<int>& enemy_indices) {
    std::set<int> seen;
    for (int i = rng; i != rng - 1; i++) {
        if (i == 256) i = 0;
        if (int index = rng_table(i) % 8;
            enemy_indices.contains(index)) {
            seen.insert(index);
        }
        if (seen.size() == enemy_indices.size()) {
            return (i + 1) % 256;
        }
    }
    return -1;
}

void print_init_table(int players, const std::set<int>& enemies) {
    std::println("RNG Post initialization for {} characters and {} enemies", players, enemies);
    std::print("\t");
    for (int i = 0; i < 16; i++) {
        std::print("x{:1X}\t", i);
    }
    std::print("\n");
    for (int i = 0; i < 256; i++) {
        if (i % 16 == 0) {
            std::print("{:1X}x\t", i / 16);
        }
        int temp = turn_order(i, players);
        int rng = enemy_order(temp, enemies);
        std::print("{:02X}\t", rng);
        if (i % 16 == 15) std::print("\n");
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
        entities.push_back(10 - i);
    }
    for (int i = 0; i < enemies; i++) {
        exist.insert(3 + i);
    }
    while (entities.size() < 11) {
        entities.push_back(0xFF);
    }
    while (order.size() < 11UL - (3 - players)) {
        int c = rng_table(rng) % 11;
        if (entities[c] != 0xFF) {
            order.push_back(entities[c]);
            entities[c] = 0xFF;
        }
        ++rng;
    }
    std::print("[ ");
    for (int e: order) {
        if (exist.contains(e)) {
            std::print("**0x{:1X}** ", e);
        } else {
            std::print("0x{:1X} ", e);
        }
    }
    std::println("]");
}
