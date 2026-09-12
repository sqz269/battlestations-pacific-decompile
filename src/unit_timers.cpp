#include "bsp/unit_timers.hpp"

// The three timed sub-updates 00834E90 ends with. Evidence, the native
// listings these rules were taken from, and the uncertainties are in
// docs/UNIT_TIMERS.md. Every expression below keeps the native rounding
// points: a value the native code spills to a float is rounded to a float
// here, and a value it keeps on the x87 stack or spills to a double is kept
// in a double here.

namespace bsp {
namespace {

// 008348ED..00834937 and 0083493B..00834959, one axis of the interpolation.
// The difference is rounded to a float, the product with the fraction is
// rounded to a float, and the sum with the near probe is rounded again.
float water_line_axis(float near_axis, float far_axis, float fraction) noexcept
{
    const float difference =
        static_cast<float>(static_cast<double>(far_axis) - static_cast<double>(near_axis));
    const float scaled =
        static_cast<float>(static_cast<double>(difference) * static_cast<double>(fraction));
    return static_cast<float>(static_cast<double>(near_axis) + static_cast<double>(scaled));
}

} // namespace

UnitWaterLineSolve unit_water_line_point_008348a9(const OceanVec3& near_probe,
                                                  const OceanVec3& far_probe,
                                                  float surface_height) noexcept
{
    // 008348A9..008348C0: the height difference is computed on the x87 stack
    // and spilled to a double before the divide.
    const double height_difference =
        static_cast<double>(far_probe.y) - static_cast<double>(near_probe.y);
    // 008348D8..008348E9: the surface minus the near height, then the quotient
    // rounded to a float. No zero guard: a flat pair yields a non-finite
    // fraction, which unit_water_line_straddles_0083495d then rejects.
    const double numerator =
        static_cast<double>(surface_height) - static_cast<double>(near_probe.y);

    UnitWaterLineSolve solved{};
    solved.fraction = static_cast<float>(numerator / height_difference);
    solved.point.x = water_line_axis(near_probe.x, far_probe.x, solved.fraction);
    solved.point.y = water_line_axis(near_probe.y, far_probe.y, solved.fraction);
    solved.point.z = water_line_axis(near_probe.z, far_probe.z, solved.fraction);
    return solved;
}

bool unit_water_line_straddles_0083495d(float solved_y,
                                        float near_probe_y,
                                        float far_probe_y) noexcept
{
    // 00834965..0083497D, four strict compares in two pairs. Every unordered
    // compare leaves the chain on the arm that accumulates the timer, which is
    // what the two false results below reproduce.
    if (solved_y > far_probe_y && near_probe_y > solved_y) {
        return true;
    }
    if (far_probe_y > solved_y && solved_y > near_probe_y) {
        return true;
    }
    return false;
}

UnitWaveTimerStep unit_wave_timer_00834996(float timer, float delta, bool straddles) noexcept
{
    UnitWaveTimerStep step{};
    if (straddles) {
        // 00834985: the timer is cleared and the effect is still published.
        step.timer = 0.0f;
        step.expired = false;
        return step;
    }
    // 00834996..008349A8: the sum is rounded to a float and written back
    // before the comparison, so an expired timer keeps its over-threshold value.
    step.timer = static_cast<float>(static_cast<double>(timer) + static_cast<double>(delta));
    // 008349AE..008349BA, strict, against the double at 00CE3D40.
    step.expired = static_cast<double>(step.timer) > static_cast<double>(kUnitWaveTimeout);
    return step;
}

float unit_throttle_magnitude_00834a78(float throttle) noexcept
{
    // 00834A80: strict COMISS against 0.0f. The not-greater arm, which a NaN
    // throttle also takes, subtracts from negative zero rather than masking
    // the sign bit.
    if (throttle > 0.0f) {
        return throttle;
    }
    return -0.0f - throttle;
}

UnitSprayTimerStep unit_spray_timer_00834aee(float timer,
                                             float delta,
                                             float throttle_magnitude) noexcept
{
    UnitSprayTimerStep step{};
    // 00834AF6: COMISS 0.01f against the magnitude, JBE to the hold arm, so an
    // unordered compare holds rather than counts down.
    if (!(kUnitSprayThrottleEpsilon > throttle_magnitude)) {
        step.timer = kUnitSprayHold; // 00834B0D
    } else {
        // 00834AFB: the subtraction happens on the x87 stack and is stored as
        // a float.
        step.timer = static_cast<float>(static_cast<double>(timer) - static_cast<double>(delta));
    }
    // 00834B20: COMISS 0.0f against the timer, JBE to the publish arm.
    step.alive = !(0.0f > step.timer);
    return step;
}

UnitSprayPointClamp unit_spray_point_clamp_00834c04(float point_y, float surface_height) noexcept
{
    UnitSprayPointClamp clamp{};
    clamp.y = point_y;
    // 00834C04..00834C0A: the band is added on the x87 stack and stored as a float.
    const float limit =
        static_cast<float>(static_cast<double>(surface_height) + kUnitSpraySurfaceBand);
    // 00834C16: strict; an unordered compare skips the publish.
    clamp.publish = limit > point_y;
    if (!clamp.publish) {
        return clamp;
    }
    // 00834C1A..00834C20: the pulled-down height, also stored as a float.
    const float lower = static_cast<float>(static_cast<double>(limit) - kUnitSprayDepthOffset);
    // 00834C28: assign only when the lower height is not above the point.
    if (!(lower > point_y)) {
        clamp.y = lower;
    }
    return clamp;
}

float unit_spray_scalar_00834c3a(float throttle_magnitude) noexcept
{
    // 00834C46: FADD ST0,ST0, then a float store at 00834C48. Doubling is exact.
    const float doubled = static_cast<float>(static_cast<double>(throttle_magnitude) +
                                             static_cast<double>(throttle_magnitude));
    // 00834C56: the floor arm is taken only when the doubled value is strictly
    // below the floor, so a NaN keeps the doubled value and propagates.
    if (doubled < kUnitSprayScalarFloor) {
        return kUnitSprayScalarFloor;
    }
    // 00834C6C: strict, so an equal value is kept.
    if (doubled > kUnitSprayScalarCeiling) {
        return kUnitSprayScalarCeiling;
    }
    return doubled;
}

namespace {

// The body 00834820 and 00834CC0 share: two pose-relative probes, one water
// sample under the near probe, the water-line solve, and the timer. The bow
// routine adds the three publishes the stern routine does not make.
bool wave_sub_update_common(UnitWaveSubUpdateState& state,
                            UnitTimedSubUpdateHost& host,
                            float delta,
                            OceanVec3& published_point) noexcept
{
    if (state.effect == nullptr) {
        return false; // 00834838/00834840, 00834CDF/00834CE7
    }
    if (!state.pose_valid) {
        host.refresh_unit_pose(); // 0083486A, 00834CF7
    }
    const OceanVec3 near_world = host.transform_by_unit_pose(state.probe_near); // 00834882, 00834D11
    if (!state.pose_valid) {
        host.refresh_unit_pose(); // 00834892, 00834D21
    }
    const OceanVec3 far_world = host.transform_by_unit_pose(state.probe_far); // 008348A4, 00834D33
    // 008348D3, 00834D62: the sample is taken under the NEAR probe only.
    const float surface = host.water_height(near_world.x, near_world.z);

    const UnitWaterLineSolve solved =
        unit_water_line_point_008348a9(near_world, far_world, surface);
    const bool straddles =
        unit_water_line_straddles_0083495d(solved.point.y, near_world.y, far_world.y);
    const UnitWaveTimerStep step = unit_wave_timer_00834996(state.timer, delta, straddles);
    state.timer = step.timer;
    if (step.expired) {
        host.effect_stop(state.effect); // 00834A61, 00834E82
        return false;
    }
    host.effect_resume(state.effect); // 008349C2, 00834E4D
    published_point = solved.point;
    return true;
}

} // namespace

void unit_update_bow_wave_00834820(UnitWaveSubUpdateState& state,
                                   UnitTimedSubUpdateHost& host,
                                   float delta) noexcept
{
    // 00834846..0083485A, a gate only this one of the three routines applies.
    if (!state.ocean_available) {
        return;
    }
    OceanVec3 point{};
    if (!wave_sub_update_common(state, host, delta, point)) {
        return;
    }
    // 008349CE..008349F3, in the native order: the virtual, the steering, then
    // the reciprocal of MaxSpeed. There is no zero guard on MaxSpeed.
    host.effect_store_float(state.effect, kPointEffectOffValue, host.unit_virtual_38());
    host.effect_store_float(state.effect, kPointEffectOffSteering, state.steering);
    host.effect_store_float(state.effect, kPointEffectOffScale,
                            kUnitSprayScalarCeiling / state.max_speed);
    host.effect_set_point(state.effect, point); // 008349F6

    // 008349FD..00834A1B: the third row of the unit's world matrix.
    const OceanVec3 axis = host.unit_world_matrix_row2();
    host.effect_store_float(state.effect, kPointEffectOffAxisX, axis.x);
    host.effect_store_float(state.effect, kPointEffectOffAxisY, axis.y);
    host.effect_store_float(state.effect, kPointEffectOffAxisZ, axis.z);
    host.effect_store_float(state.effect, kPointEffectOffField2C, 0.0f); // 00834A20

    // 00834A2D..00834A52. The hull extent is republished rotated: the class's
    // Width, Height and Length land at +74h, +78h and +7Ch in that order.
    host.effect_store_float(state.effect, kPointEffectOffExtentA, state.hull_extent.y);
    host.effect_store_float(state.effect, kPointEffectOffExtentB, state.hull_extent.z);
    host.effect_store_float(state.effect, kPointEffectOffExtentC, state.hull_extent.x);
}

void unit_update_stern_wave_00834cc0(UnitWaveSubUpdateState& state,
                                     UnitTimedSubUpdateHost& host,
                                     float delta) noexcept
{
    OceanVec3 point{};
    if (!wave_sub_update_common(state, host, delta, point)) {
        return;
    }
    // 00834E59..00834E6F: the virtual and the reciprocal only.
    host.effect_store_float(state.effect, kPointEffectOffValue, host.unit_virtual_38());
    host.effect_store_float(state.effect, kPointEffectOffScale,
                            kUnitSprayScalarCeiling / state.max_speed);
    host.effect_set_point(state.effect, point); // 00834E72
}

void unit_update_spray_00834a70(UnitSpraySubUpdateState& state,
                                UnitTimedSubUpdateHost& host,
                                float delta) noexcept
{
    // 00834A78..00834A9A: the magnitude is taken once, before the loop.
    const float magnitude = unit_throttle_magnitude_00834a78(state.throttle);

    // The native code dereferences the class array unconditionally; the null
    // check is this reconstruction's guard, not a native branch.
    const int count = (state.spray_points == nullptr) ? 0 : state.spray_point_count;

    for (int index = 0; index < count; ++index) { // 00834AA2, 00834C9F
        // 008247D7: the producer only ever fills four slots. A class that
        // declares more spray points would walk past them in the native code;
        // this reconstruction stops instead and docs/UNIT_TIMERS.md records the
        // mismatch rather than reproducing the overrun.
        if (index >= kUnitSprayEffectSlots) {
            break;
        }
        UnitTimedEffect* const effect = state.slots[index];
        if (effect == nullptr) {
            continue; // 00834AE8
        }
        // 00834AEE..00834B29: the timer is stepped once per non-null slot, so
        // several live slots subtract the frame delta several times.
        const UnitSprayTimerStep step = unit_spray_timer_00834aee(state.timer, delta, magnitude);
        state.timer = step.timer;
        if (!step.alive) {
            host.effect_stop(effect); // 00834B2B
            continue;
        }
        host.effect_resume(effect); // 00834B35
        if (!state.pose_valid) {
            host.refresh_unit_pose(); // 00834B45
        }
        // 00834B4A..00834BE6, the inlined body of 00439820.
        const OceanVec3 world = host.transform_by_unit_pose(state.spray_points[index]);
        const float surface = host.water_height(world.x, world.z); // 00834BFF

        const UnitSprayPointClamp clamp = unit_spray_point_clamp_00834c04(world.y, surface);
        if (!clamp.publish) {
            continue; // 00834C18
        }
        OceanVec3 point = world;
        point.y = clamp.y;

        // 00834C7B..00834C85, in the native order: the scalar, then the fixed
        // 1.0f scale, then the point.
        host.effect_store_float(effect, kPointEffectOffValue, unit_spray_scalar_00834c3a(magnitude));
        host.effect_store_float(effect, kPointEffectOffScale, kUnitSprayScalarCeiling);
        host.effect_set_point(effect, point);
    }
}

} // namespace bsp
