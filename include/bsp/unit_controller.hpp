#pragma once
#include <cstdint>

#include "bsp/world_ocean.hpp"

// The unit motion controller at unit+1018h.
// Addresses: 0092BE80, 0080DEC0, 00939CB0, 009329C0, 009377E0.
// Evidence and uncertainty are recorded in docs/UNIT_CONTROLLER_UPDATE.md.
//
// Nothing here is a binary-compatible replacement. The native routines are
// __thiscall members of a class whose 0x390-byte layout is only partly
// recovered; the structs below project the fields the reconstructed rules
// touch. The rigid-body library behind the DYN_physics block is an external
// contract and is not ported: it appears only as UnitControllerHost.

namespace bsp {

// ---------------------------------------------------------------------------
// Layout of the controller object (0x390 bytes, operator new at 0080DED7).
// ---------------------------------------------------------------------------

inline constexpr int kUnitControllerSize = 0x390;               // 0080DED7
inline constexpr int kUnitControllerOffsetDisableGate = 0x14;   // 009329C9
inline constexpr int kUnitControllerOffsetOwnerUnit = 0x1C;     // 00932A27
inline constexpr int kUnitControllerOffsetContactVptr = 0x20;   // 00939CE2
inline constexpr int kUnitControllerOffsetCollisionMask = 0x24; // 00939CD5
inline constexpr int kUnitControllerOffsetBackPointer = 0x28;   // 00939CDB
inline constexpr int kUnitControllerOffsetBody = 0x2C;          // 009329D2
inline constexpr int kUnitControllerOffsetForceStaging = 0x68;  // 00933A7A
inline constexpr int kUnitControllerOffsetTorqueStaging = 0x74; // 00933AA7
inline constexpr int kUnitControllerOffsetOneShot88 = 0x88;     // step 8 of 008255B0
inline constexpr int kUnitControllerOffsetRecordArray = 0x8C;   // ctor, 20 x 0x20
inline constexpr int kUnitControllerOffsetForceSum = 0x378;     // 00933ADF
inline constexpr int kUnitControllerOffsetTorqueSum = 0x384;    // 00933B11

// 00939CD5. The mask the constructor writes into the embedded contact-listener
// subobject at +24h.
inline constexpr std::uint32_t kUnitControllerCollisionMask = 0x7FF9u;

// The mutable state the reconstructed callback reads and writes. Field names
// carry their native offsets; everything not listed in the layout table of
// docs/UNIT_CONTROLLER_UPDATE.md is deliberately absent.
struct UnitControllerState {
    bool disabled{false};             // +14h, the byte tested at 009329C9
    OceanVec3 force_staging{};        // +68h..+73h
    OceanVec3 torque_staging{};       // +74h..+7Fh
    OceanVec3 force_sum{};            // +378h..+383h
    OceanVec3 torque_sum{};           // +384h..+38Fh
};

// ---------------------------------------------------------------------------
// 0092BE80, the sub-update step 6 of 008255B0 calls on the controller.
// void __thiscall(controller, float delta), RET 4, body 0092BE80..0092BE82.
// The whole body is one RET: the call has no effect at all. It is kept here
// because the negative result is the packet's main finding.
// ---------------------------------------------------------------------------
void unit_controller_update_0092be80(UnitControllerState& state, float delta) noexcept;

// ---------------------------------------------------------------------------
// Rules recovered from 009329C0, the force callback.
// ---------------------------------------------------------------------------

// 00932A37..00932A7E. Selects the row of the settings block at +4E0h.
// IsKindOf(8) wins outright; otherwise a hull mass below 100.0 (the double at
// 00D7A220) selects row 1 and anything else row 0.
inline constexpr int kUnitControllerKindQuery = 8;          // 00932A37
inline constexpr float kUnitControllerLightHullMass = 100.0f; // 00D7A220
int unit_controller_force_row_009329c0(bool is_kind_8, float hull_mass) noexcept;

// 00932A82. Row stride and the field offsets read out of 00424C40()'s block.
inline constexpr int kUnitControllerForceRowStride = 0x38;  // 00932A82
inline constexpr int kUnitControllerForceRowBase = 0x4E0;   // 00932A96

// One settings row, in the order the native code reads it. Two linear plus
// quadratic drag pairs selected by the sign of one velocity projection, one
// more pair, one single coefficient, and the submersion curve exponent.
struct UnitControllerDragRow {
    float negative_linear{0.0f};    // +4E0h
    float negative_quadratic{0.0f}; // +4E4h
    float positive_linear{0.0f};    // +4E8h
    float positive_quadratic{0.0f}; // +4ECh
    float axis_linear{0.0f};        // +4F0h
    float axis_quadratic{0.0f};     // +4F4h
    float lateral_linear{0.0f};     // +4F8h
    float submersion_curve{1.0f};   // +50Ch, forced to 2.0f when the row is 1
};

// 00932B3C..00932B5B. Row 1 ignores the stored curve and uses the literal
// 2.0f at 00CE3958.
inline constexpr float kUnitControllerForcedCurveRow1 = 2.0f; // 00CE3958
float unit_controller_submersion_curve_009329c0(int row, float stored_curve) noexcept;

// 00932B69..00932B9F. The mass the impulse is divided by: the descriptor's
// hull mass plus the unit's extra mass at +10FCh, floored at 1.0f (00D7A24C).
inline constexpr float kUnitControllerMinimumMass = 1.0f; // 00D7A24C
float unit_controller_effective_mass_009329c0(float hull_mass, float extra_mass) noexcept;

// The per-element buoyancy record on the descriptor, stride 0x24 at
// descriptor+52Ch..+530h. Only the fields the loop reads are projected.
struct UnitHullBuoyancyElement {
    float force_scale{0.0f};    // +00h
    float upper_height{0.0f};   // +04h
    float lower_height{0.0f};   // +08h
    float reference_height{0.0f}; // +0Ch
    OceanVec3 point{};          // +18h..+20h, hull space
};

// The submersion of one element. `above` is the point's world height minus the
// ocean height under it. The native order is: clamp to the span, floor at zero,
// then subtract from the span, so a point exactly at the surface is dry and a
// point a full span under is fully wet.
struct UnitHullSubmersion {
    float span{0.0f};      // e[+08h] - e[+0Ch]
    float submerged{0.0f}; // in [0, span]
    float range{0.0f};     // e[+04h] - e[+0Ch]
};
UnitHullSubmersion unit_hull_submersion_009329c0(const UnitHullBuoyancyElement& element,
                                                 float above_water) noexcept;

// 00932CB9..00932CF9. The second transform input: the attachment point with
// its hull-space height replaced by zero, i.e. the waterline projection.
OceanVec3 unit_hull_waterline_point_009329c0(const UnitHullBuoyancyElement& element) noexcept;

// The submersion ratio after the curve. 1.0f is linear, 2.0f squares, 0.5f
// takes the square root; any other constant leaves the previous element's
// ratio in place, which is what the listing does rather than what it intends.
inline constexpr float kUnitControllerCurveLinear = 1.0f;  // 00D7A24C
inline constexpr float kUnitControllerCurveSquare = 2.0f;  // 00CE3958
inline constexpr float kUnitControllerCurveSqrt = 0.5f;    // 00CE3800
float unit_controller_submersion_ratio_009329c0(const UnitHullSubmersion& submersion,
                                                float curve,
                                                float previous_ratio) noexcept;

// 10.0, the double at 00CE3DC0. It scales the per-element force and, at the
// flush, the extra mass subtracted from the staged force's y component.
inline constexpr float kUnitControllerForceGain = 10.0f; // 00CE3DC0

// (hull_mass / count) * ratio * 10.0. The count is converted as unsigned; the
// 2^32 fixup at 00CE3978 is the sign correction, so a count that would be
// negative as an int is not reachable through a vector size.
float unit_controller_element_force_scale_009329c0(float hull_mass,
                                                   std::uint32_t element_count,
                                                   float ratio) noexcept;

// -1.0 / count, the per-axis floor of F_axis / v_axis. It stops one element's
// drag impulse from reversing the point velocity it was computed from.
inline constexpr float kUnitControllerReverseGuard = -1.0f; // 00D7A250
float unit_controller_reverse_guard_009329c0(std::uint32_t element_count) noexcept;
float unit_controller_clamp_axis_force_009329c0(float axis_force,
                                                float axis_velocity,
                                                float guard) noexcept;

// v + omega x r, the velocity of a point rigidly attached to the body. Written
// out longhand at 00932D8B (through 004F9B30) and twice in 009377E0.
OceanVec3 unit_controller_point_velocity(const OceanVec3& linear_velocity,
                                         const OceanVec3& angular_velocity,
                                         const OceanVec3& offset) noexcept;

// ---------------------------------------------------------------------------
// 009377E0, the contact callback on the subobject at controller+20h.
// ---------------------------------------------------------------------------

// 00937913. The contact kind that latches "collided this frame" on the unit
// and returns before any damage work.
inline constexpr int kUnitContactKindLatchOnly = 8; // 009378F5
// 009378C9. The flag bit that sends the contact through 009373C0.
inline constexpr std::uint32_t kUnitContactFlagClearShapeBits = 2;
// 0093789A. The kind the other body's owner must answer to be considered.
inline constexpr int kUnitContactOtherKindQuery = 6;
// 00CE3854, compared against 00779AD0() when unit+6B8h is not negative.
inline constexpr float kUnitContactSuppressThreshold = 3.0f;

// The contact point velocity of one body, in the native component order. The
// third component follows the listing, which is not the textbook cross-product
// sign pattern; see docs/UNIT_CONTROLLER_UPDATE.md.
OceanVec3 unit_contact_point_velocity_009377e0(const OceanVec3& linear_velocity,
                                              const OceanVec3& angular_velocity,
                                              const OceanVec3& contact_point,
                                              const OceanVec3& body_position) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary. One method per native call site reached from the
// callback, in call order. There are no default implementations: nothing here
// stands in for unrecovered game behaviour.
// ---------------------------------------------------------------------------
struct UnitControllerHost {
    virtual ~UnitControllerHost() = default;

    // 00C37E50 / 00C37E20 on controller+2Ch. Both clear body flag bits 0x12.
    virtual void body_set_linear_velocity(const OceanVec3& v) = 0;
    virtual void body_set_angular_velocity(const OceanVec3& v) = 0;
    // 00932A16, "or [body+50h], 4" on the disabled path.
    virtual void body_set_frozen_flag() = 0;

    // 00C31F40 / 00C31F20.
    virtual OceanVec3 body_linear_velocity() = 0;
    virtual OceanVec3 body_angular_velocity() = 0;
    // 00C35300, &body+2Ch.
    virtual OceanVec3 body_position() = 0;

    // The unit's vtable +5Ch, called with 8 at 00932A42.
    virtual bool unit_is_kind_of(int kind) = 0;
    // descriptor->f_B0h and unit->f_10FCh.
    virtual float hull_mass() = 0;
    virtual float unit_extra_mass() = 0;
    // 00424C40()'s row at +4E0h + row*38h.
    virtual UnitControllerDragRow drag_row(int row) = 0;

    // descriptor+52Ch..+530h, recomputed on every iteration natively.
    virtual std::uint32_t buoyancy_element_count() = 0;
    virtual UnitHullBuoyancyElement buoyancy_element(std::uint32_t index) = 0;
    // 004142E0 with the 4x4 built by 00C32000 then 00C33650.
    virtual OceanVec3 transform_point_to_world(const OceanVec3& hull_point) = 0;
    // 0078CF20 on (*00E188A8)->+19F0h.
    virtual float ocean_height(float x, float z) = 0;

    // 0074F2E0 with ECX = unit+10D4h: the summed external force list.
    virtual OceanVec3 sum_external_forces() = 0;
    // 00C35360 / 00C35330 on controller+2Ch.
    virtual void body_add_force(const OceanVec3& force) = 0;
    virtual void body_add_torque(const OceanVec3& torque) = 0;
};

// What one call of the callback did, so a caller can observe it without a body.
struct UnitControllerStepResult {
    bool disabled_path{false};   // the +14h gate was set
    int drag_row{0};             // the settings row selected
    float effective_mass{0.0f};  // hull mass + extra, floored at 1.0f
    std::uint32_t elements{0};   // the buoyancy element count seen
    OceanVec3 applied_force{};   // what reached 00C35360
    OceanVec3 applied_torque{};  // what reached 00C35330
    float last_submersion_ratio{0.0f}; // the ratio the final element produced
};

// 009329C0, void __thiscall(controller, float dt), RET 4.
//
// The recovered frame: the disabled early-out, the row and effective-mass
// selection, the per-element submersion and point-velocity kernel, the drain
// of the unit's external force list, the flush to the body, and the reset of
// the two accumulators. The drag core inside the element loop is provisional
// in docs/UNIT_CONTROLLER_UPDATE.md and is NOT reproduced here: this routine
// does not fabricate the force the native loop accumulates, it only carries
// through what the loop is proven to feed forward. The per-element results the
// caller needs are exposed by the pure functions above.
UnitControllerStepResult run_unit_controller_apply_forces_009329c0(UnitControllerHost& host,
                                                                   UnitControllerState& state,
                                                                   float dt);

} // namespace bsp
