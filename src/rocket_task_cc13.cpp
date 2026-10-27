// The rocket bot task (kind 0Bh). Evidence in docs/CC13_ROCKET_TASK.md; every
// address in a comment is a listing address. x87 intermediates are carried in
// double and stored to float wherever the listing has FSTP dword, as the
// strafe twin's reconstruction does (src/strafe_task.cpp).

#include "bsp/rocket_task_cc13.hpp"

#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"

// kPc24PlaneTasksCc13Bound sites here (pc24w1-sweep3-7): rocket_flight_time_00809e10
// (00809E77-00809EF3). The chains of 007B3EA0, 007B78F0, 007B4980, 007B4F60, 007B5430
// and 007B6240 in this file are not live and stay as pc24_debt.

namespace bsp {
bool rocket_on_hit_007b6d00(RocketTask& t, std::uint32_t) noexcept { t.ap.elapsed_44 = 0.0f; return true; }
int rocket_slot_return9_007b6ce0() noexcept { return 9; }
double rocket_follow_wait_007b7270(const RocketTask& t, double (*follow_wait)(void*), void* follow) {
    return t.state == RocketState::kFollow ? follow_wait(follow) : -1.0;
}
}

#include <cmath>

#include "bsp/plane_flight.hpp"
#include "bsp/robot_config.hpp"
#include "bsp/strafe_task.hpp"
#include "bsp/unit_rudder.hpp"

namespace bsp {

const std::array<RocketDrawSite, static_cast<std::size_t>(RocketDraw::kCount)> kRocketDrawSites = {{
    {RocketDraw::kGoAwayDist34, 0x007B4240u, 0x007B41E0u, "34", "U(0.9, 1.05)", true},
    {RocketDraw::kFireDist38, 0x007B42CBu, 0x007B41E0u, "38", "U(0.9, 1.1)", true},
    {RocketDraw::kCountdownD0, 0x007B4332u, 0x007B41E0u, "d0", "U(0, 1)", true},
    {RocketDraw::kGlideAngle2C, 0x007B3F0Eu, 0x007B3EA0u, "2c", "U(0.8, 1.2)", true},
    {RocketDraw::kGlideDist40, 0x007B3F49u, 0x007B3EA0u, "40", "U(-10, 40)", true},
    {RocketDraw::kRunSpeed48, 0x007B4912u, 0x007B48E0u, "48", "U(0.8, 1.1)", true},
    {RocketDraw::kAttackRunCd1C, 0x007B4DEAu, 0x007B4D90u, "ar1c", "U(0, 1)", true},
    {RocketDraw::kAimTimer18, 0x007B5322u, 0x007B5300u, "aim18", "U(0.2, 1)", true},
    {RocketDraw::kAimRepeat18, 0x007B5725u, 0x007B5430u, "fire", "U(0.8, 1.2)", true},
    {RocketDraw::kGoAwayHdgTimer30, 0x007B57EEu, 0x007B5780u, "ga30", "U(0, 5)", true},
    {RocketDraw::kGoAwayPitchTimer3C, 0x007B5827u, 0x007B5780u, "ga3c", "U(0, 5)", true},
    {RocketDraw::kGoAwayCountdown44, 0x007B584Fu, 0x007B5780u, "ga44", "U(0, 1)", true},
    {RocketDraw::kGoAwayProb18, 0x007B5874u, 0x007B5780u, "ga18", "U(0.3, 0.9)", true},
    {RocketDraw::kGoAwayPitch20, 0x007B5AC0u, 0x007B59D0u, "ga20", "U(lo, hi)", true},
    {RocketDraw::kGoAwayBank1C, 0x007B5BABu, 0x007B5AF0u, "ga1c", "U(r - 1, r + 1)", true},
    {RocketDraw::kGoAwayStandoff4C, 0x007B5FC4u, 0x007B5F10u, "ga4c", "U(1.0, 1.1)", true},
    {RocketDraw::kGoAwayWindow, 0x007B6053u, 0x007B5F10u, "ga_win", "U(10, 16)", true},
    {RocketDraw::kGoAwayEnterR, 0x007B607Bu, 0x007B5F10u, "mv_r", "U(0, 1)", true},
    {RocketDraw::kGoAwayEnterDist, 0x007B60FDu, 0x007B5F10u, "mv_dist", "U(0.5, 0.7)", true},
    {RocketDraw::kGoAwayEnterSide, 0x007B6180u, 0x007B5F10u, "mv_side", "U(0, 1)", true},
    {RocketDraw::kGoAwayHdgRearm, 0x007B6296u, 0x007B6240u, "ga30", "U(2, 5)", true},
    {RocketDraw::kGoAwayPitchRearm, 0x007B62D8u, 0x007B6240u, "ga3c", "U(2, 5)", true},
    {RocketDraw::kGoAwayTickR, 0x007B6436u, 0x007B6240u, "mv_r", "U(0, 1)", true},
    {RocketDraw::kGoAwayTickDist, 0x007B6476u, 0x007B6240u, "mv_dist", "U(0.5, 0.7)", true},
    {RocketDraw::kGoAwayTickSide, 0x007B64F1u, 0x007B6240u, "mv_side", "U(0, 1)", true},
    {RocketDraw::kUnreachedTimerCtor, 0x007B3D8Du, 0x007B3D60u, "unreached", "U(0, hi)", false},
    {RocketDraw::kUnreachedRunSpeed, 0x007B48B2u, 0x007B4890u, "unreached", "U(0.8, 1.1)", false},
    {RocketDraw::kUnreachedTimerTick, 0x007B59B4u, 0x007B5980u, "unreached", "U(lo, hi)", false},
}};

namespace {

constexpr double kEpsilon = 1e-10;                    // 00CE3820, FLD qword
constexpr double kTwoPi = 6.2831854820251465;         // 00CE3828, FLD qword
constexpr double kPi = 3.1415927410125732;            // 00CE3D28; 00CE3D18 is -pi
constexpr double kAlignedLimit = 0.6108652551968893;  // 00D057D8, FLD qword (35 deg)
constexpr double kProbeGate = 0.05000000074505806;    // 00D7A270, FLD qword
constexpr double kProbeOffsetGate = 0.10000000149011612;   // 00D7A3A0, FLD qword
constexpr double kProbeHeadingGain = 0.8726646304130554;   // 00D057E0, FMUL qword (50 deg)
constexpr double kThirtyDegrees = 0.5235987901687622;      // 00CEC730, FMUL/FDIV qword
constexpr double kGoAwayBankScale = 0.3490658700466156;    // 00D05858, FMUL qword
constexpr double kGoAwayPitchHeadroom = 0.1745329350233078;// 00D05850, FADD qword
constexpr double kHalf = 0.5;                         // 00D7A280, FMUL qword
constexpr double kOneAndHalf = 1.5;                   // 00CE3D78, FMUL qword

float draw(RocketRandom& rng, RocketDraw id, float lo, float hi) {
    return rng.uniform_00bd2f10(kRocketDrawSites[static_cast<std::size_t>(id)].key, lo, hi);
}

float length_or_zero(double sum) noexcept {
    // The pattern every distance in the task uses: the sum is stored float,
    // compared against double [00CE3820], then sqrt and stored float.
    const float s = static_cast<float>(sum);
    if (!(static_cast<double>(s) > kEpsilon)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

void write_speed(RocketPlanCommand& c, float speed) noexcept {
    c.speed_written = true;    // plan+2B4h = speed, +2B0h = 0, +2D8h = 1
    c.speed_2b4 = speed;
}

void write_throttle(RocketPlanCommand& c, float throttle) noexcept {
    c.throttle_written = true;   // plan+278h, +27Ch = 1, +2A8h = 0, +2ACh = 1, +2D8h = 0
    c.throttle_278 = throttle;
    c.air_brake_2a8 = 0.0f;
}

bool rocket_attack_state(RocketState s) noexcept {
    // 007B7050-007B7090: aim, goaway, attackrun, prepare, gotowards.
    return s == RocketState::kAim || s == RocketState::kGoAway ||
           s == RocketState::kAttackRun || s == RocketState::kPrepare ||
           s == RocketState::kGoTowards;
}

bool formation_state(RocketState s) noexcept {
    return s == RocketState::kMoveTo || s == RocketState::kFollow || s == RocketState::kPrepare;
}

// 009FB700 (thiscall on approach+4): the climb or drop angle that closes
// `desired - unit y`, by Pilot/General/ClimbDist and DropDist.
float altitude_pitch_009fb700(float desired, const RocketGoAwayInputs& in) noexcept {
    const float e = desired - in.unit_position[1];
    if (0.0f < e) {
        float f = e / in.tuning.climb_dist_544;
        if (!(0.0f <= f)) f = 0.0f;
        else if (1.0f < f) f = 1.0f;        // [00D7A24C]
        return in.class_climb_1ec * f;
    }
    const float f = e / in.tuning.drop_dist_548;
    float g = -1.5f;                        // [00D1F97C]
    if (-1.5f <= f) g = 0.0f < f ? 0.0f : f;
    return in.class_drop_1f0 * g;
}

// The evasive push shared by the goaway enter (with the glide gate) and tick.
RocketManoeuvre goaway_evasive(const RocketTask& t, const RocketGoAwayInputs& in,
                               bool from_enter, RocketRandom& rng) {
    RocketManoeuvre m;
    for (int i = 0; i < 3; ++i) m.point[i] = t.ap.aim_point[i];
    const float r = draw(rng, from_enter ? RocketDraw::kGoAwayEnterR : RocketDraw::kGoAwayTickR,
                         0.0f, 1.0f);
    bool flik = t.ga.prob_18 > r;           // FCOMI, JBE to the tightturn arm
    if (flik && from_enter) {
        // 007B60B9-007B60DC: float(approach+40h - 50.0 [00CE3938]) > unit y.
        flik = static_cast<float>(static_cast<double>(t.ap.glide_dist_40) - 50.0) >
               in.unit_position[1];
    }
    if (flik) {
        const float u = draw(rng, from_enter ? RocketDraw::kGoAwayEnterDist
                                             : RocketDraw::kGoAwayTickDist,
                             0.5f, 0.7f);                       // [00CE3800], [00CE3E18]
        m.kind = 2;                                             // 007B5E20 + 009BC0A0
        m.distance = static_cast<float>(static_cast<double>(u) * t.ap.goaway_dist_34);
    } else {
        const float u = draw(rng, from_enter ? RocketDraw::kGoAwayEnterSide
                                             : RocketDraw::kGoAwayTickSide,
                             0.0f, 1.0f);
        m.kind = 1;                                             // 007B5C70/007B5D30 + 009BC030
        m.side = 0.5f > u;                                      // [00CE3800], JA
    }
    return m;
}

RocketFlightInputs flight_inputs(RocketTaskHost& host) {
    RocketFlightInputs f;
    f.unit_position = host.unit_position();
    f.unit_heading_50 = host.heading_50();
    f.class_climb_1e4 = host.class_climb_1e4();
    f.tuning = host.tuning();
    return f;
}

RocketGoAwayInputs goaway_inputs(const RocketTask& t, RocketTaskHost& host) {
    RocketGoAwayInputs g;
    g.unit_position = host.unit_position();
    g.unit_heading_50 = host.heading_50();
    g.bomber_kind = host.bomber_kind();
    g.target_present_70 = t.ap.ref.tracking;
    if (g.target_present_70) {
        g.target_is_unit_5 = host.target_is_unit_5();
        if (g.target_is_unit_5) g.target_extent = host.target_extent_007b5be0();
        g.target_y = host.target_y_100();
    }
    g.parity_f876b0 = host.parity_f876b0();
    g.class_climb_1ec = host.class_climb_1ec();
    g.class_drop_1f0 = host.class_drop_1f0();
    g.alt_loadout = host.alt_loadout_007b58d0();
    g.tuning = host.tuning();
    return g;
}

// 007B78F0 whole: 009FADA0, +44h, 007C1EB0, the aim point and the range test.
void approach_update(RocketTask& t, RocketTaskHost& host, float dt) {
    RocketApproach& ap = t.ap;
    std::array<float, 3> point{{0.0f, 0.0f, 0.0f}};
    const bool has_target = host.update_target_ref_009fada0(ap.ref, dt, point);   // 007B7902
    ap.ref.tracking = has_target;
    ap.has_rocket_d4 = host.aimed_rocket_007c1eb0(ap.rocket);                    // 007B7918
    ap.aim_point = rocket_aim_point_007b4390(point, ap.has_rocket_d4,            // 007B7941
                                             ap.rocket.ignition_delay_e0);
    RocketRangeInputs in;
    in.target_present_70 = has_target;
    in.target_disabled_5d = has_target && host.target_marked_5d();
    in.attack_dist_664 = host.tuning().attack_dist_664;
    in.alt_loadout = host.alt_loadout_007b58d0();
    rocket_approach_update_007b78f0(ap, dt, host.unit_position(), in);
}

// The new state's vt[4]. Leaving an attack state calls 007B3DC0 (a plain RET).
RocketPlanCommand enter_state(RocketTask& t, RocketState s, RocketTaskHost& host,
                              RocketRandom& rng) {
    RocketPlanCommand c;
    switch (s) {
        case RocketState::kGoTowards:
            rocket_gotowards_enter_007b48e0(t, host.ctl_alt_398(), host.class_travel_speed_18c(),
                                            host.robot_row(), rng);
            host.pick_target_ref_009fa260(t.ap.ref);   // 007B497A JMP 009FA260
            break;
        case RocketState::kAim:
            rocket_aim_enter_007b5300(t.aim, rng);
            break;
        case RocketState::kGoAway: {
            const RocketGoAwayResult r = rocket_goaway_enter_007b5f10(t, goaway_inputs(t, host),
                                                                      host, rng);
            c = r.plan;
            if (r.manoeuvre.kind != 0) host.push_manoeuvre_00999f50(r.manoeuvre);
            break;
        }
        case RocketState::kAttackRun:
            rocket_attackrun_enter_007b4f40(t.ar);
            break;
        default:
            host.enter_formation_state(s);
            break;
    }
    return c;
}

}  // namespace

const char* rocket_state_name(RocketState s) noexcept {
    switch (s) {
        case RocketState::kMoveTo: return "moveto";
        case RocketState::kFollow: return "follow";
        case RocketState::kPrepare: return "prepare";
        case RocketState::kGoTowards: return "gotowards";
        case RocketState::kAim: return "aim";
        case RocketState::kGoAway: return "goaway";
        case RocketState::kAttackRun: return "attackrun";
        default: return "none";
    }
}

RocketStrikeRow rocket_strike_row(const PilotBotParameters& row) noexcept {
    RocketStrikeRow s;
    s.too_close_104 = row.strike_too_close_104;
    s.go_away_distance_108 = row.strike_go_away_distance_108;
    s.attack_angle_10c = row.strike_attack_angle_10c;
    s.target_prec_110 = row.strike_target_prec_110;
    s.homing_angle_114 = row.strike_homing_angle_114;
    s.repeat_time_118 = row.strike_repeat_time_118;
    s.fire_angle_11c = row.strike_fire_angle_11c;
    s.fire_dist_120 = {row.strike_fire_dist_1_120, row.strike_fire_dist_2_124,
                       row.strike_fire_dist_3_128};
    s.section_chance_130 = row.strike_section_damage_chance_130;
    s.engine_room_134 = row.strike_engine_room_weight_134;
    s.magazine_138 = row.strike_magazine_weight_138;
    s.fueltank_13c = row.strike_fueltank_weight_13c;
    return s;
}

int rocket_target_size_007b3c00(const RocketTargetKinds& k) noexcept {
    if (!k.present || !k.ship_06) return 2;          // null or vt[5Ch](6) false
    if (k.carrier_09 || k.battleship_0d) return 0;   // vt[5Ch](9), (0Dh)
    if (k.cargo_0b || k.cruiser_0a) return 1;        // vt[5Ch](0Bh), (0Ah)
    return 2;
}

bool rocket_platform_qualifies_007c1eb0(const RocketPlatformView& p,
                                        bool air_ground_same_f874fd) noexcept {
    if (!p.weapon_platform_25 || !p.carries_rocket_33) return false;   // vt[5Ch](25h), vt[210h](33h, 0)
    if (p.ignition_delay_e0 <= 0.0f) return false;   // (+E0h <= 0) == param_2 (0)
    return !p.anti_air_e4 || air_ground_same_f874fd; // param_3 = 0: +E4h clear, or [00F874FD]
}

void rocket_glide_seed_007b3ea0(RocketApproach& ap, float ctl_alt_398,
                                float attack_angle_10c, RocketRandom& rng) {
    // 007B3EA9-007B3EB7: atan2(ctl+398h, +34h) stored float; 007B3ED9 FCOMIP,
    // JBE: the smaller of it and AttackAngle ([+14h]+100h = row+10Ch).
    const float a = static_cast<float>(std::atan2(static_cast<double>(ctl_alt_398),
                                                  static_cast<double>(ap.goaway_dist_34)));
    const float m = attack_angle_10c > a ? a : attack_angle_10c;
    const float u = draw(rng, RocketDraw::kGlideAngle2C, 0.8f, 1.2f);       // 007B3F0E
    ap.glide_angle_2c = static_cast<float>(static_cast<double>(u) * m);    // 007B3F20
    ap.glide_tan_30 = static_cast<float>(std::tan(static_cast<double>(ap.glide_angle_2c)));
    const float v = draw(rng, RocketDraw::kGlideDist40, -10.0f, 40.0f);    // 007B3F49
    // 007B3F4E-007B3F56: FLD +30h, FMUL +34h, FADDP, one float store.
    ap.glide_dist_40 = static_cast<float>(static_cast<double>(ap.glide_tan_30) * ap.goaway_dist_34 +
                                          static_cast<double>(v));
}

void rocket_goaway_construct_007b5780(RocketGoAwayState& ga, RocketRandom& rng) {
    ga = RocketGoAwayState{};
    ga.done_24 = false;
    ga.hdg_lo_28 = 2.0f;                                                       // [00CE3958]
    ga.hdg_hi_2c = 5.0f;                                                       // [00CE3850]
    ga.hdg_timer_30 = draw(rng, RocketDraw::kGoAwayHdgTimer30, 0.0f, 5.0f);   // 007B57EE
    ga.pitch_lo_34 = 2.0f;
    ga.pitch_hi_38 = 5.0f;
    ga.pitch_timer_3c = draw(rng, RocketDraw::kGoAwayPitchTimer3C, 0.0f, 5.0f);   // 007B5827
    ga.period_40 = 1.0f;                                                       // [00D7A24C]
    ga.countdown_44 = -draw(rng, RocketDraw::kGoAwayCountdown44, 0.0f, 1.0f); // 007B584F
    ga.prob_18 = draw(rng, RocketDraw::kGoAwayProb18, 0.3f, 0.9f);           // 007B5874
    ga.standoff_4c = rocket::kGoAwayStandoffSeed;     // 007B587C, the rocket's own store
}

void rocket_task_construct_007b6a40(RocketTask& t, RocketTaskHost& host, RocketRandom& rng) {
    t = RocketTask{};
    RocketApproach& ap = t.ap;
    const RocketTuning tu = host.tuning();
    const RocketStrikeRow row = host.robot_row();
    // 009F9CE0 (approach head): +24h = max(1.0, MaxSpd / Pilot/Strike/ReferenceSpeed).
    const float ratio = host.class_speed_188() / tu.reference_speed_668;
    ap.speed_ratio_24 = 1.0f < ratio ? ratio : 1.0f;
    const double k = ap.speed_ratio_24;
    // 007B4240-007B4296: min(U(0.9, 1.05) * GoAwayDistance, AttackDist * 0.8) * +24h.
    const float u34 = draw(rng, RocketDraw::kGoAwayDist34, 0.9f, 1.05f);
    const float ga = static_cast<float>(static_cast<double>(u34) * row.go_away_distance_108);
    const float cap = static_cast<float>(static_cast<double>(tu.attack_dist_664) *
                                         rocket::kAttackDistCap);
    const float m = ga > cap ? cap : ga;               // 007B426C FCOMPI, JBE
    ap.goaway_dist_34 = static_cast<float>(static_cast<double>(m) * k);
    // 007B4299-007B42ED: U(0.9, 1.1) * FireDist[007B3C00(target)] (spilled as a
    // double) * +24h.
    const int size = rocket_target_size_007b3c00(host.target_kinds());
    const float u38 = draw(rng, RocketDraw::kFireDist38, 0.9f, 1.1f);
    ap.fire_dist_38 = static_cast<float>(static_cast<double>(u38) *
                                         static_cast<double>(row.fire_dist_120[size]) * k);
    ap.too_close_3c = static_cast<float>(k * row.too_close_104);       // 007B42FF, no draw
    ap.elapsed_44 = 3600.0f;                           // 007B42F6, [00CFDEB0]
    ap.in_range_4c = false;                            // 007B42FB
    // 007B4302: 009FB200, the target ref, with its first timer U(0, 2.0).
    ap.ref = approach_target_ref_construct_009fb200(
        rng.uniform_00bd2f10(kRocketRefTimerKey, 0.0f, 2.0f));
    ap.period_cc = 1.0f;                               // 007B432A
    ap.countdown_d0 = -draw(rng, RocketDraw::kCountdownD0, 0.0f, 1.0f);   // 007B4332
    ap.has_rocket_d4 = false;                          // 007B4341
    rocket_glide_seed_007b3ea0(ap, host.ctl_alt_398(), row.attack_angle_10c, rng);
    // 007B68FE-007B694E: gotowards +18h = +1Ch = 1400, +20h = 0; aim +18h = 0.
    t.gt = RocketGoTowardsState{};
    t.aim = RocketAimState{};
    rocket_goaway_construct_007b5780(t.ga, rng);       // 007B695F
    // 007B4D90: +18h = 1.0, +1Ch = -U(0, 1), +20h = 0.
    t.ar.period_18 = 1.0f;
    t.ar.countdown_1c = -draw(rng, RocketDraw::kAttackRunCd1C, 0.0f, 1.0f);   // 007B4DEA
    t.ar.offset_20 = 0.0f;
    // 007B6AA0-007B6AC6: moveto for the flight leader, else follow, entered by
    // its vt[4]; then BSP_BotApproach_BindToTask (the host's).
    t.state = host.flight_leader_007b8ad0() ? RocketState::kMoveTo : RocketState::kFollow;
    host.enter_formation_state(t.state);
}

void rocket_approach_update_007b78f0(RocketApproach& ap, float dt,
                                     const std::array<float, 3>& unit_position,
                                     const RocketRangeInputs& in) noexcept {
    ap.elapsed_44 = static_cast<float>(static_cast<double>(ap.elapsed_44) + dt);   // 007B7915
    // 007B7943-007B7A0A: +50h the 3-D and +54h the horizontal distance.
    const float dx = ap.aim_point[0] - unit_position[0];
    const float dy = ap.aim_point[1] - unit_position[1];
    const float dz = ap.aim_point[2] - unit_position[2];
    const double x2 = static_cast<double>(dx) * dx;
    const double y2 = static_cast<double>(dy) * dy;
    const double z2 = static_cast<double>(dz) * dz;
    ap.dist3_50 = length_or_zero((y2 + x2) + z2);
    ap.horiz_54 = length_or_zero(z2 + (x2 + 0.0));
    // 007B7A0F-007B7A2D: the once-a-period range test.
    const float cd = ap.countdown_d0;
    if (dt < cd) {                                     // 007B7A21 FCOMI, JC
        ap.countdown_d0 = cd - dt;                     // 007B7A89
        return;
    }
    ap.countdown_d0 = static_cast<float>((static_cast<double>(ap.period_cc) - dt) + cd);
    if (!in.target_present_70 || in.target_disabled_5d) {   // 007B7A33-007B7A42
        ap.in_range_4c = false;
        return;
    }
    if (ap.in_range_4c) {
        // 007B7A4E: 007B58D0 (ctl+369h && [00E17BF2]) keeps it in range;
        // otherwise it stays while AttackDist + 100.0 > +54h.
        if (in.alt_loadout) return;
        ap.in_range_4c = static_cast<double>(in.attack_dist_664) + rocket::kRangeHysteresis >
                         static_cast<double>(ap.horiz_54);
        return;
    }
    ap.in_range_4c = in.attack_dist_664 > ap.horiz_54;   // 007B7AB9 FCOMIP, JBE
}

RocketState rocket_engaged_entry_007b6fb0(int control_mode_370, bool in_range_444) noexcept {
    if (control_mode_370 == 0) return RocketState::kPrepare;               // 007B6FC9
    return in_range_444 ? RocketState::kGoTowards : RocketState::kAttackRun;  // 007B6FEF
}

RocketState rocket_rule_007b7050(const RocketRuleInputs& in) noexcept {
    // The engaged test, in both arms: +444h, or ctl+370h == 2 and +468h. The
    // strafe twin also accepts its kind-41h target (+44Ch); the rocket has none.
    const bool engaged = in.in_range_444 || (in.control_mode_370 == 2 && in.ref_target_468);
    if (rocket_attack_state(in.current)) {
        if (engaged) {
            if (in.control_mode_370 == 0) return RocketState::kPrepare;
            switch (in.current) {
                case RocketState::kPrepare:
                    return rocket_engaged_entry_007b6fb0(in.control_mode_370, in.in_range_444);
                case RocketState::kAttackRun:
                    return in.in_range_444 ? RocketState::kGoTowards : in.current;
                case RocketState::kGoAway:
                    return in.goaway_done_6b8 ? RocketState::kGoTowards : in.current;
                case RocketState::kGoTowards:
                    return in.gotowards_ready ? RocketState::kAim : in.current;
                case RocketState::kAim:
                    return (in.aim_off_691 || in.aim_no_rocket_690) ? RocketState::kGoAway
                                                                    : in.current;
                default:
                    return in.current;
            }
        }
    } else if (engaged) {
        return rocket_engaged_entry_007b6fb0(in.control_mode_370, in.in_range_444);
    }
    return in.flight_leader ? RocketState::kMoveTo : RocketState::kFollow;   // 007B71AF
}

bool rocket_gotowards_ready_007b6c00(const RocketGoTowardsState& gt, float field_188,
                                     float fire_dist_38) noexcept {
    // 007B6C00 and 009CC2F0 match instruction for instruction (local/rk1_twin.py).
    return strafe_gotowards_ready_009cc2f0(gt.aligned_20, gt.dist_18, field_188, fire_dist_38);
}

float rocket_aim_lift_00809de0(float ignition_delay_e0) noexcept {
    // 00809DE1-00809DFE: each product stored float; FMUL qword [00CF9058].
    const float t2 = ignition_delay_e0 * ignition_delay_e0;
    return static_cast<float>(static_cast<double>(t2) * rocket::kAimLiftGravity);
}

std::array<float, 3> rocket_aim_point_007b4390(const std::array<float, 3>& ref_point,
                                               bool has_rocket_d4,
                                               float ignition_delay_e0) noexcept {
    // 007B4396-007B43AF: 00809DE0 on [approach+D4h]+314h (the rocket's class)
    // when +D4h is set, else 0; then +74h + 0, lift + +78h, +7Ch + 0.
    const float lift = has_rocket_d4 ? rocket_aim_lift_00809de0(ignition_delay_e0) : 0.0f;
    return {ref_point[0] + 0.0f, lift + ref_point[1], ref_point[2] + 0.0f};
}

float rocket_flight_time_00809e10(float range, float launch_speed, float vmax_d8,
                                  float accel_dc) noexcept {
    const double v0 = launch_speed;
    const double vmax = vmax_d8;
    const double a = accel_dc;
    // 00809E5D FCOMI, JB: at or above VMax the rocket coasts.
    if (!(v0 < vmax)) return static_cast<float>(static_cast<double>(range) / v0);
    // 00809E75-00809EA1: the distance spent reaching VMax, stored float.
    float reach_a;
    float reach;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 00809E77-00809E9D: DC C9 FMUL 0.5 v0, D8 CB FMUL 0.5 * VMax, D8 CB FMUL
        // * VMax, DE FA FDIVP / a; then D8 CB FMUL v0 * 0.5 v0, D8 F2 FDIV / a,
        // DE E9 FSUBP reach_a - that. Each op PC24.
        const double half_v0 = cc13x87::x87_mul24(kHalf, v0);
        reach_a = static_cast<float>(cc13x87::x87_div24(
            cc13x87::x87_mul24(cc13x87::x87_mul24(kHalf, vmax), vmax), a));
        reach = static_cast<float>(cc13x87::x87_sub24(
            reach_a, cc13x87::x87_div24(cc13x87::x87_mul24(v0, half_v0), a)));
    } else {
        reach_a = static_cast<float>(kHalf * vmax * vmax / a);
        reach = static_cast<float>(static_cast<double>(reach_a) - (kHalf * v0) * v0 / a);
    }
    if (static_cast<double>(range) > reach) {
        // 00809EAD-00809EC3: (R - reach) / VMax + (VMax - v0) / a. The FSTP at
        // 00809EB7 float-rounds (VMax - v0) / a; the cruise quotient stays on the
        // x87 stack until the FADD at 00809EBF.
        float out;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // DE E4 FSUBRP VMax - v0, DE F1 FDIVRP / a (stored), DE E1 FSUBRP R - reach,
            // DE F1 FDIVRP / VMax, D8 44 FADD accel_time: each op PC24.
            const float accel_time = static_cast<float>(
                cc13x87::x87_div24(cc13x87::x87_sub24(vmax, v0), a));
            out = static_cast<float>(cc13x87::x87_add24(
                cc13x87::x87_div24(cc13x87::x87_sub24(range, reach), vmax), accel_time));
        } else {
            const float accel_time = static_cast<float>((vmax - v0) / a);
            out = static_cast<float>((static_cast<double>(range) - reach) / vmax + accel_time);
        }
        return out;
    }
    // 00809ED1-00809F10: sqrt(2 (R + 0.5 v0 (v0 / a)) / a) - v0 / a.
    const float q = static_cast<float>(v0 / a);
    const float p = static_cast<float>((kHalf * v0) * q);
    double s;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 00809EEB DE C2 FADDP R + p, DC C0 FADD ST0,ST0, DE F1 FDIVRP / a, each PC24.
        const double sum = cc13x87::x87_add24(range, p);
        s = static_cast<float>(cc13x87::x87_div24(cc13x87::x87_add24(sum, sum), a));
    } else {
        s = static_cast<float>(2.0 * (static_cast<double>(range) + p) / a);
    }
    const float root = static_cast<float>(std::sqrt(s));
    return static_cast<float>(static_cast<double>(root) - q);
}

float rocket_projtime_007b5390(float flight_time) noexcept {
    if (flight_time < 0.0f) return 0.0f;                                   // [00D7A218]
    if (30.0 < static_cast<double>(flight_time)) return rocket::kProjtimeCap;  // 00CE7630 / 00CE38C8
    return flight_time;
}

void rocket_gotowards_enter_007b48e0(RocketTask& t, float ctl_alt_398, float travel_speed_18c,
                                     const RocketStrikeRow& row, RocketRandom& rng) {
    t.gt.aligned_20 = false;                                                // 007B48E4
    rocket_glide_seed_007b3ea0(t.ap, ctl_alt_398, row.attack_angle_10c, rng);
    const float u = draw(rng, RocketDraw::kRunSpeed48, 0.8f, 1.1f);        // 007B4912
    t.ap.run_speed_48 = static_cast<float>(static_cast<double>(u) * travel_speed_18c);
    // 007B4920-007B496D: the target ref re-armed with the Strike row.
    ApproachTargetRefState& ref = t.ap.ref;
    ref.spread_48 = {row.target_prec_110, row.target_prec_110, row.target_prec_110};
    ref.dirty_41 = true;
    ref.section_chance_64 = row.section_chance_130;
    ref.weight_68 = row.engine_room_134;
    ref.weight_6c = row.magazine_138;
    ref.weight_70 = row.fueltank_13c;
}

RocketPlanCommand rocket_gotowards_tick_007b4980(RocketTask& t, const RocketFlightInputs& in,
                                                 RocketTaskHost& host) {
    RocketPlanCommand c;
    write_speed(c, t.ap.run_speed_48);                                      // 007B4996
    const std::array<float, 3>& p = t.ap.aim_point;
    const std::array<float, 3>& u = in.unit_position;
    const float dx = p[0] - u[0], dy = p[1] - u[1], dz = p[2] - u[2];
    const double x2 = static_cast<double>(dx) * dx;
    const double y2 = static_cast<double>(dy) * dy;
    const double z2 = static_cast<double>(dz) * dz;
    t.gt.dist_18 = length_or_zero((y2 + x2) + z2);                         // 007B4A59
    t.gt.horiz_1c = length_or_zero(z2 + x2);                               // 007B4A94
    // 007B4A90-007B4ACA: pi/2 - atan2(dz, dx), wrapped into [0, 2pi).
    const float h0 = heading_command_009f9e40(p[0], p[2], u[0], u[2]);
    // 007B4ACC-007B4BAD: 007F0280 over the box (150, 150, 500).
    float off = 0.0f;
    {
        const float ext[3] = {150.0f, 150.0f, 500.0f};   // [00CE3808] x2, [00CE397C]
        const NearFieldProbeResult pr = host.near_field_probe_007f0280(ext);
        if (static_cast<double>(std::fabs(pr.out_a[0])) > kProbeGate) {
            const float o = static_cast<float>(-static_cast<double>(pr.out_a[0]) *
                                               pr.out_b[1] * pr.out_b[2]);   // 007B4B67
            if (static_cast<double>(std::fabs(o)) > kProbeOffsetGate) {
                off = static_cast<float>(static_cast<double>(o) * kProbeHeadingGain);
            }
        }
    }
    // 007B4BB1-007B4C0C: fmod by 2pi, then into (-pi, pi].
    const float sum = h0 + off;
    float h = static_cast<float>(std::fmod(static_cast<double>(sum), kTwoPi));
    if (!(-kPi < static_cast<double>(h))) {
        h = static_cast<float>(static_cast<double>(h) + kTwoPi);
    } else if (static_cast<double>(h) > kPi) {
        h = static_cast<float>(static_cast<double>(h) - kTwoPi);
    }
    c.heading_written = true;                                               // 007B4C19
    c.heading_2c0 = h;
    // 007B4C2D-007B4CA4: aligned within 35 degrees (double [00D057D8]).
    float e = wrapped_angle_subtract_00438b10(h, in.unit_heading_50);
    if (!(e > 0.0f)) e = -0.0f - e;                                         // [00D7A208]
    t.gt.aligned_20 = static_cast<double>(e) < kAlignedLimit;
    // 007B4CA0-007B4D29: the glide-limited height above the point.
    float yp = -dy;
    const float g = t.gt.horiz_1c * t.ap.glide_tan_30;
    if (!(g < yp)) {
        const float tt = static_cast<float>((static_cast<double>(g) - yp) * kHalf);
        const float cc = static_cast<float>(kOneAndHalf * yp);
        yp = tt > cc ? yp - cc : yp - tt;
    }
    // 007B4D3D: 009F9ED0(-0.0 - that, +1Ch); plan+2BCh, mode 2.
    c.pitch_written = true;
    c.pitch_2bc = pitch_command_to_point_009f9ed0(-0.0f - yp, t.gt.horiz_1c, in.class_climb_1e4);
    c.pitch_mode_2d0 = 2;
    c.cone_written = true;                                                  // 007B4D52
    c.cone_40 = in.tuning.cone_678;
    // 007B4D72: 009FABE0(h, 0099B630(plan)), the plan's own target pitch.
    c.gun_direction_written = true;
    c.gun_heading_from_plan = true;
    c.gun_pitch = c.pitch_2bc;
    return c;
}

void rocket_attackrun_enter_007b4f40(RocketAttackRunState& ar) noexcept {
    ar.offset_20 = 0.0f;   // 007B4F43
    ar.period_18 = 0.4f;   // 007B4F50, [00CE7804]
}

RocketPlanCommand rocket_attackrun_tick_007b4f60(RocketTask& t, const RocketFlightInputs& in,
                                                 float dt, RocketTaskHost& host) {
    RocketPlanCommand c;
    const std::array<float, 3>& p = t.ap.aim_point;
    const std::array<float, 3>& u = in.unit_position;
    const float h0 = heading_command_009f9e40(p[0], p[2], u[0], u[2]);   // 007B4F90-007B4FDE
    const float cd = t.ar.countdown_1c;
    if (dt < cd) {                                                          // 007B4FF2 FCOMI, JB
        t.ar.countdown_1c = cd - dt;                                        // 007B5104
    } else {
        t.ar.countdown_1c = static_cast<float>((static_cast<double>(t.ar.period_18) - dt) + cd);
        // 007B4FFD-007B507E: 007F0280 over (100, 60, 120); +20h =
        // -out_a.x * out_b.y * out_b.z (stored float) * pi/6.
        const float ext[3] = {100.0f, 60.0f, 120.0f};   // [00CE3D08], [00CEB4B0], [00D05804]
        const NearFieldProbeResult pr = host.near_field_probe_007f0280(ext);
        const float o = static_cast<float>(-static_cast<double>(pr.out_a[0]) *
                                           pr.out_b[1] * pr.out_b[2]);      // 007B5070
        t.ar.offset_20 = static_cast<float>(static_cast<double>(o) * kThirtyDegrees);
    }
    c.heading_written = true;                                               // 007B5092-007B50AA
    c.heading_2c0 = wrapped_angle_add_00438aa0(h0, t.ar.offset_20);
    // 007B50B6: the horizontal distance is approach+54h (strafe recomputes 009CAD00).
    const float dist = t.ap.horiz_54;
    // 007B50DD-007B512C: (ceiling * 0.9 - unit y) clamped to [0, 400].
    float hgt = static_cast<float>(static_cast<double>(in.tuning.ceiling_210) *
                                   0.8999999761581421 - u[1]);              // [00D7A390]
    if (0.0f > hgt) hgt = 0.0f;
    else if (static_cast<double>(hgt) > 400.0) hgt = 400.0f;                // [00CE3D90] / [00CFD710]
    const float ad = in.tuning.attack_dist_664;                             // 007B5137, +664h
    const float den = ad > dist ? dist : ad;
    const float ratio = hgt / den;
    const float factor = clamped_interpolate_00419010(0.1f, 0.3f, 0.5f, 1.0f, ratio);   // 007B51A4
    c.cruise_written = true;                                                // 007B51C7, 009FBA50
    c.cruise_base = t.ap.glide_dist_40;
    c.cruise_low = t.ap.fire_dist_38;
    c.cruise_high = dist;
    c.cruise_scale = factor;
    write_throttle(c, 0.98f);                                               // [00CE6650]
    c.cone_written = true;                                                  // 007B5214, +670h
    c.cone_40 = in.tuning.cone_670;
    // 007B5237: 009FABE0(009A1A20(plan), 0099B630(plan)); the heading mode is 2.
    c.gun_direction_written = true;
    c.gun_heading_from_plan = true;
    return c;
}

void rocket_aim_enter_007b5300(RocketAimState& aim, RocketRandom& rng) {
    aim.off_1d = false;                                                     // 007B5308
    aim.no_rocket_1c = false;
    aim.timer_18 = draw(rng, RocketDraw::kAimTimer18, 0.2f, 1.0f);         // 007B5322, [00CE54A0]
}

RocketAimResult rocket_aim_tick_007b5430(RocketTask& t, const RocketAimInputs& in, float dt,
                                         RocketRandom& rng) {
    RocketAimResult r;
    RocketPlanCommand& c = r.plan;
    write_speed(c, t.ap.run_speed_48);                                      // 007B5441
    const std::array<float, 3>& p = t.ap.aim_point;
    const std::array<float, 3>& u = in.unit_position;
    // 007B5471: 009F9E40, the heading to the point, mode 2.
    c.heading_written = true;
    c.heading_2c0 = heading_command_009f9e40(p[0], p[2], u[0], u[2]);
    // 007B5498-007B5532: atan2(dy, horizontal), plan+2BCh, mode 1.
    const float dx = p[0] - u[0], dy = p[1] - u[1], dz = p[2] - u[2];
    const double x2 = static_cast<double>(dx) * dx;
    const float hd = length_or_zero(static_cast<double>(dz) * dz + (x2 + 0.0));
    const float pitch = static_cast<float>(std::atan2(static_cast<double>(dy),
                                                      static_cast<double>(hd)));
    c.pitch_written = true;
    c.pitch_2bc = pitch;
    c.pitch_mode_2d0 = 1;
    // 007B5586-007B5622: f = max(z, 0.75 |local|) (FMUL qword [00CEC9D8]).
    const std::array<float, 3>& l = in.local_point;
    const double lx2 = static_cast<double>(l[0]) * l[0];
    const double ly2 = static_cast<double>(l[1]) * l[1];
    const double lz2 = static_cast<double>(l[2]) * l[2];
    const float len = length_or_zero((ly2 + lx2) + lz2);
    const float q = static_cast<float>(static_cast<double>(len) * rocket::kForwardRangeScale);
    const float f = q > l[2] ? q : l[2];
    r.forward_range = f;
    // 007B5632: 007B5390, the lead time from the rocket's flight time.
    if (t.ap.has_rocket_d4) {
        const RocketAimedRocket& k = t.ap.rocket;
        t.ap.ref.projtime_44 = rocket_projtime_007b5390(
            rocket_flight_time_00809e10(f, k.launch_speed, k.vmax_d8, k.accel_dc));
        r.projtime_written = true;
    }
    // 007B5639-007B5666: no rocket; behind or inside the too-close distance.
    t.aim.no_rocket_1c = !t.ap.has_rocket_d4;
    t.aim.off_1d = 0.0f > l[2] || t.ap.too_close_3c > f;
    // 007B5663-007B5675: the release timer.
    t.aim.timer_18 = t.aim.timer_18 - dt;
    if (0.0f > t.aim.timer_18 && t.ap.fire_dist_38 > f && !t.aim.no_rocket_1c &&
        !t.aim.off_1d) {
        // 007B56A5-007B56C5: |(x, y, 0)| (0042B2F0) / f against FireAngle.
        const float lat = length_or_zero(ly2 + lx2);
        const double ratio = static_cast<double>(lat) / f;
        if (static_cast<double>(in.row.fire_angle_11c) > ratio) {
            // 007B56C7-007B5700: 0080AA20 on the rocket's homing block, then
            // 007BBBA0 on the unit; the timer re-arms to U(0.8, 1.2) * RepeatTime.
            r.release.fire = true;
            for (int i = 0; i < 3; ++i) r.release.homing_point[i] = t.ap.ref.body_offset_28[i];
            r.release.homing_angle = in.row.homing_angle_114;
            const float w = draw(rng, RocketDraw::kAimRepeat18, 0.8f, 1.2f);   // 007B5725
            t.aim.timer_18 = static_cast<float>(static_cast<double>(w) * in.row.repeat_time_118);
        }
    }
    c.cone_written = true;                                                  // 007B573F: 0
    c.cone_40 = 0.0f;
    // 007B575B: 009FABE0(009A1A20(plan), pitch); the heading mode is 2.
    c.gun_direction_written = true;
    c.gun_heading_from_plan = true;
    c.gun_pitch = pitch;
    return r;
}

void rocket_goaway_pitch_007b59d0(RocketGoAwayState& ga, const RocketApproach& ap,
                                  const RocketGoAwayInputs& in, RocketRandom& rng) {
    // 007B59DA: approach+70h only (the strafe twin also tries +54h).
    if (in.target_present_70 && !(30.0 > static_cast<double>(in.unit_position[1]))) {
        const float desired = in.target_y + ap.glide_dist_40;              // 007B5A3C
        const float pv = altitude_pitch_009fb700(desired, in);             // 007B5A47
        const float lo = pv > 0.0f ? 0.0f : pv;
        const float ph = static_cast<float>(static_cast<double>(pv) + kGoAwayPitchHeadroom);
        const float hi = in.class_climb_1ec > ph ? ph : in.class_climb_1ec;
        ga.pitch_20 = draw(rng, RocketDraw::kGoAwayPitch20, lo, hi);       // 007B5AC0
        return;
    }
    ga.pitch_20 = in.class_climb_1ec;                                       // 007B5AD4
}

void rocket_goaway_bank_007b5af0(RocketGoAwayState& ga, const RocketApproach& ap,
                                 const RocketGoAwayInputs& in, RocketTaskHost& host,
                                 RocketRandom& rng) {
    // 007B5AF6-007B5B2D: 009FD570(point, unit, +4Ch, &+48h, approach+54h, 1.0).
    const float b = host.fly_to_heading_009fd570(ap.aim_point, ga.standoff_4c, ga.side_48,
                                                 ap.horiz_54);
    const float e = wrapped_angle_subtract_00438b10(b, in.unit_heading_50);
    float r = static_cast<float>(static_cast<double>(e) / kThirtyDegrees);  // [00CEC730]
    if (-1.0f > r) r = -1.0f;
    else if (r > 1.0f) r = 1.0f;
    const float w = draw(rng, RocketDraw::kGoAwayBank1C, r - 1.0f, r + 1.0f);   // 007B5BAB
    ga.bank_1c = static_cast<float>(static_cast<double>(w) * kGoAwayBankScale * -1.0);
}

RocketGoAwayResult rocket_goaway_enter_007b5f10(RocketTask& t, const RocketGoAwayInputs& in,
                                                RocketTaskHost& host, RocketRandom& rng) {
    RocketGoAwayResult res;
    RocketGoAwayState& ga = t.ga;
    // 007B5F25-007B5F51: +48h = -1.0 when [00F876B0] is odd (signed), else 1.0.
    ga.side_48 = (in.parity_f876b0 % 2) != 0 ? -1.0f : 1.0f;
    // 007B5F56-007B5FCD: +4Ch = U(1.0, 1.1) * max(AttackDist * 0.8, extent).
    float cap = static_cast<float>(static_cast<double>(in.tuning.attack_dist_664) *
                                   rocket::kAttackDistCap);
    if (in.target_present_70 && in.target_is_unit_5) {
        if (!(cap > in.target_extent)) cap = in.target_extent;             // 007B5F9B FCOMPI, JA
    }
    const float u = draw(rng, RocketDraw::kGoAwayStandoff4C, 1.0f, 1.1f);  // 007B5FC4
    ga.standoff_4c = static_cast<float>(static_cast<double>(u) * cap);
    res.plan.cone_written = true;                                           // 007B5FE3, +674h
    res.plan.cone_40 = in.tuning.cone_674;
    rocket_goaway_bank_007b5af0(ga, t.ap, in, host, rng);                   // 007B5FEA
    rocket_goaway_pitch_007b59d0(ga, t.ap, in, rng);                        // 007B5FF1
    ga.done_24 = false;                                                     // 007B5FF9
    // 007B6000-007B6062: a plane of neither kind 10h nor 16h hit within
    // U(10, 16) s (approach+44h, spilled as a double) evades.
    if (!in.bomber_kind) {
        const float w = draw(rng, RocketDraw::kGoAwayWindow, 10.0f, 16.0f);   // 007B6053
        if (static_cast<double>(w) > static_cast<double>(t.ap.elapsed_44)) {
            res.manoeuvre = goaway_evasive(t, in, true, rng);
        }
    }
    return res;
}

RocketGoAwayResult rocket_goaway_tick_007b6240(RocketTask& t, const RocketGoAwayInputs& in,
                                               float dt, RocketTaskHost& host,
                                               RocketRandom& rng) {
    RocketGoAwayResult res;
    RocketGoAwayState& ga = t.ga;
    RocketPlanCommand& c = res.plan;
    ga.hdg_timer_30 = ga.hdg_timer_30 - dt;                                 // 007B6277
    if (0.0f > ga.hdg_timer_30) {
        const float w = draw(rng, RocketDraw::kGoAwayHdgRearm, ga.hdg_lo_28, ga.hdg_hi_2c);
        ga.hdg_timer_30 = w + ga.hdg_timer_30;                              // 007B62A0
        rocket_goaway_bank_007b5af0(ga, t.ap, in, host, rng);
    }
    ga.pitch_timer_3c = ga.pitch_timer_3c - dt;                             // 007B62B9
    if (0.0f > ga.pitch_timer_3c) {
        const float w = draw(rng, RocketDraw::kGoAwayPitchRearm, ga.pitch_lo_34, ga.pitch_hi_38);
        ga.pitch_timer_3c = w + ga.pitch_timer_3c;                          // 007B62E2
        rocket_goaway_pitch_007b59d0(ga, t.ap, in, rng);
    }
    // 007B62EE-007B6357: bank mode 1, pitch mode 1, full throttle, no brake.
    c.bank_written = true;
    c.bank_2c4 = ga.bank_1c;
    c.pitch_written = true;
    c.pitch_2bc = ga.pitch_20;
    c.pitch_mode_2d0 = 1;
    write_throttle(c, 1.0f);
    // 007B6357-007B63DE: the once-a-period completion test.
    const float cd = ga.countdown_44;
    if (dt < cd) {
        ga.countdown_44 = cd - dt;                                          // 007B63D5
    } else {
        ga.countdown_44 = static_cast<float>((static_cast<double>(ga.period_40) - dt) + cd);
        ga.done_24 = false;                                                 // 007B6373
        // 007B637A-007B6394: in the alternate loadout with no rocket left the
        // goaway never completes; in it with one, the distance is 0.4 times.
        if (!(in.alt_loadout && !t.ap.has_rocket_d4)) {
            float thr = t.ap.goaway_dist_34;
            if (in.alt_loadout) {
                thr = static_cast<float>(static_cast<double>(thr) * rocket::kAltLoadoutGoAwayScale);
            }
            ga.done_24 = t.ap.horiz_54 > thr;                               // 007B63CB FCOMPI
        }
    }
    // 007B63E1-007B6420: the evasive push within 1 s of a hit.
    if (!in.bomber_kind && 1.0f > t.ap.elapsed_44) {
        res.manoeuvre = goaway_evasive(t, in, false, rng);
        t.ap.elapsed_44 = 25.0f;                                            // 007B6571, [00CE89CC]
        res.elapsed_set_25 = true;
    }
    c.cone_written = true;                                                  // 007B6593, +674h
    c.cone_40 = in.tuning.cone_674;
    // 007B65B1: 009FABE0(009A1A20(plan), +20h); the heading mode is 1 here, so
    // 009A1A20 derives the heading from the unit's roll (host).
    c.gun_direction_written = true;
    c.gun_heading_from_plan = false;
    c.gun_pitch = ga.pitch_20;
    return res;
}

RocketArmResult rocket_task_arm_007b7c40(RocketTask& t, RocketTaskHost& host,
                                         RocketRandom& rng, float dt) {
    RocketArmResult res;
    res.before = t.state;
    approach_update(t, host, dt);                                           // 007B7C52
    res.has_target = t.ap.ref.tracking;
    host.set_moveto_ranges_009bde80(t.ap.glide_dist_40, t.ap.glide_dist_40,   // 007B7C7F
                                    t.ap.fire_dist_38);
    // 007B7C8E: the rule; a change calls the old state's vt[8] and the new one's vt[4].
    RocketRuleInputs ri;
    ri.current = t.state;
    ri.in_range_444 = t.ap.in_range_4c;
    ri.control_mode_370 = host.attack_mode_370();
    ri.ref_target_468 = t.ap.ref.tracking;
    ri.gotowards_ready = t.state == RocketState::kGoTowards &&
        rocket_gotowards_ready_007b6c00(t.gt, host.class_speed_188(), t.ap.fire_dist_38);
    ri.goaway_done_6b8 = t.ga.done_24;
    ri.aim_no_rocket_690 = t.aim.no_rocket_1c;
    ri.aim_off_691 = t.aim.off_1d;
    ri.flight_leader = host.flight_leader_007b8ad0();
    const RocketState next = rocket_rule_007b7050(ri);
    if (next != t.state) {
        if (formation_state(t.state)) host.exit_formation_state(t.state);   // else 007B3DC0, RET
        t.state = next;
        const RocketPlanCommand ec = enter_state(t, next, host, rng);
        if (ec.cone_written) host.apply_plan(ec);
    }
    // The current state's vt[0Ch](dt).
    switch (t.state) {
        case RocketState::kGoTowards:
            res.plan = rocket_gotowards_tick_007b4980(t, flight_inputs(host), host);
            host.apply_plan(res.plan);
            break;
        case RocketState::kAttackRun:
            res.plan = rocket_attackrun_tick_007b4f60(t, flight_inputs(host), dt, host);
            host.apply_plan(res.plan);
            break;
        case RocketState::kAim: {
            RocketAimInputs ai;
            ai.unit_position = host.unit_position();
            ai.local_point = host.world_to_body(t.ap.aim_point);
            ai.row = host.robot_row();
            const RocketAimResult r = rocket_aim_tick_007b5430(t, ai, dt, rng);
            res.plan = r.plan;
            host.apply_plan(res.plan);
            if (r.release.fire) {
                host.release_rocket(r.release);
                res.released = true;
            }
            break;
        }
        case RocketState::kGoAway: {
            const RocketGoAwayResult r = rocket_goaway_tick_007b6240(t, goaway_inputs(t, host),
                                                                     dt, host, rng);
            res.plan = r.plan;
            host.apply_plan(res.plan);
            if (r.manoeuvre.kind != 0) host.push_manoeuvre_00999f50(r.manoeuvre);
            break;
        }
        case RocketState::kNone:
            break;
        default:
            host.tick_formation_state(t.state, dt);
            break;
    }
    t.task_2e4 = 0xFF;                                                      // 007B7CA8
    res.after = t.state;
    return res;
}

RocketCruiseProfileWrite rocket_task_cruise_profile_007b7af0(RocketTask& t, RocketTaskHost& host,
                                                             RocketRandom& rng,
                                                             const RocketCruiseProfileInputs& in) {
    RocketCruiseProfileWrite w;
    const RocketTuning tu = host.tuning();
    w.cruising_alt_660 = tu.cruising_alt_660;
    if (host.flight_leader_007b8ad0()) {                                    // 007B7AFE, JZ 007B7B7F
        w.write_394 = !in.ctl_38d && 0.0f > in.ctl_380 && !in.ctl_3a9;      // 007B7B18-007B7B3A
        w.clear_3a9 = !in.ctl_38d;
        w.write_398 = !in.ctl_38c && 0.0f > in.ctl_37c && !in.ctl_3aa;      // 007B7B3E-007B7B70
        w.clear_3aa = !in.ctl_38c;
        // 007B7B7A: the glide seed reads ctl+398h after the write above.
        const float alt_398 = w.write_398 ? tu.cruising_alt_660 : host.ctl_alt_398();
        rocket_glide_seed_007b3ea0(t.ap, alt_398, host.robot_row().attack_angle_10c, rng);
    }
    approach_update(t, host, 0.0f);                                         // 007B7B8B, dt 0
    return w;                                                               // then JMP 0099B740
}

int rocket_step_result_007b6f90(RocketState s) noexcept {
    return s == RocketState::kAim ? 2 : 0;   // (+310h != +674h) - 1 & 2
}

int rocket_still_valid_007b7200(const RocketStillValidInputs& in) noexcept {
    if (!in.has_control_block) return 2;                                    // 007B720B
    if (!in.ctl_vt114) return 0;
    if (in.director_command != rocket::kCommandClass &&
        in.director_command != rocket::kAttackAnyClass) {
        return 0;
    }
    if (!in.ref_target_468 || in.command_target_matches) return 1;
    return 0;
}

}  // namespace bsp
