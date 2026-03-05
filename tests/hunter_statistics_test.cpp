#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "hunter_statistics.h"

TEST(HunterStatisticsTest, AddSeedsFoundReturnsCumulativeTotal) {
    HunterStatistics stats(/*total=*/100);
    EXPECT_EQ(stats.add_seeds_found(3), 3);
    EXPECT_EQ(stats.add_seeds_found(5), 8);
    EXPECT_EQ(stats.total_seeds_found(), 8);
}

TEST(HunterStatisticsTest, AddSeedsProcessedReturnsCumulativeTotal) {
    HunterStatistics stats(/*total=*/100);
    EXPECT_EQ(stats.add_seeds_processed(10), 10);
    EXPECT_EQ(stats.add_seeds_processed(20), 30);
    EXPECT_EQ(stats.seeds_processed(), 30);
}

TEST(HunterStatisticsTest, AddSeedsFoundIsThreadSafe) {
    constexpr int kThreadCount = 100;
    HunterStatistics stats(/*total=*/1);
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&stats] { stats.add_seeds_found(1); });
    }
    for (auto& t: threads) t.join();
    EXPECT_EQ(stats.total_seeds_found(), static_cast<size_t>(kThreadCount));
}

TEST(HunterStatisticsTest, ProgressNoOutputBelowFirstMilestone) {
    HunterStatistics stats(/*total=*/100);
    stats.add_seeds_processed(9);
    testing::internal::CaptureStdout();
    stats.maybe_print_progress();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty());
}

TEST(HunterStatisticsTest, ProgressPrintsAtTenPercent) {
    HunterStatistics stats(/*total=*/100);
    stats.add_seeds_processed(10);
    testing::internal::CaptureStdout();
    stats.maybe_print_progress();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("10%"), std::string::npos);
}

TEST(HunterStatisticsTest, ProgressDoesNotDoublePrint) {
    HunterStatistics stats(/*total=*/100);
    stats.add_seeds_processed(10);
    testing::internal::CaptureStdout();
    stats.maybe_print_progress();
    stats.maybe_print_progress();
    std::string output = testing::internal::GetCapturedStdout();
    size_t first = output.find("10%");
    ASSERT_NE(first, std::string::npos);
    EXPECT_EQ(output.find("10%", first + 1), std::string::npos);
}

TEST(HunterStatisticsTest, ProgressPrintsSequentialMilestones) {
    HunterStatistics stats(/*total=*/100);
    testing::internal::CaptureStdout();
    stats.add_seeds_processed(10);
    stats.maybe_print_progress();
    stats.add_seeds_processed(10);
    stats.maybe_print_progress();
    stats.add_seeds_processed(10);
    stats.maybe_print_progress();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("10%"), std::string::npos);
    EXPECT_NE(output.find("20%"), std::string::npos);
    EXPECT_NE(output.find("30%"), std::string::npos);
}