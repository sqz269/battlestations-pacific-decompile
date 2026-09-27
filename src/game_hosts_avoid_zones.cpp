// Packet cc9_avoid_zone_registry, docs/AVOID_ZONE_REGISTRY.md. Names are
// hypotheses. See the header for the ABI of each routine.
#include "bsp/game_hosts_avoid_zones.hpp"

#include <cmath>

namespace bsp::game {

namespace {

// 00424680 with [00CE3990] (0.17453286, ten degrees): 00423960(10) sets the
// half extent [00CE3968] 12000.0, the cell 24000.0 [00CE3960] / 10, the scale
// [00CE3958] 2.0 and a zeroed 10 * 10 grid; +14h is tan(angle) through 00412E20.
bsp::TerrainGridLayerRecord default_layer_00424680() {
    bsp::TerrainGridLayerRecord layer;
    layer.class_name = "TerrainGridLayer";
    layer.dimension = 10;
    layer.half_extent = 12000.0f;
    layer.cell_size = static_cast<float>(24000.0 / 10.0);
    layer.file_scalar = 2.0f;
    layer.slope_limit = static_cast<float>(std::tan(static_cast<double>(0.17453286f)));
    layer.grid.assign(100, 0);
    return layer;
}

}  // namespace

AvoidZoneCell avoid_zone_world_to_cell_00417fa0(const bsp::TerrainGridLayerRecord& layer,
                                                float x, float z) noexcept {
    AvoidZoneCell c;
    // 00417FA0-00417FB1 / 00417FB3-00417FCA: FADD then FDIV in the x87, each
    // stored to a float.
    const float u = static_cast<float>((static_cast<double>(x) + layer.half_extent) /
                                       layer.cell_size);
    const float v = static_cast<float>((static_cast<double>(z) + layer.half_extent) /
                                       layer.cell_size);
    // 00417FCD-00417FDA: (float)(n - 2.0).
    const float hi = static_cast<float>(static_cast<double>(layer.dimension) - 2.0);
    // 00417FBE COMISS 0.0 > u -> 0, else u > hi -> hi (00417FED).
    c.u = (0.0f > u) ? 0.0f : ((u > hi) ? hi : u);
    c.v = (0.0f > v) ? 0.0f : ((v > hi) ? hi : v);
    return c;
}

float avoid_zone_cell_height_0041bae0(const bsp::TerrainGridLayerRecord& layer,
                                      int ix, int iz) noexcept {
    const int last = layer.dimension - 1;
    // 0041BAE4-0041BAFA and 0041BB00-0041BB0C: < 0 -> 0, > n - 1 -> n - 1.
    if (ix < 0) ix = 0; else if (ix > last) ix = last;
    if (iz < 0) iz = 0; else if (iz > last) iz = last;
    const long long index = static_cast<long long>(layer.dimension) * iz + ix;  // 0041BB0E
    if (index < 0 || index >= static_cast<long long>(layer.grid.size())) return 0.0f;
    // 0041BB2D MOVZX, FILD, FMUL [layer+10h], FSTP float.
    return static_cast<float>(static_cast<double>(layer.grid[static_cast<std::size_t>(index)]) *
                              layer.file_scalar);
}

float avoid_zone_sample_0041bc20(const bsp::TerrainGridLayerRecord& layer,
                                 float x, float z) noexcept {
    const AvoidZoneCell c = avoid_zone_world_to_cell_00417fa0(layer, x, z);
    // 0041BC49 / 0041BC4F CVTTSS2SI: truncation.
    const int ix = static_cast<int>(c.u);
    const int iz = static_cast<int>(c.v);
    // 0041BC59 FISUB then FSTP float: the u fraction.
    const float fu = static_cast<float>(static_cast<double>(c.u) - ix);
    const float h11 = avoid_zone_cell_height_0041bae0(layer, ix + 1, iz + 1);  // 0041BC6D
    const float h01 = avoid_zone_cell_height_0041bae0(layer, ix, iz + 1);      // 0041BC7A
    const float h10 = avoid_zone_cell_height_0041bae0(layer, ix + 1, iz);      // 0041BC8A
    const float h00 = avoid_zone_cell_height_0041bae0(layer, ix, iz);          // 0041BC97
    // 0041BCA0-0041BCBB: a = h00 + fu * (h10 - h00), stored as a float.
    const float a = static_cast<float>(static_cast<double>(h00) +
        static_cast<double>(fu) * (static_cast<double>(h10) - h00));
    // 0041BCBF-0041BCD1: b = h01 + fu * (h11 - h01), stored as a float.
    const float b = static_cast<float>(static_cast<double>(h01) +
        static_cast<double>(fu) * (static_cast<double>(h11) - h01));
    // 0041BCE1-0041BCED: the v fraction, FISUB then FSTP float.
    const float fv = static_cast<float>(static_cast<double>(c.v) - iz);
    // 0041BCF1-0041BCF9: a + fv * (b - a), stored as a float.
    return static_cast<float>(static_cast<double>(a) +
        static_cast<double>(fv) * (static_cast<double>(b) - a));
}

GameAvoidZoneRegistry::GameAvoidZoneRegistry() {
    layers_.push_back(default_layer_00424680());  // 0042476E-004247B7
}

GameAvoidZoneRegistry& GameAvoidZoneRegistry::instance() {
    static GameAvoidZoneRegistry registry;  // 004C17D0, [00E17620]
    return registry;
}

void GameAvoidZoneRegistry::load_scene_layers(
    const std::vector<bsp::TerrainGridLayerRecord>& file_layers) {
    layers_.clear();
    layers_.push_back(default_layer_00424680());
    for (const bsp::TerrainGridLayerRecord& layer : file_layers) layers_.push_back(layer);
    ++scene_loads_;
}

int GameAvoidZoneRegistry::select_layer_by_slope_0041df40(float value,
                                                          bool value_is_angle) const noexcept {
    return bsp::select_terrain_grid_layer(layers_, value, value_is_angle);
}

const bsp::TerrainGridLayerRecord* GameAvoidZoneRegistry::layer(int index) const noexcept {
    if (index < 0 || static_cast<std::size_t>(index) >= layers_.size()) return nullptr;
    return &layers_[static_cast<std::size_t>(index)];
}

int GameAvoidZoneRegistry::squadron_layer_34c() const noexcept {
    return select_layer_by_slope_0041df40(1.5f, true);  // 007F1DB2-007F1DC5, [00CE380C]; stored at 007F1DCA
}

}  // namespace bsp::game
