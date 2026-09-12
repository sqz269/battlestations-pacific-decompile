// The middle of 009F3F80: the obstacle sectors, the reverse-manoeuvre arms and
// the throttle ceiling.
//
// Packet cc_ai_obstacle_tables. Every routine carries the address it comes
// from, the original ABI and its coverage. Ghidra was read-only for the code
// reading; names are hypotheses, not recovered symbols. See
// docs/SHIP_AI_OBSTACLE_TABLES.md for the evidence and the uncertainties.

#include "bsp/ship_ai_obstacle_tables.hpp"

#include <cmath>

#include "bsp/unit_motion.hpp" // unit_clamp_00415690, unit_step_towards_0042ac60
#include "bsp/unit_rudder.hpp" // clamped_interpolate_00419010, the wrapped angles

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef, __fastcall(const float* a, const float* b),
// RET 0, body 00415550-0041558E. 00415565 FCOMI ST0,ST1 with ST0 = *a and
// ST1 = *b, then JBE returns *b: the larger wins and an unordered compare
// returns *b. One call site here, 009F4406.
float max_by_ref_00415550(float a, float b) noexcept {
    return (a > b) ? a : b;
}

// The index arithmetic of 009D6B40, clamped into the profile so the projection
// cannot read outside the 65 bytes at blk+4h. The native has no such guard; it
// never needs one because all three call sites pass a window inside
// [-1.5, +1.5], which maps to bins 8..56.
int clamp_bin(int bin) noexcept {
    if (bin < 0) {
        return 0;
    }
    if (bin > kShipAiThrottleProfileBins - 1) {
        return kShipAiThrottleProfileBins - 1;
    }
    return bin;
}

} // namespace

// ---------------------------------------------------------------------------
// 009D6B40, the throttle profile
// ---------------------------------------------------------------------------
// 009D6B7F..009D6B99: FLD [value]; FLD 2.0; FADD; FLD 16.0; FMUL ST(2),ST(0);
// FLD 0.5; FADD ST(3),ST(0); FXCH ST(3); CALL 00BF7420. The helper at 00BF7420
// is CVTTSD2SI (00BF7435), so the conversion truncates toward zero.
int ship_ai_throttle_profile_index_009d6b40(float value) noexcept {
    const double scaled = (static_cast<double>(value) + kShipAiThrottleProfileOrigin)
                              * kShipAiThrottleProfileScale
                          + kShipAiThrottleProfileRound;
    return static_cast<int>(scaled);
}

// 009D6CE0..009D6CF0: FILD bin; FMUL 0.0625; FSUBRP against the 2.0 still on
// the stack from 009D6B7F, so the result is bin * 0.0625 - 2.0, stored as a
// float at 009D6CF0.
float ship_ai_throttle_profile_value_009d6b40(int bin) noexcept {
    return static_cast<float>(static_cast<double>(bin) * kShipAiThrottleProfileStep
                              - kShipAiThrottleProfileOrigin);
}

float ship_ai_apply_throttle_profile_009d6b40(const ShipAiThrottleProfile& profile,
                                              float value, float low, float high) noexcept {
    // 009D6B4F, CMP byte ptr [EBX+41h],0 with EBX = this: when the byte is set
    // the routine jumps straight to the clamp at 009D6CF2 and the table is not
    // consulted at all.
    if (profile.bypass_41) {
        return unit_clamp_00415690(value, low, high);
    }

    // 009D6B63 FCOMI ST0,ST1 with ST0 = low and ST1 = high, then JC: the table
    // walk runs only on a strict low < high, which an unordered compare does
    // not take. Otherwise 009D6B67..009D6B75 stores the midpoint and returns
    // it; 009D6CF2's clamp is not reached on that path.
    if (!(low < high)) {
        return static_cast<float>((static_cast<double>(low) + static_cast<double>(high))
                                  * kShipAiThrottleProfileRound);
    }

    const int low_bin = clamp_bin(ship_ai_throttle_profile_index_009d6b40(low));
    const int high_bin = clamp_bin(ship_ai_throttle_profile_index_009d6b40(high));
    int mid = ship_ai_throttle_profile_index_009d6b40(value);

    // 009D6BC4..009D6BD4: mid is pulled into [low_bin, high_bin], low first.
    if (mid < low_bin) {
        mid = low_bin;
    } else if (mid > high_bin) {
        mid = high_bin;
    }
    mid = clamp_bin(mid);

    // 009D6BD6..009D6BDF: a zero byte at the current bin ends the routine at
    // 009D6D48, which pops the stack and returns without touching the value,
    // not even to clamp it.
    const std::uint8_t here = profile.bin[static_cast<std::size_t>(mid)];
    if (here == 0) {
        return value;
    }

    // 009D6BE5..009D6C42, the downward scan, and 009D6C44..009D6C77, the
    // upward one. Each tracks the cheapest byte it has seen and the bin where
    // it first saw it, and each remembers the bin nearest to mid whose byte is
    // strictly worse than the current one (the "wall"). Both stop early when
    // the running best reaches zero.
    std::uint8_t best_down = here;
    std::uint8_t best_up = here;
    int best_down_bin = mid;
    int best_up_bin = mid;
    int wall_down = low_bin;
    int wall_up = high_bin;

    int cursor = mid;
    while (cursor > low_bin && best_down != 0) {
        const std::uint8_t weight = profile.bin[static_cast<std::size_t>(cursor - 1)];
        --cursor;
        if (weight < best_down) {
            best_down = weight;
            best_down_bin = cursor;
        } else if (weight > here && cursor > wall_down) {
            wall_down = cursor;
        }
    }

    cursor = mid;
    while (cursor < high_bin && best_up != 0) {
        const std::uint8_t weight = profile.bin[static_cast<std::size_t>(cursor + 1)];
        ++cursor;
        if (weight < best_up) {
            best_up = weight;
            best_up_bin = cursor;
        } else if (weight > here && cursor < wall_up) {
            wall_up = cursor;
        }
    }

    // 009D6C79..009D6CDC, the choice. All the byte compares are unsigned
    // (JBE/JNC); all the bin compares are signed (JGE/JL).
    int chosen = 0;
    if (best_up > best_down) {
        chosen = best_down_bin; // 009D6C7D
    } else if (best_up < best_down) {
        chosen = best_up_bin; // 009D6C89
    } else if (best_down < here) {
        // 009D6C97: the nearer of the two wins and a tie goes up.
        chosen = ((mid - best_down_bin) >= (best_up_bin - mid)) ? best_up_bin : best_down_bin;
    } else if (wall_down == low_bin && wall_up == high_bin) {
        // 009D6CC1: neither scan met a worse bin, so the nearer edge wins.
        chosen = ((mid - wall_down) < (wall_up - mid)) ? wall_down : wall_up;
    } else {
        // 009D6CCD: otherwise the side with more room to the end of the table.
        chosen = (wall_down < (kShipAiThrottleProfileBins - 1 - wall_up)) ? wall_down : wall_up;
    }

    // 009D6CE0 writes the bin's value back through the pointer and falls into
    // the clamp at 009D6CF2.
    const float snapped = ship_ai_throttle_profile_value_009d6b40(chosen);
    return unit_clamp_00415690(snapped, low, high);
}

// ---------------------------------------------------------------------------
// 009F41AB..009F4332, the danger level and its two dwell timers
// ---------------------------------------------------------------------------
void ship_ai_step_danger_009f41ab(ShipAiObstacleState& obs, float target, float dt) noexcept {
    const float current = obs.danger_a84;
    const float rise_step = static_cast<float>(kShipAiDangerRiseRate) * dt;
    const float band = static_cast<float>(kShipAiDangerBand);

    // 009F41D7 FCOMI with ST0 = target and ST1 = current + 0.05, then JBE.
    if (target > current + band) {
        // Rising. 009F41ED: the rise timer fills at 2/s and the fall timer
        // drains at 1/s; the step only runs once the rise timer is armed.
        if (obs.danger_rise_a88 < kShipAiDangerArmed) {
            obs.danger_rise_a88 += rise_step;
        }
        if (obs.danger_fall_a8c > 0.0f) {
            obs.danger_fall_a8c -= dt;
        }
        if (!(obs.danger_rise_a88 < kShipAiDangerArmed)) { // 009F423A JC skips the step
            obs.danger_a84 = unit_step_towards_0042ac60(current, target, rise_step);
        }
        return;
    }

    // 009F4252 FCOMIP with ST0 = current - 0.05 and ST1 = target, then JBE.
    if (current - band > target) {
        // Falling. 009F4260: the fall timer fills at 1/s, the rise timer
        // drains at 1/s, and the step is dt, not 2 * dt.
        if (obs.danger_fall_a8c < kShipAiDangerArmed) {
            obs.danger_fall_a8c += dt;
        }
        if (obs.danger_rise_a88 > 0.0f) {
            obs.danger_rise_a88 -= dt;
        }
        if (!(obs.danger_fall_a8c < kShipAiDangerArmed)) { // 009F42B3 JC skips the step
            obs.danger_a84 = unit_step_towards_0042ac60(current, target, dt);
        }
        return;
    }

    // 009F42C8, inside the band: the step always runs, at 2 * dt, and both
    // timers drain.
    obs.danger_a84 = unit_step_towards_0042ac60(current, target, rise_step);
    if (obs.danger_fall_a8c > 0.0f) {
        obs.danger_fall_a8c -= dt;
    }
    if (obs.danger_rise_a88 > 0.0f) {
        obs.danger_rise_a88 -= dt;
    }
}

// ---------------------------------------------------------------------------
// 009EC7C0 BSP_UnitBot_ComputeThrottleCeiling
// ---------------------------------------------------------------------------
float ship_ai_throttle_ceiling_009ec7c0(const ShipAiAutoThrustSettings& settings,
                                        const ShipAiThrottleCeilingInputs& inputs,
                                        bool committed_ahead_364,
                                        float heading_error, float danger, float cap) noexcept {
    // 009EC7ED and 009EC7F2: the danger level widens the heading error by up to
    // Navigator.AutoThrust.HdgDiffDangerMul.
    const float danger_mul =
        clamped_interpolate_00419010(0.0f, 1.0f, 1.0f, settings.hdg_diff_danger_mul, danger);
    const float error = danger_mul * heading_error;

    // 009EC7FA and 009EC862: the magnitude, taken twice as 0.0f - error, which
    // keeps a negative zero negative.
    const float magnitude = (error > 0.0f) ? error : (-0.0f - error);

    // 009EC859 and 009EC8C1: two falling ramps of the same magnitude, one over
    // the _Slow pair and one over the _Fast pair.
    const float slow_ramp = clamped_interpolate_00419010(settings.hdg_diff_value_min_slow, 1.0f,
                                                         settings.hdg_diff_value_max_slow, 0.0f,
                                                         magnitude);
    const float fast_ramp = clamped_interpolate_00419010(settings.hdg_diff_value_min_fast, 1.0f,
                                                         settings.hdg_diff_value_max_fast, 0.0f,
                                                         magnitude);

    // 009EC8F7 and 009EC92D: each ramp becomes a throttle between the caller's
    // cap (ramp 1.0) and the matching ThrustMin (ramp 0.0).
    const float slow_thrust =
        clamped_interpolate_00419010(1.0f, cap, 0.0f, settings.thrust_min_slow, slow_ramp);
    const float fast_thrust =
        clamped_interpolate_00419010(1.0f, cap, 0.0f, settings.thrust_min_fast, fast_ramp);

    // 009EC93C..009EC98A: the live ring throttle picks between them. Above it
    // the slow branch wins; at or below it the fast branch is allowed to pull
    // the ceiling down to the live value but no further.
    const float live = (inputs.live_throttle > 0.0f) ? inputs.live_throttle
                                                     : (-0.0f - inputs.live_throttle);
    float ceiling = slow_thrust;
    if (!(slow_thrust > live)) {
        ceiling = (fast_thrust < live) ? fast_thrust : live;
    }

    // 009EC98A..009EC9C7: the cruise speed setting, when enabled, is a hard
    // ceiling. docs/CRUISE_COMMAND.md has the same division at 009E12BD with
    // 0080FC30 in place of the cached blk+3C4h.
    if (inputs.commanded_speed_enabled) {
        const float commanded = inputs.commanded_speed / inputs.reference_speed;
        if (commanded < ceiling) {
            ceiling = commanded;
        }
    }

    // 009EC9CD..009EC9F9: astern, the whole ceiling is negated and floored.
    if (!committed_ahead_364) {
        const float astern = -ceiling;
        return (astern < kShipAiAsternThrottleFloor) ? kShipAiAsternThrottleFloor : astern;
    }
    return ceiling;
}

// ---------------------------------------------------------------------------
// 009D8B90, the neighbour's closing speed
// ---------------------------------------------------------------------------
float ship_ai_neighbour_closing_speed_009d8b90(const ShipAiNeighbourRecord& neighbour,
                                               float our_x, float our_z, int pass_side,
                                               float neighbour_speed) noexcept {
    // 009D8B96..009D8BB5: three gates, any of which returns 0.0f at 009D8C49.
    if (neighbour.owner == nullptr || neighbour.owner->gone_5e || neighbour.disabled_68) {
        return 0.0f;
    }

    // 009D8BDC..009D8C01: the dot product of (our position - its position)
    // with its forward vector, in the x/z plane.
    const float dot = (our_x - neighbour.x) * neighbour.forward_x
                      + (our_z - neighbour.z) * neighbour.forward_z;

    // 009D8C05..009D8C38: ahead of it, side 1 flips the sign; behind it, side 2
    // does. The flip is 0.0f - speed (00D7A208), so a zero stays signed.
    if (dot > 0.0f) {
        return (pass_side == 1) ? (-0.0f - neighbour_speed) : neighbour_speed;
    }
    return (pass_side == 2) ? (-0.0f - neighbour_speed) : neighbour_speed;
}

// ---------------------------------------------------------------------------
// The sector index
// ---------------------------------------------------------------------------
int ship_ai_rudder_bucket_009f454c(float rudder, bool speed_above_astern_floor) noexcept {
    // 009F4554..009F4592. Every test is COMISS followed by JBE, so an
    // unordered compare falls through to the last bucket.
    int bucket = 4;
    if (rudder > kShipAiRudderBucketHard) {
        bucket = 0;
    } else if (rudder > kShipAiRudderBucketSoft) {
        bucket = 1;
    } else if (rudder > kShipAiRudderBucketSoftNeg) {
        bucket = 2;
    } else if (rudder > kShipAiRudderBucketHardNeg) {
        bucket = 3;
    }
    // 009F4594: the hull moving astern mirrors the bucket.
    if (!speed_above_astern_floor) {
        bucket = 4 - bucket;
    }
    return bucket;
}

int ship_ai_sector_index_009f45a4(int direction_index, int bucket) noexcept {
    // 009F45A8 LEA EDX,[EDX+EDX*2]; ADD EDX,EDX; LEA EAX,[EDX+ECX]; IMUL 0x2C.
    return direction_index * kShipAiObstacleGroupSize + bucket;
}

// ---------------------------------------------------------------------------
// 009F40CA..009F4166, the four booleans
// ---------------------------------------------------------------------------
ShipAiObstacleFlags ship_ai_obstacle_flags_009f40ca(const ShipAiControlBlock& blk,
                                                    const ShipAiObstacleState& obs,
                                                    float body_axis_speed) noexcept {
    ShipAiObstacleFlags flags{};

    // 009F403D and 009F4057: the two speed gates, both doubles.
    flags.speed_above_astern_floor =
        static_cast<double>(body_axis_speed) > -static_cast<double>(kShipAiRudderSpeedFloor);
    flags.speed_below_ahead_floor =
        static_cast<double>(body_axis_speed) < static_cast<double>(kShipAiRudderSpeedFloor);

    const ShipAiThrottleDirection latch = blk.direction; // blk+35Ch, 009F40C0
    const bool committed_ahead = blk.yaw_rate_subtracts_364; // blk+364h

    // 009F40CA..009F40EA.
    flags.hull_with_latch =
        (latch == ShipAiThrottleDirection::Stopped)
        || (latch == ShipAiThrottleDirection::Ahead && flags.speed_above_astern_floor)
        || (latch == ShipAiThrottleDirection::Astern && flags.speed_below_ahead_floor);

    // 009F40EF..009F4116.
    flags.committed_with_latch = (latch == ShipAiThrottleDirection::Stopped)
                                 || (latch == ShipAiThrottleDirection::Ahead && committed_ahead)
                                 || (latch == ShipAiThrottleDirection::Astern && !committed_ahead);

    // 009F411B..009F4137.
    flags.moving_or_stopped =
        (latch == ShipAiThrottleDirection::Stopped)
        || (committed_ahead ? flags.speed_above_astern_floor : flags.speed_below_ahead_floor);

    // 009F413C..009F4166, two byte compares of blk+36Ch against blk+364h.
    const bool escape_matches = (obs.escape_enabled_36c == committed_ahead);
    flags.direction_mismatch =
        (latch == ShipAiThrottleDirection::Ahead && !escape_matches)
        || (latch == ShipAiThrottleDirection::Astern && escape_matches);

    return flags;
}

// ---------------------------------------------------------------------------
// 009F40CA..009F4B98, the middle
// ---------------------------------------------------------------------------
void ship_ai_drive_order_ring_middle_009f40ca(ShipAiControlBlock& blk, ShipAiObstacleState& obs,
                                              const ShipAiObstacleFrame& frame,
                                              const ShipAiAutoThrustSettings& settings,
                                              const ShipAiThrottleCeilingInputs& ceiling_inputs,
                                              ShipAiObstacleHost& host) {
    const ShipAiObstacleFlags flags =
        ship_ai_obstacle_flags_009f40ca(blk, obs, frame.body_axis_speed);

    // -----------------------------------------------------------------------
    // 009F4168..009F4392: the danger level
    // -----------------------------------------------------------------------
    // 009F416E FLD [ESI+37Ch]; 009F4174 FDIV [EAX+9CCh]: the clearance in half
    // widths. 009F4189..009F41A2: InterpolateClamped(1.0, 1.0, 4.0, 0.0, ratio),
    // so one half width of clearance is full danger and four is none.
    const float clearance_ratio = obs.clearance_37c / frame.unit_half_width_9cc;
    const float danger_target =
        clamped_interpolate_00419010(kShipAiDangerClearanceMin, 1.0f, kShipAiDangerClearanceMax,
                                     0.0f, clearance_ratio);
    ship_ai_step_danger_009f41ab(obs, danger_target, frame.dt);

    // 009F4332..009F4392: any danger at all raises the turn-assist load latch.
    if (obs.danger_a84 > kShipAiDangerLoadFloor) {
        float load = obs.danger_a84 + obs.danger_a84; // 009F434A FADD ST0,ST0
        if (static_cast<double>(load) > kShipAiOrderSlewRate) { // 009F435A, the 1.5 at 00CE3D78
            load = kShipAiLoadLatchHigh;
        }
        host.raise_turn_assist_load_102c(load);
    }

    // 009F4394..009F43C4: the bearing the obstacle arm measures against. The
    // heading already carries the 009F409E half turn when the latch is Astern;
    // this adds a second one when the hull is moving against the latch.
    const float avoid_reference =
        flags.hull_with_latch ? frame.heading
                              : wrapped_angle_add_00438aa0(frame.heading, kShipAiObstacleHalfTurn);

    // -----------------------------------------------------------------------
    // 009F43C6..009F44B2: the throttle
    // -----------------------------------------------------------------------
    if (blk.direction == ShipAiThrottleDirection::Stopped) {
        blk.desired_throttle = 0.0f; // 009F43D2
    } else if (blk.throttle_hold_1c8 != 0) {
        // 009F43EC..009F4406: the cap handed to the ceiling is
        // max(1.0f, blk+344h), and blk+344h is what 009F4DA0 wrote into
        // brain+34Ch one chain slot earlier.
        const float cap = max_by_ref_00415550(1.0f, obs.throttle_limit_344);
        const float ceiling = ship_ai_throttle_ceiling_009ec7c0(
            settings, ceiling_inputs, blk.yaw_rate_subtracts_364, frame.heading_error,
            obs.danger_a84, cap);
        blk.desired_throttle = ceiling; // 009F4439, an FST that keeps the value

        // 009F443F..009F448A: when the escape flag disagrees with the committed
        // direction the throttle is limited by blk+344h on the committed side.
        if (flags.direction_mismatch) {
            const float limit = obs.throttle_limit_344;
            if (blk.yaw_rate_subtracts_364) {
                if (limit < ceiling) {
                    blk.desired_throttle = limit; // 009F4462
                }
            } else if (ceiling < -limit) {
                blk.desired_throttle = -limit; // 009F4482
            }
        }
    }

    // 009F448E..009F44B2: the profile at blk+4h snaps the throttle, with a
    // window wider than the throttle's own range, so only the profile moves it.
    blk.desired_throttle =
        ship_ai_apply_throttle_profile_009d6b40(obs.profile, blk.desired_throttle, -1.5f, 1.5f);

    // -----------------------------------------------------------------------
    // 009F44B3..009F4500: the sector group and the rudder law
    // -----------------------------------------------------------------------
    int direction_index = 0;
    if (!flags.speed_above_astern_floor) {
        direction_index = 1; // 009F44DC, the hull is making way astern
    } else if (!flags.speed_below_ahead_floor) {
        direction_index = 0; // 009F44D2, ahead
    } else {
        direction_index = blk.yaw_rate_subtracts_364 ? 0 : 1; // 009F44C3, stopped
    }

    if (blk.mode != ShipAiSteeringMode::Rudder) { // 009F44E4, blk+1C4h
        blk.desired_rudder = host.rudder_law_009da250(frame.heading_error); // 009F44F7
    }

    // -----------------------------------------------------------------------
    // 009F4502..009F487C: the obstacle sectors
    // -----------------------------------------------------------------------
    // 009F4511..009F4544: the rudder limit blk+348h, the other field 009F4DA0
    // wrote (brain+350h). The low bound is 0.0f - blk+348h (00D7A208).
    if (flags.hull_with_latch && flags.committed_with_latch && obs.published_33c < 0.0f) {
        blk.desired_rudder = unit_clamp_00415690(blk.desired_rudder,
                                                 -0.0f - obs.rudder_limit_348,
                                                 obs.rudder_limit_348);
    }

    const int bucket = ship_ai_rudder_bucket_009f454c(blk.desired_rudder,
                                                      flags.speed_above_astern_floor);
    const int index = ship_ai_sector_index_009f45a4(direction_index, bucket);
    // The bucket is 0..4 and the group is six wide, so index + 1 never leaves
    // the array; the native reads blk + index * 2Ch + 848h with the same bound.
    const ShipAiObstacleSector& here = obs.sector[static_cast<std::size_t>(index)];
    const ShipAiObstacleSector& next = obs.sector[static_cast<std::size_t>(index + 1)];

    if (here.blocked || next.blocked) { // 009F45B3 and 009F45C0
        if (kShipAiObstacleHold > blk.clamp_354) {
            blk.clamp_354 = kShipAiObstacleHold; // 009F45DE
        }
        host.raise_secondary_load_1034(kShipAiLoadLatchHigh); // 009F4606
        host.raise_turn_assist_load_102c(kShipAiLoadLatchHigh); // 009F462A

        // 009F4632..009F464E: of the two adjacent sectors, the one that is
        // actually blocked is chosen, with the tie going to the inner sector
        // for buckets 0..2 and to the outer one above.
        int chosen_bucket = bucket;
        if (bucket <= 2) {
            if (!here.blocked) {
                ++chosen_bucket;
            }
        } else if (next.blocked) {
            ++chosen_bucket;
        }
        const ShipAiObstacleSector& sector = obs.sector[static_cast<std::size_t>(
            ship_ai_sector_index_009f45a4(direction_index, chosen_bucket))];

        // 009F4653..009F46A0: the rudder is taken from the sector's avoidance
        // bearing instead of the goal heading.
        const float avoid_error =
            wrapped_angle_subtract_00438b10(sector.avoid_bearing, avoid_reference);
        blk.desired_rudder = host.rudder_law_009da250(avoid_error);

        if (blk.direction == ShipAiThrottleDirection::Stopped) { // 009F4699
            // 009F46A8..009F4720: with no latch, the throttle band is simply
            // reversed against the way the hull is moving.
            if (flags.speed_above_astern_floor) {
                if (!flags.speed_below_ahead_floor) {
                    blk.desired_throttle =
                        unit_clamp_00415690(blk.desired_throttle, -1.0f, 0.0f); // 009F46E0
                }
            } else if (flags.speed_below_ahead_floor) {
                blk.desired_throttle =
                    unit_clamp_00415690(blk.desired_throttle, 0.0f, 1.0f); // 009F471B
            }
        } else {
            // 009F4725..009F47EE: the astern latch. It is armed either because
            // the neighbour has picked the other passing side, or because it is
            // closing faster than three quarters of this hull's reference speed.
            bool back_off = false;
            ShipAiNeighbourRecord* blocker = sector.blocker;
            if (flags.hull_with_latch && flags.committed_with_latch && blocker != nullptr
                && blocker->owner != nullptr && !blocker->owner->gone_5e) {
                if (blocker->pass_side_88 != sector.pass_side) {
                    back_off = true; // 009F475B, the sides disagree
                } else if (blocker->owner->avoidance_184) { // 009F4768
                    const float neighbour_speed = host.neighbour_body_axis_speed_0092d730(*blocker);
                    const float closing = ship_ai_neighbour_closing_speed_009d8b90(
                        *blocker, obs.position_x, obs.position_z, sector.pass_side,
                        neighbour_speed);
                    const float threshold = static_cast<float>(
                        static_cast<double>(obs.reference_speed_3c4) * kShipAiNeighbourSpeedFraction);
                    if (closing > threshold) { // 009F4799
                        back_off = true;
                    }
                }
            }

            if (back_off) {
                obs.backoff_timer_380 = kShipAiObstacleBackoffSeconds; // 009F47A7
                // 009F47AF..009F47EE: a blocker that is itself nearly stopped
                // adds the difference to the stall accumulator.
                const float other = std::fabs(host.neighbour_body_axis_speed_0092d730(*blocker));
                if (kShipAiStallNeighbourSpeed > other) {
                    obs.stall_time_384 += kShipAiStallNeighbourSpeed - other;
                }
            }

            if (obs.backoff_timer_380 > 0.0f) { // 009F47FC
                // 009F4805..009F487B: while the latch runs the profile window is
                // [-1, 0]: astern only. This is the one thing that makes an AI
                // ship back off an obstacle.
                blk.desired_throttle = ship_ai_apply_throttle_profile_009d6b40(
                    obs.profile, blk.desired_throttle, -1.0f, 0.0f);
            } else {
                // 009F4820..009F4878: otherwise a fresh ceiling on the sector's
                // own heading error, with no danger and no cap, and a profile
                // window that keeps half a throttle of room on the other side.
                const float ceiling =
                    ship_ai_throttle_ceiling_009ec7c0(settings, ceiling_inputs,
                                                      blk.yaw_rate_subtracts_364, avoid_error,
                                                      1.0f, 1.0f);
                blk.desired_throttle = ceiling; // 009F4849, an FST
                const float low = (ceiling < 0.0f) ? ceiling : -0.5f;
                const float high = (ceiling < 0.0f) ? 0.5f : ceiling;
                blk.desired_throttle =
                    ship_ai_apply_throttle_profile_009d6b40(obs.profile, blk.desired_throttle, low,
                                                            high);
            }
        }
    }

    // -----------------------------------------------------------------------
    // 009F4880..009F4B98: the escape manoeuvre
    // -----------------------------------------------------------------------
    if (blk.direction == ShipAiThrottleDirection::Stopped) { // 009F4880
        return;
    }
    // 009F488D and 009F489E: the section runs while the astern latch is not
    // armed, or once the stall accumulator has passed ten.
    if (!(obs.backoff_timer_380 < 0.0f) && !(obs.stall_time_384 > kShipAiStallThreshold)) {
        return;
    }

    // 009F48B5..009F48D8: are all four inner sectors of each group blocked?
    bool all_blocked_ahead = true;
    bool all_blocked_astern = true;
    for (int i = 0; i < 4; ++i) {
        if (!obs.sector[static_cast<std::size_t>(1 + i)].blocked) {
            all_blocked_ahead = false;
        }
        if (!obs.sector[static_cast<std::size_t>(7 + i)].blocked) {
            all_blocked_astern = false;
        }
    }
    // 009F48DA: the group that matters is the one the hull is moving in.
    const bool all_blocked = (direction_index == 0) ? all_blocked_ahead : all_blocked_astern;

    int request = 0; // EDI, 009F48AF
    if (all_blocked) {
        host.raise_secondary_load_1034(kShipAiLoadLatchHigh); // 009F4912
        if (direction_index == 0) {
            if (blk.desired_throttle > 0.0f) {
                blk.desired_throttle = 0.0f; // 009F492B
            }
            if (blk.yaw_rate_subtracts_364
                && static_cast<double>(frame.body_axis_speed)
                       < static_cast<double>(kShipAiRudderSpeedFloor)) {
                request = 1; // 009F4972
            }
        } else {
            if (blk.desired_throttle < 0.0f) {
                blk.desired_throttle = 0.0f; // 009F4951
            }
            if (!blk.yaw_rate_subtracts_364
                && static_cast<double>(frame.body_axis_speed)
                       > -static_cast<double>(kShipAiRudderSpeedFloor)) {
                request = 1;
            }
        }
    }

    // 009F4977..009F4A57: which of the four requests, if any, is raised. Only
    // whether the value is zero is ever read again.
    if (!flags.committed_with_latch) {
        if (!obs.escape_enabled_36c && obs.escape_mode_370 == 0 && blk.throttle_hold_1c8 != 0) {
            const bool side = blk.yaw_rate_subtracts_364 ? all_blocked_astern : all_blocked_ahead;
            if (!side) {
                request = 2; // 009F49C8
            }
        }
    } else if (flags.hull_with_latch && obs.stall_time_384 >= kShipAiStallThreshold) {
        if (flags.speed_above_astern_floor && flags.speed_below_ahead_floor) {
            request = 3; // 009F49F4, the hull has stopped and is still blocked
        }
    } else if (flags.moving_or_stopped) {
        if (obs.escape_mode_370 == 0) {
            if (request == 0) {
                obs.escape_latch_378 = 0; // 009F4A10
            }
        } else if (obs.escape_latch_378 == 0) {
            if (obs.escape_mode_370 == 1 || obs.escape_mode_370 == 3
                || obs.stall_time_384 > kShipAiStallThreshold) {
                request = 4;                                          // 009F4A44
                host.raise_turn_assist_load_102c(kShipAiLoadLatchLow); // 009F4A51
            }
        }
    }

    // 009F4A59..009F4A9A: the gate on the flip.
    bool run_escape = false;
    if (blk.direction == ShipAiThrottleDirection::Stopped || !obs.escape_enabled_36c
        || !flags.committed_with_latch || all_blocked) {
        run_escape = (request != 0);
    } else if (request != 0) {
        run_escape = true;
    } else {
        // 009F4A7D: with no request the flip runs only when the latch does not
        // already record this direction.
        run_escape = blk.yaw_rate_subtracts_364 ? (obs.escape_latch_378 != 2)
                                                : (obs.escape_latch_378 != 1);
    }
    if (!run_escape) {
        return;
    }

    // 009F4AA0..009F4B95: stop the committed direction, and once the hull has
    // actually stopped moving that way and the dwell timer has run out, commit
    // to the other one.
    host.raise_secondary_load_1034(kShipAiLoadLatchLow); // 009F4AB4
    if (blk.yaw_rate_subtracts_364) {
        if (blk.desired_throttle > 0.0f) {
            blk.desired_throttle = 0.0f; // 009F4AD2
        }
        if (static_cast<double>(frame.body_axis_speed)
            < static_cast<double>(kShipAiRudderSpeedFloor)) { // 009F4AE4
            if (blk.timer_368 < 0.0f) {                       // 009F4AEE
                obs.stall_time_384 = 0.0f;
                blk.yaw_rate_subtracts_364 = false;
                blk.direction_value_374 = -1.0f;
                blk.timer_368 = kShipAiObstacleBackoffSeconds;
            }
            obs.escape_latch_378 = 1; // 009F4B26
        }
    } else {
        if (blk.desired_throttle < 0.0f) {
            blk.desired_throttle = 0.0f; // 009F4B3B
        }
        if (static_cast<double>(frame.body_axis_speed)
            > -static_cast<double>(kShipAiRudderSpeedFloor)) { // 009F4B4D
            if (blk.timer_368 < 0.0f) {                        // 009F4B53
                obs.stall_time_384 = 0.0f;
                blk.yaw_rate_subtracts_364 = true;
                blk.direction_value_374 = -1.0f;
                blk.timer_368 = kShipAiObstacleBackoffSeconds;
            }
            obs.escape_latch_378 = 2; // 009F4B8B
        }
    }
}

} // namespace bsp
