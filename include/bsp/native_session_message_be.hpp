#pragma once
#include "bsp/native_session_message_base.hpp"

namespace bsp {
// BEh leader-promotion message, profile00D02E5C. Padding11h..13h and1Bh is
// retained. The predicate also accepts46h; that does not establish a46h payload.
struct NativeSessionMessageBE {
    NativeSessionMessageStorage base;
    std::uint16_t sender_18;
    std::uint8_t relay_1a, retained_1b;
    std::int32_t member_index_1c;
};
static_assert(sizeof(NativeSessionMessageBE) == 0x20);
static_assert(offsetof(NativeSessionMessageBE, sender_18) == 0x18);
static_assert(offsetof(NativeSessionMessageBE, relay_1a) == 0x1a);
static_assert(offsetof(NativeSessionMessageBE, member_index_1c) == 0x1c);

// Five source thunks, in the native scalar-delete/write/read/predicate/true
// order. No borrowed context is needed by this profile. Original game ABI,
// allocator identity, network routing and gameplay remain unproved.
const std::uint32_t* native_session_message_be_profile_00d02e5c();
NativeSessionMessageBE* construct_native_session_message_be_zero_0075ad60(
    NativeSessionMessageBE*);
bool native_session_message_is_be_0075ada0(std::uint32_t type);
NativeSessionMessageBE* delete_native_session_message_be_0075adc0(
    NativeSessionMessageBE*, std::uint32_t flags);
void write_native_session_message_be_007ef6f0(const NativeSessionMessageBE*, NativeBitCursor*);
void read_native_session_message_be_007ef710(NativeSessionMessageBE*, NativeSessionReadStream*);

// Partial00768530: only BE arm00769AAB..00769ACBh and its virtual-reader join
// 0076A27B..0076A298h. Caller has already selectedBEh and rewound the selector
// byte; stream starts at the extended header. All other selector arms, native
// allocation failure/private EH and complete factory ABI are outside this API.
// Uses the existing throwing allocation provider. Owner+14h starts null;
// deferred delivery's sender/owner copy is a separate required route.
NativeSessionMessageBE* create_native_session_message_be_00769aab(NativeSessionReadStream*);
}  // namespace bsp
