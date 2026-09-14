#pragma once
#include "bsp/ship_hit_record.hpp"
#include <cstddef>

// The authored mesh-category enum that decides whether a hit reaches the
// part-damage arm. docs/PART_DAMAGE_REACHABILITY.md.
//
// 007149D0 is a linear case-insensitive search over the NULL-terminated pointer
// table at 00E08138 (Ghidra names it PTR_s_lwing_00e08138). It returns the index
// of the first match or -1. Two producers use it:
//
//   * 007273A3, inside the GeomMesh resource payload parser 00727310, on the
//     name read from the resource stream; the result lands in the geometry
//     element's +4h at 007273DB.
//   * 0087CF7B, inside 0087CA80 BSP_DamageableClass_ReadLuaFields, on the Lua
//     key `MshCategory` (00D0E190); the result lands in the 30h-byte part
//     descriptor's +4h at 0087CF8E.
//
// The hit record's +30h is a copy of the geometry element's +4h (00723F62), so
// this enum is what docs/SHIP_HIT_RECORD.md's rule R4 compares against 0Dh.

namespace bsp {

// Table order at 00E08138, index 0 first. Names are the literal bytes at
// 00CE4454, 00CE444C, 00CE4440, 00CE4438, 00CE442C, 00CE4420, 00CE4414,
// 00CE4408, 00CE43FC, 00CE43F4, 00CE43EC, 00CE43E4, 00CE43DC, 00CE43D4 and
// 00CE43CC. The terminating null pointer is at 00E08174.
inline constexpr std::size_t kMeshCategoryCount = 15;
extern const char* const kMeshCategoryNames[kMeshCategoryCount];

// "none", the value the three hard-coded narrowphase shapes of
// docs/HIT_NARROWPHASE.md write into the record's +30h.
inline constexpr int kMeshCategoryNone = 10;
// "fizika" (Hungarian for physics), the hull-segment category rule R4 needs.
inline constexpr int kMeshCategoryFizika = 13;
static_assert(kMeshCategoryFizika == kShipHitSegmentKindBreakable,
    "R4's 0Dh gate is the 'fizika' row of the 00E08138 table");

// 007149D0. Case-insensitive whole-string compare through 00438E10; -1 when the
// name is not in the table. A null name is the caller's 00E19BF4 empty-string
// fallback at 0072739E, which matches nothing.
int mesh_category_from_name_007149d0(const char* name) noexcept;

// The kind stored in the geometry element, i.e. 007149D0 plus the remap at
// 007273B1..007273B6: a "steering" element (7) is filed as "body" (9). The Lua
// descriptor reader 0087CA80 does NOT apply this remap.
int geom_mesh_element_kind_00727310(const char* name) noexcept;

// 00937C90 sizes the unit controller's per-part health vector to exactly 20
// (00937FF5 PUSH 0x14 into the resize 004A8F10 on controller+30Ch) and walks
// "fizika_%02d" for indices 0..19 (00938D88 CMP EAX,0x14 / 00938D8F JL). A
// geometry element's +8h has to land in this range for 0092D1F0 to act on it:
// it indexes the signed group byte at controller+34Ch+index and the float at
// controller+310h[index].
inline constexpr int kShipHullSegmentSlots = 20;

// The range test 0092D1F0's two bounds checks amount to, given a 20-slot
// controller. This is the admission rule for a decoded element+8h, not a
// statement that the slot is populated: a "fizika_NN" node the model does not
// carry leaves controller+34Ch+NN at -1 and 0092D1F0 returns at once (0092D210).
bool hull_segment_index_in_range(int segment_index) noexcept;

}  // namespace bsp
