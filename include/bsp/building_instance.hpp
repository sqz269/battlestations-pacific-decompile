#pragma once
#include "bsp/camera_projection.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace bsp {
inline constexpr std::size_t building_instance_float4_count = 9;
inline constexpr std::size_t building_instance_stride = 144;
// Exact declaration name supplied by constructor00b450d0. The renderer resolves
// it and combines it after the selected section's first stream declaration.
inline constexpr char building_instance_vertex_format[] =
    "uf44uf44uf44uf44uf44uf44uf44uf44uf44.mvfm";
using BuildingInstanceData = std::array<float, 36>;

// Ordered point-light list values from transform+164. Native source fields
// +1EC..+1F8 and +184..+190 are copied without position/radius conversion.
struct BuildingInstancePointLight {
    std::array<float, 4> position_radius{};
    std::array<float, 4> color{};
};

// Pure value projection of00b55780. Original virtual-call slot+8 receives a
// generator in ECX (unused in this body), stack render entry and144-byte output,
// RET8. Caller supplies the already refreshed world matrix: native entry+0C
// transform+F0;00b6db70 is called natively when transform valid bit2 is clear.
// CameraMatrix preserves that established 16-float storage order.
//
// Nine float4s: three transposed affine world rows, three ordered light position/
// radius records, then three light colors. Copies at most the first three lights
// and zeros absent records. Last writes replace color0.w with entry visibility,
// color1.w with the clamped light count asfloat, color2.w with material diffuse
// alpha (native entry+4 section+20 ->00b179f0(0)+0C).
//
// Valid source-list counts are0..INT_MAX; larger host vectors are rejected
// unchanged. Native corrupt negative DWORD counts have a different unsigned
// branch/conversion behavior, outside this valid-container interface. No native
// object lifetime or ABI projection; x87 NaN conversion/FP state is not modeled.
// Output is staged, including when its bytes alias an input through the caller.
// Evidence: docs/BUILDING_INSTANCE_DATA.md.
bool write_building_instance_data_00b55780(const CameraMatrix& refreshed_world,
    const std::vector<BuildingInstancePointLight>& ordered_point_lights,
    float visibility, float material_diffuse_alpha,
    BuildingInstanceData& output, std::string& error);
}
