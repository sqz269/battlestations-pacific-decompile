// 009E76D0, the bearing-ring scan, and the four slot scorers behind it.
//
// Packet cc_ai_ring_scan, worker agent/cc-ai-ring-scan.
// Evidence, address by address, in docs/SHIP_AI_RING_SCAN.md. Ghidra was
// read-only for this packet; every descriptive name is a hypothesis.
//
// Every float expression below was transcribed from the listing, not from the
// decompiler. Where the image computes in x87 with a double memory operand the
// projection uses `double` and rounds to float at the same store the image
// does. Comparison order and branch polarity are preserved instruction for
// instruction, so the NaN behaviour matches; that is why several tests read as
// `!(a <= b)` instead of `a > b`.

#include "bsp/ship_ai_ring_scan.hpp"

#include <cstring>

#include "bsp/unit_rudder.hpp"

namespace bsp {

namespace {

// The image makes a float non-negative by masking the sign bit of its storage
// (AND at 009E6421, 009E68D4, 009E68F0, 009E6776 and 009E7E27), not by calling
// fabsf. The two agree except that the mask leaves a NaN a NaN.
float sign_masked(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits &= 0x7FFFFFFFu;
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

float bits_to_float(std::uint32_t bits) noexcept {
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

std::uint32_t float_to_bits(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

} // namespace

// ---------------------------------------------------------------------------
// 009E6400, the mode-4 decay score
// ---------------------------------------------------------------------------

float ship_ai_ring_scan_decay_score_009e6400(float bearing,
                                             float slot_angle) noexcept {
    // 009E6403..009E6419: the slot's bearing is the SECOND argument.
    const float wrapped = wrapped_angle_subtract_00438b10(bearing, slot_angle);
    // 009E641D..009E6426, the sign mask, then a float reload at 009E642A.
    const float magnitude = sign_masked(wrapped);
    // 009E642E FCOMIP of the constant against the magnitude, JBE to the
    // subtracting arm: an unordered pair takes the dividing arm.
    double ramp = 0.0;
    if (kShipAiRingScanDecayPeak <= static_cast<double>(magnitude)) {
        ramp = (kShipAiRingScanDecayEdge - static_cast<double>(magnitude)) /
               kShipAiRingScanDecaySpan;
    } else {
        ramp = static_cast<double>(magnitude) / kShipAiRingScanDecaySpan;
    }
    // 009E644C, the one float store, then 009E6450 FLD1 / FCOMIP / JBE.
    const float ramped = static_cast<float>(ramp);
    if (!(static_cast<double>(ramped) <= 1.0)) {
        return kShipAiRingScanScoreCeiling;
    }
    return ramped;
}

void ship_ai_ring_scan_decay_slot_009e6400(ShipAiApproachSlotScore& score,
                                           float bearing,
                                           float slot_angle) noexcept {
    // 009E6464 and 009E6473, both stores land in slot+2Ch.
    score.normalized_2c = ship_ai_ring_scan_decay_score_009e6400(bearing, slot_angle);
}

// ---------------------------------------------------------------------------
// 009E5DA0, the ship-class rating that fills slot+18h
// ---------------------------------------------------------------------------

void ship_ai_ring_scan_class_score_009e5da0(const ShipAiAttackMoveRingSlot& ring_slot,
                                            ShipAiApproachSlotScore& score,
                                            ShipAiRingScanClassQuery query,
                                            ShipAiRingScanClassScoreHost& host) {
    // 009E5DA3..009E5DBB: word 5 of the routine's own copy of the block is
    // replaced in place with wrap(word5 - slot.angle_08).
    const float reference = bits_to_float(query.word[kShipAiRingScanQueryBearingWord]);
    const float wrapped = wrapped_angle_subtract_00438b10(reference, ring_slot.angle_08);
    query.word[kShipAiRingScanQueryBearingWord] = float_to_bits(wrapped);

    // 009E5DBF..009E5DC9, the rating, and the four dwords the image copies out
    // of the block afterwards (009E5DCC..009E5DED).
    score.raw_18 = host.rate_bearing_0095eb40(query);
    score.spare_1c = bits_to_float(query.word[12]);
    score.spare_20 = bits_to_float(query.word[11]);
    score.spare_24 = bits_to_float(query.word[10]);
    score.spare_28 = bits_to_float(query.word[13]);
}

// ---------------------------------------------------------------------------
// 009E6870, the standoff-arc score
// ---------------------------------------------------------------------------

float ship_ai_ring_scan_arc_cost_009e6870(float centre, float side, float span,
                                          float slot_angle) noexcept {
    // 009E688A and 009E68A2: the centre is the FIRST argument of both helpers.
    const float edge_add = wrapped_angle_add_00438aa0(centre, side);
    const float edge_sub = wrapped_angle_subtract_00438b10(centre, side);
    // 009E68BC and 009E68E3, both with the slot's bearing first, then the sign
    // masks at 009E68D4 and 009E68F0.
    const float to_add = sign_masked(wrapped_angle_subtract_00438b10(slot_angle, edge_add));
    const float to_sub = sign_masked(wrapped_angle_subtract_00438b10(slot_angle, edge_sub));
    // 009E6902 FCOMI of to_add against to_sub with JBE keeping to_add.
    const float nearest = (to_add <= to_sub) ? to_add : to_sub;

    // 009E6925..009E692D: the scaled span is float-stored before the compare.
    const float scaled_span = static_cast<float>(kShipAiRingScanOneFifth *
                                                 static_cast<double>(span));
    // 009E6931 FCOMIP of the span against the nearest edge distance, JC to the
    // subtracting arm: the divide runs when span >= nearest or the pair is
    // unordered.
    if (span < nearest) {
        // 009E693E..009E6945: nearest - (span - scaled_span).
        return static_cast<float>(static_cast<double>(nearest) -
                                  (static_cast<double>(span) - scaled_span));
    }
    // 009E6935..009E6938: (nearest / span) * scaled_span.
    return static_cast<float>(static_cast<double>(nearest) / span * scaled_span);
}

float ship_ai_ring_scan_arc_score_009e6870(float centre, float side, float span,
                                           float slot_angle,
                                           float tune_04) noexcept {
    const float cost = ship_ai_ring_scan_arc_cost_009e6870(centre, side, span, slot_angle);
    // 009E6956..009E6970: interp(0, tune+4h, pi, 0, cost).
    return clamped_interpolate_00419010(0.0f, tune_04, kShipAiRingScanArcZeroAt,
                                        0.0f, cost);
}

void ship_ai_ring_scan_arc_slot_009e6870(const ShipAiAttackMoveRingSlot& ring_slot,
                                         ShipAiApproachSlotScore& score,
                                         float centre, float side, float span,
                                         float tune_04) noexcept {
    // 009E6975, the one store: slot+30h.
    score.penalty_30 = ship_ai_ring_scan_arc_score_009e6870(centre, side, span,
                                                            ring_slot.angle_08,
                                                            tune_04);
}

// ---------------------------------------------------------------------------
// 009E6640, the obstacle probe
// ---------------------------------------------------------------------------

float ship_ai_ring_scan_probe_length_009e6640(float range, float bearing,
                                              float slot_angle) noexcept {
    // 009E6743..009E6776: wrap(bearing - slot.angle) then the sign mask.
    const float magnitude = sign_masked(
        wrapped_angle_subtract_00438b10(bearing, slot_angle));
    // 009E677F..009E67AA, then the multiply at 009E67AF.
    const float fraction = clamped_interpolate_00419010(
        kShipAiRingScanProbeNearAngle, 0.0f, kShipAiRingScanProbeFarAngle, 1.0f,
        magnitude);
    return static_cast<float>(static_cast<double>(fraction) * range);
}

bool ship_ai_ring_scan_side_allows_009e6640(float bearing, float slot_angle,
                                            bool override_a,
                                            bool override_b) noexcept {
    // 009E6648..009E6658: with both bytes clear the gate is not even evaluated.
    if (!override_a && !override_b) {
        return true;
    }
    // 009E665A..009E666B, the slot's bearing first.
    const float wrapped = wrapped_angle_subtract_00438b10(slot_angle, bearing);
    // 009E6681 COMISS of 0.0 against the difference with JA, and 009E668D
    // COMISS of the difference against 0.0 with JBE: an unordered pair takes
    // neither refusal.
    if (override_a && 0.0f > wrapped) {
        return false;
    }
    if (override_b && wrapped > 0.0f) {
        return false;
    }
    return true;
}

float ship_ai_ring_scan_probe_009e6640(ShipAiAttackMoveRingSlot& ring_slot,
                                       ShipAiApproachSlotScore& score,
                                       float seconds, float bearing,
                                       bool override_a, bool override_b,
                                       float range,
                                       ShipAiRingScanHost& host) {
    const bool allowed = ship_ai_ring_scan_side_allows_009e6640(
        bearing, ring_slot.angle_08, override_a, override_b);
    if (!allowed) {
        // 009E6697, the refused slot's clear distance is forced to zero.
        ring_slot.reset_44 = 0.0f;
    }

    // 009E669C..009E66A3, one float store, then 009E66AB stores the same value
    // into slot+48h and 009E66AE FLDZ / FCOMIP / JBE re-probes on a strictly
    // negative timer only.
    const float timer = static_cast<float>(static_cast<double>(ring_slot.jitter_48) - seconds);
    ring_slot.jitter_48 = timer;

    if (!(0.0f <= timer)) {
        // 009E66BA..009E66D4, the re-probe seed.
        ring_slot.reset_44 = range;
        score.blocked_40 = false;
        ring_slot.jitter_48 = kShipAiRingScanProbePeriod;

        const std::uint32_t space = host.probe_space_vtable_0218();
        // 009E66F8, the pose byte, then 009E6705.
        if (!host.unit_pose_fresh_00c8()) {
            host.refresh_unit_pose_00414db0();
        }
        // 009E6710..009E673E, the start point and its adjustment.
        ShipAiAttackMoveXZ start = host.unit_world_xz();
        start = host.probe_origin_00417b10(space, start,
                                           kShipAiRingScanProbeOriginClearance,
                                           kShipAiRingScanProbeOriginMode);

        const float length = ship_ai_ring_scan_probe_length_009e6640(
            range, bearing, ring_slot.angle_08);

        // 009E67D6..009E6804. The image float-stores dir_x * length and
        // length * dir_z separately (009E67E4, 009E67EC) and adds the start
        // point to each (009E67F4, 009E6800); the operand order differs
        // between the two adds but float addition makes that immaterial.
        ShipAiAttackMoveXZ end{};
        const float step_x = static_cast<float>(static_cast<double>(ring_slot.dir_x_0c) * length);
        const float step_z = static_cast<float>(static_cast<double>(length) * ring_slot.dir_z_14);
        end.x = static_cast<float>(static_cast<double>(step_x) + start.x);
        end.z = static_cast<float>(static_cast<double>(start.z) + step_z);

        ShipAiAttackMoveXZ hit{};
        if (host.probe_hit_0041b4e0(space, start, end, hit)) {
            // 009E6813..009E6838.
            score.blocked_40 = true;
            ShipAiAttackMoveXZ delta{};
            delta.x = static_cast<float>(static_cast<double>(start.x) - hit.x);
            delta.z = static_cast<float>(static_cast<double>(start.z) - hit.z);
            ring_slot.reset_44 = host.planar_length_00414c60(delta);
        }
    }

    // 009E683B..009E6866.
    if (allowed && !score.blocked_40) {
        return 1.0f;
    }
    return static_cast<float>(static_cast<double>(ring_slot.reset_44) / range);
}

// ---------------------------------------------------------------------------
// 009E76D0, the ring scan
// ---------------------------------------------------------------------------

void ship_ai_ring_scan_normalise_009e76d0(float scores[kShipAiApproachSlotCount],
                                          float best) noexcept {
    // 009E779E..009E7804, ten divides per iteration, six iterations. The image
    // keeps the maximum in ST1 for the whole run and never tests it.
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        scores[i] = static_cast<float>(static_cast<double>(scores[i]) / best);
    }
}

void ship_ai_ring_scan_apply_verdict_009e76d0(ShipAiApproachSlotScore& score,
                                              float normalised,
                                              float tune_reject_penalty) noexcept {
    // 009E7822 FCOMIP of the score against the threshold with JA to the accept
    // arm: an unordered pair is rejected.
    if (static_cast<double>(normalised) > kShipAiRingScanAcceptAbove) {
        // 009E7852, the only thing an accepted slot gets.
        score.blocked_40 = false;
        return;
    }
    // 009E782D..009E784B, in the image's store order.
    score.evade_38 = 0.0f;
    score.avoid_3c = 0.0f;
    score.normalized_2c = 0.0f;
    score.penalty_30 = kShipAiRingScanRejectBase - tune_reject_penalty;
}

int ship_ai_ring_scan_winner_009e76d0(
    const ShipAiApproachSlotScore slots[kShipAiApproachSlotCount]) noexcept {
    // 009E79CA..009E79E1, slot 0 seeds both the index and the total.
    int winner = 0;
    float best = ship_ai_approach_slot_total_009e76d0(slots[0]);
    for (int i = 1; i < kShipAiApproachSlotCount; ++i) {
        const float total = ship_ai_approach_slot_total_009e76d0(slots[i]);
        // 009E79FF FCOMIP of the candidate against the incumbent with JBE
        // skipping the update, so the first maximum wins and a NaN never
        // displaces the incumbent.
        if (!(total <= best)) {
            best = total;
            winner = i;
        }
    }
    return winner;
}

bool ship_ai_ring_scan_reverse_goal_009e76d0(float cruise_speed_0490,
                                             float goal_range_11e0,
                                             float unit_heading_11ec,
                                             float goal_bearing) noexcept {
    // 009E7E04..009E7E27, the wrapped turn and its sign mask.
    const float turn = sign_masked(
        wrapped_angle_subtract_00438b10(unit_heading_11ec, goal_bearing));
    // 009E7E41..009E7E68.
    const float margin = clamped_interpolate_00419010(
        kShipAiRingScanGoalTurnNear, kShipAiRingScanGoalMarginNear,
        kShipAiRingScanGoalTurnFar, kShipAiRingScanGoalMarginFar, turn);
    // 009E7E6D FSUBR against the widened speed, 009E7E71 the float store, then
    // 009E7E7D FCOMIP against the widened range with JBE skipping the reversal.
    const float slack = static_cast<float>(static_cast<double>(cruise_speed_0490) - margin);
    return !(static_cast<double>(slack) <= static_cast<double>(goal_range_11e0));
}

void ship_ai_ring_scan_009e76d0(ShipAiApproachState& state,
                                ShipAiAttackMoveRingSlot ring[kShipAiApproachSlotCount],
                                ShipAiApproachSlotScore slots[kShipAiApproachSlotCount],
                                float seconds,
                                ShipAiRingScanHost& host) {
    // 009E76D0..009E76F4. One float store, then the in-place wrap.
    const float advanced = static_cast<float>(
        static_cast<double>(seconds) * kShipAiRingScanOneFifth +
        static_cast<double>(state.wobble_phase_1214));
    state.wobble_phase_1214 = host.wrap_phase_00605070(advanced);

    // 009E76F9, the mode test.
    if (state.mode_1234 == ShipAiApproachMode::inside_3) {
        // 009E79B4..009E79C8: every blocked byte is cleared and nothing is
        // scored, so the winner scan below runs on the previous frame's values.
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            slots[i].blocked_40 = false;
        }
    } else {
        // 009E7709..009E778B, the scoring pass with its running maximum.
        float scores[kShipAiApproachSlotCount] = {};
        float best = 0.0f;
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            scores[i] = ship_ai_ring_scan_probe_009e6640(
                ring[i], slots[i], seconds, state.slot_scale_11dc,
                state.override_11d4, state.override_11d5,
                state.turn_radius_11f0, host);
            // 009E776C FCOMIP of the score against the maximum with JBE.
            if (!(scores[i] <= best)) {
                best = scores[i];
            }
        }

        // 009E778D COMISS of 1.0 against the maximum with JBE skipping the
        // normalisation, so it runs only on a strictly smaller, ordered maximum.
        if (kShipAiRingScanScoreCeiling > best) {
            ship_ai_ring_scan_normalise_009e76d0(scores, best);
        }

        const float penalty = host.tune_reject_penalty_04();
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            ship_ai_ring_scan_apply_verdict_009e76d0(slots[i], scores[i], penalty);
        }
    }

    // 009E79CA..009E7C28, the winner and its bearing.
    const int winner = ship_ai_ring_scan_winner_009e76d0(slots);
    float bearing = ring[winner].angle_08;
    state.selected_bearing_11f8 = bearing;

    // 009E7C72..009E7C92. The override arm runs only with nested+1209h set and
    // both side overrides clear.
    if (state.flag_1209 && !state.override_11d4 && !state.override_11d5) {
        // 009E7CA1, the pose byte, then the open-coded refresh.
        if (!host.unit_pose_fresh_00c8()) {
            host.rebuild_unit_world_matrix();
        }
        // 009E7D7A..009E7DDD.
        const ShipAiAttackMoveXZ goal = host.brain_goal_0b2c();
        const ShipAiAttackMoveXZ position = host.unit_world_xz();
        const float dz = static_cast<float>(static_cast<double>(goal.z) - position.z);
        const float dx = static_cast<float>(static_cast<double>(goal.x) - position.x);
        float heading = ship_ai_approach_heading_from_delta(dx, dz);
        if (ship_ai_ring_scan_reverse_goal_009e76d0(host.unit_cruise_speed_0490(),
                                                    state.goal_range_11e0,
                                                    state.unit_heading_11ec,
                                                    heading)) {
            // 009E7E83..009E7E9C.
            heading = wrapped_angle_add_00438aa0(heading, kShipAiRingScanGoalReverse);
        }
        // 009E7EA6 and 009E7EAE.
        state.selected_bearing_11f8 = heading;
        bearing = heading;
    }

    // 009E7EB4..009E7ECB.
    host.commit_bearing_009e5e90(bearing, seconds);
}

} // namespace bsp
