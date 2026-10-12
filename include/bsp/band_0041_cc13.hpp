#pragma once

// Bands 0041-0047, 004A-0054, 005F-006A and 009E (cc13-gp41, docs/CC13_GP_0041_009E.md).
// New C++ interfaces, not the native register ABI. Names are hypotheses, not recovered symbols.

#include <string_view>

namespace bsp::band_0041_cc13 {

// Inputs of 0041D380 BSP_AvoidZone_NameAppliesToLayer (__stdcall(entity, float* layer, int id),
// RET 0Ch, AL result). The host resolves the entity part: `entity_present` is entity != 0,
// `id_matches` is "id == 0 or 00923810(entity, 1) == id", `has_name` is entity+154h != 0 and
// `name` the string at entity+158h ("" when that pointer is null).
struct AvoidZoneNameQuery {
    bool entity_present{};
    bool id_matches{};
    bool has_name{};
    std::string_view name;
    float layer{};
};

// 0041D380. A name beginning "AvoidZone" applies to every layer, except that a name beginning
// "AvoidZoneG" applies only when sscanf("AvoidZoneG %s %d #%03d") reads a word whose first three
// characters are "all" and a group number equal to the layer truncated to int (CVTTSS2SI at
// 0041D3B9). Any other name does not apply. Native differences this does not model: the word is
// scanned into an 8-byte stack buffer (a longer word overruns it), and when no number is read the
// native compares the layer with the entity pointer value left in the same stack slot; this returns
// false for that case.
bool avoid_zone_name_applies_to_layer_0041d380(const AvoidZoneNameQuery& query);

}  // namespace bsp::band_0041_cc13
