#pragma once
#include "bsp/legacy_crt_math.hpp"
#include "bsp/native_plane_set.hpp"

namespace bsp {
// Borrow the actual CRT dispatch word and original D7A208 negative-zero word.
// The handler is fixed to the existing complete C27489 provider. Its existing
// LegacyCrtMathRuntime must already bind actual E16BD0 and the real errno
// accessor, and outlive calls. Construction does not change either runtime.
struct NativeCameraFrustumContext final {
    const CameraAxesCrtAccess length_access;
    const volatile std::uint32_t* const negative_zero_00d7a208;
    NativeCameraFrustumContext(const volatile std::uint32_t& actual_0109dd78,
        const volatile std::uint32_t& actual_00d7a208) noexcept
        : length_access{&actual_0109dd78, &legacy_crt_87except_00c27489},
          negative_zero_00d7a208(&actual_00d7a208) {}
};
static_assert(offsetof(NativeCameraFrustumContext, length_access) == 0);
static_assert(offsetof(NativeCameraFrustumContext, negative_zero_00d7a208) == 8);
static_assert(sizeof(NativeCameraFrustumContext) == 12);

// B650B0: original ECX=float4, no stack arguments, EAX=same, RET. Full shared
// 419440/CRT length; native FCOMI/JBE chooses +0 reciprocal for nonpositive
// or unordered length, then performs all four multiplies/stores. EDX adds the
// concrete context; no FSQRT-only fallback or caller-selected error handler.
void* __fastcall normalize_native_camera_plane_00b650b0(
    void* actual_float4, const NativeCameraFrustumContext*);

// B653F0: original ECX=96-byte six-float4 destination, stack=64-byte matrix,
// EAX=destination, RET4. Original forward coefficient writes can alias later
// matrix reads. Negate six D values with SUBSS from current original constant,
// then normalize in order. EDX supplies context, preserved in one extra stack
// word. No camera cache flag/count/owner or hierarchy operation is performed.
void* __fastcall extract_native_camera_frustum_00b653f0(void* actual_six_float4,
    const NativeCameraFrustumContext*, const void* actual_matrix);

// B658E0: ECX=plane set, stack=96-byte six-float4 source, flags DWORD, RET8.
// Twenty-four ordered FLD/FSTP pairs and six full flags stores. Caller B70710
// passes7; all input flag bits are preserved. Count+140 and records6..15 stay
// untouched. EDX is explicitly unused so the remaining arguments stay stacked.
void __fastcall assign_native_camera_frustum_planes_00b658e0(
    void* actual_plane_set, void* unused_edx, const void* actual_six_float4,
    std::uint32_t flags);

// Raw byte views deliberately preserve unaligned/overlapping native accesses.
// Names are hypotheses. These explicit C++ entry/context interfaces do not
// assert original caller ABI, complete camera hierarchy or game validation.
} // namespace bsp
