#include "rng_hunter.h"

#include <cstdlib>
#include <utility>
#include <format>
#include <iostream>

#include "rng_table.h"

void init(time_t seed) {
	srand(seed);
}

bool load(bool log) {
	for (int i = 0; i < 42; i++) {
		std::ignore = rand();
	}
	return true;
}

bool room(bool log) {
	for (int i = 0; i < 33; i++) {
		std::ignore = rand();
	}
	return true;
}

bool battle(bool log) {
	std::ignore = rand();
	return true;
}

bool battle_with_rng(int rng_val, bool log) {
	int val = (rand() % 0xFF) + 1;
	if (val == rng_val) {
		if (log) {
			std::cout << std::format("\tbattle rng: {:02X}", rng_val) << std::endl;
		}
		return true;
	}
	if (log) {
		std::cout << std::format("\tbattle rng: {:02X} DOES NOT MATCH!", rng_table(val)) << std::endl;
	}
	return false;
}

bool battle_with_crits(int threshold, int min_crits, int max_turns, bool log) {
	int crits = 0;
	int val = (rand() % 0xFF) + 1;
	int initial_val = val;
	for (int i = 0; i < max_turns; i++) {
		if (crit_table(val, threshold)) {
			++crits;
		}
		val = (val + 1) % 0xFF;
	}
	if(log) {
		std::cout << std::format("\tbattle crits: {} in {} turns from {:02X}", crits, max_turns, initial_val) << std::endl;
	}
	if (crits >= min_crits) return true;
	return false;
}

bool new_game(bool log) {
	for (int i = 0; i < 35; i++) {
		std::ignore = rand();
	}
	return true;
}