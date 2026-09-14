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

PilotCommandBlock evaluate_plan_slots_0099bc00(const PlanSlot slots[kPlanSlotCount],
                                               float rate, float dt) {
    PilotCommandBlock out;

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

PilotCommandBlock build_pilot_command_0099bee0(const PlanSlot slots[kPlanSlotCount],
                                               const PilotRequestBytes& requests,
                                               float rate, float dt) {
    PilotCommandBlock out = evaluate_plan_slots_0099bc00(slots, rate, dt);  // 0099BF01
    out.request16 = requests.byte16;  // 0099BF0C, from bot+2DCh
    out.request15 = requests.byte15;  // 0099BF15, from bot+2E5h
    out.request14 = requests.byte14;  // 0099BF1E, from bot+2E4h
    return out;
}

void commit_pilot_command_007b8c90(const PilotCommandBlock& cmd, PilotCommandBlock& unit_block) {
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

}  // namespace bsp
