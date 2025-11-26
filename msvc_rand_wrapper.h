#pragma once

#include <vector>

class MSVCRandWrapper {
  public:
	MSVCRandWrapper() = default;

	void srand(unsigned int seed);

	int rand();

	// Generates |size| random values based on |seed| and returns them.
	// The existing seed is not modified.
	std::vector<int> PopulateRandTable(unsigned int seed, int size);

  private:
	unsigned int seed_;
};

