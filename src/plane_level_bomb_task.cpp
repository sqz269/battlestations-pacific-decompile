#include "bsp/plane_level_bomb_task.hpp"

#include <cmath>

#include "bsp/dive_bomb_task.hpp"
#include "bsp/plane_flight.hpp"
#include "bsp/unit_rudder.hpp"

// docs/SQUADRON_LAND_TASK.md section 5ee. Names are hypotheses; every address
// below is the instruction the line reproduces.

namespace bsp {
namespace {

float abs_wrapped(float v) noexcept {
    // The listing's |x|: `x > 0 ? x : -0.0 - x` (009B5E4B-009B5E65 and twins).
    return v > 0.0f ? v : -0.0f - v;
}

float planar(float dx, float dz) noexcept {
    // The 1e-10 cutoff every distance here takes (double [00CE3820]).
    const float d2 = dx * dx + dz * dz;
    return static_cast<double>(d2) > 1e-10 ? std::sqrt(d2) : 0.0f;
}

}  // namespace

const char* level_bomb_state_name(LevelBombState s) noexcept {
    switch (s) {
        case LevelBombState::kMoveTo: return "moveto (LevelBomb)";
        case LevelBombState::kFollow: return "follow (LevelBomb)";
        case LevelBombState::kAttackRun: return "LevelBomb/attackrun";
        case LevelBombState::kAim: return "LevelBomb/aim";
        case LevelBombState::kPrepare: return "LevelBomb/prepare";
        case LevelBombState::kRelease: return "LevelBomb/release";
        case LevelBombState::kGoAway: return "LevelBomb/goaway";
        default: return "none";
    }
}

void level_bomb_approach_update_009b7c90(LevelBombApproach& a,
                                         const LevelBombApproachInputs& in) noexcept {
    a.close_cc = false;                                   // 009B7CAC
    a.drop_alt_b0 = in.control_drop_alt_398;              // 009B7CB3
    // 009B7CBF-009B7CF7.
    a.has_ordnance_ce = in.release_pending_c25 || in.issue_requests_c20 > 0 ||
                        in.has_general_bomb || in.has_paratroopers;
    if (!in.has_target) {                                 // 009B7CF3, 009B80E9
        a.in_range_cd = false;
        return;
    }
    const float dx = in.aim[0] - in.unit_position[0];     // 009B7D24-009B7D4A
    const float dy = in.aim[1] - in.unit_position[1];
    const float dz = in.aim[2] - in.unit_position[2];
    // 009B7D4E-009B7D67: 007BCC80(-dy) + 0.15 on the unit (ECX).
    a.fall_time_a4 = static_cast<float>(
        static_cast<double>(weapon_fall_time_007bcc80(-dy, in.unit_velocity[1])) +
        level_bomb::kFallTimeBias);
    // 009B7E24-009B7EB5, the bomb branch (007B9500 false): the straight-line
    // point the release reaches after +A8h + +A4h seconds. The paratrooper
    // branch 009B7D7D-009B7E1F (007BCCF0) is not modelled.
    const float k = a.release_delay_a8 + a.fall_time_a4;
    for (int i = 0; i < 3; ++i) a.impact[i] = in.unit_position[i] + in.unit_velocity[i] * k;
    a.impact[1] = in.aim[1];                              // 009B7EC8
    // 009B7ED1-009B7F21, 009B7F29-009B7F65.
    const float d = planar(dx, dz);
    a.planar_bc = d;
    a.bearing_c0 = heading_command_009f9e40(in.aim[0], in.aim[2], in.unit_position[0],
                                            in.unit_position[2]);
    // 009B7F58-009B7FF4: the latch, entered below +B8h and left above 1.1 x +B8h
    // (or held while ctl+369h and [00E17BF2]).
    bool latch;
    if (a.in_range_cd) {
        latch = in.reload_369_e17bf2 ||
                static_cast<double>(a.in_range_b8) * level_bomb::kLatchRelease > d;
    } else {
        latch = a.in_range_b8 > d;
    }
    a.in_range_cd = latch;
    if (a.has_ordnance_ce) {
        // 009B7FAF-009B802F: the target ref's projection time.
        float t = static_cast<float>(static_cast<double>(
            a.release_delay_a8 + a.fall_time_a4 + a.time_error_c4) - level_bomb::kProjTimeBias);
        if (0.0f > t) t = 0.0f;                           // 009B7FD8
        else if (static_cast<double>(t) > level_bomb::kProjTimeCap) t = 30.0f;   // 009B8025
        a.projtime_74 = t;
        return;
    }
    // 009B803C-009B80E6: a spent member keeps its latch only while its leader
    // is within +B8h of this aim point.
    if (in.reload_369_e17bf2) return;
    if (in.unit_is_leader) return;
    const float ld = planar(in.aim[0] - in.leader_position[0], in.aim[2] - in.leader_position[2]);
    if (!(a.in_range_b8 > ld)) a.in_range_cd = false;
}

bool level_bomb_is_attacking_009b7b70(LevelBombState s) noexcept {
    return s == LevelBombState::kAttackRun || s == LevelBombState::kAim ||
           s == LevelBombState::kPrepare || s == LevelBombState::kRelease ||
           s == LevelBombState::kGoAway;
}

LevelBombState level_bomb_entry_state_009b8820(const LevelBombEntryInputs& in) noexcept {
    if (in.release_pending_c25) return LevelBombState::kRelease;            // 009B8844
    if (!in.leader && in.mode_370 == 0) return LevelBombState::kPrepare;    // 009B8873
    if (in.has_ordnance_ce) {
        return in.in_range_cd ? LevelBombState::kAim : LevelBombState::kAttackRun;
    }
    return LevelBombState::kGoAway;                                          // 009B88B6
}

LevelBombState level_bomb_next_state_009b88f0(const LevelBombTransitionInputs& in) noexcept {
    using S = LevelBombState;
    const LevelBombEntryInputs& e = in.entry;
    // 009B8935-009B8962: the latch, or a forced attack on a live latched target.
    const bool engaged = e.in_range_cd ||
        (e.mode_370 == 2 && in.has_latched_target && !in.target_disabled_5d);
    const S approach = e.leader ? S::kMoveTo : S::kFollow;   // 009B8AFE-009B8B3F
    if (!level_bomb_is_attacking_009b7b70(in.current)) {
        return engaged ? level_bomb_entry_state_009b8820(e) : approach;   // 009B89E8
    }
    if (!engaged) return approach;
    if (e.mode_370 == 0 && !e.leader) return S::kPrepare;   // 009B8972-009B898E
    if (in.break_off) return S::kGoAway;                    // 009B899E, 009B8A8F
    switch (in.current) {
        case S::kPrepare:
            if (in.prepare_released_a0) return S::kRelease;           // 009B89C3
            if (!e.leader && in.prepare_waits) return S::kPrepare;    // 009B89DB
            return level_bomb_entry_state_009b8820(e);                // 009B89EA
        case S::kAttackRun:
            return e.in_range_cd ? S::kAim : S::kAttackRun;           // 009B8A0F
        case S::kAim: {
            // 009B7B10 / 009B7B40 on the aim state.
            const bool release = in.aim_release_18 || (in.aim_abort_19 && in.force_release_3ae);
            if (release) return S::kRelease;                          // 009B8A38
            const bool abort = in.aim_abort_19 && !in.force_release_3ae;
            if (!abort && e.has_ordnance_ce) return S::kAim;
            return S::kGoAway;                                        // 009B8A63
        }
        case S::kRelease:
            // 009B8A6B-009B8A91: leave once release+1Ch drops below zero.
            return (0.0f > in.release_timer_1c) ? S::kGoAway : S::kRelease;
        case S::kGoAway:
            return (in.goaway_ready && e.has_ordnance_ce) ? S::kAim : S::kGoAway;   // 009B8AC3
        default:
            return in.current;
    }
}

bool level_bomb_should_break_off_009b8d80(const LevelBombBreakOffInputs& in) noexcept {
    if (!in.base_0099c230) return false;                              // 009B8D8D
    if (!in.has_latched_target || in.target_disabled_5d) return true; // 009B8D9E-009B8DA8
    if (in.reload_369_e17bf2) return false;                           // 009B8DC4
    if (in.attacking && in.has_ordnance_ce) return false;             // 009B8DD6-009B8DDF
    // 009B8E2E-009B8E68: min(2500, SafeDist) x task+41Ch <= the 3D distance.
    const float cap = in.safe_dist_450 < level_bomb::kSafeDistCap ? in.safe_dist_450
                                                                  : level_bomb::kSafeDistCap;
    return !(cap * in.speed_ratio_41c > in.distance);
}

bool level_bomb_goaway_ready_009b7ab0(float standoff_18, float planar_bc,
                                      bool reload_369_e17bf2,
                                      bool has_ordnance_ce) noexcept {
    if (reload_369_e17bf2 && !has_ordnance_ce) return false;          // 009B7ACF
    return standoff_18 < planar_bc;                                   // 009B7AF1
}

LevelBombAimGeometry level_bomb_aim_geometry_009b5c86(float bearing_c0, float heading,
                                                      const float aim[3],
                                                      const float impact[3],
                                                      float turn_radius_268) noexcept {
    LevelBombAimGeometry g;
    g.bearing_error = wrapped_angle_subtract_00438b10(bearing_c0, heading);    // 009B5CB9
    // 009B5CC4-009B5D3F: the bearing from the predicted impact point to the
    // aim point (FLD dz, FLD dx, atan2), and the planar miss.
    const float b = heading_command_009f9e40(aim[0], aim[2], impact[0], impact[2]);
    g.impact_distance = planar(aim[0] - impact[0], aim[2] - impact[2]);
    g.impact_bearing_error = wrapped_angle_subtract_00438b10(b, heading);      // 009B5D94
    g.ratio = g.impact_distance / turn_radius_268;                            // 009B5DA6
    g.probe_length = clamped_interpolate_00419010(0.1f, 120.0f, 0.6f, 300.0f, g.ratio);
    return g;
}

void level_bomb_aim_close_009b5dc3(const LevelBombAimGeometry& g, bool has_ordnance_ce,
                                   bool force_release_3ae, bool& flag_18,
                                   bool& flag_19) noexcept {
    // 009B5DC3-009B5E76: close to the impact solution and the target swings
    // wide - abort (19h), or with a forced release pull out (18h).
    if (!(g.ratio < level_bomb::kAimCloseRatio)) return;
    const float lim = clamped_interpolate_00419010(0.1f, 0.5235987901687622f, 2.0f,
                                                   2.6179940700531006f, g.ratio);
    if (abs_wrapped(g.bearing_error) > lim && has_ordnance_ce) {
        if (!force_release_3ae) {
            flag_19 = true;
        } else {
            flag_19 = false;
            if (abs_wrapped(g.bearing_error) > level_bomb::kAimPullOut) flag_18 = true;
        }
    }
}

LevelBombAimCommand level_bomb_aim_tick_009b5c80(const LevelBombAimInputs& in,
                                                 bool& flag_18, bool& flag_19) noexcept {
    using namespace level_bomb;
    const LevelBombAimGeometry& g = in.g;
    LevelBombAimCommand out;
    float alt_offset = 0.0f;   // [ESP+1Ch], 0 unless the steering block runs
    float vert = 0.0f;         // [ESP+24h]
    if (!flag_19) {
        float nudge = 0.0f;
        if (in.probe_ran &&
            static_cast<double>(abs_wrapped(in.probe.out_a[0])) > kAimProbeTrigger) {
            float s = -in.probe.out_a[0] * in.probe.out_b[1] * in.probe.out_b[2];
            const float t = static_cast<float>(-in.probe.out_a[2] * kAimProbeVert);
            vert = (-1.0f > t) ? -1.0f : (t > 1.0f ? 1.0f : t);       // 009B5F79-009B5FC7
            if (s > 0.5f) {
                s = static_cast<float>((s - 0.5) + (s - 0.5));          // 009B5FA2
            } else if (-0.5f > s) {
                s = static_cast<float>((s + 0.5) + (s + 0.5));          // 009B5FDC
            } else {
                s = 0.0f;
            }
            const float w = clamped_interpolate_00419010(150.0f, 0.15f, 350.0f, 1.0f,
                                                         g.impact_distance);
            nudge = static_cast<float>(s * kAimHeadingNudge) * w;     // 009B6027-009B6039
        }
        // 009B603D-009B6119: the height offset from out_a.y, +-80.
        if (in.probe_ran) {
            const float u = static_cast<float>(in.probe.out_a[1] * kAimAltOffsetGain);
            alt_offset = (kAimAltOffsetLo > u) ? kAimAltOffsetLo
                                               : (u > kAimAltOffsetHi ? kAimAltOffsetHi : u);
        }
        const float b = wrapped_angle_add_00438aa0(in.bearing_c0, nudge);   // 009B607F
        const float e = wrapped_angle_subtract_00438b10(b, in.heading);
        const float lim = clamped_interpolate_00419010(0.3f, 0.0872664675116539f, 1.5f,
                                                       1.3962634801864624f, g.ratio);
        const float ec = (-lim > e) ? -lim : (e > lim ? lim : e);       // 009B60E2-009B6132
        out.writes_heading = true;
        out.heading_2c0 = wrapped_angle_add_00438aa0(in.heading, ec);  // 009B6150
        out.bank_limit_2c8 = clamped_interpolate_00419010(0.1f, 0.2f, 0.8f, 1.2f, g.ratio);
    }
    // 009B61B4-009B62E3: the altitude.
    const float base = in.aim_y + in.drop_alt_b0 + alt_offset;
    float dy = in.unit_y - base;
    if (!(dy > 0.0f)) dy = -0.0f - dy;
    if (static_cast<double>(g.impact_distance) < kAimLevelBelow) {
        out.pitch_level = true;                                         // 009B6238-009B6243
    } else {
        out.pitch_base = base;
        out.pitch_low = kAimPitchLow;
        out.pitch_high = g.impact_distance;
        const float x = static_cast<float>(
            dy / (static_cast<double>(g.impact_distance) - kAimDenomBias));
        out.pitch_scale = clamped_interpolate_00419010(0.05f, 0.1f, 0.35f, 1.0f, x);
    }
    // 009B62E8-009B6384: the speed.
    const float span = static_cast<float>(static_cast<double>(in.max_speed_ac) -
                                          in.level_flight_speed);
    const float k = clamped_interpolate_00419010(0.0f, 0.4f, 0.3f, 1.0f, g.ratio);
    out.speed_2b4 = k * span * vert + in.max_speed_ac;
    out.close_cc = kAimBayClose > g.impact_distance;                   // 009B6394
    // 009B639A-009B6428: the release (18h) and abort (19h) bearings.
    if (!flag_18 && !(kAimNearImpact > g.impact_distance)) {
        flag_18 = false;
        if (abs_wrapped(g.impact_bearing_error) > kAimAbortBearing && 1.0f > g.ratio &&
            !in.force_release_3ae) {
            flag_19 = true;
        }
    } else if (abs_wrapped(g.impact_bearing_error) > kAimReleaseBearing) {
        flag_18 = true;
    }
    // 009B6428-009B645B.
    out.step_c8 = (!flag_18 &&
                   !(static_cast<double>(in.attack_dist_b4) * 0.5 > g.impact_distance)) ? 3 : 1;
    return out;
}

float level_bomb_spacing_speed(float max_speed_188, float level_flight_speed,
                               float frac) noexcept {
    const float top = static_cast<float>(max_speed_188 * level_bomb::kSpeedMaxMul);
    return clamped_interpolate_00419010(0.0f, top, level_bomb::kSpeedFracTop,
                                        level_flight_speed, frac);
}

float level_bomb_speed_fraction(float max_speed_188, float level_flight_speed,
                                float speed) noexcept {
    const float top = static_cast<float>(max_speed_188 * level_bomb::kSpeedMaxMul);
    return clamped_interpolate_00419010(top, 0.0f, level_flight_speed,
                                        level_bomb::kSpeedFracTop, speed);
}

LevelBombRunCommand level_bomb_attackrun_009b5320(const LevelBombRunInputs& in) noexcept {
    LevelBombRunCommand out;
    out.heading_2c0 = wrapped_angle_add_00438aa0(in.bearing_c0, in.offset_20);   // 009B533C
    // 009B5360-009B5382: min(+BCh, 2000).
    const float dist = (level_bomb::kRunDistCap > in.planar_bc) ? in.planar_bc : 2000.0f;
    // 009B546B-009B549B: max(1400 - y, 50).
    float h = static_cast<float>(level_bomb::kRunCeiling - in.unit_y);
    if (level_bomb::kRunHeightFloor > h) h = 50.0f;
    out.pitch_base = in.aim_y + in.drop_alt_b0;                       // 009B54C1
    out.pitch_low = in.attack_dist_b4;
    out.pitch_high = in.planar_bc;
    out.pitch_scale = clamped_interpolate_00419010(0.1f, 0.4f, 0.35f, 1.0f, h / dist);
    out.speed_2b4 = level_bomb_spacing_speed(in.max_speed_188, in.level_flight_speed,
                                             in.speed_frac_24);       // 009B5534-009B5585
    return out;
}

LevelBombSpacing level_bomb_spacing_gaps(float own_distance, unsigned own_id,
                                         const LevelBombPeer* peers,
                                         std::size_t count) noexcept {
    LevelBombSpacing s;
    const double far_edge = level_bomb::kPeerOrderBand + own_distance;
    for (std::size_t i = 0; i < count; ++i) {
        const float d = peers[i].distance;
        const bool behind = static_cast<float>(far_edge) < d ||
            (static_cast<float>(own_distance - level_bomb::kPeerOrderBand) < d &&
             own_id < peers[i].id);
        if (behind) {
            if (s.behind_gap < 0.0f || d - own_distance < s.behind_gap) {
                s.behind_gap = d - own_distance;
                if (static_cast<double>(s.behind_gap) < level_bomb::kPeerGapFloor)
                    s.behind_gap = level_bomb::kPeerGapFloorValue;
            }
        } else if (s.ahead_gap < 0.0f || own_distance - d < s.ahead_gap) {
            s.ahead_gap = own_distance - d;
            if (static_cast<double>(s.ahead_gap) < level_bomb::kPeerGapFloor)
                s.ahead_gap = level_bomb::kPeerGapFloorValue;
        }
    }
    return s;
}

float level_bomb_spacing_fraction(const LevelBombSpacing& s) noexcept {
    float f = 0.0f;
    if (s.ahead_gap > 0.0f) {
        f = clamped_interpolate_00419010(180.0f, 1.0f, 400.0f, 0.0f, s.ahead_gap);
        if (s.behind_gap > 0.0f) {
            f -= clamped_interpolate_00419010(0.1f, 0.5f, 0.5f, 0.0f,
                                              s.behind_gap / s.ahead_gap);
        }
        if (f < 0.0f) f = 0.0f;
        else if (f > 1.0f) f = 1.0f;
    }
    return f;
}

float level_bomb_spacing_tolerance(const LevelBombSpacing& s, float current_40) noexcept {
    float t = current_40;
    if (s.ahead_gap > 0.0f) {
        t = clamped_interpolate_00419010(250.0f, 0.0f, 500.0f, 1.2f, s.ahead_gap);
    }
    if (s.behind_gap > 0.0f) {
        const float b = clamped_interpolate_00419010(250.0f, 0.0f, 500.0f, 1.2f, s.behind_gap);
        if (b < t) t = b;
    }
    return t;
}

void level_bomb_release_enter_009b5b00(LevelBombReleaseState& r, bool release_pending_c25,
                                       float current_speed, float level_flight_speed,
                                       float bank_c68, float pitch_c64) noexcept {
    r.requested_18 = release_pending_c25;                             // 009B5B1A
    r.timer_1c = level_bomb::kReleaseHold;                            // 009B5B1D
    r.speed_20 = level_flight_speed > current_speed ? level_flight_speed : current_speed;
    r.bank_hold_19 = abs_wrapped(bank_c68) > level_bomb::kReleaseLevelAngle;
    r.pitch_hold_1a = abs_wrapped(pitch_c64) > level_bomb::kReleaseLevelAngle;
}

float level_bomb_goaway_standoff_base(float safe_dist_450, bool has_extent_target,
                                      float target_extent) noexcept {
    float d = safe_dist_450;                                          // 009B56BE
    if (has_extent_target && !(d > target_extent)) d = target_extent; // 009B56F4
    if (d > level_bomb::kGoAwayCap) d = level_bomb::kGoAwayCap;       // 009B5711
    return d;
}

float level_bomb_goaway_heading_009b5845(float heading, float fly_to_heading,
                                         float planar_bc, float probe) noexcept {
    float turn = wrapped_angle_subtract_00438b10(fly_to_heading, heading);   // 009B5840
    if (level_bomb::kAimNearImpact > planar_bc) {
        turn = 0.0f;                                                  // 009B5862
    } else {
        if (level_bomb::kGoAwayTurnLo > turn) turn = level_bomb::kGoAwayTurnLo;
        else if (turn > level_bomb::kGoAwayTurnHi) turn = level_bomb::kGoAwayTurnHi;
    }
    // 009B58A1-009B58BE: push along the probe when it opposes the turn.
    const bool push = (turn >= 0.0f && 0.0f > probe) || (!(0.0f < turn) && probe > 0.0f);
    if (push) {
        turn = static_cast<float>(probe * level_bomb::kGoAwayNudge + turn);
    }
    return wrapped_angle_add_00438aa0(heading, turn);                 // 009B58D8
}

bool level_bomb_attitude_gate_007cc8e0(float bank_c68, float pitch_c64,
                                       float angle_max_550) noexcept {
    return std::fabs(bank_c68) <= angle_max_550 && std::fabs(pitch_c64) <= angle_max_550;
}

}  // namespace bsp
