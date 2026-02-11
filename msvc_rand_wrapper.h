#pragma once

#include <cstdint>
#include <ctime>
#include <unordered_map>
#include <utility>

/**
 * Wraps the MSVC C runtime linear congruential generator (LCG) with parameters
 * a=214013, c=2531011. Supports multi-step advances via exponentiation by squaring
 * and single-step reversal. Used to replicate the RNG behavior of Chrono Trigger (SNES)
 * as emulated through the MSVC runtime.
 */
class MSVCRandWrapper {
  public:
	MSVCRandWrapper();

	// Seeds the generator with the given value (typically a Unix timestamp).
	void srand(uint32_t seed);

	// Advances the LCG state by `n` steps and returns the output of the final step.
	// Output is (seed >> 16) & 0x7FFF, matching MSVC's rand() behavior.
	int rand(int n = 1);

	// Reverses the LCG by one step, restoring the state prior to the last rand(1) call.
	void unrand();
  private:
	uint32_t seed_;

	// Precomputed LCG parameters for multi-step jumps: seed' = seed * a + c
	std::unordered_map<int, std::pair<uint32_t, uint32_t>> lcg_params_;
};