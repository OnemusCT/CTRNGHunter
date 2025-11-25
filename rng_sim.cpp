#include "rng_sim.h"

#include <utility>
#include <format>
#include <iostream>

#include "rng_table.h"

void RNGSim::init(unsigned int seed) {
	rng_.srand(seed);
}

void RNGSim::roll_rng(int n, const std::string& type, bool log) {
	std::vector<int> values(n);
	for (int i = 0; i < n; i++) {
		values[i] = rng_.rand();
	}
	if (log) {
		std::cout << "\t" << type << ": (";
		for (int i : values) {
			std::cout << std::format("{:04X} ", i);
		}
		std::cout << ")" << std::endl;
	}
}
bool RNGSim::load(bool log) {
	roll_rng(42, "load", log);
	return true;
}

bool RNGSim::room(bool log) {
	roll_rng(33, "room", log);
	return true;
}

bool RNGSim::portal(bool log) {
	roll_rng(1, "portal", log);
	return true;
}

bool RNGSim::battle(bool log) {
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	if (log) std::cout << std::format("\tbattle: {:02X} ({:04X})", val, r) << std::endl;
	return true;
}

bool RNGSim::battle_with_rng(int rng_val, bool log) {
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	if (val == rng_val) {
		if (log) std::cout << std::format("\tbattle rng: {:02X} ({:04X})", rng_val, r) << std::endl;
		return true;
	}
	if (log) std::cout << std::format("\tbattle rng: {:02X} DOES NOT MATCH! ({:04X})", val, r) << std::endl;
	return false;
}

bool RNGSim::battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, bool log) {
	int crits = 0;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	int initial_val = val;
	int t_index = 0;
	for (int i = 0; i < max_turns; i++) {
		if (crit_table(val, threshold[t_index])) {
			++crits;
		}
		val = (val + 1) % 0xFF;
		t_index = (t_index + 1) % threshold.size();
	}
	if (log) std::cout << std::format("\tbattle crits: {} in {} turns from {:02X} ({:04X})", crits, max_turns, initial_val, r) << std::endl;
	if (crits >= min_crits) return true;
	return false;
}

bool RNGSim::new_game(bool log) {
	roll_rng(35, "new_game", log);
	return true;
}

bool RNGSim::heal(int num, bool log) {
	roll_rng(num, "heal", log);
	return true;
}