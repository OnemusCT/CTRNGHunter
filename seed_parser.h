#pragma once

#include <string>

time_t string_to_seed(const std::string& in);
std::string seed_to_string(time_t in);