#pragma once

#include "bsp/scene_file.hpp"

#include <array>
#include <cstdint>

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
} // namespace bsp
