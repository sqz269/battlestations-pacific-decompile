#include "bsp/hud_ship_screen.hpp"

#include <cmath>

// Packet cc9_ship_screen_update, parts 1 and 2; packet cc9_ship_screen_parts34,
// parts 3 and 4. docs/SHIP_SCREEN_UPDATE.md.

namespace bsp {
namespace {

constexpr int kKindSubmarine = 8;                    // 0064A972 PUSH 8
constexpr float kStickBackLimit = -0.5f;             // 00CE69D0
constexpr float kStickFullAngle = 2.356194496154785f;   // 00E08CAC
constexpr float kStickBackAngle = -1.1780972480773926f; // 00CF5C68
constexpr double kStickBackScale = 1.1780972480773926;  // 00CF5C60
constexpr double kStickOffset = 0.5;                 // 00D7A280
constexpr double kStickRange = 1.5;                  // 00CE3D78
constexpr float kStickSnap = 0.0010000000474974513f; // 00D7A23C
constexpr double kStickEaseDivisor = 3.0;            // 00D7A2B0
constexpr float kStickEaseArg = 0.10000000149011612f;   // 00CF5C1C
constexpr double kFlashDecayRate = 4.0;              // 00D7A328
constexpr double kFlashHideBelow = 0.05000000074505806; // 00D7A270

// Part 3.
constexpr double kHlFadeUpLimit = 1.0;                 // 00D7A24C, float 1.0
constexpr float kCircleShowAbove = 0.009999999776482582f;  // 00D7A238
constexpr double kSubCircleHideFrom = 0.9900000095367432;  // 00CED5D0

// Part 4.
constexpr double kDirDeadZone = 0.05000000074505806;   // 00D7A270
constexpr double kDirDeadZoneNeg = -0.05000000074505806; // 00CF5C78
constexpr double kDirRange = 0.949999988079071;        // 00CF5C80
constexpr double kDirFloor = -1.0;                     // 00D7A250
constexpr float kDirFloorF = -1.0f;                    // 00D7A260
constexpr float kDirBiasPos = 10.0f;                   // 00CF5C10
constexpr float kDirBiasNeg = -10.0f;                  // 00CE6848
constexpr double kPiF = 3.1415927410125732;            // 00CE3D28
constexpr double kDegrees = 180.0;                     // 00CE3D20
constexpr double kDirScale = 0.2967059810956319;       // 00CF5C70
constexpr double kTwoPiF = 6.2831854820251465;         // 00CE3828
constexpr double kDirRateGain = 4.0;                   // 00D7A328
constexpr double kKnotsPerMetre = 1.9440000057220459;  // 00CF6100
constexpr double kGaugeRateGain = 4.0;                 // 00D7A328
constexpr float kGaugeRateMax = 1.0f;                  // 00D7A24C
constexpr double kGaugeDamping = 0.875;                // 00CE42E0
constexpr float kGaugeRoundHalf = 0.5f;                // 00CE3800
constexpr double kGaugeRoundUp = 1.0;                  // 00D7A210
constexpr int kGaugeDigits = 2;                        // 0043DF90: two 0043DCA0 pushes

const ShipScreenWidget kHlWidgets[4] = {ShipScreenWidget::Hl1, ShipScreenWidget::Hl2,
                                        ShipScreenWidget::Hl3, ShipScreenWidget::Hl4};

const ShipScreenWidget kFlashWidgets[4] = {ShipScreenWidget::Flash0, ShipScreenWidget::Flash1,
                                           ShipScreenWidget::Flash2, ShipScreenWidget::Flash3};

// A shown circle: the four copies at 0064F834, 0064F9FA, 0064FAB8,
// 0064FB6C and 0064FC8F.
void show_circle(ShipScreenState& screen, ShipScreenHost& host, ShipScreenWidget circle,
                 float ratio) {
    host.set_visible(circle, true);
    host.circle_progress(circle, ratio, screen.unit_changed_188);
}

// The 0.01 test the surface circles share: FCOMIP/COMISS with JBE, so a NaN
// ratio (0/0) hides the circle.
void circle_by_ratio(ShipScreenState& screen, ShipScreenHost& host, ShipScreenWidget circle,
                     float ratio) {
    if (ratio > kCircleShowAbove) {
        show_circle(screen, host, circle, ratio);
    } else {
        host.set_visible(circle, false);
    }
}

}  // namespace

void ship_screen_tail_0064f665(ShipScreenState& screen, ShipScreenHost& host, float dt);

void ship_screen_relation_icon_0064a960(ShipScreenState& screen, ShipScreenHost& host) {
    if (!screen.has_unit) return;                                   // 0064A96B
    if (!host.unit_is_kind_of(kKindSubmarine) && host.unit_in_formation()) {  // 0064A974, 0064A980
        host.set_visible(ShipScreenWidget::Relation, true);         // 0064A998
        // 0064A99A..0064A9CB: +88h(state, 0, 1.0f), state = (leader != unit).
        host.select_state(ShipScreenWidget::Relation, host.unit_leads_formation() ? 0 : 1, 0,
                          1.0f);
        return;
    }
    host.set_visible(ShipScreenWidget::Relation, false);            // 0064A9DE
}

void ship_screen_flashes_0064abd0(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    // 0064ABDD: a clear enable zeroes all four intensities first.
    if (!screen.flash_enabled_1ac) {
        for (float& v : screen.flash_intensity) v = 0.0f;
    }
    // 0064ABFA..0064AC0C: one factor, float-stored.
    const float factor = static_cast<float>(1.0 - static_cast<double>(dt) * kFlashDecayRate);
    for (float& v : screen.flash_intensity) v = v * factor;         // 0064AC10..0064AC3F
    for (int i = 0; i < 4; ++i) {
        const float v = screen.flash_intensity[i];
        if (kFlashHideBelow > static_cast<double>(v)) {             // 0064AC42 and siblings
            host.set_visible(kFlashWidgets[i], false);
            continue;
        }
        host.set_alpha(kFlashWidgets[i], v > 1.0f ? 1.0f : v);      // widget +4Ch
        host.set_visible(kFlashWidgets[i], true);
    }
}

void ship_screen_stick_0064a9f0(ShipScreenHost& host, float ease) {
    const float t = host.unit_throttle();                          // 0064A9F9
    float angle;
    if (kStickBackLimit > t) {                                      // 0064AA09 JA
        angle = kStickBackAngle;
    } else if (t > 1.0f) {                                          // 0064AA1E JA
        angle = kStickFullAngle;
    } else if (t == 0.0f) {
        angle = 0.0f;
    } else if (t == 1.0f) {
        angle = kStickFullAngle;
    } else if (t == kStickBackLimit) {
        angle = kStickBackAngle;
    } else if (t > 0.0f) {
        angle = static_cast<float>(static_cast<double>(kStickFullAngle) * t);   // 0064AA68
    } else {
        const double scaled = static_cast<double>(t) * kStickBackScale;       // 0064AA73
        angle = static_cast<float>(scaled + scaled);
    }
    if (ease != 0.0f) {                                             // 0064AA87..0064AA8E
        const float current = host.rotation(ShipScreenWidget::Stick);   // [+48h]+48h
        angle = static_cast<float>((static_cast<double>(angle) - current) * ease + current);
    }
    host.set_rotation(ShipScreenWidget::Stick, angle);              // 0064AABF, +44h
}

float ship_screen_ease_stick_0064e358(float current, float throttle) noexcept {
    // 0064E358..0064E39F: clamp((throttle + 0.5) / 1.5, 0, 1) through 00415620.
    float target = static_cast<float>((static_cast<double>(throttle) + kStickOffset) / kStickRange);
    if (target < 0.0f) target = 0.0f;
    if (target > 1.0f) target = 1.0f;
    if (current == target) return current;                           // 0064E3B8 FUCOMIP
    const float gap = std::fabs(static_cast<float>(static_cast<double>(target) - current));
    if (gap < kStickSnap) return target;                             // 0064E3E2
    return static_cast<float>((static_cast<double>(target) - current) / kStickEaseDivisor +
                              current);                               // 0064E3E4..0064E3EC
}

void ship_screen_controls_0064e415(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    constexpr int kKindShip = 6;          // 0064E42B PUSH 6
    constexpr int kActionTurn = 0x98;     // 0064E46E, 0064E482
    constexpr int kActionRepair = 0xEF;   // 0064E616, 0064F317
    bool repair_section = false;
    if (host.controlled_present() && host.controlled_is_kind_of(kKindShip) &&
        host.controlled_is_local_player()) {                          // 0064E415..0064E445
        if (!host.controlled_is_kind_of(kKindSubmarine)) {            // 0064E456
            if (host.input_pressed(kActionTurn) ||
                (host.input_held(kActionTurn) && screen.turn_timer_160 >= 0.0f)) {
                if (0.0f > screen.turn_timer_160) {                   // 0064E4B4
                    screen.turn_timer_160 = dt;                       // 0064E4D2
                    host.turn_to_camera_order();
                } else {
                    screen.turn_timer_160 = screen.turn_timer_160 + dt;   // 0064E562..0064E575
                    if (screen.turn_timer_160 > 1.0f && host.turn_timer_expired_009539e0()) {
                        screen.turn_timer_160 = -1.0f;                // 0064E598
                    }
                }
            } else if (screen.turn_timer_160 >= 0.0f) {               // 0064E5A2..0064E5B1
                screen.turn_timer_160 = -1.0f;
                host.turn_to_camera_release();
            }
        }
        // 0064E5C9..0064E5FB.
        if (host.controlled_present() && host.controlled_is_kind_of(kKindShip) &&
            host.controlled_class_repair()) {
            repair_section = true;
        }
    }
    if (repair_section && !screen.repair_mode_157) {                  // 0064E601
        if (host.input_held(kActionRepair)) {                         // 0064E61F
            // 0064E62C..0064F30C: hints, NoRepairGUI, the warning flags, the
            // analog selector and the highlight text.
            host.repair_menu_open(dt);
        } else if (host.input_released(kActionRepair) && screen.pick_pending_156) {
            host.repair_order_route();                                // 0064F311..0064F393
        }
        // 0064F39A: the warning pulse runs while any flag is set.
        if (screen.warn_flags[0] || screen.warn_flags[2] || screen.warn_flags[1] ||
            screen.warn_flags[3]) {
            host.warning_pulse(dt);
        }
    }
    if (screen.repair_mode_157) {                                     // 0064F496
        host.repair_mode_panel(dt);
    } else if (host.other_screen_gate()) {                            // 0064F633..0064F655
        host.other_screen_00545360();                                 // 0064F65A
    }
    ship_screen_tail_0064f665(screen, host, dt);
}

void ship_screen_update_0064dd30(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    if (screen.slide_pending_119) host.relation_slide_block();        // 0064DD5F
    host.pipe_sight_block(dt);                                        // 0064DE92
    ship_screen_relation_icon_0064a960(screen, host);                 // 0064E30E
    screen.flash_enabled_1ac = host.flash_view_mode_is_1();           // 0064E31A..0064E335
    ship_screen_flashes_0064abd0(screen, host, dt);                   // 0064E33B
    if (!screen.active_05 || !screen.has_unit) return;                // 0064E340..0064E352
    screen.stick_44 = ship_screen_ease_stick_0064e358(screen.stick_44, host.unit_throttle());
    ship_screen_stick_0064a9f0(host, kStickEaseArg);                  // 0064E410
    if (!host.controls_bound()) {
        host.remainder_from_0064e415(dt);
        return;
    }
    ship_screen_controls_0064e415(screen, host, dt);                  // 0064E415
}

void ship_screen_damage_panel_0064f665(ShipScreenState& screen, ShipScreenHost& host,
                                       float dt) {
    constexpr int kKindShip = 6;                                      // 0064F672 PUSH 6
    // 0064F665..0064F678: the controlled unit (00E188D8), a ship.
    if (!host.controlled_present() || !host.controlled_is_kind_of(kKindShip)) return;
    // 0064F67E..0064F6B5: Icon_5 (periscope) for a submarine, Icon_3 (engine)
    // otherwise.
    const bool submarine = host.controlled_is_kind_of(kKindSubmarine);
    host.set_visible(ShipScreenWidget::Icon5, submarine);
    host.set_visible(ShipScreenWidget::Icon3, !submarine);
    // 0064F6B7..0064F71C: +150h from the repair priority unit+A44h. Any other
    // value leaves +150h as it was.
    const ShipRepairTaskTerms task = host.controlled_repair_task();
    switch (task.priority_24) {
    case 0: screen.damage_select_150 = -1; break;
    case 4: screen.damage_select_150 = 0; break;
    case 3: screen.damage_select_150 = 1; break;
    case 5: screen.damage_select_150 = 2; break;
    case 1: screen.damage_select_150 = 2; break;
    case 2: screen.damage_select_150 = 3; break;
    default: break;
    }
    // 0064F726..0064F7C3: Hl_1..4 fade through the colour virtuals +54h and
    // +50h. The selected one grows by 2*dt while below 1.0 and is set to 1.0
    // otherwise (no clamp after the add); the others shrink by 2*dt while
    // above zero and are set to 0 otherwise.
    for (int i = 0; i < 4; ++i) {
        float rgba[4];
        host.color(kHlWidgets[i], rgba);
        const float alpha = rgba[3];                                  // [ESP+A0h]
        if (i == screen.damage_select_150) {
            if (kHlFadeUpLimit > alpha) {                             // 0064F751 COMISS
                rgba[3] = static_cast<float>(
                    (static_cast<double>(dt) + dt) + alpha);          // 0064F75B..0064F76B
            } else {
                rgba[3] = 1.0f;                                       // 0064F7A0
            }
        } else if (alpha > 0.0f) {                                    // 0064F780
            rgba[3] = static_cast<float>(
                static_cast<double>(alpha) - (static_cast<double>(dt) + dt));  // 0064F785
        } else {
            rgba[3] = 0.0f;                                           // 0064F7A0
        }
        host.set_color(kHlWidgets[i], rgba);                          // 0064F7B8
    }
    // 0064F7C9..0064F7E0: circle_3, by the submarine test again.
    if (host.controlled_is_kind_of(kKindSubmarine)) {
        // 0064F7E6..0064F82E: (settings+4C4h - unit+125Ch) / settings+4C4h,
        // shown strictly between 0.01 and 0.99.
        const float held = host.controlled_float_125c();
        const float full = host.settings_4c4();
        const float ratio = static_cast<float>(
            (static_cast<double>(full) - held) / full);
        if (ratio > kCircleShowAbove && kSubCircleHideFrom > ratio) {
            show_circle(screen, host, ShipScreenWidget::Circle3, ratio);
        } else {
            host.set_visible(ShipScreenWidget::Circle3, false);       // 0064F8C6
        }
    } else {
        // 0064F8DA..0064F9C3: the settings descriptor's EngineJam seconds, then
        // 0093A3F0 on unit+A20h over them.
        const float jam_full = host.settings_engine_jam_seconds();
        const float ratio = static_cast<float>(
            static_cast<double>(task.engine_jam_seconds) / jam_full);  // 0064F9D7 FDIV
        circle_by_ratio(screen, host, ShipScreenWidget::Circle3, ratio);
    }
    // 0064FA7E..0064FB36: circle_1, task+34h over unit+A60h.
    circle_by_ratio(screen, host, ShipScreenWidget::Circle1,
                    static_cast<float>(static_cast<double>(task.seconds_34) / task.total_40));
    // 0064FB38..0064FBEA: circle_2, task+38h over unit+A5Ch.
    circle_by_ratio(screen, host, ShipScreenWidget::Circle2,
                    static_cast<float>(static_cast<double>(task.seconds_38) / task.total_3c));
    // 0064FBEC..0064FD22: circle_4, the worst (full - current) / full over the
    // device list; the running maximum is a float compared in x87 precision.
    float worst = 0.0f;
    const int devices = host.controlled_device_count();
    for (int i = 0; i < devices; ++i) {
        const ShipDeviceTerms d = host.controlled_device(i);
        if (!d.kind_04 || d.kind_0f || !d.kind_20) continue;          // 0064FC05..0064FC30
        if (d.config_80 == 1 || !d.byte_378) continue;                // 0064FC32..0064FC47
        const double full = d.full_36c;
        const double damage = (full - d.current_370) / full;          // 0064FC49..0064FC5F
        if (damage > worst) worst = static_cast<float>(damage);       // 0064FC67..0064FC6D
    }
    if (worst > kCircleShowAbove) {                                   // 0064FC82 COMISS
        show_circle(screen, host, ShipScreenWidget::Circle4, worst);
    } else {
        host.set_visible(ShipScreenWidget::Circle4, false);           // 0064FD15
    }
}

float ship_screen_dir_0064aad0(ShipScreenHost& host) {
    const float x = static_cast<float>(-static_cast<double>(host.unit_ordered_rudder()));
    float r;
    if (static_cast<double>(x) > kDirDeadZone) {                      // 0064AAF0 FCOMI
        const float v = static_cast<float>((static_cast<double>(x) - kDirDeadZone) / kDirRange);
        r = (static_cast<double>(v) > 1.0) ? 1.0f : v;                // 0064AB00..0064AB16
    } else if (kDirDeadZoneNeg > static_cast<double>(x)) {            // 0064AB1E
        const float v = static_cast<float>((static_cast<double>(x) + kDirDeadZone) / kDirRange);
        r = (kDirFloor > static_cast<double>(v)) ? kDirFloorF : v;    // 0064AB34..0064AB44
    } else {
        r = 0.0f;                                                     // 0064AB50
    }
    float offset = 0.0f;                                              // 0064AB61
    if (r != 0.0f) {                                                  // 0064AB55..0064AB67
        const float bias = r > 0.0f ? kDirBiasPos : kDirBiasNeg;      // 0064AB69..0064AB78
        offset = static_cast<float>(static_cast<double>(bias) * kPiF / kDegrees +
                                    static_cast<double>(r) * kDirScale);   // 0064AB86..0064ABA1
    }
    const float angle = static_cast<float>(kTwoPiF - offset);         // 0064ABAC FSUBR
    host.set_rotation(ShipScreenWidget::Dir, angle);                  // 0064ABC3, +44h
    return r;                                                         // 0064ABC5 FLD [ESP]
}

float ship_screen_gauge_round_004396f0(float value) noexcept {
    const float whole = static_cast<float>(std::floor(static_cast<double>(value)));  // 00BF85B0
    const float fraction = static_cast<float>(static_cast<double>(value) - whole);   // 00439716
    if (std::fabs(fraction) > kGaugeRoundHalf) {                      // 00439731 COMISS
        return static_cast<float>(static_cast<double>(whole) + kGaugeRoundUp);
    }
    return whole;
}

namespace {
// 0043B2F0, __thiscall(gauge, float), RET 4: 0043ABA0 on each digit with the
// value's magnitude (sign bit cleared), then +10h = value and +18h = 0.
void gauge_write_0043b2f0(ShipScreenState::Gauge& gauge, int index, ShipScreenHost& host,
                          float value) {
    const float magnitude = std::fabs(value);                         // 0043B300 AND 7FFFFFFFh
    for (int digit = 0; digit < kGaugeDigits; ++digit) {
        host.gauge_digit(index, digit, magnitude);                    // 0043B33F
    }
    gauge.value_10 = value;                                           // 0043B359
    gauge.primed_18 = false;                                          // 0043B35E
}
}  // namespace

void ship_screen_gauge_0043b370(ShipScreenState::Gauge& gauge, int index, ShipScreenHost& host,
                                float value, float dt) {
    const float target = ship_screen_gauge_round_004396f0(value);     // 0043B37C
    if (!gauge.primed_18) {                                           // 0043B381
        gauge.value_10 = target;                                      // 0043B392
        gauge_write_0043b2f0(gauge, index, host, target);
        gauge.rate_14 = 0.0f;                                         // 0043B3A0
        gauge.primed_18 = true;                                       // 0043B3A5
        return;
    }
    const float error = static_cast<float>(static_cast<double>(target) - gauge.value_10);
    const float gap = static_cast<float>(static_cast<double>(error) - gauge.rate_14);
    float gain = static_cast<float>(static_cast<double>(dt) * kGaugeRateGain);   // 0043B3C0
    if (0.0f > gain) {
        gain = 0.0f;                                                  // 0043B3DA
    } else if (gain > kGaugeRateMax) {
        gain = kGaugeRateMax;                                         // 0043B440
    }
    float rate = static_cast<float>(static_cast<double>(gain) * gap + gauge.rate_14);
    rate = static_cast<float>(static_cast<double>(rate) * kGaugeDamping);         // 0043B3FB
    gauge.rate_14 = rate;                                             // 0043B409
    const float next = static_cast<float>(static_cast<double>(gain) * rate + gauge.value_10);
    gauge.value_10 = next;                                            // 0043B419
    gauge_write_0043b2f0(gauge, index, host, next);
    gauge.primed_18 = true;                                           // 0043B424
}

void ship_screen_gauges_0064fd24(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    // 0064FD24..0064FD50: 00939F70 and 00939F80 on +184h's task; both results
    // are popped (FSTP ST0), so the two getters have no effect.
    float r = 0.0f;                                                   // 0064FD68
    if (screen.has_unit) r = ship_screen_dir_0064aad0(host);          // 0064FD5D
    // 0064FD71..0064FDB5: FUCOMIP; an unordered compare takes the update arm.
    if (screen.dir_target_128 != r) {
        const float old = screen.dir_target_128;
        screen.dir_target_128 = r;                                    // 0064FD9B
        screen.dir_rate_12c = static_cast<float>(
            (static_cast<double>(old) - r) * kDirRateGain + screen.dir_rate_12c);
    }
    if (screen.dir_rate_12c != 0.0f) host.dir_clock_step(screen);     // 0064FDBB..0064FDCE
    if (!screen.has_unit) return;                                     // 0064FFAB
    // 0064FFB1..0064FFDE: the speed gauge +78h.
    const float knots = static_cast<float>(
        static_cast<double>(host.unit_forward_speed()) * kKnotsPerMetre);
    ship_screen_gauge_0043b370(screen.gauges[0], 0, host, knots, dt);
    // 0064FFE3..00650088: the gauge +7Ch from one of three sources.
    bool second = true;
    float second_value = 0.0f;
    if (screen.gauge_recon_105) {
        second_value = static_cast<float>(host.unit_int_638());       // 0064FFF9 CVTSI2SS
    } else if (screen.gauge_class_106) {
        // 00650019..0065002C: UCOMISS against 0.0f; only "equal" reads zero.
        const float gate = host.unit_float_1124();
        second_value = gate == 0.0f ? 0.0f : static_cast<float>(host.unit_class_int_790());
    } else if (screen.gauge_sub_104) {
        second_value = static_cast<float>(host.unit_device_count_00852300());   // 00650076
    } else {
        second = false;
    }
    if (second) ship_screen_gauge_0043b370(screen.gauges[1], 1, host, second_value, dt);
    // 0065008D..006500BC: the torpedo gauge +80h.
    if (screen.gauge_torpedo_107) {
        const float stock = static_cast<float>(host.unit_torpedo_stock_00815850());
        ship_screen_gauge_0043b370(screen.gauges[2], 2, host, stock, dt);
    }
}

void ship_screen_tail_0064f665(ShipScreenState& screen, ShipScreenHost& host, float dt) {
    if (!host.damage_bound()) {
        host.remainder_from_0064f665(dt);
        return;
    }
    ship_screen_damage_panel_0064f665(screen, host, dt);
    if (host.gauges_bound()) {
        ship_screen_gauges_0064fd24(screen, host, dt);
    } else {
        host.remainder_from_0064fd24(dt);
    }
    screen.unit_changed_188 = false;                                  // 006500C1
}

}  // namespace bsp
