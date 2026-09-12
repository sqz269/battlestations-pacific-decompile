// Where the ship AI's goal vector comes from, and how a goal becomes a path
// point. Every routine here is a projection of a native body; the address, the
// original ABI, the evidence and the uncertainty are in
// docs/SHIP_AI_GOAL_VECTOR.md and repeated above each routine.
//
// Nothing here is a drop-in binary replacement. The native routines work on
// the brain object and the control block directly; these take the fields the
// listing reads and call a host for every native call site.

#include "bsp/ship_ai_goal_vector.hpp"

#include <cmath>

namespace bsp {
namespace {

// 009F1552 and 009DB893 accumulate in the x87 register file and only the
// FSTP that follows rounds to float32. Both helpers keep that shape: the
// intermediate products stay double, the result is rounded once.
float round_to_float(double value) { return static_cast<float>(value); }

}  // namespace

// 009E2FB0, `ShipAiGoalTargetRecord* __thiscall(record)(descriptor)`, RET 4.
// ECX = the record at brain+0AF8h, the stack argument is the descriptor
// 0071EB60 returned. Sole call site 009F1473.
//
// 009E2FBA reads the byte at descriptor+1h and 009E2FC4 substitutes the zero
// vector at 00F87574 when it is clear, so the position is staged before the
// resolve and survives it. 009E2FF3 compares the resolved object with the one
// already latched and only re-registers the observer pair when they differ.
void ship_ai_latch_command_target_009e2fb0(const ShipAiGoalCommandDescriptor& command,
                                           std::uint32_t resolved_target,
                                           ShipAiGoalTargetRecord& record,
                                           ShipAiGoalVectorHost& host) {
    // 009E2FBF / 009E2FC4: the source of the triple, chosen before the call.
    float x = kShipAiGoalNoPositionX;
    float y = kShipAiGoalNoPositionY;
    float z = kShipAiGoalNoPositionZ;
    if (command.has_position) {
        x = command.x;
        y = command.y;
        z = command.z;
    }

    // 009E2FEE..009E3010: the observer pair follows the latched object.
    if (record.target_object_14 != resolved_target) {
        if (record.target_object_14 != 0) {
            host.observer_unregister_006952a0(record.target_object_14);
        }
        record.target_object_14 = resolved_target;
        if (resolved_target != 0) {
            host.observer_register_00694a60(resolved_target);
        }
    }

    // 009E3012..009E302F: the triple lands whether or not the object changed.
    record.offset_x_18 = x;
    record.offset_y_1c = y;
    record.offset_z_20 = z;
}

// 009DBCC0, `void __thiscall(record)(float out[3])`, RET 4. Sole call site
// 009F156B with ECX = brain+0AF8h.
//
// 009DBCC7 reads the latched object. With none, 009DBD1C takes the stored
// triple as it stands. With one, 009DBCDE..009DBCED passes the stored triple
// as ECX, a stack destination, and the object's matrix at +0CCh to
// 004142E0 BSP_Vector3f_TransformAffinePoint, so the triple is a point in the
// object's frame, not a world position. 009DBCCE checks the object's pose-valid
// byte at +0C8h first and refreshes through 00414DB0 when it is clear.
void ship_ai_resolve_goal_position_009dbcc0(const ShipAiGoalTargetRecord& record,
                                            ShipAiGoalVectorHost& host,
                                            float& out_x, float& out_y, float& out_z) {
    if (record.target_object_14 == 0) {
        out_x = record.offset_x_18;
        out_y = record.offset_y_1c;
        out_z = record.offset_z_20;
        return;
    }

    if (!host.target_pose_valid_00c8(record.target_object_14)) {
        host.refresh_target_pose_00414db0(record.target_object_14);
    }
    host.transform_by_target_matrix_004142e0(record.target_object_14,
                                             record.offset_x_18, record.offset_y_1c,
                                             record.offset_z_20, out_x, out_y, out_z);
}

// 009F1420's goal half, `void __thiscall(brain)(float seconds)`, RET 4.
// Call site 009F516C with ECX = ai+58h and the float at ai+0B18h, the
// accumulated sub-tick delta; 009F515B gates the whole call on ai+0B18h having
// reached ai+0B14h, so this is not a per-frame routine.
//
// Covers 009F1420..009F158A. 009F158A..009F1BB8, the two further countdowns at
// brain+0B48h / +0B50h and the proximity scan they drive, is not projected.
ShipAiGoalRefreshResult ship_ai_refresh_goal_vector_009f1420(ShipAiGoalVectorState& state,
                                                             ShipAiGoalTargetRecord& record,
                                                             float seconds,
                                                             ShipAiGoalVectorHost& host) {
    ShipAiGoalRefreshResult result{};

    // 009F1457: the throttle-ceiling suppression byte starts every tick clear.
    // 009F145E clears brain+3ADh in the same pair of stores; that byte belongs
    // to the state steps and is not modelled here.
    state.speed_commanded_0b38 = false;

    // 009F146B / 009F1473: the active command, latched onto the record.
    const ShipAiGoalCommandDescriptor command = host.active_command_0071eb60();
    const std::uint32_t resolved = host.resolve_command_target_00521ea0();
    ship_ai_latch_command_target_009e2fb0(command, resolved, record, host);

    // 009F1478..009F1499: the raw object, then the same object behind
    // vtable[5Ch](2). Both are kept on the brain.
    state.raw_target_0b20 = record.target_object_14;
    std::uint32_t filtered = record.target_object_14;
    if (filtered == 0 || !host.target_is_kind_vtable_005c(filtered, 2)) {
        filtered = 0;
    }
    state.filtered_target_0b24 = filtered;

    // 009F149F..009F14B6: FCOMI on the countdown against the delta. The
    // constructor seeds the countdown negative, so the first tick expires.
    const bool expired = !(seconds < state.refresh_countdown_0b58);
    result.timer_expired = expired;
    if (!expired) {
        // 009F15B1..009F15B3.
        state.refresh_countdown_0b58 = state.refresh_countdown_0b58 - seconds;
    } else {
        // 009F14BC..009F14C4: the period is added on top of the overshoot, so
        // the schedule does not drift.
        state.refresh_countdown_0b58 =
            (state.refresh_period_0b54 - seconds) + state.refresh_countdown_0b58;
        // 009F14D2: open, then narrowed.
        state.target_visible_0b28 = true;
        if (filtered != 0) {
            // 009F14DB..009F1504: a target on another side has to be one this
            // side's recon actually holds.
            if (host.target_side_0054(filtered) != host.unit_side_0054()) {
                state.target_visible_0b28 =
                    host.recon_knows_target_009dfbe0(host.unit_side_0054(), filtered);
            }
            // 009F150A..009F1522: a surface target reopens the gate.
            if (!state.target_visible_0b28 &&
                host.target_is_surface_00922dc0(state.raw_target_0b20)) {
                state.target_visible_0b28 = true;
            }
        }
    }

    // 009F1529..009F1562: refresh when the gate is open, and also whenever the
    // stored goal is shorter than one unit, which is how an unset goal keeps
    // asking. The squared length is accumulated at x87 precision and rounded
    // once by the store at 009F1554.
    bool refresh = state.target_visible_0b28;
    if (!refresh) {
        const double gx = static_cast<double>(state.goal_x_0b2c);
        const double gy = static_cast<double>(state.goal_y_0b30);
        const double gz = static_cast<double>(state.goal_z_0b34);
        const float length_sq = round_to_float((gx * gx + gy * gy) + gz * gz);
        refresh = !(kShipAiGoalKeepLengthSq <= length_sq);
    }

    if (refresh) {
        // 009F156B..009F1584.
        ship_ai_resolve_goal_position_009dbcc0(record, host, state.goal_x_0b2c,
                                               state.goal_y_0b30, state.goal_z_0b34);
        result.goal_rewritten = true;
    }
    return result;
}

// 009DB820, `float __thiscall(adapter)(void)`, RET, body 009DB820-009DB8CC.
// 009DB823 reaches the brain through adapter+4h and 009DB82F the unit through
// brain+0AA8h; 009DB835/009DB854 refresh the unit's pose first. The differences
// at 009DB871 and 009DB87D are pose minus goal, each rounded to float32 by its
// store, and the squared sum is rounded again at 009DB895. The comparison at
// 009DB8A1 is against the double 1e-10 at 00CE3820 and the square root at
// 009DB8A7 goes through the CRT helper 00BF7030.
float ship_ai_goal_planar_distance_009db820(float goal_x, float goal_z,
                                            float pose_x, float pose_z) {
    const float dx = pose_x - goal_x;
    const float dz = pose_z - goal_z;
    const float distance_sq =
        round_to_float(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    if (!(static_cast<double>(distance_sq) > kShipAiGoalDistanceEpsilonSq)) {
        return 0.0f;  // 009DB8BC
    }
    return round_to_float(std::sqrt(static_cast<double>(distance_sq)));
}

// 009EE580..009EE670, inside BSP_ShipAi_ControlsStep (body 009ED6B0-009EF228).
// Reached only when the station-keeping arm at 009EDA28 did not run: that arm
// leaves through 009EE57B JMP 009EF206 and never falls through, which is why
// EDI still holds the literal 2 that 009ED7EC put there when 009EE580 compares
// it against the steering mode.
//
// The caller owns the write-back of the record's point into blk+0A94h /
// +0A9Ch; that is the output block in src/ship_ai_navigation.cpp.
ShipAiPathPickResult ship_ai_pick_path_point_009ee580(const ShipAiPathPickState& state,
                                                      float seconds,
                                                      ShipAiPathPointRecord& record,
                                                      float& blk_speed_scale_39c,
                                                      ShipAiPathPickHost& host) {
    ShipAiPathPickResult result{};

    // 009EE580..009EE59F: the three flags clear before the gate decides.
    result.entered = (state.steering_mode_1c4 == 2) || state.astern;
    if (!result.entered) {
        return result;
    }

    // 009EE5BA: the path branch pins the slot the station-keeping arm computes.
    blk_speed_scale_39c = kShipAiPathPickSpeedScale;

    // 009EE5C2: the goal latched on blk+1DCh becomes a plan.
    host.refresh_path_plan_009ed3e0(seconds);

    // 009EE5C7..009EE5F4: the plan is asked for the point nearest the pose.
    record.query_x_00 = state.pose_x_184;
    record.query_z_04 = state.pose_z_188;
    host.next_path_point_009e3c00(record);

    // 009EE600: no node means no publish. The output block still runs on the
    // record's stale point, which is what 009EE609 jumps to.
    if (record.node_18 == 0) {
        return result;
    }
    result.point_found = true;

    // 009EE60B..009EE630: the band's floor, raised to the path's own width for
    // one direction code only.
    float low = kShipAiPathPublishLowFloor;
    if (record.direction_1c == 1) {
        const float width = host.path_width_2f4_08();
        if (low < width) {
            low = width;
        }
    }

    // 009EE63A..009EE64C: the band's top, one x87 multiply rounded once.
    const float high = round_to_float(
        static_cast<double>(host.path_node_width_20(record.node_18)) * kShipAiPathPublishHighScale);

    // 009EE66C.
    host.publish_lateral_offset_00815f30(record.node_18, record.direction_1c, low, high);
    result.published = true;
    result.publish_low = low;
    result.publish_high = high;
    return result;
}

// 007ADC30, `bool __thiscall(slot)(void)`, body 007ADC30-007ADC50, call site
// 009E59F8. 007ADC33..007ADC3E is the compiler's `slot+4h != 0` test written
// as NEG/SBB/TEST against a constant; 007ADC45 calls the command's vtable[0Ch]
// and 007ADC49 answers false only when that count is positive.
bool ship_ai_command_slot_has_no_legs_007adc30(bool slot_has_command, int leg_count) {
    if (!slot_has_command) {
        return true;  // 007ADC4E
    }
    return leg_count <= 0;
}

}  // namespace bsp
