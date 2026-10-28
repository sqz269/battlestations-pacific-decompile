// The kamikaze and drop-kamikaze bot tasks, 009AB740-009AF830. Packet
// cc13-kamikaze1; evidence in docs/CC13_KAMIKAZE_TASK.md. Names are hypotheses.
// Comments cite the instruction each line reproduces; constants are read from
// the image at their load width (qword -> double, dword -> float).

#include "bsp/kamikaze_task_cc13.hpp"

#include <cmath>

#include "bsp/bot_tasks.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/sw1b_switch_reach.hpp"

namespace bsp {

namespace {

// Image constants (address, width).
constexpr double kHalfPiD = 1.5707963705062866;    // 00CE3830 qword
constexpr double kTwoPiD = 6.2831854820251465;     // 00CE3828 qword
constexpr float kPiF = 3.14159274f;                // 00D7A264 dword
constexpr double kDistEpsD = 1e-10;                // 00CE3820 qword
constexpr float kNegZeroF = -0.0f;                 // 00D7A208 dword

float to_f(double v) { return static_cast<float>(v); }

using cc13x87::x87_add24;
using cc13x87::x87_div24;
using cc13x87::x87_mul24;
using cc13x87::x87_sqrt24;
using cc13x87::x87_sub24;

// 009AC0B0 / 009AC10B-009AC13B: |p - unit| in 3-D, the squared sum rounded to
// float, 0 at or below 1e-10.
float distance3_009ac0b0(const std::array<float, 3>& p, const std::array<float, 3>& u) {
    const float dx = to_f(static_cast<double>(p[0]) - u[0]);
    const float dy = to_f(static_cast<double>(p[1]) - u[1]);
    const float dz = to_f(static_cast<double>(p[2]) - u[2]);
    float d2;
    if constexpr (kKamikazeX87FloatCc13Bound) {
        // 009AC10B-009AC119: dz^2 + (dx^2 + dy^2), each step rounded to float
        // (24-bit x87 on the main thread).
        d2 = to_f(static_cast<double>(to_f(static_cast<double>(dz) * dz)) +
                  to_f(static_cast<double>(to_f(static_cast<double>(dx) * dx)) +
                       to_f(static_cast<double>(dy) * dy)));
    } else {
        d2 = to_f(static_cast<double>(dy) * dy + static_cast<double>(dx) * dx +
                  static_cast<double>(dz) * dz);
    }
    // pc24-lint: allow sw1b reach counter, replica of the OFF arm (log only)
    sw1b_note(Sw1bReach::KamikazeX87Float,                               // sw1b reach
              to_f(static_cast<double>(to_f(static_cast<double>(dz) * dz)) +
                   to_f(static_cast<double>(to_f(static_cast<double>(dx) * dx)) +
                        to_f(static_cast<double>(dy) * dy))),
              to_f(static_cast<double>(dy) * dy + static_cast<double>(dx) * dx +
                   static_cast<double>(dz) * dz));
    if (!(static_cast<double>(d2) > kDistEpsD)) return 0.0f;
    return to_f(std::sqrt(static_cast<double>(d2)));
}

// 0042B2F0 on (dx, 0, dz): the planar length.
float planar_length(float dx, float dz) {
    float len;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 0042B2F9-0042B309: (y*y + x*x) + z*z with y = 0, each op at PC24, then FSTP
        // DWORD; 0042B30C-0042B319: +0 at or below the double 1e-10 (00CE3820, unordered
        // included); else 00BF7030's FSQRT at PC24 and FSTP DWORD.
        const float d2 = to_f(x87_add24(x87_mul24(dx, dx), x87_mul24(dz, dz)));
        len = (static_cast<double>(d2) > kDistEpsD) ? to_f(x87_sqrt24(d2)) : 0.0f;
    } else {
        len = to_f(std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz));
    }
    return len;
}

// pi/2 - atan2(dz, dx), wrapped into [0, 2pi): 009AC228-009AC25C, 009ADEBB-009ADEF8.
float bearing_to(float dx, float dz) {
    const float r = to_f(std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    float h = to_f(kHalfPiD - r);
    if (0.0f > h) h = to_f(static_cast<double>(h) + kTwoPiD);
    return h;
}

// The [-1, 1] clamp 009ACA6D-009ACA89 and its three siblings: -1 first, then +1.
float clamp_unit(float v) {
    if (-1.0f > v) return -1.0f;
    return (v > 1.0f) ? 1.0f : v;
}

float interp(float x0, float y0, float x1, float y1, float x) {
    return dive_bomb_interpolate_clamped_00419010(x0, y0, x1, y1, x);
}

void throttle_full(KamikazePlanCommand& c) {
    // plan+278h = 1.0 [00D7A24C], +27Ch = 1, +2A8h = 0.0, +2ACh = 1, +2D8h = 0.
    c.writes_throttle_slots = true;
}

}  // namespace

bool kamikaze_rocket_like(bool unit_kind_17, bool pilot_fires_c24) noexcept {
    return unit_kind_17 && !pilot_fires_c24;   // 009AB94C-009AB959
}

const char* kamikaze_state_name(KamikazeState s) noexcept {
    switch (s) {
        case KamikazeState::kMoveTo: return "moveto";
        case KamikazeState::kFollow: return "follow";
        case KamikazeState::kPrepare: return "prepare";
        case KamikazeState::kGoTowards: return "gotowards";
        case KamikazeState::kAim: return "aim";
        default: return "none";
    }
}

// ---------------------------------------------------------------------------
// Kamikaze approach
// ---------------------------------------------------------------------------

void kamikaze_approach_redraw_009ab850(KamikazeApproach& ap, float ctl_alt_cruise_394,
                                       float ctl_alt_second_398, KamikazeRandom& rng) {
    // 009AB854-009AB865: +48h = +34h * +24h.
    ap.attack_range_48 = to_f(static_cast<double>(ap.profile.attack_range_34) * ap.speed_ratio_24);
    // 009AB885: U(-40 [00D1FAF0], 80 [00CE5444]) + ctl+398h; below 15.0 (qword
    // 00CF3F20) it is 15.0 [00CE5380].
    float a = to_f(static_cast<double>(rng.uniform_00bd2f10("4c", -40.0f, 80.0f)) +
                   ctl_alt_second_398);
    if (15.0 > static_cast<double>(a)) a = 15.0f;
    ap.climb_alt_4c = a;
    // 009AB8DF: U(-80 [00D1FAEC], 80) + ctl+394h; below 20.0 (qword 00CE3D88) it is 20.0.
    float c = to_f(static_cast<double>(rng.uniform_00bd2f10("50", -80.0f, 80.0f)) +
                   ctl_alt_cruise_394);
    if (20.0 > static_cast<double>(c)) c = 20.0f;
    ap.cruise_alt_50 = c;
}

void kamikaze_approach_seed_009ab920(KamikazeApproach& ap, const KamikazeSeedInputs& in,
                                     KamikazeRandom& rng) {
    const bool rocket = kamikaze_rocket_like(in.unit_kind_17, in.pilot_fires_c24);
    const KamikazeProfile& p = rocket ? in.rocket_like : in.fighter_like;
    // 009AB95B/009AB96A: tuning+474h or +490h, the variant's ReferenceSpeed, into 009F9CE0.
    // Documented deviation (cc13-D1): 009F9CE0 divides with no guard (a zero
    // reference gives +inf); the guard is host glue for unloaded tuning, since
    // the authored reference and class speeds are positive. 009F9CE0's head
    // caches (unit, +538h, +9D4h, +DF4h, the robots row, +28h = -1.0), the
    // vtable 00D1FAF4 and the 009FB200 construct are the host's (KamikazeApproach
    // models the fields this window reads).
    ap.speed_ratio_24 = (p.reference_speed_44 > 0.0f && in.class_speed_188 > 0.0f)
        ? bot_task_speed_ratio(in.class_speed_188, p.reference_speed_44) : 1.0f;
    // 009AB98D-009AB99E: +48h = +24h * 1200.0 (qword 00D1FAF8); 009AB850 overwrites it.
    ap.attack_range_48 = to_f(static_cast<double>(ap.speed_ratio_24) * 1200.0);
    ap.in_range_5c = false;                    // 009AB9A8
    ap.proj_time_error_64 = 0.0f;              // 009AB9B4
    ap.rocket_like_5d = rocket;                // 009AB9E4
    ap.profile = p;                            // 009AB9E9-009ABA09, REP MOVSD of 7 dwords
    kamikaze_approach_redraw_009ab850(ap, in.ctl_alt_cruise_394, in.ctl_alt_second_398, rng);
}

void kamikaze_aim_error_009abf60(KamikazeApproach& ap, const KamikazeRobotRow& row,
                                 KamikazeRandom& rng) {
    ApproachTargetRefState& s = ap.ref;
    // 009ABF6A-009ABF7F: sub+48h/+4Ch/+50h = row+90h (TargetPrecision); +41h = 1.
    s.spread_48 = {row.target_precision_90, row.target_precision_90, row.target_precision_90};
    s.dirty_41 = true;
    // Lower bounds are -0.0 - x (SUBSS from 00D7A208).
    const float dh = rng.uniform_00bd2f10("h", kNegZeroF - row.target_h_error_94,
                                          row.target_h_error_94);       // 009ABFB2
    const float dv = rng.uniform_00bd2f10("v", kNegZeroF - row.target_v_error_98,
                                          row.target_v_error_98);       // 009ABFE5
    ap.proj_time_error_64 = rng.uniform_00bd2f10("64", kNegZeroF - row.proj_time_error_9c,
                                                 row.proj_time_error_9c);   // 009AC018
    // 009AC020-009AC037: sub+54h = dv, +58h = 0, +5Ch = dh (the centre, in this
    // order; the bias below takes them the other way round).
    s.centre_54 = {dv, 0.0f, dh};
    s.dirty_41 = true;                          // 009AC03A
    // 009AC041-009AC081: sub+64h = row+80h, +68h/+6Ch/+70h = row+84h/88h/8Ch.
    s.section_chance_64 = row.section_damage_chance_80;
    s.weight_68 = row.engine_room_weight_84;
    s.weight_6c = row.magazine_weight_88;
    s.weight_70 = row.fueltank_weight_8c;
    // 009AC092: 009FA380(sub, (dh, 0, dv)): bias = it, dirty = 1. 009AC09E then
    // tail-jumps to 009FA260, the pick. Not made here (cc13-D1 follow-up,
    // blocked): KzHost (game_hosts_units.cpp) has no hook to pick at enter time,
    // and kz_install_009af720 calls 009AC160 directly. The pick happens one call
    // later instead: the host's next 009FADA0 (kz_target_point) copies the
    // dirty ref into the unit's hull aim ref and marks it dirty, and
    // aim_point_009fada0 -> hull_aim_world_point runs
    // approach_target_ref_pick_009fa260 on that dirty byte. cc13-kzpick: under
    // kKamikazeEnterPickCc13Bound both 009AC160 call sites (kamikaze_enter and
    // kz_install_009af720) call KamikazeTaskHost::pick_target_ref_009fa260 next,
    // which KzHost overrides, so the pick is made at enter as here.
    s.bias_34 = {dh, 0.0f, dv};
}

std::array<float, 3> kamikaze_aim_point_009aba30(const std::array<float, 3>& ref_point,
                                                 float eta_60) noexcept {
    std::array<float, 3> out = ref_point;          // 009ABA30-009ABA5B
    if (1.0f > out[1]) out[1] = 1.0f;               // 009ABA46/009ABA60, COMISS 1.0
    // 009ABA65-009ABA9C: + 00419010(8.0 [00CE3918], 40.0 [00CE685C], 2.0 [00CE3958], 0, +60h).
    out[1] = to_f(static_cast<double>(interp(8.0f, 40.0f, 2.0f, 0.0f, eta_60)) + out[1]);
    return out;
}

KamikazeUpdateResult kamikaze_approach_update_009af0a0(KamikazeApproach& ap,
                                                       const KamikazeUpdateInputs& in) {
    // Documented deviation (cc13-D1): the image clears the gun block
    // [+1Ch]+34h/+38h and then runs 009FADA0(dt) between the +58h store and
    // +5Ch; here both are the caller's (kamikaze_task_arm_009af510), 009FADA0
    // first. The order is unobservable in the live host, whose gun-block clear
    // is empty; the unit pose refresh is inside the host's position query.
    KamikazeUpdateResult r;
    ap.band_58 = 1;                                  // 009AF0B2
    ap.in_range_5c = ap.rocket_like_5d;              // 009AF0D8-009AF0DB
    if (!in.has_target) return r;                    // 009AF0DE
    if (in.target_kind_0e) {                         // 009AF0E9-009AF103
        ap.ref.spread_48 = {0.0f, 0.0f, 0.0f};
        ap.ref.dirty_41 = true;
    }
    // 009AF127-009AF155: planar distance to the 009ABA30 point (0042B2F0 on (dx, 0, dz)).
    const float dx = to_f(static_cast<double>(in.aim_point[0]) - in.unit_position[0]);
    const float dz = to_f(static_cast<double>(in.aim_point[2]) - in.unit_position[2]);
    ap.planar_dist_54 = planar_length(dx, dz);
    // 009AF160-009AF186: forward speed, 10.0 [00CE38B8] below 10.0 (qword 00CE3DC0).
    float s = in.forward_speed_b2c;
    if (10.0 > static_cast<double>(s)) s = 10.0f;
    // 009AF19F-009AF1A5: +60h = +54h / s.
    ap.eta_60 = to_f(static_cast<double>(ap.planar_dist_54) / s);
    // 009AF1A8-009AF1BF: clamp to [2.0, 5.0], then 009AF1C2-009AF1E5 to [0, 30].
    float t = 2.0f;
    if (!(2.0f > ap.eta_60)) {
        t = 5.0f;
        if (!(ap.eta_60 > 5.0f)) t = ap.eta_60;
    }
    if (0.0f > t) t = 0.0f;
    else if (static_cast<double>(t) > 30.0) t = 30.0f;
    ap.ref.projtime_44 = t;                          // 009AF1F1, approach+ACh
    // 009AF1ED-009AF20A: a fighter-like plane is in range below AttackRange.
    if (!ap.rocket_like_5d) ap.in_range_5c = ap.attack_range_48 > ap.planar_dist_54;
    return r;
}

// ---------------------------------------------------------------------------
// gotowards
// ---------------------------------------------------------------------------

void kamikaze_gotowards_enter_009ac160(KamikazeGoTowardsState& st, KamikazeApproach& ap,
                                       const KamikazeRobotRow& row, float ctl_alt_cruise_394,
                                       float ctl_alt_second_398, KamikazeRandom& rng) {
    st.turnaround_1d = false;                       // 009AC16B
    st.aligned_1c = false;                          // 009AC16E
    st.pitch_20 = 0.0f;                             // 009AC171
    kamikaze_approach_redraw_009ab850(ap, ctl_alt_cruise_394, ctl_alt_second_398, rng);  // 009AC176
    kamikaze_aim_error_009abf60(ap, row, rng);      // 009AC17F
}

KamikazePlanCommand kamikaze_gotowards_tick_009ac190(KamikazeGoTowardsState& st,
                                                     KamikazeApproach& ap,
                                                     const KamikazeGoTowardsInputs& in) {
    KamikazePlanCommand c;
    const bool rocket = in.rocket_like;                                  // BL, 009AC1A3-009AC1BA
    const std::array<float, 3>& p = in.aim_point;                        // 009AC1C8
    const float dx = to_f(static_cast<double>(p[0]) - in.unit_position[0]);   // 009AC1E0
    const float dz = to_f(static_cast<double>(p[2]) - in.unit_position[2]);   // 009AC1F1
    float d = distance3_009ac0b0(p, in.unit_position);                  // 009AC1FF
    if (10.0 > static_cast<double>(d)) d = 10.0f;                        // 009AC20C, 00CE38B8
    st.dist_18 = d;
    const float h_target = bearing_to(dx, dz);                           // [ESP+18h]
    const float err = wrapped_angle_subtract_00438b10(h_target, in.heading_50);   // 009AC28B
    const float abs_err = (err > 0.0f) ? err : kNegZeroF - err;          // 009AC294-009AC2B0
    const float d02 = to_f(static_cast<double>(st.dist_18) * 0.20000000298023224);  // 00CE3D10
    // 009AC2E2-009AC301: unit y (as a double) minus the aim point's y.
    const float h = to_f(static_cast<double>(in.unit_position[1]) - p[1]);
    // 009AC305-009AC33D: R = (k * |err| + 3.0 [00D7A2B0]) * class+188h.
    const float k = rocket ? 1.2f : 0.8f;                                // 00CE3814 / 00CE74F8
    float turn_dist;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009AC329-009AC33D: FMUL, FADD qword 3.0, FMUL, each rounded at PC24.
        turn_dist = to_f(x87_mul24(x87_add24(x87_mul24(k, abs_err), 3.0), in.class_speed_188));
    } else {
        turn_dist = to_f((static_cast<double>(k) * abs_err + 3.0) * in.class_speed_188);
    }
    float heading = h_target;
    if (st.turnaround_1d) {
        // 009AC347-009AC35E: hold the turnaround while dist <= R + 50.0 (qword 00CE3938).
        bool hold;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009AC350 FADD rounds R + 50.0 at PC24 before 009AC358 FCOMIP.
            hold = static_cast<double>(st.dist_18) <= x87_add24(turn_dist, 50.0);
        } else {
            hold = static_cast<double>(st.dist_18) <= static_cast<double>(turn_dist) + 50.0;
        }
        if (hold)
            heading = wrapped_angle_add_00438aa0(h_target, kPiF);       // 009AC3EC
        else
            st.turnaround_1d = false;
    } else {
        // 009AC367-009AC38F: m = (h - 100.0 [00D7A220 qword]) * (1.5 or 1.0).
        const float f = rocket ? 1.5f : 1.0f;                            // 00CE380C / 00D7A24C
        float m;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009AC389 FSUBR ST1 (h - 100.0), 009AC38B FMUL, each at PC24.
            m = to_f(x87_mul24(x87_sub24(h, 100.0), f));
        } else {
            m = to_f((static_cast<double>(h) - 100.0) * f);
        }
        if (!(m <= turn_dist)) m = turn_dist;                            // 009AC39F
        if (m > st.dist_18) {                                            // 009AC3C0
            // 009AC3C2-009AC3CE: a rocket-like plane turns only when h >= 0.2 d.
            if (!(rocket && d02 > h)) {
                st.turnaround_1d = true;                                 // 009AC3D4
                heading = wrapped_angle_add_00438aa0(h_target, kPiF);
            }
        }
    }
    c.writes_heading_2c0 = true;                                         // 009AC40B, +2CCh = 2
    c.heading_2c0 = heading;
    c.writes_turn_scale_2e8 = true;                                      // 009AC42B
    c.turn_scale_2e8 = 1.2f;
    // 009AC433-009AC455: the direct pitch, atan2(-h, d), kept in +20h.
    const float pitch_direct = to_f(std::atan2(-static_cast<double>(h),
                                               static_cast<double>(st.dist_18)));
    st.pitch_20 = pitch_direct;
    float pitch = pitch_direct;
    // 009AC4BF-009AC4DA: the turbo range test, d > +3Ch + 10.0.
    bool beyond_turbo = false;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009AC4CE FADD qword 10.0 rounds at PC24 before 009AC4D6 FCOMI.
        beyond_turbo = static_cast<double>(st.dist_18) >
                       x87_add24(ap.profile.turbo_range_3c, 10.0);
    } else {
        beyond_turbo = static_cast<double>(st.dist_18) >
                       static_cast<double>(ap.profile.turbo_range_3c) + 10.0;
    }
    if (st.turnaround_1d) {
        // 009AC45B-009AC4BA: climb to +4Ch over +48h, clamped to [-30, 20] degrees.
        const float rise = to_f(static_cast<double>(ap.climb_alt_4c) - h);
        const float v = to_f(std::atan2(static_cast<double>(rise),
                                        static_cast<double>(ap.attack_range_48)));
        if (-0.5235988f > v) pitch = -0.5235988f;                       // 00CEC728
        else if (v > 0.34906587f) pitch = 0.34906587f;                  // 00CE398C
        else pitch = v;
    } else if (beyond_turbo) {   // 009AC4BF-009AC4DA

        // 009AC4E0-009AC4F6: e = h - d / +48h * +4Ch.
        float q;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009AC4E8 FDIV, 009AC4EB FMUL, each at PC24, 009AC4EE FSTP DWORD.
            q = to_f(x87_mul24(x87_div24(st.dist_18, ap.attack_range_48), ap.climb_alt_4c));
        } else {
            q = to_f(static_cast<double>(st.dist_18) / ap.attack_range_48 *
                     ap.climb_alt_4c);
        }
        const float e = to_f(static_cast<double>(h) - q);
        // 009AC4FE-009AC548: 0.6 (00CEFF98) when e >= 0, else 0.8 (00CE3D40).
        const double kk = (0.0f <= e) ? 0.6000000238418579 : 0.800000011920929;
        float y;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009AC504/009AC527 FMUL qword, 009AC50A/009AC52D FADDP (+ h), each at
            // PC24, then FCHS and FSTP DWORD.
            y = to_f(-x87_add24(x87_mul24(e, kk), h));
        } else {
            y = to_f(-(static_cast<double>(e) * kk + h));
        }
        const float p2 = to_f(std::atan2(static_cast<double>(y),
                                         static_cast<double>(st.dist_18)));
        // 009AC54C-009AC5E1: clamp p2 into [direct - w, direct + a], a and w ramps
        // of d / TurboRange: a = 00419010(0.8, 0, 1.5, 15 deg), w = 00419010(1, 0, 2, 20 deg).
        const float u = to_f(static_cast<double>(st.dist_18) / ap.profile.turbo_range_3c);
        const float a = interp(0.8f, 0.0f, 1.5f, 0.2617994f, u);        // 00D05AA8
        const float hi = to_f(static_cast<double>(pitch_direct) + a);
        const float w = interp(1.0f, 0.0f, 2.0f, 0.34906587f, u);
        const float lo = to_f(static_cast<double>(pitch_direct) - w);
        pitch = unit_clamp_00415690(p2, lo, hi);                         // 009AC5E1
    }
    if (rocket) {
        // 009AC5EE-009AC65D: the slow rocket cannot climb: min with
        // 00419010(0.5, -25 deg, 1.1, 5 deg, |v| / LevelFlight).
        const float s = to_f(static_cast<double>(in.speed_b1c) / in.level_flight_speed);
        const float lim = interp(0.5f, -0.43633232f, 1.1f, 0.08726647f, s);
        if (!(lim > pitch)) pitch = lim;
    }
    c.writes_pitch_target_2bc = true;                                    // 009AC674
    c.pitch_target_2bc = pitch;
    c.pitch_mode_2d0 = 1;                                                // 009AC67C
    // 009AC682-009AC70E: aligned when not turning, |err| < 25 deg (qword 00D1F6C0)
    // and wrap(pitch - unit pitch) < 20 deg (signed).
    bool aligned = false;
    if (!st.turnaround_1d && 0.4363323152065277 > static_cast<double>(abs_err)) {
        aligned = 0.34906587f > wrapped_angle_subtract_00438b10(pitch, in.pitch_c64);
    }
    st.aligned_1c = aligned;
    throttle_full(c);                                                    // 009AC714-009AC735
    if (!rocket && st.turnaround_1d) c.boost_2e4 = true;                // 009AC73F-009AC74C
    // 009AC753-009AC7AD: the range band.
    if (ap.attack_range_48 > st.dist_18) ap.band_58 = 1;
    else if ((rocket ? 3000.0f : 2000.0f) > st.dist_18) ap.band_58 = 3;  // 00CFA424 / 00CFFD60
    else ap.band_58 = 0xFF;
    return c;
}

bool kamikaze_gotowards_ready_009aefc0(const KamikazeGoTowardsState& st,
                                       const KamikazeApproach& ap, bool rocket_like) noexcept {
    if (!st.aligned_1c) return false;                                    // 009AEFC5
    if (!(static_cast<double>(ap.attack_range_48) + 2000.0 >             // 00CF0DD8 qword
          static_cast<double>(st.dist_18))) return false;
    if (ap.profile.turbo_range_3c > st.dist_18) return true;             // 009AEFE6-009AEFED
    if (!rocket_like) return false;                                      // 009AEFF7-009AF008
    return st.pitch_20 > -ap.profile.turbo_angle_40;                     // 009AF00A-009AF01B
}

// ---------------------------------------------------------------------------
// aim
// ---------------------------------------------------------------------------

void kamikaze_aim_enter_009ac8d0(KamikazeAimState& st, float roll_c68) noexcept {
    st.locked_18 = false;
    st.window_time_1c = 0.0f;
    st.precision_timer_20 = 0.0f;
    st.bank_side_19 = roll_c68 < 0.0f;     // "<= 0 and != 0"
    st.precision_mul_24 = 1.0f;            // 00D7A24C
}

KamikazePlanCommand kamikaze_aim_tick_009ac910(KamikazeAimState& st, KamikazeApproach& ap,
                                               const KamikazeRobotRow& row,
                                               const KamikazeAimInputs& in, KamikazeRandom& rng) {
    KamikazePlanCommand c;
    const bool no_fire = !in.pilot_fires_c24;                            // 009AC999 SETE
    ap.band_58 = 0;                                                      // 009AC983
    // 009AC96E-009AC9CA: forward depth floored at 30 (qword 00CE7630 test, 00CE38C8 store).
    float zc = in.aim_body[2];
    if (30.0 > static_cast<double>(zc)) zc = 30.0f;
    const float u = to_f(static_cast<double>(in.aim_body[0]) / zc);     // [ESP+1Ch]
    const float v = to_f(static_cast<double>(in.aim_body[1]) / zc);     // [ESP+20h]
    // 009AC9CE-009ACA38: the precision timer.
    const float t = to_f(static_cast<double>(st.precision_timer_20) - in.dt);
    st.precision_timer_20 = t;
    if (0.0f > t) {
        st.precision_timer_20 = to_f(static_cast<double>(
            rng.uniform_00bd2f10("t", 0.8f, 1.25f)) * row.maneuver_timer_a8);   // 009ACA03
        st.precision_mul_24 = rng.uniform_00bd2f10("mul", row.maneuver_mul_lo_a0,
                                                   row.maneuver_mul_hi_a4);     // 009ACA2F
    }
    const double mul = st.precision_mul_24;
    c.writes_yaw_slot_284 = true;
    c.writes_pitch_slot_29c = true;
    if (st.locked_18) {
        // 009ACA54-009ACAEC: gains 36 (00D1FBC8) and 24 (00CE4790), qwords.
        const float side = st.bank_side_19 ? 1.0f : -1.0f;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009ACA54 FMUL qword 36, 009ACA62 FMUL +24h; 009ACA93 FMUL qword 24,
            // 009ACAB0 FMUL +24h; each at PC24.
            c.yaw_284 = clamp_unit(to_f(x87_mul24(x87_mul24(u, 36.0), mul)));
            c.pitch_29c = clamp_unit(to_f(x87_mul24(x87_mul24(v, 24.0), mul)));
        } else {
            c.yaw_284 = clamp_unit(to_f(static_cast<double>(u) * 36.0 * mul));
            c.pitch_29c = clamp_unit(to_f(static_cast<double>(v) * 24.0 * mul));
        }
        // 009ACAF2-009ACB5F: bank = max(base - eta, 1) * side * 0.06 (qword 00CF0AC0).
        float b = to_f(static_cast<double>(no_fire ? 12.0f : 5.0f) - ap.eta_60);
        if (1.0f > b) b = 1.0f;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009ACB55 FMUL side, 009ACB59 FMUL qword 0.06, each at PC24.
            c.bank_2c4 = to_f(x87_mul24(x87_mul24(b, side), 0.05999999865889549));
        } else {
            c.bank_2c4 = to_f(static_cast<double>(b) * side * 0.05999999865889549);
        }
    } else {
        // 009ACB6A-009ACC07: gains 20 (00CE3D88) and 15 (00CF3F20); bank 0.
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009ACB6A FMUL qword 20, 009ACB78 FMUL +24h; 009ACBA9 FMUL qword 15,
            // 009ACBC6 FMUL +24h; each at PC24.
            c.yaw_284 = clamp_unit(to_f(x87_mul24(x87_mul24(u, 20.0), mul)));
            c.pitch_29c = clamp_unit(to_f(x87_mul24(x87_mul24(v, 15.0), mul)));
        } else {
            c.yaw_284 = clamp_unit(to_f(static_cast<double>(u) * 20.0 * mul));
            c.pitch_29c = clamp_unit(to_f(static_cast<double>(v) * 15.0 * mul));
        }
        c.bank_2c4 = 0.0f;
    }
    c.writes_bank_2c4 = true;                                            // 009ACC0F, +2CCh = 1
    // 009ACC15-009ACC54: the bank side latch, 20 degree hysteresis.
    if (st.bank_side_19) {
        if (in.roll_c68 > 0.34906587f) st.bank_side_19 = false;
    } else if (-0.34906587f > in.roll_c68) {
        st.bank_side_19 = true;
    }
    throttle_full(c);                                                    // 009ACC5F-009ACC7B
    // 009ACC82-009ACCF6: the lock window, widened below 1000 m.
    float w = no_fire ? 0.025f : 0.035f;                                 // 00D1FBBC / 00D1F3C8
    const float d = in.distance;
    if (1000.0 > static_cast<double>(d)) {
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009ACCCE/009ACCE8 FDIV (1000 / max(d, 200)), 009ACCD2/009ACCEC FMUL w,
            // each at PC24.
            const float den = (200.0 > static_cast<double>(d)) ? 200.0f : d;
            w = to_f(x87_mul24(x87_div24(1000.0, den), w));
        } else {
            if (200.0 > static_cast<double>(d)) w = to_f(1000.0 / 200.0f * w);
            else w = to_f(1000.0 / d * w);
        }
    }
    // 009ACCF8-009ACD30: inside TurboRange and inside the window on both axes.
    const float neg_w = -w;
    const bool inside = ap.profile.turbo_range_3c > d && w > u && u > neg_w && w > v &&
                        v > neg_w;
    if (!inside) {
        st.window_time_1c = 0.0f;                                        // 009ACD7E
        return c;
    }
    if (!st.locked_18) {
        const float tw = to_f(static_cast<double>(st.window_time_1c) + in.dt);
        st.window_time_1c = tw;
        if (tw > 3.0f) st.locked_18 = true;                              // 00CE3854
        if (!st.locked_18) return c;
    }
    c.boost_2e4 = true;                                                  // 009ACD66
    return c;
}

// ---------------------------------------------------------------------------
// Kamikaze rule, profile, validity
// ---------------------------------------------------------------------------

KamikazeState kamikaze_initial_state_009aeef0(bool flight_leader, bool rocket_like) noexcept {
    if (!flight_leader) return KamikazeState::kFollow;                   // +524h
    return rocket_like ? KamikazeState::kGoTowards : KamikazeState::kMoveTo;
}

KamikazeState kamikaze_rule_009af310(const KamikazeRuleInputs& in) noexcept {
    // 009AF34A-009AF370: give up unless rocket-like, or the target is alive and
    // either in range or the attack mode is 2.
    const bool abandon = !in.rocket_like_455 &&
        (!in.has_target_478 || in.target_marked_5d ||
         (!in.in_range_454 && in.attack_mode_370 != 2));
    const KamikazeState formation = in.flight_leader
        ? (in.rocket_like_455 ? KamikazeState::kGoTowards : KamikazeState::kMoveTo)
        : KamikazeState::kFollow;
    const bool attacking = in.current == KamikazeState::kAim ||
                           in.current == KamikazeState::kGoTowards ||
                           in.current == KamikazeState::kPrepare;
    if (attacking) {
        if (abandon) return formation;                                   // 009AF372-009AF3C6
        if (in.current == KamikazeState::kGoTowards && in.gotowards_ready)
            return KamikazeState::kAim;                                  // 009AF3CE-009AF3EC
        return in.current;
    }
    // 009AF3F7-009AF470: from moveto/follow straight to gotowards.
    return abandon ? formation : KamikazeState::kGoTowards;
}

KamikazeAltitudeWrite kamikaze_cruise_profile_009af480(const KamikazeApproach& ap,
                                                       const KamikazeAltitudeChannels& ch) noexcept {
    KamikazeAltitudeWrite w;
    if (!ch.cruise_locked_38d) {
        w.clears_one_shot_3a9 = true;                                    // 009AF4B7
        PilotAltitudeChannel a{false, ch.cruise_current_380, ch.cruise_one_shot_3a9};
        if (bot_task_altitude_write_applies(a)) {
            w.writes_cruise_394 = true;
            w.cruise_394 = ap.profile.cruising_alt_2c;                   // task+424h
            w.dirty_3ad = true;
        }
    }
    if (!ch.second_locked_38c) {
        w.clears_one_shot_3aa = true;                                    // 009AF4E0
        PilotAltitudeChannel a{false, ch.second_current_37c, ch.second_one_shot_3aa};
        if (bot_task_altitude_write_applies(a)) {
            w.writes_second_398 = true;
            w.second_398 = ap.profile.attack_alt_38;                     // task+430h
            w.dirty_3ad = true;
        }
    }
    return w;
}

void kamikaze_task_cruise_profile_009af480(KamikazeTask& task,
                                           KamikazeCruiseProfileTaskHost& host,
                                           KamikazeRandom& rng) {
    // 009AF484: 0099B660, the base, is a bare RET.
    host.apply_altitude_write(
        kamikaze_cruise_profile_009af480(task.ap, host.altitude_channels()));  // 009AF489-009AF4E0
    kamikaze_approach_redraw_009ab850(task.ap, host.ctl_alt_cruise_394(),
                                      host.ctl_alt_second_398(), rng);        // 009AF4EB
    KamikazeUpdateInputs ui = host.approach_update_inputs_009af0a0(task.ap, 0.0f);  // 009AF4F8
    ui.aim_point = kamikaze_aim_point_009aba30(ui.aim_point, task.ap.eta_60);
    kamikaze_approach_update_009af0a0(task.ap, ui);
    host.abandon_if_stale_0099b740();                                    // 009AF501 JMP
}

int kamikaze_command_still_valid(const KamikazeCommandCheck& in,
                                 std::uint32_t own_class) noexcept {
    if (!in.has_control_block) return 2;
    if (!in.control_has_command) return 0;
    if (in.current_command != own_class && in.current_command != kAttackAnyCommandClass)
        return 0;
    if (in.latched_target == nullptr || in.command_target == in.latched_target) return 1;
    return 0;
}

// ---------------------------------------------------------------------------
// Kamikaze driver
// ---------------------------------------------------------------------------

namespace {

bool is_formation(KamikazeState s) {
    return s == KamikazeState::kMoveTo || s == KamikazeState::kFollow ||
           s == KamikazeState::kPrepare;
}

void kamikaze_enter(KamikazeTask& task, KamikazeTaskHost& host, KamikazeRandom& rng,
                    KamikazeState s) {
    if (s == KamikazeState::kGoTowards) {
        kamikaze_gotowards_enter_009ac160(task.gotowards, task.ap, host.robot_row(),
                                          host.ctl_alt_cruise_394(), host.ctl_alt_second_398(),
                                          rng);
        if constexpr (kKamikazeEnterPickCc13Bound) {
            host.pick_target_ref_009fa260(task.ap.ref);   // 009AC09E JMP 009FA260
        }
    } else if (s == KamikazeState::kAim) {
        kamikaze_aim_enter_009ac8d0(task.aim, host.roll_c68());
    } else if (is_formation(s)) {
        host.enter_formation_state(s);
    }
}

// 009AF230: old vtable[8], store, new vtable[4]. gotowards and aim exit through
// 007B3DC0, a bare RET.
void kamikaze_set_state(KamikazeTask& task, KamikazeTaskHost& host, KamikazeRandom& rng,
                        KamikazeState next) {
    if (next == task.state) return;
    if (is_formation(task.state)) host.exit_formation_state(task.state);
    task.state = next;
    kamikaze_enter(task, host, rng, next);
}


}  // namespace

void kamikaze_task_construct_009aeef0(KamikazeTask& task, KamikazeTaskHost& host,
                                      const KamikazeSeedInputs& seed, KamikazeRandom& rng) {
    kamikaze_approach_seed_009ab920(task.ap, seed, rng);
    task.state = kamikaze_initial_state_009aeef0(host.flight_leader_007b8ad0(),
                                                 task.ap.rocket_like_5d);
    kamikaze_enter(task, host, rng, task.state);                         // 009AEF77
}

KamikazeArmResult kamikaze_task_arm_009af510(KamikazeTask& task, KamikazeTaskHost& host,
                                             KamikazeRandom& rng, float dt) {
    KamikazeArmResult r;
    r.before = task.state;
    KamikazeApproach& ap = task.ap;
    // 009AF521: the approach update. Documented deviation (cc13-D1): 009FADA0
    // and the gun-block clear are 009AF0A0's own first calls in the image
    // (clear, then 009FADA0); they are hoisted here, 009FADA0 first, which the
    // live host cannot observe (its clear_gun_block_34_38 is empty). Entering
    // gotowards here runs 009AC160 -> 009ABF60 without their 009FA260 tail pick;
    // the host picks at the next 009FADA0 instead (see 009ABF60; picked at
    // enter under kKamikazeEnterPickCc13Bound).
    std::array<float, 3> ref_point{};
    const bool has_target = host.update_target_ref_009fada0(ap.ref, dt, ref_point);
    r.has_target = has_target;
    KamikazeUpdateInputs ui;
    ui.has_target = has_target;
    ui.target_kind_0e = has_target && host.target_kind_0e();
    ui.aim_point = kamikaze_aim_point_009aba30(ref_point, ap.eta_60);
    ui.unit_position = host.unit_position();
    ui.forward_speed_b2c = host.forward_speed_b2c();
    host.clear_gun_block_34_38();
    kamikaze_approach_update_009af0a0(ap, ui);
    // 009AF530: the rule.
    KamikazeRuleInputs ri;
    ri.current = task.state;
    ri.rocket_like_455 = ap.rocket_like_5d;
    ri.in_range_454 = ap.in_range_5c;
    ri.has_target_478 = has_target;
    ri.target_marked_5d = has_target && host.target_marked_5d();
    ri.attack_mode_370 = host.attack_mode_370();
    ri.flight_leader = host.flight_leader_007b8ad0();
    const bool rocket_now = kamikaze_rocket_like(host.unit_kind_17(), host.pilot_fires_c24());
    ri.gotowards_ready = kamikaze_gotowards_ready_009aefc0(task.gotowards, ap, rocket_now);
    kamikaze_set_state(task, host, rng, kamikaze_rule_009af310(ri));
    // 009AF548: the state tick, with the point as 009ABA30 now computes it.
    const std::array<float, 3> aim = kamikaze_aim_point_009aba30(ref_point, ap.eta_60);
    if (task.state == KamikazeState::kGoTowards) {
        KamikazeGoTowardsInputs gi;
        gi.rocket_like = rocket_now;
        gi.aim_point = aim;
        gi.unit_position = ui.unit_position;
        gi.heading_50 = host.heading_50();
        gi.pitch_c64 = host.pitch_c64();
        gi.class_speed_188 = host.class_speed_188();
        gi.speed_b1c = host.speed_b1c();
        gi.level_flight_speed = host.level_flight_speed_007c47f0();
        r.plan = kamikaze_gotowards_tick_009ac190(task.gotowards, ap, gi);
        host.apply_plan(r.plan);
    } else if (task.state == KamikazeState::kAim) {
        KamikazeAimInputs ai;
        ai.pilot_fires_c24 = host.pilot_fires_c24();
        ai.aim_body = host.world_to_body(aim);
        ai.distance = distance3_009ac0b0(aim, ui.unit_position);
        ai.roll_c68 = host.roll_c68();
        ai.dt = dt;
        r.plan = kamikaze_aim_tick_009ac910(task.aim, ap, host.robot_row(), ai, rng);
        host.apply_plan(r.plan);
    } else if (is_formation(task.state)) {
        host.tick_formation_state(task.state, dt);
    }
    task.task_2e4 = ap.band_58;                                          // 009AF54A
    r.after = task.state;
    return r;
}

// ---------------------------------------------------------------------------
// Drop-kamikaze
// ---------------------------------------------------------------------------

const char* drop_kamikaze_state_name(DropKamikazeState s) noexcept {
    switch (s) {
        case DropKamikazeState::kMoveTo: return "moveto";
        case DropKamikazeState::kFollow: return "follow";
        case DropKamikazeState::kAttackRun: return "attackrun";
        case DropKamikazeState::kPrepare: return "prepare";
        case DropKamikazeState::kDone: return "done";
        case DropKamikazeState::kRelease: return "release";
        case DropKamikazeState::kGoAway: return "goAway";
        default: return "none";
    }
}

bool drop_kamikaze_in_attack_set_009add40(DropKamikazeState s) noexcept {
    return s == DropKamikazeState::kDone || s == DropKamikazeState::kGoAway ||
           s == DropKamikazeState::kAttackRun || s == DropKamikazeState::kPrepare ||
           s == DropKamikazeState::kRelease;
}

void drop_kamikaze_approach_seed_009abab0(DropKamikazeApproach& ap,
                                          const DropKamikazeSeedInputs& in) {
    // 009ABAD4-009ABAE5: 009F9CE0 with tuning+458h Pilot/LevelBomb/ReferenceSpeed.
    // Documented deviation (cc13-D1): the zero guard is host glue (009F9CE0 has
    // none; the authored speeds are positive), and the head caches, vtable
    // 00D1FB00 and 009FB200 construct are the host's.
    ap.speed_ratio_24 = (in.level_bomb_reference_speed_458 > 0.0f && in.class_speed_188 > 0.0f)
        ? bot_task_speed_ratio(in.class_speed_188, in.level_bomb_reference_speed_458) : 1.0f;
    ap.rounds_pending_2c = 0;                                            // 009ABAFC
    ap.class_18c_a4 = in.class_18c;                                      // 009ABB15
    ap.alt_a8 = 1200.0f;                                                 // 00CFD714
    ap.inside_c0 = false;                                                // 009ABB38
    ap.target_dist_b4 = 99999.0f;                                        // 00D119AC
    // 009ABB23-009ABB78: startDist = +24h * 2000.0 (qword 00CF0DD8); attackDist =
    // startDist * 1.4 (qword 00D045F0). The image keeps one x87 chain, but at
    // the main thread's 24-bit precision its intermediate is the float startDist,
    // so taking attackDist from the stored startDist is exact.
    ap.start_dist_ac = to_f(static_cast<double>(ap.speed_ratio_24) * 2000.0);
    ap.attack_dist_b0 = to_f(static_cast<double>(ap.start_dist_ac) * 1.399999976158142);
    ap.target_hdg_b8 = 0.0f;
    ap.avoid_flag_bc = 0xFF;
    ap.has_ordnance_c1 = true;                                           // 009ABB7E
    // 009ABB84-009ABBA2: 009FA380(sub, 00F87574 = zero fill), dirty, spread 0.5.
    ap.ref.bias_34 = {0.0f, 0.0f, 0.0f};
    ap.ref.dirty_41 = true;
    ap.ref.spread_48 = {0.5f, 0.5f, 0.5f};
}

void drop_kamikaze_approach_update_009ade00(DropKamikazeApproach& ap,
                                            const DropKamikazeUpdateInputs& in) {
    // Documented deviation (cc13-D1): 009FADA0(dt) (009ADE0F), the 007B93E0
    // ordnance query, the 009ABBC0 aim point and the unit/leader pose refreshes
    // are the caller's (drop_kamikaze_task_arm_009ae8a0 and the host's queries),
    // made before this runs; none reads what this function writes.
    ap.alt_a8 = in.ctl_alt_second_398;                                   // 009ADE24
    ap.has_ordnance_c1 = in.has_ordnance;                                // 009ADE33
    if (!in.has_target) {                                                // 009ADE2F
        ap.inside_c0 = false;
        return;
    }
    // 009ADE60-009ADEC9: planar distance, 0 at or below 1e-10.
    const float dx = to_f(static_cast<double>(in.aim_point[0]) - in.unit_position[0]);
    const float dz = to_f(static_cast<double>(in.aim_point[2]) - in.unit_position[2]);
    float d2;
    if constexpr (kKamikazeX87FloatCc13Bound) {
        // 009ADE81-009ADE89: dx^2 + dz^2, each step rounded to float.
        d2 = to_f(static_cast<double>(to_f(static_cast<double>(dx) * dx)) +
                  to_f(static_cast<double>(dz) * dz));
    } else {
        d2 = to_f(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    }
    // pc24-lint: allow sw1b reach counter, replica of the OFF arm (log only)
    sw1b_note(Sw1bReach::KamikazeX87Float,                               // sw1b reach
              to_f(static_cast<double>(to_f(static_cast<double>(dx) * dx)) +
                   to_f(static_cast<double>(dz) * dz)),
              to_f(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz));
    const float dist = (static_cast<double>(d2) > kDistEpsD)
        ? to_f(std::sqrt(static_cast<double>(d2))) : 0.0f;
    ap.target_dist_b4 = dist;
    ap.target_hdg_b8 = bearing_to(dx, dz);                               // 009ADF0D
    if (ap.inside_c0 && !in.reload_enabled) {                            // sw1b reach
        sw1b_note(Sw1bReach::KamikazeX87Float,
                  (to_f(static_cast<double>(ap.start_dist_ac) + 100.0) > dist)
                      != (static_cast<double>(ap.start_dist_ac) + 100.0 > dist));
    }
    bool inside;
    if (!ap.inside_c0) {
        inside = ap.start_dist_ac > dist;                                // 009AE023
    } else if (in.reload_enabled) {
        inside = true;                                                   // 009AE037
    } else if constexpr (kKamikazeX87FloatCc13Bound) {
        // 009ADF38-009ADF48: FADD qword 100.0 rounds to float, then JA.
        inside = to_f(static_cast<double>(ap.start_dist_ac) + 100.0) > dist;
    } else {
        inside = static_cast<double>(ap.start_dist_ac) + 100.0 > dist;   // 009ADF38, qword
    }
    ap.inside_c0 = inside;
    if (ap.has_ordnance_c1 || in.reload_enabled) return;                 // 009ADF5D-009ADF76
    // 009ADF7C-009AE043: an empty follower keeps "inside" only while its leader
    // is within attackDist of the aim point.
    if (in.leader_is_self) return;
    const float lx = to_f(static_cast<double>(in.aim_point[0]) - in.leader_position[0]);
    const float lz = to_f(static_cast<double>(in.aim_point[2]) - in.leader_position[2]);
    float l2;
    bool leader_close;
    if constexpr (kKamikazeX87FloatCc13Bound) {
        // 009ADFF1-009AE011: l2 = lz^2 + lx^2 and attack^2, each rounded to float.
        l2 = to_f(static_cast<double>(to_f(static_cast<double>(lz) * lz)) +
                  to_f(static_cast<double>(lx) * lx));
        leader_close = to_f(static_cast<double>(ap.attack_dist_b0) * ap.attack_dist_b0) > l2;
    } else {
        l2 = to_f(static_cast<double>(lx) * lx + static_cast<double>(lz) * lz);
        leader_close = static_cast<double>(ap.attack_dist_b0) * ap.attack_dist_b0 > l2;
    }
    {                                                                     // sw1b reach
        const float l2_on = to_f(static_cast<double>(to_f(static_cast<double>(lz) * lz)) +
                                 to_f(static_cast<double>(lx) * lx));
        // pc24-lint: allow sw1b reach counter, replica of the OFF arm (log only)
        const float l2_off = to_f(static_cast<double>(lx) * lx + static_cast<double>(lz) * lz);
        sw1b_note(Sw1bReach::KamikazeX87Float,
                  (to_f(static_cast<double>(ap.attack_dist_b0) * ap.attack_dist_b0) > l2_on)
                      != (static_cast<double>(ap.attack_dist_b0) * ap.attack_dist_b0 > l2_off));
    }
    ap.inside_c0 = ap.inside_c0 && leader_close;
}

DropKamikazeProfileWrite drop_kamikaze_cruise_profile_009ae9d0(
    DropKamikazeApproach& ap, bool flight_leader, const KamikazeAltitudeChannels& ch,
    float rocket_cruising_alt_45c, float rocket_drop_dist_460, KamikazeRandom& rng) {
    DropKamikazeProfileWrite w;
    if (flight_leader) {
        // 009AE9F0-009AEA50: both channels take tuning+45Ch RocketLike/CruisingAlt.
        KamikazeApproach tmp;
        tmp.profile.cruising_alt_2c = rocket_cruising_alt_45c;
        tmp.profile.attack_alt_38 = rocket_cruising_alt_45c;
        w.altitude = kamikaze_cruise_profile_009af480(tmp, ch);
    }
    // 009AEA82: startDist = U(0.9, 1.2) * ratio * DropDist (tuning+460h);
    // 009AEAB0: attackDist = ratio * 500.0 (qword 00CE3840) + startDist.
    // 009AEA87-009AEAB8: draw * (ratio * drop), then ratio * 500.0 + startDist.
    const float draw = rng.uniform_00bd2f10("ac", 0.89999998f, 1.20000005f);
    const double ratio = ap.speed_ratio_24;
    if constexpr (kKamikazeX87FloatCc13Bound) {
        // Each step rounds to float (24-bit x87 on the main thread).
        ap.start_dist_ac = to_f(static_cast<double>(draw) * to_f(ratio * rocket_drop_dist_460));
        ap.attack_dist_b0 = to_f(static_cast<double>(to_f(ratio * 500.0)) + ap.start_dist_ac);
    } else {
        ap.start_dist_ac = to_f(static_cast<double>(draw) * (ratio * rocket_drop_dist_460));
        ap.attack_dist_b0 = to_f(ratio * 500.0 + ap.start_dist_ac);
    }
    {                                                                     // sw1b reach
        const float start_on = to_f(static_cast<double>(draw) * to_f(ratio * rocket_drop_dist_460));
        // pc24-lint: allow sw1b reach counter, replica of the OFF arm (log only)
        const float start_off = to_f(static_cast<double>(draw) * (ratio * rocket_drop_dist_460));
        sw1b_note(Sw1bReach::KamikazeX87Float,
                  !sw1b_same_bits(start_on, start_off)
                      || !sw1b_same_bits(to_f(static_cast<double>(to_f(ratio * 500.0)) + start_on),
                                         to_f(ratio * 500.0 + start_off)));
    }
    return w;
}

void drop_kamikaze_task_cruise_profile_009ae9d0(DropKamikazeTask& task,
                                                DropKamikazeCruiseProfileHost& host,
                                                KamikazeRandom& rng) {
    // 009AE9D5: 0099B660, the base, is a bare RET.
    const bool leader = host.flight_leader_007b8ad0();                   // 009AE9E0
    const DropKamikazeProfileWrite w = drop_kamikaze_cruise_profile_009ae9d0(
        task.ap, leader, host.altitude_channels(), host.rocket_cruising_alt_45c(),
        host.rocket_drop_dist_460(), rng);
    // 009AE9E9-009AEA52 run before the draw; the draw touches no channel byte.
    if (leader) host.apply_altitude_write(w.altitude);
    drop_kamikaze_approach_update_009ade00(
        task.ap, host.approach_update_inputs_009ade00(task.ap, 0.0f));  // 009AEAC3
    host.abandon_if_stale_0099b740();                                    // 009AEACF JMP
}

void drop_kamikaze_follow_enter_009ad180(DropKamikazeFollowState& st,
                                         float level_flight_speed) noexcept {
    // The state-field half of 009AD180 (cc13-D1 re-check). drop_enter makes the
    // rest: 009BED80 (enter_formation_state(kFollow)), then squadron+3E4h = 3 and
    // 007ED260 (set_squadron_formation_shape_007ed260(3)), then this. The image
    // stores +A0h/+98h before 009BED80; 009BED80 writes only +84h..+94h and the
    // squadron, so the order is unobservable.
    st.released_a0 = false;                  // 009AD18C
    st.release_timer_98 = -1.0f;             // 00D7A260
    st.follow_8c = 1.0f;
    st.speed_9c = level_flight_speed;
}

void drop_kamikaze_arm_release_009ae060(DropKamikazeFollowState& st, float speed_b1c,
                                        float level_flight_speed) noexcept {
    st.release_timer_98 = 3.0f;              // 00CE3854
    st.speed_9c = (speed_b1c < level_flight_speed) ? level_flight_speed : speed_b1c;
}

bool drop_kamikaze_release_pending_009adcc0(const DropKamikazeFollowState& st) noexcept {
    return !st.released_a0 && st.release_timer_98 > -0.5f;   // 00CE69D0
}

DropKamikazeFollowTick drop_kamikaze_follow_tick_009ad1f0(DropKamikazeFollowState& st,
                                                          bool has_leader_3d0, float roll_c68,
                                                          float pitch_c64, float dt) noexcept {
    DropKamikazeFollowTick r;
    if (st.released_a0) return r;                                        // 009AD1F3
    if (!(st.release_timer_98 > 0.0f)) {
        // 009AD341-009AD37F: plain follow.
        r.sets_avoid_3 = has_leader_3d0;
        st.follow_8c = 1.0f;
        r.runs_follow_base = true;                                       // 009AD371 009C1FD0
        r.plan.clears_gun_block_40 = true;
        return r;
    }
    r.sets_avoid_1 = true;                                               // 009AD224
    const float t = to_f(static_cast<double>(st.release_timer_98) - dt);
    st.release_timer_98 = t;
    bool release = !(t > 0.0f);                                          // 009AD240 JAE
    if (!release) {
        // 009AD25C-009AD278: wings level (bank 0, mode 1), pitch 5 deg (mode 2).
        r.plan.writes_bank_2c4 = true;
        r.plan.bank_2c4 = 0.0f;
        // 009AD288-009AD2C5: release once |roll| and |pitch| are under 15 degrees.
        const float ar = (roll_c68 > 0.0f) ? roll_c68 : kNegZeroF - roll_c68;
        const float ap_ = (pitch_c64 > 0.0f) ? pitch_c64 : kNegZeroF - pitch_c64;
        if (0.2617994f > ar && 0.2617994f > ap_) {
            release = true;
        } else {
            st.release_timer_98 = (st.release_timer_98 > 1.0f) ? st.release_timer_98 : 1.0f;  // 00415550
        }
    }
    if (release) {
        st.released_a0 = true;                                           // 009AD2E7
        r.releases = true;                                               // 009AD2F0, +2Ch -= 1
    }
    // 009AD2FC-009AD333: speed hold and the 5 degree pitch, on both arms.
    r.plan.writes_speed_2b4 = true;
    r.plan.speed_2b4 = st.speed_9c;
    r.plan.writes_pitch_target_2bc = true;
    r.plan.pitch_target_2bc = 0.08726647f;                               // 00CEDF5C
    r.plan.pitch_mode_2d0 = 2;
    return r;
}

KamikazePlanCommand drop_kamikaze_attackrun_tick_009acfc0(const DropKamikazeApproach& ap,
                                                          float unit_y) noexcept {
    KamikazePlanCommand c;
    c.writes_heading_2c0 = true;                                         // 009ACFD3
    c.heading_2c0 = ap.target_hdg_b8;
    // 009ACFE5-009AD010: D = min(targetDist, 2000).
    const float d = (2000.0 > static_cast<double>(ap.target_dist_b4)) ? ap.target_dist_b4 : 2000.0f;
    // 009AD02B-009AD05B: a = 1400.0 (qword 00D1F8D0) - y, floor 50.
    float a = to_f(1400.0 - unit_y);
    if (50.0 > static_cast<double>(a)) a = 50.0f;                        // 00CEB4D4
    const float frac = to_f(static_cast<double>(a) / d);
    // 009AD09D: 00419010(0.1 [00D7A2F0], 0.4 [00CE7804], 0.35 [00CF6560], 1.0, frac).
    const float s = interp(0.1f, 0.4f, 0.35f, 1.0f, frac);
    c.calls_cruise_altitude_009fba50 = true;                             // 009AD0C6
    c.cruise_base = ap.alt_a8;
    c.cruise_low = ap.start_dist_ac;
    c.cruise_high = ap.target_dist_b4;
    c.cruise_scale = s;
    throttle_full(c);                                                    // 009AD0D0-009AD0FA
    return c;
}

void drop_kamikaze_release_enter_009ad6a0(DropKamikazeReleaseState& st, bool unit_c25,
                                          float speed_b1c, float level_flight_speed) noexcept {
    st.released_18 = unit_c25;
    st.timer_1c = 3.0f;                                                  // 00CE3854
    st.speed_20 = (speed_b1c < level_flight_speed) ? level_flight_speed : speed_b1c;
}

DropKamikazeReleaseTick drop_kamikaze_release_tick_009ad5e0(DropKamikazeReleaseState& st,
                                                            bool unit_c25, float dt) noexcept {
    DropKamikazeReleaseTick r;
    if (!st.released_18) {                                               // 009AD5F2
        st.released_18 = true;
        r.releases = true;                                               // 007BBBA0, +2Ch -= 1
    }
    r.plan.writes_bank_2c4 = true;                                       // wings level, mode 1
    r.plan.bank_2c4 = 0.0f;
    r.plan.writes_pitch_target_2bc = true;                               // 5 deg, mode 1
    r.plan.pitch_target_2bc = 0.08726647f;
    r.plan.pitch_mode_2d0 = 1;
    r.plan.writes_speed_2b4 = true;
    r.plan.speed_2b4 = st.speed_20;
    if (st.released_18 && !unit_c25) st.timer_1c = to_f(static_cast<double>(st.timer_1c) - dt);
    else st.timer_1c = 3.0f;
    return r;
}

void drop_kamikaze_goaway_enter_009ad430(DropKamikazeGoAwayState& st, bool step_odd) noexcept {
    st.side_1c = step_odd ? 1.0f : -1.0f;
}

KamikazePlanCommand drop_kamikaze_goaway_exit_009ad460() noexcept {
    KamikazePlanCommand c;
    c.writes_turn_scale_2e8 = true;
    c.turn_scale_2e8 = 1.0f;
    return c;
}

DropKamikazeGoAwayTick drop_kamikaze_goaway_tick_009ad480(DropKamikazeGoAwayState& st,
                                                          const DropKamikazeApproach& ap,
                                                          bool reload_enabled) noexcept {
    DropKamikazeGoAwayTick r;
    st.reengage_18 = reload_enabled && ap.has_ordnance_c1;               // 009AD48D-009AD4C0
    throttle_full(r.plan);
    r.plan.calls_pitch_command_009fb800 = true;                          // 009AD503
    r.plan.pitch_altitude = ap.alt_a8;
    r.plan.pitch_reference = 1.0f;
    r.flyto_standoff = ap.start_dist_ac;                                 // 009AD51A
    r.flyto_range = ap.target_dist_b4;                                   // 009AD50A
    r.flyto_offset_scale = 1.0f;
    r.plan.writes_heading_2c0 = true;                                    // 009AD566, from 009FD570
    r.plan.writes_turn_scale_2e8 = true;                                 // 009AD57B
    r.plan.turn_scale_2e8 = 0.5f;
    r.plan.clears_gun_block_40 = true;                                   // 009AD58C
    return r;
}

DropKamikazeState drop_kamikaze_engaged_entry_009ae530(const DropKamikazeEntryInputs& in) noexcept {
    if (in.unit_c25) return DropKamikazeState::kRelease;                 // 009AE53F
    if (in.attack_mode_370 == 0) return DropKamikazeState::kPrepare;
    if (in.has_ordnance_4b9)
        return in.inside_4b8 ? DropKamikazeState::kRelease : DropKamikazeState::kAttackRun;
    return in.flight_leader ? DropKamikazeState::kGoAway : DropKamikazeState::kDone;
}

bool drop_kamikaze_should_break_off_009ae1b0(const DropKamikazeBreakOffInputs& in) noexcept {
    if (!in.base_allows) return false;
    if (!in.has_target_440 || in.target_marked_5d) return true;
    if (!in.reload_enabled) {
        if (!drop_kamikaze_in_attack_set_009add40(in.current)) return true;
        if (!in.has_ordnance_4b9) return true;
    }
    return false;
}

DropKamikazeState drop_kamikaze_rule_009ae610(const DropKamikazeRuleInputs& in) noexcept {
    using S = DropKamikazeState;
    const bool engage = in.inside_4b8 || (in.attack_mode_370 == 2 && in.has_target_440);
    const S formation = in.flight_leader ? S::kMoveTo : S::kFollow;
    DropKamikazeEntryInputs e;
    e.unit_c25 = in.unit_c25;
    e.attack_mode_370 = in.attack_mode_370;
    e.has_ordnance_4b9 = in.has_ordnance_4b9;
    e.inside_4b8 = in.inside_4b8;
    e.flight_leader = in.flight_leader;
    if (!drop_kamikaze_in_attack_set_009add40(in.current)) {
        return engage ? drop_kamikaze_engaged_entry_009ae530(e) : formation;  // 009AE82C
    }
    if (!engage) return formation;                                       // 009AE651-009AE6B8
    if (in.attack_mode_370 == 0) return S::kPrepare;                     // 009AE6BB-009AE6D3
    const S breakoff_to = in.flight_leader ? S::kGoAway : S::kDone;
    switch (in.current) {
        case S::kDone:                                                   // 009AE6DE
            if (in.flight_leader) return S::kGoAway;
            if (in.reload_enabled && in.has_ordnance_4b9)
                return drop_kamikaze_engaged_entry_009ae530(e);
            return in.current;
        case S::kGoAway:                                                 // 009AE732
            return in.goaway_reengage_744 ? drop_kamikaze_engaged_entry_009ae530(e) : in.current;
        default:
            break;
    }
    if (in.should_break_off) return breakoff_to;                         // 009AE756-009AE76D
    if (in.current == S::kPrepare) {                                     // 009AE773
        if (in.prepare_released_660) return S::kRelease;
        if (!in.prepare_pending) return drop_kamikaze_engaged_entry_009ae530(e);
        return in.current;
    }
    if (in.current == S::kAttackRun)                                     // 009AE7BA
        return in.inside_4b8 ? S::kRelease : in.current;
    if (in.current == S::kRelease && 0.0f > in.release_timer_724)       // 009AE7E5
        return breakoff_to;
    return in.current;
}

DropKamikazeState drop_kamikaze_initial_state_009adbf0(bool flight_leader) noexcept {
    return flight_leader ? DropKamikazeState::kMoveTo : DropKamikazeState::kFollow;
}

int drop_kamikaze_step_result_009addb0(bool has_ordnance_c1) noexcept {
    return has_ordnance_c1 ? 2 : 0;   // NEG/SBB/AND 2
}

bool drop_kamikaze_manual_release_009ae210(DropKamikazeApproach& ap, DropKamikazeState current,
                                           bool manual_control_72c,
                                           DropKamikazeFollowState& prepare, float speed_b1c,
                                           float level_flight_speed) noexcept {
    if (ap.has_ordnance_c1 && manual_control_72c) {
        ++ap.rounds_pending_2c;                                          // task+424h
        if (current == DropKamikazeState::kPrepare)
            drop_kamikaze_arm_release_009ae060(prepare, speed_b1c, level_flight_speed);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Drop-kamikaze driver
// ---------------------------------------------------------------------------

namespace {

bool drop_is_formation(DropKamikazeState s) {
    return s == DropKamikazeState::kMoveTo || s == DropKamikazeState::kFollow;
}

void drop_enter(DropKamikazeTask& task, DropKamikazeTaskHost& host, DropKamikazeState s) {
    using S = DropKamikazeState;
    switch (s) {
        case S::kMoveTo:
        case S::kFollow:
            host.enter_formation_state(s);
            break;
        case S::kPrepare:
        case S::kDone:
            // 009AD180: BSP_BotStateFollow_Enter first (the host's follow enter).
            host.enter_formation_state(S::kFollow);
            // 009AD19F-009AD1AF: squadron+3E4h = 3, then 007ED260.
            host.set_squadron_formation_shape_007ed260(3);
            drop_kamikaze_follow_enter_009ad180(s == S::kPrepare ? task.prepare : task.done,
                                                host.level_flight_speed_007c47f0());
            break;
        case S::kAttackRun:
            break;                                   // 009ACFB0 zeroes state+18h, unread
        case S::kRelease:
            drop_kamikaze_release_enter_009ad6a0(task.release, host.unit_c25(), host.speed_b1c(),
                                                 host.level_flight_speed_007c47f0());
            break;
        case S::kGoAway:
            drop_kamikaze_goaway_enter_009ad430(task.goaway, host.step_odd_00f876b0());
            break;
        default:
            break;
    }
}

void drop_exit(DropKamikazeTaskHost& host, DropKamikazeState s) {
    using S = DropKamikazeState;
    if (drop_is_formation(s)) host.exit_formation_state(s);
    else if (s == S::kPrepare || s == S::kDone) host.exit_formation_state(S::kFollow);  // 009AD1E0
    else if (s == S::kGoAway) host.apply_plan(drop_kamikaze_goaway_exit_009ad460());
}

void drop_set_state(DropKamikazeTask& task, DropKamikazeTaskHost& host, DropKamikazeState next) {
    if (next == task.state) return;                  // 009AE290
    drop_exit(host, task.state);
    task.state = next;
    drop_enter(task, host, next);
}

}  // namespace

void drop_kamikaze_task_construct_009adbf0(DropKamikazeTask& task, DropKamikazeTaskHost& host,
                                           const DropKamikazeSeedInputs& seed) {
    drop_kamikaze_approach_seed_009abab0(task.ap, seed);
    // 009AD720: the follow variants start with timer -1, released 0.
    task.prepare = DropKamikazeFollowState{};
    task.done = DropKamikazeFollowState{};
    task.state = drop_kamikaze_initial_state_009adbf0(host.flight_leader_007b8ad0());
    drop_enter(task, host, task.state);
}

DropKamikazeArmResult drop_kamikaze_task_arm_009ae8a0(DropKamikazeTask& task,
                                                      DropKamikazeTaskHost& host, float dt) {
    using S = DropKamikazeState;
    DropKamikazeArmResult r;
    r.before = task.state;
    DropKamikazeApproach& ap = task.ap;
    ap.avoid_flag_bc = 0xFF;                                             // 009AE8B2
    // 009AE8BC: the approach update. Documented deviation (cc13-D1): 009FADA0,
    // the ordnance query and the pose refreshes, made inside 009ADE00 in the
    // image, are host calls made here first; the state functions return plans
    // that host.apply_plan writes after each returns. Neither order is
    // observable: the queries read nothing 009ADE00 writes.
    std::array<float, 3> point{};
    const bool has_target = host.update_target_ref_009fada0(ap.ref, dt, point);
    DropKamikazeUpdateInputs ui;
    ui.has_target = has_target;
    ui.has_ordnance = host.has_drop_ordnance_007b93e0();
    ui.ctl_alt_second_398 = host.ctl_alt_second_398();
    ui.reload_enabled = host.reload_enabled_007b58d0();
    ui.aim_point = point;                                                // 009ABBC0
    ui.unit_position = host.unit_position();
    ui.leader_is_self = host.leader_is_self();
    if (!ui.leader_is_self) ui.leader_position = host.leader_position();
    drop_kamikaze_approach_update_009ade00(ap, ui);
    // 009AE8C1-009AE8F7: the moveto ranges (alt - 100.0 qword, alt, startDist).
    host.set_moveto_ranges_009bde80(to_f(static_cast<double>(ap.alt_a8) - 100.0), ap.alt_a8,
                                    ap.start_dist_ac);
    // 009AE906: the rule.
    const bool leader = host.flight_leader_007b8ad0();
    DropKamikazeRuleInputs ri;
    ri.current = task.state;
    ri.inside_4b8 = ap.inside_c0;
    ri.has_target_440 = has_target;
    ri.attack_mode_370 = host.attack_mode_370();
    ri.flight_leader = leader;
    ri.reload_enabled = ui.reload_enabled;
    ri.has_ordnance_4b9 = ap.has_ordnance_c1;
    ri.goaway_reengage_744 = task.goaway.reengage_18;
    DropKamikazeBreakOffInputs bi;
    bi.base_allows = host.break_off_allowed();
    bi.has_target_440 = has_target;
    bi.target_marked_5d = has_target && host.target_marked_5d();
    bi.reload_enabled = ui.reload_enabled;
    bi.current = task.state;
    bi.has_ordnance_4b9 = ap.has_ordnance_c1;
    ri.should_break_off = drop_kamikaze_in_attack_set_009add40(task.state) &&
                          drop_kamikaze_should_break_off_009ae1b0(bi);
    ri.prepare_released_660 = task.prepare.released_a0;
    ri.prepare_pending = drop_kamikaze_release_pending_009adcc0(task.prepare);
    ri.release_timer_724 = task.release.timer_1c;
    ri.unit_c25 = host.unit_c25();
    drop_set_state(task, host, drop_kamikaze_rule_009ae610(ri));
    // 009AE91E: the state tick.
    auto release_one = [&]() {
        host.request_ordnance_release_007bbba0();
        --ap.rounds_pending_2c;
        ++r.releases;
    };
    switch (task.state) {
        case S::kMoveTo:
        case S::kFollow:
            host.tick_formation_state(task.state, dt);
            break;
        case S::kPrepare:
        case S::kDone: {
            DropKamikazeFollowState& fs = (task.state == S::kPrepare) ? task.prepare : task.done;
            const DropKamikazeFollowTick t = drop_kamikaze_follow_tick_009ad1f0(
                fs, host.has_leader_3d0(), host.roll_c68(), host.pitch_c64(), dt);
            if (t.sets_avoid_3) ap.avoid_flag_bc = 3;
            if (t.sets_avoid_1) ap.avoid_flag_bc = 1;
            if (t.runs_follow_base) host.tick_formation_state(S::kFollow, dt);
            if (t.releases) release_one();
            host.apply_plan(t.plan);
            break;
        }
        case S::kAttackRun:
            host.apply_plan(drop_kamikaze_attackrun_tick_009acfc0(ap, host.unit_position()[1]));
            break;
        case S::kRelease: {
            ap.avoid_flag_bc = 1;                                        // 009AD5E3
            const DropKamikazeReleaseTick t =
                drop_kamikaze_release_tick_009ad5e0(task.release, host.unit_c25(), dt);
            if (t.releases) release_one();
            host.apply_plan(t.plan);
            break;
        }
        case S::kGoAway: {
            DropKamikazeGoAwayTick t =
                drop_kamikaze_goaway_tick_009ad480(task.goaway, ap, ui.reload_enabled);
            t.plan.heading_2c0 = host.fly_to_heading_009fd570(point, t.flyto_standoff,
                                                              task.goaway.side_1c,
                                                              t.flyto_range,
                                                              t.flyto_offset_scale);
            host.apply_plan(t.plan);
            break;
        }
        default:
            break;
    }
    task.task_2e4 = ap.avoid_flag_bc;                                    // 009AE92D
    // 009AE920-009AE98A: owed manual releases go out outside attackrun/release/prepare.
    if (ap.rounds_pending_2c > 0 && host.manual_control_72c() && task.state != S::kAttackRun &&
        task.state != S::kRelease && task.state != S::kPrepare) {
        release_one();
    }
    r.after = task.state;
    return r;
}

}  // namespace bsp
