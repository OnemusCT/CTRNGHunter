#pragma once

#include <vector>
#include <unordered_map>

class MSVCRandWrapper {
  public:
	MSVCRandWrapper();

	void srand(time_t seed);

	int rand(int n = 1);

	void unrand();
  private:
	time_t seed_;

	std::unordered_map<int, uint32_t> a_;
	std::unordered_map<int, uint32_t> c_;
};

