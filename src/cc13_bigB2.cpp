// cc13_bigB2: large leftovers of the segment 15-51 sweep (packet cc13-bigB2,
// docs/CC13_BIGB2.md). Names are hypotheses; see the header for the hooks.
//
// x87 note: these bodies run on the main thread, which runs with 24-bit
// precision control (docs/CC13_FPU_PRECISION.md), so each x87 arithmetic step
// is modelled as a float operation in the image's order. Register-register
// x87 operand directions were read from the bytes, not the listing text. EH
// frames are noted, not modelled.

#include "bsp/cc13_bigB2.hpp"

#include <cstring>

namespace bsp {

namespace {

using Str = Cc13SweepBNativeString;
using Byte = std::uint8_t;

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

float f32_bits(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

void* ptr(void* base, std::size_t offset) {
    return at<void*>(base, offset);
}

Byte* addr(void* base, std::size_t offset) {
    return static_cast<Byte*>(base) + offset;
}

// The inlined checked-iterator dereference test: container null, or the node
// is the container's head (end()), calls 00BF6713.
void check_deref(void* container, void* node, Cc13BigB2Host& host) {
    if (container == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node == ptr(container, 4)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
}

// The inlined native-string release: data goes back to the sized pool with
// length + 1 bytes.
void release(Str& text, Cc13BigB2Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined NativeString assignment from a C string (the 0041E870 shape
// without the constructor zeroing): a new length reallocates (pool block
// length + 1, the old block returned afterwards, NUL at the end), length 0
// frees; then the bytes are copied when there is a buffer.
void assign_cstring(Str& target, const char* text, Cc13BigB2Host& host) {
    const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0;
    if (length != target.length) {
        if (length == 0) {
            if (target.data != nullptr) {
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), target.data,
                                                      target.length + 1, 1);
            }
            target.data = nullptr;
            target.length = 0;
        } else {
            void* const pool = host.sized_pool_get_00419cc0();
            char* const buffer = static_cast<char*>(host.sized_pool_allocate_block_00bd1120(pool, length + 1, 1));
            if (target.data != nullptr) {
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), target.data,
                                                      target.length + 1, 1);
            }
            target.data = buffer;
            target.length = length;
            buffer[length] = 0;
        }
    }
    if (target.data != nullptr) {
        host.crt_memcpy_00bf7680(target.data, text, target.length);
    }
}

// The one-byte search set at 00CF100C ("|").
constexpr char kBarSet[] = "|";

// The reverse search 004BCB80 inlined at 006709C5-00670A01 (used only by the
// four-or-fewer path's object source): start at min(length - 1, 7FFFFFFFh)
// and walk down to the first byte; -1 when the buffer is null, the string is
// empty or no byte matches.
std::int32_t rfind_bar_inline(const Str& text) {
    if (text.data == nullptr || text.length == 0) {
        return -1;
    }
    const std::uint32_t start = text.length > 0x7FFFFFFFu ? 0x7FFFFFFFu : text.length - 1;
    const char* cursor = text.data + start;
    for (;;) {
        if (*cursor == kBarSet[0]) {
            return static_cast<std::int32_t>(cursor - text.data);
        }
        if (cursor <= text.data) {
            return -1;
        }
        --cursor;
    }
}

// The slot-label tail shared by the six name sources: keep the text after the
// last '|' when it is at most three bytes (unsigned length - pos - 1 <= 2),
// set it on the slot's text widget +9Ch[slot] and show it; otherwise hide the
// text widget. The substring temporary is released before the text is set.
void show_label_suffix(void* screen, std::uint32_t slot, Str& label, std::int32_t pos, Cc13BigB2Host& host) {
    const std::uint32_t tail = label.length - static_cast<std::uint32_t>(pos);
    if (tail - 1 > 2) {
        host.widget_vcall_34_set_visible(ptr(screen, 0x9C + slot * 4), 0);
        return;
    }
    Str piece{};
    Str* const got = static_cast<Str*>(host.native_string_substring_00469840(
        &label, &piece, pos + 1, static_cast<std::int32_t>(tail - 1)));
    if (&label != got) {
        host.native_string_resize_0041dd40(&label, got->length, 1);
        if (got->length != 0) {
            host.crt_memcpy_00bf7680(label.data, got->data, label.length);
        }
    }
    release(piece, host);
    host.gui_text_set_localised_source_00abaed0(ptr(screen, 0x9C + slot * 4), &label, 1);
    host.widget_vcall_34_set_visible(ptr(screen, 0x9C + slot * 4), 1);
}

// The checked ++ on the cursor {+350h container, +354h node} over the list at
// +344h (head +348h), wrapping from end() to begin().
void advance_cursor(void* screen, Cc13BigB2Host& host) {
    check_deref(ptr(screen, 0x350), ptr(screen, 0x354), host);
    at<void*>(screen, 0x354) = ptr(ptr(screen, 0x354), 0);
    void* const container = ptr(screen, 0x350);
    void* const head = ptr(screen, 0x348);
    void* const self = addr(screen, 0x344);
    if (container == nullptr || container != self) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (ptr(screen, 0x354) == head) {
        at<void*>(screen, 0x350) = self;
        at<void*>(screen, 0x354) = ptr(ptr(screen, 0x348), 0);
    }
}

// One slot label (00670203-00670826 for more than four entries,
// 006708A5-00670F36 for four or fewer). The cursor value (node+8h) is
// {object +0, std::list +4 (head at node+10h), widget +10h}. Name sources:
// the object's +14h name; else the list's first element e: when e+3Ch is set
// and (e+3Ch)->+5Ch(9) is false, (e+3Ch)->+14h(); when e+3Ch is null and e
// is non-null, e->+14h(). Every access repeats the inlined checks.
void slot_label(void* screen, std::uint32_t slot, bool inline_object_rfind, Cc13BigB2Host& host) {
    Str label{0, nullptr};
    auto check = [&]() { check_deref(ptr(screen, 0x350), ptr(screen, 0x354), host); };
    auto node = [&]() { return ptr(screen, 0x354); };
    auto first_element = [&]() -> void* {
        void* const head = ptr(node(), 0x10);
        void* const first = ptr(head, 0);
        if (first == head) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        return ptr(first, 8);
    };

    check();
    if (ptr(node(), 8) != nullptr) {
        check();
        assign_cstring(label, host.object_vcall_14_name(ptr(node(), 8)), host);
        const std::int32_t pos = inline_object_rfind
                                     ? rfind_bar_inline(label)
                                     : host.native_string_rfind_any_004bcb80(&label, kBarSet, 0x7FFFFFFFu);
        show_label_suffix(screen, slot, label, pos, host);
    } else {
        check();
        if (ptr(first_element(), 0x3C) != nullptr) {
            check();
            if (!host.object_vcall_5c_is_kind(ptr(first_element(), 0x3C), 9)) {
                check();
                assign_cstring(label, host.object_vcall_14_name(ptr(first_element(), 0x3C)), host);
                const std::int32_t pos = host.native_string_rfind_any_004bcb80(&label, kBarSet, 0x7FFFFFFFu);
                show_label_suffix(screen, slot, label, pos, host);
            }
        } else {
            check();
            if (first_element() != nullptr) {
                check();
                assign_cstring(label, host.object_vcall_14_name(first_element()), host);
                const std::int32_t pos = host.native_string_rfind_any_004bcb80(&label, kBarSet, 0x7FFFFFFFu);
                show_label_suffix(screen, slot, label, pos, host);
            }
        }
    }
    advance_cursor(screen, host);
    release(label, host);
}

// The inlined world-matrix cache of an entity (006712E1-006713B5): when the
// valid byte +C8h is clear, refresh the parent +3Ch (00414DB0), take local
// +74h times the parent's world +CCh (00413920) or the local matrix alone,
// copy the 16 floats to +CCh, set +C8h and clear +10Ch.
void refresh_world_cache(void* entity, Cc13BigB2Host& host) {
    if (at<Byte>(entity, 0xC8) != 0) {
        return;
    }
    if (ptr(entity, 0x3C) != nullptr) {
        host.entity_pose_refresh_world_00414db0(ptr(entity, 0x3C));
    }
    float product[16];
    const float* source;
    if (ptr(entity, 0x3C) != nullptr) {
        source = host.matrix_multiply_00413920(reinterpret_cast<const float*>(addr(entity, 0x74)), product,
                                               reinterpret_cast<const float*>(addr(ptr(entity, 0x3C), 0xCC)));
    } else {
        source = reinterpret_cast<const float*>(addr(entity, 0x74));
    }
    for (std::size_t i = 0; i < 16; ++i) {
        at<float>(entity, 0xCC + i * 4) = source[i];
    }
    at<Byte>(entity, 0xC8) = 1;
    at<Byte>(entity, 0x10C) = 0;
}

// The icon-state cache at screen+42Ch (std::map<int, int>, head +430h) keyed
// by *(owner->+538h + 70h): a miss adds a state to the icon +1CCh with the
// texture (owner->+538h)->+D4h and uv {0, 0, 1, 1} and stores its index.
// `recheck` repeats the caller's inlined cursor test before each owner read
// (the +1F4h block does, the list loop does not).
template <class Recheck, class Owner>
std::int32_t icon_state_for(void* screen, Owner owner, Recheck recheck, Cc13BigB2Host& host) {
    void* const map = addr(screen, 0x42C);
    void* const head = ptr(screen, 0x430);
    recheck();
    void* result[2];
    void** const found = static_cast<void**>(
        host.icon_map_find_00614060(map, result, addr(ptr(owner(), 0x538), 0x70)));
    if (found[0] == nullptr || found[0] != map) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (found[1] == head) {
        const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        recheck();
        const std::int32_t state = host.gui_icon_add_state_index_00ab64c0(
            ptr(screen, 0x1CC), ptr(ptr(owner(), 0x538), 0xD4), uv);
        recheck();
        *host.icon_map_subscript_006171b0(map, addr(ptr(owner(), 0x538), 0x70)) = state;
        return state;
    }
    recheck();
    return *host.icon_map_subscript_006171b0(map, addr(ptr(owner(), 0x538), 0x70));
}

// A by-value copy of a cursor-list value: {object, std::list (proxy, head,
// size), widget}, as the image builds it on the stack at [esp+3Ch].
struct ValueCopy {
    void* object;
    void* list[3];
    void* widget;
};

void* player_of(Cc13BigB2Host& host) {
    void* const game = host.global_00e188a8();
    return ptr(game, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)) * 4);
}

}  // namespace

// ---------------------------------------------------------------------------
// 00670020 support-manager screen refresh (hypothesis)
// __fastcall(ECX = screen), RET. Frame: EBP-aligned, 0E4h locals, EH handler
// 00C7BF30 (states 0-10h guard the label strings, the list copies and the
// "InferiorFailure" temporaries; not modelled). Main thread, 24-bit x87.
// Callers: 00671D60, 006723F0 (close fade), 00674B80 (Enter), 00675C40
// (cc9's per-frame update).
//
// Screen fields: slot widgets +8Ch[4], slot text +9Ch[4], +0CCh[4] page
// marks (shown at +30Ch), selected-unit icon +1CCh, gauges +12Ch/+130h/+134h,
// unit list cursor +300h/+304h (node+8h = unit), unit -> widget map +3FCh,
// value list +338h (head +33Ch), the cursor list +344h (head +348h, size
// +34Ch) with cursor +350h/+354h, the slot offset +30Ch, the last count
// +45Ch, the icon-state map +42Ch, the z +2D0h.
//
// 0067004C-00670096 every slot: +88h(3, 0, 1.0), +34h(1), text +34h(0),
// mark +34h(0). +1F4h set: 0065A550. With entries (+34Ch): 0066FB00; a count
// change while +2B1h is clear resets the cursor to begin(), +308h = 0, +288h
// = 1, +28Ch = 1.0f; +2B1h = 0; +45Ch = count; 0065A2B0; then the cursor
// steps back +30Ch times (checked --, wrapping begin() to end()). More than
// four entries: slots 0..3 get +88h(0, 0, 1.0) and a label each. Four or
// fewer: every slot +34h(0); the first `count` slots +88h(0, 0, 1.0),
// +34h(1) and a label; the rest +88h(3, 0, 1.0) and text +34h(0). Labels:
// slot_label() above; the cursor advances once per slot.
//
// 00670F90: 0065A2B0; the icon state ([esp+1Ch]) is 0. +1F4h set: the
// cursor unit kind 46h -> icon +88h(0, 0, 1.0); kind 9 -> icon +88h(state,
// 0, 1.0) with the cached state; kind 45h -> icon +88h(2, 0, 1.0). Then
// every marker of the +3FCh map: +88h(0 when its unit is the cursor unit,
// else 1; 0, 1.0), +34h(1), and the unit's cached world translation
// (+0FCh/+100h/+104h) through 00658580 to 00AA7D00.
//
// 00671425 (entries only): the cursor value is copied (object, 005AAAF0 list
// copy, widget), its list copied again (005AAAF0), and the first copy's list
// destroyed inline (nodes and head through 00BF65AC). For each unit of that
// list: a fresh list (004E79F0) is assigned, inline clear plus 00664AF0,
// from unit+328h when the local player's +28h is 0, else unit+31Ch. Empty
// (or the same list): +204h is set. Otherwise the unit's +3Ch target t:
// kind 46h -> t->+198h("InferiorFailure", 0, &flag), and with the flag the
// health 00923BE0 (float-stored) as h46; kind 45h -> the same into h45;
// kind 9 -> when t+54h equals the player's +28h, the cached icon state;
// each kind sets its "seen" byte. The fresh list goes through 004E8150.
// Gauges +12Ch/+130h/+134h then +34h(0), and:
//   45h and 46h seen: icon +88h(3, 0, 1.0); h46 -> +134h shown, quad
//     ((1 - h46) * 0.5, [00CF6B0C] = 3F49096Ch (0.78530), 0, 1), +4Ch(0.4f); h45 ->
//     +130h shown, quad((1 - h45) * 0.5, [00CF6B08] = 407B50B1h (3.92680), 0, 1),
//     +4Ch(0.4f);
//   only 46h: icon +88h(0, 0, 1.0); h46 -> +12Ch shown, quad(1 - h46, 0, 0,
//     1), +4Ch(0.4f);
//   otherwise kind 9 seen: icon +88h(state or 1 when the state is 0, 0,
//     1.0); else 45h seen: icon +88h(2, 0, 1.0); h45 -> +12Ch shown, quad(1 -
//     h45, 0, 0, 1), +4Ch(0.4f).
// The list copy is tidied (005A0B80). Still with entries: every +338h
// widget +34h(0); every +344h widget +88h(1, 0, 1.0) then +34h(1) (each
// through a value copy and 005A0B80); the cursor widget +88h(0, 0, 1.0) and
// its resolved position (00AA6750) set back with z = +2D0h (00AA8240).
// Finally the page mark +0CCh[+30Ch] +34h(1).
//
// x87: 1.0 - h (DE E1 = FSUBRP st(1), st: st(1) = st - st(1)) and the
// FMUL by the double 0.5 at 00D7A280 each round to float under 24-bit PC.
// Uncertain: the tag dword pushed to 00664AF0 is a stale stack slot in the
// image (passed as 0 here; the callee ignores the empty tag). The x87 copies
// of the 16 matrix floats are modelled as float copies (an SNaN would be
// quieted by FLD in the image). No RNG draws (00BD2F10 is not called).
void cc13_bigB2_hud_support_refresh_view_00670020(void* screen, Cc13BigB2Host& host) {
    host.hud_support_sync_cursor_iterators_0065a2b0(screen);
    for (std::uint32_t k = 0; k < 4; ++k) {
        host.widget_vcall_88(ptr(screen, 0x8C + k * 4), 3, 0, 1.0f);
        host.widget_vcall_34_set_visible(ptr(screen, 0x8C + k * 4), 1);
        host.widget_vcall_34_set_visible(ptr(screen, 0x9C + k * 4), 0);
        host.widget_vcall_34_set_visible(ptr(screen, 0xCC + k * 4), 0);
    }
    if (at<std::int32_t>(screen, 0x1F4) != 0) {
        host.hud_support_update_slot_strip_0065a550(screen);
    }

    if (at<std::uint32_t>(screen, 0x34C) != 0) {
        host.hud_support_refresh_0066fb00(screen);
        const std::uint32_t count = at<std::uint32_t>(screen, 0x34C);
        if (at<std::uint32_t>(screen, 0x45C) != count && at<Byte>(screen, 0x2B1) == 0) {
            void* const begin = ptr(ptr(screen, 0x348), 0);
            at<void*>(screen, 0x350) = addr(screen, 0x344);
            at<void*>(screen, 0x354) = begin;
            at<std::int32_t>(screen, 0x308) = 0;
            at<Byte>(screen, 0x288) = 1;
            at<float>(screen, 0x28C) = 1.0f;
        }
        at<Byte>(screen, 0x2B1) = 0;
        at<std::uint32_t>(screen, 0x45C) = count;
        host.hud_support_sync_cursor_iterators_0065a2b0(screen);

        // 0067012C: step the cursor back +30Ch times.
        for (std::int32_t i = 0; i < at<std::int32_t>(screen, 0x30C); ++i) {
            void* const self = addr(screen, 0x344);
            void* const begin = ptr(ptr(screen, 0x348), 0);
            void* const container = ptr(screen, 0x350);
            if (container == nullptr || container != self) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (ptr(screen, 0x354) == begin) {
                at<void*>(screen, 0x350) = self;
                at<void*>(screen, 0x354) = ptr(screen, 0x348);
            }
            if (ptr(screen, 0x350) == nullptr) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* const previous = ptr(ptr(screen, 0x354), 4);
            at<void*>(screen, 0x354) = previous;
            if (previous == ptr(ptr(screen, 0x350), 4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
        }

        if (at<std::uint32_t>(screen, 0x34C) > 4) {
            for (std::uint32_t slot = 0; slot < 4; ++slot) {
                host.widget_vcall_88(ptr(screen, 0x8C + slot * 4), 0, 0, 1.0f);
                slot_label(screen, slot, false, host);
            }
        } else {
            for (std::uint32_t k = 0; k < 4; ++k) {
                host.widget_vcall_34_set_visible(ptr(screen, 0x8C + k * 4), 0);
            }
            for (std::uint32_t slot = 0; slot < at<std::uint32_t>(screen, 0x34C); ++slot) {
                host.widget_vcall_88(ptr(screen, 0x8C + slot * 4), 0, 0, 1.0f);
                host.widget_vcall_34_set_visible(ptr(screen, 0x8C + slot * 4), 1);
                slot_label(screen, slot, true, host);
            }
            const std::int32_t shown = at<std::int32_t>(screen, 0x34C);
            if (shown < 4) {
                for (std::int32_t slot = shown; slot < 4; ++slot) {
                    host.widget_vcall_88(ptr(screen, 0x8C + static_cast<std::size_t>(slot) * 4), 3, 0, 1.0f);
                    host.widget_vcall_34_set_visible(ptr(screen, 0x9C + static_cast<std::size_t>(slot) * 4), 0);
                }
            }
        }
    }

    // 00670F90
    host.hud_support_sync_cursor_iterators_0065a2b0(screen);
    std::int32_t icon_state = 0;
    if (at<std::int32_t>(screen, 0x1F4) != 0) {
        auto unit_check = [&]() { check_deref(ptr(screen, 0x300), ptr(screen, 0x304), host); };
        auto cursor_unit = [&]() { return ptr(ptr(screen, 0x304), 8); };
        unit_check();
        if (host.object_vcall_5c_is_kind(cursor_unit(), 0x46)) {
            host.widget_vcall_88(ptr(screen, 0x1CC), 0, 0, 1.0f);
        } else {
            unit_check();
            if (host.object_vcall_5c_is_kind(cursor_unit(), 9)) {
                icon_state = icon_state_for(screen, cursor_unit, unit_check, host);
                host.widget_vcall_88(ptr(screen, 0x1CC), icon_state, 0, 1.0f);
            } else {
                unit_check();
                if (host.object_vcall_5c_is_kind(cursor_unit(), 0x45)) {
                    host.widget_vcall_88(ptr(screen, 0x1CC), 2, 0, 1.0f);
                }
            }
        }

        // 00671209: the map markers ({+3FCh, head->left} checked iterator).
        void* const tree = addr(screen, 0x3FC);
        void* iterator[2] = {tree, ptr(ptr(screen, 0x400), 0)};
        for (;;) {
            void* const container = iterator[0];
            void* const node = iterator[1];
            void* const end = ptr(screen, 0x400);
            if (container == nullptr || container != tree) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (node == end) {
                break;
            }
            check_deref(container, node, host);
            unit_check();
            const std::int32_t state = ptr(node, 0x0C) == cursor_unit() ? 0 : 1;
            if (node == ptr(container, 4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            host.widget_vcall_88(ptr(node, 0x10), state, 0, 1.0f);
            if (node == ptr(container, 4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            host.widget_vcall_34_set_visible(ptr(node, 0x10), 1);
            if (node == ptr(container, 4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* const unit = ptr(node, 0x0C);
            refresh_world_cache(unit, host);
            float map_xy[2];
            host.hud_support_world_to_map_00658580(screen, map_xy, at<float>(unit, 0xFC), at<float>(unit, 0x100),
                                                   at<float>(unit, 0x104));
            if (node == ptr(container, 4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            host.widget_set_local_xy_00aa7d00(ptr(node, 0x10), map_xy[0], map_xy[1]);
            host.tree_iterator_increment_00654650(iterator);
        }
    }

    if (at<std::uint32_t>(screen, 0x34C) != 0) {
        // 00671432: copy the cursor value, copy its list, destroy the first
        // copy's list inline.
        check_deref(ptr(screen, 0x350), ptr(screen, 0x354), host);
        Byte* const value = addr(ptr(screen, 0x354), 8);
        ValueCopy first_copy;
        first_copy.object = ptr(value, 0);
        host.list_copy_005aaaf0(first_copy.list, value + 4);
        first_copy.widget = ptr(value, 0x10);
        void* units[3];
        host.list_copy_005aaaf0(units, first_copy.list);
        {
            void* const head = first_copy.list[1];
            void* node = ptr(head, 0);
            at<void*>(head, 0) = head;
            at<void*>(head, 4) = head;
            first_copy.list[2] = nullptr;
            while (node != first_copy.list[1]) {
                void* const next = ptr(node, 0);
                host.crt_operator_delete_00bf65ac(node);
                node = next;
            }
            host.crt_operator_delete_00bf65ac(first_copy.list[1]);
            first_copy.list[1] = nullptr;
        }

        bool seen_45 = false;      // [esp+23h]
        bool health_46 = false;    // [esp+24h]
        bool health_45 = false;    // [esp+25h]
        bool seen_46 = false;      // [esp+26h]
        bool seen_9 = false;       // [esp+27h]
        float h46 = 0.0f;          // [esp+68h]
        float h45 = 0.0f;          // [esp+60h]
        if (reinterpret_cast<std::uintptr_t>(units[2]) != 0) {
            void* node = ptr(units[1], 0);
            while (node != units[1]) {
                void* const unit = ptr(node, 8);
                void* fresh[3];
                fresh[1] = host.list_buy_head_node_004e79f0(fresh);
                fresh[2] = nullptr;
                void* const source =
                    at<std::int32_t>(player_of(host), 0x28) == 0 ? addr(unit, 0x328) : addr(unit, 0x31C);
                bool filled = false;
                if (static_cast<void*>(fresh) != source) {
                    void* const source_head = ptr(source, 4);
                    void* const source_begin = ptr(source_head, 0);
                    void* old = ptr(fresh[1], 0);
                    at<void*>(fresh[1], 0) = fresh[1];
                    at<void*>(fresh[1], 4) = fresh[1];
                    fresh[2] = nullptr;
                    while (old != fresh[1]) {
                        void* const next = ptr(old, 0);
                        host.crt_operator_delete_00bf65ac(old);
                        old = next;
                    }
                    host.list_insert_range_00664af0(fresh, fresh, ptr(fresh[1], 0), source, source_begin, source,
                                                    source_head, 0);
                    filled = reinterpret_cast<std::uintptr_t>(fresh[2]) != 0;
                }
                if (!filled) {
                    if (at<Byte>(screen, 0x204) == 0) {
                        at<Byte>(screen, 0x204) = 1;
                    }
                } else if (ptr(unit, 0x3C) != nullptr) {
                    std::uint8_t flag = 0;  // [esp+22h]
                    if (host.object_vcall_5c_is_kind(ptr(unit, 0x3C), 0x46)) {
                        Str name;
                        host.native_string_assign_0041e870(&name, "InferiorFailure");
                        host.unit_vcall_198_query(ptr(unit, 0x3C), &name, 0, &flag);
                        release(name, host);
                        seen_46 = true;
                        if (flag != 0) {
                            health_46 = true;
                            h46 = static_cast<float>(host.unit_health_00923be0(ptr(unit, 0x3C)));
                        }
                    } else if (host.object_vcall_5c_is_kind(ptr(unit, 0x3C), 0x45)) {
                        Str name;
                        host.native_string_assign_0041e870(&name, "InferiorFailure");
                        host.unit_vcall_198_query(ptr(unit, 0x3C), &name, 0, &flag);
                        release(name, host);
                        seen_45 = true;
                        if (flag != 0) {
                            health_45 = true;
                            h45 = static_cast<float>(host.unit_health_00923be0(ptr(unit, 0x3C)));
                        }
                    } else if (host.object_vcall_5c_is_kind(ptr(unit, 0x3C), 9)) {
                        void* const player = player_of(host);
                        if (at<std::int32_t>(ptr(unit, 0x3C), 0x54) == at<std::int32_t>(player, 0x28)) {
                            icon_state = icon_state_for(
                                screen, [&]() { return ptr(unit, 0x3C); }, []() {}, host);
                        }
                        seen_9 = true;
                    }
                }
                host.list_clear_004e8150(fresh);
                if (node == units[1]) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                node = ptr(node, 0);
            }
        }

        // 006718BF: the gauges.
        host.widget_vcall_34_set_visible(ptr(screen, 0x12C), 0);
        host.widget_vcall_34_set_visible(ptr(screen, 0x130), 0);
        host.widget_vcall_34_set_visible(ptr(screen, 0x134), 0);
        const float kQuarterPi = f32_bits(0x3F49096Cu);       // 00CF6B0C
        const float kFiveQuarterPi = f32_bits(0x407B50B1u);   // 00CF6B08
        const float kGaugeFloat = f32_bits(0x3ECCCCCDu);      // 00CE7804
        if (seen_45 && seen_46) {
            host.widget_vcall_88(ptr(screen, 0x1CC), 3, 0, 1.0f);
            if (health_46) {
                host.widget_vcall_34_set_visible(ptr(screen, 0x134), 1);
                const float rest = 1.0f - h46;
                const float half = static_cast<float>(static_cast<double>(rest) * 0.5);
                host.gui_widget_set_quad_floats_00abe6e0(ptr(screen, 0x134), half, kQuarterPi, 0.0f, 1.0f);
                host.widget_vcall_4c_set_float(ptr(screen, 0x134), kGaugeFloat);
            }
            if (health_45) {
                host.widget_vcall_34_set_visible(ptr(screen, 0x130), 1);
                const float rest = 1.0f - h45;
                const float half = static_cast<float>(static_cast<double>(rest) * 0.5);
                host.gui_widget_set_quad_floats_00abe6e0(ptr(screen, 0x130), half, kFiveQuarterPi, 0.0f, 1.0f);
                host.widget_vcall_4c_set_float(ptr(screen, 0x130), kGaugeFloat);
            }
        } else if (!seen_45 && seen_46) {
            host.widget_vcall_88(ptr(screen, 0x1CC), 0, 0, 1.0f);
            if (health_46) {
                host.widget_vcall_34_set_visible(ptr(screen, 0x12C), 1);
                host.gui_widget_set_quad_floats_00abe6e0(ptr(screen, 0x12C), 1.0f - h46, 0.0f, 0.0f, 1.0f);
                host.widget_vcall_4c_set_float(ptr(screen, 0x12C), kGaugeFloat);
            }
        } else if (seen_9) {
            host.widget_vcall_88(ptr(screen, 0x1CC), icon_state != 0 ? icon_state : 1, 0, 1.0f);
        } else if (seen_45) {
            host.widget_vcall_88(ptr(screen, 0x1CC), 2, 0, 1.0f);
            if (health_45) {
                host.widget_vcall_34_set_visible(ptr(screen, 0x12C), 1);
                host.gui_widget_set_quad_floats_00abe6e0(ptr(screen, 0x12C), 1.0f - h45, 0.0f, 0.0f, 1.0f);
                host.widget_vcall_4c_set_float(ptr(screen, 0x12C), kGaugeFloat);
            }
        }
        host.list_tidy_005a0b80(units);

        if (at<std::uint32_t>(screen, 0x34C) != 0) {
            // 00671B1C: hide the +338h widgets.
            void* const hidden_list = addr(screen, 0x338);
            for (void* node = ptr(ptr(screen, 0x33C), 0); node != ptr(hidden_list, 4); node = ptr(node, 0)) {
                if (node == ptr(hidden_list, 4)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                ValueCopy copy;
                copy.object = ptr(node, 8);
                host.list_copy_005aaaf0(copy.list, addr(node, 0x0C));
                copy.widget = ptr(node, 0x18);
                host.widget_vcall_34_set_visible(copy.widget, 0);
                host.list_tidy_005a0b80(copy.list);
                if (node == ptr(hidden_list, 4)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
            }
            // 00671B9B: show the +344h widgets.
            void* const shown_list = addr(screen, 0x344);
            for (void* node = ptr(ptr(screen, 0x348), 0); node != ptr(shown_list, 4); node = ptr(node, 0)) {
                if (node == ptr(shown_list, 4)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                ValueCopy copy;
                copy.object = ptr(node, 8);
                host.list_copy_005aaaf0(copy.list, addr(node, 0x0C));
                copy.widget = ptr(node, 0x18);
                host.widget_vcall_88(copy.widget, 1, 0, 1.0f);
                host.list_tidy_005a0b80(copy.list);
                if (node == ptr(shown_list, 4)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                ValueCopy again;
                again.object = ptr(node, 8);
                host.list_copy_005aaaf0(again.list, addr(node, 0x0C));
                again.widget = ptr(node, 0x18);
                host.widget_vcall_34_set_visible(again.widget, 1);
                host.list_tidy_005a0b80(again.list);
                if (node == ptr(shown_list, 4)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
            }
            // 00671C75: the cursor widget.
            check_deref(ptr(screen, 0x350), ptr(screen, 0x354), host);
            Byte* const cursor_value = addr(ptr(screen, 0x354), 8);
            ValueCopy current;
            current.object = ptr(cursor_value, 0);
            host.list_copy_005aaaf0(current.list, cursor_value + 4);
            current.widget = ptr(cursor_value, 0x10);
            host.list_tidy_005a0b80(current.list);
            host.widget_vcall_88(current.widget, 0, 0, 1.0f);
            float resolved[3];
            host.widget_get_resolved_position_00aa6750(current.widget, resolved);
            const float position[3] = {resolved[0], resolved[1], at<float>(screen, 0x2D0)};
            host.widget_set_resolved_position_00aa8240(current.widget, position);
        }
    }

    // 00671D23
    host.widget_vcall_34_set_visible(ptr(screen, 0xCC + static_cast<std::size_t>(at<std::int32_t>(screen, 0x30C)) * 4),
                                     1);
}

namespace {

// A 14h-byte LuaPlus LuaObject value, constructed and destroyed only through
// the host.
struct LuaObj {
    std::uint8_t bytes[0x14];
};

// The std::vector<int> of objective keys on the stack at [esp+30h]: {proxy,
// first, last, end}; the image zeroes first/last/end and leaves the proxy.
struct KeyVector {
    void* proxy;
    std::int32_t* first;
    std::int32_t* last;
    std::int32_t* end;
};

// One objective category: its table key and the four parallel image vectors
// (names: NativeString, bits: vector<bool>, ints: vector<int>, ids:
// NativeString). `primary_shape` selects the primary block's code shape: it
// calls 00442190 / 0049F2D0 / 0041E870 (party path) and fetches the table
// straight into the holder, where the other two blocks inline those or
// assign through a temporary.
struct ObjectiveCategory {
    const char* key;
    std::uint32_t names;
    std::uint32_t bits;
    std::uint32_t ints;
    std::uint32_t ids;
    bool primary_shape;
};

bool above(const void* a, const void* b) {
    return reinterpret_cast<std::uintptr_t>(a) > reinterpret_cast<std::uintptr_t>(b);
}

// The inlined NativeString(const char*) body (0041E870 without the call):
// zero, 0041DD40 resize to strlen with preserve 1, then copy length + 1 bytes.
void construct_cstring_inline(Str& target, const char* text, Cc13BigB2Host& host) {
    target.length = 0;
    target.data = nullptr;
    host.native_string_resize_0041dd40(&target, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (target.data != nullptr) {
        host.crt_memcpy_00bf7680(target.data, text, target.length + 1);
    }
}

// The vector<bool> push: 0049F2D0 in the primary block; elsewhere the inlined
// insert(end(), value): end() is {vector, first word, 0} (checked first <=
// last) advanced by the size (0048D3B0, only when non-zero), then 0049E150.
void push_objective_bit(const ObjectiveCategory& c, bool value, Cc13BigB2Host& host) {
    void* const bits = host.image_global(c.bits);
    if (c.primary_shape) {
        host.bit_vector_push_back_0049f2d0(bits, value ? 1 : 0);
        return;
    }
    void* const first_word = ptr(bits, 8);
    if (above(first_word, ptr(bits, 0x0C))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    struct {
        void* container;
        void* word;
        std::uint32_t offset;
    } where{bits, first_word, 0};
    const std::uint32_t size = at<std::uint32_t>(bits, 0);
    if (size > 0) {
        host.bit_iterator_advance_0048d3b0(&where, static_cast<std::int32_t>(size));
    }
    void* result[3];
    host.bit_vector_insert_0049e150(bits, result, where.container, where.word, where.offset, value ? 1 : 0);
}

// One objective entry: names += char('@' + letter) + obj.Text, bits +=
// obj.Success (false when nil), ints += value, ids += obj.ID. `party` is the
// per-player path of a Party objective (the primary block calls 0041E870 for
// both strings there; every other place inlines it).
void emit_objective_entry(const ObjectiveCategory& c, void* obj, std::uint32_t letter, std::int32_t value,
                          bool party, Cc13BigB2Host& host) {
    const bool call_construct = c.primary_shape && party;
    {
        LuaObj field;
        const char* const text = host.lua_object_get_string_00b662b0(
            host.lua_object_get_field_00b67800(obj, &field, "Text"));
        Str text_string;
        if (call_construct) {
            host.native_string_assign_0041e870(&text_string, text);
        } else {
            construct_cstring_inline(text_string, text, host);
        }
        Str letter_string;
        Str joined;
        void* const lhs = host.native_string_construct_byte_00531030(
            &letter_string, static_cast<std::int8_t>(static_cast<std::uint8_t>(letter) + 0x40));
        void* const result = host.native_string_concat_004261a0(lhs, &joined, &text_string);
        host.string_vector_append_00450540(host.image_global(c.names), result);
        release(joined, host);
        release(letter_string, host);
        release(text_string, host);
        host.lua_object_destroy_00b67700(&field);
    }
    {
        LuaObj outer;
        LuaObj inner;
        bool made_inner = false;
        bool success = false;
        if (!host.lua_object_is_nil_00b65fb0(host.lua_object_get_field_00b67800(obj, &outer, "Success"))) {
            made_inner = true;
            success = host.lua_object_get_boolean_00b66250(
                          host.lua_object_get_field_00b67800(obj, &inner, "Success")) != 0;
        }
        push_objective_bit(c, success, host);
        if (made_inner) {
            host.lua_object_destroy_00b67700(&inner);
        }
        host.lua_object_destroy_00b67700(&outer);
    }
    const std::int32_t pushed = value;
    host.int_vector_push_back_006257d0(host.image_global(c.ints), &pushed);
    {
        LuaObj field;
        const char* const id = host.lua_object_get_string_00b662b0(host.lua_object_get_field_00b67800(obj, &field, "ID"));
        Str id_string;
        if (call_construct) {
            host.native_string_assign_0041e870(&id_string, id);
        } else {
            construct_cstring_inline(id_string, id, host);
        }
        host.string_vector_append_00450540(host.image_global(c.ids), &id_string);
        release(id_string, host);
        host.lua_object_destroy_00b67700(&field);
    }
}

// The begin()/end() checks on an int vector before a range use: nested in
// the image (the second test only runs after a failed first one).
void check_key_range(const KeyVector& keys, Cc13BigB2Host& host) {
    if (above(keys.first, keys.last)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
        if (above(keys.first, keys.last)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    }
}

// One category block (primary 00627E3E, secondary 00628768, hidden
// 006291F7).
void rebuild_objective_category(const ObjectiveCategory& c, void* objectives, LuaObj& table, KeyVector& keys,
                                Cc13BigB2Host& host) {
    if (c.primary_shape) {
        host.lua_object_get_field_00b67800(objectives, &table, c.key);
    } else {
        LuaObj fetched;
        host.lua_object_assign_00b67690(&table, host.lua_object_get_field_00b67800(objectives, &fetched, c.key));
        host.lua_object_destroy_00b67700(&fetched);
    }
    if (host.lua_object_is_nil_00b65fb0(&table)) {
        return;
    }

    // Collect the integer keys.
    LuaObj key;
    LuaObj value;
    host.lua_object_construct_00b65f50(&key);
    host.lua_object_construct_00b65f50(&value);
    host.lua_object_iterate_first_00b67080(&table, &key, &value);
    while (!host.lua_object_is_unbound_00b66420(&table, &key)) {
        const std::int32_t k = host.lua_object_get_integer_00b66290(&key);
        if (c.primary_shape) {
            host.dword_vector_push_back_00442190(&keys, &k);
        } else {
            std::int32_t* const first = keys.first;
            std::int32_t* const last = keys.last;
            if (first != nullptr &&
                static_cast<std::uint32_t>(last - first) < static_cast<std::uint32_t>(keys.end - first)) {
                *last = k;
                keys.last = last + 1;
            } else {
                if (above(first, last)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                void* result[2];
                host.dword_vector_insert_one_00441f30(&keys, result, &keys, last, &k);
            }
        }
        host.lua_object_iterate_next_00b67190(&table, &key, &value);
    }
    check_key_range(keys, host);
    host.int_sort_00623170(keys.first, keys.last, static_cast<std::int32_t>(keys.last - keys.first));

    for (std::uint32_t index = 0;
         keys.first != nullptr && index < static_cast<std::uint32_t>(keys.last - keys.first); ++index) {
        LuaObj obj;
        host.lua_object_get_by_index_00b67720(&table, &obj, keys.first[index]);

        // HideFromRewards nil and Active true.
        bool counted = false;
        {
            LuaObj hide;
            LuaObj active;
            bool made_active = false;
            if (host.lua_object_is_nil_00b65fb0(host.lua_object_get_field_00b67800(&obj, &hide, "HideFromRewards"))) {
                made_active = true;
                counted = host.lua_object_get_boolean_00b66250(
                              host.lua_object_get_field_00b67800(&obj, &active, "Active")) != 0;
            }
            if (made_active) {
                host.lua_object_destroy_00b67700(&active);
            }
            host.lua_object_destroy_00b67700(&hide);
        }
        if (counted) {
            LuaObj field;
            const bool has_index =
                !host.lua_object_is_nil_00b65fb0(host.lua_object_get_field_00b67800(&obj, &field, "PlayerIndex"));
            host.lua_object_destroy_00b67700(&field);
            std::int32_t player_index = 0;
            bool single = true;
            if (has_index) {
                player_index = host.lua_object_get_integer_00b66290(
                    host.lua_object_get_field_00b67800(&obj, &field, "PlayerIndex"));
                host.lua_object_destroy_00b67700(&field);
            } else {
                const bool has_party =
                    !host.lua_object_is_nil_00b65fb0(host.lua_object_get_field_00b67800(&obj, &field, "Party"));
                host.lua_object_destroy_00b67700(&field);
                if (has_party) {
                    single = false;
                    const std::int32_t party = host.lua_object_get_integer_00b66290(
                        host.lua_object_get_field_00b67800(&obj, &field, "Party"));
                    host.lua_object_destroy_00b67700(&field);
                    std::uint32_t slot = 0;
                    for (std::int32_t offset = 0x18CC; offset < 0x18EC; offset += 4, ++slot) {
                        void* const player = ptr(host.global_00e188a8(), static_cast<std::size_t>(offset));
                        if (at<std::int32_t>(player, 0x28) == party) {
                            emit_objective_entry(c, &obj, slot, static_cast<std::int32_t>(slot), true, host);
                        }
                    }
                } else {
                    void* const game = host.global_00e188a8();
                    void* const local = ptr(game, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)) * 4);
                    player_index = at<std::int32_t>(local, 0x20);
                }
            }
            if (single && player_index >= 0) {
                emit_objective_entry(c, &obj, index, player_index, false, host);
            }
        }
        host.lua_object_destroy_00b67700(&obj);
    }

    // keys.erase(begin(), end()) inlined: the moved tail is always empty, so
    // memmove_s never runs and last becomes first.
    check_key_range(keys, host);
    std::int32_t* const erase_end = keys.last;
    if (keys.first != erase_end) {
        const std::int32_t tail = static_cast<std::int32_t>(keys.last - erase_end);
        std::int32_t* const new_last = keys.first + tail;
        if (tail > 0) {
            host.crt_memmove_s_00bf67a7(keys.first, static_cast<std::uint32_t>(tail) * 4, erase_end,
                                        static_cast<std::uint32_t>(tail) * 4);
        }
        keys.last = new_last;
    }
    host.lua_object_destroy_00b67700(&value);
    host.lua_object_destroy_00b67700(&key);
}

// The begin()/end() checks before a whole-vector erase of an image vector
// (first at +4, last at +8): two independent tests.
void clear_image_vector(std::uint32_t address, bool ints, Cc13BigB2Host& host) {
    void* const vector = host.image_global(address);
    if (above(ptr(vector, 4), ptr(vector, 8))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (above(ptr(vector, 4), ptr(vector, 8))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* result[2];
    if (ints) {
        host.int_vector_erase_range_006226f0(vector, result, vector, ptr(vector, 4), vector, ptr(vector, 8));
    } else {
        host.string_vector_erase_range_004954f0(vector, result, vector, ptr(vector, 4), vector, ptr(vector, 8));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 00627960 mission objective lists rebuild (hypothesis)
// No register or stack arguments (Ghidra's __fastcall signature is not used
// by the body), RET. EH frame 00C78FAD with states up to 48h and a flag word
// at [esp+10h] for the conditional LuaObject temporaries of the `a && b`
// tests; not modelled, but each temporary is destroyed at the image's point.
// Callers: 008BDEC0 (Objectives_ClientRefresh) and 00920A20 (cc9 band).
// The Ghidra decompilation of this body drops 240 blocks as unreachable; it
// was reconstructed from the listing alone.
//
// game+1FE4h == 2 returns at once. Otherwise the twelve image vectors are
// emptied: name strings 00E196CC/DC/EC (004954F0), success bits
// 00E196FC/E19710/E19724 (005F6190), player ints 00E19738/48/58 (006226F0),
// ids 00E19768/78/88 (004954F0). Mission.Objectives is tested through the
// globals of the Lua state at game+1A0Ch (not nil Mission, then not nil
// Objectives, temporaries destroyed newest first). When present, for
// "primary", "secondary" and "hidden" in turn: the integer keys of the table
// are collected and sorted (00623170), and each entry whose HideFromRewards
// is nil and Active true is emitted per player: PlayerIndex when set; else,
// with a Party, once for every player slot 0..7 (game+18CCh..) whose +28h is
// the party, lettered by the slot and pushing the slot; else the local
// player's +20h. A single-player entry is lettered by its position in the
// sorted keys and skipped when the index is negative. Emitting appends
// char('@' + letter) + Text, Success, the player value and ID to the four
// vectors. Afterwards the table and the key vector (operator delete) and
// Objectives are released. Finally, with game+1FE4h non-zero, 00773860 on
// game+1EF0h.
//
// Uncertain: the bool pushed to 0049F2D0 / 0049E150 is AL in a dword whose
// upper bytes the image does not define (passed as 0/1). No RNG draws.
void cc13_bigB2_mission_objectives_rebuild_00627960(Cc13BigB2Host& host) {
    if (at<std::int32_t>(host.global_00e188a8(), 0x1FE4) == 2) {
        return;
    }
    clear_image_vector(0x00E196CC, false, host);
    clear_image_vector(0x00E196DC, false, host);
    clear_image_vector(0x00E196EC, false, host);
    host.bit_vector_clear_005f6190(host.image_global(0x00E196FC));
    host.bit_vector_clear_005f6190(host.image_global(0x00E19710));
    host.bit_vector_clear_005f6190(host.image_global(0x00E19724));
    clear_image_vector(0x00E19738, true, host);
    clear_image_vector(0x00E19748, true, host);
    clear_image_vector(0x00E19758, true, host);
    clear_image_vector(0x00E19768, false, host);
    clear_image_vector(0x00E19778, false, host);
    clear_image_vector(0x00E19788, false, host);

    auto machine = [&]() { return addr(host.global_00e188a8(), 0x1A0C); };
    bool present = false;
    {
        LuaObj globals;
        LuaObj mission;
        LuaObj globals2;
        LuaObj mission2;
        LuaObj objectives;
        bool made_second = false;
        host.lua_object_get_field_00b67800(host.lua_globals_00b67980(machine(), &globals), &mission, "Mission");
        if (!host.lua_object_is_nil_00b65fb0(&mission)) {
            made_second = true;
            host.lua_object_get_field_00b67800(
                host.lua_object_get_field_00b67800(host.lua_globals_00b67980(machine(), &globals2), &mission2,
                                                   "Mission"),
                &objectives, "Objectives");
            present = !host.lua_object_is_nil_00b65fb0(&objectives);
        }
        if (made_second) {
            host.lua_object_destroy_00b67700(&objectives);
            host.lua_object_destroy_00b67700(&mission2);
            host.lua_object_destroy_00b67700(&globals2);
        }
        host.lua_object_destroy_00b67700(&mission);
        host.lua_object_destroy_00b67700(&globals);
    }

    if (present) {
        LuaObj objectives;
        {
            LuaObj globals;
            LuaObj mission;
            host.lua_object_get_field_00b67800(
                host.lua_object_get_field_00b67800(host.lua_globals_00b67980(machine(), &globals), &mission,
                                                   "Mission"),
                &objectives, "Objectives");
            host.lua_object_destroy_00b67700(&mission);
            host.lua_object_destroy_00b67700(&globals);
        }
        KeyVector keys;
        keys.first = nullptr;
        keys.last = nullptr;
        keys.end = nullptr;
        LuaObj table;
        static const ObjectiveCategory kCategories[3] = {
            {"primary", 0x00E196CC, 0x00E196FC, 0x00E19738, 0x00E19768, true},
            {"secondary", 0x00E196DC, 0x00E19710, 0x00E19748, 0x00E19778, false},
            {"hidden", 0x00E196EC, 0x00E19724, 0x00E19758, 0x00E19788, false},
        };
        for (const ObjectiveCategory& category : kCategories) {
            rebuild_objective_category(category, &objectives, table, keys, host);
        }
        host.lua_object_destroy_00b67700(&table);
        if (keys.first != nullptr) {
            host.crt_operator_delete_00bf65ac(keys.first);
        }
        keys.first = nullptr;
        keys.last = nullptr;
        keys.end = nullptr;
        host.lua_object_destroy_00b67700(&objectives);
    }

    void* const game = host.global_00e188a8();
    if (at<std::int32_t>(game, 0x1FE4) != 0) {
        host.session_send_game_setup_00773860(addr(game, 0x1EF0));
    }
}

namespace {

// A NativeString temporary built either by the inlined constructor (zero,
// 0041DD40, memcpy length + 1) or by a 0041E870 call, as the image does at
// that point.
void make_string(Str& out, const char* text, bool inline_body, Cc13BigB2Host& host) {
    if (inline_body) {
        construct_cstring_inline(out, text, host);
    } else {
        host.native_string_assign_0041e870(&out, text);
    }
}

// FindChild(parent, name, 1) with the name temporary released afterwards; the
// result is stored to screen+store_offset (when given) before the release,
// as the image does.
void* bind_child(void* screen, void* parent, const char* name, std::ptrdiff_t store_offset, Cc13BigB2Host& host) {
    Str text;
    construct_cstring_inline(text, name, host);
    void* const widget = host.widget_find_child_00aa7e00(parent, &text, 1);
    if (store_offset >= 0) {
        at<void*>(screen, static_cast<std::size_t>(store_offset)) = widget;
    }
    release(text, host);
    return widget;
}

// The scroller wiring of the chat (+158h) and event (+0CCh) panes: attach the
// text box, then the bar with length back - slider (sizes' +4h, float
// subtraction) and the arrows, then the range textbox - clipbox.
void wire_scroller(void* screen, std::size_t scroller, std::size_t textbox, std::size_t back, std::size_t slider,
                   std::size_t up, std::size_t down, std::size_t clipbox, Cc13BigB2Host& host) {
    host.gui_scroller_attach_content_00683790(
        addr(screen, scroller), static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(ptr(screen, textbox))));
    const float back_height = host.widget_get_size_00aa6740(ptr(screen, back))[1];
    const float slider_height = host.widget_get_size_00aa6740(ptr(screen, slider))[1];
    host.gui_scroller_attach_scroll_bar_00683380(addr(screen, scroller), ptr(screen, slider),
                                                 back_height - slider_height, ptr(screen, up), ptr(screen, down));
    const float text_height = host.widget_get_size_00aa6740(ptr(screen, textbox))[1];
    const float clip_height = host.widget_get_size_00aa6740(ptr(screen, clipbox))[1];
    host.gui_scroller_set_scroll_range_006834a0(addr(screen, scroller), text_height - clip_height);
}

// One map-icon list (005DB7EA for +1F4h on the settings group, 005DDC89 for
// +240h on the page +18h): a clone of "map_001_Icon" per map record (434h
// bytes, vector at (*00E198AC)+5Ch+24h), named "map_Icon" with the 1-based
// index through 004260B0/004261A0 and vtable +2Ch, placed at the template's
// resolved position, given the shader "guidefault_noalpha.mshd" and the eight
// radar-shot textures "gui/radarshots/" + base.substr(0, 14h) + suffix, where
// base is the record path (+20h) after the last '/' and before ".scn". The
// template and every clone are then hidden.
//   first_list: the +1F4h shape (inline strings, inline push_back with
//   0043DB60, the two record dereference checks in sequence); otherwise the
//   +240h shape (0041E870 calls, 0043DCA0, nested checks).
void build_map_icon_list(void* screen, bool first_list, void* settings_group, Cc13BigB2Host& host) {
    const bool inl = first_list;
    const std::size_t list = first_list ? 0x1F4 : 0x240;
    auto page = [&]() { return first_list ? settings_group : ptr(screen, 0x18); };
    auto records = [&]() { return addr(ptr(ptr(host.image_global(0x00E198AC), 0), 0x5C), 0x24); };

    void* iterator_container;
    Byte* iterator_record;
    {
        void* const vec = records();
        Byte* const first = static_cast<Byte*>(ptr(vec, 4));
        if (above(first, ptr(vec, 8))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        iterator_container = vec;
        iterator_record = first;
    }
    std::int32_t index = 1;
    static const char* const kSuffixes[8] = {"_C.tga",   "_D.tga",   "_E.tga",   "_ICH.tga",
                                             "_ICL.tga", "_ICM.tga", "_ICS.tga", "_S.tga"};
    for (;;) {
        void* const vec = records();
        void* const last = ptr(vec, 8);
        if (above(ptr(vec, 4), last)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (iterator_container == nullptr || iterator_container != vec) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        Str template_name;
        if (iterator_record == last) {
            // Exit: hide the template (found again) and every clone.
            make_string(template_name, "map_001_Icon", inl, host);
            host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(page(), &template_name, 1), 0);
            release(template_name, host);
            for (std::uint32_t i = 0;; ++i) {
                void** const items = static_cast<void**>(ptr(screen, list + 4));
                if (items == nullptr ||
                    i >= static_cast<std::uint32_t>(static_cast<void**>(ptr(screen, list + 8)) - items)) {
                    break;
                }
                if (ptr(screen, list + 4) == nullptr ||
                    i >= static_cast<std::uint32_t>(static_cast<void**>(ptr(screen, list + 8)) -
                                                    static_cast<void**>(ptr(screen, list + 4)))) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                host.widget_vcall_34_set_visible(static_cast<void**>(ptr(screen, list + 4))[i], 0);
            }
            return;
        }

        // Clone the template under the page and append it.
        make_string(template_name, "map_001_Icon", inl, host);
        void* parent = page();
        void* clone =
            host.widget_clone_subtree_00aab4c0(host.widget_find_child_00aa7e00(parent, &template_name, 1), parent);
        if (first_list) {
            void** const items = static_cast<void**>(ptr(screen, list + 4));
            void** const end = static_cast<void**>(ptr(screen, list + 8));
            const std::uint32_t size = items == nullptr ? 0 : static_cast<std::uint32_t>(end - items);
            if (items != nullptr &&
                size < static_cast<std::uint32_t>(static_cast<void**>(ptr(screen, list + 12)) - items)) {
                *end = clone;
                at<void**>(screen, list + 8) = end + 1;
            } else {
                void* const where = ptr(screen, list + 8);
                if (above(items, where)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                void* result[2];
                host.dword_vector_insert_one_0043db60(addr(screen, list), result, addr(screen, list), where, &clone);
            }
        } else {
            host.pointer_vector_push_back_0043dca0(addr(screen, list), &clone);
        }
        release(template_name, host);

        // back(), checked.
        void** const back_end = static_cast<void**>(ptr(screen, list + 8));
        if (above(ptr(screen, list + 4), back_end)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void** const back_slot = back_end - 1;
        if (above(back_slot, ptr(screen, list + 8)) || above(ptr(screen, list + 4), back_slot)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (!above(ptr(screen, list + 8), back_slot)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* const icon = *back_slot;

        // Name: "map_Icon" with the index.
        {
            Str stem;
            make_string(stem, "map_Icon", inl, host);
            Str number;
            Str joined;
            void* const lhs = host.native_string_from_int_004260b0(&number, index);
            host.widget_vcall_2c(icon, host.native_string_concat_004261a0(lhs, &joined, &stem));
            release(joined, host);
            release(number, host);
            release(stem, host);
        }
        // Position from the template.
        {
            Str name;
            make_string(name, "map_001_Icon", inl, host);
            float resolved[3];
            host.widget_set_resolved_position_00aa8240(
                icon, host.widget_get_resolved_position_00aa6750(
                          host.widget_find_child_00aa7e00(page(), &name, 1), resolved));
            release(name, host);
        }
        // base = record path after the last '/'.
        Str separator;
        make_string(separator, "/", inl, host);
        if (first_list) {
            if (iterator_container == nullptr) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (!above(ptr(iterator_container, 8), iterator_record)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (!above(ptr(iterator_container, 8), iterator_record)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
        } else {
            if (iterator_container == nullptr) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (!above(ptr(iterator_container, 8), iterator_record)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
                if (!above(ptr(iterator_container, 8), iterator_record)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
            }
        }
        void* const path = iterator_record + 0x20;
        Str base;
        const std::int32_t slash = host.native_string_rfind_00467cf0(path, &separator, 0x7FFFFFFFu);
        host.native_string_substring_00469840(path, &base, slash + 1, 0x7FFFFFFF);
        release(separator, host);
        // Cut at ".scn" (strstr; -1 when absent or the buffer is null).
        {
            std::int32_t cut = -1;
            if (base.data != nullptr) {
                const char* const hit = host.crt_strstr_00bf9440(base.data, ".scn");
                if (hit != nullptr) {
                    cut = static_cast<std::int32_t>(hit - base.data);
                }
            }
            Str piece;
            Str* const got = static_cast<Str*>(host.native_string_substring_00469840(&base, &piece, 0, cut));
            if (&base != got) {
                host.native_string_resize_0041dd40(&base, got->length, 1);
                if (got->length != 0) {
                    host.crt_memcpy_00bf7680(base.data, got->data, base.length);
                }
            }
            release(piece, host);
        }
        {
            Str shader;
            make_string(shader, "guidefault_noalpha.mshd", inl, host);
            host.gui_icon_set_shader_name_00ab4630(icon, &shader);
            release(shader, host);
        }
        for (std::int32_t layer = 0; layer < 8; ++layer) {
            Str suffix;
            make_string(suffix, kSuffixes[layer], inl, host);
            Str head;
            Str prefixed;
            Str full;
            void* const cut = host.native_string_substring_00469840(&base, &head, 0, 0x14);
            void* const lhs = host.native_string_prefixed_0043c130(&prefixed, "gui/radarshots/", cut);
            void* const name = host.native_string_concat_004261a0(lhs, &full, &suffix);
            const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
            host.gui_set_state_texture_00ab44b0(icon, layer, name, uv);
            release(full, host);
            release(prefixed, host);
            release(head, host);
            release(suffix, host);
        }
        release(base, host);

        if (!above(ptr(iterator_container, 8), iterator_record)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        iterator_record += 0x434;
        ++index;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 005DB050 BSP_MultiGameLobbyScreen_BindWidgets (vtable 00CF23B0 slot 18)
// __thiscall(ECX = screen), RET. 0C3Ch bytes, 4159 instructions, EH frame
// 00C74967 (states up to 84h) noted, not modelled. Every widget name is a
// NativeString temporary built inline (zero, 0041DD40, memcpy) up to the
// second map list, which builds its strings through 0041E870 calls; each is
// passed to FindChild 00AA7E00(name, 1) and released after the result is
// stored. The listener 00AA6BC0 is the screen+0Ch sub-object.
//
// Page +14h: players_Group -> box_players_Text +210h, players_FrameBox +0A4h
// (byte +78h = 1, listener flag 1); settings_Group (a local) ->
// game_mode_var_Text +20Ch, setting_template_Group +214h (+34h(0)),
// settings_FrameBox +8Ch (1), scroll_up2_Icon +98h, scroll_down2_Icon +9Ch,
// scroll_slider2_FrameBox +90h (flag 0 each), setting_Clipbox +0A0h ->
// textbox_Group (local) attached to the scroller +30h (00683790) ->
// Settings_Listbox +94h; the scroller bar 00683380(slider, back - slider
// heights, up, down) with scroll_back2_Icon's height (stored as a double,
// FSUBR; rounded to float under 24-bit PC). Map list +1F4h (above). Then
// unlock_Group -> unlock_FrameBox +20h (1), lock_Icon +24h; noseart_Group ->
// New_5_Text set to " " (00ABBE50) and "FE.multi_lobby_noseart" ellipsized
// at 0.13333334f (00CF2318, 00ABB000(.., 1)), noseart_FrameBox +28h (1),
// noseart_Icon +2Ch; chat_Group +130h -> ChatModeGlobal/USA/IJN_Icon
// +144h/+148h/+14Ch (0), chat_Clipbox +1B8h -> textbox_Group +134h ->
// MessageTemplate_Text +1B4h, chat_FrameBox +150h (0), scroll_back_Icon
// +140h, scroll_up_Icon +138h (0), scroll_down_Icon +13Ch (0),
// scroll_slider_FrameBox +154h (0), scroller +158h; event_Group +0B0h ->
// event_Clipbox +12Ch -> textbox_Group +0B4h -> MessageTemplate_Text +128h,
// event_FrameBox +0C4h (no listener), scroll_back/up/down/slider +0C0h/
// +0B8h/+0BCh/+0C8h (0 for the last three), scroller +0CCh. Page +18h:
// Settings_Listbox +228h (0; byte +141h = 1, +12Ch = 0.008333334f
// (00CEF0B8), +130h = 0.0027777778f (00CEE600), +134h = 0), setting_name_Text
// +218h and setting_value_Text +21Ch (+34h(0) each), scroll_left/right_Icon
// +220h/+224h (0). Map list +240h. Last, selected_description_Text +22Ch.
//
// Simplified: none of the arithmetic is wider than one float subtraction.
// No 00BD2F10 draws.
void cc13_bigB2_lobby_bind_widgets_005db050(void* screen, Cc13BigB2Host& host) {
    void* const listener = addr(screen, 0x0C);

    void* group = bind_child(screen, ptr(screen, 0x14), "players_Group", -1, host);
    bind_child(screen, group, "box_players_Text", 0x210, host);
    bind_child(screen, group, "players_FrameBox", 0xA4, host);
    at<Byte>(ptr(screen, 0xA4), 0x78) = 1;
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0xA4), listener, 1);

    void* const settings_group = bind_child(screen, ptr(screen, 0x14), "settings_Group", -1, host);
    bind_child(screen, settings_group, "game_mode_var_Text", 0x20C, host);
    bind_child(screen, settings_group, "setting_template_Group", 0x214, host);
    host.widget_vcall_34_set_visible(ptr(screen, 0x214), 0);
    bind_child(screen, settings_group, "settings_FrameBox", 0x8C, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x8C), listener, 1);
    bind_child(screen, settings_group, "scroll_up2_Icon", 0x98, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x98), listener, 0);
    bind_child(screen, settings_group, "scroll_down2_Icon", 0x9C, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x9C), listener, 0);
    bind_child(screen, settings_group, "scroll_slider2_FrameBox", 0x90, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x90), listener, 0);
    bind_child(screen, settings_group, "setting_Clipbox", 0xA0, host);
    void* const textbox = bind_child(screen, ptr(screen, 0xA0), "textbox_Group", -1, host);
    host.gui_scroller_attach_content_00683790(addr(screen, 0x30),
                                              static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(textbox)));
    bind_child(screen, textbox, "Settings_Listbox", 0x94, host);
    {
        Str name;
        construct_cstring_inline(name, "scroll_back2_Icon", host);
        const double back_height =
            host.widget_get_size_00aa6740(host.widget_find_child_00aa7e00(settings_group, &name, 1))[1];
        void* const down = ptr(screen, 0x9C);
        void* const up = ptr(screen, 0x98);
        const float slider_height = host.widget_get_size_00aa6740(ptr(screen, 0x90))[1];
        const float length = static_cast<float>(back_height - static_cast<double>(slider_height));
        host.gui_scroller_attach_scroll_bar_00683380(addr(screen, 0x30), ptr(screen, 0x90), length, up, down);
        release(name, host);
    }

    build_map_icon_list(screen, true, settings_group, host);

    group = bind_child(screen, ptr(screen, 0x14), "unlock_Group", -1, host);
    bind_child(screen, group, "unlock_FrameBox", 0x20, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x20), listener, 1);
    bind_child(screen, group, "lock_Icon", 0x24, host);

    group = bind_child(screen, ptr(screen, 0x14), "noseart_Group", -1, host);
    {
        void* const text = bind_child(screen, group, "New_5_Text", -1, host);
        host.gui_text_set_localised_cstring_00abbe50(text, " ", 1);
        Str label;
        construct_cstring_inline(label, "FE.multi_lobby_noseart", host);
        host.gui_text_set_ellipsized_source_00abb000(text, &label, f32_bits(0x3E088889u), 1);  // 00CF2318
        release(label, host);
    }
    bind_child(screen, group, "noseart_FrameBox", 0x28, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x28), listener, 1);
    bind_child(screen, group, "noseart_Icon", 0x2C, host);

    bind_child(screen, ptr(screen, 0x14), "chat_Group", 0x130, host);
    bind_child(screen, ptr(screen, 0x130), "ChatModeGlobal_Icon", 0x144, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x144), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "ChatModeUSA_Icon", 0x148, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x148), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "ChatModeIJN_Icon", 0x14C, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x14C), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "chat_Clipbox", 0x1B8, host);
    bind_child(screen, ptr(screen, 0x1B8), "textbox_Group", 0x134, host);
    bind_child(screen, ptr(screen, 0x134), "MessageTemplate_Text", 0x1B4, host);
    bind_child(screen, ptr(screen, 0x130), "chat_FrameBox", 0x150, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x150), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "scroll_back_Icon", 0x140, host);
    bind_child(screen, ptr(screen, 0x130), "scroll_up_Icon", 0x138, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x138), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "scroll_down_Icon", 0x13C, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x13C), listener, 0);
    bind_child(screen, ptr(screen, 0x130), "scroll_slider_FrameBox", 0x154, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x154), listener, 0);
    wire_scroller(screen, 0x158, 0x134, 0x140, 0x154, 0x138, 0x13C, 0x1B8, host);

    bind_child(screen, ptr(screen, 0x14), "event_Group", 0xB0, host);
    bind_child(screen, ptr(screen, 0xB0), "event_Clipbox", 0x12C, host);
    bind_child(screen, ptr(screen, 0x12C), "textbox_Group", 0xB4, host);
    bind_child(screen, ptr(screen, 0xB4), "MessageTemplate_Text", 0x128, host);
    bind_child(screen, ptr(screen, 0xB0), "event_FrameBox", 0xC4, host);
    bind_child(screen, ptr(screen, 0xB0), "scroll_back_Icon", 0xC0, host);
    bind_child(screen, ptr(screen, 0xB0), "scroll_up_Icon", 0xB8, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0xB8), listener, 0);
    bind_child(screen, ptr(screen, 0xB0), "scroll_down_Icon", 0xBC, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0xBC), listener, 0);
    bind_child(screen, ptr(screen, 0xB0), "scroll_slider_FrameBox", 0xC8, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0xC8), listener, 0);
    wire_scroller(screen, 0xCC, 0xB4, 0xC0, 0xC8, 0xB8, 0xBC, 0x12C, host);

    bind_child(screen, ptr(screen, 0x18), "Settings_Listbox", 0x228, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x228), listener, 0);
    at<Byte>(ptr(screen, 0x228), 0x141) = 1;
    at<float>(ptr(screen, 0x228), 0x12C) = f32_bits(0x3C088889u);  // 00CEF0B8
    at<float>(ptr(screen, 0x228), 0x130) = f32_bits(0x3B360B61u);  // 00CEE600
    at<float>(ptr(screen, 0x228), 0x134) = 0.0f;
    bind_child(screen, ptr(screen, 0x18), "setting_name_Text", 0x218, host);
    host.widget_vcall_34_set_visible(ptr(screen, 0x218), 0);
    bind_child(screen, ptr(screen, 0x18), "setting_value_Text", 0x21C, host);
    host.widget_vcall_34_set_visible(ptr(screen, 0x21C), 0);
    bind_child(screen, ptr(screen, 0x18), "scroll_left_Icon", 0x220, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x220), listener, 0);
    bind_child(screen, ptr(screen, 0x18), "scroll_right_Icon", 0x224, host);
    host.gui_widget_set_listener_00aa6bc0(ptr(screen, 0x224), listener, 0);

    build_map_icon_list(screen, false, nullptr, host);

    Str name;
    host.native_string_assign_0041e870(&name, "selected_description_Text");
    at<void*>(screen, 0x22C) = host.widget_find_child_00aa7e00(ptr(screen, 0x18), &name, 1);
    release(name, host);
}

}  // namespace bsp
