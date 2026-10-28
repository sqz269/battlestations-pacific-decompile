#pragma once

// Reach counters for cc13's plane-task and air-ops OFF switches (cc9-sweep3,
// log only). Each switched site computes both arms and counts the calls where
// they would differ, in either build. The other arm is evaluated under
// PlaneTaskReachShadow so that its nested calls are not counted twice. Printed
// by GameMissionLuaHost's report as `summary mission plane task reach (sw3)`.
// Nothing here changes a result.

#include <atomic>
#include <cstring>

namespace bsp {

struct PlaneTaskReachPair {
    std::atomic<unsigned long long> calls{0};
    std::atomic<unsigned long long> differs{0};   // the output (or decision) differs
    std::atomic<unsigned long long> inner{0};     // an intermediate value differs
};

struct PlaneTaskReachSw3 {
    // kStrafeX87FaithfulCc13Bound
    PlaneTaskReachPair strafe_countdown;   // 009CCF06 decrement decision
    PlaneTaskReachPair strafe_in_range;    // 009CCF58-009CCFF8; inner = threshold bits
    PlaneTaskReachPair strafe_off_nose;    // 00996510
    PlaneTaskReachPair strafe_distance;    // 00996300; inner = d2 bits
    // kFollowWaitFloatHelpersCc13Bound
    PlaneTaskReachPair follow_wait;        // 009BE3E0
    // kInterpClamped00419010NanCc13Bound: calls reaching the hi arm, differs = NaN v
    PlaneTaskReachPair interp_hi;
    // kRetreatX87FaithfulCc13Bound
    PlaneTaskReachPair retreat_zone;       // 009C8D40 stored direction / enter point
    PlaneTaskReachPair retreat_countdown;  // 009C9FB4 / 009CA135 decision; inner = countdown bits
    PlaneTaskReachPair retreat_arm;        // 009CA02C state
    PlaneTaskReachPair retreat_inside;     // 009C9EA0
    PlaneTaskReachPair retreat_steer;      // 009C9310 any output field
    // kDiveBombNanComparesCc13Bound
    PlaneTaskReachPair dive_ceiling;       // 009C7F23-009C7F46 ceiling bits
    PlaneTaskReachPair dive_breakoff;      // 009C8A90 answer
    // kAirOpsArriveSquadronWalkCc13Bound: differs = slot chosen; inner = deck slots after
    PlaneTaskReachPair airops_arrive;
    // kAirOpsReturnSlotClassArmCc13Bound: per returned slot, differs = +10h
    // differs; inner = the class+134h reader was installed
    PlaneTaskReachPair airops_return;
    // kVector3DivideInPlacePc24Cc13Bound (0042AE80): differs = any component's bits
    PlaneTaskReachPair vec3_divide;
    // kFreeAttackAcceptTestCc13Bound (009F830C): differs = the gate's answer;
    // inner = a category 1/2 command with a resolved target (the test ran)
    PlaneTaskReachPair free_attack;
    // cc9-sweep3c, the plane-logic OFF switches:
    // kSeg67bDogfightAimSpeedCorrOffBound (009A7932): calls = not-head-on aim
    // ticks; differs = plan+2B0h is not already 1
    PlaneTaskReachPair aim_speed_corr_off;
    // kDiveReleaseAltActivationOnlyBound (009C89E4-009C8A17): calls = ticks with
    // a script release altitude; differs = a non-activation tick changes the
    // block's state; inner = the leader block runs at all
    PlaneTaskReachPair dive_release_alt;
    // kMoveToActivationLuaAltOrderBound (009C375A): calls = the install's
    // leader store runs; differs = it clears a live +3ADh before 009C36BF
    PlaneTaskReachPair moveto_lua_alt_order;
    // kAiPlaneBagEquipmentBound (00A04608): calls = records that hold a round;
    // differs = the carried bag Equipment differs from the default
    PlaneTaskReachPair ai_bag_equipment;
    // kTorpedoArmFallthroughReleaseBound (009D49DA): calls = fall-throughs to
    // 0099B6A0; differs = consumed changes (a squadron); inner = 007BBBA0 runs
    PlaneTaskReachPair torpedo_fallthrough;
};

inline PlaneTaskReachSw3 g_plane_task_reach_sw3;
inline thread_local int g_plane_task_reach_sw3_shadow = 0;

struct PlaneTaskReachShadow {
    PlaneTaskReachShadow() noexcept { ++g_plane_task_reach_sw3_shadow; }
    ~PlaneTaskReachShadow() { --g_plane_task_reach_sw3_shadow; }
    PlaneTaskReachShadow(const PlaneTaskReachShadow&) = delete;
    PlaneTaskReachShadow& operator=(const PlaneTaskReachShadow&) = delete;
};

inline void plane_task_reach_count(PlaneTaskReachPair& pair, bool differs,
                                   bool inner = false) noexcept {
    if (g_plane_task_reach_sw3_shadow != 0) return;
    pair.calls.fetch_add(1, std::memory_order_relaxed);
    if (differs) pair.differs.fetch_add(1, std::memory_order_relaxed);
    if (inner) pair.inner.fetch_add(1, std::memory_order_relaxed);
}

// Bitwise, so two NaNs with the same payload compare equal.
inline bool plane_task_reach_bits_differ(float a, float b) noexcept {
    return std::memcmp(&a, &b, sizeof a) != 0;
}

}  // namespace bsp
