// cc13-gp41 (docs/CC13_GP_0041_009E.md). Evidence in include/bsp/band_0041_cc13.hpp.

#include "bsp/band_0041_cc13.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>

namespace bsp::band_0041_cc13 {
namespace {

bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

// sscanf(name, "AvoidZoneG %s %d #%03d", word, &group, ...): the literal, whitespace, a
// non-whitespace word, whitespace, then a decimal integer. Returns false when no integer is read.
bool scan_group(std::string_view name, std::string& word, long& group) {
    std::size_t i = 10;  // past "AvoidZoneG"
    auto skip_ws = [&] {
        while (i < name.size() && std::isspace(static_cast<unsigned char>(name[i]))) ++i;
    };
    skip_ws();
    const std::size_t w0 = i;
    while (i < name.size() && !std::isspace(static_cast<unsigned char>(name[i]))) ++i;
    if (i == w0) return false;
    word.assign(name.substr(w0, i - w0));
    skip_ws();
    const std::string rest(name.substr(i));
    char* end = nullptr;
    group = std::strtol(rest.c_str(), &end, 10);
    return end != rest.c_str();
}

}  // namespace

bool avoid_zone_name_applies_to_layer_0041d380(const AvoidZoneNameQuery& query) {
    if (!query.entity_present || !query.id_matches) return false;  // 0041D38A, 0041D3A7
    // 0041D3B9: CVTTSS2SI, truncation toward zero.
    const long layer = static_cast<long>(std::trunc(query.layer));
    const std::string_view name = query.has_name ? query.name : std::string_view("<null name>");
    if (!starts_with(name, "AvoidZone")) return false;   // _strncmp(name, "AvoidZone", 9)
    if (!starts_with(name, "AvoidZoneG")) return true;   // _strncmp(name, "AvoidZoneG", 10)
    std::string word;
    long group = 0;
    if (!scan_group(name, word, group)) return false;
    return group == layer && word.compare(0, 3, "all") == 0;  // _strncmp(word, "all", 3)
}

}  // namespace bsp::band_0041_cc13
