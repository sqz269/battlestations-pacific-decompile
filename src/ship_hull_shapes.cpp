#include "bsp/ship_hull_shapes.hpp"

#include <cmath>

// docs/SHIP_HULL_SHAPES.md. Semantic C++ for MSVC Win32, not a drop-in binary
// replacement; every descriptive name is a hypothesis, not a recovered symbol.

namespace bsp {
namespace {

// The double 0.5 at 00D7A280, the same constant bsp/dyn_collision_pass.hpp records as
// kDynAabbHalf for the broad-phase proxy.
constexpr float kHalf = 0.5f;

float abs_00401170(float v) noexcept { return std::fabs(v); }

}  // namespace

DynShapeDescriptor dyn_shape_descriptor_default_00931a10() noexcept {
    // 00931A18..00931A8C, in the order the routine writes the eighteen dwords:
    //   +00h +04h +08h +0Ch zero, +10h the immediate 4 (00931A85), +14h zero,
    //   +18h/+28h/+38h the float 1.0f at 00D7A24C, every other transform slot zero.
    DynShapeDescriptor desc;
    desc.restitution = 0.0f;
    desc.friction = 0.0f;
    desc.group = 0;
    desc.mask = 0;
    desc.type = DynShapeType::kConvexMesh;
    desc.geometry = nullptr;
    desc.row0[0] = 1.0f; desc.row0[1] = 0.0f; desc.row0[2] = 0.0f;
    desc.row1[0] = 0.0f; desc.row1[1] = 1.0f; desc.row1[2] = 0.0f;
    desc.row2[0] = 0.0f; desc.row2[1] = 0.0f; desc.row2[2] = 1.0f;
    desc.position[0] = 0.0f; desc.position[1] = 0.0f; desc.position[2] = 0.0f;
    return desc;
}

DynAabb dyn_convex_mesh_shape_bounds_00c57c40(const DynShapeDescriptor& shape,
                                              const DynAabb& mesh_local) noexcept {
    // 00C57C63..00C57CBD, one FSUB and one FADD per axis against the double at 00D7A2F8.
    const float eps = kDynConvexMeshBoundsEpsilon;
    const float lo[3] = {mesh_local.min.x - eps, mesh_local.min.y - eps,
                         mesh_local.min.z - eps};
    const float hi[3] = {mesh_local.max.x + eps, mesh_local.max.y + eps,
                         mesh_local.max.z + eps};

    // 00C57CC1..00C57D1F: the three sums, then each times 0.5.
    const float centre[3] = {(lo[0] + hi[0]) * kHalf, (lo[1] + hi[1]) * kHalf,
                             (lo[2] + hi[2]) * kHalf};
    // 00C57D23..00C57D56: hi - lo per axis (FSUBRP), then each times the same 0.5.
    const float half[3] = {(hi[0] - lo[0]) * kHalf, (hi[1] - lo[1]) * kHalf,
                           (hi[2] - lo[2]) * kHalf};

    // 00C57D5A..00C57DCE. Row-vector convention: the shape's transform lives at
    // shape+34h..+60h and the three outputs read column 0, 1 and 2 of the basis rows in
    // turn, adding the translation last.
    const float* const r0 = shape.row0;
    const float* const r1 = shape.row1;
    const float* const r2 = shape.row2;
    const float* const t = shape.position;
    const float world_centre[3] = {
        centre[0] * r0[0] + centre[1] * r1[0] + centre[2] * r2[0] + t[0],
        centre[0] * r0[1] + centre[1] * r1[1] + centre[2] * r2[1] + t[1],
        centre[0] * r0[2] + centre[1] * r1[2] + centre[2] * r2[2] + t[2],
    };

    // 00C57DD2..00C57EBA. Nine 00401170 calls supply the magnitudes; the half extent goes
    // through the same column pattern with every basis element replaced by its magnitude.
    const float world_half[3] = {
        abs_00401170(r0[0]) * half[0] + abs_00401170(r1[0]) * half[1] +
            abs_00401170(r2[0]) * half[2],
        abs_00401170(r0[1]) * half[0] + abs_00401170(r1[1]) * half[1] +
            abs_00401170(r2[1]) * half[2],
        abs_00401170(r0[2]) * half[0] + abs_00401170(r1[2]) * half[1] +
            abs_00401170(r2[2]) * half[2],
    };

    // 00C57EBE..00C57F37, shape+0Ch then shape+18h.
    DynAabb out;
    out.min.x = world_centre[0] - world_half[0];
    out.min.y = world_centre[1] - world_half[1];
    out.min.z = world_centre[2] - world_half[2];
    out.max.x = world_centre[0] + world_half[0];
    out.max.y = world_centre[1] + world_half[1];
    out.max.z = world_centre[2] + world_half[2];
    return out;
}

DynAabb dyn_body_local_bounds_00c55fc0(const DynAabb* shape_bounds,
                                       std::size_t count) noexcept {
    // 00C55FC0..00C55FEC: +FLT_MAX at 00D7A248 into B+38h..+40h and -FLT_MAX at 00D7A244
    // into B+44h..+4Ch. Every native caller has already linked a shape, so a zero count
    // only happens here; a native body that never gets a shape keeps 00C43CA0's zeros
    // instead, because 00C55FC0 is then never reached.
    constexpr float kPosMax = 3.402823466e+38f;   // 7F7FFFFF
    constexpr float kNegMax = -3.402823466e+38f;  // FF7FFFFF
    DynAabb out;
    out.min.x = kPosMax; out.min.y = kPosMax; out.min.z = kPosMax;
    out.max.x = kNegMax; out.max.y = kNegMax; out.max.z = kNegMax;
    if (shape_bounds == nullptr) {
        return out;
    }

    // 00C56000..00C5617C, the list walk from B+70h through shape+208h. Each shape offers
    // both of its corners to both ends: shape+0Ch first (00C56000..00C560B7), then
    // shape+18h (00C560BA..00C56171).
    for (std::size_t i = 0; i < count; ++i) {
        const float corners[2][3] = {
            {shape_bounds[i].min.x, shape_bounds[i].min.y, shape_bounds[i].min.z},
            {shape_bounds[i].max.x, shape_bounds[i].max.y, shape_bounds[i].max.z},
        };
        float* const lo[3] = {&out.min.x, &out.min.y, &out.min.z};
        float* const hi[3] = {&out.max.x, &out.max.y, &out.max.z};
        for (int c = 0; c < 2; ++c) {
            for (int a = 0; a < 3; ++a) {
                if (*lo[a] > corners[c][a]) {
                    *lo[a] = corners[c][a];
                }
                if (corners[c][a] > *hi[a]) {
                    *hi[a] = corners[c][a];
                }
            }
        }
    }
    return out;
}

std::uint32_t ship_hull_shape_category_bit(int unit_category, bool class_flag_808) noexcept {
    // 009394FB..0093957C. `ADD EAX,-7 / CMP EAX,7 / JA 0093957C` leaves EDI at the zero
    // 009394F7 set, so anything outside 7..14 contributes nothing.
    switch (unit_category) {
        case 7:  return 0x40u;    // 0093951C
        case 8:  return 0x2000u;  // 00939570
        case 9:  return 0x20u;    // 00939515
        case 10:
            // 0093952A..0093954B: SBB/AND 800h/ADD 800h, so the byte being set doubles it.
            return class_flag_808 ? 0x1000u : 0x800u;
        case 11: return 0x400u;   // 00939523
        case 12:
            // 0093954D..0093956E, the same idiom on 100h.
            return class_flag_808 ? 0x200u : 0x100u;
        case 13: return 0x10u;    // 0093950E
        case 14: return 0x80u;    // 00939577
        default: return 0u;
    }
}

float ship_hull_shape_friction(ShipPhysicsMaterial material) noexcept {
    // settings+514h + material*38h, read at 00939365. The values are the installed
    // shipglobals.lua `Friction` keys docs/SHIP_HULL_BODY.md tabulated, not code
    // constants: the loader 0083B5E0 writes them and defaults to 1.0f.
    switch (material) {
        case ShipPhysicsMaterial::kShip:      return 0.5f;
        case ShipPhysicsMaterial::kTBoat:     return 0.5f;
        case ShipPhysicsMaterial::kSubmarine: return 1.0f;
    }
    return 1.0f;
}

ShipHullShapeSet ship_hull_shapes_collect_00938f61(const ShipHullShapeInputs& in,
                                                   ShipHullShapeHost& host) {
    ShipHullShapeSet out;

    // 00938DB9, 00938DE2, 00938F4E and 00938F9D resolve the four owners before the walk;
    // the host resolves them and returns only the records that matched, because the walk
    // itself carries no state past the comparison.
    (void)host.model_find_node_by_name("front");
    (void)host.model_find_node_by_name("back");
    (void)host.model_find_node_by_name("firstnode");
    (void)host.model_primary_node();
    const std::vector<ShipHullCollisionRecord> records = host.model_hull_collision_records();

    // 009394F9 and 00939533/00939556 are read once per record in the native loop; the
    // answer cannot change inside it, so the projection reads them once.
    const std::uint32_t category_bit =
        ship_hull_shape_category_bit(host.vehicle_class_category(), host.vehicle_class_flag_808());
    const float friction = ship_hull_shape_friction(in.material);

    out.shapes.reserve(records.size() + 1u);
    out.shape_bounds.reserve(records.size() + 1u);

    // 009392FA..009396B8. The default record supplies the type and the identity rotation;
    // the loop writes the friction (00939370), the geometry (009393D1), the translation
    // (0093946E..0093947A), the mask (009394A9 then 009395E2) and the group (009394DD),
    // then pushes the record's address into the body descriptor's vector (009396A1).
    for (const ShipHullCollisionRecord& record : records) {
        DynShapeDescriptor desc = dyn_shape_descriptor_default_00931a10();
        desc.friction = friction;
        desc.geometry = record.geometry;
        desc.position[0] = record.offset.x;
        desc.position[1] = record.offset.y;
        desc.position[2] = record.offset.z;
        desc.mask = kShipHullShapeMaskBase | category_bit;
        desc.group = kShipHullShapeGroup;
        out.shapes.push_back(desc);
        out.shape_bounds.push_back(dyn_convex_mesh_shape_bounds_00c57c40(desc, record.mesh_local));
    }

    // 009396DD..0093971A. Both class floats must be <= 0 and the model must have the node.
    const bool periscope_allowed = !(host.vehicle_class_field_510() > 0.0f) &&
                                   !(host.vehicle_class_field_514() > 0.0f) &&
                                   in.has_periscope_node;
    if (periscope_allowed) {
        // 0093971A..009399BF. This descriptor is a stack local, not an element of the
        // vector above: its type is written explicitly (4 at 00939724), its rotation comes
        // from the node's own local matrix through 00C336C0 at 0093989A, and its
        // translation is that matrix's origin plus the record's float3
        // (009398E1..00939919) rather than the float3 alone.
        DynShapeDescriptor desc = dyn_shape_descriptor_default_00931a10();
        desc.type = DynShapeType::kConvexMesh;
        const ShipHullCollisionRecord record = host.periscope_collision_record();
        host.periscope_node_local_transform(desc.row0, desc.row1, desc.row2, desc.position);
        desc.friction = 1.0f;  // the float 1.0f at 00D7A24C, stored at 0093987A
        desc.geometry = record.geometry;
        desc.position[0] += record.offset.x;
        desc.position[1] += record.offset.y;
        desc.position[2] += record.offset.z;
        desc.group = kShipHullPeriscopeGroup;  // 009398C4
        desc.mask = kShipHullPeriscopeMask;    // 009398D6
        out.shapes.push_back(desc);
        out.shape_bounds.push_back(dyn_convex_mesh_shape_bounds_00c57c40(desc, record.mesh_local));
    }

    // 00C5D8C0..00C5D8E7 attaches every entry through 00C5C940, and each attach ends in
    // 00C55FC0, so the body's box after the last one is the union over the whole list.
    out.body_bounds = dyn_body_local_bounds_00c55fc0(
        out.shape_bounds.empty() ? nullptr : out.shape_bounds.data(), out.shape_bounds.size());
    return out;
}

}  // namespace bsp
