// The plane depth-charge bot task (kind 1). Evidence in
// docs/CC13_DEPTH_CHARGE_TASK.md; every address in a comment is a listing
// address. x87 intermediates are carried in double and stored to float wherever
// the listing has FSTP dword, as the rocket twin does (src/rocket_task_cc13.cpp).
//
// kPc24PlaneTasksCc13Bound (OFF, pc24w1-sweep3-2) rounds per op at 24 bits in
// 009A3300, 009A5F50, 009A65F0 (its 0042B2F0 length copy included), 009A3770,
// 009A3CF0, 009A4010 and 009A4940. 009A3290 and 009A3AE0 already round once
// (an exact factor); see their pc24-lint allow notes. Movers when ON: depth-charge
// plane approach, release timing and release points.

#include "bsp/depth_charge_task_cc13.hpp"

#include <cmath>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/geometry_helpers.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"
#include "bsp/robot_config.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {

const std::array<DepthChargeDrawSite, static_cast<std::size_t>(DepthChargeDraw::kCount)>
    kDepthChargeDrawSites = {{
        {DepthChargeDraw::kRunSpeed30, 0x009A33F8u, 0x009A3390u, "30", "U(0.9, 1.1)"},
        {DepthChargeDraw::kAimAltSpread34, 0x009A3440u, 0x009A3390u, "34", "U(0, AimAltRange/2 - AimAltRange/1)"},
        {DepthChargeDraw::kManeuverAlt38, 0x009A346Fu, 0x009A3390u, "38", "U(ManeuverAltRange/1, ManeuverAltRange/2)"},
        {DepthChargeDraw::kFlyAbove40, 0x009A34AAu, 0x009A3390u, "40", "U(1.1, 1.3)"},
        {DepthChargeDraw::kBiasX, 0x009A31D1u, 0x009A31A0u, "bx", "U(-TargetHError, TargetHError)"},
        {DepthChargeDraw::kBiasZ, 0x009A3201u, 0x009A31A0u, "bz", "U(-TargetVError, TargetVError)"},
        {DepthChargeDraw::kProjError6C, 0x009A3281u, 0x009A31A0u, "6c", "U(-CalcTargetPosError, CalcTargetPosError)"},
        {DepthChargeDraw::kAttackRunCd1C, 0x009A36FAu, 0x009A36A0u, "ar1c", "U(0, 1)"},
        {DepthChargeDraw::kLeaveTimer18, 0x009A39DEu, 0x009A39C0u, "lv18", "U(4, 7)"},
        {DepthChargeDraw::kGoAwayStandoff1C, 0x009A3CD3u, 0x009A3C60u, "ga1c", "U(200, 400)"},
        {DepthChargeDraw::kAimCooldown1C, 0x009A4646u, 0x009A4010u, "aim1c", "U(0.9, 1.1)"},
        {DepthChargeDraw::kPrepareGate98, 0x009A5360u, 0x009A5330u, "pr98", "U(1.5, 3.0)"},
    }};

namespace {

constexpr double kEpsilon = 1e-10;                       // 00CE3820, FLD qword
constexpr double kHalf = 0.5;                            // 00D7A280, FLD/FMUL qword
constexpr double kPointFour = 0.4000000059604645;        // 00CE65D0, FMUL qword
constexpr double kProbeGate = 0.05000000074505806;       // 00D7A270, FLD qword
constexpr double kThirtyDegrees = 0.5235987901687622;    // 00CEC730, FMUL qword
constexpr double kFifteenDegrees = 0.2617993950843811;   // 00D19628, FMUL qword
constexpr double kGoAwayClamp = 0.4363323152065277;      // 00D1F6C0 / -00D1F6D0, FLD qword
constexpr float kGoAwayClampF = 0.4363323152065277f;     // 00CEB664 / -00D1F6C8, MOVSS
constexpr double kProjCap = 30.0;                        // 00CE7630, FLD qword

float draw(DepthChargeRandom& rng, DepthChargeDraw id, float lo, float hi) {
    return rng.uniform_00bd2f10(kDepthChargeDrawSites[static_cast<std::size_t>(id)].key, lo, hi);
}

float length_or_zero(double sum) noexcept {
    // 0042B2F0 and the inline copies: the sum is stored float, compared against
    // double [00CE3820], then sqrt and stored float.
    const float s = static_cast<float>(sum);
    if (!(static_cast<double>(s) > kEpsilon)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

// 00415550 BSP_Math_MaxFloatByRef(a, b): FCOMIP a against b, JBE returns b.
float max_by_ref_00415550(float a, float b) noexcept { return a > b ? a : b; }
// 00415510 BSP_Math_MinFloatByRef(a, b): FCOMIP b against a, JBE returns b.
float min_by_ref_00415510(float a, float b) noexcept { return b > a ? a : b; }

// float(+3Ch + +34h), the aim altitude, rounded by FSTP before every use.
float aim_altitude(const DepthChargeApproach& ap) noexcept {
    return ap.aim_alt_base_3c + ap.aim_alt_spread_34;
}

void write_throttle(DepthChargePlanCommand& c, float value) noexcept {
    c.writes_throttle = true;   // plan+278h, +27Ch = 1, +2A8h = 0, +2ACh = 1, +2D8h = 0
    c.throttle_278 = value;
}

void write_cruise(DepthChargePlanCommand& c, float base, float low, float high,
                  float scale) noexcept {
    c.calls_cruise_altitude_009fba50 = true;
    c.cruise_base = base;
    c.cruise_low = low;
    c.cruise_high = high;
    c.cruise_scale = scale;
}

DepthChargeFollowState& follow_state(DepthChargeTask& t, DepthChargeState s) noexcept {
    return s == DepthChargeState::kPrepare ? t.prepare : t.done;
}

bool is_follow_variant(DepthChargeState s) noexcept {
    return s == DepthChargeState::kDone || s == DepthChargeState::kPrepare;
}

bool is_formation(DepthChargeState s) noexcept {
    return s == DepthChargeState::kMoveTo || s == DepthChargeState::kFollow;
}

// 009A5330 on the prepare state: +9Ch += 1; re-arm the gate when it is not
// already open (COMISS 0, +98h; JC skips while 0 < +98h).
void prepare_arm_009a5330(DepthChargeFollowState& fs, DepthChargeRandom& rng) {
    fs.salvo_9c += 1;                                                  // 009A533B
    if (!(0.0f < fs.gate_98)) {
        fs.gate_98 = draw(rng, DepthChargeDraw::kPrepareGate98, 1.5f, 3.0f);   // 00CE380C, 00CE3854
    }
}

// The new state's vt[4]. The window's own enters, else the host's formation.
void enter_state(DepthChargeTask& t, DepthChargeState s, DepthChargeTaskHost& host,
                 DepthChargeRandom& rng) {
    switch (s) {
        case DepthChargeState::kAttackRun: depth_charge_attackrun_enter_009a3750(t.ar); break;
        case DepthChargeState::kLeave: depth_charge_leave_enter_009a39c0(t.lv, rng); break;
        case DepthChargeState::kTurnTo: depth_charge_turnto_enter_009a3ae0(t, host); break;
        case DepthChargeState::kGoAway: depth_charge_goaway_enter_009a3c60(t, host, rng); break;
        case DepthChargeState::kAim: depth_charge_aim_enter_009a3fe0(t, host, rng); break;
        case DepthChargeState::kDone:
        case DepthChargeState::kPrepare: depth_charge_follow_enter_009a4890(t, s, host); break;
        default: host.enter_formation_state(s); break;
    }
}

// The old state's vt[8]. attackrun/leave/turnto/goaway/aim inherit 007B3DC0, a
// plain RET.
int exit_state(DepthChargeTask& t, DepthChargeState s, DepthChargeTaskHost& host) {
    if (is_follow_variant(s)) return depth_charge_follow_exit_009a48d0(t, s, host) ? 1 : 0;
    if (is_formation(s)) host.exit_formation_state(s);
    return 0;
}

// 009A56C0 and its inline copies in 009A57D0/009A5870.
int set_state(DepthChargeTask& t, DepthChargeState next, DepthChargeTaskHost& host,
              DepthChargeRandom& rng, int& releases) {
    if (next == DepthChargeState::kUnchanged || next == t.state) return 0;
    releases += exit_state(t, t.state, host);
    t.state = next;
    enter_state(t, next, host, rng);
    return 1;
}

}  // namespace

DepthChargeRow depth_charge_row(const PilotBotParameters& row) noexcept {
    DepthChargeRow r;
    r.target_h_error_0cc = row.depth_charge_targeth_error_0cc;
    r.target_v_error_0d0 = row.depth_charge_targetv_error_0d0;
    r.calc_target_pos_error_0d4 = row.depth_charge_calc_target_pos_error_0d4;
    r.cancel_target_dist_0d8 = row.depth_charge_cancel_target_dist_0d8;
    r.target_point_select_prec_0dc = row.depth_charge_target_point_select_prec_0dc;
    return r;
}

// ---------------------------------------------------------------------------
// The approach
// ---------------------------------------------------------------------------
void depth_charge_pick_aim_offset_009a31a0(DepthChargeApproach& ap, const DepthChargeRow& row,
                                           DepthChargeTaskHost& host, DepthChargeRandom& rng) {
    // 009A31A3-009A3201: -0.0 [00D7A208] - e, SUBSS, so U(-e, e).
    const float bx = draw(rng, DepthChargeDraw::kBiasX, -row.target_h_error_0cc,
                          row.target_h_error_0cc);
    const float bz = draw(rng, DepthChargeDraw::kBiasZ, -row.target_v_error_0d0,
                          row.target_v_error_0d0);
    ap.ref.bias_34 = {bx, 0.0f, bz};                                  // 009FA380
    ap.ref.dirty_41 = true;                                           // 009FA395
    const float s = row.target_point_select_prec_0dc;                 // 009A3235
    ap.ref.spread_48 = {s, s, s};
    ap.ref.dirty_41 = true;                                           // 009A324E
    host.pick_target_ref_009fa260(ap.ref);                            // 009A3252
    ap.proj_error_6c = draw(rng, DepthChargeDraw::kProjError6C, -row.calc_target_pos_error_0d4,
                            row.calc_target_pos_error_0d4);           // 009A3286
}

float depth_charge_fall_lead_009a3300(float aim_point_y, float unit_y,
                                      float unit_velocity_y) noexcept {
    const float fall = weapon_fall_time_007bcc80(unit_y, unit_velocity_y);   // 009A332F
    const float v = 0.0f < aim_point_y ? 0.0f : aim_point_y;          // 009A3352 FCOMIP, JBE
    // 009A336E FMUL qword 0.02 [00D7A2F8], FSUBR qword 0.15 [00CE6618], FSTP
    // float, then FADD the fall time.
    float k;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A336E..009A337A: the FMUL rounds to 24 bits before the FSUBR.
        k = static_cast<float>(cc13x87::x87_sub24(
            0.15000000596046448, cc13x87::x87_mul24(v, 0.019999999552965164)));
    } else {
        k = static_cast<float>(0.15000000596046448 - static_cast<double>(v) *
                                                         0.019999999552965164);
    }
    return static_cast<float>(static_cast<double>(k) + fall);
}

float depth_charge_stick_half_009a3290(int remaining_rounds, float run_speed_30) noexcept {
    const float n = static_cast<float>(static_cast<double>(remaining_rounds) * kPointFour);   // 009A32AA
    // 009A32B8 FCOMI 0.5 against n; above: 0.5 * 0.5f, else 0.5 * n.
    const double m = kHalf > static_cast<double>(n) ? kHalf * 0.5f : kHalf * n;
    // pc24-lint: allow 0.5 * n (or 0.5 * 0.5f) is exact, so the FMULP / FMUL pair at 009A32D0..009A32D2 rounds once
    return static_cast<float>(m * run_speed_30);
}

float depth_charge_projtime_007b5350(float x) noexcept {
    if (0.0f > x) return 0.0f;                                        // 007B5359 COMISS, JA
    if (static_cast<double>(x) > kProjCap) return 30.0f;              // 007B5368, [00CE38C8]
    return x;
}

void depth_charge_approach_update_009a5f50(DepthChargeApproach& ap, DepthChargeTaskHost& host,
                                           float dt) {
    const DepthChargeControlView ctl = host.control();
    const DepthChargeTuning tu = host.tuning();
    if (!ctl.flag_3ad) {                                              // 009A5F6B
        ap.aim_alt_spread_34 = 0.0f;
        ap.maneuver_alt_38 = tu.maneuver_alt_range_1_49c;
    }
    // 009A5FA6 and 009A5FD8: FCOMIP ctl+394h against the field, JBE takes ctl+394h.
    ap.aim_alt_base_3c = ctl.alt_394 > ap.aim_alt_base_3c ? ap.aim_alt_base_3c : ctl.alt_394;
    ap.maneuver_alt_38 = ctl.alt_394 > ap.maneuver_alt_38 ? ap.maneuver_alt_38 : ctl.alt_394;
    ap.has_ordnance_74 = host.has_depth_charge_ordnance_007b94f0();   // 009A5FF7

    auto finish = [&ap, dt]() {
        ap.elapsed_54 = static_cast<float>(static_cast<double>(ap.elapsed_54) + dt);   // 009A62E7
    };
    auto no_target = [&ap, &finish]() {
        ap.in_range_75 = false;                                       // 009A62D7
        ap.off_track_68 = 0.0f;
        finish();
    };
    if (!ap.ref.tracking) {                                           // 009A6005 approach+94h
        no_target();
        return;
    }
    if (host.target_spotted_count_1e8() >= 1) {                       // 009A602F
        std::array<float, 3> point{{0.0f, 0.0f, 0.0f}};
        ap.ref.tracking = host.update_target_ref_009fada0(ap.ref, dt, point);   // 009A6043
        ap.ref.world_point_1c = point;
        ap.lost_timer_70 = 0.0f;                                      // 009A604B
    } else if (depth_charge::kTargetLostRadius > ap.range_48) {       // 009A62A1 COMISS
        ap.lost_timer_70 = static_cast<float>(static_cast<double>(ap.lost_timer_70) + dt);
        if (ap.lost_timer_70 > tu.target_lost_time_4bc) {             // 009A62C3
            host.clear_target_ref_009fb5e0(ap.ref);                   // 009A62D2
            ap.ref.tracking = false;
            no_target();
            return;
        }
    }

    const std::array<float, 3> pos = host.unit_position();
    const std::array<float, 3>& p = ap.ref.world_point_1c;           // vt[0] 009A35A0
    const std::array<float, 2> d{{p[0] - pos[0], p[2] - pos[2]}};     // 009A6072, 009A6083
    ap.range_48 = length_2d_00414c60(d);                              // 009A609D
    ap.bearing_4c = heading_angle_00414eb0(d);                        // 009A60A9
    ap.rel_bearing_50 = wrapped_angle_subtract_00438b10(ap.bearing_4c, host.unit_heading_50());
    // 009A60D4-009A6102: +44h + 100 (qword) against +48h; while in range the
    // alt-loadout test keeps it.
    bool inside;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A60DA: the FADD stays on the stack, rounded to 24 bits, for the FCOMIP.
        inside = cc13x87::x87_add24(ap.attack_range_44, depth_charge::kAttackRangeHysteresis) >
                 static_cast<double>(ap.range_48);
    } else {
        inside = static_cast<double>(ap.attack_range_44) + depth_charge::kAttackRangeHysteresis >
                 static_cast<double>(ap.range_48);
    }
    if (!ap.in_range_75) {
        ap.in_range_75 = inside;
    } else {
        ap.in_range_75 = inside || ctl.alt_loadout;                   // 009A60F3 007B58D0
    }

    if (!ap.has_ordnance_74) {                                        // 009A6102, the 009A63F2 arm
        ap.ref.projtime_44 = depth_charge_projtime_007b5350(0.0f);    // 009A63FB
        ap.release_point_f0 = pos;                                    // 009A641C-009A6434
        ap.stick_half_58 = 0.0f;                                      // 009A643F
        ap.off_track_68 = 0.0f;
        if (ctl.alt_loadout) {                                        // 009A6449
            finish();
            return;
        }
        const DepthChargeLeaderView leader = host.leader();          // ctl+3D0h
        if (!leader.is_self) {                                        // 009A645C
            // A null leader would reach 00427EB0 with ECX = 0 in the image.
            const std::array<float, 2> dl{{p[0] - leader.position[0], p[2] - leader.position[2]}};
            if (!(ap.attack_range_44 > length_2d_00414c60(dl))) {    // 009A64B1, JBE
                ap.in_range_75 = false;
                finish();
                return;
            }
        }
        ap.in_range_75 = ap.attack_range_44 > ap.range_48;           // 009A64BD
        finish();
        return;
    }

    // 009A610F: the release lead, the projtime and the release point.
    const std::array<float, 3> vel = host.unit_velocity_34();
    ap.fall_lead_64 = depth_charge_fall_lead_009a3300(p[1], pos[1], vel[1]);
    const float speed = host.unit_speed_38();                         // 009A611E
    const float m = max_by_ref_00415550(1.0f, speed);                 // 009A6141 [00D7A24C]
    const float t1 = static_cast<float>(static_cast<double>(ap.range_48) / m);   // 009A6146
    const float t2 = min_by_ref_00415510(20.0f, t1);                  // 009A6164 [00CE3930]
    const float proj = static_cast<float>(static_cast<double>(t2) + ap.proj_error_6c);
    ap.ref.projtime_44 = depth_charge_projtime_007b5350(proj);        // 009A617B
    const double lead = ap.fall_lead_64;
    const float ox = static_cast<float>(vel[0] * lead);               // 009A61AC
    const float oy = static_cast<float>(vel[1] * lead);               // 009A61B5
    const float oz = static_cast<float>(lead * vel[2]);               // 009A61BC
    ap.release_point_f0 = {static_cast<float>(static_cast<double>(ox) + pos[0]),
                           0.0f,                                      // 009A621F overwrites +F4h
                           static_cast<float>(static_cast<double>(pos[2]) + oz)};
    (void)oy;   // 009A61E1-009A61F5 store unit y + oy to +F4h, then zero it
    const std::array<float, 2> dr{{p[0] - ap.release_point_f0[0], p[2] - ap.release_point_f0[2]}};
    const float dist = length_2d_00414c60(dr);                        // 009A6256
    const float h = heading_angle_00414eb0(dr);                       // 009A6266
    const float a = wrapped_angle_subtract_00438b10(host.unit_heading_50(), h);   // 009A627C
    const float r = 0.0f < a ? a : -0.0f - a;                         // 009A628B, 009A6309
    ap.off_track_68 = r;                                              // 009A6321
    const float lim = max_by_ref_00415550(60.0f, ap.stick_half_58);  // 009A6341 [00CEB4B0]
    if (static_cast<double>(dist) > lim) {                            // 009A634A, JBE
        ap.run_in_60 = ap.range_48;                                   // 009A6356
        ap.release_bias_5c = 0.0f;
        finish();
        return;
    }
    const float c = static_cast<float>(std::cos(static_cast<double>(r)));   // 009A6380 FCOS
    ap.run_in_60 = static_cast<float>(static_cast<double>(c) * dist);       // 009A6396
    if (ap.stick_half_58 > ap.run_in_60) {                            // 009A63A3 FCOMI, JBE
        ap.run_in_60 = static_cast<float>(static_cast<double>(ap.stick_half_58) - 1.0);   // 00D7A210
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009A63B5..009A63C2: stick * dt, * qword 0.1, + bias, each rounded to 24 bits.
            ap.release_bias_5c = static_cast<float>(cc13x87::x87_add24(
                cc13x87::x87_mul24(cc13x87::x87_mul24(ap.stick_half_58, dt), 0.10000000149011612),
                ap.release_bias_5c));
        } else {
            ap.release_bias_5c = static_cast<float>(
                static_cast<double>(ap.stick_half_58) * dt * 0.10000000149011612 +   // 00D7A3A0
                ap.release_bias_5c);
        }
    } else {
        ap.release_bias_5c = 0.0f;                                    // 009A63E3
    }
    finish();
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
void depth_charge_task_construct_009a5240(DepthChargeTask& t, DepthChargeTaskHost& host,
                                          DepthChargeRandom& rng) {
    t = DepthChargeTask{};
    DepthChargeApproach& ap = t.ap;
    const DepthChargeTuning tu = host.tuning();
    const DepthChargeClass cls = host.class_block();
    const DepthChargeRow row = host.robot_row();
    // 009F9CE0 with ReferenceSpeed (+4B8h): +24h = max(1.0, MaxSpd / it).
    const float ratio = cls.max_spd_188 / tu.reference_speed_4b8;
    ap.speed_ratio_24 = 1.0f < ratio ? ratio : 1.0f;
    ap.drop_balance_2c = 0;                                           // 009A33F1
    // 009A33F8-009A340C: the draw spilled as a double, times 007C47F0.
    const double u30 = draw(rng, DepthChargeDraw::kRunSpeed30, 0.9f, 1.1f);
    ap.run_speed_30 = static_cast<float>(static_cast<double>(cls.level_flight_speed) * u30);
    const float span = tu.aim_alt_range_2_498 - tu.aim_alt_range_1_494;   // 009A342F FSTP float
    ap.aim_alt_spread_34 = draw(rng, DepthChargeDraw::kAimAltSpread34, 0.0f, span);
    ap.maneuver_alt_38 = draw(rng, DepthChargeDraw::kManeuverAlt38, tu.maneuver_alt_range_1_49c,
                              tu.maneuver_alt_range_2_4a0);
    ap.aim_alt_base_3c = tu.aim_alt_range_1_494;                      // 009A3482
    // 009A34AA-009A34E4: U(1.1, 1.3) * TurnCircleRadius + FlyAboveDist * +24h.
    const double u40 = draw(rng, DepthChargeDraw::kFlyAbove40, 1.1f, 1.3f);
    ap.fly_above_40 = static_cast<float>(u40 * cls.turn_circle_radius_268 +
                                         static_cast<double>(tu.fly_above_dist_4a8) *
                                             ap.speed_ratio_24);
    ap.attack_range_44 = static_cast<float>(ap.fly_above_40 * depth_charge::kRangeOverFlyAbove);
    ap.range_48 = ap.attack_range_44;                                 // 009A34E4
    ap.bearing_4c = 0.0f;
    ap.rel_bearing_50 = 0.0f;
    ap.elapsed_54 = depth_charge::kElapsedSeed;
    ap.stick_half_58 = depth_charge::kStickHalfSeed;
    ap.release_bias_5c = 0.0f;
    ap.off_track_68 = 0.0f;
    ap.lost_timer_70 = 0.0f;
    ap.has_ordnance_74 = true;                                        // 009A351D
    ap.in_range_75 = false;
    ap.arm_request_76 = false;
    ap.weapon_select_78 = depth_charge::kWeaponSelectNone;
    // 009A3530: 009FB200 on approach+7Ch with the command's target, first timer U(0, 2).
    ap.ref = approach_target_ref_construct_009fb200(
        rng.uniform_00bd2f10(kDepthChargeRefTimerKey, 0.0f, 2.0f));
    ap.ref.tracking = true;                                           // sub+18h = the target
    ap.ref.has_target_40 = true;
    ap.release_point_f0 = host.unit_position();                       // 009A354D-009A356D
    depth_charge_pick_aim_offset_009a31a0(ap, row, host, rng);        // 009A3573
    ap.fall_lead_64 = depth_charge_fall_lead_009a3300(                // 009A357A
        ap.ref.world_point_1c[1], ap.release_point_f0[1], host.unit_velocity_34()[1]);
    ap.stick_half_58 =                                                // 009A3581
        depth_charge_stick_half_009a3290(host.remaining_rounds_007c1db0(), ap.run_speed_30);

    // 009A4DC0: moveto and follow (the host's), done, attackrun, goaway, aim,
    // prepare, leave, turnto.
    t.done = DepthChargeFollowState{};                                // +98h = -1, +9Ch = 0
    t.ar.period_18 = 1.0f;                                            // 009A36F5
    t.ar.countdown_1c = -draw(rng, DepthChargeDraw::kAttackRunCd1C, 0.0f, 1.0f);   // 009A36FF FCHS
    t.ar.offset_20 = 0.0f;
    t.ga = DepthChargeGoAwayState{};
    t.aim = DepthChargeAimState{};
    t.prepare = DepthChargeFollowState{};
    t.lv.timer_18 = depth_charge::kLeaveTimerSeed;
    t.tt.bank_18 = 0.0f;
    // 009A529D-009A52C7: moveto for the flight leader, else follow, entered by
    // its vt[4]; then 009F9980 (BSP_BotApproach_BindToTask, the host's).
    t.state = host.flight_leader_007b8ad0() ? DepthChargeState::kMoveTo : DepthChargeState::kFollow;
    host.enter_formation_state(t.state);
}

// ---------------------------------------------------------------------------
// The rule and the predicates
// ---------------------------------------------------------------------------
bool depth_charge_turnto_aim_ready_009a5610(float heading_diff_009a19b0,
                                            float rel_bearing_50) noexcept {
    const float x = heading_diff_009a19b0;
    const float ax = x > 0.0f ? x : -0.0f - x;                        // 009A5639
    const float ab = rel_bearing_50 > 0.0f ? rel_bearing_50 : -0.0f - rel_bearing_50;   // 009A5654
    const float m = kHalf > static_cast<double>(ax) ? 0.5f : ax;      // 009A567D, [00CE3800]
    return m > ab;                                                    // 009A569E
}

bool depth_charge_turnto_give_up_009a5390(float range_48, float fly_above_40) noexcept {
    return static_cast<double>(fly_above_40) * kHalf > static_cast<double>(range_48);   // 009A539F
}

bool depth_charge_goaway_done_009a53b0(const DepthChargeGoAwayState& ga, float range_48,
                                       bool alt_loadout, bool has_ordnance_74) noexcept {
    float s = ga.standoff_1c;
    if (alt_loadout) {                                                // 009A53BC, 009A53CA
        if (!has_ordnance_74) return false;                           // 009A53D9
        s = static_cast<float>(static_cast<double>(s) - depth_charge::kArmBand);   // 009A53E0
    }
    return range_48 > s;                                              // 009A53EF
}

bool depth_charge_should_break_off_009a65f0(const DepthChargeTask& t, DepthChargeTaskHost& host) {
    if (!host.break_off_base_0099c230()) return false;                // 009A65F6
    if (!t.ap.ref.tracking || host.target_marked_5d()) return true;   // 009A660E, 009A6618
    if (host.control().alt_loadout) return false;                     // 009A6624-009A6634
    if (depth_charge_is_attack_state(t.state) && t.ap.has_ordnance_74) return false;   // 009A663F
    const std::array<float, 3> pos = host.unit_position();            // 009A6658
    const std::array<float, 3>& p = t.ap.ref.world_point_1c;
    const float dx = p[0] - pos[0], dy = p[1] - pos[1], dz = p[2] - pos[2];
    bool keep;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 0042B2F9..0042B307 (the 0042B2F0 copy): (dy*dy + dx*dx) + dz*dz per op.
        const float len = length_or_zero(cc13x87::x87_add24(
            cc13x87::x87_add24(cc13x87::x87_mul24(dy, dy), cc13x87::x87_mul24(dx, dx)),
            cc13x87::x87_mul24(dz, dz)));
        // 009A66A0..009A66AD: the FMUL stays on the stack, rounded to 24 bits, for the FCOMIP.
        const double safe = cc13x87::x87_mul24(host.tuning().safe_dist_4b0, t.ap.speed_ratio_24);
        keep = !(safe > len);
    } else {
        const float len = length_or_zero((static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) +
                                         static_cast<double>(dz) * dz);   // 009A669B 0042B2F0
        const double safe = static_cast<double>(host.tuning().safe_dist_4b0) * t.ap.speed_ratio_24;
        keep = !(safe > len);                                         // 009A66AD, JA keeps
    }
    return keep;
}

int depth_charge_rule_009a5870(DepthChargeTask& t, DepthChargeTaskHost& host,
                               DepthChargeRandom& rng, int& releases) {
    DepthChargeTransitionInputs in;
    in.current = t.state;
    in.control_mode = host.control().attack_mode_370;
    in.flag_46C = t.ap.has_ordnance_74;
    in.flag_46D = t.ap.in_range_75;
    in.has_latched_target = t.ap.ref.tracking;
    in.approach_has_ordnance = t.ap.has_ordnance_74;
    in.aim_pull_out = t.aim.pull_out_18;
    in.leave_countdown = t.lv.timer_18;
    const bool attacking = depth_charge_is_attack_state(t.state);
    const bool engaged = in.flag_46D || (in.control_mode == 2 && in.has_latched_target);
    // 007B8AD0 is called only on the approach-state branches.
    if (!engaged) in.unit_has_no_follow_target = host.flight_leader_007b8ad0();
    // vt[1Ch] only past the prepare and done tests (009A5929).
    if (attacking && engaged && in.control_mode != 0 && t.state != DepthChargeState::kDone) {
        in.should_break_off = depth_charge_should_break_off_009a65f0(t, host);
    }
    if (attacking && engaged && in.control_mode != 0 && !in.should_break_off) {
        if (t.state == DepthChargeState::kTurnTo) {
            in.turn_to_aim_ready = depth_charge_turnto_aim_ready_009a5610(
                host.heading_diff_009a19b0(), t.ap.rel_bearing_50);
            if (!in.turn_to_aim_ready) {
                in.turn_to_give_up =
                    depth_charge_turnto_give_up_009a5390(t.ap.range_48, t.ap.fly_above_40);
            }
        } else if (t.state == DepthChargeState::kGoAway) {
            in.go_away_finished = depth_charge_goaway_done_009a53b0(
                t.ga, t.ap.range_48, host.control().alt_loadout, t.ap.has_ordnance_74);
        }
    }
    return set_state(t, depth_charge_next_state(in), host, rng, releases);
}

int depth_charge_step_result_009a5d80(const DepthChargeTask& t) noexcept {
    if (!depth_charge_is_attack_state(t.state)) return 1;            // 009A5DA2
    return t.ap.arm_request_76 ? 2 : 0;                               // 009A5D93 NEG/SBB/AND 2
}

int depth_charge_slot20_009a5780(const DepthChargeTask& t, const DepthChargeTargetFlags& f,
                                 float cancel_target_dist_0d8) noexcept {
    if (f.present && f.flag_5c && !f.flag_5d && !f.flag_60 && !f.flag_5e &&
        t.ap.has_ordnance_74 && t.ap.run_in_60 < cancel_target_dist_0d8) {   // 009A57BF, JNC
        return 0;
    }
    return 1;
}

int depth_charge_drop_request_009a5d00(DepthChargeTask& t, DepthChargeTaskHost& host,
                                       DepthChargeRandom& rng) {
    if (t.ap.has_ordnance_74 && host.device_drop_requested()) {       // 009A5D03, 009A5D21
        t.ap.drop_balance_2c += 1;                                    // 009A5D27
        if (t.state == DepthChargeState::kPrepare) prepare_arm_009a5330(t.prepare, rng);
    }
    return 1;
}

int depth_charge_hit_notice_009a54c0(DepthChargeTask& t) noexcept {
    t.ap.elapsed_54 = 0.0f;                                           // 009A54C3 task+44Ch
    return 1;
}

int depth_charge_flush_009a5710(DepthChargeTask& t, DepthChargeTaskHost& host) {
    int releases = 0;
    if (host.device_drop_requested() && t.ap.drop_balance_2c > 0) {   // 009A5728, 009A572E
        do {
            host.release_ordnance_007bbba0();                         // 009A5746
            ++releases;
            t.ap.drop_balance_2c -= 1;
        } while (t.ap.drop_balance_2c > 0);
    }
    t.ap.drop_balance_2c = 0;                                         // 009A5763
    releases += exit_state(t, t.state, host);                         // 009A5775 JMP vt[8]
    return releases;
}

// ---------------------------------------------------------------------------
// The states
// ---------------------------------------------------------------------------
void depth_charge_attackrun_enter_009a3750(DepthChargeAttackRunState& ar) noexcept {
    ar.offset_20 = 0.0f;
    ar.period_18 = 0.4000000059604645f;                               // 00CE7804
}

DepthChargePlanCommand depth_charge_attackrun_tick_009a3770(DepthChargeTask& t,
                                                            DepthChargeTaskHost& host, float dt) {
    DepthChargePlanCommand c;
    DepthChargeAttackRunState& ar = t.ar;
    const DepthChargeApproach& ap = t.ap;
    const float bearing = ap.bearing_4c;                              // 009A377D
    const float cd = ar.countdown_1c;
    if (dt < cd) {                                                    // 009A3797 FCOMI, JC
        ar.countdown_1c = static_cast<float>(static_cast<double>(cd) - dt);
    } else {
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009A37A2..009A37C8: FSUBR period - dt, FADDP + cd.
            ar.countdown_1c = static_cast<float>(
                cc13x87::x87_add24(cc13x87::x87_sub24(ar.period_18, dt), cd));
        } else {
            ar.countdown_1c = static_cast<float>((static_cast<double>(ar.period_18) - dt) + cd);
        }
        const float ext[3] = {80.0f, 60.0f, 120.0f};                  // 00CE5444, 00CEB4B0, 00D05804
        const float w[3] = {0.0f, 0.0f, 0.0f};
        const NearFieldProbeResult pr = host.near_field_probe_007f0280(ext, w);   // 009A3805
        float u;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009A380A..009A3818: FCHS, FMUL, FMUL, each product rounded to 24 bits.
            u = static_cast<float>(cc13x87::x87_mul24(
                cc13x87::x87_mul24(-static_cast<double>(pr.out_a[0]), pr.out_b[1]), pr.out_b[2]));
        } else {
            u = static_cast<float>(-static_cast<double>(pr.out_a[0]) * pr.out_b[1] *
                                   pr.out_b[2]);                      // 009A3818
        }
        ar.offset_20 = static_cast<float>(static_cast<double>(u) * kThirtyDegrees);   // 009A3826
    }
    c.writes_heading_2c0 = true;                                      // 009A384C, mode 2
    c.heading_2c0 = wrapped_angle_add_00438aa0(bearing, ar.offset_20);
    write_cruise(c, aim_altitude(ap), ap.fly_above_40, ap.range_48, 0.9f);   // 009A388C, 00CE3860
    write_throttle(c, 0.9800000190734863f);                           // 009A389E, 00CE6650
    c.writes_cone_40 = true;                                          // 009A38D9
    c.cone_40 = host.tuning().angle_move_to_670;
    c.calls_gun_direction_009fabe0 = true;                            // 009A38FC
    c.gun_heading_from_plan = true;                                   // 009A1A20(plan)
    return c;
}

void depth_charge_leave_enter_009a39c0(DepthChargeLeaveState& lv, DepthChargeRandom& rng) {
    lv.timer_18 = draw(rng, DepthChargeDraw::kLeaveTimer18, 4.0f, 7.0f);   // 00CE3D34, 00CF5BF8
}

DepthChargePlanCommand depth_charge_leave_tick_009a39f0(DepthChargeTask& t, float dt) noexcept {
    DepthChargePlanCommand c;
    t.ap.weapon_select_78 = depth_charge::kWeaponSelectDepthCharge;   // 009A39F7
    t.lv.timer_18 = static_cast<float>(static_cast<double>(t.lv.timer_18) - dt);
    c.calls_pitch_command_009fb800 = true;                            // 009A3A20
    c.pitch_altitude = 2000.0f;                                       // 00CFFD60
    c.pitch_reference = 1.5f;                                         // 00CE380C
    c.writes_bank_2c4 = true;                                         // 009A3A35, mode 1
    c.bank_2c4 = 0.0f;
    write_throttle(c, 1.0f);                                          // 009A3A4D [00D7A24C]
    return c;
}

void depth_charge_turnto_enter_009a3ae0(DepthChargeTask& t, DepthChargeTaskHost& host) {
    const float sign = t.ap.rel_bearing_50 > 0.0f ? -1.0f : 1.0f;     // 009A3AFD
    const float mult = host.bomber_kind() ? 1.1f : 1.3f;              // 00CE6448, 00CEB4B4
    // pc24-lint: allow sign is +-1, so roll * sign is exact and the FMUL pair at 009A3B59..009A3B5D rounds once
    t.tt.bank_18 = static_cast<float>(static_cast<double>(host.class_block().turn_roll_25c) *
                                      sign * mult);                   // 009A3B4D-009A3B61
}

DepthChargePlanCommand depth_charge_turnto_tick_009a3b70(DepthChargeTask& t,
                                                         DepthChargeTaskHost& host) {
    DepthChargePlanCommand c;
    DepthChargeApproach& ap = t.ap;
    ap.weapon_select_78 = depth_charge::kWeaponSelectDepthCharge;     // 009A3B74
    c.writes_bank_2c4 = true;                                         // 009A3B84, mode 1
    c.bank_2c4 = t.tt.bank_18;
    c.writes_speed_2b4 = true;                                        // 009A3BAC
    c.speed_2b4 = host.class_block().travel_speed_190;
    const float low = static_cast<float>(static_cast<double>(ap.fly_above_40) *
                                         1.399999976158142);          // 00D045F0
    write_cruise(c, ap.maneuver_alt_38, low, ap.range_48, 0.6000000238418579f);   // 00CE3D30
    return c;
}

void depth_charge_goaway_enter_009a3c60(DepthChargeTask& t, DepthChargeTaskHost& host,
                                        DepthChargeRandom& rng) {
    // 009A3C60-009A3C75: [00F876B0] % 2, signed; odd gives -1.
    t.ga.side_20 = (host.parity_f876b0() % 2) != 0 ? -1.0f : 1.0f;
    t.ga.altitude_18 = t.ap.has_ordnance_74 ? t.ap.maneuver_alt_38 : host.control().alt_394;
    t.ap.arm_request_76 = false;                                      // 009A3CC1
    const double u = draw(rng, DepthChargeDraw::kGoAwayStandoff1C, 200.0f, 400.0f);   // 00CE386C, 00CFD710
    t.ga.standoff_1c = static_cast<float>(u + host.class_block().turn_circle_radius_268);
}

float depth_charge_goaway_heading_009a3cf0(DepthChargeTask& t, DepthChargeTaskHost& host) {
    const DepthChargeApproach& ap = t.ap;
    const float h0 = host.fly_to_heading_009fd570(ap.ref.world_point_1c, t.ga.standoff_1c,
                                                  t.ga.side_20, ap.range_48);   // 009A3D2D
    const float ext[3] = {60.0f, 50.0f, 80.0f};                       // 00CEB4B0, 00CEB4D4, 00CE5444
    const float w[3] = {0.0f, 0.0f, 0.0f};
    const NearFieldProbeResult pr = host.near_field_probe_007f0280(ext, w);   // 009A3D95
    float u;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A3D9A..009A3DB3: FCHS, FMUL, FMUL, each product rounded to 24 bits.
        u = static_cast<float>(cc13x87::x87_mul24(
            cc13x87::x87_mul24(-static_cast<double>(pr.out_a[0]), pr.out_b[1]), pr.out_b[2]));
    } else {
        u = static_cast<float>(-static_cast<double>(pr.out_a[0]) * pr.out_b[1] *
                               pr.out_b[2]);                          // 009A3DB3
    }
    const float e = wrapped_angle_subtract_00438b10(h0, host.unit_heading_50());   // 009A3DC7
    float c;
    if (-kGoAwayClamp > static_cast<double>(e)) {                     // 009A3DDA
        c = -kGoAwayClampF;
    } else if (static_cast<double>(e) > kGoAwayClamp) {               // 009A3DF2
        c = kGoAwayClampF;
    } else {
        c = e;
    }
    if ((c > 0.0f && 0.0f > u) || (0.0f > c && u > 0.0f)) {           // 009A3E0B-009A3E2F
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009A3E31..009A3E3F: FMUL qword 30 degrees, FADD c, each rounded to 24 bits.
            c = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(u, kThirtyDegrees), c));
        } else {
            c = static_cast<float>(static_cast<double>(u) * kThirtyDegrees + c);
        }
    }
    return wrapped_angle_add_00438aa0(host.unit_heading_50(), c);    // 009A3E5C
}

DepthChargePlanCommand depth_charge_goaway_tick_009a3e70(DepthChargeTask& t,
                                                         DepthChargeTaskHost& host) {
    DepthChargePlanCommand c;
    write_throttle(c, 1.0f);                                          // 009A3E88
    c.calls_pitch_command_009fb800 = true;                            // 009A3EC2
    c.pitch_altitude = t.ga.altitude_18;
    c.pitch_reference = 1.0f;
    c.writes_heading_2c0 = true;                                      // 009A3ED3, mode 2
    c.heading_2c0 = depth_charge_goaway_heading_009a3cf0(t, host);    // 009A3ECE
    c.writes_cone_40 = true;                                          // 009A3EF3
    c.cone_40 = host.tuning().angle_go_away_674;
    // 009A3F09: the heading is computed a second time for the gun direction;
    // the fly-to solver writes the side back each call.
    c.calls_gun_direction_009fabe0 = true;
    c.gun_heading = depth_charge_goaway_heading_009a3cf0(t, host);
    return c;
}

void depth_charge_aim_enter_009a3fe0(DepthChargeTask& t, DepthChargeTaskHost& host,
                                     DepthChargeRandom& rng) {
    t.aim.pull_out_18 = false;                                        // 009A3FEB
    t.aim.cooldown_1c = 0.0f;
    t.aim.byte_19 = false;
    depth_charge_pick_aim_offset_009a31a0(t.ap, host.robot_row(), host, rng);   // 009A3FF6
    t.ap.stick_half_58 =                                              // 009A3FFF JMP 009A3290
        depth_charge_stick_half_009a3290(host.remaining_rounds_007c1db0(), t.ap.run_speed_30);
}

DepthChargeAimResult depth_charge_aim_tick_009a4010(DepthChargeTask& t, DepthChargeTaskHost& host,
                                                    DepthChargeRandom& rng, float dt) {
    DepthChargeAimResult out;
    DepthChargePlanCommand& c = out.plan;
    DepthChargeApproach& ap = t.ap;
    DepthChargeAimState& aim = t.aim;
    aim.cooldown_1c = static_cast<float>(static_cast<double>(aim.cooldown_1c) - dt);   // 009A4025
    const float bearing = ap.bearing_4c;                              // 009A4028
    const float r = ap.run_in_60;                                     // 009A402D
    const float h = host.unit_heading_50();                           // 009A4044
    const float a = static_cast<float>(
        static_cast<double>(clamped_interpolate_00419010(80.0f, 0.8f, 300.0f, 2.0f, r)) * 100.0);
    const float b = clamped_interpolate_00419010(50.0f, 0.3490658700466156f, 350.0f,
                                                 1.3089970350265503f, r);   // 009A40BD
    // 009A40D4-009A4122: extents (A, 150, 250), weights (A * 0.4, 0, 0), mode 1.
    const float ext[3] = {a, 150.0f, 250.0f};                         // 00CE3808, 00CE77B0
    const float w[3] = {static_cast<float>(static_cast<double>(a) * kPointFour), 0.0f, 0.0f};
    const NearFieldProbeResult pr = host.near_field_probe_007f0280(ext, w);
    float o = 0.0f;
    float speed_bias = 0.0f;                                          // [ESP+1Ch]
    const float ax = pr.out_a[0] > 0.0f ? pr.out_a[0] : -pr.out_a[0];   // AND 7FFFFFFFh
    if (static_cast<double>(ax) > kProbeGate) {                       // 009A415A
        float u;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // 009A4164..009A417A: FCHS a0, FMULP b1, FMULP b2; 009A417E..009A418A: FCHS a2,
            // FMUL b0, FMULP b1. Each product rounds to 24 bits.
            u = static_cast<float>(cc13x87::x87_mul24(
                cc13x87::x87_mul24(-static_cast<double>(pr.out_a[0]), pr.out_b[1]), pr.out_b[2]));
            speed_bias = static_cast<float>(cc13x87::x87_mul24(
                cc13x87::x87_mul24(-static_cast<double>(pr.out_a[2]), pr.out_b[0]), pr.out_b[1]));
        } else {
            u = static_cast<float>(-static_cast<double>(pr.out_a[0]) * pr.out_b[1] *
                                   pr.out_b[2]);                      // 009A417A
            speed_bias = static_cast<float>(-static_cast<double>(pr.out_a[2]) * pr.out_b[0] *
                                            pr.out_b[1]);             // 009A418A
        }
        if (u > 0.5f) {                                               // 009A4198 [00CE3800]
            o = static_cast<float>((static_cast<double>(u) - kHalf) * 2.0);
        } else if (-0.5f > u) {                                       // 009A41B4 [00CE69D0]
            o = static_cast<float>((static_cast<double>(u) + kHalf) * 2.0);
        } else {
            o = 0.0f;
        }
        const float k = clamped_interpolate_00419010(100.0f, 0.25f, 350.0f, 1.0f, r);   // 009A41FF
        const float of = static_cast<float>(static_cast<double>(o) * kFifteenDegrees);  // 009A420E
        o = static_cast<float>(static_cast<double>(k) * of);
    }
    const float desired = wrapped_angle_add_00438aa0(bearing, o);     // 009A422C
    const float diff = wrapped_angle_subtract_00438b10(desired, h);   // 009A4247
    const float nb = -b;
    const float cl = nb > diff ? nb : (diff > b ? b : diff);          // 009A4264, 009A4274
    c.writes_heading_2c0 = true;                                      // 009A42B3, mode 2
    c.heading_2c0 = wrapped_angle_add_00438aa0(h, cl);
    c.writes_turn_2c8 = true;                                         // 009A42FF
    c.turn_2c8 = clamped_interpolate_00419010(250.0f, 0.2f, 500.0f, 1.2f, r);
    // 009A4307-009A432B: e = r - (float(+5Ch + +58h) + 150), floored at 1.
    const float window = ap.release_bias_5c + ap.stick_half_58;
    float e;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A4323..009A4329: FADD qword 150, FSUBP, each rounded to 24 bits.
        e = static_cast<float>(cc13x87::x87_sub24(
            r, cc13x87::x87_add24(window, depth_charge::kReleaseCeiling)));
    } else {
        e = static_cast<float>(static_cast<double>(r) -
                               (static_cast<double>(window) + depth_charge::kReleaseCeiling));
    }
    if (1.0f > e) e = 1.0f;                                           // 009A4340
    const std::array<float, 3> pos = host.unit_position();
    const float alt = aim_altitude(ap);                               // 009A436B
    float g;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A4373..009A437B: FSUB qword 30, FSUBP, FDIV e, each rounded to 24 bits.
        g = static_cast<float>(cc13x87::x87_div24(
            cc13x87::x87_sub24(pos[1], cc13x87::x87_sub24(alt, kProjCap)), e));
    } else {
        g = static_cast<float>(
            (static_cast<double>(pos[1]) - (static_cast<double>(alt) - kProjCap)) / e);   // 009A437B
    }
    const float scale = clamped_interpolate_00419010(0.01f, 0.8f, 0.35f, 1.8f, g);   // 009A43B2
    write_cruise(c, alt, ap.fly_above_40, ap.range_48, scale);        // 009A43DF
    const DepthChargeClass cls = host.class_block();
    const float spd_a = clamped_interpolate_00419010(100.0f, ap.run_speed_30, 450.0f,
                                                     cls.travel_speed_190, r);   // 009A4418
    const float k1 = clamped_interpolate_00419010(0.15f, 0.0f, 0.7f, 2.0f, g);   // 009A444F
    const float k2 = static_cast<float>(
        static_cast<double>(clamped_interpolate_00419010(450.0f, 1.2f, 1000.0f, 0.0f, r)) * k1);
    const float lo = static_cast<float>(static_cast<double>(alt) + 80.0);    // 00CF1440
    const float hi = static_cast<float>(static_cast<double>(alt) + 180.0);   // 00CE3D20
    const float alt_f = clamped_interpolate_00419010(lo, 0.0f, hi, 1.0f, pos[1]);   // 009A4500
    const float v = static_cast<float>(static_cast<double>(alt_f) * k2);
    const float speed = clamped_interpolate_00419010(0.0f, spd_a, 1.0f, cls.level_flight_speed, v);
    c.writes_speed_2b4 = true;                                        // 009A4565
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A4540..009A4565: FMUL qword 15, FADD speed, each rounded to 24 bits.
        c.speed_2b4 = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(speed_bias, 15.0), speed));
    } else {
        c.speed_2b4 = static_cast<float>(static_cast<double>(speed_bias) * 15.0 + speed);   // 00CF3F20
    }
    aim.pull_out_18 = ap.rel_bearing_50 > depth_charge::kPullOutBearing;   // 009A4572
    // 009A4584-009A45A3: arm inside 200 m of the release window.
    double run_left;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A4595: the FSUB stays on the stack, rounded to 24 bits, for the FCOMIP.
        run_left = cc13x87::x87_sub24(r, window);
    } else {
        run_left = static_cast<double>(r) - static_cast<double>(window);
    }
    if (!(depth_charge::kArmBand > run_left)) {
        ap.arm_request_76 = false;                                    // 009A465D
        return out;
    }
    ap.arm_request_76 = true;                                         // 009A45A9
    ap.weapon_select_78 = depth_charge::kWeaponSelectDepthCharge;     // 009A45AF
    if (!ap.ref.tracking || host.target_marked_5d() || !ap.has_ordnance_74) return out;
    if (!(0.0f > aim.cooldown_1c)) return out;                        // 009A45DD
    if (!depth_charge_in_release_window(ap.run_in_60, ap.stick_half_58, ap.release_bias_5c)) {
        return out;                                                   // 009A45E5 009A3FA0
    }
    const float ceiling = static_cast<float>(static_cast<double>(aim_altitude(ap)) +
                                             depth_charge::kReleaseCeiling);   // 009A4607
    if (!(ceiling > host.unit_position()[1])) return out;             // 009A4615
    host.release_ordnance_007bbba0();                                 // 009A4620
    out.released = true;
    ap.drop_balance_2c -= 1;                                          // 009A462D
    const double u = draw(rng, DepthChargeDraw::kAimCooldown1C, 0.9f, 1.1f);   // 009A4646
    aim.cooldown_1c = static_cast<float>(u * kPointFour);             // 009A4653
    return out;
}

void depth_charge_follow_enter_009a4890(DepthChargeTask& t, DepthChargeState s,
                                        DepthChargeTaskHost& host) {
    DepthChargeFollowState& fs = follow_state(t, s);
    fs.gate_98 = -1.0f;                                               // 009A489B [00D7A260]
    host.enter_formation_state(s);                                    // 009A48A3 009BED80
    host.set_control_3e4(4);                                          // 009A48AE
    host.assign_formation_indices_007ed260();                         // 009A48B8
    fs.speed_8c = 1.0f;                                               // 009A48C5
}

bool depth_charge_follow_exit_009a48d0(DepthChargeTask& t, DepthChargeState s,
                                       DepthChargeTaskHost& host) {
    DepthChargeFollowState& fs = follow_state(t, s);
    bool released = false;
    if (!(fs.gate_98 < 0.0f)) {                                       // 009A48DB COMISS, JC
        host.release_ordnance_007bbba0();                             // 009A48EA
        t.ap.drop_balance_2c -= 1;
        released = true;
    }
    host.exit_formation_state(s);                                     // 009A48F9 JMP 009BDE40
    return released;
}

DepthChargeFollowResult depth_charge_follow_tick_009a4940(DepthChargeTask& t, DepthChargeState s,
                                                          DepthChargeTaskHost& host, float dt) {
    DepthChargeFollowResult out;
    DepthChargePlanCommand& c = out.plan;
    DepthChargeFollowState& fs = follow_state(t, s);
    DepthChargeApproach& ap = t.ap;
    host.tick_formation_state(s, dt, fs.speed_8c);                    // 009A4950 009C1FD0
    if (fs.gate_98 > 0.0f) {                                          // 009A4966, JBE
        ap.weapon_select_78 = depth_charge::kWeaponSelectDepthCharge; // 009A4975
        c.writes_cone_40 = true;                                      // 009A4982
        c.cone_40 = 0.0f;
        ap.arm_request_76 = true;                                     // 009A498A
        fs.gate_98 = static_cast<float>(static_cast<double>(fs.gate_98) - dt);   // 009A499B
        if (static_cast<double>(ap.run_in_60) > depth_charge::kHighAltitude) {   // 009A49AA
            fs.gate_98 = -1.0f;
        }
        const bool drop = !(0.0f < fs.gate_98) ||                     // 009A49C0 JNC
            depth_charge_in_release_window(ap.run_in_60, ap.stick_half_58, ap.release_bias_5c) ||
            ap.off_track_68 > depth_charge::kPullOutBearing;          // 009A49ED
        if (drop) {
            host.release_ordnance_007bbba0();                         // 009A4A89
            out.released = true;
            ap.drop_balance_2c -= 1;
            fs.salvo_9c -= 1;
            if (fs.salvo_9c <= 0) fs.gate_98 = -1.0f;                 // 009A4AA5
            ap.stick_half_58 = depth_charge_stick_half_009a3290(      // 009A4ABA
                host.remaining_rounds_007c1db0(), ap.run_speed_30);
            if (fs.salvo_9c > 0) fs.gate_98 = 0.30000001192092896f;   // 009A4ACC [00CE69C8]
            return out;
        }
        c.writes_pitch_2bc = true;                                    // 009A49FD
        c.pitch_2bc = 0.0f;
        c.pitch_mode_2d0 = 2;
        // 009A4A43 00415690: clamp in place into [-0.3, 0.3], low first, strict.
        float a = ap.rel_bearing_50;
        if (a < -0.30000001192092896f) a = -0.30000001192092896f;    // [00D06888]
        if (a > 0.30000001192092896f) a = 0.30000001192092896f;      // [00CE69C8]
        c.writes_heading_2c0 = true;                                  // 009A4A72, mode 2
        c.heading_2c0 = wrapped_angle_add_00438aa0(host.unit_heading_50(), a);
        return out;
    }
    c.writes_cone_40 = true;                                          // 009A4AF8
    c.cone_40 = host.tuning().angle_prepare_66c;
    const DepthChargeLeaderView leader = host.leader();
    if (!leader.present) return out;                                  // 009A4AFB
    ap.arm_request_76 = leader.arm_9c1 != 0;                          // 009A4B12
    const float ly = leader.position[1];
    if (static_cast<double>(ly) > depth_charge::kHighAltitude) {      // 009A4B39
        fs.speed_8c = 1.0f;
        return out;
    }
    const std::array<float, 3>& p = ap.ref.world_point_1c;           // 009A4B76 vt[0]
    const std::array<float, 2> d{{p[0] - leader.position[0], p[2] - leader.position[2]}};
    const float hdg = heading_angle_00414eb0(d);                      // 009A4BA5
    const float e0 = wrapped_angle_subtract_00438b10(leader.heading_50, hdg);   // 009A4BBB
    const float e = 0.0f < e0 ? e0 : -0.0f - e0;                      // 009A4BCA
    const float dist = length_2d_00414c60(d);                         // 009A4BFA
    const float k1 = clamped_interpolate_00419010(0.5f, 1.0f, 1.5f, 0.2f, e);   // 009A4C31
    const DepthChargeClass cls = host.class_block();
    float q;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 009A4C36..009A4C4D: FMUL, FADD, FMULP k1, each rounded to 24 bits.
        q = static_cast<float>(cc13x87::x87_mul24(
            k1, cc13x87::x87_add24(cc13x87::x87_mul24(ap.speed_ratio_24, host.tuning().fly_above_dist_4a8),
                                   cls.turn_circle_radius_268)));
    } else {
        q = static_cast<float>(
            static_cast<double>(k1) *
            (static_cast<double>(ap.speed_ratio_24) * host.tuning().fly_above_dist_4a8 +
             cls.turn_circle_radius_268));                            // 009A4C36-009A4C4D
    }
    const float k2 = clamped_interpolate_00419010(250.0f, 1.0f, 400.0f, 0.01f, ly);   // 009A4C84
    const float kq = static_cast<float>(static_cast<double>(k2) * q);
    const float x = static_cast<float>(static_cast<double>(dist) / kq);   // 009A4C98 FDIVR
    fs.speed_8c = clamped_interpolate_00419010(1.1f, 1.0f, 0.4f, 0.0f, x);   // 009A4CC7
    return out;
}

// ---------------------------------------------------------------------------
// The arm and the profile
// ---------------------------------------------------------------------------
DepthChargeArmResult depth_charge_task_arm_009a66c0(DepthChargeTask& t, DepthChargeTaskHost& host,
                                                    DepthChargeRandom& rng, float dt) {
    DepthChargeArmResult res;
    res.before = t.state;
    DepthChargeApproach& ap = t.ap;
    ap.weapon_select_78 = depth_charge::kWeaponSelectNone;            // 009A66D5 task+470h
    depth_charge_approach_update_009a5f50(ap, host, dt);              // 009A66DF
    const float alt = aim_altitude(ap);                               // 009A66FD, 009A670F
    host.set_moveto_ranges_009bde80(alt, alt, ap.fly_above_40);       // 009A671A task+438h
    // 009A6729: the rule, with the exit and enter it runs.
    res.transitions = depth_charge_rule_009a5870(t, host, rng, res.releases);
    // 009A6741: the current state's vt[0Ch](dt).
    switch (t.state) {
        case DepthChargeState::kAttackRun:
            res.plan = depth_charge_attackrun_tick_009a3770(t, host, dt);
            break;
        case DepthChargeState::kLeave: res.plan = depth_charge_leave_tick_009a39f0(t, dt); break;
        case DepthChargeState::kTurnTo: res.plan = depth_charge_turnto_tick_009a3b70(t, host); break;
        case DepthChargeState::kGoAway: res.plan = depth_charge_goaway_tick_009a3e70(t, host); break;
        case DepthChargeState::kAim: {
            const DepthChargeAimResult r = depth_charge_aim_tick_009a4010(t, host, rng, dt);
            res.plan = r.plan;
            res.releases += r.released ? 1 : 0;
            break;
        }
        case DepthChargeState::kDone:
        case DepthChargeState::kPrepare: {
            const DepthChargeFollowResult r = depth_charge_follow_tick_009a4940(t, t.state, host, dt);
            res.plan = r.plan;
            res.releases += r.released ? 1 : 0;
            break;
        }
        default:
            host.tick_formation_state(t.state, dt, 1.0f);
            break;
    }
    host.apply_plan(res.plan);
    t.task_2e4 = ap.weapon_select_78;                                 // 009A6750
    // 009A6743-009A67B0: the manual-release passthrough.
    if (ap.drop_balance_2c > 0 && host.device_drop_requested() &&
        manual_release_allowed(t.state)) {
        host.release_ordnance_007bbba0();                             // 009A67AB
        ap.drop_balance_2c -= 1;
        ++res.releases;
    }
    res.after = t.state;
    return res;
}

DepthChargeCruiseProfileWrite depth_charge_cruise_profile_009a6500(
    DepthChargeTask& t, DepthChargeTaskHost& host, const DepthChargeCruiseProfileInputs& in) {
    DepthChargeCruiseProfileWrite w;
    const DepthChargeTuning tu = host.tuning();
    // 009A6506 0099B660 is an empty body.
    w.leader = host.flight_leader_007b8ad0();                         // 009A6511
    if (w.leader) {
        if (!in.ctl_38d) {                                            // 009A652B
            if (0.0f > in.ctl_380 && !in.ctl_3a9) {                   // 009A6534, 009A653A
                w.write_394 = true;
                w.value_394 = tu.cruising_alt_4a4;                    // 009A6540
            }
            w.clear_3a9 = true;                                       // 009A654D
        }
        if (!in.ctl_38c) {                                            // 009A6562
            if (0.0f > in.ctl_37c && !in.ctl_3aa) {                   // 009A656B, 009A6570
                w.write_398 = true;
                w.value_398 = tu.aim_alt_range_2_498;                 // 009A6576
            }
            w.clear_3aa = true;                                       // 009A6583
        }
    }
    // 009A658C-009A65CE: +44h = max(+44h, float(AttackDist * +24h)).
    host.apply_cruise_profile_009a6500(w);
    const float x = static_cast<float>(static_cast<double>(tu.attack_dist_4ac) * t.ap.speed_ratio_24);
    t.ap.attack_range_44 = t.ap.attack_range_44 > x ? t.ap.attack_range_44 : x;
    depth_charge_approach_update_009a5f50(t.ap, host, 0.0f);          // 009A65D6
    return w;   // 009A65E1 JMP 0099B740, the host's
}

}  // namespace bsp
