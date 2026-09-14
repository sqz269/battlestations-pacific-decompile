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
// This earlier projection has different axis labels and request-byte fields
// from pilot_controls.hpp's PilotCommandBlock; keep their C++ identities distinct.
struct PlaneAiCommandProjection {
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
PlaneAiCommandProjection evaluate_plan_slots_0099bc00(const PlanSlot slots[kPlanSlotCount],
                                               float rate, float dt);

// 0099BEE0, __thiscall(bot, void* out, float rate, float dt), RET 0Ch. 0099BC00 plus the three
// request bytes. Note that only `byte14` has a reader anywhere in the image.
PlaneAiCommandProjection build_pilot_command_0099bee0(const PlanSlot slots[kPlanSlotCount],
                                               const PilotRequestBytes& requests,
                                               float rate, float dt);

// 007B8C90, void __thiscall(unit, const void* cmd), RET 4. Copies the six dwords into
// unit+9FCh..+A10h and sets the pending byte unit+A14h.
void commit_pilot_command_007b8c90(const PlaneAiCommandProjection& cmd, PlaneAiCommandProjection& unit_block);

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

// ---------------------------------------------------------------------------
// 0099D300 BSP_PilotBot_PlanControls, two of the five axis arms.
// docs/PILOT_BOT_PLAN_CONTROLS.md, "The axis arms read from the listing", carries the
// evidence. These are the arms' *laws*; the scratch quantities they consume are passed in
// because this packet did not trace them to named inputs. A host that cannot supply them
// must refuse, not substitute a guess.

// The per-tick attitude quantities the arms share, all formed at 0099D46E-0099D510.
struct PilotBotFrame {
    float bank = 0.0f;        // unit+C68h, 0099D4B7
    float pitch = 0.0f;       // unit+C64h, 0099D4BF
    float sin_bank = 0.0f;    // [ESP+20h], 0099D4FA FSIN
    float cos_bank = 0.0f;    // [ESP+30h], 0099D504 FCOS
    float cos_pitch = 0.0f;   // [ESP+2Ch], 0099D50E FCOS
    float abs_bank = 0.0f;    // [ESP+1Ch], 0099D4D1 AND EAX,7FFFFFFFh
};

// The plane half of the game-tuning singleton, 0042E740 + 538h (EBX at 0099D487).
struct PilotBotTuning {
    float blend_x0 = 0.0f;  // +7Ch, 0099E8C2
    float blend_x1 = 0.0f;  // +80h, 0099E8B2
    float rate = 0.0f;      // +9Ch, 0099E850 / 0099E91E; divides both yaw terms
};

// The three scratch slots the yaw arm consumes. Each has been located to its reaching
// definitions (see the doc) but not resolved to a named quantity, so they are inputs here.
struct PilotBotYawScratch {
    float base_num = 0.0f;   // [ESP+6Ch] at 0099E82B; defs 0099DDD0 (=0), 0099DF87
    float base_gain = 0.0f;  // [ESP+38h] at 0099E81A; defs 0099DD9D (=0), 0099E027
    float turn_num = 0.0f;   // [ESP+10h] at 0099E8F6; defs 0099E3CB (=0), 0099E6D2/E6E8/E729
};

// 0099E81A-0099EA46. Returns the value stored to the yaw slot's `desired` (task+284h).
// The caller must have established that the arm runs at all: it is reached only through
// 0099E75E's `JNZ` on a non-zero task+2D4h.
float plan_yaw_0099e81a(const PilotBotFrame& frame, const PilotBotTuning& tuning,
                        const PilotBotYawScratch& scratch, float yaw_spd);

// 0099E68D-0099E752. The pitch arm's terminal law: a plain saturation of its demand.
// `demand` is [ESP+44h], formed at 0099E664-0099E689 from x87-stack values this packet did
// not trace; it is an input for the same reason.
float plan_pitch_0099e68d(float demand);

// 0099E996-0099EA2B. After an axis writes `desired`, the bot folds the size of the change
// into the re-plan interval at task+2ECh. The caller picks `reference` the way 0099E996 does:
// the slot's `desired` when its active byte is set, otherwise its `prev`.
float axis_urgency_0099e996(float urgency, float reference, float committed);

// ---------------------------------------------------------------------------
// The producers of `plan_yaw_0099e81a`'s three scratch inputs (packet
// cc7_pilot_bot_axis_arms_2). With these the yaw arm is closed apart from two opaque
// sources named in each struct.

// 0099DE8A-0099DF87. The base numerator: a deadbanded, step-limited heading error.
// Runs only when task+2CCh == 2; on every other path the numerator stays at the zero
// stored by 0099DDD0.
struct PilotBotHeadingTerm {
    float heading_error = 0.0f;  // 00438B10(task+2C0h, unit->vtable[50h]()), 0099DEB8.
                                 // vtable[50h] is 0074E260 on all nine plane classes:
                                 // FLD [ECX+0C6Ch] / RET, so it is the unit+C6Ch heading.
    float speed_scale = 0.0f;    // [ESP+28h] = 1 / max(unit[+340h] * 0.4f, 1.0f), 0099D4B3
    float deadband = 0.0f;       // tuning+3Ch, 0099DEC5
    float rate_a = 0.0f;         // class+1C8h, 0099DF0F
    float rate_b = 0.0f;         // class+1ACh, 0099DF1B
    float rate_scale = 0.0f;     // tuning+38h, 0099DF21
    float keep = 0.0f;           // tuning+34h, 0099DF54 / 0099DF60
};
float yaw_base_numerator_0099de8a(const PilotBotHeadingTerm& in);

// 0099DFFB-0099E027. The base gain: a bank fade, 1 below tuning+7Ch and 0 above tuning+80h.
// Note the y-endpoints are the reverse of the blend fraction's inside plan_yaw_0099e81a.
float yaw_base_gain_0099dffb(const PilotBotTuning& tuning, float abs_bank);

// 0099E69B-0099E6D2, and the identical mirror at 0099E703-0099E729. The turn numerator,
// produced by the pitch arm and consumed by the yaw arm.
struct PilotBotTurnTerm {
    float sin_bank = 0.0f;              // [ESP+20h]
    float cos_bank = 0.0f;              // [ESP+30h]
    float cos_pitch = 0.0f;             // [ESP+2Ch]
    float slide_ratio = 0.0f;           // class+1B8h, 0099E6A8
    float yaw_spd = 0.0f;               // class+1B0h, 0099E6AE
    float rate_b = 0.0f;                // class+1ACh, 0099E630
    float negative_pitch_ratio = 0.0f;  // class+1D8h, 0099E6BA
    float speed_factor = 0.0f;          // R = 007D9A70(unit+AB0h), 0099E5D7.
                                        // See plane_speed_factor_007d9a70 below.
    bool inverted = false;              // sign(cos(bank)) < 0; [ESP+44h] from 0099DDCA
};
float yaw_turn_numerator_0099e69b(const PilotBotTurnTerm& in);

// 0099D602-0099D6C6. Before any arm runs, a direct stick input on the unit overrides the
// axis and cancels its mode word, so the computed arm does not run this tick. The native
// test is the MSVC exact-equality idiom (UCOMISS / LAHF / TEST AH,44h / JNP), which skips
// only on an ordered compare equal to zero.
struct PilotBotStickOverride {
    float yaw = 0.0f;    // unit+998h, 0099D608 -> slot(yaw),   clears task+2D4h
    float pitch = 0.0f;  // unit+99Ch, 0099D656 -> slot(pitch), clears task+2D0h
    float roll = 0.0f;   // unit+9A0h, 0099D694 -> slot(roll),  clears task+2CCh
};
// True when the axis is overridden; `out` then receives the clamped value.
bool stick_override_0099d620(float stick_axis, float* out);

// 0099DC7A-0099DC97. A ceiling on the power axis: when the slot's own previous value is
// above 0.6f (00CE3D30) the desired power is pinned there.
float power_ceiling_0099dc7a(float previous_power, float desired, bool* wrote);

// ---------------------------------------------------------------------------
// The yaw arm's last two inputs (packet cc7_yaw_remaining_inputs).

// unit->vtable[50h] is 0074E260 on every plane class (00D05F20, 00D06638, 00D1A000,
// 00D19D28, 00D06920, 00D00070, 00D0BA80, 00D00308, 00D1A2D8 all hold it at +50h), and its
// whole body is `FLD dword ptr [ECX+0C6Ch] / RET`. Other entity families override the slot,
// so the identification is the plane family's, not a universal one.
inline constexpr int kUnitHeading = 0xC6C;  // the wrapped atan2 that 007C1ACA writes

// 007C0F40, the bomb-load factor. 1.0f unless a weapon slot still carries bomb-class
// ordnance, then interpolated toward a class field; turbo scales whatever results.
struct PlaneBombLoadFactor {
    bool carries_bomb = false;   // slot->vtable[210h](2Ah, 0) over +974h/+994h, 007C0F72
    float slot_weight = 0.0f;    // slot->vtable[214h]() of the first such slot, 007C0F9D:
                                 // the remaining bomb-load fraction, see
                                 // bomb_load_fraction_006e4130 below.
    float loaded_scale = 1.0f;   // class+15Ch, 007C0FA6
    bool turbo = false;          // plane+BC8h, 007C0FCF
    float turbo_scale = 1.0f;    // class+608h, 007C0FDE; the turbo block is +5FCh..+608h
};
float bomb_load_factor_007c0f40(const PlaneBombLoadFactor& in);

// 007D9A70, the speed factor R that the yaw turn term remaps onto [0.1, 0.9R+0.1].
struct PlaneSpeedFactor {
    float free_flight_scalar = 0.0f;  // unit+908h: controller+8h is the plane unit itself,
                                      // 007D9A79. docs/PILOT_CONTROLS.md pairs it with +904h
                                      // against 5.0f; this interpolation's x range is 3..6.
    float forward_speed = 0.0f;  // 007D99C0 BSP_PlaneFlight_ForwardSpeed(sub), 007D9AB4
    float max_speed = 0.0f;      // class+184h, in the speed block, 007D9ABC
    float dyn_c0 = 0.0f;         // [controller+10h]+C0h, 007D9AFC. **Runtime-only**: three
                                 // producers, all per-tick physics — 007DB2A4 copies a source
                                 // scalar in, 007DC6C5 zeroes it or takes a computed value,
                                 // and 007D902F decays it in the integrator's tail. Nothing
                                 // authored reaches it, so a host that does not run the plane
                                 // integrator cannot supply it and must refuse the yaw axis.
    float bomb_load = 1.0f;      // 007C0F40, above
};
// Nothing in the image writes 00F8731C or 00F87320, so the ratio interpolation's two
// x-endpoints are both 0.0f and 00419010 returns its y0 — the `c` term is a constant zero in
// this build. The parameters for them are therefore gone; see the doc for the evidence.
float plane_speed_factor_007d9a70(const PlaneSpeedFactor& in);

// 006E4130 (MBombPlatform) and 006E3700 (MMultipleBombPlatform), vtable[214h]. The remaining
// bomb-load fraction that 007C0F40 interpolates on. Both divide a kind-2Ah count from
// vtable[21Ch](2Ah) by the platform's capacity at +488h; MBombPlatform adds +484h first and
// returns 0.0f outright when the capacity is zero.
struct BombLoadFraction {
    int remaining = 0;    // slot->vtable[21Ch](2Ah), 006E4144 / 006E3706. Not read.
    int pending = 0;      // slot+484h, 006E414E; MBombPlatform only
    int capacity = 0;     // slot+488h, 006E415C / 006E3720
    bool single = true;   // MBombPlatform (true) vs MMultipleBombPlatform (false)
};
float bomb_load_fraction_006e4130(const BombLoadFraction& in);

// ---------------------------------------------------------------------------
// The task's three target fields (packet cc7_pilot_bot_task_inputs).

// The pilot bot's target/mode pairs, all set by the bot state machine.
namespace pilot_task_off {
inline constexpr int kBankTargetLimited = 0x2BC;  // the slewed bank target, 0099DD4E
inline constexpr int kSpeedTarget = 0x2B4;        // read at 0099D8C6
inline constexpr int kHeadingTarget = 0x2C0;      // read at 0099DEAF; valid when mode 2CCh == 2
inline constexpr int kRollMode = 0x2CC;           // 0, 1 or 2; 2 means "hold kHeadingTarget"
inline constexpr int kPitchMode = 0x2D0;          // 0, 1 or 2; 2 means "hold unit+C84h"
inline constexpr int kSpeedMode = 0x2D8;          // 0 or 1; 1 means "hold kSpeedTarget"
}  // namespace pilot_task_off

// 0099D8C1-0099D8EB. The speed-hold arm, corrected: the quantity compared against the speed
// target is the **constant** 0.001f (00D7A23C, loaded at 0099D7A7 on both entry paths), not a
// computed demand. So this is the full-stop case — when a speed hold is pending and the target
// is below 0.001f, the bot commands idle power and full air brake, then clears the mode.
struct SpeedHoldResult {
    bool fired = false;
    float power = 0.0f;      // 0099D8CF stores the same 0.001f constant
    float air_brake = 0.0f;  // 0099D8DD stores 1.0f (00D7A24C)
};
SpeedHoldResult speed_hold_0099d8c1(int speed_mode, float speed_target);

}  // namespace bsp
