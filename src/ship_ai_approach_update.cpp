// 009F3090, the nested update that produces the attackmove approach point.
//
// Packet cc_ai_approach_update, worker agent/cc-ai-approach-update.
// Evidence, address by address, in docs/SHIP_AI_APPROACH_UPDATE.md. Ghidra was
// read-only for this packet; every descriptive name is a hypothesis.
//
// Every float expression below was transcribed from the listing, not from the
// decompiler. Where the image computes in x87 80-bit registers with a double
// memory operand this projection uses `double` and rounds to float at the same
// store the image does. Comparison order and branch polarity are preserved
// instruction for instruction, so the NaN behaviour matches; that is why
// several tests read as `!(a <= b)` instead of `a > b`.

#include "bsp/ship_ai_approach_update.hpp"

#include <cmath>
#include <cstring>

#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"

namespace bsp {

namespace {

// The image makes a float non-negative by masking the sign bit of its storage
// (AND ECX, 0x7FFFFFFF at 009E7619, 009E7688, 009E75A3, 009E6D7A, 009E6DE2,
// 009E7412 and 009E738C), not by calling fabsf. The two agree except that the
// mask leaves a NaN a NaN with its payload intact.
float sign_masked(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits &= 0x7FFFFFFFu;
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

// 00415510 BSP_Math_MinFloatByRef: FCOMIP of b against a with JBE returning b,
// so an unordered pair returns b. 00415550 is the mirror.
float min_by_ref_00415510(float a, float b) noexcept { return (b < a) ? b : a; }
float max_by_ref_00415550(float a, float b) noexcept { return (b > a) ? b : a; }

} // namespace

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

float ship_ai_approach_goal_range_009f1bc0(float goal_x, float goal_z,
                                           float unit_x, float unit_z) noexcept {
    const float dx = static_cast<float>(static_cast<double>(goal_x) - unit_x);
    const float dz = static_cast<float>(static_cast<double>(goal_z) - unit_z);
    // 009F1C96..009F1CAC: dx*dx + 0*0 + dz*dz, one float store.
    const float sum = static_cast<float>(static_cast<double>(dx) * dx +
                                         0.0 * 0.0 +
                                         static_cast<double>(dz) * dz);
    // 009F1CBA FCOMI then JBE: the square root runs only on the strictly
    // greater, ordered side; everything else stores 0.0f.
    if (!(static_cast<double>(sum) > kApproachRangeEpsilonSq)) {
        return 0.0f;
    }
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

float ship_ai_approach_heading_from_delta(float dx, float dz) noexcept {
    const float angle =
        static_cast<float>(std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    float heading = static_cast<float>(kApproachQuarterTurn - static_cast<double>(angle));
    // 009E7DCF / 009E9617, FCOMIP of 0.0 against the heading then JBE.
    if (!(0.0f <= heading)) {
        heading = static_cast<float>(static_cast<double>(heading) + kApproachFullTurn);
    }
    return heading;
}

float ship_ai_approach_bearing_weight_009e74d0(float unit_heading,
                                               float slot_angle,
                                               float tune_10) noexcept {
    const float diff = wrapped_angle_subtract_00438b10(unit_heading, slot_angle);
    return clamped_interpolate_00419010(0.0f, tune_10, kApproachHalfTurn, 0.0f,
                                        sign_masked(diff));
}

float ship_ai_approach_evade_weight_009e74d0(float evade_reference,
                                             float slot_angle,
                                             float tune_14, float tune_18,
                                             float gain) noexcept {
    // 009E7663: the product is float-stored before it becomes an endpoint.
    const float strength =
        static_cast<float>(static_cast<double>(tune_14) * gain);
    const float diff = wrapped_angle_subtract_00438b10(evade_reference, slot_angle);
    return clamped_interpolate_00419010(0.0f, strength, tune_18, 0.0f,
                                        sign_masked(diff));
}

int ship_ai_approach_evade_slot_009e74d0(float selected_bearing,
                                         int current_slot) noexcept {
    // 009E754B: the draw only happens while the field still holds the idle
    // sentinel 60.
    if (current_slot != kShipAiApproachSlotCount) {
        return current_slot;
    }
    // 009E7557..009E7571, _ftol2 truncation toward zero then the (unreachable)
    // wrap. See the header: `selected_bearing` is a bearing in (-pi, pi], so
    // the sum never reaches 60 and the subtraction never runs.
    int slot = static_cast<int>(kApproachEvadeSlotOffset +
                                static_cast<double>(selected_bearing));
    if (slot >= kShipAiApproachSlotCount) {
        slot -= kShipAiApproachSlotCount;
    }
    return slot;
}

float ship_ai_approach_normalized_score_009e7fc0(float raw, float maximum,
                                                 float tune_00) noexcept {
    // 009E81E4..009E81FC: divide, multiply, one float store. The image never
    // tests the divisor.
    return static_cast<float>(static_cast<double>(raw) / maximum * tune_00);
}

float ship_ai_approach_avoid_weight_009e9190(float strength, float slot_angle,
                                             float avoid_heading) noexcept {
    // 009E968D then 009E96B4: the wrapped difference is passed SIGNED. There is
    // no sign mask here, unlike 009E74D0.
    const float diff = wrapped_angle_subtract_00438b10(slot_angle, avoid_heading);
    return clamped_interpolate_00419010(0.0f, strength, kApproachHalfTurn, 0.0f,
                                        diff);
}

float ship_ai_approach_slot_total_009e76d0(const ShipAiApproachSlotScore& slot) noexcept {
    // 009E7BE0..009E7BEE, in the image's accumulation order.
    double total = slot.avoid_3c;
    total += slot.normalized_2c;
    total += slot.penalty_30;
    total += slot.bearing_34;
    total += slot.evade_38;
    return static_cast<float>(total);
}

float ship_ai_approach_throttle_seed_009e6a90(float heading_error_abs) noexcept {
    return clamped_interpolate_00419010(kApproachThrottleSeedX0,
                                        kApproachThrottleSeedY0,
                                        kApproachThrottleSeedX1,
                                        kApproachThrottleSeedY1,
                                        heading_error_abs);
}

float ship_ai_approach_command_limit_009e6a90(float command, float limit) noexcept {
    const float low = -limit;
    // 009E6BE6: JBE leaves the -limit store only when -limit is strictly above
    // the command and the pair is ordered.
    if (low > command) {
        return low;
    }
    // 009E6E4D: the same shape against the upper bound.
    if (command > limit) {
        return limit;
    }
    return command;
}

int ship_ai_approach_slot_of_bearing_009e5e90(float bearing) noexcept {
    float shifted =
        static_cast<float>(static_cast<double>(bearing) + kApproachSlotHalfWidth);
    // 009E5EAA, FCOMIP of 0.0 against the sum then JBE.
    if (!(0.0f <= shifted)) {
        shifted = static_cast<float>(static_cast<double>(shifted) + kApproachFullTurn);
    }
    const double scaled = static_cast<double>(shifted) * kApproachSlotDivisor;
    // 009E5ECC, _ftol2 (00BF7420 is CVTTSD2SI), truncation toward zero.
    return static_cast<int>(scaled / kApproachFullTurn);
}

void ship_ai_approach_turn_costs_009e5e90(const bool blocked[kShipAiApproachSlotCount],
                                          int from_slot, int to_slot,
                                          int& backward_cost,
                                          int& forward_cost) noexcept {
    // 009E5F65, SETZ on the committed slot's byte seeds the latch.
    bool latch = !blocked[from_slot];
    backward_cost = 0;
    forward_cost = 0;

    int slot = from_slot; // 009E5F6A, MOV EDX,ECX
    do {
        if (blocked[slot]) {
            if (latch) {
                backward_cost += kApproachBlockedSlotCost; // 009E5F7F
            }
        } else {
            latch = true;                                  // 009E5F84
            backward_cost += 1;
        }
        slot -= 1;
        if (slot < 0) {
            slot = kShipAiApproachSlotCount - 1;            // 009E5F8E
        }
    } while (slot != to_slot);

    slot = from_slot; // 009E5F9B, ECX still holds the committed slot
    do {
        if (blocked[slot]) {
            if (latch) {
                forward_cost += kApproachBlockedSlotCost;   // 009E5FAB
            }
        } else {
            latch = true;                                   // 009E5FB0
            forward_cost += 1;
        }
        slot += 1;
        if (slot >= kShipAiApproachSlotCount) {
            slot = 0;                                       // 009E5FBD
        }
    } while (slot != to_slot);
}

float ship_ai_approach_commanded_heading_009e5e90(float reference, float bearing,
                                                  bool add_arm) noexcept {
    if (add_arm) {
        // 009E5F06..009E5F40.
        const float turn = wrapped_angle_subtract_00438b10(bearing, reference);
        if (!(static_cast<double>(turn) > kApproachTurnDeadband)) {
            return wrapped_angle_add_00438aa0(reference, kApproachMaxTurnStep);
        }
        if (!(kApproachMaxTurnStep > turn)) {
            return wrapped_angle_add_00438aa0(reference, kApproachMaxTurnStep);
        }
        return bearing;
    }
    // 009E5FD3..009E6009.
    const float turn = wrapped_angle_subtract_00438b10(reference, bearing);
    if (!(static_cast<double>(turn) > kApproachTurnDeadband)) {
        return wrapped_angle_subtract_00438b10(reference, kApproachMaxTurnStep);
    }
    if (kApproachMaxTurnStep > turn) {
        return bearing;
    }
    return wrapped_angle_subtract_00438b10(reference, kApproachMaxTurnStep);
}

// ---------------------------------------------------------------------------
// 009D68B0, the circle-tangent primitive
// ---------------------------------------------------------------------------

ShipAiAttackMoveXZ ship_ai_circle_tangent_009d68b0(
    const ShipAiCircleTangentCircle& circle, const ShipAiAttackMoveXZ& point,
    float clearance, int side, ShipAiCircleTangentHost& host) {
    // 009D68C7, SETZ: side 0 picks the second tangent, anything else the first.
    const int index = (side == 0) ? 1 : 0;

    ShipAiAttackMoveXZ first{point.x, point.z}; // 009D68D6/009D68E6 preload
    ShipAiAttackMoveXZ second{point.x, point.z};
    ShipAiAttackMoveXZ chosen{point.x, point.z};

    bool have_tangent = host.tangent_points_009d6550(point, first, second);
    if (have_tangent) {
        chosen = (index == 0) ? first : second; // 009D68F5
    } else {
        // 009D6970: the distance from the circle centre to the query point.
        const float cdx = static_cast<float>(static_cast<double>(circle.x) - point.x);
        const float cdz = static_cast<float>(static_cast<double>(circle.z) - point.z);
        const float sum = static_cast<float>(static_cast<double>(cdx) * cdx +
                                             static_cast<double>(cdz) * cdz);
        bool far_enough = false;
        if (static_cast<double>(sum) > kApproachTangentEpsilonSq) {
            const float distance = host.sqrt_00bf7030(sum);
            // 009D69B7, COMISS of 1.0f against the distance then JBE.
            far_enough = !(kApproachTangentMinSeparation > distance);
        }
        if (!far_enough) {
            // 009D69C6, the degenerate arm: a random point on the circle.
            const float angle =
                host.random_stream1_00bd2f10(0.0f, kApproachTangentRandomTurn);
            const float c = std::cos(angle); // 009D69EA FCOS, float-stored
            const float s = std::sin(angle); // 009D69FC FSIN, float-stored
            ShipAiAttackMoveXZ out{};
            out.x = static_cast<float>(static_cast<double>(circle.x) +
                                       static_cast<double>(circle.radius) * s);
            out.z = static_cast<float>(static_cast<double>(circle.z) +
                                       static_cast<double>(circle.radius) * c);
            return out;
        }
        // Otherwise fall through to 009D690D with the query point still in the
        // chosen slots, which makes the separation below exactly zero.
    }

    // 009D690D..009D6954, the separation between the chosen point and the query
    // point, then the clearance test.
    const float dx = static_cast<float>(static_cast<double>(chosen.x) - point.x);
    const float dz = static_cast<float>(static_cast<double>(chosen.z) - point.z);
    const float sum = static_cast<float>(static_cast<double>(dx) * dx +
                                         static_cast<double>(dz) * dz);
    float separation = 0.0f;
    if (static_cast<double>(sum) > kApproachTangentEpsilonSq) {
        separation = host.sqrt_00bf7030(sum);
    }
    // 009D6A50, FCOMIP of the clearance against the separation then JBE.
    if (!(clearance > separation)) {
        return chosen;
    }
    ShipAiAttackMoveXZ offset_first{};
    ShipAiAttackMoveXZ offset_second{};
    host.offset_points_004f47b0(point, clearance, offset_first, offset_second);
    return (index == 0) ? offset_first : offset_second; // 009D6A8F
}

// ---------------------------------------------------------------------------
// 009F3090
// ---------------------------------------------------------------------------

void ship_ai_approach_update_009f3090(ShipAiApproachUpdateHost& host,
                                      float seconds) {
    host.frame_state_009f1bc0(seconds);        // 009F309B
    host.reset_scores_009e7fc0();              // 009F30A2
    host.choose_standoff_range_009e6e80();     // 009F30A9
    host.refresh_avoidance_009e9190(seconds);  // 009F30B8
    host.score_evade_009e74d0(seconds);        // 009F30C7
    host.select_slot_009e76d0(seconds);        // 009F30D6
    host.limit_throttle_009e6a90();            // 009F30DD
}

// ---------------------------------------------------------------------------
// 009E7FC0
// ---------------------------------------------------------------------------

void ship_ai_approach_reset_scores_009e7fc0(ShipAiApproachState& state,
                                            ShipAiApproachSlotScore slots[],
                                            bool has_target,
                                            ShipAiApproachScoreResetHost& host) {
    state.cleared_1218 = 0.0f; // 009E7FD1
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        // 009E7FE0..009E7FF8, six floats at slot+18h..+2Ch.
        slots[i].raw_18 = 0.0f;
        slots[i].spare_1c = 0.0f;
        slots[i].spare_20 = 0.0f;
        slots[i].spare_24 = 0.0f;
        slots[i].spare_28 = 0.0f;
        slots[i].normalized_2c = 0.0f;
    }

    // 009E8005..009E8029: either override clears the clearance flag, both
    // together end the pass.
    if (state.override_11d4 || state.override_11d5) {
        state.clearance_valid_12ba = false;
    }
    if (state.override_11d4 && state.override_11d5) {
        return;
    }

    // 009E802F: modes 1, 2 and 3 do nothing else.
    const ShipAiApproachMode mode = state.mode_1234;
    if (mode == ShipAiApproachMode::hold_1 ||
        mode == ShipAiApproachMode::inside_3 ||
        mode == ShipAiApproachMode::unassigned_2) {
        return;
    }

    if (mode == ShipAiApproachMode::standoff_4) {
        // 009E8055: the decay pass runs only inside the 300 m window.
        const double elapsed = static_cast<double>(state.goal_range_11e0) -
                               state.standoff_range_11e4;
        if (!(kApproachDecayWindow > elapsed)) {
            return;
        }
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            host.decay_slot_009e6400(i, state.avoid_radius_1290); // 009E8080
        }
        return;
    }

    // 009E8095, the mode-0 arm.
    if (has_target) {
        host.target_kind_probe_vtable_005c(5); // 009E80AB, result discarded
    }
    if (!host.brain_flag_0b28()) { // 009E80B0
        return;
    }
    const float reference = host.nested_reference_127c(); // 009E80BD
    const float lookahead = host.unit_lookahead_0494();   // 009E80CD
    // 009E80DF, FCOMI then JA: a reference above the lookahead ends the pass.
    if (reference > lookahead) {
        return;
    }
    if (state.flag_1208 && !state.flag_1209) {
        // 009E80FB, the mirror test, so the two must be equal to continue.
        if (lookahead > reference) {
            return;
        }
    }

    bool allowed = false;
    if (has_target) {
        allowed = host.zone_allows_target_00864fd0(host.brain_target_0b20()); // 009E8116
    } else {
        const ShipAiApproachPoint probe = host.probe_point_009e6120(); // 009E8129
        allowed = host.zone_allows_point_00864ba0(probe);              // 009E8130
    }
    if (!allowed) { // 009E8135
        return;
    }

    // 009E813D..009E8178.
    state.avoid_radius_1290 = state.slot_scale_11dc;
    state.clearance_valid_12ba = true; // 009E8171 and 009E8178 set +12BCh/+12BDh

    // 009E8192..009E81DA, the scoring pass and its running maximum, which the
    // image seeds with 1.0f so the normalisation never divides by less.
    float maximum = kApproachBearingGain;
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        slots[i].raw_18 = host.score_slot_009e5da0(i); // 009E81A7
        // 009E81BA, FCOMI then JA: the maximum only moves on the not-above side.
        if (!(maximum > slots[i].raw_18)) {
            maximum = slots[i].raw_18;
        }
    }

    const float scale = host.tune_scale_00(); // 009E81FA
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        slots[i].normalized_2c =
            ship_ai_approach_normalized_score_009e7fc0(slots[i].raw_18, maximum, scale);
    }
}

// ---------------------------------------------------------------------------
// 009E74D0
// ---------------------------------------------------------------------------

void ship_ai_approach_score_evade_009e74d0(ShipAiApproachState& state,
                                           const ShipAiAttackMoveRingSlot ring[],
                                           ShipAiApproachSlotScore slots[],
                                           float seconds,
                                           ShipAiApproachEvadeHost& host) {
    float gain = kApproachBearingGain; // 009E74EC, 1.0f
    int evade_slot = static_cast<int>(state.selected_bearing_11f8); // 009E74E8

    // 009E7502, COMISS then JBE: the whole latch only runs while the timer is
    // strictly positive.
    if (state.evade_timer_11fc > 0.0f) {
        if (host.unit_evade_flag_1128() > 0.0f) { // 009E7527
            state.evade_timer_11fc = static_cast<float>(
                static_cast<double>(state.evade_timer_11fc) + seconds); // 009E7534
        }
        // 009E7542, COMISS of the timer against 40.0f then JBE.
        if (state.evade_timer_11fc > kApproachEvadeArmRange) {
            state.evade_slot_1200 = ship_ai_approach_evade_slot_009e74d0(
                state.selected_bearing_11f8, state.evade_slot_1200);
            evade_slot = state.evade_slot_1200;
            gain = kApproachEvadeGain; // 009E758D
            // 009E7589..009E75B2. See the header: this difference is always
            // about 30, so the reset never fires. Reproduced, not corrected.
            const float delta = static_cast<float>(
                static_cast<double>(state.evade_slot_1200) - state.selected_bearing_11f8);
            if (kApproachEvadeResetWindow > sign_masked(delta)) {
                state.evade_timer_11fc = kApproachEvadeIdle;             // 009E75BC
                state.evade_slot_1200 = kShipAiApproachSlotCount;        // 009E75C4
            }
        }
    }

    const float evade_reference = static_cast<float>(evade_slot); // 009E75CA
    const float tune_bearing = host.tune_bearing_10();
    const float tune_evade = host.tune_evade_14();
    const float tune_span = host.tune_evade_span_18();
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        slots[i].bearing_34 = ship_ai_approach_bearing_weight_009e74d0(
            state.unit_heading_11ec, ring[i].angle_08, tune_bearing);
        slots[i].evade_38 = ship_ai_approach_evade_weight_009e74d0(
            evade_reference, ring[i].angle_08, tune_evade, tune_span, gain);
    }
}

// ---------------------------------------------------------------------------
// 009E6A90
// ---------------------------------------------------------------------------

void ship_ai_approach_limit_throttle_009e6a90(ShipAiApproachState& state,
                                              ShipAiApproachThrottleHost& host) {
    // 009E6AB5 then 009E6ABB: the heading error against the commanded heading.
    const float heading = host.unit_heading_vtable_0050();
    const float error =
        wrapped_angle_subtract_00438b10(heading, state.commanded_heading_120c);
    const float error_abs = sign_masked(error);

    // 009E6ADE: the command still holding 009F1BC0's 9999.0f sentinel is seeded
    // from the heading error.
    if (static_cast<double>(state.commanded_throttle_1210) > kApproachThrottleUnsetKnee) {
        state.commanded_throttle_1210 = ship_ai_approach_throttle_seed_009e6a90(error_abs);
    }

    float limit = kApproachThrottleSeedY0; // 009E6B2E, 1.0f
    std::uint32_t target = 0;

    if (state.mode_1234 == ShipAiApproachMode::standoff_4) {
        if (host.unit_evade_flag_1128() > 0.0f) { // 009E6B4E
            limit = 0.0f;
        } else if (!state.override_11d4 && !state.override_11d5) {
            float reference = state.standoff_range_11e4; // 009E6B75
            target = host.engagement_target_009e5e00(); // 009E6B85
            if (target != 0) {
                reference = static_cast<float>(
                    static_cast<double>(host.target_radius_07c4(target)) -
                    kApproachDecayWindow); // 009E6B90, FILD then FSUB 300.0
            }
            const float distance = static_cast<float>(
                static_cast<double>(state.goal_range_11e0) - reference); // 009E6BA0
            // 009E6BB8, FCOMIP of 50.0 against the distance then JBE.
            if (kApproachStopWindow > static_cast<double>(distance)) {
                limit = 0.0f;
            } else {
                limit = clamped_interpolate_00419010(0.0f, kApproachCreepFloor,
                                                     kApproachCreepKnee,
                                                     kApproachThrottleSeedY0,
                                                     distance); // 009E6C2B
                const float radius = host.unit_is_group_leader_00778890()
                                         ? kApproachLeaderStopRadius
                                         : kApproachCreepKnee; // 009E6C45/009E6C4F
                // 009E6C6B, FCOMIP of (radius + reference) against the range.
                const float outer = static_cast<float>(
                    static_cast<double>(radius) + reference);
                if (outer > state.goal_range_11e0 && target != 0 &&
                    host.target_accepted_vtable_0234(target)) {
                    limit = kApproachAcceptedThrottle; // 009E6C8C
                }
            }
        }
    } else if (state.mode_1234 == ShipAiApproachMode::unassigned_2) {
        const float distance = static_cast<float>(
            static_cast<double>(state.goal_range_11e0) - state.standoff_range_11e4);
        limit = clamped_interpolate_00419010(kApproachMode2X0, kApproachMode2Y0,
                                             kApproachMode2X1,
                                             kApproachThrottleSeedY0,
                                             distance); // 009E6CE2
    }

    // 009E6CEB: a non-positive limit skips the gate entirely. The arms that
    // jumped straight to 009E6CFD reach the gate with their limit intact.
    if (limit > 0.0f) {
        if (state.speed_gate_1204 == 1) {
            // 009E6D24, FCOMIP then JBE.
            const float threshold = static_cast<float>(
                static_cast<double>(state.goal_range_11e0) - kApproachGateRange);
            if (state.standoff_range_11e4 > threshold) {
                limit = 0.0f; // 009E6D2E
            }
        } else if (state.speed_gate_1204 == 2) {
            const float distance = static_cast<float>(
                static_cast<double>(state.goal_range_11e0) - state.standoff_range_11e4);
            // 009E6D70, FCOMI then JC: below -50 is the astern arm.
            if (static_cast<double>(distance) < kApproachAsternWindow) {
                if (kApproachHeadingDeadband > error_abs) { // 009E6DEC
                    const double ratio =
                        (-static_cast<double>(distance) - kApproachStopWindow) /
                        kApproachAsternDivisor; // 009E6DF7..009E6E10
                    const float capped = clamp_float_by_ref_00415620(
                        static_cast<float>(ratio), 0.0f, kApproachAsternCap);
                    state.commanded_throttle_1210 =
                        -min_by_ref_00415510(limit, capped); // 009E6E40
                }
            } else if (kApproachHeadingDeadband > error_abs) { // 009E6D83
                const double ratio =
                    (static_cast<double>(distance) - kApproachStopWindow) /
                    kApproachGateRange; // 009E6D8E..009E6DA1
                const float capped = clamp_float_by_ref_00415620(
                    static_cast<float>(ratio), 0.0f, kApproachAheadCap);
                limit = min_by_ref_00415510(limit, capped); // 009E6DD0
            }
        }
    }

    state.commanded_throttle_1210 =
        ship_ai_approach_command_limit_009e6a90(state.commanded_throttle_1210, limit);
}

// ---------------------------------------------------------------------------
// 009E6E80
// ---------------------------------------------------------------------------

void ship_ai_approach_choose_standoff_009e6e80(ShipAiApproachState& state,
                                               bool has_target,
                                               ShipAiApproachStandoffHost& host) {
    host.construct_scratch_00954940(); // 009E6E8F

    bool chosen = false;
    if (state.override_11d4 && state.override_11d5) { // 009E6E94
        state.standoff_range_11e4 = kApproachRangeDisabled;
        chosen = true;
    }
    if (!chosen) {
        const float override_range = host.tune_range_override_1c(); // 009E6EC5
        // 009E6ECA, COMISS then JC: only a value below zero (or unordered)
        // continues; anything else is the standoff range.
        if (!(override_range < kApproachRangeOverrideFloor)) {
            state.standoff_range_11e4 = override_range;
            chosen = true;
        }
    }
    if (!chosen && (state.mode_1234 == ShipAiApproachMode::hold_1 ||
                    state.mode_1234 == ShipAiApproachMode::inside_3)) {
        state.standoff_range_11e4 = kApproachRangeDisabled; // 009E6EE8/009E6EED
        chosen = true;
    }
    if (!chosen && has_target && host.target_is_kind_vtable_005c(8) &&
        !host.shipclass_allows_close_00827f70()) { // 009E6EFC..009E6F18
        state.standoff_range_11e4 = kApproachRangeDisabled;
        chosen = true;
    }

    if (!chosen && state.mode_1234 == ShipAiApproachMode::standoff_4) {
        // 009E6F29..009E6FB3.
        double base = kApproachFallbackRadius;
        if (has_target && host.target_is_kind_vtable_005c(0x1C)) {
            base = static_cast<double>(host.target_radius_07c4()) - kApproachDecayWindow;
        }
        float low = static_cast<float>(base); // 009E6F64, one float store
        if (host.unit_is_group_leader_00778890()) {
            low = static_cast<float>(static_cast<double>(low) - kApproachLeaderBonus);
        }
        const float high = static_cast<float>(kApproachStandoffSpread +
                                              static_cast<double>(low)); // 009E6F97
        const float current = state.standoff_range_11e4;
        if (current > high || low > current) { // 009E6FA9 and 009E6FAF
            state.standoff_range_11e4 = host.random_stream1_00bd2f10(low, high);
        }
    } else if (!chosen && state.mode_1234 == ShipAiApproachMode::unassigned_2) {
        // 009E700B..009E7122.
        float low = 0.0f;
        float high = 0.0f;
        if (has_target && host.target_is_kind_vtable_005c(0x1C)) {
            const float fraction = clamped_interpolate_00419010(
                kApproachGunRangeNear, kApproachGunFractionNear,
                kApproachGunRangeFar, kApproachGunFractionFar,
                host.unit_gun_reference_09c8()); // 009E706A
            const std::int32_t gun_range = host.target_gun_range_07a0();
            low = static_cast<float>(static_cast<double>(fraction) * gun_range); // FIMUL
            high = static_cast<float>(static_cast<double>(gun_range) *
                                      kApproachTargetRangeHigh); // 009E708D
        } else {
            low = static_cast<float>(kApproachNoTargetLow);   // 009E7079
            high = static_cast<float>(kApproachNoTargetHigh); // 009E7095
        }
        if (host.unit_is_group_leader_00778890()) { // 009E70A7
            high = static_cast<float>(static_cast<double>(high) * kApproachLeaderShrink);
            const double floor_value = kApproachLeaderFloor * high;
            if (static_cast<double>(low) > floor_value) { // 009E70CE
                low = static_cast<float>(floor_value);
            }
        }
        const float current = state.standoff_range_11e4;
        if (current > high || low > current) { // 009E70F4 and 009E717A
            state.standoff_range_11e4 = host.random_stream1_00bd2f10(low, high);
        }
    } else if (!chosen && state.flag_1208) {
        // 009E7138..009E719E, the cruise band.
        const float speed = host.unit_cruise_speed_0490();
        const float low = static_cast<float>(kApproachCruiseLow * speed);  // 009E7154
        const float high = static_cast<float>(static_cast<double>(speed) *
                                              kApproachCruiseHigh);       // 009E715A
        const float current = state.standoff_range_11e4;
        if (current > high || low > current) {
            state.standoff_range_11e4 = host.random_stream1_00bd2f10(low, high);
        }
    } else if (!chosen) {
        // 009E71A5..009E72E8, the 119-step curve scan. Every sample keeps the
        // SMALLEST score (009E72AE, FCOMIP then JC), seeded with FLT_MAX.
        state.standoff_range_11e4 = static_cast<float>(
            static_cast<double>(host.curve_base_00952530()) + kApproachDecayWindow);
        float x = kApproachScanStep; // 009E71C3
        const float step = static_cast<float>(static_cast<double>(kApproachScanStep) *
                                              kApproachScanStepScale); // 009E71D7
        const float reference = host.curve_reference_009523c0();       // 009E71EF
        float best = kApproachScanSeedScore;                           // 009E71E5
        for (int i = 0; i < kApproachScanSteps; ++i) {
            const float primary = host.curve_primary_00955a40(x);      // 009E721A
            const float secondary = host.curve_secondary_00955a40(x);  // 009E722D
            // 009E723C, FCOMI then JBE: a non-positive primary is skipped.
            if (primary > 0.0f) {
                const float weight = clamped_interpolate_00419010(
                    0.0f, kApproachScanWeightHigh, kApproachThrottleSeedY0,
                    kApproachThrottleSeedY0,
                    static_cast<float>(static_cast<double>(primary) / reference));
                const float floor_value =
                    max_by_ref_00415550(kApproachBearingGain, secondary); // 009E727F
                const float ratio = static_cast<float>(
                    static_cast<double>(host.nested_scan_scale_1284()) / primary);
                const float score = static_cast<float>(
                    static_cast<double>(static_cast<float>(
                        static_cast<double>(floor_value) * ratio)) * weight);
                if (!(best < score)) { // 009E72AE, FCOMIP then JC
                    best = score;
                    state.standoff_range_11e4 = x;
                }
            }
            x = static_cast<float>(static_cast<double>(step) + x); // 009E72D5
        }
    }

    // 009E6FBD, the common tail.
    float turn_radius = static_cast<float>(
        static_cast<double>(host.unit_turn_radius_00811a30(kApproachThrottleSeedY0)) *
        kApproachTurnRadiusScale); // 009E6FD0
    if (!(kApproachTurnRadiusKnee <= static_cast<double>(turn_radius))) {
        turn_radius = kApproachTurnRadiusFloor; // 009E6FEE
    }
    if (state.clearance_valid_12ba && state.clearance_12b4 > kApproachRangeOverrideFloor &&
        host.unit_clearance_count_0080df40() > 0) { // 009E72F3..009E7322
        const float limit = static_cast<float>(
            static_cast<double>(state.clearance_12b4) - turn_radius); // 009E732E
        state.standoff_range_11e4 =
            min_by_ref_00415510(state.standoff_range_11e4, limit); // 009E7338
    }

    // 009E733F..009E7375.
    float offset = static_cast<float>(
        (static_cast<double>(state.goal_range_11e0) - state.standoff_range_11e4) /
        turn_radius);
    if (state.flag_1208) {
        // 009E7360, COMISS of 0.0 against the offset then JBE.
        offset = (0.0f > offset)
                     ? static_cast<float>(static_cast<double>(offset) - kApproachSideBias)
                     : static_cast<float>(static_cast<double>(offset) + kApproachSideBias);
    }
    const float span_weight = clamped_interpolate_00419010(
        kApproachSideWeightX0, kApproachSideWeightY0, kApproachSideWeightX1, 0.0f,
        sign_masked(offset)); // 009E73C0

    float side_weight = 0.0f;
    if (!state.flag_1208 && state.mode_1234 == ShipAiApproachMode::free_0) {
        if (0.0f > offset) { // 009E73D9
            side_weight = kApproachHalfTurn;
            state.speed_gate_1204 = 0; // 009E73E8
        } else {
            state.speed_gate_1204 = 2; // 009E73F4
        }
    } else {
        const float magnitude = sign_masked(offset);
        // 009E7420, FCOMIP then JBE.
        const float capped = (magnitude > kApproachThrottleSeedY0)
                                 ? kApproachBearingGain
                                 : magnitude;
        if (0.0f > offset) { // 009E7436, COMISS then JBE
            // 009E7441..009E7469: q = c + c*c, float-stored at 009E7455, then
            // (0.5 * q) * (q + pi * q).
            const float q = static_cast<float>(static_cast<double>(capped) +
                                               static_cast<double>(capped) * capped);
            side_weight = static_cast<float>(
                (kApproachSideCurveScale * q) *
                (static_cast<double>(q) + kApproachHalfTurn * q));
        }
    }

    const float tune_04 = host.tune_slot_04(); // 009E7489
    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        host.score_slot_009e6870(i, side_weight, span_weight,
                                 state.slot_scale_11dc, tune_04); // 009E74B7
    }
}

// ---------------------------------------------------------------------------
// 009E9190
// ---------------------------------------------------------------------------

void ship_ai_approach_refresh_avoidance_009e9190(ShipAiApproachState& state,
                                                 const ShipAiAttackMoveRingSlot ring[],
                                                 ShipAiApproachSlotScore slots[],
                                                 float seconds,
                                                 ShipAiApproachAvoidHost& host) {
    if (state.mode_1234 == ShipAiApproachMode::hold_1 ||
        state.mode_1234 == ShipAiApproachMode::inside_3) {
        // 009E96D8: modes 1 and 3 write 0.0f into every slot, through the same
        // interpolation with both ordinates zero.
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            slots[i].avoid_3c =
                ship_ai_approach_avoid_weight_009e9190(0.0f, ring[i].angle_08, 0.0f);
        }
        return;
    }

    state.avoid_refresh_11f4 = static_cast<float>(
        static_cast<double>(state.avoid_refresh_11f4) - seconds); // 009E91DC
    // 009E91E4, FCOMIP of 0.0 against the timer then JBE.
    if (!(0.0f <= state.avoid_refresh_11f4)) {
        state.avoid_refresh_11f4 = host.random_stream1_00bd2f10(
            kApproachAvoidRefreshLow, kApproachAvoidRefreshHigh); // 009E9209
        const std::uint32_t target = host.brain_target_0b20();
        const int count = host.candidate_count_008053c0();
        const ShipAiApproachPoint unit_pos = host.unit_world_position();
        for (int i = 0; i < count; ++i) {
            const std::uint32_t candidate = host.candidate_at(i);
            if (candidate == 0 || !host.candidate_is_kind_vtable_005c(candidate)) {
                continue; // 009E9243, 009E9257
            }
            if (candidate == target) {
                continue; // 009E9263
            }
            host.refresh_pose_00414db0(candidate); // 009E9290
            const ShipAiApproachPoint p = host.entity_world_position(candidate);
            const double dx = static_cast<double>(p.x) - unit_pos.x;
            const double dz = static_cast<double>(p.z) - unit_pos.z;
            const float distance_sq =
                static_cast<float>(dx * dx + 0.0 * 0.0 + dz * dz); // 009E92CB
            const float radius = static_cast<float>(
                static_cast<double>(host.entity_speed_0494(candidate)) +
                kApproachAvoidSpeedPad); // 009E92D5
            // 009E92E5, FCOMIP of radius*radius against the distance then JBE.
            if (!(static_cast<double>(radius) * radius >
                  static_cast<double>(distance_sq))) {
                continue;
            }
            host.insert_traffic_record(candidate); // 009E9342..009E9397
        }
    }

    // 009E93B0, the accumulation pass. The seed is the three-float global at
    // 00F87574, which is zero on disk. 009E942B and 009E9432 also write the
    // byte at nested+1279h and the float at nested+1254h; neither has a reader
    // in this packet, so neither is a member of ShipAiApproachState.
    ShipAiApproachPoint accumulator{0.0f, 0.0f, 0.0f};
    const ShipAiApproachPoint unit_pos = host.unit_world_position();
    for (int record = host.traffic_record_count() - 1; record >= 0; --record) {
        if (!host.traffic_record_active_009e6170(record, unit_pos,
                                                 kApproachAvoidProbeRange)) {
            host.erase_traffic_record(record); // 009E9588
            continue;
        }
        host.advance_traffic_record_009e6240(record, seconds, unit_pos); // 009E950F
        const float weight = host.traffic_record_weight_0120(record);
        const ShipAiApproachPoint dir = host.traffic_record_direction_010c(record);
        accumulator.x = static_cast<float>(static_cast<double>(accumulator.x) +
                                           static_cast<double>(weight) * dir.x);
        accumulator.y = static_cast<float>(static_cast<double>(accumulator.y) +
                                           static_cast<double>(weight) * dir.y);
        accumulator.z = static_cast<float>(static_cast<double>(accumulator.z) +
                                           static_cast<double>(weight) * dir.z);
    }

    // 009E95B5..009E966A.
    float heading = 0.0f;
    float strength = 0.0f;
    const float length_sq = static_cast<float>(
        static_cast<double>(accumulator.x) * accumulator.x +
        static_cast<double>(accumulator.y) * accumulator.y +
        static_cast<double>(accumulator.z) * accumulator.z);
    // 009E95F4, FCOMIP of the squared length against 1.0 then JBE.
    if (length_sq > 1.0f) {
        heading = ship_ai_approach_heading_from_delta(accumulator.x, accumulator.z);
        strength = clamped_interpolate_00419010(0.0f, 0.0f,
                                                host.tune_avoid_span_0c(),
                                                host.tune_avoid_strength_08(),
                                                host.vector_length_0042b2f0(accumulator));
    }

    for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
        slots[i].avoid_3c =
            ship_ai_approach_avoid_weight_009e9190(strength, ring[i].angle_08, heading);
    }
}

// ---------------------------------------------------------------------------
// 009E76D0
// ---------------------------------------------------------------------------

void ship_ai_approach_select_slot_009e76d0(ShipAiApproachState& state,
                                           const ShipAiAttackMoveRingSlot ring[],
                                           ShipAiApproachSlotScore slots[],
                                           float seconds,
                                           ShipAiApproachSelectHost& host) {
    // 009E76D0..009E76F4: the wobble phase, advanced and wrapped in place.
    state.wobble_phase_1214 = host.wrap_angle_00605070(static_cast<float>(
        kApproachPhaseRate * seconds + static_cast<double>(state.wobble_phase_1214)));

    if (state.mode_1234 == ShipAiApproachMode::inside_3) {
        // 009E79B4: mode 3 clears every blocked byte and scores nothing.
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            slots[i].blocked_40 = false;
        }
    } else {
        // 009E7722..009E778B, the per-slot scorer and its running maximum.
        float scores[kShipAiApproachSlotCount] = {};
        float best = 0.0f;
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            scores[i] = host.score_slot_009e6640(i, seconds, state.slot_scale_11dc,
                                                 state.override_11d4,
                                                 state.override_11d5,
                                                 state.turn_radius_11f0);
            if (scores[i] > best) { // 009E776C, FCOMIP then JBE
                best = scores[i];
            }
        }
        // 009E7795, COMISS of 1.0f against the maximum then JBE: the scores are
        // normalised only when the maximum is below 1.
        if (kApproachBearingGain > best) {
            for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
                scores[i] = static_cast<float>(static_cast<double>(scores[i]) / best);
            }
        }
        // 009E7822..009E79AA, the accept/reject pass.
        const float penalty = host.tune_reject_penalty_04();
        for (int i = 0; i < kShipAiApproachSlotCount; ++i) {
            if (static_cast<double>(scores[i]) > kApproachSlotKeepThreshold) {
                slots[i].blocked_40 = false; // 009E7852
            } else {
                slots[i].normalized_2c = 0.0f;
                slots[i].evade_38 = 0.0f;
                slots[i].avoid_3c = 0.0f;
                slots[i].penalty_30 = static_cast<float>(
                    static_cast<double>(kApproachRejectPenaltyBase) - penalty);
            }
        }
    }

    // 009E79CA..009E7C19, the winner.
    int best_slot = 0;
    float best_total = ship_ai_approach_slot_total_009e76d0(slots[0]);
    for (int i = 1; i < kShipAiApproachSlotCount; ++i) {
        const float total = ship_ai_approach_slot_total_009e76d0(slots[i]);
        if (total > best_total) { // 009E7BFA, FCOMIP then JBE
            best_total = total;
            best_slot = i;
        }
    }
    float bearing = ring[best_slot].angle_08; // 009E7C1C
    state.selected_bearing_11f8 = bearing;    // 009E7C28

    // 009E7C3E..009E7C78: the wobble is computed and thrown away. The sine is
    // multiplied by the double at 00D7A258, which is 0.0, and the result of the
    // wrapped add at 009E7C6D is popped by the FSTP ST0 at 009E7C78.

    if (state.flag_1209 && !state.override_11d4 && !state.override_11d5) {
        // 009E7C98..009E7EAE: the bearing comes straight from the brain's goal.
        host.refresh_unit_pose();
        const ShipAiAttackMoveXZ goal = host.brain_goal_0b2c();
        const ShipAiAttackMoveXZ unit = host.unit_world_xz();
        float heading = ship_ai_approach_heading_from_delta(
            static_cast<float>(static_cast<double>(goal.x) - unit.x),
            static_cast<float>(static_cast<double>(goal.z) - unit.z));
        const float speed = host.unit_cruise_speed_0490();
        const float turn =
            wrapped_angle_subtract_00438b10(state.unit_heading_11ec, heading);
        const float window = clamped_interpolate_00419010(
            kApproachReverseWindowX0, kApproachReverseWindowY0,
            kApproachReverseWindowX1, kApproachReverseWindowY1,
            sign_masked(turn)); // 009E7E68
        // 009E7E7D, FCOMIP then JBE, both operands widened to double first.
        if (static_cast<double>(speed) - window > static_cast<double>(state.goal_range_11e0)) {
            heading = wrapped_angle_add_00438aa0(heading, kApproachHalfTurn); // 009E7E97
        }
        state.selected_bearing_11f8 = heading; // 009E7EA6
        bearing = heading;
    }

    host.commit_bearing_009e5e90(bearing, seconds); // 009E7ECB
}

// ---------------------------------------------------------------------------
// 009E5E90
// ---------------------------------------------------------------------------

void ship_ai_approach_commit_bearing_009e5e90(
    ShipAiApproachState& state,
    const bool blocked[kShipAiApproachSlotCount], float bearing) {
    const int slot = ship_ai_approach_slot_of_bearing_009e5e90(bearing);
    bool add_arm = false;
    if (slot == state.committed_slot_11e8) {
        // 009E5EEF..009E5F00: the sign of the turn picks the arm.
        const float turn =
            wrapped_angle_subtract_00438b10(state.unit_heading_11ec, bearing);
        add_arm = (turn < 0.0f); // JNC takes the other arm on turn >= 0
    } else {
        int backward = 0;
        int forward = 0;
        ship_ai_approach_turn_costs_009e5e90(blocked, state.committed_slot_11e8,
                                             slot, backward, forward);
        add_arm = (backward > forward); // 009E5FC3, CMP then SETLE
    }
    state.commanded_heading_120c = ship_ai_approach_commanded_heading_009e5e90(
        state.unit_heading_11ec, bearing, add_arm);
}

// ---------------------------------------------------------------------------
// 009F1BC0, the part this packet read
// ---------------------------------------------------------------------------

void ship_ai_approach_frame_state_009f1bc0(ShipAiApproachState& state,
                                           bool has_target, bool has_zone,
                                           float seconds,
                                           ShipAiApproachPointHost& host) {
    state.commanded_throttle_1210 = kApproachCommandUnset; // 009F1BF7
    state.timer_1220 = static_cast<float>(
        static_cast<double>(state.timer_1220) - seconds); // 009F1C07
    state.timer_1224 = static_cast<float>(
        static_cast<double>(state.timer_1224) - seconds); // 009F1C13
    state.unit_heading_11ec = host.unit_heading_vtable_0050(); // 009F1C26

    host.refresh_unit_pose_00414db0();
    const ShipAiApproachPoint unit_pos = host.unit_world_position(); // 009F1C45
    const ShipAiApproachPoint goal = host.brain_goal_0b2c();         // 009F1C6A
    state.goal_range_11e0 = ship_ai_approach_goal_range_009f1bc0(
        goal.x, goal.z, unit_pos.x, unit_pos.z); // 009F1CDE

    host.unit_is_kind_vtable_005c(8); // 009F1D0F, saved for the later arms
    const float class_radius = static_cast<float>(
        static_cast<double>(host.shipclass_radius_0500()) * kApproachClassRadiusScale);
    const float turn_radius = static_cast<float>(
        static_cast<double>(host.unit_turn_radius_00811a30(kApproachThrottleSeedY0)) *
        kApproachTurnRadiusScale); // 009F1D41
    // 009F1D53, FCOMIP then JBE: the larger of the two.
    state.turn_radius_11f0 = (turn_radius > class_radius) ? turn_radius : class_radius;

    state.retarget_timer_11d8 = static_cast<float>(
        static_cast<double>(state.retarget_timer_11d8) - seconds); // 009F1D84
    // 009F1D8C, FCOMIP of 0.0 against the timer then JBE.
    if (!(0.0f <= state.retarget_timer_11d8)) {
        state.flag_11d6 = false; // 009F1DAA
        state.retarget_timer_11d8 = host.random_stream1_00bd2f10(
            kApproachAvoidRefreshLow, kApproachAvoidRefreshHigh); // 009F1DB4
    }
    state.timer_121c = static_cast<float>(
        static_cast<double>(state.timer_121c) - seconds); // 009F1E1E

    // 009F1E36..009F1F45, the approach point. Without a target zone, or when
    // the unit's own avoid-zone group is at or above the target's, the point is
    // the brain's goal vector copied verbatim.
    bool displaced = false;
    if (has_target && has_zone &&
        host.unit_zone_group_0570() < host.target_zone_group_vtable_002c()) {
        host.unit_avoid_radius_0082adc0(); // 009F1E77, result unused by the call below
        const ShipAiAttackMoveXZ exit =
            host.zone_exit_point_00417b10(unit_pos, kApproachZoneQueryRadius);
        state.point_1228.x = exit.x; // 009F1EA2
        state.point_1228.y = 0.0f;   // 009F1EAD
        state.point_1228.z = exit.z; // 009F1EB5
        const float dx = static_cast<float>(
            static_cast<double>(state.point_1228.x) - goal.x);
        const float dz = static_cast<float>(
            static_cast<double>(state.point_1228.z) - goal.z);
        const float moved = static_cast<float>(static_cast<double>(dx) * dx +
                                               static_cast<double>(dz) * dz);
        // 009F1F03, FCOMIP of the squared offset against 1.0 then JBE.
        displaced = (moved > 1.0f);
    } else {
        state.point_1228 = goal; // 009F1F2D..009F1F3D
    }
    (void)displaced; // 009F1F65 feeds the mode latch, which is outside this range
}

} // namespace bsp
