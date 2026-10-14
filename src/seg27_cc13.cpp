// Packet cc13-seg27 reconstructions. Evidence per function in the comments below and in
// docs/CC13_SEG27.md; conventions in include/bsp/seg27_cc13.hpp.
#include "bsp/seg27_cc13.hpp"

#include <cstring>
#include <iterator>

namespace bsp::seg27_cc13 {
namespace {

Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }
void* next_of(void* node) noexcept { return raw(node).ptr(0); }
void* prev_of(void* node) noexcept { return raw(node).ptr(4); }
void* value_of(void* node) noexcept { return raw(node).ptr(8); }
// The head node of the list whose proxy dword sits at screen+off (head at off+4h).
void* head_at(Raw s, std::size_t off) noexcept { return s.ptr(off + 4); }
// `lea reg,[screen+off]; test reg,reg`: the container-null test of a checked iterator.
bool null_container(Raw s, std::size_t off) noexcept {
    return reinterpret_cast<std::uintptr_t>(s.p) + off == 0;
}

} // namespace

// ---------------------------------------------------------------------------
// Session message 82h
// ---------------------------------------------------------------------------

// 00656280: 0075B430(82h) with ECX = msg, then in this order +20h = a2, word +18h = 0,
// byte +1Ah = 0, +4h = 1, byte +1Ch = 0, vtable 00CF64C4, +24h = a3, word +28h = entity+174h
// (0 with no entity), +2Ch = a5. The first stack argument is not read.
Raw session_message130_construct_00656280(Raw m, std::uint32_t /*unused*/, std::uint32_t a2, std::uint32_t a3,
    void* entity, std::uint32_t a5, SessionMessage130Host& host) {
    host.construct_base_0075b430(m, kSessionMessage130Type);
    m.at<std::uint32_t>(0x20) = a2;
    m.at<std::uint16_t>(0x18) = 0;
    m.at<std::uint8_t>(0x1A) = 0;
    m.at<std::uint32_t>(0x04) = 1;
    m.at<std::uint8_t>(0x1C) = 0;
    m.at<std::uint32_t>(0x00) = kSessionMessage130Vtable;
    m.at<std::uint32_t>(0x24) = a3;
    std::uint16_t id = 0;
    if (entity != nullptr) id = raw(entity).at<std::uint16_t>(0x174);
    m.at<std::uint16_t>(0x28) = id;
    m.at<std::uint32_t>(0x2C) = a5;
    return m;
}

// 00656320: header (0075B480), bool +1Ch, 6 bits +20h, 3 bits +24h, a presence bit for word
// +28h and, when set, its 12 bits (00429120), then 4 bits +2Ch. Every bit write takes
// ECX = the cursor itself (the read 006563A0 uses cursor + 4h).
void session_message130_write_00656320(Raw m, void* cursor, SessionMessage130Host& host) {
    host.write_header_0075b480(m, cursor);
    host.write_bool_004290b0(cursor, m.at<std::uint8_t>(0x1C));
    host.write_unsigned_00429070(cursor, m.at<std::uint32_t>(0x20), 6);
    host.write_unsigned_00429070(cursor, m.at<std::uint32_t>(0x24), 3);
    const bool present = m.at<std::uint16_t>(0x28) != 0;
    host.write_bool_004290b0(cursor, present ? 1 : 0);
    if (present) host.write_word_bits_00429120(cursor, m.at<std::uint16_t>(0x28), 12);
    host.write_unsigned_00429070(cursor, m.at<std::uint32_t>(0x2C), 4);
}

// 00656250: object+0Ch is a CRITICAL_SECTION* followed by a lock depth at cs+18h. With a lock it
// enters, bumps the depth, reads +50h, drops the depth and leaves; without one it only reads.
// Only caller 00675C40 (the support-manager update).
std::uint32_t locked_read_50_00656250(Raw object, LockedReadHost& host) {
    void* const cs = object.ptr(0x0C);
    if (cs != nullptr) {
        host.enter_critical_section(cs);
        raw(cs).at<std::int32_t>(0x18) += 1;
    }
    const std::uint32_t value = object.at<std::uint32_t>(0x50);
    if (cs != nullptr) {
        raw(cs).at<std::int32_t>(0x18) += -1;
        host.leave_critical_section(cs);
    }
    return value;
}

// ---------------------------------------------------------------------------
// Support-manager unit lists
// ---------------------------------------------------------------------------

// 0065A200: +34Ch (size of the second unit list +344h) non-zero -> 0Fh. Otherwise with units in
// list +1ECh (size +1F4h) the cursor iterator +300h/+304h is dereferenced (two checks before
// each read) and its unit asked for kind 9 (-> 2), then kind 45h (-> 15h, else 14h).
std::uint8_t support_manager_selected_icon_kind_0065a200(Raw s, EntityKindHost& host) {
    if (s.at<std::uint32_t>(0x34C) != 0) return 0x0F;
    if (s.at<std::uint32_t>(0x1F4) == 0) return 0;
    auto deref = [&]() -> void* {
        if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
        if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
        return value_of(s.ptr(0x304));
    };
    if (host.entity_is_kind_5c(deref(), 9)) return 2;
    return host.entity_is_kind_5c(deref(), 0x45) ? 0x15 : 0x14;
}

// 0065A4D0: the iterator is passed by value; a null container is reported once, a head node
// before each of the three kind tests (45h, 9, 46h).
std::int32_t support_unit_slot_kind_0065a4d0(CheckedIter it, EntityKindHost& host) {
    if (it.container == nullptr) host.checked_iterator_report_00bf6713();
    void* const head = raw(it.container).ptr(4);
    if (it.node == head) host.checked_iterator_report_00bf6713();
    if (host.entity_is_kind_5c(value_of(it.node), 0x45)) return 0;
    if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
    if (host.entity_is_kind_5c(value_of(it.node), 9)) return 1;
    if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
    return host.entity_is_kind_5c(value_of(it.node), 0x46) ? 2 : -1;
}

// 0065A550 (only caller 00670020). +308h is the cursor's index in list +1ECh, +30Ch its column
// in the four-slot strip. The kinds of the column's unit and the units before it (stepping back
// with wrap-around past the head) fill kinds[0..+30Ch]; the units after it fill the rest up to
// column 3, wrapping to the first unit only while the list holds more than four. Then the four
// icons +8Ch..+98h are hidden, and each filled column's icon is shown with frame = kind (as a
// word, so -1 is FFFFh), 0, 1.0 (vtable +88h) and its partner +9Ch.. shown unless the kind is 3.
void support_manager_update_slot_strip_0065a550(Raw s, SupportStripHost& host) {
    void* const list = s.p + 0x1EC;
    std::int32_t kinds[4] = {};
    void* node = next_of(head_at(s, 0x1EC));
    std::int32_t count = 0;
    if (s.at<std::int32_t>(0x308) > 0) {
        do {
            if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
            node = next_of(node);
            ++count;
        } while (count < s.at<std::int32_t>(0x308));
    }
    std::int32_t column = s.at<std::int32_t>(0x30C);
    kinds[column] = support_unit_slot_kind_0065a4d0(CheckedIter{list, node}, host);
    std::int32_t back = column - 1;
    count = 1;
    if (back >= 0) {
        count = back + 2;
        do {
            void* const begin = next_of(raw(list).ptr(4));
            if (null_container(s, 0x1EC)) host.checked_iterator_report_00bf6713();
            if (node == begin) node = raw(list).ptr(4);
            if (null_container(s, 0x1EC)) host.checked_iterator_report_00bf6713();
            node = prev_of(node);
            if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
            kinds[back] = support_unit_slot_kind_0065a4d0(CheckedIter{list, node}, host);
            --back;
        } while (back >= 0);
    }
    node = next_of(raw(list).ptr(4));
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x308); ++i) {
        if (null_container(s, 0x1EC)) host.checked_iterator_report_00bf6713();
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    for (std::int32_t col = s.at<std::int32_t>(0x30C) + 1; col <= 3; ++col) {
        if (null_container(s, 0x1EC)) host.checked_iterator_report_00bf6713();
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        void* const head = raw(list).ptr(4);
        node = next_of(node);
        if (node == head) {
            if (s.at<std::uint32_t>(0x1F4) <= 4) break;
            node = next_of(raw(list).ptr(4));
        }
        const std::int32_t kind = support_unit_slot_kind_0065a4d0(CheckedIter{list, node}, host);
        ++count;
        kinds[col] = kind;
    }
    for (std::size_t off = 0x8C; off <= 0x98; off += 4) host.widget_set_visible_34(s.ptr(off), false);
    for (std::int32_t i = 0; i < count; ++i) {
        void* const icon = s.ptr(0x8C + 4 * i);
        host.widget_set_visible_34(icon, true);
        host.widget_set_frame_88(icon, static_cast<std::uint16_t>(kinds[i]), 0, 1.0f);
        host.widget_set_visible_34(s.ptr(0x9C + 4 * i), kinds[i] != 3);
    }
}

namespace {
// 0065A740's strip over the list at `list` (size at list+8h): the same walk as 0065A550 with the
// index +310h, the column +314h, the frame from 00653200 and the wrap threshold `size <= 4`.
// The backward step tests the head twice (0065A83D and 0065A847).
void payload_strip(Raw s, std::size_t list_off, PayloadStripHost& host) {
    void* const list = s.p + list_off;
    std::int32_t frames[4] = {};
    void* node = next_of(head_at(s, list_off));
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x310); ++i) {
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
    std::int32_t column = s.at<std::int32_t>(0x314);
    frames[column] = host.payload_icon_frame_00653200(s, value_of(node));
    std::int32_t back = column - 1;
    std::int32_t count = 1;
    if (back >= 0) {
        count = back + 2;
        do {
            void* const begin = next_of(raw(list).ptr(4));
            if (null_container(s, list_off)) host.checked_iterator_report_00bf6713();
            if (node == begin) node = raw(list).ptr(4);
            if (null_container(s, list_off)) host.checked_iterator_report_00bf6713();
            node = prev_of(node);
            if (node == raw(list).ptr(4)) {
                host.checked_iterator_report_00bf6713();
                if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
            }
            frames[back] = host.payload_icon_frame_00653200(s, value_of(node));
            --back;
        } while (back >= 0);
    }
    node = next_of(raw(list).ptr(4));
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x310); ++i) {
        if (null_container(s, list_off)) host.checked_iterator_report_00bf6713();
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    for (std::int32_t col = s.at<std::int32_t>(0x314) + 1; col <= 3; ++col) {
        if (null_container(s, list_off)) host.checked_iterator_report_00bf6713();
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        void* const head = raw(list).ptr(4);
        node = next_of(node);
        if (node == head) {
            if (raw(list).at<std::uint32_t>(8) <= 4) break;
            node = next_of(raw(list).ptr(4));
        }
        if (null_container(s, list_off)) host.checked_iterator_report_00bf6713();
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        const std::int32_t frame = host.payload_icon_frame_00653200(s, value_of(node));
        ++count;
        frames[col] = frame;
    }
    for (std::int32_t i = 0; i < count; ++i) {
        void* const icon = s.ptr(0x8C + 4 * i);
        host.widget_set_visible_34(icon, true);
        host.widget_set_frame_88(icon, static_cast<std::uint16_t>(frames[i]), 0, 1.0f);
    }
}
} // namespace

// 0065A740 (callers 0066D020 and the update 00675C40). Hides the three icon rows +8Ch, +9Ch and
// +CCh (four each, interleaved per column), shows the highlight +CCh[+314h], then lays the
// payload strip over list +2E4h when it holds entries, else over list +37Ch when that does.
void support_manager_update_payload_strip_0065a740(Raw s, PayloadStripHost& host) {
    for (std::size_t i = 0; i < 4; ++i) {
        host.widget_set_visible_34(s.ptr(0x8C + 4 * i), false);
        host.widget_set_visible_34(s.ptr(0x9C + 4 * i), false);
        host.widget_set_visible_34(s.ptr(0xCC + 4 * i), false);
    }
    host.widget_set_visible_34(s.ptr(0xCC + 4 * s.at<std::int32_t>(0x314)), true);
    if (s.at<std::uint32_t>(0x2EC) != 0) {
        payload_strip(s, 0x2E4, host);
    } else if (s.at<std::uint32_t>(0x384) != 0) {
        payload_strip(s, 0x37C, host);
    }
}

namespace {
// 0065AB40 / 0065AE90 share this body over the list at `list` (index +310h).
void* catalog_entry_at(Raw s, std::size_t list, CheckedIteratorHost& host) {
    void* node = next_of(head_at(s, list));
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x310); ++i) {
        if (null_container(s, list)) host.checked_iterator_report_00bf6713();
        if (node == head_at(s, list)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    if (null_container(s, list)) host.checked_iterator_report_00bf6713();
    if (node == head_at(s, list)) host.checked_iterator_report_00bf6713();
    return value_of(node);
}
} // namespace

// 0065AB40: node +8h of list +37Ch at index +310h (0066B6C0 keeps it in +3A4h).
void* support_manager_catalog_entry_0065ab40(Raw s, CheckedIteratorHost& host) {
    return catalog_entry_at(s, 0x37C, host);
}

// 0065AE90: the same over list +2E4h (0066B6C0 keeps it in +2A0h).
void* support_manager_catalog_entry_2e4_0065ae90(Raw s, CheckedIteratorHost& host) {
    return catalog_entry_at(s, 0x2E4, host);
}

// 0065BBA0: `lea eax,[esp+4]; push eax; add ecx,1F8h; call 0065B1F0; ret 4`. Caller: the Lua
// binding SupportManagerEnable (src/options_seg56_cc13.cpp calls it panel_list_remove_0065bba0).
void support_manager_list_remove_0065bba0(Raw s, void* entity, SupportListRemoveHost& host) {
    host.list_remove_0065b1f0(s.p + 0x1F8, entity);
}

// 0065C9A0: a linear walk of the map +50h (head +54h, size +58h) for the node whose key +0Ch is
// the entity; its value +10h goes to +334h. An empty map or no match stores entity+134h. The
// iterator is checked against end (container null or foreign) at the top of every pass.
void support_manager_select_value_0065c9a0(Raw s, void* entity, SupportMapHost& host) {
    if (s.at<std::uint32_t>(0x58) != 0) {
        void* const map = s.p + 0x50;
        CheckedIter it{map, next_of(s.ptr(0x54))};
        for (;;) {
            void* const end = s.ptr(0x54);
            if (it.container == nullptr || it.container != map) host.checked_iterator_report_00bf6713();
            if (it.node == end) break;
            if (it.container == nullptr) host.checked_iterator_report_00bf6713();
            if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
            if (raw(it.node).ptr(0x0C) == entity) {
                if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
                s.at<std::uint32_t>(0x334) = raw(it.node).at<std::uint32_t>(0x10);
                return;
            }
            host.tree_iterator_increment_00654d50(it);
        }
    }
    s.at<std::uint32_t>(0x334) = raw(entity).at<std::uint32_t>(0x134);
}

// 0065EE90 (caller 0066D020). Nothing without +5Ch. With the map +50h empty the first catalog
// unit (list +37Ch) becomes +3A4h. Otherwise +4Ch is looked up in list +37Ch: found at index i,
// +310h = i and +314h = min(i, 3); not found, the first unit is taken. 0065C9A0 then runs on
// +3A4h on every path.
void support_manager_sync_catalog_cursor_0065ee90(Raw s, SupportMapHost& host) {
    if (s.at<std::uint32_t>(0x5C) == 0) return;
    if (s.at<std::uint32_t>(0x58) != 0) {
        s.ptr(0x3A4) = s.ptr(0x4C);
        void* node = next_of(s.ptr(0x380));
        std::int32_t index = 0;
        for (;;) {
            if (node == s.ptr(0x380)) break;
            if (node == s.ptr(0x380)) host.checked_iterator_report_00bf6713();
            if (value_of(node) == s.ptr(0x3A4)) {
                s.at<std::int32_t>(0x310) = index;
                s.at<std::int32_t>(0x314) = index < 3 ? index : 3;
                support_manager_select_value_0065c9a0(s, s.ptr(0x3A4), host);
                return;
            }
            ++index;
            if (node == s.ptr(0x380)) host.checked_iterator_report_00bf6713();
            node = next_of(node);
        }
    }
    void* const first = next_of(s.ptr(0x380));
    if (first == s.ptr(0x380)) host.checked_iterator_report_00bf6713();
    s.ptr(0x3A4) = value_of(first);
    support_manager_select_value_0065c9a0(s, s.ptr(0x3A4), host);
}

// 0065EFA0 (caller 0066D020). The cursor unit's id (+174h of the value at +304h) is looked up in
// the map +414h (0065CD40). Missing: +2A0h = the first entry of list +2E4h. Present: the map is
// walked from its first node to the node with that key (0057F5F0 steps), whose count +10h sets
// +32Ch = min(count, 3) and +330h = count - 3 (or 0), and +2A0h = entry `count` of list +2E4h.
void support_manager_sync_weapon_cursor_0065efa0(Raw s, SupportMapHost& host) {
    auto deref_cursor = [&]() -> void* {
        if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
        if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
        return value_of(s.ptr(0x304));
    };
    const std::uint32_t key = raw(deref_cursor()).at<std::uint16_t>(0x174);
    void* const map = s.p + 0x414;
    void* const end_before = s.ptr(0x418);
    const CheckedIter found = host.map_find_u16_0065cd40(map, key);
    if (found.container == nullptr || found.container != map) host.checked_iterator_report_00bf6713();
    if (found.node == end_before) {
        void* const first = next_of(s.ptr(0x2E8));
        if (first == s.ptr(0x2E8)) host.checked_iterator_report_00bf6713();
        s.ptr(0x2A0) = value_of(first);
        return;
    }
    CheckedIter it{map, next_of(raw(map).ptr(4))};
    for (;;) {
        void* const end = raw(map).ptr(4);
        if (it.container == nullptr || it.container != map) host.checked_iterator_report_00bf6713();
        if (it.node == end) return;
        if (it.container == nullptr) host.checked_iterator_report_00bf6713();
        if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
        const std::uint16_t id = raw(deref_cursor()).at<std::uint16_t>(0x174);
        if (raw(it.node).at<std::uint16_t>(0x0C) == id) break;
        host.tree_iterator_increment_0057f5f0(it);
    }
    Raw const node = raw(it.node);
    void* const head = raw(it.container).ptr(4);
    if (it.node == head) host.checked_iterator_report_00bf6713();
    if (node.at<std::int32_t>(0x10) < 3) {
        s.at<std::int32_t>(0x330) = 0;
        if (it.node == head) host.checked_iterator_report_00bf6713();
        s.at<std::int32_t>(0x32C) = node.at<std::int32_t>(0x10);
    } else {
        if (it.node == head) host.checked_iterator_report_00bf6713();
        s.at<std::int32_t>(0x330) = node.at<std::int32_t>(0x10) - 3;
        s.at<std::int32_t>(0x32C) = 3;
    }
    void* entry = next_of(s.ptr(0x2E8));
    for (std::int32_t i = 0;; ++i) {
        if (it.node == head) host.checked_iterator_report_00bf6713();
        if (i >= node.at<std::int32_t>(0x10)) break;
        if (entry == s.ptr(0x2E8)) host.checked_iterator_report_00bf6713();
        entry = next_of(entry);
    }
    if (entry == s.ptr(0x2E8)) host.checked_iterator_report_00bf6713();
    s.ptr(0x2A0) = value_of(entry);
}

// 00668570 (caller 00675C40). With units in list +1ECh and a cursor unit whose byte +5Dh is
// clear: that unit's health (00923BE0, returned unrounded). Else with entries in list +344h:
// the cursor value +350h/+354h is copied (unit pointer and reference holder, 005AAAF0) and
// released (005A0B80); a non-null unit is copied again and its health is returned rounded to a
// float (00668720 `fstp float ptr`) before the release. Else the controlled unit *00E188D8's
// health when it exists with bytes +5Dh and +5Eh clear (unrounded), else 1.0.
double support_manager_selection_health_00668570(Raw s, SupportHealthHost& host) {
    auto check = [&](std::size_t cont, std::size_t node) {
        if (s.ptr(cont) == nullptr) host.checked_iterator_report_00bf6713();
        if (s.ptr(node) == raw(s.ptr(cont)).ptr(4)) host.checked_iterator_report_00bf6713();
    };
    if (s.at<std::uint32_t>(0x1F4) != 0) {
        check(0x300, 0x304);
        if (value_of(s.ptr(0x304)) != nullptr) {
            check(0x300, 0x304);
            if (raw(value_of(s.ptr(0x304))).at<std::uint8_t>(0x5D) == 0) {
                check(0x300, 0x304);
                return host.unit_health_00923be0(value_of(s.ptr(0x304)));
            }
        }
    }
    bool use_entry = false;
    if (s.at<std::uint32_t>(0x34C) != 0) {
        check(0x350, 0x354);
        Raw const value{static_cast<std::uint8_t*>(s.ptr(0x354)) + 8};
        alignas(4) std::uint8_t holder[0x10] = {};
        void* const unit = value.ptr(0);
        host.unit_ref_copy_005aaaf0(holder, value.p + 4);
        host.unit_ref_release_005a0b80(holder);
        use_entry = unit != nullptr;
    }
    if (use_entry) {
        check(0x350, 0x354);
        Raw const value{static_cast<std::uint8_t*>(s.ptr(0x354)) + 8};
        alignas(4) std::uint8_t holder[0x10] = {};
        void* const unit = value.ptr(0);
        host.unit_ref_copy_005aaaf0(holder, value.p + 4);
        const float health = static_cast<float>(host.unit_health_00923be0(unit));
        host.unit_ref_release_005a0b80(holder);
        return health;
    }
    void* const controlled = host.controlled_unit();
    if (controlled == nullptr || raw(controlled).at<std::uint8_t>(0x5D) != 0 ||
        raw(controlled).at<std::uint8_t>(0x5E) != 0) {
        return 1.0;
    }
    return host.unit_health_00923be0(controlled);
}

// 00669170 (callers: the screen's Enter 00674B80, 0066D020, 00675C40). Map +408h is keyed by
// unit id (word +0Ch) and holds a map<entry*, int> at node+10h. The node for the cursor unit's
// id (+174h) is found by a linear walk; the native copies its inner map into a local (00665340)
// and destroys the copy unused (00664450, free) after walking the original for the key +2A0h,
// whose value +10h becomes +334h. Without a match +334h = (+2A0h)+134h. Once a match is found
// the outer walk stops at its next pass.
void support_manager_select_weapon_value_00669170(Raw s, SupportWeaponMapHost& host) {
    if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
    if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
    const std::uint16_t id = raw(value_of(s.ptr(0x304))).at<std::uint16_t>(0x174);
    void* const outer_map = s.p + 0x408;
    CheckedIter outer{outer_map, next_of(s.ptr(0x40C))};
    bool done = false;
    for (;;) {
        void* const outer_end = s.ptr(0x40C);
        if (outer.container == nullptr || outer.container != outer_map) host.checked_iterator_report_00bf6713();
        if (outer.node == outer_end) break;
        if (done) return;
        if (outer.container == nullptr) host.checked_iterator_report_00bf6713();
        if (outer.node == raw(outer.container).ptr(4)) host.checked_iterator_report_00bf6713();
        if (raw(outer.node).at<std::uint16_t>(0x0C) == id) {
            if (outer.node == raw(outer.container).ptr(4)) host.checked_iterator_report_00bf6713();
            void* const inner_map = static_cast<std::uint8_t*>(outer.node) + 0x10;
            alignas(4) std::uint8_t copy[0x0C] = {};
            host.map_copy_construct_00665340(copy, inner_map);
            if (outer.node == raw(outer.container).ptr(4)) host.checked_iterator_report_00bf6713();
            CheckedIter inner{inner_map, next_of(raw(inner_map).ptr(4))};
            for (;;) {
                if (outer.node == raw(outer.container).ptr(4)) host.checked_iterator_report_00bf6713();
                void* const inner_end = raw(inner_map).ptr(4);
                if (inner.container == nullptr || inner.container != inner_map) host.checked_iterator_report_00bf6713();
                if (inner.node == inner_end || done) break;
                if (inner.container == nullptr) host.checked_iterator_report_00bf6713();
                if (inner.node == raw(inner.container).ptr(4)) host.checked_iterator_report_00bf6713();
                if (raw(inner.node).ptr(0x0C) == s.ptr(0x2A0)) {
                    if (inner.node == raw(inner.container).ptr(4)) host.checked_iterator_report_00bf6713();
                    s.at<std::uint32_t>(0x334) = raw(inner.node).at<std::uint32_t>(0x10);
                    done = true;
                }
                host.tree_iterator_increment_00654eb0(inner);
            }
            Raw const local{copy};
            host.map_erase_range_00664450(copy, next_of(local.ptr(4)), local.ptr(4));
            host.free_00bf65ac(local.ptr(4));
            local.ptr(4) = nullptr;
            local.at<std::uint32_t>(8) = 0;
        }
        host.tree_iterator_increment_006582d0(outer);
    }
    if (!done) s.at<std::uint32_t>(0x334) = raw(s.ptr(0x2A0)).at<std::uint32_t>(0x134);
}

// ---------------------------------------------------------------------------
// Support sites
// ---------------------------------------------------------------------------

namespace {

enum class PairList { A /* 0066B260, 00663CC0 */, B /* 0066AFA0, 00663190 */ };

// push_back(pair()) as the native expands it: a temporary pair whose unit list gets a fresh
// head (0059FD60) and size 0, _Buynode from (head, head->prev, &temp), _Incsize(1), the two link
// stores; then the temporary's list is destroyed, by 005A0B80 (`tidy`) or inline (head reset,
// nodes and head freed). The site and widget dwords of the temporary are not initialised by
// the native either; the caller overwrites them in the new node.
void push_back_empty_pair(void* list, PairList kind, bool tidy_call, SupportSiteListHost& host) {
    alignas(4) std::uint8_t temp[0x14] = {};
    Raw t{temp};
    t.ptr(0x08) = host.list_buy_head_0059fd60(temp + 0x04);
    t.at<std::uint32_t>(0x0C) = 0;
    void* const head = raw(list).ptr(4);
    void* node = nullptr;
    if (kind == PairList::A) {
        node = host.pair_list_buy_node_0066b260(list, head, prev_of(head), temp);
        host.pair_list_inc_size_00663cc0(list, 1);
    } else {
        node = host.pair_list_buy_node_0066afa0(list, head, prev_of(head), temp);
        host.pair_list_inc_size_00663190(list, 1);
    }
    raw(head).ptr(4) = node;
    raw(prev_of(node)).ptr(0) = node;
    if (tidy_call) {
        host.unit_list_tidy_005a0b80(temp + 0x04);
        return;
    }
    void* const temp_head = t.ptr(0x08);
    void* p = raw(temp_head).ptr(0);
    raw(temp_head).ptr(0) = temp_head;
    raw(temp_head).ptr(4) = temp_head;
    t.at<std::uint32_t>(0x0C) = 0;
    while (p != temp_head) {
        void* const next = raw(p).ptr(0);
        host.free_00bf65ac(p);
        p = next;
    }
    host.free_00bf65ac(temp_head);
}

// back() with the two head tests the native makes before writing into it.
void* back_checked(void* list, SupportSiteListHost& host) {
    void* const back = prev_of(raw(list).ptr(4));
    if (back == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
    if (back == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
    return back;
}

// The site pair: site +8h, then the unit list assigned from the site's own list +784h unless it
// is that very list.
void fill_site_pair(void* back, void* site, SupportSiteListHost& host) {
    raw(back).ptr(8) = site;
    void* const src = static_cast<std::uint8_t*>(site) + 0x784;
    void* const dst = static_cast<std::uint8_t*>(back) + 0x0C;
    if (dst != src) {
        void* const src_head = raw(src).ptr(4);
        host.unit_list_assign_00665d20(dst, CheckedIter{src, next_of(src_head)}, CheckedIter{src, src_head}, site);
    }
}

// The lone-unit pair: site 0, the unit pushed onto the pair's own list (005A0650, 005A5880).
void fill_unit_pair(void* back, void* unit, SupportSiteListHost& host) {
    raw(back).ptr(8) = nullptr;
    void* const inner = static_cast<std::uint8_t*>(back) + 0x0C;
    void* const head = raw(inner).ptr(4);
    void* const node = host.unit_list_buy_node_005a0650(inner, head, prev_of(head), &unit);
    host.unit_list_inc_size_005a5880(inner, 1);
    raw(head).ptr(4) = node;
    raw(prev_of(node)).ptr(0) = node;
}

// The map icon of a pair: a clone of the screen's SpawnThing_Icon template
// ((*00E198C4)+BCh -> +1D8h, 00AAB4C0(0)) placed at its own resolved x, y and the running depth,
// which then drops by 1.0 (00D7A210, double) for the next icon.
void add_pair_icon(void* back, float* depth, SupportSiteListHost& host) {
    void* const templ = raw(host.ingame_interface().ptr(0xBC)).ptr(0x1D8);
    void* const clone = host.widget_clone_subtree_00aab4c0(templ, 0);
    float pos[3];
    host.widget_resolved_position_00aa6750(clone, pos);
    const float placed[3] = {pos[0], pos[1], *depth};
    host.widget_set_resolved_position_00aa8240(clone, placed);
    raw(back).ptr(0x18) = clone;
    *depth = static_cast<float>(static_cast<double>(*depth) - 1.0);
}

void* first_site_node(SupportSiteListHost& host) { return raw(host.game().ptr(0x19CC)).ptr(0x16C); }
void* first_unit_node(SupportSiteListHost& host) { return raw(host.game().ptr(0x19CC)).ptr(0x3B8); }

} // namespace

// 0066DD00 (callers 0066E510): clear (006654C0), then one pair per site with +78Ch set
// (temporary destroyed through 005A0B80) and one per unit with +344h clear (inline destroy).
void support_sites_build_0066dd00(void* list, SupportSiteListHost& host) {
    host.pair_list_clear_006654c0(list);
    for (void* n = first_site_node(host); n != nullptr; n = raw(n).ptr(4)) {
        void* const site = raw(n).ptr(8);
        if (raw(site).at<std::uint32_t>(0x78C) == 0) continue;
        push_back_empty_pair(list, PairList::A, true, host);
        fill_site_pair(back_checked(list, host), site, host);
    }
    for (void* n = first_unit_node(host); n != nullptr; n = raw(n).ptr(4)) {
        void* const unit = raw(n).ptr(8);
        if (raw(unit).at<std::uint32_t>(0x344) != 0) continue;
        push_back_empty_pair(list, PairList::A, false, host);
        fill_unit_pair(back_checked(list, host), unit, host);
    }
}

// 0066DF30 (callers: the register 0066EA60 and 0066E3C0). Every old pair's icon +18h is deleted
// (vtable +4h(1)), the list cleared (00665210), the depth reset to -12.0 (00CE781C), and the
// pairs rebuilt as in 0066DD00 (both temporaries destroyed inline) with one icon each.
void support_sites_build_with_icons_0066df30(void* list, float* depth, SupportSiteListHost& host) {
    for (void* node = next_of(raw(list).ptr(4));;) {
        if (node == raw(list).ptr(4)) break;
        void* const icon = raw(node).ptr(0x18);
        if (icon != nullptr) host.object_delete_04(icon, 1);
        if (node == raw(list).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    host.pair_list_clear_00665210(list);
    *depth = -12.0f;
    for (void* n = first_site_node(host); n != nullptr; n = raw(n).ptr(4)) {
        void* const site = raw(n).ptr(8);
        if (raw(site).at<std::uint32_t>(0x78C) == 0) continue;
        push_back_empty_pair(list, PairList::B, false, host);
        void* const back = back_checked(list, host);
        fill_site_pair(back, site, host);
        add_pair_icon(back, depth, host);
    }
    for (void* n = first_unit_node(host); n != nullptr; n = raw(n).ptr(4)) {
        void* const unit = raw(n).ptr(8);
        if (raw(unit).at<std::uint32_t>(0x344) != 0) continue;
        push_back_empty_pair(list, PairList::B, false, host);
        void* const back = back_checked(list, host);
        fill_unit_pair(back, unit, host);
        add_pair_icon(back, depth, host);
    }
}

namespace {
// The filter both 0066E2B0 and 0066E3C0 run: a site pair passes when the site's side +54h is
// `side`; a lone-unit pair when its unit's side is `side` and `cls` is -1 or the unit can supply
// it (0084D050). Passing pairs are copied onto `out`. 0066E3C0 also tests its iterator's
// container at the top of each pass (`checked_container`).
void filter_pairs(std::int32_t side, void* pairs, void* out, std::int32_t cls, PairList kind,
    bool checked_container, SupportSiteListHost& host) {
    auto push = [&](void* value) {
        void* const head = raw(out).ptr(4);
        void* node = nullptr;
        if (kind == PairList::A) {
            node = host.pair_list_buy_node_0066b260(out, head, prev_of(head), value);
            host.pair_list_inc_size_00663cc0(out, 1);
        } else {
            node = host.pair_list_buy_node_0066afa0(out, head, prev_of(head), value);
            host.pair_list_inc_size_00663190(out, 1);
        }
        raw(head).ptr(4) = node;
        raw(prev_of(node)).ptr(0) = node;
    };
    void* node = next_of(raw(pairs).ptr(4));
    for (;;) {
        void* const end = raw(pairs).ptr(4);
        if (checked_container && pairs == nullptr) host.checked_iterator_report_00bf6713();
        if (node == end) break;
        if (checked_container && pairs == nullptr) host.checked_iterator_report_00bf6713();
        if (checked_container && node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
        void* const value = static_cast<std::uint8_t*>(node) + 8;
        if (raw(node).ptr(8) != nullptr) {
            if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
            if (raw(raw(node).ptr(8)).at<std::int32_t>(0x54) == side) {
                if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
                push(value);
            }
        } else {
            if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
            void* const inner_head = raw(node).ptr(0x10);
            void* const first = next_of(inner_head);
            if (first == inner_head) host.checked_iterator_report_00bf6713();
            void* const unit = value_of(first);
            if (raw(unit).at<std::int32_t>(0x54) == side &&
                (cls == -1 || host.site_can_supply_class_0084d050(unit, cls))) {
                if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
                push(value);
            }
        }
        if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
}
} // namespace

// 0066E2B0 (caller 0066E510): clears `out` (006654C0) and filters.
void support_sites_filter_0066e2b0(std::int32_t side, void* pairs, void* out, std::int32_t cls,
    SupportSiteListHost& host) {
    host.pair_list_clear_006654c0(out);
    filter_pairs(side, pairs, out, cls, PairList::A, false, host);
}

// 0066E3C0 (caller 0066FB00): rebuilds `pairs` with icons (0066DF30, EDX = depth), clears `out`
// (00665210) and filters with the player index as the class argument.
void support_sites_rebuild_and_filter_0066e3c0(std::int32_t side, void* pairs, void* out, std::int32_t index,
    float* depth, SupportSiteListHost& host) {
    support_sites_build_with_icons_0066df30(pairs, depth, host);
    host.pair_list_clear_00665210(out);
    filter_pairs(side, pairs, out, index, PairList::B, true, host);
}

// 0066E510: a local pair list (head from 006599C0), built by 0066DD00, filtered into `out`
// (0066E2B0), then cleared (006654C0) and its head freed.
void support_sites_collect_0066e510(std::int32_t side, void* out, std::int32_t cls, SupportSiteListHost& host) {
    alignas(4) std::uint8_t local[0x0C] = {};
    Raw l{local};
    l.ptr(4) = host.list_buy_head_006599c0(local);
    l.at<std::uint32_t>(8) = 0;
    support_sites_build_0066dd00(local, host);
    support_sites_filter_0066e2b0(side, local, out, cls, host);
    host.pair_list_clear_006654c0(local);
    host.free_00bf65ac(l.ptr(4));
}

// 0066E590 (callers BSP_AiPlanner_TrySpawnGroup, BSP_Ai_CollectSpawnPointClassRecords,
// BSP_Session_SpawnPlayerVehicles): 0066E510, then every pair whose unit list is empty (size
// +14h) is unlinked, its list tidied (005A0B80), the node freed and the size dropped.
void support_sites_collect_nonempty_0066e590(std::int32_t side, void* out, std::int32_t cls,
    SupportSiteListHost& host) {
    support_sites_collect_0066e510(side, out, cls, host);
    void* node = next_of(raw(out).ptr(4));
    while (node != raw(out).ptr(4)) {
        if (raw(node).at<std::uint32_t>(0x14) != 0) {
            if (node == raw(out).ptr(4)) host.checked_iterator_report_00bf6713();
            node = next_of(node);
            continue;
        }
        if (node == raw(out).ptr(4)) host.checked_iterator_report_00bf6713();
        void* const victim = node;
        node = next_of(node);
        if (out == nullptr) host.checked_iterator_report_00bf6713();
        if (victim == raw(out).ptr(4)) {
            host.checked_iterator_report_00bf6713();
            if (victim == raw(out).ptr(4)) continue;
        }
        raw(prev_of(victim)).ptr(0) = next_of(victim);
        raw(next_of(victim)).ptr(4) = prev_of(victim);
        host.unit_list_tidy_005a0b80(static_cast<std::uint8_t*>(victim) + 0x0C);
        host.free_00bf65ac(victim);
        raw(out).at<std::uint32_t>(8) -= 1;
    }
}

// 0066FB00 (callers: Enter 00674B80, HasAvailableEntries, 00670020, 00675C40). Rebuilds the site
// pairs for the local player (index game+18ECh, side (game+18CCh[index])+28h) into +344h through
// 0066E3C0, then moves each pair's icon +18h to the map position of the first unit of its list:
// the unit's world matrix is refreshed first when its byte +C8h is clear (00414DB0 on a parent
// +3Ch, 00413920 parent product or the local +74h, 004134F0 into +CCh, +C8h = 1, +10Ch = 0),
// then 00658580(world x +FCh, y +100h, z +104h) and 00AA7D00(x, y).
void support_manager_refresh_site_icons_0066fb00(Raw s, SupportMapIconsHost& host) {
    Raw const game = host.game();
    const std::int32_t index = game.at<std::int32_t>(0x18EC);
    const std::int32_t side = raw(game.ptr(0x18CC + 4 * index)).at<std::int32_t>(0x28);
    support_sites_rebuild_and_filter_0066e3c0(side, s.p + 0x338, s.p + 0x344, index,
        &s.at<float>(0x2D0), host);
    for (void* node = next_of(s.ptr(0x348));;) {
        if (node == s.ptr(0x348)) break;
        if (node == s.ptr(0x348)) host.checked_iterator_report_00bf6713();
        void* const inner_head = raw(node).ptr(0x10);
        void* const first = next_of(inner_head);
        if (first == inner_head) host.checked_iterator_report_00bf6713();
        Raw const unit = raw(value_of(first));
        if (unit.at<std::uint8_t>(0xC8) == 0) {
            if (unit.ptr(0x3C) != nullptr) host.entity_refresh_world_00414db0(unit.ptr(0x3C));
            alignas(4) std::uint8_t product[0x40];
            const void* src = unit.p + 0x74;
            if (unit.ptr(0x3C) != nullptr) {
                src = host.matrix_multiply_00413920(unit.p + 0x74, product,
                    static_cast<std::uint8_t*>(unit.ptr(0x3C)) + 0xCC);
            }
            host.matrix_copy_004134f0(unit.p + 0xCC, src);
            unit.at<std::uint8_t>(0xC8) = 1;
            unit.at<std::uint8_t>(0x10C) = 0;
        }
        float map[2];
        host.world_to_map_00658580(s, map, unit.at<float>(0xFC), unit.at<float>(0x100), unit.at<float>(0x104));
        if (node == s.ptr(0x348)) host.checked_iterator_report_00bf6713();
        host.widget_set_local_xy_00aa7d00(raw(node).ptr(0x18), map[0], map[1]);
        if (node == s.ptr(0x348)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
}

// ---------------------------------------------------------------------------
// 00660190
// ---------------------------------------------------------------------------

// The four column highlights +CCh.. are hidden (and +CCh[+30Ch] once more), as is the slot
// highlight +DCh[+324h]. With the second list +344h in use +320h = 0. Otherwise, with units, the
// cursor unit decides: of kind 9 or 45h its slot table (unit+72Ch for kind 45h, unit+1188h for
// the rest; 0 for a null unit, which the native then reads through) is walked under its lock
// (cs at +0Ch, count +50h re-read every pass, 58h-byte records from +4Ch) into the caller's
// record (0065CA50) until one is out (00658530) or in state +2Ch 2 or 5; that index selects the
// row (+324h = min(i, 3), +320h = the overflow) and the record's +28h goes to +220h. A walk that
// runs off the end changes nothing. Of kind 46h, the last element (4Ch bytes) of the vector
// unit+794h whose +4h is set gives +220h its +30h. Last +CCh[+30Ch] is hidden and +DCh[+324h]
// shown (tail call).
void support_manager_select_deck_slot_00660190(Raw s, void* record, SupportDeckSlotHost& host) {
    for (std::size_t i = 0; i < 4; ++i) host.widget_set_visible_34(s.ptr(0xCC + 4 * i), false);
    host.widget_set_visible_34(s.ptr(0xCC + 4 * s.at<std::int32_t>(0x30C)), false);
    host.widget_set_visible_34(s.ptr(0xDC + 4 * s.at<std::int32_t>(0x324)), false);
    auto cursor_unit = [&]() -> void* {
        if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
        if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
        return value_of(s.ptr(0x304));
    };
    if (s.at<std::uint32_t>(0x34C) != 0) {
        s.at<std::int32_t>(0x320) = 0;
    } else if (s.at<std::uint32_t>(0x1F4) != 0) {
        bool slot_table = host.entity_is_kind_5c(cursor_unit(), 9);
        if (!slot_table) slot_table = host.entity_is_kind_5c(cursor_unit(), 0x45);
        if (!slot_table) {
            if (host.entity_is_kind_5c(cursor_unit(), 0x46)) {
                Raw const unit = raw(cursor_unit());
                auto size = [&]() {
                    return static_cast<std::uint32_t>(static_cast<std::int32_t>(
                        static_cast<std::uint8_t*>(unit.ptr(0x798)) - static_cast<std::uint8_t*>(unit.ptr(0x794))) / 0x4C);
                };
                for (std::uint32_t i = 0; unit.ptr(0x794) != nullptr && i < size(); ++i) {
                    if (unit.ptr(0x794) == nullptr || !(i < size())) host.checked_iterator_report_00bf6713();
                    Raw const e{static_cast<std::uint8_t*>(unit.ptr(0x794)) + i * 0x4C};
                    if (e.at<std::uint32_t>(0x04) != 0) s.at<std::uint32_t>(0x220) = e.at<std::uint32_t>(0x30);
                }
            }
        } else {
            const bool carrier = host.entity_is_kind_5c(cursor_unit(), 0x45);
            void* const unit = cursor_unit();
            Raw const table = raw(unit == nullptr ? nullptr : static_cast<std::uint8_t*>(unit) + (carrier ? 0x72C : 0x1188));
            std::int32_t i = 0;
            std::uint32_t offset = 0;
            for (;;) {
                void* const cs = table.ptr(0x0C);
                if (cs != nullptr) {
                    host.enter_critical_section(cs);
                    raw(cs).at<std::int32_t>(0x18) += 1;
                }
                const std::int32_t count = table.at<std::int32_t>(0x50);
                if (cs != nullptr) {
                    raw(cs).at<std::int32_t>(0x18) += -1;
                    host.leave_critical_section(cs);
                }
                if (i >= count) {
                    i = -1;
                    break;
                }
                host.air_ops_slot_record_assign_0065ca50(record, static_cast<std::uint8_t*>(table.ptr(0x4C)) + offset);
                if (host.air_ops_slot_record_is_out_00658530(record)) break;
                const std::int32_t state = raw(record).at<std::int32_t>(0x2C);
                if (state == 2 || state == 5) break;
                ++i;
                offset += 0x58;
            }
            if (i >= 0) {
                if (i > 3) {
                    s.at<std::int32_t>(0x320) = i - 3;
                    s.at<std::int32_t>(0x324) = 3;
                } else {
                    s.at<std::int32_t>(0x324) = i;
                    s.at<std::int32_t>(0x320) = 0;
                }
                host.air_ops_slot_record_assign_0065ca50(record, static_cast<std::uint8_t*>(table.ptr(0x4C)) + i * 0x58);
                s.at<std::uint32_t>(0x220) = raw(record).at<std::uint32_t>(0x28);
            }
        }
    }
    host.widget_set_visible_34(s.ptr(0xCC + 4 * s.at<std::int32_t>(0x30C)), false);
    host.widget_set_visible_34(s.ptr(0xDC + 4 * s.at<std::int32_t>(0x324)), true);
}

// ---------------------------------------------------------------------------
// 006735D0
// ---------------------------------------------------------------------------

namespace {
void build_containers(Raw s, const SupportScreenContainer* first, const SupportScreenContainer* last,
    SupportScreenConstructHost& host) {
    for (const SupportScreenContainer* c = first; c != last; ++c) {
        void* const container = s.p + c->offset;
        void* const head = host.container_buy_head_node(c->buy_head, container);
        raw(container).ptr(4) = head;
        if (c->isnil != 0) {
            raw(head).at<std::uint8_t>(c->isnil) = 1;
            void* const h = raw(container).ptr(4);
            raw(h).ptr(4) = h;  // parent
            raw(h).ptr(0) = h;  // left
            raw(h).ptr(8) = h;  // right
        }
        raw(container).at<std::uint32_t>(8) = 0;
    }
}

// 00673618..006736C6, in order.
constexpr SupportScreenContainer kSupportContainersA[] = {
    {0x034, 0x006593A0u, 0},    {0x040, 0x00659B90u, 0x21}, {0x050, 0x00659B40u, 0x15},
    {0x15C, 0x00659480u, 0},    {0x1EC, 0x00659550u, 0},    {0x1F8, 0x00659550u, 0},
};
// 0067375D..006737B2.
constexpr SupportScreenContainer kSupportContainersB[] = {
    {0x270, 0x006595C0u, 0}, {0x27C, 0x00659350u, 0}, {0x2E4, 0x00659630u, 0}, {0x2F0, 0x00659630u, 0},
};
// 006737BD..006737EE.
constexpr SupportScreenContainer kSupportContainersC[] = {
    {0x338, 0x006596A0u, 0}, {0x344, 0x006596A0u, 0},
};
// 006737F9..00673972.
constexpr SupportScreenContainer kSupportContainersD[] = {
    {0x35C, 0x0059FD60u, 0},    {0x370, 0x004E79F0u, 0},    {0x37C, 0x006596C0u, 0},
    {0x388, 0x006596C0u, 0},    {0x394, 0x004870F0u, 0},    {0x3AC, 0x004C1630u, 0},
    {0x3B8, 0x004C1630u, 0},    {0x3C4, 0x004C1630u, 0},    {0x3E0, 0x00659760u, 0},
    {0x3EC, 0x00659760u, 0},    {0x3FC, 0x00659EB0u, 0x15}, {0x408, 0x00659F40u, 0x1D},
    {0x414, 0x005828E0u, 0x15}, {0x420, 0x005828E0u, 0x15}, {0x42C, 0x00659F90u, 0x15},
};
} // namespace

// The front-end screen base (004F7180), the vtable 00CF6B44, eight cleared dwords +10h..+2Ch,
// then the containers and the plain fields in the native's order: two observer links (+208h,
// vtable 00CE75CC, byte +218h = 1; +258h, vtable 00CE3CF0, byte +268h = 1), the object at +224h
// (vtable 00CF6544, +230h = -1, +24Ch = 0Fh), and the two cursor iterators +300h and +350h
// cleared. Each tree head gets its _Isnil byte set and its three links pointed at itself.
Raw support_manager_screen_construct_006735d0(Raw s, SupportScreenConstructHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x00) = kSupportManagerScreenVtable;
    for (std::size_t off = 0x10; off <= 0x2C; off += 4) s.at<std::uint32_t>(off) = 0;
    build_containers(s, std::begin(kSupportContainersA), std::end(kSupportContainersA), host);
    s.at<std::uint32_t>(0x20C) = 0;
    s.at<std::uint32_t>(0x210) = 0;
    s.at<std::uint32_t>(0x214) = 0;
    s.at<std::uint32_t>(0x21C) = 0;
    s.at<std::uint8_t>(0x218) = 1;
    s.at<std::uint32_t>(0x208) = 0x00CE75CCu;
    s.at<std::uint32_t>(0x224) = 0x00CF6544u;
    s.at<std::uint32_t>(0x228) = 0;
    s.at<std::uint32_t>(0x22C) = 0;
    s.at<std::int32_t>(0x230) = -1;
    s.at<std::uint32_t>(0x24C) = 0x0F;
    s.at<std::uint32_t>(0x248) = 0;
    s.at<std::uint8_t>(0x238) = 0;
    s.at<std::uint32_t>(0x250) = 0;
    s.at<std::uint32_t>(0x254) = 0;
    s.at<std::uint32_t>(0x25C) = 0;
    s.at<std::uint32_t>(0x260) = 0;
    s.at<std::uint32_t>(0x264) = 0;
    s.at<std::uint32_t>(0x26C) = 0;
    s.at<std::uint8_t>(0x268) = 1;
    s.at<std::uint32_t>(0x258) = 0x00CE3CF0u;
    build_containers(s, std::begin(kSupportContainersB), std::end(kSupportContainersB), host);
    s.at<std::uint32_t>(0x300) = 0;
    s.at<std::uint32_t>(0x304) = 0;
    build_containers(s, std::begin(kSupportContainersC), std::end(kSupportContainersC), host);
    s.at<std::uint32_t>(0x350) = 0;
    s.at<std::uint32_t>(0x354) = 0;
    build_containers(s, std::begin(kSupportContainersD), std::end(kSupportContainersD), host);
    return s;
}

// ---------------------------------------------------------------------------
// 00672660
// ---------------------------------------------------------------------------

namespace {
// A list whose elements need no destructor, torn down inline: the head's links reset, the size
// cleared, every node freed (the loop re-reads the head), the head freed and nulled.
void destroy_list_inline(Raw s, std::size_t off, SupportScreenDestructHost& host) {
    void* const head = s.ptr(off + 4);
    void* p = raw(head).ptr(0);
    raw(head).ptr(0) = head;
    raw(s.ptr(off + 4)).ptr(4) = s.ptr(off + 4);
    s.at<std::uint32_t>(off + 8) = 0;
    while (p != s.ptr(off + 4)) {
        void* const next = raw(p).ptr(0);
        host.free_00bf65ac(p);
        p = next;
    }
    host.free_00bf65ac(s.ptr(off + 4));
    s.ptr(off + 4) = nullptr;
}

void destroy_tree(Raw s, std::size_t off, std::uint32_t erase_range, SupportScreenDestructHost& host) {
    void* const tree = s.p + off;
    void* const head = s.ptr(off + 4);
    host.tree_erase_range(erase_range, tree, CheckedIter{tree, raw(head).ptr(0)}, CheckedIter{tree, head});
    host.free_00bf65ac(s.ptr(off + 4));
    s.ptr(off + 4) = nullptr;
    s.at<std::uint32_t>(off + 8) = 0;
}

// An observer link ({vtable, ..., endpoint +14h}): its vtable, the pair unregistered when the
// endpoint is set (006952A0, EDX = the link), then 00695870 on the link.
void destroy_observer_link(Raw link, std::uint32_t vtable, SupportScreenDestructHost& host) {
    link.at<std::uint32_t>(0) = vtable;
    if (link.ptr(0x14) != nullptr) host.observer_unregister_pair_006952a0(link.ptr(0x14), link.p);
    host.callback_owner_destroy_00695870(link.p);
}

void release_handle(Raw s, std::size_t off, SupportScreenDestructHost& host) {
    void* const h = s.ptr(off);
    if (h == nullptr) return;
    if (host.interlocked_decrement(&raw(h).at<long>(4)) == 0) host.object_virtual_00(h);
    s.ptr(off) = nullptr;
}
} // namespace

// 00672660 (the only caller is the scalar deleting destructor 006739F0). The vtable is restored,
// then: each site pair's map icon (+338h pairs, node+18h) and each value +10h of the map +3FCh
// (stepped by 00654650) is deleted (vtable +4h(1)), with the native's checked-iterator tests;
// the icon pairs +7Ch../+ACh.. (four each) are deleted; the map +42Ch is emptied by hand
// (00659850 on each right subtree, nodes freed, head relinked); the pages +16Ch and +78h are
// removed (004C12B0, 00AA31F0); then every container is destroyed in reverse order of
// construction (trees through their erase-range routine, the two pair lists through 00665210,
// the others inline), the two observer links +224h/+258h and +208h with the string at +238h, the
// eight sound handles +2Ch..+10h (refcount at +4h, slot 0 at zero), and last the front-end
// screen base (004F71A0).
void support_manager_screen_destruct_00672660(Raw s, SupportScreenDestructHost& host) {
    s.at<std::uint32_t>(0) = kSupportManagerScreenVtable;
    void* const pairs = s.p + 0x338;
    for (void* node = next_of(s.ptr(0x33C));;) {
        void* const end = s.ptr(0x33C);
        if (pairs == nullptr) host.checked_iterator_report_00bf6713();
        if (node == end) break;
        if (pairs == nullptr) host.checked_iterator_report_00bf6713();
        if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
        if (raw(node).ptr(0x18) != nullptr) host.object_delete_04(raw(node).ptr(0x18), 1);
        if (node == raw(pairs).ptr(4)) host.checked_iterator_report_00bf6713();
        node = next_of(node);
    }
    void* const map = s.p + 0x3FC;
    CheckedIter it{map, raw(s.ptr(0x400)).ptr(0)};
    for (;;) {
        void* const end = raw(map).ptr(4);
        if (it.container == nullptr || it.container != map) host.checked_iterator_report_00bf6713();
        if (it.node == end) break;
        if (it.container == nullptr) host.checked_iterator_report_00bf6713();
        if (it.node == raw(it.container).ptr(4)) host.checked_iterator_report_00bf6713();
        if (raw(it.node).ptr(0x10) != nullptr) host.object_delete_04(raw(it.node).ptr(0x10), 1);
        host.tree_iterator_increment_00654650(it);
    }
    for (std::size_t i = 0; i < 4; ++i) {
        if (s.ptr(0x7C + 4 * i) != nullptr) host.object_delete_04(s.ptr(0x7C + 4 * i), 1);
        if (s.ptr(0xAC + 4 * i) != nullptr) host.object_delete_04(s.ptr(0xAC + 4 * i), 1);
    }
    {
        void* const tree = s.p + 0x42C;
        void* node = raw(s.ptr(0x430)).ptr(4);
        while (raw(node).at<std::uint8_t>(0x15) == 0) {
            host.tree_erase_subtree_00659850(tree, raw(node).ptr(8));
            void* const left = raw(node).ptr(0);
            host.free_00bf65ac(node);
            node = left;
        }
        void* const head = s.ptr(0x430);
        raw(head).ptr(4) = head;
        s.at<std::uint32_t>(0x434) = 0;
        raw(s.ptr(0x430)).ptr(0) = s.ptr(0x430);
        raw(s.ptr(0x430)).ptr(8) = s.ptr(0x430);
    }
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.ptr(0x16C));
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.ptr(0x78));
    destroy_tree(s, 0x42C, 0x006647B0u, host);
    destroy_tree(s, 0x420, 0x0058B000u, host);
    destroy_tree(s, 0x414, 0x0058B000u, host);
    destroy_tree(s, 0x408, 0x0066DBC0u, host);
    destroy_tree(s, 0x3FC, 0x00663370u, host);
    for (std::size_t off : {0x3ECu, 0x3E0u, 0x3C4u, 0x3B8u, 0x3ACu, 0x394u, 0x388u, 0x37Cu, 0x370u, 0x35Cu}) {
        destroy_list_inline(s, off, host);
    }
    for (std::size_t off : {0x344u, 0x338u}) {
        host.pair_list_clear_00665210(s.p + off);
        host.free_00bf65ac(s.ptr(off + 4));
        s.ptr(off + 4) = nullptr;
    }
    for (std::size_t off : {0x2F0u, 0x2E4u, 0x27Cu, 0x270u}) destroy_list_inline(s, off, host);
    Raw const holder{s.p + 0x224};
    holder.at<std::uint32_t>(0) = 0x00CF6544u;
    destroy_observer_link(Raw{holder.p + 0x34}, 0x00CE3C8Cu, host);
    if (holder.at<std::uint32_t>(0x28) >= 0x10) host.free_00bf65ac(holder.ptr(0x14));
    holder.at<std::uint32_t>(0x28) = 0x0F;
    holder.at<std::uint32_t>(0x24) = 0;
    holder.at<std::uint8_t>(0x14) = 0;
    destroy_observer_link(Raw{s.p + 0x208}, 0x00CE74FCu, host);
    for (std::size_t off : {0x1F8u, 0x1ECu, 0x15Cu}) destroy_list_inline(s, off, host);
    destroy_tree(s, 0x50, 0x006641B0u, host);
    destroy_tree(s, 0x40, 0x0066FDD0u, host);
    destroy_list_inline(s, 0x34, host);
    for (std::size_t off = 0x2C; off >= 0x10; off -= 4) release_handle(s, off, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// ---------------------------------------------------------------------------
// 0066A130
// ---------------------------------------------------------------------------

namespace {
// clear() inlined for a list of plain values: head relinked, size 0, nodes freed (the loop
// re-reads the head).
template <class Host>
void clear_list_inline(Raw s, std::size_t off, Host& host) {
    void* p = raw(s.ptr(off + 4)).ptr(0);
    raw(s.ptr(off + 4)).ptr(0) = s.ptr(off + 4);
    raw(s.ptr(off + 4)).ptr(4) = s.ptr(off + 4);
    s.at<std::uint32_t>(off + 8) = 0;
    while (p != s.ptr(off + 4)) {
        void* const next = raw(p).ptr(0);
        host.free_00bf65ac(p);
        p = next;
    }
}

// Adds the class's picture (texture class+D4h, UV 0, 0, 1, 1 from 00D7A24C) as a state of the
// big icon +1CCh when the map +42Ch has no state for `key` yet, and records the index there.
// `between` runs between the add and the subscript (the native's head test on its loop node).
template <class Between>
void ensure_icon_state(Raw s, const void* key, void* klass, SupportEntriesHost& host, Between between) {
    void* const map = s.p + 0x42C;
    void* const end = s.ptr(0x430);
    const CheckedIter found = host.icon_map_find_00614060(map, key);
    if (found.container == nullptr || found.container != map) host.checked_iterator_report_00bf6713();
    if (found.node != end) return;
    const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    const std::uint32_t state = host.gui_icon_add_state_00ab64c0(s.ptr(0x1CC), raw(klass).ptr(0xD4), uv);
    between();
    *host.icon_map_subscript_006171b0(map, key) = state;
}
} // namespace

// 0066A130 (callers: Enter 00674B80, 0066D020, 00675C40). Lists +2E4h, +37Ch and +394h are cleared.
// With the second unit list +344h in use: 006697E0 collects the catalog into list +370h, and each
// of its entries {class id +8h, +0Ch, count +10h} pushes its vehicle class (00964790(id, 1)) onto
// +37Ch and its count onto +394h and gets an icon state. Otherwise, with units, the cursor unit
// decides: kind 45h or 9 walks its slot table (unit+72Ch for kind 45h, else +1188h; 0 for a null
// unit, read through by the native) list at +40h: each class there that is listed in +2F0h while
// byte +71h is set, or whose count +0Ch is non-zero, is pushed onto +2E4h with an icon state, and
// +2F0h then becomes a copy of +2E4h (00668290). Kind 46h walks the vector at unit+770h (20h-byte
// elements): each element whose class +4h is in list +388h while +71h is set, or whose +8h is
// non-zero, pushes the class onto +37Ch with an icon state (key class+70h), and +388h becomes a
// copy of +37Ch (00669EE0). Returns 1 when +37Ch or +2E4h holds anything.
std::int32_t support_manager_build_entries_0066a130(Raw s, SupportEntriesHost& host) {
    clear_list_inline(s, 0x2E4, host);
    clear_list_inline(s, 0x37C, host);
    clear_list_inline(s, 0x394, host);
    auto push = [&](std::size_t off, const void* value, void* (SupportEntriesHost::*buy)(void*, void*, void*, const void*),
                    void (SupportEntriesHost::*inc)(void*, int)) {
        void* const list = s.p + off;
        void* const head = raw(list).ptr(4);
        void* const node = (host.*buy)(list, head, prev_of(head), value);
        (host.*inc)(list, 1);
        raw(head).ptr(4) = node;
        raw(prev_of(node)).ptr(0) = node;
    };
    if (s.at<std::uint32_t>(0x34C) != 0) {
        host.support_collect_catalog_006697e0(s);
        for (void* node = next_of(s.ptr(0x374));;) {
            if (node == s.ptr(0x374)) break;
            if (node == s.ptr(0x374)) host.checked_iterator_report_00bf6713();
            void* const klass = host.vehicle_class_get_or_create_00964790(raw(node).at<std::uint32_t>(8), true);
            push(0x37C, &klass, &SupportEntriesHost::list37c_buy_node_00659cf0, &SupportEntriesHost::list37c_inc_size_00663230);
            if (node == s.ptr(0x374)) host.checked_iterator_report_00bf6713();
            const std::uint32_t count = raw(node).at<std::uint32_t>(0x10);
            push(0x394, &count, &SupportEntriesHost::list394_buy_node_004857f0, &SupportEntriesHost::list394_inc_size_0048fc80);
            if (node == s.ptr(0x374)) host.checked_iterator_report_00bf6713();
            const std::uint32_t key = raw(node).at<std::uint32_t>(8);
            ensure_icon_state(s, &key, klass, host, [&]() {
                if (node == s.ptr(0x374)) host.checked_iterator_report_00bf6713();
            });
            if (node == s.ptr(0x374)) host.checked_iterator_report_00bf6713();
            node = next_of(node);
        }
    } else if (s.at<std::uint32_t>(0x1F4) != 0) {
        void* const cursor = s.p + 0x300;
        auto cursor_unit = [&]() -> void* {
            if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
            if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
            return value_of(s.ptr(0x304));
        };
        bool slot_table = host.entity_is_kind_5c(cursor_unit(), 0x45);
        if (!slot_table) slot_table = host.entity_is_kind_5c(cursor_unit(), 9);
        if (!slot_table) {
            if (host.entity_is_kind_5c(*host.list_iterator_deref_00658a60(cursor), 0x46)) {
                void* const vec = static_cast<std::uint8_t*>(*host.list_iterator_deref_00658a60(cursor)) + 0x770;
                if (host.vector_size_00653470(vec) != 0) {
                    host.vector_at_00653490(vec, 0);
                    for (std::uint32_t i = 0;; ++i) {
                        Raw const v = raw(vec);
                        if (v.ptr(4) == nullptr) break;
                        if (!(i < static_cast<std::uint32_t>(
                                      (static_cast<std::uint8_t*>(v.ptr(8)) - static_cast<std::uint8_t*>(v.ptr(4))) >> 5))) {
                            break;
                        }
                        Raw const e = raw(host.vector_at_00653490(vec, i));
                        bool listed = false;
                        for (void* node = next_of(s.ptr(0x38C));;) {
                            if (node == s.ptr(0x38C)) break;
                            if (node == s.ptr(0x38C)) host.checked_iterator_report_00bf6713();
                            if (raw(node).ptr(8) == e.ptr(4)) {
                                listed = true;
                                break;
                            }
                            if (node == s.ptr(0x38C)) host.checked_iterator_report_00bf6713();
                            node = next_of(node);
                        }
                        if ((s.at<std::uint8_t>(0x71) != 0 && listed) || e.at<std::uint32_t>(8) != 0) {
                            push(0x37C, e.p + 4, &SupportEntriesHost::list37c_buy_node_00659cf0,
                                &SupportEntriesHost::list37c_inc_size_00663230);
                            void* const klass = e.ptr(4);
                            ensure_icon_state(s, static_cast<std::uint8_t*>(klass) + 0x70, klass, host, []() {});
                        }
                    }
                    host.list_assign_00669ee0(s.p + 0x388, s.p + 0x37C);
                }
            }
        } else {
            const bool carrier = host.entity_is_kind_5c(cursor_unit(), 0x45);
            void* const unit = *host.list_iterator_deref_00658a60(cursor);
            Raw const table = raw(unit == nullptr ? nullptr : static_cast<std::uint8_t*>(unit) + (carrier ? 0x72C : 0x1188));
            for (void* node = next_of(table.ptr(0x44));;) {
                if (node == table.ptr(0x44)) break;
                bool listed = false;
                for (void* other = next_of(s.ptr(0x2F4));;) {
                    if (other == s.ptr(0x2F4)) break;
                    if (other == s.ptr(0x2F4)) host.checked_iterator_report_00bf6713();
                    if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                    if (raw(other).ptr(8) == raw(node).ptr(8)) {
                        listed = true;
                        break;
                    }
                    if (other == s.ptr(0x2F4)) host.checked_iterator_report_00bf6713();
                    other = next_of(other);
                }
                bool take = s.at<std::uint8_t>(0x71) != 0 && listed;
                if (!take) {
                    if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                    take = raw(node).at<std::uint32_t>(0x0C) != 0;
                }
                if (take) {
                    if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                    void* const klass = raw(node).ptr(8);
                    push(0x2E4, &klass, &SupportEntriesHost::list2e4_buy_node_00659c90,
                        &SupportEntriesHost::list2e4_inc_size_006630f0);
                    if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                    void* const k = raw(node).ptr(8);
                    void* const map = s.p + 0x42C;
                    void* const end = s.ptr(0x430);
                    const CheckedIter found = host.icon_map_find_00614060(map, static_cast<std::uint8_t*>(k) + 0x70);
                    if (found.container == nullptr || found.container != map) host.checked_iterator_report_00bf6713();
                    if (found.node == end) {
                        const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                        if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                        const std::uint32_t state =
                            host.gui_icon_add_state_00ab64c0(s.ptr(0x1CC), raw(raw(node).ptr(8)).ptr(0xD4), uv);
                        if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                        *host.icon_map_subscript_006171b0(map, static_cast<std::uint8_t*>(raw(node).ptr(8)) + 0x70) = state;
                    }
                }
                if (node == table.ptr(0x44)) host.checked_iterator_report_00bf6713();
                node = next_of(node);
            }
            void* const src = s.p + 0x2E4;
            if (s.p + 0x2F0 != src) {
                void* const head = raw(src).ptr(4);
                host.list_assign_range_00668290(s.p + 0x2F0, CheckedIter{src, next_of(head)}, CheckedIter{src, head}, 0);
            }
        }
    }
    return (s.at<std::uint32_t>(0x384) == 0 && s.at<std::uint32_t>(0x2EC) == 0) ? 0 : 1;
}

// ---------------------------------------------------------------------------
// 0066D020
// ---------------------------------------------------------------------------

namespace {
// 0066D192..0066D261 / 0066D39B..: the payload icon +158h takes state 3's texture and UV from the
// selected weapon's device class (resolved twice, once for the texture +7Ch and once for the UV
// +90h..+9Ch of (class+74h)+34h), then frames 0 and 3; the plain group +F0h is hidden and the
// payload group +F4h shown.
void show_weapon_payload_icon(Raw s, void* entry, SupportSelectionHost& host) {
    auto device = [&]() {
        Raw const weapon = raw(host.entry_weapon_009552e0(entry, s.at<std::int32_t>(0x334)));
        void* const id = raw(raw(weapon.ptr(4)).ptr(8)).ptr(4);
        return raw(host.device_class_resolve_00443490(id, false));
    };
    void* const texture = raw(raw(device().ptr(0x74)).ptr(0x34)).ptr(0x7C);
    Raw const uv_src = raw(raw(device().ptr(0x74)).ptr(0x34));
    const std::uint32_t uv[4] = {uv_src.at<std::uint32_t>(0x90), uv_src.at<std::uint32_t>(0x94),
                                 uv_src.at<std::uint32_t>(0x98), uv_src.at<std::uint32_t>(0x9C)};
    host.gui_set_state_texture_uv_00ab2690(s.ptr(0x158), 3, texture, uv);
    host.widget_set_frame_88(s.ptr(0x158), 0, 0, 1.0f);
    host.widget_set_frame_88(s.ptr(0x158), 3, 0, 1.0f);
    host.widget_set_visible_34(s.ptr(0xF0), false);
    host.widget_set_visible_34(s.ptr(0xF4), true);
}

void finish_payload_frame(Raw s, int frame, SupportSelectionHost& host) {
    host.widget_set_frame_88(s.ptr(0x158), frame, 0, 1.0f);
    host.widget_set_visible_34(s.ptr(0xF0), false);
    host.widget_set_visible_34(s.ptr(0xF4), true);
}
} // namespace

// Shows the resource text +1A8h and clears the weapon index +334h. The unit path (no second-list
// entries, units present, cursor unit not kind 46h): hides +D8h, rebuilds (0066A130, stopping on
// 0), frames the four strip icons 3, sets +5Ch = 2, runs 0065A740 and 0065EFA0, takes the entry
// 0065AE90 into +2A0h with +2B0h = 1; flag 10h selects weapon 1. Entries without flag 0Fh, or
// with 17h, end with frame 1 (17h) or the plain group. The rest run 00669170, 00666DC0 and
// 00662210 and show the weapon's icon (or frame 0 without a weapon). The catalog path (second
// list in use or a kind-46h cursor unit) needs 0066A130 and catalog entries (+384h), runs
// 0065EE90, +5Ch = 2, 00665F30, +2B0h = 1, 0065A740 and 0066B6C0 on +3A4h, and 00662020 after the
// flag-10h test; entries with 0Fh and without 17h show the weapon's icon (or frame 1/0); a 17h
// entry gets frame 1 and nothing else; a kind-6 entry named "globals.unitclass_kamikazeboat"
// (00435C40, case-insensitive) gets frame 2 and the payload group, the rest the plain group.
void support_manager_refresh_selection_0066d020(Raw s, SupportSelectionHost& host) {
    host.widget_set_visible_34(s.ptr(0x1A8), true);
    s.at<std::uint32_t>(0x334) = 0;
    bool catalog = s.at<std::uint32_t>(0x34C) != 0;
    if (!catalog) {
        if (s.at<std::uint32_t>(0x1F4) == 0) return;
        if (s.ptr(0x300) == nullptr) host.checked_iterator_report_00bf6713();
        if (s.ptr(0x304) == raw(s.ptr(0x300)).ptr(4)) host.checked_iterator_report_00bf6713();
        catalog = host.entity_is_kind_5c(value_of(s.ptr(0x304)), 0x46);
    }
    if (!catalog) {
        if (s.at<std::uint32_t>(0x1F4) == 0) return;
        host.widget_set_visible_34(s.ptr(0xD8), false);
        if (host.support_build_entries_0066a130(s) == 0) return;
        for (std::size_t off = 0x8C; off <= 0x98; off += 4) host.widget_set_frame_88(s.ptr(off), 3, 0, 1.0f);
        s.at<std::uint32_t>(0x5C) = 2;
        host.support_update_payload_strip_0065a740(s);
        host.support_sync_weapon_cursor_0065efa0(s);
        void* const entry = host.support_catalog_entry_2e4_0065ae90(s);
        s.ptr(0x2A0) = entry;
        s.at<std::uint8_t>(0x2B0) = 1;
        if (host.entry_has_flag_18(entry, 0x10)) s.at<std::uint32_t>(0x334) = 1;
        if (!host.entry_has_flag_18(s.ptr(0x2A0), 0x0F) || host.entry_has_flag_18(s.ptr(0x2A0), 0x17)) {
            if (host.entry_has_flag_18(s.ptr(0x2A0), 0x17)) {
                host.widget_set_frame_88(s.ptr(0x158), 1, 0, 1.0f);
                host.widget_set_visible_34(s.ptr(0xF0), false);
                host.widget_set_visible_34(s.ptr(0xF4), true);
            } else {
                host.widget_set_visible_34(s.ptr(0xF0), true);
                host.widget_set_visible_34(s.ptr(0xF4), false);
            }
            return;
        }
        host.support_select_weapon_value_00669170(s);
        host.support_build_hangar_loadout_ring_00666dc0(s);
        host.support_highlight_hangar_loadout_00662210(s);
        if (s.at<std::uint32_t>(0x334) != 0) {
            show_weapon_payload_icon(s, s.ptr(0x2A0), host);
        } else {
            finish_payload_frame(s, 0, host);
        }
        return;
    }
    if (host.support_build_entries_0066a130(s) == 0) return;
    if (s.at<std::uint32_t>(0x384) == 0) return;
    for (std::size_t off = 0x8C; off <= 0x98; off += 4) host.widget_set_frame_88(s.ptr(off), 3, 0, 1.0f);
    host.support_sync_catalog_cursor_0065ee90(s);
    s.at<std::uint32_t>(0x5C) = 2;
    host.support_build_loadout_ring_00665f30(s);
    s.at<std::uint8_t>(0x2B0) = 1;
    host.support_update_payload_strip_0065a740(s);
    host.support_refresh_info_panel_0066b6c0(s);
    if (host.entry_has_flag_18(s.ptr(0x3A4), 0x10)) s.at<std::uint32_t>(0x334) = 1;
    host.support_highlight_loadout_00662020(s);
    if (host.entry_has_flag_18(s.ptr(0x3A4), 0x0F) && !host.entry_has_flag_18(s.ptr(0x3A4), 0x17)) {
        if (s.at<std::uint32_t>(0x334) != 0) {
            show_weapon_payload_icon(s, s.ptr(0x3A4), host);
        } else {
            finish_payload_frame(s, host.entry_has_flag_18(s.ptr(0x3A4), 0x17) ? 1 : 0, host);
        }
        return;
    }
    if (host.entry_has_flag_18(s.ptr(0x3A4), 0x17)) {
        host.widget_set_frame_88(s.ptr(0x158), 1, 0, 1.0f);
        return;
    }
    bool kamikaze = false;
    if (host.entry_has_flag_18(s.ptr(0x3A4), 6)) {
        alignas(4) std::uint8_t wanted[8];
        alignas(4) std::uint8_t name[8];
        host.native_string_from_cstr_0041e870(wanted, "globals.unitclass_kamikazeboat");
        host.native_string_from_cstr_0041e870(name, static_cast<const char*>(raw(s.ptr(0x3A4)).ptr(0x54)));
        kamikaze = host.native_string_equals_insensitive_00435c40(name, wanted);
        Raw const n{name};
        if (n.ptr(4) != nullptr) host.native_string_pool_return(n.ptr(4), n.at<std::uint32_t>(0) + 1);
        Raw const w{wanted};
        if (w.ptr(4) != nullptr) host.native_string_pool_return(w.ptr(4), w.at<std::uint32_t>(0) + 1);
    }
    if (kamikaze) {
        host.widget_set_frame_88(s.ptr(0x158), 2, 0, 1.0f);
        host.widget_set_visible_34(s.ptr(0xF0), false);
        host.widget_set_visible_34(s.ptr(0xF4), true);
    } else {
        host.widget_set_visible_34(s.ptr(0xF0), true);
        host.widget_set_visible_34(s.ptr(0xF4), false);
    }
}

// ---------------------------------------------------------------------------
// 00673130
// ---------------------------------------------------------------------------

// The info panel page +16Ch with the resource text +1A8h and the bar icons +180h (piros),
// +184h (vilagos) and +188h (sotet); +1ACh is the bar's full width. Hidden outright when byte
// +9h or +8h is set, 0066FFA0 reports nothing available, or ((*00E198C4)+C0h)+4h is set. A
// hidden page is shown (bars 0, 0, 1) in game modes 0, 1, 2, 3 and 8 or with +66h set; a shown
// page is hidden again outside those modes when +66h is clear. Byte +4h stops everything after
// that. In modes 0..3 the text reads "ingame.sm_cp|.: " + the player's support points
// (00946F00, _ftol2) and the dark bar's width is points * +1ACh / cap (00942130, double
// quotient stored as a float); in the other modes the text is "ingame.sm_support" and the bar is
// full. Either way the dark bar keeps its height and is shown and the red bar gets width 0.
// The mode is re-read (004BCA50) for every comparison, as the native does.
void support_manager_update_info_panel_00673130(Raw s, SupportInfoPanelHost& host) {
    auto mode_is = [&](std::int32_t m) { return host.effective_game_mode_004bca50(host.game()) == m; };
    auto mode_in_set = [&](bool with_8) {
        return mode_is(0) || mode_is(1) || mode_is(2) || mode_is(3) || (with_8 && mode_is(8));
    };
    auto hide_all = [&]() {
        host.widget_set_visible_34(s.ptr(0x16C), false);
        host.widget_set_visible_34(s.ptr(0x180), false);
        host.widget_set_visible_34(s.ptr(0x184), false);
        host.widget_set_visible_34(s.ptr(0x188), false);
    };
    if (s.at<std::uint8_t>(0x09) != 0 || s.at<std::uint8_t>(0x08) != 0 || !host.has_available_entries_0066ffa0(s) ||
        raw(host.ingame_interface().ptr(0xC0)).at<std::uint8_t>(0x04) != 0) {
        hide_all();
        return;
    }
    if (!host.widget_is_visible_38(s.ptr(0x16C))) {
        if (mode_in_set(true) || s.at<std::uint8_t>(0x66) != 0) {
            host.widget_set_visible_34(s.ptr(0x16C), true);
            host.widget_set_visible_34(s.ptr(0x180), false);
            host.widget_set_visible_34(s.ptr(0x184), false);
            host.widget_set_visible_34(s.ptr(0x188), true);
        }
    } else {
        if (s.at<std::uint8_t>(0x04) != 0) return;
        if (!mode_in_set(true) && host.widget_is_visible_38(s.ptr(0x16C)) && s.at<std::uint8_t>(0x66) == 0) {
            hide_all();
            return;
        }
    }
    if (s.at<std::uint8_t>(0x04) != 0) return;
    if (!mode_in_set(false)) {
        alignas(4) std::uint8_t text[8];
        host.native_string_from_cstr_0041e870(text, "ingame.sm_support");
        host.text_set_localised_source_00abaed0(s.ptr(0x1A8), text, true);
        host.native_string_destroy_0041dd20(text);
        const float bar[2] = {s.at<float>(0x1AC), host.widget_size_00aa6740(s.ptr(0x188))[1]};
        host.widget_set_size_58(s.ptr(0x188), bar);
        host.widget_set_visible_34(s.ptr(0x188), true);
        const float red[2] = {0.0f, host.widget_size_00aa6740(s.ptr(0x188))[1]};
        host.widget_set_size_58(s.ptr(0x180), red);
        return;
    }
    const std::int32_t player = host.game().at<std::int32_t>(0x18EC);
    alignas(4) std::uint8_t number[8];
    alignas(4) std::uint8_t label[8];
    alignas(4) std::uint8_t joined[8];
    host.native_string_from_int_004260b0(number, host.ftol_00bf7420(host.support_points_00946f00(player)));
    host.native_string_from_cstr_0041e870(label, "ingame.sm_cp|.: ");
    void* const full = host.native_string_concat_004261a0(label, joined, number);
    host.text_set_localised_source_00abaed0(s.ptr(0x1A8), full, true);
    host.native_string_destroy_0041dd20(joined);
    host.native_string_destroy_0041dd20(label);
    host.native_string_destroy_0041dd20(number);
    const float height = host.widget_size_00aa6740(s.ptr(0x188))[1];
    const std::int32_t player2 = host.game().at<std::int32_t>(0x18EC);
    const double scaled = host.support_points_00946f00(player2) * static_cast<double>(s.at<float>(0x1AC));
    const float bar[2] = {static_cast<float>(scaled / host.support_points_cap_00942130(player2)), height};
    host.widget_set_size_58(s.ptr(0x188), bar);
    host.widget_set_visible_34(s.ptr(0x188), true);
    const float red[2] = {0.0f, host.widget_size_00aa6740(s.ptr(0x188))[1]};
    host.widget_set_size_58(s.ptr(0x180), red);
}

// ---------------------------------------------------------------------------
// 0065D070
// ---------------------------------------------------------------------------

// For each new member e (vector element, ECX of 0077D600) while the request's leader +21Ch is
// set: with the same side (+54h) and a leader of kind 6, e gets command 00E08F68 on an 18h-byte
// target block {byte valid = 1, word +2h = leader id +174h, +4h leader, +8h..+10h the zero vector
// 00F87574, +14h 0.0}; with a different side, e gets command 00E08F78 on 00465080(leader, 0.0)
// when the leader is neither kind 0Fh nor 8, or is kind 0Fh and e is kind 13h (the native calls
// the leader's kind 0Fh test twice on the second path, as here). Afterwards the leader link is
// dropped (006952A0, EDX = request+208h; +21Ch = 0) and the vector storage freed.
void spawn_request_order_members_0065d070(Raw request, std::uint32_t /*proxy*/, void** first, void** last,
    void** /*end*/, SpawnMemberOrdersHost& host) {
    alignas(4) std::uint8_t target[0x18];
    for (std::uint32_t i = 0;
         first != nullptr && i < static_cast<std::uint32_t>((reinterpret_cast<std::intptr_t>(last) -
                                                             reinterpret_cast<std::intptr_t>(first)) >> 2);
         ++i) {
        void* const leader = request.ptr(0x21C);
        void* const member = first[i];
        if (leader == nullptr) continue;
        if (raw(leader).at<std::uint32_t>(0x54) == raw(member).at<std::uint32_t>(0x54)) {
            if (!host.entity_is_kind_5c(leader, 6)) continue;
            void* const holder = request.ptr(0x21C);
            float zero[3];
            host.zero_vector_00f87574(zero);
            Raw t{target};
            t.at<std::uint16_t>(0x00) = 0;
            t.ptr(0x04) = holder;
            std::memcpy(target + 0x08, zero, sizeof zero);
            std::uint16_t id = 0;
            if (holder == nullptr) {
                t.at<std::uint8_t>(0x00) = 0;
            } else {
                t.at<std::uint8_t>(0x00) = 1;
                id = raw(holder).at<std::uint16_t>(0x174);
            }
            t.at<std::uint16_t>(0x02) = id;
            t.at<float>(0x14) = 0.0f;
            host.entity_issue_command_0077d600(member, 0x00E08F68u, target, 1);
            continue;
        }
        bool issue = false;
        if (!host.entity_is_kind_5c(request.ptr(0x21C), 0x0F) && !host.entity_is_kind_5c(request.ptr(0x21C), 8)) {
            issue = true;
        } else if (host.entity_is_kind_5c(request.ptr(0x21C), 0x0F) && host.entity_is_kind_5c(member, 0x13)) {
            issue = true;
        }
        if (issue) {
            void* const t = host.command_target_from_entity_00465080(target, request.ptr(0x21C), 0.0f);
            host.entity_issue_command_0077d600(member, 0x00E08F78u, t, 1);
        }
    }
    if (request.ptr(0x21C) != nullptr) {
        host.observer_unregister_pair_006952a0(request.ptr(0x21C), request.p + 0x208);
        request.ptr(0x21C) = nullptr;
    }
    if (first != nullptr) host.free_00bf65ac(first);
}

// ---------------------------------------------------------------------------
// Bomb / torpedo view
// ---------------------------------------------------------------------------

namespace {
// The camera's world position (+FCh..+104h, refreshed first when +C8h is clear) into
// view+28h..+30h.
void take_camera_position(Raw view, void* camera, BombViewHost& host) {
    Raw const c = raw(camera);
    if (c.at<std::uint8_t>(0xC8) == 0) host.entity_refresh_world_00414db0(camera);
    view.at<float>(0x28) = c.at<float>(0xFC);
    view.at<float>(0x2C) = c.at<float>(0x100);
    view.at<float>(0x30) = c.at<float>(0x104);
}
} // namespace

// 0067B4D0 (caller BSP_InGameInterface_ApplyPendingInterface, the 26h arm at 0068B163). With no
// camera in +4Ch or +50h one is built: the torpedo view (3CCh bytes, 00435120, into +50h) for no
// target or a target of kind 2Bh, else the bomb view (3F8h bytes, 00435280, into +4Ch); both
// zero-filled before construction and left 0 when new fails. The new camera is attached with the
// identity matrix (1.0 at 00D7A24C) through 00923870(0, game+19CCh, &m) (ECX = the camera, not
// tested) and registered with the view (00694A60, EDX = view+8h). Then the bomb camera, when
// present, gets the target and launcher (004307F0), becomes the camera mover (004BC410) and gives
// its position to +28h..+30h; the torpedo camera the same with 0042FEE0(target). Last the target
// is registered with the view (00694A60, ECX = target, not tested).
void bomb_view_ensure_camera_0067b4d0(Raw view, void* target, std::uint32_t launcher, BombViewHost& host) {
    if (view.ptr(0x4C) == nullptr && view.ptr(0x50) == nullptr) {
        const bool torpedo = target == nullptr || host.entity_is_kind_5c(target, 0x2B);
        const std::uint32_t size = torpedo ? 0x3CCu : 0x3F8u;
        void* const memory = host.operator_new_00bf55be(size);
        host.memset_00bf79f0(memory, 0, size);
        void* camera = nullptr;
        if (memory != nullptr) {
            camera = torpedo ? host.torpedo_view_camera_construct_00435120(memory)
                             : host.bomb_view_camera_construct_00435280(memory);
        }
        view.ptr(torpedo ? 0x50 : 0x4C) = camera;
        const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        host.camera_attach_00923870(camera, 0, host.game().ptr(0x19CC), identity);
        host.observer_register_pair_00694a60(view.ptr(torpedo ? 0x50 : 0x4C), view.p + 8);
    }
    if (view.ptr(0x4C) != nullptr) {
        host.bomb_view_set_target_004307f0(view.ptr(0x4C), target, launcher);
        host.game_set_camera_mover_004bc410(host.game(), view.ptr(0x4C));
        take_camera_position(view, view.ptr(0x4C), host);
    }
    if (view.ptr(0x50) != nullptr) {
        host.torpedo_view_set_target_0042fee0(view.ptr(0x50), target);
        host.game_set_camera_mover_004bc410(host.game(), view.ptr(0x50));
        take_camera_position(view, view.ptr(0x50), host);
    }
    host.observer_register_pair_00694a60(target, view.p + 8);
}

namespace {
// One camera's velocity pass of 0067B7E0: (world position - +28h..+30h) / dt, each component
// stored as a float before and after the division; the position is taken into +28h..+30h and
// 00B0D6A0 gets the tuning floats +D8h, +DCh, +E0h (0042E740) and the velocity.
void report_camera_velocity(Raw view, std::size_t field, float dt, BombViewHost& host) {
    Raw c = raw(view.ptr(field));
    if (c.at<std::uint8_t>(0xC8) == 0) host.entity_refresh_world_00414db0(c.p);
    const float dx = c.at<float>(0xFC) - view.at<float>(0x28);
    const float dy = c.at<float>(0x100) - view.at<float>(0x2C);
    const float dz = c.at<float>(0x104) - view.at<float>(0x30);
    c = raw(view.ptr(field));
    const float vx = dx / dt;
    const float vy = dy / dt;
    const float vz = dz / dt;
    if (c.at<std::uint8_t>(0xC8) == 0) host.entity_refresh_world_00414db0(c.p);
    view.at<float>(0x28) = c.at<float>(0xFC);
    view.at<float>(0x2C) = c.at<float>(0x100);
    view.at<float>(0x30) = c.at<float>(0x104);
    Raw const t_e0 = host.game_tuning_0042e740();
    Raw const t_dc = host.game_tuning_0042e740();
    Raw const t_d8 = host.game_tuning_0042e740();
    host.render_set_transition_00b0d6a0(t_d8.at<float>(0xD8), t_dc.at<float>(0xDC), t_e0.at<float>(0xE0), vx, vy, vz);
}
} // namespace

// 0067B7E0. Once the game time *00F876A4 passes +54h + 0.25 (00D7A348, double) and the input
// manager's +1FA8h flag is clear or its +1FA4h is not above 0.0 (00D7A218): unless 0068A9C0
// applies a pending level change, the HUD root is re-selected (006485A0) when
// ((*00E198C4)+9Ch)+20h is clear, and a queued pick (+48h, at +40h/+44h) is handed to
// (*00E198C4)+4Ch (+44h, +48h, byte +4Ch = 1); +48h is cleared either way. Then for dt > 0.0
// each of the bomb (+4Ch) and torpedo (+50h) cameras reports its velocity.
void bomb_view_update_0067b7e0(Raw view, float dt, BombViewHost& host) {
    if (static_cast<double>(view.at<float>(0x54)) + 0.25 < static_cast<double>(host.game_time_00f876a4())) {
        Raw const input{raw(host.input_manager_004bec00().ptr(4)).p + 0x1F80};
        if (!(input.at<std::uint8_t>(0x28) != 0 && input.at<float>(0x24) > 0.0f)) {
            if (!host.apply_pending_level_change_0068a9c0(host.ingame_interface())) {
                Raw const iface = host.ingame_interface();
                if (raw(iface.ptr(0x9C)).at<std::uint8_t>(0x20) == 0) {
                    host.force_select_unit_006485a0(iface.ptr(0x40));
                    if (view.at<std::uint8_t>(0x48) != 0) {
                        Raw const pick = raw(host.ingame_interface().ptr(0x4C));
                        pick.at<float>(0x44) = view.at<float>(0x40);
                        pick.at<float>(0x48) = view.at<float>(0x44);
                        pick.at<std::uint8_t>(0x4C) = 1;
                    }
                }
                view.at<std::uint8_t>(0x48) = 0;
            }
        }
    }
    if (view.ptr(0x4C) != nullptr && dt > 0.0f) report_camera_velocity(view, 0x4C, dt, host);
    if (view.ptr(0x50) != nullptr && dt > 0.0f) report_camera_velocity(view, 0x50, dt, host);
}

// ---------------------------------------------------------------------------
// Front end
// ---------------------------------------------------------------------------

void hud_screen24_register_0067b390(Raw screen, FrontEndRegisterHost& host) {
    host.front_end_screen_register_004f71d0(screen);
}

// 0067C970: the device vector at backend+B4h (first +B8h, last +BCh). With a non-empty vector
// its first element (re-checked, report on an empty one) is removed when non-null (00A90EE0);
// then 00A917E0(2, 1, 0) and byte 00E1987C = 1.
void rebind_primary_input_0067c970(PrimaryInputHost& host) {
    Raw const backend = host.input_backend();
    Raw const vec{backend.p + 0xB4};
    auto* const first = static_cast<std::uint8_t*>(vec.ptr(4));
    auto size = [&]() {
        return static_cast<std::uint32_t>((static_cast<std::uint8_t*>(vec.ptr(8)) - first) >> 2);
    };
    if (first != nullptr && size() != 0) {
        if (first == nullptr || size() == 0) host.checked_iterator_report_00bf6713();
        void* const device = raw(vec.ptr(4)).ptr(0);
        if (device != nullptr) host.input_remove_device_00a90ee0(host.input_backend(), device);
    }
    host.input_configure_class_00a917e0(host.input_backend(), 2, 1, 0);
    host.write_global_u8(0x00E1987Cu, 1);
}

} // namespace bsp::seg27_cc13
