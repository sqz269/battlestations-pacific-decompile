#pragma once
#include "bsp/native_bit_cursor_read.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual D3Ch member at session target+10h. Caller-owned backing and three
// separately allocated 10h cursors. No initialization or automatic destruction.
// profile_00 holds the recovered numeric stamp, not an executable host vtable.
struct NativeSessionTransportBuffersStorage {
    std::uint32_t profile_00;
    std::uint8_t backing_04[3 * 0x464];
    NativeBitCursor* cursors_d30[3];
};
static_assert(sizeof(NativeSessionTransportBuffersStorage) == 0xd3c);
static_assert(offsetof(NativeSessionTransportBuffersStorage, backing_04) == 4);
static_assert(offsetof(NativeSessionTransportBuffersStorage, cursors_d30) == 0xd30);

// Complete ordinary 00783080: ECX=member, no stack arguments, EAX=member, RET.
// Uses the existing BF681B host-CRT allocation boundary with both sizes=10h.
// A null allocation publishes null without touching that buffer. A throw leaves
// earlier publications and the remaining preimages; no catch/rollback is added.
// Historical CRT handler/exception identity and native vtable ABI are unbound.
NativeSessionTransportBuffersStorage* construct_native_session_transport_buffers_00783080(
    NativeSessionTransportBuffersStorage* actual_member);

// Complete ordinary 007830F0 through RET at 00783118: ECX=member, no arguments.
// Caller keeps the member and all reached cursor/backing consumers quiescent.
// Stamp profile, freshly load/free each of three cells ascending; no clearing
// or enclosing-target free. Stored Ghidra body metadata still truncates its
// returning-free tail; complete reviewed native bytes establish this boundary.
void destroy_native_session_transport_buffers_007830f0(
    NativeSessionTransportBuffersStorage* actual_member) noexcept;
} // namespace bsp
