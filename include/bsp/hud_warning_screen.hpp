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
#include <cstdint>

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

// Part 3: 0064A400 (__thiscall(screen 46h, float dt), RET 4, body
// 0064A400..0064A449): screen 26h's 0051F330, then with 46h's mover (+20h)
// screen 2Eh's 005454B0. Screen 26h is the binoculars screen
// (BSP_HudBinocularsScreen_Register 0051ED60); its +40h is the same
// ShipCaptain mover, handed over by 0051E730 from 0064DA40.
struct BinocularsState {
    bool view_24{false};          // +24h, set only by action 75h (0051F316)
    int mode_30{0};               // +30h, 1 = binoculars up; the register stores 0
    bool flag_34{false};          // +34h
    bool flag_35{false};          // +35h
    float zoom_38{1.0f};          // +38h, the register stores 1.0 (00D7A24C)
};

// The input manager's view axes and GlobalConfig+4, as 0051EF00/0051F050
// read them.
struct BinocularsViewTerms {
    float yaw_axis_1584{0.0f};    // [input+4]+1584h
    float pitch_axis_15b4{0.0f};  // [input+4]+15B4h
    float raise_axis_15e4{0.0f};  // [input+4]+15E4h
    float config_04{0.0f};        // [00432650()]+4h
};

struct ShipViewInputHost {
    virtual ~ShipViewInputHost() = default;
    virtual BinocularsViewTerms view_terms() = 0;
    virtual bool input_pressed(int action) = 0;       // 004C43C0
    // 0051EF6B..0051EFA6: the raise toggle 0051E7E0 behind the controlled
    // unit's kind tests. Reached only on input.
    virtual void raise_toggle_0051e7e0(BinocularsState& screen) = 0;
    // 0051EFB7..0051F011: the raised arm (0051EAA0, 0051E6E0, 00452BD0).
    virtual void raised_view(BinocularsState& screen, float dt) = 0;
    virtual void set_model_visible(bool visible) = 0; // [+20h] vtable +34h
    // 00452B80(0) on 00E081A0: byte = 0, 00B0D020 on [00F8D39C] with
    // (0, 0, 0, 0, 0, 1.0), +4h = [00CE3C68].
    virtual void lens_effect_off_00452b80() = 0;
    // The mover at 26h+40h (0 when none): its yaw +384h, pitch +388h and the
    // pitch limits +3ECh/+3F0h.
    virtual bool mover_present() = 0;
    virtual float mover_yaw() = 0;
    virtual void set_mover_yaw(float yaw) = 0;
    virtual float mover_pitch() = 0;
    virtual void set_mover_pitch(float pitch) = 0;
    virtual float mover_min_pitch() = 0;
    virtual float mover_max_pitch() = 0;
    // 0051F1C0..0051F2FA: the view arm with +24h set. Reached only after
    // action 75h.
    virtual void view_arm(BinocularsState& screen, float dt) = 0;
    // 0064A41F..0064A441: screen 2Eh's 005454B0(0.5, 0.5, 26h+38h), which
    // stores 2Eh+48h..+50h and, with game+19C4h clear, screen 4Dh's +44h/+48h
    // through 00637620.
    virtual void screen_2eh_005454b0(float a, float b, float c) = 0;
    virtual bool ship_view_mover_20() = 0;            // 46h+20h non-null
};

// 0064A400 with 0051F330 = 0051EF00 then 0051F050's pose part.
void ship_view_input_0064a400(BinocularsState& binoculars, ShipViewInputHost& host,
                              float dt);

// Screen 29h, the unit pick (registry slot 41, vtable 00CECCF8, enter
// 00521670): its update 00527260 (vtable +20h, __thiscall(screen, float dt),
// RET 4; no Ghidra function starts there, Ghidra folds it after 00526A40;
// start 00527260 with INT3 before it, exclusive end 00527BE0, where
// FUN_00527BE0 follows the RET 4 at 00527BDD) and its worker 00526A40
// (__thiscall(screen, unit* exclude, bool* by_ray, float zoom), RET 0Ch,
// body 00526A40..00527256). Packet cc9_screen_29h,
// docs/SHIP_SCREEN_UPDATE.md section 24. Units are index + 1, 0 for none.
// A new C++ interface over the screen's fields; not ABI-compatible.
struct UnitPickScreenState {
    std::size_t exclude_hit_50{0}; // +50h, the ray hit of kind 1Eh (0 otherwise)
    bool section_named_54{false};  // +54h points at a section name, not 00CE43EC ""
    std::size_t pick_4c{0};        // +4Ch, the unit 00526A40 picked
    std::size_t payload_b0{0};     // +B0h, the pick, or its vtable +140h owner
    bool by_ray_b4{false};         // +B4h, set when the pick came from the ray
    int countdown_b8{0};           // +B8h
    bool sound_d8{false};          // +D8h, the tail's sound request
    float timer_ec{0.0f};          // +ECh, counts down by dt, blocks the lock logic
};

// The two unit lists 00526A40 walks: the local team's live units
// (game+1974h, 004C3CB0's first walk over team record +DDCh) and its
// kind-35h and grey-arrow units (game+19BCh, the second walk's last list).
enum class UnitPickList : int { TeamUnits = 0, Kind35 = 1 };

struct UnitPickHost {
    virtual ~UnitPickHost() = default;
    // --- 00526A40 ---
    virtual float lock_zoom_modifier_5c() = 0;          // [00432650()]+5Ch
    // 00419010 BSP_Math_InterpolateClamped(1, 1, 0, 1/modifier, zoom).
    virtual float interpolate_clamped_00419010(float inv_modifier, float zoom) = 0;
    virtual bool game_19c4() = 0;                       // [00E188A8]+19C4h
    virtual std::size_t spectated_unit_005a1310() = 0;  // [[00E198C4]+54h] 005A1310(0)
    virtual std::size_t firing_unit_004b4b00() = 0;     // the controlled unit or its +3D0h
    virtual bool is_kind_of(std::size_t unit, int class_id) = 0; // vtable +5Ch
    virtual int unit_slot_1b4(std::size_t unit) = 0;    // unit+1B4h
    virtual bool slot_auto_engage_00927f10(int slot) = 0;
    // game+19FCh's camera node after 00B6DB70: position +120h..+128h and
    // forward +110h..+118h.
    virtual void camera_basis(float position[3], float forward[3]) = 0;
    // 009043A0 on [game+19CCh]: the segment query; true with the hit unit.
    virtual bool ray_pick_009043a0(const float from[3], const float to[3],
                                   std::size_t ignore, std::size_t& hit) = 0;
    // The hit branch 00526C74..00526DB2 (section name, geometry, kind 1Eh),
    // reached only when the ray hits a unit and the firing unit is not a
    // plane. It starts from the hit as the pick; returns the pick or 0.
    virtual std::size_t ray_hit_branch(UnitPickScreenState& screen, std::size_t hit) = 0;
    virtual bool game_1fe4() = 0;                       // [00E188A8]+1FE4h non-zero
    virtual int game_difficulty_6ac() = 0;              // [00E188A8]+6ACh
    virtual float lock_radius_multiplier(int index) = 0; // [00432650()]+40h[index]
    virtual std::size_t list_size(UnitPickList list) = 0;
    virtual std::size_t list_unit(UnitPickList list, std::size_t i) = 0; // node+8h, may be 0
    virtual int member_count_3cc(std::size_t squadron) = 0;
    virtual std::size_t member_3d0(std::size_t squadron, int i) = 0;
    virtual bool grey_arrow_contains_008ddf90(std::size_t unit) = 0; // game+21A4h[slot]
    virtual bool flag_5d(std::size_t unit) = 0;
    // unit+FCh..+104h after the 00414DB0 refresh when +C8h is clear.
    virtual void unit_position(std::size_t unit, float out[3]) = 0;
    // The GunBot arm 00526EE8..00526F2C (a plane firing unit): 00901C20.
    virtual void intercept_point_00901c20(std::size_t firing, std::size_t unit,
                                          float out[3]) = 0;
    // 0043A660(point, out, 1, 1) on the camera: the clip mask and screen xy.
    virtual unsigned project_0043a660(const float point[3], float& x, float& y) = 0;
    // The kind-44h filter at 005271CB (0052720C 005220C0), after a ray hit.
    virtual bool ray_hit_filter_005220c0(std::size_t pick) = 0;
    // The four bytes 0043F080 tests: +5Ch set, +5Dh, +60h and +5Eh clear.
    virtual bool alive_and_visible(std::size_t unit) = 0;

    // --- 00527260 ---
    virtual float binoculars_zoom() = 0;          // [00E198C4]+4Ch (26h) +38h, else 1.0
    virtual std::size_t controlled_unit() = 0;    // 00E188D8
    virtual std::size_t owner_140(std::size_t unit) = 0; // vtable +140h
    virtual bool team_record_19() = 0;            // [game+18CCh+[game+18ECh]*4]+19h
    virtual bool byte_e0e350() = 0;               // [00E0E350]
    virtual int interface_id() = 0;               // [00E198C4]+4h
    // Byte +0Bh or +10h of [[input+4]+30h*action+2Ch], read directly.
    virtual bool action_byte(int action, int byte_offset) = 0;
    virtual bool input_pressed(int action) = 0;   // 004C43C0
    virtual void reset_c0_00525170() = 0;
    // The lock branches, each reached only after an input flag; the address
    // is the branch's first instruction. True when the branch ran to 00527B74
    // (it stores +D8h itself), false when its own gates sent it on.
    virtual bool lock_branch(UnitPickScreenState& screen, std::uint32_t address) = 0;
    virtual void tail_sound_00a7e490() = 0;       // 00527B86..00527BCB
};

// 00526A40. Returns the pick (index + 1) or 0.
std::size_t unit_pick_00526a40(UnitPickScreenState& screen, UnitPickHost& host,
                               std::size_t exclude, float zoom);

// 00527260.
void unit_pick_screen_update_00527260(UnitPickScreenState& screen, UnitPickHost& host,
                                      float dt);


}  // namespace bsp
