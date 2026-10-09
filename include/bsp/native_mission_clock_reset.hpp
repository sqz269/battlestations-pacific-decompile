#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission clock reset requires MSVC Win32 x87 and MOVSS.
#endif

namespace bsp {
// Borrow the five distinct actual cells of the SAME clock state. This context
// owns no storage and supplies no shared clock owner, initial value or schedule.
// Every reference must remain valid through the call. The step must be the
// qualified binary64 D7A270 operand (bits 3FA99999A0000000), and the selector the
// actual mutable 0109EEA4 cell, read by the existing converter at call time.
struct NativeMissionClockResetContext {
    volatile float& clock_00f876a4;
    volatile float& accumulated_00f876a8;
    volatile float& remainder_00f876ac;
    volatile std::uint32_t& step_count_00f876b0;
    volatile float& interpolation_00f876b4;
    const volatile double& step_00d7a270;
    const volatile std::uint32_t& conversion_mode_0109eea4;
};

// Complete ordinary 00874640..00874690 reset/set sequence. The DWORD carries
// the original float argument bits; no C++ floating conversion precedes FLD.
// Preserves the ordered bit stores, retained x87 operands across the existing
// raw converter, signed count reload and rounded remainder store/reload.
// New C++ ABI: no original RET4, scratch-register/EFLAGS, SEH, drop-in or runtime
// admission. Shared-cell identity, reset paths and owner lifetime are external.
void reset_native_mission_clock_00874640(
    std::uint32_t input_float_bits,
    const NativeMissionClockResetContext&);
} // namespace bsp
