// Packet cc13-seg62, segment 65 (0097AB70..0098C510): the mission-event
// subscriptions, the warnings and the warning manager. See
// include/bsp/cc13_seg62.hpp and docs/CC13_SEG62.md. Names are hypotheses;
// build-tested only.

#include "bsp/cc13_seg62.hpp"

#include <cstring>

namespace bsp::cc13_seg62 {

namespace {

using BuyHead = Object (Seg62Host::*)(Object);
using EraseTree = void (Seg62Host::*)(Object, Object, Object, Object, Object, Object);

// A subscription is {vtable +0, native string +4 {size, data}, sets from +0Ch}.
// Each set is {set vtable +0, tree +4 {head +8, size +0Ch}}; its tree is
// built as every constructor inlines it: the head from the set's allocator,
// its "is nil" byte set, its left (+0), parent (+4) and right (+8) pointing at
// itself (written +4, +0, +8), and the size cleared.
void init_set(Seg62Host& host, Object subscription, std::size_t set, std::uint32_t set_vtable, BuyHead buy,
              std::size_t nil_byte) {
    at<std::uint32_t>(subscription, set) = set_vtable;
    const Object tree = subscription + set + 0x4;
    const Object head = (host.*buy)(tree);
    at<std::uint32_t>(tree, 0x4) = addr(head);
    at<std::uint8_t>(head, nil_byte) = 1;
    at<std::uint32_t>(obj(at<std::uint32_t>(tree, 0x4)), 0x4) = at<std::uint32_t>(tree, 0x4);
    at<std::uint32_t>(obj(at<std::uint32_t>(tree, 0x4)), 0x0) = at<std::uint32_t>(tree, 0x4);
    at<std::uint32_t>(obj(at<std::uint32_t>(tree, 0x4)), 0x8) = at<std::uint32_t>(tree, 0x4);
    at<std::uint32_t>(tree, 0x8) = 0;
}

// The destructors' inlined tree teardown: erase [[head], head], free the
// head (00BF65AC), clear head and size.
void destroy_set(Seg62Host& host, Object subscription, std::size_t set, EraseTree erase) {
    const Object tree = subscription + set + 0x4;
    const Object head = obj(at<std::uint32_t>(tree, 0x4));
    std::uint32_t out[2];
    (host.*erase)(tree, reinterpret_cast<Object>(out), tree, obj(at<std::uint32_t>(head, 0x0)), tree, head);
    host.free_00bf65ac(obj(at<std::uint32_t>(tree, 0x4)));
    at<std::uint32_t>(tree, 0x4) = 0;
    at<std::uint32_t>(tree, 0x8) = 0;
}

// The common constructor head: the string cleared, the vtable installed.
void begin_construct(Object subscription, std::uint32_t vtable) {
    at<std::uint32_t>(subscription, 0x4) = 0;
    at<std::uint32_t>(subscription, 0x8) = 0;
    at<std::uint32_t>(subscription, 0x0) = vtable;
}

// The common destructor tail: the base vtable 00D1AF88 and the string's
// block returned to the sized pool (00419CC0 / 00BD1510(data, size + 1, 1)).
void end_destruct(Seg62Host& host, Object subscription) {
    at<std::uint32_t>(subscription, 0x0) = 0x00d1af88;
    const std::uint32_t data = at<std::uint32_t>(subscription, 0x8);
    if (data != 0) {
        const Object pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, obj(data), at<std::uint32_t>(subscription, 0x4) + 1, 1);
    }
}

// The set kinds: (set vtable, head allocator, nil byte, eraser).
constexpr std::uint32_t kSetA = 0x00d1b670;  // 0096BA00, nil +0Fh, 00971840
constexpr std::uint32_t kSetB = 0x00d1b674;  // 0096BA50, nil +11h, 00971910
constexpr std::uint32_t kSetC = 0x00d1b678;  // 0096BAA0, nil +0Eh, 009719E0
constexpr std::uint32_t kSetS = 0x00d1b73c;  // 004C26B0, nil +15h, 004D1A50 (string keys)

void set_a(Seg62Host& host, Object s, std::size_t set) { init_set(host, s, set, kSetA, &Seg62Host::tree_buy_head_0096ba00, 0xf); }
void set_b(Seg62Host& host, Object s, std::size_t set) { init_set(host, s, set, kSetB, &Seg62Host::tree_buy_head_0096ba50, 0x11); }
void set_c(Seg62Host& host, Object s, std::size_t set) { init_set(host, s, set, kSetC, &Seg62Host::tree_buy_head_0096baa0, 0xe); }
void set_s(Seg62Host& host, Object s, std::size_t set) { init_set(host, s, set, kSetS, &Seg62Host::tree_buy_head_004c26b0, 0x15); }
void kill_a(Seg62Host& host, Object s, std::size_t set) { destroy_set(host, s, set, &Seg62Host::tree_erase_00971840); }
void kill_b(Seg62Host& host, Object s, std::size_t set) { destroy_set(host, s, set, &Seg62Host::tree_erase_00971910); }
void kill_c(Seg62Host& host, Object s, std::size_t set) { destroy_set(host, s, set, &Seg62Host::tree_erase_009719e0); }
void kill_s(Seg62Host& host, Object s, std::size_t set) { destroy_set(host, s, set, &Seg62Host::tree_erase_004d1a50); }

}  // namespace

// The subscription constructors are __fastcall(ECX = this) -> EAX = this and
// the destructor bodies __fastcall(ECX = this), RET; each called from its
// factory in 0097E360 or its scalar deleting destructor. SEH frames are not
// modelled. Sets are listed by offset with their kind (A/B/C/S above).

// 0097AB70 "fire" destructor: set A +0Ch.
void fire_subscription_destruct_0097ab70(Seg62Host& host, Object s) {
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097AC10 / 0097ACC0 "repair" (vtable 00D1B6EC): A +0Ch, B +1Ch.
Object repair_subscription_construct_0097ac10(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b6ec);
    set_a(host, s, 0xc);
    set_b(host, s, 0x1c);
    return s;
}
void repair_subscription_destruct_0097acc0(Seg62Host& host, Object s) {
    kill_b(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097AD80 / 0097AE30 "target" (00D1B6FC): A +0Ch, A +1Ch.
Object target_subscription_construct_0097ad80(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b6fc);
    set_a(host, s, 0xc);
    set_a(host, s, 0x1c);
    return s;
}
void target_subscription_destruct_0097ae30(Seg62Host& host, Object s) {
    kill_a(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097AEF0 / 0097AFA0 "ammoType" (00D1B70C): A +0Ch, B +1Ch.
Object ammo_type_subscription_construct_0097aef0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b70c);
    set_a(host, s, 0xc);
    set_b(host, s, 0x1c);
    return s;
}
void ammo_type_subscription_destruct_0097afa0(Seg62Host& host, Object s) {
    kill_b(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097B060 / 0097B140 "stock" (00D1B71C): A +0Ch, B +1Ch, B +2Ch.
Object stock_subscription_construct_0097b060(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b71c);
    set_a(host, s, 0xc);
    set_b(host, s, 0x1c);
    set_b(host, s, 0x2c);
    return s;
}
void stock_subscription_destruct_0097b140(Seg62Host& host, Object s) {
    kill_b(host, s, 0x2c);
    kill_b(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097B230 / 0097B2B0 "shipLanded" (00D1B72C): A +0Ch.
Object ship_landed_subscription_construct_0097b230(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b72c);
    set_a(host, s, 0xc);
    return s;
}
void ship_landed_subscription_destruct_0097b2b0(Seg62Host& host, Object s) {
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097C2C0 / 0097C410 "hit" (00D1B740): A +0Ch, A +1Ch, A +2Ch, S +3Ch,
// B +4Ch, and three 0Ch-byte members at +5Ch, +68h, +74h that get only their
// vtable 00D1AF44 (the destructor does not touch them).
Object hit_subscription_construct_0097c2c0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b740);
    set_a(host, s, 0xc);
    set_a(host, s, 0x1c);
    set_a(host, s, 0x2c);
    set_s(host, s, 0x3c);
    set_b(host, s, 0x4c);
    at<std::uint32_t>(s, 0x5c) = 0x00d1af44;
    at<std::uint32_t>(s, 0x68) = 0x00d1af44;
    at<std::uint32_t>(s, 0x74) = 0x00d1af44;
    return s;
}
void hit_subscription_destruct_0097c410(Seg62Host& host, Object s) {
    kill_b(host, s, 0x4c);
    kill_s(host, s, 0x3c);
    kill_a(host, s, 0x2c);
    kill_a(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097C560 / 0097C610 "failure" (00D1B750): A +0Ch, S +1Ch, and a 0Ch-byte
// member +2Ch with only its vtable 00D1AF44.
Object failure_subscription_construct_0097c560(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b750);
    set_a(host, s, 0xc);
    set_s(host, s, 0x1c);
    at<std::uint32_t>(s, 0x2c) = 0x00d1af44;
    return s;
}
void failure_subscription_destruct_0097c610(Seg62Host& host, Object s) {
    kill_s(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097C6D0 / 0097C7B0 "zone" (00D1B760): A +0Ch, S +1Ch, C +2Ch.
Object zone_subscription_construct_0097c6d0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b760);
    set_a(host, s, 0xc);
    set_s(host, s, 0x1c);
    set_c(host, s, 0x2c);
    return s;
}
void zone_subscription_destruct_0097c7b0(Seg62Host& host, Object s) {
    kill_c(host, s, 0x2c);
    kill_s(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097C8A0 / 0097C9B0 "commandEvent" (00D1B770): A +0Ch, A +1Ch, S +2Ch, S +3Ch.
Object command_event_subscription_construct_0097c8a0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b770);
    set_a(host, s, 0xc);
    set_a(host, s, 0x1c);
    set_s(host, s, 0x2c);
    set_s(host, s, 0x3c);
    return s;
}
void command_event_subscription_destruct_0097c9b0(Seg62Host& host, Object s) {
    kill_s(host, s, 0x3c);
    kill_s(host, s, 0x2c);
    kill_a(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097CAD0 / 0097CB80 "gui" (00D1B780): S +0Ch, C +1Ch.
Object gui_subscription_construct_0097cad0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b780);
    set_s(host, s, 0xc);
    set_c(host, s, 0x1c);
    return s;
}
void gui_subscription_destruct_0097cb80(Seg62Host& host, Object s) {
    kill_c(host, s, 0x1c);
    kill_s(host, s, 0xc);
    end_destruct(host, s);
}

// 0097CC40 / 0097CCF0 "generate" (00D1B790): S +0Ch, B +1Ch.
Object generate_subscription_construct_0097cc40(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b790);
    set_s(host, s, 0xc);
    set_b(host, s, 0x1c);
    return s;
}
void generate_subscription_destruct_0097ccf0(Seg62Host& host, Object s) {
    kill_b(host, s, 0x1c);
    kill_s(host, s, 0xc);
    end_destruct(host, s);
}

// 0097CDB0 / 0097CE60 "player" (00D1B7A0): B +0Ch, S +1Ch.
Object player_subscription_construct_0097cdb0(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b7a0);
    set_b(host, s, 0xc);
    set_s(host, s, 0x1c);
    return s;
}
void player_subscription_destruct_0097ce60(Seg62Host& host, Object s) {
    kill_s(host, s, 0x1c);
    kill_b(host, s, 0xc);
    end_destruct(host, s);
}

// 0097CF20 / 0097CFA0 "musicOver" (00D1B7B0): S +0Ch.
Object music_over_subscription_construct_0097cf20(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b7b0);
    set_s(host, s, 0xc);
    return s;
}
void music_over_subscription_destruct_0097cfa0(Seg62Host& host, Object s) {
    kill_s(host, s, 0xc);
    end_destruct(host, s);
}

// 0097D040 / 0097D120 "chat" (00D1B7C0): B +0Ch, S +1Ch, B +2Ch.
Object chat_subscription_construct_0097d040(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b7c0);
    set_b(host, s, 0xc);
    set_s(host, s, 0x1c);
    set_b(host, s, 0x2c);
    return s;
}
void chat_subscription_destruct_0097d120(Seg62Host& host, Object s) {
    kill_b(host, s, 0x2c);
    kill_s(host, s, 0x1c);
    kill_b(host, s, 0xc);
    end_destruct(host, s);
}

// 0097D210 / 0097D2C0 "entityKilled" (00D1B7D0): A +0Ch, S +1Ch.
Object entity_killed_subscription_construct_0097d210(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b7d0);
    set_a(host, s, 0xc);
    set_s(host, s, 0x1c);
    return s;
}
void entity_killed_subscription_destruct_0097d2c0(Seg62Host& host, Object s) {
    kill_s(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097D380 / 0097D430 "hpEvent" (00D1B7E0): A +0Ch, S +1Ch.
Object hp_event_subscription_construct_0097d380(Seg62Host& host, Object s) {
    begin_construct(s, 0x00d1b7e0);
    set_a(host, s, 0xc);
    set_s(host, s, 0x1c);
    return s;
}
void hp_event_subscription_destruct_0097d430(Seg62Host& host, Object s) {
    kill_s(host, s, 0x1c);
    kill_a(host, s, 0xc);
    end_destruct(host, s);
}

// 0097B4D0 __thiscall(ECX = warning, reader), RET 4. Slot 1 of the string-list
// warning vtable 00D1B4F8 (pairs with the writer 0096D330). 009768F0 loads the
// base warning; the reader enters "msg" (00CE4AD8); for i = 0, 1, ... while
// vtable[14h]({1, i}) answers true, a local native string is read with
// vtable[10h]({1, i}, {0, &string}), appended to the vector +6Ch (00450540)
// and released to the pool; then the reader leaves the table. SEH 00CAB3C8
// not modelled.
void string_list_warning_load_0097b4d0(Seg62Host& host, Object warning, Object reader) {
    host.warning_load_archive_009768f0(warning, reader);
    host.reader_vt4_enter(reader, TaggedValue{0, addr("msg")});
    for (std::uint32_t i = 0; host.reader_vt14_has(reader, TaggedValue{1, i}); ++i) {
        std::uint32_t text[2] = {0, 0};
        host.reader_vt10_read(reader, TaggedValue{1, i}, TaggedValue{0, addr(text)});
        host.string_vector_append_00450540(warning + 0x6c, reinterpret_cast<Object>(text));
        if (text[1] != 0) {
            const Object pool = host.pool_singleton_00419cc0();
            host.pool_return_block_00bd1510(pool, obj(text[1]), text[0] + 1, 1);
        }
    }
    host.reader_vt8_leave(reader);
}

// 0097B960 ECX = warning manager, stack (message, unused), RET 8. The session
// message E7h handler (caller 007782FE, BSP_Session_PumpReceiveQueue). A local
// vector<string> {proxy, first, last, end} is cleared, the message's string
// +18h appended (00450540), and 00976CB0(&vector, 0, 0) reports it; the
// vector's strings are destroyed (00432050 with ECX = first, EDX = last and the
// image's two unused words: the vector address and the message) and its
// storage freed (00BF65AC). SEH 00CAB418 not modelled.
void session_warning_string_message_0097b960(Seg62Host& host, Object manager, Object message) {
    std::uint32_t strings[4] = {0, 0, 0, 0};
    const Object vector = reinterpret_cast<Object>(strings);
    host.string_vector_append_00450540(vector, message + 0x18);
    host.warning_report_first_string_00976cb0(manager, vector, 0, 0);
    if (strings[1] != 0) {
        host.string_range_destroy_00432050(obj(strings[1]), obj(strings[2]), addr(vector), addr(message));
        host.free_00bf65ac(obj(strings[1]));
    }
}

namespace {

constexpr std::uint32_t kWarningManager = 0x00f8a0c4;  // [00F8A0C4], the warning manager
constexpr std::uint32_t kClock = 0x00f876a4;           // the float clock

// The activates' common head: +0Ch = 1, +10h = [00F876A4] (MOVSS copy).
void mark_active(Seg62Host& host, Object warning) {
    at<std::uint32_t>(warning, 0xc) = 1;
    at<float>(warning, 0x10) = host.global_f32(kClock);
}

// Looks `key` up in the manager's name map (+108h, 008EC460), picks a variant
// (00979B50) and copies it into the local native string `out` {0, 0} (resize
// with fill 1, memcpy when the source size is non-zero; skipped when the
// variant is the local itself).
void copy_picked_variant(Seg62Host& host, Object key, std::uint32_t* out) {
    const Object manager = obj(host.global_u32(kWarningManager));
    const std::uint32_t entry = host.name_map_lookup_008ec460(manager + 0x108, key);
    const Object picked = host.pick_variant_00979b50(manager, entry);
    out[0] = 0;
    out[1] = 0;
    if (reinterpret_cast<Object>(out) == picked) return;
    host.string_resize_0041dd40(reinterpret_cast<Object>(out), at<std::uint32_t>(picked, 0x0), 1);
    if (at<std::uint32_t>(picked, 0x0) != 0) host.memcpy_00bf7680(obj(out[1]), obj(at<std::uint32_t>(picked, 0x4)), out[0]);
}

// 0097D650 on the local message with an entity (ECX = [00F8A0C4]).
void resolve_sounds(Seg62Host& host, std::uint32_t* text, std::uint32_t entity) {
    host.resolve_entity_sound_keys_0097d650(obj(host.global_u32(kWarningManager)), reinterpret_cast<Object>(text),
                                            obj(entity));
}

// The kill/hit test: a set entity whose relation to the local party (record
// [game + 18CCh + [game+18ECh]*4], party object at +28h; 00803CE0) is 2 adds
// no message.
bool relation_blocks(Seg62Host& host, std::uint32_t entity) {
    if (entity == 0) return false;
    const Object game = obj(host.global_u32(0x00e188a8));
    const Object record = obj(at<std::uint32_t>(game, 0x18cc + at<std::int32_t>(game, 0x18ec) * 4));
    return host.recon_relation_00803ce0(obj(at<std::uint32_t>(record, 0x28)), obj(entity)) == 2;
}

void release_local(Seg62Host& host, const std::uint32_t* text) {
    if (text[1] == 0) return;
    const Object pool = host.pool_singleton_00419cc0();
    host.pool_return_block_00bd1510(pool, obj(text[1]), text[0] + 1, 1);
}

// The plain activate: copy the picked variant of key +4h, add it, release.
void activate_plain(Seg62Host& host, Object warning) {
    mark_active(host, warning);
    std::uint32_t text[2];
    copy_picked_variant(host, warning + 0x4, text);
    host.warning_add_message_00976b00(warning, reinterpret_cast<Object>(text));
    release_local(host, text);
}

// The kill-style activate: sounds resolved for the entity at `entity_offset`,
// and no message when the relation test blocks it.
void activate_relation(Seg62Host& host, Object warning, std::size_t entity_offset) {
    mark_active(host, warning);
    std::uint32_t text[2];
    copy_picked_variant(host, warning + 0x4, text);
    resolve_sounds(host, text, at<std::uint32_t>(warning, entity_offset));
    if (!relation_blocks(host, at<std::uint32_t>(warning, entity_offset))) {
        host.warning_add_message_00976b00(warning, reinterpret_cast<Object>(text));
    }
    release_local(host, text);
}

}  // namespace

// The warning Activate slots (vtable slot 3), all __fastcall(ECX = warning),
// RET, with SEH frames not modelled. The variant pick (00979B50) draws from
// BSP_RandomThreads_NextU32 (00BD2FC0), not the gameplay stream 00BD2F10.

// 0097F490, type-1 entity warning (vtable 00D1B26C): the plain activate.
void entity_warning_activate_0097f490(Seg62Host& host, Object warning) { activate_plain(host, warning); }

// 0097F7E0, recon warning (00D1B2A8). With the byte +98h set the key is +4h
// concatenated with "player" (00D199D0; 0041E870, 004261A0), and the two
// temporaries are released after the copy (the concatenation first). Sounds
// are resolved for +80h; the message is added unconditionally.
void recon_warning_activate_0097f7e0(Seg62Host& host, Object warning) {
    mark_active(host, warning);
    std::uint32_t player[2];
    std::uint32_t joined[2];
    const bool suffixed = at<std::uint8_t>(warning, 0x98) != 0;
    Object key = warning + 0x4;
    if (suffixed) {
        host.string_construct_0041e870(reinterpret_cast<Object>(player), "player");
        key = host.string_concat_004261a0(warning + 0x4, reinterpret_cast<Object>(joined), reinterpret_cast<Object>(player));
    }
    std::uint32_t text[2];
    copy_picked_variant(host, key, text);
    if (suffixed) {
        release_local(host, joined);
        release_local(host, player);
    }
    resolve_sounds(host, text, at<std::uint32_t>(warning, 0x80));
    host.warning_add_message_00976b00(warning, reinterpret_cast<Object>(text));
    release_local(host, text);
}

// 0097F970, kill warning (00D1B2CC): victim +6Ch, relation test.
void kill_warning_activate_0097f970(Seg62Host& host, Object warning) { activate_relation(host, warning, 0x6c); }

// 0097FA90, hit warning: entity +80h, relation test.
void hit_warning_activate_0097fa90(Seg62Host& host, Object warning) { activate_relation(host, warning, 0x80); }

// 0097FBB0, exit-zone warning (00D1B344): sounds for +80h, then the message.
void exit_zone_warning_activate_0097fbb0(Seg62Host& host, Object warning) {
    mark_active(host, warning);
    std::uint32_t text[2];
    copy_picked_variant(host, warning + 0x4, text);
    resolve_sounds(host, text, at<std::uint32_t>(warning, 0x80));
    host.warning_add_message_00976b00(warning, reinterpret_cast<Object>(text));
    release_local(host, text);
}

// 0097FCB0, confirm warning (00D1B380): the plain activate.
void confirm_warning_activate_0097fcb0(Seg62Host& host, Object warning) { activate_plain(host, warning); }

// 0097FD90, new-unit warning (00D1B460): the plain activate.
void new_unit_warning_activate_0097fd90(Seg62Host& host, Object warning) { activate_plain(host, warning); }

// 0097FE70, type-9 warning (00D1B484): entity +6Ch, relation test.
void type9_warning_activate_0097fe70(Seg62Host& host, Object warning) { activate_relation(host, warning, 0x6c); }

// 0097FF90, power-up warning (00D1B4B0): the plain activate.
void powerup_warning_activate_0097ff90(Seg62Host& host, Object warning) { activate_plain(host, warning); }

// 00980070, the warning whose vtable slot is 00D1B4E0: the plain activate.
void warning_activate_00980070(Seg62Host& host, Object warning) { activate_plain(host, warning); }

namespace {

// One channel argument as the dispatchers take it from their stack: the raw
// stack word and its kind. The kind fixes the 8- or 0Ch-byte argument object
// the image allocates and the Lua variant constructor it uses.
enum class ArgKind : std::uint8_t {
    Entity,  // object vtable 00D1AF24, +4h word = entity ? [entity+174h] : 0; Lua 00884A90(entity)
    Int,     // object vtable 00D1AF2C, +4h = value; Lua 00884A00(value)
    String,  // a NativeString*: 0Ch-byte object, 00426060 copy on the stack, 0096DC00; Lua 00884FC0(string*)
    Bool,    // object vtable 00D1AF34, +4h = (byte)value; Lua 00884A30(whole word)
    Float,   // object vtable 00D1AF3C, +4h = the float; Lua 008849D0(float)
    EntityObserved,  // as Entity, but built inside an allocated object's scope with a stack
                     // observer of the entity (006E0E00, vtable then 00CF938C; destroyed by
                     // 006E0860 after the push), 00984800's second argument
};

struct ChannelArg {
    ArgKind kind;
    std::uint32_t raw;
};

// The argument object the image builds for one argument (operator new; the
// constructor runs only on a non-null allocation, and a null result is pushed).
Object build_arg_object(Seg62Host& host, const ChannelArg& arg, Object observer) {
    if (arg.kind == ArgKind::String) {
        const Object object = host.operator_new_00bf681b(0xc);
        if (object == nullptr) return nullptr;
        std::uint32_t by_value[2];
        host.string_copy_construct_00426060(reinterpret_cast<Object>(by_value), obj(arg.raw));
        return host.string_arg_construct_0096dc00(object, reinterpret_cast<Object>(by_value));
    }
    const Object object = host.operator_new_00bf681b(0x8);
    if (object == nullptr) return nullptr;
    switch (arg.kind) {
    case ArgKind::Entity:
        at<std::uint32_t>(object, 0x0) = 0x00d1af24;
        at<std::uint16_t>(object, 0x4) = arg.raw != 0 ? at<std::uint16_t>(obj(arg.raw), 0x174) : 0;
        break;
    case ArgKind::Int:
        at<std::uint32_t>(object, 0x0) = 0x00d1af2c;
        at<std::uint32_t>(object, 0x4) = arg.raw;
        break;
    case ArgKind::Bool:
        at<std::uint32_t>(object, 0x0) = 0x00d1af34;
        at<std::uint32_t>(object, 0x4) = static_cast<std::uint8_t>(arg.raw);
        break;
    case ArgKind::Float:
        at<std::uint32_t>(object, 0x0) = 0x00d1af3c;
        at<std::uint32_t>(object, 0x4) = arg.raw;
        break;
    case ArgKind::EntityObserved: {
        host.observer_construct_006e0e00(observer, obj(arg.raw));
        at<std::uint32_t>(observer, 0x0) = 0x00cf938c;
        const std::uint16_t id = arg.raw != 0 ? at<std::uint16_t>(obj(arg.raw), 0x174) : 0;
        at<std::uint32_t>(object, 0x0) = 0x00d1af24;
        at<std::uint16_t>(object, 0x4) = id;
        break;
    }
    case ArgKind::String:
        break;
    }
    return object;
}

void push_lua_variant(Seg62Host& host, Object variants, const ChannelArg& arg) {
    std::uint8_t variant[0x20];
    switch (arg.kind) {
    case ArgKind::Entity: host.lua_variant_entity_00884a90(variant, obj(arg.raw)); break;
    case ArgKind::Int: host.lua_variant_int_00884a00(variant, arg.raw); break;
    case ArgKind::String: host.lua_variant_string_00884fc0(variant, obj(arg.raw)); break;
    case ArgKind::Bool: host.lua_variant_bool_00884a30(variant, arg.raw); break;
    case ArgKind::Float: host.lua_variant_008849d0(variant, arg.raw); break;
    case ArgKind::EntityObserved: host.lua_variant_entity_00884a90(variant, obj(arg.raw)); break;
    }
    host.lua_variant_vector_push_006edf00(variants, variant);
    host.lua_variant_destruct_00884af0(variant);
}

// The body every simple channel dispatcher inlines (00983C10 is the model),
// inside the manager lock [mgr+24h] (EnterCriticalSection, then its count
// +18h incremented; on exit decremented and the section left): +104h and
// +105h cleared; the channel name built in a native
// string (resize to the name length with fill 1, memcpy of length + 1
// bytes), looked up by 00980150 on the map +F8h and released. A channel with
// subscriptions (+8h non-zero) gets the argument objects pushed into a local
// vector (00975EA0), 0097B8C0(channel, &arguments, &callbacks), the argument
// objects deleted (vtable[0](1)) and cleared; when callback names came back
// a 0Ch-byte Lua variant vector gets one variant per argument and every
// name is called (00887E50 on [game+1A08h] with (0, name, variants, 0, -1)),
// the vector is emptied (006B88E0(0)) and its storage and itself freed, and
// +105h set; the callback names are released to the pool and their storage
// freed, then the argument storage. The stack guard object (vtable 00CE37FC)
// has no other effect; the dead read of +104h before the unlock is not
// modelled.
// How a dispatcher builds the channel name and frees the callback names.
struct DispatchStyle {
    bool construct_name;   // 0041E870 instead of resize + memcpy (00984BA0)
    bool range_destroy;    // 00432050 over the callback names instead of the inline pool loop (00984BA0)
};
constexpr DispatchStyle kInlineStyle{false, false};

Object lock_manager(Seg62Host& host, Object manager) {
    const Object section = obj(at<std::uint32_t>(manager, 0x24));
    if (section != nullptr) {
        host.enter_critical_section(section);
        at<std::uint32_t>(section, 0x18) = at<std::uint32_t>(section, 0x18) + 1;
    }
    return section;
}

void unlock_manager(Seg62Host& host, Object section) {
    if (section == nullptr) return;
    at<std::uint32_t>(section, 0x18) = at<std::uint32_t>(section, 0x18) - 1;
    host.leave_critical_section(section);
}

void dispatch_channel_body(Seg62Host& host, Object manager, const char* channel, const ChannelArg* args,
                           std::size_t count, DispatchStyle style) {
    at<std::uint8_t>(manager, 0x104) = 0;
    at<std::uint8_t>(manager, 0x105) = 0;
    std::uint32_t name[2] = {0, 0};
    if (style.construct_name) {
        host.string_construct_0041e870(reinterpret_cast<Object>(name), channel);
    } else {
        host.string_resize_0041dd40(reinterpret_cast<Object>(name), static_cast<std::uint32_t>(std::strlen(channel)), 1);
        if (name[1] != 0) host.memcpy_00bf7680(obj(name[1]), channel, name[0] + 1);
    }
    const Object found = host.channel_index_00980150(manager + 0xf8, reinterpret_cast<Object>(name));
    release_local(host, name);
    if (at<std::uint32_t>(found, 0x8) != 0) {
        std::uint32_t arguments[4] = {0, 0, 0, 0};  // {proxy, first, last, end}
        for (std::size_t i = 0; i < count; ++i) {
            std::uint8_t observer[0x20];
            observer[0] = 0;
            const Object object = build_arg_object(host, args[i], observer);
            host.arg_vector_push_00975ea0(reinterpret_cast<Object>(arguments), &object);
            if (args[i].kind == ArgKind::EntityObserved && object != nullptr) host.observer_destruct_006e0860(observer);
        }
        std::uint32_t callbacks[4] = {0, 0, 0, 0};
        host.channel_evaluate_0097b8c0(manager, found, reinterpret_cast<Object>(arguments), reinterpret_cast<Object>(callbacks));
        for (std::uint32_t i = 0; arguments[1] != 0 && i < ((arguments[2] - arguments[1]) >> 2); ++i) {
            const Object object = obj(at<std::uint32_t>(obj(arguments[1]), i * 4));
            if (object != nullptr) {
                host.vt0_delete(object, 1);
                at<std::uint32_t>(obj(arguments[1]), i * 4) = 0;
            }
        }
        if (callbacks[1] != 0 && ((callbacks[2] - callbacks[1]) >> 3) != 0) {
            const Object variants = host.operator_new_00bf681b(0xc);
            if (variants != nullptr) {
                at<std::uint32_t>(variants, 0x0) = 0;
                at<std::uint32_t>(variants, 0x4) = 0;
                at<std::uint32_t>(variants, 0x8) = 0;
            }
            for (std::size_t i = 0; i < count; ++i) push_lua_variant(host, variants, args[i]);
            for (std::uint32_t i = 0; callbacks[1] != 0 && i < ((callbacks[2] - callbacks[1]) >> 3); ++i) {
                const Object lua_host = obj(at<std::uint32_t>(obj(host.global_u32(0x00e188a8)), 0x1a08));
                host.lua_call_named_00887e50(lua_host, 0, obj(callbacks[1] + i * 8), variants, 0, -1);
            }
            if (variants != nullptr) {
                host.lua_variant_vector_resize_006b88e0(variants, 0);
                host.free_00bf6989(obj(at<std::uint32_t>(variants, 0x0)));
                host.free_00bf65ac(variants);
            }
            at<std::uint8_t>(manager, 0x105) = 1;
        }
        if (callbacks[1] != 0) {
            if (style.range_destroy) {
                host.string_range_destroy_00432050(obj(callbacks[1]), obj(callbacks[2]), addr(callbacks), 0);
            } else {
                for (std::uint32_t element = callbacks[1]; element != callbacks[2]; element += 8) {
                    release_local(host, reinterpret_cast<const std::uint32_t*>(obj(element)));
                }
            }
            host.free_00bf65ac(obj(callbacks[1]));
        }
        callbacks[1] = 0;
        callbacks[2] = 0;
        callbacks[3] = 0;
        if (arguments[1] != 0) host.free_00bf65ac(obj(arguments[1]));
    }
}

void dispatch_channel(Seg62Host& host, Object manager, const char* channel, const ChannelArg* args, std::size_t count) {
    const Object section = lock_manager(host, manager);
    dispatch_channel_body(host, manager, channel, args, count, kInlineStyle);
    unlock_manager(host, section);
}

}  // namespace

// The channel dispatchers below are __thiscall(ECX = warning manager, the
// channel's arguments on the stack), RET 4 per argument; each is the
// dispatch_channel body above with its channel name and argument kinds, read
// from the listing (argument objects and Lua constructors in stack order).
// SEH frames are not modelled.

// 00982990 "surrender" (00D03A6C): (int). RET 4. Callers: the side-empty report
// (mission_state_frame.cpp host hook report_side_empty_00982990).
void dispatch_surrender_00982990(Seg62Host& host, Object manager, std::uint32_t value) {
    const ChannelArg args[] = {{ArgKind::Int, value}};
    dispatch_channel(host, manager, "surrender", args, 1);
}

// 00983C10 "repair" (00CFD994): (entity, int repair setting). RET 8.
void dispatch_repair_00983c10(Seg62Host& host, Object manager, Object entity, std::uint32_t setting) {
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::Int, setting}};
    dispatch_channel(host, manager, "repair", args, 2);
}

// 00983F50 "zone" (00D1B884): (entity, zone name string*, bool word). RET 0Ch.
void dispatch_zone_00983f50(Seg62Host& host, Object manager, Object entity, Object zone, std::uint32_t flag) {
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::String, addr(zone)}, {ArgKind::Bool, flag}};
    dispatch_channel(host, manager, "zone", args, 3);
}

// 00984EB0 "stock" (00CF86B8): (entity, int, int). RET 0Ch. The air-operations
// notification 006CA883 / 006CA899.
void dispatch_stock_00984eb0(Seg62Host& host, Object manager, Object entity, std::uint32_t a, std::uint32_t b) {
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::Int, a}, {ArgKind::Int, b}};
    dispatch_channel(host, manager, "stock", args, 3);
}

// 00985250 "gui" (00D1B880): (string*, bool word). RET 8.
void dispatch_gui_00985250(Seg62Host& host, Object manager, Object text, std::uint32_t flag) {
    const ChannelArg args[] = {{ArgKind::String, addr(text)}, {ArgKind::Bool, flag}};
    dispatch_channel(host, manager, "gui", args, 2);
}

// 00985920 "player" (00D199D0): (int player, string*). RET 8.
void dispatch_player_00985920(Seg62Host& host, Object manager, std::uint32_t player, Object text) {
    const ChannelArg args[] = {{ArgKind::Int, player}, {ArgKind::String, addr(text)}};
    dispatch_channel(host, manager, "player", args, 2);
}

// 00985C50 "musicOver" (00D1B868): (string*). RET 4.
void dispatch_music_over_00985c50(Seg62Host& host, Object manager, Object text) {
    const ChannelArg args[] = {{ArgKind::String, addr(text)}};
    dispatch_channel(host, manager, "musicOver", args, 1);
}

// 00984BA0 "ammoType" (00D08770): __thiscall(ECX = manager, squadron), RET 4.
// Inside the lock, a null squadron does nothing; otherwise its ammunition
// kind (007EDAD0) becomes the int argument, the channel name is constructed
// with 0041E870, the arguments are (entity squadron, int kind), and the
// callback names are destroyed with 00432050 (ECX = first, EDX = last, the
// vector address and an unused stack word, modelled as 0).
void dispatch_ammo_type_00984ba0(Seg62Host& host, Object manager, Object squadron) {
    const Object section = lock_manager(host, manager);
    if (squadron != nullptr) {
        const std::uint32_t kind = host.squadron_ammo_type_007edad0(squadron);
        const ChannelArg args[] = {{ArgKind::Entity, addr(squadron)}, {ArgKind::Int, kind}};
        dispatch_channel_body(host, manager, "ammoType", args, 2, DispatchStyle{true, true});
    }
    unlock_manager(host, section);
}

// 00985590 "generate" (00D1B874): __thiscall(ECX = manager, generated object),
// RET 4. Inside the lock, the object's vtable[10h]() name is copied into a
// native string (0041E870) and its +C4h word taken; the arguments are (that
// string, int +C4h); the string is released to the pool before the unlock.
void dispatch_generate_00985590(Seg62Host& host, Object manager, Object generated) {
    const Object section = lock_manager(host, manager);
    std::uint32_t name[2];
    host.string_construct_0041e870(reinterpret_cast<Object>(name), host.vt10_name(generated));
    const std::uint32_t value = at<std::uint32_t>(generated, 0xc4);
    const ChannelArg args[] = {{ArgKind::String, addr(name)}, {ArgKind::Int, value}};
    dispatch_channel_body(host, manager, "generate", args, 2, kInlineStyle);
    release_local(host, name);
    unlock_manager(host, section);
}

namespace {

std::uint32_t float_bits(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    return bits;
}

// The damage-warning tail of 009832F0 / 00983780 (and the first half of
// 00982C50's), still inside the lock and run whether or not the channel had
// subscriptions: unless the manager's +104h or +D0h byte is set, the local
// slot [game+18ECh] is in 0..7, the clock [00F876A4] is past 4.0f (00CE3D34;
// COMISS, unordered fails), the entity's +5Ch is set and +5Dh, +60h, +5Eh are
// clear, its suppression byte (0077EDF0 on +D4h) is clear, and it is the
// local unit [00E188D8] (for a kind-0Fh entity its +9D4h platform is compared
// instead): an 84h-byte entity warning is built with the id `name`
// (00974150; the id string is constructed only when the allocation
// succeeded) and reported (009763E0); the id string is then released.
void report_local_damage_warning(Seg62Host& host, Object manager, Object entity, const char* name) {
    if (at<std::uint8_t>(manager, 0x104) != 0) return;
    if (at<std::uint8_t>(manager, 0xd0) != 0) return;
    const std::int32_t slot = at<std::int32_t>(obj(host.global_u32(0x00e188a8)), 0x18ec);
    if (slot < 0 || slot > 7) return;
    if (!(host.global_f32(kClock) > 4.0f)) return;
    if (at<std::uint8_t>(entity, 0x5c) == 0) return;
    if (at<std::uint8_t>(entity, 0x5d) != 0) return;
    if (at<std::uint8_t>(entity, 0x60) != 0) return;
    if (at<std::uint8_t>(entity, 0x5e) != 0) return;
    const Object key = entity;
    if (*host.entity_suppress_slot_0077edf0(manager + 0xd4, &key) != 0) return;
    const std::uint32_t local_unit = host.global_u32(0x00e188d8);
    const bool local = host.vt5c_is_kind_of(entity, 0xf) ? at<std::uint32_t>(entity, 0x9d4) == local_unit
                                                         : addr(entity) == local_unit;
    if (!local) return;
    Object warning = host.operator_new_00bf681b(0x84);
    std::uint32_t id[2] = {0, 0};
    bool id_built = false;
    if (warning != nullptr) {
        host.string_construct_0041e870(reinterpret_cast<Object>(id), name);
        id_built = true;
        warning = host.entity_warning_construct_00974150(warning, entity, reinterpret_cast<Object>(id));
    }
    host.warning_report_009763e0(manager, warning);
    if (id_built) release_local(host, id);
}

void dispatch_damage(Seg62Host& host, Object manager, Object entity, float seconds, const char* channel) {
    const Object section = lock_manager(host, manager);
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::Float, float_bits(seconds)}};
    dispatch_channel_body(host, manager, channel, args, 2, kInlineStyle);
    report_local_damage_warning(host, manager, entity, channel);
    unlock_manager(host, section);
}

}  // namespace

// 009832F0 "leak" (00D0054C): __thiscall(ECX = manager, entity, float seconds),
// RET 8 (caller 0093A513). The shared dispatch body with args (entity,
// float), then the local damage warning "leak".
void dispatch_leak_009832f0(Seg62Host& host, Object manager, Object entity, float seconds) {
    dispatch_damage(host, manager, entity, seconds, "leak");
}

// 00983780 "fire" (00CE6798): as 009832F0 with "fire" (caller 0093A484).
void dispatch_fire_00983780(Seg62Host& host, Object manager, Object entity, float seconds) {
    dispatch_damage(host, manager, entity, seconds, "fire");
}

// 00982C50 "failure" (00D1B88C): __thiscall(ECX = manager, entity, const
// string* kind, float seconds), RET 0Ch. Callers 0093C0C2 and 0095AD52. The
// shared dispatch body with args (entity, string, float); then, inside the
// lock, the same gate as the leak/fire warning up to the suppression byte,
// except that the entity also passes when it answers vtable[5Ch](45h)
// (airfield) or vtable[5Ch](46h) (shipyard) instead of being the local unit.
// The warning id is "failure" (resize 7 with fill 1 + memcpy) concatenated
// with *kind (004261A0); when the entity's party +180h is the local slot
// [game+18ECh], an airfield replaces the id with "failureairfield" (inline
// copy) and a shipyard with "failureshipyard" (00425F40), and when the id is
// a key of the manager's name map ([00F8A0C4]+108h, 008E65E0; the checked
// iterator test 00BF6713 kept; head +10Ch) an 84h-byte entity warning is
// built (00974150) and reported to this manager (009763E0). The id is always
// released. SEH not modelled.
void dispatch_failure_00982c50(Seg62Host& host, Object manager, Object entity, Object kind, float seconds) {
    const Object section = lock_manager(host, manager);
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::String, addr(kind)},
                               {ArgKind::Float, float_bits(seconds)}};
    dispatch_channel_body(host, manager, "failure", args, 3, kInlineStyle);
    do {
        if (at<std::uint8_t>(manager, 0x104) != 0) break;
        if (at<std::uint8_t>(manager, 0xd0) != 0) break;
        const std::int32_t slot = at<std::int32_t>(obj(host.global_u32(0x00e188a8)), 0x18ec);
        if (slot < 0 || slot > 7) break;
        if (!(host.global_f32(kClock) > 4.0f)) break;
        if (at<std::uint8_t>(entity, 0x5c) == 0) break;
        if (at<std::uint8_t>(entity, 0x5d) != 0) break;
        if (at<std::uint8_t>(entity, 0x60) != 0) break;
        if (at<std::uint8_t>(entity, 0x5e) != 0) break;
        const Object key = entity;
        if (*host.entity_suppress_slot_0077edf0(manager + 0xd4, &key) != 0) break;
        const bool airfield = host.vt5c_is_kind_of(entity, 0x45);
        const bool shipyard = host.vt5c_is_kind_of(entity, 0x46);
        const std::uint32_t local_unit = host.global_u32(0x00e188d8);
        const bool local = host.vt5c_is_kind_of(entity, 0xf) ? at<std::uint32_t>(entity, 0x9d4) == local_unit
                                                             : addr(entity) == local_unit;
        if (!local && !airfield && !shipyard) break;
        std::uint32_t prefix[2] = {0, 0};
        host.string_resize_0041dd40(reinterpret_cast<Object>(prefix), 7, 1);
        if (prefix[1] != 0) host.memcpy_00bf7680(obj(prefix[1]), "failure", prefix[0] + 1);
        std::uint32_t id[2];
        host.string_concat_004261a0(reinterpret_cast<Object>(prefix), reinterpret_cast<Object>(id), kind);
        release_local(host, prefix);
        if (at<std::int32_t>(entity, 0x180) == at<std::int32_t>(obj(host.global_u32(0x00e188a8)), 0x18ec)) {
            if (airfield) {
                std::uint32_t text[2];
                const Object source = host.string_construct_0041e870(reinterpret_cast<Object>(text), "failureairfield");
                if (reinterpret_cast<Object>(id) != source) {
                    host.string_resize_0041dd40(reinterpret_cast<Object>(id), at<std::uint32_t>(source, 0x0), 1);
                    if (at<std::uint32_t>(source, 0x0) != 0) {
                        host.memcpy_00bf7680(obj(id[1]), obj(at<std::uint32_t>(source, 0x4)), id[0]);
                    }
                }
                release_local(host, text);
            } else if (shipyard) {
                std::uint32_t text[2];
                const Object source = host.string_construct_0041e870(reinterpret_cast<Object>(text), "failureshipyard");
                host.string_assign_header_00425f40(reinterpret_cast<Object>(id), source);
                release_local(host, text);
            }
            const Object names = obj(host.global_u32(kWarningManager));
            const std::uint32_t head = at<std::uint32_t>(names, 0x10c);
            std::uint32_t found[2];
            const Object it = host.tree_find_008e65e0(names + 0x108, reinterpret_cast<Object>(found),
                                                      reinterpret_cast<Object>(id));
            if (at<std::uint32_t>(it, 0x0) == 0 || obj(at<std::uint32_t>(it, 0x0)) != names + 0x108) {
                host.invalid_parameter_00bf6713();
            }
            if (at<std::uint32_t>(it, 0x4) != head) {
                Object warning = host.operator_new_00bf681b(0x84);
                if (warning != nullptr) {
                    warning = host.entity_warning_construct_00974150(warning, entity, reinterpret_cast<Object>(id));
                }
                host.warning_report_009763e0(manager, warning);
            }
        }
        release_local(host, id);
    } while (false);
    unlock_manager(host, section);
}

// 00984800 "target" (00CFD964): __thiscall(ECX = manager, entity, target),
// RET 8. Callers 0071F7E8 and 0071F909 (command execution). The shared
// dispatch body with args (entity, target); the target's argument object is
// built while a stack observer of the target exists (006E0E00, its vtable then
// set to 00CF938C; 006E0860 after the push, only when the allocation
// succeeded). Both Lua variants are entities (00884A90).
void report_target_00984800(Seg62Host& host, Object manager, Object entity, Object target) {
    const ChannelArg args[] = {{ArgKind::Entity, addr(entity)}, {ArgKind::EntityObserved, addr(target)}};
    dispatch_channel(host, manager, "target", args, 2);
}

// 00986820 "shipLanded" (00D1B844): __thiscall(ECX = manager, ship), RET 4.
// Inside the lock: when 00923810(ship, 1) answers an object, the event runs
// only if that object (asked again) answers vtable[5Ch](44h); the name is
// constructed with 0041E870 and the args are (entity ship).
void report_ship_landed_00986820(Seg62Host& host, Object manager, Object ship) {
    const Object section = lock_manager(host, manager);
    bool run = true;
    if (host.ship_landing_target_00923810(ship, 1) != nullptr) {
        run = host.vt5c_is_kind_of(host.ship_landing_target_00923810(ship, 1), 0x44);
    }
    if (run) {
        const ChannelArg args[] = {{ArgKind::Entity, addr(ship)}};
        dispatch_channel_body(host, manager, "shipLanded", args, 1, DispatchStyle{true, false});
    }
    unlock_manager(host, section);
}

// 0097DC80 __thiscall(ECX = manager, out string, const vector<string>* keys,
// entity), RET 0Ch, EAX = out. No xrefs in the saved analysis. Picks index
// 00BD2FC0(1) % count (an empty vector divides by zero as in the image; the
// checked index test 00BF6713 kept), copies that string into out (cleared
// first; resize with fill 1 + memcpy unless out is the element), then
// 0097D650(out, entity) resolves its sound keys. Draws from
// BSP_RandomThreads_NextU32 stream 1, not the gameplay stream 00BD2F10.
// SEH 00CAB979 not modelled.
Object pick_sound_key_0097dc80(Seg62Host& host, Object manager, Object out, Object keys, Object entity) {
    const std::uint32_t count =
        at<std::uint32_t>(keys, 0x4) == 0 ? 0 : (at<std::uint32_t>(keys, 0x8) - at<std::uint32_t>(keys, 0x4)) >> 3;
    const std::uint32_t index = host.random_threads_next_00bd2fc0(1) % count;
    if (at<std::uint32_t>(keys, 0x4) == 0 || index >= ((at<std::uint32_t>(keys, 0x8) - at<std::uint32_t>(keys, 0x4)) >> 3)) {
        host.invalid_parameter_00bf6713();
    }
    const Object picked = obj(at<std::uint32_t>(keys, 0x4) + index * 8);
    at<std::uint32_t>(out, 0x0) = 0;
    at<std::uint32_t>(out, 0x4) = 0;
    if (out != picked) {
        host.string_resize_0041dd40(out, at<std::uint32_t>(picked, 0x0), 1);
        if (at<std::uint32_t>(picked, 0x0) != 0) {
            host.memcpy_00bf7680(obj(at<std::uint32_t>(out, 0x4)), obj(at<std::uint32_t>(picked, 0x4)), at<std::uint32_t>(out, 0x0));
        }
    }
    host.resolve_entity_sound_keys_0097d650(manager, out, entity);
    return out;
}

// 0098A710 __thiscall(ECX = node, float* min, float* max), RET 8. Copies the
// bounds +10Ch..+114h into min and +118h..+120h into max. Caller 009034B1.
void node_bounds_0098a710(Object node, float* box_min, float* box_max) {
    box_min[0] = at<float>(node, 0x10c);
    box_min[1] = at<float>(node, 0x110);
    box_min[2] = at<float>(node, 0x114);
    box_max[0] = at<float>(node, 0x118);
    box_max[1] = at<float>(node, 0x11c);
    box_max[2] = at<float>(node, 0x120);
}

// 0098B9C0 __thiscall(ECX = collision node, a, b, c, d), RET 10h. Vtable slot
// (00CE89F0, 00CEA064, 00CF8B20). Only when the segment (a, b) overlaps the
// node box (0085CAD0 with ECX = +13Ch, EDX = +148h) is 0098AC20(a, b, c, d, 0)
// called. blast_damage.hpp names only the vtable offset.
void collision_trace_segment_gated_0098b9c0(Seg62Host& host, Object node, std::uint32_t a, std::uint32_t b,
                                            std::uint32_t c, std::uint32_t d) {
    if (!host.segment_overlaps_aabb_0085cad0(node + 0x13c, node + 0x148, a, b)) return;
    host.collision_trace_segment_0098ac20(node, a, b, c, d, 0);
}

// 0098B370 __thiscall(ECX = spatial index, segments, a, b), RET 0Ch -> AL.
// The segment list holds 18h-byte {from[3], to[3]} entries with the count at
// +C0h. No segments: false. One: from/to are copied to the stack and
// 0098ADD0(from, to, a, b, 0) answers. More than one: the entries from the
// SECOND on are queried in order (the image starts at +18h) until one hits
// (true); none hitting is false. Callers 0084C1D3 (projectile sweep) and the
// mission camera; projectile_impact.hpp holds it only as a host hook.
bool spatial_sweep_segments_0098b370(Seg62Host& host, Object index, Object segments, std::uint32_t a, std::uint32_t b) {
    const std::uint32_t count = at<std::uint32_t>(segments, 0xc0);
    if (count == 0) return false;
    if (count == 1) {
        const float from[3] = {at<float>(segments, 0x0), at<float>(segments, 0x4), at<float>(segments, 0x8)};
        const float to[3] = {at<float>(segments, 0xc), at<float>(segments, 0x10), at<float>(segments, 0x14)};
        return host.spatial_query_segment_0098add0(index, from, to, a, b, 0);
    }
    const Object end = segments + count * 0x18;
    for (Object entry = segments + 0x18; entry != end; entry += 0x18) {
        if (host.spatial_query_segment_0098add0(index, reinterpret_cast<const float*>(entry),
                                                reinterpret_cast<const float*>(entry + 0xc), a, b, 0)) {
            return true;
        }
    }
    return false;
}

// 0098B450 __thiscall(ECX = spatial index, const float min[3], const float
// max[3]), RET 8 -> AL. Maps both corners to grid cells (0098AD60) and walks
// the 96h x 96h cell lists (+84h, row stride 258h; node next +4h, object +8h)
// over [low row..high row] x [low column..high column] without clamping. True
// at the first object whose entity (+4Ch) answers vtable[5Ch](6) and whose box
// (+13Ch..+148h) overlaps [min, max] (0085BF20). cc13_left55_seg56a holds it
// as a host hook (00894DAE).
bool spatial_any_ship_in_box_0098b450(Seg62Host& host, Object index, const float* box_min, const float* box_max) {
    std::int32_t low[2];
    std::int32_t high[2];
    host.spatial_cell_of_point_0098ad60(low, box_min);
    host.spatial_cell_of_point_0098ad60(high, box_max);
    for (std::int32_t row = low[0]; row <= high[0]; ++row) {
        for (std::int32_t column = low[1]; column <= high[1]; ++column) {
            for (Object node = obj(at<std::uint32_t>(index, 0x84 + (row * 0x96 + column) * 4)); node != nullptr;
                 node = obj(at<std::uint32_t>(node, 0x4))) {
                const Object object = obj(at<std::uint32_t>(node, 0x8));
                if (!host.vt5c_is_kind_of(obj(at<std::uint32_t>(object, 0x4c)), 6)) continue;
                if (host.aabb_overlaps_0085bf20(box_min, box_max, object + 0x13c, object + 0x148)) return true;
            }
        }
    }
    return false;
}

// 0098B7D0 __fastcall(ECX = spatial node), RET. Caller 006D3E0F
// (BSP_AirField_ReadRunwayProperties). With a shape count at +F8h, the box of
// the first shape ([+D0h], min +4h..+0Ch, max +10h..+18h) is widened by every
// further shape of the array +D4h.. (component-wise: a smaller min / larger max
// replaces the current one; FCOMIP, so a NaN never replaces) and handed to
// 0098A920(min, max). No shapes: nothing.
void spatial_node_union_shape_bounds_0098b7d0(Seg62Host& host, Object node) {
    const std::uint32_t count = at<std::uint32_t>(node, 0xf8);
    if (count == 0) return;
    const Object first = obj(at<std::uint32_t>(node, 0xd0));
    float box_min[3] = {at<float>(first, 0x4), at<float>(first, 0x8), at<float>(first, 0xc)};
    float box_max[3] = {at<float>(first, 0x10), at<float>(first, 0x14), at<float>(first, 0x18)};
    for (std::size_t slot = 0xd4; slot != 0xd0 + count * 4; slot += 4) {
        const Object shape = obj(at<std::uint32_t>(node, slot));
        for (int i = 0; i < 3; ++i) {
            const float low = at<float>(shape, 0x4 + i * 4);
            if (box_min[i] > low) box_min[i] = low;
        }
        for (int i = 0; i < 3; ++i) {
            const float high = at<float>(shape, 0x10 + i * 4);
            if (high > box_max[i]) box_max[i] = high;
        }
    }
    host.spatial_node_set_bounds_0098a920(node, box_min, box_max);
}

}  // namespace bsp::cc13_seg62
