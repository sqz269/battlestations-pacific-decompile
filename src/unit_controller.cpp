#include "bsp/unit_controller.hpp"

#include <cmath>

namespace bsp {
namespace {

OceanVec3 add(const OceanVec3& a, const OceanVec3& b) noexcept {
    return OceanVec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

} // namespace

// 0092BE80: RET 4. The body is empty; the parameters exist only so the call
// shape matches step 6 of 008255B0.
void unit_controller_update_0092be80(UnitControllerState& state, float delta) noexcept {
    (void)state;
    (void)delta;
}

// 00932A37..00932A7E. The IsKindOf(8) result short-circuits the mass test.
// The native comparison is FCOMIP with 100.0 in ST0 and the mass in ST1
// followed by JA, i.e. row 1 exactly when 100.0 > mass.
int unit_controller_force_row_009329c0(bool is_kind_8, float hull_mass) noexcept {
    if (is_kind_8) {
        return 2;
    }
    return (kUnitControllerLightHullMass > hull_mass) ? 1 : 0;
}

// 00932B3C..00932B5B.
float unit_controller_submersion_curve_009329c0(int row, float stored_curve) noexcept {
    return (row == 1) ? kUnitControllerForcedCurveRow1 : stored_curve;
}

// 00932B69..00932B9F. The native test is "1.0 <= sum ? sum : 1.0", so a NaN
// sum would take the 1.0 branch; the branch order is preserved here.
float unit_controller_effective_mass_009329c0(float hull_mass, float extra_mass) noexcept {
    const float sum = hull_mass + extra_mass;
    if (kUnitControllerMinimumMass <= sum) {
        return sum;
    }
    return kUnitControllerMinimumMass;
}

// 00932D90..00932E42. The clamp order is native: cap at the span, floor at
// zero, then subtract from the span.
UnitHullSubmersion unit_hull_submersion_009329c0(const UnitHullBuoyancyElement& element,
                                                 float above_water) noexcept {
    UnitHullSubmersion out{};
    out.span = element.lower_height - element.reference_height;
    out.range = element.upper_height - element.reference_height;

    float above = above_water;
    if (out.span < above) {
        above = out.span;
    }
    if (above < 0.0f) {
        above = 0.0f;
    }
    out.submerged = out.span - above;
    return out;
}

// 00932CC4..00932CF9. The y lane is written as an explicit zero from XORPS.
OceanVec3 unit_hull_waterline_point_009329c0(const UnitHullBuoyancyElement& element) noexcept {
    return OceanVec3{element.point.x, 0.0f, element.point.z};
}

// The curve chain. The final else is the listing's behaviour, not a design:
// with no matching constant the previous element's ratio survives into this
// element. On the first element that value is whatever the stack held.
float unit_controller_submersion_ratio_009329c0(const UnitHullSubmersion& submersion,
                                                float curve,
                                                float previous_ratio) noexcept {
    float ratio = submersion.submerged / submersion.span;
    if (curve != kUnitControllerCurveLinear) {
        if (curve == kUnitControllerCurveSquare) {
            ratio = ratio * ratio;
        } else if (curve == kUnitControllerCurveSqrt) {
            ratio = std::sqrt(ratio);
        } else {
            ratio = previous_ratio;
        }
    }
    return ratio;
}

// 00933063 region: (hull_mass / count) * ratio * 10.0, the count converted as
// unsigned through the 2^32 fixup at 00CE3978.
float unit_controller_element_force_scale_009329c0(float hull_mass,
                                                   std::uint32_t element_count,
                                                   float ratio) noexcept {
    const float count = static_cast<float>(element_count);
    return (hull_mass / count) * ratio * kUnitControllerForceGain;
}

// -1.0 / count.
float unit_controller_reverse_guard_009329c0(std::uint32_t element_count) noexcept {
    return kUnitControllerReverseGuard / static_cast<float>(element_count);
}

// "if (F / v < guard) F = guard * v". The native code performs the division
// unguarded, so a zero axis velocity relies on the comparison against an
// infinity or a NaN failing; that behaviour is preserved.
float unit_controller_clamp_axis_force_009329c0(float axis_force,
                                                float axis_velocity,
                                                float guard) noexcept {
    if (axis_force / axis_velocity < guard) {
        return guard * axis_velocity;
    }
    return axis_force;
}

// v + omega x r.
OceanVec3 unit_controller_point_velocity(const OceanVec3& linear_velocity,
                                         const OceanVec3& angular_velocity,
                                         const OceanVec3& offset) noexcept {
    return add(linear_velocity, cross_004f9b30(angular_velocity, offset));
}

// 0093791B..009379B0, written out longhand in the native body. Expanding the
// three components shows it is exactly v + omega x r on the offset from the
// body position to the contact point.
OceanVec3 unit_contact_point_velocity_009377e0(const OceanVec3& linear_velocity,
                                              const OceanVec3& angular_velocity,
                                              const OceanVec3& contact_point,
                                              const OceanVec3& body_position) noexcept {
    const OceanVec3 r{contact_point.x - body_position.x,
                      contact_point.y - body_position.y,
                      contact_point.z - body_position.z};
    return OceanVec3{
        linear_velocity.x + (r.z * angular_velocity.y - r.y * angular_velocity.z),
        linear_velocity.y + (r.x * angular_velocity.z - angular_velocity.x * r.z),
        linear_velocity.z + (angular_velocity.x * r.y - r.x * angular_velocity.y),
    };
}

UnitControllerStepResult run_unit_controller_apply_forces_009329c0(UnitControllerHost& host,
                                                                   UnitControllerState& state,
                                                                   float dt) {
    UnitControllerStepResult result{};

    // 009329C9. The disabled path parks the body and still falls through to
    // the accumulator reset at 00933B40.
    if (state.disabled) {
        result.disabled_path = true;
        host.body_set_linear_velocity(OceanVec3{});
        host.body_set_angular_velocity(OceanVec3{});
        host.body_set_frozen_flag();
        state.force_sum = OceanVec3{};
        state.torque_sum = OceanVec3{};
        return result;
    }

    // 00932A2C. The extra mass is read once, at entry, and used twice.
    const float extra_mass = host.unit_extra_mass();
    const float hull_mass = host.hull_mass();

    const int row = unit_controller_force_row_009329c0(
        host.unit_is_kind_of(kUnitControllerKindQuery), hull_mass);
    const UnitControllerDragRow settings = host.drag_row(row);
    const float curve = unit_controller_submersion_curve_009329c0(row, settings.submersion_curve);
    const float effective_mass = unit_controller_effective_mass_009329c0(hull_mass, extra_mass);

    result.drag_row = row;
    result.effective_mass = effective_mass;

    const OceanVec3 linear_velocity = host.body_linear_velocity();
    const OceanVec3 angular_velocity = host.body_angular_velocity();
    const OceanVec3 body_position = host.body_position();

    // 00932C70..009339E5. The element count is re-read from the descriptor's
    // begin/end pair on every iteration.
    float previous_ratio = 0.0f;
    for (std::uint32_t index = 0; index < host.buoyancy_element_count(); ++index) {
        const UnitHullBuoyancyElement element = host.buoyancy_element(index);

        const OceanVec3 world = host.transform_point_to_world(element.point);
        const OceanVec3 world_flat =
            host.transform_point_to_world(unit_hull_waterline_point_009329c0(element));
        const OceanVec3 offset{world_flat.x - body_position.x,
                               world_flat.y - body_position.y,
                               world_flat.z - body_position.z};

        const float above = world.y - host.ocean_height(world.x, world.z);
        const UnitHullSubmersion submersion = unit_hull_submersion_009329c0(element, above);
        const OceanVec3 point_velocity =
            unit_controller_point_velocity(linear_velocity, angular_velocity, offset);

        previous_ratio =
            unit_controller_submersion_ratio_009329c0(submersion, curve, previous_ratio);
        (void)unit_controller_element_force_scale_009329c0(
            hull_mass, host.buoyancy_element_count(), previous_ratio);
        (void)point_velocity;

        // The drag core that turns the point velocity and the settings row
        // into this element's contribution is provisional in
        // docs/UNIT_CONTROLLER_UPDATE.md and is deliberately not written here.
        (void)settings;
        (void)dt;
        ++result.elements;
    }
    result.last_submersion_ratio = previous_ratio;

    // 00933A27..00933ACE, in listing order.
    state.force_staging.z = state.torque_staging.z;
    state.force_staging.y -= extra_mass * kUnitControllerForceGain;

    const OceanVec3 external = host.sum_external_forces();
    state.torque_staging = add(state.torque_staging, external);

    state.force_sum = add(state.force_sum, state.force_staging);
    state.torque_sum = add(state.torque_sum, state.torque_staging);

    result.applied_force = state.force_sum;
    result.applied_torque = state.torque_sum;

    // 00933B01 and 00933B38.
    host.body_add_force(state.force_sum);
    host.body_add_torque(state.torque_sum);

    // 00933B40. Only the two accumulators are cleared; the staging vectors at
    // +68h and +74h keep their values into the next step.
    state.force_sum = OceanVec3{};
    state.torque_sum = OceanVec3{};
    return result;
}

} // namespace bsp
