#pragma once
#include "bsp/native_frame_clock_publication.hpp"
#include "bsp/native_session_target_lifetime.hpp"

namespace bsp {
// Complete [007839A0,007839CA): native ECX target, no stack arguments, RET.
// Borrow the actual current 01090AB0 publication and D68D50 method context.
// Supply the actual 16-byte caller sample-stack preimage, as for the existing
// transport stamper; no zero preimage, clock or alternate provider is made.
// The concrete sampler's RETURNED pointer supplies both signed qwords. Keep
// FILD/FILD/FDIVP/FSTP and its sole binary32 store at actual target+D68.
void refresh_native_session_target_timestamp_007839a0(
    NativeSessionTargetStorage* actual_target,
    const NativeFrameClockPublicationContext& clock,
    const ClockTimestamp& actual_sample_stack_preimage);

// New Source ABI. The existing publication adapter admits only a non-null
// actual D68D50 clock and borrowed current slot+20 == BEE080; unsupported
// profiles throw Source logic_error, not the original fault/exception ABI.
// Entry does not normalize ambient x87 control/stack state; the original
// arithmetic's status/stack effects remain. Verification includes masked
// stack overflow; unmasked exceptions and QPC failure stack
// contents are not guaranteed. No caller reachability, publication producer,
// derived target, socket, binary replacement or game behavior is established.
} // namespace bsp
