#include "bsp/native_unit_health_message.hpp"
#include "bsp/random_threads.hpp"

#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session transport broadcast reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
__declspec(noinline) void __cdecl broadcast_invalid_parameter() {
    _invalid_parameter_noinfo();
}
} // namespace

void NativeUnitHealthMessageSerializedCalls::broadcast_session_message_00784830(
    void* transport, NativeSessionMessageStorage* message) {
    using U = std::uint32_t;
    static_assert(offsetof(TrackedCriticalSection, native) == 0);
    static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
    const auto peers = bind_peer_send_primary(transport);
    auto* const initial_head = peers.sentinel_0c;
    auto* node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*initial_head).next_00;
    for (;;) {
        // Native CMP EBP,EBP makes the initial owner-identity CRT guard dead.
        auto* const end = peers.sentinel_0c;
        if (node == end) return;
        if (node == peers.sentinel_0c) broadcast_invalid_parameter();
        void* const target = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).peer_08;
        auto& section_field = target_critical_section_04(target);
        auto* const captured = section_field;
        ::EnterCriticalSection(&captured->native);
        auto& enter_depth = *reinterpret_cast<volatile U*>(&captured->depth);
        enter_depth = enter_depth + 1u;
        serialize_session_message_00783c80(transport, target, message);
        auto* const current = section_field;
        auto& leave_depth = *reinterpret_cast<volatile U*>(&current->depth);
        leave_depth = leave_depth - 1u;
        ::LeaveCriticalSection(&current->native);
        if (node == peers.sentinel_0c) broadcast_invalid_parameter();
        node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).next_00;
    }
}
} // namespace bsp
