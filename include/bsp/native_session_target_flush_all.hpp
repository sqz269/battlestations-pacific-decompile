#pragma once
#include "bsp/native_session_target_lifetime.hpp"
#include "bsp/native_session_transport_flush.hpp"

namespace bsp {
// Complete [00783490,0078350B): native ECX transport, stack target, RET4.
// Reuse the existing pure actual-storage aliases and final concrete transport
// +20 providers. No new flush callback, profile mapping or clock is introduced.
class NativeUnitHealthMessageFlushAllCalls : public NativeUnitHealthMessageTransportCalls {
public:
    using NativeUnitHealthMessageTransportCalls::NativeUnitHealthMessageTransportCalls;

    // Capture entry lock; visit fresh actual D40/D44/D48 cursor cells in order.
    // Nonzero modulo32 byte count calls current transport+20, then resets the
    // CAPTURED cursor using its POST-CALL base even when the provider suppresses
    // delivery or cannot enqueue. Empty cursors receive no reset. Finally read
    // actual target+4 again, decrement depth, then leave that current lock.
    void flush_session_target_00783490(void* actual_transport,
        NativeSessionTargetStorage* actual_target);
};

// New Source interface, not the native calling ABI. Every reached lock/cursor,
// writable carry byte, transport profile and concrete provider binding must be
// valid and stay live. Current-thread lock ownership is required for Leave.
// Depth and cursor-address arithmetic wrap as DWORDs; no guards, size clamps,
// exception cleanup or default delivery success are added. Existing Source
// profile-admission exceptions are not the original native fault/private EH
// contract. Whole session/transport factories and socket sending remain open.
} // namespace bsp
