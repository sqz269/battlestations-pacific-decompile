// Front-end and HUD screens of segments 15 and 22 (packet cc13-fe4).
// Evidence and status: docs/CC13_FE_4.md. Every routine was written from the
// live decompile (bsp.py ghidra decompile) and checked against disasm-raw where
// ECX, a stack argument, x87 or a store order mattered.

#include "bsp/frontend_seg15_22_cc13.hpp"
#include "bsp/segs_13_15_22_44_cc13.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string.h>

namespace bsp::frontend_seg15_22_cc13 {

namespace {

constexpr std::size_t kWanted = 0x4;   // front-end screen base: wanted byte
constexpr std::size_t kActive = 0x5;   // front-end screen base: active byte

std::uint8_t* data_of(Raw array) { return static_cast<std::uint8_t*>(array.ptr(0)); }
std::int32_t& size_of(Raw array) { return array.at<std::int32_t>(4); }
std::int32_t& capacity_of(Raw array) { return array.at<std::int32_t>(8); }

// The image's remove-page idiom: 004C12B0 then 00AA31F0(page at off).
void remove_page(Raw s, std::size_t off, Seg15Host& host) {
    host.gui_remove_and_destroy_page_00aa31f0(s.ptr(off));
}

// Visible-state of an image screen: *(screen + 4).
bool screen_visible(Raw screen) { return screen.at<std::uint8_t>(kWanted) != 0; }

}  // namespace

// Bodies read from disasm-raw; see docs/CC13_FE_4.md.
const TrivialSlot kTrivialSlots[] = {
    {0x00527D30, TrivialSlotKind::ReturnInt, 0xE, 0, 0x00000000, "BSP_ControlLayoutScreen_ScreenId"},
    {0x00527D40, TrivialSlotKind::Adjustor, 0x8, 0, 0x00527E40, "BSP_ControlLayoutScreen_Destroy_Adjustor8"},
    {0x00528C60, TrivialSlotKind::Empty, 0x0, 0, 0x00000000, "BSP_HudCountersScreen_BindLayout"},
    {0x0052B060, TrivialSlotKind::ReturnInt, 0x51, 0, 0x00000000, "BSP_HudCountersScreen_GetScreenId"},
    {0x0052B070, TrivialSlotKind::ReturnByte, 0x1, 0, 0x00000000, "BSP_HudCountersScreen_IsSelfManaged"},
    {0x0052FD60, TrivialSlotKind::ReturnInt, 0x8, 0, 0x00000000, "BSP_CreditsScreen_GetScreenId"},
    {0x00530640, TrivialSlotKind::Empty, 0x0, 8, 0x00000000, "BSP_MenuPromptScreen_OnListboxSelectionChangedNoOp"},
    {0x00532310, TrivialSlotKind::ReturnInt, 0x55, 0, 0x00000000, "BSP_MenuPromptScreen_GetScreenId"},
    {0x00532320, TrivialSlotKind::ReturnByte, 0x1, 0, 0x00000000, "BSP_MenuPromptScreen_IsSelfManaged"},
    {0x00532330, TrivialSlotKind::Adjustor, 0x8, 0, 0x00532A00, "CG_scalar_deleting_dtor_00532a00_Adjustor8"},
    {0x00532340, TrivialSlotKind::Adjustor, 0xC, 0, 0x00532A00, "CG_scalar_deleting_dtor_00532a00_AdjustorC"},
    {0x00533B30, TrivialSlotKind::Empty, 0x0, 8, 0x00000000, "BSP_HudFormationScreen_OnListboxActivateNoOp"},
    {0x005361B0, TrivialSlotKind::TailJump, 0x0, 0, 0x00535C30, "TMPL_List_Clear_005361b0"},
    {0x00536250, TrivialSlotKind::TailJump, 0x0, 0, 0x00535DD0, "TMPL_List_Clear_00536250"},
    {0x00538CD0, TrivialSlotKind::ReturnInt, 0x2A, 0, 0x00000000, "BSP_HudFormationScreen_GetScreenId"},
    {0x00538CE0, TrivialSlotKind::ReturnByte, 0x1, 0, 0x00000000, "BSP_HudFormationScreen_OccludesLowerLevels"},
    {0x00538CF0, TrivialSlotKind::Adjustor, 0x78, 0, 0x00538F60, "CG_scalar_deleting_dtor_00538f60_Adjustor78"},
    {0x00538D00, TrivialSlotKind::Adjustor, 0x64, 0, 0x00538F60, "CG_scalar_deleting_dtor_00538f60_Adjustor64"},
    {0x00538D10, TrivialSlotKind::Adjustor, 0x8, 0, 0x00538F60, "CG_scalar_deleting_dtor_00538f60_Adjustor8"},
    {0x0053BA50, TrivialSlotKind::ReturnInt, 0x57, 0, 0x00000000, "BSP_FrameStatsScreen_GetScreenId"},
    {0x0053BA60, TrivialSlotKind::ReturnByte, 0x1, 0, 0x00000000, "BSP_FrameStatsScreen_IsSelfManaged"},
    {0x0053BA90, TrivialSlotKind::Empty, 0x0, 0, 0x00000000, "BSP_FrameStatsScreen_Exit"},
    {0x0053D280, TrivialSlotKind::Empty, 0x0, 0, 0x00000000, "BSP_HudFreeCameraScreen_Exit"},
    {0x0053FF00, TrivialSlotKind::TailJump, 0x0, 0, 0x0053F250, "TMPL_Rec30Vector_Tidy_0053ff00"},
    {0x00540360, TrivialSlotKind::ReturnInt, 0x2B, 0, 0x00000000, "BSP_HudFreeCameraScreen_GetScreenId"},
    {0x00540370, TrivialSlotKind::Adjustor, 0x8, 0, 0x00540C80, "CG_scalar_deleting_dtor_00540c80_Adjustor8"},
    {0x005CE5B0, TrivialSlotKind::ReturnInt, 0x36, 0, 0x00000000, "BSP_HudMovieCameraDebugScreen_GetScreenId"},
    {0x005CE5C0, TrivialSlotKind::Adjustor, 0x8, 0, 0x005CE6D0, "CG_scalar_deleting_dtor_005ce6d0_Adjustor8"},
    {0x005CE740, TrivialSlotKind::Empty, 0x0, 0, 0x00000000, "BSP_MultiChatScreen_EnterNoOp"},
    {0x005CFE40, TrivialSlotKind::ReturnInt, 0x15, 0, 0x00000000, "BSP_MultiChatScreen_GetScreenId"},
    {0x005CFE50, TrivialSlotKind::ReturnByte, 0x1, 0, 0x00000000, "BSP_MultiChatScreen_IsSelfManaged"},
    {0x005D2F40, TrivialSlotKind::ReturnInt, 0x1D, 0, 0x00000000, "BSP_MultiErrorScreen_GetScreenId"},
    {0x005D2F50, TrivialSlotKind::Adjustor, 0x8, 0, 0x005D2F70, "CG_scalar_deleting_dtor_005d2f70_Adjustor8"},
    {0x005D3580, TrivialSlotKind::ReturnInt, 0x19, 0, 0x00000000, "BSP_HudPauseMultiScreen_GetScreenId"},
    {0x005D3590, TrivialSlotKind::Adjustor, 0x8, 0, 0x005D3880, "CG_scalar_deleting_dtor_005d3880_Adjustor8"},
    // @@TRIVIAL@@
};
const std::size_t kTrivialSlotCount = sizeof(kTrivialSlots) / sizeof(kTrivialSlots[0]);

// ---------------------------------------------------------------------------
// Containers
// ---------------------------------------------------------------------------

// 00529700 (and its twin 00529760): __thiscall(array, n), RET 4.
void ptr_array_reserve_00529700(Raw array, std::int32_t n, Seg15Host& host) {
    if (n < 1) n = 1;
    if (capacity_of(array) >= n) return;
    auto* fresh = static_cast<std::uint32_t*>(host.operator_new_00bf55be(static_cast<std::size_t>(n) * 4));
    auto* old = reinterpret_cast<std::uint32_t*>(data_of(array));
    for (std::int32_t i = 0; i < size_of(array); ++i) {
        if (fresh + i) fresh[i] = old[i];
    }
    host.free_00bf6989(old);
    array.ptr(0) = fresh;
    capacity_of(array) = n;
}

// 005297C0 (reserve 00529760) and 005299D0 (reserve 00529700): resize, zeroing
// the new slots; shrinking just drops the size.
void ptr_array_resize_005297c0(Raw array, std::int32_t n, Seg15Host& host) {
    if (capacity_of(array) < n) ptr_array_reserve_00529700(array, n, host);
    auto* data = reinterpret_cast<std::uint32_t*>(data_of(array));
    for (std::int32_t i = size_of(array); i < n; ++i) data[i] = 0;
    size_of(array) = n;
}

// 00529900 (reserve 00529700) and 00529940 (reserve 00529760), RET 4: the
// argument is a pointer to the value.
void ptr_array_push_back_00529900(Raw array, std::uint32_t value, Seg15Host& host) {
    if (size_of(array) == capacity_of(array)) {
        std::int32_t grow = capacity_of(array) * 2;
        if (grow < 2) grow = 1;
        ptr_array_reserve_00529700(array, grow, host);
    }
    reinterpret_cast<std::uint32_t*>(data_of(array))[size_of(array)] = value;
    ++size_of(array);
}

// 005298C0, RET 4: the 16-bit element version (reserve BSP_WordArray_Reserve).
void word_array_push_back_005298c0(Raw array, std::uint16_t value, Seg15Host& host) {
    if (size_of(array) == capacity_of(array)) {
        std::int32_t grow = capacity_of(array) * 2;
        if (grow < 2) grow = 1;
        if (capacity_of(array) < grow) {
            auto* fresh = static_cast<std::uint16_t*>(host.operator_new_00bf55be(static_cast<std::size_t>(grow) * 2));
            auto* old = reinterpret_cast<std::uint16_t*>(data_of(array));
            for (std::int32_t i = 0; i < size_of(array); ++i) fresh[i] = old[i];
            host.free_00bf6989(old);
            array.ptr(0) = fresh;
            capacity_of(array) = grow;
        }
    }
    reinterpret_cast<std::uint16_t*>(data_of(array))[size_of(array)] = value;
    ++size_of(array);
}

// 00529620, RET 4.
void ptr_array_erase_at_00529620(Raw array, std::int32_t index) {
    auto* data = reinterpret_cast<std::uint32_t*>(data_of(array));
    for (; index < size_of(array) - 1; ++index) data[index] = data[index + 1];
    --size_of(array);
}

// 00529650, RET 4: the argument is a pointer to the value.
std::int32_t ptr_array_index_of_00529650(Raw array, std::uint32_t value) {
    auto* data = reinterpret_cast<std::uint32_t*>(data_of(array));
    for (std::int32_t i = 0; i < size_of(array); ++i) {
        if (data[i] == value) return i;
    }
    return -1;
}

// 00529C50 / 0052AE40, __fastcall(array): called from unwind funclets.
void ptr_array_destroy_00529c50(Raw array, Seg15Host& host) {
    ptr_array_resize_005297c0(array, 0, host);
    host.free_00bf6989(array.ptr(0));
}

// 00529C70, RET 4: each record is copy-constructed (00523480) and re-vtabled to
// 00CE75CC, then every old record is destroyed through its slot 0 with flag 0.
void rec18_array_reserve_00529c70(Raw array, std::int32_t n, Seg15Host& host) {
    if (n < 1) n = 1;
    if (capacity_of(array) >= n) return;
    auto* fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::size_t>(n) * 0x18));
    for (std::int32_t i = 0; i < size_of(array); ++i) {
        Raw dst{fresh + i * 0x18};
        host.observer_handle_copy_00523480(dst, Raw{data_of(array) + i * 0x18});
        dst.at<std::uint32_t>(0) = 0x00CE75CC;
    }
    for (std::int32_t i = 0; i < size_of(array); ++i) {
        host.object_virtual_arg(Raw{data_of(array) + i * 0x18}, 0x0, 0);
    }
    host.free_00bf6989(array.ptr(0));
    array.ptr(0) = fresh;
    capacity_of(array) = n;
}

// ---------------------------------------------------------------------------
// Control layout screen (0Eh)
// ---------------------------------------------------------------------------

bool control_layout_take_button_flags_00527cb0(Raw layout, std::uint8_t out[4]) {
    if (layout.at<std::uint8_t>(0x2C) == 0) return false;
    for (int i = 0; i < 4; ++i) out[i] = layout.at<std::uint8_t>(0x28 + i);
    layout.at<std::uint8_t>(0x2C) = 0;
    return true;
}

// 00527D00, __thiscall(screen) -> EAX: base constructor, the two listener bases
// at +8h / +Ch, then this screen's three vtables.
Raw control_layout_construct_00527d00(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0xC) = kWidgetListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = kControlLayoutVtable;
    s.at<std::uint32_t>(0x8) = kControlLayoutListboxListenerVtable;
    s.at<std::uint32_t>(0xC) = kControlLayoutWidgetListenerVtable;
    return s;
}

// 00527DA0, __fastcall(screen): restore the vtables, drop both pages (+10h, +14h).
void control_layout_destruct_00527da0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = kControlLayoutVtable;
    s.at<std::uint32_t>(0x8) = kControlLayoutListboxListenerVtable;
    s.at<std::uint32_t>(0xC) = kControlLayoutWidgetListenerVtable;
    remove_page(s, 0x10, host);
    remove_page(s, 0x14, host);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

// 00527D50, widget-listener slot +0Ch, __thiscall(listener, widget), RET 4: a
// type-3 widget gets alpha (+4Ch) 0.5f (00CE3800) when its byte +D4h is set,
// else 1.0f (00D7A24C).
void control_layout_listener_current0c_00527d50(void* widget, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) return;
    const bool flag = static_cast<std::uint8_t*>(widget)[0xD4] != 0;
    host.widget_set_float_4c(widget, flag ? 0.5f : 1.0f);
}

// 00527E30, listbox-listener slot +4h, RET 8: raise the accept cue (+1Dh).
void control_layout_listbox_activate_00527e30(Seg15Host& host) {
    host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1D) = 1;
}

// 00527E60, slot +10h: base register, both pages, +2Ch cleared, then the screen's
// own slot +14h (00527F90).
void control_layout_register_00527e60(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x10) = host.gui_load_page_00aa5840("FE_controls_X360_listbox");
    s.ptr(0x14) = host.gui_load_page_00aa5840("FE_controls_X360");
    s.at<std::uint8_t>(0x2C) = 0;
    host.object_virtual(s, 0x14);
}

// 00527F90, slot +14h: the three group widgets of page +14h go to +1Ch/+20h/+24h;
// then for each of the two name triples (00E08674) and each group, the three
// children's texts (+F4h) are copied into the next native strings of 00E18DC0
// (outer triple, then group, then name).
void control_layout_bind_widgets_00527f90(Raw s, Seg15Host& host) {
    for (std::size_t g = 0; g < kControlLayoutGroups.size(); ++g) {
        s.ptr(0x1C + g * 4) = host.gui_find_child_00aa7e00(s.ptr(0x14), kControlLayoutGroups[g]);
    }
    Raw texts = host.global_block(kStickLabelTexts);
    std::size_t slot = 0;
    for (std::size_t triple = 0; triple < 2; ++triple) {
        for (std::size_t g = 0; g < 3; ++g) {
            for (std::size_t n = 0; n < 3; ++n, ++slot) {
                auto* child = static_cast<std::uint8_t*>(
                    host.gui_find_child_00aa7e00(s.ptr(0x1C + g * 4), kStickLabelNameValues[triple * 3 + n]));
                Raw dst{texts.p + slot * 8};
                if (dst.p != child + 0xF4) host.native_string_copy_0041dd40(dst, Raw{child + 0xF4});
            }
        }
    }
}

// 00528920, slot +1Ch: in the in-game options (*(game + 61Fh)) re-select frame 0
// with the control title.
void control_layout_exit_00528920(Seg15Host& host) {
    if (host.global_object(kGameGlobal).at<std::uint8_t>(0x61F) != 0) {
        host.front_end_frame_select_00518d60(0, "FE.opt_title_control");
    }
}

// 005289B0, slot +20h, __thiscall(screen, float), RET 4: back (action 4Bh)
// returns to the options screen (interface 0Ch).
void control_layout_update_005289b0(Seg15Host& host) {
    if (host.action_edge_or_repeat_004d92b0(0x4B)) host.front_end_push_interface_004cc460(host.global_object(kOptionsManagerGlobal), 0xC, 0);
}

// 005289E0, widget-listener slot +4h, RET 4: a type-3 widget gets alpha 1.0;
// a name (+F0h, "" when null) starting with A3h returns to the options screen.
// The A7h and A2h tests in the listing branch to the same exit.
void control_layout_listener_current04_005289e0(void* widget, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) return;
    host.widget_set_float_4c(widget, 1.0f);
    auto* w = static_cast<std::uint8_t*>(widget);
    const auto* name = *reinterpret_cast<const std::uint8_t* const*>(w + 0xF0);
    const std::uint8_t first = name ? name[0] : 0;  // 00E18DB8 is ""
    if (first == 0xA7 || first == 0xA2) return;
    if (first == 0xA3) host.front_end_push_interface_004cc460(host.global_object(kOptionsManagerGlobal), 0xC, 0);
}

// 00528A60, slot +24h, __thiscall(screen, out), RET 4.
void control_layout_collect_children_00528a60(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x10));
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x14));
}

// ---------------------------------------------------------------------------
// HUD counters (51h)
// ---------------------------------------------------------------------------

// 00528B00: count the remaining time (+98h) down by the frame; at or below zero
// hide the entry widget (+0h, +34h(0)); otherwise format by +A0h (0 h:m:s,
// 1 m:s, 2 s) into the text at +28h. Any other mode leaves the 128-byte buffer
// uninitialised in the image; this model passes "".
void hud_counter_format_time_00528b00(Raw entry, float seconds, Seg15Host& host) {
    const float left = entry.at<float>(0x98) - seconds;
    entry.at<float>(0x98) = left;
    if (!(0.0f < left)) {
        host.widget_set_visible_34(entry.ptr(0x0), false);
        return;
    }
    char text[128] = {};
    const int whole = static_cast<int>(left);  // CVTTSS2SI
    switch (entry.at<std::int32_t>(0xA0)) {
        case 0: std::snprintf(text, sizeof text, "%02d:%02d:%02d", whole / 3600, (whole % 3600) / 60, (whole % 3600) % 60); break;
        case 1: std::snprintf(text, sizeof text, "%02d:%02d", whole / 60, whole % 60); break;
        case 2: std::snprintf(text, sizeof text, "%02d", whole); break;
        default: break;
    }
    host.text_set_localised_00abbe50(entry.ptr(0x28), text);
}

// 00528BE0: store the value on the timer entry at +10h and choose its format.
void hud_counter_set_value_00528be0(Raw counter, float value) {
    Raw entry{static_cast<std::uint8_t*>(counter.ptr(0x10))};
    entry.at<float>(0x98) = value;
    if (static_cast<double>(value) >= kCounterHoursThreshold) entry.at<std::int32_t>(0xA0) = 0;
    else if (static_cast<double>(value) >= kCounterMinutesThreshold) entry.at<std::int32_t>(0xA0) = 1;
    else entry.at<std::int32_t>(0xA0) = 2;
}

// 00528E90: frac = (value +30h above 0.1 ? value : 0.1f) / maximum +68h. The bar
// +18h gets scale {frac, 1}, +20h {frac * a, 1}, +1Ch {clamp(a - b, 0, 1) * frac, 1}.
void hud_counter_update_bar_00528e90(Raw counter, float a, float b, Seg15Host& host) {
    const float value = counter.at<float>(0x30);
    float frac = value > kCounterBarFloorCompare ? value : kCounterBarFloor;
    frac = frac / counter.at<float>(0x68);
    float xy[2] = {frac, 1.0f};
    host.widget_set_scale_48(counter.ptr(0x18), xy);
    float xy2[2] = {frac * a, 1.0f};
    host.widget_set_scale_48(counter.ptr(0x20), xy2);
    float t = a - b;
    if (!(0.0f <= t)) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    float xy3[2] = {t * frac, 1.0f};
    host.widget_set_scale_48(counter.ptr(0x1C), xy3);
}

// 00528F70: a visible entry whose widget +4h lies left of the panel edge (and
// right of -900) is hidden and flagged +A5h while the manager panel
// (*(interface + C0h) + 4) is up or the support manager is in its mode.
void hud_counters_hide_under_manager_panel_00528f70(Raw s, Seg15Host& host) {
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x24); ++i) {
        Raw entry{static_cast<std::uint8_t**>(s.ptr(0x20))[i]};
        if (!host.widget_is_visible_38(entry.ptr(0x0))) continue;
        const bool wide = host.global_object(kDisplayModeGlobal).at<std::uint8_t>(0xD) != 0;
        const float edge = wide ? kCounterManagerPanelEdgeWide : kCounterManagerPanelEdge;
        float pos[3];
        host.widget_resolved_position_00aa6750(entry.ptr(0x4), pos);
        if (pos[0] > edge) continue;
        host.widget_resolved_position_00aa6750(entry.ptr(0x4), pos);
        if (!(pos[0] > kCounterWrapBelow)) continue;
        Raw iface = host.global_object(kInterfaceGlobal);
        if (Raw{static_cast<std::uint8_t*>(iface.ptr(0xC0))}.at<std::uint8_t>(4) == 0
            && !host.support_manager_mode_006529e0(Raw{static_cast<std::uint8_t*>(iface.ptr(0xBC))})) {
            continue;
        }
        host.widget_set_visible_34(entry.ptr(0x0), false);
        entry.at<std::uint8_t>(0xA5) = 1;
    }
}

namespace {
// Shape of 00529200 and 00529120: x of widget +8h (below -900 wrapped by +1000,
// through float stores) left of the edge re-shows a flagged entry; otherwise
// the entry is hidden and flagged.
void hide_right_of(Raw s, Seg15Host& host, double narrow, double wide_edge, std::size_t flag) {
    const bool wide = host.global_object(kDisplayModeGlobal).at<std::uint8_t>(0xD) != 0;
    const float edge = static_cast<float>(wide ? wide_edge : narrow);
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x24); ++i) {
        Raw entry{static_cast<std::uint8_t**>(s.ptr(0x20))[i]};
        float pos[3];
        host.widget_resolved_position_00aa6750(entry.ptr(0x8), pos);
        float x = pos[0];
        if (x < kCounterWrapBelow) x = static_cast<float>(x + kCounterWrapAdd);
        if (x < edge) {
            if (entry.at<std::uint8_t>(flag) != 0) {
                host.widget_set_visible_34(entry.ptr(0x0), true);
                entry.at<std::uint8_t>(flag) = 0;
            }
        } else {
            host.widget_set_visible_34(entry.ptr(0x0), false);
            entry.at<std::uint8_t>(flag) = 1;
        }
    }
}
}  // namespace

// 00529200: edge 0.76 (0.877 wide), flag +A7h.
void hud_counters_hide_under_screen58_00529200(Raw s, Seg15Host& host) {
    hide_right_of(s, host, kCounterScreen58Edge, kCounterScreen58EdgeWide, 0xA7);
}

// 00529120: edge 0.936 (1.052 wide), flag +A9h.
void hud_counters_hide_right_of_edge_00529120(Raw s, Seg15Host& host) {
    hide_right_of(s, host, kCounterScreenC0Edge, kCounterScreenC0EdgeWide, 0xA9);
}

// 005293B0: x of widget +8h left of 0.71 (0.89 wide), or the screen at
// interface +54h down, re-shows an entry flagged +A8h; otherwise hide and flag.
void hud_counters_hide_under_screen54_005293b0(Raw s, Seg15Host& host) {
    const bool wide = host.global_object(kDisplayModeGlobal).at<std::uint8_t>(0xD) != 0;
    const float edge = wide ? kCounterScreen54EdgeWide : kCounterScreen54Edge;
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x24); ++i) {
        Raw entry{static_cast<std::uint8_t**>(s.ptr(0x20))[i]};
        float pos[3];
        host.widget_resolved_position_00aa6750(entry.ptr(0x8), pos);
        Raw screen54{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0x54))};
        if (pos[0] < edge || !screen_visible(screen54)) {
            if (entry.at<std::uint8_t>(0xA8) != 0) {
                host.widget_set_visible_34(entry.ptr(0x0), true);
                entry.at<std::uint8_t>(0xA8) = 0;
            }
        } else {
            host.widget_set_visible_34(entry.ptr(0x0), false);
            entry.at<std::uint8_t>(0xA8) = 1;
        }
    }
}

// 00529810, __fastcall(screen): per-frame occlusion against the interface's
// screens +58h, +C0h (or the support manager mode), +60h (+5h and +70h) and +54h.
// The last step tail-jumps to 00529200 or, after 005292E0, to 00529120.
void hud_counters_update_occlusion_00529810(Raw s, Seg15Host& host) {
    auto iface_screen = [&](std::size_t off) {
        return Raw{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(off))};
    };
    if (screen_visible(iface_screen(0x58))) hud_counters_hide_under_screen58_00529200(s, host);
    else host.counters_restore_hidden_a7_005292e0(s);
    if (screen_visible(iface_screen(0xC0)) || host.support_manager_mode_006529e0(iface_screen(0xBC))) {
        hud_counters_hide_under_manager_panel_00528f70(s, host);
    }
    Raw s60 = iface_screen(0x60);
    if (s60.at<std::uint8_t>(kActive) != 0 && s60.at<std::uint8_t>(0x70) != 0) host.counters_hide_all_a6_005290d0(s);
    else host.counters_restore_hidden_a6_00529350(s);
    if (screen_visible(iface_screen(0x54))) hud_counters_hide_under_screen54_005293b0(s, host);
    if (screen_visible(iface_screen(0x58))) {
        hud_counters_hide_under_screen58_00529200(s, host);
        return;
    }
    host.counters_restore_hidden_a7_005292e0(s);
    hud_counters_hide_right_of_edge_00529120(s, host);
}

namespace {
// 0052AB40 / 0052AB90 shape: when the counter entry in the slot is visible,
// hide it, erase it from the visible list (+20h) and re-lay out (tail jump to
// 00529A30). The entry is the slot's pointer; the list search compares it.
void remove_visible(Raw s, std::size_t slot, Seg15Host& host) {
    Raw entry{static_cast<std::uint8_t*>(s.ptr(slot))};
    if (!host.widget_is_visible_38(entry.ptr(0x0))) return;
    host.widget_set_visible_34(entry.ptr(0x0), false);
    Raw list{s.p + 0x20};
    ptr_array_erase_at_00529620(list, ptr_array_index_of_00529650(list, s.at<std::uint32_t>(slot)));
    host.counters_relayout_00529a30(s);
}
}  // namespace

// 0052AB40, __fastcall(screen): entry slot +0Ch.
void hud_counters_remove_counter_a_0052ab40(Raw s, Seg15Host& host) { remove_visible(s, 0xC, host); }

// 0052AB90, __fastcall(screen): the countdown entry slot +10h.
void hud_counters_cancel_countdown_0052ab90(Raw s, Seg15Host& host) { remove_visible(s, 0x10, host); }

// 0052B030: base constructor, vtable 00CED0AC, +0Ch..+28h zeroed.
Raw hud_counters_construct_0052b030(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x0) = kHudCountersVtable;
    for (std::size_t off = 0x14; off <= 0x28; off += 4) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    return s;
}

// 0052AD50, RET 4: append a copy of *source (00523480, re-vtabled 00CE75CC).
void rec18_array_push_back_0052ad50(Raw array, Raw source, Seg15Host& host) {
    if (size_of(array) == capacity_of(array)) {
        std::int32_t grow = capacity_of(array) * 2;
        if (grow < 2) grow = 1;
        rec18_array_reserve_00529c70(array, grow, host);
    }
    Raw dst{data_of(array) + size_of(array) * 0x18};
    host.observer_handle_copy_00523480(dst, source);
    dst.at<std::uint32_t>(0) = 0x00CE75CC;
    ++size_of(array);
}

// 0052ADD0, RET 4: index -1 is ignored. Otherwise the last record's observed
// pointer (+14h) moves into the slot (unregister the old pair, register the
// new one), and the last record is destroyed (slot 0, flag 0) and popped.
void rec18_array_remove_at_swap_last_0052add0(Raw array, std::int32_t index, Seg15Host& host) {
    if (index == -1) return;
    const std::int32_t last = size_of(array) - 1;
    if (index != last) {
        Raw slot{data_of(array) + index * 0x18};
        void* moved = Raw{data_of(array) + last * 0x18}.ptr(0x14);
        void* current = slot.ptr(0x14);
        if (current != moved) {
            if (current) host.observer_pair_unregister_006952a0(slot, current);
            slot.ptr(0x14) = moved;
            if (moved) host.observer_pair_register_00694a60(slot, moved);
        }
    }
    host.object_virtual_arg(Raw{data_of(array) + last * 0x18}, 0x0, 0);
    --size_of(array);
}

// 0052AE60, RET 4: new records are {vtable 00CE75CC, 0, 0, 0, set byte 1, 0};
// shrinking destroys from the end (slot 0, flag 0).
void rec18_array_resize_0052ae60(Raw array, std::int32_t n, Seg15Host& host) {
    if (capacity_of(array) < n) rec18_array_reserve_00529c70(array, n, host);
    for (std::int32_t i = size_of(array); i < n; ++i) {
        Raw r{data_of(array) + i * 0x18};
        r.at<std::uint32_t>(0x4) = 0;
        r.at<std::uint32_t>(0x8) = 0;
        r.at<std::uint32_t>(0xC) = 0;
        r.at<std::uint32_t>(0x14) = 0;
        r.at<std::uint8_t>(0x10) = 1;
        r.at<std::uint32_t>(0x0) = 0x00CE75CC;
    }
    while (n < size_of(array)) {
        --size_of(array);
        host.object_virtual_arg(Raw{data_of(array) + size_of(array) * 0x18}, 0x0, 0);
    }
    size_of(array) = n;
}

// 0052C0A0, __fastcall(array): unwind-funclet destroy.
void rec18_array_destroy_0052c0a0(Raw array, Seg15Host& host) {
    rec18_array_resize_0052ae60(array, 0, host);
    host.free_00bf6989(array.ptr(0));
}

// 0052B400: unit HP display, subtype 0: owner in +1Ch, the text in +4Ch and the
// unit ids (each unit's word +174h) in the word array +40h.
void hud_send_display_unit_hp_0052b400(std::uint32_t owner, Raw units, std::string_view text, Seg15Host& host) {
    HudMessage35 m;
    m.value_1c = owner;
    m.text_4c = text;
    const std::int32_t count = units.at<std::int32_t>(4);
    std::uint16_t ids_local[64];
    std::uint16_t* ids = count <= 64 ? ids_local
                                     : static_cast<std::uint16_t*>(host.operator_new_00bf55be(count * 2u));
    for (std::int32_t i = 0; i < count; ++i) {
        ids[i] = Raw{static_cast<std::uint8_t**>(units.ptr(0))[i]}.at<std::uint16_t>(0x174);
    }
    m.unit_ids_40 = ids;
    m.unit_count_40 = count;
    host.session_send_message35_00772330(m);
    if (ids != ids_local) host.free_00bf6989(ids);
}

// 0052B570: hide form of the unit HP display (+24h = 1).
void hud_send_hide_unit_hp_0052b570(std::uint32_t owner, Seg15Host& host) {
    HudMessage35 m;
    m.value_1c = owner;
    m.hide_24 = 1;
    host.session_send_message35_00772330(m);
}

// 0052B620: score display, subtype 1, texts in +28h and +30h.
void hud_send_display_score_0052b620(std::uint32_t id, std::uint32_t owner, std::string_view a,
                                     std::string_view b, Seg15Host& host) {
    HudMessage35 m;
    m.value_18 = id;
    m.value_1c = owner;
    m.subtype_20 = 1;
    m.text_28 = a;
    m.text_30 = b;
    host.session_send_message35_00772330(m);
}

// 0052B780: the score display with two values (+38h, +3Ch) and +25h = 1.
void hud_send_display_score_pair_0052b780(std::uint32_t id, std::uint32_t owner, std::string_view a,
                                          std::string_view b, std::uint32_t v0, std::uint32_t v1, Seg15Host& host) {
    HudMessage35 m;
    m.value_18 = id;
    m.value_1c = owner;
    m.subtype_20 = 1;
    m.text_28 = a;
    m.text_30 = b;
    m.values_38[0] = v0;
    m.values_38[1] = v1;
    m.pair_25 = 1;
    host.session_send_message35_00772330(m);
}

// 0052B900: hide form of the score display.
void hud_send_hide_score_0052b900(std::uint32_t id, std::uint32_t owner, Seg15Host& host) {
    HudMessage35 m;
    m.value_18 = id;
    m.value_1c = owner;
    m.subtype_20 = 1;
    m.hide_24 = 1;
    host.session_send_message35_00772330(m);
}

namespace {
void push_visible(Raw s, std::uint32_t entry, Seg15Host& host) {
    ptr_array_push_back_00529900(Raw{s.p + 0x20}, entry, host);
}
}  // namespace

// 0052B9B0: the screen becomes wanted; a hidden countdown entry (+10h) joins
// the visible list; lay it out (0052A860) and re-lay out the list.
void hud_counters_show_countdown_0052b9b0(Raw s, std::string_view text, float seconds, Seg15Host& host) {
    s.at<std::uint8_t>(kWanted) = 1;
    Raw entry{static_cast<std::uint8_t*>(s.ptr(0x10))};
    if (!host.widget_is_visible_38(entry.ptr(0x0))) push_visible(s, s.at<std::uint32_t>(0x10), host);
    host.counter_layout_countdown_0052a860(entry, text, seconds);
    host.counters_relayout_00529a30(s);
}

// 0052BFA0: the HP entry (+0Ch) joins the visible list only when hidden and
// none of its occlusion flags (+A5h..+A9h) is set; then 0052BB10 lays it out.
void hud_counters_show_unit_hp_0052bfa0(Raw s, std::uint32_t value, std::string_view text, Seg15Host& host) {
    s.at<std::uint8_t>(kWanted) = 1;
    Raw entry{static_cast<std::uint8_t*>(s.ptr(0xC))};
    if (!host.widget_is_visible_38(entry.ptr(0x0)) && entry.at<std::uint8_t>(0xA6) == 0
        && entry.at<std::uint8_t>(0xA5) == 0 && entry.at<std::uint8_t>(0xA8) == 0
        && entry.at<std::uint8_t>(0xA7) == 0 && entry.at<std::uint8_t>(0xA9) == 0) {
        push_visible(s, s.at<std::uint32_t>(0xC), host);
    }
    host.counter_layout_score_0052bb10(entry, value, text);
    host.counters_relayout_00529a30(s);
}

// 0052C850: grow the score-entry array (+14h) with null slots up to index;
// an empty slot gets a clone of the HP entry (+0Ch) in a zeroed 0ACh block,
// which is also appended to the visible list.
Raw hud_counters_ensure_entry_0052c850(Raw s, std::int32_t index, Seg15Host& host) {
    Raw entries{s.p + 0x14};
    while (size_of(entries) < index + 1) ptr_array_push_back_00529900(entries, 0, host);
    auto* slots = reinterpret_cast<std::uint8_t**>(data_of(entries));
    if (slots[index] == nullptr) {
        slots[index] = host.counter_entry_clone_0052c640(Raw{static_cast<std::uint8_t*>(s.ptr(0xC))}).p;
        slots = reinterpret_cast<std::uint8_t**>(data_of(entries));
        push_visible(s, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slots[index])), host);
    }
    return Raw{reinterpret_cast<std::uint8_t**>(data_of(entries))[index]};
}

// 0052C990: screen wanted, entry by index, 00529D70 layout, re-lay out.
void hud_counters_show_score_0052c990(Raw s, std::int32_t index, std::string_view a, std::string_view b,
                                      Seg15Host& host) {
    s.at<std::uint8_t>(kWanted) = 1;
    host.counter_layout_timer_00529d70(hud_counters_ensure_entry_0052c850(s, index, host), a, b);
    host.counters_relayout_00529a30(s);
}

// 0052CAC0: as 0052C990 with the two values and the 0052A100 layout.
void hud_counters_show_score_pair_0052cac0(Raw s, std::int32_t index, std::string_view a, std::string_view b,
                                           std::uint32_t v0, std::uint32_t v1, Seg15Host& host) {
    s.at<std::uint8_t>(kWanted) = 1;
    host.counter_layout_value_0052a100(hud_counters_ensure_entry_0052c850(s, index, host), a, b, v0, v1);
    host.counters_relayout_00529a30(s);
}

// 0052C0C0, __fastcall(entry): delete the root widget (slot 1, flag 1) when the
// entry owns it (+A4h), release the strings +90h, +88h, +7Ch, then the record
// array +70h.
void hud_counter_entry_destruct_0052c0c0(Raw entry, Seg15Host& host) {
    if (entry.at<std::uint8_t>(0xA4) != 0 && entry.ptr(0x0) != nullptr) host.object_delete_04(entry.ptr(0x0));
    host.native_string_release(Raw{entry.p + 0x90});
    host.native_string_release(Raw{entry.p + 0x88});
    host.native_string_release(Raw{entry.p + 0x7C});
    rec18_array_destroy_0052c0a0(Raw{entry.p + 0x70}, host);
}

// 0052CDC0, __fastcall(slot): destroy and free *slot, then clear it.
void hud_counter_entry_destroy_and_free_0052cdc0(Raw slot, Seg15Host& host) {
    Raw entry{static_cast<std::uint8_t*>(slot.ptr(0))};
    if (entry.p == nullptr) return;
    hud_counter_entry_destruct_0052c0c0(entry, host);
    host.free_00bf6989(entry.p);
    slot.ptr(0) = nullptr;
}

// 0052CBF0, slot +10h: page GUI_counters into +8h; the HP entry (+0Ch) is built
// from the page and the twelve names; the countdown entry (+10h) clones it.
void hud_counters_register_0052cbf0(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x8) = host.gui_load_page_00aa5840("GUI_counters");
    static constexpr std::string_view kNames[12] = {
        "New_Group", "Keret_L_Icon", "Keret_M_Icon", "Keret_R_Icon", "flag_A_Icon", "flag_B_Icon",
        "Text_A_Text", "Text_B_Text", "hp_alap_Icon", "Hp_light_Icon", "Hp_dark_Icon", "BG_FrameBox"};
    s.ptr(0xC) = host.counter_entry_construct_0052c1b0(s.ptr(0x8), kNames).p;
    s.ptr(0x10) = host.counter_entry_clone_0052c640(Raw{static_cast<std::uint8_t*>(s.ptr(0xC))}).p;
}

// 0052CDF0: vtable 00CED0AC, empty the visible list, free the HP entry, every
// score entry and the countdown entry, drop the page, then free both arrays.
void hud_counters_destruct_0052cdf0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = kHudCountersVtable;
    ptr_array_resize_005297c0(Raw{s.p + 0x20}, 0, host);
    hud_counter_entry_destroy_and_free_0052cdc0(Raw{s.p + 0xC}, host);
    Raw entries{s.p + 0x14};
    for (std::int32_t i = 0; i < size_of(entries); ++i) {
        hud_counter_entry_destroy_and_free_0052cdc0(Raw{data_of(entries) + i * 4}, host);
    }
    hud_counter_entry_destroy_and_free_0052cdc0(Raw{s.p + 0x10}, host);
    remove_page(s, 0x8, host);
    ptr_array_destroy_00529c50(Raw{s.p + 0x20}, host);
    ptr_array_destroy_00529c50(entries, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// 0052CF00: an existing score entry leaves the visible list and is freed; the
// list is re-laid out in every case.
void hud_counters_remove_entry_widget_0052cf00(Raw s, std::int32_t index, Seg15Host& host) {
    Raw entries{s.p + 0x14};
    if (index < size_of(entries)) {
        Raw slot{data_of(entries) + index * 4};
        if (slot.at<std::uint32_t>(0) != 0) {
            Raw list{s.p + 0x20};
            ptr_array_erase_at_00529620(list, ptr_array_index_of_00529650(list, slot.at<std::uint32_t>(0)));
            hud_counter_entry_destroy_and_free_0052cdc0(Raw{data_of(entries) + index * 4}, host);
        }
    }
    host.counters_relayout_00529a30(s);
}

// 0052D1D0, slot +24h, RET 4: the page slot +8h.
void hud_counters_collect_children_0052d1d0(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x8));
}

// ---------------------------------------------------------------------------
// MSVC 8 vector insert (shared by the vector instances of these screens)
// ---------------------------------------------------------------------------

void vector_insert_n(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                     const VectorElementOps& ops, Seg15Host& host) {
    const std::size_t sz = ops.size;
    auto first = [&] { return static_cast<std::uint8_t*>(vec.ptr(4)); };
    auto last = [&] { return static_cast<std::uint8_t*>(vec.ptr(8)); };
    auto end = [&] { return static_cast<std::uint8_t*>(vec.ptr(0xC)); };
    auto elems = [&](std::uint8_t* a, std::uint8_t* b) { return static_cast<std::uint32_t>((b - a) / sz); };
    // The value is copied first (it may live inside the vector).
    std::uint8_t tmp_storage[0x40];
    Raw tmp{tmp_storage};
    ops.copy_construct(tmp, value, host);
    std::uint32_t capacity = first() ? elems(first(), end()) : 0;
    if (count != 0) {
        const std::uint32_t size = first() ? elems(first(), last()) : 0;
        // max - size < count throws through the xlen helper (not modelled further).
        if (capacity < size + count) {
            capacity = (ops.max_count - capacity / 2 < capacity) ? 0 : capacity + capacity / 2;
            if (capacity < size + count) capacity = size + count;
            auto* fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * sz));
            std::uint8_t* out = fresh;
            for (std::uint8_t* p = first(); p != where; p += sz, out += sz) ops.copy_construct(Raw{out}, Raw{p}, host);
            for (std::uint32_t i = 0; i < count; ++i, out += sz) ops.copy_construct(Raw{out}, tmp, host);
            for (std::uint8_t* p = where; p != last(); p += sz, out += sz) ops.copy_construct(Raw{out}, Raw{p}, host);
            if (first()) {
                for (std::uint8_t* p = first(); p != last(); p += sz) ops.destroy(Raw{p}, host);
                host.free_00bf65ac(first());
            }
            vec.ptr(0xC) = fresh + capacity * sz;
            vec.ptr(8) = fresh + (size + count) * sz;
            vec.ptr(4) = fresh;
        } else if (elems(where, last()) < count) {
            std::uint8_t* old_last = last();
            std::uint8_t* out = where + count * sz;
            for (std::uint8_t* p = where; p != old_last; p += sz, out += sz) ops.copy_construct(Raw{out}, Raw{p}, host);
            out = old_last;
            for (std::uint32_t i = count - elems(where, old_last); i != 0; --i, out += sz) {
                ops.copy_construct(Raw{out}, tmp, host);
            }
            vec.ptr(8) = old_last + count * sz;
            for (std::uint8_t* p = where; p != old_last; p += sz) ops.assign(Raw{p}, tmp, host);
        } else {
            std::uint8_t* old_last = last();
            std::uint8_t* out = old_last;
            for (std::uint8_t* p = old_last - count * sz; p != old_last; p += sz, out += sz) {
                ops.copy_construct(Raw{out}, Raw{p}, host);
            }
            vec.ptr(8) = out;
            std::uint8_t* dst = old_last;
            for (std::uint8_t* p = old_last - count * sz; p != where;) {
                p -= sz;
                dst -= sz;
                ops.assign(Raw{dst}, Raw{p}, host);
            }
            for (std::uint8_t* p = where; p != where + count * sz; p += sz) ops.assign(Raw{p}, tmp, host);
        }
    }
    ops.destroy(tmp, host);
}

// ---------------------------------------------------------------------------
// Shared widget-listener slots
// ---------------------------------------------------------------------------

void listener_alpha_from_d4(void* widget, bool requery_other, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) {
        if (requery_other) host.widget_type_5c(widget);
        return;
    }
    host.widget_set_float_4c(widget, static_cast<std::uint8_t*>(widget)[0xD4] != 0 ? 0.5f : 1.0f);
}

void credits_listener_current18_0052d210(void* widget, bool flag, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) {
        host.widget_type_5c(widget);
        return;
    }
    host.widget_set_float_4c(widget, flag ? 0.5f : 1.0f);
}

void credits_listener_current14_0052d200(void* widget, Seg15Host& host) { host.widget_type_5c(widget); }

void credits_listener_current04_0052e5f0(void* widget, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) {
        host.widget_type_5c(widget);
        return;
    }
    host.widget_set_float_4c(widget, 1.0f);
    const auto* name = *reinterpret_cast<const std::uint8_t* const*>(static_cast<std::uint8_t*>(widget) + 0xF0);
    if (name && name[0] == 0xA3) host.front_end_push_interface_004cc460(host.global_object(kMainFrontEndGlobal), 1, 0);
}

// ---------------------------------------------------------------------------
// Credits
// ---------------------------------------------------------------------------

int credits_format_wide_0052d1e0(wchar_t* buffer, std::size_t count, const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    const int n = std::vswprintf(buffer, count, format, args);
    va_end(args);
    return n;
}

// 0052DFB0 / 0052E6D0: return the native string block at +4/+8 to the pool.
void native_string_release_at4_0052dfb0(Raw record, Seg15Host& host) { host.native_string_release(Raw{record.p + 4}); }

// 0052E0F0: zero the wide string, size it to the source length, copy through
// the terminator.
Raw wide_string_construct_0052e0f0(Raw dst, const std::uint16_t* source, Seg15Host& host) {
    dst.at<std::uint32_t>(0) = 0;
    dst.at<std::uint32_t>(4) = 0;
    std::uint32_t n = 0;
    while (source[n] != 0) ++n;
    host.wide_string_resize_004c53e0(dst, n);
    auto* out = static_cast<std::uint16_t*>(dst.ptr(4));
    if (out) {
        std::uint32_t i = 0;
        do { out[i] = source[i]; } while (source[i++] != 0);
    }
    return dst;
}

// 0052E2C0: type, the string (resize + memcpy unless it is the same object),
// then the six dwords +0Ch..+20h.
void credits_item_copy_construct_0052e2c0(Raw dst, Raw src, Seg15Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    dst.at<std::uint32_t>(4) = 0;
    dst.at<std::uint32_t>(8) = 0;
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 4}, Raw{src.p + 4});
    for (std::size_t off = 0xC; off <= 0x20; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
}

// 0052E340: as the copy without zeroing the string first.
void credits_item_assign_0052e340(Raw dst, Raw src, Seg15Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 4}, Raw{src.p + 4});
    for (std::size_t off = 0xC; off <= 0x20; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
}

void credits_item_placement_copy_0052e3b0(Raw slot, Raw src, Seg15Host& host) {
    if (slot.p) credits_item_copy_construct_0052e2c0(slot, src, host);
}

// 0052E410: assigns backwards (type, string, six dwords) and returns the new
// destination start.
std::uint8_t* credits_item_copy_backward_0052e410(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
                                                  Seg15Host& host) {
    while (last != first) {
        last -= 0x24;
        dest_end -= 0x24;
        credits_item_assign_0052e340(Raw{dest_end}, Raw{last}, host);
    }
    return dest_end;
}

namespace {
void credits_item_destroy(Raw item, Seg15Host& host) { native_string_release_at4_0052dfb0(item, host); }
constexpr VectorElementOps kCreditsItemOps{0x24, credits_item_copy_construct_0052e2c0, credits_item_assign_0052e340,
                                           credits_item_destroy, 0x071C71C7};
}  // namespace

void credits_item_vector_insert_n_0052f9e0(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                                           Seg15Host& host) {
    vector_insert_n(vec, where, count, value, kCreditsItemOps, host);
}

// 0052FCE0: listener vtable 00CED1E0 at +8h; vectors +28h and +50h emptied;
// scroll speed +3Ch = 0.0444f (00CED1DC), +40h = 0.5f, +44h = 0.01389f (00CED1D8).
Raw credits_construct_0052fce0(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kWidgetListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = 0x00CED200;
    s.at<std::uint32_t>(0x8) = 0x00CED1E0;
    for (std::size_t off : {0x28u, 0x2Cu, 0x30u, 0x50u, 0x54u, 0x58u}) s.at<std::uint32_t>(off) = 0;
    s.at<float>(0x3C) = 0.044444446f;
    s.at<float>(0x40) = 0.5f;
    s.at<float>(0x44) = 0.013888889f;
    return s;
}

// 0052FD70, slot +24h, RET 4: the page slot +0Ch.
void credits_collect_children_0052fd70(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0xC));
}

// 0052FD80: vtables, page +0Ch, the item vector (+50h..+58h: destroy the range,
// free), the vector at +28h (freed), then the base.
void credits_destruct_0052fd80(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CED200;
    s.at<std::uint32_t>(0x8) = 0x00CED1E0;
    remove_page(s, 0xC, host);
    if (s.ptr(0x50) != nullptr) {
        for (auto* p = static_cast<std::uint8_t*>(s.ptr(0x50)); p != s.ptr(0x54); p += 0x24) {
            credits_item_destroy(Raw{p}, host);
        }
        host.free_00bf65ac(s.ptr(0x50));
    }
    s.at<std::uint32_t>(0x50) = 0;
    s.at<std::uint32_t>(0x54) = 0;
    s.at<std::uint32_t>(0x58) = 0;
    if (s.ptr(0x28) != nullptr) host.free_00bf65ac(s.ptr(0x28));
    s.at<std::uint32_t>(0x28) = 0;
    s.at<std::uint32_t>(0x2C) = 0;
    s.at<std::uint32_t>(0x30) = 0;
    host.front_end_screen_destruct_004f71a0(s);
}

// ---------------------------------------------------------------------------
// Menu prompt (55h)
// ---------------------------------------------------------------------------

void menu_prompt_listener_set_toggle_005309f0(void* widget, bool flag, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) return;
    Raw w{static_cast<std::uint8_t*>(widget)};
    if (host.global_object(kGameGlobal).at<std::int32_t>(0x5D4) == 2 && w.ptr(0x70) != nullptr) {
        Raw{static_cast<std::uint8_t*>(w.ptr(0x70))}.at<std::uint32_t>(0xD8) = flag ? 1u : 0u;
    }
    host.widget_set_float_4c(widget, flag ? 0.5f : 1.0f);
}

void singleton_00e18e6c_clear_base_00530cc0(Raw object, Seg15Host& host) {
    host.global_block(0x00E18E6C).at<std::uint32_t>(0) = 0;
    object.at<std::uint32_t>(0) = 0x00CE3818;
}

// 00530E00, __thiscall(object, flags), RET 4 -> EAX object.
Raw singleton_00e18e6c_deleting_dtor_00530e00(Raw object, std::uint8_t flags, Seg15Host& host) {
    singleton_00e18e6c_clear_base_00530cc0(object, host);
    if (flags & 1) host.free_00bf6989(object.p);
    return object;
}

// 005311C0, slot +10h: page _DialogueBox (+14h, hidden) with its
// dialog_BG_FrameBox (+38h), page _DialogueFarBlackPlane (+18h, hidden),
// selection +258h = -1, +25Ch = 0.
void menu_prompt_register_005311c0(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x14) = host.gui_load_page_00aa5840("_DialogueBox");
    host.widget_set_visible_34(s.ptr(0x14), false);
    s.ptr(0x38) = host.gui_find_child_00aa7e00(s.ptr(0x14), "dialog_BG_FrameBox");
    s.ptr(0x18) = host.gui_load_page_00aa5840("_DialogueFarBlackPlane");
    host.widget_set_visible_34(s.ptr(0x18), false);
    s.at<std::int32_t>(0x258) = -1;
    s.at<std::uint8_t>(0x25C) = 0;
}

void menu_prompt_record_copy_construct_00531c90(Raw dst, Raw src, Seg15Host& host) {
    auto copy_string = [&](std::size_t off) {
        dst.at<std::uint32_t>(off) = 0;
        dst.at<std::uint32_t>(off + 4) = 0;
        if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + off}, Raw{src.p + off});
    };
    copy_string(0x0);
    dst.at<std::uint32_t>(0x8) = src.at<std::uint32_t>(0x8);
    dst.at<std::uint32_t>(0xC) = src.at<std::uint32_t>(0xC);
    copy_string(0x10);
    dst.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x18);
    for (std::size_t off = 0x20; off <= 0x34; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    copy_string(0x38);
    dst.at<std::uint8_t>(0x40) = src.at<std::uint8_t>(0x40);
    dst.at<std::uint32_t>(0x44) = src.at<std::uint32_t>(0x44);
}

// The list is {allocator, head node, size}; a node is {next, prev, record}.
void menu_prompt_record_list_clear_00531f80(Raw list, Seg15Host& host) {
    Raw head{static_cast<std::uint8_t*>(list.ptr(4))};
    auto* node = static_cast<std::uint8_t*>(head.ptr(0));
    head.ptr(0) = head.p;
    head.ptr(4) = head.p;
    list.at<std::uint32_t>(8) = 0;
    while (node != head.p) {
        auto* next = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
        host.menu_prompt_record_destroy_00530f90(Raw{node + 8});
        host.free_00bf65ac(node);
        node = next;
    }
}

// 005321F0: members in reverse declaration order, as the SEH states 6..2 run.
void menu_prompt_destroy_005321f0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CED3C4;
    s.at<std::uint32_t>(0x8) = 0x00CED3B0;
    s.at<std::uint32_t>(0xC) = 0x00CED3AC;
    s.at<std::uint32_t>(0x10) = 0x00CED38C;
    remove_page(s, 0x14, host);
    Raw list{s.p + 0x278};
    menu_prompt_record_list_clear_00531f80(list, host);
    host.free_00bf65ac(list.ptr(4));
    list.ptr(4) = nullptr;
    Raw strings{s.p + 0x268};  // 004324A0 inlined; the stack words are the vector and the screen
    if (strings.ptr(4) != nullptr) {
        host.native_string_range_destroy_00432050(strings.ptr(4), strings.ptr(8), strings.p, s.p);
        host.free_00bf65ac(strings.ptr(4));
    }
    strings.ptr(4) = nullptr;
    strings.ptr(8) = nullptr;
    strings.ptr(0xC) = nullptr;
    host.eh_vector_destruct_00bf7c6e(s.p + 0x60, 0x48, 7, 0x00530F90);
    host.eh_vector_destruct_00bf7c6e(s.p + 0x40, 0x10, 2, 0x004324A0);
    singleton_00e18e6c_clear_base_00530cc0(Raw{s.p + 0xC}, host);  // inlined in the image
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

void menu_prompt_collect_children_00532350(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x14));
}

void menu_prompt_collect_visible_pages_005329c0(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x14));
    if (host.widget_is_visible_38(s.ptr(0x18))) host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x18));
}

// The four buttons (screen +28h..+34h) complete with 1, 0, 2, 3.
void menu_prompt_listbox_activate_00532d70(Raw listener, void* list, Seg15Host& host) {
    static constexpr int kResults[4] = {1, 0, 2, 3};
    for (int i = 0; i < 4; ++i) {
        if (list == listener.ptr(0x20 + i * 4)) {
            host.menu_prompt_complete_00532c50(Raw{listener.p - 8}, kResults[i]);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// HUD formation (2Ah)
// ---------------------------------------------------------------------------

// 005337C0: actions 118h, 119h, 4Ah (004D92B0 with ECX = *00E188A8).
void read_directional_actions_005337c0(std::uint8_t* a, std::uint8_t* b, std::uint8_t* c, Seg15Host& host) {
    *a = host.action_edge_or_repeat_004d92b0(0x118);
    *b = host.action_edge_or_repeat_004d92b0(0x119);
    *c = host.action_edge_or_repeat_004d92b0(0x4A);
}

// 005353A0: only on the formation page (*(interface + 5Ch) + 170h == 128h):
// actions 11Ah, 11Bh, 4Ah. The trailing test of the vector at *(00F8BBF4) + 94h
// is an inlined front() check whose call (00BF6713) cannot be reached.
void hud_formation_read_navigation_actions_005353a0(std::uint8_t* a, std::uint8_t* b, std::uint8_t* c,
                                                    Seg15Host& host) {
    Raw page{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0x5C))};
    if (page.at<std::int32_t>(0x170) != 0x128) return;
    *a = host.action_edge_or_repeat_004d92b0(0x11A);
    *b = host.action_edge_or_repeat_004d92b0(0x11B);
    *c = host.action_edge_or_repeat_004d92b0(0x4A);
}

namespace {
struct WidgetShow {
    std::uint16_t off;
    bool show;
};
void apply_shows(Raw s, const WidgetShow* rows, std::size_t n, Seg15Host& host) {
    for (std::size_t i = 0; i < n; ++i) host.widget_set_visible_34(s.ptr(rows[i].off), rows[i].show);
}
}  // namespace

// 00533830: the formation-mode widget set (order as in the image).
void hud_formation_show_formation_widgets_00533830(Raw s, Seg15Host& host) {
    static constexpr WidgetShow kRows[] = {
        {0xD4, false}, {0xD8, false}, {0xDC, false}, {0xE0, true}, {0xE8, true}, {0xE4, true}, {0xEC, true},
        {0xFC, false}, {0x108, false}, {0x104, false}, {0x100, false}, {0x10C, true}, {0xF0, true}, {0xF4, true},
        {0xF8, true}};
    apply_shows(s, kRows, sizeof kRows / sizeof kRows[0], host);
}

// 00533920: the group's (+138h) unit +14h, its +114h object's +50h value
// chooses +D8h (shown when zero) against +D4h (shown when non-zero).
void hud_formation_refresh_order_widgets_00533920(Raw s, Seg15Host& host) {
    Raw group{static_cast<std::uint8_t*>(s.ptr(0x138))};
    Raw leader{static_cast<std::uint8_t*>(group.ptr(0x14))};
    Raw order{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(host.object_virtual(leader, 0x114)))};
    const bool has = host.object_virtual(order, 0x50) != 0;
    host.widget_set_visible_34(s.ptr(0xD8), !has);
    host.widget_set_visible_34(s.ptr(0xD4), has);
    static constexpr WidgetShow kRows[] = {
        {0xE0, false}, {0xE8, false}, {0xE4, false}, {0xDC, true}, {0xEC, true}, {0xFC, false}, {0x108, false},
        {0x104, false}, {0x100, false}, {0x10C, true}, {0xF0, true}, {0xF4, true}, {0xF8, true}};
    apply_shows(s, kRows, sizeof kRows / sizeof kRows[0], host);
}

// 00533A30: hide every formation widget.
void hud_formation_hide_all_widgets_00533a30(Raw s, Seg15Host& host) {
    static constexpr std::uint16_t kOffs[] = {0xD8, 0xD4, 0xE0, 0xE8, 0xDC, 0xE4, 0xEC, 0xFC,
                                              0x108, 0x104, 0x100, 0x10C, 0xF0, 0xF4, 0xF8};
    for (std::uint16_t off : kOffs) host.widget_set_visible_34(s.ptr(off), false);
}

void hud_formation_listener_call_slot04_00533b20(Raw listener, Seg15Host& host) {
    host.object_virtual(listener, 0x4);
}

// Unordered compares are unequal (FUCOMIP, LAHF, TEST AH,44h, JP).
bool vec2f_equal_00533d30(const float* a, const float* b) { return a[0] == b[0] && a[1] == b[1]; }

// 005347C0: vtable 00CED548, _free(+28h), _free(+2Ch), root vtable 00CE4974.
void session_message_formation_destruct_005347c0(Raw message, Seg15Host& host) {
    message.at<std::uint32_t>(0) = 0x00CED548;
    host.free_00bf6989(message.ptr(0x28));
    host.free_00bf6989(message.ptr(0x2C));
    message.at<std::uint32_t>(0) = 0x00CE4974;
}

// 00534BC0: with the formation UI flag (*(*(game + 19CCh) + 4ACh)) and a group
// (screen +138h, listener +D4h), run the screen's Exit (+1Ch) then Enter (+18h).
void hud_formation_refresh_if_enabled_00534bc0(Raw listener, Seg15Host& host) {
    Raw ui{static_cast<std::uint8_t*>(host.global_object(kGameGlobal).ptr(0x19CC))};
    if (ui.at<std::uint8_t>(0x4AC) == 0 || listener.ptr(0xD4) == nullptr) return;
    Raw screen{listener.p - 0x64};
    host.object_virtual(screen, 0x1C);
    host.object_virtual(screen, 0x18);
}

// 00534C00: collapse (0, 1), input contexts by the controlled unit's class
// (*(*00E188D8 + C4h): 8 submarine, 0Eh torpedo boat, else ship), collapse
// (0, 0), then tail-jump to leave the unit request level.
void hud_formation_restore_unit_input_contexts_00534c00(Seg15Host& host) {
    host.interface_collapse_overlays_0068ab80(0, 1);
    const std::int32_t kind = host.global_object(0x00E188D8).at<std::int32_t>(0xC4);
    host.interface_set_input_contexts(kind == 8 ? 0x00689FC0u : kind == 0xE ? 0x00689FE0u : 0x00689FA0u);
    host.interface_collapse_overlays_0068ab80(0, 0);
    host.interface_leave_unit_request_level_0068ab30();
}

namespace {
// Record: +1Ch value, +20h pressed, +24h held time, +28h held flag.
bool released_positive(Raw record) {
    const bool held = record.at<std::uint8_t>(0x28) != 0 && record.at<float>(0x24) > 0.0f;
    return !held && record.at<std::uint8_t>(0x20) != 0 && record.at<float>(0x1C) > 0.0f;
}
}  // namespace

bool input_action_released_positive_00534e90(Raw input, std::int32_t action) {
    return released_positive(Raw{static_cast<std::uint8_t*>(input.ptr(4)) + action * 0x30});
}

bool input_action_released_this_frame_00535ee0(std::int32_t action, Seg15Host& host) {
    return input_action_released_positive_00534e90(host.input_manager_004bec00(), action);
}

// 00536100: for the member listbox (listener +34h) keep the selection in +14h
// and move the cursor to the member: x = -m.x10 / [listener-48h] + 0.5,
// y = m.x20 / [listener-44h] + 0.5 (double 0.5 at 00D7A280, stored as floats).
void hud_formation_listbox_selection_changed_00536100(Raw listener, void* list, Seg15Host& host) {
    if (list != listener.ptr(0x34)) return;
    const std::int32_t index = host.listbox_selected_index_00a9c920(listener.ptr(0x34));
    Raw member = host.unit_group_member_record_0070d070(listener.ptr(0xC0), index);
    listener.at<std::int32_t>(0x14) = index;
    const float nx = -member.at<float>(0x10);
    const float ny = -member.at<float>(0x20);
    const float x = static_cast<float>(static_cast<double>(nx / Raw{listener.p - 0x48}.at<float>(0)) + 0.5);
    const float y = static_cast<float>(static_cast<double>(-ny / Raw{listener.p - 0x44}.at<float>(0)) + 0.5);
    host.gui_set_cursor_position_00aa1fa0(x, y);
}

// 00538D20, slot +24h, RET 4: pages +98h, +A0h, +A8h.
void hud_formation_collect_children_00538d20(Raw s, void* out, Seg15Host& host) {
    for (std::size_t off : {0x98u, 0xA0u, 0xA8u}) host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + off));
}

// 00538D60: map[member] = new 1A0h marker model (005A6AD0 with the scene +0Ch
// and screen+20h); a marker with a scene node (+128h) gets visibility 0.0f, 0.
void hud_formation_add_member_marker_00538d60(Raw s, void* map, std::uint32_t member, Seg15Host& host) {
    void** slot = host.map_subscript_00538910(map, member);
    void* block = host.operator_new_00bf681b(0x1A0);
    Raw marker = block ? host.unit_marker_model_construct_005a6ad0(block, member, s.ptr(0xC), s.p + 0x20) : Raw{nullptr};
    *slot = marker.p;
    if (marker.ptr(0x128) != nullptr) host.scene_node_set_visibility_00b6da70(marker.ptr(0x128), 0.0f, 0);
}

// 00538E30: base, the 3D scene sub-object at +8h, the listener bases at +64h
// and +78h, this screen's vtables, two map heads (+114h, +120h), a vector
// (+12Ch..+134h) and two list heads (+198h, +1A4h).
Raw hud_formation_construct_00538e30(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    host.simple_3d_scene_construct_0093cb10(Raw{s.p + 0x8});
    s.at<std::uint32_t>(0x64) = 0x00CE3CD4;
    s.at<std::uint32_t>(0x68) = 0;
    s.at<std::uint32_t>(0x6C) = 0;
    s.at<std::uint32_t>(0x70) = 0;
    s.at<std::uint8_t>(0x74) = 0;
    s.at<std::uint32_t>(0x78) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0x8) = 0x00CED6F8;
    s.at<std::uint32_t>(0x0) = 0x00CED6FC;
    s.at<std::uint32_t>(0x64) = 0x00CED6E0;
    s.at<std::uint32_t>(0x78) = 0x00CED6CC;
    for (std::size_t head_off : {0x114u, 0x120u}) {
        Raw head{static_cast<std::uint8_t*>(host.tree_buy_head_node_005359a0())};
        s.ptr(head_off) = head.p;
        head.at<std::uint8_t>(0x15) = 1;  // nil
        head.ptr(4) = head.p;
        head.ptr(0) = head.p;
        head.ptr(8) = head.p;
        s.at<std::uint32_t>(head_off + 4) = 0;
    }
    s.at<std::uint32_t>(0x12C) = 0;
    s.at<std::uint32_t>(0x130) = 0;
    s.at<std::uint32_t>(0x134) = 0;
    s.ptr(0x198) = host.list_buy_head_node(0x00535720);
    s.at<std::uint32_t>(0x19C) = 0;
    s.ptr(0x1A4) = host.list_buy_head_node(0x005356C0);
    s.at<std::uint32_t>(0x1A8) = 0;
    s.at<std::uint8_t>(0x142) = 0;
    s.at<std::uint8_t>(0x95) = 0;
    s.at<std::uint8_t>(0x94) = 0;
    return s;
}

// ---------------------------------------------------------------------------
// Frame statistics (57h)
// ---------------------------------------------------------------------------

// 0053B9E0, __fastcall(record): ten dwords zeroed (store order +0..+0Ch, +18h,
// +1Ch, +10h, +14h, +20h, +24h).
void record_zero_init_0053b9e0(Raw record) {
    for (std::size_t off : {0x0u, 0x4u, 0x8u, 0xCu, 0x18u, 0x1Cu, 0x10u, 0x14u, 0x20u, 0x24u}) {
        record.at<std::uint32_t>(off) = 0;
    }
}

// 0053BA10: base, +8h and +78h cleared, vtable 00CEDA10, +68h..+74h cleared.
Raw frame_stats_construct_0053ba10(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = 0;
    s.at<std::uint32_t>(0x78) = 0;
    s.at<std::uint32_t>(0x0) = 0x00CEDA10;
    for (std::size_t off = 0x68; off <= 0x74; off += 4) s.at<std::uint32_t>(off) = 0;
    return s;
}

// 0053BA70, slot +18h: the minimum (+7Ch) starts at FLT_MAX (00D7A248), the
// maximum (+80h) at -FLT_MAX (00D7A244).
void frame_stats_enter_0053ba70(Raw s) {
    s.at<float>(0x7C) = 3.4028234663852886e+38f;
    s.at<float>(0x80) = -3.4028234663852886e+38f;
}

int frame_stats_printf_0053bab0(char* line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int n = std::vsnprintf(line, 0x210, format, args);
    va_end(args);
    return n;
}

// 0053BB40: +4Ch..+90h zeroed (pairs, high dword first as in the image).
void frame_stats_reset_accumulators_0053bb40(Raw stats) {
    for (std::size_t off = 0x4C; off <= 0x90; off += 4) stats.at<std::uint32_t>(off) = 0;
}

void frame_stats_destruct_0053c440(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CEDA10;
    remove_page(s, 0xC, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// 0053C510: end the profiler record and take its elapsed seconds; when the
// overlay is active (+5h) in mode +8h: 1 averages four profiler counters
// over each 0.25 s window, 2 counts frames over each 2.0 s window, 3 reports
// 1 / elapsed every frame. The rate updates the min (+7Ch) and max (+80h)
// and goes to 0053BCC0. Then the record is reset and a new one begun.
void frame_stats_accumulate_0053c510(Raw stats, Seg15Host& host) {
    host.profiler_record_end_00be32b0();
    const float elapsed = host.profiler_elapsed_seconds_00be3360();
    const std::int32_t mode = stats.at<std::int32_t>(0x8);
    auto min_max = [&](float rate) {
        if (stats.at<float>(0x7C) > rate) stats.at<float>(0x7C) = rate;
        if (rate > stats.at<float>(0x80)) stats.at<float>(0x80) = rate;
    };
    if (stats.at<std::uint8_t>(kActive) != 0 && mode != 0) {
        if (mode == 1) {
            stats.at<float>(0x68) = host.profiler_counter_value(0x0109D014) + stats.at<float>(0x68);
            stats.at<float>(0x6C) = host.profiler_counter_value(0x0109DB08) + stats.at<float>(0x6C);
            const float a = host.profiler_counter_value(0x0109DB34);
            const float b = host.profiler_counter_value(0x0109DB14);
            stats.at<std::int32_t>(0x78) += 1;
            stats.at<float>(0x70) = a + b + stats.at<float>(0x70);
            const float window = elapsed + stats.at<float>(0x74);
            stats.at<float>(0x74) = window;
            if (window > 0.25f) {
                const float n = static_cast<float>(stats.at<std::int32_t>(0x78));
                const float rate = n / window;
                min_max(rate);
                stats.at<float>(0x68) = stats.at<float>(0x68) / n;
                stats.at<float>(0x6C) = stats.at<float>(0x6C) / n;
                stats.at<float>(0x70) = stats.at<float>(0x70) / n;
                host.frame_stats_format_lines_0053bcc0(stats, rate);
                stats.at<float>(0x74) = 0.0f;
                stats.at<std::int32_t>(0x78) = 0;
                stats.at<float>(0x68) = 0.0f;
                stats.at<float>(0x6C) = 0.0f;
                stats.at<float>(0x70) = 0.0f;
            }
        } else if (mode == 2) {
            stats.at<std::int32_t>(0x78) += 1;
            const float window = elapsed + stats.at<float>(0x74);
            stats.at<float>(0x74) = window;
            if (window > 2.0f) {
                const float rate = static_cast<float>(stats.at<std::int32_t>(0x78)) / window;  // FIDIVR
                min_max(rate);
                host.frame_stats_format_lines_0053bcc0(stats, rate);
                stats.at<float>(0x74) = 0.0f;
                stats.at<std::int32_t>(0x78) = 0;
            }
        } else if (mode == 3) {
            host.frame_stats_format_lines_0053bcc0(stats, 1.0f / elapsed);
        }
    }
    host.profiler_record_reset_00be3300();
    host.profiler_record_begin_00be3260();
}

void frame_stats_collect_children_0053d220(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0xC));
}

// ---------------------------------------------------------------------------
// Free camera (2Bh)
// ---------------------------------------------------------------------------

namespace {
// Reference-counted handle {vtable, count +4}: InterlockedIncrement /
// InterlockedDecrement (00CE2220), slot 0 (no arguments) when the count drops
// to zero.
void ref_add(void* handle) {
    if (handle) ++*reinterpret_cast<long*>(static_cast<std::uint8_t*>(handle) + 4);
}
void ref_release(void* handle, Seg15Host& host) {
    if (handle && --*reinterpret_cast<long*>(static_cast<std::uint8_t*>(handle) + 4) == 0) {
        host.object_virtual(Raw{static_cast<std::uint8_t*>(handle)}, 0x0);
    }
}
void ref_assign(Raw dst, std::size_t off, void* value, Seg15Host& host) {
    void* old = dst.ptr(off);
    if (old == value) return;
    dst.ptr(off) = value;
    ref_add(value);
    ref_release(old, host);
}
}  // namespace

void free_camera_observer_released_0053d2c0(Raw observer, void* subject) {
    if (subject == observer.ptr(0x1C)) observer.ptr(0x1C) = nullptr;
}

void free_camera_set_matrix_0053d700(const void* source, Seg15Host& host) {
    host.matrix_copy_004134f0(host.global_block(0x00E192B0).p, source);
    host.global_block(0x00E192A9).at<std::uint8_t>(0) = 1;
}

// 0053DC70: vtable 00CEDC60, +28h..+30h cleared, the three vec3s +4h, +10h,
// +1Ch from the vec3 at 00F87574.
void free_camera_effect_entry_construct_0053dc70(Raw entry, Seg15Host& host) {
    entry.at<std::uint32_t>(0) = 0x00CEDC60;
    entry.at<std::uint32_t>(0x28) = 0;
    entry.at<std::uint32_t>(0x2C) = 0;
    entry.at<std::uint32_t>(0x30) = 0;
    Raw origin = host.global_block(0x00F87574);
    for (std::size_t i = 0; i < 3; ++i) entry.at<std::uint32_t>(0x1C + i * 4) = origin.at<std::uint32_t>(i * 4);
    for (std::size_t i = 0; i < 3; ++i) entry.at<std::uint32_t>(0x10 + i * 4) = entry.at<std::uint32_t>(0x1C + i * 4);
    for (std::size_t i = 0; i < 3; ++i) entry.at<std::uint32_t>(0x4 + i * 4) = entry.at<std::uint32_t>(0x10 + i * 4);
}

// 0053DCE0: a live point effect (+2Ch) has its children stopped and byte +9
// set, then its reference is released.
void free_camera_effect_entry_destruct_0053dce0(Raw entry, Seg15Host& host) {
    entry.at<std::uint32_t>(0) = 0x00CEDC60;
    if (void* effect = entry.ptr(0x2C)) {
        host.point_effect_stop_children_00867b10(effect);
        static_cast<std::uint8_t*>(effect)[9] = 1;
    }
    if (entry.ptr(0x2C)) {
        ref_release(entry.ptr(0x2C), host);
        entry.ptr(0x2C) = nullptr;
    }
}

// 0053DF40: release the handles +2Ch, +28h, +24h, then the string +1Ch.
void free_camera_effect_record_destroy_0053df40(Raw record, Seg15Host& host) {
    for (std::size_t off : {0x2Cu, 0x28u, 0x24u}) {
        if (record.ptr(off)) {
            ref_release(record.ptr(off), host);
            record.ptr(off) = nullptr;
        }
    }
    host.native_string_release(Raw{record.p + 0x1C});
}

void free_camera_effect_entry_copy_0053e030(Raw dst, Raw src) {
    dst.at<std::uint32_t>(0) = 0x00CEDC60;
    for (std::size_t off = 0x4; off <= 0x28; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    dst.ptr(0x2C) = nullptr;
    if (void* h = src.ptr(0x2C)) {
        dst.ptr(0x2C) = h;
        ref_add(h);
    }
    dst.at<std::uint32_t>(0x30) = src.at<std::uint32_t>(0x30);
}

// 0053E0E0: 5InchExplosionArmour into +28h, 5InchBulletTracer into +24h, then
// the settings +0h 1000.0, +4h 0, +8h 0.1, +0Ch 4.0, +10h 300.0, +14h 30.0,
// +18h byte 0.
void free_camera_acquire_test_effects_0053e0e0(Raw block, Seg15Host& host) {
    void* explosion = host.effect_handle_acquire_00871ba0("5InchExplosionArmour");
    ref_assign(block, 0x28, explosion, host);
    ref_release(explosion, host);
    void* tracer = host.effect_handle_acquire_00871ba0("5InchBulletTracer");
    ref_assign(block, 0x24, tracer, host);
    ref_release(tracer, host);
    block.at<std::uint32_t>(0x4) = 0;
    block.at<float>(0xC) = 4.0f;
    block.at<float>(0x8) = 0.1f;
    block.at<float>(0x0) = 1000.0f;
    block.at<float>(0x10) = 300.0f;
    block.at<std::uint8_t>(0x18) = 0;
    block.at<float>(0x14) = 30.0f;
}

void free_camera_effect_record_copy_0053e300(Raw dst, Raw src, Seg15Host& host) {
    for (std::size_t off = 0; off <= 0x14; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    dst.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x18);
    dst.at<std::uint32_t>(0x1C) = 0;
    dst.at<std::uint32_t>(0x20) = 0;
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 0x1C}, Raw{src.p + 0x1C});
    for (std::size_t off : {0x24u, 0x28u, 0x2Cu}) {
        dst.ptr(off) = nullptr;
        if (void* h = src.ptr(off)) {
            dst.ptr(off) = h;
            ref_add(h);
        }
    }
}

void free_camera_effect_record_assign_0053e460(Raw dst, Raw src, Seg15Host& host) {
    for (std::size_t off = 0; off <= 0x14; off += 4) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    dst.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x18);
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 0x1C}, Raw{src.p + 0x1C});
    for (std::size_t off : {0x24u, 0x28u, 0x2Cu}) ref_assign(dst, off, src.ptr(off), host);
}

// 0053E6F0, cdecl(first, last, dest_end) -> dest_end - (last - first).
std::uint8_t* free_camera_effect_record_copy_backward_0053e6f0(std::uint8_t* first, std::uint8_t* last,
                                                               std::uint8_t* dest_end, Seg15Host& host) {
    while (last != first) {
        last -= 0x30;
        dest_end -= 0x30;
        free_camera_effect_record_assign_0053e460(Raw{dest_end}, Raw{last}, host);
    }
    return dest_end;
}

// 0053E850: the effect-entry list (node {next, prev, entry}) emptied; each
// entry destroyed (0053DCE0) and its node freed (00BF65AC).
void free_camera_effect_list_clear_0053e850(Raw list, Seg15Host& host) {
    Raw head{static_cast<std::uint8_t*>(list.ptr(4))};
    auto* node = static_cast<std::uint8_t*>(head.ptr(0));
    head.ptr(0) = head.p;
    head.ptr(4) = head.p;
    list.at<std::uint32_t>(8) = 0;
    while (node != head.p) {
        auto* next = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
        free_camera_effect_entry_destruct_0053dce0(Raw{node + 8}, host);
        host.free_00bf65ac(node);
        node = next;
    }
}

namespace {
constexpr VectorElementOps kEffectRecordOps{0x30, free_camera_effect_record_copy_0053e300,
                                            free_camera_effect_record_assign_0053e460,
                                            free_camera_effect_record_destroy_0053df40, 0x05555555};
}  // namespace

void free_camera_effect_record_insert_n_0053ff10(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value,
                                                 Seg15Host& host) {
    vector_insert_n(vec, where, count, value, kEffectRecordOps, host);
}

// 005401F0, __fastcall(block at screen+64h): vector +4h emptied (+8h..+10h),
// list head +18h, size +1Ch, +20h..+28h cleared.
void free_camera_construct_effect_lists_005401f0(Raw block, Seg15Host& host) {
    block.at<std::uint32_t>(0x8) = 0;
    block.at<std::uint32_t>(0xC) = 0;
    block.at<std::uint32_t>(0x10) = 0;
    block.ptr(0x18) = host.list_buy_head_node_0053da70();
    for (std::size_t off = 0x1C; off <= 0x28; off += 4) block.at<std::uint32_t>(off) = 0;
}

// 00540260: clear the list (+14h), free its head, tidy the vector (+4h).
void free_camera_release_effects_00540260(Raw block, Seg15Host& host) {
    Raw list{block.p + 0x14};
    free_camera_effect_list_clear_0053e850(list, host);
    host.free_00bf65ac(list.ptr(4));
    list.ptr(4) = nullptr;
    host.effect_vector_tidy_0053f250(Raw{block.p + 4});
}

// 005402D0: vtables, free the settings block (+60h), drop the page (+1Ch),
// release the effects (+64h), the callback owner (+8h), then the base.
void free_camera_destruct_005402d0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CEDCA8;
    s.at<std::uint32_t>(0x8) = 0x00CEDC90;
    host.free_00bf65ac(s.ptr(0x60));
    remove_page(s, 0x1C, host);
    free_camera_release_effects_00540260(Raw{s.p + 0x64}, host);
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

void free_camera_collect_children_00540380(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x1C));
}

// 00540BA0: base, callback owner at +8h (00CE3CD4, cleared), vtables, +24h
// cleared, the 44h settings block (+60h, bytes +0 and +1Ch cleared), the
// effect lists (+64h), speed +2Ch 500.0, +30h 1.7453293 (100 degrees),
// +34h 111.5, +38h..+48h and +54h..+5Ch cleared.
Raw free_camera_construct_00540ba0(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = 0x00CE3CD4;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x0) = 0x00CEDCA8;
    s.at<std::uint32_t>(0x8) = 0x00CEDC90;
    s.at<std::uint32_t>(0x24) = 0;
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x44));
    if (block) {
        block[0] = 0;
        block[0x1C] = 0;
    }
    s.ptr(0x60) = block;
    free_camera_construct_effect_lists_005401f0(Raw{s.p + 0x64}, host);
    s.at<float>(0x2C) = 500.0f;
    s.at<float>(0x30) = 1.7453292608261108f;
    s.at<float>(0x34) = 111.5f;
    for (std::size_t off : {0x54u, 0x58u, 0x5Cu, 0x38u, 0x3Cu, 0x40u, 0x44u, 0x48u}) s.at<std::uint32_t>(off) = 0;
    return s;
}

// ---------------------------------------------------------------------------
// Debug text lists, command bar, small STL instances
// ---------------------------------------------------------------------------

std::uint8_t* int_bool_uninit_copy_00542200(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest) {
    for (; first != last; first += 8, dest += 8) {
        if (dest) {
            std::memcpy(dest, first, 4);
            dest[4] = first[4];
        }
    }
    return dest;
}

void int_bool_uninit_fill_n_00542cb0(std::uint8_t* dest, std::uint32_t count, const std::uint8_t* value) {
    for (; count != 0; --count, dest += 8) {
        if (dest) {
            std::memcpy(dest, value, 4);
            dest[4] = value[4];
        }
    }
}

void pod8_fill_00542bb0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value) {
    for (; first != last; first += 8) std::memcpy(first, value, 8);
}

std::uint8_t* pod8_copy_backward_00543050(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest_end) {
    while (last != first) {
        last -= 8;
        dest_end -= 8;
        std::memcpy(dest_end, last, 8);
    }
    return dest_end;
}

namespace {
void int_bool_copy(Raw dst, Raw src, Seg15Host&) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    dst.at<std::uint8_t>(4) = src.at<std::uint8_t>(4);
}
void pod8_assign(Raw dst, Raw src, Seg15Host&) { std::memcpy(dst.p, src.p, 8); }
void pod_destroy(Raw, Seg15Host&) {}
constexpr VectorElementOps kIntBoolOps{8, int_bool_copy, pod8_assign, pod_destroy, 0x1FFFFFFF};
}  // namespace

void int_bool_vector_insert_n_00543e90(Raw vec, std::uint8_t* where, std::uint32_t count, Raw value, Seg15Host& host) {
    vector_insert_n(vec, where, count, value, kIntBoolOps, host);
}

// The rows' std::string text is inline (buffer +8h) while the capacity (+1Ch)
// is below 10h, else the pointer at +8h. The vector bound checks before each
// access call 00BF6713 only for an index outside the vector (not modelled).
void debug_text_list_refresh_rows_00542620(Raw list, Seg15Host& host) {
    for (std::uint32_t i = 0;; ++i) {
        auto* first = static_cast<std::uint8_t*>(list.ptr(0x14));
        auto* last = static_cast<std::uint8_t*>(list.ptr(0x18));
        if (first == nullptr || i >= static_cast<std::uint32_t>((last - first) / 0x24)) break;
        Raw row{first + i * 0x24};
        const char* text = row.at<std::uint32_t>(0x1C) < 0x10 ? reinterpret_cast<const char*>(row.p + 8)
                                                              : static_cast<const char*>(row.ptr(8));
        host.text_set_localised_cstr_00abbe50(row.ptr(0x20), text);
        const float* colour;
        if (row.at<std::uint8_t>(0) == 0) {
            colour = &list.at<float>(0x48);
        } else if (list.at<std::uint8_t>(0x24) != 0 && list.at<std::uint32_t>(0x20) == i) {
            colour = &list.at<float>(0x38);
        } else {
            colour = &list.at<float>(0x28);
        }
        host.widget_set_colour_50(row.ptr(0x20), colour);
    }
    host.widget_set_visible_34(list.ptr(0x8), true);
}

// 00542D70 (00542300 / 00541AE0), 00542EE0 (00542890 / 005427E0), 005439B0
// (00543690 / 00541B30).
void debug_text_list_poll_up_down(Raw list, std::uint32_t on_46, std::uint32_t on_47, Seg15Host& host) {
    const bool a47 = host.action_edge_or_repeat_004d92b0(0x47);
    if (host.action_edge_or_repeat_004d92b0(0x46)) {
        host.tail_call(on_46, list);
    } else if (a47) {
        host.tail_call(on_47, list);
    }
}

// 00543410: buttons are IntBool elements {widget, enabled} in the vector at +8h.
// ButtonLamp_Icon is looked up and not used; ButtonText_Text is coloured
// {32, 32, 32, 1} when disabled, else {255, 255, 230, 1}, and an enabled button
// is focused when it is the selection (+14h).
void command_bar_set_button_00543410(Raw bar, std::uint32_t index, Seg15Host& host) {
    Raw button{static_cast<std::uint8_t*>(bar.ptr(8)) + index * 8};
    host.gui_find_child_00aa7e00(button.ptr(0), "ButtonLamp_Icon");
    void* text = host.gui_find_child_00aa7e00(button.ptr(0), "ButtonText_Text");
    if (button.at<std::uint8_t>(4) == 0) {
        const float grey[4] = {32.0f, 32.0f, 32.0f, 1.0f};
        host.widget_set_colour_50(text, grey);
        return;
    }
    const float lit[4] = {255.0f, 255.0f, 230.0f, 1.0f};
    host.widget_set_colour_50(text, lit);
    host.widget_set_focus_60(button.ptr(0), index == bar.at<std::uint32_t>(0x14));
}

namespace {
void sbo_reset(std::uint8_t* e) {
    Raw r{e};
    r.at<std::uint32_t>(0x1C) = 0xF;
    r.at<std::uint32_t>(0x18) = 0;
    r.at<std::uint8_t>(0x8) = 0;
}
void sbo_copy_fields(std::uint8_t* dst, const std::uint8_t* src, Seg15Host& host) {
    dst[0] = src[0];
    host.legacy_string_assign_substring(Raw{dst + 4}, Raw{const_cast<std::uint8_t*>(src) + 4});
    std::memcpy(dst + 0x20, src + 0x20, 4);
}
}  // namespace

// 00543A60, __fastcall(ECX slot, EDX src).
void byte_sbo_placement_copy_00543a60(std::uint8_t* slot, const std::uint8_t* src, Seg15Host& host) {
    if (!slot) return;
    slot[0] = src[0];
    sbo_reset(slot);
    host.legacy_string_assign_substring(Raw{slot + 4}, Raw{const_cast<std::uint8_t*>(src) + 4});
    std::memcpy(slot + 0x20, src + 0x20, 4);
}

// 00543C20: free a heap buffer (capacity above 0Fh), then reset to empty.
void byte_sbo_destroy_range_00543c20(std::uint8_t* first, std::uint8_t* last, Seg15Host& host) {
    for (; first != last; first += 0x24) {
        Raw r{first};
        if (r.at<std::uint32_t>(0x1C) > 0xF) host.free_00bf65ac(r.ptr(0x8));
        sbo_reset(first);
    }
}

std::uint8_t* byte_sbo_copy_00543ca0(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest,
                                     Seg15Host& host) {
    for (; first != last; first += 0x24, dest += 0x24) sbo_copy_fields(dest, first, host);
    return dest;
}

std::uint8_t* byte_sbo_copy_backward_00543cf0(const std::uint8_t* first, const std::uint8_t* last,
                                              std::uint8_t* dest_end, Seg15Host& host) {
    while (last != first) {
        last -= 0x24;
        dest_end -= 0x24;
        sbo_copy_fields(dest_end, last, host);
    }
    return dest_end;
}

// 00543D40: the catch handler (destroy what was built, rethrow) is not modelled.
std::uint8_t* byte_sbo_uninit_copy_00543d40(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* dest,
                                            Seg15Host& host) {
    for (; first != last; first += 0x24, dest += 0x24) {
        if (dest) {
            dest[0] = first[0];
            sbo_reset(dest);
            host.legacy_string_assign_substring(Raw{dest + 4}, Raw{const_cast<std::uint8_t*>(first) + 4});
            std::memcpy(dest + 0x20, first + 0x20, 4);
        }
    }
    return dest;
}

void byte_sbo_fill_005443b0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value, Seg15Host& host) {
    for (; first != last; first += 0x24) sbo_copy_fields(first, value, host);
}

// ===========================================================================
// Segment 22
// ===========================================================================

std::uint8_t* vehicle_camera_array_at_005cd820(Raw array, std::int32_t index, Seg15Host& host) {
    if (array.at<std::int32_t>(4) <= index) host.vehicle_camera_array_resize(array, index + 1);
    return static_cast<std::uint8_t*>(array.ptr(0)) + index * 0x5C;
}

// 005CE4D0: vtables, page +1Ch, the two trees (+70h through 005CE340, +64h
// through 005CD710: erase all, free the head, clear head and size), the
// callback owner (+8h), then the base.
void movie_camera_debug_destruct_005ce4d0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CF1968;
    s.at<std::uint32_t>(0x8) = 0x00CF1950;
    remove_page(s, 0x1C, host);
    host.tree_erase_all(0x005CE340, Raw{s.p + 0x70});
    host.free_00bf65ac(s.ptr(0x74));
    s.at<std::uint32_t>(0x74) = 0;
    s.at<std::uint32_t>(0x78) = 0;
    host.tree_erase_all(0x005CD710, Raw{s.p + 0x64});
    host.free_00bf65ac(s.ptr(0x68));
    s.at<std::uint32_t>(0x68) = 0;
    s.at<std::uint32_t>(0x6C) = 0;
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

void movie_camera_debug_collect_children_005ce5d0(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x1C));
}

// 005CE610: base, callback owner at +8h, vtables, the two tree heads (+68h
// nil byte +15h, +74h nil byte +21h), sizes, +30h cleared.
Raw movie_camera_debug_construct_005ce610(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = 0x00CE3CD4;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x0) = 0x00CF1968;
    s.at<std::uint32_t>(0x8) = 0x00CF1950;
    struct HeadSpec { std::size_t off; std::uint32_t routine; std::size_t nil; };
    for (const HeadSpec& h : {HeadSpec{0x68, 0x005CC2E0, 0x15}, HeadSpec{0x74, 0x005CC330, 0x21}}) {
        Raw head{static_cast<std::uint8_t*>(host.tree_buy_head_node(h.routine))};
        s.ptr(h.off) = head.p;
        head.at<std::uint8_t>(h.nil) = 1;
        head.ptr(4) = head.p;
        head.ptr(0) = head.p;
        head.ptr(8) = head.p;
        s.at<std::uint32_t>(h.off + 4) = 0;
    }
    s.at<std::uint32_t>(0x30) = 0;
    return s;
}

// 005CF090: append src's UTF-16 characters (no terminator handling beyond the resize).
void wide_string_append_005cf090(Raw dst, Raw src, Seg15Host& host) {
    const std::uint32_t n = src.at<std::uint32_t>(0);
    if (n == 0) return;
    const std::uint32_t old = dst.at<std::uint32_t>(0);
    host.wide_string_resize_004c53e0(dst, n + old);
    std::memcpy(static_cast<std::uint16_t*>(dst.ptr(4)) + old, src.ptr(4), n * 2);
}

// 005CF0F0: the native string +0Ch, then the wide string +4h ((count + 1) * 2 bytes).
void chat_overlay_line_destroy_005cf0f0(Raw line, Seg15Host& host) {
    host.native_string_release(Raw{line.p + 0xC});
    host.wide_string_release(Raw{line.p + 0x4});
}

// 005CF180: the native string +8h, then the wide string +0h.
void chat_history_line_destroy_005cf180(Raw line, Seg15Host& host) {
    host.native_string_release(Raw{line.p + 0x8});
    host.wide_string_release(Raw{line.p + 0x0});
}

// 005CF260, slot +10h: page _Chat into +3Ch, +8Ch and +8Dh cleared, then the
// screen's slot +14h.
void multi_chat_register_005cf260(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x3C) = host.gui_load_page_00aa5840("_Chat");
    s.at<std::uint8_t>(0x8C) = 0;
    s.at<std::uint8_t>(0x8D) = 0;
    host.object_virtual(s, 0x14);
}

// 005CF320: only in a session of mode 1 (*(game + 1FE4h)): an empty sender,
// the arguments and ids -1, -1.
void session_post_system_chat_line_005cf320(std::int32_t channel, std::int32_t kind, Raw a, Raw b, Seg15Host& host) {
    if (host.global_object(kGameGlobal).at<std::int32_t>(0x1FE4) != 1) return;
    host.session_send_chat_00772170(host.native_string_temp(""), channel, kind, a, b, -1, -1);
}

void warning_manager_post_loss_chat_line_005cf3d0(std::int32_t side, Raw a, Raw b, Seg15Host& host) {
    if (side == 0) session_post_system_chat_line_005cf320(9, 5, a, b, host);
    else if (side == 1) session_post_system_chat_line_005cf320(10, 5, a, b, host);
}

void chat_overlay_line_copy_005cf460(Raw dst, Raw src, Seg15Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    host.wide_string_copy_construct_004c8dd0(Raw{dst.p + 4}, Raw{src.p + 4});
    dst.at<std::uint32_t>(0xC) = 0;
    dst.at<std::uint32_t>(0x10) = 0;
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 0xC}, Raw{src.p + 0xC});
    dst.at<std::uint32_t>(0x14) = src.at<std::uint32_t>(0x14);
}

void chat_history_line_copy_005cf4f0(Raw dst, Raw src, Seg15Host& host) {
    host.wide_string_copy_construct_004c8dd0(dst, src);
    dst.at<std::uint32_t>(0x8) = 0;
    dst.at<std::uint32_t>(0xC) = 0;
    if (dst.p != src.p) host.native_string_copy_0041dd40(Raw{dst.p + 0x8}, Raw{src.p + 0x8});
    dst.at<std::uint32_t>(0x10) = src.at<std::uint32_t>(0x10);
}

Raw wide_string_concat_005cf570(Raw a, Raw out, Raw b, Seg15Host& host) {
    host.wide_string_copy_construct_004c8dd0(out, a);
    wide_string_append_005cf090(out, b, host);
    return out;
}

namespace {
std::uint8_t* chat_buy_node(std::size_t bytes, void* next, void* prev, Raw line,
                            void (*copy)(Raw, Raw, Seg15Host&), Seg15Host& host) {
    auto* node = static_cast<std::uint8_t*>(host.operator_new_00bf55be(bytes));
    if (node) {
        Raw{node}.ptr(0) = next;
        Raw{node}.ptr(4) = prev;
        copy(Raw{node + 8}, line, host);
    }
    return node;
}
}  // namespace

std::uint8_t* chat_overlay_list_buy_node_005cfa40(void* next, void* prev, Raw line, Seg15Host& host) {
    return chat_buy_node(0x20, next, prev, line, chat_overlay_line_copy_005cf460, host);
}

std::uint8_t* chat_history_list_buy_node_005cfae0(void* next, void* prev, Raw line, Seg15Host& host) {
    return chat_buy_node(0x1C, next, prev, line, chat_history_line_copy_005cf4f0, host);
}

// 005CFB80: the three list widgets (+44h..+4Ch) through 00AA83F0, then the
// overlay list and both history lists emptied (lines destroyed, nodes freed),
// and +8Dh set.
void multi_chat_clear_logs_005cfb80(Raw s, Seg15Host& host) {
    for (std::size_t off : {0x44u, 0x48u, 0x4Cu}) host.widget_clear_00aa83f0(s.ptr(off));
    struct ListSpec { std::size_t off; void (*destroy)(Raw, Seg15Host&); };
    for (const ListSpec& l : {ListSpec{0x68, chat_overlay_line_destroy_005cf0f0},
                              ListSpec{0x74, chat_history_line_destroy_005cf180},
                              ListSpec{0x80, chat_history_line_destroy_005cf180}}) {
        Raw head{static_cast<std::uint8_t*>(s.ptr(l.off + 4))};
        auto* node = static_cast<std::uint8_t*>(head.ptr(0));
        head.ptr(0) = head.p;
        head.ptr(4) = head.p;
        s.at<std::uint32_t>(l.off + 8) = 0;
        while (node != head.p) {
            auto* next = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
            l.destroy(Raw{node + 8}, host);
            host.free_00bf65ac(node);
            node = next;
        }
    }
    s.at<std::uint8_t>(0x8D) = 1;
}

// 005CFDA0: base, text-listener base at +8h, vtables 00CF19C0 / 00CF1998,
// list heads +6Ch (005CEE20), +78h and +84h (005CEE40) with sizes cleared.
Raw multi_chat_construct_005cfda0(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    host.text_listener_construct_00a97260(Raw{s.p + 0x8});
    s.at<std::uint32_t>(0x8) = 0x00CF19C0;
    s.at<std::uint32_t>(0x0) = 0x00CF1998;
    s.ptr(0x6C) = host.chat_list_buy_head(0x005CEE20);
    s.at<std::uint32_t>(0x70) = 0;
    s.ptr(0x78) = host.chat_list_buy_head(0x005CEE40);
    s.at<std::uint32_t>(0x7C) = 0;
    s.ptr(0x84) = host.chat_list_buy_head(0x005CEE40);
    s.at<std::uint32_t>(0x88) = 0;
    return s;
}

// 005CFE60: vtables, page +3Ch, the lists +80h, +74h (005CF950) and +68h
// (005CF910) cleared with their heads freed, the text listener, the base.
void multi_chat_destroy_005cfe60(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CF1998;
    s.at<std::uint32_t>(0x8) = 0x00CF19C0;
    remove_page(s, 0x3C, host);
    for (std::size_t off : {0x80u, 0x74u}) {
        host.chat_list_clear(0x005CF950, Raw{s.p + off});
        host.free_00bf65ac(s.ptr(off + 4));
        s.ptr(off + 4) = nullptr;
    }
    host.chat_list_clear(0x005CF910, Raw{s.p + 0x68});
    host.free_00bf65ac(s.ptr(0x6C));
    s.ptr(0x6C) = nullptr;
    host.text_listener_destroy_00a97140(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

void multi_chat_exit_005cff40(Raw s, Seg15Host& host) {
    multi_chat_clear_logs_005cfb80(s, host);
    s.at<std::uint8_t>(0x8D) = 0;
}

void multi_chat_collect_children_005d0020(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x3C));
}

// 005D07E0: prefix = the player name (game +6A0h, or +68Ch when empty) + ": "
// (00CF1A80); both the input text (+5Ch) and the channel text (+60h) are
// fitted (0067D940 with screen+20h and +28h), coloured from the config
// (+218h for the input, +208h or +218h by the team flag +8Ch for the channel)
// and get the cursor mode (*00E12F34 == 0).
void multi_chat_refresh_mode_label_005d07e0(Raw s, Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    Raw name{game.p + (game.at<std::uint32_t>(0x6A0) != 0 ? 0x6A0 : 0x68C)};
    Raw prefix = host.native_string_temp("");
    host.native_string_concat_004261a0(name, prefix, host.native_string_temp(": "));
    const bool cursor_mode = host.global_block(0x00E12F34).at<std::uint8_t>(0) == 0;
    host.text_fit_range_one_line_0067d940(s.ptr(0x5C), prefix, Raw{s.p + 0x20}, s.at<std::uint32_t>(0x28));
    host.widget_set_colour_50(s.ptr(0x5C), &host.global_config_00432650().at<float>(0x218));
    host.text_set_cursor_mode_00ab95f0(s.ptr(0x5C), cursor_mode);
    host.text_fit_range_one_line_0067d940(s.ptr(0x60), prefix, Raw{s.p + 0x20}, s.at<std::uint32_t>(0x28));
    const std::size_t colour = 0x208 + (s.at<std::uint8_t>(0x8C) != 0 ? 0x10 : 0);
    host.widget_set_colour_50(s.ptr(0x60), &host.global_config_00432650().at<float>(colour));
    host.text_set_cursor_mode_00ab95f0(s.ptr(0x60), host.global_block(0x00E12F34).at<std::uint8_t>(0) == 0);
}

// 005D0B10: channel 8 (all) in the lobby without the team flag; in a mission
// or with the team flag the local slot (game +18ECh) must be 0..7 (else the
// sound request +1Ch is raised and nothing is sent) and the channel is 9 or 10
// by the slot record's +28h. The line (+20h) is sent with the player name and
// the record's ids (+80h, +84h), then cleared with the cursor (+28h).
void multi_chat_send_current_line_005d0b10(Raw s, Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    std::int32_t channel = 8;
    if (host.global_object(kInterfaceGlobal).p != nullptr || s.at<std::uint8_t>(0x8C) != 0) {
        const std::int32_t slot = game.at<std::int32_t>(0x18EC);
        if (slot < 0 || slot > 7) {
            host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1C) = 1;
            return;
        }
        Raw rec{static_cast<std::uint8_t*>(game.ptr(0x18CC + slot * 4))};
        channel = (rec.at<std::uint32_t>(0x28) != 0 ? 1 : 0) + 9;
    }
    Raw rec{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4))};
    Raw name{game.p + (game.at<std::uint32_t>(0x6A0) != 0 ? 0x6A0 : 0x68C)};
    host.session_send_chat_00772170(name, channel, 0, Raw{s.p + 0x20}, host.native_string_temp(""),
                                    rec.at<std::int32_t>(0x80), rec.at<std::int32_t>(0x84));
    host.native_string_truncate(Raw{s.p + 0x20}, 0);
    s.at<std::uint32_t>(0x28) = 0;
    multi_chat_refresh_mode_label_005d07e0(s, host);
}

void multi_chat_set_team_only_005d1000(Raw s, bool team_only, Seg15Host& host) {
    s.at<std::uint8_t>(0x8C) = team_only ? 1 : 0;
    multi_chat_refresh_mode_label_005d07e0(s, host);
    host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1A) = 1;
}

// 005D1020: an input longer than 3Ch characters is cut to 3Ch and the cursor
// clamped to it; the label is refreshed in every case.
void multi_chat_clamp_input_length_005d1020(Raw listener, Seg15Host& host) {
    Raw screen{listener.p - 8};
    if (screen.at<std::uint32_t>(0x20) > 0x3C) {
        host.native_string_truncate(Raw{screen.p + 0x20}, 0x3C);
        if (screen.at<std::int32_t>(0x28) >= 0x3D) screen.at<std::int32_t>(0x28) = 0x3C;
    }
    multi_chat_refresh_mode_label_005d07e0(screen, host);
}

// 005D10F0: a non-empty input (case-insensitive compare against "") is sent
// (tail jump to 005D0B10 with ECX = screen).
void multi_chat_submit_input_005d10f0(Raw listener, Seg15Host& host) {
    Raw screen{listener.p - 8};
    if (screen.at<std::uint32_t>(0x20) != 0) multi_chat_send_current_line_005d0b10(screen, host);
}

void multi_chat_on_open_request_005d14b0(Raw listener, Seg15Host& host) {
    multi_chat_set_chat_mode_005d11f0(Raw{listener.p - 8},
                                      host.global_object(kInterfaceGlobal).p != nullptr ? 3 : 1, host);
}

// 005D11F0: entering or leaving mode 4 parks / unparks the screens 28h, 55h,
// 57h, 5Dh, 15h (004D6480). The three list widgets follow the mode (+44h mode
// 4, +48h mode 3, +4Ch modes 1 and 2); modes 3 and 4 set +8Dh; modes 2 and 4
// open the text editor (+64h = 1, closing the multiplayer pause interface when
// it is up in a mission), refresh the label and show the input (+60h);
// otherwise the editor closes and +60h is hidden.
void multi_chat_set_chat_mode_005d11f0(Raw s, std::int32_t mode, Seg15Host& host) {
    const std::int32_t old = s.at<std::int32_t>(0x40);
    if (old == mode) return;
    static constexpr std::int32_t kParked[5] = {0x28, 0x55, 0x57, 0x5D, 0x15};
    if (old == 4) host.gui_park_screens_004d6480(kParked, 5, false);
    else if (mode == 4) host.gui_park_screens_004d6480(kParked, 5, true);
    s.at<std::int32_t>(0x40) = mode;
    host.widget_set_visible_34(s.ptr(0x44), mode == 4);
    host.widget_set_visible_34(s.ptr(0x48), mode == 3);
    host.widget_set_visible_34(s.ptr(0x4C), mode == 2 || mode == 1);
    if (mode == 4 || mode == 3) s.at<std::uint8_t>(0x8D) = 1;
    if (mode == 2 || mode == 4) {
        s.at<std::uint8_t>(0x64) = 1;
        if (host.global_object(kInterfaceGlobal).p != nullptr
            && host.global_object(kMultiMenuGlobal).at<std::uint8_t>(0x3C) != 0) {
            host.multi_menu_close_pause_006882a0();
        }
        host.text_editor_set_active_00a966e0(Raw{s.p + 0x8}, true, s.ptr(0x5C));
        multi_chat_refresh_mode_label_005d07e0(s, host);
        host.widget_set_visible_34(s.ptr(0x60), true);
    } else {
        s.at<std::uint8_t>(0x64) = 0;
        host.text_editor_set_active_00a966e0(Raw{s.p + 0x8}, false, s.ptr(0x5C));
        host.widget_set_visible_34(s.ptr(0x60), false);
    }
}

// ---------------------------------------------------------------------------
// Multiplayer error screen (1Dh)
// ---------------------------------------------------------------------------

// 005D1F90, slot +1Ch: code +54h = 0, join detail 00E08898 = 1.
void multi_error_exit_005d1f90(Raw s, Seg15Host& host) {
    s.at<std::int32_t>(0x54) = 0;
    host.global_block(0x00E08898).at<std::int32_t>(0) = 1;
}

void listener_focus_text_dim_005d1fb0(void* widget, bool flag, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) return;
    host.widget_set_float_4c(widget, flag ? 0.5f : 1.0f);
}

// 005D2010: pop every error interface (1Bh) off the multiplayer menu; unless
// the online service is signed in with a selected user in sign-in state 2,
// request state 10h (in a mission), pop up to interface 0Fh and request
// states 9 then 4.
void multi_error_dismiss_005d2010(Seg15Host& host) {
    while (host.global_object(kMultiMenuGlobal).at<std::int32_t>(0x20) == 0x1B) host.multi_menu_pop_request_006881f0();
    if (host.online_service_virtual(0x14) && host.xenon_has_selected_user_00a3e510()
        && host.xenon_sign_in_state_00a3ead0() == 2) {
        return;
    }
    if (host.global_object(kInterfaceGlobal).p != nullptr) host.game_request_state_004d7920(0x10);
    host.multi_menu_pop_up_to_00688b20(0xF);
    host.game_request_state_004d7920(9);
    host.game_request_state_004d7920(4);
}

namespace {
void error_leave(Seg15Host& host) {
    multi_error_dismiss_005d2010(host);
    host.session_set_mode_0076fad0(0);
}
bool marketplace_case(std::int32_t code, Seg15Host& host) {
    return code == 4 && host.global_block(0x00E08898).at<std::int32_t>(0) == 4;
}
}  // namespace

// 005D2240: back (4Bh) leaves; in buttons mode 1 accept (4Ah) opens the
// marketplace for the DLC failure (code 4, detail 4) or leaves; the three
// buttons are updated with the frame time.
void multi_error_update_005d2240(Raw s, float seconds, Seg15Host& host) {
    if (host.action_edge_or_repeat_004d92b0(0x4B)) error_leave(host);
    if (s.at<std::int32_t>(0x50) == 1 && host.action_edge_or_repeat_004d92b0(0x4A)) {
        if (marketplace_case(s.at<std::int32_t>(0x54), host)) {
            host.x_show_marketplace_ui_00a4d446(host.xenon_active_user_slot_00a3eac0());
        } else {
            error_leave(host);
        }
    }
    for (std::size_t off : {0x20u, 0x24u, 0x28u}) host.widget_update_40(s.ptr(off), seconds);
}

// 005D2310, widget listener at screen+0Ch, RET 4: a type-3 widget gets alpha
// 1.0; its name starting A2h opens the marketplace in the DLC case or leaves,
// A3h leaves.
void multi_error_on_activate_005d2310(Raw listener, void* widget, Seg15Host& host) {
    if (host.widget_type_5c(widget) != 3) return;
    host.widget_set_float_4c(widget, 1.0f);
    const auto* name = *reinterpret_cast<const std::uint8_t* const*>(static_cast<std::uint8_t*>(widget) + 0xF0);
    const std::uint8_t first = name ? name[0] : 0;  // 00E19538 is ""
    Raw screen{listener.p - 0xC};
    if (first == 0xA2) {
        if (marketplace_case(screen.at<std::int32_t>(0x54), host)) {
            host.x_show_marketplace_ui_00a4d446(host.xenon_active_user_slot_00a3eac0());
            return;
        }
        error_leave(host);
    } else if (first == 0xA3) {
        error_leave(host);
    }
}

// 005D23E0, listbox listener at screen+8h, RET 8: button +20h opens the
// marketplace in the DLC case; +24h / +28h leave.
void multi_error_on_button_005d23e0(Raw listener, void* button, Seg15Host& host) {
    Raw screen{listener.p - 8};
    if (button == screen.ptr(0x20)) {
        if (marketplace_case(screen.at<std::int32_t>(0x54), host)) {
            host.x_show_marketplace_ui_00a4d446(host.xenon_active_user_slot_00a3eac0());
        }
    } else if (button == screen.ptr(0x24) || button == screen.ptr(0x28)) {
        error_leave(host);
    }
}

// 005D2440: page _MultiError into +10h, code +54h = 0.
void multi_error_register_005d2440(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x10) = host.gui_load_page_00aa5840("_MultiError");
    s.at<std::int32_t>(0x54) = 0;
}

// 005D24F0: the message (+14h) and the second line (+18h) by code; codes 1, 6
// and 7 also drop the session mode to 0. Any other code only re-lays out.
void multi_error_show_message_005d24f0(Raw s, Seg15Host& host) {
    void* text = s.ptr(0x14);
    void* line2 = s.ptr(0x18);
    const char* key = nullptr;
    switch (s.at<std::int32_t>(0x54)) {
        case 1: host.session_set_mode_0076fad0(0); key = "FE_pc.multi_liveerror"; break;
        case 6: host.session_set_mode_0076fad0(0); key = "FE_pc.multi_liveerrorduplicate"; break;
        case 7:
            host.session_set_mode_0076fad0(0);
            key = host.global_object(kGameGlobal).at<std::int32_t>(0x1FE4) == 1 ? "FE.multi_terminated"
                                                                                : "ingame.multi_gameclosed_xbox";
            break;
        case 2: key = "FE.multi_conlost"; break;
        case 3: key = "ingame.multi_kicked"; break;
        case 5: key = "FE.multi_cablelost_live"; break;
        case 4: {
            const std::int32_t detail = host.global_block(0x00E08898).at<std::int32_t>(0);
            if (detail == 2) key = "FE.multi_connerr_full";
            else if (detail == 3) key = "FE.multi_connerr_publicfull";
            else if (detail == 5) key = "FE.multi_connerr_ranknotfit";
            else if (detail == 4) {
                // "" (0), the DLC key, then the text's wide string (+ECh) with the
                // wide string at 00E1953C appended, submitted back.
                host.text_set_localised_00abbe50(text, "");
                host.text_set_localised_00abbe50(text, "FE.multi_connerr_dlcfail|.");
                std::uint8_t joined[8] = {};
                wide_string_concat_005cf570(Raw{static_cast<std::uint8_t*>(text) + 0xEC}, Raw{joined},
                                            host.global_block(0x00E1953C), host);
                host.text_submit_utf16_00ab6ab0(text, Raw{joined});
                host.wide_string_release(Raw{joined});
                host.text_set_localised_00abbe50(line2, "");
                host.multi_error_layout_buttons_005d20a0(s);
                return;
            } else if (!host.online_service_virtual(0x198)) {
                key = "FE.multi_connerr";
            } else {
                host.text_set_localised_00abbe50(text, "");
                host.text_set_localised_00abbe50(line2, "");  // flag 0 in the image
                host.text_set_localised_00abbe50(line2, "FE.multi_sessoff");
                host.multi_error_layout_buttons_005d20a0(s);
                return;
            }
            break;
        }
        default:
            host.multi_error_layout_buttons_005d20a0(s);
            return;
    }
    host.text_set_localised_00abbe50(text, key);
    host.text_set_localised_00abbe50(line2, "");
    host.multi_error_layout_buttons_005d20a0(s);
}

// 005D2E70: a signed-out online service forces code 5; code 1 clears +58h; an
// active screen re-shows the message.
void multi_error_set_error_code_005d2e70(Raw s, std::int32_t code, Seg15Host& host) {
    const std::int32_t applied = host.online_service_virtual(0x14) ? code : 5;
    s.at<std::int32_t>(0x54) = applied;
    if (applied == 1) s.at<std::uint32_t>(0x58) = 0;
    if (s.at<std::uint8_t>(kActive) != 0) multi_error_show_message_005d24f0(s, host);
}

// 005D2EB0: vtables, page +10h, the two string vectors +30h, listener base, base.
void multi_error_destroy_005d2eb0(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CF1BF4;
    s.at<std::uint32_t>(0x8) = 0x00CF1BE0;
    s.at<std::uint32_t>(0xC) = 0x00CF1BC0;
    remove_page(s, 0x10, host);
    host.eh_vector_destruct_00bf7c6e(s.p + 0x30, 0x10, 2, 0x004324A0);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

void multi_error_collect_children_005d2f60(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x10));
}

// ---------------------------------------------------------------------------
// Multiplayer pause screen (19h)
// ---------------------------------------------------------------------------

void prompt_result_leave_multi_menu_005d3410(std::int32_t result, Seg15Host& host) {
    if (result != 1) return;
    host.multi_menu_pop_request_006881f0();
    host.game_end_scene_004d7970(1);
}

void prompt_result_set_quit_flag_005d3430(std::int32_t result, Seg15Host& host) {
    if (result == 1) host.global_block(0x00E1AE75).at<std::uint8_t>(0) = 1;
}

namespace {
void typed_dim(void* widget, bool flag, Seg15Host& host) {
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_float_4c(widget, flag ? 0.5f : 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 6) {
        host.widget_virtual_88(widget, flag ? 1 : 0, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_virtual_84(widget, flag ? 1 : 0);
}
}  // namespace

void listener_focus_dim_typed_005d3440(void* widget, bool flag, Seg15Host& host) { typed_dim(widget, flag, host); }

void listener_refresh_dim_typed_005d35a0(void* widget, Seg15Host& host) {
    typed_dim(widget, static_cast<std::uint8_t*>(widget)[0xD4] != 0, host);
}

void pause_multi_store_selection_005d34e0(void* listbox, Seg15Host& host) {
    host.global_block(0x00E19550).at<std::int32_t>(0) = host.listbox_selected_index_00a9c920(listbox);
}

// 005D3540: base, listener bases, vtables 00CF1D70 / 00CF1D5C / 00CF1D3C, +24h cleared.
Raw pause_multi_construct_005d3540(Raw s, Seg15Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0xC) = kWidgetListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = 0x00CF1D70;
    s.at<std::uint32_t>(0x8) = 0x00CF1D5C;
    s.at<std::uint32_t>(0xC) = 0x00CF1D3C;
    s.at<std::uint32_t>(0x24) = 0;
    return s;
}

// 005D3710: vtables, page +10h, stored selection 00E19550 = 0, listener base, base.
void pause_multi_destroy_005d3710(Raw s, Seg15Host& host) {
    s.at<std::uint32_t>(0x0) = 0x00CF1D70;
    s.at<std::uint32_t>(0x8) = 0x00CF1D5C;
    s.at<std::uint32_t>(0xC) = 0x00CF1D3C;
    remove_page(s, 0x10, host);
    host.global_block(0x00E19550).at<std::int32_t>(0) = 0;
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

// 005D3790: a type-3 widget gets alpha 1.0; a name starting A2h activates the
// listbox selection through the listbox listener (+8h) slot +4h with
// 00425E50(listbox +20h); A3h resumes (toggle pause); B7h and B8h do nothing.
// Types 6 and 12h get +88h(2, 0, 1.0f) / +84h(2).
void pause_multi_on_activate_005d3790(Raw listener, void* widget, Seg15Host& host) {
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_float_4c(widget, 1.0f);
        const auto* name = *reinterpret_cast<const std::uint8_t* const*>(static_cast<std::uint8_t*>(widget) + 0xF0);
        const std::uint8_t first = name ? name[0] : 0;  // 00E19554 is ""
        if (first == 0xA2) {
            Raw listbox_listener{listener.p - 4};
            host.object_virtual_arg(listbox_listener, 0x4, host.call_00425e50(listener.ptr(0x14)));
        } else if (first == 0xA3) {  // the B7h / B8h tests branch to the same exit
            host.game_toggle_pause_menu_004db030();
        }
        return;
    }
    if (host.widget_type_5c(widget) == 6) {
        host.widget_virtual_88(widget, 2, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_virtual_84(widget, 2);
}

// 005D38A0: page GUI_pause_multi into +10h.
void pause_multi_register_005d38a0(Raw s, Seg15Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x10) = host.gui_load_page_00aa5840("GUI_pause_multi");
}

// 005D3960: +1Ch = 1; when the pause screen (*(interface + D8h)) has +10h set,
// show its page (0, 1); then select the default frame (tail jump 004C9680).
void pause_multi_exit_005d3960(Raw s, Seg15Host& host) {
    s.at<std::uint8_t>(0x1C) = 1;
    Raw pause{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0xD8))};
    if (pause.at<std::uint8_t>(0x10) != 0) host.pause_screen_show_page_005fd890(0, 1);
    host.front_end_frame_select_default_004c9680();
}

void pause_multi_collect_children_005d3990(Raw s, void* out, Seg15Host& host) {
    host.push_page_slot_004d6790(out, reinterpret_cast<void**>(s.p + 0x10));
}

// ---------------------------------------------------------------------------
// Multiplayer game lobby helpers
// ---------------------------------------------------------------------------

bool lobby_slot_swap_has_pair_005d4910(Raw lobby) {
    return lobby.at<std::int32_t>(0x2F0) != -1 && lobby.at<std::int32_t>(0x2F4) != -1;
}

void lobby_settings_copy_stored_options_005d49e0(const std::uint32_t* src, Seg15Host& host) {
    Raw stored = host.global_block(0x00E1955C);
    for (std::size_t i = 0; i < 14; ++i) stored.at<std::uint32_t>(i * 4) = src[i];
}

// Four unconditional mode reads, then mode0 == 0 || mode1 == 1 || mode2 == 2 || mode3 == 3.
bool game_is_standard_game_mode_005d4a10(Seg15Host& host) {
    const bool a = host.game_effective_mode_004bca50() == 0;
    const bool b = host.game_effective_mode_004bca50() == 1;
    const bool c = host.game_effective_mode_004bca50() == 2;
    const bool d = host.game_effective_mode_004bca50() == 3;
    return a || b || c || d;
}

bool session_record_flag_274_005d50e0(Raw record) { return record.at<std::uint8_t>(0x274) != 0; }

void* game_local_slot_record_005d5130(Raw game) {
    const std::int32_t slot = game.at<std::int32_t>(0x18EC);
    return (slot >= 0 && slot < 8) ? game.ptr(0x18CC + slot * 4) : nullptr;
}

bool multi_lobby_local_slot_valid_005d51f0(Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    const std::int32_t slot = game.at<std::int32_t>(0x18EC);
    return game.at<std::int32_t>(0x18C8) >= 0 && slot >= 0 && slot < 8;
}

bool multi_lobby_local_player_ready_005d5220(Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    if (!multi_lobby_local_slot_valid_005d51f0(host)) return false;
    return Raw{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4))}.at<std::uint8_t>(0xC) != 0;
}

// 005D5260: every present human slot (+750h set, +751h clear) must be ready
// (+754h); then with two or more such players, or the flag game+218Ch, mode 7
// may start and any other mode needs participants on both sides (004BB770).
bool lobby_ready_to_start_005d5260(Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    std::int32_t humans = 0;
    for (std::int32_t i = 0; i < 8; ++i) {
        Raw slot{game.p + 0x750 + i * 0x118};
        if (slot.at<std::uint8_t>(0) != 0 && slot.at<std::uint8_t>(1) == 0) {
            ++humans;
            if (slot.at<std::uint8_t>(4) == 0) return false;
        }
    }
    if (humans < 2 && game.at<std::uint8_t>(0x218C) == 0) return false;
    if (host.game_effective_mode_004bca50() == 7) return true;
    std::int32_t a = 0, b = 0;
    host.game_count_participants_by_side_004bb770(&a, &b);
    return a > 0 && b > 0;
}

// 005D5490: the settings panel (+18h) visible and the settings list (+228h) byte +85h.
bool multi_game_lobby_settings_open_005d5490(Raw s, Seg15Host& host) {
    return host.widget_is_visible_38(s.ptr(0x18))
        && Raw{static_cast<std::uint8_t*>(s.ptr(0x228))}.at<std::uint8_t>(0x85) != 0;
}

std::int32_t lobby_settings_slot_for_menu_row_005d5630(std::int32_t row, Seg15Host& host) {
    for (std::int32_t slot = 0; slot < 0xE; ++slot) {
        if (host.lobby_settings_menu_row_005d5500(slot) == row) return slot;
    }
    return -1;
}

void multi_mode_selector_set_mode_005d5660(std::int32_t mode, Seg15Host& host) {
    const std::int32_t applied = (mode >= 0 && mode < 4) ? host.global_block(0x00E19568).at<std::int32_t>(0) : mode;
    host.global_block(0x00E19564).at<std::int32_t>(0) = mode;
    host.game_set_game_mode_004bc890(applied, 0);
}

// 005D5690: kick every present slot whose name (+58h) matches the target name
// (native string 00E195CC / data 00E195D0) case-insensitively; a null target
// matches an empty name.
void prompt_result_kick_player_005d5690(std::int32_t result, Seg15Host& host) {
    if (result != 1) return;
    Raw game = host.global_object(kGameGlobal);
    const char* target = static_cast<const char*>(host.global_block(0x00E195D0).ptr(0));
    for (std::int32_t i = 0; i < 8; ++i) {
        Raw slot{game.p + 0x748 + i * 0x118};
        if (slot.at<std::uint8_t>(8) == 0) continue;
        const char* name = reinterpret_cast<const char*>(slot.p + 0x58);
        const bool match = target == nullptr ? name[0] == 0 : _stricmp(target, name) == 0;
        if (match) host.session_kick_player_peer_0076c790(slot);
    }
}

// 005D57F0: accept (00E0895C, 00E08960) and leave the tactical library when it
// is on the multiplayer menu stack (interface 0Bh); otherwise refuse. The pending
// swap 00E0895C is reset to -1 on every path.
void prompt_result_accept_slot_swap_005d57f0(std::int32_t result, Seg15Host& host) {
    Raw pending = host.global_block(0x00E0895C);
    const std::int32_t a = pending.at<std::int32_t>(0);
    const std::int32_t b = host.global_block(0x00E08960).at<std::int32_t>(0);
    if (result == 1) {
        host.session_swap_slot_accept_00771ac0(a, b);
        if (host.global_object(kMainFrontEndGlobal).p != nullptr && host.global_object(kMultiMenuGlobal).p != nullptr
            && host.multi_menu_has_interface_00687860(0xB)) {
            host.tactical_library_leave_00515bc0();
        }
    } else {
        host.session_swap_slot_refuse_00771cc0(a, b);
    }
    pending.at<std::int32_t>(0) = -1;
}

// 005D5790: +281h = the local player's ready byte; the client (session mode 2) sets +280h.
void multi_game_lobby_refresh_local_ready_005d5790(Raw s, Seg15Host& host) {
    s.at<std::uint8_t>(0x281) = multi_lobby_local_player_ready_005d5220(host) ? 1 : 0;
    if (host.global_object(kGameGlobal).at<std::int32_t>(0x1FE4) == 2) s.at<std::uint8_t>(0x280) = 1;
}

// 005D5870: editable only for the host (session mode 1): the settings panel
// (+18h) shown and the settings list (+228h) focused; when editable the two
// buttons +94h / +A8h lose focus, the stored options are copied to +284h..,
// +2BCh = game+60Ch, the swap pair is saved (+2F8h, +2FCh) and the scroller
// (+30h) is reset (range 0.0, no content).
void multi_game_lobby_set_editable_005d5870(Raw s, bool editable, Seg15Host& host) {
    if (host.global_object(kGameGlobal).at<std::int32_t>(0x1FE4) != 1) editable = false;
    host.widget_set_visible_34(s.ptr(0x18), editable);
    host.widget_set_focus_60(s.ptr(0x228), editable);
    if (!editable) return;
    host.widget_set_focus_60(s.ptr(0x94), false);
    host.widget_set_focus_60(s.ptr(0xA8), false);
    Raw stored = host.global_block(0x00E1955C);
    for (std::size_t i = 0; i < 14; ++i) s.at<std::uint32_t>(0x284 + i * 4) = stored.at<std::uint32_t>(i * 4);
    s.at<std::uint32_t>(0x2BC) = host.global_object(kGameGlobal).at<std::uint32_t>(0x60C);
    s.at<std::uint32_t>(0x2F8) = s.at<std::uint32_t>(0x2F0);
    s.at<std::uint32_t>(0x2FC) = s.at<std::uint32_t>(0x2F4);
    host.scroller_set_range_006834a0(Raw{s.p + 0x30}, 0.0f);
    host.scroller_attach_content_00683790(Raw{s.p + 0x30}, nullptr);
}

std::int32_t multi_game_lobby_find_setting_row_005d5930(Raw s, std::int32_t id) {
    auto* first = static_cast<std::uint8_t*>(s.ptr(0x2E4));
    if (first == nullptr) return 0;
    const auto count = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(s.ptr(0x2E8)) - first) / 0x24);
    for (std::uint32_t i = 0; i < count; ++i) {
        Raw row{first + i * 0x24};
        if (row.at<std::int32_t>(0) != id) continue;
        auto* begin = static_cast<std::uint8_t*>(row.ptr(8));
        return begin ? static_cast<std::int32_t>((static_cast<std::uint8_t*>(row.ptr(0xC)) - begin) >> 3) : 0;
    }
    return 0;
}

// 005D62B0: mode 00E19564 (0..3 through 00E19568) applied; the settings copied
// to game+2020h; broadcasts of the settings, the scene index and, with a scene
// (game+5FCh), its sub-scene (00E195C4 with scene +1098h).
void multi_game_lobby_apply_settings_005d62b0(Seg15Host& host) {
    std::int32_t mode = host.global_block(0x00E19564).at<std::int32_t>(0);
    if (static_cast<std::uint32_t>(mode) < 4) mode = host.global_block(0x00E19568).at<std::int32_t>(0);
    host.game_set_game_mode_004bc890(mode, 0);
    Raw game = host.global_object(kGameGlobal);
    Raw stored = host.global_block(0x00E1955C);
    for (std::size_t i = 0; i < 14; ++i) game.at<std::uint32_t>(0x2020 + i * 4) = stored.at<std::uint32_t>(i * 4);
    host.session_broadcast_multi_settings_0076c9c0(stored);
    host.session_broadcast_scene_index_00772390();
    if (game.ptr(0x5FC) != nullptr) {
        host.session_broadcast_sub_scene_00772430(host.global_block(0x00E195C4),
                                                  Raw{static_cast<std::uint8_t*>(game.ptr(0x5FC))}.at<std::uint32_t>(0x1098));
    }
}

// 005D6350: the settings slot shown on menu row 1 takes the value (with no
// such slot the index is -1 and the image writes 00E19558); +10h = 2.
void multi_game_lobby_set_setting_for_row1_005d6350(Raw s, std::uint32_t value, Seg15Host& host) {
    const std::int32_t slot = lobby_settings_slot_for_menu_row_005d5630(1, host);
    host.global_block(static_cast<std::uint32_t>(0x00E1955C + slot * 4)).at<std::uint32_t>(0) = value;
    s.at<std::int32_t>(0x10) = 2;
}

void* multi_game_lobby_selected_player_slot_005d63a0(Raw s, Seg15Host& host) {
    std::int32_t index = host.listbox_selected_index_00a9c920(s.ptr(0xA8));
    Raw game = host.global_object(kGameGlobal);
    for (std::int32_t i = 0; i < 8; ++i) {
        std::uint8_t* slot = game.p + 0x748 + i * 0x118;
        if (slot[8] == 0) continue;
        if (index == 0) return slot;
        --index;
    }
    return nullptr;
}

// 005D6BA0: unless the profile-hints owner disables the check (+8h), every
// present human slot's content words (+C7h / +CBh past its +751h byte) must contain the
// required masks game+2060h / +2064h.
bool multi_game_lobby_slots_meet_content_masks_005d6ba0(Seg15Host& host) {
    if (host.profile_hints_owner_004c1e90().at<std::uint32_t>(8) != 0) return true;
    Raw game = host.global_object(kGameGlobal);
    const std::uint32_t need_a = game.at<std::uint32_t>(0x2060);
    const std::uint32_t need_b = game.at<std::uint32_t>(0x2064);
    for (std::int32_t i = 0; i < 8; ++i) {
        Raw slot{game.p + 0x751 + i * 0x118};
        if (Raw{slot.p - 1}.at<std::uint8_t>(0) == 0 || slot.at<std::uint8_t>(0) != 0) continue;
        if ((slot.at<std::uint32_t>(199) & need_a) != need_a || (slot.at<std::uint32_t>(0xCB) & need_b) != need_b) {
            return false;
        }
    }
    return true;
}

// 005D6CD0: pending load prompt 00E08958 = -1, dismiss prompt slot 1.
void session_dismiss_load_prompt_005d6cd0(Seg15Host& host) {
    host.global_block(0x00E08958).at<std::int32_t>(0) = -1;
    host.menu_prompt_dismiss_slot_00532a20(1);
}

bool multi_game_lobby_local_map_available_005d8b30(Seg15Host& host) {
    Raw game = host.global_object(kGameGlobal);
    Raw scene{static_cast<std::uint8_t*>(game.ptr(0x5FC))};
    if (scene.p == nullptr || scene.at<std::uint8_t>(0x914) == 0) return false;
    Raw rec{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4))};
    return host.tactical_library_map_unlocked_0050fd10(host.lobby_map_index_005d8650(rec.at<std::uint32_t>(0x28)));
}

namespace {
Raw lobby_screen_from_multi_menu(Seg15Host& host) {
    return Raw{static_cast<std::uint8_t*>(host.global_object(kMultiMenuGlobal).ptr(0x48))};
}
}  // namespace

// 005D98C0: tail jump to 005D8410 (leave to menu).
void prompt_result_leave_lobby_005d98c0(std::int32_t result, Seg15Host& host) {
    if (result == 1) host.lobby_call(0x005D8410, lobby_screen_from_multi_menu(host));
}

void prompt_result_close_game_005d9fe0(std::int32_t result, Seg15Host& host) {
    if (result == 1) multi_game_lobby_close_game_005d9ef0(lobby_screen_from_multi_menu(host), host);
}

// 005D9D60: with a local slot 0..7: map icon 005D6C10, map availability
// 005D98E0, then (tail jump) the slot reset 005D5340.
void multi_game_lobby_on_local_slot_changed_005d9d60(Raw s, Seg15Host& host) {
    const std::int32_t slot = host.global_object(kGameGlobal).at<std::int32_t>(0x18EC);
    if (slot < 0 || slot >= 8) return;
    host.lobby_call(0x005D6C10, s);
    host.lobby_call(0x005D98E0, s);
    host.lobby_call(0x005D5340, s);
}

// 005D9EF0: broadcast game closed (0); if the online service is hosting
// (+180h), end it (+170h(0)), +270h = 1 and 00E08700 = 0; otherwise leave to
// the menu (tail jump 005D8410).
void multi_game_lobby_close_game_005d9ef0(Raw s, Seg15Host& host) {
    host.session_broadcast_game_closed_007728b0(0);
    if (host.online_service_virtual(0x180)) {
        host.online_service_virtual_arg(0x170, 0);
        s.at<std::uint8_t>(0x270) = 1;
        host.global_block(0x00E08700).at<std::uint8_t>(0) = 0;
        return;
    }
    host.lobby_call(0x005D8410, s);
}

// 005D9F50: a selected slot other than the local record goes to the kick
// confirmation (tail jump 005D8530 with ECX = the slot).
void multi_game_lobby_request_selected_slot_005d9f50(Raw s, Seg15Host& host) {
    void* selected = multi_game_lobby_selected_player_slot_005d63a0(s, host);
    if (selected == nullptr) return;
    Raw game = host.global_object(kGameGlobal);
    if (selected != game_local_slot_record_005d5130(game)) host.lobby_call(0x005D8530, Raw{static_cast<std::uint8_t*>(selected)});
}

// 005D9F90: with the local map available, the scene loader (*(*00E198AC + 70h))
// takes the map index (+9Ch), the map (+A0h) and +A4h = 1, and the tactical
// library (interface 0Bh) is pushed.
void multi_game_lobby_apply_local_map_005d9f90(Seg15Host& host) {
    if (!multi_game_lobby_local_map_available_005d8b30(host)) return;
    Raw game = host.global_object(kGameGlobal);
    Raw rec{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4))};
    const std::uint32_t map = rec.at<std::uint32_t>(0x28);
    const std::int32_t index = host.lobby_map_index_005d8650(map);
    Raw loader{static_cast<std::uint8_t*>(host.global_object(kMainFrontEndGlobal).ptr(0x70))};
    loader.at<std::int32_t>(0x9C) = index;
    loader.at<std::uint32_t>(0xA0) = map;
    loader.at<std::uint8_t>(0xA4) = 1;
    host.multi_menu_push_request_00688ad0(0xB);
}

void two_vector_record_construct_005dac60(Raw record) {
    for (std::size_t off : {0x8u, 0xCu, 0x10u, 0x18u, 0x1Cu, 0x20u}) record.at<std::uint32_t>(off) = 0;
}

void lobby_map_entry_copy_005daf10(Raw dst, Raw src, Seg15Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    host.checked_string_vector_copy_00557bc0(Raw{dst.p + 4}, Raw{src.p + 4});
    host.checked_string_vector_copy_00557bc0(Raw{dst.p + 0x14}, Raw{src.p + 0x14});
}

// 005D8410: empty help line, +270h = 0, leave to the main multiplayer
// interface, clear the chat logs (*(*00E198B4 + 54h)), clear the lists +134h
// and +B4h with their scrollers (+158h, +CCh: range 0.0, no content); outside
// a dedicated session (game+218Ch clear) 00E19614 = 0. The PUSH EDI before
// 00688C70 is the delayed save of EDI, not an argument.
void multi_game_lobby_leave_to_menu_005d8410(Raw s, Seg15Host& host) {
    host.command_bar_set_help_line_0054a0c0(host.native_string_temp(""));
    s.at<std::uint8_t>(0x270) = 0;
    host.multi_menu_leave_to_main_00688c70();
    multi_chat_clear_logs_005cfb80(Raw{static_cast<std::uint8_t*>(host.global_object(kMultiMenuGlobal).ptr(0x54))}, host);
    host.widget_clear_00aa83f0(s.ptr(0x134));
    host.scroller_set_range_006834a0(Raw{s.p + 0x158}, 0.0f);
    host.scroller_attach_content_00683790(Raw{s.p + 0x158}, nullptr);
    host.widget_clear_00aa83f0(s.ptr(0xB4));
    host.scroller_set_range_006834a0(Raw{s.p + 0xCC}, 0.0f);
    host.scroller_attach_content_00683790(Raw{s.p + 0xCC}, nullptr);
    if (host.global_object(kGameGlobal).at<std::uint8_t>(0x218C) == 0) host.global_block(0x00E19614).at<std::uint32_t>(0) = 0;
}

void multi_game_lobby_delete_row_objects_005d8a70(Raw s, Seg15Host& host) {
    for (std::uint32_t i = 0;; ++i) {
        auto* first = static_cast<std::uint8_t**>(s.ptr(0x234));
        if (first == nullptr || i >= static_cast<std::uint32_t>(static_cast<std::uint8_t**>(s.ptr(0x238)) - first)) break;
        if (first[i] != nullptr) {
            host.object_delete_04(first[i]);
            first[i] = nullptr;
        }
    }
    // erase(begin, end): nothing is moved, end = begin.
    if (s.ptr(0x234) != s.ptr(0x238)) s.ptr(0x238) = s.ptr(0x234);
}

void lobby_map_entry_destruct_005daca0(Raw entry, Seg15Host& host) {
    for (std::size_t vec : {0x14u, 0x4u}) {
        auto* first = static_cast<std::uint8_t*>(entry.ptr(vec + 4));
        if (first != nullptr) {
            for (auto* p = first; p != entry.ptr(vec + 8); p += 8) host.native_string_release(Raw{p});
            host.free_00bf65ac(first);
        }
        entry.at<std::uint32_t>(vec + 4) = 0;
        entry.at<std::uint32_t>(vec + 8) = 0;
        entry.at<std::uint32_t>(vec + 0xC) = 0;
    }
}

void multi_game_lobby_on_player_row_activate_005d8700(Raw s, Seg15Host& host) {
    const std::int32_t index = host.listbox_selected_index_00a9c920(s.ptr(0xA8));
    host.global_block(0x00E08958).at<std::int32_t>(0) = index;
    auto* row = static_cast<std::uint8_t*>(host.gui_find_child_00aa7e00(s.ptr(0x250 + index * 4), "player_name_Text"));
    Raw target = host.global_block(0x00E195D4);
    if (target.p != row + 0xF4) host.native_string_copy_0041dd40(target, Raw{row + 0xF4});
}

// ---------------------------------------------------------------------------
// cc13-fe7: lobby routines (docs/CC13_FE_7.md)
// ---------------------------------------------------------------------------
void multi_game_lobby_reset_slots_005d5340(Seg15Host& host) {
    if (host.online_service_virtual(0x198) && host.global_object(0x00F8A2FC).at<std::uint8_t>(0x4D) != 0) {
        for (std::int32_t slot = 0; slot < 8; ++slot) host.session_slot_ai_enable_00771e80(slot, 0);
        return;
    }
    if (static_cast<std::uint32_t>(host.game_effective_mode_004bca50()) > 3) return;
    const Raw game = host.global_object(0x00E188A8);
    std::uint8_t side_taken[3] = {0, 0, 0};
    for (std::size_t k = 0; k < 8; ++k) {
        const Raw record{game.p + 0x750 + k * 0x118};
        if (record.at<std::uint8_t>(0) != 0 && record.at<std::uint8_t>(1) == 0) {
            side_taken[record.at<std::int32_t>(0x20)] = 1;
        }
    }
    for (std::int32_t slot = 0; slot < 8; ++slot) {
        const Raw record{static_cast<std::uint8_t*>(host.global_object(0x00E188A8).ptr(0x18CC + slot * 4))};
        const std::int32_t side = record.at<std::int32_t>(0x28);
        if (side_taken[side] != 0) {
            host.session_slot_ai_enable_00771e80(slot, 0);
        } else {
            host.session_slot_ai_enable_00771e80(slot, 1);
            side_taken[record.at<std::int32_t>(0x28)] = 1;
        }
    }
}

void multi_game_lobby_set_focus_005d67e0(Raw s, void* widget, Seg15Host& host) {
    if (s.ptr(0x1C) == widget) return;
    if (s.ptr(0x1C) == s.ptr(0xA4)) s.at<std::int32_t>(0xAC) = host.listbox_selected_index_00a9c920(s.ptr(0xA8));
    host.input_manager_update_00a92c40(0.0f);
    if (s.ptr(0x1C) != nullptr) host.widget_virtual_84(s.ptr(0x1C), 0);
    s.ptr(0x1C) = widget;
    if (widget != nullptr) host.widget_virtual_84(widget, 1);
    void* const focus = s.ptr(0x1C);
    s.at<std::uint8_t>(0x1B1) = focus == s.ptr(0x150);
    s.at<std::uint8_t>(0x125) = focus == s.ptr(0xC4);
    s.at<std::uint8_t>(0x89) = focus != s.ptr(0x150) && focus != s.ptr(0xC4);
    static_cast<std::uint8_t*>(s.ptr(0xA4))[0x78] = focus != s.ptr(0xA4);
    if (static_cast<const std::uint8_t*>(s.ptr(0x228))[0x85] != 0) return;
    if (s.ptr(0x1C) == s.ptr(0xA4)) {
        void* const players = s.ptr(0xA8);
        host.listbox_select_index_00a9c7c0(players, s.at<std::int32_t>(0xAC));
        s.at<float>(0x2D0) = static_cast<float>(s.at<float>(0x2D0) + 100.0);  // 00D7A220
        host.listbox_clear_other_two_00a9cdf0(players, s.ptr(0x94), s.ptr(0xA8));
    } else {
        host.listbox_clear_00a9cdc0(s.ptr(0x94));
        host.listbox_clear_00a9cdc0(s.ptr(0xA8));
    }
}

namespace {
// Shared by 005D6C10 and 005D98E0: a texture handle (one reference) and its
// uv rectangle (0, 0, 1, 1 before the call).
struct LobbyIcon {
    void* handle = nullptr;
    float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};  // 00D7A24C = 1.0
};
}  // namespace

void multi_game_lobby_refresh_map_icon_005d6c10(Raw s, Seg15Host& host) {
    LobbyIcon icon;
    host.tactical_library_map_icon_004fd500(Raw{static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x70))},
                                            &icon.handle, icon.uv);
    if (icon.handle != nullptr) host.widget_set_state_texture_00ab2690(s.ptr(0x2C), 0, icon.handle, icon.uv);
    if (icon.handle != nullptr) host.refcounted_release(icon.handle);
}

void multi_game_lobby_confirm_kick_005d8530(Raw record, Seg15Host& host) {
    if (record.p == nullptr) return;
    const char* const name = reinterpret_cast<const char*>(record.p + 0x58);
    host.native_string_assign_cstr(host.global_block(0x00E195CC), name);
    host.game_participant_side_has_room_004bb850(record);  // result unused
    host.menu_prompt_raise_00531b00(1, "FE.multi_lobby_kickplayerconfirm", 1, 0x005D5690, 0,  // 00CF2160
                                    host.global_block(0x00E195CC), 0.0f, 0, "", 1);
    host.menu_prompt_focus_last_00530c20();
}

void multi_game_lobby_prompt_swap_slot_005d8800(std::int32_t requester, std::int32_t target, Seg15Host& host) {
    const Raw game = host.global_object(0x00E188A8);
    if (host.global_block(0x00E0895C).at<std::int32_t>(0) != -1 || target != game.at<std::int32_t>(0x18EC)) {
        host.session_swap_slot_refuse_00771cc0(requester, target);
        return;
    }
    host.global_block(0x00E0895C).at<std::int32_t>(0) = requester;
    host.global_block(0x00E08960).at<std::int32_t>(0) = target;
    const char* const name = reinterpret_cast<const char*>(host.global_object(0x00E188A8).p + 0x748 + requester * 0x118 + 0x58);
    host.menu_prompt_raise_00531b00(1, "FE.multi_lobby_wantswapslot", 1, 0x005D57F0, 0,  // 00CF2184
                                    host.native_string_temp(name), 0.0f, 0, "", 1);
    host.menu_prompt_focus_first_00530670();
}

void multi_game_lobby_show_swap_refused_005d8950(std::int32_t slot, Seg15Host& host) {
    if (host.global_block(0x00E08958).at<std::int32_t>(0) == -1) return;
    host.menu_prompt_raise_00531b00(1, "FE.multi_lobby_swapslotrefused", 2, 0, 0,  // 00CF21A0
                                    host.native_string_temp(""), 0.0f, 0, "", 1);
    host.global_block(0x00E08958).at<std::int32_t>(0) = -1;
    host.global_block(0x00E1959C).at<float>(static_cast<std::size_t>(slot) * 4) = static_cast<float>(
        host.global_object(0x00E188A8).at<float>(0x64C) + kLobbySwapRetrySeconds);
}

void multi_game_lobby_refresh_map_availability_005d98e0(Raw s, Seg15Host& host) {
    const bool available = multi_game_lobby_local_map_available_005d8b30(host);
    s.at<std::uint8_t>(0x2DC) = available ? 1 : 0;
    const Raw library{static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x70))};
    host.tactical_library_set_maps_available_004fbe90(library, available);
    if (s.at<std::uint8_t>(0x2DC) == 0) {
        host.widget_virtual_88(s.ptr(0x24), 1, 0, 1.0f);
        return;
    }
    LobbyIcon icon;
    host.tactical_library_available_icon_004fd3a0(
        Raw{static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x70))}, &icon.handle, icon.uv);
    if (icon.handle != nullptr) {
        host.widget_set_state_texture_00ab2690(s.ptr(0x24), 2, icon.handle, icon.uv);
        host.widget_virtual_88(s.ptr(0x24), 2, 0, 1.0f);
    } else {
        host.widget_virtual_88(s.ptr(0x24), 0, 0, 1.0f);
    }
    if (icon.handle != nullptr) host.refcounted_release(icon.handle);
}

void multi_game_lobby_exit_005d9a10(Raw s, Seg15Host& host) {
    host.listbox_clear_rows_00a9bec0(s.ptr(0xA8));
    host.listbox_clear_rows_00a9bec0(s.ptr(0x94));
    host.menu_prompt_dismiss_slot_00532a20(1);
    host.global_object(0x00E19698).at<std::uint8_t>(4) = 0;
    multi_game_lobby_delete_row_objects_005d8a70(s, host);
    host.gui_manager_reset_screens_00aa0f70();
    host.scroller_set_range_006834a0(Raw{s.p + 0x30}, 0.0f);
    host.scroller_attach_content_00683790(Raw{s.p + 0x30}, nullptr);
    s.at<std::uint32_t>(0xAC) = 0;
}

std::uint64_t multi_game_lobby_map_mask_005d9c50(Seg15Host& host) {
    const Raw map = host.lobby_selected_map_record_005d8650();
    const Raw name{map.p + 0x10};
    if (name.at<std::uint32_t>(0) == 0) return 0;
    const std::string text = host.native_string_value(name);
    // 00467CF0 reverse find of "_" (00CE7890), 00469840 substring after it
    // (the whole name when there is no '_': -1 + 1 = 0).
    const std::size_t underscore = text.rfind('_');
    const std::size_t start = underscore == std::string::npos ? 0 : underscore + 1;
    const std::string suffix = text.substr(start);
    const long number = std::atol(suffix.c_str());
    const std::uint32_t shift = static_cast<std::uint32_t>(number - 1) & 0xFF;  // __allshl reads CL
    return shift >= 64 ? 0 : (std::uint64_t{1} << shift);
}

void multi_game_lobby_on_listbox_activate_005d9d90(Raw s, void* widget, std::uint32_t /*flag*/, Seg15Host& host) {
    if (host.global_object(0x00E188A8).at<std::uint8_t>(0x2164) != 0) return;
    const auto* const editor = static_cast<const std::uint8_t*>(s.ptr(0x228));
    if (host.widget_is_visible_38(s.ptr(0x18)) && editor[0x85] != 0 && widget != s.ptr(0x228)) return;
    if (widget == s.ptr(0x94) || widget != s.ptr(0xA8)) return;
    if (s.ptr(0x1C) != s.ptr(0xA4)) multi_game_lobby_set_focus_005d67e0(s, s.ptr(0xA4), host);
    const std::int32_t row = host.listbox_selected_index_00a9c920(s.ptr(0xA8));
    const Raw game = host.global_object(0x00E188A8);
    void* const record = game.ptr(0x18CC + row * 4);
    const std::int32_t local = game.at<std::int32_t>(0x18EC);
    void* const local_record = local >= 0 && local <= 7 ? game.ptr(0x18CC + local * 4) : nullptr;
    if (record == local_record || host.global_block(0x00E08958).at<std::int32_t>(0) != -1) {
        host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1C) = 1;
        return;
    }
    const std::int32_t self = game.at<std::int32_t>(0x18C8);
    const std::int32_t picked = host.listbox_selected_index_00a9c920(s.ptr(0xA8));
    if (static_cast<const std::uint8_t*>(record)[9] != 0) {
        host.session_request_select_player_007718f0(self, picked);
        multi_game_lobby_on_player_row_activate_005d8700(s, host);
        return;
    }
    const float now = host.global_object(0x00E188A8).at<float>(0x64C);
    if (now > host.global_block(0x00E1959C).at<float>(static_cast<std::size_t>(picked) * 4)) {
        host.session_swap_slot_request_00771990(self, picked);
        host.global_block(0x00E19598).at<float>(0) =
            static_cast<float>(host.global_object(0x00E188A8).at<float>(0x64C) + kLobbySwapRetrySeconds);
        multi_game_lobby_on_player_row_activate_005d8700(s, host);
    } else {
        host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1C) = 1;
    }
}

void multi_game_lobby_on_list_select_005da000(Raw s, void* widget, Seg15Host& host) {
    const auto* const editor = static_cast<const std::uint8_t*>(s.ptr(0x228));
    if (host.widget_is_visible_38(s.ptr(0x18)) && editor[0x85] != 0) return;
    // When the focus is the chat box (+150h), notify (*[*00E198B4+54h]+8h)'s +0Ch.
    auto leave_chat = [&] {
        if (s.ptr(0x1C) != s.ptr(0x150)) return;
        auto* const owner = static_cast<std::uint8_t*>(host.global_object(0x00E198B4).ptr(0x54));
        host.object_virtual(Raw{owner + 8}, 0x0C);
    };
    const Raw w{static_cast<std::uint8_t*>(widget)};
    if (host.widget_type_5c(widget) == 2) {
        host.listbox_select_index_00a9c7c0(s.ptr(0xA8), w.at<std::int32_t>(0xD8));
        leave_chat();
        multi_game_lobby_set_focus_005d67e0(s, s.ptr(0xA4), host);
        void* const row = host.listbox_row_00a9be00(s.ptr(0xA8), host.listbox_selected_index_00a9c920(s.ptr(0xA8)));
        host.object_virtual_arg(Raw{s.p + 8}, 0x04, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(row)));
        return;
    }
    if (host.widget_type_5c(widget) == 3) return;
    if (host.widget_type_5c(widget) != 0x12) return;
    if (widget == s.ptr(0xA4)) {
        leave_chat();
        multi_game_lobby_set_focus_005d67e0(s, s.ptr(0xA4), host);
    } else if (widget == s.ptr(0x20)) {
        leave_chat();
        multi_game_lobby_set_focus_005d67e0(s, s.ptr(0x20), host);
        multi_game_lobby_apply_local_map_005d9f90(host);  // ECX = screen in the image
    } else if (widget == s.ptr(0x8C)) {
        leave_chat();
        multi_game_lobby_set_focus_005d67e0(s, s.ptr(0x8C), host);
        host.input_manager_update_00a92c40(0.0f);
        multi_game_lobby_set_editable_005d5870(s, true, host);
    } else if (widget == s.ptr(0x28)) {
        leave_chat();
        multi_game_lobby_set_focus_005d67e0(s, s.ptr(0x28), host);
        const Raw game = host.global_object(0x00E188A8);
        const Raw local_record{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4))};
        auto* const library = static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x70));
        Raw{library}.at<std::int32_t>(0x98) = local_record.at<std::int32_t>(0x28);
        Raw{static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x70))}.at<std::int32_t>(0x94) = 8;
        host.multi_menu_push_request_00688ad0(0x0B);  // tail jump with the argument replaced
    }
}

void multi_game_lobby_confirm_leave_005da210(Raw s, Seg15Host& host) {
    const auto* const editor = static_cast<const std::uint8_t*>(s.ptr(0x228));
    if (host.widget_is_visible_38(s.ptr(0x18)) && editor[0x85] != 0) {
        multi_game_lobby_set_editable_005d5870(s, false, host);
        return;
    }
    multi_game_lobby_set_focus_005d67e0(s, s.ptr(0xA4), host);
    const Raw game = host.global_object(0x00E188A8);
    if (game.at<std::uint8_t>(0x218C) != 0) {
        multi_game_lobby_leave_to_menu_005d8410(s, host);
        return;
    }
    if (game.at<std::int32_t>(0x1FE4) != 1) {
        host.menu_prompt_raise_00531b00(1, "FE.multi_lobby_clientleaveconfirm", 1, 0x005D98C0, 0,  // 00CF21D0
                                        host.native_string_temp(""), 0.0f, 0, "", 1);
        host.menu_prompt_focus_last_00530c20();
        return;
    }
    const Raw players{static_cast<std::uint8_t*>(game.ptr(0x2078) != nullptr ? game.ptr(0x2078) : game.ptr(0x207C))};
    if (players.at<std::int32_t>(0x10) > 1) {
        host.menu_prompt_raise_00531b00(1, "FE.multi_lobby_serverleaveconfirm", 1, 0x005D9FE0, 0,  // 00CF21F4
                                        host.native_string_temp(""), 0.0f, 0, "", 1);
        host.menu_prompt_focus_last_00530c20();
        return;
    }
    multi_game_lobby_leave_to_menu_005d8410(s, host);
}

void multi_game_lobby_apply_selected_map_005da530(Raw s, Seg15Host& host) {
    if (s.at<std::int32_t>(0x2F0) != -1 && s.at<std::int32_t>(0x2F4) != -1) {
        auto entry = [&] {
            const Raw list = host.lobby_map_list_005d5160(s);
            return Raw{static_cast<std::uint8_t*>(list.ptr(8)) + s.at<std::int32_t>(0x2F4) * 8};
        };
        const char* const text = static_cast<const char*>(entry().ptr(4));
        host.game_set_command_line_text_004c8950(text != nullptr ? text : "");  // 00E19605 when null
        const Raw name = entry();
        if (name.p != host.global_block(0x00E195C4).p) host.native_string_copy_0041dd40(host.global_block(0x00E195C4), name);
    }
    host.game_select_scene_record_004c6890(host.global_block(0x00E19594).at<std::int32_t>(0));
    host.game_effective_mode_004bca50();  // the argument of 005D9C50, unused there
    const std::uint64_t mask = multi_game_lobby_map_mask_005d9c50(host);
    s.at<std::uint32_t>(0x2C4) = static_cast<std::uint32_t>(mask >> 32);
    s.at<std::uint32_t>(0x2C0) = static_cast<std::uint32_t>(mask);
    host.global_object(0x00E188A8).at<std::uint32_t>(0x2060) = s.at<std::uint32_t>(0x2C8) | static_cast<std::uint32_t>(mask);
    host.global_object(0x00E188A8).at<std::uint32_t>(0x2064) =
        s.at<std::uint32_t>(0x2CC) | s.at<std::uint32_t>(0x2C4);
    multi_game_lobby_apply_settings_005d62b0(host);
    s.at<std::uint32_t>(0x10) = 1;
    multi_game_lobby_confirm_leave_005da210(s, host);
}

bool multi_game_lobby_check_client_compatibility_005d71b0(Raw s, Seg15Host& host) {
    Raw game = host.global_object(0x00E188A8);
    if (game.at<std::int32_t>(0x18C8) < 0) return false;
    const std::int32_t local = game.at<std::int32_t>(0x18EC);
    if (local < 0 || local > 7) return false;
    if (game.ptr(0x5FC) == nullptr || static_cast<const std::uint8_t*>(game.ptr(0x5FC))[0x914] == 0) return false;
    if (game.at<std::int32_t>(0x1FE4) == 1) {
        if (!multi_game_lobby_slots_meet_content_masks_005d6ba0(host)) {
            host.menu_prompt_raise_00531b00(2, "globals.some_clients_incompatible", 2, 0, 0,  // 00CF20A8
                                            host.native_string_temp(""), 0.0f, 0, "", 1);
            return false;
        }
        if (host.global_object(0x00E188A8).at<std::int32_t>(0x1FE4) == 1) {
            if (lobby_ready_to_start_005d5260(host)) {
                session_dismiss_load_prompt_005d6cd0(host);  // ECX = screen in the image
                s.at<std::uint8_t>(0x271) = 1;
                const Raw online = host.global_object(0x00F8A2FC);
                if (online.p != nullptr) online.at<std::uint8_t>(0x75) = 1;
                host.session_collect_incompatible_clients_0076f360();
                return true;
            }
        }
        game = host.global_object(0x00E188A8);
    }
    if (game.at<std::int32_t>(0x1FE4) == 0 || game.at<std::uint8_t>(0x218C) != 0) return false;
    if (game.at<std::int32_t>(0x1FE4) != 1 && static_cast<const std::uint8_t*>(game.ptr(0x207C))[0xC4] == 0) return false;
    multi_game_lobby_refresh_local_ready_005d5790(s, host);
    const Raw now = host.global_object(0x00E188A8);
    const auto* const record = static_cast<const std::uint8_t*>(now.ptr(0x18CC + now.at<std::int32_t>(0x18EC) * 4));
    host.session_request_player_ready_00771dd0(now.at<std::int32_t>(0x18C8), record[0x0C] == 0, 1);
    return false;
}

void multi_game_lobby_on_focus_005d6060(Raw s, void* widget, bool focus, Seg15Host& host) {
    if (host.widget_type_5c(widget) == 2) return;
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_float_4c(widget, focus ? kLobbyFocusDimAlpha : 1.0f);  // 00D7A24C
        return;
    }
    for (std::size_t off : {0xA4u, 0x20u, 0x8Cu, 0x28u, 0x150u}) {
        if (widget == s.ptr(off)) return;
    }
    auto apply = [&] {  // 005D6105
        if (host.widget_type_5c(widget) == 6) {
            host.widget_virtual_88(widget, focus ? 1 : 0, 0, 1.0f);
        } else if (host.widget_type_5c(widget) == 0x12) {
            host.widget_virtual_84(widget, focus ? 1 : 0);
        }
    };
    if (widget == s.ptr(0x90)) {
        if (s.at<std::uint8_t>(0x78) == 0) apply();
        return;
    }
    if (widget == s.ptr(0x154)) {
        if (s.at<std::uint8_t>(0x1A0) == 0) apply();
        return;
    }
    if (multi_game_lobby_settings_open_005d5490(s, host)) {
        if (widget == s.ptr(0x220) || widget == s.ptr(0x224)) apply();
        return;
    }
    void* const first = s.ptr(0x144);
    if (widget != first && widget != s.ptr(0x148) && widget != s.ptr(0x14C)) {
        apply();
        return;
    }
    if (focus) {
        if (widget == first) {
            host.widget_virtual_88(first, 1, 0, 1.0f);
        } else {
            host.widget_virtual_88(s.ptr(0x148), 1, 0, 1.0f);
            host.widget_virtual_88(s.ptr(0x14C), 1, 0, 1.0f);
        }
        return;
    }
    // Unfocused: which side is lit follows byte [*00E198B4+54h]+8Ch.
    const bool second_side = static_cast<const std::uint8_t*>(host.global_object(0x00E198B4).ptr(0x54))[0x8C] != 0;
    host.widget_virtual_88(s.ptr(0x144), second_side ? 0 : 2, 0, 1.0f);
    host.widget_virtual_88(s.ptr(0x148), second_side ? 2 : 0, 0, 1.0f);
    host.widget_virtual_88(s.ptr(0x14C), second_side ? 2 : 0, 0, 1.0f);
}

void byte_sbo_uninit_fill_n_005443f0(std::uint8_t* dest, std::uint32_t count, const std::uint8_t* value,
                                     Seg15Host& host) {
    for (; count != 0; --count, dest += 0x24) {
        if (dest) {
            dest[0] = value[0];
            sbo_reset(dest);
            host.legacy_string_assign_substring(Raw{dest + 4}, Raw{const_cast<std::uint8_t*>(value) + 4});
            std::memcpy(dest + 0x20, value + 0x20, 4);
        }
    }
}

// ---------------------------------------------------------------------------
// cc13-fe10 additions (docs/CC13_FE_10.md): read from the live decompile and
// disasm-raw; the listener routines are rebased from ECX to the screen.
// ---------------------------------------------------------------------------
namespace {

bool widget_focused(void* widget) { return static_cast<const std::uint8_t*>(widget)[kWidgetFocusedByte] != 0; }

// The type-6 / type-12h focus tail shared by the lobby listeners (005D5FFE).
void lobby_focus_tail(void* widget, Fe10Host& host) {
    if (host.widget_type_5c(widget) == 6) {
        host.widget_virtual_88(widget, widget_focused(widget) ? 1 : 0, 0, 1.0f);
    } else if (host.widget_type_5c(widget) == 0x12) {
        host.widget_virtual_84(widget, widget_focused(widget) ? 1 : 0);
    }
}

// Each element of a pointer array {first, last} at off: 00AA8240(&pos).
void place_all(Raw s, std::size_t off, const float pos[3], Fe10Host& host) {
    void** const first = static_cast<void**>(s.ptr(off));
    void** const last = static_cast<void**>(s.ptr(off + 4));
    if (first == nullptr) return;
    for (void** it = first; it < last; ++it) {
        host.widget_set_resolved_position_00aa8240(*it, pos);
    }
}

}  // namespace

void multi_game_lobby_on_activate_005d5d50(Raw s, void* widget, Fe10Host& host) {
    if (host.widget_type_5c(widget) == 2) return;
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_float_4c(widget, widget_focused(widget) ? kLobbyFocusDimAlpha : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    for (std::size_t off : {0x20u, 0x8Cu, 0x28u, 0x150u}) {
        if (widget == s.ptr(off)) return;
    }
    // Scroll arrows: stop the pane's auto scroll; thumbs: end the drag.
    struct Pane { std::size_t arrow_a, arrow_b, thumb, block; };
    for (const Pane& p : {Pane{0x9C, 0x98, 0x90, 0x30}, Pane{0x13C, 0x138, 0x154, 0x158},
                          Pane{0xBC, 0xB8, 0xC8, 0xCC}}) {
        if (widget == s.ptr(p.arrow_a) || widget == s.ptr(p.arrow_b)) {
            host.scroller_set_auto_scroll_00683300(Raw{s.p + p.block}, 0, 0.0f, 0);
            lobby_focus_tail(widget, host);
            return;
        }
        if (widget == s.ptr(p.thumb)) {
            host.scroller_end_thumb_drag_00683320(Raw{s.p + p.block});
            lobby_focus_tail(widget, host);
            return;
        }
    }
    if (multi_game_lobby_settings_open_005d5490(s, host)) {
        if (widget == s.ptr(0x220) || widget == s.ptr(0x224)) lobby_focus_tail(widget, host);
        return;
    }
    void* const side = s.ptr(0x144);
    void* const left = s.ptr(0x148);
    void* const right = s.ptr(0x14C);
    if (widget != side && widget != left && widget != right) {
        lobby_focus_tail(widget, host);
        return;
    }
    const bool pair_focused = widget_focused(left) || widget_focused(right);
    const bool side_flag = static_cast<const std::uint8_t*>(host.global_object(kMultiMenuGlobal).ptr(0x54))[0x8C] != 0;
    if (side_flag) {
        host.widget_virtual_88(left, pair_focused ? 1 : 2, 0, 1.0f);
        host.widget_virtual_88(right, pair_focused ? 1 : 2, 0, 1.0f);
        host.widget_virtual_88(side, widget_focused(side) ? 1 : 0, 0, 1.0f);
    } else {
        host.widget_virtual_88(left, pair_focused ? 1 : 0, 0, 1.0f);
        host.widget_virtual_88(right, pair_focused ? 1 : 0, 0, 1.0f);
        host.widget_virtual_88(side, widget_focused(side) ? 1 : 2, 0, 1.0f);
    }
}

void multi_game_lobby_build_map_preview_005d6e30(Raw s, Fe10Host& host) {
    void* const settings = host.gui_find_child_00aa7e00(s.ptr(0x14), "settings_Group");
    float pos[3];
    host.widget_resolved_position_00aa6750(host.gui_find_child_00aa7e00(settings, "map_001_Icon"), pos);
    place_all(s, 0x1F8, pos, host);
    host.widget_resolved_position_00aa6750(host.gui_find_child_00aa7e00(s.ptr(0x18), "map_001_Icon"), pos);
    place_all(s, 0x244, pos, host);
}

void multi_game_lobby_step_selected_slot_ai_005d9660(Raw s, bool step_a, bool step_b, Fe10Host& host) {
    if (host.online_service_virtual(0x198) && host.global_object(0x00F8A2FC).at<std::uint8_t>(0x4D) != 0) return;
    if (!step_a && !step_b) return;
    s.ptr(0xA8) = host.gui_find_child_00aa7e00(s.ptr(0x14), "Players_Listbox");
    void* const listbox = s.ptr(0xA8);
    const Raw game = host.global_object(kGameGlobal);
    auto slot_record = [&]() -> const std::uint8_t* {
        return static_cast<const std::uint8_t*>(game.ptr(0x18CC + host.listbox_selected_index_00a9c920(listbox) * 4));
    };
    auto set_level = [&](std::int32_t level) {
        host.session_set_slot_ai_skill_level_0076cc40(host.listbox_selected_index_00a9c920(listbox), level);
    };
    auto set_ai = [&](std::int32_t on) {
        host.session_slot_ai_enable_00771e80(host.listbox_selected_index_00a9c920(listbox), on);
    };
    const std::uint8_t* record =
        host.listbox_selected_index_00a9c920(listbox) <= -1 ? nullptr : slot_record();
    if ((record == nullptr || record[0x0A] == 0) && !game_is_standard_game_mode_005d4a10(host)) {
        set_ai(1);
        set_level(step_b ? 5 : 3);
        return;
    }
    const std::int32_t kind = *reinterpret_cast<const std::int32_t*>(slot_record() + 0x2C);
    if ((kind == 3 || kind == 1) && step_a) return set_level(4);
    if (kind == 4) {
        if (step_a) return set_level(5);
        if (step_b) return set_level(game_is_standard_game_mode_005d4a10(host) ? 1 : 3);
    } else if (kind == 5 && step_b) {
        return set_level(4);
    }
    if (!game_is_standard_game_mode_005d4a10(host)) return set_ai(0);
    if (host.global_block(0x00F88A30).at<std::uint8_t>(0) != 0) return;
    if ((kind != 3 && kind != 1) || !step_b) return;
    if (game_is_standard_game_mode_005d4a10(host)) return set_level(5);
    set_ai(0);
}

void pause_multi_build_menu_rows_005d39a0(void* listbox, std::int32_t wanted, Fe10Host& host) {
    host.listbox_set_listener_00a9ac40(listbox, nullptr);
    host.listbox_clear_rows_00a9bec0(listbox);
    void* const row_template = host.gui_find_child_00aa7e00(listbox, "MainListbox_Text");
    std::int32_t selected = 0;
    std::int32_t shown = 0;
    for (int i = 0; i < kPauseRowCount; ++i) {
        if (i == wanted) selected = shown;
        if (host.global_block(kPauseRowFlags + i).at<std::uint8_t>(0) == 0) continue;
        void* const row = host.widget_clone_00aab4c0(row_template);
        if (i == 2) host.widget_virtual_7c(row, host.award_tracker_004e1ca0().at<std::int32_t>(0x9C) != 0 ? 0 : 1);
        host.listbox_insert_row_00a9d750(listbox, row);
        host.text_set_localised_cstr_00abbe50(row, host.global_block(kPauseRowKeys + i * 4).at<const char*>(0));
        ++shown;
    }
    if (wanted == 0) {
        const std::uint32_t count = host.listbox_row_block_00a9ac50(listbox).at<std::uint32_t>(8);
        std::uint32_t& remembered = host.global_block(kPauseRememberedRow).at<std::uint32_t>(0);
        if (remembered >= count) remembered = count;
        selected = static_cast<std::int32_t>(remembered);
    }
    host.listbox_select_index_00a9c7c0(listbox, selected);
    host.widget_virtual_80(listbox, 1);
    void* const owner = host.global_object(kInterfaceGlobal).ptr(0xE4);
    host.listbox_set_listener_00a9ac40(listbox, owner ? static_cast<std::uint8_t*>(owner) + 8 : nullptr);
    host.listbox_reset_highlight_00a9cf30(listbox, 1, 1);
}

void pause_multi_enter_005d3b40(Raw s, Fe10Host& host) {
    const Raw game = host.global_object(kGameGlobal);
    host.front_end_frame_select_00518d60(0, game.at<std::uint8_t>(0x218C) != 0 ? "ingame.paused"
                                                                               : "ingame.pause_multi_title");
    s.ptr(0x20) = host.gui_find_child_00aa7e00(s.ptr(0x10), "Main_Listbox");
    s.ptr(0x14) = host.gui_find_child_00aa7e00(s.ptr(0x20), "felki_bal_Icon");
    s.ptr(0x18) = host.gui_find_child_00aa7e00(s.ptr(0x20), "felki_jobb_Icon");
    host.widget_set_visible_34(s.ptr(0x14), false);
    host.widget_set_visible_34(s.ptr(0x18), false);
    host.listbox_set_listener_00a9ac40(s.ptr(0x20), s.p + 8);
    host.widget_set_focus_60(s.ptr(0x20), true);
    s.at<std::uint8_t>(0x1C) = 0;
    auto flag = [&](int i) -> std::uint8_t& { return host.global_block(kPauseRowFlags + i).at<std::uint8_t>(0); };
    flag(0) = 1;
    flag(1) = 0;
    flag(3) = 1;
    flag(4) = 1;
    flag(2) = 1;
    const bool live = game.at<std::uint8_t>(0x218C) == 0;
    const auto* const local = static_cast<const std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4));
    const bool row5 = live && local[0x1B] == 0;
    flag(5) = row5 ? 1 : 0;
    flag(6) = (live && !row5) ? 1 : 0;
    flag(7) = 1;
    flag(8) = 1;
    pause_multi_build_menu_rows_005d39a0(s.ptr(0x20), 0, host);
    std::int32_t awards_row = 0;
    for (int i = 0; i < 2; ++i) {
        if (flag(i) != 0) ++awards_row;
    }
    void* const listbox = s.ptr(0x20);
    float w = *reinterpret_cast<const float*>(static_cast<std::uint8_t*>(host.listbox_row_00a9be00(listbox, awards_row)) + 0x114);
    w = static_cast<float>(w / kPauseIconScreenWidth);
    w = static_cast<float>(w * kPauseIconHalf);
    w = static_cast<float>(w + kPauseIconGap);
    float pos[3];
    host.widget_resolved_position_00aa6750(host.listbox_row_00a9be00(listbox, awards_row), pos);
    const float left[3] = {pos[0] - w, static_cast<float>(pos[1] - kPauseIconLift), static_cast<float>(pos[2] - 0.0)};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x14), left);
    host.widget_resolved_position_00aa6750(host.listbox_row_00a9be00(listbox, awards_row), pos);
    const float right[3] = {pos[0] + w, static_cast<float>(pos[1] - kPauseIconLift), static_cast<float>(pos[2] + 0.0)};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x18), right);
    s.at<float>(0x24) = kPauseInputDelay;
}

void pause_multi_update_005d3f20(Raw s, float seconds, Fe10Host& host) {
    const Raw pause{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0xD8))};
    if (pause.at<std::uint8_t>(0x10) != 0) {
        host.pause_screen_scroller_update_00683820();
        host.pause_screen_update_hints_006037d0();
    } else if (host.award_tracker_004e1ca0().at<std::int32_t>(0x84) == 0) {
        host.widget_set_visible_34(s.ptr(0x14), false);
        host.widget_set_visible_34(s.ptr(0x18), false);
    } else {
        // Walk the tracker's std::map (+7Ch, head +80h) for an entry whose
        // string list (+14h of the node) is not empty.
        bool any = false;
        const Raw head{static_cast<std::uint8_t*>(host.award_tracker_004e1ca0().ptr(0x80))};
        for (Raw node{static_cast<std::uint8_t*>(head.ptr(0))}; node.p != head.p;
             node = host.tree_iterator_next_004b7de0(node)) {
            if (host.pooled_string_list_copy_count_004d48a0(Raw{node.p + 0x14}) != 0) {
                any = true;
                break;
            }
        }
        host.widget_set_visible_34(s.ptr(0x14), any);
        host.widget_set_visible_34(s.ptr(0x18), any);
    }
    const float delay = s.at<float>(0x24);
    const Raw game = host.global_object(kGameGlobal);
    if (delay > 0.0f && game.at<std::uint8_t>(0x218C) == 0) {
        s.at<float>(0x24) = delay - seconds;
    } else if ((host.action_pressed_004c43c0(0x4B) || host.action_pressed_004c43c0(1)) &&
               host.menu_command_screen_00425d10().at<std::uint8_t>(4) == 0) {
        if (pause.at<std::uint8_t>(0x10) != 0) {
            host.pause_screen_show_page_005fd890(0, 1);
            return;
        }
        host.game_toggle_pause_menu_004db030();
        host.global_block(kPauseRememberedRow).at<std::uint32_t>(0) = 0;
        return;
    }
    const MainMenuCommandArgument commands[5] = {
        {0xA2, "globals.select", 0}, {0x00, "", 1}, {0xB3, "globals.navigate", 1}, {0x00, "", 1},
        {0xA3, "globals.back", 2}};
    host.command_bar_rebuild_0054b530(commands);
    host.command_bar_set_help_line_0054a0c0(host.native_string_temp(""));
    host.auto_test_step_00427190(0x10);
}

void multi_game_lobby_select_map_by_name_005d92b0(Raw s, Raw name, std::int32_t kind, Fe10Host& host) {
    const char* const wanted = name.at<const char*>(4);
    if (wanted == nullptr || _stricmp(wanted, "") == 0) {
        s.at<std::int32_t>(0x2F4) = -1;
        s.at<std::int32_t>(0x2F0) = -1;
        host.native_string_assign_cstr(host.global_block(kLobbyMapNameString), "");
        host.native_string_assign_cstr(host.global_block(kLobbySubSceneString), "");
        return;
    }
    auto* const first = static_cast<std::uint8_t*>(s.ptr(0x2E4));
    auto* const last = static_cast<std::uint8_t*>(s.ptr(0x2E8));
    const std::int32_t records = first ? static_cast<std::int32_t>((last - first) / 0x24) : 0;
    for (std::int32_t i = 0; i < records; ++i) {
        const Raw record{first + i * 0x24};
        if (record.at<std::int32_t>(0) != kind) continue;
        auto* const names = static_cast<std::uint8_t*>(record.ptr(8));
        if (names == nullptr) continue;
        const std::int32_t count = static_cast<std::int32_t>((static_cast<std::uint8_t*>(record.ptr(0xC)) - names) / 8);
        for (std::int32_t j = 0; j < count; ++j) {
            const Raw entry{names + j * 8};
            const std::uint32_t length = entry.at<std::uint32_t>(0);
            if (length != name.at<std::uint32_t>(0)) continue;
            if (length != 0 && _stricmp(entry.at<const char*>(4), name.at<const char*>(4)) != 0) continue;
            const Raw target = host.global_block(kLobbyMapNameString);
            if (entry.p != target.p) host.native_string_copy_0041dd40(target, entry);
            s.at<std::int32_t>(0x2F0) = i;
            s.at<std::int32_t>(0x2F4) = j;
            return;
        }
    }
    multi_game_lobby_select_map_by_name_005d92b0(s, host.native_string_temp(""), -1, host);
    host.native_string_assign_cstr(Raw{host.global_object(kGameGlobal).p + 0x600}, "");
}

void multi_chat_format_message_row_005d09a0(void* text, Raw sender, std::int32_t colour, Raw message,
                                            Fe10Host& host) {
    if (sender.at<std::uint32_t>(0) == 0) {
        host.text_submit_utf16_00ab6ab0(text, message);
    } else {
        std::uint8_t sep[8], who[8], joined[8], line[8];
        wide_string_construct_0052e0f0(Raw{sep}, kChatSeparator, host);
        const char* const data = sender.at<const char*>(4);
        // A null sender buffer falls back to the C string stored at 00E19530.
        host.wide_string_from_cstring_004c5e60(
            Raw{who}, data ? data : reinterpret_cast<const char*>(host.global_block(kChatEmptyName).p));
        wide_string_concat_005cf570(Raw{who}, Raw{joined}, Raw{sep}, host);
        wide_string_concat_005cf570(Raw{joined}, Raw{line}, message, host);
        host.text_submit_utf16_00ab6ab0(text, Raw{line});
        host.wide_string_release(Raw{line});
        host.wide_string_release(Raw{joined});
        host.wide_string_release(Raw{who});
        host.wide_string_release(Raw{sep});
    }
    const Raw config = host.global_config_00432650();
    host.widget_set_colour_50(text, reinterpret_cast<const float*>(config.p + 0x208 + colour * 0x10));
}

bool multi_chat_rebuild_message_rows_005d0c60(Raw s, void* row_template, void* list, Fe10Host& host) {
    if (s.at<std::uint8_t>(0x8D) == 0) return false;
    struct Source { std::size_t list, message, sender, colour; bool downward; };
    Source source{};
    switch (s.at<std::int32_t>(0x40)) {
    case 1:
    case 2: source = {0x80, 0x08, 0x10, 0x18, true}; break;
    case 3: source = {0x68, 0x0C, 0x14, 0x1C, false}; break;
    case 4: source = {0x74, 0x08, 0x10, 0x18, false}; break;
    default:
        s.at<std::uint8_t>(0x8D) = 0;
        return true;
    }
    const float* const start = host.widget_local_position_00aa67f0(row_template);
    float pos[3] = {start[0], start[1], start[2]};
    host.widget_clear_00aa83f0(list);
    auto* const head = static_cast<std::uint8_t*>(s.ptr(source.list + 4));
    for (auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0)); node != head;
         node = static_cast<std::uint8_t*>(Raw{node}.ptr(0))) {
        void* const row = host.widget_clone_under_00aab4c0(row_template, list);
        multi_chat_format_message_row_005d09a0(row, Raw{node + source.sender}, Raw{node}.at<std::int32_t>(source.colour),
                                               Raw{node + source.message}, host);
        host.widget_set_local_position_00aa7dc0(row, pos);
        host.widget_set_visible_34(row, s.at<std::uint8_t>(0x8D) != 0);
        const double height = host.text_normalized_height_00ab6bd0(row);
        pos[1] = source.downward ? static_cast<float>(height + pos[1]) : static_cast<float>(pos[1] - height);
    }
    const float top = host.widget_local_position_00aa67f0(row_template)[1];
    const float size[2] = {host.widget_size_00aa6740(list)[0], source.downward ? pos[1] - top : top - pos[1]};
    host.widget_set_size_58(list, size);
    s.at<std::uint8_t>(0x8D) = 0;
    return true;
}

void multi_chat_load_layout_005d14d0(Raw s, Fe10Host& host) {
    s.ptr(0x44) = host.gui_find_child_00aa7e00(s.ptr(0x3C), "ChatModeOn_Group");
    s.ptr(0x5C) = host.gui_find_child_00aa7e00(s.ptr(0x44), "SendMessage_Text");
    s.ptr(0x54) = host.gui_find_child_00aa7e00(s.ptr(0x44), "MessageTemplate_Text");
    host.widget_set_visible_34(s.ptr(0x54), false);
    s.ptr(0x48) = host.gui_find_child_00aa7e00(s.ptr(0x3C), "ChatModeOff_Group");
    s.ptr(0x50) = host.gui_find_child_00aa7e00(s.ptr(0x48), "MessageTemplate_Text");
    host.widget_set_visible_34(s.ptr(0x50), false);
    s.ptr(0x4C) = host.gui_find_child_00aa7e00(s.ptr(0x3C), "Lobby_Group");
    s.ptr(0x60) = host.gui_find_child_00aa7e00(s.ptr(0x4C), "SendMessage_Text");
    host.native_string_assign_cstr(Raw{s.p + 0x20}, "");
    s.at<std::int32_t>(0x40) = 0;
    host.widget_set_visible_34(s.ptr(0x44), false);
    host.widget_set_visible_34(s.ptr(0x48), false);
    host.widget_set_visible_34(s.ptr(0x4C), false);
    s.at<std::uint8_t>(0x64) = 0;
    host.text_editor_set_active_00a966e0(Raw{s.p + 8}, false, s.ptr(0x5C));
    host.widget_set_visible_34(s.ptr(0x60), false);
    multi_chat_refresh_mode_label_005d07e0(s, host);
    host.sound_request_singleton_004c1b90().at<std::uint8_t>(0x1A) = 1;
}

void* fingerprint_payload_get_singleton_0053bc00(Fe10Host& host) {
    void*& instance = host.global_block(kFingerprintPayloadGlobal).ptr(0);
    if (instance != nullptr) return instance;
    const Raw manager = host.singleton_lifetime_manager_00415350();
    auto* const lock = static_cast<std::uint8_t*>(manager.ptr(0x10));
    if (lock) {
        host.critical_section_enter(lock);
        ++Raw{lock}.at<std::int32_t>(0x18);
    }
    if (instance == nullptr) {
        void* const block = host.operator_new_00bf681b(0x204);
        instance = block ? host.fingerprint_payload_construct_00be9760(block) : nullptr;
        host.singleton_lifetime_register_00bd0c30(host.singleton_lifetime_manager_00415350(), instance);
    }
    if (lock) {
        --Raw{lock}.at<std::int32_t>(0x18);
        host.critical_section_leave(lock);
    }
    return instance;
}

void free_camera_bind_settings_00540d50(Raw block, Fe10Host& host) {
    const Raw settings = host.game_settings_00424c40();
    alignas(4) std::uint8_t storage[0x30] = {};
    const Raw record{storage};
    auto* const first = static_cast<std::uint8_t*>(settings.ptr(0x740));
    const std::uint32_t count =
        first ? static_cast<std::uint32_t>((static_cast<std::uint8_t*>(settings.ptr(0x744)) - first) / 0x30) : 0;
    for (std::uint32_t i = 0; i < count; ++i) {
        const Raw src{first + i * 0x30};
        record.at<float>(0x10) = src.at<float>(0x1C);
        record.at<float>(0x14) = src.at<float>(0x20);
        host.handle_assign(Raw{record.p + 0x24}, src.ptr(0x24));
        host.handle_assign(Raw{record.p + 0x28}, src.ptr(0x28));
        host.handle_assign(Raw{record.p + 0x2C}, src.ptr(0x2C));
        record.at<float>(0x04) = src.at<float>(0x0C);
        record.at<float>(0x0C) = src.at<float>(0x18);
        record.at<float>(0x08) = src.at<float>(0x14);
        record.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x10);
        record.at<float>(0x00) = src.at<float>(0x08);
        host.native_string_copy_0041dd40(Raw{record.p + 0x1C}, src);
        host.free_camera_effect_vector_push_back_00540ca0(Raw{block.p + 4}, record);
    }
    if (count == 0) {
        host.free_camera_acquire_test_effects_0053e0e0(record);
        host.free_camera_effect_vector_push_back_00540ca0(Raw{block.p + 4}, record);
    }
    host.free_camera_effect_record_destroy_0053df40(record);
}

void hud_formation_destruct_00538a80(Raw s, Fe10Host& host) {
    s.at<std::uint32_t>(0x00) = 0x00CED6FC;
    s.at<std::uint32_t>(0x08) = 0x00CED6F8;
    s.at<std::uint32_t>(0x64) = 0x00CED6E0;
    s.at<std::uint32_t>(0x78) = 0x00CED6CC;
    for (std::size_t off : {0x98u, 0xA4u, 0x9Cu, 0xA0u, 0xA8u}) remove_page(s, off, host);
    // std::list of refcounted handles at +1A0h (head +1A4h, size +1A8h).
    auto clear_list = [&](std::size_t head_off, std::size_t size_off) {
        auto* const head = static_cast<std::uint8_t*>(s.ptr(head_off));
        auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0));
        Raw{head}.ptr(0) = head;
        Raw{head}.ptr(4) = head;
        s.at<std::uint32_t>(size_off) = 0;
        while (node != head) {
            auto* const next = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
            host.free_00bf65ac(node);
            node = next;
        }
        host.free_00bf65ac(head);
        s.ptr(head_off) = nullptr;
    };
    {
        auto* const head = static_cast<std::uint8_t*>(s.ptr(0x1A4));
        for (auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0)); node != head;
             node = static_cast<std::uint8_t*>(Raw{node}.ptr(0))) {
            host.refcounted_release(Raw{node}.ptr(8));
        }
    }
    clear_list(0x1A4, 0x1A8);
    clear_list(0x198, 0x19C);
    if (s.ptr(0x12C) != nullptr) host.free_00bf65ac(s.ptr(0x12C));
    s.ptr(0x12C) = nullptr;
    s.ptr(0x130) = nullptr;
    s.ptr(0x134) = nullptr;
    for (std::size_t tree : {0x11Cu, 0x110u}) {
        host.tree_erase_all(0x00538360, Raw{s.p + tree});  // erase(begin(), end())
        host.free_00bf65ac(s.ptr(tree + 4));
        s.ptr(tree + 4) = nullptr;
        s.at<std::uint32_t>(tree + 8) = 0;
    }
    s.at<std::uint32_t>(0x78) = kListboxListenerBaseVtable;
    host.callback_owner_destroy_00695870(Raw{s.p + 0x64});
    host.simple_3d_scene_destroy_0093cc80(Raw{s.p + 8});
    host.front_end_screen_destruct_004f71a0(s);
}

void hud_counters_apply_counter_record_0052cf60(Raw s, Raw r, Fe10Host& host) {
    const std::int32_t kind = r.at<std::int32_t>(0x20);
    if (kind == 1) {
        if (r.at<std::uint8_t>(0x24) != 0) {
            hud_counters_remove_entry_widget_0052cf00(s, r.at<std::int32_t>(0x18), host);
            return;
        }
        const std::string a = host.native_string_value(Raw{r.p + 0x28});
        const std::string b = host.native_string_value(Raw{r.p + 0x30});
        if (r.at<std::uint8_t>(0x25) != 0) {
            hud_counters_show_score_pair_0052cac0(s, r.at<std::int32_t>(0x18), a, b, r.at<std::uint32_t>(0x38),
                                                  r.at<std::uint32_t>(0x3C), host);
        } else {
            hud_counters_show_score_0052c990(s, r.at<std::int32_t>(0x18), a, b, host);
        }
        return;
    }
    if (kind != 0) return;
    if (r.at<std::uint8_t>(0x24) != 0) {
        hud_counters_remove_counter_a_0052ab40(s, host);
        return;
    }
    alignas(4) std::uint8_t storage[0xC] = {};
    const Raw units{storage};
    const auto* const ids = static_cast<const std::uint16_t*>(r.ptr(0x40));
    for (std::int32_t i = 0; i < r.at<std::int32_t>(0x44); ++i) {
        const std::int32_t id = ids[i];
        auto registry = [&](std::uint32_t base_global, std::uint32_t table_global) {
            const std::int32_t index = id - host.global_block(base_global).at<std::int32_t>(0);
            auto* const table = static_cast<std::uint8_t*>(host.global_block(table_global).ptr(0));
            return Raw{table + index * 0x10}.ptr(0xC);
        };
        void* const unit = id < host.global_block(0x00F89A10).at<std::int32_t>(0) ? registry(0x00F89A0C, 0x00F89A54)
                                                                                  : registry(0x00F89A60, 0x00F89AA8);
        if (size_of(units) == capacity_of(units)) {
            ptr_array_reserve_00529700(units, capacity_of(units) * 2 > 1 ? capacity_of(units) * 2 : 1, host);  // image instance 00529760
        }
        auto* const slot = reinterpret_cast<void**>(data_of(units)) + size_of(units);
        if (slot) *slot = unit;
        ++size_of(units);
    }
    hud_counters_show_unit_hp_0052bfa0(s, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(units.p)),
                                       host.native_string_value(Raw{r.p + 0x4C}), host);
    ptr_array_resize_005297c0(units, 0, host);
    host.free_00bf6989(units.ptr(0));
}

void hud_counters_relayout_00529a30(Raw s, Fe10Host& host) {
    const Raw entries{s.p + 0x20};
    auto entry = [&](std::int32_t i) { return Raw{static_cast<std::uint8_t**>(entries.ptr(0))[i]}; };
    auto kept = [&](std::int32_t i) {
        const Raw e = entry(i);
        if (host.widget_is_visible_38(e.ptr(0))) return true;
        for (std::size_t off : {0xA5u, 0xA6u, 0xA8u, 0xA7u, 0xA9u}) {
            if (e.at<std::uint8_t>(off) != 0) return true;
        }
        return false;
    };
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x24);) {
        if (!kept(i)) {
            ptr_array_erase_at_00529620(entries, i);
            continue;
        }
        host.widget_local_position_00aa67f0(entry(i).ptr(8));  // result unused in the image
        ++i;
    }
    bool parked = false;
    if (s.at<std::int32_t>(0x24) != 0) {
        float first[3];
        host.widget_resolved_position_00aa6750(entry(0).ptr(0), first);
        parked = kCounterWrapBelow > first[0];
    }
    const bool wide = host.global_object(kDisplayModeGlobal).at<std::uint8_t>(0x0D) != 0;
    float x;
    if (static_cast<const std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0xC0))[4] == 0 &&
        !host.support_manager_mode_006529e0(Raw{static_cast<std::uint8_t*>(host.global_object(kInterfaceGlobal).ptr(0xBC))})) {
        x = wide ? -kCounterRowStart : kCounterRowStart;
    } else {
        x = wide ? kCounterRowStartManagerWide : kCounterRowStartManager;
    }
    for (std::int32_t i = 0; i < s.at<std::int32_t>(0x24); ++i) {
        if (!kept(i)) continue;
        float pos[3];
        host.widget_resolved_position_00aa6750(entry(i).ptr(0), pos);
        pos[0] = x;
        host.widget_set_resolved_position_00aa8240(entry(i).ptr(0), pos);
        const float* const size = host.widget_size_00aa6740(entry(i).ptr(0x2C));
        x = static_cast<float>((size[0] + kCounterRowGap) + x);
        if (parked) {
            pos[0] = static_cast<float>(pos[0] - kCounterWrapAdd);
            host.widget_set_resolved_position_00aa8240(entry(i).ptr(0), pos);
        }
    }
    hud_counters_update_occlusion_00529810(s, host);
}

void free_camera_register_00540fe0(Raw s, Fe10Host& host) {
    host.front_end_screen_register_004f71d0(s);
    const Raw block{s.p + 0x64};
    block.at<float>(0x24) = 0.0f;
    block.at<std::int32_t>(0x20) = 0;
    block.at<float>(0x28) = 0.0f;
    free_camera_bind_settings_00540d50(block, host);
    s.ptr(0x1C) = host.gui_load_page_00aa5840("GUI_freecam");
    s.ptr(0x20) = host.gui_find_child_00aa7e00(s.ptr(0x1C), "weapon_Text");
    host.widget_set_visible_34(s.ptr(0x20), false);
}

// Packet cc13-trivialslots: the kTrivialSlots bodies; evidence and ABI per routine
// are in the header.
std::int32_t control_layout_screen_screen_id_00527d30(Raw) noexcept { return 0xE; }
Raw control_layout_screen_destroy_adjustor8_00527d40(Raw self, std::uint32_t flags, segs_13_15_22_44_cc13::SegBHost& host) {
    return segs_13_15_22_44_cc13::control_layout_scalar_delete_00527e40(Raw{self.p - 0x8}, flags, host);
}
void hud_counters_screen_bind_layout_00528c60(Raw) noexcept {}
std::int32_t hud_counters_screen_get_screen_id_0052b060(Raw) noexcept { return 0x51; }
bool hud_counters_screen_is_self_managed_0052b070(Raw) noexcept { return true; }
std::int32_t credits_screen_get_screen_id_0052fd60(Raw) noexcept { return 8; }
void menu_prompt_screen_on_listbox_selection_changed_no_op_00530640(Raw) noexcept {}
std::int32_t menu_prompt_screen_get_screen_id_00532310(Raw) noexcept { return 0x55; }
bool menu_prompt_screen_is_self_managed_00532320(Raw) noexcept { return true; }
void hud_formation_screen_on_listbox_activate_no_op_00533b30(Raw) noexcept {}
std::int32_t hud_formation_screen_get_screen_id_00538cd0(Raw) noexcept { return 0x2A; }
bool hud_formation_screen_occludes_lower_levels_00538ce0(Raw) noexcept { return true; }
std::int32_t frame_stats_screen_get_screen_id_0053ba50(Raw) noexcept { return 0x57; }
bool frame_stats_screen_is_self_managed_0053ba60(Raw) noexcept { return true; }
void frame_stats_screen_exit_0053ba90(Raw) noexcept {}
void hud_free_camera_screen_exit_0053d280(Raw) noexcept {}
void rec30vector_tidy_0053ff00(Raw self, Seg15Host& host) {
    host.effect_vector_tidy_0053f250(self);
}
std::int32_t hud_free_camera_screen_get_screen_id_00540360(Raw) noexcept { return 0x2B; }
std::int32_t hud_movie_camera_debug_screen_get_screen_id_005ce5b0(Raw) noexcept { return 0x36; }
void multi_chat_screen_enter_no_op_005ce740(Raw) noexcept {}
std::int32_t multi_chat_screen_get_screen_id_005cfe40(Raw) noexcept { return 0x15; }
bool multi_chat_screen_is_self_managed_005cfe50(Raw) noexcept { return true; }
std::int32_t multi_error_screen_get_screen_id_005d2f40(Raw) noexcept { return 0x1D; }
std::int32_t hud_pause_multi_screen_get_screen_id_005d3580(Raw) noexcept { return 0x19; }

}  // namespace bsp::frontend_seg15_22_cc13
