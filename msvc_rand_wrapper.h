#pragma once

#include <vector>

class MSVCRandWrapper {
  public:
	MSVCRandWrapper() = default;

	void srand(time_t seed);

	int rand();

	void unrand();

  private:
	time_t seed_;
};

