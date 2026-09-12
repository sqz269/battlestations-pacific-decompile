#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_instance.hpp" // kUnitOffBowAnchorSink, kUnitOffSternAnchorSink
#include "bsp/world_ocean.hpp"   // OceanVec3

// The bow and stern water anchors at unit+9F0h and unit+9F4h, and 004842C0,
// the routine that puts a world point into a point effect.
// Addresses: 004842C0, 00815370, 00823F09, 00825870, 00867D00, 00B6DAE0.
// Evidence and uncertainty are recorded in docs/UNIT_WATER_ANCHORS.md.
//
// The object at each of the two slots is a point-effect instance
// (docs/POINT_EFFECT_INSTANCE.md, PointEffectInstanceStorage, 0x114 bytes)
// created by 00822C20 from the ship class's `BowParticle` template. Step 5 of
// 008255B0 hands each one a world point every frame through 004842C0, and
// 00815AA0 hands both the same scalar through 00815370.
//
// Nothing here is a binary-compatible replacement, and the descriptive names
// are hypotheses rather than recovered symbols.

namespace bsp {

// ---------------------------------------------------------------------------
// Class-block fields the two anchors are built from.
// ---------------------------------------------------------------------------

// `BowParticle` in docs/SHIP_CLASS_FIELDS.md. Both anchors are created from
// this single template; there is no separate stern template.
inline constexpr std::size_t kShipClassOffAnchorTemplate = 0x5AC; // 00823F12
// Two 4x4 matrices, 0x40 bytes each. Their translation rows are at +5E0h and
// +620h. 0082FE30 writes the bow matrix as an identity at 00830195 and its
// translation at 008302B9; the stern matrix is the identity at 008301FF.
inline constexpr std::size_t kShipClassOffBowAnchorMatrix = 0x5B0;   // 008258C9
inline constexpr std::size_t kShipClassOffSternAnchorMatrix = 0x5F0; // 0082591A
// The translation row of the bow matrix, which is also the creation gate.
inline constexpr std::size_t kShipClassOffBowAnchorPosition = 0x5E0; // 00823F27

// ---------------------------------------------------------------------------
// Point-effect instance fields 004842C0 and 00815370 touch. The names of the
// fields themselves are in PointEffectInstanceStorage; these constants record
// the offsets the two routines address them by.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kPointEffectOffScalarTargets = 0x0C;      // 00815375
inline constexpr std::size_t kPointEffectOffScalarTargetCount = 0x10;  // 00815378
inline constexpr std::size_t kPointEffectOffAuxTargets = 0x18;         // 008153AA
inline constexpr std::size_t kPointEffectOffAuxTargetCount = 0x1C;     // 008153AD
inline constexpr std::size_t kPointEffectOffParent = 0x8C;             // 004842D9
inline constexpr std::size_t kPointEffectOffRelativeMatrix = 0xD0;     // 00867D63
// +100h..+108h is the translation row of that matrix, which is the only part
// of it 004842C0 writes.
inline constexpr std::size_t kPointEffectOffRelativePosition = 0x100;  // 00484300
inline constexpr std::size_t kPointEffectOffNode = 0x110;              // 004842C6

// 00CFBC84, 0xC0A00000. Step 5 lifts the two anchors onto the water only when
// the bow point is above this height.
inline constexpr float kUnitAnchorWaterGate = -5.0f;
// 00D7A280, the double 0.5. The lift added to the sampled water height.
inline constexpr double kUnitAnchorWaterLift = 0.5;

// ---------------------------------------------------------------------------
// 004842C0, __thiscall(point effect, const float3* world_point), RET 4, body
// 004842C0..00484341, 42 instructions, no flow gaps.
//
// Two things happen, in this order:
//
//  1. 004842D7 calls the node at +110h through its vtable +30h with the
//     incoming point. For the node class 00B6F5A0 builds (vtable 00D62C88)
//     that slot is 00B6DAE0, which writes the three floats into the node's
//     world matrix translation at node+120h..+128h and tail-jumps to the
//     node's virtual +34h with the world matrix as the argument.
//  2. 004842D9..0048433F caches the same point in the effect's relative
//     matrix. With a parent at +8Ch the point is put through the parent's
//     inverse world matrix (00B6E0D0 then 004142E0) first; with no parent the
//     three floats are stored unchanged.
//
// Nothing reads the cached translation by that offset: 00867D00
// (BSP_Effect_AdvanceTransform) reads the whole matrix at +D0h, multiplies it
// by the parent's world matrix at parent+F0h and pushes the product into the
// same node. So step 1 places the effect for this frame and step 2 is what
// keeps it on the hull as the unit moves.
// ---------------------------------------------------------------------------

// The host stands for the two native calls 004842C0 makes into objects this
// module does not model.
struct UnitWaterAnchorHost {
    virtual ~UnitWaterAnchorHost() = default;

    // The node vtable slot +30h at 004842D7. For the effect's own node that is
    // 00B6DAE0, BSP_Transform_SetWorldPosition.
    virtual void node_set_world_position(const OceanVec3& world_point) = 0;

    // 00B6E0D0 then 004142E0 at 004842EC and 004842F9: the point through the
    // parent's inverse world matrix. Called only when the effect has a parent.
    virtual OceanVec3 parent_relative_point(const OceanVec3& world_point) = 0;
};

// The result is what lands in the relative matrix's translation row.
OceanVec3 unit_publish_anchor_point_004842c0(bool has_parent,
                                             const OceanVec3& world_point,
                                             UnitWaterAnchorHost& host) noexcept;

// ---------------------------------------------------------------------------
// 00825870..008259CE, step 5 of 008255B0 (docs/UNIT_INSTANCE_UPDATE.md owns
// the step; the water rule is repeated here because it is the anchors'
// contract). Each anchor's world point is the translation row of the class
// matrix times the unit's node world matrix. When the bow point is above
// kUnitAnchorWaterGate both points are dropped onto the water surface sampled
// under them and lifted by kUnitAnchorWaterLift; otherwise both keep the
// height the matrix product gave them.
//
// The gate is read from the BOW point only, and it decides for both.
// ---------------------------------------------------------------------------
struct UnitAnchorWaterPoints {
    OceanVec3 bow{};
    OceanVec3 stern{};
};
UnitAnchorWaterPoints unit_anchor_water_points_00825946(const OceanVec3& bow_from_matrix,
                                                        const OceanVec3& stern_from_matrix,
                                                        float bow_water_height,
                                                        float stern_water_height) noexcept;

// ---------------------------------------------------------------------------
// 00823F09..00823F5E, the creation gate inside 00822C20. Both anchors are
// created, or neither: the class must carry the template at +5ACh and a bow
// matrix whose translation is not all zero. The three component tests are
// FCOMI/LAHF/TEST AH,0x44 pairs, so any non-zero component passes and a NaN
// component passes as well.
// ---------------------------------------------------------------------------
bool unit_water_anchors_created_00823f09(bool template_present,
                                         const OceanVec3& bow_anchor_position) noexcept;

// ---------------------------------------------------------------------------
// 00815370, BSP_EffectGroup_SetScalar, __thiscall(effect, float value), RET 4,
// body 00815370..008153DD. 00815AA0 calls it on both anchors.
//
// Two arrays of pointers with their counts, at +0Ch/+10h and +18h/+1Ch. Every
// element gets element->vtable[14h](value). The first loop skips null
// elements (00815388); the second does not test for null at all (008153C0),
// which is the asymmetry docs/UNIT_WATER_ANCHORS.md records.
// ---------------------------------------------------------------------------
struct UnitEffectScalarTarget;

struct UnitEffectScalarHost {
    virtual ~UnitEffectScalarHost() = default;
    // The element vtable slot +14h at 008153A1 and 008153CF.
    virtual void target_set_scalar(UnitEffectScalarTarget* target, float value) = 0;
};

struct UnitEffectScalarArrays {
    UnitEffectScalarTarget* const* primary{nullptr}; // +0Ch
    std::int32_t primary_count{0};                   // +10h
    UnitEffectScalarTarget* const* auxiliary{nullptr}; // +18h
    std::int32_t auxiliary_count{0};                   // +1Ch
};

void unit_effect_set_scalar_00815370(const UnitEffectScalarArrays& arrays,
                                     float value,
                                     UnitEffectScalarHost& host) noexcept;

} // namespace bsp
