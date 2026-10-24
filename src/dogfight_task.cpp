// The dogfight bot task, packet cc9_dogfight_task. See include/bsp/dogfight_task.hpp
// and docs/DOGFIGHT_TASK.md. Reconstructed and build-tested; the moveto rule is a
// labelled stand-in.
#include "bsp/dogfight_task.hpp"

#include <cmath>

#include "bsp/dive_bomb_task.hpp"  // dive_bomb_interpolate_clamped_00419010
#include "bsp/plane_flight.hpp"   // heading_command_009f9e40, pitch_command_to_point_009f9ed0

namespace bsp {

const char* dogfight_state_name(DogfightState s) noexcept {
    switch (s) {
    case DogfightState::kMoveTo: return "moveto";
    case DogfightState::kFollow: return "follow";
    case DogfightState::kPrepare: return "prepare";
    case DogfightState::kAim: return "aim";
    case DogfightState::kManeuver: return "maneuver";
    case DogfightState::kAttackRun: return "attackrun";
    case DogfightState::kAvoidRoll: return "avoid_roll";
    case DogfightState::kAvoidTurn: return "avoid_turn";
    default: return "none";
    }
}

bool dogfight_engaged_009aafa0(bool latch_4c8, int control_mode_370,
                               bool target_4c4) noexcept {
    return latch_4c8 || (control_mode_370 == 2 && target_4c4);
}

DogfightState dogfight_unengaged_state_009aafa0(bool is_flight_leader) noexcept {
    return is_flight_leader ? DogfightState::kMoveTo : DogfightState::kFollow;
}

DogfightMoveToCommand dogfight_moveto_standin(const DogfightMoveToInputs& in) noexcept {
    DogfightMoveToCommand out;
    const double dx = static_cast<double>(in.target_pos[0]) - in.own_pos[0];
    const double dz = static_cast<double>(in.target_pos[2]) - in.own_pos[2];
    out.horizontal_range = static_cast<float>(std::sqrt(dx * dx + dz * dz));
    out.heading = heading_command_009f9e40(in.target_pos[0], in.target_pos[2],
                                           in.own_pos[0], in.own_pos[2]);
    const float distance = (out.horizontal_range > in.min_distance)
        ? out.horizontal_range : in.min_distance;
    out.pitch = pitch_command_to_point_009f9ed0(in.cruising_alt - in.own_pos[1], distance,
                                                in.class_climb_angle_1e4);
    return out;
}

// ---------------------------------------------------------------------------
// The engaged half, packet cc9_dogfight_engaged. docs/DOGFIGHT_ENGAGED.md.
// ---------------------------------------------------------------------------

namespace {
float interp_00419010(float x0, float y0, float x1, float y1, float x) noexcept {
    return dive_bomb_interpolate_clamped_00419010(x0, y0, x1, y1, x);
}
}  // namespace

float dogfight_range_score_009aa630(float distance, float shoot_distance) noexcept {
    // 009AA7C7-009AA877. R = ShootDistance * (double)0.8f; DC C9 at 009AA7E4 is
    // FMUL ST(1),ST, so R is what 009AA7ED stores.
    const float r = static_cast<float>(shoot_distance * 0.800000011920929);
    if (r > distance) {
        return interp_00419010(100.0f, 0.5f, r * 0.8f, 1.0f, distance);
    }
    return interp_00419010(static_cast<float>(r * 1.2000000476837158), 1.0f,
                           static_cast<float>(r * 3.0), 0.25f, distance);
}

float dogfight_angle_score_009aa630(const float local[3]) noexcept {
    // 009AA884-009AA914.
    const float z = (1.0f > local[2]) ? 1.0f : local[2];
    const float tx = local[0] / z;
    const float ty = local[1] / z;
    const float t = static_cast<float>(std::sqrt(static_cast<double>(tx) * tx +
                                                 static_cast<double>(ty) * ty));
    return interp_00419010(0.25f, 1.0f, 1.5f, 0.2f, t);
}

float dogfight_target_score_009aa630(const DogfightCandidate& c, float shoot_distance,
                                     float noncurrent_draw) noexcept {
    const float behind = (c.local[2] < 0.0f) ? 0.1f : 1.0f;  // 009AA7E6, 00D7A2F0
    float s = dogfight_range_score_009aa630(c.distance, shoot_distance) * behind;
    s = dogfight_angle_score_009aa630(c.local) * s;
    for (int i = 0; i < c.wingmates_on_it; ++i) {
        s = static_cast<float>(s * 0.699999988079071);  // 009AA958, 00CEFFA0
    }
    if (!c.is_current_target) s *= noncurrent_draw;     // 009AA984
    return s;
}

bool dogfight_needs_reselect_009aac70(bool target_live, float timer_after_dt,
                                      float previous_distance,
                                      float shoot_distance) noexcept {
    if (!target_live) return true;
    // 009AACE9-009AAD0E: FCOMIP / JBE, then COMISS 0 > timer.
    const double limit = static_cast<double>(shoot_distance) + 200.0;
    return static_cast<double>(previous_distance) > limit && timer_after_dt < 0.0f;
}

DogfightLatch dogfight_latch_009aac70(bool has_target, float distance,
                                      float attack_dist, float ratio_24,
                                      bool was_latched,
                                      float min_muzzle_speed) noexcept {
    DogfightLatch out;
    if (!has_target) return out;
    const float hysteresis = was_latched ? 150.0f : 0.0f;  // 00CE3808
    if (!(attack_dist * ratio_24 + hysteresis > distance)) return out;  // 009AADCC
    out.latched = true;
    const double t = static_cast<double>(distance) / min_muzzle_speed;  // 009AADE3
    const float tf = static_cast<float>(t);
    if (0.0f > tf) {
        out.time_to_target = 0.0f;
    } else {
        out.time_to_target = (tf > 30.0) ? 30.0f : tf;  // 00CE7630 / 00CE38C8
    }
    return out;
}

void dogfight_off_axis_009aac70(const float local[3], float out_xy[2]) noexcept {
    const float z = (1.0f > local[2]) ? 1.0f : local[2];  // 009AAEC6 FLD1 / FCOMI
    out_xy[0] = local[0] / z;
    out_xy[1] = local[1] / z;
}

namespace {
DogfightState unengaged(bool leader) noexcept {
    return leader ? DogfightState::kMoveTo : DogfightState::kFollow;
}
DogfightTransition engage_entry_009a9d90(const DogfightTransitionInputs& in) noexcept {
    DogfightTransition t;
    if (in.squadron_mode_370 == 0) {
        t.next = DogfightState::kPrepare;
    } else if (in.latch_4c8) {
        t.next = DogfightState::kManeuver;
        t.reset_maneuver_6bc_6d8 = true;
    } else {
        t.next = DogfightState::kAttackRun;
    }
    return t;
}
}  // namespace

DogfightTransition dogfight_transition_009aafa0(const DogfightTransitionInputs& in) noexcept {
    DogfightTransition t;
    t.next = in.current;
    const bool eng = dogfight_engaged_009aafa0(in.latch_4c8, in.squadron_mode_370,
                                               in.target_squadron_4c4);
    if (in.current == DogfightState::kMoveTo || in.current == DogfightState::kFollow ||
        in.current == DogfightState::kNone) {
        // 009AB16D. kNone is this host's pre-install value; the image's
        // constructor leaves +310h on moveto or follow.
        if (eng) return engage_entry_009a9d90(in);
        t.next = unengaged(in.is_flight_leader);
        return t;
    }
    if (!eng) {  // 009AAFE8
        t.next = unengaged(in.is_flight_leader);
        return t;
    }
    if (in.squadron_mode_370 == 0) {  // 009AB00D
        t.next = DogfightState::kPrepare;
        return t;
    }
    if (in.current == DogfightState::kPrepare) return engage_entry_009a9d90(in);  // 009AB030
    if (in.current != DogfightState::kAim && in.aaa80) {  // 009AB047
        t.next = DogfightState::kAim;
        return t;
    }
    switch (in.current) {
    case DogfightState::kAttackRun:  // 009AB066
        if (in.latch_4c8) {
            t.next = DogfightState::kManeuver;
            t.reset_maneuver_6bc_6d8 = true;
        }
        break;
    case DogfightState::kAim:  // 009AB0A0
        if (in.aim_too_close_6a0) {
            t.next = in.avoid_pick_turn ? DogfightState::kAvoidTurn
                                        : DogfightState::kAvoidRoll;
        } else if (in.aim_bored_98f0) {
            t.next = DogfightState::kManeuver;
            t.maneuver_from_aim_8560 = true;
        }
        break;
    case DogfightState::kManeuver:  // 009AB0EC
        if (in.maneuver_on_target_9bd0) t.next = DogfightState::kAim;
        break;
    case DogfightState::kAvoidRoll:  // 009AB113
    case DogfightState::kAvoidTurn:  // 009AB142
        if (0.0f > in.avoid_timer) {
            t.next = DogfightState::kManeuver;
            t.maneuver_from_avoid_86f0 = true;
        }
        break;
    default:
        break;
    }
    return t;
}

bool dogfight_on_target_009a9bd0(float local_z, float tan_x, float tan_y) noexcept {
    if (!(local_z > 1.0f)) return false;
    const double sq = static_cast<double>(tan_y) * tan_y + static_cast<double>(tan_x) * tan_x;
    const float len = (sq <= 1e-10) ? 0.0f : static_cast<float>(std::sqrt(sq));
    return 0.800000011920929 > static_cast<double>(len);  // 009A9C38 FCOMIP / JBE
}

// 009A75C0-009A7641, __fastcall(ECX = aim state). +18h = 0; +1Ch = U(0.8
// [00CE74F8], 1.4 [00D06874]) * row+210h (009A75EE-009A75FF); +20h = U(0.5
// [00CE3800], 0.8) * row+20Ch * approach+24h (009A7624-009A763C); +24h = +25h
// = 0. Both draws are 00BD2F10 on stream 1. Each FMUL rounds to float at PC24,
// as the float products here do.
// DOCUMENTED DEVIATION (cc13-D2): the two draws are parameters. The units host
// makes them in this order, from stream-1 keys behind its own
// kDogfightEnterDrawsBound, and uses the midpoints 1.1 and 0.65 while that is
// off. It also passes approach+24h as 1.0. Both are the caller's matters.
DogfightAimState dogfight_aim_enter_009a75c0(const DogfightPilotRow& row, float ratio_24,
                                             float draw_boring, float draw_close) noexcept {
    DogfightAimState st;
    st.bored_limit_1c = draw_boring * row.boring_time;               // 009A75F3
    st.too_close_20 = draw_close * row.follow_dist * ratio_24;       // 009A7629/35
    return st;
}

DogfightAimCommand dogfight_aim_tick_009a76e0(DogfightAimState& st, const DogfightAimInputs& in,
                                              const DogfightPilotRow& row) noexcept {
    st.head_on_25 = in.local_z > 1.0f && in.opposing;
    if (st.head_on_25 && in.distance < st.too_close_20) st.too_close_24 = true;
    float rate = 0.0f;
    if (in.gun_locked_48) {
        rate = -4.0f;  // 00CF1430
    } else if (in.distance < row.aim_shoot_distance) {
        rate = interp_00419010(0.25f, -1.0f, 2.0f, 2.0f, in.tan_len);
    }
    st.bored_18 = rate * in.dt + st.bored_18;
    if (st.bored_18 < 0.0f) st.bored_18 = 0.0f;
    DogfightAimCommand c;
    if (!st.head_on_25) {
        c.speed_from_target = true;
        c.desired_speed = (in.distance - row.follow_dist) + in.target_speed;
    } else {
        c.head_on_fraction = interp_00419010(st.too_close_20, 0.3f, row.aim_shoot_distance,
                                             1.0f, in.distance);
    }
    return c;
}

// 009A8560-009A85A4, __fastcall(ECX = maneuver state). +18h = 0 (009A8567),
// then +34h = U(0.3 [00CE69C8], 1.1 [00CE6448]) * row+230h. The draw is
// stream 1 (009A8596), followed by one FMUL that is stored as float.
// DOCUMENTED DEVIATION (cc13-D2): the draw is a parameter, and the +18h store
// is the caller's. The units host draws behind kDogfightEnterDrawsBound and
// uses the midpoint 0.7 while that switch is off.
float dogfight_maneuver_pursuit_range_009a8560(float shoot_distance, float draw) noexcept {
    return draw * shoot_distance;
}

// 009A7DE0-009A7E76, __fastcall(ECX = avoid state). This models only the
// first store: +18h = U([00CEE07C], 1.5 [00CE380C]) * row+214h (009A7E09-
// 009A7E1A, stream 1, one FMUL stored as float). The rest of the enter is in
// the units host's avoid enter (df_avoid_enter, behind its
// kDogfightAvoidBodiesBound): +24h = 0, +20h = +1Ch = 0, then +1Ch = U(0.8,
// 1.0) when unit+C68h > 0 (COMISS/JBE), else U([00D05E14], -1.0).
// DOCUMENTED DEVIATION (cc13-D2): this is a fragment, and the draw is a
// parameter (made by the caller behind kDogfightEnterDrawsBound).
float dogfight_avoid_timer_009a7de0(float avoid_time, float draw) noexcept {
    return draw * avoid_time;
}

DogfightSteer dogfight_maneuver_standin(const float own_pos[3], const float aim[3],
                                        float horizontal_range, float shoot_distance,
                                        float ceiling_210, float class_climb_angle) noexcept {
    DogfightSteer s;
    s.heading = heading_command_009f9e40(aim[0], aim[2], own_pos[0], own_pos[2]);
    float a = aim[1] - own_pos[1];
    float d = horizontal_range;
    if (!(d <= static_cast<float>(shoot_distance * 3.0))) {
        a = ceiling_210 - (own_pos[1] + 50.0f);
        if (static_cast<double>(a) > 300.0) a = 300.0f;
        d = 500.0f;
    }
    if (d < 1.0f) d = 1.0f;
    s.pitch = pitch_command_to_point_009f9ed0(a, d, class_climb_angle);
    return s;
}

// ---------------------------------------------------------------------------
// The gun controller, packet cc9_dogfight_gun. docs/DOGFIGHT_GUN.md.
// ---------------------------------------------------------------------------

bool dogfight_gun_tick_009fc7c0(DogfightGunState& st, const DogfightGunInputs& in) noexcept {
    st.burst_timer_50 -= in.dt;                        // 009FC7C9
    if (st.hold_4c >= 0.0f) st.hold_4c -= in.dt;       // 009FC7D0
    st.fire_48 = false;
    bool fired = false;
    if (in.has_target) {
        const float x = in.lead_local[0], y = in.lead_local[1], z = in.lead_local[2];
        const float d = static_cast<float>(std::sqrt(static_cast<double>(x) * x +
                                                     static_cast<double>(y) * y +
                                                     static_cast<double>(z) * z));
        const float far_limit = in.search_range_30 > in.shoot_distance + 200.0f
            ? in.search_range_30 : in.shoot_distance + 200.0f;
        if (d > 1.0f && far_limit > d) {
            const float lateral = static_cast<float>(
                std::sqrt(static_cast<double>(x) * x + static_cast<double>(y) * y));
            bool envelope = false;
            if (st.hold_4c > 0.0f) {
                envelope = true;
            } else if (!(d * in.lateral_cap_38 <= lateral) && !(z <= 1.0f) &&
                       !(in.shoot_distance <= z)) {
                // With an auto target the don't-shoot radius is divided by 1.8.
                const float area = in.dont_shoot_3c / 1.8f;
                envelope = area < lateral;  // +0Ch is 0 from the ctor: the second arm is lateral > 0
            }
            if (envelope && !in.unit_disabled && !in.finder_busy &&
                (st.burst_4b || st.burst_timer_50 < 0.0f)) {
                st.fire_48 = true;
                fired = true;
                ++st.fire_ticks;
            }
        }
    }
    // 009FCE4F-009FCED9: the burst clock.
    if (!st.burst_4b) {
        if (st.burst_timer_50 < 0.0f && st.fire_48) {
            st.burst_4b = true;
            ++st.bursts;
            st.burst_timer_50 = in.burst_draw;
        }
    } else if (st.burst_timer_50 < 0.0f) {
        st.burst_4b = false;
        st.burst_timer_50 = in.delay_draw;
    }
    return fired;
}

// 007B4ED0-007B4F39, __thiscall(ECX = pilot plan, float f), RET 4, all SSE.
// brake = -0.0 [00D7A208] - f. The clamps (007B4EED-007B4F0E) are COMISS
// 0,v / JBE then COMISS v,1 / JBE. An unordered compare takes both JBEs, so a
// NaN passes through unchanged, as the `0 > v` / `v > 1` tests here do. The
// cc13-backfill note that the image yields 0 for a NaN was wrong (not a defect,
// cc13-D2). The stores +278h = throttle, +27Ch = 1, +2A8h = brake, +2ACh = 1
// and +2D8h = 0 are made by the units host from this pair, as host glue.
DogfightThrottle dogfight_throttle_007b4ed0(float f) noexcept {
    DogfightThrottle t;
    float brake = -0.0f - f;                           // 00D7A208 is -0.0f
    if (0.0f > brake) brake = 0.0f; else if (brake > 1.0f) brake = 1.0f;
    float thr = f;
    if (0.0f > thr) thr = 0.0f; else if (thr > 1.0f) thr = 1.0f;
    t.throttle = thr;
    t.air_brake = brake;
    return t;
}

// ---------------------------------------------------------------------------
// unit+C50h, packet cc9_plane_gunfire. docs/PLANE_GUNFIRE.md section 3.
// ---------------------------------------------------------------------------

PlaneNeighbourRadii plane_neighbour_radii_007e11d0(float refresh_period_88) noexcept {
    PlaneNeighbourRadii r;
    const float base = static_cast<float>((static_cast<double>(refresh_period_88) + 3.0) * 180.0);
    r.near_any = base;
    r.enemy_plane = (base <= 1200.0f) ? 1200.0f : base;
    r.friendly_plane = (static_cast<double>(base) <= 500.0) ? 500.0f : base;
    return r;
}

float plane_finder_score_007deec0(const PlaneFinderParams& p, const float local[3],
                                  float dy_above, float horizontal) noexcept {
    const float z = local[2];
    if (!(1.0f < z) || !(z < p.range_bc)) return 0.0f;
    const float tx = local[0] / z;
    const float ty = local[1] / z;
    const float tan2 = tx * tx + ty * ty;
    if (!(tan2 < p.cone_b8 * p.cone_b8)) return 0.0f;
    float inner = static_cast<float>(p.inner_c0 * 0.75);
    const float half = static_cast<float>(p.cone_b8 * 0.5);
    if (half < inner) inner = half;
    float ramp;
    if (z <= p.near_c4) {
        ramp = interp_00419010(1.0f, 0.0f, p.near_c4, 1.0f, z);
    } else {
        ramp = interp_00419010(p.range_bc, 0.4f, p.near_c4, 1.0f, z);
    }
    float s = interp_00419010(p.cone_b8 * p.cone_b8, 0.0f, inner * inner, 1.0f, tan2) * ramp;
    if (0.0f < dy_above) {
        const float h = (horizontal < 1.0f) ? 1.0f : horizontal;
        s = interp_00419010(0.3f, 1.0f, 2.0f, 0.2f, dy_above / h) * s;
    }
    return s;
}

// ---------------------------------------------------------------------------
// The dogfight moveto's speed slot, packet cc9_dogfight_moveto.
// ---------------------------------------------------------------------------

float dogfight_moveto_speed_009becd0(float max_spd, float level_flight_speed, float sep,
                                     float wait_dist_1, float wait_dist_2,
                                     bool has_squadron_and_class, float wingmen_007ef2c0) noexcept {
    if (!has_squadron_and_class) return max_spd;                  // 009BED72
    const float k = interp_00419010(wait_dist_1, 0.0f, wait_dist_2, 0.5f, sep);  // 009BED0F
    const float m = (k > wingmen_007ef2c0) ? k : wingmen_007ef2c0;  // 009BED35-009BED4C
    return (max_spd - level_flight_speed) * m + level_flight_speed;  // 009BED4C-009BED66
}

float follow_wait_value_009be3e0(bool in_position_85, const float member_minus_station[3],
                                 float leader_heading, float dont_wait_hdg, float wait_hdg,
                                 float good_position_dist, float nearby_dist) noexcept {
    // 009BE3E0-009BE58A, __fastcall(ECX = follow state) -> ST0. The +85h test
    // (009BE3E6) is here. The unit/squadron/leader null gate (009BE3F6-009BE413,
    // 1.0) and the operands belong to the caller: the leader heading (vslot +50h
    // at 009BE494, stored float) and d = unit +FCh..+104h - station +30h..+38h
    // (009BE44A-009BE478, float).
    // DEFECT ROW (cc13-D2): the OFF arm differs from the image's helpers in the
    // ways listed at kFollowWaitFloatHelpersCc13Bound.
    if (in_position_85) return 1.0f;
    const float x = member_minus_station[0];
    const float z = member_minus_station[2];
    float result;
    if constexpr (kFollowWaitFloatHelpersCc13Bound) {
        constexpr double kHalfPiF = 1.5707963705062866;    // [00CE3830]
        constexpr double kTwoPiF = 6.2831854820251465;     // [00CE3828]
        constexpr double kPiF = 3.1415927410125732;        // [00CE3D28], -[00CE3D18]
        // 007B4E90: atan2 (00BF701A) stored float, pi/2 - a stored float, plus
        // 2pi stored float when 0 > h.
        const float a = static_cast<float>(std::atan2(static_cast<double>(z), static_cast<double>(x)));
        float hdg = static_cast<float>(kHalfPiF - a);
        if (0.0f > hdg) hdg = static_cast<float>(hdg + kTwoPiF);
        // 00438B10(hdg, leader): the difference stored float, then 2pi steps
        // stored float until it lies in (-pi, pi]. An unordered first compare
        // takes the upper arm, whose JBE returns the NaN unchanged.
        float d = hdg - leader_heading;                    // 00438B10-00438B18
        if (!(-kPiF < d) && !std::isnan(d)) {              // 00438B26 FCOMI, JC
            do { d = static_cast<float>(d + kTwoPiF); } while (!(-kPiF < d));
        } else if (d > kPiF) {                             // 00438B59 FCOMI, JBE
            do { d = static_cast<float>(d - kTwoPiF); } while (d > kPiF);
        }
        // 009BE4B8-009BE4D2: 0 < d keeps d, else -0.0 - d (SUBSS).
        const float ang = (d > 0.0f) ? d : -0.0f - d;
        // 009BE503 00419010(+2Ch, 0, +30h, 1, ang), stored float.
        float v = interp_00419010(dont_wait_hdg, 0.0f, wait_hdg, 1.0f, ang);
        // 009BE510 FLD1 / FCOMIP / JBE: only 1 > v takes the distance term.
        if (1.0f > v) {
            // 0042B2F0 on (dx, 0, dz): zz + (0*0 + xx), stored float; a sum
            // not above 1e-10 [00CE3820] is length 0, else FSQRT stored float.
            const float xx = x * x;
            const float zz = z * z;
            const float sum = zz + (0.0f + xx);
            const float h = (static_cast<double>(sum) > 1e-10) ? std::sqrt(sum) : 0.0f;
            // 009BE54D 00419010(+18h, 1, +34h, 0, h) then FADD v, stored float.
            const float s = interp_00419010(good_position_dist, 1.0f, nearby_dist, 0.0f, h) + v;
            // 00415510(&s, &1.0): 1.0 <= s or unordered gives 1.0, else s.
            v = (1.0f > s) ? s : 1.0f;
        }
        result = v;
    } else {
        // 007B4E90: pi/2 - atan2(z, x), wrapped into [0, 2pi).
        double hdg = 1.5707963267948966 - std::atan2(static_cast<double>(z), static_cast<double>(x));
        if (hdg < 0.0) hdg += 6.283185307179586;
        double d = hdg - leader_heading;
        while (d > 3.141592653589793) d -= 6.283185307179586;
        while (d < -3.141592653589793) d += 6.283185307179586;
        const float ang = static_cast<float>(d < 0.0 ? -d : d);
        float v = interp_00419010(dont_wait_hdg, 0.0f, wait_hdg, 1.0f, ang);
        if (v < 1.0f) {
            const float h = static_cast<float>(std::sqrt(static_cast<double>(x) * x +
                                                         static_cast<double>(z) * z));
            v = v + interp_00419010(good_position_dist, 1.0f, nearby_dist, 0.0f, h);
            if (v > 1.0f) v = 1.0f;
        }
        result = v;
    }
    return result;
}

float squadron_wingmen_value_007ef2c0(const float* member_values, int count,
                                      int formation_shape_3e4) noexcept {
    if (count <= 1 || formation_shape_3e4 == 0) return 1.0f;       // 007EF2CA, 007EF2D6
    float best = 1.0f;                                             // 007EF2DF
    for (int i = 1; i < count; ++i) {
        const float v = member_values[i];
        if (v < 0.0f) continue;                                    // COMISS / JC
        if (!(v > best)) best = v;                                 // FCOMIP / JA
    }
    return best;
}

}  // namespace bsp
