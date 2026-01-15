#pragma once

#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <format>
#include <iostream>

// Semantic type for roll results
struct RollResult {
    int seed;
    int value;

    RollResult() = default;
    RollResult(int s, int v) : seed(s), value(v) {}
    RollResult(const RollResult&) = default;
    RollResult& operator=(const RollResult&) = default;
    RollResult(RollResult&&) noexcept = default;
    RollResult& operator=(RollResult&&) noexcept = default;
    ~RollResult() = default;


    // Format as "seed (value)" in hex
    std::string to_string() const {
        return std::format("0x{:02X} (0x{:02X})", seed, value);
    }

    friend std::ostream& operator<<(std::ostream& os, const RollResult& result) {
        return os << result.to_string();
    }
};

class RNGTable {
public:
    RNGTable() : seed_(0) {}
    RNGTable(int seed) : seed_(seed) {}

    void seed(int i);
    int get_seed() const;

    int roll(int limit = 0x100);
    std::pair<int, int> roll_with_seed(int limit = 0x100);

    RollResult roll_result(int limit = 0x100);
    std::vector<RollResult> roll_multiple(int count, int limit = 0x100);
    RollResult roll_until(std::function<bool(int)> condition, int limit = 0x100);
    int peek(int limit = 0x100) const;

private:
    int seed_;
};

int rng_table(int i);
bool crit_table(int i, int threshold);