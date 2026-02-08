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

	enum LogLevel {
		NONE = 0,		// No logging
		PARTIAL = 1,	// Log the manipulated parts (extra rooms, heals, fight RNG)
		FULL = 2		// Log everything
	};

	virtual void init(time_t seed) = 0;

	virtual bool load(LogLevel log_level) = 0;

	virtual bool room(int num, LogLevel log_level) = 0;

	virtual bool extra_rooms(LogLevel log_level) = 0;

	virtual bool battle(LogLevel log_level) = 0;

	virtual bool battle_with_rng(std::vector<int> rng_vals, std::string_view name, LogLevel log_level) = 0;

	virtual bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) = 0;

	virtual bool new_game(LogLevel log_level) = 0;

	virtual bool portal(LogLevel log_level) = 0;

	virtual bool heal(int num, LogLevel log_level) = 0;

	virtual bool extra_heal(LogLevel log_level) = 0;

	virtual bool disable_extra_rooms(LogLevel log_level) = 0;

	virtual bool enable_extra_rooms(LogLevel log_level) = 0;

	virtual bool enable_extra_heals(LogLevel log_level) = 0;

	virtual bool disable_extra_heals(LogLevel log_level) = 0;

	virtual void roll_back_rng(int steps) = 0;

	virtual void roll_back_last_rng() = 0;

	virtual void burn(int num, LogLevel log_level) = 0;

	virtual std::unordered_map<std::string, int> get_extra_rooms_per_encounter() = 0;
	virtual std::unordered_map<std::string, int> get_battle_rng_per_encounter() = 0;
	virtual std::unordered_map<std::string, int> get_extra_heals_per_encounter() = 0;


	static std::unique_ptr<RNGSim> Create();


};

