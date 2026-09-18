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
// CORRECTED (packet cc7-recon-slot): both have readers, in 007BB6E0's tail.
inline constexpr int kRequestByte15 = 0x2E5;  // 0099BF0F -> buf+15h -> unit+9F9h, 007BB8D6
inline constexpr int kRequestByte16 = 0x2DC;  // 0099BF06 -> buf+16h -> unit+9FAh, 007BB8C2,
                                              // gated on unit+5Dh (out of action)
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

// 0099E490-0099E689 (packet cc7-pitchroll, docs/PILOT_PLANNER_PITCH_ROLL.md): the
// demand `plan_pitch_0099e68d` was declared against, and the nose-up floor that
// is the only thing keeping a planned bot from following its pitch target into
// the ground. There is NO altitude term anywhere in this chain - nothing reads a
// world position, a Y coordinate or a sea height - so what holds the nose up is
// an attitude floor that rises with bank and with heading error.
struct PilotBotPitchInputs {
    float bank = 0.0f;         // unit+C68h
    float pitch = 0.0f;        // unit+C64h
    float held_pitch = 0.0f;   // slot 12: unit+C84h in mode 2, else unit+C64h
    float pitch_target = 0.0f; // plan+2BCh on entry; the floor can only raise it
    float heading_error = 0.0f;  // the yaw arm's base numerator, |h| drives the ramp
    float control_authority = 0.0f;  // 007D9A70's return, through unit+AB0h
    float dt_scale = 1.0f;     // 1 / max(unit+340h * 0.4, 1.0)
    // Class fields.
    float turn_roll = 0.0f;            // class+25Ch TurnRoll, the bank normaliser
    float pitch_spd = 0.0f;            // class+1ACh
    float yaw_spd = 0.0f;              // class+1B0h
    float slide_ratio = 0.0f;          // class+1B8h
    float negative_pitch_ratio = 0.0f; // class+1D8h
    // Tuning, all Pilot/General/*.
    float pitch_turn_max_pitch = 0.0f;   // tuning+88h, singleton +5C0h
    float pitch_turn_hdg_range_1 = 0.0f; // tuning+8Ch, +5C4h
    float pitch_turn_hdg_range_2 = 0.0f; // tuning+90h, +5C8h
    float pitch_ctrl_set_time_mul = 0.0f;  // tuning+A0h, +5D8h
};

struct PilotBotPitchResult {
    float floored_target = 0.0f;  // plan+2BCh after 0099E512
    float demand = 0.0f;          // [ESP+44h] at 0099E689
};

// The demand, and the floor that is applied to the target on the way.
// `plan_pitch_0099e68d(result.demand)` is then the stored `desired`.
PilotBotPitchResult pilot_pitch_demand_0099e490(const PilotBotPitchInputs& in);

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
// CORRECTED (packet cc7-pitchroll): plan+2BCh is the **pitch** target, not a
// bank target. Its arm is gated on task+2D0h - which this same header calls
// kPitchMode - and it terminates at the pitch slot's desired at +29Ch. The
// algebra docs/PILOT_BOT_PLAN_CONTROLS.md recorded for the "bank-target arm" is
// right; only the axis was wrong. The bank target is plan+2C4h (0099E23E).
inline constexpr int kPitchTargetFloored = 0x2BC;  // the pitch target, 0099DD4E / 0099E512
inline constexpr int kBankTargetLimited = 0x2C4;   // the bank target, 0099E23E
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


// ---------------------------------------------------------------------------
// The roll arm (packets cc7-pitchroll and cc7-hdgdiff).
// docs/PILOT_PLANNER_PITCH_ROLL.md and docs/PILOT_HEADING_DIFF_SCALE.md.

// 0099D0A0, `float __thiscall(plan, float w)`, RET 4. The four tuning keys it
// reads are all `Pilot/General/HdgDiffCalc*`, and they name it: it answers **how
// much further the plane would turn while rolling back to level from bank `w`**.
// That is the scale the bank target divides the heading error by, so the whole
// magnitude of a coordinated turn rests on it.
//
// Read as physics: `T` is a roll-out time - bank over roll rate, plus a
// 1/RollAccel allowance for getting the roll started - and `Q * sin(W)` is the
// turn rate the airframe holds at that bank. Their product is an angle.
struct PilotHeadingDiffInputs {
    float bank_request = 0.0f;   // the argument; the caller passes min(|bank|*1.5, maxBank)
    float pitch_angle = 0.0f;    // unit+C64h
    float pitch_command = 0.0f;  // plan+29Ch when the pitch slot is active, else plan+298h
    bool unit_is_null = false;   // plan+2F0h == 0 -> the 0.5f early return
    // unit->vtable[5Ch](10h) || (16h). Caps the rate multiplier at 1.0.
    bool caps_rate_at_one = false;
    float roll_spd = 0.0f;        // class+1A8h
    float pitch_spd = 0.0f;       // class+1ACh
    float yaw_spd = 0.0f;         // class+1B0h
    float slide_ratio = 0.0f;     // class+1B8h
    float roll_accel = 0.0f;      // class+1BCh
    float turn_roll_spd = 0.0f;   // class+1C8h
    float turn_roll = 0.0f;       // class+25Ch
    float hdg_diff_calc_limit_1 = 0.0f;    // tuning+4Ch, singleton +584h
    float hdg_diff_calc_limit_2 = 0.0f;    // tuning+50h, +588h
    float hdg_diff_calc_min_pitch = 0.0f;  // tuning+54h, +58Ch
    float hdg_diff_calc_min_roll = 0.0f;   // tuning+58h, +590h
};

float pilot_heading_diff_0099d0a0(const PilotHeadingDiffInputs& in);

// 0099DE93-0099E25C, the bank target `plan+2C4h`, and 0099E2BA-0099E39D, the
// roll command that servos onto it.
struct PilotBotRollInputs {
    float heading_error = 0.0f;  // the yaw arm's base numerator, the same [ESP+6Ch]
    float bank = 0.0f;           // unit+C68h
    float pitch_error = 0.0f;    // wrap(measured pitch - plan+2BCh), the roll limit's schedule
    float dt_scale = 1.0f;       // 1 / max(unit+340h * 0.4, 1.0)
    float turn_scale_2e8 = 1.0f; // plan+2E8h, 1.0f out of the plan reset
    float bank_limit_2c8 = 20.0f;  // plan+2C8h, 20.0f out of the plan reset
    // IGNORED since packet cc8_bank_command_inputs, and kept only so existing
    // callers still compile. 0047B880 is read now (0047B880-0047B8BB, RET 0,
    // __thiscall(unit)): it returns `!(vtable[5Ch](10h) || vtable[5Ch](16h))`,
    // and slot 5Ch on a plane is BSP_PlaneInstance_IsKindOf (0074E400, installed
    // in vtable 00D05F20). 0099D0A0 tests the same disjunction on the same unit
    // at 0099D1C2-0099D1D9 to set `caps_rate_at_one`, so the two are one bit and
    // its complement and cannot be supplied independently. The law derives
    // `!scale.caps_rate_at_one` instead. Class ids 10h and 16h sit between the
    // plane base 0Fh and PlaneSquadronGen 18h (docs/ENTITY_CLASS_IDS.md), so
    // they are two concrete plane types; every other plane takes the SMALL
    // limit, and since the cap enters as min(maxBank, cap) Large is the weaker
    // of the two.
    bool small_turn_roll_limit = false;
    PilotHeadingDiffInputs scale;   // the 0099D0A0 call at 0099E0C1
    // Tuning.
    float turn_roll_limit_small = 0.0f;   // tuning+64h, singleton +59Ch
    float turn_roll_limit_large = 0.0f;   // tuning+68h, +5A0h
    float turn_roll_pitch_limit_pitch_1 = 0.0f;  // tuning+6Ch, +5A4h
    float turn_roll_pitch_limit_pitch_2 = 0.0f;  // tuning+70h, +5A8h
    float turn_roll_pitch_limit_roll_1 = 0.0f;   // tuning+74h, +5ACh
    float turn_roll_pitch_limit_roll_2 = 0.0f;   // tuning+78h, +5B0h
    float pitch_turn_hdg_range_1 = 0.0f;  // tuning+8Ch, +5C4h
    float pitch_turn_hdg_range_2 = 0.0f;  // tuning+90h, +5C8h
    float soft_roll_ctrl = 0.0f;   // tuning+40h, +578h
    float soft_roll_mul = 0.0f;    // tuning+44h, +57Ch
    // tuning+48h, singleton +580h. The singleton records it as derived by
    // 007E6FD4 as (1 - SoftRollMul) * SoftRollCtrl - which is exactly the value
    // that makes the soft zone join the outside branch continuously. The two
    // were recovered in different packets and they agree, so the join is a
    // property of the data rather than a coincidence.
    float soft_roll_offset = 0.0f;
    float roll_spd = 0.0f;        // class+1A8h
    float roll_accel = 0.0f;      // class+1BCh
    float waggle_limit = 0.0f;    // tuning+0h, singleton +538h
};

struct PilotBotRollResult {
    float bank_target = 0.0f;   // plan+2C4h
    float desired = 0.0f;       // the roll slot's desired, clamped to [-1, +1]
    float hdg_ramp = 0.0f;      // the slot the PITCH arm's floor also reads
};

PilotBotRollResult pilot_plan_roll_0099e2ba(const PilotBotRollInputs& in);

// ---------------------------------------------------------------------------
// 0099E26E-0099E39D, the OTHER entry to the roll region: the servo arm.
//
// The mode-2 path 0099DE93-0099E25C computes a bank target and writes it to
// plan+2C4h; pilot_plan_roll_0099e2ba above models that one. A task that leaves
// plan+2CCh at anything but 2 reaches 0099E26E by the JNZ at 0099DE8D, PAST the
// planner's own MOV [ESI+2CCh],1 at 0099E264, and this arm runs instead: it
// computes NO target and servos toward whatever plan+2C4h already holds.
// docs/DIVE_BOMB_TASK.md, "The roll-arm gate".
//
// The dive-bomb turndown 009C44F0 is the caller that matters: at 009C4646 it
// writes plan+2C4h = pi and at 009C464E plan+2CCh = 1, handing the planner a
// 180-degree bank target to finish the roll to inverted.
//
// Jump senses from the branch bytes: 0099E2EB `76` JBE, 0099E312 `76` JBE,
// 0099E330 `76` JBE.
// ---------------------------------------------------------------------------
struct PilotBotRollServoInputs {
    // plan+2C4h, the target the task wrote. 0099E2C5 loads it as the first
    // argument to BSP_Math_SubtractWrappedAngle at 0099E2CE.
    float bank_target_2c4 = 0.0f;
    // The measured bank the error is taken against, [ESP+34h] at 0099E2BA.
    float bank = 0.0f;
    // [ESP+28h] at 0099E2D3, the scale on the raw error.
    float error_scale = 1.0f;
    // The three tuning floats the arm reads off EBX: +40h the band inside which
    // the proportional gain +44h applies, and +48h the constant rate outside it.
    float band_40 = 0.0f;
    float gain_44 = 1.0f;
    float rate_48 = 0.0f;
    // The rate limit 0099E344-0099E367 builds: (desc+1A8h / desc+1BCh / EBX[0])
    // times desc+1A8h. Supplied whole because its three fields are a contract.
    float rate_limit = 1.0f;
    // [ESP+2Ch] at 0099E36B, the fifth argument to BSP_Math_InterpolateClamped.
    float interpolant = 0.0f;
};
struct PilotBotRollServoResult {
    float bank_error = 0.0f;   // SubtractWrappedAngle(target, bank) * scale
    bool used_gain = false;    // the 0099E314 arm rather than 0099E332/0099E337
    float desired_290 = 0.0f;  // plan+290h, the roll command
};
// 0099E2BA-0099E39D.
PilotBotRollServoResult pilot_roll_servo_0099e26e(const PilotBotRollServoInputs& in);


// ---------------------------------------------------------------------------
// 0099D300's throttle arms. The plan's throttle slot is index 0 of five, so its
// `current` is plan+274h, its `desired` plan+278h and its `active` byte
// plan+27Ch (base plan+274h, stride 0Ch). An exhaustive store census over 278h
// finds four writes in this routine, and they are NOT one arm: each has its own
// gate, and two of them run only while the aircraft is on the ground.
//
//   0099D399  the centred-stick arm. Gate: plan+26Ch == 2 (0099D309), the byte
//             at [plan+2F0h + 9C2h + 8*[00F876B8]] set (0099D329), plan+270h
//             non-null (0099D33D) and the same byte on it set (0099D345).
//             Writes yaw, pitch, roll and air brake to 0.0 with their active
//             bytes and modes, and the throttle to [00D7A24C] = 1.0.
//   0099D8CF  the one-shot engine cut. Gate: plan+2D8h != 0 at 0099D7A5 (which
//             is what puts 0.001f from [00D7A23C] in XMM0 and jumps to the arm),
//             then plan+2D8h == 1 at 0099D8C1 and 0.001f > plan+2B4h at
//             0099D8C6. Writes the throttle 0.001f - below the 0.01f thrust gate
//             at 007DB76C, so the engine is off - and the air brake, then clears
//             plan+2D8h at 0099D8EB. NOT state gated: this is the only throttle
//             arm a FLYING plane can reach.
//   0099DC31  the ground demand. Inside `unit+900h == 5` (0099D8FD) and the
//             slot already active (0099D90A). One signed demand, seeded from the
//             slot's own value at 0099D977-0099D998 and accumulated at 0099DBC3,
//             is clamped to [-1, 1] by 00415690 and split: the throttle takes
//             max(0.001f, d) through 00415550 and the air brake max(0.0f, -d).
//   0099DC8F  the ground cap. Same state gate, slot NOT active. Reads the
//             slot's `current` and, when it exceeds [00CE3D30] = 0.6, commands
//             0.6.
//
// So the 0.6 CAP is ground only. The DEMAND is not: 0099D8CD's taken side jumps to
// 0099D924, which is past the state test at 0099D8FD, so a one-shot armed with a
// desired speed at or above 0.001f reaches the demand arm in any flight state.
// plan+2B4h is that desired speed - 009C1850 writes it at 009C189A and raises
// plan+2D8h at 009C18A7 in the same breath - and plan+2B8h, which EBP addresses
// from 0099D769, is the reference the demand divides the forward speed by.
// docs/PILOT_BOT_THROTTLE_DEMANDS.md.
inline constexpr float kPilotThrottleCutValue = 0.001f;    // 00D7A23C
inline constexpr float kPilotThrottleErrorLow = -6.9444447f;  // 00D1F3DC, -25 km/h
inline constexpr float kPilotThrottleErrorHigh = 6.9444447f;  // 00D0686C, +25 km/h
inline constexpr float kPilotThrottleGroundCap = 0.6f;     // 00CE3D30

struct PilotBotThrottleInputs {
    // The slot, read the way 0099D977-0099D998 and 0099DC7A read it.
    float slot_current = 0.0f;   // plan+274h
    float slot_desired = 0.0f;   // plan+278h
    bool slot_active = false;    // plan+27Ch

    int flight_state = 7;        // unit+900h; only 5 reaches the ground arms
    int air_brake_mode = 0;      // plan+2D8h; 1 arms the one-shot engine cut
    float one_shot_threshold = 0.0f;  // plan+2B4h, compared against 0.001f

    // The centred-stick gate, as four already-evaluated conditions so the caller
    // supplies the two per-slot bytes rather than this function chasing them.
    bool centred_mode_26c = false;     // plan+26Ch == 2
    bool centred_byte_2f0 = false;     // [plan+2F0h + 9C2h + 8*index]
    bool centred_block_270 = false;    // plan+270h non-null
    bool centred_byte_270 = false;     // [plan+270h + 9C2h + 8*index]

    // The demand's increment, 0099D99E-0099DBC7, now derived here rather than
    // supplied. It is a proportional speed controller:
    //
    //   ratio = measured_speed / speed_scale                 ; 0099D99E, 0099D9A3
    //   error = desired_speed - ratio                        ; 0099DA52, 0099DA58
    //   error -= correction                                  ; 0099DAC8
    //   inc    = Interp(-6.9444, -2.0, +6.9444, +2.0, error) ; 0099DB9E
    //   if (inc > 0) inc *= 0.6                              ; 0099DBB1, 00CEFF98
    //   inc   *= pending                                     ; 0099DBBF
    //
    // The interpolation's endpoints are +-6.9444 m/s, which is 25 km/h, mapped
    // onto +-2.0, so the controller saturates a quarter of the way to a typical
    // cruise error. `pending` is |slot value - plan+274h| from 0099D7E8-0099D81E,
    // so the increment is scaled by how far the slot has already been asked to
    // move and is ZERO for a slot whose desired equals its current - which is
    // every slot straight out of 0099B450's reset.
    float measured_speed = 0.0f;   // 007D99C0's forward speed
    float speed_scale = 1.0f;      // plan+2B8h, EBP from 0099D769
    float desired_speed = 0.0f;    // plan+2B4h, which 009C189A writes
    // SUBSTITUTION, labelled: |slot value - plan+274h| from 0099D7E8-0099D81E
    // was read as the whole story and is not. 0099D87E writes this slot again,
    // between that block and the increment, taking a running minimum against
    // XMM4 - [ESP+0x24] at 0099D85E-0099D892. Until that is read the multiplier
    // is a stand-in. docs/TORPEDO_GLIDE_THROTTLE_WIRING.md.
    float pending = 0.0f;
    // 0099DA97's unit vtable+38h is 007B8E60 in all nine plane vtables, and
    // 007B8E60 is `FLD [ECX+0B1Ch]; RET` - a cached float, not a computed
    // speed. 0042B2F0 is the 3D length of the vec3 at unit+AE0h. The term
    // 0099DAC8 subtracts is (unit+B1Ch - |unit+AE0h|) * 20.0 (00CE3D88) *
    // `pending`.
    //
    // SUBSTITUTION, labelled: exhaustive store censuses over B1Ch and AE0h find
    // no unit-range writer at all - five and three sites, all elsewhere in the
    // image. A store through a base already offset into the object would not
    // appear, so this is "no literal displacement writer", not "never written".
    // If both are in fact zero the whole term vanishes; the caller supplies it
    // so a run can decide.
    float error_correction = 0.0f;
    // 0099DB29-0099DB56, the dead band. 0099DB56 is JBE, so the skip needs
    // 0.5 AT OR BELOW desired/|ratio|, not above it: all four of
    // |error| <= 0.83333 (the double at 00D09450), ratio >= 1.0 (00D7A24C),
    // desired/|ratio| <= 1.5 (00CE380C) and desired/|ratio| >= 0.5 (00CE3800).
    // desired/|ratio| is formed at 0099DAEC-0099DAFF.
    bool dead_band_skips = false;
};

struct PilotBotThrottleResult {
    bool wrote_throttle = false;
    float throttle_desired = 0.0f;   // plan+278h
    bool wrote_air_brake = false;
    float air_brake_desired = 0.0f;  // plan+2A8h
    bool clears_air_brake_mode = false;  // plan+2D8h = 0
    bool centred_all_axes = false;   // the 0099D399 arm zeroed the other four
};

PilotBotThrottleResult pilot_plan_throttle_0099d300(const PilotBotThrottleInputs& in);

}  // namespace bsp
