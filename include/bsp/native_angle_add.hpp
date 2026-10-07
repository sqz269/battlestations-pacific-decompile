#pragma once
#include <cstddef>
#include <type_traits>

namespace bsp {
// Pure aliases to actual minus-pi CE3D18, pi CE3D28 and two-pi CE3828 cells.
// No early floating observations, copies, ownership or control-word changes.
struct NativeAngleAddConstants {
    const volatile double* const minus_pi;
    const volatile double* const pi;
    const volatile double* const two_pi;
    NativeAngleAddConstants(const volatile double& negative,
        const volatile double& positive, const volatile double& period) noexcept
        : minus_pi(&negative), pi(&positive), two_pi(&period) {}
};
static_assert(sizeof(void*)==4 && sizeof(NativeAngleAddConstants)==12);
static_assert(std::is_standard_layout<NativeAngleAddConstants>::value);
static_assert(offsetof(NativeAngleAddConstants,minus_pi)==0);
static_assert(offsetof(NativeAngleAddConstants,pi)==4);
static_assert(offsetof(NativeAngleAddConstants,two_pi)==8);

// Complete 00438AA0..00438B0E, exclusive: two original float stack words,
// ST0 result, RET8. SOURCE adds constants in ECX; EDX is unused on entry.
// Original x87 add/float spill, comparisons, repeated double corrections and
// per-iteration float spills remain. No finite guard, clamp or iteration cap.
// Caller provides live actual constant cells and a terminating native domain.
// Component verification uses masked exceptions and at least three free FP
// stack slots. Historical ABI integration/faults/gameplay are not established.
float __fastcall native_angle_add_00438aa0(const NativeAngleAddConstants*,
    void* unused_edx, float left, float right);
} // namespace bsp
