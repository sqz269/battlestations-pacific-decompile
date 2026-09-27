#pragma once

// Packet cc9_avoid_zone_registry, docs/AVOID_ZONE_REGISTRY.md.
//
// The avoid-zone REGISTRY, [00E17620]: the process singleton
// BSP_AvoidZoneRegistry_GetSingleton 004C17D0 builds (operator new(14h),
// constructor 00424730), a std::list of TerrainGridLayer (28h bytes each).
// The constructor leaves one default layer in the list (00424680 with the ten
// degree [00CE3990]: n = 10, half extent 12000, cell 2400, scale 2.0, all-zero
// grid, 00423960); the scene load 004248A0 (called from 004D5251 when the
// scene's `.nav` opened) appends the file's layers without clearing.
//
// The avoid-zone MANAGER, [00E17624], is a different object; this host does
// not duplicate it. It is GameAvoidZoneRuntime in the ship AI host, and its
// consumer contract is written in docs/AVOID_ZONE_REGISTRY.md.
//
// Names are hypotheses, not recovered symbols.

#include "bsp/scene_contents_hosts.hpp"

#include <cstddef>
#include <vector>

namespace bsp::game {

// 00417FA0, __thiscall(layer, float2* out, float x, float z), RET 0Ch.
// u = (x + half) / cell, v = (z + half) / cell, each stored as a float and
// clamped to [0, n - 2.0] ([00D7A308] is the double 2.0).
struct AvoidZoneCell {
    float u = 0.0f;
    float v = 0.0f;
};
AvoidZoneCell avoid_zone_world_to_cell_00417fa0(const bsp::TerrainGridLayerRecord& layer,
                                                float x, float z) noexcept;

// 0041BAE0, __thiscall(layer, int ix, int iz), RET 8: both indices clamped to
// [0, n - 1], the byte at grid[iz * n + ix] times layer+10h (the per-file
// scale), stored as a float. Returns 0 for an empty grid (the image would
// call the CRT's invalid-parameter handler 00BF6713).
float avoid_zone_cell_height_0041bae0(const bsp::TerrainGridLayerRecord& layer,
                                      int ix, int iz) noexcept;

// 0041BC20, __thiscall(layer, float x, float z), RET 8: the bilinear sample of
// the four cells around (x, z), with the image's float roundings of the two
// fractions, the two row blends and the result.
float avoid_zone_sample_0041bc20(const bsp::TerrainGridLayerRecord& layer,
                                 float x, float z) noexcept;

class GameAvoidZoneRegistry final {
public:
    // 004C17D0. One per process, as the image's; it starts as the
    // constructor leaves it, with the default layer only.
    static GameAvoidZoneRegistry& instance();

    // 00424730 then 004248A0: the default layer followed by the scene file's
    // layers. The image appends a second scene's layers to the first's; this
    // host starts each load from the constructor's state instead (one mission
    // per process in every measured run). Labelled.
    void load_scene_layers(const std::vector<bsp::TerrainGridLayerRecord>& file_layers);

    // 0041DF40 over the list, through select_terrain_grid_layer.
    int select_layer_by_slope_0041df40(float value, bool value_is_angle) const noexcept;
    const bsp::TerrainGridLayerRecord* layer(int index) const noexcept;
    std::size_t layer_count() const noexcept { return layers_.size(); }
    std::size_t scene_loads() const noexcept { return scene_loads_; }

    // squadron+34Ch, written at 007F1DCA with 0041DF40(1.5 [00CE380C], true):
    // the same layer for every squadron, because neither argument is the
    // squadron's. -1 only if the list were empty, which the constructor rules out.
    int squadron_layer_34c() const noexcept;

private:
    GameAvoidZoneRegistry();
    std::vector<bsp::TerrainGridLayerRecord> layers_;
    std::size_t scene_loads_ = 0;
};

}  // namespace bsp::game
