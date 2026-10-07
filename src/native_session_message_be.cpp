#include "bsp/native_session_message_be.hpp"

#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
using M = NativeSessionMessageBE;

M* __fastcall scalar_delete(M* m, void*, U flags) {
    return delete_native_session_message_be_0075adc0(m, flags);
}
void __fastcall write(const M* m, void*, NativeBitCursor* cursor) {
    write_native_session_message_be_007ef6f0(m, cursor);
}
void __fastcall read(M* m, void*, NativeSessionReadStream* stream) {
    read_native_session_message_be_007ef710(m, stream);
}
bool __fastcall is_be(const M*, void*, U type) {
    return native_session_message_is_be_0075ada0(type);
}
bool __fastcall always(const M*, void*) {
    return native_session_message_always_true_004499c0();
}
const U be_profile[] = {
    reinterpret_cast<U>(&scalar_delete), reinterpret_cast<U>(&write),
    reinterpret_cast<U>(&read), reinterpret_cast<U>(&is_be), reinterpret_cast<U>(&always)};
}  // namespace

const U* native_session_message_be_profile_00d02e5c() { return be_profile; }

M* construct_native_session_message_be_zero_0075ad60(M* message) {
    // 0075AD60..0075AD88h, ECX=this, RET; this receive-side constructor does
    // not call0075B430 or capture the game-selected owner used by emission.
    volatile auto& m = *message;
    m.base.field_08 = 0;
    m.base.field_0c = 0;
    m.base.selected_owner_14 = nullptr;
    m.base.type_10 = 0;
    m.sender_18 = 0;
    m.base.delivery_04 = 1;
    m.relay_1a = 0;
    m.base.profile_00 = be_profile;
    m.member_index_1c = 0;
    return message;
}

bool native_session_message_is_be_0075ada0(U type) { return type == 0xbeu || type == 0x46u; }

M* delete_native_session_message_be_0075adc0(M* message, U flags) {
    // Shared root-profile stamp and free-on-bit0 behavior; no payload teardown.
    return reinterpret_cast<M*>(delete_native_session_message_root_00449910(&message->base, flags));
}

void write_native_session_message_be_007ef6f0(const M* message, NativeBitCursor* cursor) {
    // Original ECX=message, stack cursor, RET4. Header precedes the three bits.
    write_native_session_message_header_0075b480(message, cursor);
    write_native_signed_dword_bits_00429090(
        cursor, static_cast<U>(static_cast<const volatile M&>(*message).member_index_1c), 3);
}

void read_native_session_message_be_007ef710(M* message, NativeSessionReadStream* stream) {
    // Original ECX=message, stack wrapper, RET4.00428D30 SIGN-EXTENDS bit2:
    // payload4..7 becomes -4..-1. Do not replace it with an unsigned reader.
    read_native_session_message_header_0075b4c0(message, stream);
    read_native_i32_bits_00428d30(&stream->cursor_04, &message->member_index_1c, 3);
}

M* create_native_session_message_be_00769aab(NativeSessionReadStream* stream) {
    // BE arm allocates20h at00769AAD, calls0075AD60 at00769ABF, and joins the
    // common CALL[profile+8] at0076A283. The allocator throws on failure.
    auto* message = static_cast<M*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x20, sizeof(M)}));
    construct_native_session_message_be_zero_0075ad60(message);
    using Reader = void(__thiscall*)(M*, NativeSessionReadStream*);
    reinterpret_cast<Reader>(message->base.profile_00[2])(message, stream);
    return message;
}
}  // namespace bsp
