#include "bsp/plane_ground_ops.hpp"

#include <cmath>

// Reconstruction of the plane's ground, water, takeoff and landing behaviour.
// docs/PLANE_GROUND_OPS.md carries the evidence and the coverage table; every
// routine below names the native site it projects.

namespace bsp {
namespace {

float max_float(float a, float b) { return a < b ? b : a; }
float min_float(float a, float b) { return b < a ? b : a; }

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x): a clamped linear ramp
// from (x0, y0) to (x1, y1). The native routine is x87; this mirrors its shape.
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) {
    if (x1 == x0) {
        return x <= x0 ? y0 : y1;
    }
    const float t = (x - x0) / (x1 - x0);
    if (t <= 0.0f) {
        return y0;
    }
    if (t >= 1.0f) {
        return y1;
    }
    return y0 + t * (y1 - y0);
}

}  // namespace

const char* flight_state_debug_label_007ccb30(int state) {
    // The jump table at 007CCBC0; the handler each entry reaches pushes one string.
    switch (state) {
        case 0: return "N/A plane state";  // 007CCB48 -> 00D05E74
        case 1: return "Inside";           // 007CCB57 -> 00D05E6C
        case 2: return "Locked";           // 007CCB66 -> 00D05E64
        case 3: return "huh?";             // 007CCBB1 -> 00D05E34, the out-of-range arm
        case 4: return "Runway";           // 007CCB75 -> 00D05E5C
        case 5: return "Runway on Path";   // 007CCB84 -> 00D05E4C
        case 6: return "Water";            // 007CCB93 -> 00D05E44
        case 7: return "Flying";           // 007CCBA2 -> 00D05E3C
        default: return nullptr;           // 007CCB3F `ja` reaches the same "huh?"
    }
}

PlaneGroundMotionArm select_motion_arm_007cec30(bool control_mode_gate, int flight_state) {
    if (control_mode_gate) {
        return PlaneGroundMotionArm::FreeFlight;  // 007CEC43 falls through to 007CEC6E
    }
    if (flight_state == static_cast<int>(PlaneFlightState::Runway) ||
        flight_state == static_cast<int>(PlaneFlightState::RunwayOnPath)) {
        return PlaneGroundMotionArm::GroundRoll;  // 007CEC7B / 007CEC80
    }
    if (flight_state == static_cast<int>(PlaneFlightState::Water)) {
        return PlaneGroundMotionArm::Water;  // 007CEC99, [ESI+5F0h] with ESI = unit+310h
    }
    return PlaneGroundMotionArm::None;
}

PlaneSetStateEffects set_flight_state_007c1430(const PlaneSetStateInputs& in) {
    PlaneSetStateEffects out;

    // 007C1434-007C143C: the throttle zeroing happens before the equality check, so
    // a redundant request for state 2 still cuts the throttle.
    out.zero_throttle = in.requested_state == static_cast<int>(PlaneFlightState::Locked);

    if (in.current_state == in.requested_state) {
        out.state = in.current_state;  // 007C144E returns 0 and stores nothing else
        return out;
    }

    out.changed = true;
    out.state = in.requested_state;
    out.write_state_change_stamp = true;  // 007C146A, -1.0f
    out.notify = true;                    // 007C11E0(0) on every arm

    switch (in.requested_state) {
        case static_cast<int>(PlaneFlightState::Launching):  // 007C1532
            out.force_full_throttle = true;
            break;

        case static_cast<int>(PlaneFlightState::Runway):        // 007C147F, shared
        case static_cast<int>(PlaneFlightState::RunwayOnPath):  // 007C1558 -> 007C147F
            if (in.class_min_water_spd != 0.0f) {
                out.write_landed_after_flight = true;
                out.landed_after_flight =
                    in.airborne_clock > kPlaneGroundLandedClockThreshold;  // 007C14A2
                out.write_state_enter_marker = true;                       // 007C14BB, 3
            }
            break;

        case static_cast<int>(PlaneFlightState::Water):  // 007C150A
            out.clear_byte_910 = true;
            out.clear_landed_after_flight = in.landed_after_flight;
            out.clear_water_accumulator = true;
            break;

        case static_cast<int>(PlaneFlightState::Flying):  // 007C14D0
            out.clear_byte_910 = true;
            out.clear_landed_after_flight = in.landed_after_flight;
            out.write_airborne_clock = true;
            // 007C14E4 / 007C14E9: only Runway and Launching reset the clock.
            out.airborne_clock =
                (in.current_state == static_cast<int>(PlaneFlightState::Runway) ||
                 in.current_state == static_cast<int>(PlaneFlightState::Launching))
                    ? 0.0f
                    : kPlaneGroundSpawnedAirborneClock;
            break;

        default:
            break;  // 007C1542, the shared tail for 0, 1, 2 and anything above 7
    }
    return out;
}

PlaneSpawnPlacement choose_spawn_placement_007c6340(const PlaneSpawnPlacementInputs& in) {
    PlaneSpawnPlacement out;
    if (in.current_state == static_cast<int>(PlaneFlightState::Launching)) {
        out.launch_arm = true;  // 007C6347: clear the pose bytes and return
        out.state = PlaneFlightState::Launching;
        return out;
    }
    // 007C636A: the height probe against the class reference, or any class with a
    // planing speed, forces the airborne placement.
    const bool airborne =
        (in.class_height_reference - kPlaneGroundLandedYawScale) <= in.height_probe ||
        in.class_min_water_spd != 0.0f;
    if (airborne) {
        out.state = PlaneFlightState::Flying;                 // 007C63F4
        out.airborne_clock = kPlaneGroundSpawnedAirborneClock;  // 007C6417
    } else {
        out.state = PlaneFlightState::Water;  // 007C6481
        out.airborne_clock = 0.0f;
    }
    return out;
}

PlaneBeginFlyingEffects begin_flying_007c7110(const PlaneBeginFlyingInputs& in) {
    PlaneBeginFlyingEffects out;
    if (in.net_mode == kPlaneGroundNetModeClient) {
        // 007C711A: the client only repairs the controller when the free-flight gate
        // is false; it never notifies the surface.
        out.run_client_fallback = !in.free_flight_gate;
    } else if ((in.current_state == static_cast<int>(PlaneFlightState::Runway) ||
                in.current_state == static_cast<int>(PlaneFlightState::RunwayOnPath)) &&
               in.has_ground_contact_owner) {
        out.notify_surface_of_departure = true;  // 007C7154
    }
    if (in.current_state != static_cast<int>(PlaneFlightState::Flying)) {
        out.changed = true;
        out.airborne_clock =
            (in.current_state == static_cast<int>(PlaneFlightState::Runway) ||
             in.current_state == static_cast<int>(PlaneFlightState::Launching))
                ? 0.0f
                : kPlaneGroundSpawnedAirborneClock;
    }
    return out;
}

PlaneGroundLawResult ground_roll_law_007dccf0(const PlaneGroundLawInputs& in) {
    PlaneGroundLawResult out;
    // 007DCCF9 / 007DCD01: without ground contact only RunwayOnPath keeps the ground
    // law; every other state falls back to the free-flight step 007DC830.
    if (!in.ground_contact &&
        in.flight_state != static_cast<int>(PlaneFlightState::RunwayOnPath)) {
        return out;
    }
    out.took_ground_arm = true;                     // 007DCDC6 returns 1
    out.mode = PlaneControllerMode::GroundRoll;     // 007DCD24, ctl+FCh = 1
    out.clear_mode_bytes = true;                    // 007DCD2E / 007DCD31
    out.core_law_arg_a = in.state_is_locked ? 0.0f : 1.0f;  // 007DCD34
    out.ground_frame[0] = 0.0f;                     // 007DCD9x
    out.ground_frame[1] = std::cos(in.class_ground_pitch);
    out.ground_frame[2] = std::sin(in.class_ground_pitch);
    out.ground_frame[3] = in.tuning_runway_smooth_strength;  // 007DCDB9
    return out;
}

bool runway_steer_gate_007cc0c9(const PlaneGroundSteerGateInputs& in) {
    // 007CC0CF / 007CC0D3: a plane that has just landed, or one being driven along a
    // path, has no steering authority at all.
    bool gate = !in.landed_after_flight &&
                in.flight_state != static_cast<int>(PlaneFlightState::RunwayOnPath);
    // 007CC0F5: a squadron with the override byte set leaves the gate as it stands.
    if (!in.has_squadron || !in.squadron_override) {
        gate = gate && (in.tuning_player_control_spd < in.speed);  // 007CC126 AND
    }
    return gate;
}

PlaneWreckFuseResult wreck_fuse_007cbff0(float fuse, float step, bool steer_gate) {
    PlaneWreckFuseResult out;
    out.fuse = fuse - step;  // 007CC001-007CC00B
    if (!steer_gate) {
        // 007CC013-007CC02B: clamped down, so losing authority shortens the fuse.
        out.fuse = min_float(out.fuse, kPlaneGroundLandedClockThreshold);
    }
    out.explode = !(0.0f < out.fuse);  // 007CC036 COMISS with zero, JC to continue
    return out;
}

PlaneRunwayYawResult runway_yaw_factor_007da540(const PlaneRunwayYawInputs& in) {
    PlaneRunwayYawResult out;
    // 007DA563: at or above the limit the runway band does not apply.
    if (!(in.forward_speed < in.tuning_yaw_turn_spd_limit_dup)) {
        return out;
    }
    out.runway_band_applies = true;
    const float scaled = in.class_yaw_spd * in.tuning_yaw_turn_spd_mul;  // 007DA590
    const float inverse = in.class_yaw_spd == 0.0f
                              ? 0.0f
                              : kPlaneGroundYawTurnNumerator / in.class_yaw_spd;  // 007DA586
    float rate = max_float(scaled, inverse);                                       // 007DA5A2
    if (in.class_is_10h_or_16h && in.landed_after_flight) {
        rate *= kPlaneGroundLandedYawScale;  // 007DA5DC
    }
    out.yaw_rate = rate;
    out.blend = interpolate_clamped(in.tuning_yaw_turn_spd_limit, in.forward_speed,
                                    in.tuning_yaw_turn_spd_limit_dup, 1.0f,
                                    in.forward_speed);  // 007DA5E6-007DA611
    return out;
}

PlaneWireRopeResult wire_rope_band_007dbeee(const PlaneWireRopeInputs& in) {
    PlaneWireRopeResult out;
    out.brake_term = in.wheel_brake * in.load_max;  // 007DBEF6-007DBF0C
    out.accumulator = in.accumulator;
    if (!in.brake_flag) {
        out.accumulator = 0.0f;  // 007DC085
        out.reset = true;
        return out;
    }
    if (!(kPlaneGroundLiftOffEpsilon < in.accumulator)) {
        out.held = true;  // 007DBF22 leaves ctl+ACh untouched
        return out;
    }
    // 007DBFE4-007DC015: the wire pulls proportionally to the ground track speed.
    out.accumulator += in.ground_speed_xz * in.scale * in.tuning_wire_rope;
    // 007DC01C 00415510 BSP_Math_MinFloatByRef(&MaxWireRope, &ctl+ACh).
    out.accumulator = min_float(out.accumulator, in.tuning_max_wire_rope);
    if (in.has_ground_contact_owner) {
        // 007DC04E-007DC081: a shorter deck brakes harder.
        const float deck = interpolate_clamped(
            kPlaneGroundWireDeckShortLen, kPlaneGroundWireDeckShortMul,
            kPlaneGroundWireDeckLongLen, kPlaneGroundWireDeckLongMul, in.owner_deck_length);
        out.accumulator *= deck;
    }
    return out;
}

PlaneLiftOffAction lift_off_request_007cc1b3(const PlaneLiftOffInputs& in) {
    if (in.flight_state == static_cast<int>(PlaneFlightState::RunwayOnPath)) {
        return PlaneLiftOffAction::None;  // 007CC1BA
    }
    const float height = in.ground_height - in.class_ground_reference;  // 007CC1CC
    if (in.net_mode != kPlaneGroundNetModeClient &&
        kPlaneGroundLiftOffEpsilon < height &&
        kPlaneGroundLiftOffEpsilon < in.vertical_rate) {
        return PlaneLiftOffAction::SendTakeoffMessage;  // 007CC212
    }
    if (in.ground_contact) {
        return PlaneLiftOffAction::None;  // 007CC245
    }
    if (in.owner_is_class_9) {
        return PlaneLiftOffAction::SendTakeoffMessage;  // 007CC26E, off the deck edge
    }
    if (in.net_mode != kPlaneGroundNetModeClient) {
        return PlaneLiftOffAction::StampPendingByte;  // 007CC2AD
    }
    return PlaneLiftOffAction::None;
}

bool water_impact_is_splash_007cbb91(const PlaneWaterImpactInputs& in) {
    return in.speed > in.class_max_water_spd ||
           in.vertical_rate < -in.tuning_water_max_vspd ||
           in.pitch < -in.tuning_water_max_down_pitch ||
           in.pitch > in.tuning_water_max_up_pitch ||
           std::fabs(in.roll) > in.tuning_water_max_roll;
}

PlaneWaterDrownResult water_drown_test_007cbd79(const PlaneWaterDrownInputs& in) {
    PlaneWaterDrownResult out;
    const float lift_speed = in.tuning_level_flight_mul * in.class_stall_spd;  // 007CBD7F
    const float blend = interpolate_clamped(in.tuning_normal_yaw_control_spd, 1.0f,
                                            lift_speed, in.interpolate_y1,
                                            in.speed);  // 007CBD95-007CBDBD
    out.threshold = in.tuning_water_max_depth * blend;   // 007CBDCC
    out.engine_drowns = out.threshold < in.law_depth;    // 007CBDE0 JBE continues
    return out;
}

float water_accumulator_step_007cbe7a(float accumulator, float law_depth, float speed,
                                      float class_min_water_spd) {
    const float depth = law_depth + kPlaneGroundLandedYawScale;  // 007CBE7A, + 0.5
    if (!(0.0f < depth)) {
        return accumulator;  // 007CBE9B
    }
    if (!(speed < class_min_water_spd)) {
        return accumulator;  // 007CBEB5
    }
    const float shortfall = 1.0f - speed / class_min_water_spd;  // 007CBEB7-007CBEBD
    return accumulator + depth * shortfall;                      // 007CBEC5
}

PlaneWaterTakeoffAction water_takeoff_request_007cbed9(int flight_state, int net_mode) {
    if (flight_state == static_cast<int>(PlaneFlightState::Flying)) {
        return PlaneWaterTakeoffAction::None;  // 007CBEED
    }
    if (net_mode != kPlaneGroundNetModeClient) {
        return PlaneWaterTakeoffAction::SendStateChangeMessage;  // 007CBF04 falls through
    }
    if (flight_state == static_cast<int>(PlaneFlightState::Water) ||
        flight_state == static_cast<int>(PlaneFlightState::Runway) ||
        flight_state == static_cast<int>(PlaneFlightState::RunwayOnPath)) {
        return PlaneWaterTakeoffAction::SetFlyingDirectly;  // 007CBF68
    }
    return PlaneWaterTakeoffAction::None;
}

PlaneGroundStepOutcome run_ground_roll_step_007cbfa0(PlaneGroundOpsHost& host, float step) {
    host.run_pre_pass_007c5ac0(step);  // 007CBFC3

    const bool landed = host.landed_after_flight();
    if (!landed) {
        host.arm_ground_subsystem_007cbfd2();  // 007CBFC8 entry pass
    }

    if (host.out_of_action()) {  // 007CBFF0, unit+5Dh
        // The gate the fuse reads is the one the previous step left behind.
        const PlaneWreckFuseResult fuse =
            wreck_fuse_007cbff0(host.wreck_fuse(), step, host.runway_steer_gate());
        host.set_wreck_fuse(fuse.fuse);
        if (fuse.explode) {
            const int mode = host.net_mode();
            if (mode != 0 && mode != 1) {
                return PlaneGroundStepOutcome::FuseExpiredQuiet;  // 007CC05D
            }
            host.raise_effect("explosion");  // 007CC063, 00D05A18
            return PlaneGroundStepOutcome::Exploded;
        }
    }

    host.set_runway_steer_gate(runway_steer_gate_007cc0c9(host.steer_gate_inputs()));

    host.clamp_controls_007caf10(step);  // 007CC13F

    const bool locked = host.flight_state() == static_cast<int>(PlaneFlightState::Locked);
    host.run_ground_law_007dccf0(step, locked);  // 007CC15E

    if (landed) {
        if (host.surface_still_holds_007cc186()) {
            host.run_surface_hold_007b8da0();  // 007CC18E
        }
    } else {
        host.arm_ground_subsystem_007cbfd2();  // 007CC195 exit pass
    }

    switch (lift_off_request_007cc1b3(host.lift_off_inputs())) {
        case PlaneLiftOffAction::SendTakeoffMessage:
            host.send_takeoff_message_00762a00();
            return PlaneGroundStepOutcome::RequestedTakeoff;
        case PlaneLiftOffAction::StampPendingByte:
            host.stamp_pending_byte_007cc2ad();
            return PlaneGroundStepOutcome::StampedPending;
        case PlaneLiftOffAction::None:
            break;
    }
    return PlaneGroundStepOutcome::RanLaw;
}

}  // namespace bsp
