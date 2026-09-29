#pragma once
// The hull body's collision box from a ship class's model. Packet cc9_mmod_hull_convex_box,
// docs/GUNNERY_OPEN_ITEMS.md sections 47.3 (step 2), 47.4 and 49.5 to 49.7.
//
// Chain in the image (names are hypotheses):
//   * The model instance is class+50h's vtable[8h]() (007137F0 -> B891A0). B891A0 builds one
//     node per `Hierarchy` item and publishes the first constructed node, hierarchy record 0,
//     at instance+0Ch (B894FB, B89600..B89606; docs/NATIVE_MODEL_GRAPH_AQ.md).
//   * Each record's items are published with that record's node. 0071B710 (ECX instance,
//     stack (item, node), RET 8) appends {item, node} to instance+4Ch through 0071AFC0
//     (0071B77C) when the item IsKindOf the ConvexObject token (006F9A80, slot +0Ch 006F9D40).
//   * 00937C90's walk 00938F61..0093918C keeps the pairs whose node is `firstnode`, the node
//     at instance+0Ch, `front` or `back`. The three names go through 0071AD50, an exact,
//     case-sensitive match on a Note item's name that returns the node the Note was published
//     with (GUNNERY_OPEN_ITEMS 53). So the kept set is the ConvexObjects of hierarchy record 0
//     and of every record listing a Note named firstnode, front or back (section 55: 39 of the
//     40 ship models whose record 0 lists none get their shapes this way; saratoga has none).
//   * The ConvexObject parse 006FAD70 centres the points on their box (006F9EE0, +14h),
//     builds the Dyn hull 00C5DEB0 from the re-centred points, and the shape adds the centre
//     back as its translation with an identity rotation. 00C57C40 widens the hull's box
//     (00C389C0's min/max at data+18h/+24h) by 0.02, and 00C55FC0 unions the shapes.
//
// Host representation: the box is taken as the raw vertex min/max widened by
// kDynConvexMeshBoundsEpsilon, which equals the chain above up to float rounding, because every
// extreme vertex is on the hull. The hull's 0.001 dedup and its 4096-vertex limit are not
// reproduced here. Not the native layout or ABI.
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/world_ocean.hpp"

namespace bsp {

// The body-space box the kept convex shapes give (00C55FC0's union of 00C57C40's boxes),
// already widened by 0.02, so it goes straight into ShipHullBodyInputs::aabb_min/aabb_max.
// shape_count 0 means no owner record lists a ConvexObject; the caller then leaves the inputs
// at their zero default (section 55: saratoga alone among this installation's 88 ship models).
struct MmodHullConvexBox {
    OceanVec3 min{};
    OceanVec3 max{};
    std::uint32_t shape_count{0};   // kept {ConvexObject, node} pairs, as 0071B710 appends them
    std::uint32_t note_owner_records{0};  // records other than 0 kept through a Note
    std::uint32_t point_count{0};   // their vertices, for the log line
};

// Returns false with `error` set when the model cannot be read; returns true with
// shape_count 0 when it reads but no owner record lists a ConvexObject.
bool read_mmod_hull_convex_box(const std::vector<std::uint8_t>& mmod_bytes,
                               MmodHullConvexBox& out, std::string& error);

}  // namespace bsp
