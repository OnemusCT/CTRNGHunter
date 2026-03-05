#pragma once

#include <set>
#include <ctime>

// Prints the raw RNG output and battle RNG index for each of the first `num_output`
// calls from the given seed.
void print_rng_values(time_t seed, int num_output);

// Prints whether each of the 256 RNG table entries would produce a critical hit
// at the given crit chance threshold.
void print_crit_values(int threshold);

// Simulates Chrono Trigger's battle turn-order initialization for player characters
// and enemies.
// Starting from RNG table index `rng`, walks the table assigning slots to `players`
// characters and 8 enemy slots. Returns the RNG index after all 11 slots are assigned.
int turn_order(int rng, int players = 3);

// Continues turn-order initialization for enemies. Starting from RNG table index `rng`,
// walks the table until every enemy in `enemy_indices` has been assigned a slot.
// Returns the RNG index after all enemies are placed.
int enemy_order(int rng, const std::set<int>& enemy_indices);

// Prints a 16x16 table showing the post-initialization RNG index for every possible
// starting RNG index (0x00-0xFF), given `players` party members and `enemies` slots.
void print_init_table(int players = 3, const std::set<int>& enemies = {0});

// Prints the full turn processing order for a battle starting at RNG index `rng`,
// with `players` party members and `enemies` enemy count. Active entities are bolded.
void print_init_order(int rng, int players, int enemies);
