#include "bsp/plane_flight.hpp"

#include <cmath>

// Reconstruction of the plane flight controller. docs/PLANE_FLIGHT.md carries the
// evidence and the coverage table; every name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef and 00415510 BSP_Math_MinFloatByRef, the two
// helpers the native laws call instead of writing the comparison inline.
float max_float(float a, float b) { return a < b ? b : a; }
float min_float(float a, float b) { return b < a ? b : a; }

// 00BF7420, the CRT float-to-int conversion 007BB6E0 uses. MSVC's _ftol
// truncates toward zero.
int ftol(float v) { return static_cast<int>(v); }

}  // namespace

int dynamics_mirror_to_tuning_offset(std::uint32_t mirror_address) {
    // 007EAADC loads EDI with 00F872F0 and 007E2D03 loads ESI with tuning+210h, so
    // the bias between the two windows is 00F872F0 - 210h = 00F870E0.
    return static_cast<int>(mirror_address - (kDynamicsMirrorBase -
                                              static_cast<std::uint32_t>(kDynamicsMirrorTuningBase)));
}

bool dynamics_mirror_contains(std::uint32_t mirror_address) {
    return mirror_address >= kDynamicsMirrorBase &&
           mirror_address < kDynamicsMirrorBase + static_cast<std::uint32_t>(kDynamicsMirrorBytes);
}

PlaneControlLatch latch_control_input_007b9770(const PlaneControlInput& live) {
    PlaneControlLatch out;
    out.roll = live.roll;          // 007B9770 FLD [ECX+9E4h] -> 007B9783 FSTP [ECX+BB0h]
    out.pitch = live.pitch;        // 007B9789 -> 007B979C
    out.yaw = live.yaw;            // 007B97A2 -> 007B97A8
    out.throttle = live.throttle;  // 007B97AE -> 007B97BA
    out.aux = live.aux;            // 007B97C6 -> 007B97CC
    // The three bytes are permuted, not copied straight across.
    out.byte_c8 = live.byte_f8;  // 007B9776 MOVZX EAX,[ECX+9F8h] -> 007B978F
    out.byte_c9 = live.byte_fa;  // 007B977D MOV DL,[ECX+9FAh]   -> 007B97B4
    out.byte_ca = live.byte_f9;  // 007B9795 MOVZX EAX,[ECX+9F9h] -> 007B97C0
    return out;
}

float quantize_control_axis_007bb6e0(float value) {
    // 007BB6E6..007BB72A for the first axis; the other four repeat the block with
    // 007BB730, 007BB763 and the two that follow. ST3 holds 127.0 (00CFD408) and
    // ST4 holds 128.5 (00D05998) for the whole body.
    const int q = ftol(value * kPlaneControlQuantSteps + kPlaneControlQuantBias);
    if (q >= 0xFF) {
        return 1.0f;  // 007BB710 FLD ST1, the FLD1 pushed at 007BB701
    }
    if (q <= 1) {
        return -1.0f;  // 007BB719 FLD ST0, the -1.0 pushed at 007BB708
    }
    return static_cast<float>(q - 0x80) / kPlaneControlQuantSteps;
}

float heading_command_009f9e40(float target_x, float target_z, float unit_x, float unit_z) {
    // 009F9E5D..009F9EA5. The x87 order is FLD dz, FLD dx, CALL atan2, so the
    // library sees y = dz and x = dx.
    const float dz = target_z - unit_z;
    const float dx = target_x - unit_x;
    float heading = kPlaneQuarterTurn - std::atan2(dz, dx);
    if (heading < 0.0f) {
        heading += kPlaneFullTurn;  // 009F9E9F FADD qword [00CE3828]
    }
    return heading;
}

float pitch_command_009fb800(const PlanePitchCommandInputs& in) {
    // 009FB809..009FB82D: the ceiling clamp comes first and uses the same
    // Dynamics/Ceiling minus 50 that 009FBA50 computes.
    const float altitude = min_float(in.desired_altitude, in.ceiling - kPlaneCeilingMargin);

    // 009FB849 FLD [ESP+10h] / FSUB [EDI+100h]: the error against the world Y.
    const float error = altitude - in.unit_world_y;

    // 009FB858..009FB86E: err * ((reference + 1) * 0.5).
    const float weighted = error * ((in.reference + 1.0f) * 0.5f);

    if (weighted > 0.0f) {
        // 009FB88D..009FB94B, the climb arm.
        const float limit = max_float(in.class_climb_angle * kPlaneAngleLimitScale,
                                      kPlaneClimbAngleFloor);
        float t = weighted / in.climb_dist;  // 009FB8CC FDIV [EAX+544h]
        if (t < 0.0f) {
            t = 0.0f;  // 009FB8E0
        } else if (t > in.reference) {
            t = in.reference;  // 009FB8F3
        }
        return min_float(in.class_climb_angle * t, limit);  // 009FB918 / 009FB92C
    }

    // 009FB96E..009FBA4D, the dive arm. DropAngle carries both the gain and the cap,
    // and the result is negated through 00D7A208 (-0.0f).
    const float limit = max_float(in.class_drop_angle * kPlaneAngleLimitScale,
                                  kPlaneDiveAngleFloor);
    float t = -weighted / in.drop_dist;  // 009FB9B8 FCHS then FDIV [EAX+548h]
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > in.reference) {
        t = in.reference;
    }
    return -min_float(in.class_drop_angle * t, limit);
}

PlaneCruiseAltitudeResult cruise_altitude_command_009fba50(const PlaneCruiseAltitudeInputs& in) {
    PlaneCruiseAltitudeResult out;

    // 009FBA51..009FBA77: the range term never goes negative.
    const float span = max_float(in.range_high - in.range_low, 0.0f);

    // 009FBA82 FLD [EAX+210h] / FSUB qword [00CE3938].
    float ceiling_limit = in.ceiling - kPlaneCeilingMargin;
    if (in.has_squadron) {
        // 009FBA9B..009FBABF: the squadron's own limit wins when it is lower.
        ceiling_limit = min_float(ceiling_limit, in.squadron_limit);
    }

    float altitude = in.base_altitude;
    if (span > 0.0f) {
        // 009FBAD2..009FBAE5: span * scale * class+518h, added to the base.
        altitude += span * in.scale * in.class_gain;
    }

    out.unclamped_altitude = altitude;                        // 009FBB06 FSTP [ESP+4]
    out.clamped_altitude = min_float(altitude, ceiling_limit);  // 009FBAF7 / 009FBB10
    out.ceiling_limit = ceiling_limit;
    out.returned_in_st0 = in.scale;  // the value 009FBAC5 leaves on the x87 stack
    return out;
}

PlaneMotionArm select_motion_arm_007ce040(const PlaneMotionDispatchInputs& in) {
    if (in.control_mode_gate) {
        return PlaneMotionArm::FreeFlight;  // 007CEC43 JZ, so a true answer takes this arm
    }
    if (in.ground_water_mode == 4 || in.ground_water_mode == 5) {
        return PlaneMotionArm::GroundRoll;  // 007CEC7B / 007CEC80
    }
    if (in.surface_mode == 6) {
        return PlaneMotionArm::Surface;  // 007CEC99
    }
    return PlaneMotionArm::None;  // 007CECA0 JNZ skips the pose commit as well
}

PlaneMotionArm run_plane_fixed_step_007ce040(PlaneFlightHost& host, float step) {
    // 007CE08F, the level-4 unit tick. Its own body runs the two virtual arms; the
    // host reports unit+520h back so the ordering stays visible here.
    const bool suppress_slot_1d8 = host.unit_game_object_tick_00953cc0(step);
    host.class_input_poll_0095dc40(step);  // 00953CFD, vtable[+1F0h]
    if (!suppress_slot_1d8) {
        // 00953D7D CMP byte [ESI+210h],0 / 00953D84 JNZ: the slot runs only when
        // unit+520h is clear. docs/TICK_ELEMENT_OVERRIDES.md had this inverted.
        host.out_of_action_countdown_007c6c30(step);  // 00953D98, vtable[+1D8h]
    }

    PlaneMotionDispatchInputs dispatch;
    dispatch.control_mode_gate = host.free_flight_gate_00d06130_38();  // 007CEC3F
    dispatch.ground_water_mode = host.ground_water_mode();
    dispatch.surface_mode = host.surface_mode();

    const PlaneMotionArm arm = select_motion_arm_007ce040(dispatch);
    switch (arm) {
        case PlaneMotionArm::FreeFlight:
            if (!host.airborne_time_frozen()) {
                host.accumulate_airborne_time(step);  // 007CEC4E, gated on unit+9E0h
            }
            host.free_flight_007cc2f0(step);  // 007CEC6E
            break;
        case PlaneMotionArm::GroundRoll:
            host.ground_roll_007cbfa0(step);  // 007CEC92
            break;
        case PlaneMotionArm::Surface:
            host.surface_007cba50(step);  // 007CECAF
            break;
        case PlaneMotionArm::None:
            break;
    }

    if (arm != PlaneMotionArm::None) {
        host.commit_step_pose_0085dc80();  // 007CECBA, on unit+674h
    }

    // 007CE96F, far below the dispatch in listing order: the live control block
    // becomes the previous-step snapshot the rate law 007DA710 reads next step.
    host.latch_control_input_007b9770();
    return arm;
}

}  // namespace bsp
