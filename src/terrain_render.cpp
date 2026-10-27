// Terrain and foliage render kernels. Evidence in docs/CC13_TERRAIN_RENDER.md;
// names are hypotheses.
#include "bsp/terrain_render.hpp"
#include "bsp/cc13_x87_model.hpp"

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

TerrainBoxVisibility terrain_box_classify_planes_00ae05e0(const std::vector<TerrainPlane>& planes,
    const TerrainVec3f& centre, const TerrainVec3f& half_extent) noexcept
{
    // cc13-fixture6: per x87 operation at 24-bit precision control (main thread, the bush cull
    // 00AE1A10), in the listing's order. 00AE065B-00AE06AF: r = (|b| e.y + |a| e.x) + |c| e.z,
    // each |v| stored to float after FABS; s = (a p.x + b p.y) + c p.z. r and s are FSTP'd to
    // float, then s - d is stored to float (00AE06B7). 00AE06C6-00AE06D4: -r > s returns
    // Outside (JA, so an unordered compare does not); r > s gives Intersecting.
    using namespace cc13x87;
    TerrainBoxVisibility result = TerrainBoxVisibility::Inside;
    for (const TerrainPlane& plane : planes) {
        const float aa = std::fabs(plane.a), ab = std::fabs(plane.b), ac = std::fabs(plane.c);
        const float reach = static_cast<float>(x87_add24(
            x87_add24(x87_mul24(ab, half_extent.y), x87_mul24(aa, half_extent.x)), x87_mul24(ac, half_extent.z)));
        const float dot = static_cast<float>(x87_add24(
            x87_add24(x87_mul24(plane.a, centre.x), x87_mul24(plane.b, centre.y)), x87_mul24(plane.c, centre.z)));
        const float side = static_cast<float>(x87_sub24(dot, plane.d));
        if (-reach > side) {
            return TerrainBoxVisibility::Outside;
        }
        if (reach > side) {
            result = TerrainBoxVisibility::Intersecting;
        }
    }
    return result;
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

std::unique_ptr<BushQuadNode> bush_quad_node_read_00ae0f10(FoliageQuadReader& reader,
    const TerrainVec3f& centre, const TerrainVec3f& half_extent, float y_offset)
{
    auto node = std::make_unique<BushQuadNode>();
    const std::int32_t leaf = reader.read_dword_38(0);  // 00AE0F3C, EBX
    node->first_vertex = reader.read_dword_38(0);       // 00AE0F48 -> +2Ch
    node->centre = centre;
    node->half_extent = half_extent;
    node->child_count = 0;                              // 00AE0F78
    if (leaf != 0) {
        node->colour = 0xFFFFFFFFu;                     // 00AE0F87..00AE0FA8
    } else {
        // 00AE1018..00AE1051: the children's half extent; 0.5 is the double 00D7A280.
        const TerrainVec3f h{half_extent.x * 0.5f, half_extent.y * 0.5f,
            half_extent.z * 0.5f};
        const TerrainVec3f child_centres[4] = {
            {centre.x - h.x, centre.y - h.y, centre.z - h.z}, // 00AE1079
            {centre.x + h.x, 0.0f, centre.z - h.z},           // 00AE1111, XORPS for y
            {centre.x + h.x, centre.y + h.y, centre.z + h.z}, // 00AE11A7
            {centre.x - h.x, 0.0f, centre.z + h.z},           // 00AE1247, XORPS for y
        };
        for (int i = 0; i < 4; ++i) {
            if (reader.read_dword_38(0) == 0) {
                continue;
            }
            ++node->child_count;
            node->children[static_cast<std::size_t>(i)] =
                bush_quad_node_read_00ae0f10(reader, child_centres[i], h, y_offset);
        }
    }
    // 00AE0FAE..00AE1005, shared by both arms.
    node->vertex_end = reader.read_dword_38(0);
    const float y0 = static_cast<float>(
        static_cast<double>(reader.read_float_44(0)) + y_offset);
    const float y1 = static_cast<float>(
        static_cast<double>(reader.read_float_44(0)) + y_offset);
    node->centre.y = static_cast<float>((static_cast<double>(y0) + y1) * 0.5);
    node->half_extent.y = static_cast<float>((static_cast<double>(y1) - y0) * 0.5);
    return node;
}

std::unique_ptr<FoliageQuadNode> foliage_quad_node_read_00ae5550(FoliageQuadReader& reader,
    float x0, float z0, float x1, float z1)
{
    auto node = std::make_unique<FoliageQuadNode>();
    node->x0 = x0;
    node->z0 = z0;
    node->x1 = x1;
    node->z1 = z1;
    const std::int32_t leaf = reader.read_dword_38(0);  // 00AE55A7, EBP
    node->instance_first = reader.read_dword_38(0);     // 00AE55B3 -> +48h
    node->value_54 = 0;
    node->child_count = 0;
    if (leaf == 0) {
        // 00AE55EB..00AE5644: the difference stays on the x87 stack.
        const float mx = static_cast<float>(x0 + (static_cast<double>(x1) - x0) * 0.5);
        const float mz = static_cast<float>(z0 + (static_cast<double>(z1) - z0) * 0.5);
        const float rects[4][4] = {
            {x0, z0, mx, mz}, // 00AE566D
            {mx, z0, x1, mz}, // 00AE56E6
            {mx, mz, x1, z1}, // 00AE5760
            {x0, mz, mx, z1}, // 00AE57DA
        };
        for (int i = 0; i < 4; ++i) {
            if (reader.read_dword_38(0) == 0) {
                continue;
            }
            ++node->child_count;
            const float* r = rects[i];
            node->children[static_cast<std::size_t>(i)] =
                foliage_quad_node_read_00ae5550(reader, r[0], r[1], r[2], r[3]);
        }
    }
    // 00AE55CA..00AE55E4 (leaf) and 00AE582B..00AE5853 (internal).
    node->instance_end = reader.read_dword_38(0);
    node->y0 = reader.read_float_44(0);
    node->y1 = reader.read_float_44(0);
    return node;
}

TerrainPatchHeights terrain_heightmap_patch_00add3f0(const std::uint8_t* image,
    std::uint32_t pitch, std::uint32_t row0, std::uint32_t col0, float h_base,
    float h_top, float s_low, float s_high) noexcept
{
    TerrainPatchHeights patch;
    // 00ADD672..00ADD699: computed once per patch row, kept as float.
    const float scale = static_cast<float>(
        (static_cast<double>(h_top) - h_base) / (static_cast<double>(s_high) - s_low));
    float minimum = 1.0e10f; // 00CE4970
    float last = 0.0f;
    for (std::uint32_t z = 0; z < 33; ++z) {
        for (std::uint32_t x = 0; x < 33; ++x) {
            // 00ADD764..00ADD770: MOVZX of image[(row0 + z) * pitch + x + col0].
            float s = static_cast<float>(image[(row0 + z) * pitch + x + col0]);
            if (s_low > s) s = s_low;   // 00ADD78B, FCOMIP / JBE
            if (s > s_high) s = s_high; // 00ADD7AB, FCOMIP / JBE
            const float h = static_cast<float>(
                static_cast<double>(s) * scale + h_base); // 00ADD7C0..00ADD7DB
            patch.heights[z][x] = h;
            if (!(h > minimum)) minimum = h;          // 00ADD7F6, FCOMI / JA
            last = (minimum > h) ? minimum : h;       // 00ADD806, always h
        }
    }
    patch.min_height = minimum;
    patch.last_height = last;
    return patch;
}

} // namespace bsp
