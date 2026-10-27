// pool_pool21_cc13: 00675C40 BSP_HudSupportManagerScreen_Update (packet cc13-pool21).
//
// Address: 00675C40..0067AF29 (5595 instructions, one body; the only RET is 0067AF29, RET 4).
// Original ABI: __thiscall(ECX = the 480h-byte support-manager screen; float dt at [EBP+8]),
// RET 4. Frame: PUSH EBP / MOV EBP,ESP / AND ESP,-8, SEH handler 00C7CDFD with the state
// dword at [ESP+1DCh] (after the three register pushes), SUB ESP,1C8h. EBX holds the
// conditional-destruction bits of the temporaries (mirrored at [ESP+14h]).
// Evidence: vtable 00CF6B44 slot 8 (+20h), the screen's constructor 006735D0, Enter 00674B80,
// Exit 00662470; the callees are the seg-27 support-manager routines named in the ledger.
// EH states: the temporaries (NativeStrings, 14h-byte support records and std::list copies)
// each get a state; the unwind funclets of 00C7CDFD only release them, so the states are
// recorded at each site and are not modelled.
// x87: dt is consumed only by FSUB on float slots (rounded through a float store) and passed
// on by FLD/FSTP; the main thread runs with 24-bit precision control, so plain float
// arithmetic reproduces it. The pulse +1E0h steps by the doubles 0.04f / 0.8f. The 4Bh bar
// (0067A0A7) keeps points * f[+1ACh] in a double slot after a 24-bit rounding and FDIVRs by
// the cap. 00946F00 and 00942130 are bound as double, so both the product (rounded to float
// here) and the FDIVR quotient can double-round (53 then 24 bits) against the x87 24-bit
// rounding; the result differs by at most the last place.
// Host overlap: the deck-page launch 0067A4C8..0067A5E6 (006BF310, 006C4780, 00656280,
// 0077C2A0) is modelled live in src/game_hosts_mission_frame.cpp and
// include/bsp/air_operations.hpp (docs/AIROPS_LAUNCH_TICK.md); nothing here is wired.
// Uncertainty: every descriptive name is a hypothesis; the hint identifiers and the field
// meanings are inferred from the strings and the callee names.

#include "bsp/pool_pool21_cc13.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::pool_pool21_cc13 {
namespace {

using cc13_big1::at;
using cc13_big1::ptr;

// The image's NativeString {length, data}.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

// A std::list object {proxy, head, size}; nodes {next, prev, value at +8}.
struct NativeList {
    void* proxy;
    void* head;
    std::uint32_t size;
};

// A checked list iterator {owner, node} (the 8-byte temporaries handed to 0059FB60).
struct CheckedIter {
    void* owner;
    void* node;
};

// The 14h-byte {site, std::list<int>, int} support record copied by 00668540.
struct SupportRecord {
    alignas(4) std::uint8_t bytes[0x14];
};

void* node_next(void* node) noexcept { return at<void*>(node, 0); }

// The game's local player record: [game + 18CCh + [game + 18ECh] * 4].
void* local_player(Pool21Host& host) {
    void* const game = host.game_00e188a8();
    return at<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)));
}

// The inlined NativeString destructor: the block goes back to the sized pool when it has data.
void release_inline(const NativeStr& str, Pool21Host& host) {
    char* const data = str.data;
    if (data != nullptr) {
        const std::uint32_t bytes = str.length + 1;
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, bytes, 1);
    }
}

// The tutorial-hint gate the update inlines ten times: A = name; B = the active hint;
// show = (B != A (00449AF0) || the active hint, copied again (C), is empty) and the active
// hint, copied a third time (D), is not blocking (00690B00). The temporaries die in the
// order D, C, B, A, through 0041DD20 or, at the SMPERMANENTSPLEVEL site, inlined.
bool hint_gate(const char* name, bool inline_release, Pool21Host& host) {
    NativeStr a{}, b{}, c{}, d{};
    bool built_c = false, built_d = false;
    bool show = false;
    host.native_string_assign_0041e870(&a, name);
    void* const b_ret = host.award_tracker_copy_active_hint_name_0068e5b0(host.award_tracker_get_singleton_004e1ca0(), &b);
    bool test_blocking = true;
    if (!host.native_string_not_equal_ci_00449af0(b_ret, &a)) {
        void* const c_ret =
            host.award_tracker_copy_active_hint_name_0068e5b0(host.award_tracker_get_singleton_004e1ca0(), &c);
        built_c = true;
        if (at<std::uint32_t>(c_ret, 0) != 0) test_blocking = false;
    }
    if (test_blocking) {
        void* const d_ret =
            host.award_tracker_copy_active_hint_name_0068e5b0(host.award_tracker_get_singleton_004e1ca0(), &d);
        built_d = true;
        show = !host.award_tracker_is_active_hint_blocking_00690b00(host.award_tracker_get_singleton_004e1ca0(), d_ret);
    }
    if (inline_release) {
        if (built_d) release_inline(d, host);
        if (built_c) release_inline(c, host);
        release_inline(b, host);
        release_inline(a, host);
    } else {
        if (built_d) host.native_string_destroy_0041dd20(&d);
        if (built_c) host.native_string_destroy_0041dd20(&c);
        host.native_string_destroy_0041dd20(&b);
        host.native_string_destroy_0041dd20(&a);
    }
    return show;
}

// The trigger that follows each gate: the "...SPECT" variant when the local player record's
// byte +19h is set, then 00690FD0(name, 0).
void hint_trigger(const char* name, const char* spectator_name, bool inline_release, Pool21Host& host) {
    NativeStr s{};
    host.native_string_assign_0041e870(&s, at<std::uint8_t>(local_player(host), 0x19) != 0 ? spectator_name : name);
    host.award_tracker_trigger_hint_00690fd0(host.award_tracker_get_singleton_004e1ca0(), &s, 0);
    if (inline_release)
        release_inline(s, host);
    else
        host.native_string_destroy_0041dd20(&s);
}

void hint_gate_and_trigger(const char* name, const char* spectator_name, bool inline_release, Pool21Host& host) {
    if (hint_gate(name, inline_release, host)) hint_trigger(name, spectator_name, inline_release, host);
}

// 00675DD3 / 0067600C: drop the selection back to the site page (+5Ch = 1).
void reset_to_site_page(void* s, Pool21Host& host) {
    host.support_refresh_sites_0066fb00(s);
    at<std::uint32_t>(s, 0x3A4) = 0;
    at<std::uint32_t>(s, 0x2A0) = 0;
    at<std::uint32_t>(s, 0x308) = 0;
    at<std::uint32_t>(s, 0x30C) = 0;
    at<std::int32_t>(s, 0x5C) = 1;
    host.support_sync_cursor_iterators_0065a2b0(s);
    host.support_clear_loadout_icons_0065abb0(s);
}

// The current record of the site list +344h through the checked iterator +350h (00658B50).
void* current_record(void* s, Pool21Host& host) {
    return host.support_map_iterator_value_00658b50(static_cast<std::uint8_t*>(s) + 0x350);
}

// 00675D1E..00675E0B: on the unit page (+5Ch == 2) with records (+34Ch), leave it when the
// current record's site (+0) is no longer the local player's (site +54h vs player +28h).
// Temporaries: record at [ESP+12Ch] (EH 0, bit 1), record at [ESP+4Ch] (bit 2).
void check_selected_site_owner(void* s, Pool21Host& host) {
    SupportRecord first{}, second{};
    std::uint32_t built = 0;
    bool reset = false;
    if (at<std::int32_t>(s, 0x5C) == 2 && at<std::uint32_t>(s, 0x34C) != 0) {
        void* const r1 = host.int_list_int_record_copy_00668540(&first, current_record(s, host));
        built = 1;
        if (at<std::uint32_t>(r1, 0) != 0) {
            void* const r2 = host.int_list_int_record_copy_00668540(&second, current_record(s, host));
            built = 3;
            void* const player = local_player(host);
            reset = at<std::int32_t>(at<void*>(r2, 0), 0x54) != at<std::int32_t>(player, 0x28);
        }
    }
    if (built & 2) host.list_tidy_005a0b80(second.bytes + 4);
    if (built & 1) host.list_tidy_005a0b80(first.bytes + 4);
    if (reset) reset_to_site_page(s, host);
}

// One "InferiorFailure" query: object vtable +198h(name, 0, &out) with the name built by
// 0041E870 and released by 0041DD20.
void query_inferior_failure(void* unit, std::uint8_t* out, NativeStr& name, Pool21Host& host) {
    host.native_string_assign_0041e870(&name, "InferiorFailure");
    host.object_vtable198(at<void*>(unit, 0x3C), &name, 0, out);
    host.native_string_destroy_0041dd20(&name);
}

// 00675E0D..00676071: on the unit page, scan the current record's unit list; a unit whose
// +3Ch object reports InferiorFailure for flag 46h (or 45h) drops the page back to the
// sites when list +37Ch holds an entry of kind 6 (or 0Fh), then rebuilds the entries.
// Temporaries: record [ESP+4Ch] (EH 1), list copy [ESP+60h] (EH 3), names [ESP+14h] (EH 4)
// and [ESP+3Ch] (EH 5).
void check_inferior_failure(void* s, Pool21Host& host) {
    if (at<std::int32_t>(s, 0x5C) != 2 || at<std::uint32_t>(s, 0x34C) == 0) return;
    SupportRecord record{};
    NativeList units{};
    host.int_list_int_record_copy_00668540(&record, current_record(s, host));
    host.list_copy_005aaaf0(&units, record.bytes + 4);
    host.list_tidy_005a0b80(record.bytes + 4);
    bool kind_0f = false;  // [ESP+13h]
    bool kind_06 = false;  // [ESP+3Bh]
    if (units.size != 0) {
        for (void* node = node_next(units.head); node != units.head;) {
            void* const unit = at<void*>(node, 8);
            if (at<void*>(unit, 0x3C) != nullptr) {
                std::uint8_t failed = 0;  // [ESP+3Ah]
                NativeStr name{};
                if (host.unit_has_flag_vtable5c(at<void*>(unit, 0x3C), 0x46)) {
                    query_inferior_failure(unit, &failed, name, host);
                    if (failed != 0) kind_06 = true;
                } else if (host.unit_has_flag_vtable5c(at<void*>(unit, 0x3C), 0x45)) {
                    query_inferior_failure(unit, &failed, name, host);
                    if (failed != 0) kind_0f = true;
                }
            }
            if (node == units.head) host.crt_invalid_parameter_noinfo_00bf6713();
            node = node_next(node);
        }
        if (kind_0f || kind_06) {
            // List +37Ch {proxy, head +380h, size +384h}. 00675F9C compares the iterator's
            // owner with itself (always equal), so its 00BF6713 at 00675FA4 is dead.
            void* node = node_next(ptr(s, 0x380));
            while (node != ptr(s, 0x380)) {
                bool hit = false;
                if (kind_06) {
                    if (node == ptr(s, 0x380)) host.crt_invalid_parameter_noinfo_00bf6713();
                    hit = host.object_vtable18(at<void*>(node, 8), 6);
                }
                if (!hit && kind_0f) {
                    if (node == ptr(s, 0x380)) host.crt_invalid_parameter_noinfo_00bf6713();
                    hit = host.object_vtable18(at<void*>(node, 8), 0xF);
                }
                if (hit) {
                    reset_to_site_page(s, host);
                    host.support_build_entries_0066a130(s);
                    at<std::uint32_t>(s, 0x310) = 0;
                    at<std::uint32_t>(s, 0x324) = 0;
                    at<std::uint32_t>(s, 0x314) = 0;
                    break;
                }
                if (node == ptr(s, 0x380)) host.crt_invalid_parameter_noinfo_00bf6713();
                node = node_next(node);
            }
        }
    }
    host.list_tidy_005a0b80(&units);
}

// The arrow pair shown by 00652AA0 / 00652B00 or hidden through widget vtable +34h(0).
void hide_pair(void* s, std::size_t first, Pool21Host& host) {
    host.widget_vtable34(ptr(s, first), 0);
    host.widget_vtable34(ptr(s, first + 4), 0);
}

// 00676071..00676254: scrolls, fades, list rebuild and the arrow pairs.
void step_scrolls_and_arrows(void* s, float dt, Pool21Host& host) {
    host.support_step_catalog_ring_00668d40(s, dt);
    host.support_step_slot_ring_00661b50(s, dt);
    const float row_timer = at<float>(s, 0x2E0);
    if (row_timer != 0.0f) {  // UCOMISS / LAHF / TEST AH,44h: NaN counts as nonzero
        const float left = row_timer - dt;
        at<float>(s, 0x2E0) = left;
        if (left < 0.0f) at<float>(s, 0x2E0) = 0.0f;  // FLDZ / FCOMIP / JBE
        host.support_layout_scroll_rows_006619e0(s);
    }
    host.support_update_name_label_fade_0065bd90(s, dt);
    if (host.game_effective_mode_004bca50(host.game_00e188a8()) != 8) host.support_rebuild_lists_0066a8a0(s);
    if (at<std::uint8_t>(s, 0x2DC) == 0 && at<std::uint8_t>(s, 0x2C1) == 0 && at<std::int32_t>(s, 0x298) > 4 &&
        at<std::uint32_t>(s, 0x34C) != 0)
        host.support_show_arrows_f8_00652aa0(s);
    else
        hide_pair(s, 0xF8, host);
    if (at<std::int32_t>(s, 0x5C) == 1) {
        std::int32_t count = at<std::int32_t>(s, 0x34C);
        if (count == 0) count = at<std::int32_t>(s, 0x1F4);
        if (at<std::uint8_t>(s, 0x2DD) == 0 && at<std::uint8_t>(s, 0x2C0) == 0 && count > 4)
            host.support_show_arrows_100_00652b00(s);
        else
            hide_pair(s, 0x100, host);
    }
    if (at<std::int32_t>(s, 0x5C) == 2) {
        std::int32_t count = at<std::int32_t>(s, 0x2EC);
        if (count == 0) count = at<std::int32_t>(s, 0x384);
        if (at<std::uint8_t>(s, 0x2DD) == 0 && at<std::uint8_t>(s, 0x2C0) == 0 && count > 4)
            host.support_show_arrows_100_00652b00(s);
        else
            hide_pair(s, 0x100, host);
    }
    if (at<std::uint32_t>(s, 0x2C8) != 0) {
        if (at<std::uint8_t>(s, 0x2C1) == 0) {
            host.support_begin_scroll_back_00661240(s);
            at<std::int32_t>(s, 0x2C8) += -1;
        }
        at<std::uint8_t>(s, 0x154) = 0;
        host.support_step_slot_ring_00661b50(s, dt);
        host.widget_vtable34(ptr(s, 0xDC), 0);
    }
    host.support_prune_large_slots_006605c0(s);
    host.support_update_map_icons_0065bf00(s);
    if (at<std::uint8_t>(s, 0x2B0) != 0 && at<std::uint8_t>(s, 0x2C0) == 0)
        host.support_update_slot_highlights_0065fbf0(s, dt);
}

// 00676254..006770D8: the tutorial hints, skipped while "SM1STUSE" is the active hint.
// The "SM1STUSE" comparison is the inlined NativeString operator!= (empty checks, then
// _stricmp); both strings are released inline (EH 6).
void update_hints(void* s, Pool21Host& host) {
    NativeStr first_use{};
    host.native_string_resize_0041dd40(&first_use, 8, 1);
    char* const data = first_use.data;  // EDI
    if (data != nullptr) host.crt_memcpy_00bf7680(data, "SM1STUSE", first_use.length + 1);
    NativeStr active{};
    void* const active_ret =
        host.award_tracker_copy_active_hint_name_0068e5b0(host.award_tracker_get_singleton_004e1ca0(), &active);
    std::int32_t order;
    if (at<std::uint32_t>(active_ret, 0) == 0)
        order = first_use.length != 0 ? -1 : 0;  // NEG / SBB
    else if (first_use.length == 0)
        order = 1;
    else
        order = host.crt_stricmp_00bf7fbf(at<const char*>(active_ret, 4), data);
    const bool differs = order != 0;
    release_inline(active, host);
    if (data != nullptr) {
        const std::uint32_t bytes = first_use.length + 1;
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, bytes, 1);
    }
    if (!differs) return;

    const std::int32_t page = at<std::int32_t>(s, 0x5C);
    if (page == 1) {
        // 00676332: EH 7..0Ch; this site releases its strings inline.
        hint_gate_and_trigger("SMPERMANENTSPLEVEL", "SMPERMANENTSPLEVELSPECT", true, host);
    } else if (page == 0) {
        // 0067659D: EH 0Dh..18h.
        if (at<std::uint8_t>(s, 0x70) == 0)
            hint_gate_and_trigger("SMPERMANENTUNITLEVEL", "SMPERMANENTUNITLEVELSPECT", false, host);
        else
            hint_gate_and_trigger("SMPERMANENTUNITLEVELPLANE", "SMPERMANENTUNITLEVELPLANESPECT", false, host);
    } else if (page == 2) {
        // 00676943: EH 19h..2Fh.
        bool selected_branch = at<std::uint32_t>(s, 0x34C) != 0;
        if (!selected_branch && at<std::uint32_t>(s, 0x1F4) != 0) {
            void** const slot = host.support_unit_iterator_value_00658a60(static_cast<std::uint8_t*>(s) + 0x300);
            selected_branch = host.unit_has_flag_vtable5c(*slot, 0x46);
        }
        bool ship;
        if (selected_branch) {
            // 00676D17: object +3A4h of kind 17h or 6.
            ship = ptr(s, 0x3A4) != nullptr &&
                   (host.object_vtable18(ptr(s, 0x3A4), 0x17) || host.object_vtable18(ptr(s, 0x3A4), 6));
        } else {
            // 00676980: object +2A0h of kind 17h.
            ship = ptr(s, 0x2A0) != nullptr && host.object_vtable18(ptr(s, 0x2A0), 0x17);
        }
        if (ship)
            hint_gate_and_trigger("SMPERMANENTSHIP", "SMPERMANENTSHIPSPECT", false, host);
        else
            hint_gate_and_trigger("SMPERMANENTPLANE", "SMPERMANENTPLANESPECT", false, host);
    }
}

// A checked iterator {list, front} over a std::list copy, dereferenced by 0059FB60.
void* list_front_value(NativeList& list, Pool21Host& host) {
    CheckedIter it{&list, node_next(list.head)};
    return *host.list_iterator_value_0059fb60(&it);
}

// 006770D8..006773D0: on the unit page, when the front unit of list +35Ch (head +360h) is not
// the current record's front unit (word +174h), retitle text +170h from the record and fall
// back to the site page with iterator +350h at the start of list +344h.
// Temporaries: record [ESP+4Ch] (EH 31h), lists [ESP+70h] (32h) and [ESP+60h] (33h..35h).
void check_selected_unit_changed(void* s, Pool21Host& host) {
    if (at<std::int32_t>(s, 0x5C) != 2 || at<std::uint32_t>(s, 0x34C) == 0) return;
    void* const head = ptr(s, 0x360);
    void* const front = node_next(head);
    if (front == head) host.crt_invalid_parameter_noinfo_00bf6713();
    const std::uint16_t front_id = at<std::uint16_t>(at<void*>(front, 8), 0x174);
    // The inlined checked dereference of iterator +350h and of its record's list front.
    std::uint8_t* const iter = static_cast<std::uint8_t*>(s) + 0x350;
    if (at<void*>(iter, 0) == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
    if (at<void*>(iter, 4) == at<void*>(at<void*>(iter, 0), 4)) host.crt_invalid_parameter_noinfo_00bf6713();
    void* const rec_node = at<void*>(iter, 4);
    void* const rec_front = node_next(at<void*>(rec_node, 0x10));
    if (rec_front == at<void*>(rec_node, 0x10)) host.crt_invalid_parameter_noinfo_00bf6713();
    if (front_id == at<std::uint16_t>(at<void*>(rec_front, 8), 0x174)) return;

    SupportRecord record{};
    void* const site = at<void*>(host.int_list_int_record_copy_00668540(&record, current_record(s, host)), 0);
    host.list_tidy_005a0b80(record.bytes + 4);
    void* const value = current_record(s, host);
    if (site != nullptr) {
        void* const r = host.int_list_int_record_copy_00668540(&record, value);
        const char* const name = host.object_vtable14_name(at<void*>(r, 0));  // the PUSH 1 is 00ABBE50's
        host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
        host.list_tidy_005a0b80(record.bytes + 4);
    } else {
        NativeList outer{}, inner{};
        bool built_inner = false;
        bool flagged = false;  // [ESP+13h]
        host.list_copy_005aaaf0(&outer, static_cast<std::uint8_t*>(value) + 4);
        if (at<void*>(list_front_value(outer, host), 0x3C) != nullptr) {
            host.list_copy_005aaaf0(&inner, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
            built_inner = true;
            flagged = host.unit_has_flag_vtable5c(at<void*>(list_front_value(inner, host), 0x3C), 9);
        }
        if (built_inner) host.list_tidy_005a0b80(&inner);
        host.list_tidy_005a0b80(&outer);
        NativeList units{};
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        void* named = nullptr;
        bool use_name = true;
        if (flagged) {
            named = at<void*>(list_front_value(units, host), 0x3C);  // 006772BB
        } else {
            const bool counted = at<std::uint32_t>(list_front_value(units, host), 0x15C) != 0;  // SETA BL
            host.list_tidy_005a0b80(&units);
            use_name = counted;
            if (counted) {
                host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
                named = list_front_value(units, host);  // 00677312
            }
        }
        if (use_name) {
            const char* const name = host.object_vtable14_name(named);  // 00677349
            host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
            host.list_tidy_005a0b80(&units);
        } else {
            host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), "globals.unitclass_spawnpoint", 1);
        }
    }
    host.support_layout_name_label_0065bbc0(s);
    host.support_clear_loadout_icons_0065abb0(s);
    at<std::int32_t>(s, 0x5C) = 1;
    at<std::uint8_t>(s, 0x288) = 1;
    at<float>(s, 0x28C) = 1.0f;  // [00D7A24C]
    at<void*>(iter, 0) = static_cast<std::uint8_t*>(s) + 0x344;
    at<void*>(iter, 4) = node_next(ptr(s, 0x348));
    at<std::uint32_t>(s, 0x308) = 0;
    at<std::uint32_t>(s, 0x30C) = 0;
}

// The inlined checked dereference of the iterator at screen+off {owner, node}: 00BF6713 for a
// null owner and for the end node (owner +4h).
void* checked_node(void* s, std::size_t off, Pool21Host& host) {
    if (ptr(s, off) == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
    if (ptr(s, off + 4) == at<void*>(ptr(s, off), 4)) host.crt_invalid_parameter_noinfo_00bf6713();
    return ptr(s, off + 4);
}

// The selected unit: node +8h of the checked iterator +300h {owner, node +304h}.
void* checked_selected_unit(void* s, Pool21Host& host) { return at<void*>(checked_node(s, 0x300, host), 8); }

// The inlined copy of the current record (node +8h of iterator +350h) into a stack record:
// site first, then the list (005AAAF0), then the trailing dword.
void* inline_copy_current_record(void* s, SupportRecord& out, Pool21Host& host) {
    std::uint8_t* const value = static_cast<std::uint8_t*>(checked_node(s, 0x350, host)) + 8;
    void* const site = at<void*>(value, 0);
    at<void*>(out.bytes, 0) = site;
    host.list_copy_005aaaf0(out.bytes + 4, value + 4);
    at<std::uint32_t>(out.bytes, 0x10) = at<std::uint32_t>(value, 0x10);
    return site;
}

// 006773D0..00677482: copy the current record into +358h, refresh the info panel and step the
// pulse +1E0h (0.04 a frame through the double 00CE4D68, direction byte +1DDh, reversing at
// zero and at 0.8 (00CE3D40); the zero turn also toggles +1DCh).
void sync_record_and_pulse(void* s, Pool21Host& host) {
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        void* const node = checked_node(s, 0x350, host);
        host.int_list_int_record_assign_006697a0(static_cast<std::uint8_t*>(s) + 0x358,
                                                 static_cast<std::uint8_t*>(node) + 8);
    }
    host.support_refresh_info_panel_0066b6c0(s);
    // FLD float / FSUB|FADD double / FSTP float at 24-bit precision: one float rounding.
    float pulse = at<float>(s, 0x1E0);
    pulse = at<std::uint8_t>(s, 0x1DD) != 0 ? pulse - 0.04f : pulse + 0.04f;
    at<float>(s, 0x1E0) = pulse;
    if (pulse <= 0.0f) {  // COMISS 0, pulse: JC skips when 0 < pulse or unordered
        at<std::uint8_t>(s, 0x1DD) = 0;
        at<std::uint8_t>(s, 0x1DC) = at<std::uint8_t>(s, 0x1DC) == 0 ? 1 : 0;
    } else if (pulse >= 0.8f) {  // FCOMIP pulse, 0.8: JC skips below (and NaN)
        at<std::uint8_t>(s, 0x1DD) = 1;
    }
}

// 0067748F..0067758D: with a unit list (+1F4h), a null or retired (+5Dh) selected unit drops
// the page to the sites, rebuilding the unit markers and retitling +170h.
void check_selected_unit_retired(void* s, Pool21Host& host) {
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool rebuild = checked_selected_unit(s, host) == nullptr;
    if (!rebuild) rebuild = at<std::uint8_t>(checked_selected_unit(s, host), 0x5D) != 0;
    if (!rebuild) return;
    host.support_rebuild_00668770(s);
    at<std::int32_t>(s, 0x5C) = 1;
    at<std::uint32_t>(s, 0x318) = 0;
    host.support_sync_cursor_iterators_0065a2b0(s);
    const char* const name = host.object_vtable14_name(checked_selected_unit(s, host));
    host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
    host.support_layout_name_label_0065bbc0(s);
    host.support_rebuild_00670020(s);
    at<std::uint32_t>(s, 0x324) = 0;
    at<std::uint32_t>(s, 0x32C) = 0;
    at<std::uint32_t>(s, 0x334) = 0;
    at<std::uint8_t>(s, 0x3A8) = 0;
}

// 0067758D..00677707: on the site or unit page (+5Ch 0 or 2) with a unit list, place every
// unit marker of map +3FCh {proxy, head +400h; node key +0Ch unit, value +10h widget}: frame
// 0 for the selected unit, 1 otherwise, shown, at the unit's map position (+FCh..+104h, the
// pose refreshed first unless +C8h is set).
void place_unit_markers(void* s, Pool21Host& host) {
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    const std::int32_t page = at<std::int32_t>(s, 0x5C);
    if (page != 2 && page != 0) return;
    void* const owner_expected = static_cast<std::uint8_t*>(s) + 0x3FC;
    CheckedIter it{owner_expected, at<void*>(ptr(s, 0x400), 0)};  // [ESP+14h], [ESP+18h]
    for (;;) {
        void* const end = ptr(s, 0x400);
        if (it.owner == nullptr || it.owner != owner_expected) host.crt_invalid_parameter_noinfo_00bf6713();
        if (it.node == end) break;
        if (it.owner == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
        if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        void* const selected_node = checked_node(s, 0x300, host);
        const bool selected = at<void*>(it.node, 0xC) == at<void*>(selected_node, 8);
        if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        host.widget_vtable88(at<void*>(it.node, 0x10), selected ? 0 : 1, 0, 1.0f);
        if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        host.widget_vtable34(at<void*>(it.node, 0x10), 1);
        if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        void* const unit = at<void*>(it.node, 0xC);
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        float map[2];  // [ESP+1Ch]
        host.support_world_to_map_00658580(s, map, at<float>(unit, 0xFC), at<float>(unit, 0x100),
                                           at<float>(unit, 0x104));
        if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
        host.widget_set_local_xy_00aa7d00(at<void*>(it.node, 0x10), map[0], map[1]);
        host.tree_iterator_increment_00654650(&it);
    }
}

// 00677707..0067788A: on records, a retired current site (+5Dh) drops the page to the sites
// and retitles +170h with the site's name. The record copies live at [ESP+4Ch] (EH 37h).
void check_current_site_retired(void* s, Pool21Host& host) {
    if (at<std::uint32_t>(s, 0x34C) == 0) return;
    SupportRecord record{};
    void* const site = inline_copy_current_record(s, record, host);
    host.list_tidy_005a0b80(record.bytes + 4);
    if (site == nullptr) return;
    inline_copy_current_record(s, record, host);
    const std::uint8_t retired = at<std::uint8_t>(at<void*>(record.bytes, 0), 0x5D);
    host.list_tidy_005a0b80(record.bytes + 4);
    if (retired == 0) return;
    host.support_refresh_sites_0066fb00(s);
    at<std::int32_t>(s, 0x5C) = 1;
    at<std::uint32_t>(s, 0x318) = 0;
    host.support_sync_cursor_iterators_0065a2b0(s);
    inline_copy_current_record(s, record, host);
    const char* const name = host.object_vtable14_name(at<void*>(record.bytes, 0));
    host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
    host.list_tidy_005a0b80(record.bytes + 4);
    host.support_layout_name_label_0065bbc0(s);
    at<std::uint32_t>(s, 0x324) = 0;
    at<std::uint32_t>(s, 0x328) = 0;
    at<std::uint32_t>(s, 0x32C) = 0;
    at<std::uint32_t>(s, 0x334) = 0;
    at<std::uint8_t>(s, 0x3A8) = 0;
}

// 0067788A..00677917: the health bar +180h ({health, 1.0} through vtable +48h), the view
// refresh and close fade; false (jump to 0067AEFE) when there is neither a unit list nor a
// record, after hiding the four widgets +CCh..+D8h.
bool step_health_and_fade(void* s, float dt, Pool21Host& host) {
    float bar[2];  // [ESP+1Ch]
    bar[0] = host.support_selection_health_00668570(s);
    bar[1] = 1.0f;  // [00D7A24C]
    host.widget_vtable48(ptr(s, 0x180), bar);
    if (at<std::int32_t>(s, 0x5C) == 1 && at<std::uint8_t>(s, 0x288) == 0) host.support_rebuild_00670020(s);
    if (at<std::uint8_t>(s, 0x288) != 0) host.support_step_close_fade_006723f0(s, dt);
    if (at<std::uint32_t>(s, 0x1F4) == 0 && at<std::uint32_t>(s, 0x34C) == 0) {
        for (std::size_t i = 0; i < 4; ++i) host.widget_vtable34(ptr(s, 0xCC + 4 * i), 0);
        return false;
    }
    return true;
}

// 00677917..00677B97: action 118h (back to the deck page, +5Ch = 0, the previous page in
// +60h) unless the screen is scrolling (+2C1h, +2DCh) or the effective mode is 4..7; the
// points readout +1A8h then shows "ingame.sm_na" (unit list in mode 8) or "ingame.sm_cp|.: "
// with the player's points, and bar +188h is resized to +1ACh * points / cap.
void handle_back_to_deck(void* s, void* item, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x118)) return;
    const std::int32_t mode = host.game_effective_mode_004bca50(host.game_00e188a8());
    if (at<std::uint32_t>(s, 0x298) == 0) return;
    const std::int32_t page = at<std::int32_t>(s, 0x5C);
    if (page == 0 || at<std::uint8_t>(s, 0x2C1) != 0 || at<std::uint8_t>(s, 0x2DC) != 0) return;
    if (mode == 4 || mode == 5 || mode == 6 || mode == 7) return;
    at<std::int32_t>(s, 0x60) = page;
    at<std::int32_t>(s, 0x5C) = 0;
    host.widget_vtable34(ptr(s, 0x12C), 0);
    host.widget_vtable34(ptr(s, 0x130), 0);
    host.widget_vtable34(ptr(s, 0x134), 0);
    host.support_select_deck_slot_00660190(s, item);
    host.support_play_left_right_sound_0065f800(s);
    host.support_clear_loadout_icons_0065abb0(s);
    if (at<std::uint32_t>(s, 0x1F4) != 0 && host.game_effective_mode_004bca50(host.game_00e188a8()) == 8) {
        NativeStr na{};  // [ESP+1Ch], EH 38h
        void* const text = host.native_string_assign_0041e870(&na, "ingame.sm_na");
        host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), text, 1);
        host.native_string_destroy_0041dd20(&na);
        return;
    }
    // Five fresh 004BCA50 calls: modes 0, 1, 2, 3 and 8.
    bool shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == 0;
    if (!shown) shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == 1;
    if (!shown) shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == 2;
    if (!shown) shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == 3;
    if (!shown) shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == 8;
    if (!shown) return;
    const float points = static_cast<float>(
        host.player_support_points_00946f00(at<std::uint32_t>(host.game_00e188a8(), 0x18EC)));  // [ESP+14h]
    const float cap = static_cast<float>(
        host.team_support_points_cap_00942130(at<std::uint32_t>(host.game_00e188a8(), 0x18EC)));  // [ESP+2Ch]
    const float* const size = host.gui_widget_get_size_00aa6740(ptr(s, 0x188));
    float bar[2];  // [ESP+1Ch]
    bar[0] = at<float>(s, 0x1AC) * points / cap;  // FMUL / FDIV at 24-bit precision
    bar[1] = size[1];
    host.widget_vtable58_set_size(ptr(s, 0x188), bar);
    host.widget_vtable34(ptr(s, 0x180), 0);
    host.widget_vtable34(ptr(s, 0x184), 0);
    host.widget_vtable34(ptr(s, 0x188), 1);
    const std::int32_t whole =
        host.crt_ftol2_00bf7420(host.player_support_points_00946f00(at<std::uint32_t>(host.game_00e188a8(), 0x18EC)));
    NativeStr number{}, label{}, joined{};  // [ESP+3Ch] (EH 39h), [ESP+44h] (3Ah), [ESP+1Ch] (3Bh)
    void* const number_ret = host.native_string_from_int_004260b0(&number, whole);
    void* const label_ret = host.native_string_assign_0041e870(&label, "ingame.sm_cp|.: ");
    void* const joined_ret = host.native_string_concat_004261a0(label_ret, &joined, number_ret);
    host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
    host.native_string_destroy_0041dd20(&joined);
    host.native_string_destroy_0041dd20(&label);
    host.native_string_destroy_0041dd20(&number);
}

// 00677B97..00677C2B: action 119h hides the cursor widget +DCh[+324h]; on the deck page it
// returns to the remembered page (+60h: 1 sites through 00671D60, 2 units through 0066D020
// with +71h set) unless scrolling, then plays the left/right sound.
void handle_return_from_deck(void* s, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x119)) return;
    host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
    if (at<std::int32_t>(s, 0x5C) == 0) {
        if (at<std::int32_t>(s, 0x60) == 1 && at<std::uint8_t>(s, 0x2C1) == 0 && at<std::uint8_t>(s, 0x2DC) == 0) {
            host.support_show_selection_info_00671d60(s);
            at<std::int32_t>(s, 0x5C) = 1;
            at<std::int32_t>(s, 0x60) = 0;
        } else if (at<std::int32_t>(s, 0x60) == 2 &&  // 00677BF3 re-tests the cached page (0)
                   at<std::uint8_t>(s, 0x2C1) == 0 && at<std::uint8_t>(s, 0x2DC) == 0) {
            at<std::uint8_t>(s, 0x71) = 1;
            host.support_refresh_selection_0066d020(s);
            at<std::int32_t>(s, 0x5C) = 2;
            at<std::int32_t>(s, 0x60) = 0;
        }
    }
    host.support_play_left_right_sound_0065f800(s);
}

// The loadout icon +158h for an entry with a chosen value +334h: state 3 gets the device
// class's texture ([[[dev+74h]+34h]+7Ch]) and UV ([[dev+74h]+34h]+90h..9Ch) from 009552E0 /
// 00443490 (resolved twice), then frames 0 and 3; with no value only frame 0.
void show_loadout_icon(void* s, void* (*entry_of)(void*), Pool21Host& host) {
    const std::uint32_t value = at<std::uint32_t>(s, 0x334);
    if (value != 0) {
        void* rec = host.object_class_device_set_009552e0(entry_of(s), value);
        void* dev = host.device_class_resolve_00443490(at<std::uint32_t>(at<void*>(at<void*>(rec, 4), 8), 4), 0);
        void* const texture = at<void*>(at<void*>(at<void*>(dev, 0x74), 0x34), 0x7C);
        rec = host.object_class_device_set_009552e0(entry_of(s), at<std::uint32_t>(s, 0x334));
        dev = host.device_class_resolve_00443490(at<std::uint32_t>(at<void*>(at<void*>(rec, 4), 8), 4), 0);
        void* const info = at<void*>(at<void*>(dev, 0x74), 0x34);
        std::uint32_t uv[4];  // [ESP+1Ch]
        for (std::size_t i = 0; i < 4; ++i) uv[i] = at<std::uint32_t>(info, 0x90 + 4 * i);
        host.gui_set_state_texture_uv_00ab2690(ptr(s, 0x158), 3, texture, uv);
        host.widget_vtable88(ptr(s, 0x158), 0, 0, 1.0f);
        host.widget_vtable88(ptr(s, 0x158), 3, 0, 1.0f);
    } else {
        host.widget_vtable88(ptr(s, 0x158), 0, 0, 1.0f);
    }
}

void* entry_3a4(void* s) { return ptr(s, 0x3A4); }
void* entry_2a0(void* s) { return ptr(s, 0x2A0); }

// The pair +F0h / +F4h (vtable +34h) and the info panel refresh closing each step.
void show_f0_f4(void* s, std::uint8_t f0, std::uint8_t f4, Pool21Host& host) {
    host.widget_vtable34(ptr(s, 0xF0), f0);
    host.widget_vtable34(ptr(s, 0xF4), f4);
    host.support_refresh_info_panel_0066b6c0(s);
}

// Entry +3A4h that is neither 0Fh-only nor 17h: a kind-6 entry whose class name (+54h)
// equals "globals.unitclass_kamikazeboat" (00435C40) shows loadout frame 2; then the pair
// +F0h/+F4h and the info panel. The two names are built by 0041E870 and released through
// 0041DD20 under bits 8h/10h (100h/200h at 00678F6C).
void show_other_catalog_entry(void* s, Pool21Host& host) {
    bool kamikaze = false;  // [ESP+13h] / BL
    if (host.object_vtable18(ptr(s, 0x3A4), 6)) {
        NativeStr wanted{}, name{};
        void* const wanted_ret = host.native_string_assign_0041e870(&wanted, "globals.unitclass_kamikazeboat");
        void* const name_ret = host.native_string_assign_0041e870(&name, at<const char*>(ptr(s, 0x3A4), 0x54));
        kamikaze = host.native_string_equals_ci_00435c40(name_ret, wanted_ret);
        host.native_string_destroy_0041dd20(&name);
        host.native_string_destroy_0041dd20(&wanted);
    }
    host.support_clear_loadout_icons_0065abb0(s);
    if (kamikaze) {
        host.widget_vtable88(ptr(s, 0x158), 2, 0, 1.0f);
        show_f0_f4(s, 0, 1, host);
    } else {
        show_f0_f4(s, 1, 0, host);
    }
}

// 00677C60..0067821A: action 11Ah on the unit page (+5Ch == 2): step the catalog cursor
// back. Records (+34Ch) or a 46h-flagged selected unit step list +37Ch (count +384h, cursor
// +310h, window +314h) and present entry +3A4h; a 9h/45h-flagged selected unit then steps
// list +2E4h (count +2ECh) and presents entry +2A0h.
void handle_prev_on_unit_page(void* s, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C0) != 0) return;
    at<std::uint32_t>(s, 0x334) = 0;
    bool catalog = at<std::uint32_t>(s, 0x34C) != 0;
    if (!catalog) {
        if (at<std::uint32_t>(s, 0x1F4) == 0) return;
        void** const slot = host.support_unit_iterator_value_00658a60(static_cast<std::uint8_t*>(s) + 0x300);
        catalog = host.unit_has_flag_vtable5c(*slot, 0x46);
    }
    if (catalog) {
        if (at<std::int32_t>(s, 0x314) == 0 && at<std::uint32_t>(s, 0x384) <= 4) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
        host.support_play_unit_to_unit_sound_0065f6b0(s);
        const std::uint32_t count = at<std::uint32_t>(s, 0x384);
        at<std::uint32_t>(s, 0x310) = (count + at<std::uint32_t>(s, 0x310) - 1) % count;
        const std::int32_t window = at<std::int32_t>(s, 0x314);
        if (window > 0) {
            at<std::int32_t>(s, 0x314) = window - 1;
        } else {
            at<std::uint8_t>(s, 0x154) = 1;
            host.support_begin_scroll_forward_00660b10(s);
        }
        host.support_update_payload_strip_0065a740(s);
        at<void*>(s, 0x3A4) = host.support_catalog_entry_at_index_0065ab40(s);
        host.widget_vtable34(ptr(s, 0xCC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x314))), 1);
        if (host.object_vtable18(ptr(s, 0x3A4), 0xF) && !host.object_vtable18(ptr(s, 0x3A4), 0x17)) {
            host.support_select_value_0065c9a0(s, ptr(s, 0x3A4));
            show_loadout_icon(s, entry_3a4, host);
            host.support_build_loadout_ring_00665f30(s);
            host.support_highlight_loadout_00662020(s);
            at<std::uint8_t>(s, 0x2B0) = 1;
            show_f0_f4(s, 0, 1, host);
        } else if (host.object_vtable18(ptr(s, 0x3A4), 0x17)) {
            host.support_clear_loadout_icons_0065abb0(s);
            host.widget_vtable88(ptr(s, 0x158), 1, 0, 1.0f);
            show_f0_f4(s, 0, 1, host);
        } else {
            show_other_catalog_entry(s, host);  // EH 49h
        }
    }
    // 00677F94
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    void** slot = host.support_unit_iterator_value_00658a60(static_cast<std::uint8_t*>(s) + 0x300);
    if (!host.unit_has_flag_vtable5c(*slot, 9)) {
        slot = host.support_unit_iterator_value_00658a60(static_cast<std::uint8_t*>(s) + 0x300);
        if (!host.unit_has_flag_vtable5c(*slot, 0x45)) return;
    }
    const std::int32_t window = at<std::int32_t>(s, 0x314);
    if (window == 0 && at<std::uint32_t>(s, 0x2EC) <= 4) {
        host.support_play_unavailable_sound_0065f170(s);
        return;
    }
    const std::uint32_t count = at<std::uint32_t>(s, 0x2EC);
    at<std::uint32_t>(s, 0x310) = (count + at<std::uint32_t>(s, 0x310) - 1) % count;
    if (window > 0) {
        at<std::int32_t>(s, 0x314) = window - 1;
    } else {
        at<std::uint8_t>(s, 0x154) = 1;
        host.support_begin_scroll_forward_00660b10(s);
    }
    host.support_update_payload_strip_0065a740(s);
    void* const entry = host.support_catalog_entry_2e4_at_index_0065ae90(s);
    at<void*>(s, 0x2A0) = entry;
    if (host.object_vtable18(entry, 0xF) && !host.object_vtable18(ptr(s, 0x2A0), 0x17)) {
        host.support_select_weapon_value_00669170(s);
        show_loadout_icon(s, entry_2a0, host);
        host.support_build_hangar_loadout_ring_00666dc0(s);
        host.support_highlight_hangar_loadout_00662210(s);
        at<std::uint8_t>(s, 0x2B0) = 1;  // stored between the +F0h load and its call
        show_f0_f4(s, 0, 1, host);
    } else if (host.object_vtable18(ptr(s, 0x2A0), 0x17)) {
        host.support_clear_loadout_icons_0065abb0(s);
        host.widget_vtable88(ptr(s, 0x158), 1, 0, 1.0f);
        show_f0_f4(s, 0, 1, host);
    } else {
        host.support_clear_loadout_icons_0065abb0(s);
        show_f0_f4(s, 1, 0, host);
    }
}

// The selected unit through 00658A60 on iterator +300h.
void* selected_unit(void* s, Pool21Host& host) {
    return *host.support_unit_iterator_value_00658a60(static_cast<std::uint8_t*>(s) + 0x300);
}

// The selected unit's air-ops block: +72Ch for a 45h-flagged unit, else +1188h; null for a
// null unit (each read through a fresh 00658A60).
void* selected_air_ops_block(void* s, Pool21Host& host) {
    const bool flag45 = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    std::uint8_t* const unit = static_cast<std::uint8_t*>(selected_unit(s, host));
    if (unit == nullptr) return nullptr;
    return unit + (flag45 ? 0x72C : 0x1188);
}

// The points readout +1A8h: "ingame.sm_cp|.: " + the player's points truncated (00BF7420).
void show_points_label(void* s, Pool21Host& host) {
    const std::int32_t whole =
        host.crt_ftol2_00bf7420(host.player_support_points_00946f00(at<std::uint32_t>(host.game_00e188a8(), 0x18EC)));
    NativeStr number{}, label{}, joined{};
    void* const number_ret = host.native_string_from_int_004260b0(&number, whole);
    void* const label_ret = host.native_string_assign_0041e870(&label, "ingame.sm_cp|.: ");
    void* const joined_ret = host.native_string_concat_004261a0(label_ret, &joined, number_ret);
    host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
    host.native_string_destroy_0041dd20(&joined);
    host.native_string_destroy_0041dd20(&label);
    host.native_string_destroy_0041dd20(&number);
}

// The selected unit's kind icon +1CCh: frame 0 for flag 46h, 1 for 9h, 2 for 45h.
void show_unit_kind_icon(void* s, Pool21Host& host) {
    std::int32_t frame;
    if (host.unit_has_flag_vtable5c(selected_unit(s, host), 0x46))
        frame = 0;
    else if (host.unit_has_flag_vtable5c(selected_unit(s, host), 9))
        frame = 1;
    else if (host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45))
        frame = 2;
    else
        return;
    host.widget_vtable88(ptr(s, 0x1CC), static_cast<std::uint32_t>(frame), 0, 1.0f);
}

// The slots readout +1A8h in mode 8: "ingame.sm_slots|.: " + the idle slots, counted by the
// shipyard (008434D0) for a unit that is neither 9h nor 45h, else by the air-ops block
// (006BDC30). The air-ops count is joined through 0043C130 after 11Ah (00678494) and through
// 0041E870 + 004263B0 after 11Bh (00679556).
void show_slots_label(void* s, bool after_next, Pool21Host& host) {
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) {
        NativeStr label{}, joined{};  // [ESP+8Ch] (EH 3Fh), [ESP+94h] (40h)
        void* const label_ret = host.native_string_assign_0041e870(&label, "ingame.sm_slots|.: ");
        const std::int32_t idle = host.shipyard_count_idle_slots_008434d0(selected_unit(s, host));
        void* const joined_ret = host.native_string_concat_int_004263b0(label_ret, &joined, idle);
        host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
        host.native_string_destroy_0041dd20(&joined);
        host.native_string_destroy_0041dd20(&label);
    } else if (after_next) {
        void* const block = selected_air_ops_block(s, host);  // EBX
        NativeStr label{}, joined{};                           // [ESP+8Ch] (EH 4Bh), [ESP+84h] (4Ch)
        void* const label_ret = host.native_string_assign_0041e870(&label, "ingame.sm_slots|.: ");
        const std::int32_t idle = host.air_ops_count_idle_slots_006bdc30(block);
        void* const joined_ret = host.native_string_concat_int_004263b0(label_ret, &joined, idle);
        host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
        host.native_string_destroy_0041dd20(&joined);
        host.native_string_destroy_0041dd20(&label);
    } else {
        const std::int32_t idle = host.air_ops_count_idle_slots_006bdc30(selected_air_ops_block(s, host));
        NativeStr number{}, joined{};  // [ESP+9Ch] (EH 3Dh), [ESP+84h] (3Eh)
        void* const number_ret = host.native_string_from_int_004260b0(&number, idle);
        void* const joined_ret = host.native_string_prefixed_0043c130(&joined, "ingame.sm_slots|.: ", number_ret);
        host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
        host.native_string_destroy_0041dd20(&joined);
        host.native_string_destroy_0041dd20(&number);
    }
}

// 0067861B..00678927: retitle +170h from the current record: the site's name; else the
// record's front unit's +3Ch object when it is 9h-flagged; else the front unit itself when
// its +15Ch count is nonzero, else "globals.unitclass_spawnpoint"; with no records the
// selected unit's name. Always followed by 0065BBC0.
// Temporaries: records [ESP+170h] (bit 1) and [ESP+140h] (EH 44h); lists [ESP+B8h] (45h),
// [ESP+C4h] (46h), [ESP+ACh] (47h), [ESP+12Ch], [ESP+4Ch] (48h).
void retitle_from_current_record(void* s, Pool21Host& host) {
    bool has_site = false;
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        SupportRecord record{};
        void* const r = host.int_list_int_record_copy_00668540(&record, current_record(s, host));
        has_site = at<void*>(r, 0) != nullptr;
        host.list_tidy_005a0b80(record.bytes + 4);
    }
    const char* name;
    if (has_site) {
        SupportRecord record{};
        void* const r = host.int_list_int_record_copy_00668540(&record, current_record(s, host));
        name = host.object_vtable14_name(at<void*>(r, 0));
        host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
        host.list_tidy_005a0b80(record.bytes + 4);
        host.support_layout_name_label_0065bbc0(s);
        return;
    }
    bool flagged = false;
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        NativeList outer{}, inner{};
        bool built_inner = false;
        host.list_copy_005aaaf0(&outer, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        if (at<void*>(list_front_value(outer, host), 0x3C) != nullptr) {
            host.list_copy_005aaaf0(&inner, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
            built_inner = true;
            flagged = host.unit_has_flag_vtable5c(at<void*>(list_front_value(inner, host), 0x3C), 9);
        }
        if (built_inner) host.list_tidy_005a0b80(&inner);
        host.list_tidy_005a0b80(&outer);
    }
    if (flagged) {
        NativeList units{};
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        name = host.object_vtable14_name(at<void*>(list_front_value(units, host), 0x3C));
        host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
        host.list_tidy_005a0b80(&units);
        host.support_layout_name_label_0065bbc0(s);
        return;
    }
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        NativeList units{};
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        const bool counted = at<std::uint32_t>(list_front_value(units, host), 0x15C) != 0;  // SETA BL
        host.list_tidy_005a0b80(&units);
        if (counted) {
            host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
            name = host.object_vtable14_name(list_front_value(units, host));
            host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
            host.list_tidy_005a0b80(&units);
            host.support_layout_name_label_0065bbc0(s);
            return;
        }
        name = "globals.unitclass_spawnpoint";
    } else {
        name = host.object_vtable14_name(selected_unit(s, host));
    }
    host.gui_text_set_localised_cstring_00abbe50(ptr(s, 0x170), name, 1);
    host.support_layout_name_label_0065bbc0(s);
}

// 0067821A..00678927: action 11Ah on the site page (+5Ch == 1): step the site cursor +308h
// back over the records (+34Ch) or the units (+1F4h), window +30Ch, refresh the view and
// the readouts, then retitle.
void handle_prev_on_site_page(void* s, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C0) != 0) return;
    if (at<std::uint32_t>(s, 0x30C) == 0) {
        std::uint32_t n = at<std::uint32_t>(s, 0x34C);
        if (n != 0 && n <= 4) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
        n = at<std::uint32_t>(s, 0x1F4);  // 0067824C re-tests the cached +30Ch (still 0)
        if (n != 0 && n <= 4) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
    }
    host.support_play_unit_to_unit_sound_0065f6b0(s);
    const std::int32_t window = at<std::int32_t>(s, 0x30C);
    if (window != 0) host.widget_vtable34(ptr(s, 0xCC + 4 * static_cast<std::size_t>(window)), 0);
    std::uint32_t n = at<std::uint32_t>(s, 0x34C);
    if (n == 0) n = at<std::uint32_t>(s, 0x1F4);
    if (n != 0) at<std::uint32_t>(s, 0x308) = (at<std::uint32_t>(s, 0x308) + n - 1) % n;
    if (at<std::int32_t>(s, 0x30C) > 0) {
        at<std::int32_t>(s, 0x30C) -= 1;
    } else {
        at<std::uint8_t>(s, 0x154) = 1;
        host.support_begin_scroll_forward_00660b10(s);
    }
    host.support_rebuild_00670020(s);
    bool slots = false;
    if (at<std::uint32_t>(s, 0x1F4) != 0) {
        show_unit_kind_icon(s, host);
        if (at<std::uint32_t>(s, 0x1F4) != 0 && host.game_effective_mode_004bca50(host.game_00e188a8()) == 8) {
            show_slots_label(s, false, host);
            slots = true;
        }
    }
    if (!slots) show_points_label(s, host);
    if (at<std::uint32_t>(s, 0x1F4) != 0) show_unit_kind_icon(s, host);
    retitle_from_current_record(s, host);
}

// 00678C3A: the cursor marker +128h tracks +220h (shown at its map position) or hides.
void place_cursor_marker(void* s, void* target, Pool21Host& host) {
    at<void*>(s, 0x220) = target;
    if (target == nullptr) {
        host.widget_vtable34(ptr(s, 0x128), 0);
        return;
    }
    host.widget_vtable34(ptr(s, 0x128), 1);
    const float* const position = host.entity_world_position_refreshed_00427eb0(ptr(s, 0x220));
    float by_value[3];  // the WorldToMap argument slot, filled by 00414220
    host.vector3f_assign_00414220(by_value, position);
    float map[2];  // [ESP+1Ch]
    host.support_world_to_map_00658580(s, map, by_value[0], by_value[1], by_value[2]);
    host.widget_set_local_xy_00aa7d00(ptr(s, 0x128), map[0], map[1]);
}

// 0067892C..00678CB8: action 11Ah on the deck page (+5Ch == 0): with records, step the deck
// cursor +2C4h back over +3E8h entries (window +324h); with a 46h-flagged selected unit,
// walk its slot entries back (+3 mod count) to the next one 006626E0 resolves (+228h) and
// track +254h; with a 9h/45h-flagged one, walk the air-ops slots to the next out (or state
// 2 / 5) record through the 58h air-ops slot record and track its +28h.
void handle_prev_on_deck_page(void* s, void* item, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C1) != 0 || at<std::uint8_t>(s, 0x2DC) != 0) return;
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        const std::uint32_t n = at<std::uint32_t>(s, 0x3E8);
        if (n == 0) return;
        if (at<std::uint32_t>(s, 0x324) == 0 && n < 5) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
        host.support_play_unit_to_unit_sound_0065f6b0(s);
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
        const std::uint32_t count = at<std::uint32_t>(s, 0x3E8);
        at<std::uint32_t>(s, 0x2C4) = (count + at<std::uint32_t>(s, 0x2C4) - 1) % count;
        const std::int32_t window = at<std::int32_t>(s, 0x324);
        if (window > 0) {
            at<std::int32_t>(s, 0x324) = window - 1;
        } else {
            at<std::uint8_t>(s, 0x154) = 0;
            host.support_begin_scroll_back_00661240(s);
        }
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1);
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1);
        return;
    }
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) {
        if (host.unit_has_flag_vtable5c(selected_unit(s, host), 0x46) && at<std::int32_t>(s, 0x298) > 1) {
            host.support_play_unit_to_unit_sound_0065f6b0(s);
            host.widget_vtable34(ptr(s, 0xF0), 1);
            host.widget_vtable34(ptr(s, 0xF4), 0);
            host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
            void* const unit = selected_unit(s, host);         // EDI
            const std::int32_t start = at<std::int32_t>(s, 0x324);  // EBX
            std::int32_t index = (start + 3) % host.unit_slot_count_00656440(unit);  // CDQ / IDIV
            while (index != at<std::int32_t>(s, 0x324)) {
                alignas(4) std::uint8_t entry[0x4C];  // [ESP+184h], EH 3Ch
                void* const copied = host.unit_slot_entry_copy_00846fd0(unit, entry, static_cast<std::uint32_t>(index));
                host.observer_string_record_assign_006626e0(static_cast<std::uint8_t*>(s) + 0x224, copied);
                host.unit_slot_entry_destroy_0065bb00(entry);
                if (at<std::uint32_t>(s, 0x228) != 0) {
                    place_cursor_marker(s, ptr(s, 0x254), host);
                    at<std::int32_t>(s, 0x324) = index;
                    break;
                }
                index = (index + 3) % host.unit_slot_count_00656440(unit);
            }
        }
    } else if (at<std::int32_t>(s, 0x298) > 1 && at<std::uint32_t>(s, 0x324) != 0) {
        host.support_play_unit_to_unit_sound_0065f6b0(s);
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
        void* const block = selected_air_ops_block(s, host);  // EDI
        const std::int32_t start = at<std::int32_t>(s, 0x324);  // [ESP+14h]
        std::int32_t index = (start + 3) % host.locked_record_read_at50_00656250(block);
        if (index != start) {
            for (;;) {
                host.air_ops_slot_record_assign_0065ca50(
                    item, static_cast<std::uint8_t*>(ptr(block, 0x4C)) + static_cast<std::size_t>(index) * 0x58);
                const bool out = host.air_ops_slot_record_is_out_00658530(item);
                const std::int32_t state = at<std::int32_t>(item, 0x2C);
                if (out || state == 2 || state == 5) {
                    place_cursor_marker(s, ptr(item, 0x28), host);
                    at<std::int32_t>(s, 0x324) = index;
                    break;
                }
                index = (index + 3) % host.locked_record_read_at50_00656250(block);
                if (index == at<std::int32_t>(s, 0x324)) break;
            }
        }
    }
    host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1);  // 00678CA2
}

// 00678CEF..006792CA: action 11Bh on the unit page: the forward mirror of
// handle_prev_on_unit_page (cursor +1, window up to 3). Its 0Fh path neither highlights nor
// refreshes the info panel, the second (list +2E4h) step hides the slot widget at +30Ch
// rather than +314h, and an entry of neither kind leaves the widgets alone.
void handle_next_on_unit_page(void* s, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C0) != 0) return;
    bool catalog = at<std::uint32_t>(s, 0x34C) != 0;
    if (!catalog) {
        if (at<std::uint32_t>(s, 0x1F4) == 0) return;
        catalog = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x46);
    }
    if (catalog) {
        const std::uint32_t count = at<std::uint32_t>(s, 0x384);  // EBX
        const std::int32_t window = at<std::int32_t>(s, 0x314);   // ECX
        if (static_cast<std::uint32_t>(window) == count - 1) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
        at<std::uint32_t>(s, 0x310) = (at<std::uint32_t>(s, 0x310) + 1) % count;
        at<std::uint32_t>(s, 0x334) = 0;
        if (window < 3) {
            at<std::int32_t>(s, 0x314) = window + 1;
        } else {
            at<std::uint8_t>(s, 0x154) = 0;
            host.support_begin_scroll_forward_00660b10(s);
        }
        host.support_update_payload_strip_0065a740(s);
        host.support_play_unit_to_unit_sound_0065f6b0(s);
        at<void*>(s, 0x3A4) = host.support_catalog_entry_at_index_0065ab40(s);
        host.widget_vtable34(ptr(s, 0xCC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x314))), 1);
        if (host.object_vtable18(ptr(s, 0x3A4), 0xF) && !host.object_vtable18(ptr(s, 0x3A4), 0x17)) {
            host.support_select_value_0065c9a0(s, ptr(s, 0x3A4));
            show_loadout_icon(s, entry_3a4, host);
            host.support_build_loadout_ring_00665f30(s);
            at<std::uint8_t>(s, 0x2B0) = 1;
            host.widget_vtable34(ptr(s, 0xF0), 0);
            host.widget_vtable34(ptr(s, 0xF4), 1);
        } else if (host.object_vtable18(ptr(s, 0x3A4), 0x17)) {
            host.support_clear_loadout_icons_0065abb0(s);
            host.widget_vtable88(ptr(s, 0x158), 1, 0, 1.0f);
            show_f0_f4(s, 0, 1, host);
        } else {
            show_other_catalog_entry(s, host);  // EH 57h
        }
    }
    // 00679053
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) return;
    if (at<std::uint32_t>(s, 0x314) == at<std::uint32_t>(s, 0x2EC) - 1) {
        host.support_play_unavailable_sound_0065f170(s);
        return;
    }
    at<std::uint32_t>(s, 0x334) = 0;
    host.widget_vtable34(ptr(s, 0xCC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x30C))), 0);
    at<std::uint32_t>(s, 0x310) = (at<std::uint32_t>(s, 0x310) + 1) % at<std::uint32_t>(s, 0x2EC);
    const std::int32_t window = at<std::int32_t>(s, 0x314);
    if (window < 3) {
        at<std::int32_t>(s, 0x314) = window + 1;
    } else {
        at<std::uint8_t>(s, 0x154) = 0;
        host.support_begin_scroll_forward_00660b10(s);
    }
    host.support_update_payload_strip_0065a740(s);
    void* const entry = host.support_catalog_entry_2e4_at_index_0065ae90(s);
    at<void*>(s, 0x2A0) = entry;
    if (host.object_vtable18(entry, 0xF) && !host.object_vtable18(ptr(s, 0x2A0), 0x17)) {
        host.support_select_weapon_value_00669170(s);
        show_loadout_icon(s, entry_2a0, host);
        host.support_build_hangar_loadout_ring_00666dc0(s);
        host.support_highlight_hangar_loadout_00662210(s);
        at<std::uint8_t>(s, 0x2B0) = 1;
        show_f0_f4(s, 0, 1, host);
    } else if (host.object_vtable18(ptr(s, 0x2A0), 0x17)) {
        host.support_clear_loadout_icons_0065abb0(s);
        host.widget_vtable88(ptr(s, 0x158), 1, 0, 1.0f);
        show_f0_f4(s, 0, 1, host);
    }
}

// One forward step of the site cursor +308h over count n (window +30Ch up to 3).
void step_site_cursor_forward(void* s, std::uint32_t n, Pool21Host& host) {
    at<std::uint32_t>(s, 0x308) = (at<std::uint32_t>(s, 0x308) + 1) % n;
    const std::int32_t window = at<std::int32_t>(s, 0x30C);
    if (window < 3) {
        at<std::int32_t>(s, 0x30C) = window + 1;
    } else {
        at<std::uint8_t>(s, 0x154) = 0;
        host.support_begin_scroll_forward_00660b10(s);
    }
}

// 006792CA..00679963: action 11Bh on the site page: the forward mirror of
// handle_prev_on_site_page. With both records and units the cursor steps twice, and the
// kind icon is not re-shown after the readout.
void handle_next_on_site_page(void* s, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C0) != 0) return;
    std::uint32_t n = at<std::uint32_t>(s, 0x34C);
    if (n != 0 && at<std::uint32_t>(s, 0x30C) == n - 1) {
        host.support_play_unavailable_sound_0065f170(s);
        return;
    }
    n = at<std::uint32_t>(s, 0x1F4);
    if (n != 0 && at<std::uint32_t>(s, 0x30C) == n - 1) {
        host.support_play_unavailable_sound_0065f170(s);
        return;
    }
    host.support_play_unit_to_unit_sound_0065f6b0(s);
    const std::int32_t window = at<std::int32_t>(s, 0x30C);
    if (window != 3) host.widget_vtable34(ptr(s, 0xCC + 4 * static_cast<std::size_t>(window)), 0);
    if ((n = at<std::uint32_t>(s, 0x34C)) != 0) step_site_cursor_forward(s, n, host);
    if ((n = at<std::uint32_t>(s, 0x1F4)) != 0) step_site_cursor_forward(s, n, host);
    host.support_rebuild_00670020(s);
    bool slots = false;
    if (at<std::uint32_t>(s, 0x1F4) != 0) {
        show_unit_kind_icon(s, host);
        if (at<std::uint32_t>(s, 0x1F4) != 0 && host.game_effective_mode_004bca50(host.game_00e188a8()) == 8) {
            show_slots_label(s, true, host);
            slots = true;
        }
    }
    if (!slots) show_points_label(s, host);
    retitle_from_current_record(s, host);
}

// 00679963..00679C8C: action 11Bh on the deck page: the forward mirror of
// handle_prev_on_deck_page (+1 mod count). The record step falls through to the unit step;
// the found slot is tracked by storing +220h directly (no marker placement); an air-ops
// unit with +298h <= 1 or at the last slot plays the unavailable sound.
void handle_next_on_deck_page(void* s, void* item, Pool21Host& host) {
    if (at<std::uint8_t>(s, 0x2C1) != 0 || at<std::uint8_t>(s, 0x2DC) != 0) return;
    if (at<std::uint32_t>(s, 0x34C) != 0 && at<std::uint32_t>(s, 0x3E8) != 0) {
        if (at<std::uint32_t>(s, 0x324) == at<std::uint32_t>(s, 0x3E8) - 1) {
            host.support_play_unavailable_sound_0065f170(s);
            return;
        }
        host.support_play_unit_to_unit_sound_0065f6b0(s);
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
        at<std::uint32_t>(s, 0x2C4) = (at<std::uint32_t>(s, 0x2C4) + 1) % at<std::uint32_t>(s, 0x3E8);
        const std::int32_t window = at<std::int32_t>(s, 0x324);
        if (window < 3) {
            at<std::int32_t>(s, 0x324) = window + 1;
        } else {
            at<std::uint8_t>(s, 0x154) = 1;
            host.support_begin_scroll_back_00661240(s);
        }
        host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1);
    }
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) {
        if (host.unit_has_flag_vtable5c(selected_unit(s, host), 0x46) && at<std::int32_t>(s, 0x298) > 1) {
            host.support_play_unit_to_unit_sound_0065f6b0(s);
            host.widget_vtable34(ptr(s, 0xF0), 1);
            host.widget_vtable34(ptr(s, 0xF4), 0);
            host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
            void* const unit = selected_unit(s, host);
            const std::int32_t start = at<std::int32_t>(s, 0x324);
            std::int32_t index = (start + 1) % host.unit_slot_count_00656440(unit);
            while (index != at<std::int32_t>(s, 0x324)) {
                alignas(4) std::uint8_t entry[0x4C];  // [ESP+184h], EH 4Ah
                void* const copied = host.unit_slot_entry_copy_00846fd0(unit, entry, static_cast<std::uint32_t>(index));
                host.observer_string_record_assign_006626e0(static_cast<std::uint8_t*>(s) + 0x224, copied);
                host.unit_slot_entry_destroy_0065bb00(entry);
                if (at<std::uint32_t>(s, 0x228) != 0) {
                    void* const target = ptr(s, 0x254);
                    at<std::int32_t>(s, 0x324) = index;
                    at<void*>(s, 0x220) = target;
                    break;
                }
                index = (index + 1) % host.unit_slot_count_00656440(unit);
            }
        }
    } else if (at<std::int32_t>(s, 0x298) <= 1) {
        host.support_play_unavailable_sound_0065f170(s);
    } else {
        void* const block = selected_air_ops_block(s, host);  // EBX
        if (at<std::int32_t>(s, 0x324) == host.locked_record_read_at50_00656250(block) - 1) {
            host.support_play_unavailable_sound_0065f170(s);
        } else {
            host.support_play_unit_to_unit_sound_0065f6b0(s);
            host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
            const std::int32_t start = at<std::int32_t>(s, 0x324);  // [ESP+14h]
            std::int32_t index = (start + 1) % host.locked_record_read_at50_00656250(block);
            if (index != start) {
                for (;;) {
                    host.air_ops_slot_record_assign_0065ca50(
                        item, static_cast<std::uint8_t*>(ptr(block, 0x4C)) + static_cast<std::size_t>(index) * 0x58);
                    const bool out = host.air_ops_slot_record_is_out_00658530(item);
                    const std::int32_t state = at<std::int32_t>(item, 0x2C);
                    if (out || state == 2 || state == 5) {
                        at<std::int32_t>(s, 0x324) = index;
                        at<void*>(s, 0x220) = ptr(item, 0x28);
                        break;
                    }
                    index = (index + 1) % host.locked_record_read_at50_00656250(block);
                    if (index == at<std::int32_t>(s, 0x324)) break;
                }
            }
        }
    }
    host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1);  // 00679C76
}

// 00677C2B..00678CBA and 00678CBA..00679C8E: actions 11Ah (previous) and 11Bh (next),
// dispatched on the page +5Ch.
void handle_prev_next(void* s, void* item, Pool21Host& host) {
    if (host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x11A)) {
        switch (at<std::int32_t>(s, 0x5C)) {
        case 0: handle_prev_on_deck_page(s, item, host); break;
        case 1: handle_prev_on_site_page(s, host); break;
        case 2: handle_prev_on_unit_page(s, host); break;
        default: break;
        }
    }
    if (host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x11B)) {
        switch (at<std::int32_t>(s, 0x5C)) {
        case 0: handle_next_on_deck_page(s, item, host); break;
        case 1: handle_next_on_site_page(s, host); break;
        case 2: handle_next_on_unit_page(s, host); break;
        default: break;
        }
    }
}

// The loadout icon +158h for value 0: frame 1 when the entry is 17h-flagged, else 0.
void show_empty_loadout_icon(void* s, void* entry, Pool21Host& host) {
    const std::uint32_t frame = host.object_vtable18(entry, 0x17) ? 1 : 0;
    host.widget_vtable88(ptr(s, 0x158), frame, 0, 1.0f);
}

// 00679C8E..0067A051: action 116h on the unit page steps the loadout value +334h through
// 0..count (00951F10, signed IDIV). An air-ops unit (9h or 45h, with +1F4h set) steps the
// hangar entry +2A0h (refused when 17h-flagged; a 10h-flagged entry skips 0); any other
// unit steps the catalog entry +3A4h (refused unless 0Fh-flagged and not 17h-flagged): a
// 10h-flagged entry skips the last value when +1F4h is set (00679D57) and skips 0 when it
// is not (0067A017). Then the icon, the highlight and the change-weapon sound.
void handle_cycle_loadout(void* s, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x116)) return;
    if (at<std::int32_t>(s, 0x5C) != 2) return;
    const bool has_unit = at<std::uint32_t>(s, 0x1F4) != 0;
    bool hangar = false;
    if (has_unit) {
        hangar = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
        if (!hangar) hangar = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    }
    if (hangar) {  // 00679E62
        if (host.object_vtable18(ptr(s, 0x2A0), 0x17)) return;
        const std::int32_t count = host.vehicle_class_equipment_count_00951f10(ptr(s, 0x2A0));
        at<std::int32_t>(s, 0x334) = (at<std::int32_t>(s, 0x334) + 1) % (count + 1);
        if (host.object_vtable18(ptr(s, 0x2A0), 0x10) && at<std::uint32_t>(s, 0x334) == 0)
            at<std::uint32_t>(s, 0x334) = 1;
        if (at<std::uint32_t>(s, 0x334) != 0)
            show_loadout_icon(s, entry_2a0, host);
        else
            show_empty_loadout_icon(s, ptr(s, 0x2A0), host);
        host.support_highlight_hangar_loadout_00662210(s);
    } else {  // 00679CF6 (+1F4h set) / 00679FBA (+1F4h clear)
        if (!host.object_vtable18(ptr(s, 0x3A4), 0xF)) return;
        if (host.object_vtable18(ptr(s, 0x3A4), 0x17)) return;
        const std::int32_t count = host.vehicle_class_equipment_count_00951f10(ptr(s, 0x3A4));
        at<std::int32_t>(s, 0x334) = (at<std::int32_t>(s, 0x334) + 1) % (count + 1);
        if (host.object_vtable18(ptr(s, 0x3A4), 0x10)) {
            if (has_unit) {
                if (at<std::int32_t>(s, 0x334) == host.vehicle_class_equipment_count_00951f10(ptr(s, 0x3A4)) - 1)
                    at<std::uint32_t>(s, 0x334) = 0;
            } else if (at<std::uint32_t>(s, 0x334) == 0) {
                at<std::uint32_t>(s, 0x334) = 1;
            }
        }
        if (at<std::uint32_t>(s, 0x334) != 0)  // 00679D75
            show_loadout_icon(s, entry_3a4, host);
        else
            show_empty_loadout_icon(s, ptr(s, 0x3A4), host);
        host.support_highlight_loadout_00662020(s);
    }
    host.support_play_change_weapon_sound_0065f410(s);
}

// Five fresh 004BCA50 calls (0067A231): modes 0, 1, 2, 3 and 8 show the points readout.
bool mode_shows_points(Pool21Host& host) {
    static constexpr std::int32_t modes[] = {0, 1, 2, 3, 8};
    for (const std::int32_t mode : modes)
        if (host.game_effective_mode_004bca50(host.game_00e188a8()) == mode) return true;
    return false;
}

// show_points_label with the three NativeStrings released inline (0067A2BE..0067A396):
// number [ESP+3Ch] (EH 5Ch), label [ESP+44h] (5Dh), joined [ESP+1Ch] (5Eh).
void show_points_label_inline(void* s, Pool21Host& host) {
    const std::int32_t whole =
        host.crt_ftol2_00bf7420(host.player_support_points_00946f00(at<std::uint32_t>(host.game_00e188a8(), 0x18EC)));
    NativeStr number{}, label{}, joined{};
    void* const number_ret = host.native_string_from_int_004260b0(&number, whole);
    void* const label_ret = host.native_string_assign_0041e870(&label, "ingame.sm_cp|.: ");
    void* const joined_ret = host.native_string_concat_004261a0(label_ret, &joined, number_ret);
    host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), joined_ret, 1);
    release_inline(joined, host);
    release_inline(label, host);
    release_inline(number, host);
}

// 0067A051..0067A3DB: action 4Bh on the unit page returns to the site page (+60h = 2): the
// selection info, the sound, the points bar +188h (width = points * f[+1ACh] / cap, the
// product rounded to the 24-bit precision control and kept in a double slot [ESP+60h], then
// FDIVR by the cap and FSTP to a float), the readout +1A8h (slots in mode 8 with +1F4h set,
// points in modes 0-3/8, else "ingame.sm_support"), then +180h/+184h hidden, +188h shown.
void handle_back_to_sites(void* s, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x4B)) return;
    if (at<std::int32_t>(s, 0x5C) != 2) return;
    host.support_show_selection_info_00671d60(s);
    host.support_play_sp_to_unit_sound_0065f2c0(s);
    const float height = host.gui_widget_get_size_00aa6740(ptr(s, 0x188))[1];  // [ESP+2Ch]
    const std::uint32_t player = at<std::uint32_t>(host.game_00e188a8(), 0x18EC);  // EBX
    const double product =
        static_cast<float>(host.player_support_points_00946f00(player) * at<float>(s, 0x1AC));
    float bar[2];  // [ESP+1Ch]
    bar[0] = static_cast<float>(product / host.team_support_points_cap_00942130(player));
    bar[1] = height;
    host.widget_vtable58_set_size(ptr(s, 0x188), bar);
    if (at<std::uint32_t>(s, 0x1F4) != 0 && host.game_effective_mode_004bca50(host.game_00e188a8()) == 8) {
        show_slots_label(s, true, host);  // label [ESP+70h], joined [ESP+60h] (EH 5Ah/5Bh, 58h/59h)
    } else if (mode_shows_points(host)) {
        show_points_label_inline(s, host);
    } else {
        NativeStr text{};  // [ESP+60h], EH 5Fh
        void* const text_ret = host.native_string_assign_0041e870(&text, "ingame.sm_support");
        host.gui_text_set_localised_source_00abaed0(ptr(s, 0x1A8), text_ret, 1);
        host.native_string_destroy_0041dd20(&text);
    }
    host.widget_vtable34(ptr(s, 0x180), 0);
    host.widget_vtable34(ptr(s, 0x184), 0);
    host.widget_vtable34(ptr(s, 0x188), 1);
    host.support_clear_loadout_icons_0065abb0(s);
    at<std::uint32_t>(s, 0x310) = 0;
    at<std::uint32_t>(s, 0x314) = 0;
    at<std::int32_t>(s, 0x60) = 2;
}

// The input manager's pad state: [[[004BEC00()]+4h] + 2FD0h + 2Ch].
std::uint8_t* pad_state(Pool21Host& host) {
    void* const manager = host.input_manager_singleton_004bec00();
    return static_cast<std::uint8_t*>(at<void*>(ptr(manager, 4), 0x2FD0 + 0x2C));
}

// The order confirmation when the countdown +0Ch is exactly 0.0f (UCOMISS against
// 00D7A218 with the parity test, so a NaN does not confirm).
void confirm_if_idle(void* s, Pool21Host& host) {
    if (at<float>(s, 0xC) == 0.0f) host.support_confirm_order_00673a10(s);
}

// Upper bound for the stack storage at [ESP+140h] (it ends at the SEH record [ESP+1D4h]);
// the image's own sizes for the message and the target descriptor (18h) are smaller.
constexpr std::size_t kStackScratch = 0x94;

// 0067A4C8..0067A5F3: the primary button on the deck page launches from the selected air-ops
// slot: the slot record +4Ch[+324h] is copied into the 58h air-ops slot record; a state-5 slot with stock
// (006BF310 on the item's class +4h) is filled with min(stock, +2B4h) (006C4780) and a kind-3
// message (00656280 at [ESP+140h], arg +21Ch, the local player) is routed through the block's
// +7Ch (0077C2A0, EH 61h). The message gets no destructor call.
void launch_from_deck(void* s, void* item, Pool21Host& host) {
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) return;
    void* const block = selected_air_ops_block(s, host);  // EBX
    host.air_ops_slot_record_assign_0065ca50(
        item, static_cast<std::uint8_t*>(ptr(block, 0x4C)) + static_cast<std::size_t>(at<std::int32_t>(s, 0x324)) * 0x58);
    if (at<std::int32_t>(item, 0x2C) != 5) return;
    const std::int32_t stock = host.air_ops_stock_count_006bf310(block, at<std::uint32_t>(item, 4));
    if (stock <= 0) return;
    std::int32_t count = at<std::int32_t>(s, 0x2B4);
    if (stock < count) count = stock;  // CMOVL
    host.air_ops_fill_slot_006c4780(block, at<std::int32_t>(s, 0x324), at<std::uint32_t>(item, 4), count);
    alignas(8) std::uint8_t storage[kStackScratch];  // [ESP+140h]
    void* const message = host.session_message130_construct_00656280(
        storage, block, at<std::int32_t>(s, 0x324), 3, at<std::uint32_t>(s, 0x21C),
        at<std::uint32_t>(host.game_00e188a8(), 0x18EC));
    host.session_route_message_0077c2a0(ptr(block, 0x7C), message, 0, 0);
}

// 0067A3DB..0067A664: unless the active tutorial hint blocks, the pad's primary (+0Bh) and
// secondary (+10h) buttons act on the page +5Ch.
void handle_pad_buttons(void* s, void* item, Pool21Host& host) {
    const std::uint8_t primary = pad_state(host)[0xB];     // BL
    const std::uint8_t secondary = pad_state(host)[0x10];  // AL, read before the test of BL
    const std::int32_t page = at<std::int32_t>(s, 0x5C);
    if (primary != 0) {
        if (page == 0) {
            launch_from_deck(s, item, host);
        } else if (page == 1) {  // 0067A462
            host.support_refresh_selection_0066d020(s);
            host.support_play_sp_to_unit_sound_0065f2c0(s);
            at<std::int32_t>(s, 0x60) = 1;
            host.widget_vtable34(ptr(s, 0x12C), 0);
            host.widget_vtable34(ptr(s, 0x130), 0);
            host.widget_vtable34(ptr(s, 0x134), 0);
            for (std::size_t i = 0; i < 4; ++i) host.widget_vtable34(ptr(s, 0x9C + 4 * i), 0);
        } else if (page == 2) {
            host.support_set_tracked_unit_target_0065be80(s, nullptr);
            confirm_if_idle(s, host);
        }
    } else if (secondary != 0) {
        if (page == 1) {  // 0067A64B
            at<std::uint8_t>(s, 0x71) = 0;
            host.support_refresh_selection_0066d020(s);
            host.support_play_sp_to_unit_sound_0065f2c0(s);
            at<std::int32_t>(s, 0x60) = 1;
        } else if (page == 2 && host.global_00e188d8() != nullptr) {
            void* const target = at<void*>(at<void*>(host.hud_00e198c4(), 0xCC), 0x4C);
            if (target == nullptr) {
                host.support_play_unavailable_sound_0065f170(s);
            } else {
                host.support_set_tracked_unit_target_0065be80(s, target);
                confirm_if_idle(s, host);
            }
        }
    }
}

// The checked iterator over list +3E0h (head +3E4h) advanced +2C4h times by the postfix
// 00658FE0 (+2C4h first clamped to +3E8h - 1, unsigned); returns the iterator after the
// inlined dereference checks (its owner is reloaded from [ESP+2Ch] for the later end test).
CheckedIter walk_deck_slots(void* s, Pool21Host& host) {
    CheckedIter it{static_cast<std::uint8_t*>(s) + 0x3E0, node_next(ptr(s, 0x3E4))};  // [ESP+2Ch]
    if (at<std::uint32_t>(s, 0x2C4) >= at<std::uint32_t>(s, 0x3E8)) at<std::uint32_t>(s, 0x2C4) = at<std::uint32_t>(s, 0x3E8) - 1;
    CheckedIter previous{};  // [ESP+60h]
    for (std::int32_t i = 0; i < at<std::int32_t>(s, 0x2C4); ++i)
        host.list_iterator_post_increment_00658fe0(&it, &previous, 0);
    if (it.owner == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
    if (it.node == at<void*>(it.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
    return it;
}

// The order issued to the tracked unit +220h: command 00E08F08 when its class tag (vtable
// +174h, asked twice) is 00E08F98 or 00E08FA0, else 00E08F98, with a fresh target
// descriptor (004F1830 at [ESP+140h]) and flags 1. On the record path the special order
// also pings the slot widget +BCh[+324h] (vtable +4Ch, 1.0f).
void issue_tracked_order(void* s, bool ping_slot, Pool21Host& host) {
    bool special = host.entity_vtable174(ptr(s, 0x220)) == 0xE08F98;
    if (!special) special = host.entity_vtable174(ptr(s, 0x220)) == 0xE08FA0;
    alignas(8) std::uint8_t storage[kStackScratch];  // [ESP+140h]
    void* const target = host.command_target_init_raw_004f1830(storage);
    host.entity_issue_command_0077d600(ptr(s, 0x220), special ? 0xE08F08 : 0xE08F98, target, 1);
    if (special && ping_slot)
        host.widget_vtable4c(ptr(s, 0xBC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 1.0f);
}

// 0067A686..0067AAA7: action 102h on the deck page orders the slot's unit. With records
// (+34Ch, needing +3B4h) the walked node's +18h becomes +220h when node +14h is set and it
// must be 18h-flagged. With an air-ops unit the slot +4Ch[+324h] is copied into the item; a
// state-2 slot is recalled: the item copy is set to state 1 with +30h = 0 (5.0f, 00CE3850,
// when +34h was set, which is cleared), a kind-2 message (unit = the selected unit) is routed
// (EH 62h), the cursor widget hidden, +5Ch = 1, and the cursor moves to the next slot that
// is out (+28h set and state 3/4) or in state 2/5, setting +220h, +324h and +5Ch = 0.
// Otherwise (0067AA2C) a tracked unit that is 18h- or 0Fh-flagged is ordered.
void handle_order_slot(void* s, void* item, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x102)) return;
    if (at<std::int32_t>(s, 0x5C) != 0) return;
    if (at<std::uint32_t>(s, 0x34C) != 0) {
        if (at<std::uint32_t>(s, 0x3B4) == 0) return;
        const CheckedIter walked = walk_deck_slots(s, host);
        void* const node = walked.node;
        if (at<std::uint8_t>(node, 0x14) == 0) return;
        if (node == at<void*>(walked.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // [ESP+2Ch] reloaded
        at<void*>(s, 0x220) = ptr(node, 0x18);
        if (ptr(s, 0x220) == nullptr) return;
        if (!host.unit_has_flag_vtable5c(ptr(s, 0x220), 0x18)) return;
        issue_tracked_order(s, true, host);
        return;
    }
    if (at<std::uint32_t>(s, 0x1F4) == 0) return;
    bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
    if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
    if (air_ops) {
        host.air_ops_slot_record_assign_0065ca50(
            item, static_cast<std::uint8_t*>(ptr(selected_air_ops_block(s, host), 0x4C)) +
                      static_cast<std::size_t>(at<std::int32_t>(s, 0x324)) * 0x58);
        if (at<std::int32_t>(item, 0x2C) == 2) {
            const bool flag34 = at<std::uint8_t>(item, 0x34) != 0;
            at<std::int32_t>(item, 0x2C) = 1;
            at<float>(item, 0x30) = 0.0f;
            if (flag34) {
                at<float>(item, 0x30) = 5.0f;  // [00CE3850]
                at<std::uint8_t>(item, 0x34) = 0;
            }
            void* const block = selected_air_ops_block(s, host);  // EDI
            const std::uint32_t player = at<std::uint32_t>(host.game_00e188a8(), 0x18EC);  // [ESP+2Ch]
            {
                alignas(8) std::uint8_t storage[kStackScratch];  // [ESP+140h]
                void* const message = host.session_message130_construct_00656280(
                    storage, block, at<std::int32_t>(s, 0x324), 2,
                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(selected_unit(s, host))), player);
                host.session_route_message_0077c2a0(ptr(block, 0x7C), message, 0, 0);
            }
            host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(at<std::int32_t>(s, 0x324))), 0);
            at<std::int32_t>(s, 0x5C) = 1;
            std::int32_t index = (at<std::int32_t>(s, 0x324) + 1) % host.locked_record_read_at50_00656250(block);  // EBX
            while (index != at<std::int32_t>(s, 0x324)) {
                host.air_ops_slot_record_assign_0065ca50(
                    item, static_cast<std::uint8_t*>(ptr(block, 0x4C)) + static_cast<std::size_t>(index) * 0x58);
                void* const unit = ptr(item, 0x28);
                const std::int32_t state = at<std::int32_t>(item, 0x2C);
                if ((unit != nullptr && (state == 3 || state == 4)) || state == 2 || state == 5) {
                    at<void*>(s, 0x220) = unit;
                    at<std::int32_t>(s, 0x324) = index;
                    at<std::int32_t>(s, 0x5C) = 0;
                    host.widget_vtable34(ptr(s, 0xDC + 4 * static_cast<std::size_t>(index)), 1);
                    break;
                }
                index = (index + 1) % host.locked_record_read_at50_00656250(block);
            }
            return;
        }
    }
    if (ptr(s, 0x220) == nullptr) return;  // 0067AA2C
    bool orderable = host.unit_has_flag_vtable5c(ptr(s, 0x220), 0x18);
    if (!orderable) orderable = host.unit_has_flag_vtable5c(ptr(s, 0x220), 0xF);
    if (!orderable) return;
    issue_tracked_order(s, false, host);
}

void spectate(void* unit, Pool21Host& host) {
    host.ingame_hud_set_spectated_unit_00647300(at<void*>(host.hud_00e198c4(), 0x40), unit);
}

// 0067AAA7..0067AED4: action 101h spectates. On the site page: a selected unit whose +188h
// is 9 or the local player's index; else the current record's site; else the record's front
// unit's +3Ch object when it is 9h-flagged and the mode is 0-3 (three list copies at
// [ESP+4Ch], EH 63h/64h for the last two). On the deck page the slot's unit (+220h, from the
// walked node or the copied slot record +28h) is spectated, its +3D0h cached in +458h when
// 18h- or 0Fh-flagged, the map's speaking icon cleared and input action +3600h reset.
void handle_spectate(void* s, void* item, Pool21Host& host) {
    if (!host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x101)) return;
    const std::int32_t page = at<std::int32_t>(s, 0x5C);
    if (page == 1) {
        if (at<std::uint32_t>(s, 0x1F4) != 0) {
            const std::uint32_t player = at<std::uint32_t>(host.game_00e188a8(), 0x18EC);  // EDI
            const std::uint32_t owner = at<std::uint32_t>(checked_selected_unit(s, host), 0x188);
            if (owner == 9 || owner == player) {
                spectate(checked_selected_unit(s, host), host);
                return;
            }
        }
        if (at<std::uint32_t>(s, 0x34C) == 0) return;
        SupportRecord record{};  // [ESP+4Ch]
        void* const site = inline_copy_current_record(s, record, host);
        host.list_tidy_005a0b80(record.bytes + 4);
        if (site != nullptr) {
            spectate(site, host);
            return;
        }
        NativeList units{};  // [ESP+4Ch]
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        void* const object = at<void*>(list_front_value(units, host), 0x3C);
        host.list_tidy_005a0b80(&units);
        if (object == nullptr) return;
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        const bool flagged = host.unit_has_flag_vtable5c(at<void*>(list_front_value(units, host), 0x3C), 9);
        host.list_tidy_005a0b80(&units);
        if (!flagged) return;
        bool shown = false;  // four fresh 004BCA50 calls
        for (std::int32_t mode = 0; mode < 4 && !shown; ++mode)
            shown = host.game_effective_mode_004bca50(host.game_00e188a8()) == mode;
        if (!shown) return;
        host.list_copy_005aaaf0(&units, static_cast<std::uint8_t*>(current_record(s, host)) + 4);
        spectate(at<void*>(list_front_value(units, host), 0x3C), host);
        host.list_tidy_005a0b80(&units);
        return;
    }
    if (page != 0) return;
    if (at<std::uint32_t>(s, 0x34C) != 0 && at<std::uint32_t>(s, 0x3E8) != 0) {
        const CheckedIter walked = walk_deck_slots(s, host);
        void* const node = walked.node;
        if (at<std::uint8_t>(node, 0x14) != 0) {
            if (node == at<void*>(walked.owner, 4)) host.crt_invalid_parameter_noinfo_00bf6713();  // [ESP+2Ch] reloaded
            at<void*>(s, 0x220) = ptr(node, 0x18);
        } else {
            at<void*>(s, 0x220) = nullptr;
        }
    } else if (at<std::uint32_t>(s, 0x1F4) != 0) {
        bool air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 9);
        if (!air_ops) air_ops = host.unit_has_flag_vtable5c(selected_unit(s, host), 0x45);
        if (air_ops) {
            host.air_ops_slot_record_assign_0065ca50(
                item, static_cast<std::uint8_t*>(ptr(selected_air_ops_block(s, host), 0x4C)) +
                          static_cast<std::size_t>(at<std::int32_t>(s, 0x324)) * 0x58);
            at<void*>(s, 0x220) = ptr(item, 0x28);
        }
    }
    if (ptr(s, 0x220) != nullptr) {  // 0067AE5E
        bool cached = host.unit_has_flag_vtable5c(ptr(s, 0x220), 0x18);
        if (!cached) cached = host.unit_has_flag_vtable5c(ptr(s, 0x220), 0xF);
        if (cached) at<std::uint32_t>(s, 0x458) = at<std::uint32_t>(ptr(s, 0x220), 0x3D0);
    }
    void* const unit = ptr(s, 0x220);
    if (unit == nullptr) return;
    spectate(unit, host);
    host.hud_map_clear_speaking_icon_005a4860(at<void*>(host.hud_00e198c4(), 0x54));
    host.input_action_reset_00a91e20(static_cast<std::uint8_t*>(ptr(host.input_manager_singleton_004bec00(), 4)) + 0x3600);
}

} // namespace

void hud_support_manager_screen_update_00675c40(void* s, float dt, Pool21Host& host) {
    // 00675C72: the countdown +0Ch (FSUB through a float store; cleared at or below zero).
    const float timer = at<float>(s, 0xC);
    if (timer != 0.0f) {
        const float left = timer - dt;
        at<float>(s, 0xC) = left;
        if (left <= 0.0f) at<float>(s, 0xC) = 0.0f;  // FLDZ / FCOMIP / JC
    }
    void* const hud = host.hud_00e198c4();
    host.widget_vtable34(at<void*>(at<void*>(hud, 0xC4), 0x64), 1);
    NativeStr message{};  // built in 00682BD0's argument slot, released by the callee
    host.native_string_assign_0041e870(
        &message, host.global_byte_00f88a30() == 0 ? "ingame.sm_annoying_message_pc" : "ingame.sm_annoying_message_xbox");
    host.warning_set_warning2_text_00682bd0(at<void*>(host.hud_00e198c4(), 0xC4), &message);
    void* const game = host.game_00e188a8();
    if (at<std::uint8_t>(game, 0x61F) != 0 || at<std::uint8_t>(game, 0x620) != 0) return;

    check_selected_site_owner(s, host);
    check_inferior_failure(s, host);
    step_scrolls_and_arrows(s, dt, host);
    update_hints(s, host);
    check_selected_unit_changed(s, host);
    sync_record_and_pulse(s, host);

    alignas(8) std::uint8_t item[0x58];  // [ESP+D0h], EH state 36h until 0067AF05
    host.air_ops_slot_record_construct_006c1960(item, 0);
    check_selected_unit_retired(s, host);
    place_unit_markers(s, host);
    check_current_site_retired(s, host);
    if (step_health_and_fade(s, dt, host)) {
        handle_back_to_deck(s, item, host);
        handle_return_from_deck(s, host);
        handle_prev_next(s, item, host);
        handle_cycle_loadout(s, host);
        handle_back_to_sites(s, host);
        // 0067A3DB: the active hint name lives at [ESP+7Ch] (EH 60h) until 0067AED4, where it
        // is released inline.
        NativeStr active{};
        host.award_tracker_copy_active_hint_name_0068e5b0(host.award_tracker_get_singleton_004e1ca0(), &active);
        if (active.length == 0 ||
            !host.award_tracker_is_active_hint_blocking_00690b00(host.award_tracker_get_singleton_004e1ca0(), &active))
            handle_pad_buttons(s, item, host);
        if (host.input_action_was_pressed_004c43c0(host.game_00e188a8(), 0x100)) {  // 0067A664
            at<std::int32_t>(s, 0x5C) = 1;
            host.support_play_sp_to_unit_sound_0065f2c0(s);
        }
        handle_order_slot(s, item, host);
        handle_spectate(s, item, host);
        release_inline(active, host);
    }
    host.air_ops_slot_record_destruct_006bc690(item);  // 0067AEFE
}

} // namespace bsp::pool_pool21_cc13
