#include "bsp/native_session_peer_flush.hpp"

#include <cstdlib>

namespace bsp {
namespace {
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}
} // namespace

void NativeUnitHealthMessagePeerFlushCalls::flush_session_peers_007848f0(
    void* transport) {
    const auto peers = bind_peer_send_primary(transport);
    auto* const initial_head = peers.sentinel_0c;
    auto* node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(
        *initial_head).next_00;
    for (;;) {
        auto* const end_head = peers.sentinel_0c;
        if (node == end_head) return;
        if (node == peers.sentinel_0c) current_crt_invalid_parameter();
        void* const target = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(
            *node).peer_08;
        flush_session_target_00783490(transport,
            static_cast<NativeSessionTargetStorage*>(target));
        if (node == peers.sentinel_0c) current_crt_invalid_parameter();
        node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).next_00;
    }
}
} // namespace bsp
