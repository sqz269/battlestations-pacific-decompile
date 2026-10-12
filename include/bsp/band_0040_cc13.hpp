#pragma once

// Non-gameplay helpers from 00410000-0044FFFF reconstructed by cc13-sweep40:
// the download manager's package-record virtuals (vtable 00D1D7D8), the
// fraction-zero trimmer, the upper-case Lua-style string hash, the
// time-of-day setter, the eight-neighbour grid step and byte-grid sample, and
// the quadratic solver. Names are hypotheses, not recovered symbols. Evidence,
// ABIs and open questions are in docs/CC13_BAND_0040_0044.md. None is a drop-in
// binary replacement: these are C++ interfaces over models of the native data.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast and unspilled intermediates are doubles.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bsp::cc13_band0040 {

// One 18h-byte package record of the download manager (object at [00F8A304],
// 844h bytes, built by 009955F0). The records live at manager+4, count +8.
//   +0  name length, +4 name data (a null data pointer is modelled as nullopt)
//   +10h enabled byte, +11h flag (meaning unknown), +12h blocked byte
// +8h..+0Fh are not touched by the routines below.
struct DownloadPackageRecord {
    std::optional<std::string> name;
    std::uint8_t enabled_10 = 0;
    std::uint8_t flag_11 = 0;
    std::uint8_t blocked_12 = 0;
};

// A counted name as the routines receive it ({length, data}); data may be null.
struct PackageName {
    std::optional<std::string> data;
    std::int32_t length() const { return data ? static_cast<std::int32_t>(data->size()) : 0; }
};

class DownloadManagerPackages {
public:
    std::vector<DownloadPackageRecord> records;   // manager+4 / +8

    // 0043E700, ECX manager, stack name*, RET 4, returns AL. No static callers.
    // First record whose name equals the query (equal lengths, then _stricmp;
    // two empty names match) -> its +11h byte; no match -> 0.
    std::uint8_t flag11_by_name_0043e700(const PackageName& name) const;

    // 0043E780, ECX manager, stack name*, RET 4, returns AL. Same search,
    // returns the +12h (blocked) byte.
    std::uint8_t blocked_by_name_0043e780(const PackageName& name) const;

    // 0043E800, vslot 3 (+0Ch), ECX manager, stack (index, bool enable), RET 8.
    // record[index].enabled = enable && record[index].blocked == 0.
    // The index is not range-checked by the image; this model asserts it.
    void set_enabled_at_0043e800(std::int32_t index, bool enable);

    // 0043E840, vslot 2 (+8), ECX manager, stack (name*, bool enable), RET 8.
    // Finds the first record that matches the name AND is not blocked (a
    // matching blocked record is skipped) and sets enabled = enable. No match:
    // nothing changes.
    void set_enabled_by_name_0043e840(const PackageName& name, bool enable);

    // 0043E8F0, vslot 4 (+10h), ECX manager, stack names*, RET 4. Clears every
    // enabled byte, then calls vslot 2 (name, true) for each listed name.
    void enable_only_0043e8f0(const std::vector<PackageName>& names);

    // 0043E940, vslot 5 (+14h), ECX manager, stack names*, RET 4. Sets every
    // enabled byte to (blocked == 0), then calls vslot 2 (name, false) for each
    // listed name. profile_commit.cpp's disable_listed_packages_virtual_14 is
    // this slot, run on settings+98h: the settings list names the packages to
    // switch OFF.
    void disable_only_0043e940(const std::vector<PackageName>& names);

    // 0043EA80, ECX manager, stack names*, RET 4 (profile_commit.cpp's
    // keep_known_package_prefixes_0043ea80 on settings+98h). Keeps each name that
    // is a case-sensitive prefix of some record name (a record with null data
    // never matches; a null list entry reads as the empty string 00E17BF0, which
    // matches any record with data). A name with no match is replaced by the
    // last name (assign + pop_back, unordered erase) and re-examined.
    void filter_known_names_0043ea80(std::vector<PackageName>& names) const;
};

// 00438E80, __fastcall(ECX char* text), plain RET, no static callers.
// For every '.', looks for a ' ' after it. With no space, deletes '0'
// characters from the end of the text back toward the '.' (exclusive). With a
// space the scan starts on the space itself, so nothing is trimmed (the image's
// behaviour, kept as is). Finally, if the text after the last '.' is empty or
// ends in '.', that final '.' is removed. Example: "1.500" -> "1.5",
// "2.000" -> "2." -> "2", "1.50 2.30" -> "1.50 2.3".
// The image reads text[-1] when the text has no '.' and is empty; the model
// treats that byte as not '.'.
void trim_trailing_fraction_zeros_00438e80(std::string& text);

// 0043B700, __fastcall(ECX NativeString*), plain RET, returns EAX.
// h = length; step = (length | 20h) >> 5; for (left = length; left >= step;
// left -= step) h ^= toupper(*p++) + (h << 5) + (h >> 2). A null data pointer
// reads the static empty string 00E177E4. toupper is the CRT's on a sign-
// extended char, low byte kept; in the "C" locale that changes only 'a'..'z'.
std::uint32_t hash_upper_case_0043b700(std::uint32_t length, const char* data);

// 004439B0, __thiscall(ECX object, int hours, int minutes, int seconds,
// int milliseconds), RET 10h. Stores the float hour value at object+4:
// float(((minutes/60.0 + hours) + seconds/3600.0) + ms/3600000.0), then adds
// 24.0 while the value is below 0 and subtracts 24.0 while it is above 24.0f.
// Returns the stored value. Only caller 008B1CC0 (Lua integers).
float time_of_day_hours_004439b0(std::int32_t hours, std::int32_t minutes,
    std::int32_t seconds, std::int32_t milliseconds);

// 00412010, __fastcall(ECX int* x, EDX int* y, stack direction), RET 4.
// Directions 0..7 clockwise from (0,-1): 0 (0,-1), 1 (+1,-1), 2 (+1,0),
// 3 (+1,+1), 4 (0,+1), 5 (-1,+1), 6 (-1,0), 7 (-1,-1); other values do nothing.
void grid_step_direction8_00412010(std::int32_t& x, std::int32_t& y, std::uint32_t direction);

// 00412070, __fastcall(ECX const uint8_t* const* columns, EDX width, stack
// height, x, y, direction, default byte), RET 14h, returns AL. Steps (x, y)
// with 00412010 and returns columns[x][y] when 0 <= x < width and
// 0 <= y < height, else the default byte. Callers 00880080, 00880420.
std::uint8_t byte_grid_sample_neighbor_00412070(const std::uint8_t* const* columns,
    std::int32_t width, std::int32_t height, std::int32_t x, std::int32_t y,
    std::uint32_t direction, std::uint8_t fallback);

// 00439BD0, __fastcall(ECX float* root1, EDX float* root2, stack a, b, c),
// RET 0Ch, returns the discriminant in ST0. disc = float(b*b - (a*4.0)*c);
// when disc >= 0: r = float(sqrt(disc)), *root1 = float((r - b) / (2a)),
// *root2 = float((-b - r) / (2a)). The roots are untouched when disc < 0.
float solve_quadratic_00439bd0(float a, float b, float c, float& root1, float& root2);

} // namespace bsp::cc13_band0040
