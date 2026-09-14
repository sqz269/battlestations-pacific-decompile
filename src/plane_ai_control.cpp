#include "bsp/plane_ai_control.hpp"

#include <cstring>

// Reconstruction of the plane AI's control-writing step. docs/PLANE_AI_CONTROL.md carries the
// evidence, the writer census and the coverage table; every name is a hypothesis, not a
// recovered symbol. This is the command pipeline only — no steering law is reconstructed here.

namespace bsp {
namespace {

// 0099BC1C..0099BC44, the unipolar clamp. Read from the listing rather than written as
// std::clamp because the native order matters: the low bound is tested first with FLDZ /
// FCOMIP and the high bound second with COMISS, and an unordered (NaN) compare takes the
// branch that passes the value through on both. std::clamp would return the bound instead.
float clamp_unipolar(float v) {
    if (0.0f > v) {        // 0099BC2E FCOMIP, JBE at 0099BC32 falls through on 0 > v
        return 0.0f;       // 0099BC34 XORPS
    }
    if (v > 1.0f) {        // 0099BC3F COMISS against 00D7A24C
        return 1.0f;       // 0099BC44 MOVAPS
    }
    return v;              // NaN reaches here, as it does natively
}

// 0099BC72..0099BC9E and its three repeats, the bipolar clamp. Same unordered behaviour.
float clamp_bipolar(float v) {
    if (-1.0f > v) {       // 0099BC78 FCOMIP against 00D7A260
        return -1.0f;      // 0099BC7E
    }
    if (v > 1.0f) {        // COMISS against 00D7A24C
        return 1.0f;
    }
    return v;
}

// 0099BB74..0099BB7D: the native code takes |delta| by masking the sign bit of the float's
// storage, not by calling fabs. Reproduced through the same integer mask so a NaN delta keeps
// its NaN-ness and reaches the sign test below unchanged.
float abs_by_mask(float v) {
    std::uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    bits &= 0x7FFFFFFFu;
    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

}  // namespace

float slew_plan_slot_0099bb40(const PlanSlot& slot, float rate, float dt) {
    const float delta = slot.desired - slot.current;  // 0099BB5A FSUBP ST2,ST0
    const float step = rate * dt;                     // 0099BB65 FMUL

    // 0099BB89 FCOMI ST0,ST1 with ST0 = step, ST1 = |delta|; JBE at 0099BB8D takes the slew
    // branch, so the converged branch is strictly step > |delta|.
    if (step > abs_by_mask(delta)) {
        return slot.desired;  // 0099BB8F..0099BB97
    }

    // 0099BBAB COMISS 0.0, delta then 0099BBCE COMISS delta, 0.0. The integer loaded at
    // 0099BBB0 / 0099BBD1 / 0099BBDB is -1, +1 or 0, and FILD converts it before the multiply.
    // A zero delta yields 0; a NaN delta fails both compares and also yields 0, so neither
    // moves `current`.
    int sign = 0;
    if (0.0f > delta) {
        sign = -1;  // 0099BBB0
    } else if (delta > 0.0f) {
        sign = 1;   // 0099BBD1
    }

    // 0099BBBC / 0099BBE7 FMULP then FADDP.
    return slot.current + static_cast<float>(sign) * step;
}

PlaneAiCommandProjection evaluate_plan_slots_0099bc00(const PlanSlot slots[kPlanSlotCount],
                                               float rate, float dt) {
    PlaneAiCommandProjection out;

    // The walk order is the array order, and each result lands at a different command field.
    // 0099BC17 slot 0 -> 0099BC5D [ESI+0Ch]
    out.power = clamp_unipolar(
        slew_plan_slot_0099bb40(slots[static_cast<int>(PlanSlotIndex::kPower)], rate, dt));

    // 0099BC65 slot 1 (LEA ECX,[EDI+0Ch]) -> 0099BCB0 [ESI+0]
    out.yaw = clamp_bipolar(
        slew_plan_slot_0099bb40(slots[static_cast<int>(PlanSlotIndex::kYaw)], rate, dt));

    // 0099BCB7 slot 2 (LEA ECX,[EDI+18h]) -> 0099BD02 [ESI+8]
    out.roll = clamp_bipolar(
        slew_plan_slot_0099bb40(slots[static_cast<int>(PlanSlotIndex::kRoll)], rate, dt));

    // 0099BD0A slot 3 (LEA ECX,[EDI+24h]) -> 0099BD55 [ESI+4]
    out.pitch = clamp_bipolar(
        slew_plan_slot_0099bb40(slots[static_cast<int>(PlanSlotIndex::kPitch)], rate, dt));

    // 0099BD5D slot 4 (LEA ECX,[EDI+30h]) -> 0099BD76 / 0099BD97 [ESI+10h]
    out.air_brake = clamp_unipolar(
        slew_plan_slot_0099bb40(slots[static_cast<int>(PlanSlotIndex::kAirBrake)], rate, dt));

    return out;
}

PlaneAiCommandProjection build_pilot_command_0099bee0(const PlanSlot slots[kPlanSlotCount],
                                               const PilotRequestBytes& requests,
                                               float rate, float dt) {
    PlaneAiCommandProjection out = evaluate_plan_slots_0099bc00(slots, rate, dt);  // 0099BF01
    out.request16 = requests.byte16;  // 0099BF0C, from bot+2DCh
    out.request15 = requests.byte15;  // 0099BF15, from bot+2E5h
    out.request14 = requests.byte14;  // 0099BF1E, from bot+2E4h
    return out;
}

void commit_pilot_command_007b8c90(const PlaneAiCommandProjection& cmd, PlaneAiCommandProjection& unit_block) {
    unit_block.yaw = cmd.yaw;              // 007B8C96 -> unit+9FCh
    unit_block.pitch = cmd.pitch;          // 007B8C9F -> unit+A00h
    unit_block.roll = cmd.roll;            // 007B8CA8 -> unit+A04h
    unit_block.power = cmd.power;          // 007B8CB1 -> unit+A08h
    unit_block.air_brake = cmd.air_brake;  // 007B8CBA -> unit+A0Ch
    // 007B8CC3 copies the sixth dword whole, all four bytes.
    unit_block.request14 = cmd.request14;
    unit_block.request15 = cmd.request15;
    unit_block.request16 = cmd.request16;
    unit_block.request17 = cmd.request17;
    unit_block.pending = true;             // 007B8CC9 MOV byte [ECX+A14h],1
}

void seed_plan_slots_0099b450(const PlaneControlAxes& live, PlanSlot slots[kPlanSlotCount]) {
    // 0099B450 seeds BOTH halves of each triple from the live block; the task states then
    // overwrite the `desired` halves. Seeding both is what makes `current` authoritative
    // despite 0099BB40 never writing back.
    const int power = static_cast<int>(PlanSlotIndex::kPower);
    const int yaw = static_cast<int>(PlanSlotIndex::kYaw);
    const int roll = static_cast<int>(PlanSlotIndex::kRoll);
    const int pitch = static_cast<int>(PlanSlotIndex::kPitch);
    const int brake = static_cast<int>(PlanSlotIndex::kAirBrake);

    slots[power].current = slots[power].desired = live.power;      // 0099B456, unit+9F0h
    slots[yaw].current = slots[yaw].desired = live.yaw;            // 0099B476, unit+9E4h
    slots[roll].current = slots[roll].desired = live.roll;         // 0099B494, unit+9ECh
    slots[pitch].current = slots[pitch].desired = live.pitch;      // 0099B4B2, unit+9E8h
    slots[brake].current = slots[brake].desired = live.air_brake;  // 0099B4D0, unit+9F4h
}

namespace {

// 00415620 BSP_Math_ClampFloatByRef / 00415690 BSP_Math_ClampInPlace.
float clamp_unit(float v) noexcept { return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v); }

// 00415510 BSP_Math_MinFloatByRef.
float min_float(float a, float b) noexcept { return a < b ? a : b; }

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x). The project already treats this
// routine as a host hook with this parameter order (include/bsp/ship_ai_attackmove_substates.hpp,
// include/bsp/plane_flight.hpp); this packet did not re-read its body, only its RET 14h and its
// two saturating exits at 004190B9 and 004190CC.
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x <= x0) return y0;
    if (x >= x1) return y1;
    const float span = x1 - x0;
    return span == 0.0f ? y1 : y0 + (y1 - y0) * ((x - x0) / span);
}

}  // namespace

float plan_yaw_0099e81a(const PilotBotFrame& frame, const PilotBotTuning& tuning,
                        const PilotBotYawScratch& scratch, float yaw_spd) {
    // 0099E81A-0099E884. The base term is zero unless its gain is strictly positive
    // (0099E820 COMISS / 0099E829 JBE, with 0099E823 having already stored the zero).
    float base = 0.0f;
    if (scratch.base_gain > 0.0f) {
        // 0099E843 FMUL [ESP+30h] is cos(bank), not sin; 0099E850 FMUL [EBX+9Ch].
        const float denom = yaw_spd * frame.cos_bank * tuning.rate;
        base = clamp_unit(scratch.base_num / denom) * scratch.base_gain;  // 0099E86C, 0099E875
    }

    // 0099E888-0099E95C. When the turn numerator is not positive the whole second block is
    // skipped and the turn term stays at the zero stored by 0099E891 — note that the base
    // term is then *not* scaled by (1 - t), because the blend lives inside the skipped block.
    float sum = base;
    if (scratch.turn_num > 0.0f) {
        const float t = min_float(  // 0099E8E7
            1.0f, interpolate_clamped(tuning.blend_x0, 0.0f, tuning.blend_x1, 3.0f,
                                      frame.abs_bank));      // 0099E8C8, 3.0f at 00CE3854
        const float denom = yaw_spd * frame.sin_bank * tuning.rate;  // 0099E908, 0099E91E
        const float turn = clamp_unit(scratch.turn_num / denom);     // 0099E939
        sum = t * turn + (1.0f - t) * base;                          // 0099E94A-0099E96C
    }
    return clamp_unit(sum);  // 0099E98D, bounds -1.0f (00D7A260) and 1.0f (00D7A24C)
}

float plan_pitch_0099e68d(float demand) {
    // 0099E693 JBE routes demand > 1 to the +1 store; 0099E6F6/0099E6F9 route demand < -1 to
    // the -1 store (XMM4 = 00D7A260); otherwise 0099E72F falls through with XMM0 still holding
    // the demand loaded at 0099E68D. The three paths converge on the one store at 0099E739.
    return clamp_unit(demand);
}

float axis_urgency_0099e996(float urgency, float reference, float committed) {
    const float d = reference - committed;          // 0099E9BF FSUB
    const float mag = d > 0.0f ? d : -d;            // 0099E9D5 JBE / 0099E9DF, XMM4 = -0.0f
    if (mag > 0.2f) return 0.1f;                    // 00CE3D10, then 00E0E2F4
    if (mag > 0.08f) return min_float(urgency, 0.16f);  // 00D05B50, then 00E0E2F0 via 00415510
    return urgency;                                 // 0099EA17 JBE leaves it alone
}

float yaw_base_numerator_0099de8a(const PilotBotHeadingTerm& in) {
    float e = in.heading_error * in.speed_scale;  // 0099DEBD FMUL [ESP+28h]

    // 0099DECC-0099DF03: a symmetric deadband of half-width tuning+3Ch. The middle case
    // zeroes the term outright (0099DEFC XORPS).
    if (e >= in.deadband) {
        e -= in.deadband;  // 0099DED8 FSUBRP
    } else if (e <= -in.deadband) {
        e += in.deadband;  // 0099DEEE FADDP
    } else {
        return 0.0f;
    }

    // 0099DF09-0099DF87: below the limit the term is scaled by tuning+34h; at or above it
    // the term is instead moved toward zero by (1 - tuning+34h) * limit.
    const float limit = (in.rate_a + in.rate_b) * in.rate_scale;  // 0099DF1B, 0099DF21
    const float mag = e > 0.0f ? e : -e;                          // 0099DF28 / 0099DF3A
    if (limit > mag) return in.keep * e;                          // 0099DF54, 0099DF57
    const float step = (1.0f - in.keep) * limit;                  // 0099DF63 FLD1 / FSUBRP
    return e > 0.0f ? e - step : e + step;                        // 0099DF6F / 0099DF79
}

float yaw_base_gain_0099dffb(const PilotBotTuning& tuning, float abs_bank) {
    // 0099E016 pushes 1.0f as y0 and 0099E006 pushes 0.0f as y1 — the reverse of the
    // blend fraction's 0 -> 3 inside plan_yaw_0099e81a, over the same x range.
    return interpolate_clamped(tuning.blend_x0, 1.0f, tuning.blend_x1, 0.0f, abs_bank);
}

float yaw_turn_numerator_0099e69b(const PilotBotTurnTerm& in) {
    // 0099E69B-0099E6AE. sin(bank) is squared by FMUL ST0 against itself.
    const float p = in.sin_bank * in.sin_bank * in.cos_pitch * in.slide_ratio * in.yaw_spd;
    // 0099E630-0099E64A. The live x87 term the FMULP at 0099E6CC consumes; 00D7A390 = 0.9
    // and 00D7A3A0 = 0.1, so the speed factor is remapped onto [0.1, 1.0].
    const float x = in.rate_b * (in.speed_factor * 0.9f + 0.1f) * in.cos_bank;
    // 0099E6BA / 0099E6D8: NegativePitchRatio only when the plane is inverted.
    const float k = in.inverted ? in.negative_pitch_ratio : 1.0f;
    return p - k * x;  // 0099E6CE FSUBR
}

bool stick_override_0099d620(float stick_axis, float* out) {
    if (stick_axis == 0.0f) return false;  // 0099D620 UCOMISS / 0099D627 JNP
    *out = clamp_unit(stick_axis);         // 0099D629-0099D638, bounds -1.0f and 1.0f
    return true;
}

float power_ceiling_0099dc7a(float previous_power, float desired, bool* wrote) {
    if (previous_power > 0.6f) {  // 0099DC8A COMISS against 00CE3D30, 0099DC8D JBE
        if (wrote != nullptr) *wrote = true;
        return 0.6f;              // 0099DC8F stores the constant itself, not the previous
    }
    if (wrote != nullptr) *wrote = false;
    return desired;
}

}  // namespace bsp
