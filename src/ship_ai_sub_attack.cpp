// The ship brain's `sub_attack` state (packet cc9_submarine_ai_states).
// docs/SHIP_AI_SUB_ATTACK.md carries the listing evidence; addresses below are
// the native instructions each line reproduces. Every float store the image
// makes through FSTP/MOVSS is rounded to float here with f().

#include "bsp/ship_ai_sub_attack.hpp"

#include "bsp/geometry_helpers.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_rudder.hpp"

#include <cmath>

namespace bsp {
namespace {

inline float f(double value) noexcept { return static_cast<float>(value); }

constexpr float kPi = 3.1415927410125732f;          // 00D7A264
constexpr float kRearSwapLimit = 2.356194496154785f;  // 00D20E80, 3pi/4
constexpr float kForeSwapLimit = 2.094395160675049f;  // 00D2017C, 2pi/3
constexpr float kAftSwapLimit = 1.0471975803375244f;  // 00D05AAC, pi/3
constexpr float kTurnSlowStart = 0.4363323152065277f; // 00CEB664, 25 degrees
constexpr float kTurnSlowEnd = 1.0471975803375244f;   // 00D05AAC, 60 degrees

// 009E9F9E..009EA0B7 and 009EA0F2..009EA198: the target's bearing off `heading`
// through 00427C90's float difference and 00414EB0. False when the planar
// distance squared is not above 1 (009EA077 / 009EA157 JBE).
bool relative_bearing(ShipAiSubAttackHost& host, float heading, float& relative) {
    const std::array<float, 3> self = host.self_position();
    const std::array<float, 3> target = host.target_position();
    const float dx = f(static_cast<double>(target[0]) - self[0]);
    const float dz = f(static_cast<double>(target[2]) - self[2]);
    const float d2 = f(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    if (!(d2 > 1.0f)) return false;
    const float bearing = heading_angle_00414eb0({dx, dz});
    relative = wrapped_angle_subtract_00438b10(heading, bearing);
    return true;
}

// 009E9D90 / 009EA9C0 share this body: range, then 009E9CE0.
void range_and_torpedo(ShipAiSubAttackTubeCache& cache, ShipAiSubAttackHost& host,
                       float& range, float& torpedo) {
    range = ship_ai_sub_attack_range_009e4a60(host);           // FSTP float at 009E9DA7
    torpedo = ship_ai_sub_attack_torpedo_range_009e9ce0(cache, host);
}

}  // namespace

ShipAiAttackArm ship_ai_attack_arm_009f3d73(bool unit_is_submarine,
                                            bool kamikaze_class_00779aa0) noexcept {
    if (!unit_is_submarine) return ShipAiAttackArm::AttackMove;          // 009F3D7B
    return kamikaze_class_00779aa0 ? ShipAiAttackArm::Kamikaze           // 009F3D86
                                   : ShipAiAttackArm::SubAttack;         // 009F3D8E
}

float ship_ai_sub_attack_range_009e4a60(ShipAiSubAttackHost& host) {
    if (!host.target_present_0b20()) return 0.0f;                       // 009E4B81 FLDZ
    const std::array<float, 3> self = host.self_position();
    const std::array<float, 3> target = host.target_position();
    const float dx = f(static_cast<double>(target[0]) - self[0]);       // 009E4AAB
    const float dz = f(static_cast<double>(target[2]) - self[2]);       // 009E4ABC
    const float d2 = f(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    float distance = 0.0f;                                              // 009E4AF7..4AFC
    if (static_cast<double>(d2) > kSubAttackRangeEpsilonSq) {
        distance = f(std::sqrt(static_cast<double>(d2)));               // 009E4AE4 CRT sqrt
    }
    // 009E4B05..009E4B26: distance - selfLength * 0.5 is computed and stored to
    // [ESP+0Ch], a slot nothing reads afterwards; both returns load [ESP+8],
    // which holds the plain distance unless the target is a ship. So the own
    // hull is NOT subtracted. Reproduced as the listing has it.
    float result = distance;
    if (host.target_is_kind(6)) {                                       // 009E4B2A
        result = f(static_cast<double>(distance)
                   - static_cast<double>(host.target_class_length_00a0()) * kSubAttackHalf);
    }
    if (0.0f > result) return 0.0f;                                     // 009E4B52 JBE
    return result;
}

float ship_ai_sub_attack_torpedo_range_009e9ce0(ShipAiSubAttackTubeCache& cache,
                                                ShipAiSubAttackHost& host) {
    // 009E9CE4..009E9D16: the first tube is looked up only while +1Ch is null
    // and then held through the reference node at +8h. The host's tube lists
    // never change once built (009E97B0 builds once), so the cache is a flag.
    float target_length = kSubAttackDefaultTargetLength;                // 009E9D37
    if (host.target_present_0b20() && host.target_is_kind(6)) {         // 009E9D45..9D54
        target_length = host.target_class_length_00a0();                // 009E9D5C
    }
    float range = 0.0f;
    if (!host.torpedo_bot_range_00901ba0(target_length, range)) {
        return kSubAttackNoTubeRange;                                   // 009E9D80
    }
    cache.cached = true;
    return range;
}

bool ship_ai_sub_attack_approach_to_fire_009e9d90(ShipAiSubAttackTubeCache& approach_tube,
                                                 ShipAiSubAttackHost& host) {
    if (!host.target_present_0b20()) return false;                      // 009E9DA0
    float range = 0.0f;
    float torpedo = 0.0f;
    range_and_torpedo(approach_tube, host, range, torpedo);
    // 009E9DBA..009E9DCA: FADD double 150, then FCOMIP (R+150) against range, JBE.
    return static_cast<double>(torpedo) + kSubAttackEnterFireMargin > static_cast<double>(range);
}

bool ship_ai_sub_attack_fire_to_approach_009ea9c0(ShipAiSubAttackTubeCache& fire_tube,
                                                 ShipAiSubAttackHost& host) {
    if (!host.target_present_0b20()) return true;                       // 009EA9D0 -> 9EA9D2
    float range = 0.0f;
    float torpedo = 0.0f;
    range_and_torpedo(fire_tube, host, range, torpedo);
    // 009EA9F1..009EA9FF: FADD double 400; range > R+400 takes the JA.
    return static_cast<double>(range) > static_cast<double>(torpedo) + kSubAttackLeaveFireMargin;
}

void ship_ai_sub_attack_approach_step_009e4b90(ShipAiSubAttackApproach& approach,
                                               ShipAiSubAttackHost& host) {
    host.set_depth_level_008528b0(2);                                   // 009E4BA5
    if (!host.target_present_0b20()) {                                  // 009E4BB3
        host.hold_heading_and_stop_009e00a0();                          // 009E4C10
        return;
    }
    approach.range_20 = ship_ai_sub_attack_range_009e4a60(host);        // 009E4BBC
    const std::array<float, 3> target = host.target_position();         // 009E4BD7..4BFB
    host.set_navigation_goal_009de050(target[0], target[2]);            // 009E4C01 (xz, 0, 1)
}

void ship_ai_sub_attack_fire_refresh_009e9de0(ShipAiSubAttackFire& fire,
                                              ShipAiSubAttackHost& host) {
    fire.fore_ready_4c = host.tubes_ready(true, fire.horizon_5c);       // 009E9DF6
    fire.aft_ready_50 = host.tubes_ready(false, fire.horizon_5c);       // 009E9E10
    fire.fore_reload_28 = host.tubes_min_reload(true);                  // 009E9E1B
    fire.aft_reload_2c = host.tubes_min_reload(false);                  // 009E9E26
    const int total = fire.fore_ready_4c + fire.aft_ready_50;
    const int previous = fire.ready_total_20;
    fire.ready_total_20 = total;                                        // 009E9E39
    if (previous > total) fire.since_shot_24 = 0.0f;                    // 009E9E3C JLE
}

void ship_ai_sub_attack_fire_new_band_009e9e50(ShipAiSubAttackFire& fire,
                                               ShipAiSubAttackHost& host) {
    const float outer = host.uniform_00bd2f10(kSubAttackOuterLow, 1.0f);
    const float torpedo = ship_ai_sub_attack_torpedo_range_009e9ce0(fire.tube, host);
    fire.outer_58 = f(static_cast<double>(torpedo) * outer);
    const float inner = host.uniform_00bd2f10(kSubAttackInnerLow, kSubAttackInnerHigh);
    fire.inner_54 = f(static_cast<double>(inner) * fire.outer_58);
}

void ship_ai_sub_attack_fire_enter_009eabe0(ShipAiSubAttackFire& fire,
                                            ShipAiSubAttackHost& host) {
    fire.horizon_5c = kSubAttackReadyHorizon;                           // 009EABFC
    const int fore = host.tubes_ready(true, kSubAttackReadyHorizon);    // 009EAC01
    const int aft = host.tubes_ready(false, kSubAttackReadyHorizon);    // 009EAC14
    fire.ready_total_20 = fore + aft;                                   // 009EAC1D
    ship_ai_sub_attack_fire_refresh_009e9de0(fire, host);               // 009EAC20
    fire.since_shot_24 = kSubAttackIdleSeed;                            // 009EAC4A
    fire.swap_period_60 = host.uniform_00bd2f10(kSubAttackSwapLow, kSubAttackSwapHigh);
    fire.band_timer_3c = kSubAttackBandTimerSeed;                       // 009EAC67
    fire.fore_30 = true;                                                // 009EAC70
    fire.closing_38 = true;                                             // 009EAC73
    fire.left_close_48 = true;                                          // 009EAC76
    fire.close_40 = 0.0f;                                               // 009EAC79
    fire.close_time_44 = 0.0f;                                          // 009EAC7E
    fire.reverse_64 = -1.0f;                                            // 009EAC83
    fire.flip_count_6c = 0;                                             // 009EAC88
    fire.throttle_68 = 0.0f;                                            // 009EAC8F
    ship_ai_sub_attack_fire_new_band_009e9e50(fire, host);              // 009EAC98 JMP
}

void ship_ai_sub_attack_periscope_depth_009e4ee0(ShipAiSubAttackHost& host) {
    if (host.periscope_state_122c() != 2 && host.has_periscope_1214()) {
        host.set_depth_level_008528b0(1);                               // 009E4F00
        return;
    }
    if (host.target_present_0b20() && host.target_is_kind(8)) {
        host.set_depth_level_008528b0(1);                               // 009E4F2B
        return;
    }
    host.set_depth_level_008528b0(0);                                   // 009E4F3D
}

void ship_ai_sub_attack_deep_depth_009e4c70(ShipAiSubAttackHost& host) {
    host.set_depth_level_008528b0(host.allow_max_depth_0020() ? 3 : 2);
}

bool ship_ai_sub_attack_at_periscope_depth_009e4d00(ShipAiSubAttackHost& host) {
    // 009E4D0E..009E4D26: level 1, or a local Y above -4.0 (JBE on <= or unordered).
    if (host.depth_level_1268() != 1 && !(host.local_y_00a8() > kSubAttackPeriscopeFloor)) {
        return false;
    }
    const float y = host.self_position()[1];                            // 009E4D3B
    const float line = f(static_cast<double>(host.periscope_band_1204())
                         - kSubAttackPeriscopeSlack);                   // 009E4D4D..4D53
    return y > line;                                                    // 009E4D5D JBE
}

void ship_ai_sub_attack_raise_periscope_009e4d90(bool raise, ShipAiSubAttackHost& host) {
    bool value = raise;
    if (value) {
        value = ship_ai_sub_attack_at_periscope_depth_009e4d00(host)
                && host.target_present_0b20() && !host.target_is_kind(8);
    }
    if (host.periscope_state_122c() != 2) {                             // 009E4DC1
        host.set_periscope_state_122c(value ? 1 : 0);
    }
}

ShipAiPeriscopePrepassArm ship_ai_periscope_prepass_009db8f0(ShipAiPeriscopePrepassHost& host) {
    // Both band tests round band +/- 2.5 to float (FSTP [ESP+8]) and take the
    // lower arm on JA, so an unordered compare stays in the band.
    bool in_band = host.depth_level_1268() == 1;                        // 009DB8F6 JNZ
    if (in_band) {
        const float y = host.world_y_0100();                            // 009DB916
        const double band = static_cast<double>(host.periscope_band_1204());
        const float high = f(band + kSubPeriscopePrepassBand);          // 009DB91C..9DB928
        const float low = f(band - kSubPeriscopePrepassBand);           // 009DB954..9DB960
        if (y > high || low > y) in_band = false;                       // 009DB936 / 9DB96C JA
    }
    if (!in_band) {
        if (host.periscope_state_122c() == 2) return ShipAiPeriscopePrepassArm::kept_broken;
        host.set_periscope_state_122c(0);                               // 009DB9EC
        return ShipAiPeriscopePrepassArm::lowered_out_of_band;
    }
    bool held = false;
    if (!host.role_ai_held(1, held)) return ShipAiPeriscopePrepassArm::role_unavailable;
    if (!held) return ShipAiPeriscopePrepassArm::kept_player_role1;     // 009DB981
    if (host.active_state_wants_periscope_0020()) {                     // 009DB985..9DB99E
        if (host.periscope_state_122c() == 2) return ShipAiPeriscopePrepassArm::kept_broken;
        host.set_periscope_state_122c(1);                               // 009DB9AC
        return ShipAiPeriscopePrepassArm::raised_by_state;
    }
    if (!host.role_ai_held(0, held)) return ShipAiPeriscopePrepassArm::role_unavailable;
    if (!held) return ShipAiPeriscopePrepassArm::kept_player_role0;     // 009DB9C6
    if (host.periscope_state_122c() == 2) return ShipAiPeriscopePrepassArm::kept_broken;
    host.set_periscope_state_122c(0);                                   // 009DB9D4
    return ShipAiPeriscopePrepassArm::lowered_ai_held;
}

float ship_ai_sub_attack_fire_step_009e9eb0(ShipAiSubAttackFire& fire, float dt,
                                            ShipAiSubAttackHost& host,
                                            ShipAiSubAttackTick* tick) {
    if (!host.target_present_0b20()) return fire.throttle_68;           // 009E9EC4
    ship_ai_sub_attack_fire_refresh_009e9de0(fire, host);               // 009E9ECA
    fire.since_shot_24 = f(static_cast<double>(fire.since_shot_24) + dt);   // 009E9ED8
    const float range = ship_ai_sub_attack_range_009e4a60(host);        // L+0
    const float band = clamped_interpolate_00419010(fire.inner_54, 0.0f, fire.outer_58, 1.0f,
                                                    range);             // L+8, 009E9F08
    if (tick != nullptr) tick->range = range;

    // 009E9F11..009E9F69: the close latch.
    if (fire.close_40 != 0.0f) {
        if (range > kSubAttackCloseLeave) {                             // 009E9F28 JBE
            fire.close_40 = 0.0f;
            fire.left_close_48 = true;
            fire.close_time_44 = 0.0f;
        } else {
            fire.close_time_44 = f(static_cast<double>(fire.close_time_44) + dt);
        }
    } else if (static_cast<double>(kSubAttackCloseEnter) > static_cast<double>(range)) {
        fire.close_40 = 1.0f;                                           // 009E9F60
        fire.left_close_48 = false;
        fire.close_time_44 = 0.0f;
    }

    // 009E9F6E..009EA2AE: every 5 s at most, the choice of bow or stern tubes.
    bool force_band = false;                                            // L+4
    const float swap = f(static_cast<double>(dt) + fire.swap_timer_34);
    fire.swap_timer_34 = swap;                                          // 009E9F8F FST
    if (swap > kSubAttackEndCheckPeriod) {                              // 009E9F9E JBE
        bool toggle = false;
        float relative = 0.0f;
        if (fire.close_40 != 0.0f) {
            // 009E9FB6..009EA0B7: close in, the firing end more than 135 degrees off.
            if (static_cast<double>(swap)
                > static_cast<double>(fire.swap_period_60) * kSubAttackSwapFraction) {
                float heading = host.self_heading_vtable50();           // 009E9FD8
                if (!fire.fore_30) heading = wrapped_angle_add_00438aa0(heading, kPi);
                if (relative_bearing(host, heading, relative)
                    && std::fabs(relative) > kRearSwapLimit) {
                    toggle = true;
                }
            }
        } else if (fire.fore_30) {
            // 009EA0C8..009EA198.
            if (fire.fore_ready_4c == 0 && fire.since_shot_24 > kSubAttackEmptyEndSeconds
                && fire.fore_reload_28 > fire.aft_reload_2c) {
                toggle = true;                                          // 009EA0E2 JA
            } else if (fire.aft_ready_50 != 0) {
                const float heading = host.self_heading_vtable50();     // 009EA0FF
                if (relative_bearing(host, heading, relative)
                    && std::fabs(relative) > kForeSwapLimit) {
                    toggle = true;
                }
            }
        } else {
            // 009EA19D..009EA268.
            if (fire.aft_ready_50 == 0 && fire.since_shot_24 > kSubAttackEmptyEndSeconds
                && fire.aft_reload_2c > fire.fore_reload_28) {
                toggle = true;                                          // 009EA1B7 JA
            } else if (fire.fore_ready_4c != 0) {
                const float heading = host.self_heading_vtable50();     // 009EA1D4
                if (relative_bearing(host, heading, relative)
                    && kAftSwapLimit > std::fabs(relative)) {
                    toggle = true;
                }
            }
        }
        if (toggle) {
            fire.swap_timer_34 = 0.0f;                                  // 009EA272
            if (kSubAttackEndCheckPeriod > fire.since_shot_24) {        // 009EA27F JBE
                force_band = fire.fore_30 == fire.closing_38;           // 009EA285..9EA29C
            }
            fire.fore_30 = !fire.fore_30;                               // 009EA2A4..2AB
        }
    }

    // 009EA2B2..009EA307: the run-in / open-out band.
    const float band_time = f(static_cast<double>(dt) + fire.band_timer_3c);
    fire.band_timer_3c = band_time;                                     // 009EA2C1 FST
    if (band_time > kSubAttackBandPeriod || force_band) {
        const bool outside = fire.closing_38 ? fire.inner_54 > range     // 009EA2E0..2F4
                                             : range > fire.outer_58;    // 009EA2E9..2F4
        if (outside || force_band) {
            fire.closing_38 = !fire.closing_38;                         // 009EA2FD
            ship_ai_sub_attack_fire_new_band_009e9e50(fire, host);      // 009EA307
        }
    }

    // 009EA30C..009EA3DE: the aim point, led by up to 12 s of target motion.
    const std::array<float, 3> target = host.target_position();
    float aim_x = target[0];                                            // [ESP+30h]
    float aim_z = target[2];                                            // [ESP+38h]
    const std::array<float, 3> velocity = host.target_velocity_vtable34();  // 009EA352
    if (static_cast<double>(range) > static_cast<double>(kSubAttackCloseEnter)) {  // 009EA36B
        const float water = host.tubes_min_water_speed_0aec();          // 009EA371, +0AECh
        float lead = f((static_cast<double>(range) - static_cast<double>(kSubAttackCloseEnter))
                       / static_cast<double>(water));                   // 009EA376..9EA386
        if (!(kSubAttackMaxLead > lead)) lead = kSubAttackMaxLead;      // 009EA394 JBE
        const float lead_x = f(static_cast<double>(velocity[0]) * lead);
        const float lead_z = f(static_cast<double>(lead) * velocity[2]);
        aim_x = f(static_cast<double>(lead_x) + aim_x);
        aim_z = f(static_cast<double>(lead_z) + aim_z);
    }

    // 009EA3E2..009EA4CA: the heading that points the chosen end at the aim.
    const std::array<float, 3> self = host.self_position();
    const std::array<float, 3> direction{f(static_cast<double>(aim_x) - self[0]), 0.0f,
                                         f(static_cast<double>(aim_z) - self[2])};
    const std::array<float, 3> unit = host.normalize_00419510(direction);   // 009EA427
    float desired = heading_angle_00414eb0({unit[0], unit[2]});         // 009EA44C..9EA477
    if (!fire.fore_30) desired = wrapped_angle_subtract_00438b10(desired, kPi);  // 009EA49D
    const float heading = host.self_heading_vtable50();                 // 009EA4BF
    const float error = wrapped_angle_subtract_00438b10(heading, desired);  // L+0Ch, 009EA4C5

    // 009EA4CE..009EA525: the speed from where the range sits in the band.
    float speed = fire.closing_38
        ? clamped_interpolate_00419010(fire.inner_54, 0.25f, fire.outer_58, 3.0f, range)
        : clamped_interpolate_00419010(fire.outer_58, -0.25f, fire.inner_54, -3.0f, range);

    // 009EA525..009EA5B8: less the target's closing rate over our MaxSpeed.
    const std::array<float, 3> velocity2 = host.target_velocity_vtable34();  // 009EA53B
    const float closing = f((static_cast<double>(velocity2[0]) * unit[0]
                             + static_cast<double>(unit[1]) * 0.0)
                            + static_cast<double>(velocity2[2]) * unit[2]);
    float ratio = f(-static_cast<double>(closing)
                    / static_cast<double>(host.self_class_max_speed_0500()));
    if (kSubAttackRatioLow > ratio) {
        ratio = kSubAttackRatioLow;                                     // 009EA590
    } else if (ratio > kSubAttackRatioHigh) {
        ratio = kSubAttackRatioHigh;                                    // 009EA5D9
    }
    speed = f(static_cast<double>(speed) - ratio);                      // 009EA5A6
    if (!fire.fore_30) speed = f(-0.0 - static_cast<double>(speed));    // 009EA5B0..5BE

    if (0.0f > speed) {                                                 // 009EA5E7 JBE
        speed = unit_clamp_00415690(speed, -0.5f, -0.25f);              // 009EA615
        desired = wrapped_angle_subtract_00438b10(desired, kPi);        // 009EA62E
    } else {
        speed = unit_clamp_00415690(speed, 0.25f, 1.0f);                // 009EA65A
    }
    const float magnitude = std::fabs(error);
    const float limit = clamped_interpolate_00419010(kTurnSlowStart, 1.0f, kTurnSlowEnd, 0.5f,
                                                     magnitude);        // 009EA6A7

    // 009EA6B6..009EA739: a big heading error far from the band edge backs off.
    bool reverse = false;
    if (static_cast<double>(magnitude) > kSubAttackBigError) {
        const double s = speed;
        const double t = band;
        if (fire.fore_30) {
            reverse = speed > 0.0f ? t > kSubAttackBandSlack - s
                                   : kSubAttackBandNudge - s > t;
        } else {
            reverse = speed > 0.0f ? s + kSubAttackBandNudge > t
                                   : t > s + kSubAttackBandSlack;
        }
    }
    if (reverse) {
        fire.reverse_64 = kSubAttackReverseHold;                        // 009EA72B
    } else {
        fire.reverse_64 = f(static_cast<double>(fire.reverse_64) - dt); // 009EA732..739
    }
    if (fire.reverse_64 > 0.0f) {                                       // 009EA741 JBE
        speed = speed > 0.0f ? -0.5f : 0.5f;                            // 009EA746..75F
        desired = wrapped_angle_subtract_00438b10(desired, kPi);        // 009EA779
    }
    if (-limit > speed) {                                               // 009EA79C
        speed = -limit;
    } else if (speed > limit) {                                         // 009EA7AE
        speed = limit;
    }
    host.set_desired_heading_009e0040(desired);                         // 009EA7D0

    // 009EA7D5..009EA853: a sign change of the throttle needs three ticks.
    const float previous = fire.throttle_68;
    float throttle = speed;
    if (0.0 > static_cast<double>(previous) * speed) {
        fire.flip_count_6c -= 1;                                        // 009EA7EC
        if (fire.flip_count_6c > 0) throttle = previous;                // 009EA7F4 JLE
    } else {
        fire.flip_count_6c = kSubAttackReverseDebounce;                 // 009EA7FE
    }
    host.set_desired_throttle_009dbf90(throttle);                       // 009EA80B..9EA83D inline
    fire.throttle_68 = throttle;                                        // 009EA853
    if (tick != nullptr) tick->throttle = throttle;

    // 009EA843..009EA9A9: the depth.
    if (fire.close_40 != 0.0f) {
        if (host.target_present_0b20() && host.target_is_kind(8)) {     // 009EA866..9EA878
            if (host.target_position()[1] > host.self_position()[1]) {  // 009EA8A2..8B4
                ship_ai_sub_attack_deep_depth_009e4c70(host);           // 009EA8B6
            } else {
                ship_ai_sub_attack_periscope_depth_009e4ee0(host);      // 009EA8BD
            }
        } else {
            ship_ai_sub_attack_deep_depth_009e4c70(host);               // 009EA8C4..8DE inline
        }
        if (host.periscope_state_122c() != 2) host.set_periscope_state_122c(0);  // 009EA8FB
        return throttle;
    }
    const int ready = fire.fore_30 ? fire.fore_ready_4c : fire.aft_ready_50;
    if (ready > 0 && kSubAttackAimTolerance > std::fabs(error)) {       // 009EA911..93D / 958..983
        ship_ai_sub_attack_periscope_depth_009e4ee0(host);              // 009EA987
        ship_ai_sub_attack_raise_periscope_009e4d90(true, host);        // 009EA990
        return throttle;
    }
    host.set_depth_level_008528b0(2);                                   // 009EA949 / 9EA9A9
    return throttle;
}

ShipAiSubAttackTick ship_ai_sub_attack_tick_009eaa90(ShipAiSubAttackState& parent, float dt,
                                                     ShipAiSubAttackHost& host) {
    ShipAiSubAttackTick tick;
    if (!host.target_present_0b20()) return tick;                       // 009EAA9C
    tick.ran = true;
    // 009EAA10: the one-second switch.
    const float countdown = parent.countdown_cc;
    if (dt < countdown) {                                               // 009EAA28 JC
        parent.countdown_cc = f(static_cast<double>(countdown) - dt);
    } else {
        parent.countdown_cc = f(static_cast<double>(countdown)
                                + (static_cast<double>(parent.interval_c8) - dt));
        if (!parent.fire_current) {
            if (ship_ai_sub_attack_approach_to_fire_009e9d90(parent.approach.tube, host)) {
                // 007B6EE0: approach vtable[8] (007B3DC0, RET), then fire vtable[4].
                parent.fire_current = true;
                tick.switched_to_fire = true;
                ship_ai_sub_attack_fire_enter_009eabe0(parent.fire, host);
            }
        } else if (ship_ai_sub_attack_fire_to_approach_009ea9c0(parent.fire.tube, host)) {
            // fire vtable[8] (007B3DC0) and approach vtable[4] (007B3DB0) are RET.
            parent.fire_current = false;
            tick.switched_to_approach = true;
        }
    }
    if (parent.fire_current) {                                          // 009EAAB0, [this+D4h]
        tick.fire_step = true;
        ship_ai_sub_attack_fire_step_009e9eb0(parent.fire, dt, host, &tick);
    } else {
        tick.approach_step = true;
        ship_ai_sub_attack_approach_step_009e4b90(parent.approach, host);
        tick.range = parent.approach.range_20;
    }
    return tick;
}

}  // namespace bsp
