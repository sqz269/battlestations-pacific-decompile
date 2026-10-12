// Packet cc13-oprocket1: the pilot bot's opportunistic rocket sub-object
// 009FD0E0. docs/CC13_OPPORTUNISTIC_ROCKET.md. Reconstructed from the listing;
// build-tested only. Names are hypotheses.
//
// Precision: x87 intermediates are carried in double and every FSTP dword is a
// float store, as in src/rocket_task_cc13.cpp.
#include "bsp/opportunistic_rocket_cc13.hpp"

#include <cmath>

#include "bsp/gamepad_force_events.hpp"
#include "bsp/native_scalar_float_by_ref.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"

namespace bsp {

namespace {

// robots.lua DEG(): degrees to radians, stored as float.
constexpr float deg(double d) noexcept {
    return static_cast<float>(d * 3.14159265358979323846 / 180.0);
}

using CR = OpportunisticRocketClassRow;

// attack_dist slot of each class (009F99B0's index for that class's targets).
constexpr CR plane(float chance, int slot, float dist, float time, int n, float delay, float angle,
                   float homing) noexcept {
    CR r{};
    r.rocket_chance_00 = chance;
    r.attack_dist_04[static_cast<std::size_t>(slot)] = dist;
    r.attack_time_1c = time;
    r.num_rockets_20 = n;
    r.rocket_delay_24 = delay;
    r.attack_angle_28 = angle;
    r.homing_angle_2c = homing;
    return r;
}
constexpr CR ship(float chance, float d1, float d2, float d3, float time, int n, float delay,
                  float angle, float homing) noexcept {
    CR r = plane(chance, 2, d1, time, n, delay, angle, homing);
    r.attack_dist_04[3] = d2;
    r.attack_dist_04[4] = d3;
    return r;
}

}   // namespace

// scripts/datatables/robots.lua (mtime 2025-06-01), Robots["PilotBot"], by line.
const std::array<OpportunisticRocketRow, 6> kOpportunisticRocketRows = {{
    // Stun :1301-1340
    {2.0f, 3.0f, {{plane(0.1f, 0, 1000.0f, 4.0f, 2, 0.6f, deg(30), deg(0.2)),
                   plane(0.1f, 1, 1000.0f, 3.5f, 2, 0.6f, deg(10), deg(0.2)),
                   ship(0.1f, 1000.0f, 700.0f, 500.0f, 8.0f, 2, 0.6f, deg(10), deg(0.2)),
                   plane(0.1f, 5, 1000.0f, 6.0f, 2, 0.6f, deg(10), deg(0.2))}}},
    // SPNormal :611-650
    {1.0f, 2.0f, {{plane(1.0f, 0, 1000.0f, 4.0f, 4, 0.2f, deg(30), deg(0.6)),
                   plane(0.5f, 1, 1000.0f, 3.5f, 2, 0.2f, deg(6), deg(0.5)),
                   ship(1.0f, 1800.0f, 1200.0f, 800.0f, 3.5f, 4, 0.2f, deg(10), deg(0.2)),
                   plane(0.8f, 5, 1200.0f, 3.5f, 3, 0.15f, deg(4), deg(0.2))}}},
    // SPVeteran :778-817
    {2.0f, 3.0f, {{plane(1.0f, 0, 1500.0f, 3.5f, 1, 0.3f, deg(3), deg(2.0)),
                   plane(1.0f, 1, 1800.0f, 3.5f, 1, 0.3f, deg(2), deg(2.0)),
                   ship(1.0f, 1800.0f, 1600.0f, 1400.0f, 3.5f, 2, 0.3f, deg(2), deg(2.0)),
                   plane(1.0f, 5, 1200.0f, 3.5f, 1, 0.3f, deg(4), deg(2.0))}}},
    // MPNormal :916-955
    {2.0f, 3.0f, {{plane(0.5f, 0, 1000.0f, 4.0f, 1, 0.3f, deg(9), deg(0.5)),
                   plane(0.75f, 1, 1200.0f, 3.5f, 1, 0.3f, deg(6), deg(1.0)),
                   ship(0.9f, 1300.0f, 1100.0f, 900.0f, 3.5f, 2, 0.3f, deg(6), deg(1.5)),
                   plane(0.9f, 5, 1200.0f, 3.5f, 2, 0.3f, deg(4), deg(1.5))}}},
    // MPVeteran :1065-1104
    {2.0f, 3.0f, {{plane(0.5f, 0, 1200.0f, 4.0f, 1, 0.3f, deg(4), deg(2.0)),
                   plane(1.0f, 1, 1200.0f, 3.5f, 1, 0.3f, deg(6), deg(2.0)),
                   ship(1.0f, 1600.0f, 1300.0f, 1000.0f, 3.5f, 2, 0.3f, deg(4), deg(1.75)),
                   plane(1.0f, 5, 1500.0f, 3.5f, 2, 0.3f, deg(4), deg(1.75))}}},
    // Elite :1192-1231
    {2.0f, 3.0f, {{plane(1.0f, 0, 1500.0f, 3.5f, 1, 0.3f, deg(3), deg(2.0)),
                   plane(1.0f, 1, 1800.0f, 3.5f, 1, 0.3f, deg(2), deg(2.0)),
                   ship(1.0f, 1800.0f, 1600.0f, 1400.0f, 3.5f, 2, 0.3f, deg(2), deg(2.0)),
                   plane(1.0f, 5, 1200.0f, 3.5f, 1, 0.3f, deg(4), deg(2.0))}}},
}};

// 009FCEF0: +0 = -1, +0Ch = 0, +10h = +14h = [00D7A260] (-1.0), +38h = 0,
// +3Ch = 0, +40h = 0, +58h = 0. +18h..+20h are not written.
void opportunistic_rocket_construct_009fcef0(OpportunisticRocketState& s) noexcept {
    const std::array<float, 3> point = s.aim_point_18;
    s = OpportunisticRocketState{};
    s.aim_point_18 = point;
}

// 007B7870-007B78E2.
void opportunistic_rocket_set_target_007b7870(OpportunisticRocketState& s,
                                              std::size_t target_plus_one,
                                              bool target_is_plane,
                                              const std::array<float, 3>& point) noexcept {
    s.target_plus_one_38 = target_plus_one;    // 007B7878-007B789E, the +24h node
    s.aim_point_18 = point;                    // 007B78A6-007B78C2
    s.target_is_plane_3c = target_plus_one != 0 && target_is_plane;   // 007B78C7-007B78DE
}

// 007B7BB7-007B7C34 (and 009AA9F7.., 009CD0E7..).
void opportunistic_rocket_clear_target(OpportunisticRocketState& s) noexcept {
    s.target_plus_one_38 = 0;
    s.aim_point_18 = {0.0f, 0.0f, 0.0f};   // [00F87574]
    s.target_is_plane_3c = false;
}

// 009FD1E6-009FD256: kind 0Fh first, then 6, then 1Bh, 45h, 46h.
OpportunisticRocketClass opportunistic_rocket_class_for_target(
    const OpportunisticRocketTargetKinds& k) noexcept {
    if (k.plane_0f) {
        // 009FD1F9-009FD205: NEG AL / SBB / ADD 1 -> 0 when 0047B880 is true.
        return k.small_plane_0047b880 ? OpportunisticRocketClass::kSmallPlane
                                      : OpportunisticRocketClass::kLargePlane;
    }
    if (k.ship_06) return OpportunisticRocketClass::kShip;
    if (k.land_1b || k.land_45 || k.land_46) return OpportunisticRocketClass::kLandfort;
    return OpportunisticRocketClass::kNone;
}

// 009F99B0.
int opportunistic_rocket_range_slot_009f99b0(const OpportunisticRocketTargetKinds* k) noexcept {
    if (k == nullptr) return 5;
    if (k->plane_0f) return k->small_plane_0047b880 ? 0 : 1;
    if (k->ship_06) {
        if (k->kind_09 || k->kind_0d) return 2;
        if (k->kind_0b || k->kind_0a) return 3;
        return 4;
    }
    return 5;
}

// 009FD440-009FD47F: CVTSI2SS of +0Ch and of the row's NumRockets.
float opportunistic_rocket_salvo_scale(std::int32_t num_rockets, std::int32_t salvo_left) noexcept {
    return clamped_interpolate_00419010(0.0f, opportunistic_rocket::kSalvoScaleLast,
                                        static_cast<float>(num_rockets), 1.0f,
                                        static_cast<float>(salvo_left));
}

// 009FD3DD-009FD4DF.
OpportunisticRocketGate opportunistic_rocket_fire_gate(const OpportunisticRocketGateInputs& in,
                                                       const OpportunisticRocketClassRow& row) noexcept {
    namespace k = opportunistic_rocket;
    const float dist = in.attack_dist;
    // 009FD3F2-009FD3FE: FLD qword 0.4 * dist, FSTP dword.
    const float near_dist = static_cast<float>(k::kMinRangeScale * static_cast<double>(dist));
    // 009FD3EB COMISS / JBE: unordered also returns.
    if (!(in.body_aim[2] > k::kMinForward)) return OpportunisticRocketGate::kBehind;
    // 009FD40E FCOMIP(dist, range) / JBE.
    if (!(dist > in.body_range)) return OpportunisticRocketGate::kOutOfRange;
    // 009FD41F FCOMIP(AttackTime, t) / JBE.
    if (!(row.attack_time_1c > in.flight_time)) return OpportunisticRocketGate::kTooSlow;
    // 009FD42F COMISS(t, 0.25) / JBE.
    if (!(in.flight_time > k::kMinFlightTime)) return OpportunisticRocketGate::kTooClose;
    const float scale = opportunistic_rocket_salvo_scale(row.num_rockets_20, in.salvo_left);
    // 009FD43C-009FD49E: x and y spilled as qwords, x*x + y*y FSTP dword, sqrt
    // (00BF7030) FSTP dword, times the scale FSTP dword.
    const double x = static_cast<double>(in.body_aim[0]);
    const double y = static_cast<double>(in.body_aim[1]);
    const float sum = static_cast<float>(x * x + y * y);
    const float lateral_raw = static_cast<float>(std::sqrt(static_cast<double>(sum)));
    const float lateral = static_cast<float>(static_cast<double>(lateral_raw) *
                                             static_cast<double>(scale));
    // 009FD4B2 FCOMIP(5.0, lateral) / JA: under 5 m the angle is not tested.
    if (k::kLateralFloor > lateral) return OpportunisticRocketGate::kPass;
    // 009FD4CD 00415620(range, near_dist, dist), FDIVR qword lateral, then
    // FCOMIP(AttackAngle, ratio) / JBE.
    const float range = clamp_float_by_ref_00415620(in.body_range, near_dist, dist);
    const double ratio = static_cast<double>(lateral) / static_cast<double>(range);
    if (!(static_cast<double>(row.attack_angle_28) > ratio)) return OpportunisticRocketGate::kOffAngle;
    return OpportunisticRocketGate::kPass;
}

// 009FD0E0-009FD54E.
OpportunisticRocketTickResult opportunistic_rocket_tick_009fd0e0(OpportunisticRocketState& s,
                                                                 OpportunisticRocketHost& host,
                                                                 RocketRandom& rng, float dt) {
    namespace k = opportunistic_rocket;
    OpportunisticRocketTickResult out;
    // 009FD0E7-009FD107: a target whose +5Dh is set is dropped.
    if (s.target_plus_one_38 != 0 && host.target_dead_5d()) s.target_plus_one_38 = 0;
    // 009FD10A-009FD12B: the plane's own four live bytes.
    if (!host.unit_live()) return out;
    out.ran = true;
    // 009FD131-009FD153: both timers run down, as float stores.
    const float check = static_cast<float>(static_cast<double>(s.check_timer_10) -
                                           static_cast<double>(dt));
    s.check_timer_10 = check;
    s.delay_timer_14 = static_cast<float>(static_cast<double>(s.delay_timer_14) -
                                          static_cast<double>(dt));
    // 009FD156-009FD181: the cached desc, else [task+3F4h]'s row.
    const OpportunisticRocketRow* row = host.robot_row();
    if (row == nullptr) return out;

    if (0.0f > check) {   // 009FD187 COMISS / JBE
        out.decided = true;
        s.salvo_left_0c = 0;
        s.check_timer_10 = rng.uniform_00bd2f10(kOpportunisticRocketCheckKey,
                                                row->check_time_lo_140, row->check_time_hi_144);
        s.class_00 = OpportunisticRocketClass::kNone;
        if (s.target_plus_one_38 == 0) return out;   // 009FD1B8 / 009FD1C4
        const float r = rng.uniform_00bd2f10(kOpportunisticRocketChanceKey, k::kChanceLow,
                                             k::kChanceHigh);
        s.class_00 = opportunistic_rocket_class_for_target(host.target_kinds());
        if (s.class_00 == OpportunisticRocketClass::kNone) return out;   // 009FD256
        const OpportunisticRocketClassRow& c = row->classes[static_cast<std::size_t>(s.class_00)];
        // 009FD278 FCOMIP(chance, r) / JBE.
        if (c.rocket_chance_00 > r) {
            s.salvo_left_0c = c.num_rockets_20;
            out.armed = true;
        }
    }
    // 009FD284-009FD297.
    if (s.class_00 == OpportunisticRocketClass::kNone) return out;
    const OpportunisticRocketClassRow& c = row->classes[static_cast<std::size_t>(s.class_00)];
    // 009FD29D-009FD2C5.
    if (!(0.0f > s.delay_timer_14)) return out;
    if (s.salvo_left_0c == 0) return out;
    if (s.target_plus_one_38 == 0) return out;
    if (!host.target_alive_visible_0043f080()) return out;
    // 009FD2CB-009FD2F0: re-find a rocket unless the held one is still on the plane.
    if (s.rocket_plus_one_58 == 0 || !host.rocket_attached_00923810(s.rocket_plus_one_58)) {
        s.rocket_plus_one_58 = host.find_aim_rocket_007c1eb0(s.target_is_plane_3c);
    }
    if (s.rocket_plus_one_58 != 0 && host.rocket_attached_00923810(s.rocket_plus_one_58)) {
        out.evaluated = true;
        const std::size_t rocket = s.rocket_plus_one_58;
        // 009FD30F-009FD347: range to the target now and the rocket's time over it.
        const std::array<float, 3> target = host.target_position();
        const std::array<float, 3> unit = host.unit_position();
        const std::array<float, 3> sep{unit[0] - target[0], unit[1] - target[1],
                                       unit[2] - target[2]};   // 00427C90
        const float range0 = force_event_vector_length_0042b2f0(sep);
        const float time0 = host.rocket_flight_time_00809e10(rocket, range0);
        // 009FD34E-009FD39D: lead = time0 * velocity, aim = target + lead (float stores).
        const std::array<float, 3> v = host.target_velocity_vt34();
        std::array<float, 3> aim{};
        for (std::size_t i = 0; i < 3; ++i) {
            const float lead = static_cast<float>(static_cast<double>(time0) *
                                                  static_cast<double>(v[i]));
            aim[i] = static_cast<float>(static_cast<double>(target[i]) +
                                        static_cast<double>(lead));
        }
        // 009FD3A1-009FD3D4: into the plane's frame, then range and time again.
        OpportunisticRocketGateInputs g;
        g.body_aim = host.world_to_body(aim);
        g.body_range = force_event_vector_length_0042b2f0(g.body_aim);
        g.flight_time = host.rocket_flight_time_00809e10(rocket, g.body_range);
        // 009FD3D8 009F99B0(target): the slot is chosen by the target now, not
        // by the class row (a re-target between decisions can read slot 0.0).
        const OpportunisticRocketTargetKinds kinds = host.target_kinds();
        g.attack_dist = c.attack_dist_04[static_cast<std::size_t>(
            opportunistic_rocket_range_slot_009f99b0(&kinds))];
        g.salvo_left = s.salvo_left_0c;
        out.gate = opportunistic_rocket_fire_gate(g, c);
        // A failed gate returns at 009FD548 with the rocket ref still held.
        if (out.gate != OpportunisticRocketGate::kPass) return out;
        // 009FD4E3-009FD503: homing on the held rocket with the +18h point, then
        // the plane's release request.
        host.set_homing_target_0080aa20(rocket, s.target_plus_one_38, s.aim_point_18,
                                        c.homing_angle_2c);
        host.release_ordnance_007bbba0();
        out.released = true;
        // 009FD508-009FD526: one fewer; the delay is max(RocketDelay, 0.08).
        s.salvo_left_0c -= 1;
        const float floor = k::kMinRocketDelay;
        s.delay_timer_14 = max_native_float_by_ref_00415550(&c.rocket_delay_24, &floor);
    }
    // 009FD529-009FD53B: the rocket ref is dropped after a release or a miss.
    s.rocket_plus_one_58 = 0;
    return out;
}

}   // namespace bsp
