#include "msvc_rand_wrapper.h"

void MSVCRandWrapper::srand(unsigned int seed) {
	seed_ = seed;
}

int MSVCRandWrapper::rand() {
	seed_ = (seed_ * 214013) + 2531011;
	return (int)((seed_ >> 16) & 0x7FFF);
}