#include "bsp/native_unit_health_message.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include <new>
#include <cstring>
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

NativeUnitHealthMessageFlags4Calls::NativeUnitHealthMessageFlags4Calls(
    NativeUnitHealthSetterGlobals globals, NativeUnitHealthRouteGlobals route_globals,
    const NativeUnitHealthMessageProfile& profile) noexcept
    : NativeUnitHealthMessageConstructorCalls(globals, profile),
      current_game_00e188a8_(globals.current_game_00e188a8), route_globals_(route_globals) {}

void NativeUnitHealthMessageFlags4Calls::route_health_message_0077c2a0(
    void* receiver, void* message, U flags, void** clear_on_local_delivery) {
    // This opt-in adapter is admitted only for00877B90's flags4 call. With4,
    // native local delivery and its out-pointer access are unreachable.
    (void)flags;
    (void)clear_on_local_delivery;
    route_d2_flags4_0077c2a0(receiver, static_cast<M*>(message));
}

void NativeUnitHealthMessageFlags4Calls::route_d2_flags4_0077c2a0(void* receiver, M* message) {
    auto* game = static_cast<std::byte*>(current_game_00e188a8_);
    if (game == nullptr) return;
    if (*reinterpret_cast<const volatile std::int32_t*>(game + 0x5d4) < 10) return;

    const auto fields = bind_route_fields(receiver);
    auto* table = primary_table(receiver);
    const U kind5 = table[0x5c / 4];
    if (call_entity_is_kind_5c(kind5, receiver, 5) && fields.player_slot_528 >= 0) {
        table = primary_table(receiver);
        const U kind1c = table[0x5c / 4];
        if (call_entity_is_kind_5c(kind1c, receiver, 0x1c)) {
            const volatile U* profile = static_cast<const volatile M&>(*message).base.profile_00;
            using Query = bool(__thiscall*)(const M*, U);
            (void)reinterpret_cast<Query>(profile[3])(message, 0xd3);
        }
    }

    //0077C302 ALWAYS reads the default before CMOVNZ selects the literal4.
    // Retain this actual volatile observation even though its value is dead.
    const U overridden_default = route_globals_.default_flags_00e0af1c;
    (void)overridden_default;
    game = static_cast<std::byte*>(current_game_00e188a8_);
    if (session_mode_1fe4(game) != 1) return;
    const auto second_mode = session_mode_1fe4(game);
    (void)second_mode;
    // The two native mode reads use this same captured game. With no callback
    // or concurrent write between them, the second is1 too. Flags4 therefore
    // reaches neither privilege query nor relay rewrite; no extra guard added.

    if (route_globals_.audit_latch_00e18db7 != 0) {
        const volatile U* profile = static_cast<const volatile M&>(*message).base.profile_00;
        using Query = bool(__thiscall*)(const M*, U);
        (void)reinterpret_cast<Query>(profile[3])(message, 0x49);
        // Actual active D2 predicate returnsfalse. The generic router's BYTE
        // write at+1C is outside this admitted D2 branch, not a default action.
    }

    //0077C39C/C3A2 observes a fresh game/mode before testing the host-send bit.
    // Flags4 has no bit2, but that memory read still occurs in the native path.
    game = static_cast<std::byte*>(current_game_00e188a8_);
    const auto host_mode = session_mode_1fe4(game);
    (void)host_mode;
    game = static_cast<std::byte*>(current_game_00e188a8_);
    if (session_mode_1fe4(game) != 1) return;

    auto* sentinel = fields.sentinel_2a8;
    auto* node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*sentinel).next_00;
    for (;;) {
        sentinel = fields.sentinel_2a8;
        // Native self-CMP atC3E0 makes only the C3EB violation call dead.
        if (node == sentinel) break;
        if (node == fields.sentinel_2a8) invalid_route_iterator_00bf6713();
        const auto sender = fields.sender_174;
        auto* peer = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).peer_08;
        static_cast<volatile M&>(*message).sender_18 = sender;
        void* target = peer_target_50(peer);
        game = static_cast<std::byte*>(current_game_00e188a8_);
        send_message_to_nonlocal_peer_00770b50(game + 0x1ef0, target, message);
        if (node == fields.sentinel_2a8) invalid_route_iterator_00bf6713();
        node = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*node).next_00;
    }
    // Flags4 has no local bit, so no enqueue, clone, out-clear or frame release.
}
void NativeUnitHealthMessagePeerSendCalls::send_message_to_nonlocal_peer_00770b50(
    void* session, void* target, M* message) {
    send_session_message_to_nonlocal_peer_00770b50(session, target, &message->base);
}

void NativeUnitHealthMessagePeerSendCalls::send_session_message_to_nonlocal_peer_00770b50(
    void* session, void* target, NativeSessionMessageStorage* message) {
    const auto fields = bind_peer_send_session(session);
    void* primary = fields.primary_transport_188;
    if (primary != nullptr) {
        const auto peers = bind_peer_send_primary(primary);
        void* local_target = nullptr;
        if (peers.peer_count_10 != 0) {
            auto* sentinel = peers.sentinel_0c;
            auto* first = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*sentinel).next_00;
            if (first == sentinel) invalid_route_iterator_00bf6713();
            local_target = static_cast<const volatile NativeUnitHealthRoutePeerNode&>(*first).peer_08;
        }
        if (local_target == target) return;
        primary = fields.primary_transport_188;
        send_locked_session_message_00783dc0(primary, target, message);
        return;
    }
    if (fields.secondary_transport_18c == nullptr) return;
    const volatile U* profile = static_cast<const volatile NativeSessionMessageStorage&>(*message).profile_00;
    using Query = bool(__thiscall*)(const NativeSessionMessageStorage*, U);
    if (!reinterpret_cast<Query>(profile[3])(message, 0x29)) return;
    void* secondary = fields.secondary_transport_18c;
    send_locked_session_message_00783dc0(secondary, target, message);
}
void NativeUnitHealthMessageLockedSendCalls::send_locked_session_message_00783dc0(
    void* transport, void* target, NativeSessionMessageStorage* message) {
    static_assert(sizeof(TrackedCriticalSection) == 0x1c);
    static_assert(offsetof(TrackedCriticalSection, native) == 0);
    static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
    auto& section_field = target_critical_section_04(target);
    auto* captured = section_field;
    ::EnterCriticalSection(&captured->native);  // native IAT00CE2218
    auto& enter_depth = *reinterpret_cast<volatile U*>(&captured->depth);
    enter_depth = enter_depth + 1u;
    serialize_session_message_00783c80(transport, target, message);
    auto* current = section_field;  //00783DE4 reloads target+4 after serialization
    auto& leave_depth = *reinterpret_cast<volatile U*>(&current->depth);
    leave_depth = leave_depth - 1u;
    ::LeaveCriticalSection(&current->native);  // native IAT00CE2210
}
namespace {
std::int32_t message_signed_bits(U bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
U message_cursor_position(const NativeBitCursor* cursor) noexcept {
    const volatile auto& c = *cursor;
    const U current = reinterpret_cast<U>(c.current_08);
    const U base = reinterpret_cast<U>(c.base_00);
    const U bit = static_cast<U>(c.bit_0c);
    return (current - base) * 8u + bit;
}
void reset_message_cursor(NativeBitCursor* cursor) noexcept {
    volatile auto& c = *cursor;
    const auto* base = c.base_00;
    c.current_08 = base;
    c.bit_0c = 0;
    *const_cast<volatile std::uint8_t*>(base) = 0;
}
std::uint16_t message_delivery_word(const NativeSessionMessageStorage* message) noexcept {
    return *reinterpret_cast<const volatile std::uint16_t*>(
        reinterpret_cast<const std::uint8_t*>(message) + 4);
}
}

void copy_native_message_bit_slice_00428ec0(const NativeBitCursor* cursor,
    void* destination, U absolute_bit, U bits) {
    using B = std::uint8_t;
    const volatile auto& c = *cursor;
    auto* out = static_cast<volatile B*>(destination);
    U source_byte = absolute_bit >> 3;
    U source_bit = 7u - (absolute_bit & 7u);
    U output_bit = 7;
    while (bits != 0) {
        --bits;
        if (output_bit == 7) *out = 0;
        const U base = reinterpret_cast<U>(c.base_00); // reload after byte clear
        const B value = *reinterpret_cast<const volatile B*>(base + source_byte);
        if (((static_cast<U>(value) >> source_bit) & 1u) != 0)
            *out = static_cast<B>(*out + static_cast<B>(1u << output_bit));
        if (output_bit == 0) {
            output_bit = 7;
            out = reinterpret_cast<volatile B*>(reinterpret_cast<U>(out) + 1u);
        } else --output_bit;
        if (source_bit == 0) { source_bit = 7; ++source_byte; }
        else --source_bit;
    }
}

void append_native_message_msb_bits_00429540(NativeBitCursor* cursor,
    const void* source, U bits) {
    using B = std::uint8_t;
    volatile auto& c = *cursor;
    auto* in = static_cast<const volatile B*>(source);
    U source_bit = 7;
    while (bits != 0) {
        const B input = *in;
        auto* out = const_cast<volatile B*>(c.current_08);
        const B value = static_cast<B>((static_cast<U>(input) >> source_bit) & 1u);
        const U shift = static_cast<U>(c.bit_0c);
        --bits;
        *out = static_cast<B>(*out | static_cast<B>(static_cast<U>(value) << (shift & 31u)));
        // Native reloads the actual low byte after the possibly aliased OR.
        const B low_bit = *reinterpret_cast<const volatile B*>(&c.bit_0c);
        const U carry_shift = (8u - static_cast<U>(low_bit)) & 31u;
        c.bit_0c = message_signed_bits(static_cast<U>(c.bit_0c) + 1u);
        const B carry = static_cast<B>(static_cast<U>(value) >> carry_shift);
        const auto offset = c.bit_0c;
        if (offset > 7) {
            c.current_08 = reinterpret_cast<const B*>(reinterpret_cast<U>(c.current_08) + 1u);
            c.bit_0c = message_signed_bits(static_cast<U>(offset) - 8u);
            *const_cast<volatile B*>(c.current_08) = carry;
        }
        if (source_bit == 0) {
            in = reinterpret_cast<const volatile B*>(reinterpret_cast<U>(in) + 1u);
            source_bit = 7;
        } else --source_bit;
    }
}

NativeUnitHealthMessageSerializedCalls::NativeUnitHealthMessageSerializedCalls(
    NativeUnitHealthSetterGlobals globals, NativeUnitHealthRouteGlobals route_globals,
    const NativeUnitHealthMessageProfile& profile,
    const volatile std::uint16_t& tick_low) noexcept
    : NativeUnitHealthMessageLockedSendCalls(globals, route_globals, profile),
      tick_low_00f876b0_(tick_low) {}

void NativeUnitHealthMessageSerializedCalls::serialize_session_message_00783c80(
    void* transport, void* target, NativeSessionMessageStorage* message) {
    std::uint8_t scratch[0x200]; // saved native transport DWORD is not scratch
    volatile auto& m = *message;
    const U initial_delivery = m.delivery_04;
    auto* const cursor = delivery_cursor_d40(target, initial_delivery);
    U before = message_cursor_position(cursor);
    if (before == 0) {
        write_native_signed_word_bits_00429030(cursor, tick_low_00f876b0_, 16);
        write_native_signed_word_bits_00429030(cursor, message_delivery_word(message), 3);
        before = message_cursor_position(cursor);
    }
    const volatile U* profile = m.profile_00;
    using Writer = void(__thiscall*)(const NativeSessionMessageStorage*, NativeBitCursor*);
    reinterpret_cast<Writer>(profile[1])(message, cursor);
    const U after = message_cursor_position(cursor);
    if (message_signed_bits(after + 0x20u) >= 0x2320) {
        const U delta = after - before;
        copy_native_message_bit_slice_00428ec0(cursor, scratch, before, delta);
        rewind_native_bits_00428b80(cursor, delta);
        const U delivery = m.delivery_04;
        const volatile U* table = transport_primary_table(transport);
        const U entry = table[0x20 / 4];
        call_transport_flush_20(entry, transport, target, cursor, delivery);
        reset_message_cursor(cursor);
        write_native_signed_word_bits_00429030(cursor, tick_low_00f876b0_, 16);
        write_native_signed_word_bits_00429030(cursor, message_delivery_word(message), 3);
        append_native_message_msb_bits_00429540(cursor, scratch, delta);
    }
    const U delivery = m.delivery_04;
    if (delivery != 2) {
        const volatile auto& c = *cursor;
        const U partial = (static_cast<U>(c.bit_0c) & 7u) != 0 ? 1u : 0u;
        const U base = reinterpret_cast<U>(c.base_00);
        const U current = reinterpret_cast<U>(c.current_08);
        if (partial - base + current < 0x380u) return;
    }
    const volatile U* table = transport_primary_table(transport);
    const U entry = table[0x20 / 4];
    call_transport_flush_20(entry, transport, target, cursor, delivery);
    reset_message_cursor(cursor);
}
} // namespace bsp
