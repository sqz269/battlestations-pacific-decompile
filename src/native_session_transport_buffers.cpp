#include "bsp/native_session_transport_buffers.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session transport buffer reconstruction requires MSVC Win32.
#endif

namespace bsp {
NativeSessionTransportBuffersStorage* construct_native_session_transport_buffers_00783080(
    NativeSessionTransportBuffersStorage* member) {
    static_cast<volatile NativeSessionTransportBuffersStorage*>(member)->profile_00 = 0x00d04264u;
    NativeBitCursor* volatile* cell = member->cursors_d30;
    auto* backing = member->backing_04;
    for (std::uint32_t remaining = 3; remaining != 0; --remaining) {
        auto* const cursor = static_cast<NativeBitCursor*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x10, sizeof(NativeBitCursor)}));
        if (cursor) {
            volatile auto& c = *cursor;
            c.base_00 = backing;
            c.length_04 = 0x464;
            c.bit_0c = 0;
            *static_cast<volatile std::uint8_t*>(backing) = 0;
            c.current_08 = c.base_00;
        }
        *cell = cursor;
        ++cell;
        backing += 0x464;
    }
    return member;
}

void destroy_native_session_transport_buffers_007830f0(
    NativeSessionTransportBuffersStorage* member) noexcept {
    static_cast<volatile NativeSessionTransportBuffersStorage*>(member)->profile_00 = 0x00d04264u;
    NativeBitCursor* volatile* cell = member->cursors_d30;
    for (std::uint32_t remaining = 3; remaining != 0; --remaining) {
        singleton_lifetime_free(*cell);
        ++cell;
    }
}
} // namespace bsp
