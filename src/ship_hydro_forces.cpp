// 009329C0 BSP_UnitController_ApplyHydroForces, body 009329C0..00933BA9.
// docs/SHIP_HYDRO_FORCES.md. Every expression below is transcribed from the listing
// (local/hydro.asm is the dump this packet worked from); the Ghidra pseudocode moves
// several terms across branches and drops the operand order of the x87 blocks, so it was
// used only to find the shape.

#include "bsp/ship_hydro_forces.hpp"

#include <cmath>

namespace bsp {
namespace {

// The three constants the exponent ladder compares against, as floats.
constexpr float kExponentLinear = 1.0f;   // 00D7A24C
constexpr float kExponentSquare = 2.0f;   // 00CE3958
constexpr float kExponentRoot = 0.5f;     // 00CE3800
constexpr float kMaterialOneExponent = kExponentSquare;  // forced at 00932B53

// 0042B260's two constants.
constexpr float kNormalizeThreshold = 1.08420217e-10f;  // the double at 00CE3820
constexpr float kNormalizeFloor = 1.0e-5f;              // the double at 00CE3C70

constexpr float kPlaningHalf = 0.5f;  // the double at 00D7A280

float dot(const OceanVec3& a, const OceanVec3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

OceanVec3 scaled(const OceanVec3& v, float s) noexcept {
    OceanVec3 r{};
    r.x = v.x * s;
    r.y = v.y * s;
    r.z = v.z * s;
    return r;
}

OceanVec3 added(const OceanVec3& a, const OceanVec3& b) noexcept {
    OceanVec3 r{};
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

OceanVec3 subtracted(const OceanVec3& a, const OceanVec3& b) noexcept {
    OceanVec3 r{};
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// The projection every drag term is built from: (axis . direction) * axis, formed at
// 009330A8, 00933109 and 00933166 for rows 1, 2 and 0 in that order.
OceanVec3 axis_projection(const OceanVec3& axis, const OceanVec3& direction) noexcept {
    return scaled(axis, dot(axis, direction));
}

}  // namespace

ShipPhysicsMaterialRecord ship_physics_material_shipped(
    ShipPhysicsMaterial material) noexcept {
    ShipPhysicsMaterialRecord r{};
    switch (material) {
    case ShipPhysicsMaterial::kTBoat:  // ShipGlobals["Physics"]["TBoat"]
        r.drag_linear_down = 0.5f;
        r.drag_quadratic_down = 0.03f;
        r.drag_linear_up = 0.5f;
        r.drag_quadratic_up = 0.03f;
        r.drag_linear_lateral = 0.5f;
        r.drag_quadratic_lateral = 0.1f;
        r.drag_linear_forward = 0.075f;
        r.drag_quadratic_forward = 0.001f;
        r.torque_multiplier[0] = 1.0f;
        r.torque_multiplier[1] = 1.0f;
        r.torque_multiplier[2] = 2.0f;
        r.depth_exponent = 0.5f;
        r.gravity = 10.0f;
        r.friction = 0.5f;
        break;
    case ShipPhysicsMaterial::kSubmarine:  // ShipGlobals["Physics"]["Submarine"]
        r.drag_linear_down = 0.05f;
        r.drag_quadratic_down = 0.01f;
        r.drag_linear_up = 0.1f;
        r.drag_quadratic_up = 0.03f;
        r.drag_linear_lateral = 0.1f;
        r.drag_quadratic_lateral = 0.03f;
        r.drag_linear_forward = 0.001f;
        r.drag_quadratic_forward = 0.03f;
        r.torque_multiplier[0] = 1.0f;
        r.torque_multiplier[1] = 1.0f;
        r.torque_multiplier[2] = 1.0f;
        r.depth_exponent = 1.0f;
        r.gravity = 10.0f;
        r.friction = 1.0f;
        break;
    case ShipPhysicsMaterial::kShip:
    default:  // ShipGlobals["Physics"]["Ship"]
        r.drag_linear_down = 0.1f;
        r.drag_quadratic_down = 0.03f;
        r.drag_linear_up = 0.1f;
        r.drag_quadratic_up = 0.03f;
        r.drag_linear_lateral = 0.1f;
        r.drag_quadratic_lateral = 0.01f;
        r.drag_linear_forward = 0.001f;
        r.drag_quadratic_forward = 0.03f;
        r.torque_multiplier[0] = 1.0f;
        r.torque_multiplier[1] = 1.0f;
        r.torque_multiplier[2] = 2.0f;
        r.depth_exponent = 1.0f;
        r.gravity = 10.0f;
        r.friction = 0.5f;
        break;
    }
    return r;
}

OceanVec3 ship_hydro_transform_point_004142e0(const ShipHydroTransform& m,
                                              const OceanVec3& local) noexcept {
    OceanVec3 out{};
    out.x = m.row0.x * local.x + m.row1.x * local.y + m.row2.x * local.z + m.position.x;
    out.y = m.row0.y * local.x + m.row1.y * local.y + m.row2.y * local.z + m.position.y;
    out.z = m.row0.z * local.x + m.row1.z * local.y + m.row2.z * local.z + m.position.z;
    return out;
}

OceanVec3 ship_hydro_normalize_0042b260(const OceanVec3& v) noexcept {
    const float length_squared = v.x * v.x + v.y * v.y + v.z * v.z;
    // 0042B2A4: FCOMI against the threshold, JBE takes the floor. An unordered compare
    // (a NaN component) also takes the floor, which is why the test is written this way.
    float divisor = kNormalizeFloor;
    if (length_squared > kNormalizeThreshold) {
        divisor = std::sqrt(length_squared);
    }
    OceanVec3 out{};
    out.x = v.x / divisor;
    out.y = v.y / divisor;
    out.z = v.z / divisor;
    return out;
}

float ship_hydro_total_mass_00932b72(float class_mass, float leak_water_mass) noexcept {
    const float sum = class_mass + leak_water_mass;
    // 00932B87: FLD1 then FCOMIP, JBE keeps the sum when 1.0 <= sum.
    return (sum < 1.0f) ? 1.0f : sum;
}

float ship_hydro_submerged_depth_00932dd5(float height_above_water, float span) noexcept {
    float clamped = height_above_water;
    if (clamped > span) {
        clamped = span;  // 00932DDD, FCOMIP height against span
    }
    if (clamped < 0.0f) {
        clamped = 0.0f;  // 00932DF2, COMISS 0 against the survivor
    }
    return span - clamped;  // 00932E05
}

float ship_hydro_buoyancy_00932e44(const ShipBuoyancyElement& element, float depth,
                                   bool is_submarine) noexcept {
    const float mix = is_submarine ? 0.0f : kShipHydroBuoyancyShapeMix;
    const float shape_span = element.level_top - element.level_base;  // 00932E18
    // 00932E53..00932E6B, in the listing's order: d/shape, times k, plus (1 - k), times d.
    const float shape = mix * (depth / shape_span) + (1.0f - mix);
    return element.coefficient * (depth * shape);
}

float ship_hydro_depth_factor_00932fb5(float fraction, float exponent,
                                       float previous) noexcept {
    if (exponent == kExponentLinear) {
        return fraction;
    }
    if (exponent == kExponentSquare) {
        return fraction * fraction;
    }
    if (exponent == kExponentRoot) {
        return std::sqrt(fraction);
    }
    // 00933005: the last JP leaves the slot untouched, so the previous iteration's value
    // survives. This is the original's behaviour, not an omission here.
    return previous;
}

float ship_hydro_drag_scale_00933018(float class_mass, int element_count,
                                     float depth_factor) noexcept {
    // 00933028 FILD then the 00CE3978 fixup: the count is converted as unsigned.
    float count = static_cast<float>(element_count);
    if (element_count < 0) {
        count += 4294967296.0f;
    }
    return (class_mass / count) * depth_factor * kShipHydroDragGravity;
}

OceanVec3 ship_hydro_element_drag_00933199(const ShipPhysicsMaterialRecord& record,
                                           const ShipHydroTransform& m,
                                           const OceanVec3& relative_velocity,
                                           float drag_scale) noexcept {
    const float speed_squared = relative_velocity.x * relative_velocity.x +
                                relative_velocity.y * relative_velocity.y +
                                relative_velocity.z * relative_velocity.z;
    const float speed = std::sqrt(speed_squared);
    const OceanVec3 direction = ship_hydro_normalize_0042b260(relative_velocity);

    const OceanVec3 projection_up = axis_projection(m.row1, direction);      // 009330A8
    const OceanVec3 projection_forward = axis_projection(m.row2, direction); // 00933109
    const OceanVec3 projection_lateral = axis_projection(m.row0, direction); // 00933166

    // 00933199: the lateral pair, linear on the speed and quadratic on its square.
    const float lateral = speed * record.drag_linear_lateral +
                          speed_squared * record.drag_quadratic_lateral;
    // 0093324E: the forward term is LINEAR ONLY. The record's +1Ch NElore is never read.
    const float forward = speed * record.drag_linear_forward * drag_scale;
    // 009332C6: COMISS 0, direction.y with JBE, so a zero Y takes the upward pair.
    const float vertical = (direction.y >= 0.0f)
                               ? (speed * record.drag_linear_up +
                                  speed_squared * record.drag_quadratic_up)
                               : (speed * record.drag_linear_down +
                                  speed_squared * record.drag_quadratic_down);

    OceanVec3 force{};
    // 009331B5 then 009331EB: the projection is scaled by the coefficient pair first and
    // by drag_scale second, and 00933223 negates the result into the accumulator.
    force = scaled(scaled(scaled(projection_lateral, lateral), drag_scale), -1.0f);
    // 00933295: minus the forward term. `forward` already carries drag_scale because
    // 00933259 folds the scale into the scalar before touching the projection.
    force = subtracted(force, scaled(projection_forward, forward));
    // 009332EA / 00933390 then 00933316 / 009333BC: coefficient pair first, scale second.
    force = subtracted(force, scaled(scaled(projection_up, vertical), drag_scale));
    return force;
}

OceanVec3 ship_hydro_clamp_impulse_009335dd(const ShipHydroTransform& m,
                                            const OceanVec3& force,
                                            const OceanVec3& relative_velocity,
                                            float dt_over_mass, int element_count) noexcept {
    const OceanVec3 delta_v = scaled(force, dt_over_mass);  // 0093342D

    float along_forward = dot(m.row2, delta_v);  // 00933474
    float along_lateral = dot(m.row0, delta_v);  // 009334B8
    float along_up = dot(m.row1, delta_v);       // 0093350B

    const float velocity_forward = dot(m.row2, relative_velocity);  // 00933538
    const float velocity_lateral = dot(m.row0, relative_velocity);  // 00933552
    const float velocity_up = dot(m.row1, relative_velocity);       // 0093357B

    float count = static_cast<float>(element_count);
    if (element_count < 0) {
        count += 4294967296.0f;
    }
    const float limit = -1.0f / count;  // 009335DD, the double -1.0 at 00D7A250

    // 009335E7, 00933603, 0093361D. The order is forward, lateral, up.
    if (limit > along_forward / velocity_forward) {
        along_forward = limit * velocity_forward;
    }
    if (limit > along_lateral / velocity_lateral) {
        along_lateral = limit * velocity_lateral;
    }
    if (limit > along_up / velocity_up) {
        along_up = limit * velocity_up;
    }

    // 0093363D..0093375E: each component back to a force, then summed as
    // forward + lateral first (00933726 adds the up term last).
    const OceanVec3 up_part = scaled(m.row1, along_up / dt_over_mass);
    const OceanVec3 lateral_part = scaled(m.row0, along_lateral / dt_over_mass);
    const OceanVec3 forward_part = scaled(m.row2, along_forward / dt_over_mass);
    return added(added(forward_part, lateral_part), up_part);
}

OceanVec3 ship_hydro_planing_torque_0093380a(float class_length, float class_width,
                                             float depth, float span, float forward_speed,
                                             const ShipHydroTransform& m) noexcept {
    // 0093380C: Width times Length times depth, over the span, times the forward speed.
    const float magnitude = ((class_width * class_length * depth) / span) * forward_speed;
    const OceanVec3 lift = scaled(m.row1, magnitude);                       // 00933843
    const OceanVec3 arm = scaled(m.row2, class_length * kPlaningHalf);      // 00933895
    return cross_004f9b30(lift, arm);                                       // 009338F6
}

ShipHydroResult ship_hydro_apply_forces_009329c0(const ShipHydroInputs& in, float dt,
                                                 ShipHydroHost& host) noexcept {
    ShipHydroResult result{};

    if (in.disabled) {
        // 009329CF..00932A1A. Both velocities to zero, then the body's suppress-gravity
        // bit. The accumulators are zeroed on the way out of both paths (00933B40), which
        // is why this branch leaves `result` at zero rather than staging anything.
        const OceanVec3 zero{};
        host.set_linear_velocity_00c37e50(zero);
        host.set_angular_velocity_00c37e20(zero);
        host.set_body_no_gravity_flag_00932a16();
        return result;
    }

    const bool is_submarine = host.unit_category_8_vtable5c();  // 00932A42

    // 00932B3F, then the material-1 override at 00932B53.
    float exponent = in.record.depth_exponent;
    if (in.material == ShipPhysicsMaterial::kTBoat) {
        exponent = kMaterialOneExponent;
    }

    const float total_mass =
        ship_hydro_total_mass_00932b72(in.class_mass, in.leak_water_mass);  // 00932B72

    const OceanVec3 linear_velocity = host.body_linear_velocity_00c31f40();    // 00932BB0
    const OceanVec3 angular_velocity = host.body_angular_velocity_00c31f20();  // 00932BE6
    const ShipHydroTransform m = host.body_world_transform_00c33650();         // 00932C28

    // The slot at ESP+74h, which the exponent ladder leaves untouched on its fall-through
    // and which the native never initialises before the first iteration.
    float carried_depth_factor = 0.0f;

    OceanVec3 force_total{};   // controller+378h
    OceanVec3 torque_total{};  // controller+384h

    for (int i = 0; i < in.element_count; ++i) {
        const ShipBuoyancyElement& element = in.elements[i];

        // 00932CBF and 00932D02: the element point, and the same point flattened onto the
        // hull's local Y = 0 plane, which is the one the lever arm is taken from.
        const OceanVec3 world_point = ship_hydro_transform_point_004142e0(m, element.position);
        OceanVec3 flattened{};
        flattened.x = element.position.x;
        flattened.z = element.position.z;
        const OceanVec3 world_pivot = ship_hydro_transform_point_004142e0(m, flattened);
        const OceanVec3 arm = subtracted(world_pivot, m.position);  // 00932D07

        const float water = host.water_height_0078cf20(world_point.x, world_point.z);
        const float height_above_water = world_point.y - water;  // 00932D70

        // 00932D8B: the rigid-body point velocity, omega x r added to the linear one.
        const OceanVec3 relative_velocity =
            added(linear_velocity, cross_004f9b30(angular_velocity, arm));

        const float span = element.level_draft - element.level_base;  // 00932DCB
        const float depth = ship_hydro_submerged_depth_00932dd5(height_above_water, span);
        const float buoyancy = ship_hydro_buoyancy_00932e44(element, depth, is_submarine);

        const float speed_squared = relative_velocity.x * relative_velocity.x +
                                    relative_velocity.y * relative_velocity.y +
                                    relative_velocity.z * relative_velocity.z;

        OceanVec3 element_force{};
        // 00932F33 and 00932F6A: both gates, submerged at all and moving at all.
        if (depth > 0.0f && speed_squared != 0.0f) {
            ++result.submerged_elements;
            const float fraction = depth / span;  // 00932FB5
            const float depth_factor =
                ship_hydro_depth_factor_00932fb5(fraction, exponent, carried_depth_factor);
            carried_depth_factor = depth_factor;
            const float drag_scale =
                ship_hydro_drag_scale_00933018(in.class_mass, in.element_count, depth_factor);
            element_force = ship_hydro_element_drag_00933199(in.record, m, relative_velocity,
                                                            drag_scale);
        }

        // 0093341B. The clamp is applied whether or not the drag block ran; with a zero
        // force it returns zero, which is why the native does not branch around it.
        const float dt_over_mass = dt / total_mass;
        element_force = ship_hydro_clamp_impulse_009335dd(m, element_force, relative_velocity,
                                                          dt_over_mass, in.element_count);

        // 00933780: the buoyancy goes onto the WORLD Y component, not the body's up axis.
        element_force.y += buoyancy;

        // 00933799: the forward speed the planing gate tests is taken from the body's
        // linear velocity, not from this element's relative velocity.
        const float forward_speed = dot(m.row2, linear_velocity);

        // 009337BC..00933804: four gates, all of which must hold.
        if (!in.suppress_planing && i == in.element_count - 1 &&
            in.material == ShipPhysicsMaterial::kTBoat && forward_speed > 0.0f) {
            result.ran_planing_torque = true;
            torque_total = subtracted(
                torque_total, ship_hydro_planing_torque_0093380a(in.class_length,
                                                                 in.class_width, depth, span,
                                                                 forward_speed, m));
        }

        force_total = added(force_total, element_force);                       // 0093395B
        torque_total = added(torque_total, cross_004f9b30(arm, element_force));  // 009339A7
    }

    host.leak_tick_0074f930(dt);  // 00933A01

    // 00933A06..00933A4F: the scratch pair at controller+68h and +74h. The flooding weight
    // is the leak water mass against the same 10.0 the drag scale uses.
    result.flooding_weight.y = 0.0f - in.leak_water_mass * kShipHydroDragGravity;
    result.leak_torque = host.leak_heel_torque_0074f2e0();  // 00933A52

    force_total = added(force_total, result.flooding_weight);  // 00933A7A
    torque_total = added(torque_total, result.leak_torque);    // 00933AA7

    result.force = force_total;
    result.torque = torque_total;

    host.add_force_00c35360(result.force);    // 00933B01
    host.add_torque_00c35330(result.torque);  // 00933B38
    // 00933B40..00933B98 zeroes controller+378h..+38Ch on both paths; the accumulators are
    // therefore locals in effect and every call starts them at zero.
    return result;
}

}  // namespace bsp
