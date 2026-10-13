// Packet cc13-ver22 (docs/CC13_VER_22.md): the AI fighter's air-to-air decisions that
// the host's dogfight binding leaves out. Names are hypotheses, not recovered symbols.
// Every answer here is read from the disk bytes of battlestationspacific.exe; the
// callers supply the entity bytes. The aim point's lead time is already
// dogfight_latch_009aac70's time_to_target (include/bsp/dogfight_task.hpp), and the
// lead itself is approach_target_ref_lead_tail_009faf05
// (include/bsp/approach_target_ref.hpp); they are not repeated here.
#pragma once

namespace bsp {

// The tail of 009A76E0 BSP_BotStateDogfightAim_Tick (009A797F-009A79A9), run after the
// aim state's steering and throttle on every aim tick with a target (+B4h != 0).
struct DogfightAimAdoptFacts {
    bool gun_target_present{false};   // [(approach+1Ch)+74h] != 0 (009A7987)
    bool gun_target_is_current{false};  // == approach+B4h (009A798C)
    bool gun_target_has_squadron{false};  // [gun_target+9D4h] != 0 (009A7994)
    bool squadron_is_target_squadron{false};  // == approach+CCh (009A799E)
};

// True when the aim tick hands the gun controller's auto target to 009A7650
// BSP_BotTaskDogfight_SetTarget (009A79A9): a gun target that is not the current one
// and flies in the squadron the task was ordered against. gun+74h is the value the
// previous gun tick (009FC7C0, run after the task arm) left; it is not cleared at that
// tick's tail.
bool dogfight_aim_adopts_gun_target_009a797f(const DogfightAimAdoptFacts& f) noexcept;

// The states of the dogfight task as they reach the gun controller's cone
// (approach+1Ch)+40h, which 009FC7C0 compares with 1 - cos(lead, +68h) before its fine
// aim and scales by 1.25 for the 007B96F0 search; the tick zeroes it at its tail
// (009FCE64), so a state that writes nothing leaves 0.
enum class DogfightConeState {
    kAim,          // 009A7952 FLD [tuning+678h] (Angle_Strafe), 009A7960 FSTP [+40h]
    kAttackRun,    // 009A744E FLD [tuning+670h] (Angle_MoveTo), 009A7459 FSTP [+40h]
    kMoveTo,       // the generic move-to tick 009C18C0: 009C1B2D FLD [tuning+670h]
    kFollowFlyTo,  // 009BEE30's fly-to arm: 009BFD2B FLD [tuning+670h]
    kFollowHold,   // 009BEE30's other arm: 009BF9D7 FLD [tuning+66Ch] (Angle_Prepare)
    kOther,        // prepare, maneuver, avoid roll and turn: no store in the band
};

// The GameTuning offset whose float the state stores in the cone, or 0 for none.
// docs/GAME_TUNING_SINGLETON.md: +66Ch Angle_Prepare, +670h Angle_MoveTo, +678h
// Angle_Strafe (Pilot/AutoStrafeAngle, radians).
unsigned dogfight_cone_tuning_offset(DogfightConeState s) noexcept;

}  // namespace bsp
