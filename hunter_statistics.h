#pragma once

#include <atomic>
#include <ctime>
#include <mutex>
#include <print>

// Thread-safe progress tracker for multi-threaded seed searching.
// Aggregates per-thread counters and prints progress at 10% intervals.
class HunterStatistics {
    public:
        // total - Total number of seeds to process (used for percentage calculation).
        explicit HunterStatistics(time_t total) : total_seeds_found_(0),
                                                  seeds_processed_(0), total_(total), last_percentage_(0) {
        }

        size_t total_seeds_found() {
            return total_seeds_found_;
        }

        size_t seeds_processed() {
            return seeds_processed_;
        }

        // Atomically adds to the found count and returns the new total.
        size_t add_seeds_found(size_t seeds) {
            return total_seeds_found_.fetch_add(seeds) + seeds;
        }

        // Atomically adds to the processed count and returns the new total.
        size_t add_seeds_processed(size_t seeds) {
            return seeds_processed_.fetch_add(seeds) + seeds;
        }

        // Prints progress if a new 10% milestone has been reached.
        void maybe_print_progress() {
            size_t processed = seeds_processed_.load();
            size_t current_percentage = (processed * 100) / total_;
            current_percentage = (current_percentage / 10) * 10;

            if (current_percentage > last_percentage_ && current_percentage <= 100) {
                std::lock_guard lock(print_mutex_);
                if (current_percentage > last_percentage_) {
                    last_percentage_ = current_percentage;
                    std::println("{}% - {} seeds found", current_percentage, total_seeds_found_.load());
                }
            }
        }

    private:
        std::atomic<size_t> total_seeds_found_;
        std::atomic<size_t> seeds_processed_;
        std::mutex print_mutex_;

        const time_t total_;
        std::atomic<size_t> last_percentage_;
};