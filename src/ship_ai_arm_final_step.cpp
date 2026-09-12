// 009DE5B0, the unconditional last step of every arm of 009ED6B0.
// See include/bsp/ship_ai_arm_final_step.hpp for the ABI, the evidence and the
// field offsets.  Every comparison below is spelled the way the listing spells
// it, so an unordered (NaN) operand takes the same branch it takes natively.
#include "bsp/ship_ai_arm_final_step.hpp"

#include <cmath>

#include "bsp/geometry_helpers.hpp"      // heading_angle_00414eb0
#include "bsp/ship_ai_throttle_ring.hpp" // clamp_float_by_ref_00415620, heading_to_direction_006bc0c0
#include "bsp/vector_helpers.hpp"        // length_2d_00414c60

namespace bsp {
namespace {

// 00415550, `__fastcall(const float* a, const float* b)`, RET 0, body
// 00415550-0041558D: FLD b, FLD a, FCOMIP a/b, JBE returns b.
float max_float_by_ref_00415550(float a, float b) noexcept {
    return (a > b) ? a : b;
}

// 00415510, body 00415510-0041554D: FLD a, FLD b, FCOMIP b/a, JBE returns b.
float min_float_by_ref_00415510(float a, float b) noexcept {
    return (b > a) ? a : b;
}

// 009DE5C7..009DE5D4: the sign bit is cleared with AND 7FFFFFFFh on the stored
// float, not with an FPU instruction, so a NaN keeps its payload.
float absolute_float(float value) noexcept {
    return std::fabs(value);
}

} // namespace

// ---------------------------------------------------------------------------
// 009DE6DB..009DE8ED, the avoid-zone escape blend.
// ---------------------------------------------------------------------------
ShipAiArmFinalEscapeTurn ship_ai_arm_final_escape_turn_009de6db(
    float heading, float heading_target_324, float remaining_330,
    float escape_distance_14c, float look_ahead_max_3c8, float hull_radius_9c8,
    const std::array<float, 2>& escape_direction_150, bool reverse_sense) noexcept {
    ShipAiArmFinalEscapeTurn out{};

    // 009DE6E8..009DE717: the heading error, made absolute by AND 7FFFFFFFh.
    const float error =
        absolute_float(wrapped_angle_subtract_00438b10(heading, heading_target_324));

    // 009DE71B..009DE742.  Full weight inside 45 degrees, none past 80.
    const float weight = clamped_interpolate_00419010(
        kShipAiArmFinalEscapeErrorFull, 1.0f, kShipAiArmFinalEscapeErrorNone, 0.0f, error);

    // 009DE74B..009DE755: FLDZ, FLD weight, FCOMI, JBE leaves the branch.
    if (!(weight > 0.0f)) {
        return out;
    }
    out.weight_positive = true;
    // 009DE75B..009DE76C: COMISS 3.0f against blk+354h, JBE skips the store.
    out.raise_clearance_hold = true;

    // 009DE774..009DE7A1.  The remaining path length measured in hull radii,
    // with a floor of 100.0 under the radius.  The dividend is spilled as a
    // double at 009DE788 and the quotient rounded to float at 009DE7A8.
    const float radius =
        max_float_by_ref_00415550(kShipAiArmFinalHullFloor, hull_radius_9c8);
    const float path_lengths =
        static_cast<float>(static_cast<double>(remaining_330) / static_cast<double>(radius));

    // 009DE7A5..009DE7D8: nothing under three hull radii of path left, full
    // effect past five, multiplied into the heading-error weight.
    const float near_goal = static_cast<float>(
        clamped_interpolate_00419010(kShipAiArmFinalClearanceHold, 0.0f,
                                     kShipAiArmFinalPathLengthsHigh, 1.0f, path_lengths) *
        weight);

    // 009DE7DC..009DE819: how far the ship still has to travel to leave the
    // zone, as a fraction of the look-ahead ceiling blk+3C8h.
    const float strength = static_cast<float>(
        clamped_interpolate_00419010(0.0f, kShipAiArmFinalEscapeFloorWeight,
                                     look_ahead_max_3c8, 1.0f, escape_distance_14c) *
        near_goal);

    // 009DE821..009DE82B.
    if (!(strength > 0.0f)) {
        return out;
    }
    out.applied = true;

    // 009DE831..009DE853, the inlined 009D4FB0 load latch.
    out.load_request =
        static_cast<float>(kShipAiArmFinalEscapeLoadScale * static_cast<double>(strength));

    // 009DE85B: at most 30 degrees of escape steering in one tick.
    const float max_turn =
        static_cast<float>(static_cast<double>(strength) * kShipAiArmFinalEscapeMaxTurn);

    // 009DE861..009DE888: the bearing of the unit escape direction, relative to
    // the heading target the arm just published.
    float offset = wrapped_angle_subtract_00438b10(
        heading_angle_00414eb0(escape_direction_150), heading_target_324);
    // 009DE88D..009DE8A6: negated while the latch disagrees with the direction
    // the hull is actually travelling.
    if (reverse_sense) {
        offset = kShipAiArmFinalNegativeZero - offset;
    }

    // 009DE8AC..009DE8CD.
    out.turn = clamp_float_by_ref_00415620(
        offset, kShipAiArmFinalNegativeZero - max_turn, max_turn);
    return out;
}

// ---------------------------------------------------------------------------
// 009DE8F1..009DE96C, the avoidance-vector override.
// ---------------------------------------------------------------------------
ShipAiArmFinalOverride ship_ai_arm_final_avoidance_override_009de8f1(
    bool gate, float clearance_hold_354, float avoidance_x_34c, float avoidance_z_350,
    ShipAiThrottleDirection direction_35c) noexcept {
    ShipAiArmFinalOverride out{};
    // 009DE8F8: 009DA1D0 answered no.
    if (!gate) {
        return out;
    }
    // 009DE8FF: XORPS/COMISS 0.0f against blk+354h with JBE, so the hold has to
    // have counted past zero.  009E0591 is what counts it down.
    if (!(0.0f > clearance_hold_354)) {
        return out;
    }
    // 009DE908..009DE930: float(x*x + z*z) against the double 1.0e-4.
    const float square = static_cast<float>(static_cast<double>(avoidance_x_34c) * avoidance_x_34c +
                                            static_cast<double>(avoidance_z_350) * avoidance_z_350);
    if (!(static_cast<double>(square) > kShipAiArmFinalAvoidanceMinSquare)) {
        return out;
    }

    // 009DE932..009DE962: the heading target is replaced outright, not nudged.
    out.applied = true;
    out.heading_target = heading_angle_00414eb0({avoidance_x_34c, avoidance_z_350});
    if (direction_35c == ShipAiThrottleDirection::Astern) {
        out.heading_target =
            wrapped_angle_add_00438aa0(out.heading_target, kShipAiArmFinalHalfTurn);
    }
    return out;
}

// ---------------------------------------------------------------------------
// 009DEBB9..009DEE07, the turn a finished separation vector asks for.
// ---------------------------------------------------------------------------
ShipAiArmFinalSeparationTurn ship_ai_arm_final_separation_turn_009debb9(
    float separation_x, float separation_z, float heading, float heading_target_324,
    float entry_target, float turn_window_3d0, ShipAiNavTurnSide side_304,
    bool reverse_sense) noexcept {
    ShipAiArmFinalSeparationTurn out{};

    // 009DEBB9..009DEBDB: float(z*z + x*x) against the double 1.0e-10.
    const float square = static_cast<float>(static_cast<double>(separation_z) * separation_z +
                                            static_cast<double>(separation_x) * separation_x);
    if (!(static_cast<double>(square) > kShipAiArmFinalSeparationMinSquare)) {
        return out;
    }

    // 009DEBE1: 00BF7030, the CRT square root, consuming ST0 and rounded to
    // float by the store at 009DEBE6.
    const float length = static_cast<float>(std::sqrt(static_cast<double>(square)));
    // 009DEBF2: COMISS against 1.0f with JBE, so the whole turn is dropped for
    // a vector this short.
    if (!(length > kShipAiArmFinalMinSpeed)) {
        return out;
    }

    float vector_x = separation_x;
    float vector_z = separation_z;
    // 009DEC05..009DEC2F: scale the vector down to at most 100.0 long.  The
    // scale is length/100.0 and each component is divided by it separately.
    if (static_cast<double>(length) > kShipAiArmFinalSeparationMaxLength) {
        const float scale = static_cast<float>(static_cast<double>(length) /
                                               kShipAiArmFinalSeparationMaxLength);
        vector_x = vector_x / scale;
        vector_z = vector_z / scale;
    }

    // 009DEC39..009DEC6C: 00414EB0's body, inlined.  atan2 of the pair, then
    // pi/2 minus it, then one full turn added while it is below zero.
    const float bearing = heading_angle_00414eb0({vector_x, vector_z});

    // 009DEC70..009DEC83.
    float error = wrapped_angle_subtract_00438b10(bearing, heading);
    // 009DEC87..009DECB3: a bearing abaft the beam is reflected onto the near
    // side, so the ship always turns the short way away from the crowd.
    if (static_cast<double>(error) > kShipAiArmFinalQuarterTurn) {
        error = static_cast<float>(kShipAiArmFinalHalfTurnDouble - static_cast<double>(error));
    } else if (kShipAiArmFinalQuarterTurnNegative > static_cast<double>(error)) {
        error = static_cast<float>(kShipAiArmFinalHalfTurnNegative - static_cast<double>(error));
    }
    // 009DECB7: the gain.
    float turn = static_cast<float>(static_cast<double>(error) /
                                    kShipAiArmFinalSeparationGainDivisor);

    // 009DECC1..009DED09: clamped to +/- 1.8 times the arm's turn window.
    const float limit = static_cast<float>(static_cast<double>(turn_window_3d0) *
                                           kShipAiArmFinalSeparationTurnScale);
    const float negative_limit = -limit;
    if (negative_limit > turn) {
        turn = negative_limit;
    } else if (turn > limit) {
        turn = limit;
    }
    // 009DED09..009DED25.
    if (reverse_sense) {
        turn = kShipAiArmFinalNegativeZero - turn;
    }

    // 009DED2B..009DEDE4: the path point's turn side bounds the separation
    // turn by the room left between the heading target as it stood on entry
    // and the heading target as it stands now.
    if (side_304 == ShipAiNavTurnSide::ClampNonPositive) {
        // 009DED36: only a positive turn is bounded.
        if (turn > kShipAiArmFinalZero) {
            float room = wrapped_angle_subtract_00438b10(entry_target, heading_target_324);
            // 009DED60..009DED6F: a negative room becomes zero.
            if (!(0.0f <= room)) {
                room = 0.0f;
            }
            // 009DED77..009DED8B: JA skips the store, so an unordered compare
            // takes the room.
            if (!(room > turn)) {
                turn = room;
            }
        }
    } else if (side_304 == ShipAiNavTurnSide::ClampNonNegative) {
        // 009DED98: only a negative turn is bounded.
        if (0.0f > turn) {
            const float room = wrapped_angle_subtract_00438b10(entry_target, heading_target_324);
            const float floor_value = min_float_by_ref_00415510(0.0f, room);
            turn = max_float_by_ref_00415550(turn, floor_value);
        }
    }

    out.applied = true;
    out.turn = turn;
    return out;
}

// ---------------------------------------------------------------------------
// 009DEE7D..009DEEE3, the corridor range.
// ---------------------------------------------------------------------------
float ship_ai_arm_final_query_range_009dee7d(float extent_a, float extent_b,
                                             float hull_radius_9c8,
                                             float distance_32c) noexcept {
    // 009DEE55..009DEE71: the wider of the two controller extents.
    const float widest = (extent_b > extent_a) ? extent_b : extent_a;
    // 009DEE7D..009DEE87.
    float range =
        static_cast<float>(static_cast<double>(widest) * kShipAiArmFinalCorridorScale);
    // 009DEE8B..009DEEB9: floored at eight hull radii.  JA skips the store.
    const float floor_value = static_cast<float>(kShipAiArmFinalCorridorHullFloor *
                                                 static_cast<double>(hull_radius_9c8));
    if (!(range > floor_value)) {
        range = floor_value;
    }
    // 009DEEBF..009DEEDD: capped at one hull radius plus the distance to the
    // path point, so the query never looks past where the ship is going.  The
    // FADD at 009DEEBF is on the hull radius still in ST0, not on the floor.
    const float cap = hull_radius_9c8 + distance_32c;
    if (!(cap > range)) {
        range = cap;
    }
    return range;
}

// ---------------------------------------------------------------------------
// 009DE5B0 whole.
// ---------------------------------------------------------------------------
ShipAiArmFinalStepResult ship_ai_arm_final_step_009de5b0(
    ShipAiControlBlock& blk, const ShipAiNavState& nav,
    const ShipAiArmFinalStepState& state, const ShipAiArmFinalStepTuning& tuning,
    ShipAiSectorFreeBearingQuery& query, ShipAiArmFinalStepHost& host) {
    ShipAiArmFinalStepResult result{};

    // 009DE5B6..009DE5E5.  Nothing at all happens to a ship that is not moving.
    if (!(absolute_float(host.body_axis_speed_0092d730()) > kShipAiArmFinalMinSpeed)) {
        return result;
    }
    result.ran = true;

    // 009DE5EB..009DE678, the query block and the two values kept from entry.
    const float entry_target = blk.heading_target_324;               // 009DE5F1
    const float look_ahead = static_cast<float>(static_cast<double>(state.look_ahead_318) *
                                                kShipAiArmFinalLookAheadShare); // 009DE5EB
    int area_key = state.layer_key_30c;                              // 009DE5FF, query+20h
    query.origin_x = state.pose_x_184;                               // 009DE60B
    query.origin_z = state.pose_z_188;                               // 009DE623
    // 009DE633..009DE662.
    query.range = (look_ahead > blk.distance_32c) ? blk.distance_32c : look_ahead;
    query.width_b = 0.0f;                                            // 009DE66C
    query.width_a = 0.0f;                                            // 009DE672
    bool allow_free_bearing = true;                                  // 009DE678

    // 009DE67D..009DE6AB.  The latch is compared against the sign of the speed
    // the hull actually has, not against the speed it was commanded.
    const float signed_speed = host.body_axis_speed_0092d730();
    const bool reverse_sense = (signed_speed < 0.0f)
                                   ? (blk.direction == ShipAiThrottleDirection::Ahead)
                                   : (blk.direction == ShipAiThrottleDirection::Astern);

    // 009DE6A0..009DE6D7.  An astern latch turns the unit's own heading round
    // so every angle below is measured from the direction of travel.
    float heading = host.unit_heading_vtable_0050();
    if (blk.direction == ShipAiThrottleDirection::Astern) {
        heading = wrapped_angle_add_00438aa0(heading, kShipAiArmFinalHalfTurn);
    }

    // 009DE6DB..009DE8ED.
    if (state.inside_avoid_zone_160) {
        result.inside_avoid_zone = true;
        // 009DE6F5: the same byte that gates the blend suppresses the free
        // bearing query for the rest of the tick.
        allow_free_bearing = false;
        const ShipAiArmFinalEscapeTurn escape = ship_ai_arm_final_escape_turn_009de6db(
            heading, blk.heading_target_324, blk.distance_330, state.escape_distance_14c,
            nav.look_ahead_max_3c8, tuning.hull_radius_9c8, state.escape_direction_150,
            reverse_sense);
        if (escape.raise_clearance_hold && kShipAiArmFinalClearanceHold > blk.clamp_354) {
            blk.clamp_354 = kShipAiArmFinalClearanceHold;
        }
        if (escape.applied) {
            host.raise_turn_assist_load_009de853(escape.load_request);
            blk.heading_target_324 =
                wrapped_angle_add_00438aa0(blk.heading_target_324, escape.turn);
            result.escape_applied = true;
        }
    }

    // 009DE8F1..009DE96C.
    const ShipAiArmFinalOverride override_result = ship_ai_arm_final_avoidance_override_009de8f1(
        host.clearance_gate_009da1d0(), blk.clamp_354, state.avoidance_x_34c,
        state.avoidance_z_350, blk.direction);
    if (override_result.applied) {
        blk.heading_target_324 = override_result.heading_target;
        result.avoidance_override = true;
    }

    // 009DE96C..009DEE07, the traffic separation turn.
    if (state.neighbour_count_604 > 0) {
        float separation_x = 0.0f; // 009DE988
        float separation_z = 0.0f; // 009DE98E

        // 009DE994..009DEBA5: how far ahead of its own pose the ship probes,
        // as a fraction of the way to blk+174h, clamped to +/- one half.
        float fraction = host.body_axis_speed_0092d730() / host.class_reference_speed_500();
        if (kShipAiArmFinalProbeMin > fraction) {
            fraction = kShipAiArmFinalProbeMin;
        } else if (fraction > kShipAiArmFinalProbeMax) {
            fraction = kShipAiArmFinalProbeMax;
        }
        const float probe_x =
            state.pose_x_184 + fraction * (state.goal_x_174 - state.pose_x_184);
        const float probe_z =
            state.pose_z_188 + fraction * (state.goal_z_178 - state.pose_z_188);

        // 009DEA50..009DEBB2.
        for (int index = 0; index < state.neighbour_count_604; ++index) {
            const ShipAiArmFinalNeighbour& node = host.neighbour_608(index);
            if (!node.has_unit) {
                continue; // 009DEA57
            }
            if (!(node.lifetime_78 > kShipAiArmFinalZero)) {
                continue; // 009DEA69
            }
            if (!node.unit_flag_5c || node.unit_flag_5d || node.unit_flag_60 ||
                node.unit_gone_5e) {
                continue; // 009DEA73, 009DEA7D, 009DEA87, 009DEA91
            }
            // 009DEA99..009DEAAE: the two hull radii over 1.5.
            const float reach =
                static_cast<float>((static_cast<double>(node.unit_hull_radius_9c8) +
                                    static_cast<double>(tuning.hull_radius_9c8)) /
                                   kShipAiArmFinalEscapeLoadScale);
            const float offset_x = probe_x - node.centre_x_20; // 009DEAB2
            const float offset_z = probe_z - node.centre_z_24; // 009DEABD
            const float square = static_cast<float>(static_cast<double>(offset_z) * offset_z +
                                                    static_cast<double>(offset_x) * offset_x);
            if (!(square > kShipAiArmFinalSeparationNearSquare)) {
                continue; // 009DEAEE
            }
            if (!(static_cast<double>(reach) * reach > static_cast<double>(square))) {
                continue; // 009DEAFE
            }
            // 009DEB04..009DEB4C: the falloff, divided by the distance so the
            // product below is a unit vector times the weight.
            const float distance = length_2d_00414c60({offset_x, offset_z});
            const float ratio = distance / reach;
            const float weight =
                clamped_interpolate_00419010(kShipAiArmFinalSeparationKnee,
                                             kShipAiArmFinalHullFloor, 1.0f, 0.0f, ratio) /
                distance;
            separation_x += weight * offset_x; // 009DEB5C, 009DEB72
            separation_z += weight * offset_z; // 009DEB66, 009DEB7E
        }

        const ShipAiArmFinalSeparationTurn separation = ship_ai_arm_final_separation_turn_009debb9(
            separation_x, separation_z, heading, blk.heading_target_324, entry_target,
            nav.turn_window_3d0, nav.side_304, reverse_sense);
        if (separation.applied) {
            // 009DEDE8..009DEE01.
            blk.heading_target_324 =
                wrapped_angle_add_00438aa0(blk.heading_target_324, separation.turn);
            result.separation_applied = true;
        }
    }

    // 009DEE0B..009DF10F, the avoid-zone free bearing query.
    if (!allow_free_bearing) {
        return result;
    }

    int searcher = kShipAiArmFinalSearcherCurrent;
    bool direction_written = false;

    if (host.unit_leads_controller_00778890()) { // 009DEE1E
        // 009DEE37..009DEEE3.
        const float extent_a = host.controller_extent_0070d400();
        const float extent_b = host.controller_extent_0070d5d0();
        const float range = ship_ai_arm_final_query_range_009dee7d(
            extent_a, extent_b, tuning.hull_radius_9c8, blk.distance_32c);

        // 009DEEE9..009DEEF2.
        const int area = host.controller_area_key_0070e450();
        if (area != area_key) {
            // 009DEF83..009DF060, the "the controller moved to another area"
            // path.  It rebuilds the origin and the direction, re-reads the
            // area key into the query, and asks the second searcher.
            query.origin_x = state.pose_x_184; // 009DEF92
            query.origin_z = state.pose_z_188; // 009DEFA7
            const std::array<float, 2> direction =
                heading_to_direction_006bc0c0(blk.heading_target_324); // 009DEFAD
            query.direction_x = direction[0];                          // 009DEFC2
            query.direction_z = direction[1];                          // 009DEFCD
            area_key = host.controller_area_key_0070e450();            // 009DEFD3, 009DEFE5
            direction_written = true;
            searcher = kShipAiArmFinalSearcherMoved;
        }

        query.range = range; // 009DEF05 or 009DEFE9
        // 009DEEF8 / 009DEFD8: the two corridor half-widths swap sides when the
        // ship is latched astern, which is what a reversing hull does to port
        // and starboard.
        if (blk.direction == ShipAiThrottleDirection::Astern) {
            query.width_a = static_cast<float>(
                static_cast<double>(host.controller_extent_0070d5d0()) + kShipAiArmFinalWidthMargin);
            query.width_b = static_cast<float>(
                static_cast<double>(host.controller_extent_0070d400()) + kShipAiArmFinalWidthMargin);
        } else {
            query.width_a = static_cast<float>(
                static_cast<double>(host.controller_extent_0070d400()) + kShipAiArmFinalWidthMargin);
            query.width_b = static_cast<float>(
                static_cast<double>(host.controller_extent_0070d5d0()) + kShipAiArmFinalWidthMargin);
        }
    } else if (host.controller_belongs_to_another_007788b0() && blk.flag_3a5) {
        // 009DF065..009DF097.  A ship that follows someone else's controller
        // gets one width from its own beam and keeps the entry range.
        query.width_b = static_cast<float>(static_cast<double>(tuning.hull_half_width_9cc) *
                                           kShipAiArmFinalCorridorScale);
        query.width_a = query.width_b;
    }

    if (!direction_written) {
        // 009DF09B..009DF0ED: 006BC0C0's body, inlined at the call site.
        const std::array<float, 2> direction =
            heading_to_direction_006bc0c0(blk.heading_target_324);
        query.direction_x = direction[0];
        query.direction_z = direction[1];
    }

    // 009DF0FA..009DF109.
    result.free_bearing_queried = true;
    result.searcher_index = searcher;
    if (host.avoid_zone_free_bearing_009dc2e0(searcher, query, area_key)) {
        blk.heading_target_324 = query.bearing;
        result.free_bearing_accepted = true;
    }
    return result;
}

} // namespace bsp
