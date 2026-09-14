#pragma once
#include "bsp/pose_derived.hpp"
#include <cstdint>

namespace bsp {
// All lookups are pure identity lookups of existing borrowed views. Pose fields
// refer to the actual raw owner at +3C/+74/+C8/+CC/+10C, and derived_110 to its
// actual +110 matrix. No allocation, copying, fallback or second pose domain.
class NativeSpatialPoseResolver : public PoseRefreshResolver {
public:
    virtual PoseDerivedView& resolve_derived(void* actual_owner) = 0;
};

struct NativeSpatialAttachmentAccess {
    void* volatile* frame_owner_00e188a8;
    const volatile double* half_00d7a280;
    const volatile double* cell_width_00ce3d90;
    // Required actual CRT floor service, cdecl(double), x87 ST0 result. Its
    // original CRT dispatch/exception state is a service boundary, not std::floor
    // silently substituted by this implementation. The access record is stable.
    double (__cdecl* floor_00bf85b0)(double);
    const volatile std::uint32_t* truncate_dispatch_0109eea4;
    NativeSpatialPoseResolver* poses;
};

// Complete 722C20[121], ECX accumulator, four float stack words, RET10h.
// Clear the vector sign bits before x87 multiply/add; retain every spill.
void __fastcall accumulate_native_abs_scaled_00722c20(void* accumulator,
    void* unused_edx, float x, float y, float z, float scale);
// Complete 98A310[190], ECX actual index, EDX node, min/max int pairs, RET8.
// No bounds checks. Four inline link slots are a caller invariant. Inclusive
// x-major/z-minor insertion preserves all reloads, duplicate prev-zero writes,
// wrapped arithmetic and the additive packed key, including alias effects.
void __fastcall register_native_spatial_cells_0098a310(void* actual_index,
    void* actual_node, const void* minimum_pair, const void* maximum_pair);
// Complete 98AD60[103]. Native ECX output pair, EDX point, RET. Source adds one
// stack access pointer (RET4). Divide by the current double, spill float, call
// floor on its double widening, spill float again, then canonical BF7420 over
// the current actual dispatch cell. Add 75 modulo 32 bits; no clamp.
void __fastcall native_spatial_cell_of_point_0098ad60(void* output_pair,
    const void* point, const NativeSpatialAttachmentAccess*);
// Complete 98A750[392]. ECX node, EDX adds access, RET. Current published
// frame-owner +648 controls the stamp at node+154; any matching stamp is inert.
// Preserve all x87/SSE spills and reuse the canonical raw point transform.
void __fastcall rebuild_native_spatial_world_bounds_0098a750(void* actual_node,
    const NativeSpatialAttachmentAccess*);
// Complete 98B530[455], native ECX parent, child stack word, RET4. EDX adds
// access. Rebuild child.world * parent.inverse inside EACH of eight corner
// iterations; expand parent bounds with original unordered x87 branches, call
// canonical 98A920, then continue upward through current parent+108.
void __fastcall merge_native_spatial_child_bounds_0098b530(void* actual_parent,
    const NativeSpatialAttachmentAccess*, void* actual_child);
// Complete 98B920[152], same registers/stack as the preceding function. Growth
// publishes doubled capacity before allocation; checked capacity*4 allocation,
// wrapping count*4 copy, free old buffer, publish replacement, then append.
// Allocation failure does not roll back capacity. No overflow guard is added
// to the capacity doubling, count increment, index or pointer arithmetic.
void __fastcall attach_native_spatial_child_0098b920(void* actual_parent,
    const NativeSpatialAttachmentAccess*, void* actual_child);
// Complete 98BA10[316], native ECX index; node,parent,matrix,flag stack words,
// RET10h. EDX adds access. Matrix is intentionally unused. Copy current pose
// and inverse through the canonical x87 matrix-copy body, then take the child,
// grid-root or loose-root branch. Set node+158 only after the branch completes.
void __fastcall attach_native_spatial_node_0098ba10(void* actual_index,
    const NativeSpatialAttachmentAccess*, void* actual_node, void* actual_parent,
    const void* unused_matrix, std::uint32_t static_flag_word);

// Original native FH3/fault delivery, invalid-storage recovery and gameplay
// admission are not established by these explicit source interfaces.
} // namespace bsp
