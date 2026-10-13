// Front-end and HUD screens of segments 15 and 22 (packet cc13-fe4).
// Evidence and status: docs/CC13_FE_4.md. Every routine was written from the
// live decompile (bsp.py ghidra decompile) and checked against disasm-raw where
// ECX, a stack argument, x87 or a store order mattered.

#include "bsp/frontend_seg15_22_cc13.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

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

}  // namespace bsp::frontend_seg15_22_cc13
