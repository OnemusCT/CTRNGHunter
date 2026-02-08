#include <gtest/gtest.h>

#include "msvc_rand_wrapper.h"

// Helper: standalone MSVC LCG to compute expected values independently.
static int msvc_rand_ref(time_t &seed) {
	seed = (seed * 214013) + 2531011;
	return static_cast<int>((seed >> 16) & 0x7FFF);
}

TEST(MSVCRandWrapper, SingleRandMatchesMSVCLCG) {
	MSVCRandWrapper rng;
	time_t seed = 12345;
	rng.srand(seed);

	for (int i = 0; i < 100; i++) {
		int expected = msvc_rand_ref(seed);
		EXPECT_EQ(rng.rand(), expected) << "Mismatch at iteration " << i;
	}
}

TEST(MSVCRandWrapper, MultiStepRandMatchesRepeatedSingle) {
	// Calling rand(n) should advance the state the same as n individual calls,
	// and return the value of the last call.
	time_t seed = 42;

	MSVCRandWrapper rng_single;
	rng_single.srand(seed);

	MSVCRandWrapper rng_multi;
	rng_multi.srand(seed);

	// Advance single-step 10 times
	int last_val = 0;
	for (int i = 0; i < 10; i++) {
		last_val = rng_single.rand();
	}

	// Advance multi-step once
	int multi_val = rng_multi.rand(10);

	EXPECT_EQ(multi_val, last_val);

	// After this, both should produce the same next value
	EXPECT_EQ(rng_single.rand(), rng_multi.rand());
}

TEST(MSVCRandWrapper, UnrandReversesRand) {
	MSVCRandWrapper rng;
	rng.srand(99);

	int first = rng.rand();
	rng.unrand();
	int second = rng.rand();

	EXPECT_EQ(first, second);
}

TEST(MSVCRandWrapper, SeedZeroProducesKnownValues) {
	MSVCRandWrapper rng;
	rng.srand(0);

	// MSVC LCG with seed 0:
	// seed = 0 * 214013 + 2531011 = 2531011 => rand = (2531011 >> 16) & 0x7FFF = 38
	EXPECT_EQ(rng.rand(), 38);
}
