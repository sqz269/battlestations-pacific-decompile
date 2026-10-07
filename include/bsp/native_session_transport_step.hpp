#pragma once
#include "bsp/native_session_target_lifetime.hpp"
#include "bsp/native_unit_health_message.hpp"

namespace bsp {
// Pure reference view of the ACTUAL transport fields. Constructing this view
// observes no field values, manufactures no list and invokes no callback.
struct NativeSessionTransportStepView {
    const std::uint32_t* const volatile& table_00;
    NativeSessionPrimaryPeerFields peers; // actual sentinel+0C/count+10 aliases
};

// Complete [00A42280,00A422B4). Native ECX transport, no arguments, RET.
// Validate initial first node against the CAPTURED initial head; advance
// before the fresh end test; update every remaining target's actual D4C.
// Each store uses a fresh MOVSS from the borrowed actual CE3910 float cell.
void step_host_native_session_peers_00a42280(const NativeSessionPrimaryPeerFields&,
    const volatile float& actual_peer_step_ce3910);

// Complete [00A41130,00A41131): a proved one-byte native RET, not a placeholder.
void step_client_native_session_transport_00a41130() noexcept;

// Complete [00782870,00782877) on the admitted actual host/client slot words.
// Native reads current table+1C and tail-JMPs. Source maps only A42280/A41130
// to the complete functions above; raw PE table entries are NOT callable
// Source profiles. Unsupported entries fail Source admission with logic_error.
void step_current_native_session_transport_00782870(const NativeSessionTransportStepView&,
    const volatile float& actual_peer_step_ce3910);

// New cdecl Source interfaces, not native class/ABI bindings. Borrow valid
// live finite backing. Host ordinary fixtures use nonempty lists; an empty
// initial list reaches the fixed current SDK invalid-parameter service. It
// may return, and continuation is retained. Historical BF6713 internals,
// private fault/EH behavior, whole-session lifetime and gameplay are unbound.
} // namespace bsp
