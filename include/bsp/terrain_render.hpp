#pragma once
// Terrain and foliage render kernels from partition segment 81
// (00ADA420..00AF3750). Names are hypotheses, not recovered symbols; evidence
// is in docs/CC13_TERRAIN_RENDER.md. Not binary compatible: the native routines
// read scene nodes, locked vertex streams and quadtree nodes; these functions
// take the values they read and return what the native code stores. x87
// extended-precision intermediates are not reproduced bit for bit.
#include <cstdint>
#include <vector>

namespace bsp {

struct TerrainVec3f {
    float x{}, y{}, z{};
};

struct TerrainSphere {
    TerrainVec3f centre;
    float radius{};
};

// 00ADA600, __fastcall(ECX = out sphere, EDX = box {min xyz, max xyz}), RET:
// centre = (min + max) * 0.5, radius = |(max - min) * 0.5| (00419440). The
// half factor is the double 00D7A280.
TerrainSphere terrain_box_bounding_sphere_00ada600(const TerrainVec3f& min,
    const TerrainVec3f& max) noexcept;

// The foliage quadtree cell box as 00AE2FE0 reads it: x over [+00h, +08h],
// z over [+04h, +0Ch], y over [+40h, +44h], all offset by the owning node's
// world translation (+120h, +124h, +128h).
struct FoliageCellBox {
    float x0{}, z0{}, x1{}, z1{};
    float y0{}, y1{};
};

enum class FoliageCellSide : std::int32_t {
    Outside = 0,  // the vertex furthest along the normal is behind the plane
    Straddles = 1,
    Inside = 10,  // the nearest vertex is in front as well
};

// 00AE2FE0, __thiscall(cell, a, b, c, d, node*), RET 14h: the p/n-vertex
// test of the cell box against the plane a*x + b*y + c*z - d. X and Z pick
// the larger or smaller end with explicit min/max compares; Y uses +40h as the
// low end and +44h as the high end without ordering them. The caller 00AE47E0
// sums the six frustum planes, returns on any Outside, and branches on a sum
// of exactly 6 (every plane straddling).
FoliageCellSide foliage_cell_classify_plane_00ae2fe0(const FoliageCellBox& cell,
    const TerrainVec3f& translation, float a, float b, float c, float d) noexcept;

// One baked vertex of 00AE41C0's 24h-byte output: position, normal, UV and a
// packed colour (0xFFFFFFFF when the source stream has no colour semantic 10).
struct FoliageBakedVertex {
    TerrainVec3f position;
    TerrainVec3f normal;
    float u{}, v{};
    std::uint32_t colour{0xFFFFFFFFu};
};

struct FoliageSourceVertex {
    TerrainVec3f position;
    TerrainVec3f normal;
    float u{}, v{};
    bool has_colour{false};
    std::uint32_t colour{};
};

// 00AE41C0's per-vertex transform. The native routine draws two uniform
// values from the shared generator 00BD2F10 first: a yaw in [0, 2*pi)
// (00CE3D9C) and a value in [0.3, 1) (00CE69C8) that it discards (FSTP ST0 at
// 00AE420F). The scale is the caller's float (seventh argument), not the second
// draw. Position: scale * v, rotated about Y by the yaw
// (x' = c*x - s*z, z' = s*x + c*z), then offset; normal rotated the same way
// and not scaled; UV = uv * uv_scale + uv_offset.
FoliageBakedVertex foliage_bake_vertex_00ae41c0(const FoliageSourceVertex& source,
    float yaw_cos, float yaw_sin, float scale, const TerrainVec3f& offset,
    float uv_scale_u, float uv_scale_v, float uv_offset_u, float uv_offset_v) noexcept;

// 00AE441E..00AE448A: the element's triangle indices with `base` added in
// 16-bit arithmetic.
std::vector<std::uint16_t> foliage_bake_indices_00ae41c0(
    const std::vector<std::uint16_t>& source, std::uint16_t base);

} // namespace bsp
