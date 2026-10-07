#pragma once
#include "bsp/native_session_target_flush_all.hpp"

namespace bsp {
// Complete [007848F0,007849BB): native ECX transport, no arguments, RET.
// The original inlines the completed 00783490 target loop. Source composes
// that complete operation using the same actual locks/cursors/providers.
class NativeUnitHealthMessagePeerFlushCalls : public NativeUnitHealthMessageFlushAllCalls {
public:
    using NativeUnitHealthMessageFlushAllCalls::NativeUnitHealthMessageFlushAllCalls;

    // Borrow transport+0C and the reached native 0C-byte sentinel/list nodes.
    // Reload the head for each end comparison and each reachable validation;
    // read captured-node+8 after validation, flush it, validate again, then
    // read captured-node+0. Neither peer_count+10 nor a local-peer filter is used.
    void flush_session_peers_007848f0(void* actual_transport);
};

// New Source interface, not the native calling ABI. Actual backing must be
// live, valid and finite; the inherited pure aliases introduce no callbacks
// or early value observations. The fixed current CRT invalid-parameter
// service may return, so the original continuation is preserved. This does
// not reconstruct 00BF6713's old CRT globals, Watson or private fault/EH ABI.
// The native self-comparison before end testing is provably true and omitted.
// Target/transport/clock/queue admission remains that of the complete base.
} // namespace bsp
