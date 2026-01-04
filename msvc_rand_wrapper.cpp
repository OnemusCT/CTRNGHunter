#include "msvc_rand_wrapper.h"

void MSVCRandWrapper::srand(time_t seed) {
	seed_ = seed;
}

int MSVCRandWrapper::rand() {
	seed_ = (seed_ * 214013) + 2531011;
	return static_cast<int>((seed_ >> 16) & 0x7FFF);
}

void MSVCRandWrapper::unrand() {
	seed_ = (seed_ - 2531011U) * 3115528533U;
}
