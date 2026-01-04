#pragma once

#include <time.h>
#include <vector>
#include <string>
#include <string_view>

#include "msvc_rand_wrapper.h"
#include <memory>

class RNGSim {
  public:
	virtual ~RNGSim() = default;

	RNGSim() = default;

	virtual void init(time_t seed) = 0;

	virtual bool load(bool log) = 0;

	virtual bool room(bool log) = 0;

	virtual bool battle(bool log) = 0;

	virtual bool battle_with_rng(std::vector<int> rng_vals, bool log) = 0;

	virtual bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, bool log) = 0;

	virtual bool new_game(bool log) = 0;

	virtual bool portal(bool log) = 0;

	virtual bool heal(int num, bool log) = 0;

	virtual void roll_back_rng(int steps) = 0;

	virtual void roll_back_last_rng() = 0;

	virtual void burn(int num, bool log) = 0;

	static std::unique_ptr<RNGSim> Create();

};

