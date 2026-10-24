#pragma once

#include "bsp/world_ocean.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {
// Recovered Dyn convex-hull data, produced by 00C389C0. Names are hypotheses;
// these layouts are native Win32, unlike the algorithm's C++ work vectors.
struct AvoidZoneDynHullVertex {
    OceanVec3 point;                         // +00
    std::uint16_t adjacency_offset;          // +0C, in uint16 elements
    std::uint16_t unspecified_0e;            // producer does not initialize
};
struct AvoidZoneDynHullData {
    AvoidZoneDynHullVertex* vertices;        // +00
    std::uint32_t vertex_count, vertex_capacity;
    std::uint16_t* adjacency;                // +0C: degree, neighbor ids per vertex
    std::uint32_t adjacency_count, adjacency_capacity;
    OceanVec3 minimum, maximum;              // +18, +24
    std::uint16_t support_seed[27];          // +30: center slot 13 untouched
    std::uint16_t unspecified_66;
};
struct AvoidZoneDynHullHandle {
    AvoidZoneDynHullData* data;              // +00, owned
    std::uint32_t field_04;                  // constructor zero; copy preserves
};
static_assert(sizeof(AvoidZoneDynHullVertex) == 0x10);
static_assert(offsetof(AvoidZoneDynHullData, adjacency) == 0x0c);
static_assert(offsetof(AvoidZoneDynHullData, support_seed) == 0x30);
static_assert(sizeof(AvoidZoneDynHullData) == 0x68);
static_assert(sizeof(AvoidZoneDynHullHandle) == 8);

// Owner supplies its real allocator/free pair. Native entry points use
// operator_new 00BF55BE/00BF681B and free 00BF6989/00BF65AC. No null-success path.
struct AvoidZoneDynHullMemory {
    void* context;
    void* (*allocate)(void*, std::size_t);
    void (*release)(void*, void*);
};

// 00C35E80 CleanupPoints, the whole body. Original ABI: EDX = out (packed XYZ,
// room for max(count, 8) points); stack count, source, stride in bytes,
// count_out, epsilon, scale_out (may be null); RET 18h; AL = 1 on every path.
// Reads source point i only after out[0..i) is written, as the native loop does.
// With scale_out the kept points are in the unit box (divided by the extent).
// Fewer than three points or an extent axis under 1e-6, before or after the
// merge, yields the 8 corners of a box and count 8. Not ABI-compatible.
bool dyn_hull_cleanup_points_00c35e80(OceanVec3* out, std::uint32_t count,
    const void* source, std::uint32_t stride, std::uint32_t* count_out,
    float epsilon, OceanVec3* scale_out);

// Semantic result of 00C5DAE0 flag 1, the flag used by 00C5DDD0. Triangle
// order and first-encounter vertex compaction follow the recovered algorithm.
// Finite, representable point domain; exceptional CRT/pool/OOM ABI unresolved.
struct AvoidZoneDynHullTriangles {
    std::vector<OceanVec3> points;
    std::vector<std::uint32_t> indices;
};
bool avoid_zone_dyn_hull_triangles_00c5dae0(
    const OceanVec3* points, std::uint32_t count,
    AvoidZoneDynHullTriangles& output);

// 00C5DAE0 whole body: the stack descriptor and the EAX output record.
struct AvoidZoneDynHullDescriptor {
    std::uint8_t flags = 0;          // +00 bit0 flat list, bit1 reverse winding
    std::uint32_t count = 0;         // +04
    const void* points = nullptr;    // +08 three floats per point
    std::uint32_t stride = 12;       // +0C bytes between points
    float epsilon = 0.0f;            // +10 00C35E80 duplicate tolerance
    float field_14 = 0.0f;           // +14 not read by 00C5DAE0
    std::uint32_t limit = 0;         // +18 00C5D900 limit
    std::uint32_t field_1c = 0;      // +1C not read by 00C5DAE0
};
struct AvoidZoneDynHullGenerated {
    bool polygon_list = false;       // +00 1 when flags bit0 is clear
    std::uint32_t vertex_count = 0;  // +04
    std::vector<OceanVec3> vertices; // +08
    std::uint32_t triangle_count = 0;  // +0C
    std::uint32_t index_count = 0;   // +10
    std::vector<std::uint32_t> indices;  // +14
};
// Returns 0 when the output was written, 1 otherwise (output untouched).
std::uint32_t avoid_zone_dyn_hull_generate_00c5dae0(
    const AvoidZoneDynHullDescriptor& descriptor, AvoidZoneDynHullGenerated& output);

// 00C389C0 data production and 004039D0 deep copy. Existing data must have
// its first six words initialized, as the native constructors do. Temporary
// edge hash/pool allocation identity is not reproduced; output data is real.
void avoid_zone_dyn_hull_data_00c389c0(AvoidZoneDynHullData&,
    const AvoidZoneDynHullTriangles&, const AvoidZoneDynHullMemory&);
void avoid_zone_dyn_hull_copy_data_004039d0(AvoidZoneDynHullData&,
    const AvoidZoneDynHullData&, const AvoidZoneDynHullMemory&);

// Actual handle lifetime chain; construct is for uninitialized handles,
// replace destroys the previous data first. Destroy intentionally leaves
// handle words unchanged (native 00C37450). Without an explicit owner replacement
// it must be called only once. Destroy captures data once before any release;
// callbacks may republish/end the handle, but the captured data must remain live
// until its final release. Its vertices field is read after adjacency release.
void avoid_zone_dyn_hull_construct_00c5df30(AvoidZoneDynHullHandle&,
    const OceanVec3*, std::uint32_t count, const AvoidZoneDynHullMemory&);
void avoid_zone_dyn_hull_replace_00c5deb0(AvoidZoneDynHullHandle&,
    const OceanVec3*, std::uint32_t count, const AvoidZoneDynHullMemory&);
void avoid_zone_dyn_hull_copy_00c40f50(AvoidZoneDynHullHandle& destination,
    const AvoidZoneDynHullHandle& source, const AvoidZoneDynHullMemory&);
void avoid_zone_dyn_hull_destroy_00c37450(AvoidZoneDynHullHandle&,
    const AvoidZoneDynHullMemory&);
std::uint32_t avoid_zone_dyn_hull_vertex_count_00c32d20(
    const AvoidZoneDynHullHandle&) noexcept;
} // namespace bsp
