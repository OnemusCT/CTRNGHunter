#pragma once

#include <inja/inja.hpp>

#include <string>
#include <unordered_map>
#include "rng_sim.h"

enum WalkthroughType {
    SIMPLE,
    ONEMUS,
    FULL,
};

/**
 * Writes a walkthrough of the manip specific route in markdown format to the stream passed in.
 * stats_map - a map of boss name to statistics for the battle (battle rng, extra rooms, extra heals).
 * out - the stream to write to.
 */
void generate_walkthrough(WalkthroughType type, time_t seed, 
                          const std::unordered_map<std::string, RNGSim::EncounterStats>& stats_map, std::ostream& out);
