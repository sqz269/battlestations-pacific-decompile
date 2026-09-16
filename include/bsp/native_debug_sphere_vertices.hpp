#pragma once

#include <cstdint>

namespace bsp {

// Complete interior B2BF60..B2C142 of B2BB90, not an original function.
// Source ECX=output, EDX=center/radius DWORDs; stacked color and borrowed
// CEC730 double pointer; RET8. Writes 38 vertices at stride10h (608 bytes).
// The source wrapper snapshots the four input words into private scratch.
// Preserve native x87 control state, ordered spills and live angle-step reads.
// Original private-stack aliases, ambient x87 faults and parent ABI are unproved.
void __fastcall write_native_debug_sphere_vertices_00b2bf60(
    void* destination, const void* center_radius_float4, std::uint32_t color,
    const volatile double* actual_angle_step_00cec730);

} // namespace bsp
