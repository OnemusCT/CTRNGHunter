#include "seed_parser.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>

const std::map<std::string, time_t> tz_offsets = {
    {"TimeZones.ET",  -4 * 3600}, // UTC-4
    {"TimeZones.UTC", 0},
    {"TimeZones.JST", 9 * 3600},  // UTC+9
    {"TimeZones.GMT", 0},          // UTC
    {"TimeZones.CEST", 2 * 3600}  // UTC+2
};

char* strptime(const char* s,
    const char* f,
    tm* tm) {
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail()) {
        return nullptr;
    }
    return const_cast<char*>(s + input.tellg());
}

time_t string_to_seed(const std::string& in) {
    std::string tz = in.substr(0, in.find(','));
    std::string datetime = in.substr(in.find(' '));
    tm time_info = {};
    strptime(datetime.c_str(), "%d, %m, %Y, %H, %M, %S", &time_info);

    // Use _mkgmtime (Windows) or timegm (POSIX) to treat time as UTC
#ifdef _WIN32
    time_t out = _mkgmtime(&time_info);
#else
    time_t out = timegm(&time_info);
#endif

    if (out == -1) {
        strptime(datetime.c_str(), "%d, %m, %y, %H, %M, %S", &time_info);
#ifdef _WIN32
        out = _mkgmtime(&time_info);
#else
        out = timegm(&time_info);
#endif
    }
    out -= tz_offsets.find(tz)->second;
    return out;
}

std::string seed_to_string(time_t in) {
    char buff[40];
    tm time = {};
#ifdef _WIN32
    gmtime_s(&time, &in);
#else
    gmtime_r(&in, &time);
#endif
    strftime(buff, 40, "TimeZones.UTC, %d, %m, %Y, %H, %M, %S", &time);
    return {buff};
}