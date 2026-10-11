#pragma once
// Terrain and foliage render kernels from partition segment 81
// (00ADA420..00AF3750). Names are hypotheses, not recovered symbols; evidence
// is in docs/CC13_TERRAIN_RENDER.md. Not binary compatible: the native routines
// read scene nodes, locked vertex streams and quadtree nodes; these functions
// take the values they read and return what the native code stores. x87
// extended-precision intermediates are not reproduced bit for bit.
#include <array>
#include <cstdint>
#include <memory>
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

struct TerrainPlane {
    float a{}, b{}, c{}, d{}; // a*x + b*y + c*z - d
};

enum class TerrainBoxVisibility : std::int32_t {
    Intersecting = 0,
    Inside = 1,
    Outside = 2,
};

// 00AE05E0, __fastcall(ECX = plane count, EDX = planes 10h apart), stack
// (centre*, half_extent*), RET 8: the centre/half-extent box test. Per plane,
// r = |c|*e.z + |a|*e.x + |b|*e.y and s = (c*p.z + a*p.x + b*p.y) - d; s < -r
// returns Outside at once, s < r downgrades the answer to Intersecting.
TerrainBoxVisibility terrain_box_classify_planes_00ae05e0(const std::vector<TerrainPlane>& planes,
    const TerrainVec3f& centre, const TerrainVec3f& half_extent) noexcept;

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

// The two reader slots the foliage quadtree loaders call on the stream the
// group initialiser was given (00AE2450 / 00AE5870 argument 1). Both are
// __thiscall with one stack argument, always 0 here (PUSH EBP / PUSH EBX with
// the register zeroed): +38h returns a dword in EAX, +44h returns a float on
// ST0. The slot meaning is inferred from the return registers and from how
// the values are used; the stream class itself is not identified.
struct FoliageQuadReader {
    virtual ~FoliageQuadReader() = default;
    virtual std::int32_t read_dword_38(std::int32_t argument) = 0;
    virtual float read_float_44(std::int32_t argument) = 0;
};

// The 38h-byte bush quadtree node 00AE0F10 fills and 00AE1A10 walks.
struct BushQuadNode {
    TerrainVec3f centre;                 // +00h
    TerrainVec3f half_extent;            // +0Ch
    std::uint32_t colour{};              // +18h, FFFFFFFFh on a leaf; unwritten on an internal node
    std::array<std::unique_ptr<BushQuadNode>, 4> children; // +1Ch..+28h
    std::int32_t first_vertex{};         // +2Ch, the second dword of the record
    std::int32_t vertex_end{};           // +30h
    std::int32_t child_count{};          // +34h
};

// 00AE0F10, __thiscall(ECX = new 38h node, reader, const float centre[3],
// const float half[3], float y_offset), RET 10h, returns ECX. Record:
//   dword leaf, dword +2Ch,
//   (internal only) four times { dword present, child record if non-zero },
//   dword +30h, float y0, float y1.
// Every node ends with the tail, so the y centre and half extent the parent
// passed are always replaced: centre.y = (y0' + y1') * 0.5 and half.y =
// (y1' - y0') * 0.5 with yN' = yN + y_offset rounded to float. A child has half
// the parent's half extent; its centre is the parent's x and z plus or minus
// that, in the order (-,-), (+,-), (+,+), (-,+). The native y it passes is
// centre.y - h.y, 0.0, centre.y + h.y and 0.0, overwritten by the child's tail.
// A failed operator_new (38h) leaves the slot null without reading the
// child's record; that path is not reproduced (new throws).
std::unique_ptr<BushQuadNode> bush_quad_node_read_00ae0f10(FoliageQuadReader& reader,
    const TerrainVec3f& centre, const TerrainVec3f& half_extent, float y_offset);

// The 58h-byte foliage quadtree node 00AE5550 fills, 00AE47E0 culls (as a
// FoliageCellBox) and 00AE37F0 draws.
struct FoliageQuadNode {
    float x0{}, z0{}, x1{}, z1{};        // +00h..+0Ch
    // +14h..+1Ch: an empty std::vector (three null pointers); 00AE37F0 iterates it.
    std::int32_t child_count{};          // +24h, 0 for a leaf
    std::array<std::unique_ptr<FoliageQuadNode>, 4> children; // +2Ch..+38h, unwritten on a leaf
    float y0{}, y1{};                    // +40h, +44h
    std::int32_t instance_first{};       // +48h, the second dword of the record
    std::int32_t instance_end{};         // +4Ch
    std::int32_t value_54{};             // +54h, zeroed
};

// 00AE5550, __thiscall(ECX = new 58h node, reader, const float rect[4]),
// RET 8, returns ECX. Record:
//   dword leaf, dword +48h,
//   (internal only) four times { dword present, child record if non-zero },
//   dword +4Ch, float +40h, float +44h.
// The split point is (x0 + (x1 - x0) * 0.5, z0 + (z1 - z0) * 0.5) and the
// children are (x0,z0,mx,mz), (mx,z0,x1,mz), (mx,mz,x1,z1), (x0,mz,mx,z1).
// +48h..+4Ch is the instance range 00AE47E0 draws for a node.
std::unique_ptr<FoliageQuadNode> foliage_quad_node_read_00ae5550(FoliageQuadReader& reader,
    float x0, float z0, float x1, float z1);

// One 33x33 vertex patch of a heightmap terrain, as 00ADD3F0's inner loop
// (00ADD755..00ADD82A) fills it. `heights[z][x]` is what the patch's vtable
// +1Ch receives as (height, x, z).
struct TerrainPatchHeights {
    float heights[33][33]{};
    float min_height{};        // [ESP+58h], starts at 1e10 (00CE4970)
    float last_height{};       // [ESP+64h], used as the box top; see below
};

// 00ADD3F0, __thiscall(group, a1, a2, name, h_base, h_top, s_low, s_high),
// RET 1Ch: the "Terrain Visibility Group" built from an 8-bit heightmap image
// (rows flipped first, 00ADD4D0..00ADD5A8), one patch per 32x32 cells. The
// per-sample kernel, for the patch whose first sample is image[row0][col0]
// (pitch from 00BEFB70):
//   s = float(byte) clamped to [s_low, s_high] (two ordered compares, so a NaN
//       bound leaves s as it is),
//   h = s * ((h_top - h_base) / (s_high - s_low)) + h_base, rounded to float.
// The scale does not subtract s_low, so the mapping is exact only for
// s_low = 0. The minimum is tracked; the other accumulator is stored as
// max(h, minimum), which is h itself, so it ends as the height of the last
// vertex (x = z = 32), and 00ADD8E6 divides that by the world matrix's y scale
// as the patch box's top. Whether a caller passes s_low = 0 is not checked.
TerrainPatchHeights terrain_heightmap_patch_00add3f0(const std::uint8_t* image,
    std::uint32_t pitch, std::uint32_t row0, std::uint32_t col0, float h_base,
    float h_top, float s_low, float s_high) noexcept;

} // namespace bsp
