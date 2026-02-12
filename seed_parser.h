#pragma once

#include <string>

// Parses a CTManip-format timestamp string (e.g. "TimeZones.UTC, 01, 06, 2025, 12, 30, 00")
// into a Unix timestamp (time_t) seed value, adjusting for the embedded timezone offset.
time_t string_to_seed(const std::string& in);

// Converts a Unix timestamp seed back to a CTManip-format timestamp string in UTC.
std::string seed_to_string(time_t in);
