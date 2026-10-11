// Terrain and foliage render kernels. Evidence in docs/CC13_TERRAIN_RENDER.md;
// names are hypotheses.
#include "bsp/terrain_render.hpp"

#include <cmath>

namespace bsp {

TerrainSphere terrain_box_bounding_sphere_00ada600(const TerrainVec3f& min,
    const TerrainVec3f& max) noexcept
{
    const float half = 0.5f; // 00D7A280, double 0.5
    const TerrainVec3f extent{(max.x - min.x) * half, (max.y - min.y) * half, (max.z - min.z) * half};
    TerrainSphere sphere;
    sphere.centre = {(min.x + max.x) * half, (min.y + max.y) * half, (min.z + max.z) * half};
    sphere.radius = std::sqrt(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z);
    return sphere;
}

FoliageCellSide foliage_cell_classify_plane_00ae2fe0(const FoliageCellBox& cell,
    const TerrainVec3f& translation, float a, float b, float c, float d) noexcept
{
    const float x_lo = cell.x0 + translation.x;
    const float x_hi = translation.x + cell.x1;
    float x_near; // the end the plane normal points away from
    float x_far;  // the end the plane normal points toward
    if (a < 0.0f) {
        x_near = x_hi < x_lo ? x_lo : x_hi;
        x_far = x_hi <= x_lo ? x_hi : x_lo;
    } else {
        x_near = x_lo < x_hi ? x_lo : x_hi;
        x_far = x_lo <= x_hi ? x_hi : x_lo;
    }
    const float y_near = translation.y + (b < 0.0f ? cell.y1 : cell.y0);
    const float y_far_raw = b < 0.0f ? cell.y0 : cell.y1;
    const float z_hi = translation.z + cell.z1;
    const float z_lo = cell.z0 + translation.z;
    float z_near;
    float z_far;
    if (c < 0.0f) {
        z_near = z_hi < z_lo ? z_lo : z_hi;
        z_far = z_lo < z_hi ? z_lo : z_hi;
    } else {
        z_near = z_lo < z_hi ? z_lo : z_hi;
        z_far = z_hi < z_lo ? z_lo : z_hi;
    }
    // 00AE31EE: the far vertex first; behind the plane means outside.
    if (!(0.0f <= (b * (translation.y + y_far_raw) + a * x_far + c * z_far) - d)) {
        return FoliageCellSide::Outside;
    }
    if ((y_near * b + x_near * a + z_near * c) - d <= 0.0f) {
        return FoliageCellSide::Straddles;
    }
    return FoliageCellSide::Inside;
}

FoliageBakedVertex foliage_bake_vertex_00ae41c0(const FoliageSourceVertex& source,
    float yaw_cos, float yaw_sin, float scale, const TerrainVec3f& offset,
    float uv_scale_u, float uv_scale_v, float uv_offset_u, float uv_offset_v) noexcept
{
    FoliageBakedVertex out;
    // 00AE42C4..00AE434B
    const float sx = source.position.x * scale;
    const float sy = source.position.y * scale;
    const float sz = source.position.z * scale;
    const float rx = yaw_cos * sx - yaw_sin * sz;
    const float rz = yaw_sin * sx + yaw_cos * sz;
    out.position = {rx + offset.x, sy + offset.y, rz + offset.z};
    // 00AE435A..00AE4390
    out.normal = {yaw_cos * source.normal.x - yaw_sin * source.normal.z, source.normal.y,
        yaw_sin * source.normal.x + yaw_cos * source.normal.z};
    // 00AE43A5..00AE43DC
    out.u = uv_offset_u + uv_scale_u * source.u;
    out.v = source.v * uv_scale_v + uv_offset_v;
    out.colour = source.has_colour ? source.colour : 0xFFFFFFFFu;
    return out;
}

std::vector<std::uint16_t> foliage_bake_indices_00ae41c0(
    const std::vector<std::uint16_t>& source, std::uint16_t base)
{
    std::vector<std::uint16_t> out;
    out.reserve(source.size());
    for (const std::uint16_t index : source) {
        out.push_back(static_cast<std::uint16_t>(index + base));
    }
    return out;
}

} // namespace bsp
