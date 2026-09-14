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

// ---------------------------------------------------------------------------
// 007DB680 BSP_PlaneFlight_CoreLaw, the free-flight arm (ctl+FCh == 0).
// docs/PLANE_FREE_FLIGHT_PHYSICS.md carries the per-term derivation.
// ---------------------------------------------------------------------------

namespace {

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), RET 0x14. 0041901E
// FUCOMIP returns y0 when x1 == x0 exactly; otherwise the line, clamped to the
// interval the two endpoints span, in either order.
float interp_clamped_00419010(float x0, float y0, float x1, float y1, float x) {
    if (x1 == x0) return y0;
    const float v = y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
    const float lo = min_float(y0, y1);
    const float hi = max_float(y0, y1);
    return v < lo ? lo : (v > hi ? hi : v);
}

// 00CF9058, the double the lift, gravity and extra-gravity terms all multiply
// by: float 9.81f widened, 40239EB860000000.
constexpr float kGravity00cf9058 = 9.81f;

// The clamp on the lift coefficient, 00CE7D7C = -2.0f and 00CE3958 = +2.0f.
constexpr float kLiftCoeffMin00ce7d7c = -2.0f;
constexpr float kLiftCoeffMax00ce3958 = 2.0f;

// 00D7A3A0, the double 0.1f: below this forward speed the angle of attack is
// pinned to zero instead of dividing by it.
constexpr float kAoaMinForwardSpeed00d7a3a0 = 0.1f;

}  // namespace

float aero_response_curve_007d92b0(float speed_ratio, const PlaneFreeFlightTuning& tuning) {
    // 007D92D2 pushes DragRangeMin as x0 and 007D92C2 DragRangeMax as x1; the
    // two endpoints 007D92CC / 007D92BC are the immediates 0.0f and 1.0f.
    const float u = interp_clamped_00419010(tuning.drag_range_min, 0.0f, tuning.drag_range_max,
                                            1.0f, speed_ratio);
    // 007D92EA FUCOMIP against FLDZ, read through LAHF / TEST AH,44h: JP is the
    // not-equal branch, so an exact zero returns zero and never reaches the pow.
    if (u == 0.0f) return 0.0f;
    // 007D9313 takes |u| the same way the lift does, as -0.0f - u (00D7A208).
    const float magnitude = u < 0.0f ? -u : u;
    return std::pow(magnitude, tuning.drag_func_power);  // 007D9325 FYL2X / F2XM1 / FSCALE
}

float angle_of_attack_007db8b1(float body_vy, float body_vz) {
    // 007DB875..007DB89D: |vz| as vz when vz > 0.0f (00D7A218) and as
    // -0.0f - vz (00D7A208) otherwise, so both signed zeros give +0.0f.
    const float speed = body_vz > 0.0f ? body_vz : (-0.0f - body_vz);
    // 007DB8A7 FCOMIP / JBE: the divide runs when 0.1f <= |vz|.
    if (!(kAoaMinForwardSpeed00d7a3a0 <= speed)) return 0.0f;  // 007DB8AD FLDZ
    return -body_vy / body_vz;                                 // 007DB8B1 FLD / FCHS / FDIV
}

float lift_accel_007db875(const PlaneFreeFlightState& state, const PlaneFreeFlightClass& cls,
                          const PlaneFreeFlightTuning& tuning) {
    const float aoa = angle_of_attack_007db8b1(state.body_velocity[1], state.body_velocity[2]);
    // 007DB773 stored 007D99C0's result already divided by desc+184h StallSpd;
    // 007DB8C1 divides that by LevelFlight and 007DB8CF scales by ctl+9Ch.
    const float q = (state.forward_speed / cls.stall_spd) / tuning.level_flight * state.lift_scale;
    // 007DB8DD..007DB8F1, the two-path square: q*q below 1.0f, 1.0f at or above.
    const float qq = q < 1.0f ? q * q : 1.0f;
    const float coefficient = (1.0f + aoa) * qq;  // 007DB8F1 FADD, 007DB907 FMULP
    // 007DB91D / 007DB961: -2.0f wins when it is greater, then +2.0f caps.
    float clamped = coefficient;
    if (kLiftCoeffMin00ce7d7c > clamped) {
        clamped = kLiftCoeffMin00ce7d7c;
    } else if (clamped > kLiftCoeffMax00ce3958) {
        clamped = kLiftCoeffMax00ce3958;
    }
    // 007DB931 scales by AccelCheatMul, 007DB94D caps at ctl+90h.
    const float scaled = clamped * tuning.accel_cheat_mul;
    const float capped = min_float(scaled, state.lift_ramp);
    return capped * kGravity00cf9058;  // 007DB981 / 007DB98A, into dyn+20h
}

float gravity_accel_007db990(const PlaneFreeFlightState& state,
                             const PlaneFreeFlightTuning& tuning) {
    // 007DB9BF pushes the immediate 0.0f as x0 and 007DB9BB the 1.0f left on the
    // x87 stack by the lift block's FLD1 as y0; x1 and y1 are the two DeadMeat
    // rows. 007DB9D5 / 007DB9D7: dyn+2Ch -= AccelCheatMul * 9.81f * that.
    const float ramp = interp_clamped_00419010(0.0f, 1.0f, tuning.lost_drag_time,
                                               tuning.extra_gravity_mul, state.lost_drag_timer);
    float accel = -(tuning.accel_cheat_mul * kGravity00cf9058 * ramp);
    if (state.extra_gravity) {
        // 007DB9E7..007DBA2F, gated on the byte ctl+94h. Every argument is an
        // immediate or an image float: 007DBA1B 1.0f, 007DBA15 0.0f,
        // 007DBA0B 00CF87C8 = 2.5f, 007DBA01 00CE3854 = 3.0f. No AccelCheatMul.
        const float extra =
            interp_clamped_00419010(1.0f, 0.0f, 2.5f, 3.0f, state.lost_drag_timer);
        accel -= kGravity00cf9058 * extra;  // 007DBA25 / 007DBA2B
    }
    return accel;
}

float advance_lift_ramp_007db80d(float lift_ramp, float step, bool ramp_reset) {
    if (ramp_reset) return 3.0f;  // 007DB825 stores 00CE3854 when 007BBC50 returns true
    if (3.0f > lift_ramp) return lift_ramp + step;             // 007DB84A
    if (6.0f > lift_ramp) return lift_ramp + 3.0f * step;      // 007DB861, 00D7A2B0 is 3.0
    return lift_ramp;                                          // 007DB85F leaves it alone
}

PlaneDynAccumulators accumulate_free_flight_007db680(const PlaneFreeFlightState& state,
                                                     const PlaneFreeFlightClass& cls,
                                                     const PlaneFreeFlightTuning& tuning,
                                                     float step, bool ramp_reset) {
    PlaneDynAccumulators acc{};  // 007DB6B6 007D7C00 zeroes all six each step

    // 007DB744..007DB80A. The gate is unit+0BBCh > 0.01f (00D7A238); the host
    // supplies the product 007D9050 * unit+0CC8h * the 008E6430 multiplier.
    acc.body_lift[2] += state.thrust_accel;  // dyn+24h, body forward

    // 007DB80D..007DB874, before the lift reads the cap.
    PlaneFreeFlightState stepped = state;
    stepped.lift_ramp = advance_lift_ramp_007db80d(state.lift_ramp, step, ramp_reset);

    acc.body_lift[1] += lift_accel_007db875(stepped, cls, tuning);   // dyn+20h, body up
    acc.world_gravity[1] += gravity_accel_007db990(stepped, tuning);  // dyn+2Ch, world up

    // 007DBA32..007DBC76, the drag along the world velocity direction.
    const float* wv = state.world_velocity;
    const float speed_squared = wv[0] * wv[0] + wv[1] * wv[1] + wv[2] * wv[2];
    // 007DBA67 compares against the double 1e-10 (00CE3820); 007DBA92 against
    // 0.001f (00D7A23C). Either failure skips straight to the damping block.
    if (speed_squared > 1.0e-10f) {
        const float speed = std::sqrt(speed_squared);  // 007DBA7B 00BF7030
        if (speed > 0.001f) {
            const float n[3] = {wv[0] / speed, wv[1] / speed, wv[2] / speed};
            // 007D9140's signed magnitude is already scaled by the 007DBB23
            // pitch ramp at the call site; its tail multiplies in -sgn, so a
            // forward-moving plane gets a negative value that opposes motion.
            float d = state.drag_accel;
            // 007DBB6A: flying backwards multiplies by the double 5.0 (00D7A370).
            if (0.0f > state.body_velocity[2]) d *= 5.0f;
            acc.world_drag[0] += n[0] * d;  // 007DBBC6
            acc.world_drag[1] += n[1] * d;  // 007DBBCE
            acc.world_drag[2] += n[2] * d;  // 007DBBD8
            // 007DBBEA / 007DBBF3: only when ctl+98h > 1.0f. The middle
            // component is multiplied by the double 0.0 at 00D7A258, so the
            // extra push is the horizontal projection of the drag, not the x
            // axis alone: 007DBC3B multiplies n.z * d back in for dyn+18h.
            if (state.roll_drag_scale > 1.0f) {
                const float extra = state.roll_drag_scale - 1.0f;  // 007DBC15, 00D7A210 is 1.0
                acc.world_drag[0] += n[0] * d * extra;             // 007DBC43
                acc.world_drag[1] += 0.0f * d * extra;             // 007DBC4B, the zeroed term
                acc.world_drag[2] += n[2] * d * extra;             // 007DBC55
            }
        }
    }

    // 007DBD37..007DBE0D, the body-frame damping. 007DBD5A hands 007D92B0 the
    // same ratio 007DB773 stored, forward speed over StallSpd.
    const float speed_ratio = state.forward_speed / cls.stall_spd;
    const float response = aero_response_curve_007d92b0(speed_ratio, tuning);
    acc.body_damping[0] += response * (-state.body_velocity[0] * cls.x_drag);  // 007DBD7D
    acc.body_damping[1] += response * (-state.body_velocity[1] * cls.y_drag);  // 007DBD8A
    // 007DBDBD: the vertical term alone is clamped, by a bound that opens from
    // 1.0f to 100.0f as unit+908h runs from 0.5f to 20.0f (00CE3800, 00CE3930,
    // 00CE3D08). 007DBDE6 / 007DBDF6 / 007DBE0A write the clamped value back.
    const float bound =
        interp_clamped_00419010(0.5f, 1.0f, 20.0f, 100.0f, state.airborne_time);
    if (-bound > acc.body_damping[1]) {
        acc.body_damping[1] = -bound;
    } else if (acc.body_damping[1] > bound) {
        acc.body_damping[1] = bound;
    }

    // 007DBE0E..007DBEA8, free flight only (ctl+FCh == 0, which this arm is).
    // 007DBE2E subtracts Ceiling from unit+100h and 007DBE42 multiplies by
    // -CeilingForce, so the push exists only above the ceiling.
    const float ceiling_push = -(state.world_altitude - tuning.ceiling) * tuning.ceiling_force;
    if (0.0f > ceiling_push) {
        acc.world_gravity[1] += ceiling_push;  // 007DBE6B, into dyn+2Ch
        // 007DBE96: the second push fades out as the speed ratio falls from
        // 1.5f (00CE380C) to 0.5f (00CE3800); y0 is the immediate 1.0f and y1
        // the 0.0f the FLDZ above left on the x87 stack.
        const float forward_share =
            interp_clamped_00419010(1.5f, 1.0f, 0.5f, 0.0f, speed_ratio);
        acc.body_damping[2] += forward_share * ceiling_push;  // 007DBEA2, into dyn+0Ch
    }

    return acc;
}

PlaneBodyAcceleration fold_world_into_body_007d8470(const PlaneDynAccumulators& acc,
                                                    const float world_to_body[9]) {
    const auto rotate = [&world_to_body](const float* v, float* out) {
        // 0042D0D0(out, v, ctl+0B0h, 0). 007D9C39 uses the same call to build
        // the body velocity ctl+3Ch from the world velocity ctl+18h, and
        // 007DC6DA hands the fold that same matrix, so both are world -> body.
        for (int row = 0; row < 3; ++row) {
            out[row] = world_to_body[row * 3 + 0] * v[0] + world_to_body[row * 3 + 1] * v[1] +
                       world_to_body[row * 3 + 2] * v[2];
        }
    };

    PlaneBodyAcceleration result{};
    float rotated[3] = {0.0f, 0.0f, 0.0f};

    // 007D8487, the first fold: dyn+28h rotated into dyn+1Ch.
    rotate(acc.world_gravity, rotated);
    for (int i = 0; i < 3; ++i) result.pair_1c[i] = acc.body_lift[i] + rotated[i];

    // 007D84B2, the second fold: dyn+10h rotated into dyn+04h.
    rotate(acc.world_drag, rotated);
    for (int i = 0; i < 3; ++i) result.pair_04[i] = acc.body_damping[i] + rotated[i];

    // 007D8502..007D85A5, the deadband: each component of the two body
    // accumulators is zeroed when its magnitude falls under 0.001f (00D7A23C).
    const auto deadband = [](float* v) {
        for (int i = 0; i < 3; ++i) {
            const float magnitude = v[i] < 0.0f ? -v[i] : v[i];
            if (0.001f > magnitude) v[i] = 0.0f;
        }
    };
    deadband(result.pair_1c);
    deadband(result.pair_04);

    for (int i = 0; i < 3; ++i) result.total[i] = result.pair_04[i] + result.pair_1c[i];
    return result;
}

float free_flight_world_up_acceleration(const PlaneFreeFlightState& state,
                                        const PlaneFreeFlightClass& cls,
                                        const PlaneFreeFlightTuning& tuning, float step,
                                        bool ramp_reset) {
    const PlaneDynAccumulators acc =
        accumulate_free_flight_007db680(state, cls, tuning, step, ramp_reset);
    const PlaneBodyAcceleration body = fold_world_into_body_007d8470(acc, state.world_to_body);
    // The body result goes back to world through the transpose. ctl+0B0h is a
    // rotation for every pose the native builds (0085DEA0 copies it out of the
    // unit's local matrix unit+74h), so the transpose is its inverse; the
    // acceptance test drives the identity case, where this is a no-op.
    return state.world_to_body[1] * body.total[0] + state.world_to_body[4] * body.total[1] +
           state.world_to_body[7] * body.total[2];
}

// ---------------------------------------------------------------------------
// The dyn+B4h/+C0h timed direction hold. docs/PLANE_DYN_TIMED_HOLD.md carries
// the listing evidence for each of these four rules.
// ---------------------------------------------------------------------------

PlaneTimedDirectionHold arm_timed_direction_hold_007d83d0(const float direction[3],
                                                          float seconds) {
    // 007D83D7..007D83F7: three x87 copies then one MOVSS, in that order, with
    // no clamp and no null test on either argument.
    PlaneTimedDirectionHold hold;
    hold.direction[0] = direction[0];
    hold.direction[1] = direction[1];
    hold.direction[2] = direction[2];
    hold.seconds = seconds;
    return hold;
}

float decay_direction_hold_007d902f(float seconds, float step) {
    // 007D9006 COMISS against the zero XORPS left in XMM0 at 007D8EF6, then
    // 007D900F JBE returns without storing. COMISS is unordered-false, so a NaN
    // also takes the no-store exit; returning the input reproduces that.
    if (!(seconds > 0.0f)) {
        return seconds;
    }
    // 007D9015 FSUBRP forms seconds - step; 007D9023 FCOMIP compares it against
    // the same zero and 007D9027 JA selects the zero when the difference went
    // negative.
    const float decayed = seconds - step;
    return (0.0f > decayed) ? 0.0f : decayed;
}

float commit_direction_hold_007dc6c5(float seconds, bool owner_present, bool hold) {
    // 007DC6A8 JZ: with no owner the store still runs, and XMM0 is the zero
    // from 007DC692. 007DC6BD JNZ: a true predicate jumps past the store.
    if (owner_present && hold) {
        return seconds;
    }
    return 0.0f;
}

float gate_direction_hold_007d81c7(float seconds, bool game_state_is_two) {
    // 007D81B5 CMP ... ,2 / 007D81C2 JNZ: the clear is on the equal path only.
    return game_state_is_two ? 0.0f : seconds;
}

float tick_contact_timer_007d81b0(float seconds, float step, bool hold) {
    // 007D81DE COMISS / 007D81E1 JBE: only strictly positive timers are touched.
    if (!(seconds > 0.0f)) {
        return seconds;
    }
    // 007D81E3 subtracts the step and stores it; 007D8209 JNZ then skips the
    // re-arm when the predicate holds. Note the native writes the decremented
    // value first and overwrites it, so the order below matches the observable
    // result, not the two stores.
    const float decayed = seconds - step;
    return hold ? decayed : kPlaneContactTimerExpired;
}

}  // namespace bsp
