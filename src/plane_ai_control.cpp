#include "bsp/plane_ai_control.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

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

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), body read at 00419010-004190CC.
// Two details that an x-side clamp gets wrong, both now taken from the listing:
//   * 0041901E-00419030: when x1 == x0 it returns **y0** outright, whatever x is. The test is
//     the MSVC exact-equality idiom (FUCOMIP / LAHF / TEST AH,44h / JP), and equality falls
//     through to FLD [ESP+8].
//   * 00419063-004190CC: the interpolated value is clamped between the two **y** endpoints in
//     whichever order they come, not by the x range. That is what makes a descending pair
//     (y0 = 1, y1 = 0, as in yaw_base_gain_0099dffb) behave.
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x1 == x0) return y0;  // 00419026 JP not taken
    const float v = y0 + (y1 - y0) * ((x - x0) / (x1 - x0));  // 00419033-0041905F
    const float lo = y0 < y1 ? y0 : y1;
    const float hi = y0 < y1 ? y1 : y0;
    return v < lo ? lo : (v > hi ? hi : v);
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

float bomb_load_factor_007c0f40(const PlaneBombLoadFactor& in) {
    float m = 1.0f;  // 007C0F41, 00D7A24C
    if (in.carries_bomb) {
        // 007C0FC5: x0 = 0.0f (FLDZ), y0 = 1.0f (FLD1), x1 = 0.8f (00CE74F8),
        // y1 = class+15Ch. The loop takes the first matching slot and stops (007C0F76).
        m = interpolate_clamped(0.0f, 1.0f, 0.8f, in.loaded_scale, in.slot_weight);
    }
    if (in.turbo) m *= in.turbo_scale;  // 007C0FE4
    return m;
}

float plane_speed_factor_007d9a70(const PlaneSpeedFactor& in) {
    // 007D9AA9: x0 = 3.0f (00CE3854), y0 = 0, x1 = 6.0f (00CE6630), y1 = 0.25f (00CE3868).
    const float a = interpolate_clamped(3.0f, 0.0f, 6.0f, 0.25f, in.free_flight_scalar);
    // 007D9AB4/007D9ABC. Kept for fidelity: nothing consumes it in this build, because the
    // interpolation below is degenerate, but the native still performs the divide.
    const float ratio = in.forward_speed / in.max_speed;
    // 007D9AF0: x0 = [00F8731C] and x1 = [00F87320], and nothing in the image writes either,
    // so both are 0.0f and 00419010's x1 == x0 path returns y0. `c` is a constant zero here.
    const float c = interpolate_clamped(0.0f, 0.0f, 0.0f, 1.0f, ratio);
    const float v = c + in.dyn_c0 * 0.6f;  // 007D9B02 FMUL 00CEFF98, 007D9B08 FADD
    // 007D9B1C picks `a` when it is the larger; otherwise 007D9B55-007D9B68 caps at 1.0f.
    const float result = a > v ? a : min_float(v, 1.0f);
    // 007D9B3F squares the result (FMUL ST0), 007D9B45 multiplies in the bomb-load factor.
    return in.bomb_load * result * result;
}

SpeedHoldResult speed_hold_0099d8c1(int speed_mode, float speed_target) {
    SpeedHoldResult out;
    if (speed_mode != 1) return out;  // 0099D8C1 CMP ECX,1 / 0099D8C4 JNZ
    // 0099D8C6 COMISS XMM0,[ESI+2B4h] with XMM0 = 0.001f, 0099D8CD JBE skips.
    if (!(0.001f > speed_target)) return out;
    out.fired = true;
    out.power = 0.001f;     // 0099D8CF stores XMM0, the same constant
    out.air_brake = 1.0f;   // 0099D8DD stores XMM3
    return out;             // the caller also clears the mode word, 0099D8EB
}

float bomb_load_fraction_006e4130(const BombLoadFraction& in) {
    if (in.single && in.capacity == 0) return 0.0f;  // 006E4134 / 006E413D FLDZ
    const int numerator = in.single ? in.remaining + in.pending : in.remaining;  // 006E414E
    // 006E415C / 006E3720 FIDIV: an integer divide of an integer-converted numerator.
    return static_cast<float>(numerator) / static_cast<float>(in.capacity);
}



PilotBotPitchResult pilot_pitch_demand_0099e490(const PilotBotPitchInputs& in) {
    PilotBotPitchResult out;

    // 0099E496-0099E4CC. The bank fraction and the heading ramp, multiplied.
    // hdgRamp is path-dependent in the native - it holds this interpolation only
    // on the pass that re-planned the bank target, and 0.0f otherwise - so a
    // caller that never re-plans the bank gets the lowest floor. That coupling
    // between the two arms is deliberate in the original and is preserved by
    // making the ramp an explicit input rather than recomputing it here from
    // whatever is to hand.
    float bank_fraction = 0.0f;
    if (in.turn_roll != 0.0f) {
        bank_fraction = std::fabs(in.bank) / in.turn_roll;
    }
    if (bank_fraction > 1.0f) {
        bank_fraction = 1.0f;                                  // 0099E4AA
    }
    const float hdg_ramp = interpolate_clamped(
        in.pitch_turn_hdg_range_1, 0.0f, in.pitch_turn_hdg_range_2, 1.0f,
        std::fabs(in.heading_error));                          // 0099E07E
    const float q = bank_fraction * hdg_ramp;                  // 0099E4CC

    // 0099E4DC-0099E512. The floor: PitchTurnMaxPitch when the turn is hard,
    // 2.5 radians below it when the plane is level and on heading. It is applied
    // unconditionally on every pass of the law, and it can only RAISE the
    // target - which is the whole of what stops a bot flying into the sea.
    const float floor_target = in.pitch_turn_max_pitch -
        static_cast<float>(2.5 * (1.0 - static_cast<double>(q)));
    out.floored_target = (floor_target > in.pitch_target) ? floor_target : in.pitch_target;

    // 0099E51A-0099E554. The measured angle has a sideslip correction taken off
    // it - note the sin(bank) is SQUARED, and note it is subtracted from the
    // measurement rather than from the target.
    const float sin_bank = std::sin(in.bank);
    const float cos_bank = std::cos(in.bank);
    const float correction = sin_bank * sin_bank * std::cos(in.pitch) *
        in.slide_ratio * in.yaw_spd * in.pitch_ctrl_set_time_mul;
    const float measured = in.held_pitch - correction;
    const float error = wrapped_angle_subtract_00438b10(out.floored_target, measured);

    // 0099E5D7-0099E689. The demand is the required pitch rate divided by the
    // rate one unit of elevator buys, so it collapses toward knife-edge where
    // cos(bank) goes to zero - which is what the 0.001f guard and its
    // hundred-fold fallback exist for.
    const float authority = in.control_authority * 0.9f + 0.1f;   // 0099E5FC
    const bool inverted = cos_bank < 0.0f;                        // 0099E5EB
    const float k = inverted ? in.negative_pitch_ratio : 1.0f;    // 0099E61C
    const float n = (in.dt_scale != 0.0f) ? error / in.dt_scale : error;
    const float x = in.pitch_spd * authority * cos_bank;          // 0099E63A
    const float d = x * k * in.pitch_ctrl_set_time_mul;           // 0099E644
    if (std::fabs(d) > 0.001f) {
        out.demand = n / d;
    } else {
        const float sign = (cos_bank > 0.0f) ? 1.0f : ((cos_bank < 0.0f) ? -1.0f : 0.0f);
        out.demand = static_cast<float>(static_cast<double>(n) * 100.0 *
                                        static_cast<double>(sign));
    }
    return out;
}

}  // namespace bsp
