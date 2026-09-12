// The ship controller's hull body: its creation and its mass, inertia and damping.
// docs/SHIP_HULL_BODY.md.
//
// Routines read for this file:
//   00937C90 tail, 009391C2..009392EB and 009399C0..00939C05 (the game side)
//   00C5D580, 00C5D8B2..00C5D8E9 (Dyn::World::CreateBody's own call into the init)
//   00C43CA0, 00C43CA0..00C43E9C (the descriptor-to-body init, read in full)
//   0083B5E0, 0083FEE7..008403B7 (the physics material records)
//
// Nothing here is a binary-compatible replacement.

#include "bsp/ship_hull_body.hpp"

namespace bsp {
namespace {

// 00C43DA4..00C43DFC: per axis, 1/x when x > 0 and 0 otherwise. The compare is a COMISS
// against a zeroed register with JBE, so exactly zero takes the zero branch.
float reciprocal_or_zero(float x) noexcept { return (x > 0.0f) ? (1.0f / x) : 0.0f; }

}  // namespace

DynBodyDescriptor dyn_body_descriptor_default_009391c2() noexcept {
    DynBodyDescriptor desc{};
    desc.max_linear_speed = kDynBodyDefaultMaxSpeed;   // 009392D2
    desc.max_angular_speed = kDynBodyDefaultMaxSpeed;  // 009392DB
    return desc;  // every other member's in-class default is the value the block writes
}

void dyn_body_init_from_descriptor_00c43ca0(const DynBodyDescriptor& desc, DynBody& body,
                                            DynMotionState& motion) noexcept {
    body.motion = &motion;
    body.flags = desc.flags;  // 00C43CB8

    if ((desc.flags & kDynBodyFlagStatic) == 0) {
        motion.lock_torque_to_row1 = desc.lock_torque_to_row1;  // 00C43CC7
        motion.linear_velocity = desc.linear_velocity;          // 00C43CE6..00C43CF1
        motion.angular_velocity = desc.angular_velocity;        // 00C43CFA..00C43D06
        motion.max_angular_speed = desc.max_angular_speed;      // 00C43D13
        motion.max_linear_speed = desc.max_linear_speed;        // 00C43D1C
        motion.force = OceanVec3{};                             // 00C43D22..00C43D2C
        motion.torque = OceanVec3{};                            // 00C43D37..00C43D41
        motion.linear_bias = OceanVec3{};                       // 00C43D4C..00C43D56
        motion.angular_bias = OceanVec3{};                      // 00C43D61..00C43D6B
        motion.linear_damping = desc.linear_damping;            // 00C43D79
        motion.angular_damping = desc.angular_damping;          // 00C43D85
        // 00C43D8E..00C43DA1. FLD1 / FDIVRP, no guard: a zero mass gives an infinity.
        motion.inverse_mass = 1.0f / desc.mass;
        motion.inverse_inertia_body.x = reciprocal_or_zero(desc.inertia.x);  // 00C43E01
        motion.inverse_inertia_body.y = reciprocal_or_zero(desc.inertia.y);  // 00C43E07
        motion.inverse_inertia_body.z = reciprocal_or_zero(desc.inertia.z);  // 00C43E0A
        for (int i = 0; i < 9; ++i) {
            motion.inverse_inertia_world[i] = 0.0f;  // 00C43E10..00C43E46
        }
        // 00C43CDE copies the descriptor's transform into M+84h, the previous transform
        // 00C43EA0 lerps against. The reconstruction's DynMotionState does not carry it.
    }

    // 00C43E4A..00C43E55, run for a static body too.
    for (int i = 0; i < 3; ++i) {
        body.row0[i] = desc.row0[i];
        body.row1[i] = desc.row1[i];
        body.row2[i] = desc.row2[i];
        body.position[i] = desc.position[i];
    }
    body.sleep_countdown = 20;  // 00C43E57, the immediate 14h; equal to world+44h
}

ShipPhysicsMaterial ship_hull_material_00937cf1(bool unit_category_8, float mass) noexcept {
    if (unit_category_8) {
        return ShipPhysicsMaterial::kSubmarine;  // 00937D09
    }
    // 00937D30..00937D38: FCOMI of the threshold against the mass, JA keeps TBoat.
    return (mass >= kShipHullMaterialMassThreshold) ? ShipPhysicsMaterial::kShip
                                                    : ShipPhysicsMaterial::kTBoat;
}

OceanVec3 ship_hull_torque_multiplier(ShipPhysicsMaterial material) noexcept {
    // The installed scripts/datatables/shipglobals.lua, ShipGlobals["Physics"].
    if (material == ShipPhysicsMaterial::kSubmarine) {
        return OceanVec3{1.0f, 1.0f, 1.0f};
    }
    return OceanVec3{1.0f, 1.0f, 2.0f};
}

OceanVec3 ship_hull_inertia_00939a8e(float mass, const OceanVec3& extent,
                                     const OceanVec3& torque_multiplier) noexcept {
    // 00939AF0..00939B20: each span is squared into its own float slot first.
    const float ex2 = extent.x * extent.x;
    const float ey2 = extent.y * extent.y;
    const float ez2 = extent.z * extent.z;

    // 00939B2C..00939B79, in x87 order: the three cross sums are formed before the mass
    // is touched, and each is stored back to a float.
    const float sum_x = ey2 + ez2;
    const float sum_y = ex2 + ez2;
    const float sum_z = ex2 + ey2;

    // 00939B86..00939B95: mass / 12.0 computed in double and stored back as a float.
    const float k = static_cast<float>(static_cast<double>(mass) / kShipHullInertiaDivisor);

    // 00939B99..00939BFD.
    OceanVec3 inertia{};
    inertia.x = torque_multiplier.x * (k * sum_x);
    inertia.y = torque_multiplier.y * (k * sum_y);
    inertia.z = torque_multiplier.z * (k * sum_z);
    return inertia;
}

DynBodyDescriptor ship_hull_body_descriptor_009399c0(const ShipHullBodyInputs& in) noexcept {
    DynBodyDescriptor desc = dyn_body_descriptor_default_009391c2();

    // 009399E5: the unit's world matrix at unit+CCh, copied by 00C336C0, which drops the
    // fourth column of the 4x4 and leaves three rows plus the translation.
    for (int i = 0; i < 3; ++i) {
        desc.row0[i] = in.row0[i];
        desc.row1[i] = in.row1[i];
        desc.row2[i] = in.row2[i];
        desc.position[i] = in.position[i];
    }

    desc.mass = in.mass;  // 009399F7 / 00939A00, class+B0h

    // 009399FD..00939A26: the descriptor's inertia diagonal is zeroed, so the body is
    // created with a zero inverse inertia and 00939C05 supplies the real one afterwards.
    desc.inertia = OceanVec3{};

    desc.angular_damping = kShipHullAngularDamping;  // 00939A2F / 00939A3E
    // The linear damping slot keeps the zero of 00939295, and the flags the zero of
    // 009392CB: the hull body is always dynamic.

    desc.contact_listener = nullptr;  // 00939A4E stores controller+20h
    desc.owner = nullptr;             // 00939A47 stores the unit
    desc.shape_count = in.shape_count;

    // 00939A20..00939A57: the torque lock is set when the mass is below the double 100.0
    // at 00D7A220, the same threshold that picks the TBoat physics material.
    desc.lock_torque_to_row1 = in.mass < kShipHullMaterialMassThreshold;
    return desc;
}

void ship_hull_body_create_00937c90(const ShipHullBodyInputs& in, DynBody& body,
                                    DynMotionState& motion) noexcept {
    const DynBodyDescriptor desc = ship_hull_body_descriptor_009399c0(in);
    dyn_body_init_from_descriptor_00c43ca0(desc, body, motion);

    // 00939A89: the AABB is read back off the body the shapes have just been attached to.
    OceanVec3 extent{};
    extent.x = in.aabb_max.x - in.aabb_min.x;
    extent.y = in.aabb_max.y - in.aabb_min.y;
    extent.z = in.aabb_max.z - in.aabb_min.z;

    const ShipPhysicsMaterial material = ship_hull_material_00937cf1(in.unit_category_8, in.mass);
    const OceanVec3 inertia =
        ship_hull_inertia_00939a8e(in.mass, extent, ship_hull_torque_multiplier(material));

    // 00939C05, the same mutator docs/RIGID_BODY_INTEGRATION.md reconstructed.
    dyn_body_set_inertia_00c37e70(body, inertia);
}

}  // namespace bsp
