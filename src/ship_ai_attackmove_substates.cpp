// The five attackmove sub-state steps and the three predicates that pick them.
//
// Packet cc_ai_attackmove_substates, worker agent/cc-ai-attackmove-substates.
// Evidence, address by address, in docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md.
// Ghidra was read-only for this packet; every descriptive name is a hypothesis.
//
// Every float expression below was taken from the listing, not from the
// decompiler. Where the image computes in x87 80-bit registers with a double
// operand the reconstruction uses `double` and rounds to float at the same
// store the image does; that is the closest a C++ projection gets, and it is
// noted in the doc's Uncertainties section.

#include "bsp/ship_ai_attackmove_substates.hpp"

#include <cmath>

namespace bsp {

namespace {

// The wrap idiom 009E56BF..009E56E5 and 00605070 share: a single conditional
// add or subtract of a full turn, not a loop.
float wrap_once_to_half_turn(float a) {
    if (!(kAttackMoveRingWrapLow < static_cast<double>(a))) {
        return static_cast<float>(static_cast<double>(a) + kAttackMoveRingFullTurn);
    }
    if (static_cast<double>(a) > kAttackMoveRingWrapHigh) {
        return static_cast<float>(static_cast<double>(a) - kAttackMoveRingFullTurn);
    }
    return a;
}

// 009E56ED..009E5712 and 009E25F8..009E2616: the bearing-to-heading change of
// basis, then one conditional add of a full turn when the result went negative.
float heading_from_bearing(float bearing) {
    float h = static_cast<float>(kAttackMoveRingQuarterTurn - static_cast<double>(bearing));
    if (!(0.0f <= h)) {
        h = static_cast<float>(static_cast<double>(h) + kAttackMoveRingFullTurn);
    }
    return h;
}

} // namespace

// ---------------------------------------------------------------------------
// 00852860
// ---------------------------------------------------------------------------
bool ship_ai_attackmove_altitude_gate_00852860(float world_y, float ceiling_a,
                                               float ceiling_b) {
    // 00852871 FLD +100h, 00852877 FLD +1204h, 0085287D FADD +1200h,
    // 00852884 FDIV double 3.0, 0085288A FXCH, 0085288C FCOMIP ST0,ST1 with
    // ST0 = world_y. The JBE at 00852890 is the TRUE side once the
    // 00852892..008528AC logical NOT is applied.
    const double limit =
        (static_cast<double>(ceiling_a) + static_cast<double>(ceiling_b)) /
        kAttackMoveAltitudeGateDivisor;
    return !(static_cast<double>(world_y) > limit);
}

// ---------------------------------------------------------------------------
// 009E85B0
// ---------------------------------------------------------------------------
bool ship_ai_attackmove_engage_gate_009e85b0(ShipAiAttackMoveEngageGateHost& host) {
    if (host.brain_unit_0aa8() == 0) {
        return false; // 009E85BF, 009E85C1
    }

    float ready_a = 0.0f;
    float ready_b = 0.0f;
    host.armament_readiness_0510(ready_a, ready_b);
    // 009E85D8 JA takes the pass side on the first float; 009E85E8 JBE fails on
    // the second. Neither comparison is an `abs`, so a negative value fails.
    if (!(ready_a > 0.0f) && !(ready_b > 0.0f)) {
        return false;
    }

    if (!host.unit_pose_valid_00c8()) {
        host.refresh_unit_pose_00414db0(); // 009E85FC, 009E8600
    }
    float unit_x = 0.0f;
    float unit_z = 0.0f;
    host.unit_position_xz_00fc(unit_x, unit_z); // 009E8605, 009E861E
    float dest_x = 0.0f;
    float dest_z = 0.0f;
    host.brain_destination_0b2c(dest_x, dest_z); // 009E8610, 009E862C

    if (host.avoid_zone_containing_004178f0(dest_x, dest_z) != 0) {
        return false; // 009E865D, 009E8660
    }

    // 009E8662..009E869C. The deltas are unit minus destination; the slot that
    // held the destination x is reused for the squared length at 009E868A.
    const float dx = unit_x - dest_x;
    const float dz = unit_z - dest_z;
    const float distance_sq = dx * dx + dz * dz;
    return kAttackMoveEngageGateRangeSq > static_cast<double>(distance_sq);
}

// ---------------------------------------------------------------------------
// 009E5530, one ring slot
// ---------------------------------------------------------------------------
ShipAiAttackMoveRingSlot ship_ai_attackmove_ring_slot_009e5530(int index,
                                                               std::uint32_t unit,
                                                               std::uint32_t nested) {
    ShipAiAttackMoveRingSlot slot{};
    slot.unit_00 = unit;     // 009E5692
    slot.nested_04 = nested; // 009E5695

    // 009E5680..009E56AA. The product is stored to a float slot at 009E56A0
    // before the fmod, so the rounding happens there and not at the end.
    const float spoke = static_cast<float>(
        (static_cast<double>(index) / kAttackMoveRingDivisor) * kAttackMoveRingFullTurn);
    float bearing = static_cast<float>(
        std::fmod(static_cast<double>(spoke), kAttackMoveRingFullTurn));
    bearing = wrap_once_to_half_turn(bearing);
    slot.angle_08 = bearing; // 009E56F9

    const float heading = heading_from_bearing(bearing);
    slot.dir_x_0c = std::cos(heading); // 009E571E, 009E5734
    slot.dir_y_10 = 0.0f;              // 009E573C
    slot.dir_z_14 = std::sin(heading); // 009E572E, 009E574A

    slot.reset_44 = kAttackMoveRingScoreReset; // 009E55B1
    slot.jitter_48 = 0.0f;                     // 009E55A1, a random draw, not modelled
    return slot;
}

// ---------------------------------------------------------------------------
// 009F3240, the approach sub-state
// ---------------------------------------------------------------------------
float ship_ai_attackmove_approach_throttle_009f3240(float depth_ref, float bias) {
    // 009F32A6 FLD +494h, 009F32AC FADD double 500.0, 009F32B2 FSUB bias,
    // 009F32B6 FADD ST0,ST0, 009F32B8 store to a float slot.
    const float doubled = static_cast<float>(
        2.0 * ((static_cast<double>(depth_ref) + kAttackMoveApproachDepthOffset) -
               static_cast<double>(bias)));
    // 009F32C2 FCOMIP with 0.0 on top: the JBE side is `0.0 <= doubled`.
    if (!(0.0f <= doubled)) {
        return 0.0f; // 009F32C6
    }
    // 009F32D5 compares doubled against 1000.0; above the knee the value is
    // replaced by the separate float constant at 00CE3804, also 1000.0.
    if (static_cast<double>(doubled) > kAttackMoveApproachThrottleKnee) {
        return kAttackMoveApproachThrottleCap; // 009F32DB
    }
    return doubled; // 009F32E5
}

float ship_ai_attackmove_approach_command_009f3240(float raw) {
    if (kAttackMoveApproachCommandLow > raw) {
        return kAttackMoveApproachCommandLow; // 009F3618, 009F3623
    }
    if (raw > kAttackMoveApproachCommandHigh) {
        return kAttackMoveApproachCommandHigh; // 009F362D, 009F3630
    }
    return raw; // 009F3632
}

bool ship_ai_attackmove_approach_sweep_speed_gate_009f3240(float speed,
                                                           float reference_speed) {
    // 009F34F9 widens the body speed to double, 009F3502 FDIVR divides it by
    // the reference speed, 009F350C compares 0.4 against the ratio.
    const double ratio = static_cast<double>(speed) / static_cast<double>(reference_speed);
    return kAttackMoveApproachSweepSpeedRatio > ratio;
}

bool ship_ai_attackmove_approach_sweep_range_gate_009f3240(
    float candidate_x, float candidate_z, float target_x, float target_z,
    std::int32_t target_radius_07c4) {
    const float dx = candidate_x - target_x; // 009F3549, 009F354D
    const float dz = candidate_z - target_z; // 009F3555, 009F3559
    const float distance_sq = dx * dx + dz * dz;
    // 009F3546 IMUL, 009F357D FILD: the radius is squared as a 32-bit integer
    // and only then converted, so the product wraps exactly as the image does.
    const std::int32_t radius_sq = target_radius_07c4 * target_radius_07c4;
    return static_cast<float>(radius_sq) > distance_sq; // 009F3581, 009F3585
}

void ship_ai_attackmove_approach_step_009f3240(float seconds,
                                               ShipAiAttackMoveApproachHost& host) {
    const std::uint32_t target = host.brain_target_0b20(); // 009F3262
    if (target == 0 || host.target_retired_005d(target)) { // 009F326A, 009F3277
        host.hold_heading_and_stop_009e00a0();             // 009F3647
        return;
    }

    host.nested_update_009f3090(seconds); // 009F328F

    const float throttle = ship_ai_attackmove_approach_throttle_009f3240(
        host.unit_depth_reference_0494(), host.sub_throttle_bias_11e8());

    ShipAiAttackMoveXZ goal{};
    host.sub_goal_1230(goal.x, goal.z);                     // 009F3300, 009F32EB
    const float heading = host.sub_heading_command_1214();  // 009F3314
    host.set_navigation_goal_009de050(goal, 1, 0);          // 009F332B

    if (host.brain_steering_mode_01cc() != 3) {             // 009F3335
        host.clear_brain_turn_accumulators_0368();          // 009F3340, 009F3348
    }
    host.set_brain_heading_01e0(heading);   // 009F335C
    host.wrap_brain_heading_00605070();     // 009F3360
    host.set_brain_replan_01d4(0);          // 009F336B
    host.set_brain_steering_mode_01cc(3);   // 009F3375
    host.set_brain_throttle_0258(throttle); // 009F337B, 009F3383

    const float command = host.sub_throttle_command_1218(); // 009F339A
    const float timer = host.sub_sweep_timer_14b4() - seconds;
    host.set_sub_sweep_timer_14b4(timer); // 009F338B, 009F33A8

    // 009F33AE re-reads brain+0B20h rather than reusing `target`.
    const std::uint32_t sweep_target_raw = host.brain_target_0b20();
    std::uint32_t sweep_target = 0;
    if (sweep_target_raw != 0 &&
        host.target_is_kind_vtable_005c(sweep_target_raw, 0x1C)) { // 009F33BF
        sweep_target = host.brain_target_0b20();                   // 009F33C7
    }

    // 009F33E2 runs the sweep only while the timer is strictly negative.
    if (!(0.0f <= timer) && sweep_target != 0 &&
        !host.unit_and_target_share_side_0054(sweep_target)) { // 009F33F1, 009F3405
        host.set_sub_sweep_timer_14b4(1.0f);                   // 009F340B

        // 009F3488 indexes a 40-slot stack array; the loop bound is the group
        // count, so a group larger than 40 would overrun in the image. The
        // reconstruction stops at the array size and the doc records the gap.
        std::uint32_t candidates[40] = {};
        int count = 0;
        bool sweep = false;

        if (host.unit_is_group_leader_00778890()) { // 009F3429
            const int members = host.group_member_count_04f8(); // 009F3444
            if (members >= 1) {                                 // 009F344E
                for (int i = 0; i < members; ++i) {
                    std::uint32_t member = host.group_member_at_0070d060(i); // 009F3457
                    if (member == 0 ||
                        !host.member_is_kind_vtable_005c(member, 6)) { // 009F346B
                        member = 0;                                    // 009F3471
                    }
                    // 009F3473 reads [member+538h] even when the kind test
                    // nulled `member`, which is a null read in the image. The
                    // reconstruction skips instead of faulting; see the doc.
                    if (member == 0) {
                        continue;
                    }
                    if (host.member_armament_ready_vtable_002c(member) && // 009F347E
                        count < static_cast<int>(sizeof(candidates) / sizeof(candidates[0]))) {
                        candidates[count] = member; // 009F3488
                        ++count;
                    }
                }
                sweep = count >= 1; // 009F349F
            }
        } else if (host.unit_armament_ready_vtable_002c()) { // 009F35E3, 009F35E7
            candidates[0] = host.brain_unit_0aa8();          // 009F35F1
            count = 1;                                       // 009F35F5
            sweep = true;
        }

        if (sweep) {
            float target_x = 0.0f;
            float target_z = 0.0f;
            host.target_position_xz_00427eb0(sweep_target, target_x, target_z); // 009F34B3

            for (int i = 0; i < count; ++i) { // 009F34E0
                const std::uint32_t candidate = candidates[i];
                const float speed = host.candidate_body_speed_0092d730(candidate);   // 009F34EA
                const float reference = host.candidate_reference_speed_0080fc30(candidate); // 009F34FD
                if (!ship_ai_attackmove_approach_sweep_speed_gate_009f3240(speed, reference)) {
                    continue; // 009F3510
                }
                if (!host.candidate_pose_valid_00c8(candidate)) {  // 009F351D
                    host.refresh_candidate_pose_00414db0(candidate); // 009F3521
                }
                float candidate_x = 0.0f;
                float candidate_z = 0.0f;
                host.candidate_position_xz_00fc(candidate, candidate_x, candidate_z); // 009F3526
                const std::int32_t radius = host.target_warn_radius_07c4(sweep_target); // 009F3534
                if (!ship_ai_attackmove_approach_sweep_range_gate_009f3240(
                        candidate_x, candidate_z, target_x, target_z, radius)) {
                    continue; // 009F3585
                }
                if (!host.candidate_accepts_warning_vtable_0234(candidate, sweep_target)) {
                    continue; // 009F3592, 009F3596
                }
                host.route_warning_message_0077c2a0(candidate); // 009F35B3
            }
        }
    }

    host.set_brain_goal_hold_01d0(0); // 009F361C
    host.set_brain_command_01d8(
        ship_ai_attackmove_approach_command_009f3240(command)); // 009F3635
    host.set_brain_replan_01d4(0);                              // 009F363D
}

// ---------------------------------------------------------------------------
// 009E23B0, the engage sub-state
// ---------------------------------------------------------------------------
float ship_ai_attackmove_engage_lead_time_009e23b0(float range, float closing) {
    // 009E250F puts 0.4 on top and compares it against the closing speed; the
    // JBE side keeps the measured value, so this is a floor, not a switch.
    const float divisor =
        (kAttackMoveEngageClosingGate <= static_cast<double>(closing))
            ? closing
            : kAttackMoveEngageClosingFloor;
    // 009E2525 FLD range, 009E252F FDIV divisor, 009E2533 FSUB double 2.0.
    float lead = static_cast<float>(
        static_cast<double>(range / divisor) - kAttackMoveEngageLeadBias);
    // 009E2543 compares 0.0 against the lead; the fall-through arm zeroes it.
    if (!(0.0f <= lead)) {
        return 0.0f; // 009E254D
    }
    // 009E264F caps the lead; a NaN takes neither JA and falls through the same
    // store the in-range value uses, which this ordering reproduces.
    if (lead > kAttackMoveEngageLeadCap) {
        lead = kAttackMoveEngageLeadCap; // 009E2658
    }
    return lead;
}

bool ship_ai_attackmove_engage_step_009e23b0(ShipAiAttackMoveEngageState& state,
                                             ShipAiAttackMoveEngageHost& host) {
    if (host.brain_target_0b20() == 0) {    // 009E23C4
        host.hold_heading_and_stop_009e00a0(); // 009E26B2
        return state.attack_run_08;
    }

    if (!host.target_pose_valid_00c8()) {
        host.refresh_target_pose_00414db0(); // 009E23D7, 009E23DB
    }
    float target_x = 0.0f;
    float target_z = 0.0f;
    host.target_position_xz_00fc(target_x, target_z); // 009E23E2, 009E23FE

    if (!host.unit_pose_valid_00c8()) {
        host.refresh_unit_pose_00414db0(); // 009E240C, 009E2410
    }
    float unit_x = 0.0f;
    float unit_z = 0.0f;
    host.unit_position_xz_00fc(unit_x, unit_z); // 009E2415, 009E241F

    const float dx = target_x - unit_x; // 009E2429
    const float dz = target_z - unit_z; // 009E2435
    const float distance_sq = dx * dx + dz * dz; // 009E2441..009E2451
    // 009E245F compares the squared range against 1e-10 and only then calls the
    // square root helper; below the epsilon the range is a hard zero.
    const float range = (static_cast<double>(distance_sq) > kAttackMoveEngageRangeEpsilonSq)
                            ? std::sqrt(distance_sq) // 009E2465
                            : 0.0f;                  // 009E2478
    // 009E248F divides both deltas by the range with no zero guard, so the
    // zero-range frame yields infinities in the image. The reconstruction keeps
    // the division; the doc records it as an observed hazard.
    const float nx = dx / range; // 009E2491
    const float nz = dz / range; // 009E2499

    // 009E2483 and 009E24A9: the run latch drops as soon as the range passes
    // the exit distance, and only while the latch is already set.
    if (state.attack_run_08 && range > kAttackMoveEngageRunExitRange) {
        state.attack_run_08 = false; // 009E24B2
    }

    float target_vx = 0.0f;
    float target_vz = 0.0f;
    host.target_velocity_xz_vtable_0034(target_vx, target_vz); // 009E24CA
    float unit_vx = 0.0f;
    float unit_vz = 0.0f;
    host.unit_velocity_xz_vtable_0034(unit_vx, unit_vz); // 009E24DB

    // 009E24DD..009E2501: the relative velocity is unit minus target, projected
    // on the unit-to-target unit vector.
    const float closing = (unit_vx - target_vx) * nx + (unit_vz - target_vz) * nz;
    const float lead = ship_ai_attackmove_engage_lead_time_009e23b0(range, closing);

    // 009E2562 fetches the target velocity a second time rather than reusing
    // the pair it already has.
    float lead_vx = 0.0f;
    float lead_vz = 0.0f;
    host.target_velocity_xz_vtable_0034(lead_vx, lead_vz);

    host.set_avoidance_side_03f8(host.unit_side_0054()); // 009E2581, 009E2588

    ShipAiAttackMoveXZ intercept{};
    intercept.x = target_x + lead_vx * lead; // 009E2594, 009E25A8
    intercept.z = target_z + lead_vz * lead; // 009E259C, 009E25B4

    if (state.attack_run_08) { // 009E25BC
        host.set_avoidance_flag_03fc(false); // 009E25CC

        const float run_dx = intercept.x - unit_x; // 009E25C8
        const float run_dz = intercept.z - unit_z; // 009E25DB
        // 009E25E3 loads dz first and 009E25E7 loads dx, so the helper at
        // 009E25EB receives ST1 = dz and ST0 = dx: atan2(dz, dx), a bearing in
        // the maths convention, which the next two steps rotate into the game's
        // from-+Z heading.
        const float bearing = std::atan2(run_dz, run_dx);
        host.set_heading_and_drop_path_009dff40(heading_from_bearing(bearing)); // 009E2620
        host.set_brain_speed_scale_0af0(1.0f);                                  // 009E262F
        return state.attack_run_08;
    }

    host.set_avoidance_flag_03fc(true);                       // 009E2669
    host.set_navigation_goal_009de050(intercept, 0, 0);       // 009E267B
    if (host.plan_accepts_goal_009da610(intercept) &&         // 009E268A, 009E2691
        kAttackMoveEngageRunEnterRange > static_cast<double>(range)) { // 009E269D
        state.attack_run_08 = true;                           // 009E26A3
    }
    return state.attack_run_08;
}

// ---------------------------------------------------------------------------
// 009E26C0, the lead-pursuit sub-state
// ---------------------------------------------------------------------------
ShipAiAttackMoveXZ ship_ai_attackmove_lead_point_009e26c0(float target_x,
                                                          float target_y,
                                                          float target_z,
                                                          float target_vx,
                                                          float target_vz) {
    // 009E275F divides by a double -10.0 and 009E276F rounds the quotient to a
    // float slot before the floor comparison at 009E278E.
    const float raw = static_cast<float>(static_cast<double>(target_y) /
                                         kAttackMoveLeadTimeDivisor);
    const float lead = (static_cast<double>(kAttackMoveLeadTimeFloor) <= static_cast<double>(raw))
                           ? raw                        // 009E27A4
                           : kAttackMoveLeadTimeFloor;  // 009E279A
    ShipAiAttackMoveXZ point{};
    point.x = target_x + lead * target_vx; // 009E27BB, 009E27CD
    point.z = target_z + lead * target_vz; // 009E27C5, 009E27D9
    return point;
}

float ship_ai_attackmove_lead_arrival_radius_009e26c0(float turn_radius,
                                                      float heading_error,
                                                      bool budget_mode) {
    // 009E2853 FMUL double 1.5, 009E2867 compares 300.0 against the product.
    float radius = static_cast<float>(static_cast<double>(turn_radius) *
                                      kAttackMoveLeadRadiusScale);
    if (!(static_cast<double>(kAttackMoveLeadRadiusFloor) <= static_cast<double>(radius))) {
        radius = kAttackMoveLeadRadiusFloor; // 009E286D
    }
    // 009E28B3: the magnitude is taken as (-0.0f - d), not as fabs.
    const float magnitude = (heading_error > 0.0f) ? heading_error : (-0.0f - heading_error);
    // 009E28D8 and 009E28DD: direct mode, or an error at or above the gate.
    if (!budget_mode || !(kAttackMoveLeadHeadingErrorGate > magnitude)) {
        radius = kAttackMoveLeadRadiusDirect; // 009E28DF
    }
    return radius;
}

void ship_ai_attackmove_lead_pursuit_step_009e26c0(
    ShipAiAttackMoveLeadPursuitState& state, float seconds,
    ShipAiAttackMoveLeadPursuitHost& host) {
    const std::uint32_t target = host.brain_target_0b20(); // 009E26CE
    if (target == 0) {                                     // 009E26D4
        host.hold_heading_and_stop_009e00a0();             // 009E26DA
        return;
    }

    if (!host.unit_pose_valid_00c8()) {
        host.refresh_unit_pose_00414db0(); // 009E26F6, 009E26FA
    }
    float unit_x = 0.0f;
    float unit_z = 0.0f;
    host.unit_position_xz_00fc(unit_x, unit_z); // 009E2706, 009E2714

    if (!host.target_pose_valid_00c8()) {
        host.refresh_target_pose_00414db0(); // 009E2722, 009E2726
    }
    float target_x = 0.0f;
    float target_y = 0.0f;
    float target_z = 0.0f;
    host.target_position_00fc(target_x, target_y, target_z); // 009E2732, 009E2740, 009E2757

    float target_vx = 0.0f;
    float target_vz = 0.0f;
    host.target_velocity_xz_vtable_0034(target_vx, target_vz); // 009E2773

    ShipAiAttackMoveXZ lead = ship_ai_attackmove_lead_point_009e26c0(
        target_x, target_y, target_z, target_vx, target_vz);
    host.clamp_to_world_box_009db6c0(lead, kAttackMoveTangentBoxInset); // 009E27EF

    const float dx = lead.x - unit_x; // 009E27F4
    const float dz = lead.z - unit_z; // 009E2800
    // 009E280C loads dx then 009E2810 loads dz, so the helper at 009E2814 gets
    // ST1 = dx and ST0 = dz: atan2(dx, dz), already a from-+Z heading.
    const float desired = std::atan2(dx, dz);

    if (!state.budget_mode_0c) {
        // 009E283F. The result is popped without a store at 009E2844, so the
        // call has no effect; it is kept because it is an observable call.
        (void)host.add_wrapped_angle_00438aa0(desired,
                                              static_cast<float>(kAttackMoveRingWrapHigh));
    }

    const float turn_radius = host.ship_class_turn_radius_0082e850(); // 009E284E
    const float heading = host.unit_heading_vtable_0050();            // 009E2890
    const float error = host.subtract_wrapped_angle_00438b10(desired, heading); // 009E28A0
    const float radius = ship_ai_attackmove_lead_arrival_radius_009e26c0(
        turn_radius, error, state.budget_mode_0c);

    const float distance_sq = dx * dx + dz * dz; // 009E28ED..009E2910
    const float magnitude = (error > 0.0f) ? error : (-0.0f - error);
    // 009E2918 JA takes the goal arm when the squared range exceeds the squared
    // arrival radius. 009E2922 JBE falls to the heading arm while the error is
    // at or under the gate; above the gate 009E2929 sends direct mode to the
    // goal arm and leaves budget mode on the heading arm.
    const bool use_goal =
        (distance_sq > radius * radius) ||
        (magnitude > kAttackMoveLeadHeadingErrorGate && !state.budget_mode_0c);

    if (use_goal) {
        host.set_brain_goal_hold_01d0(1);             // 009E2A41
        host.set_navigation_goal_009de050(lead, 0, 0); // 009E2A53
    } else {
        const float heading_now = host.unit_heading_vtable_0050(); // 009E293C
        const float error_now =
            host.subtract_wrapped_angle_00438b10(desired, heading_now); // 009E294C
        // 009E295D seeds the command slot with `desired`; only a heading error
        // outside +-1 rad replaces it, and only then are the two extra calls at
        // 009E2993 and 009E2999 made.
        float command = desired; // 009E2951, 009E295D
        if (error_now > kAttackMoveLeadHeadingRateLimit) { // 009E2967, 009E2969
            command = host.add_wrapped_angle_00438aa0(
                host.unit_heading_vtable_0050(), kAttackMoveLeadHeadingRateLimit);
        } else if (-kAttackMoveLeadHeadingRateLimit > error_now) { // 009E2975, 009E297A
            command = host.add_wrapped_angle_00438aa0(
                host.unit_heading_vtable_0050(), -kAttackMoveLeadHeadingRateLimit);
        }
        host.set_desired_heading_009e0040(command); // 009E29AC

        const float error_magnitude =
            (error_now > 0.0f) ? error_now : (-0.0f - error_now); // 009E29CE
        if (state.budget_mode_0c) {
            host.set_brain_speed_scale_0af0(host.interpolate_clamped_00419010(
                kAttackMoveLeadScaleNearAngle, kAttackMoveLeadScaleNear,
                kAttackMoveLeadScaleFarAngle, kAttackMoveLeadScaleFar,
                error_magnitude)); // 009E2A0E, 009E2A1F
        } else {
            host.set_brain_speed_scale_0af0(kAttackMoveLeadScaleNear); // 009E2A33
        }
    }

    if (state.budget_mode_0c) {
        const float speed = host.unit_forward_speed_vtable_0038();      // 009E2A85
        const float rudder = host.unit_ordered_rudder_0984();           // 009E2A6A
        const float yaw = host.yaw_rate_from_rudder_0082ecb0(rudder, speed, 1.0f); // 009E2A97
        const float turned = yaw * seconds;                             // 009E2A9C
        const float magnitude_turned = (turned > 0.0f) ? turned : (-0.0f - turned);
        state.turn_budget_08 += magnitude_turned; // 009E2AD0, 009E2ADB
        if (static_cast<double>(state.turn_budget_08) > kAttackMoveLeadBudgetFullTurn) {
            state.budget_mode_0c = false; // 009E2AEF
        }
        return;
    }

    const float arm = state.arm_distance_10;                                  // 009E2AFC
    const float distance = host.vector2_length_00414c60(dx, dz);              // 009E2B0D
    const float arm_radius = host.ship_class_turn_radius_0082e850();          // 009E2B18
    if (distance > arm_radius * arm) {                                        // 009E2B2D
        state.budget_mode_0c = true;                                          // 009E2B38
        state.arm_distance_10 = static_cast<float>(static_cast<double>(arm) *
                                                   kAttackMoveLeadRadiusScale); // 009E2B3D
        state.turn_budget_08 = 0.0f;                                          // 009E2B41
    }
}

// ---------------------------------------------------------------------------
// 009F3670, the tangent-circle sub-state
// ---------------------------------------------------------------------------
float ship_ai_attackmove_tangent_radius_009f3670(float turn_radius, float jitter) {
    // 009F3763 puts the double 500.0 on top and 009F3784 keeps the larger.
    const float radius =
        (kAttackMoveTangentRadiusFloorD <= static_cast<double>(turn_radius))
            ? turn_radius
            : kAttackMoveTangentRadiusFloor;
    return jitter * radius; // 009F37C6
}

bool ship_ai_attackmove_tangent_uses_goal_009f3670(float distance_sq, float speed) {
    // 009F38C0 FMUL double 1.5, 009F38CA FMUL ST0 squares it, 009F38E8 keeps
    // the smaller of that and the cap.
    const float scaled = static_cast<float>(static_cast<double>(speed) *
                                            kAttackMoveTangentSpeedScale);
    const float squared = scaled * scaled;
    const float threshold =
        (kAttackMoveTangentSwitchCapSq <= squared) ? kAttackMoveTangentSwitchCapSq : squared;
    return distance_sq > threshold; // 009F38FE, 009F3906
}

void ship_ai_attackmove_tangent_step_009f3670(ShipAiAttackMoveTangentState& state,
                                              float seconds,
                                              ShipAiAttackMoveTangentHost& host) {
    // 009F3670..009F3684, before the null-target test.
    state.elapsed_10 += seconds;

    const std::uint32_t target = host.brain_target_0b20(); // 009F3687
    if (target == 0) {
        host.hold_heading_and_stop_009e00a0(); // 009F3692
        return;
    }

    const float release_delay = host.settings_weapon_release_delay_04d4(); // 009F369F
    if (state.elapsed_10 > release_delay) { // 009F36B1
        const std::uint32_t director = host.unit_director_vtable_0114(); // 009F36C3
        if (director != 0 && host.director_stage_0030(director) != 2) {  // 009F36D9
            const float second_delay = host.settings_weapon_release_delay_04d4(); // 009F36E1
            if (state.elapsed_10 >
                static_cast<float>(static_cast<double>(second_delay) +
                                   kAttackMoveTangentReleaseDelay)) { // 009F36F3
                host.raise_command_stage_0071e430(host.unit_director_vtable_0114(),
                                                  kAttackMoveTangentCommandObject,
                                                  1); // 009F3714, 009F3718
            }
        }
    }

    if (!host.unit_pose_valid_00c8()) {
        host.refresh_unit_pose_00414db0(); // 009F3726, 009F3731
    }
    float unit_x = 0.0f;
    float unit_z = 0.0f;
    host.unit_position_xz_00fc(unit_x, unit_z); // 009F3736, 009F373E

    const float turn_radius = host.ship_class_turn_radius_0082e850(); // 009F375A
    const float jitter = host.random_range_00bd2f10(kAttackMoveTangentJitterLow,
                                                    kAttackMoveTangentJitterHigh); // 009F37C1
    const float radius = ship_ai_attackmove_tangent_radius_009f3670(turn_radius, jitter);

    float centre_x = 0.0f;
    float centre_z = 0.0f;
    host.brain_destination_0b2c(centre_x, centre_z); // 009F379E, 009F37B1

    // 009F37EE..009F380C: the minimum step is the smaller of the radius and
    // 300. Because the radius floor is 500 and the jitter is at least 1.2, the
    // radius never drops below 600, so this min always yields 300 in play.
    const float min_step = (kAttackMoveTangentMinStepD <= static_cast<double>(radius))
                               ? kAttackMoveTangentMinStep
                               : radius;

    ShipAiAttackMoveXZ point{};
    host.circle_tangent_point_009d68b0(centre_x, centre_z, radius, unit_x, unit_z,
                                       min_step, point);              // 009F3829
    host.clamp_to_world_box_009db6c0(point, kAttackMoveTangentBoxInset); // 009F383F

    const float dx = point.x - unit_x; // 009F3844
    const float dz = point.z - unit_z; // 009F385D
    const float distance_sq = dx * dx + dz * dz; // 009F386F..009F3885

    // 009F3869 reads the current heading unconditionally; 009F3889 only replaces
    // it with the bearing to the point when the point is more than one unit away.
    float heading = host.unit_heading_vtable_0050();
    if (distance_sq > kAttackMoveTangentHeadingEpsilonSq) {
        heading = std::atan2(dx, dz); // 009F3895
    }

    const float speed = host.unit_armament_speed_00a0(); // 009F38BA
    if (ship_ai_attackmove_tangent_uses_goal_009f3670(distance_sq, speed)) {
        host.set_brain_goal_hold_01d0(1);               // 009F390A
        host.set_navigation_goal_009de050(point, 0, 0); // 009F3920
        host.set_brain_speed_scale_0af0(kAttackMoveTangentGoalScale); // 009F392F
    } else {
        host.set_desired_heading_009e0040(heading);                     // 009F3943
        host.set_brain_speed_scale_0af0(kAttackMoveTangentHeadingScale); // 009F3952
    }

    // 009F395F COMISS then 009F3966 JC: a negative budget leaves at once.
    if (state.budget_0c < 0.0f) {
        return;
    }
    const float range = host.range_to_destination_009db820(); // 009F396A
    // 009F3982 JA: the range has grown past the entry range plus a slack.
    // 009F398E JBE: otherwise it must have fallen under the re-entry range.
    const bool grown = static_cast<double>(range) >
                       static_cast<double>(state.dwell_timer_08) +
                           kAttackMoveTangentReEnterSlack;
    const bool closed = !(kAttackMoveTangentReEnterRange <= static_cast<double>(range));
    if (!grown && !closed) {
        return; // 009F3992
    }
    // 009F3998 and 009F39A4 are both inside the gate.
    state.budget_0c -= seconds;
    host.notify_siblings_009e2b60();
}

} // namespace bsp
