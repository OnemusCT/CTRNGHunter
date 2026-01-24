#include "rng_sim.h"

#include <format>
#include <iostream>
#include <string_view>

#include "rng_table.h"
#include "msvc_rand_wrapper.h"

constexpr std::string_view kLoad = "load";
constexpr std::string_view kRoom = "room";
constexpr std::string_view kPortal = "portal";
constexpr std::string_view kNewGame = "new_game";
constexpr std::string_view kHeal = "heal";
constexpr std::string_view kBurn = "burn";
constexpr std::string_view kExtraRooms = "extra_rooms";

class RNGSimImpl : public RNGSim {
public:
	RNGSimImpl() : rng_() {}

	void init(time_t seed) override;

	bool load(LogLevel log_level) override;

	bool room(int num, LogLevel log_level) override;

	bool extra_rooms(LogLevel log_level) override;

	bool battle(LogLevel log_level) override;

	bool battle_with_rng(std::vector<int> rng_vals, std::string_view name, LogLevel log_level) override;

	bool battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) override;

	bool new_game(LogLevel log_level) override;

	bool portal(LogLevel log_level) override;

	bool heal(int num, LogLevel log_level) override;

	void roll_back_rng(int steps) override;

	void burn(int num, LogLevel log_level) override;

	void roll_back_last_rng() override;

	bool disable_extra_rooms(LogLevel log_level) override;

	bool enable_extra_rooms(LogLevel log_level) override;

	std::unordered_map<std::string, int> get_extra_rooms_per_encounter() override;
	std::unordered_map<std::string, int> get_battle_rng_per_encounter() override;

private:
	void roll_rng(int n, std::string_view type, LogLevel log_level);

	MSVCRandWrapper rng_;

	int last_steps = 0;
	bool extra_rooms_enabled_ = true;
	int extra_room_count_ = 0;
	int last_battle_rng_ = 0;
	std::unordered_map<std::string, int> extra_room_map_;
	std::unordered_map<std::string, int> battle_rng_map_;
};


void RNGSimImpl::init(time_t seed) {
	rng_.srand(seed);
	extra_rooms_enabled_ = true;
	extra_room_count_ = 0;
	extra_room_map_.clear();
}

std::unordered_map<std::string, int> RNGSimImpl::get_extra_rooms_per_encounter() {
	return extra_room_map_;
}

std::unordered_map<std::string, int> RNGSimImpl::get_battle_rng_per_encounter() {
	return battle_rng_map_;
}

void RNGSimImpl::roll_rng(int n, std::string_view type, LogLevel log_level) {
	last_steps = n;
	if (log_level == LogLevel::FULL) {
		std::cout << "\t" << type << ": (";
		for (int i = 0; i < n; i++) {
			if (i != 0 && i%33 == 0) std::cout << std::endl << "\t\t";
			std::cout << std::format("{:04X} ", rng_.rand());
		}
		std::cout << ")" << std::endl;
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
		std::cout << "\textra rooms" << std::endl;
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

bool RNGSimImpl::battle(LogLevel log_level) {
	last_steps = 1;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	last_battle_rng_ = val;
	if (log_level == LogLevel::FULL) std::cout << std::format("\tbattle: {:02X} ({:04X})", val, r) << std::endl;
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
			battle_rng_map_.insert({std::string(name), val});
			extra_room_count_ = 0;
			if (log_level >= LogLevel::PARTIAL) std::cout << std::format("\tbattle rng {}: {:02X} ({:04X})", name, rng_val, r) << std::endl;
			return true;
		}
	}
	last_battle_rng_ = val;
	if (log_level >= LogLevel::PARTIAL) std::cout << std::format("\tbattle rng {}: {:02X} DOES NOT MATCH! ({:04X})", name, val, r) << std::endl;
	return false;
}

bool RNGSimImpl::battle_with_crits(std::vector<int> threshold, int min_crits, int max_turns, std::string_view name, LogLevel log_level) {
	last_steps = 1;
	int crits = 0;
	int r = rng_.rand();
	int val = (r % 0xFF) + 1;
	int initial_val = val;
	unsigned long t_index = 0;
	for (int i = 0; i < max_turns; i++) {
		if (crit_table(val, threshold[t_index])) {
			++crits;
		}
		val = (val + 1) % 0xFF;
		t_index = (t_index + 1) % threshold.size();
	}
	last_battle_rng_ = val;
	if (log_level >= LogLevel::PARTIAL) std::cout << std::format("\tbattle crits {}: {} in {} turns from {:02X} ({:04X})", name, crits, max_turns, initial_val, r) << std::endl;
	if (crits >= min_crits) {
		extra_room_map_.insert({std::string(name), extra_room_count_ });
		battle_rng_map_.insert({std::string(name), val});
		extra_room_count_ = 0;
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

std::unique_ptr<RNGSim> RNGSim::Create() {
	return std::make_unique<RNGSimImpl>();
}