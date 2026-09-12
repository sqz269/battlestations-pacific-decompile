// Reconstruction of the ship AI state steps. See include/bsp/ship_ai_state_steps.hpp
// and docs/SHIP_AI_STATE_STEPS.md. Every routine is a projection of the listing
// named in its comment; the descriptive names are hypotheses, not symbols.
#include "bsp/ship_ai_state_steps.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 009DE050, body 009DE050-009DE1A6, complete.
// __thiscall(blk)(const float* goal2d, char keep_mode, unsigned char final_leg),
// RET 0Ch. The three stack arguments are proved by the RET 0Ch and the loads at
// 009DE063 ([ESP+18h]), 009DE087 ([ESP+1Ch]) and 009DE139 ([ESP+20h]).
// ---------------------------------------------------------------------------
void ship_ai_set_navigation_goal_009de050(ShipAiGoalPlan& blk, float goal_x, float goal_z,
                                          bool keep_mode, bool final_leg,
                                          ShipAiGoalHost& host) {
    bool replan = false; // BL, zeroed at 009DE05D

    blk.throttle_hold_1c8 = 1;                                  // 009DE067
    blk.requested_direction = ShipAiThrottleDirection::Stopped;  // 009DE071

    if (blk.mode == ShipAiSteeringMode::Navigate ||              // 009DE05F
        blk.mode == ShipAiSteeringMode::NavigateAstern) {        // 009DE07D
        // How far the goal moved since the last call. One x87 chain with a
        // single store at 009DE0C9, so the sum is rounded to float32 once.
        const float mx = goal_x - blk.goal_x_1dc;                // 009DE0A0
        const float mz = goal_z - blk.goal_z_1e0;                // 009DE0AC
        const float moved_sq = mx * mx + mz * mz;                // 009DE0C1..009DE0C9
        if (kShipAiGoalMoveResetDistanceSq < moved_sq) {         // 009DE0CD, JBE 009DE0DB
            blk.crossing_314 = 0.0f;                             // 009DE0E0
            blk.flag_2fd = false;                                // 009DE0E8
            blk.flag_2fe = false;                                // 009DE0EF
        }
        // How far the goal moved since the path was planned. The float32 sum is
        // widened for the compare against the double at 00D21530.
        const float px = goal_x - blk.planned_x_1e8;             // 009DE0F6
        const float pz = goal_z - blk.planned_z_1ec;             // 009DE102
        const float planned_sq = px * px + pz * pz;              // 009DE117..009DE11F
        if (static_cast<double>(planned_sq) > kShipAiGoalReplanDistanceSq) { // 009DE123, JBE 009DE131
            replan = true;                                       // 009DE133
        }
    } else {
        host.clear_path_plan_009da4e0();                         // 009DE082
        if (!keep_mode) {                                        // 009DE087, JNZ 009DE135
            blk.mode = ShipAiSteeringMode::Navigate;             // 009DE091
            replan = true;                                       // 009DE133 via 009DE09B
        }
    }

    blk.goal_x_1dc = goal_x;                                     // 009DE137
    blk.goal_z_1e0 = goal_z;                                     // 009DE143
    blk.final_leg_1e4 = final_leg;                               // 009DE14C
    blk.mode = ShipAiSteeringMode::Navigate;                     // 009DE152

    if (replan && !keep_mode) {                                  // 009DE135/15C, 009DE15E
        blk.planned_x_1e8 = goal_x;                              // 009DE165
        blk.planned_z_1ec = goal_z;                              // 009DE171
        // 009DE17A: the distance from the pose the block carries to the goal.
        blk.plan_length_1f0 =
            host.planar_length_00414c60(goal_x - blk.pose_x_184,  // 009DE17C
                                        goal_z - blk.pose_z_188); // 009DE189, call 009DE193
    }
}

// ---------------------------------------------------------------------------
// 009DA4E0, body 009DA4E0-009DA58D, complete. __fastcall(blk), RET 0.
// ---------------------------------------------------------------------------
void ship_ai_clear_path_plan_009da4e0(ShipAiPathPlan& blk, ShipAiPathPlanHost& host) {
    if (blk.path_object_244 != 0) {                    // 009DA4E3
        host.release_path_object_vtable_0000(blk.path_object_244); // 009DA4E9
    }
    const std::uint32_t limit = host.path_limit_default_00cf58ec(); // 009DA4FB, [00CF58EC]
    blk.count_248 = 0;
    blk.path_object_244 = 0;
    blk.index_258 = 0;
    blk.value_25c = 0;
    blk.limit_254 = limit;
    blk.cursor_240 = 0;
    if (blk.path_object_2ac != 0) {                    // 009DA51E
        host.release_path_object_vtable_0000(blk.path_object_2ac); // 009DA524
    }
    blk.limit_2bc = limit;
    blk.count_2b0 = 0;
    blk.path_object_2ac = 0;
    blk.value_2c0 = 0;
    blk.value_2c4 = 0;
    blk.cursor_2a8 = 0;
    blk.flag_2fd = false;
    blk.flag_2fc = false;
    blk.flag_2fe = false;
    blk.value_250 = 0;
    blk.value_2b8 = 0;
}

// ---------------------------------------------------------------------------
// 009DFF40, body 009DFF40-009DFF91. __thiscall(brain)(float), RET 4. ECX is the
// brain itself here (LEA ESI,[ECX+8] at 009DFF41), not &state->owner as the
// three setters of bsp/ship_ai_states.hpp take it.
// ---------------------------------------------------------------------------
void ship_ai_set_heading_drop_path_009dff40(ShipAiControlBlock& blk, float heading,
                                            ShipAiHeadingHoldHost& host) {
    if (blk.mode != ShipAiSteeringMode::Heading) {  // 009DFF49
        blk.timer_368 = 0.0f;                       // 009DFF54
        blk.timer_360 = 0.0f;                       // 009DFF5C
        blk.mode = ShipAiSteeringMode::Heading;     // 009DFF64
    }
    host.clear_path_plan_009da4e0();                // 009DFF6C
    blk.desired_heading = heading;                  // 009DFF7D
    host.after_heading_stored_00605070(heading);    // 009DFF81
    blk.requested_direction = ShipAiThrottleDirection::Stopped; // 009DFF86
}

// ---------------------------------------------------------------------------
// 009E00A0, body 009E00A0-009E0126. __thiscall(&state->owner)(), RET 0.
// ---------------------------------------------------------------------------
void ship_ai_hold_heading_and_stop_009e00a0(ShipAiControlBlock& blk,
                                            ShipAiHeadingHoldHost& host) {
    const float heading = host.unit_heading_vtable_0050(); // 009E00B2, stored at 009E00B4
    if (blk.mode != ShipAiSteeringMode::Heading) {  // 009E00C5
        blk.timer_368 = 0.0f;                       // 009E00CD
        blk.timer_360 = 0.0f;                       // 009E00D5
        blk.mode = ShipAiSteeringMode::Heading;     // 009E00DD
    }
    host.clear_path_plan_009da4e0();                // 009E00E5
    blk.desired_heading = heading;                  // 009E00F6
    host.after_heading_stored_00605070(heading);    // 009E00FA
    blk.requested_direction = ShipAiThrottleDirection::Stopped; // 009E0104
    blk.throttle_hold_1c8 = 0;                      // 009E0110
    blk.desired_throttle = 0.0f;                    // 009E0116
    blk.requested_direction = ShipAiThrottleDirection::Stopped; // 009E011E
}

// ---------------------------------------------------------------------------
// 009E14C0, body 009E14C0-009E1605, complete. __thiscall(state)(float), RET 4.
// The float argument is never read; the routine's only time source is the unit's
// own body-axis speed.
// ---------------------------------------------------------------------------
void ship_ai_stop_step_009e14c0(ShipAiStopStepState& state, ShipAiControlBlock& blk,
                                ShipAiAvoidanceRequest& request, ShipAiStopStepHost& host) {
    if (!host.unit_pose_valid_00c8()) {           // 009E14D7
        host.refresh_unit_pose_00414db0();        // 009E14E1
    }
    float x = 0.0f, y = 0.0f, z = 0.0f;
    host.unit_position_00fc(x, y, z);             // 009E14EC, PUSH EBP = unit+0FCh
    if (host.position_outside_world_bounds_0071c4f0(x, y, z)) { // 009E14F3, JZ 009E1527
        // Outside the world box the `stop` state steers back to the origin
        // instead of stopping: the goal pushed at 009E1506 is the zeroed pair
        // written at 009E150C and 009E1512.
        host.set_navigation_goal_009de050(0.0f, 0.0f, /*keep_mode=*/false,
                                          /*final_leg=*/true); // 009E1518
        return;                                   // 009E1524
    }

    const float heading = host.unit_heading_vtable_0050(); // 009E1534
    host.set_desired_heading_009e0040(heading);   // 009E153C
    blk.throttle_hold_1c8 = 0;                    // 009E1549
    blk.desired_throttle = 0.0f;                  // 009E154F
    blk.requested_direction = ShipAiThrottleDirection::Stopped; // 009E1557

    if (state.making_way_08) {                    // 009E155D
        const float speed = host.unit_body_axis_speed_0092d730(); // 009E1570
        // |speed|, built as (-0.0f - speed) when speed <= 0, not as fabs.
        const float magnitude = speed > 0.0f ? speed                        // 009E157F, JBE
                                             : kShipAiStopNegativeZero - speed; // 009E158D
        if (static_cast<double>(magnitude) < kShipAiStopSpeedThreshold) {   // 009E15A5, JBE
            state.making_way_08 = false;          // 009E15B1
        }
    }

    if (!state.making_way_08) {                   // 009E15B4
        request.flag_3fc = false;                 // 009E15BB
        request.side_filter_3f8 = -1;             // 009E15C4
        request.enable_3f4 = true;                // 009E15D2
    } else {
        request.flag_3fc = true;                  // 009E15E4
        request.side_filter_3f8 = 3;              // 009E15ED
        request.enable_3f4 = true;                // 009E15FB
    }
}

// ---------------------------------------------------------------------------
// 009E5770, body 009E5770-009E59B7, complete. __thiscall(state)(float), RET 4.
// The float argument is never read.
// ---------------------------------------------------------------------------
bool ship_ai_movetopos_step_009e5770(ShipAiMoveToPosStepHost& host) {
    if (!host.unit_pose_valid_00c8()) {               // 009E579A
        host.refresh_unit_pose_00414db0();            // 009E57A5
    }
    float pos_x = 0.0f, pos_z = 0.0f;
    host.unit_position_xz_00fc(pos_x, pos_z);         // 009E57AA, 009E57C2
    float goal_x = 0.0f, goal_z = 0.0f;
    host.brain_goal_xz_0b2c(goal_x, goal_z);          // 009E57D0, 009E57B4

    const std::uint32_t slot = host.director_command_slot_0071bff0(0); // 009E57ED
    const bool final_leg = host.command_on_final_leg_007adc60(slot);   // 009E57F4

    host.set_navigation_goal_009de050(goal_x, goal_z, /*keep_mode=*/false,
                                      final_leg);     // 009E580F
    const bool reached = host.state_goal_reached_vtable_002c(goal_x, goal_z); // 009E5821

    // 009E5823/009E5829: only a reached goal on the command's final leg finishes
    // the command outright. Every other case falls into the target arm.
    if (!(reached && final_leg)) {
        if (host.director_current_command_0054() != kShipAiMoveToPosCommandObject) {
            return false;                             // 009E5837, JNZ 009E59A3
        }
        const std::uint32_t target = host.resolve_command_target_00521ea0(); // 009E5847
        if (target == 0) {
            return false;                             // 009E584E
        }
        if (!host.target_is_kind_vtable_005c(target, 0x1C)) {
            return false;                             // 009E585F, JZ 009E59A3
        }
        if (!host.target_pose_valid_00c8(target)) {   // 009E5869
            host.refresh_target_pose_00414db0(target); // 009E5874
        }
        float tx = 0.0f, tz = 0.0f;
        host.target_position_xz_00fc(target, tx, tz); // 009E5879, 009E5887
        // 009E5891..009E58B5: the vector is the unit's position minus the
        // target's, stored to float32 component by component.
        const float dx = pos_x - tx;
        const float dz = pos_z - tz;
        const float distance = host.planar_length_00414c60(dx, dz); // 009E58B9
        // 009E58CE: an 80-bit chain, the integer range at target+7A0h minus the
        // float at unit+9C8h, compared once against the float32 distance.
        const double range = static_cast<double>(host.target_range_07a0(target)) -
                             static_cast<double>(host.unit_radius_09c8());
        if (range <= static_cast<double>(distance)) { // 009E58DA, JBE 009E59A3
            return false;
        }
    }

    // 009E58E6-009E59A2, the arrival arm.
    host.message_text_assign_0041e870("finished");          // 009E58EF, 00D09FD8
    host.post_command_message_00984300(kShipAiMoveToPosCommandObject); // 009E595C
    host.release_message_text_00419cc0();                   // 009E597C, 009E5983
    host.end_command_0071e430(kShipAiMoveToPosCommandObject, 1); // 009E5997
    host.hold_heading_and_stop_009e00a0();                  // 009E599E
    return true;
}

// ---------------------------------------------------------------------------
// 009E86F0, body 009E86F0-009E881A, complete. __thiscall(state)(float), RET 4.
// ---------------------------------------------------------------------------
void ship_ai_attackmove_select_009e86f0(ShipAiAttackMoveSelector& selector,
                                        std::uint32_t state_base, float seconds,
                                        ShipAiAttackMoveSelectorHost& host) {
    // 009E8706 FCOMI ST0=seconds, ST1=countdown; JC at 009E8708 takes the
    // count-down arm when seconds is strictly below the countdown.
    if (seconds < selector.countdown_1500) {
        selector.countdown_1500 = selector.countdown_1500 - seconds; // 009E875F
        return;                                                      // 009E8769
    }
    // 009E870A FSUBR, 009E871C FADDP, 009E871E FSTP: one store, so the reload is
    // rounded to float32 once.
    selector.countdown_1500 =
        selector.countdown_1500 + (selector.interval_14fc - seconds);

    const std::uint32_t inner = state_base + 0x0008u;
    const std::uint32_t engage = state_base + 0x14C0u;
    const std::uint32_t target = host.brain_attack_target_0b20(); // 009E8714

    if (target == 0) {                                            // 009E871A, JZ 009E8724
        // The no-target arm does the transition inline instead of through
        // 007B6EE0: exit the current member, store the new one, enter it.
        if (selector.current_1508 == inner) {                     // 009E87F7
            return;
        }
        if (selector.current_1508 != 0) {                         // 009E87FB
            host.substate_exit_vtable_0008(selector.current_1508); // 009E8804
        }
        selector.current_1508 = inner;                            // 009E8806
        host.substate_enter_vtable_0004(inner);                   // 009E8813
        return;
    }

    bool switch_requested = false;
    std::uint32_t wanted = inner;
    if (host.target_is_kind_vtable_005c(target, 8)) {             // 009E8733
        if (host.call_00852860(target)) {                         // 009E873B
            wanted = host.brain_flag_0b28() ? state_base + 0x14CCu  // 009E8756
                                            : state_base + 0x14E0u; // 009E876C
            switch_requested = true;
        } else if (selector.current_1508 != engage) {             // 009E877B, JZ 009E87C2
            wanted = inner;                                       // 009E8783
            switch_requested = true;
        }
    } else if (!host.target_is_kind_vtable_005c(target, 0x1C)) {  // 009E8799
        // 009E87A2..009E87B4: staying put only when the current member is
        // already the inner state or the engage state.
        if (selector.current_1508 != inner && selector.current_1508 != engage) {
            wanted = inner;
            switch_requested = true;
        }
    } else {
        wanted = inner;                                           // 009E87B6
        switch_requested = true;
    }

    if (switch_requested) {
        host.set_current_substate_007b6ee0(wanted);                               // 009E87BD
    }

    // 009E87C2, the tail both arms fall into.
    if (selector.current_1508 == inner && host.call_009e85b0()) { // 009E87C5, 009E87CD
        host.set_current_substate_007b6ee0(engage);                               // 009E87E3
    }
}

// ---------------------------------------------------------------------------
// 009E8820, body 009E8820-009E88F9. __thiscall(state)(float), RET 4. No Ghidra
// function starts there; see the doc's no_ghidra_function table.
// ---------------------------------------------------------------------------
bool ship_ai_attackmove_step_009e8820(float seconds, ShipAiAttackMoveStepHost& host) {
    const std::uint32_t unit = host.brain_unit_0aa8();  // 009E8828
    bool hand_back = unit != 0;                         // 009E882E, JZ 009E88CE
    if (hand_back && !host.entity_is_kind_vtable_005c(unit, 9)) { // 009E883F
        hand_back = false;
    }
    if (hand_back) {
        const std::uint32_t group = host.unit_group_0284(); // 009E8852
        if (group != 0) {                                    // 009E8858
            const int count = host.group_member_count_04f8(group); // 009E8861
            bool all_able = true;                            // 009E885A
            for (int i = 0; i < count; ++i) {                // 009E8867, 009E889C
                const std::uint32_t member = host.group_member_at_0070d060(group, i); // 009E8873
                if (member != unit &&                        // 009E887B
                    !host.entity_is_kind_vtable_005c(member, 9)) { // 009E888C
                    all_able = false;                        // 009E8892
                }
            }
            if (!all_able) {                                 // 009E889E
                hand_back = false;
            }
        }
    }

    if (hand_back) {
        // Every member of the group can do what the command asks, so the state
        // gives the command back to the director instead of steering.
        const std::uint32_t director = host.unit_director_vtable_0114(); // 009E88BD
        // 009E88B8 pushes 0E08F78h, the attackmove command object 009E8540 returns.
        host.end_command_0071e430(director, 0x00e08f78u, 1); // 009E88C1
        return true;                                         // 009E88CB
    }

    // 009E88CE and 009E88DD both re-load the step's own float argument.
    host.select_substate_009e86f0(seconds);   // 009E88D8
    host.substate_step_vtable_000c(seconds);  // 009E88F0
    return false;
}

} // namespace bsp
