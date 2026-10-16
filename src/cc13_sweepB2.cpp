// cc13_sweepB2: continuation of the segment 15-51 leftover sweep (packet
// cc13-sweepB2, docs/CC13_SWEEPB2.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB): float expressions follow the image's operation
// order with a float store wherever the image stores to a float slot. That is
// exact at the main thread's 24-bit precision control; at 53 or 64 bits the
// image keeps unstored intermediates wider. EH frames are noted, not modelled.

#include "bsp/cc13_sweepB2.hpp"

#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

// A double from its image bit pattern (exact constants).
double f64_bits(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

void* image_pointer(Cc13SweepBHost& host, std::uint32_t address) {
    return *static_cast<void**>(host.image_global(address));
}

// 00F1B03C: the image's empty C string.
const char* empty_c_string(Cc13SweepBHost& host) {
    return static_cast<const char*>(host.image_global(0x00F1B03C));
}

// The game-name source both 0076F0E0 and 00775160 read: the native string at
// game+6A0h when its length word is non-zero (unsigned JA), else game+68Ch;
// its data pointer +4h, or the empty string 00F1B03C when that is null.
const char* game_name_text(void* game, Cc13SweepBHost& host) {
    std::uint8_t* name = static_cast<std::uint8_t*>(game) + 0x6A0;
    if (at<std::uint32_t>(game, 0x6A0) == 0) name = static_cast<std::uint8_t*>(game) + 0x68C;
    const char* text = at<const char*>(name, 0x4);
    return text != nullptr ? text : empty_c_string(host);
}

// The leftover of a compiled-out report in 0078DBA0 / 0078DCF0: a native
// string of the image text is sized (0041DD40(length, 1)), filled with length
// + 1 bytes and released at once to the sized pool with length + 1 bytes.
void discard_error_text(std::uint32_t text_address, std::uint32_t length, Cc13SweepBHost& host) {
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) {
        const std::uint32_t bytes = text.length + 1;
        host.crt_memcpy_00bf7680(text.data, host.image_global(text_address), bytes);
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, bytes, 1);
    }
}

// A by-value reference pointer's destructor: null -> nothing; else
// InterlockedDecrement(ref+4h) == 0 -> the object's vslot 0().
void release_reference(void* reference, Cc13SweepBHost& host) {
    if (reference == nullptr) return;
    if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(reference, 0x4)) == 0) {
        host.refcounted_vcall_00_destroy(reference);
    }
}

// A temporary native string of an image text: 0041DD40(length, 1), memcpy of
// length + 1 bytes when data is non-null, the use, then the data back to the
// sized pool with length + 1 bytes (the length re-read after the use).
template <class Use>
void with_image_string(std::uint32_t text_address, std::uint32_t length, Cc13SweepBHost& host, Use use) {
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, host.image_global(text_address), text.length + 1);
    use(&text);
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

}  // namespace

// ===========================================================================
// Segment 18
// ===========================================================================

// 0056B610 BSP_MultiSessionBrowserScreen_MoveSelection (hypothesis, ledger name).
// ABI: __thiscall(ECX screen)(scroller, listbox, clip, byte keep), RET 10h.
// Callers 005787E0 and 0057B600; src/menu_screens_2.cpp has only the hook
// scroll_follow_0056b610. Evidence: disk bytes 0056B610..0056B84C.
// - +2C4h = count [listbox+104h]; +2C8h = 0; clip top-left P = 00AA6750(clip)
//   - 00AA6740(clip) * pivot (+18h/+1Ch), each component stored as float.
// - Per row i < count (re-read): row = 00A9BE00(listbox, i); its top-left R
//   likewise; visible when R.y > P.y and P.y + clip height > R.y + row height
//   (FCOMI/JBE, FCOMPI/JBE); visible rows lower +2C4h to i when i is below it,
//   else raise +2C8h to i when above it; row vslot 60h(visible).
// - scroller byte +48h, scroller +20h and keep all clear: with a selection
//   (00A9C920 != -1) below +2C4h, 00683680(scroller, height of 00425E50(listbox));
//   otherwise with a selection above +2C8h, 00683700(scroller, that height).
// - Otherwise: the bound crossed by the selection (+2C4h when below it, else
//   +2C8h when above it), unless -1, is selected through 00A9C8F0 and stored at +2CCh.
// 00A9C920 is called again before every test, as the image does.
void cc13_sweepB2_session_browser_move_selection_0056b610(void* screen, void* scroller, void* listbox, void* clip,
                                                          std::uint8_t keep, Cc13SweepB2Host& host) {
    auto selected = [&] { return host.listbox_selected_index_00a9c920(listbox); };
    at<std::int32_t>(screen, 0x2C4) = at<std::int32_t>(listbox, 0x104);
    at<std::int32_t>(screen, 0x2C8) = 0;
    float p[3];
    host.widget_get_resolved_position_00aa6750(clip, p);
    const float* clip_size = host.widget_get_size_00aa6740(clip);
    const float clip_w = clip_size[0];
    const float clip_h = clip_size[1];
    p[0] = p[0] - clip_w * at<float>(clip, 0x18);
    p[1] = p[1] - clip_h * at<float>(clip, 0x1C);
    for (std::int32_t i = 0; i < at<std::int32_t>(listbox, 0x104); ++i) {
        void* row = host.listbox_row_00a9be00(listbox, i);
        float r[3];
        host.widget_get_resolved_position_00aa6750(row, r);
        const float* row_size = host.widget_get_size_00aa6740(row);
        const float row_w = row_size[0];
        const float row_h = row_size[1];
        r[0] = r[0] - row_w * at<float>(row, 0x18);
        r[1] = r[1] - row_h * at<float>(row, 0x1C);
        std::int32_t visible = 0;
        if (r[1] > p[1] && p[1] + clip_h > r[1] + row_h) {
            if (i < at<std::int32_t>(screen, 0x2C4)) {
                at<std::int32_t>(screen, 0x2C4) = i;
            } else if (i > at<std::int32_t>(screen, 0x2C8)) {
                at<std::int32_t>(screen, 0x2C8) = i;
            }
            visible = 1;
        }
        host.widget_vcall_60(row, visible);
    }
    if (at<std::uint8_t>(scroller, 0x48) == 0 && at<std::uint32_t>(scroller, 0x20) == 0 && keep == 0) {
        if (selected() != -1 && selected() < at<std::int32_t>(screen, 0x2C4)) {
            const float* size = host.widget_get_size_00aa6740(host.listbox_selected_row_00425e50(listbox));
            host.gui_scroller_scroll_toward_start_00683680(scroller, size[1]);
            return;
        }
        if (selected() == -1) return;
        if (selected() > at<std::int32_t>(screen, 0x2C8)) {
            const float* size = host.widget_get_size_00aa6740(host.listbox_selected_row_00425e50(listbox));
            host.gui_scroller_scroll_toward_end_00683700(scroller, size[1]);
        }
        return;
    }
    std::int32_t bound;
    if (selected() != -1) {
        bound = at<std::int32_t>(screen, 0x2C4);
        if (selected() < bound) {
            if (bound == -1) return;
            host.listbox_select_00a9c8f0(listbox, bound);
            at<std::int32_t>(screen, 0x2CC) = bound;
            return;
        }
    }
    if (selected() == -1) return;
    bound = at<std::int32_t>(screen, 0x2C8);
    if (selected() <= bound) return;
    if (bound == -1) return;
    host.listbox_select_00a9c8f0(listbox, bound);
    at<std::int32_t>(screen, 0x2CC) = bound;
}

// ===========================================================================
// Segment 19
// ===========================================================================

// 005787E0 BSP_MultiSessionBrowserScreen_OnListboxSelectionChanged (hypothesis, ledger name).
// ABI: __thiscall(ECX listener = screen+8h)(unused, list), RET 8; slot +8h of
// vtable 00CEF120, no rel32 callers. Evidence: disk bytes 005787E0..005789BB.
// - [+24h] vslot 38h false: 0056AA70(screen, list).
// - Else, when byte +E9h is clear: for k = 0..8: row = 00A9BE00([+2Ch], k);
//   state = k == 00A9C920([+2Ch]) ? 2 : 0; for "scroll_right_Icon" (00CEF0D0, 11h)
//   then "scroll_left_Icon" (00CEF0BC, 10h), each a temporary native string:
//   00AA7E00(row, &name, 1) vslot 88h(state, 0, 1.0f).
// - Then (all paths): list == [+28h] -> +260h = 10.0f (00CE38B8), 00577650(screen),
//   0056B610(screen, listener+1DCh, [+28h], [+7Ch], 0).
// EH frame 00C6F500 (the names) is not modelled.
void cc13_sweepB2_session_browser_on_listbox_selection_changed_005787e0(void* listener, std::uint32_t unused,
                                                                        std::uint32_t list, Cc13SweepB2Host& host) {
    (void)unused;
    std::uint8_t* self = static_cast<std::uint8_t*>(listener);
    void* screen = self - 8;
    if (!host.widget_vcall_38_is_shown(at<void*>(listener, 0x24))) {
        host.session_browser_focus_list_0056aa70(screen, list);
    } else if (at<std::uint8_t>(listener, 0xE9) == 0) {
        for (std::int32_t k = 0; k < 9; ++k) {
            void* row = host.listbox_row_00a9be00(at<void*>(listener, 0x2C), k);
            const std::int32_t state = k == host.listbox_selected_index_00a9c920(at<void*>(listener, 0x2C)) ? 2 : 0;
            with_image_string(0x00CEF0D0, 0x11, host, [&](Cc13SweepBNativeString* name) {
                host.widget_vcall_88(host.widget_find_child_00aa7e00(row, name, 1), state, 0, 1.0f);
            });
            with_image_string(0x00CEF0BC, 0x10, host, [&](Cc13SweepBNativeString* name) {
                host.widget_vcall_88(host.widget_find_child_00aa7e00(row, name, 1), state, 0, 1.0f);
            });
        }
    }
    if (list == at<std::uint32_t>(listener, 0x28)) {
        at<float>(listener, 0x260) = 10.0f;
        host.session_browser_refresh_row_details_00577650(screen);
        host.session_browser_move_selection_0056b610(screen, self + 0x1DC, at<std::uint32_t>(listener, 0x28),
                                                     at<std::uint32_t>(listener, 0x7C), 0);
    }
}

// ===========================================================================
// Segment 20
// ===========================================================================

// 00588A80 BSP_MainMenu_ActivateTopLevelItem (hypothesis, ledger name).
// ABI: __thiscall(ECX screen), RET. Callers 00598B60 and 005993A0. The
// item table in include/bsp/main_menu_screen.hpp (kMainMenuItemActions)
// describes the same dispatch; this is the body. Evidence: disk bytes 00588A80..00588C6F.
// - sel = 00A9C990([+1B8h]); [00E194C4] = sel.
// - 0: 00584F50(screen). 1: 005853C0(screen).
// - 3: [00E19650] = 0; 004D7920(game, 6); 004D7920(game, 14h) (game re-read).
// - 2: 006B8AD0([[game+1A08h]+4], "luaLoadControlFunctionNames()" 00CEC110, 0, 0, 2); 005886F0(screen).
// - 4: XShowGuideUI(0) (00A4D44C).
// - 5: 00585B40() and 00585810(CL = 0) -> XShowMarketplaceUI(00A3EAC0([00F8ABE8]), 0, 0, -1).
// - 6: strings A = "" (00CE3A0C) and B = "FE_pc.main_quit_confirm" (00CE76D4)
//   through 0041E870; a by-value string C = "" built in the argument area;
//   00531B00(00425D10(), 6, &B, 1, 0057D3B0, 0, &A, 0.0f, 0, C, 1) (consumes C);
//   B then A destroyed (0041DD20); 00530C20(00425D10()).
// - Anything else (and 5 when a gate fails): nothing more.
// EH frame 00C6FE58 (the strings) is not modelled.
void cc13_sweepB2_main_menu_activate_top_level_item_00588a80(void* screen, Cc13SweepB2Host& host) {
    const std::int32_t selection = host.listbox_selected_data_00a9c990(at<void*>(screen, 0x1B8));
    *static_cast<std::int32_t*>(host.image_global(0x00E194C4)) = selection;
    switch (selection) {
    case 0:
        host.main_menu_build_single_player_page_00584f50(screen);
        return;
    case 1:
        host.main_menu_build_multiplayer_page_005853c0(screen);
        return;
    case 3:
        *static_cast<std::uint32_t*>(host.image_global(0x00E19650)) = 0;
        host.game_request_state_004d7920(host.global_00e188a8(), 6);
        host.game_request_state_004d7920(host.global_00e188a8(), 0x14);
        return;
    case 2: {
        void* machine = at<void*>(at<void*>(host.global_00e188a8(), 0x1A08), 0x4);
        host.lua_machine_run_string_006b8ad0(machine, static_cast<const char*>(host.image_global(0x00CEC110)), 0,
                                             0, 2);
        host.main_menu_build_tactical_library_page_005886f0(screen);
        return;
    }
    case 4:
        host.x_show_guide_ui_00a4d44c(0);
        return;
    case 5:
        if (host.main_menu_check_connection_gate_00585b40() && host.main_menu_check_account_gate_00585810(0)) {
            const std::uint32_t user = host.xenon_trivial_00a3eac0(image_pointer(host, 0x00F8ABE8));
            host.x_show_marketplace_ui_00a4d446(user, 0, 0, 0xFFFFFFFFu);
        }
        return;
    case 6: {
        Cc13SweepBNativeString a = {0, nullptr};
        host.native_string_assign_0041e870(&a, static_cast<const char*>(host.image_global(0x00CE3A0C)));
        Cc13SweepBNativeString b = {0, nullptr};
        host.native_string_assign_0041e870(&b, static_cast<const char*>(host.image_global(0x00CE76D4)));
        Cc13SweepBNativeString c = {0, nullptr};
        host.native_string_assign_0041e870(&c, static_cast<const char*>(host.image_global(0x00CE3A0C)));
        void* menu = host.menu_command_screen_get_00425d10();
        host.menu_prompt_raise_00531b00(menu, 6, &b, 1, 0x0057D3B0, 0, &a, 0.0f, 0, c, 1);
        host.native_string_destroy_0041dd20(&b);
        host.native_string_destroy_0041dd20(&a);
        host.frontend_prompt_focus_last_navigation_00530c20(host.menu_command_screen_get_00425d10());
        return;
    }
    default:
        return;
    }
}

// 005C5470 BSP_MissionRecordHeader_Assign (hypothesis, ledger name).
// ABI: __thiscall(ECX dest)(const source*), RET 4, EAX = dest. Sole caller
// 005C7B80. Evidence: disk bytes 005C5470..005C55FD. The member-wise copy
// assignment of the record head:
// - byte +0h; the native string +4h unless it is the same object (0041DD40
//   (dest, source length, 1), then memcpy of dest's new length when the
//   source length is non-zero); dwords +0Ch and +10h;
// - twenty string vectors at +14h, +24h, ... +144h through 00506C80.
void* cc13_sweepB2_mission_record_header_assign_005c5470(void* dest, const void* source, Cc13SweepB2Host& host) {
    void* src = const_cast<void*>(source);
    at<std::uint8_t>(dest, 0x0) = at<std::uint8_t>(src, 0x0);
    auto* dst_string = reinterpret_cast<Cc13SweepBNativeString*>(static_cast<std::uint8_t*>(dest) + 0x4);
    auto* src_string = reinterpret_cast<Cc13SweepBNativeString*>(static_cast<std::uint8_t*>(src) + 0x4);
    if (dst_string != src_string) {
        host.native_string_resize_0041dd40(dst_string, src_string->length, 1);
        if (src_string->length != 0) host.crt_memcpy_00bf7680(dst_string->data, src_string->data, dst_string->length);
    }
    at<std::uint32_t>(dest, 0xC) = at<std::uint32_t>(src, 0xC);
    at<std::uint32_t>(dest, 0x10) = at<std::uint32_t>(src, 0x10);
    for (std::size_t off = 0x14; off <= 0x144; off += 0x10) {
        host.string_vector_assign_00506c80(static_cast<std::uint8_t*>(dest) + off, static_cast<std::uint8_t*>(src) + off);
    }
    return dest;
}

// 005C51F0 BSP_MissionSideBlock_CopyConstruct (hypothesis, ledger name).
// ABI: __thiscall(ECX dest)(const source*), RET 4, EAX = dest. Referenced only
// as an address at 005C85F1 inside 005C81F0 (callsite_census: 1 absolute, 0
// rel32). Evidence: disk bytes 005C51F0..005C5413. The copy-constructor twin
// of 005C5470: byte +0h; native string +4h set to {0, null} and, unless it is
// the source's own string, sized by 0041DD40(source length, 1) and filled by
// memcpy of its new length when the source length is non-zero; dwords +0Ch and
// +10h; twenty string vectors +14h..+144h copy-constructed by 00557BC0.
// EH frame 00C72DC0 (states 0..13h) is not modelled.
void* cc13_sweepB2_mission_side_block_copy_construct_005c51f0(void* dest, const void* source, Cc13SweepB2Host& host) {
    void* src = const_cast<void*>(source);
    at<std::uint8_t>(dest, 0x0) = at<std::uint8_t>(src, 0x0);
    auto* dst_string = reinterpret_cast<Cc13SweepBNativeString*>(static_cast<std::uint8_t*>(dest) + 0x4);
    auto* src_string = reinterpret_cast<Cc13SweepBNativeString*>(static_cast<std::uint8_t*>(src) + 0x4);
    dst_string->length = 0;
    dst_string->data = nullptr;
    if (dst_string != src_string) {
        host.native_string_resize_0041dd40(dst_string, src_string->length, 1);
        if (src_string->length != 0) host.crt_memcpy_00bf7680(dst_string->data, src_string->data, dst_string->length);
    }
    at<std::uint32_t>(dest, 0xC) = at<std::uint32_t>(src, 0xC);
    at<std::uint32_t>(dest, 0x10) = at<std::uint32_t>(src, 0x10);
    for (std::size_t off = 0x14; off <= 0x144; off += 0x10) {
        host.string_vector_copy_construct_00557bc0(static_cast<std::uint8_t*>(dest) + off,
                                                   static_cast<std::uint8_t*>(src) + off);
    }
    return dest;
}

// 005C5860 BSP_MissionTree_ReadStringVector (hypothesis, ledger name).
// ABI: __thiscall(ECX reader)(int kind, const char* key, vector<NativeString>*
// out), RET 0Ch (EBP frame, ESP aligned to 8). Callers 005C5DA0 and 005C6A70.
// Evidence: disk bytes 005C5860..005C597F. Vector: first +4h, last +8h.
// - Checked-iterator tests: first > last (unsigned) -> 00BF6713, twice (the
//   second after re-reading first); then 004954F0(out, &result, {out, first},
//   {out, last}) empties it.
// - reader vslot 4h(kind, key); for i = 1, 2, ...: while vslot 14h({1, i}):
//   a local string {0, null}; vslot 10h({1, i}, {0, &string}); 00450540(out,
//   &string); the string's data back to the sized pool (length + 1) when non-null.
// - reader vslot 8h().
// EH frame 00C72DF8 (the local string) is not modelled.
void cc13_sweepB2_mission_tree_read_string_vector_005c5860(void* reader, std::int32_t kind, const char* key,
                                                           void* out, Cc13SweepB2Host& host) {
    void* last = at<void*>(out, 0x8);
    if (reinterpret_cast<std::uintptr_t>(at<void*>(out, 0x4)) > reinterpret_cast<std::uintptr_t>(last)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* first = at<void*>(out, 0x4);
    if (reinterpret_cast<std::uintptr_t>(first) > reinterpret_cast<std::uintptr_t>(at<void*>(out, 0x8))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    alignas(4) std::uint8_t result[8];
    host.string_vector_erase_range_004954f0(out, result, out, first, out, last);
    host.lua_reader_vcall_04_enter(reader, kind, key);
    for (std::uint32_t i = 1;; ++i) {
        if (!host.lua_reader_vcall_14_has(reader, Cc13SweepB2Descriptor{1, reinterpret_cast<const void*>(
                                                                              static_cast<std::uintptr_t>(i))})) {
            break;
        }
        Cc13SweepBNativeString text = {0, nullptr};
        host.lua_reader_vcall_10_read(reader,
                                      Cc13SweepB2Descriptor{1, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(i))},
                                      Cc13SweepB2Descriptor{0, &text});
        host.string_vector_append_00450540(out, &text);
        if (text.data != nullptr) {
            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
        }
    }
    host.lua_reader_vcall_08_leave(reader);
}

// 005C5990 BSP_MissionTreeScreen_CollectMissingPrerequisites (hypothesis, ledger name).
// ABI: __thiscall(ECX screen)(const NativeString* name), RET 4. No rel32 callers
// (docs/PROFILE_UNLOCK_PREDICATE.md). Evidence: disk bytes 005C5990..005C5B2F;
// the "gap" after the free at 005C5B19 is the cdecl ADD ESP, 4 of a normal fall-through.
// - profile = game+650h. A local NativeString vector V (first +4h, last +8h,
//   end +0Ch; its +0h word is not written) starts with a copy of name (00450540).
// - Worklist over V (bounds re-read every pass, i kept in the argument slot):
//   005C3470(screen, &found{a, b}, &V[i]); a == -1 -> next. Else checked
//   indexes (00BF6713 when the vector is null or the index is out of range):
//   tree entry = [screen+18h] + b*34h; record = [entry+28h] + a*434h. For each
//   prerequisite p of the record's vector +7Ch/+80h (8-byte strings; the second
//   bounds test in the loop repeats the first and cannot fire): if
//   !007FC4C0(profile, p) -> 004D0640(profile+7Ch, &result, p); always 00450540(&V, p).
// - Exit: V.first non-null -> 00432050(V.first, V.last, &V, i) then free(V.first) (00BF65AC).
// EH frame 00C72E18 (V) is not modelled.
void cc13_sweepB2_mission_tree_collect_missing_prerequisites_005c5990(void* screen, const void* name,
                                                                      Cc13SweepB2Host& host) {
    std::uint8_t* profile = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650;
    alignas(4) std::uint8_t vector[0x10] = {};
    host.string_vector_append_00450540(vector, name);
    std::uint32_t i = 0;
    for (;;) {
        std::uint8_t* first = at<std::uint8_t*>(vector, 0x4);
        if (first == nullptr) break;
        if (i >= static_cast<std::uint32_t>((at<std::uint8_t*>(vector, 0x8) - first) >> 3)) break;
        std::int32_t found[2];
        host.mission_tree_find_mission_by_name_005c3470(screen, found, first + i * 8);
        const std::int32_t a = found[0];
        if (a != -1) {
            const std::uint32_t b = static_cast<std::uint32_t>(found[1]);
            std::uint8_t* trees = at<std::uint8_t*>(screen, 0x18);
            if (trees == nullptr || b >= static_cast<std::uint32_t>((at<std::uint8_t*>(screen, 0x1C) - trees) / 0x34)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            std::uint8_t* missions = at<std::uint8_t*>(screen, 0x18) + b * 0x34 + 0x24;
            std::uint8_t* records = at<std::uint8_t*>(missions, 0x4);
            if (records == nullptr ||
                static_cast<std::uint32_t>(a) >= static_cast<std::uint32_t>((at<std::uint8_t*>(missions, 0x8) - records) / 0x434)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            std::uint8_t* record = at<std::uint8_t*>(missions, 0x4) + static_cast<std::uint32_t>(a) * 0x434;
            for (std::uint32_t j = 0;; ++j) {
                std::uint8_t* p_first = at<std::uint8_t*>(record, 0x7C);
                if (p_first == nullptr) break;
                if (j >= static_cast<std::uint32_t>((at<std::uint8_t*>(record, 0x80) - p_first) >> 3)) break;
                const void* prerequisite = at<std::uint8_t*>(record, 0x7C) + j * 8;
                if (!host.profile_is_unlock_satisfied_007fc4c0(profile, prerequisite)) {
                    alignas(4) std::uint8_t result[8];
                    host.native_string_set_insert_004d0640(profile + 0x7C, result, prerequisite);
                }
                host.string_vector_append_00450540(vector, prerequisite);
            }
        }
        ++i;
    }
    if (at<void*>(vector, 0x4) != nullptr) {
        host.native_string_range_destroy_00432050(at<void*>(vector, 0x4), at<void*>(vector, 0x8), vector, i);
        host.crt_operator_delete_00bf65ac(at<void*>(vector, 0x4));
    }
}

// 005A4020 BSP_HudMapScreen_SetNodeVisibleByName (hypothesis, ledger name).
// ABI: __thiscall(ECX screen)(NativeString name by value, byte show), RET 0Ch;
// the name is consumed. Sole caller 008C03B0 (Lua binding). Evidence: disk
// bytes 005A4020..005A41F3.
// - factor = show ? 1.0f (00D7A24C) : 0.0f (stored in the flag's argument slot).
// - For the node-pointer vectors at +500h (first +504h, last +508h) and +510h
//   (first +514h, last +518h): checked-iterator tests (first > last, twice;
//   element >= re-read last before the dereference and again after the call)
//   call 00BF6713; for each element: a copy of name (0041DD40(length, 1), memcpy
//   of the copy's length when name's length is non-zero) and 005A3EF0(node,
//   copy, factor), which consumes the copy (sweepB's reconstruction, called
//   directly). The loop end is the last pointer read before the loop.
// - name's data back to the sized pool with the entry length + 1 when non-null.
// EH frame 00C716A8 (the by-value name) is not modelled.
void cc13_sweepB2_hud_map_set_node_visible_by_name_005a4020(void* screen, Cc13SweepBNativeString name, bool show,
                                                            Cc13SweepB2Host& host) {
    const std::uint32_t length = name.length;
    const float factor = show ? 1.0f : 0.0f;
    for (std::size_t base : {0x500u, 0x510u}) {
        std::uint8_t* vec = static_cast<std::uint8_t*>(screen) + base;
        void** element = at<void**>(vec, 0x4);
        if (reinterpret_cast<std::uintptr_t>(element) > reinterpret_cast<std::uintptr_t>(at<void**>(vec, 0x8))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void** last = at<void**>(vec, 0x8);
        if (reinterpret_cast<std::uintptr_t>(at<void**>(vec, 0x4)) > reinterpret_cast<std::uintptr_t>(last)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        for (; element != last; ++element) {
            Cc13SweepBNativeString copy = {0, nullptr};
            host.native_string_resize_0041dd40(&copy, length, 1);
            if (length != 0) host.crt_memcpy_00bf7680(copy.data, name.data, copy.length);
            if (element >= at<void**>(vec, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
            cc13_sweepB_node_set_child_factor_by_name_005a3ef0(*element, copy, factor, host);
            if (element >= at<void**>(vec, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
        }
    }
    if (name.data != nullptr) host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, length + 1, 1);
}

// 0059D7B0 BSP_InterfaceView_DrawGroundOrtho (hypothesis, ledger name).
// ABI: __thiscall(ECX view = [00E198C4]+54h), RET; the tail JMPs to 0093D250
// with ECX = view+8h. Caller 004CA440 BSP_Game_Render (interface level 2);
// host call src/game_render_frame.cpp render_interface_ortho_view_0059d7b0
// (hook only). Sibling of sweepB's 00535430. Evidence: disk bytes
// 0059D7B0..0059D938; x87 operand directions from tools/x87_dc_form_scan.py
// (DC C9 / DC CA write st(1)/st(2); DE E3 / DE E1 are st(i) = st0 - st(i)).
// - Look-at into a 4x4 local: eye (x, 10000.0f [00CE3D64], z), target (x, 0, z),
//   up (0, 0, 1.0f [00D7A24C]) with x = +54Ch, z = +550h; 00B71490 on camera +10h.
// - Ortho on camera +10h: (-w*0.5, w*0.5, -h*0.5, h*0.5, 1.0, 15000.0f
//   [00CED55C]), w = +30h, h = +34h, 0.5 = double 00D7A280.
// - Shadow box: W2 = (w*0.5)*2.0 and H2 = (h*0.5)*2.0 (double 00D7A308)
//   unstored; min = (x - W2, 0, z - H2), max = (x + W2, 4500.0f [00CEFFE0],
//   z + H2), each component stored as float; x and z are re-read from +54Ch /
//   +550h for this part. 00A8E860(00B7AAB0([view+18h]), [view+10h], &min, &max).
void cc13_sweepB2_interface_view_draw_ground_ortho_0059d7b0(void* view, Cc13SweepB2Host& host) {
    const float x = at<float>(view, 0x54C);
    const float z = at<float>(view, 0x550);
    const float target[3] = {x, 0.0f, z};
    const float eye[3] = {x, 10000.0f, z};
    float look_at[16];
    float* matrix = host.matrix_build_look_at_00b63f10(look_at, eye, target, 0.0f, 0.0f, 1.0f);
    host.camera_set_view_matrix_00b71490(at<void*>(view, 0x10), matrix);

    const float h = at<float>(view, 0x34);
    const float w = at<float>(view, 0x30);
    const float top = static_cast<float>(h * 0.5);
    const float bottom = static_cast<float>(-h * 0.5);
    const float right = static_cast<float>(w * 0.5);
    const float left = static_cast<float>(-w * 0.5);
    host.camera_set_gui_ortho_00aa2020(at<void*>(view, 0x10), left, right, bottom, top, 1.0f, 15000.0f);

    const float w2 = static_cast<float>(at<float>(view, 0x30) * 0.5 * 2.0);
    const float h2 = static_cast<float>(at<float>(view, 0x34) * 0.5 * 2.0);
    const float bx = at<float>(view, 0x54C);
    const float bz = at<float>(view, 0x550);
    const float max_xyz[3] = {bx + w2, 4500.0f, bz + h2};
    const float min_xyz[3] = {bx - w2, 0.0f, bz - h2};
    void* owner = host.light_shadow_map_owner_00b7aab0(at<void*>(view, 0x18));
    host.shadow_owner_render_ground_ortho_00a8e860(owner, at<void*>(view, 0x10), min_xyz, max_xyz);
    host.simple3d_scene_submit_0093d250(static_cast<std::uint8_t*>(view) + 0x8);
}

namespace {

// The inline std::map / std::set constructor: head = buy(container), its
// isnil byte = 1, parent/left/right = head (each through a re-read of +4h),
// size +8h = 0.
template <class Buy>
void construct_tree(void* container, std::size_t isnil_offset, Buy buy) {
    at<void*>(container, 0x4) = buy(container);
    at<std::uint8_t>(at<void*>(container, 0x4), isnil_offset) = 1;
    at<void*>(at<void*>(container, 0x4), 0x4) = at<void*>(container, 0x4);
    at<void*>(at<void*>(container, 0x4), 0x0) = at<void*>(container, 0x4);
    at<void*>(at<void*>(container, 0x4), 0x8) = at<void*>(container, 0x4);
    at<std::uint32_t>(container, 0x8) = 0;
}

void zero_words(void* base, std::initializer_list<std::size_t> offsets) {
    for (std::size_t off : offsets) at<std::uint32_t>(base, off) = 0;
}

}  // namespace

// 005AAEF0 BSP_InGameInterfaceScreen_Construct_005aaef0 (hypothesis, ledger name).
// ABI: __thiscall(ECX screen), RET, EAX = screen. Caller 0068CC70
// BSP_InGameInterface_Init (src/ingame_interface.cpp lists it as the +54h,
// 614h-byte screen 4Ch with vtable 00CF0134). Evidence: disk bytes 005AAEF0..005AB136.
// - 004F7180(screen); 0093CB10(screen+8h); +64h = 00CE3CD4, +68h/+6Ch/+70h = 0,
//   byte +74h = 0; [+8h] = 00CF0130; [+0h] = 00CF0134; +64h = 00CF0118.
// - Tree +78h (005A0220, isnil +11h); +D8h, +DCh, +E0h, +F4h, +F8h, +FCh = 0.
// - List +498h: +49Ch = 005356C0(+498h), +4A0h = 0; nine words +4B8h..+4E8h = 0.
// - Tree +4F4h (005359A0, isnil +15h); thirteen words +504h..+588h = 0.
// - Tree +600h (005A0270, isnil +15h); +520h, +530h, +52Ch, +538h, +534h = 0,
//   byte +528h = 0, +524h = 0, +4F0h = 0, byte +5F0h = 0, +548h = -1,
//   +108h..+11Ch = 0, +5E4h = 0.001f (00D7A23C), +5DCh = 2.0f (00CE3958),
//   +53Ch = 0, +540h = 0.0f.
// EH frame 00C71CA1 (states 0, 2, 5, 9, 0Fh) is not modelled.
void* cc13_sweepB2_ingame_interface_screen_construct_005aaef0(void* screen, Cc13SweepB2Host& host) {
    std::uint8_t* s = static_cast<std::uint8_t*>(screen);
    host.frontend_screen_construct_004f7180(screen);
    host.simple3d_scene_construct_0093cb10(s + 0x8);
    at<std::uint32_t>(screen, 0x64) = 0x00CE3CD4;
    zero_words(screen, {0x68, 0x6C, 0x70});
    at<std::uint8_t>(screen, 0x74) = 0;
    at<std::uint32_t>(screen, 0x8) = 0x00CF0130;
    at<std::uint32_t>(screen, 0x0) = 0x00CF0134;
    at<std::uint32_t>(screen, 0x64) = 0x00CF0118;
    construct_tree(s + 0x78, 0x11, [&](void* c) { return host.stl_buy_head_node_005a0220(c); });
    zero_words(screen, {0xD8, 0xDC, 0xE0, 0xF4, 0xF8, 0xFC});
    at<void*>(screen, 0x49C) = host.stl_buy_head_node_005356c0(s + 0x498);
    at<std::uint32_t>(screen, 0x4A0) = 0;
    zero_words(screen, {0x4B8, 0x4BC, 0x4C0, 0x4CC, 0x4D0, 0x4D4, 0x4E0, 0x4E4, 0x4E8});
    construct_tree(s + 0x4F4, 0x15, [&](void* c) { return host.stl_buy_head_node_005359a0(c); });
    zero_words(screen, {0x504, 0x508, 0x50C, 0x514, 0x518, 0x51C, 0x544, 0x570, 0x574, 0x578, 0x580, 0x584, 0x588});
    construct_tree(s + 0x600, 0x15, [&](void* c) { return host.stl_buy_head_node_005a0270(c); });
    zero_words(screen, {0x520, 0x530, 0x52C, 0x538, 0x534});
    at<std::uint8_t>(screen, 0x528) = 0;
    zero_words(screen, {0x524, 0x4F0});
    at<std::uint8_t>(screen, 0x5F0) = 0;
    at<std::int32_t>(screen, 0x548) = -1;
    zero_words(screen, {0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C});
    at<float>(screen, 0x5E4) = cc13_sweepB_f32(0x3A83126Fu);
    at<float>(screen, 0x5DC) = 2.0f;
    at<std::uint32_t>(screen, 0x53C) = 0;
    at<float>(screen, 0x540) = 0.0f;
    return screen;
}

// ===========================================================================
// Segment 21
// ===========================================================================

// 005CCE70 BSP_HudEngineMovieScreen_Register (hypothesis, ledger name).
// ABI: __thiscall(ECX screen), RET. Vtable +10h of HUD screen slot 38h
// (manager +A0h); no rel32 callers. src/hud_screens.cpp only lists it in a
// metadata table (kPages38 / kWidgets38). Evidence: disk bytes 005CCE70..005CD027.
// - 004F71D0(screen); bytes +9h = +8h = 0.
// - +18h = 00AA5840(004C12B0(), "GUI_movie" (00CF1944, 9), 1, 0).
// - +10h = 00AA7E00([+18h], "up_Icon" (00CF193C, 7), 1);
//   +14h = 00AA7E00([+18h], "down_Icon" (00CF1930, 9), 1).
// - +1Ch = [[0109CF04]+0Dh] ? 0.14f (00CE81A4) : 0.18f (00CE8198).
// Each name is a temporary native string (with_image_string). EH frame
// 00C73698 (states 0..2 around the strings) is not modelled.
void cc13_sweepB2_hud_engine_movie_screen_register_005cce70(void* screen, Cc13SweepB2Host& host) {
    host.frontend_screen_register_004f71d0(screen);
    at<std::uint8_t>(screen, 0x9) = 0;
    at<std::uint8_t>(screen, 0x8) = 0;
    with_image_string(0x00CF1944, 9, host, [&](Cc13SweepBNativeString* name) {
        void* manager = host.gui_manager_get_004c12b0();
        at<void*>(screen, 0x18) = host.gui_manager_load_page_00aa5840(manager, name, 1, 0);
    });
    with_image_string(0x00CF193C, 7, host, [&](Cc13SweepBNativeString* name) {
        at<void*>(screen, 0x10) = host.widget_find_child_00aa7e00(at<void*>(screen, 0x18), name, 1);
    });
    with_image_string(0x00CF1930, 9, host, [&](Cc13SweepBNativeString* name) {
        at<void*>(screen, 0x14) = host.widget_find_child_00aa7e00(at<void*>(screen, 0x18), name, 1);
    });
    const bool alt = at<std::uint8_t>(image_pointer(host, 0x0109CF04), 0xD) != 0;
    at<float>(screen, 0x1C) = alt ? cc13_sweepB_f32(0x3E0F5C29u) : cc13_sweepB_f32(0x3E3851ECu);
}

// ===========================================================================
// Segment 22
// ===========================================================================

// 005D20A0 BSP_MultiErrorScreen_LayoutButtons (hypothesis, ledger name).
// ABI: __thiscall(ECX screen), RET. Sole caller 005D24F0; host call
// src/frontend_seg15_22_cc13.cpp multi_error_layout_buttons_005d20a0 (hook
// only). Evidence: disk bytes 005D20A0..005D2216.
// Buttons A = +20h, B = +28h, C = +24h, each tested through vslot 38h (twice:
// once to measure, once to place). Width of a button = (float)(+114h / 960.0)
// (double 00CEC380).
// - total = 0, n = 1; A shown: total = (float)(wA + 0.0) (double 00D7A258),
//   n = 2; B shown: n += 1, total = (float)(wB + total); C likewise.
// - gap = (float)((00AA6740(+2Ch)[0] - total) / n) (FIDIV); x = gap.
// - A shown: 00AA78D0(A, x); x = (float)((wA + gap) + gap) (the image uses gap
//   for x, equal here); B shown: 00AA78D0(B, x), x = (float)((wB + gap) + x);
//   C shown: 00AA78D0(C, x).
void cc13_sweepB2_multi_error_layout_buttons_005d20a0(void* screen, Cc13SweepB2Host& host) {
    auto width = [&](void* button) {
        return static_cast<float>(static_cast<double>(at<float>(button, 0x114)) / 960.0);
    };
    float total = 0.0f;
    std::int32_t count = 1;
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x20))) {
        total = static_cast<float>(static_cast<double>(width(at<void*>(screen, 0x20))) + 0.0);
        count = 2;
    }
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x28))) {
        count += 1;
        total = width(at<void*>(screen, 0x28)) + total;
    }
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x24))) {
        count += 1;
        total = width(at<void*>(screen, 0x24)) + total;
    }
    const float* size = host.widget_get_size_00aa6740(at<void*>(screen, 0x2C));
    const float gap = (size[0] - total) / static_cast<float>(count);
    float x = gap;
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x20))) {
        host.widget_set_local_x_00aa78d0(at<void*>(screen, 0x20), x);
        x = (width(at<void*>(screen, 0x20)) + gap) + gap;
    }
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x28))) {
        host.widget_set_local_x_00aa78d0(at<void*>(screen, 0x28), x);
        x = (width(at<void*>(screen, 0x28)) + gap) + x;
    }
    if (host.widget_vcall_38_is_shown(at<void*>(screen, 0x24))) {
        host.widget_set_local_x_00aa78d0(at<void*>(screen, 0x24), x);
    }
}

// ===========================================================================
// Segment 26
// ===========================================================================

// 00638C60 BSP_HudMarker_ProjectOffscreenArrow (hypothesis, ledger name).
// ABI: __fastcall(ECX const float world[3], EDX float* x)(float* y, float*
// angle), RET 8. Callers 00641910, 00642040, 00643360. Evidence: disk bytes
// 00638C60..00638E3F; x87 directions from tools/x87_dc_form_scan.py (DE E2
// st(2) = st0 - st(2); D8 E9 st0 = st(1) - st0; DE E9 st(1) = st(1) - st0).
// - v = 004142E0(world, out, 00B6FCB0([[00E188A8]+19FCh])); vy = (float)(v.y *
//   4/3) (double 00CF5750 = 3FF5555560000000h); *angle = (float)atan2(vy, v.x)
//   (00BF701A with ST1 = vy, ST0 = v.x).
// - m = [[0109CF04]+0Dh] ? [00E197D0] : [00E197D8]; hw = (float)(0.5 - m);
//   hh = (float)(0.5 - [00E197C8]) (0.5 = double 00D7A280).
// - |a| below is a > 0 ? a : -0.0f - a (MOVSS from 00D7A208 = 80000000h, SUBSS).
// - |v.x| > 1e-5f (00D7A310) and hh > (|vy| / |v.x|) * hw: the arrow sits on a
//   side edge: s = sign(v.x) (-1/1, 0 when neither compare holds);
//   *x = (float)(s*hw + 0.5); *y = (float)(0.5 - hw * (vy / |v.x|)).
// - Otherwise (top or bottom edge): *x = (float)((v.x / |vy|) * hh + 0.5);
//   s = sign(vy) likewise; *y = (float)(0.5 - hh*s).
// Comparisons are written as "a > b" for the image's COMISS/FCOMI + JBE
// (not taken only when ordered and greater), so NaNs follow the image's branch.
void cc13_sweepB2_hud_marker_project_offscreen_arrow_00638c60(const float* world, float* x_out, float* y_out,
                                                             float* angle_out, Cc13SweepB2Host& host) {
    const float* view = host.camera_view_matrix_00b6fcb0(at<void*>(host.global_00e188a8(), 0x19FC));
    float v[3];
    host.vector3_transform_affine_point_004142e0(world, v, view);
    const float vy = static_cast<float>(static_cast<double>(v[1]) * f64_bits(0x3FF5555560000000ull));
    *angle_out = static_cast<float>(host.crt_atan2_00bf701a(vy, v[0]));

    const bool alt = at<std::uint8_t>(image_pointer(host, 0x0109CF04), 0xD) != 0;
    const float m = *static_cast<float*>(host.image_global(alt ? 0x00E197D0 : 0x00E197D8));
    const float hw = static_cast<float>(0.5 - m);
    const float hh = static_cast<float>(0.5 - *static_cast<float*>(host.image_global(0x00E197C8)));
    auto magnitude = [](float a) { return a > 0.0f ? a : -0.0f - a; };
    auto sign = [](float a) { return 0.0f > a ? -1 : (a > 0.0f ? 1 : 0); };

    const float x = v[0];
    if (magnitude(x) > cc13_sweepB_f32(0x3727C5ACu)) {
        const float r = magnitude(vy) / magnitude(x);
        if (hh > r * hw) {
            *x_out = static_cast<float>(sign(x)) * hw + 0.5f;
            *y_out = 0.5f - hw * (vy / magnitude(x));
            return;
        }
    }
    *x_out = x / magnitude(vy) * hh + 0.5f;
    *y_out = 0.5f - hh * static_cast<float>(sign(vy));
}

// 00640A20 BSP_InGameHudMarkers_UpdateLeadMarker (hypothesis, ledger name).
// ABI: __thiscall(ECX markers)(target), RET 4. Sole caller 00642040.
// Evidence: disk bytes 00640A20..00640C45; x87 direction from
// tools/x87_dc_form_scan.py (DE F9: st(1) = st(1) / st0). The "unresolved
// vtable +34h outputs" of docs/CC13_FE_10.md resolve as: the three pushes
// before the vslot 34h call are 00901C20's out-distance and out-aim pointers
// plus vslot 34h's own out buffer (it is RET 4 and returns that buffer, the
// shooter velocity, which is then pushed with the speed).
// - unit = +24h: non-null, vslot 5Ch(0Fh) true, and unit+1B4h == 8 or
//   00927F10(unit+1B4h); target team +54h != unit team and != 2; option byte
//   00F889CA set; [game+1ED4h] non-null. Otherwise nothing.
// - shooter = 00427EB0(unit); d2 = 00427E30(00427C90(shooter, out, 00427EB0(target)));
//   d2 > unit+49Ch -> nothing.
// - vel = unit vslot 34h(buf); 00901C20(shooter, target, unit+498h, vel, &aim,
//   &dist); 00638E50(&aim, screen) false -> nothing.
// - Markers vector +90h (used count +A0h): unless used > size (00637790), a new
//   8-byte marker (00BF681B; 0063E950(p, +8Ch) when non-null, else null) is
//   appended (00640340). marker = *006377B0(+90h, used); used += 1;
//   00AA7D00([marker+4h], screen.x, screen.y).
// - alpha = 0; when 00638E50(00427EB0(target), t) is true: d2 = 00427E30(
//   {screen.x - t.x, screen.y - t.y, 0}); alpha = (1/384 (double 00CF5978) > d2)
//   ? d2 / (1/384) : 1.0. [marker+4h] vslot 4Ch(alpha).
// Simplification: d2 / (1/384) is formed in double (the image divides in x87
// at the 24-bit precision control). EH frame 00C7A2FB (the allocation) is not modelled.
void cc13_sweepB2_hud_markers_update_lead_marker_00640a20(void* markers, void* target, Cc13SweepB2Host& host) {
    void* unit = at<void*>(markers, 0x24);
    if (unit == nullptr || !host.entity_vcall_5c_is_kind(unit, 0xF)) return;
    const std::int32_t slot = at<std::int32_t>(at<void*>(markers, 0x24), 0x1B4);
    if (slot != 8 && !host.party_slot_is_ai_held_00927f10(slot)) return;
    unit = at<void*>(markers, 0x24);
    const std::int32_t team = at<std::int32_t>(target, 0x54);
    if (at<std::int32_t>(unit, 0x54) == team || team == 2) return;
    if (*static_cast<std::uint8_t*>(host.image_global(0x00F889CA)) == 0) return;
    if (at<void*>(host.global_00e188a8(), 0x1ED4) == nullptr) return;
    const float* shooter = host.entity_world_position_00427eb0(unit);
    const float* target_position = host.entity_world_position_00427eb0(target);
    float delta[3];
    const float d2 = host.vector3_length_squared_00427e30(host.vector3_subtract_to_00427c90(shooter, delta, target_position));
    unit = at<void*>(markers, 0x24);
    if (d2 > at<float>(unit, 0x49C)) return;
    float velocity_buffer[3];
    float aim[3];
    float distance;
    const float* velocity = host.unit_vcall_34_velocity(unit, velocity_buffer);
    host.gunbot_intercept_solution_00901c20(shooter, target, at<float>(at<void*>(markers, 0x24), 0x498), velocity, aim,
                                            &distance);
    float screen[3];
    if (!host.hud_marker_project_and_clip_00638e50(aim, screen)) return;

    void* vector = static_cast<std::uint8_t*>(markers) + 0x90;
    if (!(at<std::uint32_t>(markers, 0xA0) > host.pointer_vector_size_00637790(vector))) {
        void* block = host.crt_operator_new_00bf681b(8);
        void* marker = block != nullptr ? host.hud_befogo_marker_construct_0063e950(block, at<void*>(markers, 0x8C)) : nullptr;
        host.pointer_vector_push_back_00640340(vector, &marker);
    }
    void* marker = *host.pointer_vector_at_006377b0(vector, at<std::uint32_t>(markers, 0xA0));
    at<std::uint32_t>(markers, 0xA0) += 1;
    host.widget_set_local_xy_00aa7d00(at<void*>(marker, 0x4), screen[0], screen[1]);
    float alpha = 0.0f;
    float t[3];
    if (host.hud_marker_project_and_clip_00638e50(host.entity_world_position_00427eb0(target), t)) {
        const float offset[3] = {screen[0] - t[0], screen[1] - t[1], 0.0f};
        const float d = host.vector3_length_squared_00427e30(offset);
        const double scale = f64_bits(0x3F65555555555555ull);
        alpha = scale > static_cast<double>(d) ? static_cast<float>(static_cast<double>(d) / scale) : 1.0f;
    }
    host.widget_vcall_4c_set_alpha(at<void*>(marker, 0x4), alpha);
}

// 00642960 BSP_InGameHudMarkers_AddSquadronMemberMarkers (hypothesis, ledger name).
// ABI: __thiscall(ECX markers)(squadron, byte flag, a3, a4, a5), RET 14h (the
// ledger's "cdecl" is corrected here). Sole caller 006430C0 (at 006431D9).
// Evidence: disk bytes 00642960..00642B45.
// - sel = 005224C0([[00E198C4]+CCh], squadron). ctrl = [00E188D8] and lead =
//   squadron+3D0h's first member, each refreshed through 00414DB0 when +C8h is clear.
// - d = lead+FCh.. - ctrl+FCh.. (three float stores); sq = (float)((dx*dx +
//   dy*dy) + dz*dz); nothing more unless sq > 1e-10 (double 00CE3820);
//   dist = (float)sqrt(sq); nothing more when dist == 0.0f (UCOMISS/LAHF/JNP
//   against 00D7A218: only an ordered equal returns).
// - member(i) = i <= 4 ? [squadron+3D0h + 4i] : null; for i < count +3CCh:
//   flag set: leader (007B8AD0(member)) -> 00642040(markers, member, 1, a3, a4,
//   a5 != 0, 2), else 00642040(markers, member, 0, 0, 0, 0, 2);
//   flag clear: i == sel -> 00642040(markers, member, 0, a3, a4, a5 != 0, 2),
//   else (member, 0, 0, 0, 0, 2). The count is re-read after each call.
void cc13_sweepB2_hud_markers_add_squadron_member_markers_00642960(void* markers, void* squadron, std::uint8_t flag,
                                                                   std::uint32_t a3, std::uint32_t a4,
                                                                   std::uint32_t a5, Cc13SweepB2Host& host) {
    const std::int32_t selected =
        host.squadron_selected_member_005224c0(at<void*>(host.global_00e198c4(), 0xCC), squadron);
    void* ctrl = host.global_00e188d8();
    if (at<std::uint8_t>(ctrl, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(ctrl);
    void* lead = at<void*>(squadron, 0x3D0);
    if (at<std::uint8_t>(lead, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(lead);
    const float dx = at<float>(lead, 0xFC) - at<float>(ctrl, 0xFC);
    const float dy = at<float>(lead, 0x100) - at<float>(ctrl, 0x100);
    const float dz = at<float>(lead, 0x104) - at<float>(ctrl, 0x104);
    const float sq = (dx * dx + dy * dy) + dz * dz;
    if (!(static_cast<double>(sq) > f64_bits(0x3DDB7CDFD9D7BDBBull))) return;
    const float dist = static_cast<float>(host.crt_sqrt_00bf7030(sq));
    if (dist == 0.0f) return;
    auto member = [&](std::int32_t i) -> void* {
        return static_cast<std::uint32_t>(i) <= 4 ? at<void*>(squadron, 0x3D0 + 4 * i) : nullptr;
    };
    const std::uint8_t emphasis = a5 != 0 ? 1 : 0;
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        if (flag != 0) {
            if (host.unit_is_squadron_flight_leader_007b8ad0(member(i))) {
                host.hud_markers_build_unit_marker_00642040(markers, member(i), 1, a3, a4, emphasis, 2);
            } else {
                host.hud_markers_build_unit_marker_00642040(markers, member(i), 0, 0, 0, 0, 2);
            }
        } else if (i == selected) {
            host.hud_markers_build_unit_marker_00642040(markers, member(i), 0, a3, a4, emphasis, 2);
        } else {
            host.hud_markers_build_unit_marker_00642040(markers, member(i), 0, 0, 0, 0, 2);
        }
    }
}

// 00643360 BSP_InGameHudMarkers_AddObjectiveMarker (hypothesis, ledger name).
// ABI: __thiscall(ECX markers)(float x, float y, float z by value, objective,
// record), RET 14h. Sole caller 006435D0 (call at 00643A58). Evidence: disk
// bytes 00643360..006434DB. The record's +0h is a unit or null.
// - 004B4B00() null -> return. A hit record on the stack is reset (00470470)
//   and destroyed (004704B0) around the rest; nothing else touches it.
// - unit = [record] non-null, its vslot 5Ch(8) true and 00852860(unit) true:
//   cam = [game+1ED4h] (00414DB0 when its byte +C8h is clear); cam+100h > 0
//   -> the position copy's y = 0.
// - 00638E50(&position, screen) true (on screen): unit ([record] re-read)
//   non-null -> 006430C0(markers, unit, 0, 0, 2); else 00639230(00640860(
//   markers), (float)(screen.x + 0.002) [double 00CF5988], screen.y,
//   markers+2Ch, objective, record, 1, [markers+1Ch]).
// - Off screen: only when objective+18h == 0: 00638C60(&position, &x, &y,
//   &angle) (y and angle land in the record / objective argument slots in the
//   image; the values passed on come from registers), then
//   00639230(00640860(markers), x, y, angle, objective, record, 0, [markers+1Ch]).
// The image pushes 00639230's arguments before calling 00640860 for ECX; the
// argument values are read in that order here. EH frame 00C7A468 (the hit
// record) is not modelled.
void cc13_sweepB2_hud_markers_add_objective_marker_00643360(void* markers, Cc13SweepB2Vec3 position,
                                                            void* objective, void* record, Cc13SweepB2Host& host) {
    if (host.game_controlled_vehicle_004b4b00() == nullptr) return;
    alignas(4) std::uint8_t hit_record[0x54];
    host.hit_record_reset_00470470(hit_record);
    void* unit = at<void*>(record, 0x0);
    if (unit != nullptr && host.entity_vcall_5c_is_kind(unit, 8) &&
        host.entity_is_below_altitude_third_00852860(at<void*>(record, 0x0))) {
        void* cam = at<void*>(host.global_00e188a8(), 0x1ED4);
        if (at<std::uint8_t>(cam, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(cam);
        if (at<float>(cam, 0x100) > 0.0f) position.y = 0.0f;
    }
    float screen[3];
    if (host.hud_marker_project_and_clip_00638e50(&position.x, screen)) {
        if (at<void*>(record, 0x0) != nullptr) {
            host.hud_markers_add_unit_marker_006430c0(markers, at<void*>(record, 0x0), 0, 0, 2);
        } else {
            const std::uint32_t extra = at<std::uint32_t>(markers, 0x1C);
            screen[0] = static_cast<float>(static_cast<double>(screen[0]) + f64_bits(0x3F60624DD2F1A9FCull));
            const float angle = at<float>(markers, 0x2C);
            const float y = screen[1];
            const float x = screen[0];
            void* marker = host.hud_markers_acquire_objective_marker_00640860(markers);
            host.hud_icon_set_show_for_kind_00639230(marker, x, y, angle, objective, record, 1, extra);
        }
    } else if (at<std::uint32_t>(objective, 0x18) == 0) {
        float x = 0.0f;
        float y = 0.0f;
        float angle = 0.0f;
        host.hud_marker_project_offscreen_arrow_00638c60(&position.x, &x, &y, &angle);
        const std::uint32_t extra = at<std::uint32_t>(markers, 0x1C);
        void* marker = host.hud_markers_acquire_objective_marker_00640860(markers);
        host.hud_icon_set_show_for_kind_00639230(marker, x, y, angle, objective, record, 0, extra);
    }
    host.hit_record_destroy_004704b0(hit_record);
}

// 0064B370 BSP_HudShipScreen45_Activate (hypothesis, ledger name).
// ABI: __thiscall/__fastcall(ECX screen), RET; vtable 00CF5E30 +18h, no rel32
// callers. Evidence: disk bytes 0064B370..0064B561 (docs/SHIP_SCREEN_UPDATE.md
// sections 12-13 describe it).
// - +15Ch = 0.0f; globals 00E197F4, 00E197F8, 00E197E8, 00E197F0 = 0.0f; then
//   [+40h] vslot 34h(1).
// - Controlled unit [00E188D8] non-null with vslot 5Ch(8): [+C4h] vslot 34h(0),
//   [+D4h] vslot 34h(1); otherwise [+C4h](1), [+D4h](0).
// - Three 1Ch-byte gauges: p = 00BF681B(1Ch); p ? 0043DF90(p, [+60h], [+64h])
//   : 0 -> +78h; likewise (+68h, +6Ch) -> +7Ch and (+70h, +74h) -> +80h.
// - [+90h], [+94h], [+98h], [+9Ch], [+A0h] vslot 34h(0); [+40h] vslot 34h(1).
// - bytes +108h/+109h/+10Ah = 0, +10Ch = 0.0f, byte +118h = 1, +11Ch = 0.0f,
//   byte +120h = 1, byte +188h = 0; 0064AE20(screen); byte +8h = 1; [+38h] vslot 34h(0).
// EH frame 00C7A9B1 (the three allocations) is not modelled.
void cc13_sweepB2_hud_ship_screen45_activate_0064b370(void* screen, Cc13SweepB2Host& host) {
    auto show = [&](std::size_t off, std::int32_t on) { host.widget_vcall_34_set_visible(at<void*>(screen, off), on); };
    at<float>(screen, 0x15C) = 0.0f;
    *static_cast<float*>(host.image_global(0x00E197F4)) = 0.0f;
    *static_cast<float*>(host.image_global(0x00E197F8)) = 0.0f;
    *static_cast<float*>(host.image_global(0x00E197E8)) = 0.0f;
    *static_cast<float*>(host.image_global(0x00E197F0)) = 0.0f;
    show(0x40, 1);
    void* unit = host.global_00e188d8();
    if (unit != nullptr && host.entity_vcall_5c_is_kind(unit, 8)) {
        show(0xC4, 0);
        show(0xD4, 1);
    } else {
        show(0xC4, 1);
        show(0xD4, 0);
    }
    const std::size_t sources[3] = {0x60, 0x68, 0x70};
    const std::size_t targets[3] = {0x78, 0x7C, 0x80};
    for (int k = 0; k < 3; ++k) {
        void* block = host.crt_operator_new_00bf681b(0x1C);
        void* gauge = nullptr;
        if (block != nullptr) {
            gauge = host.hud_observer_set_construct_0043df90(block, at<void*>(screen, sources[k]),
                                                             at<void*>(screen, sources[k] + 4));
        }
        at<void*>(screen, targets[k]) = gauge;
    }
    for (std::size_t off : {0x90u, 0x94u, 0x98u, 0x9Cu, 0xA0u}) show(off, 0);
    show(0x40, 1);
    at<std::uint8_t>(screen, 0x108) = 0;
    at<std::uint8_t>(screen, 0x109) = 0;
    at<std::uint8_t>(screen, 0x10A) = 0;
    at<float>(screen, 0x10C) = 0.0f;
    at<std::uint8_t>(screen, 0x118) = 1;
    at<float>(screen, 0x11C) = 0.0f;
    at<std::uint8_t>(screen, 0x120) = 1;
    at<std::uint8_t>(screen, 0x188) = 0;
    host.hud_ship_screen45_apply_unit_0064ae20(screen);
    at<std::uint8_t>(screen, 0x8) = 1;
    show(0x38, 0);
}

// 00651370 BSP_HudUnitView_EnsureSubmarineCamera (hypothesis, ledger name).
// ABI: __thiscall(ECX view), RET. Sole caller 00651760; host call
// src/ingame_gui_cc13.cpp unit_view_ensure_submarine_camera_00651370 (hook only).
// Evidence: disk bytes 00651370..006515A6; x87 directions from
// tools/x87_dc_form_scan.py (DC C9 st(1) *= st0, DC FA st(2) /= st0, DE F1
// st(1) = st0 / st(1)).
// - +2Ch null: p = 00BF55BE(410h); memset(p, 0, 410h); p non-null ->
//   00519290(p), vtables 00CF6250 (+0h), 00CF6238 (+10h), 00CF6230 (+24h),
//   00CF6218 (+170h), byte +405h = 0. +2Ch = p (0 when the allocation failed).
//   00923870(p, 0, [game+19CCh], identity 4x4); a = settings+45Ch and b =
//   settings2+458h (two 00424C40 calls); p+3ECh = (float)(pi*b/180),
//   p+3F0h = (float)(a*pi/180) (pi = double 00CE3D28 = float pi widened,
//   180 = double 00CE3D20); 00694A60(ECX = p, EDX = view+8h). The image has no
//   null test after the constructor block: a failed allocation faults.
// - Return when [game+1ED4h] == +2Ch and +24h == +28h.
// - Else unit = +24h: 0042F6C0(+2Ch, unit, [unit+1214h], &copy of unit+1218h..+1220h);
//   d = [[unit+538h]+A0h]; camera +3E8h == 0 -> +384h = +388h = 0, +38Ch =
//   +398h = +394h = +390h = d; 004BC410(game, +2Ch); +28h = +24h; byte +78h = 0.
// EH frame 00C7AEB8 (state 0 around the construction) is not modelled.
void cc13_sweepB2_hud_unit_view_ensure_submarine_camera_00651370(void* view, Cc13SweepB2Host& host) {
    if (at<void*>(view, 0x2C) == nullptr) {
        void* block = host.crt_operator_new_00bf55be(0x410);
        host.crt_memset_00bf79f0(block, 0, 0x410);
        void* camera = nullptr;
        if (block != nullptr) {
            host.ship_camera_construct_base_00519290(block);
            at<std::uint32_t>(block, 0x0) = 0x00CF6250;
            at<std::uint32_t>(block, 0x10) = 0x00CF6238;
            at<std::uint32_t>(block, 0x24) = 0x00CF6230;
            at<std::uint32_t>(block, 0x170) = 0x00CF6218;
            at<std::uint8_t>(block, 0x405) = 0;
            camera = block;
        }
        at<void*>(view, 0x2C) = camera;
        float identity[16] = {};
        identity[0] = identity[5] = identity[10] = identity[15] = 1.0f;
        host.entity_place_and_commit_00923870(camera, 0, at<std::uint32_t>(host.global_00e188a8(), 0x19CC), identity);
        void* settings_a = host.game_settings_get_00424c40();
        void* settings_b = host.game_settings_get_00424c40();
        const float pi = cc13_sweepB_f32(0x40490FDBu);
        const float a_rad = at<float>(settings_a, 0x45C) * pi / 180.0f;
        void* cam = at<void*>(view, 0x2C);
        at<float>(cam, 0x3EC) = pi * at<float>(settings_b, 0x458) / 180.0f;
        at<float>(cam, 0x3F0) = a_rad;
        host.observer_register_pair_00694a60(at<void*>(view, 0x2C), static_cast<std::uint8_t*>(view) + 0x8);
    }
    void* game = host.global_00e188a8();
    if (at<void*>(game, 0x1ED4) == at<void*>(view, 0x2C) && at<void*>(view, 0x24) == at<void*>(view, 0x28)) return;
    void* unit = at<void*>(view, 0x24);
    const float offset[3] = {at<float>(unit, 0x1218), at<float>(unit, 0x121C), at<float>(unit, 0x1220)};
    host.submarine_camera_setup_0042f6c0(at<void*>(view, 0x2C), unit, at<void*>(unit, 0x1214), offset);
    void* cam = at<void*>(view, 0x2C);
    const float distance = at<float>(at<void*>(at<void*>(view, 0x24), 0x538), 0xA0);
    if (at<std::uint32_t>(cam, 0x3E8) == 0) {
        at<float>(cam, 0x384) = 0.0f;
        at<float>(cam, 0x388) = 0.0f;
        at<float>(cam, 0x38C) = distance;
        at<float>(cam, 0x398) = distance;
        at<float>(cam, 0x394) = distance;
        at<float>(cam, 0x390) = distance;
    }
    host.game_set_camera_mover_004bc410(host.global_00e188a8(), at<void*>(view, 0x2C));
    at<void*>(view, 0x28) = at<void*>(view, 0x24);
    at<std::uint8_t>(view, 0x78) = 0;
}

// 0064B170 BSP_HudDamageDirection_Register (hypothesis, ledger name).
// ABI: __thiscall(ECX widget)(const float world_point[3]), RET 4. Callers
// 007BBCF0 and 00826F10. Evidence: disk bytes 0064B170..0064B313; x87
// directions from tools/x87_dc_form_scan.py (DC C9 st(1) = st(1)*st0, DE CA
// st(2) = st(2)*st0).
// - camera = [[00E188A8]+19FCh]; v = 004142E0(point, out, 00B6E0D0(camera));
//   w = copy of v, normalised in place by 0042B260.
// - y = w.y clamped to [-1.0 (double 00D7A250), 1.0] (FCOMI/JA then FCOMI/JBE:
//   a NaN passes through); pitch = (float)0042CF10((float)y);
//   yaw = (float)atan2(w.x, w.z) (00BF701A).
// - |pitch| > |yaw| (sign bits masked): hf = fov * 0.5 with fov = camera+1C4h
//   re-read (0.5 = double 00D7A280); pitch > hf -> +10h = 1.5f (00CE380C);
//   else -hf > pitch -> +14h = 1.5f.
// - Else the same test on yaw: yaw > hf -> +1Ch = 1.5f; -hf > yaw -> +18h = 1.5f.
void cc13_sweepB2_hud_damage_direction_register_0064b170(void* widget, const float* world_point,
                                                         Cc13SweepB2Host& host) {
    void* camera = at<void*>(host.global_00e188a8(), 0x19FC);
    const float* inverse = host.node_inverse_world_matrix_00b6e0d0(camera);
    float v[3];
    host.vector3_transform_affine_point_004142e0(world_point, v, inverse);
    float w[3] = {v[0], v[1], v[2]};
    host.geometry_normalize_with_floor_0042b260(w);
    float y = w[1];
    if (-1.0f > y) {
        y = -1.0f;
    } else if (y > 1.0f) {
        y = 1.0f;
    }
    const float pitch = host.geometry_asin_clamped_0042cf10(y);
    const float yaw = static_cast<float>(host.crt_atan2_00bf701a(w[0], w[2]));
    auto magnitude = [](float a) {
        std::uint32_t bits;
        std::memcpy(&bits, &a, sizeof bits);
        bits &= 0x7FFFFFFFu;
        std::memcpy(&a, &bits, sizeof bits);
        return a;
    };
    const bool use_pitch = magnitude(pitch) > magnitude(yaw);
    const float angle = use_pitch ? pitch : yaw;
    const float fov = at<float>(at<void*>(host.global_00e188a8(), 0x19FC), 0x1C4);
    const float half = static_cast<float>(fov * 0.5);
    const float one_and_half = cc13_sweepB_f32(0x3FC00000u);
    if (angle > half) {
        at<float>(widget, use_pitch ? 0x10 : 0x1C) = one_and_half;
    } else if (static_cast<float>(-fov * 0.5) > angle) {
        at<float>(widget, use_pitch ? 0x14 : 0x18) = one_and_half;
    }
}

// ===========================================================================
// Segment 29
// ===========================================================================

// 006AC030 BSP_InputSettings_LoadForCurrentProfile (hypothesis, ledger name).
// ABI: __thiscall(ECX settings), RET. Callers 00590E60, 0067CC60, 0067CFB0
// (hosts: src/frontend_menus.cpp input_settings_reset_006ac030, src/press_start_screen.cpp;
// hook calls only). Evidence: disk bytes 006AC030..006AC1B3. LuaObjects are 14h-byte values.
// - name = 0041E870(local, 00B662B0(00B67800(00B67980(+78h, &t1), &t2,
//   "DefaultPreset" 00CF81E0))); t2 then t1 destroyed (00B67700).
// - presets = 00B68100(00B67800(00B67980(+78h, &t3), &t4, "ControlPresets"
//   00CEE820), &t5, &name); t4 then t3 destroyed.
// - A LuaReader (14h bytes: vtable +0h, LuaObject vector +4h) is constructed by
//   004425C0 from a copy of t5 (00B66FA0) passed by value; 006ABA50(settings,
//   reader). Reader teardown inline: vtable 00CE44FC, 00441210(reader+4h),
//   vtable 00CE374C. Then t5 destroyed; name's data back to the sized pool
//   (length + 1) when non-null.
// Simplification: the by-value LuaObject is copy-constructed in a local and
// handed to the hook rather than in the call's argument area. EH frame
// 00C7FB19 (states 0..0Bh) is not modelled.
void cc13_sweepB2_input_settings_load_for_current_profile_006ac030(void* settings, Cc13SweepB2Host& host) {
    void* machine = static_cast<std::uint8_t*>(settings) + 0x78;
    alignas(4) std::uint8_t t1[0x14], t2[0x14], t3[0x14], t4[0x14], t5[0x14], by_value[0x14];
    alignas(4) std::uint8_t reader[0x14];
    Cc13SweepBNativeString name = {0, nullptr};

    void* globals = host.lua_globals_00b67980(machine, t1);
    void* preset = host.lua_object_get_field_00b67800(globals, t2, static_cast<const char*>(host.image_global(0x00CF81E0)));
    host.native_string_assign_0041e870(&name, host.lua_object_get_string_00b662b0(preset));
    host.lua_object_destroy_00b67700(t2);
    host.lua_object_destroy_00b67700(t1);

    globals = host.lua_globals_00b67980(machine, t3);
    void* presets = host.lua_object_get_field_00b67800(globals, t4, static_cast<const char*>(host.image_global(0x00CEE820)));
    host.lua_object_get_field_by_string_00b68100(presets, t5, &name);
    host.lua_object_destroy_00b67700(t4);
    host.lua_object_destroy_00b67700(t3);

    host.lua_object_copy_construct_00b66fa0(by_value, t5);
    void* constructed = host.lua_reader_construct_004425c0(reader, by_value);
    host.input_settings_read_keyboard_archive_006aba50(settings, constructed);
    at<std::uint32_t>(reader, 0x0) = 0x00CE44FC;
    host.lua_object_vector_tidy_00441210(reader + 0x4);
    at<std::uint32_t>(reader, 0x0) = 0x00CE374C;
    host.lua_object_destroy_00b67700(t5);
    if (name.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, name.length + 1, 1);
    }
}

// 006AD600 BSP_Game_ClearMissionLuaGlobals (hypothesis, ledger name).
// ABI: no arguments used (ECX ignored), RET; EBP frame with ESP aligned to 8.
// Sole caller 004DFB70 BSP_Game_LoadMissionScene. docs/CC13_SETTINGS_SAVEDATA.md
// says it is reconstructed in src/mission_load_path.cpp, but that file only
// lists it in a hook table (as "reset_effect_atlas"); this is the body.
// Evidence: disk bytes 006AD600..006AD813.
// For each name: a native string (0041DD40(length, 1), memcpy length + 1 when
// data is non-null), globals = 00B67980([00E188A8]+1A0Ch, &t), 00B67350(globals,
// &name), 00B67700(&t), the string's data back to the sized pool (length + 1).
// Names: "scoring" (00CF8268, 7), "thisTable" (00CE7494, 9), then the three C
// strings of the .data table 00E08DF0..00E08DFB (read at run time; the image
// holds 00CF5148, 00CF8240, 00CF8230), their lengths taken by an inline strlen.
// EH frame 00C7FBD0 is not modelled.
void cc13_sweepB2_game_clear_mission_lua_globals_006ad600(Cc13SweepB2Host& host) {
    auto clear = [&](const char* text, std::uint32_t length) {
        Cc13SweepBNativeString name = {0, nullptr};
        host.native_string_resize_0041dd40(&name, length, 1);
        if (name.data != nullptr) host.crt_memcpy_00bf7680(name.data, text, name.length + 1);
        alignas(4) std::uint8_t table[0x14];
        void* machine = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x1A0C;
        void* globals = host.lua_globals_00b67980(machine, table);
        host.lua_object_set_nil_00b67350(globals, &name);
        host.lua_object_destroy_00b67700(table);
        if (name.data != nullptr) {
            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, name.length + 1, 1);
        }
    };
    clear(static_cast<const char*>(host.image_global(0x00CF8268)), 7);
    clear(static_cast<const char*>(host.image_global(0x00CE7494)), 9);
    for (std::uint32_t slot = 0x00E08DF0; slot < 0x00E08DFC; slot += 4) {
        const char* text = *static_cast<const char**>(host.image_global(slot));
        clear(text, static_cast<std::uint32_t>(std::strlen(text)));
    }
}

// 006AF780 BSP_SceneEntity_RestoreFromSaveData (hypothesis, ledger name).
// ABI: __thiscall(ECX unused)(entity), RET 4. Sole caller 006AFB00, which
// passes a null entity every time (docs/CC13_SAVE_1.md; host model
// src/save_settings_cc13.cpp). Evidence: disk bytes 006AF780..006AF86D.
// - 009238A0(entity, by-value area) then 004425C0(reader, that LuaObject).
// - reader vslot 10h({0, "parentID" 00CF82B0}, {4, &parent}); the parent id's
//   storage is the entity argument slot, so it starts as the entity pointer.
// - 00BD6830(reader, {0, "localFrame" 00CF82A4}, {7, &frame}) (frame: 4x4 local).
// - entity vslot 98h(parent, [game+19CCh], &frame).
// - Reader teardown: vtable 00CE44FC, 00441210(reader+4h) (no base-vtable store here).
// Simplification: the LuaObject is filled in a local and handed to the
// 004425C0 hook rather than in the argument area. EH frame 00C80080 is not modelled.
void cc13_sweepB2_scene_entity_restore_from_save_data_006af780(void* entity, Cc13SweepB2Host& host) {
    alignas(4) std::uint8_t by_value[0x14];
    alignas(4) std::uint8_t reader[0x14];
    float frame[16];
    host.entity_saved_lua_data_009238a0(entity, by_value);
    host.lua_reader_construct_004425c0(reader, by_value);
    std::uint32_t parent = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(entity));
    host.lua_reader_vcall_10_read(reader, Cc13SweepB2Descriptor{0, host.image_global(0x00CF82B0)},
                                  Cc13SweepB2Descriptor{4, &parent});
    host.lua_reader_read_field_00bd6830(reader, Cc13SweepB2Descriptor{0, host.image_global(0x00CF82A4)},
                                        Cc13SweepB2Descriptor{7, frame});
    host.entity_vcall_98_restore_parent(entity, parent, at<std::uint32_t>(host.global_00e188a8(), 0x19CC), frame);
    at<std::uint32_t>(reader, 0x0) = 0x00CE44FC;
    host.lua_object_vector_tidy_00441210(reader + 0x4);
}

// ===========================================================================
// Segment 43
// ===========================================================================

// 0076F0E0 BSP_Session_RefreshFromOnlineSession (hypothesis, ledger name).
// ABI: __thiscall(ECX session), RET. Callers 00689540 BSP_MultiMenu_Init and
// 0076FAD0 BSP_Session_SetMode. Evidence: listing 0076F0E0..0076F1E8.
// - Online path: [00F8A2FC] non-null, its vslot 198h true, vslot 80h's record
//   +8h count non-zero: a second vslot 80h call, first node = [[rec+4]]; when
//   that is the head the checked iterator calls 00BF6713; the node's
//   std::string (+0Ch, capacity +20h, inline below 10h) is strncpy'd to +100h
//   for 20h bytes.
// - Otherwise: 00A3E510 on [00F8ABE8]; true -> strncpy(+100h, 00A3EAE0(), 20h)
//   (the PUSH 20h before the call is strncpy's count: 00A3EAE0 is RET 0);
//   false -> the game-name native string (see game_name_text).
// - Then +128h, +12Ah, +129h = 0; +170h, +174h = 0; +127h = 8; +12Dh = 1; +12Ch = 14h.
void cc13_sweepB2_session_refresh_from_online_0076f0e0(void* session, Cc13SweepB2Host& host) {
    char* name = static_cast<char*>(session) + 0x100;
    bool copied = false;
    if (image_pointer(host, 0x00F8A2FC) != nullptr &&
        host.online_session_vcall_198_is_online(image_pointer(host, 0x00F8A2FC)) &&
        at<std::uint32_t>(host.online_session_vcall_80_members(image_pointer(host, 0x00F8A2FC)), 0x8) != 0) {
        void* members = host.online_session_vcall_80_members(image_pointer(host, 0x00F8A2FC));
        void* head = at<void*>(members, 0x4);
        void* node = at<void*>(head, 0x0);
        if (node == head) host.crt_invalid_parameter_noinfo_00bf6713();
        const char* text = at<std::uint32_t>(node, 0x20) < 0x10 ? static_cast<const char*>(node) + 0xC
                                                                  : at<const char*>(node, 0xC);
        host.crt_strncpy_00bf9280(name, text, 0x20);
        copied = true;
    }
    if (!copied) {
        void* manager = image_pointer(host, 0x00F8ABE8);
        if (host.xenon_has_selected_user_00a3e510(manager)) {
            host.crt_strncpy_00bf9280(name, host.xenon_selected_username_00a3eae0(image_pointer(host, 0x00F8ABE8)),
                                      0x20);
        } else {
            host.crt_strncpy_00bf9280(name, game_name_text(host.global_00e188a8(), host), 0x20);
        }
    }
    at<std::uint8_t>(session, 0x128) = 0;
    at<std::uint8_t>(session, 0x12A) = 0;
    at<std::uint8_t>(session, 0x129) = 0;
    at<std::uint32_t>(session, 0x170) = 0;
    at<std::uint32_t>(session, 0x174) = 0;
    at<std::uint8_t>(session, 0x127) = 8;
    at<std::uint8_t>(session, 0x12D) = 1;
    at<std::uint8_t>(session, 0x12C) = 0x14;
}

// 007700E0 BSP_Session_UpdateClientClockScale (hypothesis, ledger name).
// ABI: __thiscall(ECX session), RET. Sole caller 00778450 BSP_Session_PumpStep.
// Evidence: disk bytes 007700E0..007701ED.
// - Not in play (game [00E188A8] null, game+5D4h != 0Dh, or the tick
//   [00F876B0] < 10 signed): scale 00E0B6C8 = 1.0f (MOVSS 00D7A24C).
// - In play: only when +188h == 0 (client), +18Ch non-null and 0076F8A0(+18Ch)
//   returns a peer. rate = (float)(peer[+D54h]+14h FIDIV int peer[+D54h]+4h)
//   (stored), diff = (float)(rate - session+240h) (stored); diff > 0 ->
//   diff = (float)(diff * 1.5) (double 00CE3D78). y = 00419010(1.5f [00CE380C],
//   1.0f, 3.0f [00CE3854], 1.1f [00CE6448], 0042BE90(&diff)); the scale is y
//   when diff >= 0 (COMISS 0 vs diff, JBE), else (float)(1 / y) (FLD1 FDIVRP).
// - A client with no peer, or a server, leaves the scale unchanged.
void cc13_sweepB2_session_update_client_clock_scale_007700e0(void* session, Cc13SweepB2Host& host) {
    float& scale = *static_cast<float*>(host.image_global(0x00E0B6C8));
    void* game = host.global_00e188a8();
    if (game == nullptr || at<std::int32_t>(game, 0x5D4) != 0xD ||
        *static_cast<std::int32_t*>(host.image_global(0x00F876B0)) < 10) {
        scale = 1.0f;
        return;
    }
    if (at<std::int32_t>(session, 0x188) != 0) return;
    void* peers = at<void*>(session, 0x18C);
    if (peers == nullptr) return;
    void* peer = host.session_peers_front_0076f8a0(peers);
    if (peer == nullptr) return;
    void* clock = at<void*>(peer, 0xD54);
    const float rate = at<float>(clock, 0x14) / static_cast<float>(at<std::int32_t>(clock, 0x4));
    float diff = rate - at<float>(session, 0x240);
    if (diff > 0.0f) diff = static_cast<float>(static_cast<double>(diff) * 1.5);
    const float magnitude = host.math_abs_float_by_ref_0042be90(&diff);
    const float y = host.math_interpolate_clamped_00419010(1.5f, 1.0f, 3.0f, 1.1f, magnitude);
    if (0.0f > diff) {
        scale = 1.0f / y;
    } else {
        scale = y;
    }
}

// 00770E50 BSP_WideString_Substring (hypothesis, ledger name).
// ABI: __thiscall(ECX source {u32 length, wchar_t* data})(out, int pos,
// uint count), RET 0Ch, EAX = out. Callers 00776930, 00A4BB00, 00A9F4B0,
// 00AB8F00. Evidence: disk bytes 00770E50..00770F57.
// - pos < 0 (signed): count += pos, pos = 0; else pos = min(pos, length) (unsigned).
// - Empty result (data null, length 0, or pos >= length): out is copy-
//   constructed (004C8DD0) from a local {0, null}; nothing is freed.
// - Else count = length - pos when pos + count > length (unsigned); the local
//   is sized by 004C53E0(count, 1); count != 0 -> wcsncpy(local.data,
//   data + pos, count); 004C8DD0(out, &local); then a non-null local.data
//   goes back through 00BD1510([00419CC0()], data, local.length*2 + 2, 1).
// EH frame 00C89461 (state 1 / 0 around the local) is not modelled.
void* cc13_sweepB2_wide_string_substring_00770e50(const void* source, void* out, std::int32_t pos,
                                                  std::uint32_t count, Cc13SweepB2Host& host) {
    void* src = const_cast<void*>(source);
    struct {
        std::uint32_t length;
        wchar_t* data;
    } local = {0, nullptr};
    std::uint32_t start;
    if (pos < 0) {
        count += static_cast<std::uint32_t>(pos);
        start = 0;
    } else {
        start = static_cast<std::uint32_t>(pos);
        if (start > at<std::uint32_t>(src, 0x0)) start = at<std::uint32_t>(src, 0x0);
    }
    const wchar_t* data = at<const wchar_t*>(src, 0x4);
    const std::uint32_t length = at<std::uint32_t>(src, 0x0);
    if (data == nullptr || length == 0 || start >= length) {
        host.wide_string_copy_construct_004c8dd0(out, &local);
        return out;
    }
    if (start + count > length) count = length - start;
    host.wide_string_resize_004c53e0(&local, count, 1);
    wchar_t* buffer = local.data;
    if (count != 0) host.crt_wcsncpy_00bfaeef(buffer, data + start, count);
    host.wide_string_copy_construct_004c8dd0(out, &local);
    if (buffer != nullptr) {
        const std::uint32_t bytes = local.length * 2 + 2;
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), buffer, bytes, 1);
    }
    return out;
}

// 00775160 BSP_SessionHostInfo_BuildBlob (hypothesis, ledger name).
// ABI: __thiscall(ECX owner)(void** out_blob, u32* out_size), RET 8. Vtable
// 00D039CC slot 3; no rel32 callers. Evidence: disk bytes 00775160..007752BE.
// The blob pointer is re-read from *out_blob before every store, as the image does.
// - blob = operator new(98h) (00BF681B); *out_blob = blob.
// - strncpy(blob, game name (game_name_text), 20h); blob[1Fh] = 0.
// - n = min([00F88A34], 4) (unsigned); strncpy(blob+20h, [00F88A38] or "", n); blob[20h+n] = 0.
// - +28h/+2Ch = game+698h/+69Ch (one game read); +59h = byte game+6A9h;
//   +30h = byte game+6A8h; +31h = 00574050(ECX = game+650h).
// - +50h/+54h = [00F8ABE8]+358h/+35Ch (two reads of the global).
// - strncpy(blob+32h, 008D4870(ECX = 00F88980), 10h) (the PUSH 10h is
//   strncpy's count: 008D4870 is RET 0).
// - +48h/+4Ch = game+728h/+72Ch; then ([owner+18Ch]+C8h) vslot 20h(blob).
// - +58h = byte [00F8A2FC]+4Ch; *out_size = 98h.
void cc13_sweepB2_session_host_info_build_blob_00775160(void* owner, void** out_blob, std::uint32_t* out_size,
                                                       Cc13SweepB2Host& host) {
    *out_blob = host.crt_operator_new_00bf681b(0x98);
    host.crt_strncpy_00bf9280(static_cast<char*>(*out_blob), game_name_text(host.global_00e188a8(), host), 0x20);
    at<std::uint8_t>(*out_blob, 0x1F) = 0;

    std::uint32_t n = *static_cast<std::uint32_t*>(host.image_global(0x00F88A34));
    if (n >= 4) n = 4;
    const char* tag = *static_cast<const char**>(host.image_global(0x00F88A38));
    if (tag == nullptr) tag = empty_c_string(host);
    host.crt_strncpy_00bf9280(static_cast<char*>(*out_blob) + 0x20, tag, n);
    at<std::uint8_t>(*out_blob, 0x20 + n) = 0;

    void* game = host.global_00e188a8();
    at<std::uint32_t>(*out_blob, 0x28) = at<std::uint32_t>(game, 0x698);
    at<std::uint32_t>(*out_blob, 0x2C) = at<std::uint32_t>(game, 0x69C);
    at<std::uint8_t>(*out_blob, 0x59) = at<std::uint8_t>(host.global_00e188a8(), 0x6A9);
    at<std::uint8_t>(*out_blob, 0x30) = at<std::uint8_t>(host.global_00e188a8(), 0x6A8);
    const std::uint8_t rank =
        host.session_filter_read_rank_00574050(static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650);
    at<std::uint8_t>(*out_blob, 0x31) = rank;
    at<std::uint32_t>(*out_blob, 0x50) = at<std::uint32_t>(image_pointer(host, 0x00F8ABE8), 0x358);
    at<std::uint32_t>(*out_blob, 0x54) = at<std::uint32_t>(image_pointer(host, 0x00F8ABE8), 0x35C);
    const char* language = host.game_settings_language_name_008d4870(host.image_global(0x00F88980));
    host.crt_strncpy_00bf9280(static_cast<char*>(*out_blob) + 0x32, language, 0x10);
    game = host.global_00e188a8();
    at<std::uint32_t>(*out_blob, 0x48) = at<std::uint32_t>(game, 0x728);
    at<std::uint32_t>(*out_blob, 0x4C) = at<std::uint32_t>(game, 0x72C);
    host.object_vcall_20_fill(static_cast<std::uint8_t*>(at<void*>(owner, 0x18C)) + 0xC8, *out_blob);
    at<std::uint8_t>(*out_blob, 0x58) = at<std::uint8_t>(image_pointer(host, 0x00F8A2FC), 0x4C);
    *out_size = 0x98;
}

// 00776230 BSP_PeerManager_PumpMessageQueues (hypothesis, ledger name).
// ABI: __fastcall(ECX session = game+1EF0h), RET. Callers 004E4A40 and
// 00778450 (hosts: include/bsp/session_polls.hpp and others call it as a hook).
// Evidence: disk bytes 00776230..0077648A; x87 direction from
// tools/x87_dc_form_scan.py (DE F9: st(1) = st(1) / st0).
// net() = [+188h] when set, else [+18Ch] (re-picked at every use).
// - [00E188A8] null or net() null: return.
// - Added-peer queue: lock = [net+34h]: EnterCriticalSection, lock+18h += 1;
//   list net+28h (head [+2Ch], size +30h): for each node (the head re-read per
//   pass): peer = node+8h value; 00774AB0, 007752D0, 007717A0 (session, peer);
//   the node is unlinked (prev->next, next->prev), freed (00BF65AC) and the size
//   decremented. Then 0076E850(list); lock = [net()+34h], +18h -= 1, Leave.
// - +F4h set: t = (float)(counter / frequency) from [01090AB0] vslot 20h; when
//   (t - [00E0AF18]) > 0.25 (double 00D7A348): [00E0AF18] = t, then 00784C40
//   ([+188h], 8.0f) and 007850E0([+18Ch], 8.0f) for whichever is non-null.
// - Removed-peer queue: lock [net+24h], list net+18h (head [+1Ch]): peer
//   [+D50h] set -> 007702E0(session, peer); 00772EC0(session, peer); node
//   unlinked and freed as above; 0076E850(list); unlock as above.
// The checked-iterator calls inside both loops compare the list with itself or
// test the end after the end test, so they cannot fire and are not reproduced.
// Simplification: the quotient is formed in double from the two 64-bit values
// (the image divides in x87 extended at the 24-bit precision control).
void cc13_sweepB2_peer_manager_pump_message_queues_00776230(void* session, Cc13SweepB2Host& host) {
    if (host.global_00e188a8() == nullptr) return;
    auto net = [&]() -> void* {
        void* server = at<void*>(session, 0x188);
        return server != nullptr ? server : at<void*>(session, 0x18C);
    };
    if (net() == nullptr) return;
    auto drain = [&](std::size_t lock_offset, std::size_t list_offset, auto handle) {
        void* lock = at<void*>(net(), lock_offset);
        host.enter_critical_section_00ce2218(lock);
        at<std::int32_t>(lock, 0x18) += 1;
        std::uint8_t* list = static_cast<std::uint8_t*>(net()) + list_offset;
        void* node = at<void*>(at<void*>(list, 0x4), 0x0);
        while (node != at<void*>(list, 0x4)) {
            handle(at<void*>(node, 0x8));
            void* next = at<void*>(node, 0x0);
            if (node != at<void*>(list, 0x4)) {
                at<void*>(at<void*>(node, 0x4), 0x0) = next;
                at<void*>(at<void*>(node, 0x0), 0x4) = at<void*>(node, 0x4);
                host.crt_operator_delete_00bf65ac(node);
                at<std::int32_t>(list, 0x8) += -1;
            }
            node = next;
        }
        host.stl_list_clear_0076e850(list);
        lock = at<void*>(net(), lock_offset);
        at<std::int32_t>(lock, 0x18) += -1;
        host.leave_critical_section_00ce2210(lock);
    };
    drain(0x34, 0x28, [&](void* peer) {
        host.session_on_peer_added_00774ab0(session, peer);
        host.session_server_add_player_007752d0(session, peer);
        host.session_host_refresh_scene_007717a0(session, peer);
    });
    if (at<std::uint32_t>(session, 0xF4) != 0) {
        std::int64_t buffer[2];
        const std::int64_t* clock = host.timer_vcall_20_read(image_pointer(host, 0x01090AB0), buffer);
        const float t = static_cast<float>(static_cast<double>(clock[0]) / static_cast<double>(clock[1]));
        float& last = *static_cast<float*>(host.image_global(0x00E0AF18));
        if (t - last > 0.25f) {
            last = t;
            if (at<void*>(session, 0x188) != nullptr) {
                host.network_server_kick_timed_out_00784c40(at<void*>(session, 0x188), 8.0f);
            }
            if (at<void*>(session, 0x18C) != nullptr) {
                host.network_client_check_server_silence_007850e0(at<void*>(session, 0x18C), 8.0f);
            }
        }
    }
    drain(0x24, 0x18, [&](void* peer) {
        if (at<std::uint32_t>(peer, 0xD50) != 0) host.session_host_remove_peer_player_007702e0(session, peer);
        host.session_on_peer_removed_00772ec0(session, peer);
    });
}

// 00784B10 BSP_SessionTransport_KickPeer (hypothesis, ledger name).
// ABI: __thiscall(ECX transport)(peer, byte flag), RET 8. Callers 0076C790,
// 0076D140, 0076F400, 00784C40. Evidence: disk bytes 00784B10..00784C34.
// - Linear search of the peer list (record transport+8h, head [+0Ch]) for a
//   node whose +8h == peer. The checked-iterator tests in the loop compare the
//   list with itself and the dereference test follows the end test, so the
//   00BF6713 calls there are unreachable and are not reproduced.
// - Found: a 1Ch-byte MT_KICKED message on the stack: 0075B430(msg, 14h);
//   msg+4h = 1, vtable 00D03174, +18h = flag. lock = [peer+4h]:
//   EnterCriticalSection(lock), lock+18h += 1; 00783C80(transport, peer, msg);
//   lock = [peer+4h] (re-read), lock+18h -= 1, LeaveCriticalSection(lock);
//   the message's vtable becomes 00CE4974 (inline base destructor);
//   00783490(transport, peer); transport vslot 2Ch(peer, &FADEBABEh, 4) (the
//   image reuses the peer argument slot for the word).
// EH frame 00C8A528 (state 0 around the message) is not modelled.
void cc13_sweepB2_session_transport_kick_peer_00784b10(void* transport, void* peer, std::uint8_t flag,
                                                       Cc13SweepB2Host& host) {
    void* head = at<void*>(transport, 0xC);
    void* node = at<void*>(head, 0x0);
    while (node != head && at<void*>(node, 0x8) != peer) node = at<void*>(node, 0x0);
    if (node == head) return;

    alignas(4) std::uint8_t message[0x1C] = {};
    host.session_message_construct_base_0075b430(message, 0x14);
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0x0) = 0x00D03174;
    at<std::uint8_t>(message, 0x18) = flag;
    void* lock = at<void*>(peer, 0x4);
    host.enter_critical_section_00ce2218(lock);
    at<std::int32_t>(lock, 0x18) += 1;
    host.transport_serialize_message_00783c80(transport, peer, message);
    lock = at<void*>(peer, 0x4);
    at<std::int32_t>(lock, 0x18) += -1;
    host.leave_critical_section_00ce2210(lock);
    at<std::uint32_t>(message, 0x0) = 0x00CE4974;
    host.transport_flush_target_buffers_00783490(transport, peer);
    const std::uint32_t marker = 0xFADEBABE;
    host.transport_vcall_2c_send_raw(transport, peer, &marker, 4);
}

// ===========================================================================
// Segments 45 and 46
// ===========================================================================

// 0078DBA0 BSP_OceanEnvironment_InitWaterParticles (hypothesis, ledger name).
// ABI: __thiscall(ECX environment)(source, settings), RET 8. Sole caller
// 004DE610 BSP_Game_ConstructWorld. Evidence: disk bytes 0078DBA0..0078DCE0.
// - The error text "WaterParticle initialization failed" (00D04398, 23h
//   characters) is built into a local native string (0041DD40(0x23, 1), then
//   memcpy of length + 1 bytes) and released at once to the sized pool with
//   length + 1 bytes: the leftover of a compiled-out report; reproduced as is.
// - [env+B0h] non-null: its vslot 4(1) (scalar deleting destructor), +B0h = 0.
// - block = operator new(11Ch). Non-null: a by-value reference pointer is
//   made from source ([source+8h], InterlockedIncrement(ref+4h); null source ->
//   null reference) and passed by address to 00BA43B0(block, &ref); +B0h =
//   the result (0 when the allocation failed). When the reference was made and
//   is non-null, InterlockedDecrement(ref+4h) == 0 calls its vslot 0().
// - settings non-null: 0078D940([+B0h], settings). Always 00BA44F0([+B0h]).
// EH frame 00C8ACB4 (state 0 for the block, 1 for the reference) is not modelled.
void cc13_sweepB2_ocean_init_water_particles_0078dba0(void* environment, void* source, void* settings,
                                                      Cc13SweepB2Host& host) {
    discard_error_text(0x00D04398, 0x23, host);

    void*& particles = at<void*>(environment, 0xB0);
    if (particles != nullptr) {
        host.object_vcall_04_delete(particles, 1);
        particles = nullptr;
    }
    void* block = host.crt_operator_new_00bf681b(0x11C);
    bool made_reference = false;
    void* reference = nullptr;
    void* result = nullptr;
    if (block != nullptr) {
        if (source != nullptr) {
            reference = at<void*>(source, 0x8);
            host.interlocked_increment_00ce221c(&at<std::int32_t>(reference, 0x4));
        }
        made_reference = true;
        result = host.ocean_water_particles_construct_00ba43b0(block, &reference);
    }
    particles = result;
    if (made_reference) release_reference(reference, host);
    if (settings != nullptr) host.ocean_water_particles_assign_settings_0078d940(particles, settings);
    host.ocean_water_particles_initialize_00ba44f0(particles);
}

// 0078DCF0 BSP_OceanEnvironment_InitBeams (hypothesis, ledger name).
// ABI: __thiscall(ECX environment)(source, settings), RET 8. Caller 004DE610
// (004DF8C1). Evidence: disk bytes 0078DCF0..0078DE90. The pattern of 0078DBA0
// with two references:
// - "Beam initialization failed" (00D043BC, 1Ah characters) built and freed at once.
// - [env+ACh] non-null: vslot 4(1), +ACh = 0.
// - block = operator new(FCh). Non-null: env_ref = [env+A8h] ? [[env+A8h]+0Ch]
//   (InterlockedIncrement +4h) : null; source_ref = source ? [source+8h]
//   (incremented) : null; +ACh = 00BA3A60(block, &source_ref, &env_ref).
//   Null block: +ACh = 0 and neither reference exists.
// - Release (InterlockedDecrement == 0 -> vslot 0()): source_ref first, then env_ref.
// - settings non-null: 0078D9F0([+ACh], settings). Always 00BA3BD0([+ACh]).
// EH frame 00C8ACFD (states 2 and 3 for the references) is not modelled.
void cc13_sweepB2_ocean_init_beams_0078dcf0(void* environment, void* source, void* settings, Cc13SweepB2Host& host) {
    discard_error_text(0x00D043BC, 0x1A, host);

    void*& beams = at<void*>(environment, 0xAC);
    if (beams != nullptr) {
        host.object_vcall_04_delete(beams, 1);
        beams = nullptr;
    }
    void* block = host.crt_operator_new_00bf681b(0xFC);
    void* result = nullptr;
    void* env_ref = nullptr;
    void* source_ref = nullptr;
    const bool made = block != nullptr;
    if (made) {
        void* env_source = at<void*>(environment, 0xA8);
        if (env_source != nullptr) {
            env_ref = at<void*>(env_source, 0xC);
            host.interlocked_increment_00ce221c(&at<std::int32_t>(env_ref, 0x4));
        }
        if (source != nullptr) {
            source_ref = at<void*>(source, 0x8);
            host.interlocked_increment_00ce221c(&at<std::int32_t>(source_ref, 0x4));
        }
        result = host.ocean_beams_construct_00ba3a60(block, &source_ref, &env_ref);
    }
    beams = result;
    if (made) {
        release_reference(source_ref, host);
        release_reference(env_ref, host);
    }
    if (settings != nullptr) host.ocean_beams_assign_settings_0078d9f0(beams, settings);
    host.ocean_beams_initialize_00ba3bd0(beams);
}

// 00795D90 MovieCameraKeyframe_ConvertAnchorSpace (hypothesis, ledger name).
// ABI: __thiscall(ECX key)(int new_mode), RET 4. Sole caller 0079A3B0.
// Evidence: disk bytes 00795D90..00795ECB, jump table 00795ED0 (old mode 0 ->
// 00795E69, 1 -> 00795E21, 2 -> 00795E47, 3 -> 00795DCD).
// - Returns untouched when new_mode == +2Ch, byte +27h is clear or the entity
//   +1Ch is null.
// - Old mode 0 (any new mode): M = copy of 00414E10(entity); p = 00414D10(out,
//   +4Ch, &M); +40h..+48h = p, then +34h..+3Ch = +40h..+48h.
// - Old mode 1, 2 or 3, only when new_mode == 0: M = copy of 0042D7E0(entity);
//   d = 00427C90(+4Ch, out, &M[12]) (point minus the matrix translation);
//   +34h..+3Ch = d, then +40h..+48h = +34h..+3Ch. (Cases 1-3 differ only in
//   which stack temporary receives d.)
// - After either conversion: +64h..+6Ch = +58h..+60h.
// - Then (also for old modes above 3 and for 1-3 with new_mode != 0) +2Ch = new_mode.
void cc13_sweepB2_movie_keyframe_convert_anchor_space_00795d90(void* key, std::int32_t new_mode,
                                                               Cc13SweepB2Host& host) {
    const std::int32_t old_mode = at<std::int32_t>(key, 0x2C);
    if (new_mode == old_mode || at<std::uint8_t>(key, 0x27) == 0) return;
    void* entity = at<void*>(key, 0x1C);
    if (entity == nullptr) return;
    float* point = &at<float>(key, 0x4C);
    bool converted = false;
    float matrix[16];
    float out[3];
    if (old_mode == 0) {
        std::memcpy(matrix, host.entity_pose_derived_affine_inverse_00414e10(entity), sizeof matrix);
        const float* p = host.vector3_transform_affine_point_via_scratch_00414d10(out, point, matrix);
        at<float>(key, 0x40) = p[0];
        at<float>(key, 0x44) = p[1];
        at<float>(key, 0x48) = p[2];
        at<float>(key, 0x34) = at<float>(key, 0x40);
        at<float>(key, 0x38) = at<float>(key, 0x44);
        at<float>(key, 0x3C) = at<float>(key, 0x48);
        converted = true;
    } else if (old_mode >= 1 && old_mode <= 3 && new_mode == 0) {
        std::memcpy(matrix, host.entity_pose_world_matrix_0042d7e0(entity), sizeof matrix);
        const float* d = host.vector3_subtract_to_00427c90(point, out, &matrix[12]);
        at<float>(key, 0x34) = d[0];
        at<float>(key, 0x38) = d[1];
        at<float>(key, 0x3C) = d[2];
        at<float>(key, 0x40) = at<float>(key, 0x34);
        at<float>(key, 0x44) = at<float>(key, 0x38);
        at<float>(key, 0x48) = at<float>(key, 0x3C);
        converted = true;
    }
    if (converted) {
        at<float>(key, 0x64) = at<float>(key, 0x58);
        at<float>(key, 0x68) = at<float>(key, 0x5C);
        at<float>(key, 0x6C) = at<float>(key, 0x60);
    }
    at<std::int32_t>(key, 0x2C) = new_mode;
}

// 007A49A0 BSP_MovCam_PickRandomDeckCamera (hypothesis, ledger name).
// ABI: __thiscall(ECX camera)(target, look_target, key), RET 0Ch, EAX = the
// chosen list entry or -1. Caller 007A8180. Evidence: disk bytes 007A49A0..007A4B5B.
// - target vslot 5Ch(5) false, or [target+538h]+BCh null -> -1.
// - list(key) = 007A4590([[target+538h]+BCh]+0Ch, key), re-fetched at every
//   use; best = -1e10f (00CE4ADC), chosen = -1; nothing when its count <= 0.
// - Per i: r = 00BD2F10(ECX 0, 0.0f, 1.0f); score = r when look_target is null,
//   else (list grown by 00468050(i+1) when i >= count) 0079F710(camera, xy,
//   look_target, data[i], &inside) and score = (float)((bonus - dist) + r) with
//   bonus 42.0f / 0 and dist as in 007A8C60 (sqrt only above 1e-10).
//   score > best -> list re-fetched and grown as above, chosen = data[i], best = score.
//   The loop ends when i+1 reaches the re-fetched count.
// The inside byte and the score live in argument slots (target / key) in the
// image; the target and key values themselves are kept in registers, so only
// the inside byte's starting value (the target pointer's low byte) is visible.
// RNG: 00BD2F10 is the shared stream (presentation, see 007A8C60).
std::int32_t cc13_sweepB2_movcam_pick_random_deck_camera_007a49a0(void* camera, void* target, void* look_target,
                                                                  void* key, Cc13SweepB2Host& host) {
    if (!host.entity_vcall_5c_is_kind(target, 5)) return -1;
    if (at<void*>(at<void*>(target, 0x538), 0xBC) == nullptr) return -1;
    auto list = [&]() -> void* {
        void* cameras = at<void*>(at<void*>(target, 0x538), 0xBC);
        return host.map_subscript_007a4590(static_cast<std::uint8_t*>(cameras) + 0xC, key);
    };
    auto entry = [&](std::int32_t i) -> std::int32_t {
        void* l = list();
        if (i >= at<std::int32_t>(l, 0x4)) host.pod_pointer_array_resize_00468050(l, i + 1);
        return at<std::int32_t*>(l, 0x0)[i];
    };
    float best = cc13_sweepB_f32(0xD01502F9u);
    std::int32_t chosen = -1;
    if (at<std::int32_t>(list(), 0x4) <= 0) return chosen;
    std::uint8_t inside = static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(target));
    std::int32_t i = 0;
    do {
        float score = host.random_uniform_float_range_00bd2f10(0, 0.0f, 1.0f);
        if (look_target != nullptr) {
            const std::int32_t index = entry(i);
            float xy[2];
            host.movcam_vehicle_camera_pose_0079f710(camera, xy, look_target, index, &inside);
            const float bonus = inside != 0 ? 42.0f : 0.0f;
            const float sq = xy[0] * xy[0] + xy[1] * xy[1];
            float dist;
            if (static_cast<double>(sq) > f64_bits(0x3DDB7CDFD9D7BDBBull)) {
                dist = static_cast<float>(host.crt_sqrt_00bf7030(sq));
            } else {
                dist = 0.0f;
            }
            score = (bonus - dist) + score;
        }
        if (score > best) {
            chosen = entry(i);
            best = score;
        }
        ++i;
    } while (i < at<std::int32_t>(list(), 0x4));
    return chosen;
}

// 007A4EF0 BSP_MovCam_SeedAnglesFromVehicleCamera (hypothesis, ledger name).
// ABI: __thiscall(ECX camera)(target), RET 4. Callers 007A6710 (and others).
// Evidence: disk bytes 007A4EF0..007A5101. pairs(target) = 007A4E10([camera+
// 4D0h]+70h, &target) (a vector of float pairs, begin +4h, end +8h), re-fetched at each use.
// - pairs non-empty -> return.
// - cameras = [[target+538h]+BCh] (re-read, grown by 005CD640(i+1) when i >=
//   count): i = the first index whose element (+5Ch stride) byte +58h is 0,
//   scanning only when cameras is non-null (else i = 0);
//   *007A0AC0([camera+4D0h]+64h, &target) = i.
// - cameras null or count 0: push_back {0, 0} onto pairs (004222C0) and return.
// - Else for each camera i < count (signed, re-read): push_back {0, 0};
//   0042D2E0(element+8h matrix, &a, &b, &c); pairs[i] = {-0.0f - a, -0.0f - b}
//   (MOVSS 00D7A208 / SUBSS), each store behind a checked index (00BF6713 when
//   begin is null or i >= size).
void cc13_sweepB2_movcam_seed_angles_from_vehicle_camera_007a4ef0(void* camera, void* target, Cc13SweepB2Host& host) {
    void* const key = target;
    auto pairs = [&]() -> void* {
        return host.map_subscript_007a4e10(static_cast<std::uint8_t*>(at<void*>(camera, 0x4D0)) + 0x70, &key);
    };
    void* first = pairs();
    if (at<float*>(first, 0x4) != nullptr && (at<float*>(first, 0x8) - at<float*>(first, 0x4)) / 2 != 0) return;
    auto cameras = [&]() -> void* { return at<void*>(at<void*>(target, 0x538), 0xBC); };
    auto element = [&](std::int32_t i) -> std::uint8_t* {
        void* c = cameras();
        if (i >= at<std::int32_t>(c, 0x4)) host.vehicle_camera_array_resize_005cd640(c, i + 1);
        return at<std::uint8_t*>(c, 0x0) + i * 0x5C;
    };
    std::int32_t index = 0;
    if (cameras() != nullptr) {
        while (at<std::uint8_t>(element(index), 0x58) != 0) ++index;
    }
    *host.map_subscript_007a0ac0(static_cast<std::uint8_t*>(at<void*>(camera, 0x4D0)) + 0x64, &key) = index;
    const float zero_pair[2] = {0.0f, 0.0f};
    if (cameras() == nullptr || at<std::int32_t>(cameras(), 0x4) == 0) {
        host.float_pair_vector_push_back_004222c0(pairs(), zero_pair);
        return;
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(cameras(), 0x4); ++i) {
        host.float_pair_vector_push_back_004222c0(pairs(), zero_pair);
        float a, b, c;
        host.matrix_extract_basis_angles_0042d2e0(reinterpret_cast<const float*>(element(i) + 0x8), &a, &b, &c);
        auto checked_pair = [&]() -> float* {
            void* p = pairs();
            float* begin = at<float*>(p, 0x4);
            if (begin == nullptr ||
                static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((at<float*>(p, 0x8) - begin) / 2)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            return at<float*>(p, 0x4) + i * 2;
        };
        checked_pair()[0] = -0.0f - a;
        checked_pair()[1] = -0.0f - b;
    }
}

// 007A8C60 BSP_MovCam_PickDeckPointScored (hypothesis, ledger name).
// ABI: __thiscall(ECX camera)(entity), RET 4, EAX = index or -1. Callers
// 007A8DA0 and 007A91A0. Evidence: disk bytes 007A8C60..007A8D92.
// - cameras = [[camera+38Ch]+538h]+BCh; null -> -1 (nothing else).
// - best = -1e10f (00CE4ADC = D01502F9h), chosen = -1; for i in [0, count
//   +4h signed), the count and cameras re-read after each pass:
//   0079F710(camera, xy[2], entity, i, &inside) (the image reuses the entity
//   argument slot for the inside byte); bonus = inside ? 42.0f (00D04F60) : 0;
//   sq = (float)(x*x + y*y); dist = sq > 1e-10 (double 00CE3820) ?
//   (float)sqrt(sq) (00BF7030) : 0; r = 00BD2F10(ECX = 0, 0.0f, 1.0f);
//   score = (float)(r + (bonus - dist)); score > best -> best = score, chosen = i.
// - chosen != -1 -> 007A6710(camera, chosen). Returns chosen.
// RNG: 00BD2F10 is the process-wide generator; the movie camera is
// presentation, so a host wiring this needs a presentation stream or an OFF switch.
std::int32_t cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60(void* camera, void* entity,
                                                                 Cc13SweepB2Host& host) {
    auto cameras = [&]() -> void* {
        return at<void*>(at<void*>(at<void*>(camera, 0x38C), 0x538), 0xBC);
    };
    if (cameras() == nullptr) return -1;
    float best = cc13_sweepB_f32(0xD01502F9u);
    std::int32_t chosen = -1;
    // The inside byte lives in the entity argument slot: before the first
    // write it holds the pointer's low byte, and it keeps the last value.
    std::uint8_t inside = static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(entity));
    for (std::int32_t i = 0; i < at<std::int32_t>(cameras(), 0x4); ++i) {
        float xy[2];
        host.movcam_vehicle_camera_pose_0079f710(camera, xy, entity, i, &inside);
        const float bonus = inside != 0 ? 42.0f : 0.0f;
        const float sq = xy[0] * xy[0] + xy[1] * xy[1];
        float dist;
        if (static_cast<double>(sq) > f64_bits(0x3DDB7CDFD9D7BDBBull)) {
            dist = static_cast<float>(host.crt_sqrt_00bf7030(sq));
        } else {
            dist = 0.0f;
        }
        const float r = host.random_uniform_float_range_00bd2f10(0, 0.0f, 1.0f);
        const float score = r + (bonus - dist);
        if (score > best) {
            best = score;
            chosen = i;
        }
    }
    if (chosen != -1) host.movcam_enter_vehicle_camera_007a6710(camera, chosen);
    return chosen;
}

// 007A6710 BSP_MovCam_EnterVehicleCamera (hypothesis, ledger name).
// ABI: __thiscall(ECX camera)(int index), RET 4. Callers 007A87B0, 007A8C60.
// Evidence: disk bytes 007A6710..007A68DE.
// - 007A4EF0(camera, [+38Ch]); +388h = index; cameras = [[+38Ch]+538h]+BCh,
//   grown by 005CD640(cameras, index+1) when index >= count +4h (signed).
// - A 4x4 local: identity rotation (1.0f from 00D7A24C), translation = the
//   element's +38h..+40h (element = [cameras] + index*5Ch), w = 1; copied to
//   +420h through 004134F0.
// - pairs = 007A4E10([+4D0h]+70h, &[+38Ch]); i = [+388h]; begin null or i >=
//   (end - begin)/8 (unsigned) -> 00BF6713; +3C4h/+3C8h = begin[i] (two floats).
// - *007A0AC0([+4D0h]+64h, &[+38Ch]) = [+388h].
// - cameras re-read, grown again as above for i = [+388h]; the element's
//   native string (+0h) is copied into a by-value temporary (0041DD40(length,
//   1), memcpy(length) when length != 0) and passed to 005CC5E0([+4D0h], temp),
//   which consumes it.
// - +460h = 1.0f; (camera+170h) vslot 0Ch().
void cc13_sweepB2_movcam_enter_vehicle_camera_007a6710(void* camera, std::int32_t index, Cc13SweepB2Host& host) {
    void** target = &at<void*>(camera, 0x38C);
    host.movcam_seed_angles_007a4ef0(camera, *target);
    at<std::int32_t>(camera, 0x388) = index;
    void* cameras = at<void*>(at<void*>(*target, 0x538), 0xBC);
    if (index >= at<std::int32_t>(cameras, 0x4)) host.vehicle_camera_array_resize_005cd640(cameras, index + 1);
    const std::uint8_t* element = at<std::uint8_t*>(cameras, 0x0) + index * 0x5C;
    float matrix[16] = {};
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[12] = cc13_sweepB_at<float>(const_cast<std::uint8_t*>(element), 0x38);
    matrix[13] = cc13_sweepB_at<float>(const_cast<std::uint8_t*>(element), 0x3C);
    matrix[14] = cc13_sweepB_at<float>(const_cast<std::uint8_t*>(element), 0x40);
    matrix[15] = 1.0f;
    host.matrix_copy_4x4_004134f0(&at<float>(camera, 0x420), matrix);

    const std::int32_t mode = at<std::int32_t>(camera, 0x388);
    void* pairs = host.map_subscript_007a4e10(static_cast<std::uint8_t*>(at<void*>(camera, 0x4D0)) + 0x70, target);
    const float* begin = at<const float*>(pairs, 0x4);
    if (begin == nullptr ||
        static_cast<std::uint32_t>(mode) >= static_cast<std::uint32_t>((at<const float*>(pairs, 0x8) - begin) / 2)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    begin = at<const float*>(pairs, 0x4);
    at<float>(camera, 0x3C4) = begin[mode * 2];
    at<float>(camera, 0x3C8) = begin[mode * 2 + 1];
    *host.map_subscript_007a0ac0(static_cast<std::uint8_t*>(at<void*>(camera, 0x4D0)) + 0x64, target) =
        at<std::int32_t>(camera, 0x388);

    const std::int32_t current = at<std::int32_t>(camera, 0x388);
    cameras = at<void*>(at<void*>(*target, 0x538), 0xBC);
    if (current >= at<std::int32_t>(cameras, 0x4)) host.vehicle_camera_array_resize_005cd640(cameras, current + 1);
    Cc13SweepBNativeString* name =
        reinterpret_cast<Cc13SweepBNativeString*>(at<std::uint8_t*>(cameras, 0x0) + current * 0x5C);
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, name->length, 1);
    if (name->length != 0) host.crt_memcpy_00bf7680(text.data, name->data, text.length);
    host.movcam_show_mode_text_005cc5e0(at<void*>(camera, 0x4D0), text);
    at<float>(camera, 0x460) = 1.0f;
    host.object_vcall_0c(static_cast<std::uint8_t*>(camera) + 0x170);
}

// 007A7C60 BSP_MovCam_ReceivePathPoints (hypothesis, ledger name).
// ABI: __thiscall(ECX camera)(const message*), RET 4. Sole caller 0076A870
// BSP_Session_ReceiveMovCamPathPointProperty. Evidence: disk bytes 007A7C60..007A7DD6.
// - 0079C830(camera); 0079EB10(camera+478h, 3, null).
// - rec = operator new(4Ch); non-null: vtable 00D043D8, +4h/+8h/+0Ch = 0,
//   byte +10h = 0, +14h = 0, +18h..+20h = 0.0f, +24h = 0, +28h..+38h = 0.0f,
//   +48h = 0, +40h = +3Ch = 1/3 (00D04678 = 3EAAAAABh), +44h = 0.0f. The image
//   has no null test after the allocation: the stores below go through rec
//   whether or not it is null (reproduced; a null rec faults as in the image).
// - rec+18h..+20h = message+18h..+20h; rec+48h = 0.
// - The vector at camera+478h: begin null or size (4-byte elements) <= 2 ->
//   00BF6713; begin[2] = rec.
// - A local native string of length 0 (0041DD40(0, 1), then memcpy of
//   length + 1 bytes from the empty string 00CE3A0C when data is non-null).
// - 007A5A00(005CD160([[00E198C4]+98h]), message+24h, byte message+28h,
//   message+2Ch, &local, 0); then the local data goes back to the sized pool
//   with length + 1 bytes when non-null.
// EH frame 00C8BC08 (state 0 around the local string) is not modelled.
void cc13_sweepB2_movcam_receive_path_points_007a7c60(void* camera, const void* message, Cc13SweepB2Host& host) {
    void* msg = const_cast<void*>(message);
    host.movcam_clear_path_records_0079c830(camera);
    void* vector = static_cast<std::uint8_t*>(camera) + 0x478;
    host.pointer_vector_resize_0079eb10(vector, 3, nullptr);
    void* rec = host.crt_operator_new_00bf681b(0x4C);
    if (rec != nullptr) {
        at<std::uint32_t>(rec, 0x4) = 0;
        at<std::uint32_t>(rec, 0x8) = 0;
        at<std::uint32_t>(rec, 0xC) = 0;
        at<std::uint8_t>(rec, 0x10) = 0;
        at<std::uint32_t>(rec, 0x0) = 0x00D043D8;
        at<std::uint32_t>(rec, 0x14) = 0;
        at<float>(rec, 0x18) = 0.0f;
        at<float>(rec, 0x1C) = 0.0f;
        at<float>(rec, 0x20) = 0.0f;
        at<std::uint32_t>(rec, 0x24) = 0;
        for (std::size_t off = 0x28; off <= 0x38; off += 4) at<float>(rec, off) = 0.0f;
        at<std::uint32_t>(rec, 0x48) = 0;
        const float third = cc13_sweepB_f32(0x3EAAAAABu);
        at<float>(rec, 0x40) = third;
        at<float>(rec, 0x3C) = third;
        at<float>(rec, 0x44) = 0.0f;
    }
    at<float>(rec, 0x18) = at<float>(msg, 0x18);
    at<float>(rec, 0x1C) = at<float>(msg, 0x1C);
    at<float>(rec, 0x20) = at<float>(msg, 0x20);
    at<std::uint32_t>(rec, 0x48) = 0;
    void** begin = at<void**>(vector, 0x4);
    if (begin == nullptr || static_cast<std::uint32_t>(at<void**>(vector, 0x8) - begin) <= 2) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    at<void**>(vector, 0x4)[2] = rec;

    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, 0, 1);
    char* data = text.data;
    const std::uint32_t length = text.length;
    if (data != nullptr) host.crt_memcpy_00bf7680(data, host.image_global(0x00CE3A0C), length + 1);
    // The image reads the message fields while pushing, before 005CD160.
    const float c = at<float>(msg, 0x2C);
    const std::uint8_t b = at<std::uint8_t>(msg, 0x28);
    const float a = at<float>(msg, 0x24);
    void* holder = at<void*>(host.global_00e198c4(), 0x98);
    void* movie_camera = host.movcam_ensure_target_005cd160(holder);
    host.movcam_begin_path_walk_007a5a00(movie_camera, a, b, c, &text, 0);
    if (data != nullptr) host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, length + 1, 1);
}

}  // namespace bsp
