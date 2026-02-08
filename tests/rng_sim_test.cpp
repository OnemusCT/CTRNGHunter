#include <gtest/gtest.h>

#include "rng_sim.h"
#include "msvc_rand_wrapper.h"

int CalcExpectedRNG(int r) {
	return (r % 0xFF) + 1;
}

TEST(RNGSimTest, CreateSucceeds) {
	auto sim = RNGSim::Create();
	ASSERT_NE(sim, nullptr);
}

TEST(RNGSimTest, LoadAdvancesRNG42Steps) {
	auto sim = RNGSim::Create();
	// Compare RNGSim state after load() vs manual 42-step advance.
	time_t seed = 100;

	// Manually compute: advance 42 times, then one more for battle.
	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(42);  // load
	int r = rng.rand();  // battle

	sim->init(seed);
	EXPECT_TRUE(sim->load(RNGSim::NONE));
	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, RoomAdvancesRNGCorrectly) {
	auto sim = RNGSim::Create();
	time_t seed = 200;
	sim->init(seed);
	sim->room(3, RNGSim::NONE);  // Should advance 33*3 = 99 steps

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(99);  // room(3)
	int r = rng.rand();  // battle

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, NewGameAdvances35Steps) {
	auto sim = RNGSim::Create();
	time_t seed = 300;
	sim->init(seed);
	sim->new_game(RNGSim::NONE);

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(35);
	int r = rng.rand();

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, PortalAdvances1Step) {
	auto sim = RNGSim::Create();
	time_t seed = 400;
	sim->init(seed);
	sim->portal(RNGSim::NONE);

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(1);
	int r = rng.rand();

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, HealAdvancesNSteps) {
	auto sim = RNGSim::Create();
	time_t seed = 500;
	int heal_count = 5;

	sim->init(seed);
	sim->heal(heal_count, RNGSim::NONE);

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(heal_count);
	int r = rng.rand();

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, BurnAdvancesNSteps) {
	auto sim = RNGSim::Create();
	time_t seed = 600;
	sim->init(seed);
	sim->burn(10, RNGSim::NONE);

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(10);
	int r = rng.rand();

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, RollBackLastRngUndoesLastOperation) {
	auto sim = RNGSim::Create();
	time_t seed = 700;
	sim->init(seed);
	sim->load(RNGSim::NONE);       // advance 42
	sim->room(1, RNGSim::NONE);    // advance 33
	sim->roll_back_last_rng();     // should undo room (33 steps)

	// Both should produce the same battle result
	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(42);
	int r = rng.rand();

	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}

TEST(RNGSimTest, ExtraRoomsEnabledByDefault) {
	auto sim = RNGSim::Create();
	sim->init(0);
	// extra_rooms should work when enabled (default)
	EXPECT_TRUE(sim->extra_rooms(RNGSim::NONE));
}

TEST(RNGSimTest, ExtraRoomsDisabledReturnsFalse) {
	auto sim = RNGSim::Create();
	sim->init(0);
	sim->disable_extra_rooms(RNGSim::NONE);
	EXPECT_FALSE(sim->extra_rooms(RNGSim::NONE));
}

TEST(RNGSimTest, ExtraRoomsReEnabled) {
	auto sim = RNGSim::Create();
	sim->init(0);
	sim->disable_extra_rooms(RNGSim::NONE);
	EXPECT_FALSE(sim->extra_rooms(RNGSim::NONE));
	sim->enable_extra_rooms(RNGSim::NONE);
	EXPECT_TRUE(sim->extra_rooms(RNGSim::NONE));
}

TEST(RNGSimTest, ExtraHealDisabledByDefault) {
	auto sim = RNGSim::Create();
	sim->init(0);
	EXPECT_FALSE(sim->extra_heal(RNGSim::NONE));
}

TEST(RNGSimTest, ExtraHealEnabledReturnsTrue) {
	auto sim = RNGSim::Create();
	sim->init(0);
	sim->enable_extra_heals(RNGSim::NONE);
	EXPECT_TRUE(sim->extra_heal(RNGSim::NONE));
}

TEST(RNGSimTest, BattleWithRngMatchReturnsTrue) {
	auto sim = RNGSim::Create();
	time_t seed = 800;
	MSVCRandWrapper rng;
	rng.srand(seed);
	int r = rng.rand();

	sim->init(seed);
	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test_battle", RNGSim::NONE));
}

TEST(RNGSimTest, BattleWithRngNoMatchReturnsFalse) {
	auto sim = RNGSim::Create();
	time_t seed = 900;
	MSVCRandWrapper rng;
	rng.srand(seed);
	int r = rng.rand();
	int actual_val = CalcExpectedRNG(r);

	// Use a value that definitely doesn't match
	int wrong_val = (actual_val + 1) % 0xFF;  // shift by 1 within valid range

	sim->init(seed);
	EXPECT_FALSE(sim->battle_with_rng({wrong_val}, "test_battle", RNGSim::NONE));
}

TEST(RNGSimTest, BattleWithRngTracksEncounterData) {
	auto sim = RNGSim::Create();
	time_t seed = 1000;
	MSVCRandWrapper rng;
	rng.srand(seed);
	int r = rng.rand();

	sim->init(seed);
	sim->battle_with_rng({CalcExpectedRNG(r)}, "boss1", RNGSim::NONE);

	auto rng_map = sim->get_battle_rng_per_encounter();
	ASSERT_TRUE(rng_map.count("boss1"));
	EXPECT_EQ(rng_map["boss1"], CalcExpectedRNG(r));
}

TEST(RNGSimTest, ExtraRoomsCountTrackedPerEncounter) {
	auto sim = RNGSim::Create();
	time_t seed = 1100;
	MSVCRandWrapper rng;
	rng.srand(seed);

	sim->init(seed);
	sim->extra_rooms(RNGSim::NONE);  // +1 extra room
	sim->extra_rooms(RNGSim::NONE);  // +1 extra room

	// Advance the reference RNG to match
	rng.rand(66);  // first extra_rooms
	rng.rand(66);  // second extra_rooms
	int r = rng.rand();

	sim->battle_with_rng({CalcExpectedRNG(r)}, "boss2", RNGSim::NONE);

	auto rooms_map = sim->get_extra_rooms_per_encounter();
	ASSERT_TRUE(rooms_map.count("boss2"));
	EXPECT_EQ(rooms_map["boss2"], 2);
}

TEST(RNGSimTest, InitResetsState) {
	auto sim = RNGSim::Create();
	sim->init(0);
	sim->load(RNGSim::NONE);
	sim->disable_extra_rooms(RNGSim::NONE);
	sim->enable_extra_heals(RNGSim::NONE);

	// Re-init should reset everything
	sim->init(0);

	// Extra rooms should be enabled again (default)
	EXPECT_TRUE(sim->extra_rooms(RNGSim::NONE));

	// Extra heals should be disabled again (default)
	EXPECT_FALSE(sim->extra_heal(RNGSim::NONE));
}

TEST(RNGSimTest, SequenceOfOperationsMatchesManualRNG) {
	auto sim = RNGSim::Create();
	// Full scenario: new_game -> load -> room(2) -> battle
	time_t seed = 5555;

	MSVCRandWrapper rng;
	rng.srand(seed);
	rng.rand(35);   // new_game
	rng.rand(42);   // load
	rng.rand(66);   // room(2)
	int r = rng.rand();

	sim->init(seed);
	sim->new_game(RNGSim::NONE);
	sim->load(RNGSim::NONE);
	sim->room(2, RNGSim::NONE);
	EXPECT_TRUE(sim->battle_with_rng({CalcExpectedRNG(r)}, "test", RNGSim::NONE));
}