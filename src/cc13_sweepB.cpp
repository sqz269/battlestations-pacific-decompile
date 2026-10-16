// cc13_sweepB: leftover sweep over segments 15-51 (packet cc13-sweepB,
// docs/CC13_SWEEPB.md). Names are hypotheses; see the header for the hooks.
//
// x87 note: float expressions are written in the image's operation order with
// a float store wherever the image stores to a float slot. At the main
// thread's 24-bit precision control this is exact; at 53 or 64 bits the image
// keeps unstored intermediates wider.

#include "bsp/cc13_sweepB.hpp"

#include <initializer_list>

namespace bsp {

// ===========================================================================
// Segment 15
// ===========================================================================

// 005411A0 BSP_WidgetHolder18_Show_005411A0 (hypothesis).
// ABI: __thiscall(ECX holder), RET. Evidence: 005411A0..005411AC: widget at
// holder+18h, its vtable +34h called with push 1. No callers (rel32/absolute)
// and no vtable slot, per docs/CC13_FE_10.md (cc13-fe10b).
void cc13_sweepB_holder18_show_005411a0(void* holder, Cc13SweepBHost& host) {
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(holder, 0x18), 1);
}

// 005411B0 BSP_WidgetHolder08_Show_005411B0 (hypothesis).
// ABI: __thiscall(ECX holder), RET. As 005411A0 with the widget at +8h.
void cc13_sweepB_holder08_show_005411b0(void* holder, Cc13SweepBHost& host) {
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(holder, 0x8), 1);
}

namespace {

// 005411C0 / 00541210 body: FILD x * 1/1024 (double 00CEDAE8) and FILD y /
// 768.0 (double 00CE42B0), each rounded to float, then 00AA7D00(x, y).
void cc13_sweepB_holder_set_pixel_xy(void* widget, std::int32_t x, std::int32_t y,
                                     Cc13SweepBHost& host) {
    const float fy = static_cast<float>(static_cast<double>(y) / 768.0);
    const float fx = static_cast<float>(static_cast<double>(x) * 0.0009765625);
    host.widget_set_local_xy_00aa7d00(widget, fx, fy);
}

}  // namespace

// 005411C0 BSP_WidgetHolder18_SetPixelXY_005411C0 (hypothesis).
// ABI: __thiscall(ECX holder)(int x, int y), RET 8. Evidence 005411C0..005411F6;
// the y quotient is computed and stored first, then x (no observable effect).
// The multiply by 2^-10 is exact; the divide by 768 rounds once in the x87
// precision and again to float (one rounding here at double).
void cc13_sweepB_holder18_set_pixel_xy_005411c0(void* holder, std::int32_t x, std::int32_t y,
                                                Cc13SweepBHost& host) {
    cc13_sweepB_holder_set_pixel_xy(cc13_sweepB_at<void*>(holder, 0x18), x, y, host);
}

// 00541200 BSP_Holder_SetField04_00541200 (hypothesis).
// ABI: __thiscall(ECX holder)(value), RET 4. Evidence 00541200..00541207.
void cc13_sweepB_holder_set_field04_00541200(void* holder, std::uint32_t value) {
    cc13_sweepB_at<std::uint32_t>(holder, 0x4) = value;
}

// 00541210 BSP_WidgetHolder08_SetPixelXY_00541210 (hypothesis).
// ABI: __thiscall(ECX holder)(int x, int y), RET 8. As 005411C0 with the widget
// at +8h (evidence 00541210..00541246).
void cc13_sweepB_holder08_set_pixel_xy_00541210(void* holder, std::int32_t x, std::int32_t y,
                                                Cc13SweepBHost& host) {
    cc13_sweepB_holder_set_pixel_xy(cc13_sweepB_at<void*>(holder, 0x8), x, y, host);
}

// 00535F50 formation screen drag press (hypothesis; ledger FUN_00535F50).
// ABI: __thiscall(ECX screen), RET, returns AL. Caller 0053A8E0
// BSP_HudFormationScreen_Update (action D8h pressed).
// Evidence: listing 00535F50..005360F4 (disk bytes).
// - group = screen+138h; its leader entity +14h gets 00414DB0 when its world
//   valid byte +C8h is clear; the member count +4F8h is read after that.
// - The cursor (manager 004C12B0() +5Ch/+60h) is copied once before the loop.
// - For each member record (0070D070) whose unit is non-null and not the
//   leader: sx = -rec[+10h] / screen[+30h] + 0.5, sy = rec[+20h] / screen[+34h]
//   + 0.5 (the image negates +20h, stores, and negates again), each stored as
//   float; d = dx*dx + dy*dy (float); distance = sqrt(d) when d > 1e-10
//   (double 00CE3820), else 0.
// - When the running best (seed 0.03f, 00CE3DB8) is strictly greater than the
//   distance: index = 0070D030(group, unit) into screen+8Ch, listbox
//   screen+ACh selects it, best = distance, screen+7Ch = 1, result = 1, and
//   the cursor moves to (sx, sy) through a fresh 004C12B0() call.
// The record pointer re-reads screen+138h for each call as the image does.
bool cc13_sweepB_formation_drag_pick_00535f50(void* screen, Cc13SweepBHost& host) {
    float best = 0.03f;
    bool picked = false;
    void* leader = cc13_sweepB_at<void*>(cc13_sweepB_at<void*>(screen, 0x138), 0x14);
    if (cc13_sweepB_at<std::uint8_t>(leader, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(leader);
    }
    const std::int32_t count = cc13_sweepB_at<std::int32_t>(cc13_sweepB_at<void*>(screen, 0x138), 0x4F8);
    void* manager = host.gui_manager_get_004c12b0();
    const float cursor_x = cc13_sweepB_at<float>(manager, 0x5C);
    const float cursor_y = cc13_sweepB_at<float>(manager, 0x60);
    for (std::int32_t i = 0; i < count; ++i) {
        void* record = host.unit_group_member_record_at_0070d070(cc13_sweepB_at<void*>(screen, 0x138), i);
        void* unit = cc13_sweepB_at<void*>(record, 0x0);
        if (unit == nullptr) continue;
        void* group = cc13_sweepB_at<void*>(screen, 0x138);
        if (unit == cc13_sweepB_at<void*>(group, 0x14)) continue;
        const float neg_x = -cc13_sweepB_at<float>(record, 0x10);
        const float neg_z = -cc13_sweepB_at<float>(record, 0x20);
        const float sx = static_cast<float>(neg_x / cc13_sweepB_at<float>(screen, 0x30) + 0.5);
        const float sy = static_cast<float>(-neg_z / cc13_sweepB_at<float>(screen, 0x34) + 0.5);
        const float dx = sx - cursor_x;
        const float dy = sy - cursor_y;
        const float d = dx * dx + dy * dy;
        float distance;
        if (static_cast<double>(d) > 1e-10) {
            distance = static_cast<float>(host.crt_sqrt_00bf7030(d));
        } else {
            distance = 0.0f;
        }
        if (best > distance) {
            const std::int32_t index = host.unit_group_index_of_member_0070d030(group, unit);
            cc13_sweepB_at<std::int32_t>(screen, 0x8C) = index;
            host.listbox_select_index_00a9c7c0(cc13_sweepB_at<void*>(screen, 0xAC), index);
            best = distance;
            cc13_sweepB_at<std::uint8_t>(screen, 0x7C) = 1;
            picked = true;
            host.gui_manager_set_cursor_00aa1fa0(host.gui_manager_get_004c12b0(), sx, sy);
        }
    }
    return picked;
}

// 0052C640 BSP_HudCounterEntry_ConstructClone (hypothesis).
// ABI: __thiscall(ECX entry)(source), RET 4, returns EAX = entry. Callers
// 0052C850 and the counters screen register (src/frontend_seg15_22_cc13.cpp
// calls it through counter_entry_clone_0052c640).
// Evidence: listing 0052C640..0052C83E (disk bytes).
// - Zeroes +70h, +74h, +78h, +7Ch, +80h, +88h, +8Ch, +90h, +94h; +A4h = 1,
//   +9Ch = 3, +A5h/+A6h/+A8h = 0 (EH state 3 is set here; unwind not modelled).
// - Clone root +0h = 00AAB4C0(source[+0h], null).
// - For the source widgets at +4h, +8h, +0Ch, +2Ch, +10h .. +28h (that order):
//   the clone's child with the same name, 00AA7E00(root, widget->vtable+28h(), 1).
//   The PUSH 1 before each +28h call is 00AA7E00's second argument (its RET 8
//   pops it); +28h itself takes no stack arguments.
// - Float copies (FLD/FSTP) of +3Ch..+44h, +30h..+38h, +60h..+6Ch, +48h..+5Ch.
// - Finally the clone root's vtable +34h(0) hides it.
void* cc13_sweepB_counter_entry_construct_clone_0052c640(void* entry, const void* source,
                                                        Cc13SweepBHost& host) {
    for (std::size_t off : {0x70u, 0x74u, 0x78u, 0x7Cu, 0x80u, 0x88u, 0x8Cu, 0x90u, 0x94u}) {
        cc13_sweepB_at<std::uint32_t>(entry, off) = 0;
    }
    cc13_sweepB_at<std::uint8_t>(entry, 0xA4) = 1;
    cc13_sweepB_at<std::uint32_t>(entry, 0x9C) = 3;
    cc13_sweepB_at<std::uint8_t>(entry, 0xA5) = 0;
    cc13_sweepB_at<std::uint8_t>(entry, 0xA6) = 0;
    cc13_sweepB_at<std::uint8_t>(entry, 0xA8) = 0;
    void* src = const_cast<void*>(source);
    cc13_sweepB_at<void*>(entry, 0x0) = host.widget_clone_subtree_00aab4c0(cc13_sweepB_at<void*>(src, 0x0), nullptr);
    for (std::size_t off : {0x4u, 0x8u, 0xCu, 0x2Cu, 0x10u, 0x14u, 0x18u, 0x1Cu, 0x20u, 0x24u, 0x28u}) {
        void* name = host.widget_vcall_28_name(cc13_sweepB_at<void*>(src, off));
        cc13_sweepB_at<void*>(entry, off) = host.widget_find_child_00aa7e00(cc13_sweepB_at<void*>(entry, 0x0), name, 1);
    }
    for (std::size_t off : {0x3Cu, 0x40u, 0x44u, 0x30u, 0x34u, 0x38u, 0x60u, 0x64u, 0x68u, 0x6Cu,
                            0x48u, 0x4Cu, 0x50u, 0x54u, 0x58u, 0x5Cu}) {
        cc13_sweepB_at<float>(entry, off) = cc13_sweepB_at<float>(src, off);
    }
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(entry, 0x0), 0);
    return entry;
}

// 00535430 BSP_InterfaceView_DrawTargetOrtho (hypothesis).
// ABI: __thiscall(ECX view = interface manager +5Ch), RET; the tail JMPs to
// 0093D250 with ECX = view+8h. Host call: src/game_render_frame.cpp
// render_interface_ortho_view_00535430 (hook only, no body there).
// Evidence: listing 00535430..00535618 (disk bytes).
// - Nothing when view+4h is clear. leader = [view+138h]+14h; world = leader+CCh,
//   refreshed through 00414DB0 whenever the valid byte +C8h is clear (the image
//   tests before each read group: once, once, then twice in a row).
// - view+17Ch = world[+30h]; view+180h = world[+38h]; object view+178h gets
//   vtable +30h with (view+17Ch, view+18Ch, world+38h); object view+174h gets it
//   with (view+17Ch, view+188h, view+180h).
// - Look-at into a 4x4 local: eye (world+30h, 10000.0f [00CE3D64], world+38h),
//   target (world+30h, 0, world+38h), up (world+20h, +24h, +28h); camera view+10h
//   takes it through 00B71490.
// - Ortho on camera view+10h: (-w*0.5, w*0.5, -h*0.5, h*0.5, 1.0, 15000.0f
//   [00CED55C]) with w = view+30h, h = view+34h; 0.5 is double 00D7A280.
void cc13_sweepB_interface_view_draw_target_ortho_00535430(void* view, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint8_t>(view, 0x4) == 0) return;
    void* leader = cc13_sweepB_at<void*>(cc13_sweepB_at<void*>(view, 0x138), 0x14);
    auto refresh = [&] {
        if (cc13_sweepB_at<std::uint8_t>(leader, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(leader);
    };
    auto world = [&](std::size_t off) -> float { return cc13_sweepB_at<float>(leader, 0xCC + off); };

    refresh();
    cc13_sweepB_at<float>(view, 0x17C) = world(0x30);
    refresh();
    const float w38 = world(0x38);
    cc13_sweepB_at<float>(view, 0x180) = w38;
    float vec[3] = {cc13_sweepB_at<float>(view, 0x17C), cc13_sweepB_at<float>(view, 0x18C), w38};
    host.object_vcall_30_vec3(cc13_sweepB_at<void*>(view, 0x178), vec);
    vec[0] = cc13_sweepB_at<float>(view, 0x17C);
    vec[1] = cc13_sweepB_at<float>(view, 0x188);
    vec[2] = cc13_sweepB_at<float>(view, 0x180);
    host.object_vcall_30_vec3(cc13_sweepB_at<void*>(view, 0x174), vec);
    refresh();
    refresh();

    const float target[3] = {world(0x30), 0.0f, world(0x38)};
    const float eye[3] = {world(0x30), 10000.0f, world(0x38)};
    float look_at[16];
    float* matrix = host.matrix_build_look_at_00b63f10(look_at, eye, target, world(0x20), world(0x24), world(0x28));
    host.camera_set_view_matrix_00b71490(cc13_sweepB_at<void*>(view, 0x10), matrix);

    const float w = cc13_sweepB_at<float>(view, 0x30);
    const float h = cc13_sweepB_at<float>(view, 0x34);
    const float top = static_cast<float>(h * 0.5);
    const float bottom = static_cast<float>(-h * 0.5);
    const float right = static_cast<float>(w * 0.5);
    const float left = static_cast<float>(-w * 0.5);
    host.camera_set_gui_ortho_00aa2020(cc13_sweepB_at<void*>(view, 0x10), left, right, bottom, top, 1.0f, 15000.0f);
    host.simple3d_scene_submit_0093d250(static_cast<std::uint8_t*>(view) + 0x8);
}

// ===========================================================================
// Segment 16
// ===========================================================================

// 005457C0 BSP_Entity_IsHostileToParty (hypothesis; ledger FUN_005457C0).
// ABI: __thiscall(ECX entity)(party), RET 4, returns EAX 0/1.
// Evidence 005457C0..005457D8. Callers 00547480, 00835F60 and 00836920 (rel32
// census). Hosts: src/game_hosts_commands.cpp and src/gameplay_seg52_cc13.cpp
// describe the same test inline; this is the standalone body.
bool cc13_sweepB_entity_is_hostile_to_party_005457c0(const void* entity, std::int32_t party) {
    return cc13_sweepB_at<std::int32_t>(const_cast<void*>(entity), 0x54) != party && party != 2;
}

// 00548300 BSP_HudWeaponGroupScreen_UpdateGroupSight (hypothesis; ledger FUN_00548300).
// ABI: __thiscall(ECX screen), RET. Caller 005484F0 BSP_HudWeaponGroupScreen_Update
// (host hook group_sight_00548300 in include/bsp/hud_weapon_group_screen.hpp).
// Evidence 00548300..00548339 and the jump table 0054833C (6 entries): nothing
// when +40h (bound unit) is null or the kind +44h is above 5 (unsigned); kind 0
// tail-jumps to 005464E0; kind 1 returns; kinds 2..5 call 00547A20 with 5, 2, 7, 9.
void cc13_sweepB_weapon_group_update_sight_00548300(void* screen, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint32_t>(screen, 0x40) == 0) return;
    switch (cc13_sweepB_at<std::uint32_t>(screen, 0x44)) {
    case 0: host.screen2e_clear_timed_entries_005464e0(screen); break;
    case 1: break;
    case 2: host.weapon_group_sight_00547a20(screen, 5); break;
    case 3: host.weapon_group_sight_00547a20(screen, 2); break;
    case 4: host.weapon_group_sight_00547a20(screen, 7); break;
    case 5: host.weapon_group_sight_00547a20(screen, 9); break;
    default: break;
    }
}

// 00549430 BSP_HudWeaponGroupScreen_RebindFirstUnit (hypothesis; ledger FUN_00549430).
// ABI: __thiscall(ECX screen), RET. Callers 006F3660, 006F5460, 006F7360 (the
// CommandBuilding refresh, src/cc13_seg33b_command_building.cpp
// hud_panel_refresh_00549430).
// Evidence 00549430..005494BB: when the group array end +24h is non-null the
// entry is the first one ([+20h]); otherwise a default 18h-byte entry is built
// on the stack (vtable 00CEDDA0, +4h/+8h/+0Ch/+14h zero, byte +10h = 1). Then
// 00549260(screen, entry+14h, screen+40h), and the temporary (only) is
// destroyed through 00545600. EH frame 00C6CF01 not modelled.
void cc13_sweepB_weapon_group_rebind_first_unit_00549430(void* screen, Cc13SweepBHost& host) {
    struct DefaultEntry {
        std::uint32_t vtable;
        std::uint32_t f04, f08, f0c;
        std::uint8_t f10;
        std::uint8_t pad[3];
        std::uint32_t unit14;
    };
    static_assert(sizeof(DefaultEntry) == 0x18);
    DefaultEntry temp{};
    void* entry;
    bool constructed = false;
    if (cc13_sweepB_at<std::uint32_t>(screen, 0x24) != 0) {
        entry = cc13_sweepB_at<void*>(screen, 0x20);
    } else {
        temp.vtable = 0x00CEDDA0u;  // the image's vtable address, kept as a value
        temp.f04 = temp.f08 = temp.f0c = 0;
        temp.f10 = 1;
        temp.unit14 = 0;
        entry = &temp;
        constructed = true;
    }
    host.weapon_group_bind_unit_00549260(screen, cc13_sweepB_at<void*>(entry, 0x14),
                                         cc13_sweepB_at<void*>(screen, 0x40));
    if (constructed) host.weapon_group_entry_destruct_00545600(&temp);
}

// 00554600 BSP_GuiRect_DrawOutline (hypothesis).
// ABI: __fastcall(ECX a, EDX b)(const float* rgba), RET 4. Callers 00554680,
// 00581DF0, 005D63E0. Evidence 00554600..00554673 (disk bytes).
// b' = (b.x - 0.5, b.y - 0.5), a' = (a.x - 0.5, a.y - 0.5), 0.5 double
// 00D7A280, each stored as float. The renderer (00F8D394) is read before the
// conversions. 004FB850 runs twice on rgba: first into the argument slot
// itself (the image reuses its stack word), then into a local; the renderer's
// vtable +C0h gets (&a', &b', local colour, argument-slot colour).
void cc13_sweepB_gui_rect_draw_outline_00554600(const float* a, const float* b, const float* rgba,
                                                Cc13SweepBHost& host) {
    const float b_prime[2] = {static_cast<float>(b[0] - 0.5), static_cast<float>(b[1] - 0.5)};
    const float a_prime[2] = {static_cast<float>(a[0] - 0.5), static_cast<float>(a[1] - 0.5)};
    void* renderer = host.renderer_00f8d394();
    std::uint32_t arg_slot = 0;
    std::uint32_t local = 0;
    const std::uint32_t* first = host.color_float_rgba_to_argb_004fb850(&arg_slot, rgba);
    const std::uint32_t* second = host.color_float_rgba_to_argb_004fb850(&local, rgba);
    host.renderer_vcall_c0_line(renderer, a_prime, b_prime, *second, *first);
}

// ===========================================================================
// Segment 18
// ===========================================================================

// 00568CE0 BSP_SessionSearchResult_CopyConstruct (hypothesis).
// ABI: __thiscall(ECX object)(source), RET 4, returns EAX = object.
// Evidence 00568CE0..00568D3B: vtable 00CEED04 into +0h; +4h..+18h by dword
// moves, then REP MOVSD of 9, 10 and 8 dwords at +1Ch, +40h, +68h: every byte
// +4h..+87h comes from the source. Caller 0057A9C0 (menu_screens_2.cpp
// describes the fields JoinSession reads from the copy).
void* cc13_sweepB_session_search_result_copy_00568ce0(void* object, const void* source) {
    cc13_sweepB_at<std::uint32_t>(object, 0x0) = 0x00CEED04u;  // image vtable address
    std::memcpy(static_cast<std::uint8_t*>(object) + 0x4, static_cast<const std::uint8_t*>(source) + 0x4, 0x84);
    return object;
}

// 00568EB0 BSP_Singleton00F8894C_ClearBase (hypothesis).
// ABI: __thiscall(ECX object), RET. Evidence 00568EB0..00568EC0: 00F8894C = 0,
// then the base vtable 00CE3818 into +0h. Callers: the unwind funclets 00C6F360 and 00CA2EB0.
void cc13_sweepB_singleton_f8894c_clear_base_00568eb0(void* object, Cc13SweepBHost& host) {
    host.global_00f8894c() = 0;
    cc13_sweepB_at<std::uint32_t>(object, 0x0) = 0x00CE3818u;  // image vtable address
}

// 005692E0 CG_scalar_deleting_dtor_005692e0 (compiler-generated shape).
// ABI: __thiscall(ECX object)(flags), RET 4, returns EAX = object. Evidence
// 005692E0..00569306: 00568EB0's two stores inline, then operator delete
// (00BF65AC, cdecl) when flags bit 0 is set. Held in vtable slot 00CEED6C.
void* cc13_sweepB_singleton_f8894c_scalar_delete_005692e0(void* object, std::uint32_t flags,
                                                         Cc13SweepBHost& host) {
    host.global_00f8894c() = 0;
    cc13_sweepB_at<std::uint32_t>(object, 0x0) = 0x00CE3818u;
    if ((flags & 1u) != 0) host.crt_operator_delete_00bf65ac(object);
    return object;
}

// ===========================================================================
// Segment 19
// ===========================================================================

// 0056E360 BSP_MultiSessionBrowserScreen_BeginFilterEdit (hypothesis).
// ABI: __thiscall(ECX screen), RET. Host call: src/menu_screens_2.cpp
// begin_filter_edit_0056e360 (hook only).
// Evidence 0056E360..0056E3D8: dwords +244h/+248h/+24Ch -> +250h/+254h/+258h;
// bytes +10h..+18h -> +19h..+21h (the image interleaves the loads; all loads
// of a group precede their stores, and the ranges do not overlap); then
// 0056D660(screen) and listbox [+34h] 00A9C7C0(0).
void cc13_sweepB_session_browser_begin_filter_edit_0056e360(void* screen, Cc13SweepBHost& host) {
    for (std::size_t i = 0; i < 3; ++i) {
        cc13_sweepB_at<std::uint32_t>(screen, 0x250 + 4 * i) = cc13_sweepB_at<std::uint32_t>(screen, 0x244 + 4 * i);
    }
    for (std::size_t i = 0; i < 9; ++i) {
        cc13_sweepB_at<std::uint8_t>(screen, 0x19 + i) = cc13_sweepB_at<std::uint8_t>(screen, 0x10 + i);
    }
    host.session_browser_fill_settings_0056d660(screen);
    host.listbox_select_index_00a9c7c0(cc13_sweepB_at<void*>(screen, 0x34), 0);
}

// 0057A910 BSP_MultiSessionBrowserScreen_ApplyFilterEdit (hypothesis).
// ABI: __thiscall(ECX screen), RET. Host call: src/menu_screens_2.cpp
// apply_filter_edit_0057a910 (hook only).
// Evidence 0057A910..0057A9BA: +250h..+258h -> +244h..+24Ch; +19h..+21h ->
// +10h..+18h; 005789C0(screen, [+2B0h]); 0056DD80(screen); then
// 007FBA30(ECX = [00E188A8]+650h, [+244h], [+248h], [+24Ch], screen+10h, 9).
void cc13_sweepB_session_browser_apply_filter_edit_0057a910(void* screen, Cc13SweepBHost& host) {
    for (std::size_t i = 0; i < 3; ++i) {
        cc13_sweepB_at<std::uint32_t>(screen, 0x244 + 4 * i) = cc13_sweepB_at<std::uint32_t>(screen, 0x250 + 4 * i);
    }
    for (std::size_t i = 0; i < 9; ++i) {
        cc13_sweepB_at<std::uint8_t>(screen, 0x10 + i) = cc13_sweepB_at<std::uint8_t>(screen, 0x19 + i);
    }
    host.session_browser_build_rows_005789c0(screen, cc13_sweepB_at<std::uint32_t>(screen, 0x2B0));
    host.session_browser_refresh_filter_rows_0056dd80(screen);
    host.byte_list_record_assign_007fba30(static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x650,
                                          cc13_sweepB_at<std::uint32_t>(screen, 0x244),
                                          cc13_sweepB_at<std::uint32_t>(screen, 0x248),
                                          cc13_sweepB_at<std::uint32_t>(screen, 0x24C),
                                          static_cast<std::uint8_t*>(screen) + 0x10, 9);
}

// 00571EA0 BSP_SessionBrowserRow_Create (hypothesis; it copy-constructs a
// session record). ABI: __stdcall(dest, source), RET 8. Callers 00576D20 and
// 005789C0. Evidence 00571EA0..00571FCA (disk bytes).
// - operator new(88h) (00BF681B); on success its vtable is 00CEED04 and only
//   +4h..+3Fh are copied from source[+0h] (an inline, shorter copy than
//   00568CE0's; +40h..+87h stay uninitialised). dest+0h = the block, but when
//   source[+0h] is null dest+0h = 0 and the block is not freed (as in the image).
//   A null block with a non-null source[+0h] would fault in the image too.
// - dest+4h byte = source+4h; dest+8h wstring = 0056F160(source+8h, 0, npos)
//   (no prior _Tidy of the destination in the image).
// - dwords +24h..+44h, bytes +48h/+49h, 14 dwords +4Ch..+83h, then +88h, +8Ch,
//   +98h, +9Ch, +94h, +90h.
void cc13_sweepB_session_record_copy_construct_00571ea0(void* dest, const void* source,
                                                       Cc13SweepBHost& host) {
    void* src = const_cast<void*>(source);
    void* block = host.crt_operator_new_00bf681b(0x88);
    if (block != nullptr) cc13_sweepB_at<std::uint32_t>(block, 0x0) = 0x00CEED04u;  // image vtable address
    cc13_sweepB_at<void*>(dest, 0x0) = block;
    void* result = cc13_sweepB_at<void*>(src, 0x0);
    if (result != nullptr) {
        for (std::size_t off : {0xCu, 0x10u, 0x14u, 0x18u, 0x4u, 0x8u}) {
            cc13_sweepB_at<std::uint32_t>(block, off) = cc13_sweepB_at<std::uint32_t>(result, off);
        }
        std::memcpy(static_cast<std::uint8_t*>(block) + 0x1C, static_cast<std::uint8_t*>(result) + 0x1C, 9 * 4);
    } else {
        cc13_sweepB_at<void*>(dest, 0x0) = nullptr;
    }
    cc13_sweepB_at<std::uint8_t>(dest, 0x4) = cc13_sweepB_at<std::uint8_t>(src, 0x4);
    host.wstring_assign_0056f160(static_cast<std::uint8_t*>(dest) + 0x8, static_cast<std::uint8_t*>(src) + 0x8, 0,
                                 0xFFFFFFFFu);
    for (std::size_t off = 0x24; off <= 0x44; off += 4) {
        cc13_sweepB_at<std::uint32_t>(dest, off) = cc13_sweepB_at<std::uint32_t>(src, off);
    }
    cc13_sweepB_at<std::uint8_t>(dest, 0x48) = cc13_sweepB_at<std::uint8_t>(src, 0x48);
    cc13_sweepB_at<std::uint8_t>(dest, 0x49) = cc13_sweepB_at<std::uint8_t>(src, 0x49);
    for (std::size_t off = 0x4C; off < 0x4C + 14 * 4; off += 4) {
        cc13_sweepB_at<std::uint32_t>(dest, off) = cc13_sweepB_at<std::uint32_t>(src, off);
    }
    for (std::size_t off : {0x88u, 0x8Cu, 0x98u, 0x9Cu, 0x94u, 0x90u}) {
        cc13_sweepB_at<std::uint32_t>(dest, off) = cc13_sweepB_at<std::uint32_t>(src, off);
    }
}

// ===========================================================================
// Segment 20
// ===========================================================================

namespace {

// Release a by-value native string to the sized pool when its data is set
// (00419CC0, then 00BD1510(data, length + 1, 1)), as every temporary here is.
void cc13_sweepB_release_native_string(const Cc13SweepBNativeString& s, Cc13SweepBHost& host) {
    if (s.data != nullptr) host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), s.data, s.length + 1, 1);
}

}  // namespace

// 005845D0 BSP_MainMenuScreen_AdvanceBackgroundMovie (hypothesis).
// ABI: __thiscall(ECX screen), RET. The main-menu layout binding tail-jumps
// here (include/bsp/main_menu_layout_binding.hpp). Evidence 005845D0..00584743:
// - +58h = (+58h + 1) % 3 (signed IDIV); 00AAC870(movie +340h).
// - suffix = {0, null}, 0041DD40(&suffix, 4, 1), memcpy(data, ".bik"
//   (00CEC130), length + 1) when data is set.
// - number = 004260B0(int [+44h + index*4]); name = 0043C130("movies\FE_bg_0"
//   (00CEF7C0), &number); path = 004261A0(&name, &suffix);
//   00AAE320(movie, &path, 0).
// - path, name, number and suffix are released (in that order); then
//   00AB0EB0(movie). EH 00C6F930 (states 0..3) not modelled.
void cc13_sweepB_main_menu_advance_background_movie_005845d0(void* screen, Cc13SweepBHost& host) {
    cc13_sweepB_at<std::int32_t>(screen, 0x58) = (cc13_sweepB_at<std::int32_t>(screen, 0x58) + 1) % 3;
    host.gui_movie_stop_00aac870(cc13_sweepB_at<void*>(screen, 0x340));
    Cc13SweepBNativeString suffix{0, nullptr};
    host.native_string_resize_0041dd40(&suffix, 4, 1);
    if (suffix.data != nullptr) host.crt_memcpy_00bf7680(suffix.data, ".bik", suffix.length + 1);
    Cc13SweepBNativeString number{};
    Cc13SweepBNativeString name{};
    Cc13SweepBNativeString path{};
    const std::int32_t index = cc13_sweepB_at<std::int32_t>(screen, 0x58);
    void* number_ptr = host.native_string_from_int_004260b0(&number, cc13_sweepB_at<std::int32_t>(screen, 0x44 + index * 4));
    void* name_ptr = host.native_string_prefixed_0043c130(&name, "movies\\FE_bg_0", number_ptr);
    void* path_ptr = host.native_string_concat_004261a0(name_ptr, &path, &suffix);
    host.gui_movie_set_source_00aae320(cc13_sweepB_at<void*>(screen, 0x340), path_ptr, 0);
    cc13_sweepB_release_native_string(path, host);
    cc13_sweepB_release_native_string(name, host);
    cc13_sweepB_release_native_string(number, host);
    cc13_sweepB_release_native_string(suffix, host);
    host.gui_movie_start_00ab0eb0(cc13_sweepB_at<void*>(screen, 0x340));
}

// 005A1B40 BSP_HudMap_CreateAlphaBlendModel (hypothesis).
// ABI: ECX factory, EDX parent, stack (out_node, out_sub, out_model,
// out_material, alpha_source), RET 14h. Callers 005ADDA0, 005B25D0.
// Evidence 005A1B40..005A1C5D:
// - node = factory vtable +8h(6, 1.0f) -> *out_node; sub = node+0Ch ->
//   *out_sub; 00B6D890(sub, parent).
// - type = 00B74310(); model = sub when sub vtable +0Ch(type) is true, else
//   sub+34h -> *out_model.
// - effect name "alphablend" (00CF0020) when alpha_source is non-null, else
//   "mapmodel" (00CF0014), built by 0041E870 into a stack string;
//   *out_material = 005A07B0(*out_model, &name); the string is released.
// - alpha_source non-null: "cAlpha" (00CF000C) the same way,
//   00B18B20(*out_material, &param, alpha_source), released.
// The image hands 0041E870 uninitialised stack strings; here they start
// zeroed. EH 00C71470 not modelled.
void cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40(void* factory, void* parent, void** out_node,
                                                           void** out_sub, void** out_model, void** out_material,
                                                           void* alpha_source, Cc13SweepBHost& host) {
    void* node = host.factory_vcall_08_create(factory, 6, 1.0f);
    *out_node = node;
    void* sub = cc13_sweepB_at<void*>(node, 0xC);
    *out_sub = sub;
    host.node_propagate_root_00b6d890(sub, parent);
    void* type = host.generated_model_current_type_00b74310();
    void* model = host.node_vcall_0c_is_kind_of(*out_sub, type) ? *out_sub : cc13_sweepB_at<void*>(*out_sub, 0x34);
    *out_model = model;
    Cc13SweepBNativeString name{};
    host.native_string_assign_0041e870(&name, alpha_source != nullptr ? "alphablend" : "mapmodel");
    *out_material = host.generated_model_find_material_005a07b0(*out_model, &name);
    cc13_sweepB_release_native_string(name, host);
    if (alpha_source == nullptr) return;
    Cc13SweepBNativeString param{};
    host.native_string_assign_0041e870(&param, "cAlpha");
    host.material_register_float_parameter_00b18b20(*out_material, &param, alpha_source);
    cc13_sweepB_release_native_string(param, host);
}

// 005A48C0 BSP_HudMapScreen_ResolveIconUnit (hypothesis).
// ABI: __thiscall(ECX screen)(unit_slot*, prev_slot*, bool flag), RET 0Ch.
// Caller 005B45C0. Evidence 005A48C0..005A49F5:
// - unit = *unit_slot; when non-null, kind 18h (vtable +5Ch) and screen+2Ch
//   >= 2 (signed) replace it with unit+3D0h. icon = 005A22D0(*unit_slot) when
//   that is non-null; with flag and no icon, icon = 005A1220(*unit_slot).
// - cur == prev with no icon, or with icon+118h set: skip to the end.
// - prev non-null and != cur: p = 005A22D0(prev); when p and p+118h != 0: its
//   widget +20h gets vtable +4h(1) and is cleared, then 005A21E0(p).
// - cur non-null, icon with +118h == 0: widget = flag ? screen+E8h :
//   screen+E4h; when icon+20h is null and widget non-null: icon+20h =
//   00AAB4C0(widget, null), icon+28h = flag. icon+118h = 1, +11Ch = flag,
//   +110h = (float)(clock 00F876A4 + 0.0 (double 00D7A258)).
// - *prev_slot = *unit_slot.
void cc13_sweepB_hud_map_resolve_icon_unit_005a48c0(void* screen, void** unit_slot, void** prev_slot, bool flag,
                                                     Cc13SweepBHost& host) {
    void* icon = nullptr;
    if (*unit_slot != nullptr) {
        if (host.entity_vcall_5c_is_kind(*unit_slot, 0x18) && cc13_sweepB_at<std::int32_t>(screen, 0x2C) >= 2) {
            *unit_slot = cc13_sweepB_at<void*>(*unit_slot, 0x3D0);
        }
        if (*unit_slot != nullptr) icon = host.hud_map_find_unit_icon_005a22d0(screen, *unit_slot);
    }
    if (flag && icon == nullptr) icon = host.hud_map_find_icon_carrying_unit_005a1220(screen, *unit_slot);
    void* const prev = *prev_slot;
    void* const cur = *unit_slot;
    bool skip = false;
    if (cur == prev && (icon == nullptr || cc13_sweepB_at<std::uint32_t>(icon, 0x118) != 0)) skip = true;
    if (!skip) {
        if (prev != nullptr && cur != prev) {
            void* p = host.hud_map_find_unit_icon_005a22d0(screen, prev);
            if (p != nullptr && cc13_sweepB_at<std::uint32_t>(p, 0x118) != 0) {
                if (cc13_sweepB_at<void*>(p, 0x20) != nullptr) {
                    host.object_vcall_04_delete(cc13_sweepB_at<void*>(p, 0x20), 1);
                    cc13_sweepB_at<void*>(p, 0x20) = nullptr;
                }
                host.hud_map_icon_start_blink_005a21e0(p);
            }
        }
        if (*unit_slot != nullptr && icon != nullptr && cc13_sweepB_at<std::uint32_t>(icon, 0x118) == 0) {
            void* widget = flag ? cc13_sweepB_at<void*>(screen, 0xE8) : cc13_sweepB_at<void*>(screen, 0xE4);
            if (cc13_sweepB_at<void*>(icon, 0x20) == nullptr && widget != nullptr) {
                cc13_sweepB_at<void*>(icon, 0x20) = host.widget_clone_subtree_00aab4c0(widget, nullptr);
                cc13_sweepB_at<std::uint8_t>(icon, 0x28) = flag ? 1 : 0;
            }
            cc13_sweepB_at<std::uint32_t>(icon, 0x118) = 1;
            cc13_sweepB_at<std::uint32_t>(icon, 0x11C) = flag ? 1u : 0u;
            cc13_sweepB_at<float>(icon, 0x110) = static_cast<float>(host.global_00f876a4() + 0.0);
        }
    }
    *prev_slot = *unit_slot;
}

// 005A8420 BSP_HudMapIcon_RefreshUnitBadge (hypothesis).
// ABI: __thiscall(ECX icon)(template, unread), RET 8, EAX = icon+38h.
// Caller 005ADDA0. Evidence 005A8420..005A853B:
// - +38h = 0; unit +4h not kind 1Ch (vtable +5Ch), a null template, a null
//   unit or a null unit+760h: return +38h.
// - +38h += 1; when the badge vector +98h (first +9Ch, last +A0h) already holds
//   at least +38h entries (unsigned): return +38h.
// - clone = 00AAB4C0(template, null); when non-null: vtable +34h(0); party =
//   [[G + [G+18ECh]*4 + 18CCh] + 28h] with G = [00E188A8]; row = unit+760h +
//   party*38h; 00AB64C0(clone, row+30h value, row+34h); clone vtable +88h(1,
//   0, 1.0f); 0043DCA0(+98h, &clone). Return +38h.
std::uint32_t cc13_sweepB_hud_map_icon_refresh_unit_badge_005a8420(void* icon, void* badge_template,
                                                                   std::uint32_t unused, Cc13SweepBHost& host) {
    static_cast<void>(unused);
    cc13_sweepB_at<std::uint32_t>(icon, 0x38) = 0;
    if (!host.entity_vcall_5c_is_kind(cc13_sweepB_at<void*>(icon, 0x4), 0x1C)) return cc13_sweepB_at<std::uint32_t>(icon, 0x38);
    if (badge_template == nullptr) return cc13_sweepB_at<std::uint32_t>(icon, 0x38);
    void* unit = cc13_sweepB_at<void*>(icon, 0x4);
    if (unit == nullptr || cc13_sweepB_at<void*>(unit, 0x760) == nullptr) return cc13_sweepB_at<std::uint32_t>(icon, 0x38);
    cc13_sweepB_at<std::uint32_t>(icon, 0x38) += 1;
    const std::uint32_t wanted = cc13_sweepB_at<std::uint32_t>(icon, 0x38);
    void** first = cc13_sweepB_at<void**>(icon, 0x9C);
    const std::uint32_t size =
        first == nullptr ? 0u : static_cast<std::uint32_t>(cc13_sweepB_at<void**>(icon, 0xA0) - first);
    if (size >= wanted) return wanted;
    void* clone = host.widget_clone_subtree_00aab4c0(badge_template, nullptr);
    if (clone != nullptr) {
        host.widget_vcall_34_set_visible(clone, 0);
        void* game = host.global_00e188a8();
        const std::int32_t slot = cc13_sweepB_at<std::int32_t>(game, 0x18EC);
        const std::int32_t party = cc13_sweepB_at<std::int32_t>(cc13_sweepB_at<void*>(game, 0x18CC + slot * 4), 0x28);
        auto* row = static_cast<std::uint8_t*>(cc13_sweepB_at<void*>(unit, 0x760)) + party * 0x38;
        host.gui_icon_add_state_00ab64c0(clone, cc13_sweepB_at<void*>(row, 0x30), row + 0x34);
        host.widget_vcall_88(clone, 1, 0, 1.0f);
        host.pointer_vector_push_back_0043dca0(static_cast<std::uint8_t*>(icon) + 0x98, &clone);
    }
    return cc13_sweepB_at<std::uint32_t>(icon, 0x38);
}

// 0059DA90 BSP_HudMapLayer_Destruct (hypothesis).
// ABI: __thiscall(ECX layer), RET. Caller: the scalar deleting destructor
// 0059FA20. Evidence 0059DA90..0059DADF: vtable 00CEFFE4 into +0h; when +4h
// is non-null its vtable +4h(1) and +4h = 0; the same for the eight slots
// +8h..+24h in order.
void cc13_sweepB_hud_map_layer_destruct_0059da90(void* layer, Cc13SweepBHost& host) {
    cc13_sweepB_at<std::uint32_t>(layer, 0x0) = 0x00CEFFE4u;  // image vtable address
    for (std::size_t off = 0x4; off <= 0x24; off += 4) {
        void* widget = cc13_sweepB_at<void*>(layer, off);
        if (widget != nullptr) {
            host.object_vcall_04_delete(widget, 1);
            cc13_sweepB_at<void*>(layer, off) = nullptr;
        }
    }
}

// 0059E120 BSP_Game_ClampPointToWorldBoundsXZ (hypothesis).
// ABI: stack (point*, extent, extent_v), RET 0Ch; ECX is overwritten before
// use (the caller passes the screen there, include/bsp/map_screen_update_cc13.hpp).
// Caller 005B45C0. Evidence 0059E120..0059E2B2 (disk bytes; FCOMIP pops).
// G = [00E188A8]: minX = (G+7128h > G+711Ch) ? G+711Ch : G+7128h, maxX = the
// opposite test, minZ/maxZ the same over G+7124h/G+7130h (NaN or a tie takes
// the second operand of each test, as the JBE does). lo = min - 500, hi =
// max + 500 (double 00CE3840), stored as float. Each component becomes lo when
// lo > p, else hi when p > hi, else p.
void cc13_sweepB_clamp_point_to_world_bounds_0059e120(float* point, std::uint32_t unused1,
                                                      std::uint32_t unused2, Cc13SweepBHost& host) {
    static_cast<void>(unused1);
    static_cast<void>(unused2);
    void* game = host.global_00e188a8();
    const float a = cc13_sweepB_at<float>(game, 0x711C);
    const float b = cc13_sweepB_at<float>(game, 0x7128);
    const float c = cc13_sweepB_at<float>(game, 0x7124);
    const float d = cc13_sweepB_at<float>(game, 0x7130);
    const float min_x = (b > a) ? a : b;
    const float max_x = (a > b) ? a : b;
    const float min_z = (d > c) ? c : d;
    const float max_z = (c > d) ? c : d;
    const float hi_x = static_cast<float>(max_x + 500.0);
    const float lo_x = static_cast<float>(min_x - 500.0);
    const float px = point[0];
    point[0] = (lo_x > px) ? lo_x : ((px > hi_x) ? hi_x : px);
    const float hi_z = static_cast<float>(max_z + 500.0);
    const float lo_z = static_cast<float>(min_z - 500.0);
    const float pz = point[1];
    point[1] = (lo_z > pz) ? lo_z : ((pz > hi_z) ? hi_z : pz);
}

// 0059E690 BSP_HudMapIconArray_Reserve (hypothesis; an engine counted array).
// ABI: __thiscall(ECX slot)(int n), RET 4. Callers include 0059FAD0 and the
// icon-array release (include/bsp/frontend_screens_cc13.hpp reserve_0059e690).
// Evidence 0059E690..0059E70F: cap = header ? header->capacity : 0. When n >
// cap (signed): block = 00BF55BE(n*4 + 18h); data = (block + 18h) & ~0Fh;
// the old n_old*4 bytes are copied (00BF7680) when a header with a non-zero
// capacity exists (capacity, not size, is the copied count); the new block's
// header is {data, old size or 0, n}; the old header goes to 00BF6989 (also
// when null); the slot takes the block. Otherwise header->capacity = n (the
// image writes through the header without a null test: a null header with
// n <= 0 faults there).
void cc13_sweepB_counted_array_reserve_0059e690(Cc13SweepBCountedArrayHeader** slot, std::int32_t count,
                                                Cc13SweepBHost& host) {
    Cc13SweepBCountedArrayHeader* header = *slot;
    const std::int32_t capacity = header != nullptr ? header->capacity : 0;
    if (count <= capacity) {
        header->capacity = count;
        return;
    }
    auto* block = static_cast<std::uint8_t*>(
        host.crt_operator_new_00bf55be(static_cast<std::uint32_t>(count) * 4u + 0x18u));
    auto* data = reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(block + 0x18) &
                                                 ~static_cast<std::uintptr_t>(0xF));
    header = *slot;
    if (header != nullptr && header->capacity != 0) {
        host.crt_memcpy_00bf7680(data, header->data, static_cast<std::uint32_t>(header->capacity) * 4u);
    }
    auto* fresh = reinterpret_cast<Cc13SweepBCountedArrayHeader*>(block);
    fresh->data = reinterpret_cast<void**>(data);
    header = *slot;
    fresh->size = header != nullptr ? header->size : 0;
    fresh->capacity = count;
    host.crt_free_00bf6989(*slot);
    *slot = fresh;
}

// 0059FAD0 TMPL_PointerArray_PushBack_0059fad0 (hypothesis).
// ABI: __thiscall(ECX slot)(void* const* value), RET 4. Caller 005A4A00.
// Evidence 0059FAD0..0059FB57: when size == capacity (both 0 without a
// header) it reserves max(cap + cap/4, 20h) (signed divide toward zero; JG
// keeps a larger value); then data[size] = *value and size += 1. The image
// reads header->data before its null test; after the reserve the header
// always exists, so the null arms (size 0, store 1) are not reachable.
void cc13_sweepB_counted_array_push_back_0059fad0(Cc13SweepBCountedArrayHeader** slot, void* const* value,
                                                  Cc13SweepBHost& host) {
    Cc13SweepBCountedArrayHeader* header = *slot;
    const std::int32_t size = header != nullptr ? header->size : 0;
    const std::int32_t capacity = header != nullptr ? header->capacity : 0;
    if (size == capacity) {
        std::int32_t grow = capacity + capacity / 4;
        if (grow <= 0x20) grow = 0x20;
        cc13_sweepB_counted_array_reserve_0059e690(slot, grow, host);
    }
    header = *slot;
    header->data[header->size] = *value;
    header->size += 1;
}

// 0059F460 BSP_Entity_CastIfKind02_0059f460 (hypothesis).
// ABI: __thiscall(ECX entity), RET, EAX. Evidence 0059F460..0059F47B: null
// stays null; otherwise the entity when vtable +5Ch(2) returns non-zero AL.
// Byte census: no rel32 or absolute reference (unreferenced).
void* cc13_sweepB_entity_cast_if_kind02_0059f460(void* entity, Cc13SweepBHost& host) {
    if (entity == nullptr) return nullptr;
    return host.entity_vcall_5c_is_kind(entity, 2) ? entity : nullptr;
}

// 005A2650 BSP_HudMapScreen_SetSpeakingUnit (hypothesis).
// ABI: __thiscall(ECX screen)(unit, fallback, bool flag), RET 0Ch. Callers
// include the voice playback (src/voice_playback.cpp notify_005a2650, hook).
// Evidence 005A2650..005A2724 (disk bytes). u = unit ? unit : fallback.
// - Byte [00E188A8]+19C4h clear: screen+53Ch = u, screen+540h = clock 00F876A4.
// - Set: icon = 005A22D0(screen, u). No icon: when the selected unit
//   00E188D8 is non-null, 005A22D0(screen, it) (result unused). With an icon:
//   unless one of its seven slots +48h..+60h is non-null, each slot gets a
//   clone 00AAB4C0([screen+A0h], null), and icon+74h = (float)((flag ? 1.0f
//   [00D7A24C] : 0.0f) + clock).
void cc13_sweepB_hud_map_set_speaking_unit_005a2650(void* screen, void* unit, void* fallback, bool flag,
                                                    Cc13SweepBHost& host) {
    void* chosen = unit != nullptr ? unit : fallback;
    if (cc13_sweepB_at<std::uint8_t>(host.global_00e188a8(), 0x19C4) == 0) {
        cc13_sweepB_at<void*>(screen, 0x53C) = chosen;
        cc13_sweepB_at<float>(screen, 0x540) = host.global_00f876a4();
        return;
    }
    void* icon = host.hud_map_find_unit_icon_005a22d0(screen, chosen);
    if (icon == nullptr) {
        void* selected = host.global_00e188d8();
        if (selected != nullptr) host.hud_map_find_unit_icon_005a22d0(screen, selected);
        return;
    }
    for (std::size_t i = 0; i < 7; ++i) {
        if (cc13_sweepB_at<void*>(icon, 0x48 + 4 * i) != nullptr) return;
    }
    for (std::size_t i = 0; i < 7; ++i) {
        cc13_sweepB_at<void*>(icon, 0x48 + 4 * i) =
            host.widget_clone_subtree_00aab4c0(cc13_sweepB_at<void*>(screen, 0xA0), nullptr);
    }
    const float delay = flag ? 1.0f : 0.0f;
    cc13_sweepB_at<float>(icon, 0x74) = delay + host.global_00f876a4();
}

// ===========================================================================
// Segment 23
// ===========================================================================

// 00604A20 (hypothesis: the in-game interface's "input allowed" test; ledger
// FUN_00604A20). ABI: __thiscall(ECX screen), RET, EAX 0/1. Caller 007BB9A0
// BSP_Plane_IsLocalInputEnabled (pilot_command_path.hpp models the answer as
// global_block_00604a20). Evidence 00604A20..00604A4B.
bool cc13_sweepB_interface_allows_input_00604a20(const void* screen) {
    void* s = const_cast<void*>(screen);
    const std::int32_t mode = cc13_sweepB_at<std::int32_t>(s, 0x900);
    if (mode == 7 || mode == 6 || mode == 3) return false;
    if (mode == 4 && cc13_sweepB_at<std::uint8_t>(s, 0x904) == 0) return false;
    return true;
}

// 00604A50 (hypothesis; ledger FUN_00604A50). ABI: __thiscall(ECX entity),
// RET, EAX 0/1. Callers 0060ABD0 and 007EE8F0 BSP_Unit_AttackCommandApplies
// (game_hosts_script_orders.hpp describes the same test). Evidence
// 00604A50..00604A73: vtable +5Ch(17h) non-zero and byte +C24h clear.
bool cc13_sweepB_entity_kind17_ready_00604a50(void* entity, Cc13SweepBHost& host) {
    if (!host.entity_vcall_5c_is_kind(entity, 0x17)) return false;
    return cc13_sweepB_at<std::uint8_t>(entity, 0xC24) == 0;
}

// ===========================================================================
// Segment 24
// ===========================================================================

// 006057E0 BSP_ResponseCurve_Evaluate (hypothesis).
// ABI: __thiscall(ECX curve)(float x), RET 4, ST0 result. Caller 006062B0.
// Evidence 006057E0..00605899 (disk bytes; 0060588A is DE E1, FSUBRP
// ST(1),ST(0) = 1 - c2, matching the decompiler).
// sign = -1 / 1 / 0 by COMISS (NaN gives 0); a = x when x > 0, else
// -0.0f (00D7A208) - x; t = 00419010(curve[0], 0, curve[1], 1, a);
// s = (float)(t * sign); q = (float)(t * s); result = (float)(q * c2 +
// s * (1 - c2)), c2 = curve[2].
float cc13_sweepB_response_curve_evaluate_006057e0(const float* curve, float x, Cc13SweepBHost& host) {
    std::int32_t sign;
    if (0.0f > x) {
        sign = -1;
    } else if (x > 0.0f) {
        sign = 1;
    } else {
        sign = 0;
    }
    const float sign_f = static_cast<float>(sign);
    const float a = (x > 0.0f) ? x : (-0.0f - x);
    const float t = host.math_interpolate_clamped_00419010(curve[0], 0.0f, curve[1], 1.0f, a);
    const float s = t * sign_f;
    const float c2 = curve[2];
    const float q = t * s;
    return q * c2 + s * (1.0f - c2);
}

// 006062B0 (hypothesis: an analog-axis filter step; ledger FUN_006062B0).
// ABI: __thiscall(ECX filter)(float rate, float input, float dt), RET 0Ch,
// ST0 = output. Caller 0060ABD0 BSP_HudPlaneEffectsScreen_Update.
// Evidence 006062B0..00606463 (Ghidra listing; 0060636F is DE E1, FSUBRP
// ST(1),ST(0) = phase - C). C = double 00D7A3A0 (0.1f widened); |v| is v when
// v > 0 else -0.0f - v (00D7A208).
// - |rate| > C sets +0h = 1; d = (float)(input - +0Ch); |d| > C clears +0h;
//   +0Ch = input.
// - T = 0042E740(); +8h = (float)(T[688h] * rate + +8h).
// - p = (float)(dt + +4h); +4h = p; when p > C: +4h = (float)(p - C); T =
//   0042E740(); when T[694h] > |+8h| (the absolute value taken first):
//   0042E740() again, +8h = (float)(T[698h] * +8h).
// - +8h clamped: -1.0f (00D7A260) when -1 > v, else 1.0f when v > 1.
// - +0h clear: +10h = +0Ch. Set: three 0042E740() calls give the curve
//   {T[68Ch], T[690h], T[69Ch]}; +10h = (float)006057E0(curve, +8h).
float cc13_sweepB_axis_filter_step_006062b0(void* filter, float rate, float input, float dt,
                                            Cc13SweepBHost& host) {
    const double c = 0.10000000149011612;  // 00D7A3A0
    auto magnitude = [](float v) { return (v > 0.0f) ? v : (-0.0f - v); };
    if (static_cast<double>(magnitude(rate)) > c) cc13_sweepB_at<std::uint8_t>(filter, 0x0) = 1;
    const float d = input - cc13_sweepB_at<float>(filter, 0xC);
    if (static_cast<double>(magnitude(d)) > c) cc13_sweepB_at<std::uint8_t>(filter, 0x0) = 0;
    cc13_sweepB_at<float>(filter, 0xC) = input;
    void* tuning = host.game_tuning_get_0042e740();
    cc13_sweepB_at<float>(filter, 0x8) = cc13_sweepB_at<float>(tuning, 0x688) * rate + cc13_sweepB_at<float>(filter, 0x8);
    const float phase = dt + cc13_sweepB_at<float>(filter, 0x4);
    cc13_sweepB_at<float>(filter, 0x4) = phase;
    if (static_cast<double>(phase) > c) {
        cc13_sweepB_at<float>(filter, 0x4) = static_cast<float>(static_cast<double>(phase) - c);
        const float v = magnitude(cc13_sweepB_at<float>(filter, 0x8));
        tuning = host.game_tuning_get_0042e740();
        if (cc13_sweepB_at<float>(tuning, 0x694) > v) {
            tuning = host.game_tuning_get_0042e740();
            cc13_sweepB_at<float>(filter, 0x8) = cc13_sweepB_at<float>(tuning, 0x698) * cc13_sweepB_at<float>(filter, 0x8);
        }
    }
    float value = cc13_sweepB_at<float>(filter, 0x8);
    if (-1.0f > value) {
        value = -1.0f;
    } else if (value > 1.0f) {
        value = 1.0f;
    }
    cc13_sweepB_at<float>(filter, 0x8) = value;
    if (cc13_sweepB_at<std::uint8_t>(filter, 0x0) == 0) {
        cc13_sweepB_at<float>(filter, 0x10) = cc13_sweepB_at<float>(filter, 0xC);
        return cc13_sweepB_at<float>(filter, 0x10);
    }
    float curve[3];
    curve[0] = cc13_sweepB_at<float>(host.game_tuning_get_0042e740(), 0x68C);
    curve[1] = cc13_sweepB_at<float>(host.game_tuning_get_0042e740(), 0x690);
    tuning = host.game_tuning_get_0042e740();
    const float x = cc13_sweepB_at<float>(filter, 0x8);
    curve[2] = cc13_sweepB_at<float>(tuning, 0x69C);
    cc13_sweepB_at<float>(filter, 0x10) = cc13_sweepB_response_curve_evaluate_006057e0(curve, x, host);
    return cc13_sweepB_at<float>(filter, 0x10);
}

// 0060C5B0 (hypothesis; ledger FUN_0060C5B0). ABI: __thiscall(ECX object),
// RET, EAX 0/1. Caller 007C0910 BSP_Plane_SweepCollisionPoints. Evidence
// 0060C5B0..0060C5CD: UCOMISS +198h with 0.0f (00D7A218), LAHF / TEST AH,44h /
// JP: 1 only for an ordered equal compare.
bool cc13_sweepB_field198_is_zero_0060c5b0(const void* object) {
    return cc13_sweepB_at<float>(const_cast<void*>(object), 0x198) == 0.0f;
}

// 0060CB40 BSP_Unit_AsKind9OrNull (hypothesis). ABI: __thiscall(ECX unit),
// RET, EAX. Evidence 0060CB40..0060CB5B. Byte census: unreferenced.
void* cc13_sweepB_unit_as_kind9_0060cb40(void* unit, Cc13SweepBHost& host) {
    if (unit == nullptr) return nullptr;
    return host.entity_vcall_5c_is_kind(unit, 9) ? unit : nullptr;
}

// ===========================================================================
// Segment 25
// ===========================================================================

// 00612DA0 BSP_HudPowerupManagerScreen_RefreshPromptWidgets (hypothesis).
// ABI: __thiscall(ECX screen), RET. Callers 00674B80, 0068C1F0.
// Evidence 00612DA0..00612EF6:
// - Nothing when byte +4h or [+B0h]+121h is set.
// - [[00E198C4]+BCh]+5h set: [+B0h] vtable +34h(0), return.
// - [+B0h] vtable +34h(008ED9C0([00F88C30])) (the image pushes the whole EAX;
//   the bool is passed as 0/1 here); [+D4h] +34h(!has); 00AA8240([+E4h],
//   00AA6750([+E8h], &local)); [+E8h] +34h(!has); [+ECh] +34h(!has) (each
//   "has" a fresh 008ED9C0 call).
// - p = (+F4h - d, +F8h, +FCh) with d = 0.3 (double 00CE3DC8) when
//   0066FFA0([[00E198C4]+BCh]) is false, else 0.1 (double 00D7A3A0), x
//   stored as float; 00AA8240([+B0h], &p).
void cc13_sweepB_powerup_refresh_prompt_widgets_00612da0(void* screen, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint8_t>(screen, 0x4) != 0) return;
    if (cc13_sweepB_at<std::uint8_t>(cc13_sweepB_at<void*>(screen, 0xB0), 0x121) != 0) return;
    if (cc13_sweepB_at<std::uint8_t>(cc13_sweepB_at<void*>(host.global_00e198c4(), 0xBC), 0x5) != 0) {
        host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(screen, 0xB0), 0);
        return;
    }
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(screen, 0xB0),
                                     host.powerup_manager_local_has_usable_008ed9c0(host.global_00f88c30()) ? 1 : 0);
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(screen, 0xD4),
                                     host.powerup_manager_local_has_usable_008ed9c0(host.global_00f88c30()) ? 0 : 1);
    float local[3];
    float* position = host.widget_get_resolved_position_00aa6750(cc13_sweepB_at<void*>(screen, 0xE8), local);
    host.widget_set_resolved_position_00aa8240(cc13_sweepB_at<void*>(screen, 0xE4), position);
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(screen, 0xE8),
                                     host.powerup_manager_local_has_usable_008ed9c0(host.global_00f88c30()) ? 0 : 1);
    host.widget_vcall_34_set_visible(cc13_sweepB_at<void*>(screen, 0xEC),
                                     host.powerup_manager_local_has_usable_008ed9c0(host.global_00f88c30()) ? 0 : 1);
    const bool available = host.support_manager_has_available_0066ffa0(cc13_sweepB_at<void*>(host.global_00e198c4(), 0xBC));
    const double offset = available ? 0.10000000149011612 : 0.30000001192092896;
    float p[3];
    p[1] = cc13_sweepB_at<float>(screen, 0xF8);
    p[2] = cc13_sweepB_at<float>(screen, 0xFC);
    p[0] = static_cast<float>(cc13_sweepB_at<float>(screen, 0xF4) - offset);
    host.widget_set_resolved_position_00aa8240(cc13_sweepB_at<void*>(screen, 0xB0), p);
}

// 0061A400 BSP_HudPowerupManagerScreen_AssignLists (hypothesis).
// ABI: __thiscall(ECX carousel)(vector a by value, vector b by value),
// RET 20h. Caller 0061B0F0. Evidence 0061A400..0061A472: 004F60E0(+18h, &a),
// 004F60E0(+28h, &b), 00618980(carousel); then the by-value vectors are
// destroyed: operator delete (00BF65AC) of a.first, then b.first, each only
// when non-null. EH frame 00C77E50 (unwind of the arguments) not modelled.
void cc13_sweepB_powerup_assign_lists_0061a400(void* carousel, Cc13SweepBVectorByValue a,
                                               Cc13SweepBVectorByValue b, Cc13SweepBHost& host) {
    host.pointer_vector_assign_004f60e0(static_cast<std::uint8_t*>(carousel) + 0x18, &a);
    host.pointer_vector_assign_004f60e0(static_cast<std::uint8_t*>(carousel) + 0x28, &b);
    host.powerup_carousel_refresh_label_00618980(carousel);
    if (a.first != nullptr) host.crt_operator_delete_00bf65ac(a.first);
    if (b.first != nullptr) host.crt_operator_delete_00bf65ac(b.first);
}

// ===========================================================================
// Segment 26
// ===========================================================================

// 006441B0 (hypothesis; ledger FUN_006441B0). ABI: __thiscall(ECX unit), RET,
// AL result. Byte census: unreferenced. Evidence 006441B0..00644218.
// Neither kind 18h nor 0Fh (vtable +5Ch): return with AL = 0 (EAX keeps the
// upper bytes of the last +5Ch result; a bool here). Otherwise kind 18h is
// asked again and, when true, the unit is replaced by [unit+3D0h]; kind =
// 007BCA50(u); for kinds 9, 0Fh, 0Ah, 0Bh the result is !007B9140(u, 0);
// other kinds give 0.
bool cc13_sweepB_unit_ordnance_pending_006441b0(void* unit, Cc13SweepBHost& host) {
    if (!host.entity_vcall_5c_is_kind(unit, 0x18) && !host.entity_vcall_5c_is_kind(unit, 0xF)) return false;
    void* plane = unit;
    if (host.entity_vcall_5c_is_kind(unit, 0x18)) plane = cc13_sweepB_at<void*>(unit, 0x3D0);
    const std::int32_t kind = host.plane_ordnance_icon_kind_007bca50(plane);
    if (kind == 9 || kind == 0xF || kind == 0xA || kind == 0xB) {
        return !host.unit_can_drop_ordnance_007b9140(plane, 0);
    }
    return false;
}

// ===========================================================================
// Segment 28
// ===========================================================================

// 00696450 BSP_FrontEnd_ReadAction55 (hypothesis). ABI: __fastcall(ECX out),
// RET. Caller 0051AA70. Evidence 00696450..00696463: ECX = [00E188A8],
// 004D92B0(55h), the AL result stored to *out.
void cc13_sweepB_frontend_read_action55_00696450(std::uint8_t* out, Cc13SweepBHost& host) {
    *out = host.frontend_action_edge_or_repeat_004d92b0(host.global_00e188a8(), 0x55);
}

// ===========================================================================
// Segment 29
// ===========================================================================

// 006AC260 (hypothesis; ledger FUN_006AC260). ABI: __thiscall(ECX entity)
// (float* out), RET 4. Callers 006AC5D0, 006F3CC0. Evidence 006AC260..006AC2EC
// (bytes: 006AC2B7 DE F1 FDIVRP = 1/len; 006AC2DD D8 C9 FMUL ST0,ST1).
// World refreshed through 00414DB0 when +C8h is clear; v = (world+20h, 0,
// world+28h) (world at +CCh); len = (float)00419440(&v); inv = 1/len when len
// > 0, else 0 (float); out = (inv*v.x, 0.0 (double 00D7A258)*inv, inv*v.z).
void cc13_sweepB_entity_forward_xz_006ac260(void* entity, float* out, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float v[3] = {cc13_sweepB_at<float>(entity, 0xEC), 0.0f, cc13_sweepB_at<float>(entity, 0xCC + 0x28)};
    const float length = host.vector3_length_00419440(v);
    float inv;
    if (length > 0.0f) {
        inv = 1.0f / length;
    } else {
        inv = 0.0f;
    }
    out[0] = inv * v[0];
    out[1] = static_cast<float>(0.0 * inv);
    out[2] = inv * v[2];
}

// 006BE730 BSP_NativeVector20_PushBack_006be730 (hypothesis). ABI:
// __thiscall(ECX array)(const element*), RET 4. Byte census: unreferenced.
// Evidence 006BE730..006BE7DD: when count +4h == capacity +8h: capacity =
// capacity*2 + 2 (stored), block = 00BF55BE(capacity*14h), the count
// elements copied as five dwords each (skipped for a null block, as the
// image tests the destination), old data freed (00BF6989) when non-null,
// data = block. Then data + count*14h receives the five dwords (skipped when
// that address is null) and count += 1.
void cc13_sweepB_native_vector20_push_back_006be730(void* array, const void* element, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint32_t>(array, 0x4) == cc13_sweepB_at<std::uint32_t>(array, 0x8)) {
        const std::uint32_t capacity = cc13_sweepB_at<std::uint32_t>(array, 0x8) * 2u + 2u;
        cc13_sweepB_at<std::uint32_t>(array, 0x8) = capacity;
        auto* block = static_cast<std::uint8_t*>(host.crt_operator_new_00bf55be(capacity * 0x14u));
        std::uint8_t* dest = block;
        for (std::uint32_t i = 0; i < cc13_sweepB_at<std::uint32_t>(array, 0x4); ++i, dest += 0x14) {
            if (dest != nullptr) std::memcpy(dest, cc13_sweepB_at<std::uint8_t*>(array, 0x0) + i * 0x14u, 0x14);
        }
        if (cc13_sweepB_at<void*>(array, 0x0) != nullptr) host.crt_free_00bf6989(cc13_sweepB_at<void*>(array, 0x0));
        cc13_sweepB_at<std::uint8_t*>(array, 0x0) = block;
    }
    std::uint8_t* slot = cc13_sweepB_at<std::uint8_t*>(array, 0x0) + cc13_sweepB_at<std::uint32_t>(array, 0x4) * 0x14u;
    if (slot != nullptr) std::memcpy(slot, element, 0x14);
    cc13_sweepB_at<std::uint32_t>(array, 0x4) += 1;
}

namespace {

// The std::list walk 006AC4D0 and 006AC550 share (node next +0h, value +8h;
// list object at owner+1FCh with the head at +4h). The secure-SCL checks
// (00BF6713) cannot fire on this walk and are not modelled.
template <class Visit>
void cc13_sweepB_walk_entry_list(void* owner, Visit visit) {
    void* head = cc13_sweepB_at<void*>(owner, 0x1FC + 0x4);
    for (void* node = cc13_sweepB_at<void*>(head, 0x0); node != head; node = cc13_sweepB_at<void*>(node, 0x0)) {
        void* value = cc13_sweepB_at<void*>(node, 0x8);
        void* object = cc13_sweepB_at<void*>(value, 0x0);
        if (object == nullptr) object = cc13_sweepB_at<void*>(value, 0x8);
        void* entry = object != nullptr ? static_cast<std::uint8_t*>(object) + 0x1E4 : nullptr;
        visit(entry);
    }
}

}  // namespace

// 006AC4D0 (hypothesis; ledger FUN_006AC4D0). ABI: __thiscall(ECX owner), RET.
// Caller 006F3010 BSP_CommandBuilding_RefreshPads. Evidence 006AC4D0..006AC54C:
// nothing when [00E188A8]+21D0h is null; otherwise each entry goes to
// 00491E70(ECX = [[00E188A8]+21D0h] re-read per entry, entry, 0).
void cc13_sweepB_list_register_entries_006ac4d0(void* owner, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<void*>(host.global_00e188a8(), 0x21D0) == nullptr) return;
    cc13_sweepB_walk_entry_list(owner, [&](void* entry) {
        host.manager21d0_call_00491e70(cc13_sweepB_at<void*>(host.global_00e188a8(), 0x21D0), entry, 0);
    });
}

// 006AC550 (hypothesis; ledger FUN_006AC550). ABI: __thiscall(ECX owner), RET.
// Caller 007497F0 BSP_LandingShip_OnDestroyed. Evidence 006AC550..006AC5CA:
// as 006AC4D0 with 0048CB30(ECX = manager, entry).
void cc13_sweepB_list_unregister_entries_006ac550(void* owner, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<void*>(host.global_00e188a8(), 0x21D0) == nullptr) return;
    cc13_sweepB_walk_entry_list(owner, [&](void* entry) {
        host.manager21d0_call_0048cb30(cc13_sweepB_at<void*>(host.global_00e188a8(), 0x21D0), entry);
    });
}

// ===========================================================================
// Segment 35
// ===========================================================================

// 0070BBA0 BSP_FlagParams_InitDefaults (hypothesis).
// ABI: __thiscall(ECX params), RET, EAX = params. Callers include
// src/cc13_large62.cpp (hook flag_params_init_0070bba0) and
// src/flak_note_seg36_37_argon.cpp (calls the image address).
// Evidence 0070BBA0..0070BC2B: string +8h = {0, null}; +0h = 2.5f (00CF87C8);
// +10h = 0; +4h = 1.5f (00CE380C); 0041DD40(+8h, 11, 0); when its data is
// non-null, memcpy(data, "Zaszlok.tga" (00CFD4FC), length). EH 00C8479B (the
// string's unwind) not modelled.
void* cc13_sweepB_flag_params_init_0070bba0(void* params, Cc13SweepBHost& host) {
    cc13_sweepB_at<std::uint32_t>(params, 0x8) = 0;
    cc13_sweepB_at<std::uint32_t>(params, 0xC) = 0;
    cc13_sweepB_at<float>(params, 0x0) = 2.5f;
    cc13_sweepB_at<std::uint32_t>(params, 0x10) = 0;
    cc13_sweepB_at<float>(params, 0x4) = 1.5f;
    void* string = static_cast<std::uint8_t*>(params) + 0x8;
    host.native_string_resize_0041dd40(string, 11, 0);
    void* data = cc13_sweepB_at<void*>(string, 0x4);
    if (data != nullptr) {
        host.crt_memcpy_00bf7680(data, "Zaszlok.tga", cc13_sweepB_at<std::uint32_t>(string, 0x0));
    }
    return params;
}

// ===========================================================================
// Segment 37
// ===========================================================================

// 0071B5C0 BSP_AuxResourceParser_CreateItem (hypothesis).
// ABI: __thiscall(ECX parser)(source), RET 4, EAX = item. Held in the parser
// vtable slot 00CFD848 (no rel32 caller). Evidence 0071B5C0..0071B61D:
// operator new(54h) (00BF681B); 0071ABB0 on it when non-null (else item =
// null); then 0071B3E0(item, source) without a null test; returns item. EH
// 00C84F5B (free on a throwing constructor) not modelled.
void* cc13_sweepB_aux_parser_create_item_0071b5c0(void* parser, void* source, Cc13SweepBHost& host) {
    static_cast<void>(parser);
    void* block = host.crt_operator_new_00bf681b(0x54);
    void* item = nullptr;
    if (block != nullptr) item = host.resource_item_aux_construct_0071abb0(block);
    host.resource_read_points_item_0071b3e0(item, source);
    return item;
}

// ===========================================================================
// Segment 43
// ===========================================================================

// 00786C40 BSP_PacketStatRecord_Reset (hypothesis). ABI: __thiscall(ECX
// record), RET, EAX = record. Referenced by the immediate at 0078C441 (the
// packet-statistics code), no rel32 caller. Evidence 00786C40..00786C55.
void* cc13_sweepB_packet_stat_record_reset_00786c40(void* record) {
    cc13_sweepB_at<std::uint32_t>(record, 0x0) = 0;
    cc13_sweepB_at<std::uint32_t>(record, 0x4) = 0;
    cc13_sweepB_at<std::uint32_t>(record, 0x8) = 0;
    cc13_sweepB_at<std::uint32_t>(record, 0xC) = 0;
    cc13_sweepB_at<std::uint16_t>(record, 0xC) = 0x270F;
    return record;
}

// ===========================================================================
// Segments 44 and 51: ref-pointer release
// ===========================================================================

namespace {

// Shared body of 0078D850, 0081B5B0 and 0081B5E0 (byte-identical apart from
// their addresses): ESI = *slot; when non-null, InterlockedDecrement (IAT
// 00CE2220) on object+4h, and when the new count is 0 the object's vtable
// +0h with ECX = object and no stack arguments; then *slot = 0 (also when the
// count stayed positive). ABI: __thiscall(ECX slot), RET.
void cc13_sweepB_ref_ptr_release(void** slot, Cc13SweepBHost& host) {
    void* object = *slot;
    if (object == nullptr) return;
    auto* count = reinterpret_cast<volatile std::int32_t*>(static_cast<std::uint8_t*>(object) + 0x4);
    if (host.interlocked_decrement_00ce2220(count) == 0) host.refcounted_vcall_00_destroy(object);
    *slot = nullptr;
}

}  // namespace

// 0078D850 TMPL_RefPtr_Release_0078d850. Evidence 0078D850..0078D878. Byte
// census: unreferenced.
void cc13_sweepB_ref_ptr_release_0078d850(void** slot, Cc13SweepBHost& host) {
    cc13_sweepB_ref_ptr_release(slot, host);
}

// 0081B5B0 TMPL_RefPtr_Release_0081B5B0. Evidence 0081B5B0..0081B5D8 (bytes
// 0081B5CC.. match 0078D86C..). Byte census: unreferenced.
void cc13_sweepB_ref_ptr_release_0081b5b0(void** slot, Cc13SweepBHost& host) {
    cc13_sweepB_ref_ptr_release(slot, host);
}

// 0081B5E0 TMPL_RefPtr_Release_0081B5E0. Evidence 0081B5E0..0081B608. Byte
// census: unreferenced.
void cc13_sweepB_ref_ptr_release_0081b5e0(void** slot, Cc13SweepBHost& host) {
    cc13_sweepB_ref_ptr_release(slot, host);
}

// 0081B1F0 TMPL_RefPtrArray_Destroy_0081B1F0 (hypothesis; an engine array).
// ABI: __thiscall(ECX array), RET. Three callers. Evidence 0081B1F0..0081B244:
// data = +0h, count = +4h (re-read each step, unsigned compare); each slot is
// released as in 0078D850; then free(data) (00BF6989). +0h is not cleared.
void cc13_sweepB_ref_ptr_array_destroy_0081b1f0(void* array, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<void*>(array, 0x0) == nullptr) return;
    for (std::uint32_t i = 0; i < cc13_sweepB_at<std::uint32_t>(array, 0x4); ++i) {
        void** slot = cc13_sweepB_at<void**>(array, 0x0) + i;
        cc13_sweepB_ref_ptr_release(slot, host);
    }
    host.crt_free_00bf6989(cc13_sweepB_at<void*>(array, 0x0));
}

// 0081B3C0 TMPL_RefPtrArray_Grow_0081B3C0 (hypothesis; an engine array).
// ABI: __thiscall(ECX array), RET. Byte census: unreferenced. Evidence
// 0081B3C0..0081B498: capacity +8h = capacity*2 + 2 (stored first); block =
// 00BF55BE(capacity*4); for i < count +4h (unsigned, re-read): when the new
// slot pointer is non-null it is zeroed and, if the old slot holds an object,
// takes it with InterlockedIncrement (00CE221C) on object+4h; then the old slot
// is released (InterlockedDecrement, vtable +0h at zero, slot cleared). The
// old data is freed (00BF6989) when non-null; data = block. EH 00C90EB7 not
// modelled.
void cc13_sweepB_ref_ptr_array_grow_0081b3c0(void* array, Cc13SweepBHost& host) {
    const std::uint32_t capacity = cc13_sweepB_at<std::uint32_t>(array, 0x8) * 2u + 2u;
    cc13_sweepB_at<std::uint32_t>(array, 0x8) = capacity;
    auto* block = static_cast<void**>(host.crt_operator_new_00bf55be(capacity * 4u));
    void** dest = block;
    for (std::uint32_t i = 0; i < cc13_sweepB_at<std::uint32_t>(array, 0x4); ++i, ++dest) {
        if (dest != nullptr) {
            *dest = nullptr;
            void* object = cc13_sweepB_at<void**>(array, 0x0)[i];
            if (object != nullptr) {
                *dest = object;
                host.interlocked_increment_00ce221c(
                    reinterpret_cast<volatile std::int32_t*>(static_cast<std::uint8_t*>(object) + 0x4));
            }
        }
        cc13_sweepB_ref_ptr_release(cc13_sweepB_at<void**>(array, 0x0) + i, host);
    }
    if (cc13_sweepB_at<void*>(array, 0x0) != nullptr) host.crt_free_00bf6989(cc13_sweepB_at<void*>(array, 0x0));
    cc13_sweepB_at<void**>(array, 0x0) = block;
}

// 0081C940 BSP_RefPtrRange_Release (hypothesis). ABI: __fastcall(ECX first,
// EDX last)(two unread stack words), RET 8. Seven callers. Evidence
// 0081C940..0081C97B: each slot in [first, last) released as in 0078D850 (the
// IAT pointer 00CE2220 is loaded once).
void cc13_sweepB_ref_ptr_range_release_0081c940(void** first, void** last, std::uint32_t unused1,
                                                std::uint32_t unused2, Cc13SweepBHost& host) {
    static_cast<void>(unused1);
    static_cast<void>(unused2);
    for (void** slot = first; slot != last; ++slot) cc13_sweepB_ref_ptr_release(slot, host);
}

// 00827DD0 (hypothesis; ledger FUN_00827DD0). ABI: __thiscall(ECX record)
// (float x, float y, float z), RET 0Ch. Caller: the call at 00827F24.
// Evidence 00827DD0..00827E57: +4h = 1; +8h = 00424C40()+5ACh; entity = +0h;
// when its inverse-valid byte +10Ch is clear: 00414DB0(entity) (no +C8h
// test), +10Ch = 1, then 00B63D50(entity+110h, entity+CCh); the y argument
// slot is overwritten with 0.0 and the by-value (x, 0, z) is transformed by
// 004142E0 with entity+110h into +0Ch..+14h.
void cc13_sweepB_local_point_record_set_00827dd0(void* record, float x, float y, float z, Cc13SweepBHost& host) {
    static_cast<void>(y);
    cc13_sweepB_at<std::uint8_t>(record, 0x4) = 1;
    cc13_sweepB_at<float>(record, 0x8) = cc13_sweepB_at<float>(host.game_settings_get_00424c40(), 0x5AC);
    void* entity = cc13_sweepB_at<void*>(record, 0x0);
    if (cc13_sweepB_at<std::uint8_t>(entity, 0x10C) == 0) {
        host.entity_pose_refresh_world_00414db0(entity);
        cc13_sweepB_at<std::uint8_t>(entity, 0x10C) = 1;
        host.matrix_build_affine_inverse_00b63d50(&cc13_sweepB_at<float>(entity, 0x110), &cc13_sweepB_at<float>(entity, 0xCC));
    }
    const float point[3] = {x, 0.0f, z};
    float local[3];
    host.vector3_transform_affine_point_004142e0(point, local, &cc13_sweepB_at<float>(entity, 0x110));
    cc13_sweepB_at<float>(record, 0xC) = local[0];
    cc13_sweepB_at<float>(record, 0x10) = local[1];
    cc13_sweepB_at<float>(record, 0x14) = local[2];
}

// 0081C0A0 (hypothesis; ledger FUN_0081C0A0). ABI: __thiscall(ECX unit), RET,
// EAX = const char*. Held in four unit vtables (00CF90C4, 00CFA78C, 00CFB74C,
// 00CFC3E4). Evidence 0081C0A0..0081C148:
// - First use: guard 00F87534 |= 1, the static string 00F8752C = {0, null},
//   atexit(00CDE2B0) (00BF6FF5).
// - s = +160h when +15Ch != 0 (unsigned), with the empty literal 00F87528 for
//   a null +160h; else s = [+538h]+54h. length = strlen(s), 0 for null.
// - 0041DD40(static, length, 0); when its data is non-null, memcpy(data, s,
//   static length) and return data; else return the empty literal.
const char* cc13_sweepB_unit_display_name_0081c0a0(void* unit, Cc13SweepBHost& host) {
    std::uint32_t& guard = host.global_00f87534();
    void* str = host.global_object_00f8752c();
    if ((guard & 1u) == 0) {
        guard |= 1u;
        cc13_sweepB_at<std::uint32_t>(str, 0x0) = 0;
        cc13_sweepB_at<std::uint32_t>(str, 0x4) = 0;
        host.crt_atexit_00bf6ff5(0x00CDE2B0u);
    }
    const char* s;
    if (cc13_sweepB_at<std::uint32_t>(unit, 0x15C) != 0) {
        s = cc13_sweepB_at<const char*>(unit, 0x160);
        if (s == nullptr) s = host.empty_string_00f87528();
    } else {
        s = cc13_sweepB_at<const char*>(cc13_sweepB_at<void*>(unit, 0x538), 0x54);
    }
    std::uint32_t length = 0;
    if (s != nullptr) {
        while (s[length] != '\0') ++length;
    }
    host.native_string_resize_0041dd40(str, length, 0);
    char* data = cc13_sweepB_at<char*>(str, 0x4);
    if (data == nullptr) return host.empty_string_00f87528();
    host.crt_memcpy_00bf7680(data, s, cc13_sweepB_at<std::uint32_t>(str, 0x0));
    data = cc13_sweepB_at<char*>(str, 0x4);
    return data != nullptr ? data : host.empty_string_00f87528();
}

// 0081C830 BSP_UnitInstance_ApplyControlledEffectScalar (hypothesis). ABI:
// __thiscall(ECX unit)(other, effect), RET 8. Held in four unit vtables
// (00CF92B0, 00CFA978, 00CFB938, 00CFC5D0). Evidence 0081C830..0081C936:
// - unit+700h = clock 00F876A4 (always).
// - other non-null: kind [[other+354h]+80h] in {2,3,4,6} -> +708h = +700h.
//   effect non-null and kind [[other+3F4h]+80h] in {2,3,4}: the by-value
//   reference takes a count (InterlockedIncrement 00CE221C on effect+4h);
//   00815370(effect, byte +1008h ? 0.0f : 1.0f); 0074D780(unit+FFCh,
//   &reference); the reference is released (InterlockedDecrement, vtable +0h
//   at zero). EH 00C910A8 not modelled.
void cc13_sweepB_unit_apply_controlled_effect_scalar_0081c830(void* unit, void* other, void* effect,
                                                             Cc13SweepBHost& host) {
    cc13_sweepB_at<float>(unit, 0x700) = host.global_00f876a4();
    if (other == nullptr) return;
    const std::int32_t kind = cc13_sweepB_at<std::int32_t>(cc13_sweepB_at<void*>(other, 0x354), 0x80);
    if (kind == 2 || kind == 3 || kind == 4 || kind == 6) {
        cc13_sweepB_at<float>(unit, 0x708) = cc13_sweepB_at<float>(unit, 0x700);
    }
    if (effect == nullptr) return;
    const std::int32_t kind2 = cc13_sweepB_at<std::int32_t>(cc13_sweepB_at<void*>(other, 0x3F4), 0x80);
    if (kind2 != 2 && kind2 != 3 && kind2 != 4) return;
    void* reference = effect;
    host.interlocked_increment_00ce221c(reinterpret_cast<volatile std::int32_t*>(static_cast<std::uint8_t*>(effect) + 0x4));
    const float scalar = cc13_sweepB_at<std::uint8_t>(unit, 0x1008) != 0 ? 0.0f : 1.0f;
    host.effect_group_set_scalar_00815370(effect, scalar);
    host.live_effect_references_append_0074d780(static_cast<std::uint8_t*>(unit) + 0xFFC, &reference);
    if (host.interlocked_decrement_00ce2220(reinterpret_cast<volatile std::int32_t*>(static_cast<std::uint8_t*>(effect) + 0x4)) == 0) {
        host.refcounted_vcall_00_destroy(effect);
    }
}

// 0081AF80 (hypothesis; ledger FUN_0081AF80). ABI: __thiscall(ECX owner), RET.
// Byte census: unreferenced. Evidence 0081AF80..0081B001: std::list at
// +2A4h (head +2A8h, next +0h); element = node+8h; when element+0Ch is null
// it gets operator new(1Ch) (00BF681B) with bytes +0Fh = 1, +18h = 1, +19h = 1
// and dwords +10h = +14h = 0 (the rest left uninitialised), or null on
// failure; then [element+0Ch]+14h = 3E7h (999). SCL checks not modelled; the
// image's null-element arm is unreachable (node+8h is never null).
void cc13_sweepB_list_entries_mark_999_0081af80(void* owner, Cc13SweepBHost& host) {
    void* head = cc13_sweepB_at<void*>(owner, 0x2A8);
    for (void* node = cc13_sweepB_at<void*>(head, 0x0); node != cc13_sweepB_at<void*>(owner, 0x2A8);
         node = cc13_sweepB_at<void*>(node, 0x0)) {
        void* element = static_cast<std::uint8_t*>(node) + 0x8;
        if (cc13_sweepB_at<void*>(element, 0xC) == nullptr) {
            void* block = host.crt_operator_new_00bf681b(0x1C);
            if (block != nullptr) {
                cc13_sweepB_at<std::uint8_t>(block, 0xF) = 1;
                cc13_sweepB_at<std::uint32_t>(block, 0x14) = 0;
                cc13_sweepB_at<std::uint32_t>(block, 0x10) = 0;
                cc13_sweepB_at<std::uint8_t>(block, 0x19) = 1;
                cc13_sweepB_at<std::uint8_t>(block, 0x18) = 1;
            }
            cc13_sweepB_at<void*>(element, 0xC) = block;
        }
        cc13_sweepB_at<std::uint32_t>(cc13_sweepB_at<void*>(element, 0xC), 0x14) = 0x3E7;
    }
}

// 00827820 (hypothesis; ledger FUN_00827820). ABI: __thiscall(ECX unit), RET,
// EAX 0/1. Byte census: unreferenced. Evidence 00827820..00827843: settings =
// 00424C40(); 1 when unit+114Ch > settings+464h (FCOMIP; NaN gives 0).
bool cc13_sweepB_unit_field114c_exceeds_setting_00827820(const void* unit, Cc13SweepBHost& host) {
    void* settings = host.game_settings_get_00424c40();
    return cc13_sweepB_at<float>(const_cast<void*>(unit), 0x114C) > cc13_sweepB_at<float>(settings, 0x464);
}

// 0081B610 BSP_FloatArray_Tidy_0081b610 (hypothesis). ABI: __thiscall(ECX
// array), RET. Seven callers. Evidence 0081B610..0081B626: 00818030(array, 0),
// then free (00BF6989) of the header pointer *array (no null test; free
// accepts null).
void cc13_sweepB_float_array_tidy_0081b610(void* array, Cc13SweepBHost& host) {
    host.float_array_resize_00818030(array, 0);
    host.crt_free_00bf6989(cc13_sweepB_at<void*>(array, 0x0));
}

// ===========================================================================
// Segment 45
// ===========================================================================

// 0078DAA0 BSP_OceanEnvironment_Construct (hypothesis). ABI: __thiscall(ECX
// env)(source, char flag), RET 8, EAX = env. Three callers, one in
// src/frontend_seg12_17_cc13.cpp (hook ocean_environment_construct_0078daa0).
// Evidence 0078DAA0..0078DB9C:
// - vtable 00CEB130 then 00D04390 into +0h; +4h = 1; +ACh = +B0h = 0.
// - 00B9A330(ECX = [00F889F0]) (result unused); +A8h = 00BA6FB0(source, flag).
// - spectrum = operator new(90h) -> 00B97100 (null when the allocation
//   fails); 00B9DD90(+A8h, spectrum).
// - +B4h = operator new(94h) -> 00B84E50, or null.
// - The local spectrum reference is released: InterlockedDecrement(spectrum
//   +4h), vtable +0h at zero (only when spectrum is non-null).
// EH 00C8AC86 (states 0..3) not modelled.
void* cc13_sweepB_ocean_environment_construct_0078daa0(void* env, void* source, std::uint8_t flag,
                                                       Cc13SweepBHost& host) {
    cc13_sweepB_at<std::uint32_t>(env, 0x0) = 0x00CEB130u;  // image vtable addresses
    cc13_sweepB_at<std::uint32_t>(env, 0x4) = 1;
    cc13_sweepB_at<std::uint32_t>(env, 0x0) = 0x00D04390u;
    cc13_sweepB_at<std::uint32_t>(env, 0xAC) = 0;
    cc13_sweepB_at<std::uint32_t>(env, 0xB0) = 0;
    host.trivial_00b9a330(host.global_00f889f0());
    cc13_sweepB_at<void*>(env, 0xA8) = host.ocean_wave_field_create_00ba6fb0(source, flag);
    void* block = host.crt_operator_new_00bf681b(0x90);
    void* spectrum = block != nullptr ? host.ocean_spectrum_construct_00b97100(block) : nullptr;
    host.ocean_wave_field_set_spectrum_00b9dd90(cc13_sweepB_at<void*>(env, 0xA8), spectrum);
    void* fog_block = host.crt_operator_new_00bf681b(0x94);
    cc13_sweepB_at<void*>(env, 0xB4) = fog_block != nullptr ? host.system_fog_owner_construct_00b84e50(fog_block) : nullptr;
    if (spectrum != nullptr) {
        void* slot = spectrum;
        cc13_sweepB_ref_ptr_release(&slot, host);
    }
    return env;
}

// 0078E370 BSP_MovCamInterpolator_Init (hypothesis). ABI: __thiscall(ECX
// interpolator)(word, float a, b, c, d), RET 14h, EAX = interpolator. Caller
// 0076A8A0 BSP_Session_ReceiveMovCamAddInterpolator (multiplayer_session_cc13.cpp hook
// new_interpolator_0078e370). Evidence 0078E370..0078E3AC.
void* cc13_sweepB_movcam_interpolator_init_0078e370(void* interpolator, std::uint32_t word, float a, float b,
                                                    float c, float d) {
    cc13_sweepB_at<float>(interpolator, 0x4) = a;
    cc13_sweepB_at<float>(interpolator, 0x8) = b;
    cc13_sweepB_at<float>(interpolator, 0xC) = c;
    cc13_sweepB_at<float>(interpolator, 0x10) = d;
    cc13_sweepB_at<std::uint32_t>(interpolator, 0x0) = word;
    cc13_sweepB_at<float>(interpolator, 0x14) = 0.0f;
    return interpolator;
}

// 0078FC60 BSP_MovCam_GetPathPointPair (hypothesis). ABI: __thiscall(ECX
// owner)(float* out, int index), RET 8, EAX = out. Caller 007A7200. Evidence 0078FC60..0078FC7E:
// record = 007AD070(owner, index); out = (record+34h, record+38h).
float* cc13_sweepB_movcam_path_point_pair_0078fc60(void* owner, float* out, std::int32_t index,
                                                   Cc13SweepBHost& host) {
    void* record = host.movcam_path_point_007ad070(owner, index);
    out[0] = cc13_sweepB_at<float>(record, 0x34);
    out[1] = cc13_sweepB_at<float>(record, 0x38);
    return out;
}

// 0078FCC0 MovieCameraKeyframe_GetPosition (hypothesis). ABI: __thiscall(ECX
// keyframe)(float* out), RET 4, EAX = out. Caller 00795650
// MovieCameraKeyframe_EvaluatePosition (hud_movie_camera.cpp notes 0079571B). Evidence 0078FCC0..0078FCDE (FLD/FSTP copies).
float* cc13_sweepB_movie_keyframe_get_position_0078fcc0(const void* keyframe, float* out) {
    void* k = const_cast<void*>(keyframe);
    out[0] = cc13_sweepB_at<float>(k, 0x810);
    out[1] = cc13_sweepB_at<float>(k, 0x814);
    out[2] = cc13_sweepB_at<float>(k, 0x818);
    return out;
}

// 00794B50 BSP_MovCam_AimDeltaToTargetDefault (hypothesis). ABI:
// __thiscall(ECX camera)(float* out, target), RET 8, EAX = out. Caller 007A91A0. Evidence
// 00794B50..00794B70: 00794910(camera, out, target, [[camera+4D0h]+54h]).
float* cc13_sweepB_movcam_aim_delta_default_00794b50(void* camera, float* out, void* target,
                                                     Cc13SweepBHost& host) {
    const float value = cc13_sweepB_at<float>(cc13_sweepB_at<void*>(camera, 0x4D0), 0x54);
    host.movcam_aim_delta_00794910(camera, out, target, value);
    return out;
}

// 00797E40 MovieCameraTrack_ForwardToKeyModifiers (hypothesis). ABI:
// __thiscall(ECX track)(float value), RET 4. Caller 00798C50 (twice).
// Evidence 00797E40..00797EC7: std::list at track+68h (head +6Ch, node next
// +0h, key +8h); key kind +F8h not 0 and not 3, modifier key+E4h non-null:
// modifier vtable +10h(value). The secure-SCL checks (00BF6713) cannot fire
// on this walk and are not modelled.
void cc13_sweepB_movie_track_forward_modifiers_00797e40(void* track, float value, Cc13SweepBHost& host) {
    void* head = cc13_sweepB_at<void*>(track, 0x6C);
    for (void* node = cc13_sweepB_at<void*>(head, 0x0); node != cc13_sweepB_at<void*>(track, 0x6C);
         node = cc13_sweepB_at<void*>(node, 0x0)) {
        void* key = cc13_sweepB_at<void*>(node, 0x8);
        const std::int32_t kind = cc13_sweepB_at<std::int32_t>(key, 0xF8);
        if (kind == 0 || kind == 3) continue;
        void* modifier = cc13_sweepB_at<void*>(key, 0xE4);
        if (modifier == nullptr) continue;
        host.keyframe_modifier_vcall_10(modifier, value);
    }
}

// 00793670 BSP_Recon_EnemyContactIndexOf (hypothesis). ABI: __thiscall(ECX
// slot)(unit), RET 4, EAX index or -1. Caller 007A84D0. Evidence
// 00793670..007936C3: for i in [0, +DE4h): walk the singly linked list from
// +DE8h (next +4h) to node i; record = node+8h's pointer; return i when
// record+4h == unit. The inlined accessor's out-of-range arm reads [0+4h]
// (the image faults there); a list as long as the count never takes it, and
// this code dereferences the null record the same way.
std::int32_t cc13_sweepB_recon_enemy_contact_index_of_00793670(const void* slot, const void* unit) {
    void* s = const_cast<void*>(slot);
    const std::int32_t count = cc13_sweepB_at<std::int32_t>(s, 0xDE4);
    for (std::int32_t i = 0; i < count; ++i) {
        void* record = nullptr;
        if (i >= 0 && i < count) {
            std::int32_t j = 0;
            for (void* node = cc13_sweepB_at<void*>(s, 0xDE8); node != nullptr; node = cc13_sweepB_at<void*>(node, 0x4), ++j) {
                if (j == i) {
                    record = cc13_sweepB_at<void*>(node, 0x8);
                    break;
                }
            }
        }
        if (cc13_sweepB_at<const void*>(record, 0x4) == unit) return i;
    }
    return -1;
}

// 00799DE0 BSP_MovieKeyframe_SetModifier (hypothesis). ABI: __thiscall(ECX
// keyframe)(modifier), RET 4. Callers 007A0EB0, 007A2370
// (engine_seg43_45_cc13.cpp has only the hook set_modifier_00799de0).
// Evidence 00799DE0..00799E93:
// - old +E4h: vtable +0h(1), +E4h = 0; +E4h = modifier; modifier vtable
//   +4h(keyframe); kind = [+E4h] vtable +30h().
// - kind 3: byte +25h = 1; unit = +1Ch; when non-null: kind 0Fh (vtable +5Ch)
//   -> +C0h = 0.6981317f (3F32B8C3, 00CE7D20) and, when [+E4h]+78h is set,
//   +C0h = (float)(0042E740()[84h] * +C0h); else kind 6 -> +C0h = 0.5235988f
//   (3F060A92, 00CEC724).
void cc13_sweepB_movie_keyframe_set_modifier_00799de0(void* keyframe, void* modifier, Cc13SweepBHost& host) {
    void* old = cc13_sweepB_at<void*>(keyframe, 0xE4);
    if (old != nullptr) {
        host.object_vcall_00_delete(old, 1);
        cc13_sweepB_at<void*>(keyframe, 0xE4) = nullptr;
    }
    cc13_sweepB_at<void*>(keyframe, 0xE4) = modifier;
    host.keyframe_modifier_vcall_04_attach(modifier, keyframe);
    if (host.keyframe_modifier_vcall_30_kind(cc13_sweepB_at<void*>(keyframe, 0xE4)) != 3) return;
    cc13_sweepB_at<std::uint8_t>(keyframe, 0x25) = 1;
    void* unit = cc13_sweepB_at<void*>(keyframe, 0x1C);
    if (unit == nullptr) return;
    if (host.entity_vcall_5c_is_kind(unit, 0xF)) {
        cc13_sweepB_at<float>(keyframe, 0xC0) = cc13_sweepB_f32(0x3F32B8C3u);
        if (cc13_sweepB_at<std::uint8_t>(cc13_sweepB_at<void*>(keyframe, 0xE4), 0x78) == 0) return;
        void* tuning = host.game_tuning_get_0042e740();
        cc13_sweepB_at<float>(keyframe, 0xC0) = cc13_sweepB_at<float>(tuning, 0x84) * cc13_sweepB_at<float>(keyframe, 0xC0);
        return;
    }
    if (host.entity_vcall_5c_is_kind(cc13_sweepB_at<void*>(keyframe, 0x1C), 6)) {
        cc13_sweepB_at<float>(keyframe, 0xC0) = cc13_sweepB_f32(0x3F060A92u);
    }
}

// 0079C830 BSP_MovCam_ClearPathRecords (hypothesis). ABI: __thiscall(ECX
// camera), RET. Callers 0079D640, 007A7200, 007A74F0, 007A7C60 (the hook
// clear_path_records_0079c830 in engine_seg43_45_cc13.cpp). Evidence
// 0079C830..0079C8EF: +474h = 0 when non-zero; std::vector<T*> at +478h
// (first +47Ch, last +480h): each non-null slot gets vtable +0h(1) and is
// cleared; then the inlined erase(first, last): count = (last - last) / 4 = 0,
// so the memmove_s (00BF67A7) is skipped and last = first. The secure-SCL
// iterator checks (00BF6713) cannot fire and are not modelled.
void cc13_sweepB_movcam_clear_path_records_0079c830(void* camera, Cc13SweepBHost& host) {
    if (cc13_sweepB_at<std::uint32_t>(camera, 0x474) != 0) cc13_sweepB_at<std::uint32_t>(camera, 0x474) = 0;
    void** const end = cc13_sweepB_at<void**>(camera, 0x480);
    for (void** slot = cc13_sweepB_at<void**>(camera, 0x47C); slot != end; ++slot) {
        if (*slot != nullptr) {
            host.object_vcall_00_delete(*slot, 1);
            *slot = nullptr;
        }
    }
    void** const tail = cc13_sweepB_at<void**>(camera, 0x480);
    void** const first = cc13_sweepB_at<void**>(camera, 0x47C);
    if (first == tail) return;
    const std::int32_t count = static_cast<std::int32_t>(cc13_sweepB_at<void**>(camera, 0x480) - tail);
    if (count > 0) {
        host.crt_memmove_s_00bf67a7(first, static_cast<std::uint32_t>(count) * 4u, tail,
                                    static_cast<std::uint32_t>(count) * 4u);
    }
    cc13_sweepB_at<void**>(camera, 0x480) = first + count;
}

// 00798C50 BSP_HudMovieCamera_ForwardToTrackModifiers (hypothesis). ABI:
// __thiscall(ECX camera)(float value), RET 4. Held in vtable slot 00D0471C.
// Evidence 00798C50..00798C7A: 00797E40(camera+2A4h, value), then
// 00797E40(camera+328h, value).
void cc13_sweepB_movie_camera_forward_modifiers_00798c50(void* camera, float value, Cc13SweepBHost& host) {
    host.movie_track_forward_modifiers_00797e40(static_cast<std::uint8_t*>(camera) + 0x2A4, value);
    host.movie_track_forward_modifiers_00797e40(static_cast<std::uint8_t*>(camera) + 0x328, value);
}

// 005A3EF0 BSP_Node_FindChildByName_005a3ef0 (hypothesis; it sets a factor on
// every matching child rather than returning one).
// ABI: __stdcall(node, NativeString name by value (2 words), float value),
// RET 10h. Caller 005A4020 BSP_HudMapScreen_SetNodeVisibleByName.
// Evidence 005A3EF0..005A401A (disk bytes). Children: first node+34h, next
// child+3Ch. Per child: header = 00B6D800(child); idx = position of the first
// ':' (strstr 00BF9440 with ":" at 00CF00B0) when header data is non-null,
// else -1; start = max(idx + 1, 0); sub = 00469840(header, &sub, start,
// 7FFFFFFFh). Match: equal lengths and (both empty, or _stricmp(sub, name) ==
// 0). On a match 00B6DA70(child, value, 0). sub is returned to the pool
// (00419CC0, 00BD1510(data, length + 1, 1)) when its data is non-null. After
// the loop the by-value name is released the same way. EH frame 00C71690
// (states 0/1) not modelled.
void cc13_sweepB_node_set_child_factor_by_name_005a3ef0(void* node, Cc13SweepBNativeString name, float value,
                                                        Cc13SweepBHost& host) {
    for (void* child = cc13_sweepB_at<void*>(node, 0x34); child != nullptr;
         child = cc13_sweepB_at<void*>(child, 0x3C)) {
        auto* header = static_cast<Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(child));
        std::int32_t index = -1;
        if (header->data != nullptr) {
            const char* colon = host.crt_strstr_00bf9440(header->data, ":");
            if (colon != nullptr) index = static_cast<std::int32_t>(colon - header->data);
        }
        std::int32_t start = index + 1;
        if (start < 0) start = 0;
        Cc13SweepBNativeString sub{};
        host.native_string_substring_00469840(header, &sub, start, 0x7FFFFFFF);
        if (sub.length == name.length) {
            std::int32_t compare;
            if (sub.length == 0) {
                compare = name.length != 0 ? -1 : 0;
            } else if (name.length == 0) {
                compare = 1;
            } else {
                compare = host.crt_stricmp_00bf7fbf(sub.data, name.data);
            }
            if (compare == 0) host.scene_node_set_visibility_factor_00b6da70(child, value, 0);
        }
        if (sub.data != nullptr) {
            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), sub.data, sub.length + 1, 1);
        }
    }
    if (name.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, name.length + 1, 1);
    }
}

}  // namespace bsp
