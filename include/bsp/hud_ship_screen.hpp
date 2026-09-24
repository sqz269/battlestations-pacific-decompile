#pragma once
// Screen 45h, the ship status and repair-crew HUD: its update 0064DD30
// (vtable 00CF5E30 +20h). Packet cc9_ship_screen_update,
// docs/SHIP_SCREEN_UPDATE.md. Names are descriptive hypotheses, not recovered
// symbols. New C++ interfaces over the screen's fields; not ABI-compatible.
//
// Part 1 covers the top-level gates and three block groups:
//   0064E30C 0064A960   the relation icon (+F8h)
//   0064E313 0064ABD0   the four flash icons at +18Ch
//   0064E340..0064E410  the throttle stick: +44h and 0064A9F0 on +48h
// Every other block of 0064DD30 is a host record in part 1. Part 2 binds the
// controls (0064E415..0064F665), part 3 the damage panel (0064F665..0064FD24)
// and part 4 the direction spring and digit gauges (0064FD24..006500C1).

#include <cstddef>
#include <cstdint>

namespace bsp {

// The screen's own fields part 1 reads and writes.
struct ShipScreenState {
    bool active_05{false};        // +5h, the registry's applied byte
    bool has_unit{false};         // +184h non-null
    bool slide_pending_119{false};// +119h, cleared by the enter (0064BE70); set only by
                                  // 0064A902, which part 1 does not reach
    float stick_44{-1.0f};        // +44h, the eased throttle; the enter stores -1.0 (0064BD7C)
    // +18Ch..+1ACh, the flash group 0064ABD0 walks.
    float flash_intensity[4]{};   // +19Ch..+1A8h
    bool flash_enabled_1ac{false};// +1ACh
    float turn_timer_160{-1.0f};  // +160h, the enter stores -1.0 (0064BE44)
    bool pick_pending_156{false}; // +156h
    bool repair_mode_157{false};  // +157h
    bool warn_flags[4]{};         // +BCh..+BFh, written only inside the repair menu
    // Part 3 (0064F665..0064FD24).
    int damage_select_150{-1};    // +150h, the enter stores -1 (0064BB90, param_1[54h])
    bool unit_changed_188{false}; // +188h: 0064D590 sets it when +184h changes; the
                                  // update clears it at 006500C1
    // Part 4 (0064FD24..006500C1). The enter zeroes +124h..+12Ch.
    float dir_124{0.0f};          // +124h, integrated by the clock-gated step
    float dir_target_128{0.0f};   // +128h, 0064AAD0's last answer
    float dir_rate_12c{0.0f};     // +12Ch
    // +104h..+107h: 0064AE20 sets them from the unit's class on apply; the
    // enter clears them (0064BB90).
    bool gauge_sub_104{false};    // submarine with 00852350 != 0
    bool gauge_recon_105{false};  // [[unit+538h]+C8h] > 0
    bool gauge_class_106{false};  // !+105h and 0059CB90() == 16h
    bool gauge_torpedo_107{false};// 00818300(unit) != 0
    // The three 1Ch-byte digit gauges 0064B370 builds with 0043DF90, each over
    // two digit icons: +78h speed, +7Ch recon or class count, +80h torpedoes.
    struct Gauge {
        float value_10{0.0f};
        float rate_14{0.0f};
        bool primed_18{false};    // 0043DF90 and 0064AE20 clear it
    };
    Gauge gauges[3]{};
};

// The widgets part 1 drives, by the screen offsets 0064C0F0 fills.
enum class ShipScreenWidget : int {
    Stick = 0x48,          // ship_stick_Icon
    Relation = 0xF8,       // ship_relation_Icon
    Flash0 = 0x18C,        // VillanasFelso..Jobb, in 0064C0F0's order
    Flash1 = 0x190,
    Flash2 = 0x194,
    Flash3 = 0x198,
    // Part 3: GUI_ship_damage (0064C0F0's fourth page).
    Icon3 = 0xC4,          // Icon_3_Icon, engine
    Icon5 = 0xD4,          // Icon_5_Icon, periscope
    Hl1 = 0xD8,            // Hl_1_Icon..Hl_4_Icon
    Hl2 = 0xDC,
    Hl3 = 0xE0,
    Hl4 = 0xE4,
    Circle1 = 0xE8,        // circle_1_Section..circle_4_Section
    Circle2 = 0xEC,
    Circle3 = 0xF0,
    Circle4 = 0xF4,
    // Part 4.
    Dir = 0x50,            // ship_dir_Icon
};

// The repair task embedded at unit+A20h (docs/UNIT_FIRE_AND_REPAIR.md), as
// the damage panel reads it on the controlled unit.
struct ShipRepairTaskTerms {
    int priority_24{0};           // unit+A44h, the task's +24h
    float seconds_34{0.0f};       // 00939F80, task+34h
    float seconds_38{0.0f};       // 00939F70, task+38h
    float total_3c{0.0f};         // unit+A5Ch, the divisor for +38h
    float total_40{0.0f};         // unit+A60h, the divisor for +34h
    float engine_jam_seconds{0.0f}; // 0093A3F0: the last "EngineJam" record of
                                    // the active-failure vector +18h..+1Ch
};

// One device of the controlled unit's list (+48h, next +44h), as the worst-
// device loop 0064FC05..0064FC7A reads it.
struct ShipDeviceTerms {
    bool kind_04{false};          // vtable +5Ch(4)
    bool kind_0f{false};          // vtable +5Ch(0Fh)
    bool kind_20{false};          // vtable +5Ch(20h)
    int config_80{0};             // [dev+3F4h]+80h
    bool byte_378{false};         // dev+378h
    float full_36c{0.0f};         // dev+36Ch
    float current_370{0.0f};      // dev+370h
};

struct ShipScreenHost {
    virtual ~ShipScreenHost() = default;
    // Widget virtuals.
    virtual void set_visible(ShipScreenWidget widget, bool visible) = 0;      // +34h
    virtual void set_rotation(ShipScreenWidget widget, float radians) = 0;   // +44h
    virtual float rotation(ShipScreenWidget widget) = 0;                     // widget+48h
    virtual void set_alpha(ShipScreenWidget widget, float alpha) = 0;        // +4Ch
    virtual void select_state(ShipScreenWidget widget, int state, int zero,
                              float one) = 0;                                // +88h
    // The unit at +184h.
    virtual bool unit_is_kind_of(int class_id) = 0;   // vtable +5Ch
    virtual bool unit_in_formation() = 0;             // [unit+284h] != 0
    virtual bool unit_leads_formation() = 0;          // 007788D0(unit) == unit
    virtual float unit_throttle() = 0;                // unit+980h
    // [[00E198C4]+4Ch]+30h == 1, the flash group's enable.
    virtual bool flash_view_mode_is_1() = 0;
    // The blocks part 1 does not bind, in listing order.
    virtual void relation_slide_block() = 0;          // 0064DD5F..0064DE90
    virtual void pipe_sight_block(float dt) = 0;      // 0064DE92..0064E307
    // Part 2: the controlled unit (00E188D8) and the input records.
    virtual bool controlled_present() = 0;
    virtual bool controlled_is_kind_of(int class_id) = 0;      // vtable +5Ch
    virtual bool controlled_is_local_player() = 0;             // 00927F30(unit, 0)
    virtual bool controlled_class_repair() = 0;                // [unit+538h]+D0h, "Repair"
    virtual bool input_pressed(int action) = 0;                // 004C43C0
    virtual bool input_held(int action) = 0;                   // 004C5090
    virtual bool input_released(int action) = 0;               // 00535EE0
    // Blocks that write gameplay state or open the repair menu. None is
    // reached without input; each is a record and performs nothing.
    virtual void turn_to_camera_order() = 0;           // 0064E4C3..0064E560
    virtual void turn_to_camera_release() = 0;         // 0064E5B3..0064E5C3, unit+630h
    virtual bool turn_timer_expired_009539e0() = 0;    // 0064E587
    virtual void repair_menu_open(float dt) = 0;       // 0064E62C..0064F30C
    virtual void repair_order_route() = 0;             // 0064F32E..0064F393
    virtual void warning_pulse(float dt) = 0;          // 0064F3C2..0064F48E
    virtual void repair_mode_panel(float dt) = 0;      // 0064F4A3..0064F62E
    // 0064F633..0064F65A: the screen at [00E198C4]+50h.
    virtual bool other_screen_gate() = 0;              // !+156h and [[..]+64h]+81h/+82h clear
    virtual void other_screen_00545360() = 0;
    virtual void remainder_from_0064f665(float dt) = 0; // 0064F665..006500C1
    // Part 3, the damage panel. The host's switch; when false the tail from
    // 0064F665 is the record above.
    virtual bool damage_bound() = 0;
    virtual void color(ShipScreenWidget widget, float out[4]) = 0;          // +54h
    virtual void set_color(ShipScreenWidget widget, const float rgba[4]) = 0; // +50h
    virtual ShipRepairTaskTerms controlled_repair_task() = 0;  // unit+A20h
    // GameSettings+3E4h: the failure descriptors (14h stride, name +8h,
    // seconds +Ch); the seconds of the last one named "EngineJam", else 0.
    virtual float settings_engine_jam_seconds() = 0;
    virtual float settings_4c4() = 0;                          // GameSettings+4C4h
    virtual float controlled_float_125c() = 0;                 // unit+125Ch
    virtual int controlled_device_count() = 0;                 // unit+48h list
    virtual ShipDeviceTerms controlled_device(int index) = 0;
    // A shown circle's progress: 00ABE6E0(ratio, 0, 0, 1) when +188h is set,
    // else 00AA8B00(widget, 2) with entry+0Ch = clamp(ratio, 0, 1), +10h = 1.
    virtual void circle_progress(ShipScreenWidget widget, float ratio, bool snap) = 0;
    // Part 4, the direction spring and the digit gauges. When false the tail
    // from 0064FD24 is one record.
    virtual bool gauges_bound() = 0;
    virtual void remainder_from_0064fd24(float dt) = 0;
    virtual float unit_ordered_rudder() = 0;                   // +184h unit+984h
    // 0064FDD4..0064FF9D: the step gated on the platform clock (01090AB0) and
    // the static 00E19808; it integrates +124h and decays +12Ch.
    virtual void dir_clock_step(ShipScreenState& screen) = 0;
    virtual float unit_forward_speed() = 0;                    // 0092D730([unit+1018h])
    virtual int unit_int_638() = 0;                            // +105h source
    virtual float unit_float_1124() = 0;                       // +106h gate
    virtual int unit_class_int_790() = 0;                      // [[unit+538h]+790h]
    virtual int unit_device_count_00852300() = 0;              // +104h source
    virtual int unit_torpedo_stock_00815850() = 0;             // +107h source
    // 0043ABA0(digit index, digit widget, |value|): the digit's texture roll.
    virtual void gauge_digit(int gauge, int digit, float magnitude) = 0;
    // The host's part-2 switch; when false the whole tail from 0064E415 is
    // one record, as in part 1.
    virtual bool controls_bound() = 0;
    virtual void remainder_from_0064e415(float dt) = 0;
};

// 0064A960, __thiscall(screen), RET.
void ship_screen_relation_icon_0064a960(ShipScreenState& screen, ShipScreenHost& host);
// 0064ABD0, __thiscall(flash group = screen+18Ch, float dt), RET 4.
void ship_screen_flashes_0064abd0(ShipScreenState& screen, ShipScreenHost& host, float dt);
// 0064A9F0, __thiscall(screen, float ease), RET 4.
void ship_screen_stick_0064a9f0(ShipScreenHost& host, float ease);
// 0064E358..0064E402: the eased throttle at +44h.
float ship_screen_ease_stick_0064e358(float current, float throttle) noexcept;
// 0064E415..0064F665, part 2.
void ship_screen_controls_0064e415(ShipScreenState& screen, ShipScreenHost& host, float dt);
// 0064F665..0064FD24, part 3.
void ship_screen_damage_panel_0064f665(ShipScreenState& screen, ShipScreenHost& host, float dt);
// 0064AAD0, float __thiscall(screen), RET: sets ship_dir_Icon's rotation and
// returns the normalised rudder (the float at [ESP] on return, not the angle).
float ship_screen_dir_0064aad0(ShipScreenHost& host);
// 004396F0, float __thiscall(?, float), RET 4: floor, plus 1 when the
// fraction's magnitude exceeds 0.5.
float ship_screen_gauge_round_004396f0(float value) noexcept;
// 0043B370, __thiscall(gauge, float value, float dt), RET 8, with 0043B2F0.
void ship_screen_gauge_0043b370(ShipScreenState::Gauge& gauge, int index,
                                ShipScreenHost& host, float value, float dt);
// 0064FD24..006500C1, part 4.
void ship_screen_gauges_0064fd24(ShipScreenState& screen, ShipScreenHost& host, float dt);
// 0064DD30, part 1.
void ship_screen_update_0064dd30(ShipScreenState& screen, ShipScreenHost& host, float dt);

} // namespace bsp
