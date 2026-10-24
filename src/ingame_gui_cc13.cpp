// In-game GUI screens of segments 26 and 27 (packet cc13-fe2). Evidence and
// ABI per routine are in include/bsp/ingame_gui_cc13.hpp and docs/CC13_FE_2.md.

#include "bsp/ingame_gui_cc13.hpp"

#include "bsp/band_0060_cc13.hpp"

#include <cmath>
#include <cstring>
#include <utility>

namespace bsp::ingame_gui_cc13 {

namespace {

// A page slot pointer is stored in the screen; destruction removes the page
// (004C12B0 then 00AA31F0) before the base destructor runs.
void remove_page(Raw s, std::size_t off, HudScreenHost& host) {
    host.gui_remove_and_destroy_page_00aa31f0(s.ptr(off));
}

// Front-end screen "wanted"/"active" bytes, +4h and +5h.
constexpr std::size_t kWanted = 0x4;
constexpr std::size_t kActive = 0x5;

} // namespace

const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept {
    for (const TrivialSlot& slot : kTrivialSlots) {
        if (slot.address == address) return &slot;
    }
    return nullptr;
}

void collect_children(const CollectChildrenSlot& slot, Raw screen, void* out, HudScreenServices& host) {
    for (std::uint8_t i = 0; i < slot.count; ++i) {
        host.push_page_slot_004d6790(out, &screen.ptr(slot.page_offsets[i]));
    }
}

// ---------------------------------------------------------------------------
// Tactical sheet (54h)
// ---------------------------------------------------------------------------

// 0067AF40, __thiscall(screen), returns screen. The listener sub-object at +8h
// first gets the listbox-listener base vtable, then the screen's own.
Raw tactical_sheet_construct_0067af40(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = kTacticalSheetVtable;
    s.at<std::uint32_t>(0x8) = kTacticalSheetListenerVtable;
    return s;
}

// 0067AF80, slot +4h: self-managed only in single player (session mode 0).
bool tactical_sheet_self_managed_0067af80(HudScreenHost& host) {
    return host.session_mode() == 0;
}

// 0067AFA0, exit: close the screen held at 00E1930C. If it is active its exit
// (+1Ch) runs first; then both flag bytes clear and visibility is committed
// with ECX = that screen (004F83B0).
void tactical_sheet_exit_0067afa0(HudScreenHost& host) {
    Raw linked = host.global_object(kTacticalSheetLinkedScreen);
    if (linked.at<std::uint8_t>(kActive) != 0) host.object_virtual(linked, 0x1C);
    linked.at<std::uint8_t>(kWanted) = 0;
    linked.at<std::uint8_t>(kActive) = 0;
    host.front_end_screen_commit_visibility_004f83b0(linked);
}

// 0067AFD0, __fastcall(screen): un-want the sheet. In a session (mode != 0) the
// multiplayer menu pops its interface request; otherwise the interface's
// screen at +D8h is wanted and activated at once, committed, and entered
// (+18h). Both paths finish with an input-manager update of 0.0 seconds.
void tactical_sheet_close_0067afd0(Raw s, HudScreenHost& host) {
    s.at<std::uint8_t>(kWanted) = 0;
    if (host.session_mode() != 0) {
        host.multi_menu_pop_request_006881f0();
        host.input_manager_update_00a92c40(0.0f);
        return;
    }
    Raw resume = host.interface_object(kInterfaceResumeScreen);
    resume.at<std::uint8_t>(kWanted) = 1;
    resume.at<std::uint8_t>(kActive) = 1;
    host.front_end_screen_commit_visibility_004f83b0(resume);
    host.object_virtual(resume, 0x18);
    host.input_manager_update_00a92c40(0.0f);
}

// 0067B040, destructor: restore the vtables, drop the page at +0Ch, then the
// listener base vtable and the front-end base destructor.
void tactical_sheet_destruct_0067b040(Raw s, HudScreenHost& host) {
    s.at<std::uint32_t>(0x0) = kTacticalSheetVtable;
    s.at<std::uint32_t>(0x8) = kTacticalSheetListenerVtable;
    remove_page(s, 0xC, host);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

// 0067B0B0, update(float), RET 4: either close action (4Ah, 4Bh) closes it.
void tactical_sheet_update_0067b0b0(Raw s, float /*seconds*/, HudScreenHost& host) {
    if (host.action_edge_or_repeat_004d92b0(kActionTacticalSheetCloseA)
        || host.action_edge_or_repeat_004d92b0(kActionTacticalSheetCloseB)) {
        tactical_sheet_close_0067afd0(s, host);
    }
}

// 0067B0E0, listener slot (ECX = screen+8h, two stack args, RET 8): ADD ECX,-8
// then the close.
void tactical_sheet_listbox_activate_0067b0e0(Raw listener, HudScreenHost& host) {
    tactical_sheet_close_0067afd0(Raw{listener.p - 0x8}, host);
}

// 0067B110, register: base register, load the page into +0Ch, hide it.
void tactical_sheet_register_0067b110(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0xC) = host.gui_load_page_00aa5840(kTacticalSheetPage);
    host.widget_set_visible_34(s.ptr(0xC), false);
}

// 0067B1D0, enter: the page's Main_Listbox gets the listener at +8h, is shown
// and focused (+34h(1), +60h(1)).
void tactical_sheet_enter_0067b1d0(Raw s, HudScreenHost& host) {
    void* listbox = host.gui_find_child_00aa7e00(s.ptr(0xC), kTacticalSheetListbox);
    host.listbox_set_listener_00a9ac40(listbox, s.p + 0x8);
    host.widget_set_visible_34(listbox, true);
    host.widget_set_focus_60(listbox, true);
}

// ---------------------------------------------------------------------------
// Cursor (28h)
// ---------------------------------------------------------------------------

// 0067B300.
Raw cursor_screen_construct_0067b300(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x0) = kCursorScreenVtable;
    return s;
}

// 0067B780.
void cursor_screen_destruct_0067b780(Raw s, HudScreenHost& host) {
    s.at<std::uint32_t>(0x0) = kCursorScreenVtable;
    remove_page(s, 0x8, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// 0067BCB0: base register, page "CursorGui" into +8h (left visible).
void cursor_screen_register_0067bcb0(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x8) = host.gui_load_page_00aa5840(kCursorPage);
}

// 0067BD70: "Cursor_Icon" of the page into +0Ch.
void cursor_screen_enter_0067bd70(Raw s, HudScreenHost& host) {
    s.ptr(0xC) = host.gui_find_child_00aa7e00(s.ptr(0x8), kCursorIcon);
}

// ---------------------------------------------------------------------------
// Follow screen (49h)
// ---------------------------------------------------------------------------

// 0067BE30.
Raw follow_screen_construct_0067be30(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x0) = kFollowScreenVtable;
    s.ptr(0x8) = nullptr;
    return s;
}

// 0067BE80: base register, then the screen's +14h virtual (0067BEA0, empty).
void follow_screen_register_0067be80(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    host.screen_virtual_14(s);
}

// 0067BEC0, exit.
void follow_screen_exit_0067bec0(Raw s) noexcept {
    s.ptr(0x8) = nullptr;
}

// ---------------------------------------------------------------------------
// Binoculars overlay (4Ah)
// ---------------------------------------------------------------------------

// 0067C430: base register; the page is loaded and hidden but not kept (the
// image never stores the returned pointer); +20h..+2Ch cleared.
void binoculars_overlay_register_0067c430(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    host.widget_set_visible_34(host.gui_load_page_00aa5840(kBinocularsPage), false);
    s.ptr(0x2C) = nullptr;
    s.ptr(0x28) = nullptr;
    s.ptr(0x24) = nullptr;
    s.at<std::uint8_t>(0x20) = 0;
}

// 0067C050, enter: bind the weapon-group HUD to (unit +24h, camera +2Ch) on
// *(*00E198C4 + 50h); observe the unit (member +8h) unless already observed;
// copy the unit's +980h float into +1Ch.
void binoculars_overlay_enter_0067c050(Raw s, HudScreenHost& host) {
    host.weapon_group_bind_unit_00549260(s.ptr(0x24), s.ptr(0x2C));
    void* unit = s.ptr(0x24);
    if (unit == nullptr) return;
    if (!host.observer_is_registered_00694af0(s.p + 0x8, unit)) host.observer_register_00694a60(s.p + 0x8, unit);
    s.at<float>(0x1C) = Raw{static_cast<std::uint8_t*>(s.ptr(0x24))}.at<float>(0x980);
}

// 0067C130, observer release (RET 4): clear whichever of +1Ch/+24h matches.
void binoculars_overlay_released_0067c130(Raw s, void* object) noexcept {
    if (object == s.ptr(0x1C)) {
        s.ptr(0x1C) = nullptr;
        return;
    }
    if (object == s.ptr(0x24)) s.ptr(0x24) = nullptr;
}

// 0067C0E0, __thiscall(screen, float), RET 4: binoculars view update, then with a
// camera (+2Ch) recentre screen 2Eh on (0.5, 0.5).
void binoculars_update_view_recentre_0067c0e0(Raw s, float seconds, HudScreenHost& host) {
    host.binoculars_update_view_0051f330(seconds);
    if (s.ptr(0x2C) != nullptr) host.screen2e_set_view_centre_005454b0(0.5f, 0.5f, host.interface_object(0x4C).at<float>(0x38));  // 00CE3800
}

// ---------------------------------------------------------------------------
// Bomb/torpedo view (24h)
// ---------------------------------------------------------------------------

// 0067B3A0 (RET 4).
void bomb_view_released_0067b3a0(Raw s, void* object) noexcept {
    if (object == s.ptr(0x44)) {
        s.ptr(0x44) = nullptr;
        return;
    }
    if (object == s.ptr(0x48)) s.ptr(0x48) = nullptr;
}

// 0067B3D0, __thiscall(screen, a, b), RET 8.
void bomb_view_queue_selection_0067b3d0(Raw s, std::uint32_t a, std::uint32_t b) noexcept {
    s.at<std::uint32_t>(0x40) = a;
    s.at<std::uint32_t>(0x44) = b;
    s.at<std::uint8_t>(0x48) = 1;
}

// 0067B450, exit: kill both camera movers (00926D90(2)), apply the pending
// interface level change, restore the saved transition with a zero offset.
void bomb_view_exit_0067b450(Raw s, HudScreenHost& host) {
    if (s.ptr(0x4C) != nullptr) {
        host.mission_entity_kill_00926d90(s.ptr(0x4C));
        s.ptr(0x4C) = nullptr;
    }
    if (s.ptr(0x50) != nullptr) {
        host.mission_entity_kill_00926d90(s.ptr(0x50));
        s.ptr(0x50) = nullptr;
    }
    host.interface_apply_pending_level_change_0068a9c0();
    host.render_set_transition_00b0d6a0(s.at<float>(0x1C), s.at<float>(0x20), s.at<float>(0x24), 0.0f, 0.0f, 0.0f);
}

// 0067BAA0, enter: zero +38h/+3Ch (and the bomb camera's +3F0h/+3F4h), save
// the current transition into +1Ch..+24h, install the tuning default with a
// zero offset, publish the controlled listener, stamp +54h with the clock.
void bomb_view_enter_0067baa0(Raw s, HudScreenHost& host) {
    s.at<float>(0x38) = 0.0f;
    s.at<float>(0x3C) = 0.0f;
    if (void* camera = s.ptr(0x4C)) {
        Raw c{static_cast<std::uint8_t*>(camera)};
        c.at<float>(0x3F0) = 0.0f;
        c.at<float>(0x3F4) = 0.0f;
    }
    host.render_get_transition_00b0cfc0(s.at<float>(0x1C), s.at<float>(0x20), s.at<float>(0x24));
    const float e0 = host.game_tuning_float(0xE0);
    const float dc = host.game_tuning_float(0xDC);
    const float d8 = host.game_tuning_float(0xD8);
    host.render_set_transition_00b0d6a0(d8, dc, e0, 0.0f, 0.0f, 0.0f);
    host.controlled_listener_publish_004bca80();
    s.at<float>(0x54) = host.real_clock_00f876a4();
}

// 0067BC60 (RET 4): the observer member is +14h, its observed pointer +28h.
void common_overlay_released_0067bc60(Raw s, void* object, HudScreenHost& host) {
    if (object != s.ptr(0x28)) return;
    if (s.ptr(0x28) == nullptr) return;
    host.observer_unregister_006952a0(s.p + 0x14, s.ptr(0x28));
    s.ptr(0x28) = nullptr;
}

// ---------------------------------------------------------------------------
// Spectator (43h)
// ---------------------------------------------------------------------------

// 00643DA0.
Raw spectator_construct_00643da0(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint8_t>(0x14) = 0;
    s.ptr(0x10) = nullptr;
    s.ptr(0xC) = nullptr;
    s.at<std::uint32_t>(0x0) = kSpectatorScreenVtable;
    return s;
}

// 00643E00.
void spectator_destruct_00643e00(Raw s, HudScreenHost& host) {
    s.at<std::uint32_t>(0x0) = kSpectatorScreenVtable;
    remove_page(s, 0x8, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// 00643E80: base register, page "GUI_spectator" into +8h, then the screen is
// wanted at once (+4h = 1).
void spectator_register_00643e80(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x8) = host.gui_load_page_00aa5840("GUI_spectator");
    s.at<std::uint8_t>(kWanted) = 1;
}

// 00643F40, enter: "Score_Text" into +0Ch, "Spectator_Text" into +10h (hidden).
// The find calls take the page at +8h.
void spectator_enter_00643f40(Raw s, HudScreenHost& host) {
    s.ptr(0xC) = host.gui_find_child_00aa7e00(s.ptr(0x8), "Score_Text");
    s.ptr(0x10) = host.gui_find_child_00aa7e00(s.ptr(0x8), "Spectator_Text");
    host.widget_set_visible_34(s.ptr(0x10), false);
}

// ---------------------------------------------------------------------------
// Scene init (5Ah)
// ---------------------------------------------------------------------------

// 00636D80, exit: clear the wanted byte of the screen at 00E19698.
void scene_init_exit_00636d80(HudScreenHost& host) {
    host.global_object(kSceneInitScreenGlobal).at<std::uint8_t>(kWanted) = 0;
}

// 00636D90.
Raw scene_init_construct_00636d90(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kCallbackOwnerBaseVtable;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x0) = kSceneInitVtable;
    s.at<std::uint32_t>(0x8) = kSceneInitCallbackVtable;
    return s;
}

// 00636E00: the page at +1Ch goes first, then the callback owner at +8h
// (00695870) and the base destructor.
void scene_init_destruct_00636e00(Raw s, HudScreenHost& host) {
    s.at<std::uint32_t>(0x0) = kSceneInitVtable;
    s.at<std::uint32_t>(0x8) = kSceneInitCallbackVtable;
    remove_page(s, 0x1C, host);
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

// The alpha of 00636E80: float(2t) is stored, its x87 FSIN stored to float,
// the sign cleared, then * 0.75 + 0.25 in extended precision and rounded.
float scene_init_pulse_alpha(float clock) noexcept {
    const float twice = clock + clock;
    const float s = static_cast<float>(std::sin(static_cast<double>(twice)));
    return static_cast<float>(std::fabs(static_cast<double>(s)) * kSceneInitPulseScale + kSceneInitPulseBase);
}

// 00636E80, update(float), RET 4: re-colour Message_Text (+20h, vtable +50h)
// with the colour saved at +24h..+30h and a pulsing alpha, then the autotest
// step 0Bh.
void scene_init_update_00636e80(Raw s, float /*seconds*/, HudScreenHost& host) {
    float rgba[4] = {s.at<float>(0x24), s.at<float>(0x28), s.at<float>(0x2C), s.at<float>(0x30)};
    rgba[3] = scene_init_pulse_alpha(host.game_clock_064c());
    host.widget_set_colour_50(s.ptr(0x20), rgba);
    host.auto_test_step_0b();
}

// 00636F30: base register, page "FE_sceneinit" into +1Ch, "Message_Text" into
// +20h, its colour (vtable +54h, four floats) into +24h..+30h, page hidden.
void scene_init_register_00636f30(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x1C) = host.gui_load_page_00aa5840("FE_sceneinit");
    s.ptr(0x20) = host.gui_find_child_00aa7e00(s.ptr(0x1C), "Message_Text");
    float colour[4]{};
    host.widget_get_colour_54(s.ptr(0x20), colour);
    s.at<float>(0x24) = colour[0];
    s.at<float>(0x28) = colour[1];
    s.at<float>(0x2C) = colour[2];
    s.at<float>(0x30) = colour[3];
    host.widget_set_visible_34(s.ptr(0x1C), false);
}

// 0067C0A0, binoculars exit: release the weapon-group HUD, give the unit's
// roles back (0077C470(1FEh, 0), a cc9 routine), reset the binoculars.
void binoculars_overlay_exit_0067c0a0(Raw s, HudScreenHost& host) {
    host.weapon_group_release_005470c0();
    if (s.ptr(0x24) != nullptr) host.unit_send_role_transfer_0077c470(s.ptr(0x24), 0x1FE, 0);
    host.binoculars_reset_0051e8e0();
}

// ---------------------------------------------------------------------------
// Submarine (48h) and periscope (47h)
// ---------------------------------------------------------------------------

// 00650140.
Raw submarine_construct_00650140(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x0) = kSubmarineScreenVtable;
    return s;
}

// 00650170: base register, the +14h virtual (00650B90, the layout), then +1Ch
// byte and +20h float cleared.
void submarine_register_00650170(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    host.screen_virtual_14(s);
    s.at<std::uint8_t>(0x1C) = 0;
    s.at<float>(0x20) = 0.0f;
}

// 00650910.
void submarine_destruct_00650910(Raw s, HudScreenHost& host) {
    s.at<std::uint32_t>(0x0) = kSubmarineScreenVtable;
    remove_page(s, 0x8, host);
    host.front_end_screen_destruct_004f71a0(s);
}

// 006502D0 (RET 4): the unit (+1Ch, with its +20h companion) or +24h.
void periscope_released_006502d0(Raw s, void* object) noexcept {
    if (object == s.ptr(0x1C)) {
        s.ptr(0x1C) = nullptr;
        s.ptr(0x20) = nullptr;
        return;
    }
    if (object == s.ptr(0x24)) s.ptr(0x24) = nullptr;
}

// 00650810, exit: detach the unit (006506E0), lens flare off (ECX = 00E081A0).
void periscope_exit_00650810(Raw s, HudScreenHost& host) {
    host.periscope_detach_unit_006506e0(s);
    host.lens_flare_set_enabled_00452b80(false);
}

// 00650B00.
Raw periscope_construct_00650b00(Raw s, HudScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kCallbackOwnerBaseVtable;
    for (std::size_t off : {0xCu, 0x10u, 0x14u}) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    for (std::size_t off : {0x24u, 0x28u, 0x2Cu}) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint32_t>(0x0) = kPeriscopeScreenVtable;
    s.at<std::uint32_t>(0x8) = kPeriscopeCallbackVtable;
    for (std::size_t off : {0x30u, 0x38u, 0x3Cu, 0x40u, 0x4Cu, 0x50u, 0x20u, 0x70u, 0x54u, 0x58u, 0x44u}) {
        s.at<std::uint32_t>(off) = 0;
    }
    return s;
}

// ---------------------------------------------------------------------------
// HUD markers (4Dh)
// ---------------------------------------------------------------------------

void hud_markers_set_view_centre_00637620(Raw s, float x, float y) noexcept {
    s.at<float>(0x44) = x;
    s.at<float>(0x48) = y;
}

// 006374B0: +4h/+5h bytes cleared, then 6 rounds of 4 widgets from +8h and the
// seven at +68h..+80h, each hidden (vtable +34h(0)).
void hud_marker_entry_hide_all_006374b0(Raw entry, HudScreenHost& host) {
    entry.at<std::uint8_t>(0x4) = 0;
    entry.at<std::uint8_t>(0x5) = 0;
    for (std::size_t i = 0; i < 24; ++i) host.widget_set_visible_34(entry.ptr(0x8 + i * 4), false);
    for (std::size_t off = 0x68; off <= 0x80; off += 4) host.widget_set_visible_34(entry.ptr(off), false);
}

void hud_colour_icon_set_hide_all_00637540(Raw set, HudScreenHost& host) {
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x14u, 0x10u}) host.widget_set_visible_34(set.ptr(off), false);
}

void hud_marker_endpoints_00638b20(Raw marker, float out_a[3], float out_b[3]) noexcept {
    const float* a = static_cast<const float*>(marker.ptr(0x20));
    out_a[0] = a[0];
    out_a[1] = a[1];
    const float* b = static_cast<const float*>(marker.ptr(0x24));
    out_a[2] = a[2];
    out_b[0] = b[0];
    out_b[1] = b[1];
    out_b[2] = b[2];
}

namespace {
void delete_and_clear(Raw object, std::size_t off, HudScreenHost& host) {
    if (object.ptr(off) != nullptr) {
        host.object_delete_04(object.ptr(off));
        object.ptr(off) = nullptr;
    }
}
} // namespace

// 006391C0: delete the clones at +4h, +8h, +0Ch, +14h, +10h in that order.
void hud_colour_icon_set_destruct_006391c0(Raw set, HudScreenHost& host) {
    set.at<std::uint32_t>(0x0) = kHudColourIconSetVtable;
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x14u, 0x10u}) delete_and_clear(set, off, host);
}

// 00639450 / 00639960: one clone at +4h.
void hud_name_marker_destruct_00639450(Raw marker, HudScreenHost& host) {
    marker.at<std::uint32_t>(0x0) = kHudNameMarkerVtable;
    delete_and_clear(marker, 0x4, host);
}
void hud_befogo_marker_destruct_00639960(Raw marker, HudScreenHost& host) {
    marker.at<std::uint32_t>(0x0) = kHudBefogoMarkerVtable;
    delete_and_clear(marker, 0x4, host);
}

// 0063E1C0: the NameMarker_Text clone is kept as it is.
Raw hud_name_marker_construct_0063e1c0(Raw marker, void* page, HudScreenHost& host, HudMarkerCloneHost& clone) {
    marker.at<std::uint32_t>(0x0) = kHudNameMarkerVtable;
    void* child = host.gui_find_child_00aa7e00(page, kHudNameMarkerText);
    marker.ptr(0x4) = clone.clone_subtree_00aab4c0(child, 0);
    return marker;
}

// 0063E950: the BefogoMarker_Icon clone is shown (vtable +34h(1)).
Raw hud_befogo_marker_construct_0063e950(Raw marker, void* page, HudScreenHost& host, HudMarkerCloneHost& clone) {
    marker.at<std::uint32_t>(0x0) = kHudBefogoMarkerVtable;
    void* child = host.gui_find_child_00aa7e00(page, kHudBefogoMarkerIcon);
    marker.ptr(0x4) = clone.clone_subtree_00aab4c0(child, 0);
    host.widget_set_visible_34(marker.ptr(0x4), true);
    return marker;
}

// 00640C80: the four marker-pool vectors (+34h, +4Ch, +5Ch, +6Ch: their
// begin/end/capacity words cleared), the +90h vector, and the map at +A4h whose
// head node (0063B110) is self-linked with its +11h "is nil" byte set.
Raw hud_markers_construct_00640c80(Raw s, HudScreenHost& host, void* tree_head) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kCallbackOwnerBaseVtable;
    for (std::size_t off : {0xCu, 0x10u, 0x14u}) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x0) = kHudMarkersVtable;
    s.at<std::uint32_t>(0x8) = kHudMarkersCallbackVtable;
    for (std::size_t off : {0x38u, 0x3Cu, 0x40u, 0x50u, 0x54u, 0x58u, 0x60u, 0x64u, 0x68u,
                            0x70u, 0x74u, 0x78u, 0x94u, 0x98u, 0x9Cu}) {
        s.at<std::uint32_t>(off) = 0;
    }
    s.ptr(0xA8) = tree_head;
    Raw head{static_cast<std::uint8_t*>(tree_head)};
    head.at<std::uint8_t>(0x11) = 1;
    head.ptr(0x4) = tree_head;
    head.ptr(0x0) = tree_head;
    head.ptr(0x8) = tree_head;
    s.at<std::uint32_t>(0xAC) = 0;
    return s;
}

// ---------------------------------------------------------------------------
// Observer references
// ---------------------------------------------------------------------------

Raw observer_ref_set(Raw ref, void* observed, HudScreenHost& host) {
    if (ref.ptr(0x14) != observed) {
        if (ref.ptr(0x14) != nullptr) host.observer_unregister_006952a0(ref.p, ref.ptr(0x14));
        ref.ptr(0x14) = observed;
        if (observed != nullptr) host.observer_register_00694a60(ref.p, observed);
    }
    return ref;
}

void observer_ref_released(Raw ref, void* notifier, ObserverReleaseHost& query, HudScreenHost& host) {
    if (query.released_object_04(notifier) == ref.ptr(0x14)) {
        void* old = ref.ptr(0x14);
        ref.ptr(0x14) = nullptr;
        host.observer_unregister_006952a0(ref.p, old);
    }
}

// ---------------------------------------------------------------------------
// Ship view (46h)
// ---------------------------------------------------------------------------

// 0064A360: base register, +20h camera cleared.
void ship_view_register_0064a360(Raw s, HudScreenHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x20) = nullptr;
}

// 0064A450, enter: bind the weapon-group HUD to (unit +1Ch, camera +20h) and
// observe the unit through the member at +8h.
void ship_view_enter_0064a450(Raw s, HudScreenHost& host) {
    host.weapon_group_bind_unit_00549260(s.ptr(0x1C), s.ptr(0x20));
    void* unit = s.ptr(0x1C);
    if (unit != nullptr && !host.observer_is_registered_00694af0(s.p + 0x8, unit)) {
        host.observer_register_00694a60(s.p + 0x8, unit);
    }
}

// 0064A490, exit: release the weapon-group HUD and the unit's roles.
void ship_view_exit_0064a490(Raw s, HudScreenHost& host) {
    host.weapon_group_release_005470c0();
    if (s.ptr(0x1C) != nullptr) host.unit_send_role_transfer_0077c470(s.ptr(0x1C), 0x1FE, 0);
}

// 0064A4C0 (RET 4).
void ship_view_released_0064a4c0(Raw s, void* object) noexcept {
    if (object == s.ptr(0x14)) {
        s.ptr(0x14) = nullptr;
        return;
    }
    if (object == s.ptr(0x18)) s.ptr(0x18) = nullptr;
}

void hud_widget_quad_fade_construct_0064a4f0(Raw fade) noexcept {
    for (std::size_t off = 0; off < 0x20; off += 4) fade.at<std::uint32_t>(off) = 0;
    fade.at<std::uint32_t>(0x20) = 1;
}

void hud_widget_quad_fade_bind_0064a540(Raw fade, void* w0, void* w1, void* w2, void* w3, HudScreenHost& host) {
    fade.ptr(0x4) = w1;
    fade.ptr(0xC) = w3;
    fade.ptr(0x0) = w0;
    fade.ptr(0x8) = w2;
    for (std::size_t off : {0x0u, 0x4u, 0x8u, 0xCu}) host.widget_set_visible_34(fade.ptr(off), false);
}

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

bool profile_has_pending_awards_0061eb80(Raw profile) noexcept {
    const std::uint8_t* begin = static_cast<const std::uint8_t*>(profile.ptr(0xC0));
    if (begin == nullptr) return false;
    const std::uint8_t* end = static_cast<const std::uint8_t*>(profile.ptr(0xC4));
    return (end - begin) / 0x1C != 0;
}

void rewards_recapture_scrollers_00626720(Raw s, HudScreenHost& host) {
    host.scroller_recapture_006833f0(Raw{s.p + 0x5C});
    host.scroller_recapture_006833f0(Raw{s.p + 0x190});
    host.scroller_recapture_006833f0(Raw{s.p + 0x11C});
}

// 00645F10: reset the selection state, empty the +70h string, base register,
// clear +1Ch/+CCh, drop the interface observer (member +D0h, observed +E4h),
// +BCh = 0.0f, the layout virtual (+14h, 006463E0), +88h = 1.0f, hide +2Ch.
void hud_root_register_00645f10(Raw s, HudScreenHost& host) {
    s.at<std::uint8_t>(0xC0) = 0;
    s.at<std::uint32_t>(0x78) = 0;
    s.at<std::uint8_t>(0x20) = 0;
    s.at<std::uint32_t>(0x6C) = 0;
    host.native_string_assign_empty(Raw{s.p + 0x70});
    host.front_end_screen_register_004f71d0(s);
    s.at<std::uint32_t>(0x1C) = 0;
    s.at<std::uint32_t>(0xCC) = 0;
    if (s.ptr(0xE4) != nullptr) {
        host.observer_unregister_006952a0(s.p + 0xD0, s.ptr(0xE4));
        s.ptr(0xE4) = nullptr;
    }
    s.at<float>(0xBC) = 0.0f;
    host.screen_virtual_14(s);
    s.at<float>(0x88) = 1.0f;  // 00D7A24C
    host.widget_set_visible_34(s.ptr(0x2C), false);
}

bool support_manager_is_mode1_006529f0(Raw s) noexcept { return s.at<std::int32_t>(0x5C) == 1; }
bool support_manager_is_mode2_00652a00(Raw s) noexcept { return s.at<std::int32_t>(0x5C) == 2; }

void support_manager_delete_slot_clones_00652a60(Raw s, HudScreenHost& host) {
    for (std::size_t i = 0; i < 4; ++i) {
        if (s.ptr(0x7C + i * 4) != nullptr) host.object_delete_04(s.ptr(0x7C + i * 4));
        if (s.ptr(0xAC + i * 4) != nullptr) host.object_delete_04(s.ptr(0xAC + i * 4));
    }
}

void support_manager_show_arrows_f8_00652aa0(Raw s, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(0xF8), true);
    host.widget_set_visible_34(s.ptr(0xFC), true);
}
void support_manager_hide_arrows_f8_00652ad0(Raw s, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(0xF8), false);
    host.widget_set_visible_34(s.ptr(0xFC), false);
}
void support_manager_show_arrows_100_00652b00(Raw s, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(0x100), true);
    host.widget_set_visible_34(s.ptr(0x104), true);
}
void support_manager_hide_arrows_100_00652b30(Raw s, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(0x100), false);
    host.widget_set_visible_34(s.ptr(0x104), false);
}
void powerup_manager_hide_info_00652b60(Raw s, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(0x16C), false);
}

std::string_view ship_captain_mode_name_0064b6b0(Raw camera, UnitKindHost& kinds) {
    return kinds.unit_is_kind_5c(camera.ptr(0x3AC), 0xE) ? "PTBoat" : "ShipCaptain";
}

void* ship_captain_unit_0064b6d0(Raw camera) noexcept {
    if (camera.at<std::uint8_t>(0x400) != 0) return nullptr;
    return camera.ptr(0x3AC);
}

void ship_screen_clear_timed_entry_00644240(Raw s, HudScreenHost& host) {
    if (s.ptr(0x3C) == nullptr) return;
    host.widget_delete_timed_entry_00aa8b80(s.ptr(0x3C), 0);
    host.widget_set_float_4c(s.ptr(0x3C), 0.0f);
}

// ---------------------------------------------------------------------------
// Rewards screen (04h)
// ---------------------------------------------------------------------------

namespace {
// The refcounted handle release the rewards screen inlines: InterlockedDecrement
// of +4h, vtable slot 0 at zero.
void release_handle(Raw s, std::size_t off, RewardsScreenHost& host) {
    if (s.ptr(off) != nullptr) host.handle_release(s.ptr(off));
}
} // namespace

// 0061EC00, __fastcall(screen): stop the sound at +14h (vtable +8h(0)), release
// it and clear the field, then release +10h.
void rewards_release_sound_handles_0061ec00(Raw s, RewardsScreenHost& host) {
    if (s.ptr(0x14) != nullptr) host.sound_stop_08(s.ptr(0x14));
    release_handle(s, 0x14, host);
    s.ptr(0x14) = nullptr;
    release_handle(s, 0x10, host);
    s.ptr(0x10) = nullptr;
}

namespace {
// The three scroller blocks of the listener (screen+0Ch): their up, down and
// thumb widgets sit just before each block.
struct RewardsScroller {
    std::uint16_t up, down, thumb, block;
};
constexpr RewardsScroller kRewardsScrollers[3] = {
    {0xB8, 0xBC, 0xC0, 0x5C}, {0x110, 0x114, 0x118, 0x11C}, {0x184, 0x188, 0x18C, 0x190}};

void rewards_widget_tail(void* widget, std::int32_t pressed_state, RewardsScreenHost& host) {
    const std::int32_t type = host.widget_type_5c(widget);
    if (type == 6) {
        host.widget_set_state_88(widget, pressed_state, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_set_state_84(widget, pressed_state);
}
} // namespace

// 0061D770, listener slot +0Ch (ECX = screen+0Ch, widget), RET 4: the release
// of a widget. A type-3 button snaps its float (+4Ch) back to 0.5 when its
// +D4h byte is set, else 1.0. A scroller arrow stops auto-scroll (00683300(0,
// 0.0, 0)); a thumb ends its drag (00683320). Type 6 then gets +88h(lit, 0,
// 1.0) and type 12h +84h(lit), lit = the +D4h byte.
void rewards_widget_release_0061d770(Raw listener, void* widget, RewardsScreenHost& host) {
    if (host.widget_type_5c(widget) == 3) {
        const bool lit = Raw{static_cast<std::uint8_t*>(widget)}.at<std::uint8_t>(0xD4) != 0;
        host.widget_set_float_4c(widget, lit ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    for (const RewardsScroller& sc : kRewardsScrollers) {
        if (widget == listener.ptr(sc.up) || widget == listener.ptr(sc.down)) {
            host.scroller_set_auto_scroll_00683300(Raw{listener.p + sc.block}, 0, 0.0f, 0);
            break;
        }
        if (widget == listener.ptr(sc.thumb)) {
            host.scroller_end_thumb_drag_00683320(Raw{listener.p + sc.block});
            break;
        }
    }
    const bool lit = Raw{static_cast<std::uint8_t*>(widget)}.at<std::uint8_t>(0xD4) != 0;
    rewards_widget_tail(widget, lit ? 1 : 0, host);
}

// 0062B3B0, listener slot (ECX = screen+0Ch, widget), RET 4: the press. A
// type-3 button sets its float to 1.0, then dispatches on the first byte of its
// name (+F0h, "" at 00E196AC when null): A2h continues (0062B110), A5h shows
// the gamer card of the selected row, A7h the player review, both only on the
// multiplayer page (+4Ch == 1). Arrows start auto-scroll at 00CEC178 in their
// direction, thumbs begin a drag (00683A40), +54h/+50h step the page by +1/-1.
// Types 6 and 12h then take state 2.
void rewards_widget_press_0062b3b0(Raw listener, void* widget, RewardsScreenHost& host) {
    Raw screen{listener.p - 0xC};
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_float_4c(widget, 1.0f);
        const std::uint8_t key = host.widget_name_first_byte_f0(widget);
        if (key == 0xA2) {
            host.rewards_continue_0062b110(screen);
            return;
        }
        if (key == 0xA5) {
            if (listener.at<std::int32_t>(0x4C) != 1) return;
            if (!host.rewards_selected_entry_flagged_0061d8b0(screen)) return;
            host.show_gamer_card_00a4d440(host.selected_row_xuid_0061d710(screen));
            return;
        }
        if (key != 0xA7) return;
        if (listener.at<std::int32_t>(0x4C) != 1) return;
        if (!host.online_signed_in_198()) return;
        const std::uint64_t xuid = host.selected_row_xuid_0061d710(screen);
        if (xuid == host.active_xuid_00a3eb00()) return;
        host.show_player_review_00a4d470(host.selected_row_xuid_0061d710(screen));
        return;
    }
    const float speed = kRewardsAutoScrollSpeed;
    bool handled = false;
    for (const RewardsScroller& sc : kRewardsScrollers) {
        if (widget == listener.ptr(sc.up)) {
            host.scroller_set_auto_scroll_00683300(Raw{listener.p + sc.block}, 1, speed, 1);
            handled = true;
            break;
        }
        if (widget == listener.ptr(sc.down)) {
            host.scroller_set_auto_scroll_00683300(Raw{listener.p + sc.block}, 2, speed, 1);
            handled = true;
            break;
        }
        if (widget == listener.ptr(sc.thumb)) {
            host.scroller_begin_thumb_drag_00683a40(Raw{listener.p + sc.block});
            handled = true;
            break;
        }
        // The page arrows are tested after the first scroller only.
        if (&sc == &kRewardsScrollers[0]) {
            if (widget == listener.ptr(0x54)) {
                host.rewards_step_page_00622e30(screen, 1);
                handled = true;
                break;
            }
            if (widget == listener.ptr(0x50)) {
                host.rewards_step_page_00622e30(screen, -1);
                handled = true;
                break;
            }
        }
    }
    (void)handled;
    rewards_widget_tail(widget, 2, host);
}

// 00621720, exit: hide the six widgets +40h..+54h, clear game+610h, reset the
// mission player records (00916980, ECX = *(game+21A0h)), delete every object
// of the vector +F8h (begin +FCh, end +100h) and clear it, then the sound
// release of 0061EC00 (inlined).
void rewards_exit_00621720(Raw s, RewardsScreenHost& host) {
    for (std::size_t off = 0x40; off < 0x58; off += 4) {
        if (s.ptr(off) != nullptr) host.widget_set_visible_34(s.ptr(off), false);
    }
    host.game_byte(0x610) = 0;
    host.mission_player_records_reset_00916980();
    void** begin = static_cast<void**>(s.ptr(0xFC));
    for (std::size_t i = 0; begin != nullptr && i < static_cast<std::size_t>(static_cast<void**>(s.ptr(0x100)) - begin); ++i) {
        if (begin[i] != nullptr) {
            host.object_delete_04(begin[i]);
            begin[i] = nullptr;
        }
        begin = static_cast<void**>(s.ptr(0xFC));
    }
    s.ptr(0x100) = s.ptr(0xFC);  // erase(begin, end): the tail move is empty
    rewards_release_sound_handles_0061ec00(s, host);
}

// 0062B110, __fastcall(screen): leave the scoring screen or advance its rank
// queue. With the rank page up (+258h) the first queued rank (vector +238h,
// 10h elements) is erased; while ranks remain the next one is shown (0062A970);
// otherwise the page closes (+25Ch hidden, +D4h refocused, page +58h refreshed
// through 00621C60). Leaving: game+610h cleared; from the mission menu
// (*00E198AC +3Ch) the selection is published (00580940, ECX = +58h of it) and
// interface request 1 pushed. Otherwise the multiplayer menu pops its request
// and, when the online service reports signed in (+198h) and its +4Dh byte is
// set: in a session of type 2 (game+1FE8h) it leaves the match (+180h ->
// +170h(1), +90h -> +8Ch), sets the session mode to 2 (0076FAD0 on
// game+1EF0h), pushes multiplayer request 1Ah and sets 00E19610 = 2; in type 1
// it clears 00E08700 and leaves the match likewise.
void rewards_continue_0062b110(Raw s, RewardsScreenHost& host) {
    if (s.at<std::uint8_t>(0x258) != 0) {
        host.rewards_erase_first_rank_006258a0(Raw{s.p + 0x238});
        const auto* begin = static_cast<const std::uint8_t*>(s.ptr(0x23C));
        if (begin != nullptr && (static_cast<const std::uint8_t*>(s.ptr(0x240)) - begin) / 0x10 != 0) {
            host.rewards_fill_rank_page_0062a970(s);
            return;
        }
        s.at<std::uint8_t>(0x258) = 0;
        host.widget_set_visible_34(s.ptr(0x25C), false);
        host.widget_set_focus_60(s.ptr(0xD4), true);
        host.rewards_refresh_page_00621c60(s, s.at<std::int32_t>(0x58));
        return;
    }
    host.game_byte(0x610) = 0;
    if (host.mission_menu_active_3c()) {
        host.mission_menu_publish_selection_00580940();
        host.push_interface_request_004cc460(1, 0);
        return;
    }
    const bool signed_in = host.online_signed_in_198();
    const bool online_flag = host.online_byte_4d() != 0;
    const std::int32_t session_type = host.game_int(0x1FE8);
    host.multi_menu_pop_request_006881f0();
    if (!signed_in || !online_flag) return;
    if (session_type == 2) {
        if (host.online_virtual_bool(0x180)) host.online_virtual_arg(0x170, 1);
        if (host.online_virtual_bool(0x90)) host.online_virtual_void(0x8C);
        host.session_set_mode_0076fad0(2);
        host.multi_menu_push_request_00688ad0(0x1A);
        host.global_int(0x00E19610) = 2;
        return;
    }
    if (session_type == 1) {
        host.global_byte(0x00E08700) = 0;
        if (host.online_virtual_bool(0x180)) host.online_virtual_arg(0x170, 1);
    }
}

// ---------------------------------------------------------------------------
// Native-string records
// ---------------------------------------------------------------------------

namespace {
void assign_string(Raw dst, std::size_t dst_off, Raw src, std::size_t src_off, NativeStringHost& host) {
    if (dst.p + dst_off != src.p + src_off) host.assign(Raw{dst.p + dst_off}, Raw{src.p + src_off});
}
void construct_string(Raw dst, std::size_t dst_off, Raw src, std::size_t src_off, NativeStringHost& host) {
    dst.at<std::uint32_t>(dst_off) = 0;
    dst.at<std::uint32_t>(dst_off + 4) = 0;
    assign_string(dst, dst_off, src, src_off, host);
}
} // namespace

void native_string_release_at(Raw object, std::size_t off, NativeStringHost& host) {
    if (object.ptr(off + 4) != nullptr) host.release(Raw{object.p + off});
}

void string_pair_destroy(Raw pair, NativeStringHost& host) {
    native_string_release_at(pair, 0x8, host);
    native_string_release_at(pair, 0x0, host);
}

void rewards_score_row_destroy_0061f2c0(Raw row, NativeStringHost& host) {
    native_string_release_at(row, 0x10, host);
    native_string_release_at(row, 0x8, host);
}

Raw profile_bonus_record_assign_0061f4a0(Raw dst, Raw src, NativeStringHost& host) {
    assign_string(dst, 0x0, src, 0x0, host);
    assign_string(dst, 0x8, src, 0x8, host);
    assign_string(dst, 0x10, src, 0x10, host);
    dst.at<std::uint32_t>(0x18) = src.at<std::uint32_t>(0x18);
    return dst;
}

namespace {
void score_row_copy_tail(Raw dst, Raw src) {
    dst.at<std::uint32_t>(0x18) = src.at<std::uint32_t>(0x18);
    std::memcpy(dst.p + 0x20, src.p + 0x20, 0xE * 4);  // +20h..+57h; +1Ch is not copied
    dst.at<std::uint32_t>(0x58) = src.at<std::uint32_t>(0x58);
    dst.at<std::uint8_t>(0x5C) = src.at<std::uint8_t>(0x5C);
}
} // namespace

Raw rewards_score_row_assign_0061f540(Raw dst, Raw src, NativeStringHost& host) {
    dst.at<std::uint32_t>(0x0) = src.at<std::uint32_t>(0x0);
    dst.at<std::uint32_t>(0x4) = src.at<std::uint32_t>(0x4);
    assign_string(dst, 0x8, src, 0x8, host);
    assign_string(dst, 0x10, src, 0x10, host);
    score_row_copy_tail(dst, src);
    return dst;
}

Raw rewards_score_row_copy_construct_00622790(Raw dst, Raw src, NativeStringHost& host) {
    dst.at<std::uint32_t>(0x0) = src.at<std::uint32_t>(0x0);
    dst.at<std::uint32_t>(0x4) = src.at<std::uint32_t>(0x4);
    construct_string(dst, 0x8, src, 0x8, host);
    construct_string(dst, 0x10, src, 0x10, host);
    score_row_copy_tail(dst, src);
    return dst;
}

Raw rewards_record10_copy_construct_00622870(Raw dst, Raw src, NativeStringHost& host) {
    dst.at<std::uint8_t>(0x0) = src.at<std::uint8_t>(0x0);
    dst.at<std::uint32_t>(0x4) = src.at<std::uint32_t>(0x4);
    construct_string(dst, 0x8, src, 0x8, host);
    return dst;
}

Raw string_pair_int_construct_00622f90(Raw dst, Raw first, Raw second_and_int, NativeStringHost& host) {
    construct_string(dst, 0x0, first, 0x0, host);
    construct_string(dst, 0x8, second_and_int, 0x0, host);
    dst.at<std::uint32_t>(0x10) = second_and_int.at<std::uint32_t>(0x8);
    return dst;
}

Raw string_pair_assign_00646230(Raw dst, Raw src, NativeStringHost& host) {
    assign_string(dst, 0x0, src, 0x0, host);
    assign_string(dst, 0x8, src, 0x8, host);
    return dst;
}

Raw string_pair_copy_construct_00646e20(Raw dst, Raw src, NativeStringHost& host) {
    construct_string(dst, 0x0, src, 0x0, host);
    construct_string(dst, 0x8, src, 0x8, host);
    return dst;
}

std::uint8_t* string_pair_copy_006462a0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* out, NativeStringHost& host) {
    for (; first != last; first += 0x10, out += 0x10) string_pair_assign_00646230(Raw{out}, Raw{first}, host);
    return out;
}

std::uint8_t* string_pair_copy_backward_00646350(std::uint8_t* first, std::uint8_t* last, std::uint8_t* out_end, NativeStringHost& host) {
    while (last != first) {
        last -= 0x10;
        out_end -= 0x10;
        string_pair_assign_00646230(Raw{out_end}, Raw{last}, host);
    }
    return out_end;
}

void string_pair_fill_00646f20(std::uint8_t* first, std::uint8_t* last, Raw value, NativeStringHost& host) {
    for (; first != last; first += 0x10) string_pair_assign_00646230(Raw{first}, value, host);
}

void fill_n_dwords_00646170(std::uint32_t* out, std::int32_t count, const std::uint32_t* value) noexcept {
    for (; count != 0; --count) *out++ = *value;
}

// 00625EC0 / 00647C90: destroy each element, free the block, zero the three
// words (+4h begin, +8h end, +0Ch capacity of the vector object).
void vector_tidy_destroy(Raw vector, std::size_t element_size, void (*destroy)(Raw, NativeStringHost&),
    NativeStringHost& host) {
    auto* begin = static_cast<std::uint8_t*>(vector.ptr(0x4));
    if (begin != nullptr) {
        auto* end = static_cast<std::uint8_t*>(vector.ptr(0x8));
        for (std::uint8_t* it = begin; it != end; it += element_size) destroy(Raw{it}, host);
        host.free_block(vector.ptr(0x4));
    }
    vector.ptr(0x4) = nullptr;
    vector.ptr(0x8) = nullptr;
    vector.ptr(0xC) = nullptr;
}

// ---------------------------------------------------------------------------
// Screen exits and destructors that free containers
// ---------------------------------------------------------------------------

namespace {
// Free a vector's block (+0h of the three words at `begin_off`) and zero them.
void free_vector_words(Raw s, std::size_t begin_off, ScreenTeardownHost& host) {
    if (s.ptr(begin_off) != nullptr) host.free_block(s.ptr(begin_off));
    s.ptr(begin_off) = nullptr;
    s.ptr(begin_off + 4) = nullptr;
    s.ptr(begin_off + 8) = nullptr;
}

// The exit pattern of 0063E360: delete every element through its slot 0
// (scalar deleting destructor, flag 1), null it, then erase(begin, end).
void delete_all_and_clear(Raw s, std::size_t begin_off, ScreenTeardownHost& host) {
    for (std::size_t i = 0;; ++i) {
        void** begin = static_cast<void**>(s.ptr(begin_off));
        if (begin == nullptr) break;
        if (i >= static_cast<std::size_t>(static_cast<void**>(s.ptr(begin_off + 4)) - begin)) break;
        if (begin[i] != nullptr) {
            host.object_delete_00(begin[i]);
            begin[i] = nullptr;
        }
    }
    s.ptr(begin_off + 4) = s.ptr(begin_off);
}
} // namespace

// 0063E360, markers exit: empty the four marker pools (+50h, +60h, +70h, +94h).
void hud_markers_exit_0063e360(Raw s, ScreenTeardownHost& host) {
    for (std::size_t off : {0x50u, 0x60u, 0x70u, 0x94u}) delete_all_and_clear(s, off, host);
}

// 006404A0, markers destructor: page +8Ch, the map at +A4h (erase all nodes
// with 0063FB30, free the head, zero head and size), the vectors +94h, +70h,
// +60h, +50h, +38h, the callback owner at +8h, the base.
void hud_markers_destruct_006404a0(Raw s, ScreenTeardownHost& host) {
    s.at<std::uint32_t>(0x0) = kHudMarkersVtable;
    s.at<std::uint32_t>(0x8) = kHudMarkersCallbackVtable;
    remove_page(s, 0x8C, host);
    host.tree_erase_all_0063fb30(Raw{s.p + 0xA4});
    host.free_block(s.ptr(0xA8));
    s.ptr(0xA8) = nullptr;
    s.at<std::uint32_t>(0xAC) = 0;
    for (std::size_t off : {0x94u, 0x70u, 0x60u, 0x50u, 0x38u}) free_vector_words(s, off, host);
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

// 00640D50: CALL 006404A0 (00640D53); TEST [ESP+8],1 / JE; PUSH ESI; CALL
// 00BF65AC (00640D60); MOV EAX,ESI; RET 4.
Raw hud_markers_scalar_delete_00640d50(Raw s, std::uint8_t flags, ScreenTeardownHost& host) {
    hud_markers_destruct_006404a0(s, host);
    if (flags & 1) host.free_block(s.p);
    return s;
}

// 006405F0: SUB ECX,8; JMP 00640D50 (disk bytes; INT3 padding from 006405F8).
Raw hud_markers_scalar_delete_adjustor8_006405f0(Raw secondary, std::uint8_t flags,
                                                 ScreenTeardownHost& host) {
    return hud_markers_scalar_delete_00640d50(Raw{secondary.p - 0x8}, flags, host);
}

// 00647DB0, HUD root destructor: pages +28h, +2Ch, +F8h; seven vectors
// +168h..+108h; the target observer at +D0h (0042BCA0); the vector of string
// pairs at +ACh (00647AD0); vectors +A0h, +90h; the string at +70h; the
// callback owner at +8h; the base.
inline constexpr std::uint32_t kHudRootVtable = 0x00CF5A9C;
inline constexpr std::uint32_t kHudRootCallbackVtable = 0x00CF5A84;
void hud_root_destruct_00647db0(Raw s, ScreenTeardownHost& host, NativeStringHost& strings) {
    s.at<std::uint32_t>(0x0) = kHudRootVtable;
    s.at<std::uint32_t>(0x8) = kHudRootCallbackVtable;
    remove_page(s, 0x28, host);
    remove_page(s, 0x2C, host);
    remove_page(s, 0xF8, host);
    for (std::size_t off = 0x168; off >= 0x108; off -= 0x10) free_vector_words(s, off, host);
    host.target_observer_destruct_0042bca0(Raw{s.p + 0xD0});
    host.string_pair_vector_tidy_00647ad0(Raw{s.p + 0xAC});
    free_vector_words(s, 0xA0, host);
    free_vector_words(s, 0x90, host);
    native_string_release_at(s, 0x70, strings);
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

// 0064D8D0, ship-status exit: sight blur off (00B0CF80(0.0, 0.0), ECX =
// *00F8D39C), hide +40h, free the three heap vectors +78h/+7Ch/+80h (each an
// object whose vector words start at +4h), hide +38h and reset its colour to
// opaque white (00AA85B0), hide +40h again, clear +8h, +156h, +157h.
void ship_status_exit_0064d8d0(Raw s, ScreenTeardownHost& host) {
    host.render_set_sight_blur_00b0cf80(0.0f, 0.0f);
    host.widget_set_visible_34(s.ptr(0x40), false);
    for (std::size_t off : {0x78u, 0x7Cu, 0x80u}) {
        void* holder = s.ptr(off);
        if (holder == nullptr) continue;
        free_vector_words(Raw{static_cast<std::uint8_t*>(holder)}, 0x4, host);
        host.free_block(holder);
        s.ptr(off) = nullptr;
    }
    host.widget_set_visible_34(s.ptr(0x38), false);
    const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};  // 00D7A24C
    host.widget_set_colour_00aa85b0(s.ptr(0x38), white);
    host.widget_set_visible_34(s.ptr(0x40), false);
    s.at<std::uint8_t>(0x8) = 0;
    s.at<std::uint8_t>(0x156) = 0;
    s.at<std::uint8_t>(0x157) = 0;
}

// 00650970, periscope destructor: page +6Ch, the refcounted handles +50h,
// +4Ch, +40h, +3Ch, +38h, +30h in that order, callback owner, base.
void periscope_destruct_00650970(Raw s, ScreenTeardownHost& host) {
    s.at<std::uint32_t>(0x0) = kPeriscopeScreenVtable;
    s.at<std::uint32_t>(0x8) = kPeriscopeCallbackVtable;
    remove_page(s, 0x6C, host);
    for (std::size_t off : {0x50u, 0x4Cu, 0x40u, 0x3Cu, 0x38u, 0x30u}) {
        if (s.ptr(off) != nullptr) {
            host.handle_release(s.ptr(off));
            s.ptr(off) = nullptr;
        }
    }
    host.callback_owner_destroy_00695870(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

// 00651230, periscope layout (+14h): "Periscope_Model" into +70h and
// "Tavcso_Model" (Hungarian for telescope) into +20h, from the page at +6Ch.
void periscope_load_layout_00651230(Raw s, HudScreenHost& host) {
    s.ptr(0x70) = host.gui_find_child_00aa7e00(s.ptr(0x6C), "Periscope_Model");
    s.ptr(0x20) = host.gui_find_child_00aa7e00(s.ptr(0x6C), "Tavcso_Model");
}

// 00650B90, submarine layout (+14h): page "GUI_sub" into +8h and its depth and
// air widgets into +0Ch..+18h.
void submarine_load_layout_00650b90(Raw s, HudScreenHost& host) {
    s.ptr(0x8) = host.gui_load_page_00aa5840("GUI_sub");
    s.ptr(0xC) = host.gui_find_child_00aa7e00(s.ptr(0x8), "sub_depht_arrow_dest_Icon");
    s.ptr(0x10) = host.gui_find_child_00aa7e00(s.ptr(0x8), "sub_depth_arrow_Icon");
    s.ptr(0x14) = host.gui_find_child_00aa7e00(s.ptr(0x8), "sub_air_arrow_Icon");
    s.ptr(0x18) = host.gui_find_child_00aa7e00(s.ptr(0x8), "sub_air_warning_Icon");
}

// ---------------------------------------------------------------------------
// Batch 2: rewards lifetime, marker sets, unit icons, ship and periscope views
// ---------------------------------------------------------------------------

// 0061D8B0, __fastcall(screen) -> AL: with a selected, signed-in (state 2)
// Xbox user, the online service not busy (+19Ch false), and the scoreboard
// listbox (+D4h) enabled (+85h), the flag byte +230h[selected row].
bool rewards_selected_entry_flagged_0061d8b0(Raw s, RewardsScreenHost& host) {
    if (!host.xenon_has_selected_user_00a3e510()) return false;
    if (host.xenon_selected_sign_in_state_00a3ead0() != 2) return false;
    if (host.online_virtual_bool(0x19C)) return false;
    void* listbox = s.ptr(0xD4);
    if (listbox == nullptr) return false;
    if (Raw{static_cast<std::uint8_t*>(listbox)}.at<std::uint8_t>(0x85) == 0) return false;
    if (host.listbox_selected_index_00a9c920(listbox) == -1) return false;
    return s.at<std::uint8_t>(0x230 + host.listbox_selected_index_00a9c920(s.ptr(0xD4))) != 0;
}

// 00622E00, __stdcall(file, unused), RET 8: play the file through the
// debrief voice (0061F9D0, ECX = *(*00E198AC + 60h)), then remove it from the
// file store (004FC150, 00BE80B0(file), 00BE7130).
void rewards_play_and_remove_file_00622e00(void* file, RewardsScreenHost& host) {
    host.rewards_play_test_speech_0061f9d0(file);
    host.file_store_remove_00be7130(file);
}

// 00626630, __thiscall(screen) -> EAX: base construct; listener-base vtables
// (00CEB0FC at +8h, 00CEB110 at +0Ch) then the screen's; the handles and the
// strings at +10h..+34h cleared; the three scroller blocks (+68h, +128h,
// +19Ch, 00683610) and the vectors +F8h, +220h, +238h, +248h zeroed.
inline constexpr std::uint32_t kRewardsVtable = 0x00CF50CC;
inline constexpr std::uint32_t kRewardsCallbackVtable = 0x00CF50B8;
inline constexpr std::uint32_t kRewardsListenerVtable = 0x00CF5098;
inline constexpr std::uint32_t kWidgetListenerBaseVtable = 0x00CEB110;
Raw rewards_construct_00626630(Raw s, RewardsScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0xC) = kWidgetListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = kRewardsVtable;
    s.at<std::uint32_t>(0x8) = kRewardsCallbackVtable;
    s.at<std::uint32_t>(0xC) = kRewardsListenerVtable;
    for (std::size_t off : {0x10u, 0x14u, 0x28u, 0x2Cu, 0x30u, 0x34u}) s.at<std::uint32_t>(off) = 0;
    host.scroller_construct_00683610(Raw{s.p + 0x68});
    for (std::size_t off : {0xFCu, 0x100u, 0x104u}) s.at<std::uint32_t>(off) = 0;
    host.scroller_construct_00683610(Raw{s.p + 0x128});
    host.scroller_construct_00683610(Raw{s.p + 0x19C});
    for (std::size_t off : {0x224u, 0x228u, 0x22Cu, 0x23Cu, 0x240u, 0x244u, 0x24Cu, 0x250u, 0x254u}) {
        s.at<std::uint32_t>(off) = 0;
    }
    return s;
}

// 00626770, destructor: vtables, page +20h, the sound handles (0061EC00), the
// two 10h-record vectors +248h and +238h (00 6243C0 destroys the range, then
// the block is freed), the score rows +220h (00625A00), the vector +F8h, the
// strings +30h and +28h, the handles +14h and +10h, the listener base, base.
void rewards_destruct_00626770(Raw s, RewardsScreenHost& host, NativeStringHost& strings) {
    s.at<std::uint32_t>(0x0) = kRewardsVtable;
    s.at<std::uint32_t>(0x8) = kRewardsCallbackVtable;
    s.at<std::uint32_t>(0xC) = kRewardsListenerVtable;
    host.gui_remove_and_destroy_page_00aa31f0(s.ptr(0x20));
    rewards_release_sound_handles_0061ec00(s, host);
    for (std::size_t vec : {0x248u, 0x238u}) {
        if (s.ptr(vec + 4) != nullptr) {
            host.record10_destroy_range_006243c0(Raw{s.p + vec});
            strings.free_block(s.ptr(vec + 4));
        }
        s.ptr(vec + 4) = nullptr;
        s.ptr(vec + 8) = nullptr;
        s.ptr(vec + 0xC) = nullptr;
    }
    host.score_row_vector_tidy_00625a00(Raw{s.p + 0x220});
    if (s.ptr(0xFC) != nullptr) strings.free_block(s.ptr(0xFC));
    s.ptr(0xFC) = nullptr;
    s.ptr(0x100) = nullptr;
    s.ptr(0x104) = nullptr;
    native_string_release_at(s, 0x30, strings);
    native_string_release_at(s, 0x28, strings);
    release_handle(s, 0x14, host);
    s.ptr(0x14) = nullptr;
    release_handle(s, 0x10, host);
    s.ptr(0x10) = nullptr;
    s.at<std::uint32_t>(0x8) = kListboxListenerBaseVtable;
    host.front_end_screen_destruct_004f71a0(s);
}

// 0063DEC0, __thiscall(set, page), RET 4 -> EAX: find CLOSE_Group in the page,
// then clone its Yellow, White, Coffe (sic), Blue and Red icons into +4h..+14h.
inline constexpr std::array<std::string_view, 5> kHudColourIcons{
    "Yellow_Icon", "White_Icon", "Coffe_Icon", "Blue_Icon", "Red_Icon"};
Raw hud_colour_icon_set_construct_0063dec0(Raw set, void* page, HudScreenHost& host, HudMarkerCloneHost& clone) {
    set.at<std::uint32_t>(0x0) = kHudColourIconSetVtable;
    void* group = host.gui_find_child_00aa7e00(page, "CLOSE_Group");
    for (std::size_t i = 0; i < kHudColourIcons.size(); ++i) {
        set.ptr(0x4 + i * 4) = clone.clone_subtree_00aab4c0(host.gui_find_child_00aa7e00(group, kHudColourIcons[i]), 0);
    }
    return set;
}

// 00638F50: the 24 clones at +8h..+64h, then +68h, +78h, +7Ch, +6Ch, +80h,
// +70h, +74h, each deleted through vtable +4h(1) and cleared.
inline constexpr std::uint32_t kHudUnitMarkerWidgetsVtable = 0x00CF5758;
void hud_unit_marker_widgets_destruct_00638f50(Raw w, HudScreenHost& host) {
    w.at<std::uint32_t>(0x0) = kHudUnitMarkerWidgetsVtable;
    for (std::size_t i = 0; i < 24; ++i) delete_and_clear(w, 0x8 + i * 4, host);
    for (std::size_t off : {0x68u, 0x78u, 0x7Cu, 0x6Cu, 0x80u, 0x70u, 0x74u}) delete_and_clear(w, off, host);
}

// 00640860 / 00640940, __fastcall(screen) -> EAX: hand out the next pooled
// marker; when the cursor reaches the pool size a new one is made (18h bytes,
// 0063DEC0, or 8 bytes, 0063E1C0, both with the page +8Ch) and pushed
// (00640250 / 006402C0, ECX = the vector).
void* hud_markers_acquire_pooled_marker(const MarkerPool& pool, Raw s, MarkerPoolHost& host) {
    void** begin = static_cast<void**>(s.ptr(pool.vector_off + 4));
    const std::size_t size = begin == nullptr ? 0 : static_cast<std::size_t>(static_cast<void**>(s.ptr(pool.vector_off + 8)) - begin);
    if (s.at<std::uint32_t>(pool.cursor_off) == size) {
        void* marker = host.make_marker(pool, s.ptr(0x8C));
        host.push_marker(pool, Raw{s.p + pool.vector_off}, marker);
    }
    const std::uint32_t index = s.at<std::uint32_t>(pool.cursor_off)++;
    begin = static_cast<void**>(s.ptr(pool.vector_off + 4));
    if (begin == nullptr || index >= static_cast<std::size_t>(static_cast<void**>(s.ptr(pool.vector_off + 8)) - begin)) {
        host.invalid_parameter_00bf6713();
    }
    return static_cast<void**>(s.ptr(pool.vector_off + 4))[index];
}

// 00644AE0, __fastcall(unit) -> EAX: the HUD icon index of a unit. Planes
// (kind 18h, read through their leader +3D0h) and kind 0Fh go by the ordnance
// kind (007BCA50): 9/0Dh -> class 10h, 0Ah -> class 11h, 0Bh -> 0Ch, 0Fh ->
// 16h, +C24h set -> class 13h, else class 14h. Kind 1Bh uses its record class
// (+538h -> +178h), 45h when that is 1Bh and +50h is clear. Kinds 0Ah and 0Ch
// with the record byte +808h give 4 and 7. Others use the class at +C4h. A
// "class" goes through 006440C0 (band_0060_cc13).
int hud_unit_icon_index_00644ae0(Raw unit, UnitIconHost& host) {
    auto icon = [](int cls) { return band_0060_cc13::unit_class_icon_index_006440c0(cls); };
    if (!host.unit_is_kind_5c(unit.p, 0x18) && !host.unit_is_kind_5c(unit.p, 0xF)) {
        Raw record{static_cast<std::uint8_t*>(unit.ptr(0x538))};
        if (host.unit_is_kind_5c(unit.p, 0x1B)) {
            const std::int32_t cls = record.at<std::int32_t>(0x178);
            if (cls == 0x1B && unit.at<std::uint32_t>(0x50) == 0) return icon(0x45);
            return icon(cls);
        }
        if (host.unit_is_kind_5c(unit.p, 0xA) && record.at<std::uint8_t>(0x808) != 0) return 4;
        if (host.unit_is_kind_5c(unit.p, 0xC)
            && Raw{static_cast<std::uint8_t*>(unit.ptr(0x538))}.at<std::uint8_t>(0x808) != 0) {
            return 7;
        }
        return icon(unit.at<std::int32_t>(0xC4));
    }
    Raw u = unit;
    if (host.unit_is_kind_5c(unit.p, 0x18)) u = Raw{static_cast<std::uint8_t*>(unit.ptr(0x3D0))};
    const int kind = host.plane_ordnance_icon_kind_007bca50(u.p);
    if (kind == 9 || kind == 0xD) return icon(0x10);
    if (kind == 0xA) return icon(0x11);
    if (kind == 0xB) return 0xC;
    if (kind == 0xF) return 0x16;
    if (u.at<std::uint8_t>(0xC24) != 0) return icon(0x13);
    return icon(0x14);
}

// 00646D30, __thiscall(owner, const char*), RET 4: build a native string
// (0041E870) and set it ellipsized on the text at +34h (00ABB000(&s, -1.0f, 1)).
void text_owner_set_ellipsized_00646d30(Raw owner, const char* text, HudScreenHost& host) {
    host.text_set_ellipsized_00abb000(owner.ptr(0x34), text, -1.0f, true);  // 00D7A260
}

// 00645160, __fastcall(ECX unit, EDX flag): 00645060(unit, local slot
// game+18ECh, flag).
bool unit_selectable_for_current_player_00645160(void* unit, int flag, UnitSelectHost& host) {
    return host.unit_selectable_00645060(unit, host.local_player_slot(), flag);
}

// 00645180, __stdcall(unit), RET 4: in a session, the unit's owner id (+54h)
// equals the local player record's +28h; otherwise 00645060(unit, slot, 1).
bool unit_is_local_player_unit_00645180(Raw unit, UnitSelectHost& host) {
    if (host.session_mode() != 0) return unit.at<std::int32_t>(0x54) == host.local_player_record_28();
    return host.unit_selectable_00645060(unit.p, host.local_player_slot(), 1);
}

// 00646040, __thiscall(root, iface, observed), RET 8: only while +1Ch is clear,
// store +CCh and re-point the observer member +D0h (observed +E4h).
void hud_root_set_interface_00646040(Raw s, std::uint32_t iface, void* observed, HudScreenHost& host) {
    if (s.at<std::uint32_t>(0x1C) != 0) return;
    s.at<std::uint32_t>(0xCC) = iface;
    observer_ref_set(Raw{s.p + 0xD0}, observed, host);
}

// 0064A8E0, __fastcall(screen): with a unit (+184h), arm the ping (+119h = 1,
// +30h = 1.0f), move the ping widget +F8h to (0.5, 0.5, its z) (00AA6750 /
// 00AA8240) and copy the unit's word +174h into +FCh.
void ship_screen_trigger_unit_ping_0064a8e0(Raw s, ShipScreenWidgetHost& host) {
    if (s.ptr(0x184) == nullptr) return;
    s.at<std::uint8_t>(0x119) = 1;
    s.at<float>(0x30) = 1.0f;
    float pos[3];
    host.widget_resolved_position_00aa6750(s.ptr(0xF8), pos);
    const float target[3] = {0.5f, 0.5f, pos[2]};
    host.widget_set_resolved_position_00aa8240(s.ptr(0xF8), target);
    s.at<std::uint16_t>(0xFC) = Raw{static_cast<std::uint8_t*>(s.ptr(0x184))}.at<std::uint16_t>(0x174);
}

// 0064D590, __thiscall(screen, unit), RET 4: on a new unit mark +188h and drop
// timed entry 2 of the four widgets +E8h..+F4h; re-point the observer member
// +170h (observed +184h); an active screen (+5h) re-applies the unit (0064AE20).
void ship_screen_set_unit_0064d590(Raw s, void* unit, ShipScreenWidgetHost& host) {
    if (s.ptr(0x184) != unit) {
        s.at<std::uint8_t>(0x188) = 1;
        for (std::size_t i = 0; i < 4; ++i) host.widget_delete_timed_entry_00aa8b80(s.ptr(0xE8 + i * 4), 2);
    }
    observer_ref_set(Raw{s.p + 0x170}, unit, host);
    if (s.at<std::uint8_t>(kActive) != 0) host.ship_screen_apply_unit_0064ae20(s);
}

// 00650210, __fastcall(screen): bind the weapon-group HUD to (unit +24h, camera
// +2Ch); observe the unit (member +8h); copy its +980h/+984h into +54h/+58h.
void periscope_attach_unit_00650210(Raw s, HudScreenHost& host) {
    host.weapon_group_bind_unit_00549260(s.ptr(0x24), s.ptr(0x2C));
    void* unit = s.ptr(0x24);
    if (unit == nullptr) return;
    if (!host.observer_is_registered_00694af0(s.p + 0x8, unit)) host.observer_register_00694a60(s.p + 0x8, unit);
    Raw u{static_cast<std::uint8_t*>(s.ptr(0x24))};
    s.at<float>(0x54) = u.at<float>(0x980);
    s.at<float>(0x58) = u.at<float>(0x984);
}

// 006506E0, __fastcall(screen): release the weapon-group HUD and the unit's
// roles (0077C470(1FEh, 0)); +74h = 0, +5Ch = 1.0f, view fov 1.0 (004DC940(1.0f,
// 1), ECX = game); an active screen hides +70h and +20h.
void periscope_detach_unit_006506e0(Raw s, HudScreenHost& host) {
    host.weapon_group_release_005470c0();
    if (s.ptr(0x24) != nullptr) host.unit_send_role_transfer_0077c470(s.ptr(0x24), 0x1FE, 0);
    s.at<std::uint32_t>(0x74) = 0;
    s.at<float>(0x5C) = 1.0f;
    host.game_set_view_fov_004dc940(1.0f, 1);
    if (s.at<std::uint8_t>(kActive) != 0) {
        host.widget_set_visible_34(s.ptr(0x70), false);
        host.widget_set_visible_34(s.ptr(0x20), false);
    }
}

// 00650750, enter: attach, +34h = -1.0f, hide +70h and +20h. When the unit's
// periscope state (+122Ch) is not 2 and its +1234h byte is set, mode 2: the
// camera (+2Ch) selects mode 1 (0042CB70) with its +388h zeroed, +70h shown,
// +5Ch = 1.0f; otherwise mode 0 and camera mode 0. Then the screen's update
// (+20h(0.0)) and the camera's +DCh(0.0).
void periscope_enter_00650750(Raw s, HudScreenHost& host) {
    periscope_attach_unit_00650210(s, host);
    s.at<float>(0x34) = -1.0f;
    host.widget_set_visible_34(s.ptr(0x70), false);
    host.widget_set_visible_34(s.ptr(0x20), false);
    Raw unit{static_cast<std::uint8_t*>(s.ptr(0x24))};
    if (unit.at<std::int32_t>(0x122C) != 2 && unit.at<std::uint8_t>(0x1234) != 0) {
        s.at<std::uint32_t>(0x74) = 2;
        if (s.ptr(0x2C) != nullptr) {
            host.submarine_camera_select_mode_0042cb70(s.ptr(0x2C), 1);
            Raw{static_cast<std::uint8_t*>(s.ptr(0x2C))}.at<float>(0x388) = 0.0f;
        }
        host.widget_set_visible_34(s.ptr(0x70), true);
        s.at<float>(0x5C) = 1.0f;
    } else {
        s.at<std::uint32_t>(0x74) = 0;
        if (s.ptr(0x2C) != nullptr) host.submarine_camera_select_mode_0042cb70(s.ptr(0x2C), 0);
    }
    host.screen_update_20(s, 0.0f);
    if (s.ptr(0x2C) != nullptr) host.camera_virtual_dc(s.ptr(0x2C), 0.0f);
}

// 00651760, __thiscall(screen, unit), RET 4: detach, drop the old observation,
// store the unit (its +122Ch state reset to 0 unless it is 2), attach, ensure
// the submarine camera (00651370), recentre screen 2Eh on (0.5, 0.5, +5Ch),
// then +74h = 0, +5Ch = 1.0f and view fov 1.0.
void unit_view_bind_unit_and_reset_00651760(Raw s, void* unit, HudScreenHost& host) {
    periscope_detach_unit_006506e0(s, host);
    if (s.ptr(0x24) != nullptr && host.observer_is_registered_00694af0(s.p + 0x8, s.ptr(0x24))) {
        host.observer_unregister_006952a0(s.p + 0x8, s.ptr(0x24));
    }
    s.ptr(0x24) = unit;
    Raw u{static_cast<std::uint8_t*>(unit)};
    if (u.at<std::int32_t>(0x122C) != 2) u.at<std::int32_t>(0x122C) = 0;
    periscope_attach_unit_00650210(s, host);
    host.unit_view_ensure_submarine_camera_00651370(s);
    host.screen2e_set_view_centre_005454b0(0.5f, 0.5f, s.at<float>(0x5C));
    s.at<std::uint32_t>(0x74) = 0;
    s.at<float>(0x5C) = 1.0f;
    host.game_set_view_fov_004dc940(1.0f, 1);
}

// 0067CA40, press-start storage completion: when *(0109CECC)+8 is clear,
// dismiss every prompt of the menu-command screen (00425D10, 00530650) and
// skip the title (0068D8A0, tail jump).
void press_start_on_storage_complete_0067ca40(PressStartStorageHost& host) {
    if (host.storage_request_08() != 0) return;
    host.menu_prompts_dismiss_all_00530650();
    host.title_skip_0068d8a0();
}

// ---------------------------------------------------------------------------
// Batch 3: observer-reference objects and support-manager records
// ---------------------------------------------------------------------------

// 00653400 / 006569A0: restore the class vtable, unregister the observed
// object (+14h) if any, then the callback-owner base (00695870, ECX = this).
void observer_ref_destruct(Raw ref, std::uint32_t vtable, HudScreenHost& host) {
    ref.at<std::uint32_t>(0x0) = vtable;
    if (ref.ptr(0x14) != nullptr) host.observer_unregister_006952a0(ref.p, ref.ptr(0x14));
    host.callback_owner_destroy_00695870(ref);
}

// 00658710 / 006587C0, __thiscall(ref, src), RET 4 -> EAX: clear +4h..+0Ch
// and the +10h byte, install the vtable, copy the observed pointer, set +10h
// and register when it is non-null.
Raw observer_ref_copy_construct(Raw ref, Raw src, std::uint32_t vtable, HudScreenHost& host) {
    ref.at<std::uint32_t>(0x4) = 0;
    ref.at<std::uint32_t>(0x8) = 0;
    ref.at<std::uint32_t>(0xC) = 0;
    ref.at<std::uint8_t>(0x10) = 0;
    ref.at<std::uint32_t>(0x0) = vtable;
    ref.ptr(0x14) = src.ptr(0x14);
    ref.at<std::uint8_t>(0x10) = 1;
    if (ref.ptr(0x14) != nullptr) host.observer_register_00694a60(ref.p, ref.ptr(0x14));
    return ref;
}

// 0065BB00: vtable 00CF6544, the target observer at +34h (0042BCA0), then the
// MSVC string at +14h is tidied (heap buffer freed when the capacity +28h is
// above 15; capacity 15, size +24h 0, first byte 0).
inline constexpr std::uint32_t kUnitSlotEntryVtable = 0x00CF6544;
void unit_slot_entry_destroy_0065bb00(Raw e, ScreenTeardownHost& host) {
    e.at<std::uint32_t>(0x0) = kUnitSlotEntryVtable;
    host.target_observer_destruct_0042bca0(Raw{e.p + 0x34});
    if (e.at<std::uint32_t>(0x28) > 0xF) host.free_block(e.ptr(0x14));
    e.at<std::uint32_t>(0x28) = 0xF;
    e.at<std::uint32_t>(0x24) = 0;
    e.at<std::uint8_t>(0x14) = 0;
}

// 006626E0, __thiscall(dst, src), RET 4 -> EAX: +4h..+0Ch, the legacy string at
// +10h (00408120(src+10h, 0, -1), ECX = dst+10h), +2Ch, +30h, then the observer
// member +34h re-pointed at src's +48h.
Raw observer_string_record_assign_006626e0(Raw dst, Raw src, RecordAssignHost& host) {
    dst.at<std::uint32_t>(0x4) = src.at<std::uint32_t>(0x4);
    dst.at<std::uint32_t>(0x8) = src.at<std::uint32_t>(0x8);
    dst.at<std::uint32_t>(0xC) = src.at<std::uint32_t>(0xC);
    host.legacy_string_assign_00408120(Raw{dst.p + 0x10}, Raw{src.p + 0x10});
    dst.at<std::uint32_t>(0x2C) = src.at<std::uint32_t>(0x2C);
    dst.at<std::uint32_t>(0x30) = src.at<std::uint32_t>(0x30);
    observer_ref_set(Raw{dst.p + 0x34}, src.ptr(0x48), host);
    return dst;
}

// 006683D0: vtable 00CF69A8; the vector of 1Ch legacy strings at +0Ch (begin
// +10h): destroy the range (004CE520, ECX begin, EDX end), free, zero.
inline constexpr std::uint32_t kStringVectorHolderVtable = 0x00CF69A8;
void string_vector_holder_destruct_006683d0(Raw h, RecordAssignHost& host) {
    h.at<std::uint32_t>(0x0) = kStringVectorHolderVtable;
    if (h.ptr(0x10) != nullptr) {
        host.legacy_string_range_destroy_004ce520(h.ptr(0x10), h.ptr(0x14));
        host.free_block(h.ptr(0x10));
    }
    h.ptr(0x10) = nullptr;
    h.ptr(0x14) = nullptr;
    h.ptr(0x18) = nullptr;
}

// 00669350, __thiscall(dst, src), RET 4 -> EAX: the int, then (dst != src) the
// map at +4h is emptied (006641B0) and copied from src's (0065CE20).
Raw int_map_record_assign_00669350(Raw dst, Raw src, RecordAssignHost& host) {
    dst.at<std::uint32_t>(0x0) = src.at<std::uint32_t>(0x0);
    if (dst.p + 4 != src.p + 4) {
        host.tree_erase_all_006641b0(Raw{dst.p + 4});
        host.tree_copy_0065ce20(Raw{dst.p + 4}, Raw{src.p + 4});
    }
    return dst;
}

// 00660540, __thiscall(dst, src), RET 4 -> EAX: int, native string, int.
Raw int_string_int_record_copy_construct_00660540(Raw dst, Raw src, NativeStringHost& host) {
    dst.at<std::uint32_t>(0x0) = src.at<std::uint32_t>(0x0);
    construct_string(dst, 0x4, src, 0x4, host);
    dst.at<std::uint32_t>(0xC) = src.at<std::uint32_t>(0xC);
    return dst;
}

// 0065CA50, __thiscall(dst, src), RET 4 -> EAX: the support-manager slot
// record: +4h..+10h, observer member +14h (observed +28h), +2Ch, +30h, the
// +34h byte, observer member +38h (observed +4Ch), +50h, +54h.
Raw slot_record_assign_0065ca50(Raw dst, Raw src, HudScreenHost& host) {
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x10u}) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    observer_ref_set(Raw{dst.p + 0x14}, src.ptr(0x28), host);
    dst.at<std::uint32_t>(0x2C) = src.at<std::uint32_t>(0x2C);
    dst.at<std::uint32_t>(0x30) = src.at<std::uint32_t>(0x30);
    dst.at<std::uint8_t>(0x34) = src.at<std::uint8_t>(0x34);
    observer_ref_set(Raw{dst.p + 0x38}, src.ptr(0x4C), host);
    dst.at<std::uint32_t>(0x50) = src.at<std::uint32_t>(0x50);
    dst.at<std::uint32_t>(0x54) = src.at<std::uint32_t>(0x54);
    return dst;
}

// 0065CAF0, copy constructor: vtable 00CF6564, the two observer members
// copy-constructed (00658710 at +14h, 006587C0 at +38h) and then given the
// record's own member vtables (00CF6514, 00CF652C).
inline constexpr std::uint32_t kSlotRecordVtable = 0x00CF6564;
inline constexpr std::uint32_t kSlotRecordMember14Vtable = 0x00CF6514;
inline constexpr std::uint32_t kSlotRecordMember38Vtable = 0x00CF652C;
inline constexpr std::uint32_t kObserverRefVtableA = 0x00CF6494;  // 00658710 / 006569A0
inline constexpr std::uint32_t kObserverRefVtableB = 0x00CF64AC;  // 006587C0 / 00653400
Raw slot_record_copy_construct_0065caf0(Raw dst, Raw src, HudScreenHost& host) {
    dst.at<std::uint32_t>(0x0) = kSlotRecordVtable;
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x10u}) dst.at<std::uint32_t>(off) = src.at<std::uint32_t>(off);
    observer_ref_copy_construct(Raw{dst.p + 0x14}, Raw{src.p + 0x14}, kObserverRefVtableA, host);
    dst.at<std::uint32_t>(0x14) = kSlotRecordMember14Vtable;
    dst.at<std::uint32_t>(0x2C) = src.at<std::uint32_t>(0x2C);
    dst.at<std::uint32_t>(0x30) = src.at<std::uint32_t>(0x30);
    dst.at<std::uint8_t>(0x34) = src.at<std::uint8_t>(0x34);
    observer_ref_copy_construct(Raw{dst.p + 0x38}, Raw{src.p + 0x38}, kObserverRefVtableB, host);
    dst.at<std::uint32_t>(0x38) = kSlotRecordMember38Vtable;
    dst.at<std::uint32_t>(0x50) = src.at<std::uint32_t>(0x50);
    dst.at<std::uint32_t>(0x54) = src.at<std::uint32_t>(0x54);
    return dst;
}

// 00658530, __fastcall(record) -> AL: launched and out (+28h set, +2Ch 3 or 4).
bool slot_record_is_out_00658530(Raw r) noexcept {
    if (r.ptr(0x28) == nullptr) return false;
    const std::int32_t state = r.at<std::int32_t>(0x2C);
    return state == 3 || state == 4;
}

// 0065BE80, __thiscall(screen, unit), RET 4: track the unit's target (vtable
// +140h) when the unit is kind 2, through the member +208h (observed +21Ch);
// otherwise drop the tracking.
void support_manager_set_tracked_unit_target_0065be80(Raw s, void* unit, TrackedTargetHost& host) {
    if (unit == nullptr || !host.unit_is_kind_5c(unit, 2)) {
        if (s.ptr(0x21C) != nullptr) {
            host.observer_unregister_006952a0(s.p + 0x208, s.ptr(0x21C));
            s.ptr(0x21C) = nullptr;
        }
        return;
    }
    observer_ref_set(Raw{s.p + 0x208}, host.unit_target_140(unit), host);
}

// 0066FFA0, __fastcall(screen) -> AL: outside effective game mode 8 always
// true. In mode 8 the answer is cached in +479h and recomputed when the game
// counter +648h moves past +47Ch: 0066FB00, 00668770 when +34Ch is clear, the
// cursor sync 0065A2B0; true unless both +34Ch and +1F4h are clear.
bool support_manager_has_available_entries_0066ffa0(Raw s, SupportEntriesHost& host) {
    if (host.game_effective_mode_004bca50() != 8) return true;
    const std::int32_t counter = host.game_counter_648();
    if (s.at<std::int32_t>(0x47C) != counter) {
        s.at<std::int32_t>(0x47C) = counter;
        host.support_refresh_0066fb00(s);
        if (s.at<std::uint32_t>(0x34C) == 0) host.support_rebuild_00668770(s);
        host.support_sync_cursor_0065a2b0(s);
        s.at<std::uint8_t>(0x479) = (s.at<std::uint32_t>(0x34C) == 0 && s.at<std::uint32_t>(0x1F4) == 0) ? 0 : 1;
        return s.at<std::uint8_t>(0x479) != 0;
    }
    return s.at<std::uint8_t>(0x479) != 0;
}

// ---------------------------------------------------------------------------
// Batch 4: unit library (05h), press start, text range fitting
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kUnitLibraryVtable = 0x00CF6E50;
inline constexpr std::uint32_t kUnitLibraryCallbackVtable = 0x00CF6E4C;
inline constexpr std::uint32_t kUnitLibraryListenerBaseAt64 = 0x00CF6E38;
inline constexpr std::uint32_t kUnitLibraryWidgetListenerVtable = 0x00CF6E18;

// 00680C20, __thiscall(screen) -> EAX: base construct, the 3D scene at +8h
// (0093CB10), the listener vtables at +64h/+68h (bases then own), +E0h and the
// vectors +110h and +120h cleared.
Raw unit_library_construct_00680c20(Raw s, UnitLibraryHost& host) {
    host.front_end_screen_construct_004f7180(s);
    host.simple3d_scene_construct_0093cb10(Raw{s.p + 0x8});
    s.at<std::uint32_t>(0x64) = kListboxListenerBaseVtable;
    s.at<std::uint32_t>(0x68) = kWidgetListenerBaseVtable;
    s.at<std::uint32_t>(0x0) = kUnitLibraryVtable;
    s.at<std::uint32_t>(0x8) = kUnitLibraryCallbackVtable;
    s.at<std::uint32_t>(0x64) = kUnitLibraryListenerBaseAt64;
    s.at<std::uint32_t>(0x68) = kUnitLibraryWidgetListenerVtable;
    for (std::size_t off : {0xE0u, 0x114u, 0x118u, 0x11Cu, 0x124u, 0x128u, 0x12Cu}) s.at<std::uint32_t>(off) = 0;
    return s;
}

// 0067ED30, __fastcall(screen): for each slot of the +110h vector delete the
// model part at +120h[i] (vtable +4h(1)) and release the handle at +110h[i];
// clear both vectors (erase(begin, end)); delete +10Ch; release +108h.
void unit_library_release_model_parts_0067ed30(Raw s, UnitLibraryHost& host) {
    for (std::size_t i = 0;; ++i) {
        void** handles = static_cast<void**>(s.ptr(0x114));
        if (handles == nullptr || i >= static_cast<std::size_t>(static_cast<void**>(s.ptr(0x118)) - handles)) break;
        void** parts = static_cast<void**>(s.ptr(0x124));
        if (parts == nullptr || i >= static_cast<std::size_t>(static_cast<void**>(s.ptr(0x128)) - parts)) {
            host.invalid_parameter_00bf6713();
        }
        parts = static_cast<void**>(s.ptr(0x124));
        if (parts[i] != nullptr) {
            host.object_delete_04(parts[i]);
            parts[i] = nullptr;
        }
        handles = static_cast<void**>(s.ptr(0x114));
        if (handles[i] != nullptr) {
            host.handle_release(handles[i]);
            handles[i] = nullptr;
        }
    }
    s.ptr(0x128) = s.ptr(0x124);
    s.ptr(0x118) = s.ptr(0x114);
    if (s.ptr(0x10C) != nullptr) {
        host.object_delete_04(s.ptr(0x10C));
        s.ptr(0x10C) = nullptr;
    }
    if (s.ptr(0x108) != nullptr) {
        host.handle_release(s.ptr(0x108));
        s.ptr(0x108) = nullptr;
    }
}

// 0067EF50, destructor: vtables, delete +E0h, page +6Ch, release the model
// parts, free the vectors +120h and +110h, the +64h listener base, the 3D
// scene (0093CC80, ECX = +8h), the base.
void unit_library_destruct_0067ef50(Raw s, UnitLibraryHost& host) {
    s.at<std::uint32_t>(0x0) = kUnitLibraryVtable;
    s.at<std::uint32_t>(0x8) = kUnitLibraryCallbackVtable;
    s.at<std::uint32_t>(0x64) = kUnitLibraryListenerBaseAt64;
    s.at<std::uint32_t>(0x68) = kUnitLibraryWidgetListenerVtable;
    if (s.ptr(0xE0) != nullptr) {
        host.object_delete_04(s.ptr(0xE0));
        s.ptr(0xE0) = nullptr;
    }
    host.gui_remove_and_destroy_page_00aa31f0(s.ptr(0x6C));
    unit_library_release_model_parts_0067ed30(s, host);
    for (std::size_t off : {0x124u, 0x114u}) {
        if (s.ptr(off) != nullptr) host.free_block(s.ptr(off));
        s.ptr(off) = nullptr;
        s.ptr(off + 4) = nullptr;
        s.ptr(off + 8) = nullptr;
    }
    s.at<std::uint32_t>(0x64) = kListboxListenerBaseVtable;
    host.simple3d_scene_destruct_0093cc80(Raw{s.p + 0x8});
    host.front_end_screen_destruct_004f71a0(s);
}

// 0067DED0, exit: delete the preview at +E0h, stop the movie (00AAC870, ECX =
// +94h), restart the title music (005884A0, ECX = *(*00E198AC + 58h)); a
// pending save (+104h) runs 007FA1B0(0, 00BD53C0) and is cleared.
void unit_library_exit_0067ded0(Raw s, UnitLibraryHost& host) {
    if (s.ptr(0xE0) != nullptr) {
        host.object_delete_04(s.ptr(0xE0));
        s.ptr(0xE0) = nullptr;
    }
    host.gui_movie_stop_00aac870(s.ptr(0x94));
    host.main_menu_start_title_music_005884a0();
    if (s.at<std::uint8_t>(0x104) != 0) {
        host.profile_save_then_call_007fa1b0(0, 0x00BD53C0);
        s.at<std::uint8_t>(0x104) = 0;
    }
}

// 0067E580, __fastcall(screen): +E8h = 0; show every widget of the list at
// (*(+6Ch)) +64h (node +8h); hide +78h, show +70h; stop the movie, restart the
// title music; flag *00E198BC +48h when that object exists.
void unit_library_reset_selection_0067e580(Raw s, UnitLibraryHost& host) {
    s.at<std::uint32_t>(0xE8) = 0;
    Raw page{static_cast<std::uint8_t*>(s.ptr(0x6C))};
    void* head = page.ptr(0x68);
    for (void* node = *static_cast<void**>(head); node != head; node = *static_cast<void**>(node)) {
        host.widget_set_visible_34(Raw{static_cast<std::uint8_t*>(node)}.ptr(0x8), true);
    }
    host.widget_set_visible_34(s.ptr(0x78), false);
    host.widget_set_visible_34(s.ptr(0x70), true);
    host.gui_movie_stop_00aac870(s.ptr(0x94));
    host.main_menu_start_title_music_005884a0();
    if (void* menu = host.global_object(0x00E198BC).p) Raw{static_cast<std::uint8_t*>(menu)}.at<std::uint8_t>(0x48) = 1;
}

// 0067C880, __fastcall(device) -> AL: button 0 or 0Eh held (vtable +20h).
bool press_start_device_button_held_0067c880(void* device, DeviceButtonHost& host) {
    return host.button_held_20(device, 0) || host.button_held_20(device, 0xE);
}

// 0067C8F0 -> AL: a pending invite (*00E19880) for the signed-in user: its
// slot (00A3E470) is the active user slot (00A3EAC0).
bool game_sign_in_state_valid_0067c8f0(InviteHost& host) {
    if (host.pending_invite_00e19880() == nullptr) return false;
    if (!host.xenon_has_selected_user_00a3e510()) return false;
    const std::int32_t slot = host.xenon_slot_for_invitee_00a3e470();
    return slot == host.xenon_active_user_slot_00a3eac0();
}

// 0067D870, __fastcall(ECX text, EDX prefix, src, cursor_base, first, last),
// RET 10h: text = prefix + src[first, last) (00469840, 004261A0), set as the
// source (00ABAED0(&s, 0)) with the cursor at prefix.count - first +
// cursor_base (00AB9240).
void gui_text_set_source_range_0067d870(void* text, Raw prefix, Raw src, std::int32_t cursor_base,
    std::int32_t first, std::int32_t last, TextRangeHost& host) {
    host.text_set_prefix_plus_substring(text, prefix, src, first, last - first);
    host.text_position_cursor_00ab9240(text, prefix.at<std::int32_t>(0x0) - first + cursor_base);
}

// 0067D940, __thiscall(ECX text, EDX prefix, src, cursor_base), RET 8: show the
// whole source; while it wraps (+110h line count above 1) slide a window
// [first, last) with last = min(cursor_base + 8, count): advance first until
// one line fits, then grow last while it still fits, stepping back once when
// the last growth wrapped.
void gui_text_fit_range_to_one_line_0067d940(void* text, Raw prefix, Raw src, std::int32_t cursor_base,
    TextRangeHost& host) {
    const std::int32_t count = src.at<std::int32_t>(0x0);
    gui_text_set_source_range_0067d870(text, prefix, src, cursor_base, 0, count, host);
    if (host.text_line_count_110(text) <= 1) return;
    std::int32_t last = cursor_base + 8;
    if (last >= count) last = count;
    std::int32_t first = -1;
    do {
        ++first;
        gui_text_set_source_range_0067d870(text, prefix, src, cursor_base, first, last, host);
    } while (host.text_line_count_110(text) > 1);
    while (static_cast<std::uint32_t>(last) < static_cast<std::uint32_t>(count)) {
        ++last;
        gui_text_set_source_range_0067d870(text, prefix, src, cursor_base, first, last, host);
        if (host.text_line_count_110(text) > 1) break;
    }
    if (host.text_line_count_110(text) > 1) {
        gui_text_set_source_range_0067d870(text, prefix, src, cursor_base, first, last - 1, host);
    }
}

// ---------------------------------------------------------------------------
// Batch 5: layout binders (page loads and named-child finds, in image order)
// ---------------------------------------------------------------------------

namespace {
void page(Raw s, std::size_t store, std::string_view name, HudScreenHost& host) {
    s.ptr(store) = host.gui_load_page_00aa5840(name);
}
void find(Raw s, std::size_t store, std::size_t parent, std::string_view name, HudScreenHost& host) {
    s.ptr(store) = host.gui_find_child_00aa7e00(s.ptr(parent), name);
}
void show(Raw s, std::size_t widget, bool visible, HudScreenHost& host) {
    host.widget_set_visible_34(s.ptr(widget), visible);
}
} // namespace

// 00650E00, periscope register: base register; +24h, +2Ch = 0, +60h = -1.0f,
// +1Ch byte 0; page "GUI_periscope" into +6Ch; the layout virtual (+14h,
// 00651230); four effect handles by name (00871BA0) assigned refcounted into
// +30h, +38h, +3Ch, +4Ch; +64h = 1.0f and the bytes +68h..+6Ah cleared.
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 4> kPeriscopeEffects{{
    {0x30, "Warning Dive DIVE Dive"}, {0x38, "Lvl3Water"}, {0x3C, "PeriscopeEmerge"}, {0x4C, "PeriscopeDrops"}}};
void periscope_register_00650e00(Raw s, LayoutHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x24) = nullptr;
    s.ptr(0x2C) = nullptr;
    s.at<float>(0x60) = -1.0f;  // 00D7A260
    s.at<std::uint8_t>(0x1C) = 0;
    page(s, 0x6C, "GUI_periscope", host);
    host.screen_virtual_14(s);
    for (const auto& [off, name] : kPeriscopeEffects) host.handle_assign_acquired(Raw{s.p + off}, host.effect_acquire_by_name_00871ba0(name));
    s.at<float>(0x64) = 1.0f;
    s.at<std::uint8_t>(0x68) = 0;
    s.at<std::uint8_t>(0x69) = 0;
    s.at<std::uint8_t>(0x6A) = 0;
}

// 006463E0, HUD root layout (+14h): the powerup templates, the unit panel and
// the selector. unit_name_Text has its byte +FCh cleared and is nudged by
// (+0.006, -0.005, +0.0) from its resolved position (doubles of those floats,
// 00CEC390 / 00CEC8F8 / 00D7A258).
void hud_root_load_layout_006463e0(Raw s, LayoutHost& host) {
    page(s, 0xF8, "GUI_powerups", host);
    find(s, 0xFC, 0xF8, "puptemplate_Icon", host);
    show(s, 0xFC, false, host);
    find(s, 0x100, 0xF8, "circletemplate_Section", host);
    show(s, 0x100, false, host);
    page(s, 0x2C, "GUI_unit", host);
    find(s, 0x34, 0x2C, "unit_name_Text", host);
    Raw{static_cast<std::uint8_t*>(s.ptr(0x34))}.at<std::uint8_t>(0xFC) = 0;
    float pos[3];
    host.widget_resolved_position_00aa6750(s.ptr(0x34), pos);
    const float moved[3] = {
        static_cast<float>(static_cast<double>(pos[0]) + static_cast<double>(0.006f)),
        static_cast<float>(static_cast<double>(pos[1]) - static_cast<double>(0.005f)),
        static_cast<float>(static_cast<double>(pos[2]) + 0.0)};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x34), moved);
    find(s, 0x64, 0x2C, "FlagJP_Icon", host);
    find(s, 0x68, 0x2C, "FlagUS_Icon", host);
    find(s, 0x50, 0x2C, "unit_payload_Icon", host);
    find(s, 0x7C, 0x2C, "unit_HP_damage_Icon", host);
    find(s, 0x80, 0x2C, "unit_HP_healthy_Icon", host);
    find(s, 0x84, 0x2C, "unit_command_Icon", host);
    page(s, 0x28, "GUI_selector", host);
    show(s, 0x28, false, host);
    find(s, 0x30, 0x28, "Units_Group", host);
    find(s, 0x40, 0x30, "ClosedUnitHUD_Group", host);
    find(s, 0x5C, 0x40, "Medal_Icon", host);
    find(s, 0x60, 0x40, "Medal_Text", host);
    find(s, 0x3C, 0x28, "WeaponInfo_Text", host);
    show(s, 0x40, true, host);
}

// 0064C0F0, ship-status layout (+14h): GUI_ship, GUI_repair, GUI_ship_effects
// and GUI_ship_damage. The stick's resolved y (00AA67F0 -> +4h) is kept in
// +138h; the four highlight icons (+D8h..+E4h) are shown with their colour's
// alpha set to 0 (vtable +54h read, +50h write); the four "Villanas" (Hungarian:
// flash) edge icons of GUI_ship_effects go to +18Ch..+198h and are hidden.
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 10> kShipStatusShipIcons{{
    {0x50, "ship_dir_Icon"}, {0x60, "ship_speed_num1_Icon"}, {0x64, "ship_speed_num2_Icon"},
    {0x68, "ship_recon_num1_Icon"}, {0x6C, "ship_recon_num2_Icon"}, {0x70, "ship_torpedo_num1_Icon"},
    {0x74, "ship_torpedo_num2_Icon"}, {0xF8, "ship_relation_Icon"}, {0x54, "ship_recon_Icon"},
    {0x58, "ship_torpedo_Icon"}}};
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 12> kShipStatusRepairIcons{{
    {0x90, "repair_hl_west_Icon"}, {0x94, "repair_hl_east_Icon"}, {0x98, "repair_hl_north_Icon"},
    {0x9C, "repair_hl_south_Icon"}, {0xA4, "repair_ikons_periscope_Icon"}, {0xA8, "repair_ikons_engine_Icon"},
    {0xAC, "repair_warning_west_Icon"}, {0xB0, "repair_warning_east_Icon"}, {0xB4, "repair_warning_north_Icon"},
    {0xB8, "repair_warning_south_Icon"}, {0xA0, "repair_hl_middle_Icon"}, {0xC0, "repair_Text"}}};
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 9> kShipStatusDamageIcons{{
    {0xCC, "Icon_1_Icon"}, {0xD0, "Icon_2_Icon"}, {0xC4, "Icon_3_Icon"}, {0xC8, "Icon_4_Icon"},
    {0xD4, "Icon_5_Icon"}, {0xD8, "Hl_1_Icon"}, {0xDC, "Hl_2_Icon"}, {0xE0, "Hl_3_Icon"}, {0xE4, "Hl_4_Icon"}}};
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 4> kShipStatusCircles{{
    {0xE8, "circle_1_Section"}, {0xEC, "circle_2_Section"}, {0xF0, "circle_3_Section"}, {0xF4, "circle_4_Section"}}};
inline constexpr std::array<std::pair<std::uint16_t, std::string_view>, 4> kShipStatusFlashes{{
    {0x18C, "VillanasFelso_Icon"}, {0x190, "VillanasAlso_Icon"}, {0x194, "VillanasBal_Icon"}, {0x198, "VillanasJobb_Icon"}}};
void ship_status_load_layout_0064c0f0(Raw s, LayoutHost& host) {
    page(s, 0x34, "GUI_ship", host);
    find(s, 0x48, 0x34, "ship_stick_Icon", host);
    s.at<float>(0x138) = host.widget_vector_00aa67f0(s.ptr(0x48))[1];
    for (const auto& [off, name] : kShipStatusShipIcons) find(s, off, 0x34, name, host);
    page(s, 0x38, "GUI_repair", host);
    for (const auto& [off, name] : kShipStatusRepairIcons) find(s, off, 0x38, name, host);
    page(s, 0x3C, "GUI_ship_effects", host);
    page(s, 0x40, "GUI_ship_damage", host);
    show(s, 0x40, true, host);
    for (const auto& [off, name] : kShipStatusDamageIcons) find(s, off, 0x40, name, host);
    for (std::size_t off = 0xD8; off <= 0xE4; off += 4) {
        show(s, off, true, host);
        float rgba[4];
        host.widget_get_colour_54(s.ptr(off), rgba);
        rgba[3] = 0.0f;
        host.widget_set_colour_50(s.ptr(off), rgba);
    }
    for (const auto& [off, name] : kShipStatusCircles) find(s, off, 0x40, name, host);
    // The four finds run first and the stores follow together (0064D52B..0064D53D).
    void* flashes[4];
    for (std::size_t i = 0; i < 4; ++i) flashes[i] = host.gui_find_child_00aa7e00(s.ptr(0x3C), kShipStatusFlashes[i].second);
    for (std::size_t i = 0; i < 4; ++i) s.ptr(kShipStatusFlashes[i].first) = flashes[i];
    for (const auto& [off, name] : kShipStatusFlashes) show(s, off, false, host);
}

// 0065DAA0, support-manager layout (+14h): the radial menu of the page at +78h
// and the info panel. pum_info_Text is found inside sm_close_Group (the two
// names are prepared before both calls); circle_large_Group gets the scale
// (1.0001, 1.0001) (00CF6634, 00AA86B0) and the slots are rebound (0065D220)
// before the info panel loads; map_Group's resolved position is kept in
// +1B4h..+1BCh. Hungarian names: nagykep = big picture, piros = red,
// vilagos = light, sotet = dark, keret = frame.
void support_manager_load_layout_0065daa0(Raw s, LayoutHost& host) {
    void* close_group = host.gui_find_child_00aa7e00(s.ptr(0x78), "sm_close_Group");
    s.ptr(0x178) = host.gui_find_child_00aa7e00(close_group, "pum_info_Text");
    find(s, 0x12C, 0x78, "circle_full_Section", host);
    find(s, 0x130, 0x78, "circle_left_Section", host);
    find(s, 0x134, 0x78, "circle_right_Section", host);
    find(s, 0xEC, 0x78, "center_Group", host);
    find(s, 0x1CC, 0xEC, "nagykep_Icon", host);
    find(s, 0xF0, 0xEC, "without_pload_Group", host);
    find(s, 0xF4, 0xEC, "pload_Group", host);
    find(s, 0x158, 0xF4, "icon_L_Icon", host);
    find(s, 0xF8, 0x78, "up_1_Icon", host);
    find(s, 0xFC, 0x78, "up_4_Icon", host);
    find(s, 0x100, 0x78, "up_2_Icon", host);
    find(s, 0x104, 0x78, "up_3_Icon", host);
    find(s, 0x138, 0x78, "circle_large_Group", host);
    host.widget_set_scale_00aa86b0(s.ptr(0x138), 1.0001f, 1.0001f);
    host.support_rebind_slots_0065d220(s);
    page(s, 0x16C, "GUI_support_infopanel", host);
    show(s, 0x16C, false, host);
    find(s, 0x170, 0x16C, "unit_name_Text", host);
    find(s, 0x1A8, 0x16C, "resource_Text", host);
    find(s, 0x180, 0x16C, "piros_Icon", host);
    find(s, 0x184, 0x16C, "vilagos_Icon", host);
    find(s, 0x188, 0x16C, "sotet_Icon", host);
    find(s, 0x18C, 0x16C, "keret_FrameBox", host);
    find(s, 0x190, 0x16C, "support_Icon", host);
    find(s, 0x1B0, 0x78, "map_Group", host);
    float pos[3];
    host.widget_resolved_position_00aa6750(s.ptr(0x1B0), pos);
    s.at<float>(0x1B4) = pos[0];
    s.at<float>(0x1B8) = pos[1];
    s.at<float>(0x1BC) = pos[2];
    find(s, 0x1C0, 0x1B0, "minimap_map_Icon", host);
    find(s, 0x1D8, 0x1C0, "SpawnThing_Icon", host);
    find(s, 0x128, 0x1C0, "Unit_Icon", host);
    find(s, 0x1C8, 0x1B0, "minimap_FrameBox", host);
    find(s, 0x1C4, 0x1B0, "powerup_tip_FrameBox", host);
}

// ---------------------------------------------------------------------------
// Batch 6: support-manager fades and icon frames
// ---------------------------------------------------------------------------

// 006723F0, __thiscall(screen, float dt), RET 4: count the close fade +28Ch
// down (float-stored, clamped at 0). Above 0.5 the four widgets +7Ch..+88h get
// alpha 2t - 1 (00AA8610, recursive); at or below it 00670020 runs, +30Ch is
// cleared and the alpha is 1 - 2t. At exactly 0 the screen marks +2B1h and
// clears +288h.
void support_manager_step_close_fade_006723f0(Raw s, float dt, FadeHost& host) {
    const float t = s.at<float>(0x28C) - dt;
    s.at<float>(0x28C) = t;
    if (0.0f > t) s.at<float>(0x28C) = 0.0f;
    const float v = s.at<float>(0x28C);
    if (v > 0.5f) {  // 00CE3800
        for (std::size_t off = 0x7C; off <= 0x88; off += 4) {
            host.widget_set_alpha_recursive_00aa8610(s.ptr(off),
                static_cast<float>(static_cast<double>(v + v) - 1.0));  // 00D7A210
        }
    } else {
        host.support_close_finish_00670020(s);
        s.at<std::uint32_t>(0x30C) = 0;
        for (std::size_t off = 0x7C; off <= 0x88; off += 4) {
            host.widget_set_alpha_recursive_00aa8610(s.ptr(off), 1.0f - (v + v));
        }
    }
    if (s.at<float>(0x28C) == 0.0f) {  // 00D7A218
        s.at<std::uint8_t>(0x2B1) = 1;
        s.at<std::uint8_t>(0x288) = 0;
    }
}

// 0065BD90, __thiscall(screen, float dt), RET 4: while +1E4h is set the name
// label fades in at 4/s (00D7A328) in +1E8h; reaching 1.0 clears +1E4h, shows
// the name text +170h and keeps its pixel width / 960 (00CEC380) in +74h. The
// frame +18Ch is sized (vtable +58h) to (width/960 * fade + 0.21, +198h)
// (00CF6558), the label group +78h takes the fade as its alpha, and a finished
// fade re-lays the label (0065BBC0).
void support_manager_update_name_label_fade_0065bd90(Raw s, float dt, FadeHost& host) {
    if (s.at<std::uint8_t>(0x1E4) == 0) return;
    const float f = static_cast<float>(static_cast<double>(dt) * 4.0 + static_cast<double>(s.at<float>(0x1E8)));
    s.at<float>(0x1E8) = f;
    Raw text{static_cast<std::uint8_t*>(s.ptr(0x170))};
    if (f >= 1.0f) {
        s.at<std::uint8_t>(0x1E4) = 0;
        s.at<float>(0x1E8) = 1.0f;
        host.widget_set_visible_34(s.ptr(0x170), true);
        s.at<float>(0x74) = static_cast<float>(static_cast<double>(text.at<float>(0x114)) / 960.0);
    }
    const float width = static_cast<float>(static_cast<double>(text.at<float>(0x114)) / 960.0);
    const float size[2] = {
        static_cast<float>(static_cast<double>(width) * static_cast<double>(s.at<float>(0x1E8)) + static_cast<double>(0.21f)),
        s.at<float>(0x198)};
    host.widget_set_size_58(s.ptr(0x18C), size);
    host.widget_set_alpha_recursive_00aa8610(s.ptr(0x78), s.at<float>(0x1E8));
    if (s.at<std::uint8_t>(0x1E4) == 0) host.support_layout_name_label_0065bbc0(s);
}

// 00652BA0, __stdcall(slot), RET 4 -> AL: the icon frame of a slot whose unit
// (+8h) is present (+4h): kind queries through the unit's vtable +18h.
int hud_unit_icon_frame_00652ba0(Raw slot, IconFrameHost& host) {
    if (slot.ptr(0x4) == nullptr) return 0;
    void* unit = slot.ptr(0x8);
    if (host.unit_has_kind_18(unit, 0x16)) return 0x10;
    if (host.unit_has_kind_18(unit, 0xE)) return 0xD;
    if (host.unit_has_kind_18(unit, 0x15)) return 0x10;
    if (host.unit_has_kind_18(unit, 7)) return 4;
    if (host.unit_has_kind_18(unit, 8)) return 5;
    const bool flag = Raw{static_cast<std::uint8_t*>(unit)}.at<std::uint8_t>(0x808) != 0;
    if (host.unit_has_kind_18(unit, 0xD)) return flag ? 0x15 : 0xC;
    if (host.unit_has_kind_18(unit, 10)) return flag ? 8 : 7;
    return 0xE;
}

// 00658620, __thiscall(screen, record), RET 4 -> AL: the support-manager icon
// frame of a slot record. While the screen's +1DCh is clear: state 5 -> 20h; a
// launched unit (+28h) out (state 3/4) whose type (vtable +174h) is 00E08FA0 ->
// 21h; state 1 -> 21h. Otherwise +298h counts the call and the record's unit
// (+4h) picks: kind 12h -> 0Eh, 13h or 17h -> 0Fh, 10h -> 11h, 11h -> 13h, else 11h.
inline constexpr std::uint32_t kSupportOutUnitType = 0x00E08FA0;
int support_manager_record_icon_frame_00658620(Raw s, Raw record, IconFrameHost& host) {
    const std::int32_t state = record.at<std::int32_t>(0x2C);
    const bool closed = s.at<std::uint8_t>(0x1DC) == 0;
    if (state == 5 && closed) return 0x20;
    if (record.ptr(0x28) != nullptr && (state == 3 || state == 4)
        && host.unit_type_174(record.ptr(0x28)) == kSupportOutUnitType && closed) {
        return 0x21;
    }
    if (record.at<std::int32_t>(0x2C) == 1 && closed) return 0x21;
    s.at<std::int32_t>(0x298) += 1;
    void* unit = record.ptr(0x4);
    if (host.unit_has_kind_18(unit, 0x12)) return 0xE;
    if (host.unit_has_kind_18(unit, 0x13) || host.unit_has_kind_18(unit, 0x17)) return 0xF;
    if (host.unit_has_kind_18(unit, 0x10)) return 0x11;
    return host.unit_has_kind_18(unit, 0x11) ? 0x13 : 0x11;
}

// ---------------------------------------------------------------------------
// fe2b: rewards bind layout 0061FBE0
// ---------------------------------------------------------------------------

namespace {
// Scroller set-up shared by the three scroll panes of 0061FBE0: bind the pane's
// move group (00683790), then 00683380(thumb, track, up, down) with track =
// back icon y - thumb y (00AA6740 -> +4h; the back y is held as a double and the
// difference is rounded once to float).
void rewards_setup_scroller(Raw s, std::size_t block, void* move_group, void* back, std::size_t up,
    std::size_t down, std::size_t thumb, RewardsLayoutHost& host) {
    host.scroller_bind_00683790(Raw{s.p + block}, move_group);
    const double back_y = host.widget_vector_00aa6740(back)[1];
    const float track = static_cast<float>(back_y - static_cast<double>(host.widget_vector_00aa6740(s.ptr(thumb))[1]));
    host.scroller_setup_00683380(Raw{s.p + block}, s.ptr(thumb), track, s.ptr(up), s.ptr(down));
}
void rewards_listen(Raw s, std::size_t widget, RewardsLayoutHost& host) {
    host.widget_set_listener_00aa6bc0(s.ptr(widget), s.p + 0xC, 0);
}
} // namespace

// 0061FBE0, rewards bind layout (+14h), ECX = screen. The six page groups of
// FE_scoring (+20h) are found by the names the char* cells 00E08C70..00E08C84
// hold (single_page1, multi_page1, objectives_page, detail_page,
// multiscore_page, awards_page _Group in this installation's image); every
// arrow, scroll icon and thumb gets the widget listener at +0Ch (00AA6BC0(+0Ch,
// 0)). The three scroll panes are the debriefing (+68h), detail (+128h) and
// awards (+19Ch) blocks. The tail labels pop_Group's A_Text with the key of
// action A2h (00531030 / 004C5E60 / 00531130, then 00AB80C0 and 00ABAED0).
void rewards_bind_layout_0061fbe0(Raw s, RewardsLayoutHost& host) {
    for (std::size_t off = 0x40; off <= 0x54; off += 4) s.ptr(off) = nullptr;
    find(s, 0x5C, 0x20, "prev_page_Icon", host);
    rewards_listen(s, 0x5C, host);
    find(s, 0x60, 0x20, "next_page_Icon", host);
    rewards_listen(s, 0x60, host);

    // Debriefing page.
    s.ptr(0x40) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C70));
    void* slider = host.gui_find_child_00aa7e00(s.ptr(0x40), "slider_Group");
    s.ptr(0xC4) = host.gui_find_child_00aa7e00(slider, "scroll_up_Icon");
    rewards_listen(s, 0xC4, host);
    s.ptr(0xC8) = host.gui_find_child_00aa7e00(slider, "scroll_down_Icon");
    rewards_listen(s, 0xC8, host);
    s.ptr(0xCC) = host.gui_find_child_00aa7e00(slider, "slider_FrameBox");
    rewards_listen(s, 0xCC, host);
    void* back = host.gui_find_child_00aa7e00(slider, "back_Icon");
    void* clip = host.gui_find_child_00aa7e00(s.ptr(0x40), "debriefing_Clipbox");
    s.at<float>(0xD0) = host.widget_vector_00aa6740(clip)[1];
    void* move = host.gui_find_child_00aa7e00(clip, "move_Group");
    rewards_setup_scroller(s, 0x68, move, back, 0xC4, 0xC8, 0xCC, host);

    // Multiplayer page.
    s.ptr(0x44) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C74));
    find(s, 0xD4, 0x44, "players_Listbox", host);
    find(s, 0xD8, 0xD4, "template_Group", host);
    show(s, 0xD8, false, host);

    // Objectives page.
    s.ptr(0x48) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C78));
    find(s, 0xDC, 0x48, "obj_template_Group", host);
    show(s, 0xDC, false, host);
    find(s, 0xE0, 0x48, "primary_FrameBox", host);
    find(s, 0xE4, 0x48, "secondary_FrameBox", host);
    find(s, 0xE8, 0x48, "hidden_FrameBox", host);
    find(s, 0xEC, 0x48, "primary_header_Text", host);
    find(s, 0xF0, 0x48, "secondary_header_Text", host);
    find(s, 0xF4, 0x48, "hidden_header_Text", host);

    // Detail page.
    s.ptr(0x4C) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C7C));
    find(s, 0x108, 0x4C, "clipbox_Clipbox", host);
    find(s, 0x10C, 0x108, "header_template_Group", host);
    find(s, 0x110, 0x108, "key_template_Group", host);
    find(s, 0x114, 0x108, "value_template_Group", host);
    find(s, 0x118, 0x108, "move_Group", host);
    show(s, 0x10C, false, host);
    show(s, 0x110, false, host);
    show(s, 0x114, false, host);
    show(s, 0x118, true, host);
    slider = host.gui_find_child_00aa7e00(s.ptr(0x4C), "slider_Group");
    back = host.gui_find_child_00aa7e00(slider, "back_Icon");
    s.ptr(0x120) = host.gui_find_child_00aa7e00(slider, "scroll_down_Icon");
    rewards_listen(s, 0x120, host);
    s.ptr(0x11C) = host.gui_find_child_00aa7e00(slider, "scroll_up_Icon");
    rewards_listen(s, 0x11C, host);
    s.ptr(0x124) = host.gui_find_child_00aa7e00(slider, "slider_FrameBox");
    rewards_listen(s, 0x124, host);
    rewards_setup_scroller(s, 0x128, s.ptr(0x118), back, 0x11C, 0x120, 0x124, host);

    // Awards page.
    s.ptr(0x54) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C84));
    find(s, 0x184, 0x54, "awards_Clipbox", host);
    find(s, 0x188, 0x184, "move_Group", host);
    find(s, 0x18C, 0x188, "template_Group", host);
    show(s, 0x188, true, host);
    show(s, 0x18C, false, host);
    slider = host.gui_find_child_00aa7e00(s.ptr(0x54), "slider_Group");
    back = host.gui_find_child_00aa7e00(slider, "back_Icon");
    s.ptr(0x194) = host.gui_find_child_00aa7e00(slider, "scroll_down_Icon");
    rewards_listen(s, 0x194, host);
    s.ptr(0x190) = host.gui_find_child_00aa7e00(slider, "scroll_up_Icon");
    rewards_listen(s, 0x190, host);
    s.ptr(0x198) = host.gui_find_child_00aa7e00(slider, "slider_FrameBox");
    rewards_listen(s, 0x198, host);
    rewards_setup_scroller(s, 0x19C, s.ptr(0x188), back, 0x190, 0x194, 0x198, host);

    // Multiplayer score page.
    s.ptr(0x50) = host.gui_find_child_00aa7e00(s.ptr(0x20), host.global_cstr(0x00E08C80));
    find(s, 0x208, 0x50, "rank_Icon", host);
    find(s, 0x1F8, 0x50, "player_Group", host);
    find(s, 0x1FC, 0x1F8, "multiscore_var_Text", host);
    find(s, 0x200, 0x1F8, "playername_var_Text", host);
    find(s, 0x204, 0x1F8, "rank_var_Text", host);
    find(s, 0x20C, 0x1F8, "rank_prog_Icon", host);
    find(s, 0x210, 0x1F8, "rank_prog_new_Icon", host);
    find(s, 0x214, 0x1F8, "rankgraf_FrameBox", host);

    // The rank-up pop-up and its A button label.
    find(s, 0x25C, 0x20, "pop_Group", host);
    void* text = host.gui_find_child_00aa7e00(s.ptr(0x25C), "A_Text");
    host.text_set_key_glyph_00531130(text, host.action_key_name_00531030(0xA2), s.p + 0xC);
    host.widget_set_listener_00aa6bc0(text, nullptr, 0);
    host.text_refresh_00ab80c0(text);
    host.text_set_localised_source_00abaed0(text, host.action_key_name_00531030(0xA2), true);
}

// 00629D10, rewards register: base register, page FE_scoring into +20h, the
// bind virtual (+14h, 0061FBE0), then the global vector of page-title keys at
// 00E196BC (begin 00E196C0, end 00E196C4) is emptied (004954F0) and refilled
// in page order (00450540); +3Ch byte cleared.
inline constexpr std::uint32_t kRewardsPageTitleVector = 0x00E196BC;
inline constexpr std::array<std::string_view, 6> kRewardsPageTitleKeys{
    "FE.scoring_single_page1", "FE.scoring_multi_page1", "FE.scoring_objectives",
    "FE.scoring_detailed_score", "FE.scoring_rank", "FE.scoring_awards"};
void rewards_register_00629d10(Raw s, RewardsRegisterHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x20) = host.gui_load_page_00aa5840("FE_scoring");
    host.screen_virtual_14(s);
    host.string_vector_clear_004954f0(kRewardsPageTitleVector);
    for (std::string_view key : kRewardsPageTitleKeys) host.string_vector_append_00450540(kRewardsPageTitleVector, key);
    s.at<std::uint8_t>(0x3C) = 0;
}

// 0064BB90, ship-status register (45h): base register; effect handles 1..4
// (00870CD0, ECX = &out, EDX = id, (1)) assigned refcounted into +0Ch..+18h;
// +44h = -1.0f and +124h..+12Ch = 0.0f; the layout virtual (+14h, 0064C0F0);
// the relation icon's resolved position into +24h..+2Ch; +40h hidden and the
// four damage highlights (+D8h..+E4h) shown; the screen state reset (the unit
// observation through member +170h / observed +184h is dropped); the
// "environment/environment_unavailable.fsb" sample (*00F8BBE8 cache) into +1Ch.
void ship_status_register_0064bb90(Raw s, ShipStatusRegisterHost& host) {
    host.front_end_screen_register_004f71d0(s);
    for (int id = 1; id <= 4; ++id) {
        host.handle_assign_acquired(Raw{s.p + 0x8 + static_cast<std::size_t>(id) * 4}, host.effect_acquire_00870cd0(id));
    }
    s.at<float>(0x44) = -1.0f;  // 00D7A260
    s.at<float>(0x128) = 0.0f;
    s.at<float>(0x12C) = 0.0f;
    s.at<float>(0x124) = 0.0f;
    host.screen_virtual_14(s);
    float pos[3];
    host.widget_resolved_position_00aa6750(s.ptr(0xF8), pos);
    s.at<float>(0x24) = pos[0];
    s.at<float>(0x28) = pos[1];
    s.at<float>(0x2C) = pos[2];
    show(s, 0x40, false, host);
    for (std::size_t off = 0xD8; off <= 0xE4; off += 4) show(s, off, true, host);
    s.at<std::uint32_t>(0x100) = 0;
    s.at<std::uint8_t>(0x155) = 0;
    s.at<std::uint8_t>(0x156) = 0;
    s.at<std::uint8_t>(0x157) = 0;
    s.at<std::uint32_t>(0x158) = 0;
    s.at<std::uint32_t>(0x14C) = 0;
    s.at<std::int32_t>(0x150) = -1;
    s.at<std::uint8_t>(0x144) = 1;
    if (s.ptr(0x184) != nullptr) {
        host.observer_unregister_006952a0(s.p + 0x170, s.ptr(0x184));
        s.ptr(0x184) = nullptr;
    }
    s.at<float>(0x160) = -1.0f;
    s.at<std::uint8_t>(0x8) = 0;
    s.at<std::uint8_t>(0x105) = 0;
    s.at<std::uint8_t>(0x106) = 0;
    s.at<std::uint8_t>(0x107) = 0;
    s.at<std::uint8_t>(0x104) = 0;
    s.at<std::uint8_t>(0x119) = 0;
    s.at<std::uint32_t>(0x30) = 0;
    host.handle_assign_acquired(Raw{s.p + 0x1C}, host.sound_sample_acquire_by_name("environment/environment_unavailable.fsb"));
}

// 0067F080, unit library register (05h): page UnitLibGui into +6Ch, the
// selection state cleared (+E4h = 1), then the preview scene is set up: scene
// init (0093CCC0), camera render mode 0 and clear flags 7 on the camera +10h
// (00B6FDF0 / 00B6FE10); the first directional light of the scene's lighting
// owner (+0Ch, 00B72110 -> +1Ch list) points along (1, 2, 0) / sqrt(5) (the
// squared length is formed as 0*0 + 5.0 and rounded to float before 00BF7030;
// 0 when not positive) with base diffuse (0.6, 0.6, 0.6, 1.0) (004B62E0);
// ambient (0.2, 0.2, 0.2, 1.0) (00B7AF20); *00F8BBF0 slot +4h(byte 00F88A04);
// render resources (00B107F0 on *00F8D39C with 00F88A0C, 00F88A0D,
// 00F889D8); motion blur {0, 0, 100, 600, 100, 0, 0, 0} (00B0EA20); HDR
// {0.6, 4, 0.5, 0.04, 0, 0.5, 3, 3, 10, byte 0, 1.0} (00B0E9B0); a shadow owner
// for the light (00A8FD30 / 00B7BDF0, temporary reference dropped); +108h and
// +10Ch cleared; VehicleClasses.lua and DeviceClasses.lua run (00B69D40);
// +130h = 1.
inline constexpr float kUnitLibraryMotionBlur[8] = {0.0f, 0.0f, 100.0f, 600.0f, 100.0f, 0.0f, 0.0f, 0.0f};
struct UnitLibraryHdr {
    float values[9];
    std::uint8_t flag;
    float scale;
};
inline constexpr UnitLibraryHdr kUnitLibraryHdr{{0.6f, 4.0f, 0.5f, 0.04f, 0.0f, 0.5f, 3.0f, 3.0f, 10.0f}, 0, 1.0f};
void unit_library_register_0067f080(Raw s, UnitLibraryRegisterHost& host) {
    host.front_end_screen_register_004f71d0(s);
    page(s, 0x6C, "UnitLibGui", host);
    for (std::size_t off = 0xEC; off <= 0x100; off += 4) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint8_t>(0x104) = 0;
    s.at<std::uint32_t>(0xE4) = 1;
    host.simple3d_scene_init_0093ccc0(Raw{s.p + 0x8});
    host.camera_set_render_mode_00b6fdf0(s.ptr(0x10), 0);
    host.camera_set_clear_flags_00b6fe10(s.ptr(0x10), 7);
    void* light = host.scene_first_directional_light(s.ptr(0xC));
    const float squared = static_cast<float>(static_cast<double>(0.0f * 0.0f) + 5.0);  // 00D7A370
    const float length = static_cast<float>(std::sqrt(static_cast<double>(squared)));
    const float inv = length > 0.0f ? 1.0f / length : 0.0f;
    Raw l{static_cast<std::uint8_t*>(light)};
    l.at<float>(0x1E0) = inv;
    l.at<float>(0x1E4) = inv + inv;
    l.at<float>(0x1E8) = static_cast<float>(static_cast<double>(inv) * 0.0);  // 00D7A258
    const float diffuse[4] = {0.6f, 0.6f, 0.6f, 1.0f};
    host.light_set_base_diffuse_004b62e0(light, diffuse);
    const float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    host.scene_set_ambient_00b7af20(s.ptr(0xC), ambient);
    host.render_flag_hook_f8bbf0_04();
    host.render_resources_init_00b107f0();
    host.scene_renderer_set_motion_blur_00b0ea20(kUnitLibraryMotionBlur);
    host.scene_renderer_set_hdr_00b0e9b0(kUnitLibraryHdr);
    host.light_attach_new_shadow_owner(light);
    s.ptr(0x108) = nullptr;
    s.ptr(0x10C) = nullptr;
    host.lua_run_script_00b69d40("SCRIPTS/datatables/autoload/VehicleClasses.lua");
    host.lua_run_script_00b69d40("SCRIPTS/datatables/autoload/DeviceClasses.lua");
    s.at<std::uint32_t>(0x130) = 1;
}

// ---------------------------------------------------------------------------
// fe2b: session messages of segments 26/27
// ---------------------------------------------------------------------------

const SessionMessageClass* find_session_message_class(std::uint8_t type) noexcept {
    for (const SessionMessageClass& c : kSessionMessageClasses) {
        if (c.type == type) return &c;
    }
    return nullptr;
}

// IsType: the class's own type or one of its accepted base types.
bool session_message_is_type(const SessionMessageClass& c, std::int32_t type) noexcept {
    if (type == c.type) return true;
    for (std::uint8_t i = 0; i < c.base_count; ++i) {
        if (type == c.bases[i]) return true;
    }
    return false;
}

// Constructor: 0075B430(type), clear the +18h word and +1Ah byte, store the
// arguments, +4h = 1, then the class vtable.
Raw session_message_construct(const SessionMessageClass& c, Raw m, std::uint32_t arg0, std::uint8_t arg1,
    SessionMessageHost& host) {
    host.construct_base_0075b430(m, c.type);
    m.at<std::uint16_t>(0x18) = 0;
    m.at<std::uint8_t>(0x1A) = 0;
    switch (c.ctor) {
    case SessionCtorShape::Dword1C:
        m.at<std::uint32_t>(0x1C) = arg0;
        break;
    case SessionCtorShape::Clear1C:
        m.at<std::uint8_t>(0x1C) = 0;
        break;
    case SessionCtorShape::Clear1CByte20:
        m.at<std::uint8_t>(0x1C) = 0;
        m.at<std::uint8_t>(0x20) = static_cast<std::uint8_t>(arg0);
        break;
    case SessionCtorShape::Clear1CDword20Byte24:
        m.at<std::uint8_t>(0x1C) = 0;
        m.at<std::uint32_t>(0x20) = arg0;
        break;
    case SessionCtorShape::None:
        break;
    }
    m.at<std::uint32_t>(0x4) = 1;
    m.at<std::uint32_t>(0x0) = c.vtable;
    if (c.ctor == SessionCtorShape::Clear1CDword20Byte24) m.at<std::uint8_t>(0x24) = arg1;
    return m;
}

// Write / Read: the extended header (0075B480 / 0075B4C0), then the fields in
// order.
void session_message_write(const SessionMessageClass& c, Raw m, void* cursor, SessionMessageHost& host) {
    host.write_header_0075b480(m, cursor);
    for (std::uint8_t i = 0; i < c.field_count; ++i) {
        const SessionField& f = c.fields[i];
        switch (f.kind) {
        case SessionFieldKind::Bool: host.write_bool_004290b0(cursor, m.at<std::uint8_t>(f.offset)); break;
        case SessionFieldKind::Signed: host.write_signed_00429090(cursor, m.at<std::int32_t>(f.offset), f.bits); break;
        case SessionFieldKind::Unsigned: host.write_unsigned_00429070(cursor, m.at<std::uint32_t>(f.offset), f.bits); break;
        }
    }
}

// The field reads take ECX = cursor + 4h (006503C0 `add edi,4; mov ecx,edi`,
// 00605300 / 0064A7E0 `lea ecx,[edi+4]`); the header read takes the cursor.
void session_message_read(const SessionMessageClass& c, Raw m, void* cursor, SessionMessageHost& host) {
    host.read_header_0075b4c0(m, cursor);
    void* const bits = static_cast<std::uint8_t*>(cursor) + 4;
    for (std::uint8_t i = 0; i < c.field_count; ++i) {
        const SessionField& f = c.fields[i];
        switch (f.kind) {
        case SessionFieldKind::Bool: host.read_bool_00428d70(bits, m.at<std::uint8_t>(f.offset)); break;
        case SessionFieldKind::Signed: host.read_signed_00428d30(bits, m.at<std::int32_t>(f.offset), f.bits); break;
        case SessionFieldKind::Unsigned: host.read_unsigned_00428d10(bits, m.at<std::uint32_t>(f.offset), f.bits); break;
        }
    }
}

// 006563A0, message 82h read: header, bool +1Ch, 6 bits +20h, 3 bits +24h, a
// presence bit for the 12-bit word +28h (0 when absent), 4 bits into +2Ch
// (through a local). Field reads on cursor + 4h (006563B2 `add edi,4`).
void session_message130_read_006563a0(Raw m, void* cursor, SessionMessageHost& host) {
    host.read_header_0075b4c0(m, cursor);
    void* const bits = static_cast<std::uint8_t*>(cursor) + 4;
    host.read_bool_00428d70(bits, m.at<std::uint8_t>(0x1C));
    host.read_unsigned_00428d10(bits, m.at<std::uint32_t>(0x20), 6);
    host.read_unsigned_00428d10(bits, m.at<std::uint32_t>(0x24), 3);
    std::uint8_t present = 0;
    host.read_bool_00428d70(bits, present);
    if (present == 0) {
        m.at<std::uint16_t>(0x28) = 0;
    } else {
        host.read_word_bits_00428e30(bits, m.at<std::uint16_t>(0x28), 12);
    }
    std::uint32_t last = 0;
    host.read_unsigned_00428d10(bits, last, 4);
    m.at<std::uint32_t>(0x2C) = last;
}

// Scalar deleting destructors (identical): the root vtable 00CE4974, freed
// when bit 0 of the flag is set; returns the message.
Raw session_message_scalar_delete(Raw m, std::uint8_t flags, SessionMessageHost& host) {
    m.at<std::uint32_t>(0x0) = kSessionMessageRootVtable;
    if ((flags & 1) != 0) host.free_block(m.p);
    return m;
}

// ---------------------------------------------------------------------------
// cc13-fe10 additions (docs/CC13_FE_10.md): read from the live decompile and
// disasm-raw.
// ---------------------------------------------------------------------------
void rewards_play_test_speech_0061f9d0(Raw s, Raw sample, Fe10GuiHost& host) {
    host.handle_assign_acquired(Raw{s.p + 0x10}, host.sound_sample_acquire_00a83fd0(sample));
    std::uint32_t& guard = host.global_block(kTestSpeechGuard).at<std::uint32_t>(0);
    if ((guard & 1) == 0) {
        guard |= 1;
        host.global_block(kTestSpeechClass).ptr(0) = host.sound_find_class_00a7acf0("GUITestSpeech");
    }
    const Raw speech{static_cast<std::uint8_t*>(host.global_object(0x00E198AC).ptr(0x60))};
    void* const sound_type = host.sound_find_type_00a7b0a0("Normal");
    void* const sound = host.sound_create_tracked_00a7e490(speech.ptr(0x10), host.global_block(kTestSpeechClass).ptr(0),
                                                          sound_type);
    host.handle_assign_acquired(Raw{speech.p + 0x14}, sound);
}

void rewards_on_listbox_selection_changed_00621870(Raw listener, void* /*listbox*/, void* widget, Fe10GuiHost& host) {
    const bool navigate = listener.at<std::uint8_t>(0x15) != 0;
    const std::uint8_t nav_command = navigate ? 0xB3 : 0x00;
    const std::string_view nav_label = navigate ? "globals.navigate" : "";
    if (widget != listener.ptr(0xCC) || listener.at<std::int32_t>(0x50) != 1) return;
    const Raw screen{listener.p - 8};
    if (!host.rewards_selected_entry_flagged_0061d8b0(screen)) {
        const MainMenuCommandArgument commands[5] = {{0xA2, "globals.continue", 0}, {nav_command, nav_label, 1},
                                                     {0x00, "", 1}, {0x00, "", 1}, {0x00, "", 1}};
        host.command_bar_rebuild_0054b530(commands);
        return;
    }
    const bool own = host.rewards_selected_entry_xuid_0061d710(screen) == host.xenon_active_xuid_00a3eb00();
    const MainMenuCommandArgument commands[5] = {
        {0xA2, "globals.continue", 0}, {nav_command, nav_label, 1},
        own ? MainMenuCommandArgument{0x00, "", 1} : MainMenuCommandArgument{0xA7, "globals.playerreview", 1},
        {0xA5, "globals.showgamercard", 2}, {0x00, "", 1}};
    host.command_bar_rebuild_0054b530(commands);
}

std::int32_t rewards_collect_vehicle_unlocks_00622360(Raw s, Raw id, bool count_only, Fe10GuiHost& host) {
    alignas(4) std::uint8_t globals[0x14], vehicles[0x14], key[0x14], value[0x14], name[0x14];
    host.lua_globals_00b67980(globals);
    host.lua_get_by_name_00b67800(globals, vehicles, "VehicleClass");
    host.lua_destruct_00b67700(globals);
    host.lua_construct_00b65f50(key);
    host.lua_construct_00b65f50(value);
    host.lua_construct_00b65f50(name);
    std::int32_t count = 0;
    host.lua_iterate_first_00b67080(vehicles, key, value);
    while (!host.lua_iterate_ended_00b66420(vehicles, value)) {
        if (host.lua_is_table_00b661b0(value)) {
            alignas(4) std::uint8_t unlock[0x14];
            host.lua_get_by_name_00b67800(value, unlock, "Unlock");
            if (host.lua_is_boolean_00b66000(unlock) && host.lua_get_boolean_00b66250(unlock)) {
                alignas(4) std::uint8_t unlock_id[0x14];
                host.lua_get_by_name_00b67800(value, unlock_id, "UnlockID");
                if (!host.lua_is_nil_00b65fb0(unlock_id) && host.lua_is_string_00b660a0(unlock_id)) {
                    alignas(4) std::uint8_t haystack[8] = {}, needle[8] = {};
                    host.native_string_assign_cstr_0041e870(Raw{haystack}, host.lua_get_string_00b662b0(unlock_id));
                    host.native_string_upper_ascii_004fb1f0(Raw{haystack});
                    host.native_string_copy_construct_00426060(Raw{needle}, id);
                    host.native_string_upper_ascii_004fb1f0(Raw{needle});
                    const char* const h = Raw{haystack}.at<const char*>(4);
                    const char* const n = Raw{needle}.at<const char*>(4);
                    if (h != nullptr && n != nullptr && std::strstr(h, n) != nullptr) {
                        alignas(4) std::uint8_t field[0x14];
                        host.lua_get_by_name_00b67800(value, field, "Name");
                        host.lua_assign_00b67690(name, field);
                        host.lua_destruct_00b67700(field);
                        const std::int32_t index = host.lua_get_integer_00b66290(key);
                        ++count;
                        if (!count_only) {
                            const Raw records{s.p + 0x238};
                            auto size = [&] {
                                auto* const first = static_cast<std::uint8_t*>(records.ptr(4));
                                return first ? static_cast<std::int32_t>((static_cast<std::uint8_t*>(records.ptr(8)) - first) / 0x10) : 0;
                            };
                            host.record10_vector_at_0061c960(records, size() - count).at<std::int32_t>(4) = index;
                            const char* const text = host.lua_get_string_00b662b0(name);
                            host.native_string_assign_cstr_0041e350(
                                Raw{host.record10_vector_at_0061c960(records, size() - count).p + 8}, text);
                        }
                    }
                    host.native_string_release(Raw{needle});
                    host.native_string_release(Raw{haystack});
                }
                host.lua_destruct_00b67700(unlock_id);
            }
            host.lua_destruct_00b67700(unlock);
        }
        host.lua_iterate_next_00b67190(vehicles, key, value);
    }
    host.lua_destruct_00b67700(name);
    host.lua_destruct_00b67700(value);
    host.lua_destruct_00b67700(key);
    host.lua_destruct_00b67700(vehicles);
    return count;
}

void rewards_fill_rank_page_0062a970(Raw s, Fe10GuiHost& host) {
    void* const group = s.ptr(0x25C);
    host.widget_set_visible_34(group, true);
    const bool pc_prompt = host.global_block(0x00F88A30).at<std::uint8_t>(0) != 0;
    host.widget_set_visible_34(host.gui_find_child_00aa7e00(group, "A_Text"), pc_prompt);
    host.text_set_localised_cstr_00abbe50(host.gui_find_child_00aa7e00(group, "B_Text"),
                                          pc_prompt ? "globals.continue" : "FE_pc.init_enter");
    const Raw first{static_cast<std::uint8_t*>(s.ptr(0x23C))};
    if (first.at<std::uint8_t>(0) != 0) {
        host.text_set_localised_cstr_00abbe50(host.gui_find_child_00aa7e00(group, "info_Text"), "FE.scoring_unlock_text");
        host.widget_set_visible_34(host.gui_find_child_00aa7e00(group, "rank_Icon"), false);
        host.text_set_localised_source_native_00abaed0(host.gui_find_child_00aa7e00(group, "rank_Text"),
                                                       Raw{first.p + 8}, true);
        host.widget_set_visible_34(host.gui_find_child_00aa7e00(group, "unit_Icon"), true);
        host.icon_atlas_apply_004fb150(first.at<std::int32_t>(4), host.gui_find_child_00aa7e00(group, "unit_Icon"));
        host.record10_vector_push_back_006278d0(Raw{s.p + 0x248}, Raw{static_cast<std::uint8_t*>(s.ptr(0x23C))});
        return;
    }
    host.text_set_localised_cstr_00abbe50(host.gui_find_child_00aa7e00(group, "info_Text"), "FE.scoring_rank_text");
    host.widget_set_visible_34(host.gui_find_child_00aa7e00(group, "rank_Icon"), true);
    const std::int32_t rank = static_cast<std::int32_t>(host.session_filter_rank_setting_00574050()) - 1;
    const std::int32_t frame = rank > 0 ? rank : 0;
    host.widget_virtual_88(host.gui_find_child_00aa7e00(group, "rank_Icon"), frame, 0, 1.0f);
    host.text_set_localised_cstr_00abbe50(host.gui_find_child_00aa7e00(group, "rank_Text"),
                                          host.global_block(kPlayerRankKeys + frame * 4).at<const char*>(0));
    host.widget_set_visible_34(host.gui_find_child_00aa7e00(group, "unit_Icon"), false);
}

Raw rewards_record10_assign_0061f5e0(Raw dst, Raw src, Fe10GuiHost& host) {
    dst.at<std::uint8_t>(0) = src.at<std::uint8_t>(0);
    dst.at<std::uint32_t>(4) = src.at<std::uint32_t>(4);
    if (dst.p + 8 != src.p + 8) host.native_string_resize_copy_0041dd40(Raw{dst.p + 8}, Raw{src.p + 8});
    return dst;
}

void hud_widget_holder_hide_00637590(Raw holder, Fe10GuiHost& host) {
    host.widget_set_visible_34(holder.ptr(4), false);
}

void hud_widget_holder_show_at_006375a0(Raw holder, float x, float y, const float* colour, Raw key, Fe10GuiHost& host) {
    host.widget_set_local_xy_00aa7d00(holder.ptr(4), x, y);
    host.widget_set_colour_50(holder.ptr(4), colour);
    host.text_set_localised_source_native_00abaed0(holder.ptr(4), key, false);
    host.widget_set_visible_34(holder.ptr(4), true);
}

void hud_slot46_bind_weapon_group_unit_0064a380(Raw s, Fe10GuiHost& host) {
    host.weapon_group_bind_unit_00549260(s.ptr(0x1C), s.ptr(0x20));
    void* const unit = s.ptr(0x1C);
    if (unit == nullptr) return;
    if (!host.observer_is_registered_00694af0(s.p + 8, unit)) host.observer_register_00694a60(s.p + 8, unit);
}

void hud_slot46_release_weapon_group_0064a3c0(Raw s, Fe10GuiHost& host) {
    host.weapon_group_release_005470c0();
    if (s.ptr(0x1C) != nullptr) host.unit_send_role_transfer_0077c470(s.ptr(0x1C), 0x1FE, 0);
}

namespace {
// The pose-refresh idiom: 00414DB0 when byte +C8h is clear.
void refresh_pose(void* entity, Fe10GuiHost& host) {
    if (static_cast<const std::uint8_t*>(entity)[0xC8] == 0) host.entity_refresh_world_00414db0(entity);
}
const float* world_position(void* entity) {
    return reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(entity) + 0xFC);
}
}  // namespace

void markers_add_group_member_marker_006434e0(Raw s, void* unit, std::int32_t b, Fe10GuiHost& host) {
    if (host.markers_contains_unit_0063abd0(s, unit)) return;
    refresh_pose(unit, host);
    float pos[3] = {world_position(unit)[0], world_position(unit)[1], world_position(unit)[2]};
    if (host.unit_has_kind_5c(unit, 8) && host.entity_below_altitude_third_00852860(unit)) {
        void* const camera = host.global_object(0x00E188A8).ptr(0x1ED4);
        refresh_pose(camera, host);
        if (world_position(camera)[1] > 0.0f) pos[1] = 0.0f;
    }
    float screen[3];
    if (!host.marker_project_and_clip_00638e50(pos, screen)) return;
    if (!host.unit_sensors_detect_00804350(s.ptr(0x24), unit, 5)) return;
    if (!host.interface_cc_accepts_unit_005220c0(unit)) return;
    host.markers_add_unit_marker_006430c0(s, unit, 0, b, 0);
}

void markers_update_screen_heading_006394b0(Raw s, Fe10GuiHost& host) {
    auto* const t = static_cast<std::uint8_t*>(host.global_object(0x00E188A8).ptr(0x19FC));
    if ((t[0x5C] & 2) == 0) host.transform_refresh_world_00b6db70(t);
    const Raw m{t};
    const float fx = static_cast<float>(m.at<float>(0x110) * kHeadingProbeDistance);
    const float fy = static_cast<float>(m.at<float>(0x114) * kHeadingProbeDistance);
    const float fz = static_cast<float>(kHeadingProbeDistance * m.at<float>(0x118));
    const float ay = m.at<float>(0x124) + fy;
    const float az = m.at<float>(0x128) + fz;
    const float ax = m.at<float>(0x120) + fx;
    const float a[3] = {ax, ay, az};
    const float b[3] = {ax + 0.0f, static_cast<float>(ay + kHeadingProbeUp), 0.0f + az};
    float sa[3], sb[3];
    host.camera_project_0043a660(a, sa, 2, 0);
    host.camera_project_0043a660(b, sb, 2, 0);
    const float dx = sb[0] - sa[0];
    const float dy = sb[1] - sa[1];
    const double length_sq = static_cast<double>(dx) * dx + static_cast<double>(dy) * dy;
    if (!(length_sq > kHeadingMinLengthSq)) return;
    const float angle = static_cast<float>(host.crt_atan2_00bf701a(dx, dy));
    s.at<float>(0x2C) = static_cast<float>(angle - kHeadingQuarterTurn);
}

bool hud_marker_is_within_display_range_00639810(void* unit, Fe10GuiHost& host) {
    if (host.game_controlled_vehicle_004b4b00() == nullptr || unit == nullptr) return false;
    void* const me = host.global_object(0x00E188D8).p;
    refresh_pose(me, host);
    refresh_pose(unit, host);
    const float delta[3] = {world_position(unit)[0] - world_position(me)[0], world_position(unit)[1] - world_position(me)[1],
                            world_position(unit)[2] - world_position(me)[2]};
    const float d = static_cast<float>(host.vector_length_0042b2f0(delta));
    void* const vehicle = host.game_controlled_vehicle_004b4b00();
    if (vehicle == nullptr) return false;
    const Raw v{static_cast<std::uint8_t*>(vehicle)};
    if (host.unit_has_kind_5c(vehicle, 0x0F) || host.unit_has_kind_5c(unit, 0x0F) || host.unit_has_kind_5c(unit, 0x18) ||
        host.unit_is_local_player_role_00927f30(vehicle, 3) || host.unit_is_local_player_role_00927f30(vehicle, 2)) {
        const float d2 = d * d;
        return v.at<float>(0x49C) > d2;
    }
    if (host.unit_is_local_player_role_00927f30(vehicle, 5)) return v.at<float>(0x44C) > d;
    return v.at<float>(0x490) > d;
}

void markers_reset_marker_pools_00640620(Raw s, Fe10GuiHost& host) {
    auto count = [&](std::size_t off) -> std::uint32_t {
        auto* const first = static_cast<std::uint8_t*>(s.ptr(off));
        return first ? static_cast<std::uint32_t>((static_cast<std::uint8_t*>(s.ptr(off + 4)) - first) / 4) : 0;
    };
    auto at = [&](std::size_t off, std::uint32_t i) { return static_cast<void**>(s.ptr(off))[i]; };
    for (std::uint32_t i = s.at<std::uint32_t>(0x7C); i < count(0x50); ++i) {
        hud_marker_entry_hide_all_006374b0(Raw{static_cast<std::uint8_t*>(at(0x50, i))}, host);
    }
    for (std::uint32_t i = s.at<std::uint32_t>(0x80); i < count(0x60); ++i) {
        const Raw entry{static_cast<std::uint8_t*>(at(0x60, i))};
        for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x14u, 0x10u}) host.widget_set_visible_34(entry.ptr(off), false);
    }
    for (std::uint32_t i = s.at<std::uint32_t>(0x84); i < count(0x70); ++i) {
        host.widget_set_visible_34(Raw{static_cast<std::uint8_t*>(at(0x70, i))}.ptr(4), false);
    }
    const Raw owned{s.p + 0x90};
    for (std::uint32_t i = s.at<std::uint32_t>(0xA0); i < count(0x94); ++i) {
        void*& slot = static_cast<void**>(owned.ptr(4))[i];
        if (slot != nullptr) {
            host.object_delete_04(slot);  // vtable +0(1): scalar deleting destructor
            slot = nullptr;
        }
    }
    host.pointer_vector_resize_0063ffc0(owned, s.at<std::uint32_t>(0xA0), nullptr);
    s.at<std::uint32_t>(0x7C) = 0;
    s.at<std::uint32_t>(0x80) = 0;
    s.at<std::uint32_t>(0x84) = 0;
}

Raw int_list_int_record_assign_00668540(Raw dst, Raw src, Fe10GuiHost& host) {
    dst.at<std::int32_t>(0) = src.at<std::int32_t>(0);
    host.int_list_assign_005aaaf0(Raw{dst.p + 4}, Raw{src.p + 4});
    dst.at<std::int32_t>(0x10) = src.at<std::int32_t>(0x10);
    return dst;
}

Raw int_list_int_record_assign_006697a0(Raw dst, Raw src, Fe10GuiHost& host) {
    dst.at<std::int32_t>(0) = src.at<std::int32_t>(0);
    if (dst.p + 4 != src.p + 4) host.int_list_assign_range_00665d20(Raw{dst.p + 4}, Raw{src.p + 4});
    dst.at<std::int32_t>(0x10) = src.at<std::int32_t>(0x10);
    return dst;
}

void wide_string_assign_z_0067d6b0(Raw dst, const std::uint16_t* text, Fe10GuiHost& host) {
    std::uint32_t n = 0;
    while (text[n] != 0) ++n;
    host.wide_string_assign_n_0067d5a0(dst, text, n);
}

void hud_colour_icon_set_show_for_kind_00639230(Raw set, float x, float y, float angle, Raw marker, void** unit_slot,
                                                bool offscreen, void* selected, Fe10GuiHost& host) {
    angle = static_cast<float>(angle - kHeadingQuarterTurn);  // 00CE3830
    host.marker_clamp_to_margins_00638b90(x, y);
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x14u, 0x10u}) host.widget_set_visible_34(set.ptr(off), false);
    void* icon = nullptr;
    switch (marker.at<std::int32_t>(0x18)) {
    case 0: icon = set.ptr(0x4); break;
    case 1: icon = set.ptr(0x8); break;
    case 3:
    case 4:
    case 5: icon = set.ptr(0xC); break;
    default: break;
    }
    if (void* const unit = *unit_slot) {
        const Raw game = host.global_object(0x00E188A8);
        const auto* const local = static_cast<const std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4));
        const std::int32_t team = Raw{static_cast<std::uint8_t*>(unit)}.at<std::int32_t>(0x54);
        if (team == *reinterpret_cast<const std::int32_t*>(local + 0x28)) icon = set.ptr(team == 0 ? 0x10 : 0x14);
    }
    host.widget_set_visible_34(icon, true);
    host.widget_set_rotation_44(icon, angle);
    float alpha = 1.0f;
    if (!offscreen) {
        host.widget_set_size_58(icon, (*unit_slot != nullptr && *unit_slot == selected) ? kColourIconSelectedSize
                                                                                         : kColourIconSize);
    } else {
        host.widget_set_size_58(icon, kColourIconOffscreenSize);
        alpha = static_cast<float>(host.global_block(kMarkerPulseGlobal).at<float>(0) + 0.5);  // 00D7A280
    }
    const float colour[4] = {1.0f, 1.0f, 1.0f, alpha};
    host.widget_set_colour_50(icon, colour);
    host.widget_set_local_xy_00aa7d00(icon, x, y);
    host.widget_virtual_88(icon, offscreen ? 2 : 1, 0, 1.0f);
}

void support_manager_layout_widget_1c0_006567d0(Raw s, Fe10GuiHost& host) {
    void* const widget = s.ptr(0x1C0);
    const Raw frame{static_cast<std::uint8_t*>(s.ptr(0x1C8))};
    host.widget_set_visible_34(widget, true);
    const float* const size = host.widget_vector_00aa6740(widget);
    const float sw = size[0], sh = size[1];
    const float* const pos = host.widget_vector_00aa67f0(widget);
    const Raw w{static_cast<std::uint8_t*>(widget)};
    const float left = pos[0] - static_cast<float>(w.at<float>(0x18) * sw);
    const float top = pos[1] - static_cast<float>(w.at<float>(0x1C) * sh);
    const double k = kSupportFrameBorderScale;
    const float frame_pos[3] = {static_cast<float>(left - frame.at<float>(0x10C) * k),
                                static_cast<float>(top - k * frame.at<float>(0x114)),
                                static_cast<float>(pos[2] - 1.0)};
    host.widget_set_local_position_00aa7dc0(frame.p, frame_pos);
    const float h = host.widget_vector_00aa6740(widget)[1];
    const float* const size2 = host.widget_vector_00aa6740(widget);
    const float frame_size[2] = {
        static_cast<float>((frame.at<float>(0x10C) * k + size2[0]) + frame.at<float>(0x110) * k),
        static_cast<float>((frame.at<float>(0x114) * k + h) + k * frame.at<float>(0x118))};
    host.widget_set_size_58(frame.p, frame_size);
    const float marker[3] = {static_cast<float>(s.at<float>(0x1B4) - 0.03500000014901161),
                             static_cast<float>(s.at<float>(0x1B8) - 0.02500000037252903),
                             static_cast<float>(s.at<float>(0x1BC) + 0.0)};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x1B0), marker);
}

void support_manager_layout_name_label_0065bbc0(Raw s, Fe10GuiHost& host) {
    void* const text = s.ptr(0x170);
    const Raw t{static_cast<std::uint8_t*>(text)};
    if (static_cast<float>(t.at<float>(0x114) / kGuiScreenWidth) == 0.0f) {
        const auto* const wide = static_cast<const std::uint16_t*>(t.ptr(0xF0));
        alignas(4) std::uint8_t name[8] = {};
        host.native_string_from_wide_00436630(
            Raw{name}, wide ? wide : reinterpret_cast<const std::uint16_t*>(host.global_block(0x00E19834).p));
        host.text_set_localised_cstr_00abbe50(text, "");
        host.text_set_localised_source_native_00abaed0(text, Raw{name}, true);
        host.native_string_release(Raw{name});
    }
    const float width = static_cast<float>(t.at<float>(0x114) / kGuiScreenWidth);
    if (!(width < s.at<float>(0x74))) {
        const float box[2] = {static_cast<float>(width + 0.20999999344348907), s.at<float>(0x198)};  // 00CF6558
        host.widget_set_size_58(s.ptr(0x18C), box);
        s.at<float>(0x74) = static_cast<float>(t.at<float>(0x114) / kGuiScreenWidth);
    }
    const float box_width = host.widget_vector_00aa6740(s.ptr(0x18C))[0];
    if (host.global_object(0x00E188A8).at<std::uint8_t>(0x620) != 0) {
        host.widget_set_visible_34(text, true);
        return;
    }
    const float w = static_cast<float>(t.at<float>(0x114) / kGuiScreenWidth);
    const float offset = static_cast<float>(
        ((static_cast<double>(box_width) - s.at<float>(0x194) - w) + 0.018) * 0.5 + 0.0035000001080334187);  // 00CF6550/48
    const float label[3] = {s.at<float>(0x19C) + offset, s.at<float>(0x1A0) + 0.0f, 0.0f + s.at<float>(0x1A4)};
    host.widget_set_resolved_position_00aa8240(text, label);
    host.widget_set_visible_34(text, true);
}

void support_manager_sync_cursor_iterators_0065a2b0(Raw s, Fe10GuiHost& host) {
    auto walk = [&](std::size_t list, std::size_t size, std::size_t iter) -> bool {
        const std::uint32_t n = s.at<std::uint32_t>(size);
        if (n == 0) return false;
        s.at<std::uint32_t>(0x308) = s.at<std::uint32_t>(0x308) % n;
        s.ptr(iter) = s.p + list;
        s.ptr(iter + 4) = Raw{static_cast<std::uint8_t*>(s.ptr(list + 4))}.ptr(0);  // head->next
        return true;
    };
    auto advance = [&](std::size_t iter) {
        for (std::int32_t i = 0; i < s.at<std::int32_t>(0x308); ++i) {
            s.ptr(iter + 4) = Raw{static_cast<std::uint8_t*>(s.ptr(iter + 4))}.ptr(0);
        }
    };
    if (walk(0x1EC, 0x1F4, 0x300)) advance(0x300);
    if (!walk(0x344, 0x34C, 0x350)) return;
    auto* const head = static_cast<std::uint8_t*>(s.ptr(0x348));
    for (auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0)); node != head;
         node = static_cast<std::uint8_t*>(Raw{node}.ptr(0))) {
        host.widget_set_visible_34(Raw{node}.ptr(0x18), true);
    }
    advance(0x350);
    void* const widget = Raw{static_cast<std::uint8_t*>(s.ptr(0x354))}.ptr(0x18);
    host.widget_virtual_88(widget, 0, 0, 1.0f);
    float pos[3];
    host.widget_resolved_position_00aa6750(widget, pos);
    const float placed[3] = {pos[0], pos[1], s.at<float>(0x2D0)};
    host.widget_set_resolved_position_00aa8240(Raw{static_cast<std::uint8_t*>(s.ptr(0x354))}.ptr(0x18), placed);
}

void support_manager_layout_scroll_rows_006619e0(Raw s, Fe10GuiHost& host) {
    const std::int32_t n = s.at<std::int32_t>(0x2D8);
    float y = static_cast<float>((static_cast<double>(n + 1) + s.at<float>(0x2E0)) * s.at<float>(0x2A8));
    for (std::int32_t i = n; i < 4; ++i) {
        host.widget_set_rotation_44(s.ptr(0xAC + i * 4), s.at<float>(0x2A4) + y);
        host.widget_set_rotation_44(s.ptr(0xBC + i * 4), -s.at<float>(0x2A4) - y);
        y = y + s.at<float>(0x2A8);
    }
    host.support_set_large_icon_alpha_0065e870(s, s.ptr(0xB8), 1.0f - s.at<float>(0x2E0));
    void* const icon = s.ptr(0xDC + s.at<std::int32_t>(0x324) * 4);
    float colour[4];
    host.widget_get_colour_54(icon, colour);
    if (colour[3] != 1.0f) host.widget_set_float_4c(s.ptr(0xDC + s.at<std::int32_t>(0x324) * 4), 1.0f - s.at<float>(0x2E0));
    if (s.at<float>(0x2E0) != 0.0f) return;
    s.at<std::uint8_t>(0x2C1) = 0;
    s.at<std::uint8_t>(0x2DC) = 0;
    s.at<std::uint8_t>(0x2DD) = 0;
    host.widget_set_visible_34(s.ptr(0xFC), true);
    host.widget_set_float_4c(s.ptr(0xDC + s.at<std::int32_t>(0x324) * 4), 1.0f);
}

std::int32_t support_manager_selected_stock_count_0065ac30(Raw s, Fe10GuiHost& host) {
    auto unit = [&]() -> void* { return Raw{static_cast<std::uint8_t*>(s.ptr(0x304))}.ptr(8); };
    if (host.unit_has_kind_5c(unit(), 0x45) || host.unit_has_kind_5c(unit(), 9)) {
        std::uint8_t* block = nullptr;
        if (host.unit_has_kind_5c(unit(), 0x45)) {
            if (unit()) block = static_cast<std::uint8_t*>(unit()) + 0x72C;
        } else if (unit()) {
            block = static_cast<std::uint8_t*>(unit()) + 0x1188;
        }
        auto* const head = static_cast<std::uint8_t*>(Raw{block}.ptr(0x44));
        for (auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0)); node != head;
             node = static_cast<std::uint8_t*>(Raw{node}.ptr(0))) {
            if (Raw{node}.at<std::int32_t>(8) == s.at<std::int32_t>(0x2A0)) return Raw{node}.at<std::int32_t>(0xC);
        }
        return -1;
    }
    if (!host.unit_has_kind_5c(unit(), 0x46)) return -1;
    const Raw records{static_cast<std::uint8_t*>(unit()) + 0x770};
    auto* const first = static_cast<std::uint8_t*>(records.ptr(4));
    if (first == nullptr) return -1;
    const std::uint32_t count = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(records.ptr(8)) - first) / 0x20);
    for (std::uint32_t i = 0; i < count; ++i) {
        const Raw record{first + i * 0x20};
        if (record.at<std::int32_t>(4) == s.at<std::int32_t>(0x3A4)) return record.at<std::int32_t>(8);
    }
    return -1;
}

void support_manager_exit_00662470(Raw s, Fe10GuiHost& host) {
    s.at<float>(0x74) = 0.0f;
    host.widget_set_visible_34(host.interface_object(0xC4).ptr(0x64), false);
    host.hints_dismiss_0054e610();
    if (s.at<std::uint8_t>(0x2C1) != 0) {
        if (s.ptr(0x148) != nullptr) host.object_delete_04(s.ptr(0x148));
        s.at<std::uint8_t>(0x2C1) = 0;
        host.widget_set_visible_34(s.ptr(0xF8), true);
        host.widget_set_visible_34(s.ptr(0xFC), true);
    }
    if (s.at<std::uint8_t>(0x2C0) != 0) {
        if (s.ptr(0x13C) != nullptr) host.object_delete_04(s.ptr(0x13C));
        s.at<std::uint8_t>(0x2C0) = 0;
        host.widget_set_visible_34(s.ptr(0x100), true);
        host.widget_set_visible_34(s.ptr(0x104), true);
    }
    if (s.at<std::uint8_t>(0x2DC) != 0) {
        s.at<float>(0x2E0) = 0.0f;
        support_manager_layout_scroll_rows_006619e0(s, host);
    }
    s.at<float>(0x2E0) = 0.0f;
    support_manager_layout_scroll_rows_006619e0(s, host);
    host.widget_set_visible_34(s.ptr(0xDC + s.at<std::int32_t>(0x324) * 4), false);
    auto* const head = static_cast<std::uint8_t*>(s.ptr(0x2E8));
    auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0));
    Raw{head}.ptr(0) = head;
    Raw{head}.ptr(4) = head;
    s.at<std::uint32_t>(0x2EC) = 0;
    while (node != head) {
        auto* const next = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
        host.free_00bf65ac(node);
        node = next;
    }
    host.support_clear_loadout_icons_0065abb0(s);
    host.widget_set_size_58(s.ptr(0x18C), reinterpret_cast<const float*>(s.p + 0x194));
    host.widget_set_visible_34(s.ptr(0x190), true);
    host.widget_set_visible_34(s.ptr(0x170), false);
    for (std::size_t off = 0xCC; off < 0xDC; off += 4) host.widget_set_visible_34(s.ptr(off), false);
    host.widget_set_visible_34(s.ptr(0x1B0), false);
    host.widget_set_visible_34(s.ptr(0x78), false);
    if (host.global_object(0x00E188D8).p != nullptr) {
        const Raw game = host.global_object(0x00E188A8);
        const auto* const local = static_cast<const std::uint8_t*>(game.ptr(0x18CC + game.at<std::int32_t>(0x18EC) * 4));
        if (local[0x19] == 0 && game.at<std::uint8_t>(0x19C4) == 0) host.hud_panel_set_visible3_005bca30(1);
    }
    host.interface_collapse_overlays_0068ab80(0, 0);
    host.support_play_close_sound_0065faa0(s);
    host.counters_restore_hidden_a5_00529060();
    host.counters_relayout_00529a30();
}

void support_manager_step_scroll_collapse_00662690(Raw s, float dt, Fe10GuiHost& host) {
    const float phase = s.at<float>(0x2E0);
    if (phase == 0.0f) return;
    const float next = phase - dt;
    s.at<float>(0x2E0) = next;
    if (0.0f > next) s.at<float>(0x2E0) = 0.0f;
    support_manager_layout_scroll_rows_006619e0(s, host);
}

bool game_accept_pending_invite_0067d6e0(Fe10GuiHost& host) {
    void*& pending = host.global_block(kPendingInviteGlobal).ptr(0);
    if (pending == nullptr) return false;
    const std::int32_t slot = host.xenon_find_slot_for_invitee_00a3e470();
    const Raw input = host.global_object(0x00F8BBF4);
    auto* const first = static_cast<void**>(input.ptr(0xB4 + 4));
    const bool has_device = first != nullptr && static_cast<void**>(input.ptr(0xB4 + 8)) != first && first[0] != nullptr;
    if (!has_device || host.input_device_slot_34(first[0]) != slot) {
        if (has_device) host.rebind_primary_input_0067c970();
        host.input_activate_device_slot_00a91620(2, slot);
    }
    alignas(4) std::uint8_t storage[0x98] = {};
    const Raw record{storage};
    host.session_record_construct_00571e50(record);
    record.at<std::uint8_t>(4) = 0xFF;
    // The image writes the fields without checking the allocation.
    const Raw info{static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x88))};
    if (info.p) info.at<std::uint32_t>(0) = kSessionInfoVtable;
    const Raw invite{static_cast<std::uint8_t*>(pending)};
    std::memcpy(info.p + 0x0C, invite.p + 0x40, 0x10);
    info.at<std::uint32_t>(4) = invite.at<std::uint32_t>(0x14);
    info.at<std::uint32_t>(8) = invite.at<std::uint32_t>(0x18);
    std::memcpy(info.p + 0x1C, invite.p + 0x1C, 9 * 4);
    record.ptr(0) = info.p;
    host.wide_string_assign_n_0067d5a0(Raw{record.p + 8}, kSessionRecordName, 7);
    host.online_service_join_6c(record);
    pending = nullptr;
    if (record.at<std::uint32_t>(0x20) >= 8) host.free_00bf65ac(record.ptr(0xC));
    return true;
}

void unit_library_on_listbox_activate_00680520(Raw listener, void* /*widget*/, void* listbox, Fe10GuiHost& host) {
    const Raw screen{listener.p - 0x64};
    const std::int32_t row = host.listbox_selected_index_00a9c920(listbox);
    if (listener.at<std::int32_t>(0x84) != 1) {
        if (row == 0 || row == 1) {
            host.unit_library_toggle_description_0067e9a0(screen);
        } else if (row == 2) {
            host.main_front_end_push_interface_004cc460(1, 0);
        }
        return;
    }
    if (row == 0) {
        void* const panel = listener.ptr(0x18);
        host.widget_set_visible_34(panel, !host.widget_is_visible_38(panel));
    } else if (row == 1) {
        host.unit_library_reset_selection_0067e580(screen);
    }
}

void unit_library_toggle_description_0067e9a0(Raw s, Fe10GuiHost& host) {
    s.at<std::int32_t>(0xE8) = 1;
    auto* const head = static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(s.ptr(0x6C))}.ptr(0x68));
    for (auto* node = static_cast<std::uint8_t*>(Raw{head}.ptr(0)); node != head;
         node = static_cast<std::uint8_t*>(Raw{node}.ptr(0))) {
        host.widget_set_visible_34(Raw{node}.ptr(8), false);
    }
    host.widget_set_visible_34(s.ptr(0x78), true);
    host.widget_set_visible_34(s.ptr(0x7C), true);
    host.widget_set_visible_34(s.ptr(0x70), false);
    void* const listbox = s.ptr(0x74);
    host.widget_set_focus_60(listbox, true);
    host.listbox_select_index_00a9c7c0(listbox, 0);
    host.widget_virtual_80(listbox, 1);
    host.listbox_set_listener_00a9ac40(listbox, s.p + 0x64);
    host.movie_player_stop_004f8ac0();
    host.gui_movie_stop_00aac870(s.ptr(0x94));
    if (void* const panel = host.global_block(0x00E198BC).ptr(0)) static_cast<std::uint8_t*>(panel)[0x48] = 0;
    host.widget_set_visible_34(s.ptr(0x7C), !host.widget_is_visible_38(s.ptr(0x7C)));
    const MainMenuCommandArgument commands[5] = {{0xA2, "FE.unitlib_toggledesc", 1}, {0x00, "", 1}, {0x00, "", 1},
                                                 {0xA3, "globals.back", 2}, {0x00, "", 1}};
    host.command_bar_rebuild_0054b530(commands);
    host.command_bar_set_help_line_0054a0c0("");
}

void torpedo_boat_view_bind_unit_camera_0067c1a0(Raw s, void* unit, Fe10GuiHost& host) {
    host.weapon_group_release_005470c0();
    if (s.ptr(0x24) != nullptr) host.unit_send_role_transfer_0077c470(s.ptr(0x24), 0x1FE, 0);
    if (s.ptr(0x24) != nullptr && host.observer_is_registered_00694af0(s.p + 8, s.ptr(0x24))) {
        host.observer_unregister_006952a0(s.p + 8, s.ptr(0x24));
    }
    s.ptr(0x24) = unit;
    if (s.ptr(0x2C) != nullptr) {
        host.binoculars_reset_0051e8e0();
    }
    if (s.ptr(0x2C) == nullptr) {
        void* const block = host.operator_new_00bf55be(0x404);
        std::memset(block, 0, 0x404);
        void* const camera = block ? host.ship_captain_camera_construct_0064b650(block) : nullptr;
        s.ptr(0x2C) = camera;
        const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        host.camera_attach_00923870(camera, 0, host.global_object(0x00E188A8).ptr(0x19CC), identity);
        Raw{static_cast<std::uint8_t*>(s.ptr(0x2C))}.at<float>(0x3EC) = -kTorpedoBoatPitchLimit;  // 00CEC574
        Raw{static_cast<std::uint8_t*>(s.ptr(0x2C))}.at<float>(0x3F0) = kTorpedoBoatPitchLimit;
        host.observer_register_00694a60(s.p + 8, s.ptr(0x2C));
    }
    const Raw camera{static_cast<std::uint8_t*>(s.ptr(0x2C))};
    const bool retarget = camera.ptr(0x3AC) != s.ptr(0x24);
    host.camera_mover_set_target_00432e60(camera.p, s.ptr(0x24), 0);
    if (retarget) {
        const Raw u{static_cast<std::uint8_t*>(s.ptr(0x24))};
        const float offset = static_cast<float>(Raw{static_cast<std::uint8_t*>(u.ptr(0x538))}.at<float>(0xA0) * 0.75);  // 00CEC9D8
        if (camera.at<std::int32_t>(0x3E8) == 0) {
            camera.at<float>(0x384) = 0.0f;
            camera.at<float>(0x388) = 0.0f;
            camera.at<float>(0x38C) = offset;
            camera.at<float>(0x398) = offset;
            camera.at<float>(0x394) = offset;
            camera.at<float>(0x390) = offset;
        }
        camera.at<float>(0xA8) = static_cast<float>(u.at<float>(0xA8) + 5.0);  // 00D7A370
    }
    host.game_set_camera_mover_004bc410(s.ptr(0x2C));
    host.binoculars_bind_unit_0051e730(s.ptr(0x2C));
    host.weapon_group_bind_unit_00549260(s.ptr(0x24), s.ptr(0x2C));
    if (s.ptr(0x24) != nullptr) {
        if (!host.observer_is_registered_00694af0(s.p + 8, s.ptr(0x24))) host.observer_register_00694a60(s.p + 8, s.ptr(0x24));
        s.at<float>(0x1C) = Raw{static_cast<std::uint8_t*>(s.ptr(0x24))}.at<float>(0x980);
    }
}

void hud_root_advance_award_ticker_00648060(Raw s, Fe10GuiHost& host) {
    const Raw queue{s.p + 0xAC};
    const Raw front{static_cast<std::uint8_t*>(queue.ptr(4))};
    host.string_replace_substrings_004cad40(Raw{front.p + 8}, "fe/", "gui/", 0x7FFFFFFF);
    const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    host.gui_set_state_texture_00ab44b0(s.ptr(0x5C), 0, Raw{front.p + 8}, uv);
    host.text_set_localised_source_native_00abaed0(s.ptr(0x60), front, true);
    host.string_pair_vector_erase_front_00647ca0(queue);
    for (std::size_t off : {0x5Cu, 0x60u}) {
        host.widget_set_float_4c(s.ptr(off), 1.0f);
        const Raw entry = host.widget_timed_entry_00aa8b00(s.ptr(off), 0);
        host.ticker_set_range_00ac2f20(entry, 0.0f, 1.0f);
        entry.at<float>(4) = kAwardTickerSeconds;
    }
}

void hud_root_queue_award_ticker_00648ab0(Raw s, Raw key, Fe10GuiHost& host) {
    alignas(4) std::uint8_t pair[0x10] = {};
    alignas(4) std::uint8_t text[8] = {}, icon[8] = {};
    host.award_table_text_006b9fd0(Raw{text}, key);
    host.native_string_resize_copy_0041dd40(Raw{pair}, Raw{text});
    host.native_string_release(Raw{text});
    host.award_table_icon_006b9f70(Raw{icon}, key);
    host.native_string_resize_copy_0041dd40(Raw{pair + 8}, Raw{icon});
    host.native_string_release(Raw{icon});
    host.string_pair_vector_push_back_00648a20(Raw{s.p + 0xAC}, Raw{pair});
    host.string_pair_destroy_00645fc0(Raw{pair});
    host.native_string_release(key);
}

void markers_project_part_bounds_006395e0(void* entity, float* min_x, float* min_y, float* max_x, float* max_y,
                                          Fe10GuiHost& host) {
    float box[6];
    host.transform_local_bounds_4c(host.entity_transform_18(entity), box);
    float corners[8][3];
    for (int i = 0; i < 8; ++i) {
        corners[i][0] = box[(i & 4) ? 3 : 0];
        corners[i][1] = box[(i & 2) ? 4 : 1];
        corners[i][2] = box[(i & 1) ? 5 : 2];
    }
    *min_x = *min_y = 3.4028234663852886e38f;   // 00D7A248
    *max_x = *max_y = -3.4028234663852886e38f;  // 00D7A244
    for (int i = 0; i < 7; ++i) {  // seven, as the image counts
        auto* const t = static_cast<std::uint8_t*>(host.entity_transform_18(entity));
        if ((t[0x5C] & 2) == 0) host.transform_refresh_world_00b6db70(t);
        float world[3];
        const float* const p = host.vector_transform_affine_004142e0(corners[i], world, t + 0xF0);
        float screen[3];
        host.marker_project_and_clip_00638e50(p, screen);
        *max_x = *max_x > screen[0] ? *max_x : screen[0];
        *max_y = *max_y > screen[1] ? *max_y : screen[1];
        *min_x = screen[0] > *min_x ? *min_x : screen[0];
        *min_y = screen[1] > *min_y ? *min_y : screen[1];
    }
}

// Packet cc13-trivialslots: the kTrivialSlots bodies; evidence and ABI per routine
// are in the header.
std::int32_t rewards_screen_get_screen_id_00626710(Raw) noexcept { return 4; }
std::int32_t hud_scene_init_screen_get_screen_id_00636dc0(Raw) noexcept { return 0x5A; }
bool hud_scene_init_screen_occludes_lower_levels_00636dd0(Raw) noexcept { return true; }
bool hud_scene_init_screen_is_self_managed_00636de0(Raw) noexcept { return true; }
void hud_markers_screen_listener04no_op_006375f0(Raw) noexcept {}
std::int32_t hud_markers_screen_get_screen_id_006405e0(Raw) noexcept { return 0x4D; }
std::int32_t hud_spectator_screen_get_screen_id_00643dc0(Raw) noexcept { return 0x43; }
bool hud_spectator_screen_is_self_managed_00643dd0(Raw) noexcept { return true; }
void hud_spectator_screen_exit_00643de0(Raw) noexcept {}
void hud_spectator_screen_update_00643df0(Raw) noexcept {}
std::int32_t in_game_hud_root_screen_get_screen_id_00647fe0(Raw) noexcept { return 0x44; }
std::int32_t hud_ship_status_screen_get_screen_id_0064b860(Raw) noexcept { return 0x45; }
std::int32_t hud_submarine_screen_get_screen_id_00650160(Raw) noexcept { return 0x48; }
void hud_submarine_screen_enter_00650190(Raw) noexcept {}
std::int32_t hud_periscope_screen_get_screen_id_00650ac0(Raw) noexcept { return 0x47; }
std::int32_t hud_support_manager_screen_get_screen_id_006730f0(Raw) noexcept { return 0x4E; }
void hud_tactical_sheet_screen_on_listbox_selection_changed_no_op_0067af30(Raw) noexcept {}
std::int32_t hud_tactical_sheet_screen_get_screen_id_0067af60(Raw) noexcept { return 0x54; }
std::int32_t hud_cursor_screen_get_screen_id_0067b320(Raw) noexcept { return 0x28; }
void hud_cursor_screen_exit_0067b330(Raw) noexcept {}
void hud_common_overlay_screen_register_0067b3f0(Raw self, HudScreenServices& host) {
    host.front_end_screen_register_004f71d0(self);
}
void hud_common_overlay_screen_enter_0067b400(Raw) noexcept {}
void hud_common_overlay_screen_exit_0067b410(Raw) noexcept {}
std::int32_t hud_follow_screen49_get_screen_id_0067be50(Raw) noexcept { return 0x49; }
void hud_follow_screen49_collect_children_0067be60(Raw) noexcept {}
void hud_follow_screen49_bind_layout_0067bea0(Raw) noexcept {}
void hud_follow_screen49_enter_0067beb0(Raw) noexcept {}
std::int32_t press_start_screen_get_screen_id_0067c860(Raw) noexcept { return 0x5C; }
void unit_library_screen_on_listbox_selection_changed_no_op_0067d9f0(Raw) noexcept {}
void unit_library_widget_listener_current00no_op_0067da00(Raw) noexcept {}
std::int32_t unit_library_screen_get_screen_id_0067f050(Raw) noexcept { return 5; }

} // namespace bsp::ingame_gui_cc13
