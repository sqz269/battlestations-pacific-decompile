#pragma once
// Screen 2Eh, the weapon-group screen: its unit array, the bind 00549260 that
// 0064DA40 runs, and the per-call body 005484F0 that screen 46h's update
// 0064D610 runs. Packet cc9_screen_2eh_group, docs/SHIP_SCREEN_UPDATE.md
// sections 27 and 28. Names are descriptive hypotheses, not recovered symbols.
// A new C++ interface over the screen's fields; not ABI-compatible.
//
// The screen holds a selected weapon group at +44h (0..5). Selecting a group
// makes the local player take that group's gunner roles on every unit in the
// array (00545410 -> 0077C470), and while the player holds them 005484F0
// routes fire message 79h to those units every call (0077C2A0).
//
//   group  009542B0 test (unit)                         roles (00545370)  mask (00545410)
//   0      always available                             none needed       none
//   1      never (the switch's default)                 2                 04h
//   2      permission +194h, an anti-air gun            2 or 3            0Ch
//   3      permission +198h, an artillery gun           4                 10h
//   4      permission +19Ch, a kind-7 gun (torpedo)     5                 20h
//   5      permission +1A4h, a kind-8 or kind-9 gun     7                 80h

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// One 18h-byte entry of the array at +20h (data), +24h (count), +28h
// (capacity). Vtable 00CEDDA0 over the base 00CEDD88: 005460A0 constructs it,
// 00545600 destroys it. The entry is an observer: +4h..+Ch its callback-owner
// list, +10h a byte set to 1, +14h the observed unit.
struct HudWeaponGroupEntry {
    std::size_t unit{0};    // +14h, unit index + 1; 0 none
};

struct HudWeaponGroupScreenState {
    // +40h, the observed object 00549260 stores through the observer pair at
    // +2Ch. 0064DA40 passes its +20h, the ShipCaptain mover it constructed.
    bool mover_40{false};
    std::vector<HudWeaponGroupEntry> entries;   // +20h/+24h/+28h
    int group_44{0};                // +44h, the selected group 0..5
    std::size_t target_54{0};       // +54h, [[00E198C4]+CCh]+4Ch, unit index + 1
    int row_d0{0};                  // +D0h, the widget row of the last call
    bool flag_d5{false};            // +D5h, passed to the group-5 widget +98h
    bool lst_rocket_d6{false};      // +D6h, the player unit is "LST - Rocket"
    bool hold_100{false};           // +100h, group 5's held-fire latch
    float fire_press_104{0.0f};     // +104h, [00F876A4] when action 99h was pressed
    bool one_shot_108{false};       // +108h
    bool one_shot_109{false};       // +109h
    float pitch_10c{0.0f};          // +10Ch, copied to the periscope mover's +388h
    float yaw_110{0.0f};            // +110h, copied to the periscope mover's +384h
    // +D4h, the show argument of every row widget call. 00545360 sets it and
    // 00545AC0 clears it (both from screen 45h's 0064DD30); the enter 005494C0
    // sets it and the exit 005470A0 clears it.
    bool flag_d4{false};

    // The widgets the layout 00546A20 finds (section 37). Opaque host
    // handles, 0 when absent. cells[row][column] is +5Ch + row*14h + column*4h:
    // row 0 ship_AA_Group, 1 ship_art_Group, 2 ship_torpedo_Group, 3
    // ship_DC_Group, 4 ship_rocket_Group; column 0 cross__Icon, 1 cross_F_Icon,
    // 2 cross_H_Icon, 3 cross_HF_Icon, 4 cross_L_Icon.
    std::uintptr_t cells[5][5]{};
    std::uintptr_t disable_c0{0};   // +C0h CrosshairDisable_Icon (GUI_cross_gunstate)
    std::uintptr_t aim_c4{0};       // +C4h cross_botton_Icon (ship_art_Group)
    std::uintptr_t aim_c8{0};       // +C8h cross_left_Icon
    std::uintptr_t aim_cc{0};       // +CCh cross_right_Icon
    std::uintptr_t circle_d8{0};    // +D8h circle_Section (GUI_cross_gunstate)
    std::uintptr_t gunstate_fc{0};  // +FCh GunState_Icon (GUI_cross_gunstate)
    bool layout_done{false};
};

// The GUI side of the layout. The pages are +58h (GUI_cross_ship) and +DCh
// (GUI_cross_gunstate), both loaded by the register 005468B0 through
// BSP_GuiManager_LoadPage. Lookups are BSP_GuiWidget_FindChildByName
// (00AA7E00) with its recursive flag set; the GUI resource code behind them
// is a contract, not reconstructed here.
struct HudWeaponGroupLayoutHost {
    virtual ~HudWeaponGroupLayoutHost() = default;
    virtual std::uintptr_t page_58() = 0;     // GUI_cross_ship's root
    virtual std::uintptr_t page_dc() = 0;     // GUI_cross_gunstate's root
    virtual std::uintptr_t find_child(std::uintptr_t parent, const char* name) = 0;
    // 00546DA2..00546E9D: cross_F2_Icon under a row's F cell gets local
    // bounds of half the F cell's size (00AA6740, 00AA7DC0); rows 1 and 2
    // then get a resolved position (00AA6750, 00AA8240) with y 00CE3800 or
    // 00CEDE2C. The F2 handle is not stored.
    virtual void place_f2(std::uintptr_t f_cell, std::uintptr_t f2, int row) = 0;
    virtual void set_visible(std::uintptr_t widget, bool visible) = 0;   // vtable +34h
};

// 00546A20, __fastcall(screen), vtable 00CEDF34 slot +14h, plain RET at
// 00547076. Fills the widget handles above, clears +D0h, and hides GunState.
void weapon_group_layout_00546a20(HudWeaponGroupScreenState& screen,
                                  HudWeaponGroupLayoutHost& host);

// 00545410's role mask for a group, 0 for group 0 and outside 1..5.
std::uint32_t weapon_group_role_mask_00545410(int group) noexcept;

// What the weapon-group routines ask of the process. One method per native
// call site or per native routine the body reaches; units are index + 1.
struct HudWeaponGroupScreenHost {
    virtual ~HudWeaponGroupScreenHost() = default;

    // ---- unit queries ----------------------------------------------------
    virtual bool unit_is_kind_of(std::size_t unit, int kind) = 0;        // vtable +5Ch
    virtual bool local_player_role(std::size_t unit, int role) = 0;      // 00927F30
    // 009542B0(unit, group). Group 0 answers true and group 1 false without a
    // unit query; the host answers only 2..5.
    virtual bool group_available_009542b0(std::size_t unit, int group) = 0;
    // 0077C470(mask, take) on the unit: the 4Bh role message.
    virtual void role_request_0077c470(std::size_t unit, std::uint32_t mask, bool take) = 0;

    // ---- the bind 00549260 and the release 005470C0 -----------------------
    // A kind-1Ch unit's 64h-byte records at +778h (count +77Ch): the unit at
    // each record's +14h, in order, zero entries dropped.
    virtual std::vector<std::size_t> unit_records_778(std::size_t unit) = 0;
    // 005470C0's first loop: every child on unit+48h of kind 22h gets
    // +490h = 0.
    virtual void clear_child_flags_005470c0(std::size_t unit) = 0;
    virtual void reset_widgets_005464e0() = 0;
    // 005493EA: [00E188D8] vtable +10h compared with "LST - Rocket" (00CEDF90).
    virtual bool player_unit_is_lst_rocket() = 0;
    // 00548360 past its screen-side tests: the applied byte +5h, [00E188D8]
    // of kind 18h whose +3D0h is of kind 10h, then the "LVLAA" award hint.
    virtual void lvlaa_hint_00548360() = 0;

    // ---- 005484F0 ----------------------------------------------------------
    virtual bool player_unit_present() = 0;                              // [00E188D8]
    // 0054856B..00548606 and 0054861B..00548672, each run once when its byte
    // is found set: the binocular and periscope screens' camera resets.
    virtual void one_shot_108_0054856b() = 0;
    virtual void one_shot_109_0054861b(float pitch_10c, float yaw_110) = 0;
    virtual void hide_widgets_005452f0() = 0;                            // 25 slots, +C0h..+CCh
    virtual void group_sight_00548300(int group) = 0;                    // 005464E0 / 00547A20
    virtual void rocket_widgets_00548716() = 0;                          // +C4h..+CCh, group 3
    virtual void widget_98_005486c7(bool flag_d5) = 0;                   // group 5
    virtual bool input_pressed(int action) = 0;                          // 004C43C0
    virtual bool input_held(int action) = 0;                             // 004C5090
    virtual bool input_axis_active_004d9480(int action) = 0;
    virtual float input_axis_value_004c5070(int action) = 0;
    virtual bool input_in_set_00547250(int action) = 0;                  // input+5B0h set
    virtual float clock_f876a4() = 0;
    // 00548B06..00548C42 (action 9Bh pressed with 99h held) and
    // 00548CB6..00548E45 (99h held on group 4). Each ends in the interface
    // request 004CC460(26h, unit) and returns true when it did, which ends
    // the call.
    virtual bool lock_branch_00548b06(HudWeaponGroupScreenState& screen) = 0;
    virtual bool torpedo_branch_00548cb6(HudWeaponGroupScreenState& screen) = 0;
    virtual std::size_t hud_target_00548856() = 0;                       // [[00E198C4]+CCh]+4Ch
    // The role-held block.
    virtual float gunner_distance_00548ea9(std::size_t unit, std::size_t target) = 0;
    virtual float gunner_range_49c(std::size_t unit) = 0;                // unit+49Ch
    virtual bool target_virtual_10c(std::size_t target) = 0;             // vtable +10Ch
    // 00803CE0(unit+54h, target): the side's relation to the target.
    virtual int relation_00803ce0(std::size_t unit, std::size_t target) = 0;
    virtual bool same_side_54(std::size_t unit, std::size_t target) = 0; // unit+54h == target+54h
    virtual std::uint16_t target_id_174(std::size_t target) = 0;        // target+174h
    virtual void target_debug_00548942(std::size_t target) = 0;          // [00E19B4D] set
    // The widget slot at +5Ch + row*14h + column*4h (column 0..4): whether it
    // is non-null, and its vtable +34h(byte +D4h). The body tests presence
    // before some calls (00548FCA precedes the third 00803CE0).
    virtual bool row_widget_present(int row, int column) = 0;
    virtual void show_row_widget(int row, int column, bool shown_d4) = 0;
    // 00954A10 over the mover's aim (0042D7E0, 00521370, 00427EB0), then
    // 0077C2A0(unit, message, 3, 0) for one unit.
    virtual void build_fire_message_00954a10(int group, bool pressed, bool held,
                                             bool has_target, std::uint16_t target_id) = 0;
    virtual void route_fire_message_0077c2a0(std::size_t unit) = 0;
    virtual bool camera_busy_00518f30() = 0;                             // [00E198C4]+4Ch
    // Group 5: a child of kind 24h on unit+48h whose +3F4h+80h is 9.
    virtual bool depth_charge_child_00549061(std::size_t unit) = 0;
    // 005490A6..0054911A: kind 0Eh -> 005452B0 on +80h, else 00545270 on
    // +7Ch; then 0051E8E0 on +4Ch.
    virtual void hold_camera_005490a6(std::size_t unit) = 0;
    virtual int input_mode_f8a0c4() = 0;                                 // [00F8A0C4]+178h
    virtual void show_widget_c0_005491c2() = 0;
};

// 005467B0(0), 005460A0 and 00546730 as the array operations they are, and
// the selection helpers the bind and the body share.
bool weapon_group_any_available_00545b30(const HudWeaponGroupScreenState& screen,
                                         HudWeaponGroupScreenHost& host, int group);
bool weapon_group_any_role_00545b80(const HudWeaponGroupScreenState& screen,
                                    HudWeaponGroupScreenHost& host, int role);
bool weapon_group_unit_roles_00545370(const HudWeaponGroupScreenState& screen,
                                      HudWeaponGroupScreenHost& host, std::size_t unit);
bool weapon_group_roles_held_00545e50(const HudWeaponGroupScreenState& screen,
                                      HudWeaponGroupScreenHost& host);
void weapon_group_set_roles_00545bd0(const HudWeaponGroupScreenState& screen,
                                     HudWeaponGroupScreenHost& host, bool take);
void weapon_group_hint_00548360(const HudWeaponGroupScreenState& screen,
                                HudWeaponGroupScreenHost& host);
void weapon_group_cycle_00548410(HudWeaponGroupScreenState& screen,
                                 HudWeaponGroupScreenHost& host, bool forward);
void weapon_group_select_005484b0(HudWeaponGroupScreenState& screen,
                                  HudWeaponGroupScreenHost& host, int group);

// 005484F0, __thiscall(screen 2Eh), plain RET, body 005484F0..005491F4.
// Returns false when a gate ended the call before the one-shot bytes.
bool weapon_group_screen_update_005484f0(HudWeaponGroupScreenState& screen,
                                         HudWeaponGroupScreenHost& host);

}  // namespace bsp
