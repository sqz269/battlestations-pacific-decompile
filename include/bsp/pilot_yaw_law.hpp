#pragma once

namespace bsp {

// The yaw arm of `0099D300 BSP_PilotBot_PlanControls`: how a desired heading
// becomes the yaw plan slot's `desired`.
//
// Assembled from two packets that met. `docs/PILOT_BOT_PLAN_CONTROLS.md`
// recovered the arm's body at `0099E81A`-`0099EA46` and had to stop, because the
// `base` term at `[ESP+18h]` was unread and it said so rather than guessing:
// "with `base` unknown, any C++ for this arm would be a guess wearing an
// address." `docs/PILOT_COMMAND_BAND_REPAIR.md` then traced `base` back through
// `0099DDBE`-`0099DF24`, and `docs/PILOT_BOT_TICK_GATES.md` established that the
// desired heading is already in `plan+2C0h` when the planner runs - the planner
// never touches the task's target entity.
//
// **The six tuning offsets are what makes this arm safe to wire.** The law was
// read out of the instruction stream; the offsets it reads land on
// `Pilot/General/SoftHdgMul`, `SoftHdgLimit`, `SoftHdgZone`,
// `YawTurnRollRange/1`, `YawTurnRollRange/2` and `YawCtrlSetTimeMul`, whose
// names were recovered in a different packet from `planeglobals.lua`. A deadband
// the listing applies is called `SoftHdgZone`; a soft limit it applies is called
// `SoftHdgLimit`; a bank-angle blend range is called `YawTurnRollRange`. Two
// independent sources agreeing term for term is the evidence here, not the
// listing alone.

// `0042E740 BSP_GameTuning_GetSingleton() + 538h` is the block the arm indexes,
// so `tuning+34h` is singleton `+56Ch` and so on.
struct PilotYawTuning {
    float soft_hdg_mul = 0.0f;           // tuning+34h, singleton +56Ch
    float soft_hdg_limit = 0.0f;         // tuning+38h, singleton +570h
    float soft_hdg_zone = 0.0f;          // tuning+3Ch, singleton +574h, default 0.01
    float yaw_turn_roll_range_1 = 0.0f;  // tuning+7Ch, singleton +5B4h
    float yaw_turn_roll_range_2 = 0.0f;  // tuning+80h, singleton +5B8h
    float yaw_ctrl_set_time_mul = 0.0f;  // tuning+9Ch, singleton +5D4h
};

struct PilotYawInputs {
    // plan+2CCh. The heading term is gated on this being exactly **2**; any
    // other value skips the whole block (`0099DE8D JNZ 0099E26E`) and leaves the
    // error at the zero stored at `0099DDD0`, so the arm runs on its turn term
    // alone. `0099D6C6` clears it when a direct roll stick input is present, so
    // a human roll input disables the bot's heading hold.
    int heading_mode_2cc = 0;
    float desired_heading = 0.0f;  // plan+2C0h, read once in the whole planner
    float current_heading = 0.0f;  // unit+C6Ch, through unit->vtable[50h] = 0074E260
    float bank = 0.0f;             // unit+C68h
    // unit+340h. The error is divided by `max(unit+340h * 0.4, 1.0)`, so a zero
    // here is the identity. Its producer is not established.
    float scale_340 = 0.0f;
    float yaw_spd = 0.0f;        // class+1B0h
    float turn_roll_spd = 0.0f;  // class+1C8h
    float pitch_spd = 0.0f;      // class+1ACh
    // `[ESP+10h]`, the turn arm's numerator. The turn term applies only when
    // this is positive (`0099E88E`). Its producer is NOT established - the
    // caller supplies it, and 0.0f disables the term, which is the honest
    // default rather than an invented one.
    float turn_numerator = 0.0f;
};

// Returns what `0099EA3E` stores to the yaw slot's `desired`, already clamped to
// [-1, +1]. The caller sets the slot's `active` byte, which `0099EA46` does.
float pilot_yaw_desired_0099ea3e(const PilotYawTuning& tuning,
                                 const PilotYawInputs& in) noexcept;

}  // namespace bsp
