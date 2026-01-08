#include "msvc_rand_wrapper.h"

#include <utility>

constexpr uint32_t kBaseA = 214013;
constexpr uint32_t kBaseC = 2531011;

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

MSVCRandWrapper::MSVCRandWrapper() {
    // Add precomputed values
	a_[33] = 912984189u;
	c_[33] = 2202588003u;
	a_[66] = 3569749257u;
	c_[66] = 550773434u;

}

void MSVCRandWrapper::srand(time_t seed) {
	seed_ = seed;
}

int MSVCRandWrapper::rand(int n) {
    if (n == 1) {
	    seed_ = (seed_ * kBaseA) + kBaseC;
	    return static_cast<int>((seed_ >> 16) & 0x7FFF);
    }
    uint32_t a;
    uint32_t c;
    auto it = a_.find(n);
    if (it == a_.end()) {
        std::tie(a,c) = calcLCGParams(n);
        a_.insert({n, a});
        c_.insert({n, c});
    }
    else {
        a = it->second;
        c = c_.find(n)->second;
    }
    seed_ = (seed_ * a) + c;
    return static_cast<int>((seed_ >> 16) & 0x7FFF);
}

void MSVCRandWrapper::unrand() {
	seed_ = (seed_ - 2531011U) * 3115528533U;
}

