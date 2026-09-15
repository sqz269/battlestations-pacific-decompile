#include "bsp/pilot_plan_slots.hpp"

#include <cmath>

namespace bsp {
namespace {

// 0099BC2C / 0099BC72 and their partners. Both clamp shapes are `low` first then
// `high`, and both are written as "keep unless the comparison fails", so a NaN
// takes the low bound on the first test.
float clamp_band(float value, float low, float high) noexcept {
    if (!(value >= low)) {     // FLD low ; FCOMIP ; JBE keep
        value = low;
    }
    if (!(value <= high)) {    // COMISS value, high ; JBE keep
        value = high;
    }
    return value;
}

}  // namespace

void pilot_seed_plan_slots_0099b450(PilotPlanSlot slots[5], const float live[5]) noexcept {
    // 0099B456..0099B4EB, five identical triples. `desired` and `current` take
    // the SAME live value and `active` is cleared with the zeroed DL from
    // 0099B46E.
    for (int i = 0; i < 5; ++i) {
        slots[i].current = live[i];
        slots[i].desired = live[i];
        slots[i].active = 0;
    }
}

void pilot_reset_plan_0099b450(PilotPlanState& plan, PilotPlanSlot slots[5],
                               const float live[5]) noexcept {
    pilot_seed_plan_slots_0099b450(slots, live);
    // 0099B4F7-0099B580, the rest of the reset. The two targets are zeroed with
    // the XORPS at 0099B4E8; the four constants are read from .rdata.
    plan.pitch_target_2bc = 0.0f;   // 0099B517
    plan.bank_target_2c4 = 0.0f;    // 0099B509
    plan.bank_limit_2c8 = 20.0f;    // 0099B55E, 00CE3930
    plan.turn_scale_2e8 = 1.0f;     // 0099B52C, 00D7A24C
    plan.heading_mode_2cc = 1;      // 0099B548
    plan.pitch_mode_2d0 = 2;        // 0099B54E
}

float pilot_slew_axis_0099bb40(const PilotPlanSlot& slot, float a, float b) noexcept {
    // 0099BB43..0099BB69.
    const float delta = slot.desired - slot.current;
    const float step = a * b;

    // 0099BB6D..0099BB8D. The magnitude is taken by masking the sign bit out of
    // the float's integer image (AND EAX,7FFFFFFFh at 0099BB79), and the
    // comparison is `step > |delta|` strictly - FCOMI sets ZF on equality and
    // the JBE is taken, so an exactly-reaching step takes the step path. The two
    // answers are the same number.
    if (step > std::fabs(delta)) {
        return slot.desired;               // 0099BB8F..0099BB9E, the snap
    }

    // 0099BBAB..0099BBDB. The sign is materialised as an integer -1/+1/0 and
    // brought in with FILD, so `sign(0) == 0` and an axis whose desired equals
    // its current returns current bit-exactly rather than current + 0*step.
    if (0.0f > delta) {
        return slot.current - step;
    }
    if (delta > 0.0f) {
        return slot.current + step;
    }
    return slot.current;
}

void pilot_evaluate_plan_slots_0099bc00(const PilotPlanSlot slots[5], float cmd[5],
                                        float rate, float dt) noexcept {
    // The five calls in the body's order, each result clamped into the command
    // buffer while the next call is being set up. The band differs by axis:
    // throttle and air brake cannot go negative, the three attitude axes can.
    cmd[kPilotCmdThrottle] =
        clamp_band(pilot_slew_axis_0099bb40(slots[kPilotSlotThrottle], rate, dt), 0.0f, 1.0f);
    cmd[kPilotCmdYaw] =
        clamp_band(pilot_slew_axis_0099bb40(slots[kPilotSlotYaw], rate, dt), -1.0f, 1.0f);
    cmd[kPilotCmdRoll] =
        clamp_band(pilot_slew_axis_0099bb40(slots[kPilotSlotRoll], rate, dt), -1.0f, 1.0f);
    cmd[kPilotCmdPitch] =
        clamp_band(pilot_slew_axis_0099bb40(slots[kPilotSlotPitch], rate, dt), -1.0f, 1.0f);
    cmd[kPilotCmdAirBrake] =
        clamp_band(pilot_slew_axis_0099bb40(slots[kPilotSlotAirBrake], rate, dt), 0.0f, 1.0f);
}

float pilot_quantize_control_axis_007bb6e0(float command) noexcept {
    // 007BB6E0's per-axis body. The double constants are read from the image:
    // 00CFD408 = 127.0, 00D05998 = 128.5. 00BF7420 is _ftol2_sse, whose
    // CVTTSD2SI truncates toward zero - which for the positive argument range
    // this sees is a floor, making the whole thing a round-half-up.
    const int n = static_cast<int>(static_cast<double>(command) * 127.0 + 128.5);
    if (n >= 0xFF) {
        return 1.0f;
    }
    if (n <= 1) {
        return -1.0f;
    }
    return static_cast<float>(static_cast<double>(n - 128) / 127.0);
}

}  // namespace bsp
