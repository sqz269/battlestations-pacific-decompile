#include "bsp/torpedo_first_release.hpp"

// 009FA3A0 and the 009D2287 arm. docs/TORPEDO_FIRST_RELEASE.md.

namespace bsp {

ReleaseTimerResult release_timer_tick_009fa3a0(
    const ReleaseTimerInputs& in) noexcept {
    ReleaseTimerResult r;
    r.next_countdown_10 = in.countdown_10;
    // 009FA3A3 CMP byte [ESI+0Ch],0 / 009FA3A7 JZ 009FA409: a disabled timer
    // does not even advance.
    if (!in.enabled_0c) return r;

    // 009FA3A9 FLD / 009FA3AC FSUB / 009FA3B8 FST. FST, not FSTP: the value
    // stays on the x87 stack for the comparison below, which is why the test
    // sees the already-decremented value.
    const float expired = in.countdown_10 - in.step_seconds;
    r.next_countdown_10 = expired;

    // 009FA3BB FLDZ, 009FA3BD FCOMIP ST0,ST1, 009FA3C1 JBE. ST(0) is the zero
    // and ST(1) the new countdown, so the jump is taken when 0 <= countdown
    // and the release happens only on a strictly negative one.
    if (!(expired < 0.0f)) return r;

    // 009FA3C3 MOV ECX,[ESI] / 009FA3C5 CALL 007BB110 / 009FA3CC JZ.
    if (!in.unit_can_release_007bb110) {
        r.blocked_by_predicate = true;
        return r;
    }

    r.request_release_007bbba0 = true;  // 009FA3D0
    // 009FA3D5-009FA3F1: the draw replaces the countdown.
    r.next_countdown_10 = in.reseed_draw;
    // 009FA3FB / 009FA403 JNZ / 009FA405.
    if (in.slot_byte_9c0_clear) r.set_notify_0d = true;
    return r;
}

ReleaseTimerArmResult release_timer_arm_009d2287(
    bool already_enabled_0c) noexcept {
    ReleaseTimerArmResult r;
    r.enabled_0c = already_enabled_0c;
    // 009D2279 CMP byte [EBP+0Ch],0 / 009D227D JNE 009D228B.
    if (already_enabled_0c) return r;
    r.armed = true;
    r.countdown_10 = 0.0f;  // 009D227F XORPS / 009D2282 MOVSS
    r.enabled_0c = true;    // 009D2287
    return r;
}

}  // namespace bsp
