#pragma once

#include "rng_table.h"

void init(time_t seed);

bool load(bool log);

bool room(bool log);

bool battle(bool log);

bool battle_with_rng(int rng_val, bool log);

bool battle_with_crits(int threshold, int min_crits, int max_turns, bool log);

bool new_game(bool log);