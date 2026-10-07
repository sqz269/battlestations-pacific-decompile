#pragma once
#include "bsp/native_session_target_lifetime.hpp"

namespace bsp {
// Complete [007827D0,007827E6): native ECX unsigned index; RET. Borrow the
// actual F871B0 word. Indices above 15 do not access it; valid indices use the
// native unprefixed word AND. No target ownership or concurrency guarantee.
void release_native_session_target_slot_007827d0(std::uint32_t original_index,
    volatile std::uint16_t& actual_slots_f871b0) noexcept;

// Complete [007831E0,00783226): native ECX target; signed sample on stack;
// RET4. Operates on the captured actual D54 history, retaining the first
// captured sample address and subsequent fresh index/array/count observations.
// CVTSI2SS uses ambient MXCSR between the x87 subtraction and its store.
void append_native_session_target_history_007831e0(NativeSessionTargetStorage* actual_target,
    std::int32_t original_sample) noexcept;

// Complete [00783230,00783274): native ECX target; raw float word on stack;
// RET4. Signed count is tested before initial-word publication, then refreshed
// during the loop. Every sample uses fresh array/initial observations and an
// actual FLD/FSTP pair. Final FILD/count, index-zero, FMUL/initial, FSTP/total
// retain their native order; there is no raw-word fill or empty-count bypass.
void reset_native_session_target_history_00783230(NativeSessionTargetStorage* actual_target,
    std::uint32_t original_initial_float_word) noexcept;

// New Source ABIs over already-constructed storage; no allocation or lifetime
// is added. Caller supplies valid reached history/index/array storage. No
// bounds clamp or control-word normalization is introduced. Verification uses
// masked FP exceptions and an available x87 slot; original ABI/hardware faults,
// 7833E0 timing/conversion, derived target publication and gameplay are unbound.
} // namespace bsp
