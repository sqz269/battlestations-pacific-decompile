#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_forces.hpp"   // kShipClassOffMaxSpeed, and through it unit_motion.hpp
#include "bsp/unit_instance.hpp" // kUnitOffPoseValid, kUnitOffPoseBlock, kUnitOffClassBlock
#include "bsp/world_ocean.hpp"   // OceanVec3

// The three timed sub-updates 00834E90 ends with, and the timer fields they own.
// Addresses: 00834820, 00834A70, 00834CC0, 00439820, 00866B70, 00867B10, 0078CF20.
// Evidence and uncertainty are recorded in docs/UNIT_TIMERS.md.
//
// 00834E90 (docs/UNIT_TIMED_SUBUPDATES.md, the third timed sub-update of
// 008255B0) closes with three more __thiscall(this, float delta) routines at
// 0083553E..0083556F: 00834820, 00834CC0, 00834A70, each RET 4 and each given
// the same scaled frame delta. All three drive a wake or spray point effect
// from the water surface and hold their own countdown in the unit instance.
//
// Nothing here is a binary-compatible replacement. The native routines are
// __thiscall members of a class whose layout is only partly recovered, the
// point-effect instance behind every handle is an external contract
// (docs/POINT_EFFECT_INSTANCE.md), and the descriptive names are hypotheses
// rather than recovered symbols.

namespace bsp {

// ---------------------------------------------------------------------------
// Unit fields these three routines own. The offsets kUnitOff* already declared
// in bsp/unit_instance.hpp and bsp/unit_motion.hpp are reused, not restated.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitOffBowWaveEffect = 0x9E8;   // 00834827, 008241A7
inline constexpr std::size_t kUnitOffSternWaveEffect = 0x9EC; // 00834CC8, 008246D0
inline constexpr std::size_t kUnitOffBowWaveTimer = 0x9F8;    // 00834988
inline constexpr std::size_t kUnitOffSternWaveTimer = 0x9FC;  // 00834E17
inline constexpr std::size_t kUnitOffSprayEffects = 0xA00;    // 00834AB8, 008247D1
inline constexpr std::size_t kUnitOffSprayTimer = 0xA10;      // 00834AFB
inline constexpr std::size_t kUnitOffWakeProbeNear = 0x1054;  // 00834878, 00823B87
inline constexpr std::size_t kUnitOffWakeProbeFar = 0x1060;   // 0083489A, 00823BB1

// 008247D7: the producer fills exactly four spray slots, whatever the class
// says. 00834A70's loop bound is the class count at +654h; see the bound
// mismatch recorded in docs/UNIT_TIMERS.md.
inline constexpr int kUnitSprayEffectSlots = 4;

// ---------------------------------------------------------------------------
// Ship class-block fields. kShipClassOffMaxSpeed (+500h) comes from
// bsp/unit_forces.hpp; the rest are first recorded here.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kShipClassOffHullExtent = 0x0A0;      // Length, Width, Height
inline constexpr std::size_t kShipClassOffWakeProbeNear = 0x594;   // 00823B81
inline constexpr std::size_t kShipClassOffWakeProbeFar = 0x5A0;    // 00823BAB
inline constexpr std::size_t kShipClassOffBowWaveTemplate = 0x630; // 0082416D, `BowWave`
inline constexpr std::size_t kShipClassOffSternWaveTemplate = 0x634; // 0082469A, `WaveStern`
inline constexpr std::size_t kShipClassOffSternProbeNear = 0x638;  // 00834D07
inline constexpr std::size_t kShipClassOffSternProbeFar = 0x644;   // 00834D29
inline constexpr std::size_t kShipClassOffSprayPoints = 0x650;     // 00834B4A
inline constexpr std::size_t kShipClassOffSprayPointCount = 0x654; // 00834AA2
inline constexpr std::size_t kShipClassOffSprayTemplate = 0x65C;   // 00824869, `RotorParticle`

// ---------------------------------------------------------------------------
// Literals, each with the address of the word the native code loads.
// ---------------------------------------------------------------------------

// 00CE3D40, the double 0x3FE99999A0000000, which is float 0.8f promoted. The
// bow and stern wave timers stop their effect once they pass it.
inline constexpr float kUnitWaveTimeout = 0.8f;
// 00CE3D30, 0x3F19999A. The spray timer is reloaded with this while the unit
// is under power.
inline constexpr float kUnitSprayHold = 0.6f;
// 00D7A238, 0x3C23D70A. The throttle magnitude that counts as "under power".
inline constexpr float kUnitSprayThrottleEpsilon = 0.01f;
// 00D7A2B0, the double 3.0. A spray point is published only while it is below
// the water surface plus this band.
inline constexpr double kUnitSpraySurfaceBand = 3.0;
// 00D09DB0, the double 0x4008CCCCC0000000, float 3.1f promoted. Subtracted
// from the band to give the height the point is pulled down to.
inline constexpr double kUnitSprayDepthOffset = 3.0999999046325684;
// 00D7A2F0 (0x3DCCCCCD) and the double 00D7A3A0, the same value: the floor of
// the spray scalar and the threshold the scalar is tested against.
inline constexpr float kUnitSprayScalarFloor = 0.1f;
// 00D7A24C, 0x3F800000. The ceiling of the spray scalar, and the numerator of
// the 1/MaxSpeed the wave routines publish.
inline constexpr float kUnitSprayScalarCeiling = 1.0f;

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 008348A9..0083495D (00834D38..00834DE5 is the same shape). Both wave
// routines take two probe points in world space and the water height sampled
// under the FIRST of them, then intersect the segment with that height.
//
// The native order matters and is preserved: the far-minus-near height
// difference is rounded to a double at 008348C0 before the divide, the
// fraction is rounded to a float at 008348E9, each per-axis difference is
// rounded to a float, each difference-times-fraction product is rounded to a
// float, and each sum with the near probe is rounded to a float again. There
// is no guard on a zero height difference: two probes at the same height give
// an infinite fraction and a non-finite point, which the straddle test below
// then rejects (an unordered compare takes the accumulate arm).
struct UnitWaterLineSolve {
    OceanVec3 point{};      // the interpolated point, 00834943/0083494D/00834959
    float fraction{0.0f};   // the stored float t, 008348E9
};
UnitWaterLineSolve unit_water_line_point_008348a9(const OceanVec3& near_probe,
                                                  const OceanVec3& far_probe,
                                                  float surface_height) noexcept;

// 0083495D..00834996. True when the solved height lies strictly between the
// two probe heights in either order, which is what resets the wave timer. The
// native chain is four FCOMI/FCOMIP tests, all strict; an unordered compare
// (a non-finite solved height) falls through to the accumulate arm, so this
// returns false for it.
bool unit_water_line_straddles_0083495d(float solved_y,
                                        float near_probe_y,
                                        float far_probe_y) noexcept;

// 00834985 and 00834996..008349BA. The bow and stern wave timer: cleared while
// the water line crosses the probe segment, otherwise advanced by the frame
// delta. `expired` is the strict FCOMIP at 008349B6 against kUnitWaveTimeout;
// the caller stops the effect and returns on it. The native code writes the
// advanced value back to the unit before the comparison, so the timer keeps
// the over-threshold value.
struct UnitWaveTimerStep {
    float timer{0.0f};
    bool expired{false};
};
UnitWaveTimerStep unit_wave_timer_00834996(float timer, float delta, bool straddles) noexcept;

// 00834A78..00834A9A. The magnitude of the throttle at unit+980h, taken the
// way the native code takes it: a strict COMISS against 0.0f at 00834A80 and,
// on the not-greater arm, the subtraction 0x80000000 - x at 00834A96. A NaN
// throttle takes the subtraction arm.
float unit_throttle_magnitude_00834a78(float throttle) noexcept;

// 00834AEE..00834B29. The spray timer, which is a hold rather than a
// countdown: reloaded to kUnitSprayHold while the throttle magnitude reaches
// kUnitSprayThrottleEpsilon, otherwise reduced by the frame delta. `alive` is
// the 0.0f <= timer test at 00834B20; on false the caller stops that slot's
// effect. The native code runs this per non-null slot inside the loop, so a
// unit with several live slots subtracts the delta several times per frame.
struct UnitSprayTimerStep {
    float timer{0.0f};
    bool alive{false};
};
UnitSprayTimerStep unit_spray_timer_00834aee(float timer,
                                             float delta,
                                             float throttle_magnitude) noexcept;

// 00834C04..00834C3A. The spray point is published only while its height is
// under the water surface plus kUnitSpraySurfaceBand, and is then pulled down
// to at most that limit minus kUnitSprayDepthOffset. Both the limit and the
// pulled-down height are rounded to floats at 00834C0A and 00834C20; both
// comparisons are strict.
struct UnitSprayPointClamp {
    bool publish{false};
    float y{0.0f};
};
UnitSprayPointClamp unit_spray_point_clamp_00834c04(float point_y, float surface_height) noexcept;

// 00834C3A..00834C74. The scalar the spray effect is given: twice the throttle
// magnitude clamped into [kUnitSprayScalarFloor, kUnitSprayScalarCeiling]. The
// floor arm is taken when the doubled value is strictly below the floor.
float unit_spray_scalar_00834c3a(float throttle_magnitude) noexcept;

// ---------------------------------------------------------------------------
// The host. One pure-virtual method per native call site these three routines
// make. Every method names the callee it stands for; nothing here decides what
// the callee does beyond what docs/UNIT_TIMERS.md records.
// ---------------------------------------------------------------------------

// The point-effect instance the handles point at. Incomplete by design: its
// storage is PointEffectInstanceStorage in bsp/point_effect_instance.hpp and
// this module never dereferences it.
struct UnitTimedEffect;

struct UnitTimedSubUpdateHost {
    virtual ~UnitTimedSubUpdateHost() = default;

    // 00414DB0, BSP_EntityPose_RefreshWorld, called on the unit whenever the
    // byte at kUnitOffPoseValid is clear.
    virtual void refresh_unit_pose() = 0;

    // 00439820: transform a unit-local point by the unit's pose matrix at
    // kUnitOffPoseBlock and divide by the resulting w. 00834A70 inlines the
    // same body at 00834B4A..00834BE6 instead of calling it (00439820..004398D4).
    virtual OceanVec3 transform_by_unit_pose(const OceanVec3& local_point) = 0;

    // 0078CF20 on (*00E188A8)->+19F0h, the ocean surface height under an x/z
    // pair. The two wave routines sample under their near probe; the spray
    // routine samples under the transformed spray point.
    virtual float water_height(float x, float z) = 0;

    // 00866B70: clear the effect's stop byte, and set its byte +0Ah when its
    // byte +8h is set, under the effect manager's critical section.
    virtual void effect_resume(UnitTimedEffect* effect) = 0;

    // 00867B10: move the effect's live rows into its own reference array and
    // release them, under the same critical section. The live Ghidra symbol is
    // BSP_PointEffect_StopChildren.
    virtual void effect_stop(UnitTimedEffect* effect) = 0;

    // 004842C0: publish a world point to the effect. See
    // include/bsp/unit_water_anchors.hpp for its rule.
    virtual void effect_set_point(UnitTimedEffect* effect, const OceanVec3& world_point) = 0;

    // The unit's own virtual at vtable +38h, called at 008349CE and 00834E59
    // and returning a float in ST0. Contract: unread. Both wave routines store
    // the result next to 1/MaxSpeed, which is why docs/UNIT_TIMERS.md records
    // a speed only as the hypothesis.
    virtual float unit_virtual_38() = 0;

    // 0042D7E0, BSP_EntityPose_GetWorldMatrix on the unit, of whose result
    // 00834820 uses only the three floats at +20h..+28h.
    virtual OceanVec3 unit_world_matrix_row2() = 0;

    // Stores into the point-effect instance, which this module does not model:
    // its storage is PointEffectInstanceStorage in bsp/point_effect_instance.hpp.
    // The offset is passed through so that the native field and the native
    // order of the stores stay visible at the call site.
    virtual void effect_store_float(UnitTimedEffect* effect,
                                    std::size_t offset, float value) = 0;
};

// The point-effect fields these three routines write. +50h and +54h are the
// first two floats of PointEffectInstanceStorage::fields_50, which the
// constructor leaves at 1.0f.
inline constexpr std::size_t kPointEffectOffScale = 0x50;      // 008349F3, 00834C80
inline constexpr std::size_t kPointEffectOffValue = 0x54;      // 008349D0, 00834C7B
inline constexpr std::size_t kPointEffectOffSteering = 0x58;   // 008349DD
inline constexpr std::size_t kPointEffectOffField2C = 0x2C;    // 00834A20
inline constexpr std::size_t kPointEffectOffAxisX = 0x68;      // 00834A11
inline constexpr std::size_t kPointEffectOffAxisY = 0x6C;      // 00834A16
inline constexpr std::size_t kPointEffectOffAxisZ = 0x70;      // 00834A1B
inline constexpr std::size_t kPointEffectOffExtentA = 0x74;    // 00834A48, class+A4h Width
inline constexpr std::size_t kPointEffectOffExtentB = 0x78;    // 00834A4D, class+A8h Height
inline constexpr std::size_t kPointEffectOffExtentC = 0x7C;    // 00834A52, class+A0h Length

// ---------------------------------------------------------------------------
// The state each routine reads and writes. Only the fields the rules touch are
// present; the class block appears as the values that are read out of it.
// ---------------------------------------------------------------------------

struct UnitWaveSubUpdateState {
    UnitTimedEffect* effect{nullptr}; // +9E8h for the bow, +9ECh for the stern
    // 00834846..0083485A: only 00834820 refuses to run when the game singleton
    // at 00E188A8 or its ocean at +19F0h is null. 00834CC0 and 00834A70 make
    // no such test and dereference the same two pointers regardless, which is
    // the asymmetry docs/UNIT_TIMERS.md records. The stern routine ignores
    // this field.
    bool ocean_available{false};
    float timer{0.0f};                // +9F8h for the bow, +9FCh for the stern
    bool pose_valid{false};           // kUnitOffPoseValid
    OceanVec3 probe_near{};           // unit+1054h (bow) or class+638h (stern)
    OceanVec3 probe_far{};            // unit+1060h (bow) or class+644h (stern)
    float steering{0.0f};             // kUnitOffSteering, copied by 00834820 only
    float max_speed{0.0f};            // class+500h, the divisor of the published scale
    OceanVec3 hull_extent{};          // class+A0h, +A4h, +A8h: Length, Width, Height
};

struct UnitSpraySubUpdateState {
    UnitTimedEffect* slots[kUnitSprayEffectSlots]{};  // +A00h..+A0Ch
    float timer{0.0f};                                // +A10h
    float throttle{0.0f};                             // kUnitOffThrottle
    bool pose_valid{false};                           // kUnitOffPoseValid
    const OceanVec3* spray_points{nullptr};           // class+650h
    int spray_point_count{0};                         // class+654h
};

// ---------------------------------------------------------------------------
// 00834820, the bow-wave sub-update. __thiscall(this, float delta), RET 4,
// body 00834820..00834A6D, 169 instructions, no flow gaps.
//
// Gate: the handle at +9E8h, the game singleton at 00E188A8 and its ocean at
// +19F0h must all be non-null. The routine then solves the water line between
// the two unit-local probe points, runs the wave timer, and on a live timer
// republishes the effect. Coverage: complete.
// ---------------------------------------------------------------------------
void unit_update_bow_wave_00834820(UnitWaveSubUpdateState& state,
                                   UnitTimedSubUpdateHost& host,
                                   float delta) noexcept;

// ---------------------------------------------------------------------------
// 00834CC0, the stern-wave sub-update. __thiscall(this, float delta), RET 4,
// body 00834CC0..00834E8F, 149 instructions, no flow gaps.
//
// The same routine as 00834820 with three differences: the handle is +9ECh and
// the timer +9FCh, the probe points come straight out of the class block at
// +638h and +644h instead of the unit's copies, and it publishes neither the
// steering value, nor the matrix row, nor the hull extent. Coverage: complete.
// ---------------------------------------------------------------------------
void unit_update_stern_wave_00834cc0(UnitWaveSubUpdateState& state,
                                     UnitTimedSubUpdateHost& host,
                                     float delta) noexcept;

// ---------------------------------------------------------------------------
// 00834A70, the spray sub-update. __thiscall(this, float delta), RET 4, body
// 00834A70..00834CB8, 148 instructions. The 11 bytes at
// 00834AC5..00834ACF are alignment fill no instruction reaches, not a gap.
//
// Walks the class's spray points, transforms each by the unit's pose, samples
// the water under it, and publishes it to the matching effect slot while the
// hold timer is alive and the point is near enough to the surface. Coverage:
// complete.
// ---------------------------------------------------------------------------
void unit_update_spray_00834a70(UnitSpraySubUpdateState& state,
                                UnitTimedSubUpdateHost& host,
                                float delta) noexcept;

} // namespace bsp
