#pragma once

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/avoid_zone_owner.hpp"
#include "bsp/gameplay_settings.hpp"
#include "bsp/ship_hull_body.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {
// Bounded projections of the physics tail of 00424D00 and 00423C50.
// These functions do not create groups, partition polygons, build Dyn hulls,
// or create physics bodies. The remaining native calls and ownership sequence
// are documented in docs/AVOID_ZONE_DRAFT_LAYERS.md. Names are hypotheses.

// Only the first integer of each listed class is read by 00424D00. Producers
// are 00841BEE,00841CA6,00841D5D,00841E11,00841F85,0084203D,008420F5,
// 008421AD,00842431, in this order. This is a C++ input, not a native layout.
struct AvoidZoneDraftDepthInputs {
    std::int32_t battle_ship, mother_ship, destroyer, tboat;
    std::int32_t large_landing_ship, cargo_ship, light_cruiser, heavy_cruiser;
    std::int32_t submarine;
};

// Reads settings+80h if session_mode==0, otherwise settings+F0h. The source
// must already contain the actual 0083B5E0-produced record; this is not a
// default initializer or a Lua loader. Lua keys are AvoidZoneDepthsSingle and
// AvoidZoneDepthsMulti directly under ShipGlobals (not nested Single/Multi).
AvoidZoneDraftDepthInputs avoid_zone_draft_depth_inputs(
    const GameplayTuningSettings&, std::int32_t session_mode) noexcept;

// The eight-byte temporary element is produced at 00424E01..00424E19.
// Its key is FLOAT despite its integer tuning source and later integer use.
struct AvoidZoneDraftLayer {
    float key;
    std::uint32_t class_mask;
};
static_assert(sizeof(AvoidZoneDraftLayer) == 8);
static_assert(offsetof(AvoidZoneDraftLayer, class_mask) == 4);

// Valid-container semantic projection of __thiscall vector::push_back, RET4.
// Uses C++ vector allocation/unwinding; no native iterator/allocator ABI claim.
void avoid_zone_draft_pair_append_004223b0(
    std::vector<AvoidZoneDraftLayer>&, const AvoidZoneDraftLayer&);

// Complete ordered key/mask reduction 00424DFB..00425420. Conversion uses
// CVTSI2SS, including current MXCSR rounding, before equality/deduplication.
std::vector<AvoidZoneDraftLayer> avoid_zone_draft_layers_00424dfb(
    const AvoidZoneDraftDepthInputs&);

// 00425459 CVTTSS2SI, including the integer-indefinite INT_MIN result when
// a float key is out of signed range. A C++ float-to-int cast is not equivalent.
std::int32_t avoid_zone_draft_key_00425459(const AvoidZoneDraftLayer&) noexcept;

// 00423C7C..00423C88: reuses 004120D0 but rejects a fallback group whose key
// differs. Returns -1 on that ordinary miss. Empty table is native-invalid.
std::int32_t avoid_zone_draft_exact_group_00423c50(
    const AvoidZoneTable&, std::int32_t requested_key);

using AvoidZoneDraftPoint = std::array<float, 2>;
// 00423CA8..00423CF2: snapshots source corner+0/+4 without changing the zone.
// Native count<3 skips this zone, represented by an empty result.
std::vector<AvoidZoneDraftPoint> avoid_zone_draft_points_00423cb6(
    const AvoidZoneNativeStorage&);

// The exact float passed at 00423D08 to the still-unreconstructed 004F6F20.
inline constexpr std::uint32_t kAvoidZoneDraftPartitionParameterBits = 0x3c8efa35;
float avoid_zone_draft_partition_parameter() noexcept;

struct AvoidZoneDraftHullInput {
    OceanVec3 center;
    std::vector<OceanVec3> local_points;
};

// Partial 00423C50, 00423D45..0042414E only. `piece_indices` must be the
// actual ordered output copied by 004F62C0 from 004F6F20's result. No fan or
// assumed convex polygon is substituted. Fewer than three indices returns
// false and preserves output. Other invalid indices throw, not silently skip.
// Each selected vertex becomes (x,-500,z),(x,+500,z), then all points are
// centered by the binary32 arithmetic mean of selected X/Z coordinates.
bool avoid_zone_draft_extrude_00423d81(
    const std::vector<AvoidZoneDraftPoint>& points,
    const std::vector<std::uint32_t>& piece_indices, AvoidZoneDraftHullInput& output);

// Stack shape descriptor at ESP+F8h, stores 00424204..004243DB. Offset names
// preserve the boundary without guessing the material/filter field meanings.
// +14h points to the manager-owned list node's COPIED eight-byte hull handle,
// not the temporary hull, source zone, or the hull handle's internal data.
struct AvoidZoneDraftShapeInputs {
    float field_00, field_04;
    std::uint32_t class_mask_08, field_0c, kind_10;
    const void* retained_hull_14;
    float transform_18[12];
};
static_assert(offsetof(AvoidZoneDraftShapeInputs, retained_hull_14) == 0x14);
static_assert(offsetof(AvoidZoneDraftShapeInputs, transform_18) == 0x18);
static_assert(sizeof(AvoidZoneDraftShapeInputs) == 0x48);

struct AvoidZoneDraftBodyInputs {
    DynBodyDescriptor body;
    AvoidZoneDraftShapeInputs shape;
};

// Partial 00423C50, descriptor stores only. Call only after 00C32D20>=4 and
// 0041C0F0/00C40F50 have copied the hull into the manager's +6Ch list. The
// borrowed handle must remain alive through the resulting world's use.
// Does not execute 00C5D580 or append its result to manager+5Ch.
AvoidZoneDraftBodyInputs avoid_zone_draft_body_inputs_00424204(
    const AvoidZoneDraftHullInput&, std::uint32_t class_mask, const void* retained_hull);
} // namespace bsp
