#include "bsp/native_unit_health_message.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include <new>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit health message profiles require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using M = NativeUnitHealthMessage;
M* __fastcall scalar_delete(M* message, void*, U flags) {
    return delete_native_unit_health_message_0075fe50(message, flags);
}
void __fastcall write(const M* message, void*, NativeBitCursor* cursor) {
    write_native_unit_health_message_0075fd10(message, cursor);
}
void __fastcall read(M* message, void*, NativeSessionReadStream* stream) {
    read_native_unit_health_message_0075fd60(message, stream);
}
bool __fastcall matches(const M*, void*, U type) {
    return native_unit_health_message_type_matches_0075fce0(type);
}
bool __fastcall always(const M*, void*) {
    return native_session_message_always_true_004499c0();
}
} // namespace

NativeUnitHealthMessageProfile::NativeUnitHealthMessageProfile(
    NativeUnitHealthMessageRelease& service) noexcept
    : slots{reinterpret_cast<U>(&scalar_delete), reinterpret_cast<U>(&write),
        reinterpret_cast<U>(&read), reinterpret_cast<U>(&matches), reinterpret_cast<U>(&always)},
      release(&service) {}

M* construct_native_unit_health_message_00876d30(M* message, U health,
    const NativeSessionMessageContext& context, const NativeUnitHealthMessageProfile& profile) {
    //00876D38: actual base initialization and fresh game-selected owner read.
    construct_native_session_message_0075b430(&message->base, 0xd2, context);
    volatile auto& m = *message;
    m.sender_18 = 0;
    m.relay_1a = 0;
    m.health_1c = health;
    m.base.delivery_04 = 1;
    m.base.profile_00 = profile.slots;
    return message;
}

M* delete_native_unit_health_message_0075fe50(M* message, U flags) {
    const bool deleting = (flags & 1u) != 0;
    // Borrowed source metadata must be captured while the D2 table is live.
    auto* release = reinterpret_cast<const NativeUnitHealthMessageProfile*>(
        static_cast<const volatile M&>(*message).base.profile_00)->release;
    static_cast<volatile M&>(*message).base.profile_00 =
        native_session_message_root_profile_00ce4974();
    if (deleting) release->release_message_00bf65ac(message);
    // Native0075FE69 explicitly restores EAX=ESI after the release call.
    return message;
}

void write_native_unit_health_message_0075fd10(const M* message, NativeBitCursor* cursor) {
    const volatile auto& m = *message;
    // Original ECX=message, stack cursor, RET4. Load each field at its call.
    write_native_byte_bits_00428ff0(cursor, m.base.type_10, 8);
    write_native_word_bits_00429120(cursor, m.sender_18, 12);
    write_native_bool_bit_004290b0(cursor, m.relay_1a);
    write_native_unsigned_dword_bits_00429070(cursor, m.health_1c, 8);
}

void read_native_unit_health_message_0075fd60(M* message, NativeSessionReadStream* stream) {
    // Original ECX=message, stack wrapper, RET4; cursor starts at wrapper+4.
    auto* cursor = &stream->cursor_04;
    read_native_u8_bits_00428c70(cursor, &message->base.type_10, 8);
    read_native_word_bits_00428e30(cursor, &message->sender_18, 12);
    read_native_bool_bit_00428d70(cursor, reinterpret_cast<bool*>(&message->relay_1a));
    read_native_u32_bits_00428d10(cursor, &message->health_1c, 8);
}

bool native_unit_health_message_type_matches_0075fce0(U type) {
    return type == 0xd2u || type == 0x46u;
}

NativeUnitHealthMessageConstructorCalls::NativeUnitHealthMessageConstructorCalls(
    NativeUnitHealthSetterGlobals globals, const NativeUnitHealthMessageProfile& profile) noexcept
    : globals_(globals), profile_(profile) {}

NativeUnitHealthSetterBinding NativeUnitHealthMessageConstructorCalls::bind(
    NativeUnitHealthSetterFields fields) noexcept {
    return {fields, globals_, *this};
}

void* NativeUnitHealthMessageConstructorCalls::construct_health_message_00876d30(
    NativeUnitHealthMessageFrame& frame, std::int32_t health) {
    // Default initialization begins the POD lifetime without clearing any byte.
    auto* message = ::new (static_cast<void*>(frame.bytes)) M;
    return construct_native_unit_health_message_00876d30(message, static_cast<U>(health),
        NativeSessionMessageContext{globals_.current_game_00e188a8}, profile_);
}
} // namespace bsp
