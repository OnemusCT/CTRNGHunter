#pragma once

#include <time.h>
#include <vector>
#include <string>
#include <string_view>

#include "msvc_rand_wrapper.h"

class RNGSim {
  public:
	RNGSim() = default;

	void init(unsigned int seed);

	bool load(bool log);

	bool room(bool log);

	bool battle(bool log);

	bool battle_with_rng(int rng_val, bool log);

	bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, bool log);

	bool new_game(bool log);

	bool portal(bool log);

	bool heal(int num, bool log);
  private:
	void roll_rng(int n, std::string_view type, bool log);

	MSVCRandWrapper rng_;
};

