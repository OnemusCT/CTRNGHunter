#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <string>

class RNGSim {
  public:
	virtual ~RNGSim() = default;

	RNGSim() = default;

	virtual void init(time_t seed) = 0;

	virtual bool load(bool log) = 0;

	virtual bool room(int num, bool log) = 0;

	virtual bool extra_rooms(bool log) = 0;

	virtual bool battle(bool log) = 0;

	virtual bool battle_with_rng(std::vector<int> rng_vals, std::string_view name, bool log) = 0;

	virtual bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, bool log) = 0;

	virtual bool new_game(bool log) = 0;

	virtual bool portal(bool log) = 0;

	virtual bool heal(int num, bool log) = 0;

	virtual bool disable_extra_rooms(bool log) = 0;

	virtual bool enable_extra_rooms(bool log) = 0;

	virtual void roll_back_rng(int steps) = 0;

	virtual void roll_back_last_rng() = 0;

	virtual void burn(int num, bool log) = 0;

	virtual std::unordered_map<std::string, int> get_extra_rooms_per_encounter() = 0;
	virtual std::unordered_map<std::string, int> get_battle_rng_per_encounter() = 0;


	static std::unique_ptr<RNGSim> Create();

};

