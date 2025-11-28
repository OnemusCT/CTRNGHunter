#pragma once

#include <vector>

class MSVCRandWrapper {
  public:
	MSVCRandWrapper() = default;

	void srand(unsigned int seed);

	int rand();

	void unrand();

  private:
	unsigned int seed_;
};

