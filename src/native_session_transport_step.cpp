#include "bsp/native_session_transport_step.hpp"

#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session transport step reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}
} // namespace

static_assert(offsetof(NativeSessionTargetStorage, field_d4c) == 0xd4c);

void step_host_native_session_peers_00a42280(const NativeSessionPrimaryPeerFields& peers,
    const volatile float& actual_peer_step) {
    const volatile float* const step = &actual_peer_step;
    auto* const initial_head = peers.sentinel_0c;
    auto* node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*initial_head).next_00;
    bool invalid = node == initial_head;
    for (;;) {
        if (invalid) current_crt_invalid_parameter();
        node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).next_00;
        if (node == peers.sentinel_0c) return;
        auto* const target = static_cast<NativeSessionTargetStorage*>(
            static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).peer_08);
        volatile std::uint32_t* const destination = &target->field_d4c;
        __asm {
            mov eax, step
            mov ecx, destination
            movss xmm0, dword ptr [eax]
            movss dword ptr [ecx], xmm0
        }
        invalid = node == peers.sentinel_0c;
    }
}

__declspec(naked) void step_client_native_session_transport_00a41130() noexcept {
    __asm { ret }
}

void step_current_native_session_transport_00782870(const NativeSessionTransportStepView& transport,
    const volatile float& actual_peer_step) {
    const volatile std::uint32_t* const table = transport.table_00;
    const std::uint32_t entry = table[0x1c / 4];
    if (entry == 0x00a42280u)
        step_host_native_session_peers_00a42280(transport.peers, actual_peer_step);
    else if (entry == 0x00a41130u)
        step_client_native_session_transport_00a41130();
    else
        throw std::logic_error("unsupported current native session transport +1C target");
}
} // namespace bsp
