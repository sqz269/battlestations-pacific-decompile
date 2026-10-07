#pragma once

#include <array>
#include <cstddef>

namespace bsp {

// Borrow ONE actual live root and its actual std::array<float,3> member at+38.
// The root has >=44h backing; this is a represented-field view, not a recovered
// whole class layout, constructor, executable table, arena or lifetime owner.
struct NativeLandApproachReferencePointView {
    const void* actual_root;
    const std::array<float, 3>& xyz_38;
};

// PURE extent/address coherence only. The array must already be a genuine live
// member, never a temporary XYZ snapshot, opaque-cache reinterpretation or
// translated pointer. Mismatch reports a SOURCE admission error, not a native
// guard/fallback. No represented values, callbacks or native calls are read.
NativeLandApproachReferencePointView native_land_approach_reference_point_view(
    const void* actual_root, std::size_t actual_backing_bytes,
    const std::array<float, 3>& actual_xyz_38);

// COMPLETE009AFAD0..009AFAE8:24B/8 instructions. Original ECX=root,
// stack(output12B), EAX=output, RET4. Sequential actual FLD32 root38/FSTPout0,
// FLD32 root3C/FSTPout4, FLD32 root40/FSTPout8. Reuse the UNCHANGED genuine
// copy_point_record_xyz_0049c1db provider; no new FP kernel, raw memcpy, scalar
// arithmetic, temporary snapshot or default data. Return SAME output identity.
//
// Require caller-writable12B output, valid live aligned array/backing/aliases,
// masked x87 exceptions and stack space for each load/store pair. Exact-self
// and valid overlap follow sequential current reads/writes, never a snapshot.
// Structural reentry/concurrency, invalid memory/object life, unmasked traps,
// privateEH/fault, native class/register ABI and gameplay remain excluded.
// The distinct D1FDB8/D1FF88/D1FF94 slot0 tokens are RAW UNCALLABLE profiles;
// no production virtual dispatch, lower/composite constructor, geometry,
// depth-charge substitution, default world or ownership/lifetime binding.
float* copy_native_land_approach_reference_point_009afad0(
    const NativeLandApproachReferencePointView&, float* actual_output);

} // namespace bsp
