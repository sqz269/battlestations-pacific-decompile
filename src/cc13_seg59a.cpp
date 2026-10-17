// Segment 59 scoring and awards band (packet cc13-seg59a, docs/CC13_SEG59A.md).
// Hypothesis names; offsets, strings and constants from the listing and bytes of each body.

#include "bsp/cc13_seg59a.hpp"

#include <cmath>
#include <cstring>

#include "bsp/cc13_seg33a.hpp"

namespace bsp::cc13_seg59a {

using cc13_seg33a::add;
using cc13_seg33a::at;
using cc13_seg33a::ptr;

namespace {

// Image literals (bytes read at the addresses).
constexpr const char kBadgePrefixBU[] = "BU";            // 00D1872C
constexpr const char kBadgePrefixBW[] = "BW";            // 00D18728
constexpr const char kBadgePrefixBM[] = "BM";            // 00D18724
constexpr const char kBadgePrefixBO[] = "BO";            // 00D18720
constexpr const char kRankPrefix[] = "RANK";             // 00CEF15C
constexpr const char kCounterPrefix[] = "Counter";       // 00D18794
constexpr const char kAwardGaAe[] = "GA_AE";             // 00D1879C
constexpr const char kAwardSaOc[] = "SA_OC";             // 00D187A4
constexpr const char kAwardSaIm[] = "SA_IM";             // 00D187AC
constexpr const char kAwardGaAi[] = "GA_AI";             // 00D187B4
constexpr const char kAwardGaDn[] = "GA_DN";             // 00D187BC
constexpr const char kCounterRuaMu[] = "Counter_RUA_MU"; // 00D187D4
constexpr const char kAwardGaSl[] = "GA_SL";             // 00D188B4
constexpr const char kAwardRuaMu[] = "RUA_MU";           // 00D188BC

void* slot_record(void* scoring, std::uint32_t slot) {
    return add(scoring, static_cast<std::size_t>(slot) * kSlotRecordStride);
}

// The inline release every NativeString destructor in the band expands to:
// `if (data) 00BD1510(00419CC0(data, length+1, 1), data, length+1, 1)`.
void release(Seg59aHost& host, const NativeStr& s) {
    if (s.data != nullptr) {
        void* const pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

// The inline copy the compiler emits for a by-value NativeString argument: zero both fields,
// then, unless the destination is the source, resize to the source length and copy the
// destination's (new) length when the source length is non-zero.
void copy_inline(Seg59aHost& host, NativeStr* dst, const NativeStr* src) {
    dst->length = 0;
    dst->data = nullptr;
    if (dst != src) {
        host.native_string_resize_0041dd40(dst, src->length, 1);
        if (src->length != 0) host.memcpy_00bf7680(dst->data, src->data, dst->length);
    }
}

// The inline literal construction 00913AB0 and 0090F5A0 use: zero, resize to the literal's
// length, then copy length+1 bytes (with the terminator) when data was allocated.
void literal_inline(Seg59aHost& host, NativeStr* s, const char* text, std::uint32_t length) {
    s->length = 0;
    s->data = nullptr;
    host.native_string_resize_0041dd40(s, length, 1);
    if (s->data != nullptr) host.memcpy_00bf7680(s->data, text, s->length + 1);
}

// The assignment body shared by 0090BDA0, 0090BE30 and 0090BEC0: a by-value source assigned
// into a field, then the source released.
void assign_by_value(Seg59aHost& host, NativeStr* field, NativeStr& text) {
    if (field != &text) {
        host.native_string_resize_0041dd40(field, text.length, 1);
        if (text.length != 0) host.memcpy_00bf7680(field->data, text.data, field->length);
    }
    release(host, text);
}

// `find(map, key) == end` with the checked iterator's container test (00BF6713 when the
// iterator's container is null or not the map).
bool award_map_lacks(Seg59aHost& host, void* map, const NativeStr* key) {
    void* const head = at<void*>(map, 4);
    const CheckedIter it = host.string_int_map_find_004c8b80(map, key);
    if (it.container == nullptr || it.container != map) host.invalid_parameter_00bf6713();
    return it.node == head;
}

// Grant one literal award through 0090EDE0: assign, grant (slot, &s, 1, 0), release.
void grant_literal(Seg59aHost& host, void* scoring, std::int32_t slot, const char* text) {
    NativeStr s;
    host.native_string_construct_0041e870(&s, text);
    scoring_grant_award_0090ede0(host, scoring, slot, &s, 1, 0);
    release(host, s);
}

// FISTP to a 32-bit integer under the default round-to-nearest-even control word; an
// unrepresentable value stores the integer indefinite 80000000h.
std::int32_t fistp_int32(float value) {
    const double r = std::nearbyint(static_cast<double>(value));
    if (!(r >= -2147483648.0 && r <= 2147483647.0)) return static_cast<std::int32_t>(0x80000000u);
    return static_cast<std::int32_t>(r);
}

// CVTTSS2SI: truncation toward zero; NaN and out-of-range give the indefinite 80000000h.
std::int32_t cvttss2si(float value) {
    if (!(value > -2147483904.0f && value < 2147483648.0f)) return static_cast<std::int32_t>(0x80000000u);
    return static_cast<std::int32_t>(value);
}

// The checked `config+2Ch` float vector read at [index]: 00BF6713 when the vector is unallocated
// or index >= size (unsigned compare, JC), then the element is read regardless.
float config_point_scale(Seg59aHost& host, std::uint32_t index) {
    void* const vec = add(host.global_config_00432650(), 0x2C);
    float* const first = static_cast<float*>(ptr(vec, 4));
    if (first == nullptr ||
        !(index < static_cast<std::uint32_t>((static_cast<float*>(ptr(vec, 8)) - first)))) {
        host.invalid_parameter_00bf6713();
    }
    return first[index];
}

float selected_score_scale(Seg59aHost& host, std::uint8_t flag) {
    void* const scoring = ptr(host.game_00e188a8(), 0x21A0);
    return flag != 0 ? at<float>(scoring, 0x1478) : at<float>(scoring, 0x1474);
}

}  // namespace

// 0090C000, __thiscall(ECX = scoring, unused; NativeString key by value at [ESP+4]), RET 8, AL.
// Four 00553C80(&key, prefix, 0) tests in the order BU, BW, BM, BO; true on the first hit; the
// key is released before return.
bool score_table_is_badge_earned_0090c000(Seg59aHost& host, void* /*scoring*/, NativeStr key) {
    const bool earned = host.native_string_has_prefix_at_00553c80(&key, kBadgePrefixBU, 0) ||
                        host.native_string_has_prefix_at_00553c80(&key, kBadgePrefixBW, 0) ||
                        host.native_string_has_prefix_at_00553c80(&key, kBadgePrefixBM, 0) ||
                        host.native_string_has_prefix_at_00553c80(&key, kBadgePrefixBO, 0);
    release(host, key);     // 0090C057..0090C071
    return earned;
}

// 0090EDE0, __thiscall(ECX = scoring, slot, const NativeString* token, value, byte accumulate),
// RET 10h. SEH frame (handler 00CA576B), state 0 only around the session message.
//  - mode = [game+1FE4h]. Modes 0 and 1 first run 0090C000 on a copy of the token and DISCARD
//    the result (0090EE65; AL is never read). Then a slot that is not the local one
//    ([game+18ECh]) in a networked session ([game+5D4h] == 0Dh, mode 1) is forwarded to its owner
//    as a type-14h session message and is not shown locally.
//  - Otherwise, when the token is new in the slot's award map (record+70h), a badge token goes
//    to the HUD award ticker (00648AB0) and is not reported further; a token already present is
//    not reported either.
//  - Every path stores the value (plus the stored one when `accumulate`) into the map.
//  - A token still to report that is not a badge, belongs to the local slot, is neither a short
//    "RANK" key (length < 6) nor a "Counter..." key and has an achievement id in 1..99 is queued
//    online (00A410A0) and recorded in the award tracker at game+650h (007FBE20).
void scoring_grant_award_0090ede0(Seg59aHost& host, void* scoring, std::int32_t slot, const NativeStr* token,
                                  std::int32_t value, std::uint8_t accumulate) {
    bool report = true;                                          // byte [ESP+13h] = 1
    void* game = host.game_00e188a8();
    const std::int32_t mode = at<std::int32_t>(game, 0x1FE4);
    bool forwarded = false;
    if (mode == 0 || mode == 1) {
        NativeStr copy;                                          // the by-value argument at ESP
        copy_inline(host, &copy, token);                         // 0090EE25..0090EE60
        score_table_is_badge_earned_0090c000(host, scoring, copy);   // 0090EE65, result unused
        game = host.game_00e188a8();
        if (slot != at<std::int32_t>(game, 0x18EC) && at<std::int32_t>(game, 0x5D4) == 0x0D &&
            at<std::int32_t>(game, 0x1FE4) == 1) {
            alignas(4) std::uint8_t message[kSessionMessageSize];   // [ESP+24h]
            host.session_message21_construct_00764340(message);
            at<std::int32_t>(message, 0x1C) = slot;              // [ESP+40h]
            at<std::int32_t>(message, 0x18) = 0x14;              // [ESP+3Ch]
            at<std::int32_t>(message, 0x188) = value;            // [ESP+1ACh]
            const char* src = token->data != nullptr ? token->data : host.empty_text_00f899a0();
            char* dst = reinterpret_cast<char*>(message + 0x7C);   // [ESP+0A0h], unbounded copy
            char c;
            do {
                c = *src++;
                *dst++ = c;
            } while (c != '\0');
            game = host.game_00e188a8();
            void* const peer = at<void*>(game, 0x18CC + static_cast<std::size_t>(slot) * 4);
            host.session_send_to_remote_player_00772af0(add(game, 0x1EF0), message, peer);
            report = false;                                      // 0090EF16
            forwarded = true;
        }
    }
    void* const record = slot_record(scoring, static_cast<std::uint32_t>(slot));
    void* const map = add(record, kAwardMapOffset);
    if (!forwarded) {
        if (award_map_lacks(host, map, token)) {                 // 0090EF27..0090EF5F
            NativeStr copy;
            host.native_string_copy_construct_00426060(&copy, token);
            if (score_table_is_badge_earned_0090c000(host, scoring, copy)) {
                NativeStr ticker;
                host.native_string_copy_construct_00426060(&ticker, token);
                void* const hud_root = ptr(host.hud_owner_00e198c4(), 0x40);
                host.hud_root_queue_award_ticker_00648ab0(hud_root, ticker);
                report = false;                                  // 0090EF98
            }
        } else {
            report = false;                                      // 0090EFBE
        }
    }
    std::int32_t stored = value;                                 // 0090EFA4
    if (accumulate != 0) stored = *host.string_int_map_index_005070c0(map, token) + value;
    *host.string_int_map_index_005070c0(map, token) = stored;    // 0090EFED
    if (!report) return;
    NativeStr copy;
    copy_inline(host, &copy, token);                             // 0090EFFF..0090F03A
    if (score_table_is_badge_earned_0090c000(host, scoring, copy)) return;
    if (slot != at<std::int32_t>(host.game_00e188a8(), 0x18EC)) return;
    // 0090F05D..0090F072: a "RANK" prefix only stops short keys (JC on length < 6).
    if (host.native_string_has_prefix_at_00553c80(token, kRankPrefix, 0) && token->length < 6) return;
    if (host.native_string_has_prefix_at_00553c80(token, kCounterPrefix, 0)) return;
    const std::int32_t id = host.award_registry_id_for_name_006b8da0(host.award_registry_00e19900(), token);
    if (static_cast<std::uint32_t>(id - 1) > 0x62u) return;     // 0090F095 CMP 62h, JA
    host.online_awards_grant_00a410a0(host.online_manager_00f8abe8(), id);
    void* const tracker = add(host.game_00e188a8(), 0x650);     // EDI read before the index call
    const std::int32_t current = *host.string_int_map_index_005070c0(map, token);
    host.award_tracker_record_at_least_007fbe20(tracker, token, current);
}

// 00914270, __thiscall(ECX = scoring, NativeString key by value), RET 8, EAX. 0090C000 on a
// copy; when it accepts the key, the award table record (0050FC30 on [00E19900]+10h with &key)
// supplies its +3Ch score. The key is released on both paths; 0 when not a badge.
std::int32_t award_registry_score_for_name_00914270(Seg59aHost& host, void* scoring, NativeStr key) {
    NativeStr copy;
    // 00914296..009142CB: the copy tests the SOURCE length (EBP) before the memcpy.
    copy.length = 0;
    copy.data = nullptr;
    if (&copy != &key) {
        host.native_string_resize_0041dd40(&copy, key.length, 1);
        if (key.length != 0) host.memcpy_00bf7680(copy.data, key.data, copy.length);
    }
    if (score_table_is_badge_earned_0090c000(host, scoring, copy)) {
        void* const table = add(host.award_registry_00e19900(), 0x10);
        void* const record = host.award_table_find_record_0050fc30(table, &key);
        const std::int32_t score = at<std::int32_t>(record, 0x3C);
        release(host, key);
        return score;
    }
    release(host, key);
    return 0;
}

// 0090F5A0, __thiscall(ECX = scoring, slot), RET 4. "GA_AE" is granted (value 1, no accumulate)
// the first time only: the lookup key is the inline literal (Resize 5, memcpy 6 bytes), the
// grant uses a fresh 0041E870 assignment.
void scoring_grant_ga_ae_once_0090f5a0(Seg59aHost& host, void* scoring, std::int32_t slot) {
    NativeStr key;
    literal_inline(host, &key, kAwardGaAe, 5);
    void* const map = add(slot_record(scoring, static_cast<std::uint32_t>(slot)), kAwardMapOffset);
    const bool absent = award_map_lacks(host, map, &key);
    release(host, key);
    if (absent) grant_literal(host, scoring, slot, kAwardGaAe);
}

// 0090F6C0, __thiscall(ECX = scoring, plane, kind), RET 8. The slot is [plane+1ACh] (no-op
// above 7). The slot record's ushort->int map at +1B4h is keyed by the plane's id word +174h:
//  kind 0: map[id] = slot (00667EB0), then the common find;
//  kind 4: erase the id when present (0058A3A0);
//  kind 1: grant "SA_IM" when the id is present;
//  kind 3: grant "SA_OC" unconditionally (the find is still made);
//  other kinds: nothing after the find.
void scoring_plane_achievement_0090f6c0(Seg59aHost& host, void* scoring, void* plane, std::int32_t kind) {
    const std::uint32_t slot = at<std::uint32_t>(plane, 0x1AC);
    if (slot > 7) return;
    void* const map = add(slot_record(scoring, slot), 0x1B4);
    if (kind == 0) {
        const std::int32_t id = at<std::uint16_t>(plane, 0x174);
        *host.ushort_int_map_index_00667eb0(map, &id) = static_cast<std::int32_t>(slot);
    }
    const std::int32_t id = at<std::uint16_t>(plane, 0x174);
    const CheckedIter it = host.ushort_int_map_find_0065cd40(map, &id);
    if (kind == 4) {
        void* const head = at<void*>(map, 4);
        if (it.container == nullptr || it.container != map) host.invalid_parameter_00bf6713();
        if (it.node == head) return;
        CheckedIter out;
        host.ushort_int_map_erase_0058a3a0(map, &out, it.container, it.node);
        return;
    }
    if (kind == 1) {
        void* const head = at<void*>(map, 4);
        if (it.container == nullptr || it.container != map) host.invalid_parameter_00bf6713();
        if (it.node == head) return;
        grant_literal(host, scoring, static_cast<std::int32_t>(slot), kAwardSaIm);
        return;
    }
    if (kind == 3) grant_literal(host, scoring, static_cast<std::int32_t>(slot), kAwardSaOc);
}

// 0090F860, __thiscall(ECX = scoring, slot, a, b), RET 0Ch (only caller 006F6760, the command
// building capture tick). Slots above 7 do nothing. Otherwise the counter at
// scoring+slot*284h+178h is incremented, "GA_DN" is granted when it is absent and a == 0Ch,
// then "GA_AI" when it is absent and b == 31h. Each test uses its own 0041E870 key, released
// before the grant's separate assignment.
void scoring_capture_awards_0090f860(Seg59aHost& host, void* scoring, std::uint32_t slot, std::int32_t a,
                                     std::int32_t b) {
    if (slot > 7) return;
    void* const record = slot_record(scoring, slot);
    at<std::int32_t>(record, 0x178) += 1;
    void* const map = add(record, kAwardMapOffset);
    NativeStr key;
    host.native_string_construct_0041e870(&key, kAwardGaDn);
    bool grant = award_map_lacks(host, map, &key) && a == 0x0C;
    release(host, key);
    if (grant) grant_literal(host, scoring, static_cast<std::int32_t>(slot), kAwardGaDn);
    host.native_string_construct_0041e870(&key, kAwardGaAi);
    grant = award_map_lacks(host, map, &key) && b == 0x31;
    release(host, key);
    if (grant) grant_literal(host, scoring, static_cast<std::int32_t>(slot), kAwardGaAi);
}

// 00913AB0, __thiscall(ECX = scoring, slot, kind), RET 8 (callers 00673A10 and the spawn
// manager 0094C490). kind 0Fh bumps "Counter_RUA_MU" in the slot's award map. Then, for every
// kind, "RUA_MU" is granted when that counter equals the first rank threshold of the RUA_MU
// award record (0050FC30, int vector +48h, checked [0]), and "GA_SL" is granted when absent.
void scoring_spawn_awards_00913ab0(Seg59aHost& host, void* scoring, std::int32_t slot, std::int32_t kind) {
    void* const map = add(slot_record(scoring, static_cast<std::uint32_t>(slot)), kAwardMapOffset);
    if (kind == 0x0F) {
        NativeStr counter;
        host.native_string_construct_0041e870(&counter, kCounterRuaMu);
        *host.string_int_map_index_005070c0(map, &counter) += 1;
        release(host, counter);
    }
    NativeStr counter_key;
    literal_inline(host, &counter_key, kCounterRuaMu, 14);
    NativeStr award_key;
    literal_inline(host, &award_key, kAwardRuaMu, 6);
    void* const table = add(host.award_registry_00e19900(), 0x10);
    void* const record = host.award_table_find_record_0050fc30(table, &award_key);
    std::int32_t* const first = static_cast<std::int32_t*>(ptr(record, 0x48));
    if (first == nullptr || (static_cast<std::int32_t*>(ptr(record, 0x4C)) - first) == 0) {
        host.invalid_parameter_00bf6713();
    }
    const std::int32_t count = *host.string_int_map_index_005070c0(map, &counter_key);
    const std::int32_t threshold = *first;
    release(host, award_key);
    release(host, counter_key);
    if (count == threshold) grant_literal(host, scoring, slot, kAwardRuaMu);
    NativeStr sl_key;
    literal_inline(host, &sl_key, kAwardGaSl, 5);
    const bool absent = award_map_lacks(host, map, &sl_key);
    release(host, sl_key);
    if (absent) grant_literal(host, scoring, slot, kAwardGaSl);
}

// 00908500, std::find over a checked ushort vector range. Register ABI: ECX = result iterator,
// EDX = const value*, stack (first.container, first.ptr, last.container, last.ptr), RET 10h,
// EAX = ECX. The value is read as a word; the result keeps first's container.
CheckedIter* find_ushort_00908500(CheckedIter* out, const std::int32_t* value, CheckedIter first, CheckedIter last) {
    const std::uint16_t* p = static_cast<const std::uint16_t*>(first.node);
    const std::uint16_t* const end = static_cast<const std::uint16_t*>(last.node);
    while (p != end && *p != static_cast<std::uint16_t>(*value)) ++p;
    out->node = const_cast<std::uint16_t*>(p);
    out->container = first.container;
    return out;
}

// 0090EC40, __thiscall(ECX = scoring, unit, unused), RET 8. Not on a client (mode 2). Units whose
// +170h interface slot 0 says 0Ch, or whose slot [unit+1B0h] is above 7, are ignored. A unit id
// (+174h) not yet in the slot's ushort vector (+130h) is appended and the int->int map at +0DCh
// counts one more under the unit's category (slot 0 again).
void scoring_record_unique_kill_0090ec40(Seg59aHost& host, void* scoring, void* unit, std::int32_t /*unused*/) {
    if (at<std::int32_t>(host.game_00e188a8(), 0x1FE4) == 2) return;
    void* const iface = add(unit, 0x170);
    if (host.unit_iface170_slot0(iface) == 0x0C) return;
    const std::uint32_t slot = at<std::uint32_t>(unit, 0x1B0);
    if (slot > 7) return;
    const std::int32_t id = at<std::uint16_t>(unit, 0x174);       // [ESP+24h]
    void* const record = slot_record(scoring, slot);
    void* const vec = add(record, 0x130);
    const CheckedIter end_a = host.ushort_vector_end_00725050(vec);
    const CheckedIter begin = host.ushort_vector_begin_00725020(vec);
    const CheckedIter end_b = host.ushort_vector_end_00725050(vec);
    CheckedIter found;
    find_ushort_00908500(&found, &id, begin, end_a);
    if (!host.checked_iter_equal_00722180(&found, &end_b)) return;
    const std::int32_t pushed = at<std::uint16_t>(unit, 0x174);   // reuses the unit argument slot
    host.ushort_vector_push_back_007265e0(vec, &pushed);
    const std::int32_t category = host.unit_iface170_slot0(iface);
    *host.int_int_map_index_00625900(add(record, 0xDC), &category) += 1;
}

// 0090ED40, __thiscall(ECX = scoring, slot, unit, float amount), RET 0Ch. Not on a client: the
// float map at scoring+slot*284h+118h gains `amount` under the unit's category (one FADD).
void scoring_add_unit_float_0090ed40(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit, float amount) {
    if (at<std::int32_t>(host.game_00e188a8(), 0x1FE4) == 2) return;
    const std::int32_t category = host.unit_iface170_slot0(add(unit, 0x170));
    float* const cell = host.int_float_map_index_0090e740(add(slot_record(scoring, slot), 0x118), &category);
    *cell = amount + *cell;
}

// 0090ED90, __thiscall(ECX = scoring, party, const NativeString* name, amount), RET 0Ch. For
// each of the eight records, party 0 adds to the string map at scoring+i*284h+1F0h, party 1 to
// +1FCh; any other party skips the record (the party is re-read per record).
void scoring_add_party_loss_0090ed90(Seg59aHost& host, void* scoring, std::int32_t party, const NativeStr* name,
                                     std::int32_t amount) {
    for (std::uint32_t i = 0; i < kSlotCount; ++i) {
        // ESI = scoring+1FCh+i*284h; party 0 uses ESI-0Ch.
        void* map = nullptr;
        if (party == 0) {
            map = add(scoring, 0x1F0 + i * kSlotRecordStride);
        } else if (party == 1) {
            map = add(scoring, 0x1FC + i * kSlotRecordStride);
        } else {
            continue;
        }
        *host.string_int_map_index_005070c0(map, name) += amount;
    }
}

// 0090EB80, __thiscall(ECX = scoring, slot, unit, byte second_map), RET 0Ch. Slots above 7 do
// nothing; otherwise the int->int map at record+0F4h (or +10Ch when second_map) counts the unit's
// category once more.
void scoring_count_unit_event_0090eb80(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit,
                                       std::uint8_t second_map) {
    if (slot > 7) return;
    const std::int32_t category = host.unit_iface170_slot0(add(unit, 0x170));
    const std::size_t off = second_map != 0 ? 0x10C : 0xF4;
    *host.int_int_map_index_00625900(add(slot_record(scoring, slot), off), &category) += 1;
}

// 0090EBF0, __thiscall(ECX = scoring, unit), RET 4. The unit's owner slot [unit+180h] (no-op above
// 7) counts the unit's category once more in the int->int map at record+0E8h.
void scoring_count_owner_event_0090ebf0(Seg59aHost& host, void* scoring, void* unit) {
    if (at<std::uint32_t>(unit, 0x180) > 7) return;
    const std::int32_t category = host.unit_iface170_slot0(add(unit, 0x170));
    const std::uint32_t slot = at<std::uint32_t>(unit, 0x180);
    *host.int_int_map_index_00625900(add(slot_record(scoring, slot), 0xE8), &category) += 1;
}

// 0090E670, __thiscall(ECX = scoring, slot, unit, float seconds), RET 0Ch (only caller
// 006E4D50, the bomb rack drop). Slots above 7 do nothing; otherwise an 8-byte entry {word
// unit+174h, float(int truncate(seconds))} (CVTTSS2SI/CVTSI2SS, SSE) is appended to the vector at
// record+160h through 0090D9D0. Entry bytes 2..3 are left as the stack had them.
void scoring_record_drop_interval_0090e670(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit,
                                           float seconds) {
    if (slot > 7) return;
    alignas(4) std::uint8_t entry[8]{};
    at<std::uint16_t>(entry, 0) = at<std::uint16_t>(unit, 0x174);
    at<float>(entry, 4) = static_cast<float>(cvttss2si(seconds));
    CheckedIter out;
    host.drop_record_vector_append_0090d9d0(add(slot_record(scoring, slot), 0x160), &out, entry);
}

// 00905350, __thiscall(ECX = scoring, slot, ranking), RET 8: scoring+slot*284h+8 = ranking.
void scoring_set_slot_ranking_00905350(void* scoring, std::int32_t slot, std::int32_t ranking) {
    at<std::int32_t>(slot_record(scoring, static_cast<std::uint32_t>(slot)), 8) = ranking;
}

// 0090BDA0 / 0090BE30 / 0090BEC0, __thiscall(ECX = commit record, NativeString by value), RET 8:
// assign into record+254h / +25Ch / +264h, then release the argument. SEH frame (handlers
// 00CA5528 / 00CA5568 family) state 0 only while the argument is live.
void score_record_set_condition_message_0090bda0(Seg59aHost& host, void* record, NativeStr text) {
    assign_by_value(host, &at<NativeStr>(record, 0x254), text);
}

void score_record_set_victory_message_0090be30(Seg59aHost& host, void* record, NativeStr text) {
    assign_by_value(host, &at<NativeStr>(record, 0x25C), text);
}

void score_record_set_message_264_0090bec0(Seg59aHost& host, void* record, NativeStr text) {
    assign_by_value(host, &at<NativeStr>(record, 0x264), text);
}

// 0090BFC0, __thiscall(ECX = scoring, const NativeString* name), RET 4: assignment into
// scoring+147Ch (the final-scoring Lua function name), skipped on self-assignment; the copy
// length is the destination's after resize, and the copy is skipped when the source is empty.
void scoring_set_final_function_name_0090bfc0(Seg59aHost& host, void* scoring, const NativeStr* name) {
    NativeStr* const field = &at<NativeStr>(scoring, 0x147C);
    if (field == name) return;
    host.native_string_resize_0041dd40(field, name->length, 1);
    if (name->length != 0) host.memcpy_00bf7680(field->data, name->data, field->length);
}

// 009064D0, __thiscall(ECX = scoring, float seconds), RET 4: FISTP(float(scoring[1468h] * s)).
std::int32_t scoring_map_usage_points_009064d0(void* scoring, float seconds) {
    const float product = at<float>(scoring, 0x1468) * seconds;   // FMUL, FSTP float
    return fistp_int32(product);
}

// 00910500, __thiscall(ECX = scoring, category, index), RET 8:
// FISTP(float(config[index] * float(scoring[1464h] * map1428[category]))).
std::int32_t scoring_remaining_unit_points_00910500(Seg59aHost& host, void* scoring, std::int32_t category,
                                                    std::uint32_t index) {
    const float weight = *host.int_float_map_index_0090e740(add(scoring, 0x1428), &category);
    const float scaled = at<float>(scoring, 0x1464) * weight;
    const float points = config_point_scale(host, index) * scaled;
    return fistp_int32(points);
}

// 00910570, __thiscall(ECX = scoring, category, index, count), RET 0Ch. count 0 counts as 1.
// FLD [146Ch], FIMUL count, FMUL map1428[category], FSTP; then * config[index], FISTP. Under
// the main thread's 24-bit precision control each x87 product rounds to float.
std::int32_t scoring_suicide_points_00910570(Seg59aHost& host, void* scoring, std::int32_t category,
                                             std::uint32_t index, std::int32_t count) {
    if (count == 0) count = 1;
    const float weight = *host.int_float_map_index_0090e740(add(scoring, 0x1428), &category);
    const float per = static_cast<float>(static_cast<double>(at<float>(scoring, 0x146C)) * count);
    const float scaled = per * weight;
    const float points = config_point_scale(host, index) * scaled;
    return fistp_int32(points);
}

// 00911C00, __thiscall(ECX = scoring, selector, by, target, float kills, index, byte flag),
// RET 18h. v = map1434[by][target] (009119E0 then 0090E740); selector 0 scales v by [144Ch],
// selector 2 by [1458h]; s = [[game+21A0h]+1478h] when flag else +1474h;
// FISTP(float(float(config[index] * float(v * kills)) * s)).
std::int32_t scoring_kill_points_00911c00(Seg59aHost& host, void* scoring, std::int32_t selector, std::int32_t by,
                                          std::int32_t target, float kills, std::uint32_t index, std::uint8_t flag) {
    void* const inner = host.int_map_map_index_009119e0(add(scoring, 0x1434), &by);
    float v = *host.int_float_map_index_0090e740(inner, &target);
    if (selector == 0) {
        v = at<float>(scoring, 0x144C) * v;
    } else if (selector == 2) {
        v = at<float>(scoring, 0x1458) * v;
    }
    const float s = selected_score_scale(host, flag);
    const float element = config_point_scale(host, index);
    const float t = v * kills;
    const float u = element * t;
    return fistp_int32(u * s);
}

// 00911CE0, __thiscall(ECX = scoring, by, target, selector, int damage, index, byte flag),
// RET 18h. v = float(damage * map1440[by][target]) (FILD, FMUL); selector 0 scales v by
// [1454h], selector 2 by [1460h]; s as in 00911C00; FISTP(float(float(config[index] * v) * s)).
std::int32_t scoring_damage_points_00911ce0(Seg59aHost& host, void* scoring, std::int32_t by, std::int32_t target,
                                            std::int32_t selector, std::int32_t damage, std::uint32_t index,
                                            std::uint8_t flag) {
    void* const inner = host.int_map_map_index_009119e0(add(scoring, 0x1440), &by);
    const float weight = *host.int_float_map_index_0090e740(inner, &target);
    float v = static_cast<float>(static_cast<double>(damage) * weight);
    if (selector == 0) {
        v = at<float>(scoring, 0x1454) * v;
    } else if (selector == 2) {
        v = at<float>(scoring, 0x1460) * v;
    }
    const float s = selected_score_scale(host, flag);
    const float u = config_point_scale(host, index) * v;
    return fistp_int32(u * s);
}

// 0091C560, __thiscall(ECX = scoring), tail JMP 0090FA40. Clears the final-scoring function
// name (Resize 0, memcpy of length bytes from 00CE3A0C), resets the player records (00916980),
// then per slot i: record+254h = i, record+0Ch = 2 in a session (mode != 0) else [game+6ACh]
// (re-read per slot), byte scoring+14B0h+i = 0. Then 00994310 on [00F8A2FC] and the scalar block.
void scoring_reset_for_new_mission_0091c560(Seg59aHost& host, void* scoring) {
    NativeStr* const name = &at<NativeStr>(scoring, 0x147C);
    host.native_string_resize_0041dd40(name, 0, 0);
    if (name->data != nullptr) host.memcpy_00bf7680(name->data, host.empty_text_00ce3a0c(), name->length);
    host.mission_player_records_reset_00916980(scoring);
    for (std::int32_t i = 0; i < 8; ++i) {
        void* const game = host.game_00e188a8();
        const std::int32_t player_mode = at<std::int32_t>(game, 0x1FE4) != 0 ? 2 : at<std::int32_t>(game, 0x6AC);
        const std::size_t step = static_cast<std::size_t>(i) * kSlotRecordStride;
        at<std::int32_t>(scoring, 0x254 + step) = i;                 // [ECX]
        at<std::int32_t>(scoring, 0x00C + step) = player_mode;       // [ECX-248h]
        at<std::uint8_t>(scoring, 0x14B0 + static_cast<std::size_t>(i)) = 0;
    }
    host.stats_owner_reset_00994310(host.stats_owner_00f8a2fc());
    void* const game = host.game_00e188a8();
    const std::int32_t local_slot = at<std::int32_t>(game, 0x1FE4) != 0 ? at<std::int32_t>(game, 0x18EC) : 0;
    at<float>(scoring, 0x1494) = at<float>(scoring, 0x14AC);
    at<std::uint8_t>(scoring, 0x14A4) = 0;
    at<std::uint8_t>(scoring, 0x14A5) = 0;
    at<float>(scoring, 0x149C) = at<float>(scoring, 0x14A8);
    at<std::int32_t>(scoring, 0x1424) = local_slot;
    at<std::uint8_t>(scoring, 0x1498) = 1;
    at<float>(scoring, 0x14A0) = 0.0f;
    host.scoring_reset_tail_0090fa40(scoring);
}

}  // namespace bsp::cc13_seg59a

