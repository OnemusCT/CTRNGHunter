#include "rng_sim.h"

#include <format>
#include <print>
#include <string_view>

#include "rng_table.h"
#include "msvc_rand_wrapper.h"

// Log labels for each action type, printed during FULL logging.
constexpr std::string_view kLoad = "load";
constexpr std::string_view kRoom = "room";
constexpr std::string_view kPortal = "portal";
constexpr std::string_view kNewGame = "new_game";
constexpr std::string_view kHeal = "heal";
constexpr std::string_view kExtraHeal = "extra_heal";
constexpr std::string_view kBurn = "burn";
constexpr std::string_view kExtraRooms = "extra_rooms";

// Concrete implementation of RNGSim. Wraps an MSVCRandWrapper and tracks
// per-encounter manipulation statistics (extra rooms, heals, battle RNG values).
class RNGSimImpl : public RNGSim {
public:
	RNGSimImpl() = default;

	void init(time_t seed) override;

	bool load(LogLevel log_level) override;

	bool room(int num, LogLevel log_level) override;

	bool extra_rooms(LogLevel log_level) override;

	bool battle(std::string_view name, LogLevel log_level) override;

	bool battle_with_rng(std::vector<int> rng_vals, std::string_view name, LogLevel log_level) override;

	bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) override;

	bool new_game(LogLevel log_level) override;

	bool portal(LogLevel log_level) override;

	bool heal(int num, LogLevel log_level) override;

	bool extra_heal(LogLevel log_level) override;

	void roll_back_rng(int steps) override;

	void burn(int num, LogLevel log_level) override;

	void roll_back_last_rng() override;

	bool disable_extra_rooms(LogLevel log_level) override;

	bool enable_extra_rooms(LogLevel log_level) override;

	bool enable_extra_heals(LogLevel log_level) override;

	bool disable_extra_heals(LogLevel log_level) override;

	std::unordered_map<std::string, int> get_extra_rooms_per_encounter() override;
	std::unordered_map<std::string, int> get_battle_rng_per_encounter() override;
	std::unordered_map<std::string, int> get_extra_heals_per_encounter() override;

private:
	// Advances the RNG by `n` steps. In FULL log mode, prints each individual value;
	// otherwise uses the multi-step fast path.
	void roll_rng(int n, std::string_view type, LogLevel log_level);

	MSVCRandWrapper rng_;

	int last_steps = 0;                 // Steps consumed by the most recent operation (for rollback)
	bool extra_rooms_enabled_ = true;   // Whether extra_rooms() is allowed
	bool extra_heals_enabled_ = false;  // Whether extra_heal() is allowed
	int extra_room_count_ = 0;          // Running count of extra rooms since last encounter
	int extra_heals_count_ = 0;         // Running count of extra heals since last encounter
	int last_battle_rng_ = 0;           // Battle RNG value from the most recent battle
	std::unordered_map<std::string, int> extra_room_map_;   // encounter name -> extra room count
	std::unordered_map<std::string, int> battle_rng_map_;   // encounter name -> battle RNG value
	std::unordered_map<std::string, int> extra_heals_map_;  // encounter name -> extra heal count
};


void RNGSimImpl::init(time_t seed) {
	rng_.srand(seed);
	extra_rooms_enabled_ = true;
	extra_room_count_ = 0;
	extra_room_map_.clear();
	extra_heals_enabled_ = false;
	extra_heals_map_.clear();
}

std::unordered_map<std::string, int> RNGSimImpl::get_extra_rooms_per_encounter() {
	return extra_room_map_;
}

std::unordered_map<std::string, int> RNGSimImpl::get_battle_rng_per_encounter() {
	return battle_rng_map_;
}

std::unordered_map<std::string, int> RNGSimImpl::get_extra_heals_per_encounter() {
	return extra_heals_map_;
}

void RNGSimImpl::roll_rng(int n, std::string_view type, LogLevel log_level) {
	last_steps = n;
	if (log_level == LogLevel::FULL) {
		std::print("\t{}: (", type);
		for (int i = 0; i < n; i++) {
			if (i != 0 && i%33 == 0) std::print("\n\t\t");
			std::print("{:04X} ", rng_.rand());
		}
		std::println(")");
	}
	else {
		rng_.rand(n);
	}
}
bool RNGSimImpl::load(LogLevel log_level) {
	roll_rng(42, kLoad, log_level);
	return true;
}

bool RNGSimImpl::room(int num, LogLevel log_level) {
	if (log_level == LogLevel::FULL) {
		roll_rng(33*num, std::format("{} x{}", kRoom, num), log_level);
	}
	else {
		roll_rng(33*num, kRoom, log_level);
	}
	return true;
}

bool RNGSimImpl::extra_rooms(LogLevel log_level) {
	if (!extra_rooms_enabled_) return false;
	if (log_level == LogLevel::FULL) {
		roll_rng(66, kExtraRooms, log_level);
	}
	else if (log_level == LogLevel::PARTIAL) {
		roll_rng(66, kExtraRooms, LogLevel::NONE);
		std::println("\textra rooms");
	}
	else {
		roll_rng(66, kExtraRooms, LogLevel::NONE);
	}
	++extra_room_count_;
	return true;
}

bool RNGSimImpl::portal(LogLevel log_level) {
	roll_rng(1, kPortal, log_level);
	return true;
}

bool RNGSimImpl::battle(std::string_view name, LogLevel log_level) {
	last_steps = 1;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	last_battle_rng_ = val;
	if (log_level == LogLevel::FULL) std::println("\tbattle: {} {:02X} ({:04X})", name, val, r);
	return true;
}

bool RNGSimImpl::battle_with_rng(std::vector<int> rng_vals, std::string_view name, LogLevel log_level) {
	last_steps = 1;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	for (int rng_val : rng_vals) {
		if (val == rng_val) {
			last_battle_rng_ = val;
			extra_room_map_.insert({std::string(name), extra_room_count_});
			extra_heals_map_.insert({std::string(name), extra_heals_count_});
			battle_rng_map_.insert({std::string(name), val});
			extra_room_count_ = 0;
			extra_heals_count_ = 0;
			if (log_level >= LogLevel::PARTIAL) std::println("\tbattle rng {}: {:02X} ({:04X})", name, rng_val, r);
			return true;
		}
	}
	last_battle_rng_ = val;
	if (log_level >= LogLevel::PARTIAL) std::println("\tbattle rng {}: {:02X} DOES NOT MATCH! ({:04X})", name, val, r);
	return false;
}

bool RNGSimImpl::battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) {
	last_steps = 1;
	int crits = 0;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	int initial_val = val;
	size_t t_index = 0;
	for (int i = 0; i < max_turns; i++) {
		if (crit_table(val, threshold[t_index])) {
			++crits;
		}
		val = (val + 1) % 0xFF;
		t_index = (t_index + 1) % threshold.size();
	}
	last_battle_rng_ = val;
	if (log_level >= LogLevel::PARTIAL) std::println("\tbattle crits {}: {} in {} turns from {:02X} ({:04X})", name, crits, max_turns, initial_val, r);
	if (crits >= min_crits) {
		extra_room_map_.insert({std::string(name), extra_room_count_ });
		extra_heals_map_.insert({ std::string(name), extra_heals_count_ });
		battle_rng_map_.insert({std::string(name), val});
		extra_room_count_ = 0;
		extra_heals_count_ = 0;
		return true;
	}
	return false;
}

bool RNGSimImpl::new_game(LogLevel log_level) {
	roll_rng(35, kNewGame, log_level);
	return true;
}

bool RNGSimImpl::heal(int num, LogLevel log_level) {
	roll_rng(num, kHeal, log_level);
	return true;
}

bool RNGSimImpl::extra_heal(LogLevel log_level) {
	if(extra_heals_enabled_) {
		++extra_heals_count_;
		if (log_level == LogLevel::FULL) {
			roll_rng(1, kExtraHeal, log_level);
		}
		else if (log_level == LogLevel::PARTIAL) {
			roll_rng(1, kExtraHeal, LogLevel::NONE);
			std::println("\textra heal");
		}
		else {
			roll_rng(1, kExtraHeal, LogLevel::NONE);
		}
		return true;
	}

	return false;
}

void RNGSimImpl::roll_back_rng(int steps) {
	for (int i = 0; i < steps; i++) {
		rng_.unrand();
	}
}

void RNGSimImpl::roll_back_last_rng() {
	roll_back_rng(last_steps);
	last_steps = 0;
}

void RNGSimImpl::burn(int num, LogLevel log_level) {
	roll_rng(num, kBurn, log_level);
}

bool RNGSimImpl::disable_extra_rooms(LogLevel) {
	extra_rooms_enabled_ = false;
	return true;
}

bool RNGSimImpl::enable_extra_rooms(LogLevel) {
	extra_rooms_enabled_ = true;
	return true;
}

bool RNGSimImpl::enable_extra_heals(LogLevel) {
	extra_heals_enabled_ = true;
	extra_heals_count_ = 0;
	return true;
}

bool RNGSimImpl::disable_extra_heals(LogLevel) {
	extra_heals_enabled_ = false;
	extra_heals_count_ = 0;
	return true;
}

std::unique_ptr<RNGSim> RNGSim::Create() {
	return std::make_unique<RNGSimImpl>();
}