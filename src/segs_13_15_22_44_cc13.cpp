#define _CRT_SECURE_NO_WARNINGS  // 00788250 reproduces the image's own _sprintf.
// Segments 13, 15, 22 and 44, reconstruction pass (packet cc13-segB). See the header and
// docs/CC13_SEGS_13_15_22_44.md. Every routine names its address, its original ABI and what was
// simplified; a NativeString temporary the image builds on its stack is built here the same way
// (resize or assign, use, pool return).

#include "bsp/segs_13_15_22_44_cc13.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdio>
#include <cstring>

#include "bsp/unit_rudder.hpp"  // wrapped_angle_add_00438aa0

// 0051A590 and 005427E0 divide by a row count with no empty guard, as the image does.
#pragma warning(disable : 4724)

namespace bsp::segs_13_15_22_44_cc13 {

namespace {

// The {CRITICAL_SECTION, recursion depth} pair the image allocates through 00BD0F00-family
// helpers: depth is the dword after the 18h-byte section.
struct TrackedLock {
    CRITICAL_SECTION section;
    std::int32_t depth;
};

// The image's inline NativeString construct-from-literal: {0, 0}, 0041DD40(length, 1), then
// memcpy(length + 1) when storage was obtained.
NativeStringRaw make_literal(std::string_view text, SegBHost& host) {
    NativeStringRaw s{0, nullptr};
    host.native_string_resize_0041dd40(s, static_cast<std::uint32_t>(text.size()));
    if (s.data != nullptr) {
        std::memcpy(s.data, text.data(), s.length + 1);
    }
    return s;
}

// The image's inline NativeString destructor: return (data, length + 1) when data is set.
void release(NativeStringRaw& s, SegBHost& host) {
    if (s.data != nullptr) {
        host.pool_return_00bd1510(s.data, s.length + 1);
    }
}

// Inline `if (begin) free(begin); begin = end = capacity = 0` for a vector whose begin pointer
// is at `begin_offset`.
void free_vector(Raw object, std::size_t begin_offset, SegBHost& host) {
    if (object.ptr(begin_offset) != nullptr) {
        host.crt_free_00bf65ac(object.ptr(begin_offset));
    }
    object.ptr(begin_offset) = nullptr;
    object.ptr(begin_offset + 4) = nullptr;
    object.ptr(begin_offset + 8) = nullptr;
}

// Checked operator[] of a vector<T*> whose begin/end pointers are at begin_offset/+4h: the
// image calls 00BF6713 when begin is null or index >= size, then reads the element anyway.
void* checked_element(Raw object, std::size_t begin_offset, std::uint32_t index, SegBHost& host) {
    auto* begin = static_cast<std::uint8_t*>(object.ptr(begin_offset));
    auto* end = static_cast<std::uint8_t*>(object.ptr(begin_offset + 4));
    if (begin == nullptr || static_cast<std::uint32_t>((end - begin) >> 2) <= index) {
        host.invalid_iterator_00bf6713();
    }
    return reinterpret_cast<void**>(object.ptr(begin_offset))[index];
}

void hide(void* widget, SegBHost& host) { host.widget_set_visible_34(widget, false); }

}  // namespace

// ===========================================================================
// Segment 13: the briefing screen
// ===========================================================================

// 0051A590, __fastcall(ECX = screen), RET. Called by 0051AA70 after it moved the page index
// +120h by -1 or +1. Wraps the index as (index + count) % count with an unsigned DIV (no guard:
// an empty +114h vector divides by zero, as in the image), then shows page i (vtable +34h) when
// i equals the index and hides the others. Each element access is the checked operator[].
void briefing_wrap_and_show_page_0051a590(Raw screen, SegBHost& host) {
    auto count_of = [&]() -> std::uint32_t {
        auto* begin = static_cast<std::uint8_t*>(screen.ptr(0x114));
        if (begin == nullptr) return 0;
        return static_cast<std::uint32_t>((static_cast<std::uint8_t*>(screen.ptr(0x118)) - begin) >> 2);
    };
    const std::uint32_t added = count_of();
    const std::uint32_t divisor = count_of();
    screen.at<std::uint32_t>(0x120) = (screen.at<std::uint32_t>(0x120) + added) % divisor;
    for (std::uint32_t i = 0; screen.ptr(0x114) != nullptr && i < count_of(); ++i) {
        void* page = checked_element(screen, 0x114, i, host);
        host.widget_set_visible_34(page, i == screen.at<std::uint32_t>(0x120));
    }
}

// 0051B0E0, __fastcall(ECX = NativeString), RET: the out-of-line NativeString destructor the
// briefing unwind funclets call.
void native_string_release_0051b0e0(NativeStringRaw& s, SegBHost& host) { release(s, host); }

// 0051AEB0, __thiscall(ECX = vector, result*, {container, first}, {container, last}), RET 14h:
// the checked vector<T*>::erase(first, last). The container tests cannot fail for the callers
// here (both iterators name the same live vector), so only the move is reproduced; the result
// iterator nobody reads is not returned.
void pointer_vector_erase_0051aeb0(Raw vector, std::uint8_t* first, std::uint8_t* last,
                                   SegBHost& host) {
    (void)host;
    if (first != last) {
        auto* vec_last = static_cast<std::uint8_t*>(vector.ptr(8));
        const std::ptrdiff_t n = (vec_last - last) >> 2;
        const std::size_t bytes = static_cast<std::size_t>(n) * 4;
        if (n > 0) std::memmove(first, last, bytes);  // memmove_s(first, bytes, last, bytes)
        vector.ptr(8) = first + bytes;
    }
}

// 0051B1B0, __fastcall(ECX = screen), RET. Screen vtable 00CEC948 slot 7 (exit). Clears the
// pointer vectors +2Ch and +3Ch (begin +4h, end +8h) with erase(begin, end) after the two
// checked-iterator range tests, then hides +28h, the grid page +14h, +20h and +24h.
void briefing_exit_0051b1b0(Raw screen, SegBHost& host) {
    for (const std::size_t vec : {std::size_t{0x2C}, std::size_t{0x3C}}) {
        Raw v{screen.p + vec};
        auto* end = static_cast<std::uint8_t*>(v.ptr(8));
        if (end < static_cast<std::uint8_t*>(v.ptr(4))) host.invalid_iterator_00bf6713();
        auto* begin = static_cast<std::uint8_t*>(v.ptr(4));
        if (static_cast<std::uint8_t*>(v.ptr(8)) < begin) host.invalid_iterator_00bf6713();
        pointer_vector_erase_0051aeb0(v, begin, end, host);
    }
    hide(screen.ptr(0x28), host);
    hide(screen.ptr(0x14), host);
    hide(screen.ptr(0x20), host);
    hide(screen.ptr(0x24), host);
}

// 0051B250, __thiscall(ECX = listener = screen+8h, two unused stack arguments), RET 8. Slot 2
// of 00CEC934, the selection-changed callback of Main_Listbox. When the selected index differs
// from listener+48h + listener+44h: dim the previous row (index listener+4Ch) in both row
// vectors (+28h text rows, +38h companion rows; element +4Ch is a scene node) to 0.5 and hide
// its "Text" child, light the new row to 1.0 and show its "Text" child, and store the new index.
void briefing_selection_changed_0051b250(Raw listener, SegBHost& host) {
    const auto selected = static_cast<std::uint32_t>(
        host.list_box_selected_index_00a9c920(listener.ptr(0x10)));
    if (selected == listener.at<std::uint32_t>(0x48) + listener.at<std::uint32_t>(0x44)) return;

    auto node_of = [](void* row) { return Raw{static_cast<std::uint8_t*>(row)}.ptr(0x4C); };

    void* row = checked_element(listener, 0x28, listener.at<std::uint32_t>(0x4C), host);
    host.scene_node_set_visibility_factor_00b6da70(node_of(row), kBriefingDimmedFactor, 1);
    NativeStringRaw text{};
    host.native_string_assign_0041e870(text, kBriefingTextChild.data());
    row = checked_element(listener, 0x28, listener.at<std::uint32_t>(0x4C), host);
    host.widget_set_visible_34(host.gui_find_child_by_suffix_00aaa2b0(row, text), false);
    release(text, host);
    row = checked_element(listener, 0x38, listener.at<std::uint32_t>(0x4C), host);
    host.scene_node_set_visibility_factor_00b6da70(node_of(row), kBriefingDimmedFactor, 1);

    row = checked_element(listener, 0x28, selected, host);
    host.scene_node_set_visibility_factor_00b6da70(node_of(row), kBriefingLitFactor, 1);
    NativeStringRaw text2{};
    host.native_string_assign_0041e870(text2, kBriefingTextChild.data());
    row = checked_element(listener, 0x28, selected, host);
    host.widget_set_visible_34(host.gui_find_child_by_suffix_00aaa2b0(row, text2), true);
    release(text2, host);
    row = checked_element(listener, 0x38, selected, host);
    host.scene_node_set_visibility_factor_00b6da70(node_of(row), kBriefingLitFactor, 1);
    listener.at<std::uint32_t>(0x4C) = selected;
}

// 0051B450, __thiscall(ECX = screen, bool suppress_all, bool suppress_history), RET 8. Calls
// 0054B530 on [00E1930C] with five triples: (A2h or 0, play key or empty, placement 0 when the
// play key was set else 1), (A7h or 0, history key or empty, 1), (AFh, FE_xbox.bhelp_navigate,
// 1), (A3h, globals.back, 2), (0, "", 1). Play needs suppress_all clear and game+1FE4h zero;
// history needs suppress_all and suppress_history clear and screen+104h non-zero; both keys
// are assigned with 0041E350. Supersedes the list-returning model in src/mission_tree_screens.cpp, which
// drops the placement argument and the command-bar call.
void briefing_build_help_line_0051b450(Raw screen, bool suppress_all, bool suppress_history,
                                       SegBHost& host) {
    std::uint8_t play_code = 0;
    std::uint8_t history_code = 0;
    NativeStringRaw play{0, nullptr};
    NativeStringRaw history{0, nullptr};
    std::uint32_t play_placement = 1;
    if (!suppress_all) {
        if (host.game_e188a8().at<std::int32_t>(0x1FE4) == 0) {
            play_code = kHelpPlayCode;
            host.native_string_assign_cstring_0041e350(play, kHelpPlayKey.data());
            play_placement = 0;
        }
        if (!suppress_history && screen.at<std::int32_t>(0x104) != 0) {
            history_code = kHelpHistoryCode;
            host.native_string_assign_cstring_0041e350(history, kHelpHistoryKey.data());
        }
    }
    NativeStringRaw empty = make_literal(kHelpEmptyKey, host);
    NativeStringRaw back = make_literal(kHelpBackKey, host);
    NativeStringRaw navigate = make_literal(kHelpNavigateKey, host);
    host.command_bar_rebuild_0054b530(host.command_bar_e1930c(), play_code, &play, play_placement,
                                      history_code, &history, 1, kHelpNavigateCode, &navigate, 1,
                                      kHelpBackCode, &back, 2, 0, &empty, 1);
    release(navigate, host);
    release(back, host);
    release(empty, host);
    release(history, host);
    release(play, host);
}

// 0051B6C0, __thiscall(ECX = pair, const NativeString* key, const dword* value), RET 8.
Raw string_pair_construct_0051b6c0(Raw self, const NativeStringRaw& key, const std::uint32_t& value,
                                   SegBHost& host) {
    auto& s = self.at<NativeStringRaw>(0);
    s = {0, nullptr};
    if (reinterpret_cast<const void*>(self.p) != &key) {
        host.native_string_resize_0041dd40(s, key.length);
        if (key.length != 0) std::memcpy(s.data, key.data, s.length);
    }
    self.at<std::uint32_t>(8) = value;
    return self;
}

// 0051B710, __thiscall(ECX = pair, const pair* source), RET 4: the same copy with the dword
// taken from source+8h.
Raw string_pair_copy_0051b710(Raw self, Raw source, SegBHost& host) {
    auto& s = self.at<NativeStringRaw>(0);
    const auto& key = source.at<NativeStringRaw>(0);
    s = {0, nullptr};
    if (self.p != source.p) {
        host.native_string_resize_0041dd40(s, key.length);
        if (key.length != 0) std::memcpy(s.data, key.data, s.length);
    }
    self.at<std::uint32_t>(8) = source.at<std::uint32_t>(8);
    return self;
}

// 0051B8C0, __thiscall(ECX = listener = screen+8h, two unused stack arguments), RET 8. Slot 1
// of 00CEC934 (the list-box activate callback). When the selected index equals listener+48h +
// listener+44h: in a session (game+1FE4h non-zero) 006881F0 on the multiplayer menu, otherwise
// a tail JMP to 004CC460 on the front-end manager with its own stack slots rewritten to (2, 0).
void briefing_list_activated_0051b8c0(Raw listener, SegBHost& host) {
    const std::int32_t selected = host.list_box_selected_index_00a9c920(listener.ptr(0x10));
    if (selected != listener.at<std::int32_t>(0x48) + listener.at<std::int32_t>(0x44)) return;
    if (host.game_e188a8().at<std::int32_t>(0x1FE4) != 0) {
        host.multi_menu_pop_request_interface_006881f0(host.multi_menu_e198b4());
        return;
    }
    host.front_end_push_interface_request_004cc460(host.main_front_end_e198ac(), 2, 0);
}

// 0051CBB0, __fastcall(ECX = mission data = screen+C4h), RET. Destroys the NativeString vectors
// at +2Ch (begin +30h) and +1Ch (begin +20h) through 00432050 then frees them, then returns
// the strings at +14h and +0h to the pool.
void briefing_mission_data_destroy_0051cbb0(Raw data, SegBHost& host) {
    for (const std::size_t vec : {std::size_t{0x2C}, std::size_t{0x1C}}) {
        Raw v{data.p + vec};
        if (v.ptr(4) != nullptr) {
            host.native_string_range_destroy_00432050(v.ptr(4), v.ptr(8), v);
            host.crt_free_00bf65ac(v.ptr(4));
        }
        v.ptr(4) = nullptr;
        v.ptr(8) = nullptr;
        v.ptr(0xC) = nullptr;
    }
    release(data.at<NativeStringRaw>(0x14), host);
    release(data.at<NativeStringRaw>(0x0), host);
}

// 0051CB60, __fastcall(ECX = mission data), returns it: the constructor matching 0051CBB0,
// zeroing the strings +0h / +14h and the vectors +1Ch / +2Ch (begin, end, capacity). No
// direct caller (0051E4D0 zeroes the same fields of screen+C4h inline).
Raw briefing_mission_data_construct_0051cb60(Raw data) {
    for (const std::size_t off : {0x0, 0x4, 0x14, 0x18, 0x20, 0x24, 0x28, 0x30, 0x34, 0x38}) {
        data.at<std::uint32_t>(off) = 0;
    }
    return data;
}

// 0051E050, __fastcall(ECX = screen), RET. Body of the scalar deleting destructor 0051E5F0.
// Re-installs the three vtables, destroys the pages +10h and +14h, then every page held in the
// tree at +12Ch (head +130h; node value page at node+14h), erases the tree and frees its head,
// frees the vector +114h, destroys the mission data +C4h (0051CBB0), frees the vector +B8h,
// returns the string +A8h, frees the vectors +78h, +40h and +30h, puts the base listener vtable
// back at +8h and runs the front-end screen destructor. The checked-iterator tests in the tree
// walk compare the tree's own address against null and cannot fire; they are not reproduced.
void briefing_destruct_0051e050(Raw screen, SegBHost& host) {
    screen.at<std::uint32_t>(0x0) = kBriefingVtable;
    screen.at<std::uint32_t>(0x8) = kBriefingListboxListenerVtable;
    screen.at<std::uint32_t>(0xC) = kBriefingWidgetListenerVtable;
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0x10));
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0x14));
    Raw tree{screen.p + 0x12C};
    void* node = Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(0);  // head->left = begin
    while (node != tree.ptr(4)) {
        host.gui_remove_and_destroy_page_00aa31f0(Raw{static_cast<std::uint8_t*>(node)}.ptr(0x14));
        host.tree_iter_inc_0051a850(node);
    }
    host.tree_erase_range_0051ca90(tree, Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(0),
                                   tree.ptr(4));
    host.crt_free_00bf65ac(tree.ptr(4));
    screen.ptr(0x130) = nullptr;
    screen.ptr(0x134) = nullptr;
    free_vector(screen, 0x114, host);
    briefing_mission_data_destroy_0051cbb0(Raw{screen.p + 0xC4}, host);
    free_vector(screen, 0xB8, host);
    release(screen.at<NativeStringRaw>(0xA8), host);
    free_vector(screen, 0x78, host);
    free_vector(screen, 0x40, host);
    free_vector(screen, 0x30, host);
    screen.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(screen);
}

// 0051E250, 'mov eax,3; ret': screen vtable slot 0, the INTF_BRIEFING id.
int briefing_screen_id_0051e250() { return kBriefingScreenId; }

// 0051E270, __thiscall(ECX = screen, vector* out), RET 4: push_back(screen+10h) into the
// output pointer vector (004D6790 with ECX = out).
void briefing_collect_children_0051e270(Raw screen, void* out_vector, SegBHost& host) {
    host.pointer_vector_push_back_004d6790(out_vector, screen.ptr(0x10));
}

// 0051E280, __fastcall(ECX = screen), RET. Screen vtable slot 4 (register). Base register
// 004F71D0, page FE_briefing_listbox -> +10h, the objective pages (0051DDA0, ECX = screen),
// page FE_briefing_grid -> +14h hidden at once, child Main_Listbox of +10h -> +18h, listener
// screen+8h on it (00A9AC40, ECX = list box), child MainListbox_Text of +18h -> +1Ch.
void briefing_register_0051e280(Raw screen, SegBHost& host) {
    host.front_end_screen_register_004f71d0(screen);
    NativeStringRaw name = make_literal(kBriefingListboxPage, host);
    screen.ptr(0x10) = host.gui_load_page_00aa5840(name);
    release(name, host);
    host.briefing_load_objective_pages_0051dda0(screen);
    name = make_literal(kBriefingGridPage, host);
    screen.ptr(0x14) = host.gui_load_page_00aa5840(name);
    release(name, host);
    hide(screen.ptr(0x14), host);
    name = make_literal(kBriefingMainListbox, host);
    screen.ptr(0x18) = host.gui_find_child_00aa7e00(screen.ptr(0x10), name);
    release(name, host);
    host.list_box_set_listener_00a9ac40(screen.ptr(0x18), screen.p + 0x8);
    name = make_literal(kBriefingListboxText, host);
    screen.ptr(0x1C) = host.gui_find_child_00aa7e00(screen.ptr(0x18), name);
    release(name, host);
}

// 0051E4D0, __fastcall(ECX = screen), returns screen. Base constructor 004F7180, the base
// listener vtables then the briefing vtables at +0h/+8h/+Ch, the zeroed fields, and the tree
// at +12Ch: head node from 0051ACB0 (ECX = tree) marked as nil (+19h = 1) and linked to
// itself, size +134h = 0.
Raw briefing_construct_0051e4d0(Raw screen, SegBHost& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    screen.at<std::uint32_t>(0xC) = kWidgetListenerBaseVtable;
    screen.at<std::uint32_t>(0x0) = kBriefingVtable;
    screen.at<std::uint32_t>(0x8) = kBriefingListboxListenerVtable;
    screen.at<std::uint32_t>(0xC) = kBriefingWidgetListenerVtable;
    for (const std::size_t off : {0x30, 0x34, 0x38, 0x40, 0x44, 0x48, 0x78, 0x7C, 0x80, 0xA8, 0xAC,
                                  0xB8, 0xBC, 0xC0, 0xC4, 0xC8, 0xD8, 0xDC, 0xE4, 0xE8, 0xEC, 0xF4,
                                  0xF8, 0xFC, 0x114, 0x118, 0x11C}) {
        screen.at<std::uint32_t>(off) = 0;
    }
    void* head = host.tree_buy_head_node_0051acb0(Raw{screen.p + 0x12C});
    screen.ptr(0x130) = head;
    Raw node{static_cast<std::uint8_t*>(head)};
    node.at<std::uint8_t>(0x19) = 1;
    Raw{static_cast<std::uint8_t*>(screen.ptr(0x130))}.ptr(4) = screen.ptr(0x130);
    Raw{static_cast<std::uint8_t*>(screen.ptr(0x130))}.ptr(0) = screen.ptr(0x130);
    Raw{static_cast<std::uint8_t*>(screen.ptr(0x130))}.ptr(8) = screen.ptr(0x130);
    screen.at<std::uint32_t>(0x134) = 0;
    return screen;
}

// 0051E650, __thiscall(ECX = view mover, float delta), RET 4. +388h = 00438AA0(+388h, delta)
// stored as float, then clamped: below +3ECh takes +3ECh, else above +3F0h takes +3F0h (the
// FCOMIP/JBE pair leaves a NaN unclamped).
void binoculars_add_clamped_pitch_0051e650(Raw mover, float delta) {
    const float pitch = wrapped_angle_add_00438aa0(mover.at<float>(0x388), delta);
    mover.at<float>(0x388) = pitch;
    if (mover.at<float>(0x3EC) > pitch) {
        mover.at<float>(0x388) = mover.at<float>(0x3EC);
        return;
    }
    if (pitch > mover.at<float>(0x3F0)) {
        mover.at<float>(0x388) = mover.at<float>(0x3F0);
    }
}

// 0051E6E0, __fastcall(ECX = binoculars screen), RET. 004DC940 on the game with (+38h zoom
// fov, 1); with a mover at +40h: 0042A9C0 (mover+3A0h = (+30h == 1), inlined here as the byte
// store it is), 0051E650(0.0), then the mover's vtable +DCh(0.0).
void binoculars_apply_fov_0051e6e0(Raw screen, SegBHost& host) {
    host.game_set_view_fov_004dc940(host.game_e188a8(), screen.at<float>(0x38), 1);
    if (screen.ptr(0x40) == nullptr) return;
    Raw mover{static_cast<std::uint8_t*>(screen.ptr(0x40))};
    mover.at<std::uint8_t>(0x3A0) = screen.at<std::int32_t>(0x30) == 1 ? 1 : 0;
    binoculars_add_clamped_pitch_0051e650(mover, 0.0f);
    host.unit_virtual_dc(screen.ptr(0x40), 0.0f);
}

// ===========================================================================
// Segment 15
// ===========================================================================

// 00527E40, __thiscall(ECX = control layout screen, flags), RET 4, returns the screen: the
// scalar deleting destructor of vtable 00CECF48 (destructor 00527DA0, then _free when bit 0).
Raw control_layout_scalar_delete_00527e40(Raw screen, std::uint32_t flags, SegBHost& host) {
    host.control_layout_destruct_00527da0(screen);
    if ((flags & 1) != 0) host.crt_free_00bf65ac(screen.p);
    return screen;
}

// 00534690, __thiscall(ECX = out matrix, float sx, float sy, float sz), RET 0Ch. No caller in
// the image. Builds diag(sx, sy, sz, 1.0f) on the stack (row-major 4x4, all other entries
// 0.0f) and copies it into the output with 004134F0.
void scale_matrix_00534690(void* out, float sx, float sy, float sz, SegBHost& host) {
    const float m[16] = {sx, 0.0f, 0.0f, 0.0f, 0.0f, sy, 0.0f, 0.0f,
                         0.0f, 0.0f, sz, 0.0f, 0.0f, 0.0f, 0.0f, kMatrixUnitW};
    host.matrix_copy_004134f0(out, m);
}

// 00534720, __thiscall(ECX = out matrix, float s), RET 4, returns the output: diag(s, s, s,
// 1.0f) through 004134F0. No caller in the image.
void* uniform_scale_matrix_00534720(void* out, float s, SegBHost& host) {
    const float m[16] = {s, 0.0f, 0.0f, 0.0f, 0.0f, s, 0.0f, 0.0f,
                         0.0f, 0.0f, s, 0.0f, 0.0f, 0.0f, 0.0f, kMatrixUnitW};
    host.matrix_copy_004134f0(out, m);
    return out;
}

// 00535B20, __fastcall(ECX = screen), RET. Builds an 18h-byte command message {word 0, word 0,
// dword 0, float [00F87574], [00F87578], [00F8757C], float 0.0} on the stack and issues it on
// the entity [[screen+138h]+14h] through 0077D600(00E08FB8, &message, 1), then 0068AB80(1, 1)
// on the in-game interface [00E198C4]. Only 00535F30 reaches it.
void issue_move_to_marker_00535b20(Raw screen, SegBHost& host) {
    struct Message {
        std::uint16_t w0;
        std::uint16_t w2;
        std::uint32_t d4;
        float x, y, z;
        float w;
    };
    static_assert(sizeof(Message) == 0x18);
    const float* pos = host.position_f87574();
    const Message message{0, 0, 0, pos[0], pos[1], pos[2], 0.0f};
    void* owner = screen.ptr(0x138);
    void* entity = Raw{static_cast<std::uint8_t*>(owner)}.ptr(0x14);
    host.entity_issue_command_0077d600(entity, kIssueCommandDescriptor, &message, 1);
    host.interface_collapse_overlays_0068ab80(host.interface_e198c4(), 1, 1);
}

// 00535F30, __fastcall(ECX = result code), RET: when the code is 1, tail JMP to 00535B20 with
// ECX = [[00E198C4]+5Ch].
void on_confirm_issue_move_00535f30(std::int32_t code, SegBHost& host) {
    if (code != 1) return;
    issue_move_to_marker_00535b20(
        Raw{static_cast<std::uint8_t*>(host.interface_e198c4().ptr(0x5C))}, host);
}

// 00541A10, __thiscall(ECX = command bar, index), RET 4. Buttons are 8-byte {widget, enabled
// byte} elements of the vector at +8h (end +Ch), the selection is +14h. The button's widget
// vtable +88h gets (2, 0, 1.0f) when disabled, (1, 0, 1.0f) when it is the selection, else (0,
// 0, 1.0f). Each access is a checked operator[].
void command_bar_paint_button_00541a10(Raw bar, std::uint32_t index, SegBHost& host) {
    auto check = [&]() {
        auto* begin = static_cast<std::uint8_t*>(bar.ptr(8));
        if (begin == nullptr ||
            static_cast<std::uint32_t>((static_cast<std::uint8_t*>(bar.ptr(0xC)) - begin) >> 3) <= index) {
            host.invalid_iterator_00bf6713();
        }
    };
    auto button = [&]() { return Raw{static_cast<std::uint8_t*>(bar.ptr(8)) + index * 8}; };
    check();
    std::int32_t state;
    if (button().at<std::uint8_t>(4) == 0) {
        state = 2;
    } else if (index == bar.at<std::uint32_t>(0x14)) {
        state = 1;
    } else {
        state = 0;
    }
    check();
    host.widget_virtual_88(button().ptr(0), state, 0, 1.0f);
}

namespace {
std::uint32_t text_row_count(Raw list) {
    auto* begin = static_cast<std::uint8_t*>(list.ptr(0x14));
    if (begin == nullptr) return 0;
    return static_cast<std::uint32_t>((static_cast<std::uint8_t*>(list.ptr(0x18)) - begin) / 0x24);
}
}  // namespace

// 005427E0, __fastcall(ECX = debug text list), RET. Rows are 24h bytes in the vector +14h (end
// +18h), byte +0h enabled; the cursor is +20h. With rows present, step the cursor forward
// modulo the count until an enabled row or the start is reached (checked operator[] each
// step), store it, and tail JMP to 00542620.
void debug_text_list_next_enabled_005427e0(Raw list, SegBHost& host) {
    if (list.ptr(0x14) == nullptr || text_row_count(list) == 0) return;
    std::uint32_t index = list.at<std::uint32_t>(0x20);
    do {
        index = (index + 1) % text_row_count(list);
        if (list.ptr(0x14) == nullptr || text_row_count(list) <= index) host.invalid_iterator_00bf6713();
    } while (static_cast<std::uint8_t*>(list.ptr(0x14))[index * 0x24] == 0 &&
             index != list.at<std::uint32_t>(0x20));
    list.at<std::uint32_t>(0x20) = index;
    host.debug_text_list_refresh_rows_00542620(list);
}

// 00542890, __fastcall(ECX = debug text list), RET: as 005427E0 stepping backwards; a negative
// cursor wraps to count - 1.
void debug_text_list_previous_enabled_00542890(Raw list, SegBHost& host) {
    if (list.ptr(0x14) == nullptr || text_row_count(list) == 0) return;
    std::uint32_t index = list.at<std::uint32_t>(0x20);
    do {
        index -= 1;
        if (static_cast<std::int32_t>(index) < 0) index = text_row_count(list) - 1;
        if (list.ptr(0x14) == nullptr || text_row_count(list) <= index) host.invalid_iterator_00bf6713();
    } while (static_cast<std::uint8_t*>(list.ptr(0x14))[index * 0x24] == 0 &&
             index != list.at<std::uint32_t>(0x20));
    list.at<std::uint32_t>(0x20) = index;
    host.debug_text_list_refresh_rows_00542620(list);
}

// 005431A0 / 00543210, __thiscall(ECX = command bar, bool visible), RET 4: every button widget
// of the 8-byte {widget, enabled} vector at +8h (vector object +4h) gets vtable +34h(visible).
// The checked-iterator tests are reproduced except the container-identity test (edi == edi),
// which cannot fail.
void button_vector_show_all_005431a0(Raw bar, bool visible, SegBHost& host) {
    auto* it = static_cast<std::uint8_t*>(bar.ptr(8));
    if (static_cast<std::uint8_t*>(bar.ptr(0xC)) < it) host.invalid_iterator_00bf6713();
    for (;;) {
        auto* end = static_cast<std::uint8_t*>(bar.ptr(0xC));
        if (static_cast<std::uint8_t*>(bar.ptr(8)) > end) host.invalid_iterator_00bf6713();
        if (it == end) break;
        if (it >= static_cast<std::uint8_t*>(bar.ptr(0xC))) host.invalid_iterator_00bf6713();
        host.widget_set_visible_34(Raw{it}.ptr(0), visible);
        if (it >= static_cast<std::uint8_t*>(bar.ptr(0xC))) host.invalid_iterator_00bf6713();
        it += 8;
    }
}

// 00544A70, __fastcall(ECX = debug text list), RET. Called by the list destructor 00590250.
// Deletes each row's +20h object (vtable +4h(1)) when set, then erases [begin, end) of the row
// vector (+10h object, begin +14h, end +18h): 00543CA0(end, end, begin) moves nothing, the new
// end is begin + 0 rows, 00543C20 destroys [new end, old end), and +18h = new end.
void debug_text_list_clear_00544a70(Raw list, SegBHost& host) {
    for (std::uint32_t i = 0; list.ptr(0x14) != nullptr && i < text_row_count(list); ++i) {
        if (list.ptr(0x14) == nullptr || text_row_count(list) <= i) host.invalid_iterator_00bf6713();
        void* object = Raw{static_cast<std::uint8_t*>(list.ptr(0x14)) + i * 0x24}.ptr(0x20);
        if (object != nullptr) host.object_deleting_dtor_04(object, 1);
    }
    auto* end = static_cast<std::uint8_t*>(list.ptr(0x18));
    if (static_cast<std::uint8_t*>(list.ptr(0x14)) > end) host.invalid_iterator_00bf6713();
    auto* begin = static_cast<std::uint8_t*>(list.ptr(0x14));
    if (begin > static_cast<std::uint8_t*>(list.ptr(0x18))) host.invalid_iterator_00bf6713();
    if (begin == end) return;
    auto* last = static_cast<std::uint8_t*>(list.ptr(0x18));
    host.text_row_copy_00543ca0(end, last, begin);
    auto* new_end = begin + ((last - end) / 0x24) * 0x24;
    host.text_row_destroy_range_00543c20(new_end, list.ptr(0x18), Raw{list.p + 0x10});
    list.ptr(0x18) = new_end;
}

// ===========================================================================
// Segment 44: network packet statistics
// ===========================================================================

namespace {
void record_packet(Raw stats, std::uint8_t* packet, std::int32_t length, std::int32_t direction,
                   SegBHost& host) {
    if (stats.at<std::uint8_t>(kPacketStatsEnabledOffset) == 0 || packet == nullptr) return;
    auto* lock = static_cast<TrackedLock*>(stats.ptr(kPacketStatsLockOffset));
    EnterCriticalSection(&lock->section);
    lock->depth += 1;
    host.packet_stats_accumulate_00787850(stats, packet + 8, length - 8, 0, direction);
    lock = static_cast<TrackedLock*>(stats.ptr(kPacketStatsLockOffset));  // re-read, as the image
    lock->depth -= 1;
    LeaveCriticalSection(&lock->section);
}
}  // namespace

// 00787AA0, __thiscall(ECX = stats, peer (unused), packet, length), RET 0Ch. Called by the
// receive worker 00A3CAF0. When +762900h is set and the packet is non-null, under the tracked
// lock at [+7628FCh]: 00787850(packet + 8, length - 8, 0, 0).
void packet_stats_record_received_00787aa0(Raw stats, void* peer, std::uint8_t* packet,
                                           std::int32_t length, SegBHost& host) {
    (void)peer;
    record_packet(stats, packet, length, 0, host);
}

// 00787B00: as 00787AA0 with direction 1 (the send worker 00A3C5E0).
void packet_stats_record_sent_00787b00(Raw stats, void* peer, std::uint8_t* packet,
                                       std::int32_t length, SegBHost& host) {
    (void)peer;
    record_packet(stats, packet, length, 1, host);
}

// 00787B60, __fastcall(ECX = stats), returns stats. Vtable 00D04318, then under the singleton
// manager's lock (00415350 then manager +10h) publish [00F871B4] = stats and register it
// (00BD0C30). The lock/manager fetch is the shared SingletonLifetimeCalls contract of 00994540.
void packet_stats_construct_base_00787b60(Raw stats, void*& published_f871b4,
                                          SingletonLifetimeCalls& lifetime) {
    stats.at<std::uint32_t>(0) = kPacketStatsBaseVtable;
    lifetime.lock_manager();
    published_f871b4 = stats.p;
    lifetime.register_singleton(published_f871b4);
    lifetime.unlock_manager();
}

// 00787C00, __fastcall(ECX = stats), RET. Vtable 00D04318, under the lock unregister
// [00F871B4] (00BCFCA0) and clear it, then the singleton-base vtable 00CE3818.
void packet_stats_destruct_base_00787c00(Raw stats, void*& published_f871b4,
                                         SingletonLifetimeCalls& lifetime) {
    stats.at<std::uint32_t>(0) = kPacketStatsBaseVtable;
    lifetime.lock_manager();
    lifetime.unregister_singleton(published_f871b4);
    published_f871b4 = nullptr;
    lifetime.unlock_manager();
    stats.at<std::uint32_t>(0) = kSingletonBaseVtable;
}

// 00788250, __thiscall(ECX = {int a, int b} counts, int type, stream*), RET 8. Called by the
// CSV row writer 007884D0. When both counts are non-zero: _sprintf(";%s;%d;%d", name[type], a,
// b) into a 200-byte stack buffer, a NativeString of it (0041E870), the stream's vtable
// +5Ch(&string, 0), then the string returned to the pool.
void packet_stats_format_type_column_00788250(Raw counts, std::int32_t type, void* stream,
                                              SegBHost& host) {
    if (counts.at<std::int32_t>(0) == 0 || counts.at<std::int32_t>(4) == 0) return;
    char buffer[200];
    std::sprintf(buffer, ";%s;%d;%d", host.packet_type_name_e0ab68(type), counts.at<std::int32_t>(0),
                 counts.at<std::int32_t>(4));
    NativeStringRaw text{};
    host.native_string_assign_0041e870(text, buffer);
    host.stream_virtual_5c(stream, &text, 0);
    release(text, host);
}

// 00788380, __thiscall(ECX = prefix NativeString, NativeString* out, unsigned value), RET 8,
// returns out. out = copy of prefix (resize to its length, memcpy length bytes when non-empty;
// skipped when out aliases prefix), then the decimal string of value from 005F1750 appended
// (resize to old + n, memcpy n bytes at the old end) when non-empty; the temporary is returned
// to the pool.
NativeStringRaw& packet_stats_format_field_00788380(const NativeStringRaw& prefix,
                                                    NativeStringRaw& out, std::uint32_t value,
                                                    SegBHost& host) {
    out = {0, nullptr};
    if (&out != &prefix) {
        host.native_string_resize_0041dd40(out, prefix.length);
        if (prefix.length != 0) std::memcpy(out.data, prefix.data, out.length);
    }
    NativeStringRaw digits{};
    host.native_string_from_unsigned_005f1750(digits, value);
    const std::uint32_t n = digits.length;
    if (n != 0) {
        const std::uint32_t old = out.length;
        host.native_string_resize_0041dd40(out, n + old);
        std::memcpy(out.data + old, digits.data, n);
    }
    release(digits, host);
    return out;
}

// 0078C280, __fastcall(ECX = stats), RET. Body of the scalar deleting destructor 0078C770.
// Vtable 00D04388; the tracked lock [+7628FCh], when present, is left as many times as its
// depth counts, deleted and freed; the NativeString +7649ACh returned; the vectors +762920h and
// +762910h (begin +4h, end +8h) destroyed through 0078BA70 and freed; the tree +762904h erased
// (0078BDB0) and its head freed; then the singleton-base destructor 00787C00.
void packet_stats_destruct_0078c280(Raw stats, void*& published_f871b4,
                                    SingletonLifetimeCalls& lifetime, SegBHost& host) {
    stats.at<std::uint32_t>(0) = kPacketStatsVtable;
    auto* lock = static_cast<TrackedLock*>(stats.ptr(kPacketStatsLockOffset));
    if (lock != nullptr) {
        while (lock->depth > 0) {
            lock->depth -= 1;
            LeaveCriticalSection(&lock->section);
        }
        DeleteCriticalSection(&lock->section);
        host.crt_free_00bf65ac(lock);
    }
    release(stats.at<NativeStringRaw>(kPacketStatsStringOffset), host);
    for (const std::size_t vec : {kPacketStatsVectorBOffset, kPacketStatsVectorAOffset}) {
        Raw v{stats.p + vec};
        if (v.ptr(4) != nullptr) {
            host.range_destroy_0078ba70(v.ptr(4), v.ptr(8), stats);
            host.crt_free_00bf65ac(v.ptr(4));
        }
        v.ptr(4) = nullptr;
        v.ptr(8) = nullptr;
        v.ptr(0xC) = nullptr;
    }
    Raw tree{stats.p + kPacketStatsTreeOffset};
    host.tree_erase_range_0078bdb0(tree, Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(0),
                                   tree.ptr(4));
    host.crt_free_00bf65ac(tree.ptr(4));
    tree.ptr(4) = nullptr;
    tree.ptr(8) = nullptr;
    packet_stats_destruct_base_00787c00(stats, published_f871b4, lifetime);
}

// 0078C400, __fastcall(ECX = stats), returns stats. BSP_Application_Initialize allocates the
// 7649B4h bytes and runs it when [00F871B4] is null. In order: the singleton base 00787B60;
// vtable 00D04388; 2002 per-message records of EF4h from +4h (239 entries of 10h built by
// 00403560 with constructor 00786C40, word +EF0h = 20000, word +EF2h = 0); 5005 slots of 10h
// from +74F02Ch zeroed with word +Ch = 9999; the tree +762904h with a head from 00787DA0 (nil
// byte +21h); zeroed vectors +762910h / +762920h and string +7649ACh; enabled byte +762900h
// cleared; the lock +7628FCh from 00BD1860; the message-type map {1: C1h, C2h; 2: AEh, 15h; 3:
// 55h, 8Ch} through map[key].push_back(type); ten passes writing byte FFh at +762933h + 4i and
// 0.02f at +76493Ch + 4i and +76493Ch + 8i; the colours FF0000FFh, FFFF00FFh, FF00FF00h,
// FFFF0000h at +762930h..+76293Ch; the scalars below; memset(+74F02Ch, 0, 138D0h) (which
// clears the 9999 words again); +764938h = 0.7f; and the inline assign of "" (00CE3A0C) to the
// string +7649ACh.
Raw packet_stats_construct_0078c400(Raw stats, void*& published_f871b4,
                                    SingletonLifetimeCalls& lifetime, SegBHost& host) {
    packet_stats_construct_base_00787b60(stats, published_f871b4, lifetime);
    stats.at<std::uint32_t>(0) = kPacketStatsVtable;
    std::uint8_t* record = stats.p + 4;
    for (std::int32_t i = 0x7D1; i >= 0; --i) {
        host.vector_constructor_iterator_00403560(record, 0x10, 0xEF, 0x00786C40);
        Raw{record}.at<std::uint16_t>(0xEF0) = 20000;
        Raw{record}.at<std::uint16_t>(0xEF2) = 0;
        record += 0xEF4;
    }
    std::uint8_t* slot = stats.p + 0x74F02C;
    for (std::int32_t i = 0x138C; i >= 0; --i) {
        Raw s{slot};
        s.at<std::uint32_t>(0x0) = 0;
        s.at<std::uint32_t>(0x4) = 0;
        s.at<std::uint32_t>(0x8) = 0;
        s.at<std::uint32_t>(0xC) = 0;
        s.at<std::uint16_t>(0xC) = 9999;
        slot += 0x10;
    }
    Raw tree{stats.p + kPacketStatsTreeOffset};
    tree.ptr(4) = host.tree_buy_head_node_00787da0(tree);
    Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.at<std::uint8_t>(0x21) = 1;
    Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(4) = tree.ptr(4);
    Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(0) = tree.ptr(4);
    Raw{static_cast<std::uint8_t*>(tree.ptr(4))}.ptr(8) = tree.ptr(4);
    tree.at<std::uint32_t>(8) = 0;
    for (const std::size_t off : {0x762914, 0x762918, 0x76291C, 0x762924, 0x762928, 0x76292C,
                                  0x7649AC, 0x7649B0}) {
        stats.at<std::uint32_t>(off) = 0;
    }
    stats.at<std::uint8_t>(kPacketStatsEnabledOffset) = 0;
    stats.ptr(kPacketStatsLockOffset) = host.critical_section_create_00bd1860();
    const std::int32_t type_map[6][2] = {{1, 0xC1}, {1, 0xC2}, {2, 0xAE}, {2, 0x15}, {3, 0x55}, {3, 0x8C}};
    for (const auto& entry : type_map) {
        host.int_vector_push_back_0078b560(host.type_map_subscript_0078c0e0(tree, entry[0]), entry[1]);
    }
    constexpr float kInitialRate = 0.02f;  // 00CE9BAC
    for (std::uint32_t i = 0; i < 10; ++i) {
        stats.at<std::uint8_t>(0x762933 + 4 * i) = 0xFF;
        stats.at<float>(0x76493C + 4 * i) = kInitialRate;
        stats.at<float>(0x76493C + 8 * i) = kInitialRate;
    }
    stats.at<std::uint32_t>(0x762930) = 0xFF0000FFu;
    stats.at<std::uint32_t>(0x762938) = 0xFF00FF00u;
    stats.at<std::uint32_t>(0x762934) = 0xFFFF00FFu;
    stats.at<float>(0x762964) = 0.1f;               // 00D7A2F0
    stats.at<std::uint32_t>(0x76293C) = 0xFFFF0000u;
    stats.at<std::uint32_t>(0x764990) = 0;
    stats.at<std::uint8_t>(0x764934) = 1;
    stats.at<std::uint32_t>(0x76499C) = 0xFA;
    stats.at<std::uint32_t>(0x76296C) = 10;
    stats.at<float>(0x762970) = 8192.0f;            // 00D0438C
    stats.at<std::uint8_t>(0x7649A8) = 0;
    std::memset(stats.p + 0x74F02C, 0, 0x138D0);
    stats.at<float>(0x764938) = 0.7f;               // 00CE3E18
    auto& text = stats.at<NativeStringRaw>(kPacketStatsStringOffset);
    if (text.length != 0) {
        release(text, host);
        text.data = nullptr;
        text.length = 0;
    }
    if (text.data != nullptr) std::memcpy(text.data, "", text.length);
    return stats;
}

// 0078C790, __fastcall(ECX = slot), RET: when set, the object's +4h subobject vtable +4h(1)
// (ECX = object+4h), then slot = 0.
void owned_ptr_delete_sub4_0078c790(void*& slot, SegBHost& host) {
    if (slot == nullptr) return;
    host.object_deleting_dtor_04(static_cast<std::uint8_t*>(slot) + 4, 1);
    slot = nullptr;
}

// 0078C7B0 / 0078C7D0, __fastcall(ECX = slot), RET: when set, vtable +4h(1), slot = 0.
void owned_ptr_delete_0078c7b0(void*& slot, SegBHost& host) {
    if (slot == nullptr) return;
    host.object_deleting_dtor_04(slot, 1);
    slot = nullptr;
}

// 0078D540 / 0078D570 / 0078D5A0, __fastcall(ECX = slot), returns the slot: when set,
// InterlockedDecrement(object+4h) and on zero the object's vtable +0h() (ECX = object); slot =
// 0. 0078D820 is the same body that does not return the slot.
void** refptr_release_0078d540(void*& slot, SegBHost& host) {
    void* object = slot;
    if (object != nullptr) {
        auto* count = reinterpret_cast<volatile LONG*>(static_cast<std::uint8_t*>(object) + 4);
        if (InterlockedDecrement(count) == 0) host.object_destroy_slot0(object);
        slot = nullptr;
    }
    return &slot;
}

// 0078D5D0, __fastcall(ECX = ocean environment), RET. Body of the scalar deleting destructor
// 0078D6A0: vtable 00D04390, the refcounted +B4h released (0078D540 inlined), +A8h deleted
// through its +4h subobject (0078C790 inlined), +ACh and +B0h deleted (0078C7B0 inlined), then
// the refcounted base destructor 00BD30F0.
void ocean_environment_destruct_0078d5d0(Raw ocean, SegBHost& host) {
    ocean.at<std::uint32_t>(0) = kRefCountedVtable_d04390;
    refptr_release_0078d540(ocean.ptr(0xB4), host);
    owned_ptr_delete_sub4_0078c790(ocean.ptr(0xA8), host);
    owned_ptr_delete_0078c7b0(ocean.ptr(0xAC), host);
    owned_ptr_delete_0078c7b0(ocean.ptr(0xB0), host);
    host.refcounted_destroy_base_00bd30f0(ocean);
}

// 0078D6C0, __thiscall(ECX = game world, p.x, p.y, p.z, q.x, q.y, q.z by value, float* out),
// RET 1Ch, returns AL. Sole caller 0053F2D0 (free-camera effects). The sampler 0078CF20 is
// called with the first two components of each point and its ST0 result is compared against
// the second component (as the image does; whether the caller's vectors are (x, z, y) was not
// traced). p above (p[1] > h(p)) and q not above (q[1] <= h(q)): out = 0078D1B0(p, q). p not
// above and q above: out = 0078D1B0(q, p). Otherwise false. The three floats are copied from
// the returned pointer.
bool water_segment_crossing_0078d6c0(Raw world, const float (&p)[3], const float (&q)[3],
                                     float (&out)[3], SegBHost& host) {
    float crossing[3];
    const float* result = nullptr;
    // FCOMIP + JBE: an unordered compare takes the "not above" path.
    if (!(static_cast<double>(p[1]) > host.water_height_0078cf20(world, p[0], p[1]))) {
        if (!(static_cast<double>(q[1]) > host.water_height_0078cf20(world, q[0], q[1]))) return false;
        result = host.water_bisect_crossing_0078d1b0(world, crossing, q, p);
    } else {
        if (static_cast<double>(q[1]) > host.water_height_0078cf20(world, q[0], q[1])) return false;
        result = host.water_bisect_crossing_0078d1b0(world, crossing, p, q);
    }
    out[0] = result[0];
    out[1] = result[1];
    out[2] = result[2];
    return true;
}

}  // namespace bsp::segs_13_15_22_44_cc13
