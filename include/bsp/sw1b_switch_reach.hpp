#pragma once

// Reach counters for the cc13 precision and order switches paired by cc9-sweep1b.
// In either setting each counter counts the evaluations of a switched site
// (sites) and those where the switch's ON and OFF arms would give a different
// answer (differs: a different float bit pattern, or a different compare
// outcome). "summary mission switch reach (sw1b)" in game_hosts_mission_frame.cpp
// prints them. Instrumentation only: nothing reads them back.

#include <cstdint>
#include <cstring>

namespace bsp {

enum class Sw1bReach : int {
    PlaneResist40Deadband,   // kPlaneResist40DeadbandCc13Bound, 007D85A0
    PlaneFlightX87,          // kPlaneFlightX87Cc13Bound, 007DBA67 / 007DBEBB / 007D8D6D
    KamikazeX87Float,        // kKamikazeX87FloatCc13Bound, 009AC0B0 / 009ADE60 / 009AE9D0
    PlaneYawRollOrder,       // kPlaneYawRollCouplingOrderCc13Bound, 007DAA5C
    DynamicsBuoyancy,        // kDynamicsBuoyancyWholeBodyCc13Bound, 004462D0
    PlaneDragOrder,          // kPlaneDragOrderCc13Bound, 007D91D2 / 007D924E
    OrthoOffCenterPc24,      // kOrthoOffCenterPc24Cc13Bound, ortho_off_center_lh
    MissionDetailOffsetLerp, // kMissionDetailOffsetLerpBound, 00414130
    ScaleVector4Pc24,        // kScaleVector4Pc24Cc13Bound, 00BEE140
    CameraPathClosed,        // kPc24CameraCc13Bound, 007AF297 closed test
    CameraPathKnot,          // kPc24CameraCc13Bound, 007AF5A5 knot length
    Count
};

struct Sw1bReachCounter {
    unsigned long long sites{0};
    unsigned long long differs{0};
};

inline Sw1bReachCounter& sw1b_reach(Sw1bReach k) noexcept {
    static Sw1bReachCounter table[static_cast<int>(Sw1bReach::Count)]{};
    return table[static_cast<int>(k)];
}

inline const char* sw1b_reach_name(Sw1bReach k) noexcept {
    static const char* const names[static_cast<int>(Sw1bReach::Count)] = {
        "kPlaneResist40DeadbandCc13Bound", "kPlaneFlightX87Cc13Bound",
        "kKamikazeX87FloatCc13Bound", "kPlaneYawRollCouplingOrderCc13Bound",
        "kDynamicsBuoyancyWholeBodyCc13Bound", "kPlaneDragOrderCc13Bound",
        "kOrthoOffCenterPc24Cc13Bound", "kMissionDetailOffsetLerpBound",
        "kScaleVector4Pc24Cc13Bound", "kPc24CameraCc13Bound/007AF150-closed",
        "kPc24CameraCc13Bound/007AF150-knot"};
    return names[static_cast<int>(k)];
}

inline bool sw1b_same_bits(float a, float b) noexcept {
    std::uint32_t x;
    std::uint32_t y;
    std::memcpy(&x, &a, sizeof x);
    std::memcpy(&y, &b, sizeof y);
    return x == y;
}

inline void sw1b_note(Sw1bReach k, bool differs) noexcept {
    Sw1bReachCounter& c = sw1b_reach(k);
    ++c.sites;
    if (differs) ++c.differs;
}

inline void sw1b_note(Sw1bReach k, float on, float off) noexcept {
    sw1b_note(k, !sw1b_same_bits(on, off));
}

}  // namespace bsp
