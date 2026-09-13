#pragma once
#include <cstdint>

namespace bsp {

// Complete native scalar leaves over the actual camera address. The producer
// B71A80 uses a 458h camera region within a 45Ch pool slot; NativeCameraOwner is
// a separate companion, never the pointer passed here. No lifetime is started.
// Native ECX=raw camera, one stacked DWORD, RET4; this new fastcall interface
// uses EDX=address of that raw argument word, and therefore plain RET.
// Read that current word before ANY owner write. It may overlap camera fields.
// Source pointer/owner values and private ABI storage must remain stable.

// B6FBF0..B6FC0A [27], B6FC10..B6FC2A [27]: MOVSS captures the argument,
// then AND current camera+2F0 with FFFFFF41, then MOVSS to +1D4/+1D8.
// No floating conversion, NaN normalization, comparison, clamp or equality skip.
void __fastcall set_native_camera_near_00b6fbf0(
    void* actual_camera, const void* raw_argument_word) noexcept;
void __fastcall set_native_camera_far_00b6fc10(
    void* actual_camera, const void* raw_argument_word) noexcept;

// B6FE10..B6FE1C [13]: capture complete DWORD, write +188, return it in EAX.
std::uint32_t __fastcall set_native_camera_clear_flags_00b6fe10(
    void* actual_camera, const void* raw_argument_word) noexcept;

// B6FE20..B6FE30 [17]: MOVSS argument bits to +18C; no cache invalidation.
// This leaf consumes a caller-prepared float32 word. In B3C800 its caller runs
// FLD1/FSTP32 first; reproducing that caller conversion is a separate obligation.
void __fastcall set_native_camera_clear_depth_00b6fe20(
    void* actual_camera, const void* raw_argument_word) noexcept;

// New C++ ABI, not original-call-site binary compatibility. These four leaves
// do not construct a camera/viewport, register profiles, or complete B3C800.
} // namespace bsp
