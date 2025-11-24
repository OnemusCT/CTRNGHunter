#pragma once

#include <vector>
#include <functional>
#include <string>
#include <time.h>

class RNGHunter {
  public:
    explicit RNGHunter(int max_seeds) : max_seeds_(max_seeds) {}

    bool parseFile(const std::string& filename);
    void logSeed(time_t seed);

    void clear();

    std::vector<time_t> findSeeds(time_t start, time_t end);

  private:
    int max_seeds_;
    std::vector<std::function<bool(bool)>> functions_;
};
