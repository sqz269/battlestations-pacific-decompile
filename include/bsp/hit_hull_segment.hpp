#pragma once
// The producer side of the hit record's segment and part fields: the geometry
// element a trace lands on, the growth law of the part-hit array, and the two
// routines that copy an element's authored kind and index into a record.
//
// Addresses: 00724510 00723E90 00723D60 00723AA0 00723F80 00723B70 006D2E30.
// Evidence and coverage: docs/HIT_HULL_SEGMENT.md, reports/cc7_hit_hull_segment.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native routines are __thiscall on raw pointers; these are new C++ interfaces
// over the same arithmetic, not drop-in binary replacements. The record layout
// constants and HitPartEntry come from bsp/unit_hit_path.hpp and the record
// itself from bsp/hit_narrowphase.hpp; nothing is redeclared here.
#include <cstddef>

#include "bsp/hit_narrowphase.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The geometry element, 2Ch bytes, the element type of the vector at
// [geom+0Ch, geom+10h). 00723D60 and 00723F80 both divide by this stride.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kGeomElementStride = 0x2C;      // IMUL 2E8BA2E9h / SAR EDX,3
inline constexpr std::size_t kGeomElementOffKind = 0x04;     // 00723F5F, 00724078
inline constexpr std::size_t kGeomElementOffPartIndex = 0x08; // 00723F65, 007240A4
inline constexpr std::size_t kGeomElementOffMesh = 0x0C;      // 00723B70 reads its two vectors
inline constexpr std::size_t kGeomElementOffTriBegin = 0x14;  // 00723AA0, 00723B70
inline constexpr std::size_t kGeomElementOffTriEnd = 0x18;    // the count is (end - begin) >> 1
inline constexpr std::size_t kGeomElementOffRejectScalar = 0x24; // fed to 0085BF90, unread

inline constexpr std::size_t kGeomOffElementsBegin = 0x0C;
inline constexpr std::size_t kGeomOffElementsEnd = 0x10;

// The shape's two fields the unit-part trace uses are NOT redeclared here:
// kUnitPartShapeOffNode (+1Ch) and kUnitPartShapeOffOwner (+24h) already live
// in bsp/collision_shapes.hpp. One correction belongs on the second of them:
// 00724517 MOV ECX,[ECX+24h] and 0070F74E make it the `this` of the geometry
// walk, so it is the geometry, not only an owner back-pointer.
// Only the node's two matrix offsets are new.
inline constexpr std::size_t kCollisionNodeOffWorldMatrix = 0x50;    // pushed at 0072452C
inline constexpr std::size_t kCollisionNodeOffInverseMatrix = 0x90;  // pushed at 00724525

// The kind code that routes a hit to the breakable-part damage of 0092D1F0.
// CMP dword ptr [ESI+30h],0Dh at 00826FAC, and the same test on an entry's +0h
// at 008275C1. It is never an immediate in .text: it is authored geometry data.
inline constexpr int kGeomElementKindBreakablePart = 0x0D;

// The two fields a trace copies out of an element.
struct GeomElementHit {
    int kind{kHitRecordResetIndex};       // element+4h  -> record+30h, entry+0h
    int part_index{kHitRecordResetIndex}; // element+8h  -> record+34h, entry+4h
};

// ---------------------------------------------------------------------------
// 00723E90, the segment producer. The caller has already resolved the element;
// this is what the routine writes once 00723D60 returned one, and what it
// leaves alone when it returned null.
// ---------------------------------------------------------------------------

// True when 00723D60 found an element. On false 00723E90 takes the XOR AL,AL
// path at 00723F1E and touches no field of the record at all, so a record that
// was reset by 00470470 still reads -1 / -1.
bool segment_hit_writes_record_00723e90(const GeomElementHit* element) noexcept;

// The four stores at 00723F44..00723F6C. `element_address` becomes record+38h
// verbatim (MOV [EAX+38h],ESI), which is why +38h is the element pointer and
// not a copied record. A null element leaves `record` untouched.
void apply_segment_hit_00723e90(HitRecordFill& record,
                                const GeomElementHit* element,
                                const void* element_address,
                                const HitQueryPoint& world_hit_point) noexcept;

// ---------------------------------------------------------------------------
// 00723F80 + 006D2E30, the part-hit array producer.
// ---------------------------------------------------------------------------

// The 10h-byte entry 00723F80 assembles on its own frame before the append.
// +8h is written as a literal zero at 00724044 and is never read again on this
// path; unit_hit_path.hpp's HitPartEntry models only +0h, +4h and +0Ch.
HitPartEntry make_part_hit_00723f80(const GeomElementHit& element, float distance) noexcept;

// 006D2E30's growth law: newCapacity = capacity*2 + 2, from
// LEA EAX,[EAX+EAX*1+2] at 006D2E3B. Called only when count == capacity, so
// from zero it yields 2, 6, 14, 30 ... and never returns a value <= count.
int grow_part_hit_capacity_006d2e30(int capacity) noexcept;

// What one append does to the record's three array fields. The caller owns the
// buffer; this is the bookkeeping, with no allocator and no host.
struct PartHitAppendPlan {
    bool reallocates{false};     // count == capacity at 006D2E36
    int new_capacity{0};         // +44h after the call
    std::size_t new_byte_size{0}; // the operator_new argument, newCapacity << 4
    int copy_count{0};           // entries moved from the old buffer, 006D2E56..006D2E8C
    int write_index{0};          // the slot the entry lands in, [ESI+40h] before the bump
    int new_count{0};            // +40h after ADD dword [ESI+40h],1 at 006D2EC9
};

// `count` is record+40h and `capacity` is record+44h on entry. Negative or
// inconsistent inputs are returned unchanged in new_count so a caller can see
// the native's lack of validation rather than a clamp this packet invented.
PartHitAppendPlan plan_part_hit_append_006d2e30(int count, int capacity) noexcept;

// ---------------------------------------------------------------------------
// The gate the index has to pass on the far side, for readers of a produced
// record. The damage arithmetic itself is ship_part_damage_0092d1f0 in
// bsp/ship_hit_record.hpp and is not duplicated.
// ---------------------------------------------------------------------------

// 0092D1F0's two index uses: the signed gate byte at parts+34Ch+index
// (JL at 0092D210) and the bound on the health vector [parts+310h, parts+314h)
// (the inlined check that falls into 00BF6713). A part index that fails the
// first is silently dropped; one that fails the second reaches the CRT throw.
struct PartIndexReach {
    bool gate_byte_allows{false};  // -1 < gateByte
    bool within_health_vector{false}; // index < (end - begin) / 4
};

PartIndexReach part_index_reach_0092d1f0(int part_index, signed char gate_byte,
                                         int health_slot_count) noexcept;

} // namespace bsp
