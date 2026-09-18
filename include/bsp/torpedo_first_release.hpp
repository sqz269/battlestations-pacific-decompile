#ifndef BSP_TORPEDO_FIRST_RELEASE_HPP
#define BSP_TORPEDO_FIRST_RELEASE_HPP

// Who authorises the FIRST ordnance release of a torpedo flight.
//
// docs/TORPEDO_FIRST_RELEASE.md carries the evidence.
// docs/TORPEDO_ISSUE_TIMING.md owns the loop this answers: the flight-wide
// budget unit+C58h is raised by a stage that needs unit+C20h, and unit+C20h is
// raised only by 007BBBA0 BSP_Unit_RequestOrdnanceRelease. Something must call
// 007BBBA0 without a budget or the loop never starts.
//
// The answer, from the exhaustive call-site census of 007BBBA0 (34 sites): the
// aim state's own release timer. 009D15F0 BSP_BotStateTorpedoAim_Tick arms it
// at 009D2287 when all five release flags coincide and the ground probe
// answers below 1.0, and ticks it at 009D2027 through 009FA3A0, which calls
// 007BBBA0 at 009FA3D0 behind nothing but 007BB110, the unit's own
// can-release predicate. No queued release order, no prepare state, no manual
// passthrough count.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout.

namespace bsp {

// ---------------------------------------------------------------------------
// 009FA3A0, `void __thiscall(timer, float dt)`, RET 4 at 009FA40A.
// ECX is the timer object, which the aim state holds at state+18h
// (009D2019 LEA EBP,[ESI+18h]). Its fields, all read from the listing:
//   +0h   the unit, the ECX of both 007BB110 and 007BBBA0
//   +4h   the reseed low bound  (009FA3E4, pushed first)
//   +8h   the reseed high bound (009FA3D5, pushed second)
//   +0Ch  the enable byte       (009FA3A3)
//   +10h  the countdown         (009FA3A9)
//   +0Dh  a notify byte         (009FA405)
// ---------------------------------------------------------------------------

struct ReleaseTimerInputs {
    bool enabled_0c = false;        // 009FA3A3
    float countdown_10 = 0.0f;      // 009FA3A9
    float step_seconds = 0.0f;      // 009FA3AC
    // 009FA3C5 007BB110(unit): the device at unit+DECh is enabled, its +64h is
    // not 1.0, and the per-slot byte unit+9C3h+slot*8 is clear. contract in a
    // host with no device model.
    bool unit_can_release_007bb110 = false;
    // 009FA3EA BSP_Random_UniformFloatRange(1, timer+4h, timer+8h). The caller
    // supplies the draw so the rule stays pure.
    float reseed_draw = 0.0f;
    // 009FA3FB CMP byte [unit + slot*8 + 9C0h], 0. Note 9C0h, not the 9C3h the
    // decompiler renders; JNZ skips, so the notify byte is set only when it is
    // clear. contract: unread.
    bool slot_byte_9c0_clear = false;
};

struct ReleaseTimerResult {
    // 009FA3B8 FST: the decremented value is stored back whenever the timer is
    // enabled, whether or not it fires.
    float next_countdown_10 = 0.0f;
    // 009FA3D0.
    bool request_release_007bbba0 = false;
    // 009FA405.
    bool set_notify_0d = false;
    // True when the countdown expired but 007BB110 refused.
    bool blocked_by_predicate = false;
};

ReleaseTimerResult release_timer_tick_009fa3a0(
    const ReleaseTimerInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009D2279-009D2287, the arm inside the aim tick. It runs only when all five
// release flags coincide and 009D2272 finds 1.0 above the ground probe
// 00903860. It refuses to re-arm a timer that is already enabled, and it sets
// the countdown to ZERO (009D2282 XORPS/MOVSS), so the drop lands on the next
// tick rather than after a delay.
// ---------------------------------------------------------------------------

struct ReleaseTimerArmResult {
    bool armed = false;           // false when 009D227D found it already on
    float countdown_10 = 0.0f;    // 009D2282
    bool enabled_0c = false;      // 009D2287
};

ReleaseTimerArmResult release_timer_arm_009d2287(
    bool already_enabled_0c) noexcept;

// ---------------------------------------------------------------------------
// The two release authorities that need no budget, for the record. Neither is
// reconstructed here; both are named so a host can tell them apart.
// ---------------------------------------------------------------------------

// 007CCFA0 BSP_Plane_HandleMessage, the arm at 007CD0B2-007CD0C0: `MOV ECX,EDI`
// then `CALL 007BBBA0`, `MOV AL,1`, `RET 4`. No guard of any kind. The message
// kind is the low byte of msg+10h and this arm is 0C4h.
inline constexpr int kPlaneMessageReleaseOrdnance_c4 = 0xC4;

// 009D4850 BSP_BotTaskTorpedo_TickArm, the arm at 009D4956: needs task+424h
// above zero (009D48F8), the unit at task+3FCh, the device predicate
// [unit+72Ch]->vtable[38h] and a current state that is none of task+710h,
// task+6B4h or task+740h. task+424h has no raiser in the image.
inline constexpr int kTorpedoTaskManualPassthrough_424 = 0x424;

}  // namespace bsp

#endif  // BSP_TORPEDO_FIRST_RELEASE_HPP
