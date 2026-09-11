#include "bsp/unit_controller.hpp"

#include <cmath>

#include "bsp/unit_instance.hpp"

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

// ---------------------------------------------------------------------------
// 0092BE80, 0082583E, 00815370 and 00815AA0. docs/UNIT_CONTROLLER.md.
// ---------------------------------------------------------------------------

float unit_effect_intensity_gate_0082583e(float body_axis_speed) noexcept {
    // 00825850: FLDZ, FCOMIP against the returned speed, JBE keeps the 1.0f the
    // caller already staged. The jump is taken on an unordered compare too, so
    // only an ordered negative speed reaches the XORPS at 00825858.
    if (0.0f > body_axis_speed) {
        return kUnitEffectGateAstern;
    }
    return kUnitEffectGateAhead;
}

std::size_t set_effect_group_scalar_00815370(EffectGroupHost& host, const EffectGroupSpans& spans,
                                             float value) {
    std::size_t calls = 0;
    // 00815384..008153A8: each pointer is tested before the virtual.
    for (std::size_t i = 0; i < spans.primary_count; ++i) {
        if (!host.primary_present(i)) {
            continue;
        }
        host.primary_set_scalar(i, value);
        ++calls;
    }
    // 008153C0..008153D6: no test at all. A null element here is a fault in the
    // shipped image, not a skipped iteration; the reconstruction keeps that.
    for (std::size_t i = 0; i < spans.secondary_count; ++i) {
        host.secondary_set_scalar(i, value);
        ++calls;
    }
    return calls;
}

UnitEffectIntensityResult publish_unit_effect_intensity_00815aa0(UnitEffectIntensityState& state,
                                                                 UnitEffectIntensityHost& host,
                                                                 float gate) {
    UnitEffectIntensityResult result{};

    // 00815AA3..00815AC2 is the 006FF270 expression inlined; 00815AE8 multiplies
    // it by the caller's gate.
    const float intensity = unit_intensity_scale_006ff270(host.global_intensity_override(),
                                                          state.intensity_override,
                                                          state.intensity_scale);
    result.value = intensity * gate;

    // 00815ACA and 00815AFC, before the latch: the bow and stern water anchors.
    const int pre_latch_fields[2] = {0x9F0, 0x9F4};
    for (int field : pre_latch_fields) {
        if (host.group_present(field, 0)) {
            host.group_set_scalar(field, 0, result.value);
            ++result.groups_updated;
        }
    }

    // 00815B29..00815B3F: FUCOMIP against +9D0h, TEST AH,44h, JNP to the return
    // at 00815D0D. Equal means the rest of the routine is skipped.
    if (!(state.published_latch != result.value)) {
        return result;
    }
    state.published_latch = result.value; // 00815B5C
    result.latch_changed = true;

    // 00815B45..00815BF5 and 00815C53..00815CB5, the single-pointer fields, with
    // the two array walks between them in native order.
    std::size_t field_index = 0;
    for (; field_index < 6; ++field_index) {
        const int field = kUnitEffectGroupFields[field_index];
        if (host.group_present(field, 0)) {
            host.group_set_scalar(field, 0, result.value);
            ++result.groups_updated;
        }
    }

    // 00815C19..00815C51: the part vector at +A14h, count at +A18h, signed and
    // compared with JL, so a negative count runs zero iterations.
    for (std::size_t i = 0; i < host.part_count(); ++i) {
        if (host.group_present(0xA14, i)) {
            host.group_set_scalar(0xA14, i, result.value);
            ++result.groups_updated;
        }
    }

    for (; field_index < sizeof(kUnitEffectGroupFields) / sizeof(kUnitEffectGroupFields[0]);
         ++field_index) {
        const int field = kUnitEffectGroupFields[field_index];
        if (host.group_present(field, 0)) {
            host.group_set_scalar(field, 0, result.value);
            ++result.groups_updated;
        }
    }

    // 00815CD5..00815D03: the five attachment slots at +B54h, a fixed count.
    for (std::size_t i = 0; i < kUnitAttachSlotCount; ++i) {
        if (host.group_present(0xB54, i)) {
            host.group_set_scalar(0xB54, i, result.value);
            ++result.groups_updated;
        }
    }

    return result;
}

} // namespace bsp
