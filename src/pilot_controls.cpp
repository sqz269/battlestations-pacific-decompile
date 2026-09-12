#include "bsp/pilot_controls.hpp"

#include <cmath>

// Reconstruction of the producers of the plane's control axes.
// docs/PILOT_CONTROLS.md carries the evidence and the coverage table; every name is a
// hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// 00BF7420, MSVC's _ftol: truncation toward zero, which is why the native rule carries the
// 128.5 bias instead of rounding.
int ftol(float v) noexcept { return static_cast<int>(v); }

// The shape the native clamps use: `if (low <= v) { if (high < v) v = high; } else v = low;`
// An unordered compare fails the first test and yields the low bound, so the order matters.
float clamp_native(float v, float low, float high) noexcept {
    if (low <= v) {
        if (high < v) {
            return high;
        }
        return v;
    }
    return low;
}

}  // namespace

bool pilot_state_holds_fifth_command_007bb920(int state) noexcept {
    // 007BB938-007BB94A: four CMP/JZ pairs, and only the fall-through writes 1.0f.
    return state == kPilotStateSeven || state == kPilotStateSix ||
           state == kPilotStateGroundA || state == kPilotStateGroundB;
}

float quantize_pilot_axis_007bb6e0(float command) noexcept {
    // 007BB6E6 FLD [EDI]; 007BB6EA FMUL 127.0; 007BB6F2 FADD 128.5; 007BB6FC CALL 00BF7420.
    const int q = ftol(command * kPilotQuantizeScale + kPilotQuantizeBias);
    if (q >= kPilotQuantizeCeiling) {
        return kPilotAxisHigh;  // 007BB710 FLD ST1, the 1.0f from 007BB701 FLD1
    }
    if (q <= kPilotQuantizeFloor) {
        return kPilotAxisLow;  // 007BB719, the -1.0 from 007BB708 (00D7A250)
    }
    // 007BB71D ADD EAX,-80h; 007BB724 FILD; 007BB728 FDIV ST0,ST3.
    return static_cast<float>(q - kPilotQuantizeCentre) / kPilotQuantizeScale;
}

PilotThrottleArm resolve_throttle_arm_007bb6e0(bool boost_request, const PilotBoostState& boost,
                                               bool capability_17h, bool boost_locked,
                                               bool previous_boost_byte) noexcept {
    PilotThrottleArm out;
    const bool has_charge = boost.charge > 0.0f;  // 007BB7C6 / 007BB7FA COMISS against zero

    if (capability_17h && !boost_locked) {
        // 007BB7B9-007BB7E3. The byte latches: it is set on a request with charge, cleared
        // only when the charge is gone, and otherwise left alone (007BB7D4 JC keeps it).
        if (boost_request && has_charge) {
            out.boost_byte = true;  // 007BB7CB
        } else if (has_charge) {
            out.boost_byte = previous_boost_byte;  // 007BB7D4 JC 007BB7E3
        } else {
            out.boost_byte = false;  // 007BB7DD
        }
        // 007BB7E3: the throttle is binary, 1.0f from 007BB816 or the zeroed XMM0 of 007BB7A8.
        out.throttle = out.boost_byte ? kPilotAxisHigh : 0.0f;
        return out;
    }

    // 007BB7ED. No request drops through to the analogue quantisation at 007BB82E, which also
    // clears both the boost byte and the lockout.
    if (!boost_request) {
        out.analogue = true;
        out.clear_lockout = true;
        return out;
    }
    // 007BB7F2-007BB810, then 007BB816 forces 1.0f whether or not the byte took.
    out.boost_byte = has_charge && !boost.lockout;
    out.throttle = kPilotAxisHigh;
    return out;
}

PlaneControlInput quantize_pilot_command_007bb6e0(const PilotCommandBlock& command,
                                                  const PlaneControlInput& previous,
                                                  const PilotBoostState& boost,
                                                  bool capability_17h,
                                                  bool boost_locked) noexcept {
    PlaneControlInput out = previous;  // 007BB6E0 writes only the five axes and unit+9F8h
    out.roll = quantize_pilot_axis_007bb6e0(command.roll);     // 007BB72A
    out.pitch = quantize_pilot_axis_007bb6e0(command.pitch);   // 007BB75D
    out.yaw = quantize_pilot_axis_007bb6e0(command.yaw);       // 007BB798

    const PilotThrottleArm arm = resolve_throttle_arm_007bb6e0(
        command.boost_request, boost, capability_17h, boost_locked, previous.byte_f8 != 0);
    out.byte_f8 = arm.boost_byte ? 1u : 0u;
    out.throttle = arm.analogue ? quantize_pilot_axis_007bb6e0(command.throttle)  // 007BB877
                                : arm.throttle;                                   // 007BB824
    out.aux = quantize_pilot_axis_007bb6e0(command.fifth);  // 007BB8EE, unconditional
    return out;
}

bool commit_pilot_command_007bb920(PilotCommandCommitHost& host) {
    if (host.suppressed()) {
        return false;  // 007BB927 JNZ 007BB997
    }
    if (!host.command_pending()) {
        return false;  // 007BB930 JZ 007BB997
    }
    if (!pilot_state_holds_fifth_command_007bb920(host.flight_state())) {
        host.set_fifth_command(kPilotAxisHigh);  // 007BB954
    }
    if (host.unit_capability_17h() && !host.boost_locked()) {
        host.set_throttle_command(kPilotAxisHigh);  // 007BB97A
    }
    host.quantize_commands_into_axes();  // 007BB98B
    host.clear_command_pending();        // 007BB990
    return true;
}

PlaneControlInput clamp_control_axes_007caf10(PlaneControlInput axes) noexcept {
    axes.roll = clamp_native(axes.roll, kPilotAxisLow, kPilotAxisHigh);    // 007CB161
    axes.pitch = clamp_native(axes.pitch, kPilotAxisLow, kPilotAxisHigh);  // 007CB183
    axes.yaw = clamp_native(axes.yaw, kPilotAxisLow, kPilotAxisHigh);      // 007CB1A5
    // The two unipolar axes take an immediate zero as the lower bound, not 00D7A260.
    axes.throttle = clamp_native(axes.throttle, kPilotUnipolarLow, kPilotAxisHigh);  // 007CB1C7
    axes.aux = clamp_native(axes.aux, kPilotUnipolarLow, kPilotAxisHigh);            // 007CB1EB
    return axes;
}

PilotEffectiveControl effective_control_007caf10(const PlaneControlLatch& latched,
                                                 float loss_yaw, float loss_pitch,
                                                 float loss_throttle) noexcept {
    PilotEffectiveControl out;
    // 007CB1F9-007CB24F: the native form negates against -0.0f (00D7A208) when the latched
    // value is not above zero, which is an absolute value.
    out.yaw = clamp_native(std::fabs(latched.yaw) - loss_yaw, kPilotUnipolarLow, kPilotAxisHigh);
    out.pitch =
        clamp_native(std::fabs(latched.pitch) - loss_pitch, kPilotUnipolarLow, kPilotAxisHigh);
    // 007CB2E1 has no absolute value: the throttle is already unipolar.
    out.throttle =
        clamp_native(latched.throttle - loss_throttle, kPilotUnipolarLow, kPilotAxisHigh);
    return out;
}

float player_turn_command_00519520(float error, float tolerance) noexcept {
    // The three-way compare before the store of the second command. The sign is the native
    // one: the command is the negated normalised error.
    if (error < -tolerance) {
        return kPilotAxisHigh;
    }
    if (error > tolerance) {
        return kPilotAxisLow;
    }
    return -error / tolerance;
}

float player_pitch_command_00519520(float error) noexcept {
    // The gate is 00CEC728 = -0.5236f and 00CEC724 = 0.5236f; inside it the command is the
    // error over 00CEC730 = 2.0, clamped. The tangent term is multiplied by 00D7A258 = 0.0.
    if (error < -kPilotPitchHalfRange) {
        return kPilotAxisLow;
    }
    if (error > kPilotPitchHalfRange) {
        return kPilotAxisHigh;
    }
    return clamp_native(error / kPilotPitchInputScale, kPilotAxisLow, kPilotAxisHigh);
}

void seed_plan_slots_0099b450(PilotPlanSlots& slots, const PlaneControlInput& live,
                              float descriptor_190h) noexcept {
    seed_unit_plan_slot_0099b450(slots.throttle, live.throttle);  // 0099B456-0099B470
    seed_unit_plan_slot_0099b450(slots.roll, live.roll);          // 0099B476-0099B48E
    seed_unit_plan_slot_0099b450(slots.yaw, live.yaw);            // 0099B494-0099B4AC
    seed_unit_plan_slot_0099b450(slots.pitch, live.pitch);        // 0099B4B2-0099B4CA
    seed_unit_plan_slot_0099b450(slots.fifth, live.aux);          // 0099B4D0-0099B4D8
    slots.demand = descriptor_190h;  // 0099B503 FLD [EAX+190h]; 0099B511 FSTP [ECX+2B4h]
    slots.constant_2c8 = 20.0f;      // 0099B534 / 0099B55E, 00CE3930
    slots.constant_2e8 = 1.0f;       // 0099B51F / 0099B52C, 00D7A24C
    slots.constant_2ec = 0.24f;      // 0099B578 / 0099B580, 00E0E2EC
    slots.mode_yaw = 1;              // 0099B548, +2CCh
    slots.mode_pitch = 2;            // 0099B54E, +2D0h
    slots.mode_roll = 1;             // 0099B558, +2D4h
    slots.mode_throttle = 1;         // 0099B542, +2D8h
}

void seed_plan_slots_held_0099b590(PilotPlanSlots& slots, const PlaneControlInput& live,
                                   float descriptor_190h) noexcept {
    seed_plan_slots_0099b450(slots, live, descriptor_190h);  // 0099B591 CALL 0099B450

    // 0099B59C-0099B61C: the desired half again, every active byte set, every mode zeroed.
    slots.roll.pending = live.roll;          // 0099B5A4
    slots.roll.has_pending = true;           // 0099B5AA
    slots.pitch.pending = live.pitch;        // 0099B5C4
    slots.pitch.has_pending = true;          // 0099B5CA
    slots.yaw.pending = live.yaw;            // 0099B5E2
    slots.yaw.has_pending = true;            // 0099B5E8
    slots.throttle.pending = live.throttle;  // 0099B608
    slots.throttle.has_pending = true;       // 0099B60E
    slots.fifth.pending = live.aux;          // 0099B614
    slots.fifth.has_pending = true;          // 0099B61C
    slots.mode_roll = 0;                     // 0099B5B8
    slots.mode_pitch = 0;                    // 0099B5D6
    slots.mode_yaw = 0;                      // 0099B5F4
    slots.mode_throttle = 0;                 // 0099B622
}

PilotFlightStateChange set_flight_state_007c1430(int current_state, int requested) noexcept {
    PilotFlightStateChange out;

    // 007C1434-007C143C, ahead of the equality guard: a request for state 2 zeroes the
    // throttle even when the plane is already in state 2.
    if (requested == kPilotStateThrottleZeroed) {
        out.throttle_written = true;
        out.throttle = 0.0f;
    }
    if (current_state == requested) {
        return out;  // 007C144C JZ, returns 0
    }

    out.changed = true;     // 007C145B MOV [ECX+900h],EAX
    out.stamp_written = true;  // +C04h = -1.0f, 00D7A260
    out.notify = true;         // every arm reaches 007C11E0(0)
    if (requested == kPilotStateThrottleFull) {
        out.throttle_written = true;
        out.throttle = kPilotAxisHigh;  // 007C153A, 00D7A24C
    }
    return out;
}

}  // namespace bsp
