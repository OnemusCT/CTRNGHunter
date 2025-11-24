#pragma once

#include <time.h>
#include <string>
#include <chrono>
#include <format>

std::string seed_to_date(time_t seed) {
    auto time_point = std::chrono::system_clock::from_time_t(seed);
    return std::format("ManipController.TimeZones.ET, {0:%d, %m, %Y, %H, %M, %S}", time_point);
}