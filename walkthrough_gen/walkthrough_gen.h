#pragma once

#include <inja/inja.hpp>

#include <string>
#include <unordered_map>

/**
 * Writes a walkthrough of the manip specific route in markdown format to the stream passed in.
 * rng_map - a map of boss name to RNG value (boss names are determined by their name
 *   in the rng_hunter input files).
 * rooms - a map of boss name to how many extra sets of room transitions are needed
 *   prior to the boss.
 * out - the stream to write to.
 */
void generate_walkthrough(time_t seed, const std::unordered_map<std::string, int>& rng_map,
                          const std::unordered_map<std::string, int>& rooms_map,
                          const std::unordered_map<std::string, int>& heal_map, std::ostream& out);
