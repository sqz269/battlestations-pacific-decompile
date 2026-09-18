#ifndef BSP_TORPEDO_ISSUE_TIMING_HPP
#define BSP_TORPEDO_ISSUE_TIMING_HPP

// When the plane fixed step is allowed to hand out release orders at all:
// the stage at 007CE9FD-007CEB31 inside 007CE040
// BSP_PlaneTickElement_FixedStep, which owns the only call site of
// 007C0D90 BSP_Plane_TickReleaseOrderIssue (007CEA8D, exhaustive rel32 and
// absolute-dword census: one site image wide).
//
// docs/TORPEDO_ISSUE_TIMING.md carries the evidence.
// docs/TORPEDO_RELEASE_ORDERS.md owns what happens once the stage fires
// (007C0D90 -> 007EEF30 -> 007BCBE0 and the queued count unit+C58h).
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout: the offset constants in the comments are the
// native ones, the structs are not.
//
// The correction this file carries. The stage is NOT free-running: an
// exhaustive disp32 scan of .text for every instruction that touches a
// +C20h field (reads included, 12 sites) finds the counter's one raiser at
// 007BBC00 `ADD dword [ECX+0xC20], EBX` with `EBX = 1` from 007BBBAB, the
// last instruction of 007BBBA0 BSP_Unit_RequestOrdnanceRelease before its
// `POP EBX; RET`. So the flight-wide budget is issued AFTER an aircraft has
// requested an ordnance release, not before. A store census alone misses
// that site, because it is an `ADD reg` and not a `MOV`.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, all read from the image.
// ---------------------------------------------------------------------------

// 007CEAAA / 007CEA92: the two ends of the interval draw that reseeds
// unit+C28h after an issue. 00CE3860 = 3F666666, 00CE6448 = 3F8CCCCD.
inline constexpr float kIssueIntervalLow_00ce3860 = 0.9f;
inline constexpr float kIssueIntervalHigh_00ce6448 = 1.1f;

// 007CEACF: 00D05EA4 = BF99999A. The cleanup arm runs only once the expired
// timer has gone this far past zero, so it polls rather than fires at once.
inline constexpr float kCleanupDelay_00d05ea4 = -1.2f;

// 007CEADE: 00CE69D0 = BF000000. The value the cleanup arm parks the timer
// at, which is 0.7 s of step time short of kCleanupDelay_00d05ea4, so a
// blocked cleanup retries on that cadence.
inline constexpr float kCleanupRetry_00ce69d0 = -0.5f;

// 007CEA33-007CEA4B: the four values of the plane control mode unit+900h
// that reach the stage. 7 is free flight (0074E210
// BSP_PlaneControlMode_IsFreeFlight tests the same field through unit+1D4h);
// 6 is the value 007C63F4 BSP_Plane_ChooseSpawnFlightState writes at spawn.
// 4 and 5 are not named here.
inline constexpr int kIssueModeFreeFlight_7 = 7;
inline constexpr int kIssueModeSpawned_6 = 6;
inline constexpr int kIssueModeOther_4 = 4;
inline constexpr int kIssueModeOther_5 = 5;

// 007CEA02: the app-state value that suppresses the stage,
// `CMP dword [[00E188A8]+1FE4h], 2`.
inline constexpr int kAppStateSuppressesIssue_1fe4 = 2;

// 007BBBAB: the constant 007BBBA0 adds to unit+C20h on every path, including
// its three early exits at 007BBBB0, 007BBBC6 and 007BBBDC.
inline constexpr int kReleaseRequestIncrement_007bbc00 = 1;

// ---------------------------------------------------------------------------
// 007CEA33: the control-mode test, `MOV EAX,[EDI+900h]` with EDI = ESI-310h
// from the LEA at 007CE848, so the field is on the unit, not the element.
// ---------------------------------------------------------------------------
bool issue_stage_mode_allows_007cea33(int control_mode_900) noexcept;

// ---------------------------------------------------------------------------
// 007BBC00, the tail of BSP_Unit_RequestOrdnanceRelease: the counter the
// stage spends. It is raised whether or not the request is accepted, because
// the ADD is past every early exit.
// ---------------------------------------------------------------------------
int release_request_raise_007bbc00(int issue_requests_c20) noexcept;

// ---------------------------------------------------------------------------
// The stage itself.
// ---------------------------------------------------------------------------

enum class PlaneReleaseIssueStageArm : int {
    // 007CEB31, reached from one of the five guards. The countdown is not
    // even advanced.
    kSuppressed = 0,
    // The countdown ran but has not expired, or it expired with nothing to
    // spend and nothing to clean up.
    kWaiting = 1,
    // 007CEA82-007CEABE: spend one request, call 007C0D90, reseed the timer.
    kIssue = 2,
    // 007CEB1F-007CEB29: the counter is spent and every device answered
    // false, so unit+C25h and unit+C2Ch are cleared.
    kCleanup = 3,
    // 007CEB0F: the same arm, but a device answered true, so only the timer
    // was parked.
    kCleanupBlocked = 4,
};

struct PlaneReleaseIssueStageInputs {
    // 007CEA02: `[00E188A8]+1FE4h`. contract: the app-state singleton field
    // is not modelled, so a host that has no pause reports 0.
    int app_state_1fe4 = 0;
    // 007CEA0F: the element byte ESI+6D0h = unit+9E0h. The same field
    // 007CEC4E's airborne accumulator is gated on.
    bool blocked_9e0 = false;
    // 007CEA1C: unit+C3Ah.
    bool blocked_c3a = false;
    // 007CEA29: unit+5Dh.
    bool blocked_5d = false;
    // 007CEA33: unit+900h.
    int control_mode_900 = 0;

    // 007CEA51 / 007CEA57 / 007CEA66: unit+C28h -= step, stored back with an
    // FST that keeps the value on the stack for the later COMISS.
    float interval_timer_c28 = 0.0f;
    float step_seconds = 0.0f;

    // 007CEA78: unit+C20h.
    int issue_requests_c20 = 0;

    // 007CEAC6: unit+C25h, the byte 007C0EE2 raises inside the device walk.
    bool release_pending_c25 = false;
    // 007CEB00-007CEB0F: the element's device array ESI+664h, count
    // ESI+684h, vtable slot +1FCh. True when any of them answers non-zero.
    // contract: the six vtable slots of the walk are not read.
    bool any_device_busy_1fc = false;

    // 007CEAB3: BSP_Random_UniformFloatRange(1, 00CE3860, 00CE6448), pushed
    // low first. The caller supplies the draw so the rule stays pure.
    float interval_draw = 1.0f;
    // 007CEAB8: `FMUL dword [EBX+1F4h]` with EBX = [ESI+228h] = unit+538h,
    // the class descriptor. SUBSTITUTION in a host that has no descriptor.
    float class_interval_scale_1f4 = 1.0f;
};

struct PlaneReleaseIssueStageResult {
    PlaneReleaseIssueStageArm arm = PlaneReleaseIssueStageArm::kSuppressed;
    // False when one of the five guards at 007CEA02-007CEA4B rejected.
    bool guards_passed = false;
    // The value to write back to unit+C28h. Equal to the input when the
    // guards rejected, because 007CEA51 is past them.
    float next_interval_timer_c28 = 0.0f;
    // The value to write back to unit+C20h.
    int next_issue_requests_c20 = 0;
    // 007CEA8D: whether to run 007C0D90 this step.
    bool call_issue_007c0d90 = false;
    // 007CEB22 / 007CEB29.
    bool clear_release_pending_c25 = false;
    bool clear_c2c = false;
};

PlaneReleaseIssueStageResult plane_release_issue_stage_007ce9fd(
    const PlaneReleaseIssueStageInputs& in) noexcept;

}  // namespace bsp

#endif  // BSP_TORPEDO_ISSUE_TIMING_HPP
