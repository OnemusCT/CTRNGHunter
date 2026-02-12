#pragma once

// Returns the value at index `i` in Chrono Trigger's 256-entry RNG lookup table.
// Returns -1 if `i` is out of range [0, 255].
int rng_table(int i);

// Returns true if the RNG table value at index `i` (mod 100) is <= `threshold`,
// indicating a critical hit. `threshold` represents the character's crit chance percentage.
bool crit_table(int i, int threshold);
