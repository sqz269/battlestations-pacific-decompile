#pragma once

#include <cstdint>

#include "bsp/rigid_body_integration.hpp"
#include "bsp/world_ocean.hpp"

// Where the ship controller's hull body comes from, and what mass, inertia and damping it
// is given.
//
// docs/SHIP_HULL_BODY.md carries the addresses, the original ABI and the uncertainty.
// Everything here is a semantic C++ interface for MSVC Win32, not a drop-in binary
// replacement, and every descriptive name is a hypothesis rather than a recovered symbol.
//
// docs/RIGID_BODY_INTEGRATION.md left the writer of controller+2Ch unfound. It is the
// tail of 00937C90, the sub-object builder the controller constructor 00939CB0 calls last
// (00939E2A), after the constructor has nulled controller+2Ch at 00939DBD:
//
//   009399E5  the hull's initial 3x4 transform is copied out of the unit's world matrix
//             at unit+CCh into the descriptor
//   009399F7  the descriptor's mass is the vehicle class descriptor's `Mass`, class+B0h
//   00939A6F  Dyn::World::CreateBody (00C5D580) with the world from game+18h in ECX
//   00939A86  the returned body is stored at controller+2Ch
//   00939A89  the new body's AABB is read back (00C31F90) and squared into box extents
//   00939C05  Dyn_Body_SetInertia with m/12 * (extent sums) scaled per axis
//
// The body is 88h bytes and its motion state C8h bytes; both come from the world's own
// free-list pools inside 00C5D580 (_malloc(0x21340) for 1000 bodies at 00C5D5A0,
// _malloc(200000) for 1000 motion states at 00C5D690).

namespace bsp {

// ---------------------------------------------------------------------------
// The Dyn body descriptor
// ---------------------------------------------------------------------------

// The 84h-byte descriptor Dyn::World::CreateBody takes. The layout is settled by its
// consumer 00C43CA0, which is the producer of every body and motion-state field below;
// the game's own default-construction of the same block is at 009391C2..009392EB.
struct DynBodyDescriptor {
    // desc+00h -> M+B4h at 00C43CC7. Confines the torque response to the body's row-1
    // axis; 00C41550's tail is its only reader.
    bool lock_torque_to_row1{false};
    float mass{1.0f};            // desc+04h -> M+50h as 1/mass at 00C43DA1, no zero guard
    OceanVec3 inertia{1.0f, 1.0f, 1.0f};  // desc+08h..+10h -> M+54h..+5Ch as 1/x, 0 if x<=0
    // desc+14h..+43h, twelve floats: three basis rows then the translation. 00C43E4D
    // copies them into B+08h and 00C43CDE into M+84h, the previous transform.
    float row0[3]{1.0f, 0.0f, 0.0f};
    float row1[3]{0.0f, 1.0f, 0.0f};
    float row2[3]{0.0f, 0.0f, 1.0f};
    float position[3]{0.0f, 0.0f, 0.0f};
    OceanVec3 linear_velocity{};   // desc+44h..+4Ch -> M+00h at 00C43CE6
    OceanVec3 angular_velocity{};  // desc+50h..+58h -> M+0Ch at 00C43CFA
    std::uint32_t flags{0};        // desc+5Ch -> B+50h at 00C43CB8; bit 0 makes it static
    float linear_damping{0.0f};    // desc+60h -> M+B8h at 00C43D79
    float angular_damping{0.0f};   // desc+64h -> M+BCh at 00C43D85
    float max_linear_speed{0.0f};  // desc+68h -> M+18h at 00C43D1C
    float max_angular_speed{0.0f}; // desc+6Ch -> M+1Ch at 00C43D13
    // desc+70h -> B+68h at 00C43E8D and desc+74h -> B+6Ch at 00C43E97. The game passes
    // the controller's embedded contact listener (controller+20h) and the unit.
    const void* contact_listener{nullptr};
    const void* owner{nullptr};
    // desc+78h/+7Ch/+80h are a vector<shape*>: 00C5D8C0..00C5D8E7 attaches each entry
    // through 00C5C940. This packet does not read the shape builder, so the shapes are
    // represented only by their count and the AABB they produce.
    std::uint32_t shape_count{0};
};

// The default 00937C90 builds before it overrides anything, at 009391C2..009392EB:
// mass and the inertia diagonal 1.0f, an identity transform, both velocities zero, no
// flags, no damping, both speed clamps 1000.0f, no listener, no owner, no shapes.
// 00CE3804, loaded once at 009392A7 into both speed slots at 009392D2 and 009392DB.
inline constexpr float kDynBodyDefaultMaxSpeed = 1000.0f;

DynBodyDescriptor dyn_body_descriptor_default_009391c2() noexcept;

// 00C43CA0, __cdecl-shaped but register-passed: 00C5D8B7 puts the body in EAX and
// 00C5D8B5 the descriptor in EDX, and the routine returns void. Writes the body and, when
// descriptor bit 0 is clear, the motion state. Both outputs are overwritten in full.
void dyn_body_init_from_descriptor_00c43ca0(const DynBodyDescriptor& desc, DynBody& body,
                                            DynMotionState& motion) noexcept;

// ---------------------------------------------------------------------------
// The hull's physics material
// ---------------------------------------------------------------------------

// The three records ShipGlobals["Physics"] authors, loaded by 0083B5E0 at
// 0083FEE7..008403B7 into 38h-byte records based at settings+4E0h. The material name is
// selected by index at 0083FEF2: 0 -> "Ship" (00CEB79C), 1 -> "TBoat" (00D0A780),
// 2 -> "Submarine" (00CEB7B0).
enum class ShipPhysicsMaterial : int {
    kShip = 0,
    kTBoat = 1,
    kSubmarine = 2,
};

// 00937CF1..00937D38. The unit's virtual slot 5Ch is called with the category id 8; a
// non-zero answer selects the Submarine record, otherwise the class `Mass` decides
// against the double 100.0 at 00D7A220 (`Mass >= 100` is Ship, below it TBoat).
// Naming slot 5Ch "is a submarine" is a hypothesis from the record it selects; the
// callee's body was not read, so the host below takes the answer as a bool.
inline constexpr float kShipHullMaterialMassThreshold = 100.0f;

ShipPhysicsMaterial ship_hull_material_00937cf1(bool unit_category_8, float mass) noexcept;

// The `NyomatekSzorzo` triple at record+20h (settings+500h + material*38h), read at
// 00939BCB/00939BDB/00939BEC. Key string 00D0A668, read by index 1..3 at 008401E1,
// default 1.0f (FLD1 at 008401F7). The installed shipglobals.lua authors {1,1,2} for
// Ship and TBoat and {1,1,1} for Submarine.
OceanVec3 ship_hull_torque_multiplier(ShipPhysicsMaterial material) noexcept;

// ---------------------------------------------------------------------------
// The inertia rule
// ---------------------------------------------------------------------------

// 00939A8E..00939C05. `extent` is the body's own AABB span, B+44h minus B+38h, read back
// through 00C31F90 after the body exists; the box formula is scaled per axis by the
// material's torque multiplier.
//
//   k    = mass / 12.0            (FDIV against the double 12.0 at 00CE42D0, 00939B8F,
//                                  stored back to a float at 00939B95)
//   I.x  = mul.x * k * (ey^2 + ez^2)
//   I.y  = mul.y * k * (ex^2 + ez^2)
//   I.z  = mul.z * k * (ex^2 + ey^2)
inline constexpr double kShipHullInertiaDivisor = 12.0;

OceanVec3 ship_hull_inertia_00939a8e(float mass, const OceanVec3& extent,
                                     const OceanVec3& torque_multiplier) noexcept;

// ---------------------------------------------------------------------------
// The hull body
// ---------------------------------------------------------------------------

// What the tail of 00937C90 reads to build the hull body. Every field names its producer.
struct ShipHullBodyInputs {
    // class+B0h, the vehicle class descriptor's `Mass` key, read by 00960230 at 0096043A
    // with the default 1.0f. docs/VEHICLE_CLASS_FIELDS.md.
    float mass{1.0f};
    // The unit's world matrix at unit+CCh, copied by 00C336C0 at 009399E5 after
    // 00414DB0 refreshes it when the byte at unit+C8h is clear.
    float row0[3]{1.0f, 0.0f, 0.0f};
    float row1[3]{0.0f, 1.0f, 0.0f};
    float row2[3]{0.0f, 0.0f, 1.0f};
    float position[3]{0.0f, 0.0f, 0.0f};
    // The unit's answer to virtual slot 5Ch with the category id 8 (00937CFD).
    bool unit_category_8{false};
    // The AABB the attached collision shapes give the body, read back at 00939A89. Its
    // producer is the shape attach 00C5C940, which this packet does not read, so a caller
    // has to supply it; a zero span yields a zero inertia, which 00C37E70 turns into a
    // zero inverse inertia, i.e. no angular response to torque at all.
    OceanVec3 aabb_min{};
    OceanVec3 aabb_max{};
    std::uint32_t shape_count{0};
};

// The descriptor the tail of 00937C90 hands to CreateBody: the default above with the
// mass at 009399F7, a zero inertia diagonal (009399FD/00939A07/00939A17/00939A26), the
// unit's transform at 009399E5, the angular damping 1.0f at 00939A2F/00939A3E, the
// listener at 00939A4E, the unit at 00939A47 and the row-1 torque lock at 00939A57.
//
// The linear damping stays at the default zero, and so do the flags: the hull body is
// always dynamic, never static, whatever its mass.
inline constexpr float kShipHullAngularDamping = 1.0f;  // 00D7A24C

DynBodyDescriptor ship_hull_body_descriptor_009399c0(const ShipHullBodyInputs& in) noexcept;

// The whole tail: build the descriptor, create the body from it, then apply the box
// inertia through 00C37E70 exactly as 00939C05 does. `body.motion` is set to `motion`.
void ship_hull_body_create_00937c90(const ShipHullBodyInputs& in, DynBody& body,
                                    DynMotionState& motion) noexcept;

}  // namespace bsp
