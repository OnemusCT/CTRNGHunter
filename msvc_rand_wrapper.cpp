#include "msvc_rand_wrapper.h"

#include <utility>

// MSVC LCG parameters: seed' = seed * a + c (mod 2^32)
constexpr uint32_t kBaseA = 214013;
constexpr uint32_t kBaseC = 2531011;

// Computes the combined LCG parameters (a, c) for jumping `n` steps at once
// using exponentiation by squaring. After computing, seed' = seed * a + c gives
// the same result as applying the base LCG n times.
std::pair<uint32_t, uint32_t> calcLCGParams(int n) {
    uint32_t cur_a = kBaseA;
    uint32_t cur_c = kBaseC;
    uint32_t final_a = 1;
    uint32_t final_c = 0;

    while (n > 0) {
        if (n & 1) {
            final_a *= cur_a;
            final_c = final_c * cur_a + cur_c;
        }
        // Double the jump distance: (a, c) -> (a^2, ac + c)
        cur_c = cur_c * (cur_a + 1);
        cur_a *= cur_a;
        n >>= 1;
    }

    return {final_a, final_c};
}

MSVCRandWrapper::MSVCRandWrapper() : seed_(0) {
    // Precompute up to 20 room transitions
    for (int i = 2; i <= 20; i++) {
        lcg_params_.emplace(i, calcLCGParams(i));
    }
}

void MSVCRandWrapper::srand(uint32_t seed) {
	seed_ = seed;
}

int MSVCRandWrapper::rand(int n) {
    if (n == 1) {
	    seed_ = (seed_ * kBaseA) + kBaseC;
	    return static_cast<int>((seed_ >> 16) & 0x7FFF);
    }
    auto it = lcg_params_.find(n);
    if (it == lcg_params_.end()) {
        it = lcg_params_.emplace(n, calcLCGParams(n)).first;
    }
    auto [a, c] = it->second;
    seed_ = (seed_ * a) + c;
    return static_cast<int>((seed_ >> 16) & 0x7FFF);
}

// Reverses one LCG step. The modular inverse of 214013 (mod 2^32) is 3115528533.
void MSVCRandWrapper::unrand() {
	seed_ = (seed_ - 2531011U) * 3115528533U;
}