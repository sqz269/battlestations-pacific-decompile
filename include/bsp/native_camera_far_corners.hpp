#pragma once
#include <cstdint>

namespace bsp {
// Borrow the same immutable native cells as the calling scene stage. No
// replacement camera, matrix, world cache or floating-control state is made.
struct NativeCameraFarCornersContext {
    const volatile std::uint32_t* one_00d7a24c;
    const volatile std::uint32_t* negative_one_00d7a260;
    const volatile double* half_00d7a280;
    const volatile std::uint32_t* near_00ce3800;
};

// Complete B0EB10..B0F008. Native ECX unused, stack camera/output16, RET8;
// source ECX borrows context, unused EDX retains the two native public slots.
// Original near comparison (unordered included), raw perspective/inverse,
// homogeneous division, rounded far/near scaling, eight world-rotation passes
// and final four XYZW stores are retained. Actual camera/output may alias as
// in the original; private stack aliases and exception ABI are not established.
// Uses real B642F0/B632D0/B62D10/B6DB70; no callbacks or early default rays.
void* __fastcall compute_native_camera_far_corners_00b0eb10(
    const NativeCameraFarCornersContext*, void* unused_edx,
    void* actual_camera, void* actual_output_float16);
} // namespace bsp
