// Segment 46 reference models: the MParatrooper entity and the level-bomb rack's
// paratrooper branch. Packet cc13-gpr46; docs/CC13_GPR_46.md. Read from the
// installed executable's bytes (Capstone; x87 traced with sc2_x87.py) and the
// Ghidra pseudocode; the listing decides every comparison. Unrounded x87
// intermediates are carried in double (the image keeps 80 bits); every FSTP to a
// float slot is an explicit f32().

#include "bsp/gameplay_seg46_cc13.hpp"

#include "bsp/unit_rudder.hpp"  // clamped_interpolate_00419010

#include <cmath>

namespace bsp {
namespace gameplay_seg46_cc13 {

namespace {

constexpr std::uint32_t kParatrooperKind = 0x31;      // 007AB9B1 MOV [ESI+0C4h], 31h
constexpr std::int32_t kDropperInitial = 8;            // 007AB9C1 MOV [ESI+4B8h], 8
constexpr double kAvoidRadiusStep_00d7a2b0 = 3.0;      // qword
constexpr double kAvoidRadiusBase_00cf1440 = 80.0;     // qword
constexpr float kAvoidSearch_00ce3950 = 800.0f;
constexpr double kSteerDivisor_00d7a2b0 = 3.0;         // qword, same constant
constexpr double kSteerDelay_00d7a308 = 2.0;           // qword
constexpr float kOne_00d7a24c = 1.0f;
constexpr float kNegZero_00d7a208 = -0.0f;
constexpr float kMinusOne_00d7a260 = -1.0f;
constexpr double kAnimEndInset_00d7a318 = 0.0010000000474974513;  // qword (float 0.001 widened)
constexpr float kDropJitterLo_00ceca0c = -50.0f;
constexpr float kDropJitterHi_00ceb4d4 = 50.0f;
constexpr int kKillCause = 2;                          // every PUSH 2 before 00926D90
constexpr int kModeSkipKills = 2;                      // [game+1FE4h] == 2 skips the kills
constexpr int kModeSendsLanding = 1;                   // 007AB294 CMP [EAX+1FE4h], 1

float f32(double v) noexcept { return static_cast<float>(v); }

// 00419010(x0, y0, x1, y1, x) rows used by 007ABB30.
float steer_scale(float chute) noexcept {
    // 007ABDDE-007ABE01: (0.2 -> 5.0, 1.0 -> 2.5) over the chute fraction.
    return clamped_interpolate_00419010(0.2f, 5.0f, 1.0f, 2.5f, chute);  // 00CE54A0, 00CE3850, 1, 00CF87C8
}
float lateral_cap_scale(float chute) noexcept {
    // 007ABF01-007ABF24: (0.5 -> 0.3, 1.0 -> 1.8) over the chute fraction.
    return clamped_interpolate_00419010(0.5f, 0.3f, 1.0f, 1.8f, chute);  // 00CE3800, 00CE69C8, 1, 00CF4848
}

// The two-sided clamp both halves of 007ABB30 use: low test first (FCOMI with JBE,
// so an unordered value falls through to the high test and then passes).
float clamp_lo_first(float v, float lo, float hi) noexcept {
    if (lo > v) return lo;
    if (v > hi) return hi;
    return v;
}

}  // namespace

void construct_007ab8c0(ParatrooperState& s, const soldier_traffic_cc13::SoldierGlobals& g,
                        ParatrooperHost& host) {
    // 007AB937-007AB94A: lo = [globals+14h] at [esp], hi = [globals+18h] at [esp+4], ECX = 1.
    s.open_altitude_468 = host.uniform(1, g.parachute_open_altitude_min, g.parachute_open_altitude_max);
    // 007AB952-007AB9C1, in image order.
    s.landed_470 = false;
    s.bad_landing_471 = false;
    s.in_water_472 = false;
    s.soldier_478 = 0;
    s.target_building_48c = 0;
    s.spawn_soldier_pending_490 = false;
    s.flying_458 = false;
    s.avoid_zone_done_488 = false;
    s.chute_46c = 0.0f;
    s.capture_timer_474 = 0.0f;
    s.class_id_c4 = kParatrooperKind;
    s.flight_time_494 = 0.0f;
    s.dropper_4b8 = kDropperInitial;
}

bool is_kind_of_007aba50(const ParatrooperState& s, std::uint32_t kind) noexcept {
    return kind == 0x31 || kind == 0x2A || kind == 2 || kind == 1 || kind == 0 || kind == s.class_id_c4;
}

void load_animations_007ac530(ParatrooperState& s, std::uint32_t fall, float fall_length,
                              std::uint32_t open, float open_length, std::uint32_t close,
                              float close_length, ParatrooperHost& host) {
    s.anim_fall_49c = fall;
    s.anim_open_4a4 = open;
    s.anim_close_4ac = close;
    s.fall_length_4a0 = fall_length;
    s.open_length_4a8 = open_length;
    s.close_length_4b0 = close_length;
    s.anim_current_498 = s.anim_fall_49c;
    s.anim_time_4b4 = 0.0f;
    s.flight_time_494 = 0.0f;
    host.set_chute_anim(s.anim_fall_49c, 0.0f);
}

void set_landing_pos_007ab230(ParatrooperState& s, const Vec3& point, ParatrooperHost& host) {
    s.target_47c = point;
    // 007AB27C: the ground height at the new point replaces y.
    s.target_47c.y = host.ground_height_at(s.target_47c);
    if (host.session_mode() != kModeSendsLanding) return;
    if (host.session_flag_218c()) return;
    host.send_landing_message_71(s.target_47c);  // 007AB2A8-007AB2FE
    std::int32_t dropper = 0;
    if (host.dropper_from_owner(dropper)) s.dropper_4b8 = dropper;  // 007AB303-007AB313
}

void advance_in_air_007abb30(ParatrooperState& s, const ParatrooperClassFields& cls, float dt,
                             ParatrooperHost& host) {
    // 007ABB36-007ABB6B: k = c + ((chute - 0) * (o - c)) / (1 - 0), stored to float.
    const double c = cls.slow_factor_closed_dc;
    const float k = f32(((static_cast<double>(s.chute_46c) - 0.0) *
                         (static_cast<double>(cls.slow_factor_opened_e0) - c)) / (1.0 - 0.0) + c);
    // 007ABB6F-007ABC07: v += (-v * dt) * k per axis, each product stored.
    float* v[3] = {&s.velocity_318.x, &s.velocity_318.y, &s.velocity_318.z};
    float drag[3];
    for (int i = 0; i < 3; ++i) drag[i] = f32(static_cast<double>(-*v[i]) * dt);
    for (int i = 0; i < 3; ++i) drag[i] = f32(static_cast<double>(drag[i]) * k);
    for (int i = 0; i < 3; ++i) *v[i] = f32(static_cast<double>(drag[i]) + *v[i]);
    host.bomb_advance_in_air(s, dt);  // 007ABC10
    if (!s.flying_458) return;        // 007ABC15

    if (!s.avoid_zone_done_488) {
        // 007ABC41-007ABC91: radius = (word +174h % 23) * 3.0 + 80.0.
        const unsigned id = static_cast<unsigned>(s.net_id_174) % 23u;
        const float radius = f32(static_cast<double>(id) * kAvoidRadiusStep_00d7a2b0 + kAvoidRadiusBase_00cf1440);
        float xz[2] = {s.target_47c.x, s.target_47c.z};
        host.avoid_zone_nearest_boundary(xz, kAvoidSearch_00ce3950, radius);
        s.target_47c = Vec3{xz[0], 0.0f, xz[1]};
        s.target_47c.y = host.ground_height_at(s.target_47c);  // 007ABCD9
        s.avoid_zone_done_488 = true;
    }
    // 007ABCF5-007ABD49: the velocity and its lateral magnitudes before the steer.
    const float vx = s.velocity_318.x;
    const float vy = s.velocity_318.y;
    const float vz = s.velocity_318.z;
    float bound_x = std::fabs(vx);
    float bound_z = std::fabs(vz);
    const Vec3 pos = host.world_position();
    // 007ABD56-007ABD9C: the steer accelerations, (target - pos) / 3.0.
    const float dx = f32(static_cast<double>(s.target_47c.x) - pos.x);
    const float dz = f32(static_cast<double>(s.target_47c.z) - pos.z);
    const float ax = f32(dx / kSteerDivisor_00d7a2b0);
    const float az = f32(dz / kSteerDivisor_00d7a2b0);
    // 007ABDA0-007ABE5D: the authority ramp, (flight time - 2) clamped to [0, 1].
    const float t = f32(static_cast<double>(s.flight_time_494) - kSteerDelay_00d7a308);
    float ramp = 0.0f;
    if (!(0.0f > t)) ramp = (t > kOne_00d7a24c) ? kOne_00d7a24c : t;  // COMISS ... JBE keeps t
    const float hi = f32(static_cast<double>(steer_scale(s.chute_46c)) * ramp);  // 007ABE06
    const float lo = kNegZero_00d7a208 - hi;                                     // 007ABE20 SUBSS
    const float cx = clamp_lo_first(ax, lo, hi);  // 007ABE1C-007ABE76
    const float cz = clamp_lo_first(az, lo, hi);  // 007ABE7C-007ABEB0
    // 007ABEB6-007ABEF5: the steered lateral velocity.
    const float nx = f32(static_cast<double>(vx) + static_cast<double>(cx) * dt);
    const float nz = f32(static_cast<double>(vz) + static_cast<double>(dt) * cz);
    // 007ABF24-007ABF2D: the lateral cap floor, scale(chute) * |vy|.
    const float floor_ = f32(static_cast<double>(lateral_cap_scale(s.chute_46c)) * std::fabs(vy));
    // 007ABF3B-007ABF65: FCOMI/JA keeps the magnitude only when strictly larger.
    if (!(bound_x > floor_)) bound_x = floor_;
    if (!(bound_z > floor_)) bound_z = floor_;
    // 007ABF67-007ABFE5.
    s.velocity_318.x = clamp_lo_first(nx, -bound_x, bound_x);
    s.velocity_318.y = vy;
    s.velocity_318.z = clamp_lo_first(nz, -bound_z, bound_z);
}

void update_chute_and_target_007ab360(ParatrooperState& s, const ParatrooperClassFields& cls,
                                      float dt, ParatrooperHost& host) {
    host.orthonormalize_upright();  // 007AB361-007AB391
    if (!s.flying_458) {
        // 007AB39C-007AB3E2: not flying: chute shut, no target, hold "fall" at 0.
        s.chute_46c = 0.0f;
        s.target_building_48c = 0;
        if (host.chute_anim_present()) host.set_chute_anim(s.anim_fall_49c, 0.0f);
        return;
    }
    s.flight_time_494 = f32(static_cast<double>(s.flight_time_494) + dt);  // 007AB3E5-007AB3FB
    if (s.anim_current_498 == s.anim_fall_49c) {
        Vec3 pos = host.world_position();
        float ground = host.ground_height_at(pos);
        if (0.0f > ground) ground = 0.0f;  // 007AB43D COMISS / JBE
        pos = host.world_position();
        // 007AB45D-007AB472: FCOMI(open altitude, y - ground), JB skips (unordered too).
        const double height = static_cast<double>(pos.y) - ground;
        if (static_cast<double>(s.open_altitude_468) >= height) {
            s.anim_time_4b4 = 0.0f;
            s.anim_current_498 = s.anim_open_4a4;
            s.chute_46c = kOne_00d7a24c;
        }
    } else if (s.anim_current_498 == s.anim_open_4a4 && (s.landed_470 || s.in_water_472)) {
        s.anim_current_498 = s.anim_close_4ac;  // 007AB4B4-007AB4C3
        s.anim_time_4b4 = 0.0f;
    }
    if (host.session_mode() != kModeSkipKills && s.target_building_48c != 0) {
        const std::uint32_t b = s.target_building_48c;
        if (host.building_side(b) == s.side_54) {
            host.kill_self(kKillCause);  // 007AB4F0: a friendly building
        } else if (host.party_relative(s.side_54, host.building_side(b)) == 1) {
            // 007AB509-007AB549: (Damage - building+368h) * dt to vslot +1ACh while positive.
            const float d = f32(static_cast<double>(cls.damage_fc) - host.building_368(b));
            if (d > 0.0f) host.building_damage(b, f32(static_cast<double>(d) * dt));
        }
    }
    // 007AB54F-007AB57A: this->vslot +19Ch (007ABB30) with dt * class+5Ch.
    advance_in_air_007abb30(s, cls, f32(static_cast<double>(cls.step_scale_5c) * dt), host);
}

void anim_clock_007ab0b0(ParatrooperState& s, float dt, ParatrooperHost& host) {
    // 007AB0B1-007AB0D9: the clock advances first and is stored.
    const float t = f32(static_cast<double>(dt) + s.anim_time_4b4);
    s.anim_time_4b4 = t;
    if (s.anim_current_498 == s.anim_fall_49c) {
        s.anim_time_4b4 = f32(std::fmod(static_cast<double>(t), s.fall_length_4a0));  // 00BF857A
    } else if (s.anim_current_498 == s.anim_open_4a4) {
        // 007AB0FC-007AB122: hold just short of the end.
        const float end = f32(static_cast<double>(s.open_length_4a8) - kAnimEndInset_00d7a318);
        if (t > end) s.anim_time_4b4 = end;
    } else if (s.anim_current_498 == s.anim_close_4ac) {
        // 007AB12E-007AB168: the inset end stays unrounded for the compare.
        const double end = static_cast<double>(s.close_length_4b0) - kAnimEndInset_00d7a318;
        if (static_cast<double>(t) > end) {
            s.anim_time_4b4 = f32(end);
            s.chute_46c = 0.0f;
            if (s.soldier_478 != 0) host.soldier_muzzle_effect(s.soldier_478);  // 0048A1E0(0)
            s.capture_timer_474 = f32(static_cast<double>(s.capture_timer_474) + dt);
        }
    }
    host.set_chute_anim(s.anim_current_498, s.anim_time_4b4);  // 007AB172-007AB18F, no null test
    if (s.soldier_478 != 0) {
        const Vec3 pos = host.world_position();
        if (host.soldier_clip_visible(s.soldier_478, pos)) host.soldier_show(s.soldier_478);
    }
}

void step_landing_007ac000(ParatrooperState& s, const ParatrooperClassFields& cls,
                           const soldier_traffic_cc13::SoldierGlobals& g, float dt,
                           ParatrooperHost& host) {
    const bool kills = host.session_mode() != kModeSkipKills;
    if (s.landed_470) {
        if (!(s.chute_46c <= 0.0f)) return;  // 007AC017 COMISS / JB (unordered too)
        if (s.bad_landing_471) {
            if (kills) host.kill_self(kKillCause);  // 007AC03F
            return;
        }
        if (s.soldier_478 == 0) {
            const Vec3 pos = host.world_position();
            s.target_building_48c = host.find_enemy_building(pos, s.side_54);  // 007AC077
            if (s.target_building_48c != 0) {
                s.spawn_soldier_pending_490 = true;
                host.register_fixed_step_once();  // 007AC095
            } else if (kills) {
                host.kill_self(kKillCause);  // 007AC0AF, then falls through
            }
        }
        // 007AC0B4-007AC0E9: FCOMI(timer, CaptureDuration), JB skips.
        if (s.capture_timer_474 >= cls.capture_duration_f8 && kills) host.kill_self(kKillCause);
        return;
    }
    host.extrapolate_position(s, dt);  // 007AC0FE
    if (s.in_water_472) {
        if (s.chute_46c <= 0.0f && kills) host.kill_self(kKillCause);  // 007AC10C-007AC133
        return;
    }
    if (!s.flying_458) return;
    Vec3 pos = host.world_position();
    const float ground = host.ground_height_at(pos);
    if (0.0f > ground) {
        // 007AC184-007AC1A5: over water, below sea level.
        pos = host.world_position();
        if (0.0f > pos.y) s.in_water_472 = true;
        return;
    }
    pos = host.world_position();
    if (!(ground >= pos.y)) return;  // 007AC1C3-007AC1D1, JB skips (unordered too)
    s.landed_470 = true;
    s.landing_pose_a4 = Vec3{pos.x, ground, pos.z};  // 007AC1E0-007AC22A
    Vec3 n;
    if (!host.ground_normal_at(pos, n)) return;  // 007AC244
    // 007AC24D-007AC261: (nx * 0 + ny) + 0 * nz, stored.
    const float dot = f32((static_cast<double>(n.x) * 0.0 + n.y) + 0.0 * n.z);
    if (!(dot >= kMinusOne_00d7a260)) return;  // 007AC26F JB
    if (!(kOne_00d7a24c >= dot)) return;       // 007AC27D JB
    const float slope = f32(std::acos(static_cast<double>(dot)));  // 00BF9940
    if (slope > g.max_paratrooper_gradient) s.bad_landing_471 = true;  // 007AC29F JBE skips
}

LandSoldierPlan land_and_spawn_soldier_007ab590(ParatrooperState& s, const ParatrooperClassFields& cls,
                                                ParatrooperHost& host) {
    LandSoldierPlan plan;
    if (!s.spawn_soldier_pending_490) return plan;  // 007AB590 CMP [ECX+0ACh], 0
    plan.run = true;
    plan.building = s.target_building_48c;
    host.add_capturer(s.target_building_48c);  // 006F39B0
    plan.soldier_class = cls.soldier_class_ec;
    plan.soldier_anim = cls.soldier_anim_f0;
    const Vec3 self = host.world_position();
    const Vec3 b = host.building_position(s.target_building_48c);
    plan.facing = Vec3{f32(static_cast<double>(b.x) - self.x), f32(static_cast<double>(b.y) - self.y),
                       f32(static_cast<double>(b.z) - self.z)};
    // The soldier handle lands in +478h when the host creates it; the clip test
    // and vslot +D8h follow (007AB6EC-007AB70E). The last store clears +490h.
    s.spawn_soldier_pending_490 = false;
    return plan;
}

void on_killed_007aae10(ParatrooperState& s, ParatrooperHost& host) {
    host.on_killed_lua();  // 0077D270
    if (s.soldier_478 != 0) {
        host.kill_entity(s.soldier_478, kKillCause);
        s.soldier_478 = 0;
    }
    if (s.target_building_48c != 0) host.remove_capturer(s.target_building_48c);  // 006F34B0
}

void set_target_point_006e3f90(RackParatrooperTarget& rack, const Vec3& point,
                               const Vec3& velocity) noexcept {
    rack.set_4f8 = true;
    rack.point_4fc = point;
    rack.velocity_508 = velocity;
}

bool paratrooper_drop_target_006e541f(RackParatrooperTarget& rack, bool round_is_31h,
                                      float repeat_time_494, ParatrooperHost& host, Vec3& point) {
    if (!rack.set_4f8 || !round_is_31h) return false;  // 006E541F-006E5439
    // 006E5466-006E54AE: x first, then z; y is the rack point's (007AB230 replaces it).
    const float jx = host.uniform(0, kDropJitterLo_00ceca0c, kDropJitterHi_00ceb4d4);
    point.x = f32(static_cast<double>(jx) + rack.point_4fc.x);
    point.y = rack.point_4fc.y;
    const float jz = host.uniform(0, kDropJitterLo_00ceca0c, kDropJitterHi_00ceb4d4);
    point.z = f32(static_cast<double>(jz) + rack.point_4fc.z);
    // 006E54B7-006E5515: the next round's point moves on by velocity * toRepeatTime.
    const double t = repeat_time_494;
    const float sx = f32(static_cast<double>(rack.velocity_508.x) * t);
    const float sy = f32(static_cast<double>(rack.velocity_508.y) * t);
    const float sz = f32(t * rack.velocity_508.z);
    rack.point_4fc.x = f32(static_cast<double>(sx) + rack.point_4fc.x);
    rack.point_4fc.y = f32(static_cast<double>(rack.point_4fc.y) + sy);
    rack.point_4fc.z = f32(static_cast<double>(sz) + rack.point_4fc.z);
    return true;
}

void paratrooper_drop_clear_006e55e7(RackParatrooperTarget& rack, std::int32_t rounds_left_484) noexcept {
    if (rounds_left_484 <= 0) rack.set_4f8 = false;  // 006E55EE JLE -> 006E55FE
}

}  // namespace gameplay_seg46_cc13
}  // namespace bsp
