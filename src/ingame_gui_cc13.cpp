// In-game GUI screens of segments 26 and 27 (packet cc13-fe2). Evidence and
// ABI per routine are in include/bsp/ingame_gui_cc13.hpp and docs/CC13_FE_2.md.

#include "bsp/ingame_gui_cc13.hpp"

#include "bsp/band_0060_cc13.hpp"

#include <cmath>
#include <cstring>

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

} // namespace bsp::ingame_gui_cc13
