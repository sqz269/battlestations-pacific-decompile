#pragma once

#include "bsp/scene_file.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace bsp {
// The native game stores NW at +711Ch and SE at +7128h. Z is north-positive:
// NW.z is the maximum and SE.z is the minimum. This is a value projection,
// not an overlay on GGame. Descriptive names are hypotheses.
struct WorldMapBounds {
    std::array<float, 3> north_west;
    std::array<float, 3> south_east;
    std::array<float, 3> clip_minimum() const noexcept;
    std::array<float, 3> clip_maximum() const noexcept;
};

// Modes 0..7: IslandCapture1v1/2v2/3v3/4v4, Duel, Escort, Siege, Competitive.
// Values originate in Map.MultiPlayMapSizes, not defaults fabricated here.
struct WorldMapSettings {
    float border_size_x;
    float border_size_y;
    std::array<WorldMapBounds, 8> multiplayer;
};

// Data-read portion of 004E6C00, through the two global border-size stores.
// Takes the already parsed Map block. Uses the existing property value decoder.
// A missing MultiPlayMapSizes block returns false and leaves output unchanged,
// matching the native early return before its global stores. Other missing or
// wrongly typed required values are invalid native inputs and throw here.
// Does not claim the 004D5BD0 border-object rebuild or twelve 004C7150 calls.
bool read_world_map_settings_004e6c00(const ScenePropertyBlock& map,
    WorldMapSettings& output);

// Complete bound-selection fragment 004D5EDE..004D61BF. The preceding copy
// of source globals into the game table is represented by settings.multiplayer.
// Border-object cleanup/allocation and publication before/after this fragment
// remain outside this data projection. Modes8/9 and out-of-range modes use the
// size-derived default even when forced/session flags enter the switch gate.
WorldMapBounds select_world_map_bounds_004d5ede(const WorldMapSettings&,
    std::int32_t mode_614, std::uint8_t forced_61c, std::int32_t session_1fe4) noexcept;

// Complete 0071C4F0 predicate (native ECX game, stack XYZ pointer, EAX0/1, RET4).
// Four x87 JA rejection tests: edges inclusive; NaN does not reject by itself.
// Y is ignored. Input must remain stable; point/field aliasing and native ABI
// replacement are not claimed by this new C++ interface.
bool point_outside_world_map_0071c4f0(const WorldMapBounds&,
    const std::array<float, 3>& point) noexcept;
// Packet cc9_get_closest_border_zone (docs/WORLD_MAP_BOUNDS.md, "Border zones").
// One 44h record operator_new'd by 004D5BD0 (004D61C4..004D6240) into the list
// world+7134h+edge*0Ch, laid out by 004C71C0 and given its side by one of the
// twelve 004C7150 calls at the end of 004E6C00. Float offsets: +0 length, +4
// side (2 at construction), +8 edge, +Ch index in the list, +10h..+3Ch the
// corners A, B, C and D, +40h the running offset along the edge. The Y
// components +14h, +20h, +2Ch and +38h are never written (not modelled: 0).
struct BorderZoneRecord {
    float length{0.0f};
    std::int32_t side{2};
    std::int32_t edge{0};
    std::int32_t index{0};
    std::array<float, 3> a{};
    std::array<float, 3> b{};
    std::array<float, 3> c{};
    std::array<float, 3> d{};
    float offset{0.0f};
};

// The four lists at world+7134h: edge 0 north (NW.z), 1 east (SE.x), 2 south
// (SE.z), 3 west (NW.x), three records each.
struct BorderZoneSet {
    std::array<std::vector<BorderZoneRecord>, 4> edges;
};

// 004D5BD0's rebuild tail (three records per edge, length 004B6C40(edge) / 3.0
// from 00D7A2B0), 004C71C0's layout, then the twelve constant 004C7150 calls of
// 004E6C00 (004E7142..004E7209): sides 0,1,0 on edges 0 and 2, 1,0,1 on edges 1
// and 3. 004CA930, the merge of adjacent same-side records, has no reference in
// the image and is not applied. SUBSTITUTION, labelled: the x87 sums are taken
// in double and rounded to float at each of the listing's FSTP stores.
BorderZoneSet build_border_zones_004d5bd0(const WorldMapBounds& bounds);

// 004C7730(position, side, point_out, direction_out): the nearest record of
// `side` (any when negative) by the x/z distance from `position` to the record's
// B..C edge, clamped. Returns null when no record matches; for sides 0 and 1
// there is no retry, for any other side the search repeats with -1.
struct BorderZoneHit {
    const BorderZoneRecord* zone{nullptr};
    std::array<float, 3> point{};      // (clamped x, position.y, clamped z)
    std::array<float, 3> direction{};  // (+-1 or 0, 0, +-1 or 0), outward
};
BorderZoneHit closest_border_zone_004c7730(const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side);

// 008AECD0 GetClosestBorderZone(position[, offset]) after its argument reads:
// 004C7730 with side -1 (EDI = -1, 008AED36), then, when offset > 0.0 and the
// position is inside the map (0071C4F0) and farther than 1 from the edge point,
// the edge point moved `offset` further along (point - position); otherwise the
// edge point moved `offset` along the zone's outward direction. `found` is false
// when 004C7730 returned null, whose outputs the image leaves unwritten.
std::array<float, 3> get_closest_border_zone_008aecd0(const WorldMapBounds& bounds,
    const BorderZoneSet& zones, const std::array<float, 3>& position, float offset,
    bool& found);

} // namespace bsp
