// Non-gameplay helpers from 00410000-0044FFFF, reconstructed by cc13-sweep40.
// Evidence and ABIs: docs/CC13_BAND_0040_0044.md.

#include "bsp/band_0040_cc13.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

namespace bsp::cc13_band0040 {

namespace {

constexpr double kMinutesPerHour = 60.0;           // 00CE3D68
constexpr double kSecondsPerHour = 3600.0;         // 00CE3D70
constexpr double kMillisecondsPerHour = 3600000.0; // 00CE4798
constexpr double kHoursPerDay = 24.0;              // 00CE4790 (double, add/subtract)
constexpr float kHoursPerDayLimit = 24.0f;         // 00CE4788 (float, compare)
constexpr double kQuadraticFour = 4.0;             // 00D7A328

int ascii_lower_compare(const std::string& a, const std::string& b) {
    // _stricmp (00BF7FBF) in the "C" locale.
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) {
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (x >= 'A' && x <= 'Z') x = static_cast<unsigned char>(x + 32);
        if (y >= 'A' && y <= 'Z') y = static_cast<unsigned char>(y + 32);
        if (x != y) return x < y ? -1 : 1;
    }
    return a.size() == b.size() ? 0 : (a.size() < b.size() ? -1 : 1);
}

// The record/name equality used by 0043E700, 0043E780 and 0043E840: lengths
// first; equal zero lengths match without reading data; otherwise _stricmp.
bool names_match(const PackageName& query, const DownloadPackageRecord& record) {
    const std::int32_t record_length = record.name ? static_cast<std::int32_t>(record.name->size()) : 0;
    if (query.length() != record_length) return false;
    if (record_length == 0) return true;
    return ascii_lower_compare(*query.data, *record.name) == 0;
}

} // namespace

std::uint8_t DownloadManagerPackages::flag11_by_name_0043e700(const PackageName& name) const {
    for (const DownloadPackageRecord& record : records)
        if (names_match(name, record)) return record.flag_11;
    return 0;
}

std::uint8_t DownloadManagerPackages::blocked_by_name_0043e780(const PackageName& name) const {
    for (const DownloadPackageRecord& record : records)
        if (names_match(name, record)) return record.blocked_12;
    return 0;
}

void DownloadManagerPackages::set_enabled_at_0043e800(std::int32_t index, bool enable) {
    assert(index >= 0 && static_cast<std::size_t>(index) < records.size());
    DownloadPackageRecord& record = records[static_cast<std::size_t>(index)];
    record.enabled_10 = (enable && record.blocked_12 == 0) ? 1 : 0;
}

void DownloadManagerPackages::set_enabled_by_name_0043e840(const PackageName& name, bool enable) {
    for (DownloadPackageRecord& record : records) {
        if (!names_match(name, record) || record.blocked_12 != 0) continue;
        // 0043E8B0 retests +12h, which the search already required to be zero.
        record.enabled_10 = (enable && record.blocked_12 == 0) ? 1 : 0;
        return;
    }
}

void DownloadManagerPackages::enable_only_0043e8f0(const std::vector<PackageName>& names) {
    for (DownloadPackageRecord& record : records) record.enabled_10 = 0;
    // The image dispatches through vslot 2; 0043E840 is the only known target.
    for (const PackageName& name : names) set_enabled_by_name_0043e840(name, true);
}

void DownloadManagerPackages::disable_only_0043e940(const std::vector<PackageName>& names) {
    for (DownloadPackageRecord& record : records) record.enabled_10 = record.blocked_12 == 0 ? 1 : 0;
    for (const PackageName& name : names) set_enabled_by_name_0043e840(name, false);
}

void DownloadManagerPackages::filter_known_names_0043ea80(std::vector<PackageName>& names) const {
    std::size_t index = 0;
    while (index < names.size()) {
        const std::string query = names[index].data ? *names[index].data : std::string();
        bool known = false;
        for (const DownloadPackageRecord& record : records) {
            if (!record.name) continue;                    // 0043EAD0: null record data
            const std::string& full = *record.name;
            // 0043EAE0 loop: advance while both characters are non-zero and equal;
            // the name matches when it is exhausted (prefix, case-sensitive).
            std::size_t i = 0;
            while (i < full.size() && i < query.size() && full[i] == query[i]) ++i;
            if (i == query.size()) { known = true; break; }
        }
        if (known) { ++index; continue; }
        // 0043EB13..0043EB7A: assign the last name over this one unless it is the
        // last, release the last name's storage, --count; re-examine this slot.
        if (index != names.size() - 1) names[index] = names.back();
        names.pop_back();
    }
}

void trim_trailing_fraction_zeros_00438e80(std::string& text) {
    std::size_t base = 0;   // ESI at 00438F10
    std::size_t dot = text.find('.');
    while (dot != std::string::npos) {
        const std::size_t space = text.find(' ', dot);
        if (space == std::string::npos) {
            // 00438EA7: q = last character, memmove of one byte (the terminator).
            std::size_t q = text.size() - 1;
            while (q > dot && text[q] == '0') {
                text.resize(q);
                --q;
            }
        }
        // With a space, q starts on the space and the '0' test fails at once.
        base = dot + 1;
        dot = text.find('.', base);
    }
    // 00438F10: tests the byte before the end of text+base. base never passes the
    // end, so when the tail is empty that byte is the last '.' itself. For an
    // empty text with no '.', the image reads text[-1]; modelled as "not '.'".
    const std::size_t end = text.size();
    if (end > 0 && text[end - 1] == '.') text.resize(end - 1);
}

std::uint32_t hash_upper_case_0043b700(std::uint32_t length, const char* data) {
    static const char kEmpty[4] = {};   // 00E177E4
    const char* p = data ? data : kEmpty;
    const std::uint32_t step = (length | 0x20u) >> 5;
    std::uint32_t h = length;
    for (std::uint32_t left = length; left >= step; left -= step) {
        unsigned char c = static_cast<unsigned char>(*p++);
        if (c >= 'a' && c <= 'z') c = static_cast<unsigned char>(c - 32);   // CRT toupper, "C" locale
        h ^= static_cast<std::uint32_t>(c) + (h << 5) + (h >> 2);
    }
    return h;
}

float time_of_day_hours_004439b0(std::int32_t hours, std::int32_t minutes,
    std::int32_t seconds, std::int32_t milliseconds) {
    // cc13-fixture5: Lua's SetDayTime runs on the main thread at 24-bit precision, so
    // each FDIV / FIADD / FADDP rounds to float (the double sum differed from the
    // bytes on 1362 of 4000 inputs).
    auto to_float = [](double v) { return static_cast<float>(v); };
    const float m = to_float(static_cast<double>(minutes) / kMinutesPerHour);       // 004439B4
    const float mh = to_float(static_cast<double>(m) + static_cast<double>(hours)); // 004439BA FIADD
    const float s = to_float(static_cast<double>(seconds) / kSecondsPerHour);       // 004439C2
    const float ms = to_float(static_cast<double>(milliseconds) / kMillisecondsPerHour);  // 004439CE
    float value = (mh + s) + ms;                           // 004439C8, 004439D4; FSTP float [ECX+4]
    if (0.0f > value) {
        do value = static_cast<float>(static_cast<double>(value) + kHoursPerDay);
        while (0.0f > value);                              // COMISS 0 > value
    }
    while (value > kHoursPerDayLimit)
        value = static_cast<float>(static_cast<double>(value) - kHoursPerDay);
    return value;
}

void grid_step_direction8_00412010(std::int32_t& x, std::int32_t& y, std::uint32_t direction) {
    switch (direction) {
    case 0: --y; break;
    case 1: ++x; --y; break;
    case 2: ++x; break;
    case 3: ++x; ++y; break;
    case 4: ++y; break;
    case 5: --x; ++y; break;
    case 6: --x; break;
    case 7: --x; --y; break;
    default: break;
    }
}

std::uint8_t byte_grid_sample_neighbor_00412070(const std::uint8_t* const* columns,
    std::int32_t width, std::int32_t height, std::int32_t x, std::int32_t y,
    std::uint32_t direction, std::uint8_t fallback) {
    grid_step_direction8_00412010(x, y, direction);
    if (x < 0 || y < 0 || x >= width || y >= height) return fallback;
    return columns[x][y];
}

float solve_quadratic_00439bd0(float a, float b, float c, float& root1, float& root2) {
    const float disc = static_cast<float>(static_cast<double>(b) * b
        - static_cast<double>(a) * kQuadraticFour * c);    // FSTP float [ESP+18h]
    if (disc < 0.0f) return disc;                         // JC 00439C34
    const float r = static_cast<float>(std::sqrt(static_cast<double>(disc)));   // 00BF7030, FSTP float
    const double two_a = static_cast<double>(a) + a;
    root1 = static_cast<float>((static_cast<double>(r) - b) / two_a);
    root2 = static_cast<float>((-static_cast<double>(b) - r) / two_a);
    return disc;
}

} // namespace bsp::cc13_band0040
