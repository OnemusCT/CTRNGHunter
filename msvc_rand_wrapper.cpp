#include "msvc_rand_wrapper.h"

void MSVCRandWrapper::srand(unsigned int seed) {
	seed_ = seed;
}

int MSVCRandWrapper::rand() {
	seed_ = (seed_ * 214013) + 2531011;
	return (int)((seed_ >> 16) & 0x7FFF);
}

std::vector<int> MSVCRandWrapper::PopulateRandTable(unsigned int seed, int size) {
	unsigned int curr_seed = seed_;

	srand(seed);
	std::vector<int> result(size);

	for (int i = 0; i < size; i++) {
		result[i] = rand();
	}

	seed_ = curr_seed;
	return result;
}