// The torpedo approach update 009D3420 and its helpers. Evidence in
// docs/TORPEDO_APPROACH_UPDATE.md; every address in a comment is the listing
// line the rule came from.

#include "bsp/torpedo_approach_update.hpp"

#include <cmath>

namespace bsp {
namespace {

// 009D3768 and 009D37A0: the decompiler's `0.0 - x` guarded by `x <= 0` is an
// absolute value written with the -0.0 at 00D7A208.
float abs_009d3766(float v) noexcept { return v < 0.0f ? -v : v; }

}  // namespace

void torpedo_approach_apply_profile_009d3433(TorpedoApproachState& s,
                                             const TorpedoApproachControl& ctl) noexcept {
    // 009D3433 CMP byte [ECX+3ADh],0 / 009D3440 MOVSS [ESI+78h],XMM0 with
    // XMM0 zeroed by XORPS at 009D343D.
    if (ctl.profile_dirty_3ad == 0) {
        s.alt_margin_78 = 0.0f;
    }
    // 009D3445-009D3485: FCOMI on [ESI+80h] against [ECX+39Ch]; when the
    // commanded speed is above the block's ceiling, divide the late speed by
    // the old ceiling, install the new one and multiply back, so the ratio
    // between the two speeds survives the clamp.
    if (s.speed_early_80 > ctl.speed_ceiling_39c) {
        const float ratio = s.speed_early_80 != 0.0f
                                ? s.speed_late_7c / s.speed_early_80
                                : 0.0f;
        s.speed_early_80 = ctl.speed_ceiling_39c;
        s.speed_late_7c = ratio * ctl.speed_ceiling_39c;
    }
    // 009D3489-009D34AE: FLD [ECX+398h], FCOMIP against the double 100 at
    // 00D7A220, JBE skips the store.
    if (ctl.second_altitude_398 < kTorpedoSecondAltitudeCeiling_00d7a220) {
        s.alt_floor_74 = ctl.second_altitude_398;
    }
}

float torpedo_approach_range_009d3519(const float unit_xz[2],
                                      const float target_xz[2]) noexcept {
    // 009D3519-009D352A: dx and dz from the target point minus the aircraft.
    const float dx = target_xz[0] - unit_xz[0];
    const float dz = target_xz[1] - unit_xz[1];
    // 009D3536-009D3550: the sum of squares against the double 1e-10, then
    // 00BF7030 (sqrt) or zero.
    const double d2 = static_cast<double>(dz) * dz + static_cast<double>(dx) * dx;
    if (d2 <= kTorpedoRangeEpsilonSquared_00ce3820) return 0.0f;
    return static_cast<float>(std::sqrt(d2));
}

float torpedo_approach_bearing_009d3586(const float unit_xz[2],
                                        const float target_xz[2]) noexcept {
    const float dx = target_xz[0] - unit_xz[0];
    const float dz = target_xz[1] - unit_xz[1];
    // 009D3570-009D3586: FLD dz, FLD dx, CALL 00BF701A. 009D3593 subtracts the
    // result from the double pi/2 at 00CE3830 and 009D35A7 wraps a negative
    // result by the double 2*pi at 00CE3828.
    float bearing = kHalfPi_00ce3830 -
                    static_cast<float>(std::atan2(static_cast<double>(dz),
                                                  static_cast<double>(dx)));
    if (bearing < 0.0f) bearing += kTwoPi_00ce3828;
    return bearing;
}

bool torpedo_in_range_latch_009d361e(bool latched, float range,
                                     float engage_range, bool always_engage) noexcept {
    if (!latched) {
        // 009D3770-009D377C: FCOMIP [ESI+8Ch] against the range, JA sets 1.
        return range < engage_range;
    }
    // 009D35E5-009D35F5: with the block's +369h byte and the global at
    // 00E17BF2 both set, the latch never opens.
    if (always_engage) return true;
    // 009D35FB-009D360F: the release compares the range against 1.1 times the
    // engage distance.
    return range < engage_range * kTorpedoRangeHysteresis_00ce3df0;
}

float torpedo_commanded_speed_009d3c99(float elapsed_134, float speed_late_7c,
                                       float speed_early_80) noexcept {
    // 009D3C93-009D3CB4, and the same three instructions at 009D1509 and
    // 009D4874: the late speed applies once the approach is 15 seconds old.
    return elapsed_134 >= kTorpedoSpeedSwitchSeconds_00cf3f20 ? speed_late_7c
                                                              : speed_early_80;
}

TorpedoRunSpeeds torpedo_seed_run_speeds_009d0484(float profile_late_4,
                                                  float profile_early_8,
                                                  float scale_24) noexcept {
    // 009D0484-009D0497. The scale is loaded once at 009D047D and duplicated by
    // 009D048B FLD ST(0), so the same approach+24h multiplies both fields; the
    // FXCH at 009D048F is what keeps the copy alive for the second FMUL.
    //
    // CORRECTED by packet cc8_torpedo_run_profile: the two arguments are
    // TorpReleaseDistNear (record+4h) and TorpReleaseDistFar (record+8h) from
    // PilotBotConfig.levels[idx], and the scale is
    // max(1.0, desc.MaxSpd / Pilot/Torpedo/ReferenceSpeed) from 009F9D30-009F9D61.
    // They are DISTANCES in metres. The parameter and field names still say
    // speed; renaming reaches past this packet's lease and is a follow-up.
    TorpedoRunSpeeds out;
    out.speed_late_7c = profile_late_4 * scale_24;
    out.speed_early_80 = scale_24 * profile_early_8;
    return out;
}

float torpedo_engagement_eta_009d3c93(const TorpedoApproachState& s,
                                      float unit_speed) noexcept {
    const float speed = torpedo_commanded_speed_009d3c99(
        s.elapsed_134, s.speed_late_7c, s.speed_early_80);
    // 009D3CB4-009D3CD9: the range less the standoff, floored at zero.
    float remaining = s.range_90 - speed;
    if (remaining < 0.0f) remaining = 0.0f;
    // 009D3CF8 FADD ST0,ST0 doubles it; 009D3D03 adds the closing-speed bias to
    // the aircraft's own speed and 009D3D07 divides.
    const float closing = unit_speed + s.closing_speed_bias_70;
    float eta = s.run_time_98;
    if (closing != 0.0f) eta += (remaining + remaining) / closing;
    // 009D3D26-009D3D65: below zero it stores zero, above the double 30 at
    // 00CE7630 it stores the float 30 at 00CE38C8.
    if (eta < 0.0f) return 0.0f;
    if (eta > kTorpedoEtaCeiling_00ce7630) return kTorpedoEtaCeilingValue_00ce38c8;
    return eta;
}

// CORRECTED by packet cc8_torpedo_run_profile. The name says time to target and
// the quantity is not one. The divisor is the release distance the 15 s switch
// selects, TorpReleaseDistNear or TorpReleaseDistFar scaled by approach+24h, so
// this returns a RANGE RATIO: how many release distances out the aircraft still
// is. That is why the aim tick's clause 2 at 009D22D9 compares it against a
// steering delta in radians and why the 600.0 at 00D20198 is not a speed. The
// name is kept because it is load bearing across three files; renaming it is a
// follow-up. docs/TORPEDO_RUN_PROFILE.md has the producer.
float torpedo_time_to_target_009d1500(const TorpedoApproachState& s) noexcept {
    const float speed = torpedo_commanded_speed_009d3c99(
        s.elapsed_134, s.speed_late_7c, s.speed_early_80);
    if (speed == 0.0f) return 0.0f;
    // 009D1538: the range over the selected release distance.
    const float t = s.range_90 / speed;
    // 009D1547: at or under one release distance the quotient stands.
    if (t <= 1.0f) return t;
    // 009D1565-009D1595: below the float 600 at 00CE4BC4 the metric switches to
    // one second for the first `speed` units and 600 units a second after it.
    if (speed < kTorpedoCruiseSpeedFloor_00ce4bc4) {
        return static_cast<float>((s.range_90 - speed) / kTorpedoCruiseSpeedRate_00d20198 +
                                  kTorpedoCruiseSpeedFirstSecond_00d7a210);
    }
    return t;
}

TorpedoRunTimeResult torpedo_run_time_009d1360(const TorpedoApproachState& s,
                                               const TorpedoRunTimeInputs& in) noexcept {
    TorpedoRunTimeResult out;
    // 009D13A9: the distance the torpedo carries forward while it falls.
    out.fall_lead_a0 = in.fall_time * in.unit_speed;

    // 009D13BC-009D13D0: the time to bleed off to the run speed at 80 a second,
    // floored at zero, and the distance that costs.
    float decel_time = (in.unit_speed - in.run_speed) / kTorpedoDecelRate_00cf1440;
    if (decel_time < 0.0f) decel_time = 0.0f;
    const float decel_distance =
        decel_time * (in.run_speed + in.unit_speed) * kTorpedoHalf_00d7a280;

    // 009D13E8-009D1416: the leg to fly, capped at the commanded speed and
    // reduced by the air phase.
    const float speed = torpedo_commanded_speed_009d3c99(
        s.elapsed_134, s.speed_late_7c, s.speed_early_80);
    float leg = s.range_90 < speed ? s.range_90 : speed;
    leg -= out.fall_lead_a0;

    float total = in.fall_time;
    if (leg >= 0.0f) {
        if (decel_distance <= leg) {
            // 009D1440: the whole deceleration fits, the rest runs at speed.
            total += decel_time;
            if (in.run_speed != 0.0f) total += (leg - decel_distance) / in.run_speed;
        } else {
            // 009D145C: the leg ends part way through the deceleration.
            total += decel_distance != 0.0f ? decel_time * (leg / decel_distance) : 0.0f;
        }
    }
    // 009D147A: the bias at +9Ch, then the floor at zero into +98h.
    total += s.run_time_bias_9c;
    out.run_time_98 = total < 0.0f ? 0.0f : total;
    return out;
}

float torpedo_engage_range_009d4ac4(float current_8c, float attack_dist_tuning,
                                    float speed_ratio_41c) noexcept {
    // 009D4AC4-009D4AD8: FMUL the tuning row by the speed ratio at task+41Ch,
    // then keep whichever of that and task+484h is larger.
    const float scaled = attack_dist_tuning * speed_ratio_41c;
    return scaled < current_8c ? current_8c : scaled;
}

float torpedo_sector_bearing_009d38a6(int sector) noexcept {
    // 009D38B4-009D38D8: pi/2 minus sector * pi / 18, wrapped by 2*pi.
    float bearing = kHalfPi_00ce3830 -
                    static_cast<float>(static_cast<double>(sector) * kPi_00ce3d28 /
                                       kTorpedoSectorsPerHalfTurn_00cee930);
    if (bearing < 0.0f) bearing += kTwoPi_00ce3828;
    return bearing;
}

int torpedo_home_sector_009d3bc8(float bearing_target_to_unit) noexcept {
    // 009D3BAF CALL 00BF7420, the float-to-int truncation, then 009D3BCE
    // CMP 0x23 clamps to the last sector.
    int sector = static_cast<int>(bearing_target_to_unit);
    if (sector > kTorpedoSectorCount - 1) sector = kTorpedoSectorCount - 1;
    return sector;
}

TorpedoTurnPlan torpedo_turn_plan_009d3bce(const unsigned char sector_clear[],
                                           int clear_count, int home_sector) noexcept {
    TorpedoTurnPlan plan;
    // 009D3C5A-009D3C88: with every sector clear or none clear there is no
    // turn to make and the flag at +AAh goes up.
    if (clear_count == kTorpedoSectorCount || clear_count == 0) {
        plan.no_clear_sector_aa = true;
        return plan;
    }
    if (home_sector < 0) home_sector = 0;
    if (home_sector > kTorpedoSectorCount - 1) home_sector = kTorpedoSectorCount - 1;

    // 009D3BDE-009D3BFE: walk up from the sector after the home one.
    int forward = 0;
    int i = home_sector < kTorpedoSectorCount - 1 ? home_sector + 1 : 0;
    for (int guard = 0; guard < kTorpedoSectorCount && sector_clear[i] == 0; ++guard) {
        ++i;
        ++forward;
        if (i > kTorpedoSectorCount - 1) i = 0;
    }
    // 009D3C04-009D3C24: walk down from the home sector itself.
    int backward = 0;
    int j = home_sector;
    for (int guard = 0; guard < kTorpedoSectorCount && sector_clear[j] == 0; ++guard) {
        ++backward;
        --j;
        if (j < 0) j = kTorpedoSectorCount - 1;
    }

    // 009D3C2A-009D3C4E: the cheaper walk wins, and the forward one divides by
    // -18 while the backward one divides by +18, so the sign of the result is
    // the direction of the turn.
    if (forward < backward) {
        plan.turn_offset_5c =
            static_cast<float>(static_cast<double>(forward) * kPi_00ce3d28 /
                               kTorpedoSectorsPerHalfTurnNeg_00d21430);
    } else {
        plan.turn_offset_5c =
            static_cast<float>(static_cast<double>(backward) * kPi_00ce3d28 /
                               kTorpedoSectorsPerHalfTurn_00cee930);
    }
    plan.forward_gap_68 = forward;
    plan.backward_gap_6c = backward;
    // 009D3C5E: both walks at zero means the home sector is the only clear one.
    plan.no_clear_sector_aa = (forward == 0 && backward == 0);
    return plan;
}

namespace {

// 009D37AE-009D3A2A, the sector scan. It marks each of the 36 compass sectors
// around the target clear or blocked by marching out from the scan start radius
// to the cap, checking the terrain slope past 450 units and the blocked test
// inside it.
void scan_sectors(TorpedoApproachHost& host, TorpedoApproachState& s,
                  const float target_point[3], float target_x, float target_z,
                  float start_radius, float slope_limit, float slope_step) {
    // 009D37EE-009D3808: the cap is the scan seed times 1.2, held down by the
    // engage distance.
    float cap = static_cast<float>(static_cast<double>(s.scan_radius_seed_88) *
                                   kTorpedoScanRangeFactor_00cec160);
    if (cap > s.engage_range_8c) cap = s.engage_range_8c;

    // 009D3820-009D3840: the ground under the target, floored at 20 when it is
    // below sea level.
    float ground_at_target = host.terrain_height_0041bc20(target_x, target_z);
    if (static_cast<double>(ground_at_target) < kTorpedoScanSeaLevel_00ce3d88) {
        ground_at_target = kTorpedoScanSeaFloor_00ce3930;
    }

    s.sector_clear_count_58 = 0;  // 009D380E MOV [ESI+58h],EDX
    for (int sector = 0; sector < kTorpedoSectorCount; ++sector) {
        s.sector_clear_30[sector] = 1;  // 009D38A6 MOV byte [puVar12],1
        ++s.sector_clear_count_58;
        const float bearing = torpedo_sector_bearing_009d38a6(sector);
        const float dir_x = std::cos(bearing);
        const float dir_z = std::sin(bearing);

        float radius = start_radius;
        float limit = slope_limit;
        float step = slope_step;
        while (radius < cap) {
            const float px = dir_x * radius + target_x;
            const float pz = dir_z * radius + target_z;
            bool blocked = false;
            if (static_cast<double>(radius) >= kTorpedoScanLosRadius_00d1f4c0) {
                // 009D3960-009D399A: the slope from the target's ground up to
                // the probe over the radius past 400.
                const float h = host.terrain_height_0041bc20(px, pz);
                const double denom =
                    static_cast<double>(radius) - kTorpedoScanSlopeBase_00ce3d90;
                if (denom != 0.0 &&
                    static_cast<double>(limit) <
                        (static_cast<double>(h) - ground_at_target) / denom) {
                    blocked = true;
                }
            } else {
                // 009D39CB: the short-range blocked test.
                const float probe[3] = {px, 1.0f, pz};
                blocked = host.segment_blocked_00903bc0(target_point, probe);
            }
            if (blocked) {
                // 009D39E0: unmark the sector and stop walking it.
                s.sector_clear_30[sector] = 0;
                --s.sector_clear_count_58;
                break;
            }
            radius = static_cast<float>(radius + kTorpedoScanRadiusStep_00d1f3f8);
            limit += step;
            step = static_cast<float>(step + kTorpedoScanSlopeStepGrowth_00d7a2f8);
        }
    }
    // 009D39FD: index 36 is a wrap copy of index 0.
    s.sector_clear_30[kTorpedoSectorCount] = s.sector_clear_30[0];
}

}  // namespace

TorpedoApproachUpdateResult torpedo_approach_update_009d3420(
    TorpedoApproachHost& host, TorpedoApproachState& state, float dt) {
    TorpedoApproachUpdateResult out;

    const TorpedoApproachControl ctl = host.read_control_block();
    torpedo_approach_apply_profile_009d3433(state, ctl);

    // 009D34BB: the shared sub-object at approach+B4h runs before anything else.
    host.tick_approach_subobject_009fada0(dt);

    // 009D34C5 and 009D34CD: the ordnance flag is task+52Ah, the aim -> goaway
    // input of 009D4030 step 11 and row 2 of the entry chooser 009D3F60.
    state.has_ordnance_132 = host.unit_has_torpedo_ordnance_007b93f0();
    out.has_ordnance = state.has_ordnance_132;

    float unit_xz[2] = {0.0f, 0.0f};
    host.unit_world_xz(unit_xz);

    float target_point[3] = {0.0f, 0.0f, 0.0f};
    // 009D34E3 CMP dword [ESI+CCh],0 / 009D3506 JZ 009D3E21: with no target the
    // update writes nothing but the latch and the elapsed clock, so the range
    // at +90h and the bearing at +94h keep whatever the constructor left.
    if (!host.approach_target_point(target_point)) {
        state.in_range_latch_131 = false;   // 009D3E21
        state.elapsed_134 += dt;            // 009D3E28
        out.early_out_no_target = true;
        out.range = state.range_90;
        out.bearing = state.bearing_94;
        out.eta = state.eta_f8;
        out.in_range_latch = false;
        return out;
    }

    const float target_xz[2] = {target_point[0], target_point[2]};
    state.range_90 = torpedo_approach_range_009d3519(unit_xz, target_xz);   // 009D357E
    state.bearing_94 = torpedo_approach_bearing_009d3586(unit_xz, target_xz);  // 009D35C0
    out.range = state.range_90;
    out.bearing = state.bearing_94;

    // 009D35E5 and 009D3D7B: the pair that pins the latch closed.
    const bool always_engage = ctl.always_engage_369 != 0;
    state.in_range_latch_131 = torpedo_in_range_latch_009d361e(
        state.in_range_latch_131, state.range_90, state.engage_range_8c, always_engage);

    if (!state.has_ordnance_132) {
        // 009D3D72-009D3E1F, the branch for an aircraft with no torpedoes left.
        // It only ever clears the latch: the AND at 009D3E0F and 009D3E19 can
        // take it down but never puts it up.
        out.no_ordnance_path = true;
        if (!always_engage && ctl.has_designated_target_3d0 &&
            !ctl.designated_target_is_self) {
            float designated_xz[2] = {0.0f, 0.0f};
            if (host.target_world_xz(designated_xz)) {
                // 009D3DDE-009D3E07: 00414C60 on the target point minus the
                // flight leader's position, against the engage distance.
                const float dx = target_point[0] - designated_xz[0];
                const float dz = target_point[2] - designated_xz[1];
                const float len = static_cast<float>(
                    std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz));
                if (state.engage_range_8c <= len) state.in_range_latch_131 = false;
            }
        }
        state.elapsed_134 += dt;  // 009D3E28
        out.in_range_latch = state.in_range_latch_131;
        out.eta = state.eta_f8;
        return out;
    }

    // 009D3640-009D3682, the replan timer. Below dt it just runs down; at or
    // above it the period is added back and the plan is rebuilt this tick.
    bool replan = false;
    if (dt < state.replan_timer_12c) {
        state.replan_timer_12c -= dt;  // 009D3795
    } else {
        replan = true;
        state.replan_timer_12c += state.replan_period_128 - dt;  // 009D3682
        // 009D3688: the two writers of the over-land flag.
        if (state.scan_enabled_a8 != 0) {
            if (ctl.has_terrain_34c &&
                host.terrain_height_0041bc20(unit_xz[0], unit_xz[1]) >
                    kTorpedoOverLandHeight_00ce38b8) {
                state.over_land_a9 = 1;  // 009D36BC
            }
            // 009D36CA: with unit+C50h set, 007DF360 decides it outright.
            state.over_land_a9 =
                host.target_reachable_007df360(target_point) ? 1 : state.over_land_a9;
        }
    }
    out.replanned = replan;

    if (replan && ctl.has_terrain_34c) {
        // 009D3752-009D37A9: the plan stands while the target has moved less
        // than 120 units in x and in z since the cached position.
        const float dx = abs_009d3766(target_point[0] - state.plan_target_x_ac);
        const float dz = abs_009d3766(target_point[2] - state.plan_target_z_b0);
        const bool moved = !(static_cast<double>(dx) <= kTorpedoScanRadiusStep_00d1f3f8 &&
                             static_cast<double>(dz) <= kTorpedoScanRadiusStep_00d1f3f8);
        if (moved) {
            // 009D37AE-009D3A70: the scan retries at a shorter start radius
            // while it finds no clear sector, down to the floor at 50.
            float start_radius = kTorpedoScanStartRadius_00d2143c;
            float slope_limit = kTorpedoScanSlopeLimit_00cf0a2c;
            float slope_step = kTorpedoScanSlopeStep_00d1fbbc;
            while (static_cast<double>(start_radius) > kTorpedoScanRadiusFloor_00ce3938) {
                scan_sectors(host, state, target_point, target_point[0], target_point[2],
                             start_radius, slope_limit, slope_step);
                out.scan_ran = true;
                if (state.sector_clear_count_58 >= 1) {
                    // 009D3A5B: cache the position the plan was built for.
                    state.plan_target_x_ac = target_point[0];
                    state.plan_target_z_b0 = target_point[2];
                    replan = false;
                    break;
                }
                // 009D3A3E-009D3A52: shrink and widen the slope allowance.
                start_radius =
                    static_cast<float>(start_radius - kTorpedoScanRadiusShrink_00cf1440);
                slope_step = kTorpedoScanSlopeStepReset_00d21438;
                slope_limit =
                    static_cast<float>(slope_limit + kTorpedoScanSlopeStepGrowth_00d7a2f8);
            }
        }
    }

    if (replan) {
        // 009D3AF4-009D3C88: the turn plan, built from the sector the aircraft
        // sits in as seen from the target.
        if (state.sector_clear_count_58 == kTorpedoSectorCount ||
            state.sector_clear_count_58 == 0) {
            state.turn_offset_5c = 0.0f;       // 009D3C79 stores a zeroed XMM0
            state.no_clear_sector_aa = 1;      // 009D3C7E
            state.forward_gap_68 = 0;          // 009D3C88
            state.backward_gap_6c = 0;         // 009D3C85
        } else {
            float designated_xz[2] = {0.0f, 0.0f};
            (void)host.target_world_xz(designated_xz);
            // 009D3B93-009D3BC8: atan2 of the aircraft minus the target, then
            // the truncation to a sector index.
            const float bearing_from_target =
                torpedo_approach_bearing_009d3586(target_xz, unit_xz);
            state.home_sector_64 = torpedo_home_sector_009d3bc8(bearing_from_target);
            const TorpedoTurnPlan plan = torpedo_turn_plan_009d3bce(
                state.sector_clear_30, state.sector_clear_count_58, state.home_sector_64);
            state.turn_offset_5c = plan.turn_offset_5c;   // 009D3C50
            state.forward_gap_68 = plan.forward_gap_68;   // 009D3C53
            state.backward_gap_6c = plan.backward_gap_6c; // 009D3C56
            state.no_clear_sector_aa = plan.no_clear_sector_aa ? 1 : 0;  // 009D3C64
        }
    }

    // 009D3C93-009D3D6D and 009D3E28.
    state.eta_f8 = torpedo_engagement_eta_009d3c93(state, host.unit_speed_vtable38());
    state.elapsed_134 += dt;

    out.eta = state.eta_f8;
    out.in_range_latch = state.in_range_latch_131;
    return out;
}

TorpedoAimCommand torpedo_aim_tick_009d15f0(const TorpedoAimTickInputs& in) noexcept {
    TorpedoAimCommand cmd;

    // 009D1613-009D1631: the altitude floor is 30 over land and 5 over water,
    // then the cruise band and the ground plus 5 both push it up.
    float floor_alt = in.over_land_a9 ? kTorpedoEtaCeilingValue_00ce38c8 : 5.0f;
    const float band = in.alt_margin_78 + in.alt_floor_74;
    if (floor_alt < band) floor_alt = band;
    const float ground_clearance = in.ground_height + 5.0f;  // 009D16C6, [00D7A370]
    if (floor_alt < ground_clearance) floor_alt = ground_clearance;
    cmd.commanded_altitude_floor = floor_alt;
    cmd.commanded_altitude_2bc = floor_alt;

    // 009D1B4A-009D1D16: the commanded heading is the bearing to the target
    // plus the turn offset the approach update's sector scan chose. That is the
    // single link between 009D3420's plan and the aircraft's flight path.
    float heading = in.bearing_94 + in.turn_offset_5c;
    while (heading >= kTwoPi_00ce3828) heading -= kTwoPi_00ce3828;
    while (heading < 0.0f) heading += kTwoPi_00ce3828;
    cmd.commanded_heading_2c0 = heading;
    cmd.heading_mode_2cc = 2;           // 009D1D1E
    cmd.altitude_mode_2d0 = 1;          // 009D1EE5

    // 009D1D2E: the throttle. coverage: partial - the native folds four
    // BSP_Math_InterpolateClamped chains over the bank at unit+C64h, the time
    // to target and the altitude into the commanded speed; this keeps the
    // commanded speed and the ceiling at 00CE3814 and leaves the shaping out.
    cmd.commanded_throttle_2c8 = in.commanded_speed;

    cmd.roll_limit_2e8 = 0.0f;          // 009D1D02, [00D06874]
    cmd.flag_278 = 1.0f;                // 009D1F4A, [00D7A24C]
    cmd.flag_27c = 1;                   // 009D1F55
    cmd.flag_2a8 = 0.0f;                // 009D1F5C
    cmd.flag_2ac = 1;                   // 009D1F64
    cmd.flag_2d8 = 0;                   // 009D1F6B

    // 009D2021: the approach's aim-solution byte at +130h, from the bearing
    // error against a lead the tick scales by the commanded speed.
    const float bearing_error = in.bearing_94 - in.unit_heading;
    float wrapped = bearing_error;
    while (wrapped > kHalfPi_00ce3830 * 2.0f) wrapped -= kTwoPi_00ce3828;
    while (wrapped < -kHalfPi_00ce3830 * 2.0f) wrapped += kTwoPi_00ce3828;
    cmd.aim_solution_130 = (wrapped < 0.0f ? -wrapped : wrapped) < in.time_to_target;

    return cmd;
}

}  // namespace bsp
