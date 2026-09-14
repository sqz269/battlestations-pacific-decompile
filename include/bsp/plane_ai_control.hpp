#pragma once
#include <cstdint>

// Projection of the plane AI's control-writing step: the pilot bot 0099ACD0, its five plan
// slots at task+274h, the slew limiter 0099BB40, the per-axis clamp and permutation 0099BC00,
// the request bytes 0099BEE0, and the commit 007B8C90 into the plane unit's command block at
// unit+9FCh.
//
// docs/PLANE_AI_CONTROL.md carries the evidence, the writer census and the coverage table.
// Every name here is a hypothesis, not a recovered symbol. Nothing here is a binary-compatible
// layout: the offset constants are the native ones, the structs are not.
//
// WHAT THIS IS NOT. This is the AI's command *pipeline*, not its steering law. The desired
// axis values are an input to it, produced natively by 0099D300's per-task arms, of which only
// the yaw arm is recovered (docs/PLANE_AI_CONTROL.md, "One recovered steering arm") and whose
// inputs are not fully identified. A host that feeds desired values of its own invention gets
// the game's own slew, clamp and permutation applied to a made-up demand, which is not the
// game's behaviour. No steering law is reconstructed here, deliberately.
//
// Contracts named but not reconstructed: 0099D300's pitch, roll, power and air-brake arms
// (0099E739, 0099D6B7, 0099DC8F, 0099D8DD); the band repair 0099BF30 and its table search
// 0099B940; the bot task system that supplies the task without which 0099ACD0 emits nothing
// (docs/BOT_TASKS.md, docs/BOT_TASK_STATES.md); the identity of bot+2F4h and unit+C64h.

namespace bsp {

// ---------------------------------------------------------------------------
// The plane unit's pilot command block, unit+9FCh..unit+A14h. Written whole by
// 007B8C90 and consumed by the quantiser 007BB6E0.
// ---------------------------------------------------------------------------
namespace plane_command_off {
inline constexpr int kCmdYaw = 0x9FC;      // cmd[0]; 007B8C96
inline constexpr int kCmdPitch = 0xA00;    // cmd[1]; 007B8C9F
inline constexpr int kCmdRoll = 0xA04;     // cmd[2]; 007B8CA8
inline constexpr int kCmdPower = 0xA08;    // cmd[3]; 007B8CB1
inline constexpr int kCmdAirBrake = 0xA0C; // cmd[4]; 007B8CBA
inline constexpr int kCmdRequestBytes = 0xA10;  // cmd[5]; 007B8CC3
inline constexpr int kCmdPending = 0xA14;       // 007B8CC9 stores 1
}  // namespace plane_command_off

// The bot's five plan slots, base task+274h, stride 0Ch. The array order is NOT the command
// order; see PlanSlotIndex and docs/PLANE_AI_CONTROL.md's permutation table.
namespace pilot_bot_off {
inline constexpr int kPlanSlotBase = 0x274;   // 0099BEF7 LEA ECX,[ESI+274h]
inline constexpr int kPlanSlotStride = 0x0C;  // 0099BC56 / BCA9 / BCFB / BD4E
inline constexpr int kUnit = 0x2F0;           // 0099D31D; the plane unit
inline constexpr int kRequestByte14 = 0x2E4;  // 0099BF18 -> buf+14h -> unit+A10h
inline constexpr int kRequestByte15 = 0x2E5;  // 0099BF0F -> buf+15h, no reader
inline constexpr int kRequestByte16 = 0x2DC;  // 0099BF06 -> buf+16h, no reader
}  // namespace pilot_bot_off

// 00CE3D34, the slew rate 0099B0A0 pushes for all five axes.
inline constexpr float kPlanSlewRate = 4.0f;

// Slot positions in the native array, in array order. 0099BC00 walks them in this order.
enum class PlanSlotIndex : int {
    kPower = 0,     // task+274h, seeded from unit+9F0h pwrInput
    kYaw = 1,       // task+280h, seeded from unit+9E4h yawInput
    kRoll = 2,      // task+28Ch, seeded from unit+9ECh rollInput
    kPitch = 3,     // task+298h, seeded from unit+9E8h pitchInput
    kAirBrake = 4,  // task+2A4h, seeded from unit+9F4h airBrakeInput
};
inline constexpr int kPlanSlotCount = 5;

// One {current, desired, active} triple. `active` is the pending byte 0099D300 sets alongside
// each desired write; 0099BB40 itself does not read it.
struct PlanSlot {
    float current = 0.0f;   // slot+0h; 0099B450 reseeds this from the live control block
    float desired = 0.0f;   // slot+4h; 0099D300 writes this
    bool active = false;    // slot+8h
};

// The six-dword buffer 0099ACD0 builds at [ESP+10h] and hands to 007B8C90. Field order is the
// command order, which is why the slot array needs permuting into it.
struct PilotCommandBlock {
    float yaw = 0.0f;        // cmd[0] -> unit+9FCh
    float pitch = 0.0f;      // cmd[1] -> unit+A00h
    float roll = 0.0f;       // cmd[2] -> unit+A04h
    float power = 0.0f;      // cmd[3] -> unit+A08h
    float air_brake = 0.0f;  // cmd[4] -> unit+A0Ch
    // cmd[5], three bytes plus one the native code never writes.
    std::uint8_t request14 = 0;  // buf+14h -> unit+A10h; the only one with a reader
    std::uint8_t request15 = 0;  // buf+15h; no reader in the image
    std::uint8_t request16 = 0;  // buf+16h; no reader in the image
    std::uint8_t request17 = 0;  // buf+17h; never written natively
    bool pending = false;        // unit+A14h, set to 1 by the commit
};

// The three bytes 0099BEE0 copies out of the bot after 0099BC00 returns.
struct PilotRequestBytes {
    std::uint8_t byte14 = 0;  // bot+2E4h
    std::uint8_t byte15 = 0;  // bot+2E5h
    std::uint8_t byte16 = 0;  // bot+2DCh
};

// 0099BB40, float __thiscall(slot, float rate, float dt), RET 8. The slew limiter: move
// `current` toward `desired` by at most rate*dt. Does not write back to the slot.
// Exact native edge cases: a zero delta and a NaN delta both return `current` unchanged.
float slew_plan_slot_0099bb40(const PlanSlot& slot, float rate, float dt);

// 0099BC00, __thiscall(slot_array, void* out, float rate, float dt). Slews all five slots,
// clamps power and air brake to [0,1] and yaw/roll/pitch to [-1,1], and permutes the array
// order into the command order. NaN is not filtered, matching the native COMISS behaviour.
PilotCommandBlock evaluate_plan_slots_0099bc00(const PlanSlot slots[kPlanSlotCount],
                                               float rate, float dt);

// 0099BEE0, __thiscall(bot, void* out, float rate, float dt), RET 0Ch. 0099BC00 plus the three
// request bytes. Note that only `byte14` has a reader anywhere in the image.
PilotCommandBlock build_pilot_command_0099bee0(const PlanSlot slots[kPlanSlotCount],
                                               const PilotRequestBytes& requests,
                                               float rate, float dt);

// 007B8C90, void __thiscall(unit, const void* cmd), RET 4. Copies the six dwords into
// unit+9FCh..+A10h and sets the pending byte unit+A14h.
void commit_pilot_command_007b8c90(const PilotCommandBlock& cmd, PilotCommandBlock& unit_block);

// 0099B450, __thiscall(task+4h). Reseeds every slot's `current` half from the live control
// block, in the array's own order. Call this before the task states write `desired`.
struct PlaneControlAxes {
    float yaw = 0.0f;        // unit+9E4h
    float pitch = 0.0f;      // unit+9E8h
    float roll = 0.0f;       // unit+9ECh
    float power = 0.0f;      // unit+9F0h
    float air_brake = 0.0f;  // unit+9F4h
};
void seed_plan_slots_0099b450(const PlaneControlAxes& live, PlanSlot slots[kPlanSlotCount]);

}  // namespace bsp
