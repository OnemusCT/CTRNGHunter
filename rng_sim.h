#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <string>

/**
 * Simulates Chrono Trigger's RNG consumption during a speedrun route.
 *
 * Each method corresponds to a game action that advances the RNG by a known number
 * of steps (e.g. loading a save consumes 42 RNG calls, entering a room consumes 33).
 * The simulator tracks per-encounter statistics (extra rooms, battle RNG values, extra
 * heals) for generating a walkthrough of the route.
 * Created via the RNGSim::Create() factory method.
 */
class RNGSim {
  public:
	virtual ~RNGSim() = default;

	RNGSim() = default;

	enum LogLevel {
		NONE = 0,		// No logging
		PARTIAL = 1,	// Log the manipulated parts (extra rooms, heals, fight RNG)
		FULL = 2		// Log everything
	};

	// Seeds the internal RNG and resets all state (extra rooms, heals, encounter maps).
	virtual void init(time_t seed) = 0;

	// Simulates loading a save file (advances RNG by 42 steps).
	virtual bool load(LogLevel log_level) = 0;

	// Simulates traversing `num` room transitions (advances RNG by 33 * num steps).
	virtual bool room(int num, LogLevel log_level) = 0;

	// Adds an extra pair of room transitions for RNG manipulation (advances RNG by 66 steps).
	// Returns false if extra rooms are currently disabled.
	virtual bool extra_rooms(LogLevel log_level) = 0;

	// Simulates a battle encounter (advances RNG by 1 step), computing the battle RNG value.
	virtual bool battle(std::string_view name, LogLevel log_level) = 0;

	// Simulates a battle and checks if the resulting RNG value matches any value in `rng_vals`.
	// On match, records the encounter data under `name`. Returns true if a match is found.
	virtual bool battle_with_rng(std::vector<int> rng_vals, std::string_view name, LogLevel log_level) = 0;

	// Simulates a battle and checks if enough critical hits occur.
	// Uses `threshold` (crit chances per turn, cycled), counts crits over `max_turns`,
	// and returns true if crits >= `min_crits`. Records encounter data under `name` on success.
	virtual bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) = 0;

	// Simulates starting a new game (advances RNG by 35 steps).
	virtual bool new_game(LogLevel log_level) = 0;

	// Simulates a portal/end-of-time/special room transition (advances RNG by 1 step).
	virtual bool portal(LogLevel log_level) = 0;

	// Simulates `num` healing actions (advances RNG by `num` steps).
	virtual bool heal(int num, LogLevel log_level) = 0;

	// Adds an extra heal action for RNG manipulation (advances RNG by 1 step).
	// Returns false if extra heals are currently disabled.
	virtual bool extra_heal(LogLevel log_level) = 0;

	// Disables extra room transitions. extra_rooms() will return false until re-enabled.
	virtual bool disable_extra_rooms(LogLevel log_level) = 0;

	// Enables extra room transitions (enabled by default after init).
	virtual bool enable_extra_rooms(LogLevel log_level) = 0;

	// Enables extra healing actions (disabled by default after init).
	virtual bool enable_extra_heals(LogLevel log_level) = 0;

	// Disables extra healing actions.
	virtual bool disable_extra_heals(LogLevel log_level) = 0;

	// Reverses the RNG state by `steps` individual steps.
	virtual void roll_back_rng(int steps) = 0;

	// Reverses the RNG state by the number of steps consumed by the last operation.
	virtual void roll_back_last_rng() = 0;

	// Burns (discards) `num` RNG values, advancing the state without game effect.
	virtual void burn(int num, LogLevel log_level) = 0;

	// Returns a map of encounter name -> number of extra room transition pairs added before it.
	virtual std::unordered_map<std::string, int> get_extra_rooms_per_encounter() = 0;

	// Returns a map of encounter name -> battle RNG value that was rolled.
	virtual std::unordered_map<std::string, int> get_battle_rng_per_encounter() = 0;

	// Returns a map of encounter name -> number of extra heals added before it.
	virtual std::unordered_map<std::string, int> get_extra_heals_per_encounter() = 0;

	// Factory method. Returns a new RNGSim instance.
	static std::unique_ptr<RNGSim> Create();


};

