#pragma once
#include "bsp/native_session_target_operations.hpp"

namespace bsp {
// Borrow actual application cells and the actual native divisor operand.
// No clock, cached mode, default constants or alternate converter is supplied.
struct NativeSessionTargetTickHistoryBindings {
    const volatile std::uint32_t& step_f876b0;
    const volatile float& time_f876a8;
    const volatile float& divisor_d0de84; // Native raw float word 3D4CCCCD.
    const volatile std::uint32_t& conversion_mode_0109eea4;
};

// Complete [007833E0,0078348F): original ECX target; DWORD argument on stack;
// RET4. Capture F876B0 once; signed<10 executes FLDZ/FSTP and complete reset.
// Otherwise retain FLD(time)/FDIV(divisor) in ST0 for the shared native BF7420
// converter, which reads the actual mode cell at conversion time. Preserve
// both modulo32 differences and inclusive -32768/+32768 wrap endpoints.
// A positive first difference wins as a negative sample, then a positive
// second difference, otherwise zero; invoke the complete append operation.
void update_native_session_target_tick_history_007833e0(
    NativeSessionTargetStorage* actual_target, std::uint32_t original_tick_word,
    const NativeSessionTargetTickHistoryBindings& bindings) noexcept;

// New Source ABI over the existing live target/history storage. The sole
// observed caller passes a zero-extended 16-bit packet value, but this entry
// preserves its full DWORD argument without adding masking or clamping.
// No caller/derived lifetime, timing-state producer, socket, whole native ABI
// or game reconstruction is implied. FP validation is conditional on masked
// exceptions and sufficient x87 stack space for the actual conversion path.
} // namespace bsp
