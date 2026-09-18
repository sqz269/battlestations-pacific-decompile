#include "bsp/torpedo_issue_timing.hpp"

// 007CE9FD-007CEB31, the tail stage of 007CE040 BSP_PlaneTickElement_FixedStep.
// docs/TORPEDO_ISSUE_TIMING.md.
//
// Register provenance for the whole stage, established by filtering the
// function's own listing for each register rather than from one idiom:
// ESI = ECX = `this` (007CE065), and the last write to EDI before the stage is
// 007CE848 `LEA EDI,[ESI-310h]`, with nothing but the alignment `MOV EDI,EDI`
// at 007CEAFE after it. So EDI is the unit and ESI the element embedded at
// unit+310h; the stage's ESI+910h/915h/918h/91Ch are unit+C20h/C25h/C28h/C2Ch
// and its EDI+900h is the plane control mode unit+900h.

namespace bsp {

bool issue_stage_mode_allows_007cea33(int control_mode_900) noexcept {
    // 007CEA39 JE, 007CEA41 JE, 007CEA46 JE, 007CEA4B JNE: a four-way
    // equality chain, not a range test.
    return control_mode_900 == kIssueModeFreeFlight_7 ||
           control_mode_900 == kIssueModeSpawned_6 ||
           control_mode_900 == kIssueModeOther_4 ||
           control_mode_900 == kIssueModeOther_5;
}

int release_request_raise_007bbc00(int issue_requests_c20) noexcept {
    return issue_requests_c20 + kReleaseRequestIncrement_007bbc00;
}

PlaneReleaseIssueStageResult plane_release_issue_stage_007ce9fd(
    const PlaneReleaseIssueStageInputs& in) noexcept {
    PlaneReleaseIssueStageResult r;
    r.next_interval_timer_c28 = in.interval_timer_c28;
    r.next_issue_requests_c20 = in.issue_requests_c20;

    // 007CEA02 JE, 007CEA16 JNE, 007CEA23 JNE, 007CEA2D JNE, 007CEA4B JNE.
    // All five land on 007CEB31, past the countdown, so a suppressed step
    // does not advance unit+C28h.
    if (in.app_state_1fe4 == kAppStateSuppressesIssue_1fe4) return r;
    if (in.blocked_9e0) return r;
    if (in.blocked_c3a) return r;
    if (in.blocked_5d) return r;
    if (!issue_stage_mode_allows_007cea33(in.control_mode_900)) return r;
    r.guards_passed = true;

    // 007CEA51 FLD unit+C28h, 007CEA57 FSUB the step, 007CEA66 FST back.
    // FST, not FSTP: the value stays on the x87 stack for the FCOMPI below
    // and for the COMISS of the cleanup arm, which is why both tests see the
    // already-decremented value.
    const float expired = in.interval_timer_c28 - in.step_seconds;
    r.next_interval_timer_c28 = expired;

    // 007CEA6C FLDZ, 007CEA6E FCOMPI ST(1), 007CEA72 JBE. ST(0) is the zero
    // and ST(1) the new timer, so the jump is taken when 0 <= timer and the
    // stage continues only on a strictly negative timer.
    r.arm = PlaneReleaseIssueStageArm::kWaiting;
    if (!(expired < 0.0f)) return r;

    // 007CEA78 MOV / 007CEA7E TEST / 007CEA80 JLE: signed, so zero takes the
    // cleanup arm.
    if (in.issue_requests_c20 > 0) {
        // 007CEA82 ADD EAX,-1 and 007CEA87 store back, both before the call.
        r.next_issue_requests_c20 = in.issue_requests_c20 - 1;
        r.call_issue_007c0d90 = true;  // 007CEA8D
        // 007CEAB3 the draw, 007CEAB8 FMUL the class field, 007CEABE FSTP.
        r.next_interval_timer_c28 = in.interval_draw * in.class_interval_scale_1f4;
        r.arm = PlaneReleaseIssueStageArm::kIssue;
        return r;
    }

    // 007CEAC6 CMP unit+C25h / JE: nothing to clean up if the walk never
    // raised the pending byte.
    if (!in.release_pending_c25) return r;
    // 007CEACF COMISS XMM0(-1.2) against the decremented timer, 007CEADC JBE:
    // continue only while -1.2 > timer.
    if (!(kCleanupDelay_00d05ea4 > expired)) return r;
    // 007CEAEE: the park happens before the device walk and before the count
    // test at 007CEAF6, so a blocked cleanup still retries on this cadence.
    r.next_interval_timer_c28 = kCleanupRetry_00ce69d0;
    // 007CEB0F JNE: one device answering true abandons the clear.
    if (in.any_device_busy_1fc) {
        r.arm = PlaneReleaseIssueStageArm::kCleanupBlocked;
        return r;
    }
    // 007CEB22 and 007CEB29.
    r.clear_release_pending_c25 = true;
    r.clear_c2c = true;
    r.arm = PlaneReleaseIssueStageArm::kCleanup;
    return r;
}

}  // namespace bsp
