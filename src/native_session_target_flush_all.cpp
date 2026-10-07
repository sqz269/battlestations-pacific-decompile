#include "bsp/native_session_target_flush_all.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {
void NativeUnitHealthMessageFlushAllCalls::flush_session_target_00783490(
    void* transport, NativeSessionTargetStorage* target) {
    using U = std::uint32_t;
    auto& section_field = target_critical_section_04(target);
    auto* const captured = section_field;
    ::EnterCriticalSection(&captured->native);
    auto& entry_depth = *reinterpret_cast<volatile U*>(&captured->depth);
    entry_depth = entry_depth + 1u;

    for (U delivery = 0; delivery != 3; ++delivery) {
        NativeBitCursor* const cursor = delivery_cursor_d40(target, delivery);
        volatile auto& c = *cursor;
        U bytes = (static_cast<U>(c.bit_0c) & 7u) != 0 ? 1u : 0u;
        bytes -= reinterpret_cast<U>(c.base_00);
        bytes += reinterpret_cast<U>(c.current_08);
        if (bytes == 0) continue;

        const volatile U* const table = transport_primary_table(transport);
        const U entry = table[0x20 / 4];
        call_transport_flush_20(entry, transport, target, cursor, delivery);
        auto* const base = c.base_00;
        c.current_08 = base;
        c.bit_0c = 0;
        *const_cast<volatile std::uint8_t*>(base) = 0;
    }

    auto* const current = section_field;
    auto& exit_depth = *reinterpret_cast<volatile U*>(&current->depth);
    exit_depth = exit_depth - 1u;
    ::LeaveCriticalSection(&current->native);
}
} // namespace bsp
