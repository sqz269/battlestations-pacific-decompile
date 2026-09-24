#pragma once
// Screen 50h, the in-mission warning screen (GUI_Warning): its update
// 00683020 (vtable 00CF7184 +20h, __thiscall(screen, float dt), RET 4, body
// 00683020..006832E9). Packet cc9_screen_50h, docs/SHIP_SCREEN_UPDATE.md
// section 16. Names are descriptive hypotheses, not recovered symbols. A new
// C++ interface over the screen's fields; not ABI-compatible.
//
// Four alerts, each with a flag, a hold timer and a pulse phase:
//   0 stall (a plane, 007C6E10 on [unit+3D0h])       00682ED0 "ingame.warning_stall"
//   1 oxygen (a submarine, unit+127Ch below 0.2)     00682CB0 "ingame.warning_o2"
//   2 shallow water (a ship, byte unit+1011h)         00682D60 "ingame.warning_shallowwater"
//   3 exit zone (00681F40 near the world edge)       00682E10 "ingame.warning_exitezone"

#include <cstddef>

namespace bsp {

struct WarningScreenState {
    float dt_78{0.0f};             // +78h, the update's dt
    float hold[4]{};               // +8h..+14h, set to 1.0 when an alert's test holds
    float phase[4]{};              // +18h..+24h, the pulse phase each show advances
    bool active[4]{};              // +28h..+2Bh
    bool held_2c{false};           // +2Ch, cleared by the register 00682740
    bool sound[4]{};               // +40h..+4Ch non-null: the tracked sound 00682800 made
    std::size_t last_unit_74{0};   // +74h, the controlled unit last frame (index + 1; 0 none)
};

enum class WarningWidget : int {
    Text = 0x50,        // warning_text
    Icon1 = 0x54,       // warning_1_Icon
    Icon2 = 0x58,       // warning_2_Icon
    FirstGroup = 0x68,  // first_Group
};

struct WarningScreenHost {
    virtual ~WarningScreenHost() = default;
    // The controlled unit 00E188D8.
    virtual std::size_t controlled_unit() = 0;        // index + 1, 0 when none
    virtual bool controlled_flag_5d() = 0;            // byte unit+5Dh
    virtual bool controlled_is_kind_of(int class_id) = 0;   // vtable +5Ch
    virtual bool plane_stall_007c6e10() = 0;          // 007C6E10([unit+3D0h]) != 0
    virtual bool ship_shallow_1011() = 0;             // byte unit+1011h
    virtual float submarine_127c() = 0;               // unit+127Ch
    virtual bool submarine_below_00852860() = 0;      // 00852860(unit)
    virtual bool pose_current_c8() = 0;               // byte unit+C8h
    virtual void refresh_pose_00414db0() = 0;
    // 00681F40([00E188A8], &unit+FCh, 0.0): the position near the world edge.
    virtual bool near_world_edge_00681f40() = 0;
    // Widgets.
    virtual void set_visible(WarningWidget widget, bool visible) = 0;   // +34h
    virtual void set_alpha(WarningWidget widget, float alpha) = 0;      // +4Ch
    // The tracked sound at +40h+4i: its virtual +8(stop flag), its +Ch
    // "finished" test, and the release (InterlockedDecrement on +4h, delete).
    virtual void sound_stop(int alert, bool flag) = 0;
    virtual bool sound_finished(int alert) = 0;
    virtual void sound_release(int alert) = 0;
    // The four show routines; each advances its phase, shows first_Group,
    // sets the text, pulses the alphas and starts the alert's sound.
    virtual void show_alert(WarningScreenState& screen, int alert) = 0;
};

// 00683020.
void warning_screen_update_00683020(WarningScreenState& screen, WarningScreenHost& host,
                                    float dt);

// Screen 49h's update 0067BF00 (vtable 00CF6D68 +20h, __thiscall(screen, float
// dt), RET 4; no Ghidra function, start 0067BF00, exclusive end 0067BFCA).
// Packet cc9_screen_49h, docs/SHIP_SCREEN_UPDATE.md section 17. It writes one
// field, +8h, the unit the screen follows; nothing else. Units are index + 1,
// 0 for none.
struct FollowScreen49State {
    std::size_t unit_08{0};        // +8h
};

struct FollowScreen49Host {
    virtual ~FollowScreen49Host() = default;
    virtual bool screen_29h_applied() = 0;            // [[00E198C4]+CCh]+5h
    virtual std::size_t screen_29h_unit() = 0;        // [[00E198C4]+CCh]+4Ch
    virtual std::size_t controlled_unit() = 0;        // 00E188D8
    // 00927880(controlled): vtable +114h, then that object's +18h.
    virtual std::size_t controlled_target_00927880() = 0;
    virtual bool is_kind_of(std::size_t unit, int class_id) = 0;   // vtable +5Ch
    // The four bytes 0043F080 tests: +5Ch set, +5Dh, +60h and +5Eh clear.
    virtual bool alive_and_visible(std::size_t unit) = 0;
    // [unit+3D0h] non-null and 0043F080 on it (planes only).
    virtual bool leader_3d0_alive(std::size_t unit) = 0;
};

// 0067BF00.
void follow_screen_update_0067bf00(FollowScreen49State& screen, FollowScreen49Host& host);

// Screen 46h's update 0064D610 (vtable 00CF7978 +20h, __thiscall(screen, float
// dt), RET 4; no Ghidra function, start 0064D610, exclusive end 0064D731).
// The ship view: 0064DA40 stores its unit at +1Ch. Packet cc9_screen_46h,
// docs/SHIP_SCREEN_UPDATE.md section 18. Part 1 binds the top-level flow; the
// three callees are host records.
struct ShipViewScreen46Host {
    virtual ~ShipViewScreen46Host() = default;
    virtual bool wanted_04() = 0;                     // the screen's +4h
    virtual bool has_unit_1c() = 0;                   // +1Ch non-null
    // 0064A400(dt): screen 26h's 0051F330 and screen 2Eh's 005454B0.
    virtual void view_input_0064a400(float dt) = 0;
    // 0064B870(dt): the integrated throttle and rudder controls.
    virtual void integrated_controls_0064b870(float dt) = 0;
    virtual bool screen_2eh_present() = 0;            // [00E198C4] and its +50h
    virtual void screen_2eh_005484f0() = 0;           // on [00E198C4]+50h
    virtual bool input_pressed(int action) = 0;       // 004C43C0
    virtual bool unit_virtual_234() = 0;              // [+1Ch] vtable +234h(0)
    virtual bool unit_local_player() = 0;             // 00927F30(unit, 0)
    virtual bool unit_is_kind_of(int class_id) = 0;   // vtable +5Ch
    // 0064D6C3..0064D6EA: 00812960(unit), then message 00465080 routed by
    // 0077D600; 0064D701..0064D71A: 0064A820's message routed by 0077C2A0(2).
    // Records; reached only on input action 95h.
    virtual bool unit_00812960() = 0;
    virtual void order_route_0077d600() = 0;
    virtual void order_route_0077c2a0() = 0;
};

// 0064D610.
void ship_view_update_0064d610(ShipViewScreen46Host& host, float dt);

// Part 2: 0064B870, BSP_HudUnitOrder_UpdateIntegratedControls,
// __thiscall(screen 46h, float dt), RET 4, body 0064B870..0064BB48.
struct IntegratedControlsState {
    float thrust_24{0.0f};        // +24h
    float turn_28{0.0f};          // +28h
    bool latched_30{false};       // +30h
    float latch_34{0.0f};         // +34h
};

// The input manager fields the routine reads through 004BEC00.
struct IntegratedControlsInputs {
    float turn_1be4{0.0f};        // [input+4]+1BE4h
    float thrust_1bb4{0.0f};      // [input+4]+1BB4h
    bool query_a92050{false};     // 00A92050 on [input+4]+1B90h
    bool query_a92090{false};     // 00A92090 on [input+4]+1B90h
    bool byte_1b91{false};        // [input+4]+1B91h
};

struct IntegratedControlsHost {
    virtual ~IntegratedControlsHost() = default;
    virtual bool unit_byte_6c8() = 0;                 // [+1Ch]+6C8h
    virtual IntegratedControlsInputs inputs() = 0;
    virtual bool unit_1130_clear() = 0;               // [+1Ch]+1130h == 0
    virtual bool local_player_role(int role) = 0;     // 00927F30(unit, role)
    virtual void role_transfer_0077c470(int mask, int take) = 0;
    virtual float unit_ordered_rudder() = 0;          // unit+984h
    virtual float unit_throttle() = 0;                // unit+980h
    // 0064BA97..0064BB12: the quantised order through 00816A40
    // (bsp::issue_hud_order_fragment_0064b870).
    virtual void issue_order(float thrust, float turn) = 0;
    virtual bool game_19c4() = 0;                     // [00E188A8]+19C4h
};

void integrated_controls_0064b870(IntegratedControlsState& screen,
                                  IntegratedControlsHost& host, float dt);

}  // namespace bsp
