#pragma once

// The hull's hydrodynamics: 009329C0 BSP_UnitController_ApplyHydroForces, body
// 009329C0..00933BA9, `void __thiscall(controller, float dt)`, RET 4. Slot 0 of the
// controller vtable 00D19630; the three derived callbacks 00936DC0, 00937440 and
// 00937630 chain into it, and 00937440 forwards its own dt at 00937622.
//
// It is the drag that removes a turning ship's lateral velocity and the buoyancy that
// cancels the world's gravity. Per buoyancy element of the vehicle class it samples the
// ocean, forms the relative velocity at the element, splits that into the three body
// axes, applies a linear and a quadratic drag coefficient per axis out of the physics
// material record, clamps each element's impulse so it cannot reverse the velocity, adds
// the buoyancy along world Y, and accumulates force and torque. At the end it ticks the
// leak model, stages the flooding weight and heeling torque, and flushes the totals into
// the body with 00C35360 AddForce and 00C35330 AddTorque.
//
// docs/SHIP_HYDRO_FORCES.md carries the address-by-address derivation. Every name here is
// a hypothesis, not a recovered symbol. The listing, not the pseudocode, is the evidence
// for every product and every sign: the routine is compiled to a mix of SSE scalar moves
// and x87 arithmetic and the decompiler misplaces several of the terms.
//
// Built on, never redefining: bsp/world_ocean.hpp (OceanVec3, cross_004f9b30),
// bsp/ship_hull_body.hpp (ShipPhysicsMaterial and the mass threshold),
// bsp/rigid_body_integration.hpp (DynBody, DynMotionState, the two integration phases
// that consume what this routine adds), bsp/unit_forces.hpp (the leak model at unit+10D4h
// whose total water is the unit+10FCh this routine reads).

#include <cstddef>

#include "bsp/ship_hull_body.hpp"
#include "bsp/world_ocean.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The physics material record, settings+4E0h + material*38h
// ---------------------------------------------------------------------------

// Fourteen floats, 38h bytes. The offsets are bound to their Lua keys by the PRODUCER,
// the three-iteration loop of 0083B5E0 BSP_GameSettings_LoadFromLuaGlobals at
// 0083FEE7..008403B7: each iteration pushes one key string and stores the answer at a
// fixed offset, so the pairing below is read from the stores, not inferred from the
// order of the key block. `default_` is the constant the loader stores when the key is
// missing (an FLD of the named address, or FLD1).
//
//   offset  key string     Lua key                              default
//   +00h    00D0A764       KozegellenallasiEgyutthatoL          0.0025f  (00CF01F4)
//   +04h    00D0A748       KozegellenallasiEgyutthatoN          0.01f    (00D7A238)
//   +08h    00D0A728       KozegellenallasiEgyutthatoLFel       0.0025f
//   +0Ch    00D0A708       KozegellenallasiEgyutthatoNFel       0.01f
//   +10h    00D0A6E4       KozegellenallasiEgyutthatoLOldalra   0.0025f
//   +14h    00D0A6C0       KozegellenallasiEgyutthatoNOldalra   0.01f
//   +18h    00D0A69C       KozegellenallasiEgyutthatoLElore     0.0025f
//   +1Ch    00D0A678       KozegellenallasiEgyutthatoNElore     0.01f
//   +20h..  00D0A668       NyomatekSzorzo[1..3]                 1.0f     (FLD1)
//   +2Ch    00D0A660       Kitevo                               1.5f     (00CE380C)
//   +30h    00D0A654       Gravitacio                           10.0f    (00CE38B8)
//   +34h    00D0A648       Friction                             1.0f     (FLD1)
//
// "Kozegellenallasi Egyutthato" is Hungarian for a medium-resistance (drag) coefficient;
// L is the linear term and N the quadratic one. The axis suffixes are Fel (up), Oldalra
// (sideways), Elore (forward), and the unsuffixed pair is the downward one. Which axis
// each pair drives is settled below by 009329C0's own loads, not by the names.
struct ShipPhysicsMaterialRecord {
    float drag_linear_down{0.0025f};        // +00h, used when the point moves downward
    float drag_quadratic_down{0.01f};       // +04h
    float drag_linear_up{0.0025f};          // +08h, used when the point moves upward
    float drag_quadratic_up{0.01f};         // +0Ch
    float drag_linear_lateral{0.0025f};     // +10h, the body row-0 axis
    float drag_quadratic_lateral{0.01f};    // +14h
    float drag_linear_forward{0.0025f};     // +18h, the body row-2 axis
    // +1Ch. 009329C0 never reads it: the forward term at 00933252 is linear only, and the
    // discarded CALL 00424C40 at 00932B32 is where the eighth read would have been.
    float drag_quadratic_forward{0.01f};
    float torque_multiplier[3]{1.0f, 1.0f, 1.0f};  // +20h..+28h, read by 00939BCB, not here
    float depth_exponent{1.5f};             // +2Ch, read at 00932B3F
    float gravity{10.0f};                   // +30h, read by 00937C90, not by 009329C0
    float friction{1.0f};                   // +34h, copied onto each shape by 00939365
};

// The three rows as shipped in scripts/datatables/shipglobals.lua, ShipGlobals["Physics"]
// ["Ship" | "TBoat" | "Submarine"]. The index is the ShipPhysicsMaterial the hull selects.
ShipPhysicsMaterialRecord ship_physics_material_shipped(ShipPhysicsMaterial material) noexcept;

// 00932A53..00932A8F. The same selection 00937CF1 makes for the hull body: category 8
// answers material 2, otherwise `Mass >= 100.0` (the double at 00D7A220) answers 0 and a
// lighter hull answers 1. The stride to the row is the LEA/SUB/ADD chain at
// 00932A82..00932A8F, which is index * 38h.
inline constexpr std::size_t kShipPhysicsMaterialStride = 0x38;
inline constexpr std::size_t kShipPhysicsMaterialBase = 0x4E0;  // settings+4E0h

// ---------------------------------------------------------------------------
// The buoyancy element list at class+52Ch
// ---------------------------------------------------------------------------

// One element is 24h bytes, nine floats; the list is the pointer pair
// class+52Ch (first) and class+530h (last), and the count is their difference / 24h
// (the reciprocal multiply by 38E38E39h at 00932C4B and five more sites).
//
// PRODUCER NOT READ. No function in the exported set writes class+528h..+534h; the two
// readers are 009329C0 and 00937C90's displacement sum, and the field roles below are a
// hypothesis reconciled between those two readers only. 00937C90 forms
// |(level_draft - level_base) / (level_top - level_base)| raised to Kitevo, multiplies by
// |level_top - level_base| and by `coefficient` and Gravitacio/10, and sums that over the
// list; 009329C0 measures submersion over (level_draft - level_base) and divides by
// (level_top - level_base) for the buoyancy shape factor. The packet that finds the
// writer should reconcile these names against it. See docs/SHIP_HYDRO_FORCES.md.
struct ShipBuoyancyElement {
    float coefficient{0.0f};   // +00h, scales the buoyancy force (00932EE3)
    float level_top{0.0f};     // +04h, only ever used as (level_top - level_base)
    float level_draft{0.0f};   // +08h, only ever used as (level_draft - level_base)
    float level_base{0.0f};    // +0Ch, the common subtrahend
    float unread_10{0.0f};     // +10h, neither reader touches it
    float unread_14{0.0f};     // +14h, likewise
    OceanVec3 position{};      // +18h..+20h, the element point in hull-local space
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// The body's world transform as 00C33650 hands it over: 00C32000 returns body+8h and
// 00C33650 expands that 3x4 into a 4x4 whose rows 0..2 are the body axes and whose row 3
// is the position. Row 0 is the lateral axis, row 1 the up axis and row 2 the forward
// axis, the same convention DynBody records.
struct ShipHydroTransform {
    OceanVec3 row0{1.0f, 0.0f, 0.0f};
    OceanVec3 row1{0.0f, 1.0f, 0.0f};
    OceanVec3 row2{0.0f, 0.0f, 1.0f};
    OceanVec3 position{};
};

// 004142E0 BSP_Vector3f_TransformAffinePoint, called twice per element at 00932CBF and
// 00932D02: the hull-local point through the rows plus the translation.
OceanVec3 ship_hydro_transform_point_004142e0(const ShipHydroTransform& m,
                                              const OceanVec3& local) noexcept;

// 0042B260 BSP_Geometry_NormalizeVectorWithFloor, at 00933063. Squared length against the
// double 1.08e-10 at 00CE3820: above it the divisor is the CRT square root, at or below
// it the divisor is the double 1.0e-5 at 00CE3C70, so a near-zero vector is scaled but
// never divided by zero. The existing byte-exact reproduction of the same routine is
// `normalize_camera_basis_0042b260`; this is the same rule in plain float32 under a name
// that says where this packet uses it.
OceanVec3 ship_hydro_normalize_0042b260(const OceanVec3& v) noexcept;

// 00932B72..00932B9F. The mass every force in the routine is divided by:
// max(class+B0h + unit+10FCh, 1.0f), where unit+10FCh is the leak model's total
// accumulated water (bsp/unit_forces.hpp, kUnitOffLeakWaterMass).
float ship_hydro_total_mass_00932b72(float class_mass, float leak_water_mass) noexcept;

// 00932DD5..00932E12. `height_above_water` is the element's world Y less the sampled
// water height; `span` is level_draft - level_base. The submerged depth is
// span - clamp(height_above_water, 0, span), so a point whose authored level sits at the
// surface reads fully submerged and one a whole span above it reads zero.
float ship_hydro_submerged_depth_00932dd5(float height_above_water, float span) noexcept;

// 00932E44..00932EE9. k is 0.5f (00CE3800) for a surface hull and 0.0f for a submarine
// (the vtable slot 5Ch answer at 00932E2F). The magnitude is
//   coefficient * depth * (1 - k + k * depth / (level_top - level_base))
// and it is added to the WORLD Y component of the element's force at 00933780, not to the
// body's up axis.
inline constexpr float kShipHydroBuoyancyShapeMix = 0.5f;  // 00CE3800
float ship_hydro_buoyancy_00932e44(const ShipBuoyancyElement& element, float depth,
                                   bool is_submarine) noexcept;

// 00932FB5..00933018. The submerged fraction raised to Kitevo, by an equality ladder on
// the exponent rather than a power: 1.0f (00D7A24C) leaves it alone, 2.0f (00CE3958)
// squares it, 0.5f (00CE3800) takes the square root, and ANY OTHER VALUE falls through
// leaving the previous iteration's result in place. `previous` is that carried value; it
// is the slot at ESP+74h, which the first iteration of the loop has never written.
// Material 1 forces the exponent to 2.0f at 00932B53 before the loop, so with the shipped
// rows (Ship 1.0, TBoat 0.5 overridden to 2.0, Submarine 1.0) the fall-through is latent
// and never reached; the loader's own default of 1.5f would reach it.
float ship_hydro_depth_factor_00932fb5(float fraction, float exponent,
                                       float previous) noexcept;

// 00933018..00933044. (class Mass / element count) * depth_factor * 10.0, the double at
// 00CE3DC0. The 10.0 is a literal here, equal in value to the world gravity magnitude
// (bsp/dyn_world_settings.hpp, kDynWorldGravityY); the record's own Gravitacio at +30h is
// NOT what this multiply reads.
inline constexpr float kShipHydroDragGravity = 10.0f;  // the double at 00CE3DC0
float ship_hydro_drag_scale_00933018(float class_mass, int element_count,
                                     float depth_factor) noexcept;

// What one element contributes before the impulse clamp. `relative_velocity` is the point
// velocity v + omega x r (00932D90..00932DC5); `direction` is that vector normalised.
// 00933199..00933413, term by term:
//
//   lateral  = (|v| * LOldalra + |v|^2 * NOldalra) * (row0 . dir) * row0
//   forward  = (|v| * LElore)                      * (row2 . dir) * row2     (linear only)
//   vertical = (|v| * LFel + |v|^2 * NFel) * (row1 . dir) * row1   when dir.y >= 0
//              (|v| * L    + |v|^2 * N   ) * (row1 . dir) * row1   when dir.y <  0
//   force    = -drag_scale * (lateral + forward + vertical)
//
// The branch at 00933161 tests the normalised Y against zero with COMISS 0, dir.y and
// JBE, so dir.y == 0 takes the upward pair.
OceanVec3 ship_hydro_element_drag_00933199(const ShipPhysicsMaterialRecord& record,
                                           const ShipHydroTransform& m,
                                           const OceanVec3& relative_velocity,
                                           float drag_scale) noexcept;

// 009335DD..00933637, the per-element stability clamp. For each body axis the routine
// forms dv = force * dt / total_mass, and if dv's component along the axis is more than
// 1/element_count of the velocity's own component in the opposing direction it replaces
// it with exactly -(1/element_count) * (axis . velocity). The limit is the double -1.0 at
// 00D7A250 divided by the count, and the test is `limit > (axis . dv) / (axis . v)`, so
// the whole list can remove at most the entire component in one substep. The clamped dv
// is divided by the same dt/mass at 0093363D to give the force back.
OceanVec3 ship_hydro_clamp_impulse_009335dd(const ShipHydroTransform& m,
                                            const OceanVec3& force,
                                            const OceanVec3& relative_velocity,
                                            float dt_over_mass, int element_count) noexcept;

// 0093380A..0093394B, the one branch that is neither drag nor buoyancy. When unit+5Dh is
// clear, the element is the last of the list, the material is 1 (TBoat) and the forward
// speed row2 . v is positive, the routine subtracts
//   ((Width * Length * depth / span) * forward_speed) * row1   crossed with
//   (Length * 0.5) * row2
// from the torque accumulator, which pitches the bow up. Length is class+A0h, Width is
// class+A4h and the 0.5 is the double at 00D7A280.
OceanVec3 ship_hydro_planing_torque_0093380a(float class_length, float class_width,
                                             float depth, float span, float forward_speed,
                                             const ShipHydroTransform& m) noexcept;

// ---------------------------------------------------------------------------
// The host: one method per native call site
// ---------------------------------------------------------------------------

struct ShipHydroHost {
    virtual ~ShipHydroHost() = default;

    // 00932BB0, 00C31F40: the body's linear velocity, M+00h.
    virtual OceanVec3 body_linear_velocity_00c31f40() = 0;
    // 00932BE6, 00C31F20: the body's angular velocity, M+0Ch.
    virtual OceanVec3 body_angular_velocity_00c31f20() = 0;
    // 00932C21 / 00932C28, 00C32000 then 00C33650: the body's world transform at body+8h,
    // expanded into the 4x4 the two point transforms and every dot product read.
    virtual ShipHydroTransform body_world_transform_00c33650() = 0;
    // 00932A42 and 00932E2F, the unit's vtable slot 5Ch called with 8. Both sites pass the
    // same argument and both read the answer as a bool; the callee's body was not read, so
    // the name is the record it selects, not a recovered meaning.
    virtual bool unit_category_8_vtable5c() = 0;
    // 00932D6B, 0078CF20 on the world at [00E188A8]+19F0h: the water height under the
    // element's world X and Z.
    virtual float water_height_0078cf20(float x, float z) = 0;
    // 00933A01, 0074F930 on unit+10D4h with the same dt. Ticks the leak model; its total
    // water is the unit+10FCh this routine already read at 00932A2C.
    virtual void leak_tick_0074f930(float dt) = 0;
    // 00933A52, 0074F2E0 on unit+10D4h: the heeling torque the leak weights produce.
    virtual OceanVec3 leak_heel_torque_0074f2e0() = 0;
    // 00933B01, 00C35360 AddForce on the body at controller+2Ch. The only AddForce site in
    // the image.
    virtual void add_force_00c35360(const OceanVec3& force) = 0;
    // 00933B38, 00C35330 AddTorque on the same body.
    virtual void add_torque_00c35330(const OceanVec3& torque) = 0;
    // 009329EC, 00C37E50 set linear velocity. The DISABLED path only.
    virtual void set_linear_velocity_00c37e50(const OceanVec3& v) = 0;
    // 00932A0E, 00C37E20 set angular velocity. The disabled path only.
    virtual void set_angular_velocity_00c37e20(const OceanVec3& w) = 0;
    // 00932A16, `OR dword ptr [body+50h], 4`: the disabled path sets the body's
    // suppress-gravity bit (kDynBodyFlagNoGravity). Not a call, but the third and last
    // thing that path does.
    virtual void set_body_no_gravity_flag_00932a16() = 0;
};

// ---------------------------------------------------------------------------
// The routine
// ---------------------------------------------------------------------------

struct ShipHydroInputs {
    // controller+14h. Non-zero takes the disabled path at 009329CF, which zeroes both
    // velocities, sets the body's no-gravity bit and runs none of the model.
    bool disabled{false};

    ShipPhysicsMaterial material{ShipPhysicsMaterial::kShip};  // 00932A53..00932A7E
    ShipPhysicsMaterialRecord record{};                       // settings+4E0h + i*38h

    float class_mass{0.0f};    // class+B0h, unit+538h is the class descriptor
    float class_length{0.0f};  // class+A0h, the Lua key Length
    float class_width{0.0f};   // class+A4h, the Lua key Width
    // unit+10FCh, the leak model's total accumulated water. Added to the mass at 00932B78
    // and subtracted from the buoyancy as a weight at 00933A3A.
    float leak_water_mass{0.0f};
    // unit+5Dh. Non-zero suppresses the planing torque at 00933639.
    bool suppress_planing{false};

    const ShipBuoyancyElement* elements{nullptr};  // class+52Ch
    int element_count{0};                          // (class+530h - class+52Ch) / 24h
};

// What the routine staged before flushing, so a caller can see the two halves the native
// keeps at controller+378h and controller+384h.
struct ShipHydroResult {
    OceanVec3 force{};   // controller+378h..+380h at the AddForce site
    OceanVec3 torque{};  // controller+384h..+38Ch at the AddTorque site
    // controller+68h..+70h and +74h..+7Ch, the two scratch vectors the tail stages: the
    // flooding weight (0, -leak_water_mass * 10, 0) and 0074F2E0's heeling torque.
    OceanVec3 flooding_weight{};
    OceanVec3 leak_torque{};
    int submerged_elements{0};  // elements whose depth was > 0, i.e. that produced drag
    bool ran_planing_torque{false};
};

// 009329C0 whole. The element loop runs for index 0 .. element_count-1; the native's
// index starts at zero (XOR EBP,EBP at 00932C1F) and only increments, so every
// `TEST EBP,EBP / JL` branch inside the loop is dead code in this build and this
// reconstruction does not carry it. docs/SHIP_HYDRO_FORCES.md lists those ranges.
ShipHydroResult ship_hydro_apply_forces_009329c0(const ShipHydroInputs& in, float dt,
                                                 ShipHydroHost& host) noexcept;

}  // namespace bsp
