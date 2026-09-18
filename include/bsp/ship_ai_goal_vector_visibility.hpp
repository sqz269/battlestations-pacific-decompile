#pragma once

#include <cstddef>

#include "bsp/recon_slot_lists.hpp"

// The goal vector's "target visible" gate, `brain+0B28h`: the two narrowing
// tests 009F1420 runs after it opens the flag at 009F14D2.
//
// docs/SHIP_AI_GOAL_VECTOR_VISIBILITY.md carries the evidence. Every name below
// is a hypothesis, not a recovered symbol.
//
// The surface half of the gate (00922DC0 -> 00922C80) is NOT redeclared here:
// bsp/attack_target_classify.hpp already reconstructs it as
// entity_is_surface_target_00922c80 / entity_surface_target_tail_00922c80 over
// EntityTargetFacts, from the same listing. This header adds only the recon
// half, 008053C0 + 009DFBE0, which had no reconstruction.

namespace bsp {

// ---------------------------------------------------------------------------
// 008053C0 BSP_Recon_EnsureSlot
// ---------------------------------------------------------------------------

// `__fastcall(int side) -> ReconSlot*`. The slot table is the three-entry
// pointer array at 00F874BC, indexed by the side value the caller reads from
// `unit+54h` (008053D9 `MOV EAX,[ESI*4 + 0F874BCh]`). A null entry is filled in
// place: operator new of 12A0h bytes at 008053E4/008053E9, constructed by
// 008050E0(side) at 00805404, stored back at 0080540D. So the call never fails
// for a valid side and never returns a different slot for the same side.
inline constexpr std::uint32_t kReconSlotTableAddress008053c0 = 0x00f874bc;
inline constexpr std::size_t kReconSlotAllocationSize008053c0 = 0x12a0;

// ---------------------------------------------------------------------------
// 009DFBE0, the walk the goal vector calls "does this side know that target"
// ---------------------------------------------------------------------------

// The list it walks is the head of triple 4, `slot+0E0Ch`
// (kReconTripleAllOffset + 4): the union of own, enemy, neutral and unknown
// that 00807933..00807966 builds and that nothing publishes to Lua.
inline constexpr std::size_t kReconUnionListHeadOffset009dfbe0 =
    kReconTripleAllOffset + 4;  // 0E0Ch, read at 009DFBE0

// One published entry. 009DFBF0 loads the record from `node+8h` and 009DFBF3
// compares `record+4h` against the target, so the record's +4h is the entity.
struct ReconUnionRecord {
    const void* entity{nullptr};  // record+4h
};

// One link. 009DFBF8 advances through `node+4h`.
struct ReconUnionListNode {
    const ReconUnionListNode* next{nullptr};  // node+4h
    const ReconUnionRecord* record{nullptr};  // node+8h
};

// 009DFBE0, `__fastcall(ReconSlot* slot)(void* target)`, RET 4 (009DFC01 and
// 009DFC06 both `RET 4`, so the one stack argument is callee-popped and the
// slot arrives in ECX). Returns the matching record, or null; the goal vector
// takes only its truth (009F14FF `TEST EAX,EAX` / `SETNZ DL`).
const ReconUnionRecord* recon_union_find_target_009dfbe0(
    const ReconUnionListNode* head, const void* target) noexcept;

// ---------------------------------------------------------------------------
// What being in that list means, from docs/RECON_SLOT_LISTS.md
// ---------------------------------------------------------------------------

// Rule (a): the class scan 00806480 visits twenty-two class ids and only those.
// The test is on the unit's own stamped id at +C4h, not on IsKindOf, because
// the scan walks the world registry's per-class list for each id.
bool recon_scan_visits_class_00806480(int class_id) noexcept;

// Everything the union membership rule asks of one target, for one side.
struct ReconUnionMemberFacts {
    bool present{false};       // the target exists at all
    bool same_side{false};     // relation "own": triple 0, which is never drained
    bool scanned_class{false}; // rule (a)
    ReconUnitGateBytes gate{}; // rule (b), the four bytes 008074D5 reads
    // Rules (d)/(e): the level the sensor pass left on the entry. An enemy or
    // neutral entry below `blip` is dropped by the drain at 00807644 /
    // 008077C2 and so is absent from triple 4 as well.
    ReconDetectionLevel level{ReconDetectionLevel::none};
};

// True when the side's union triple would hold this target.
bool recon_union_contains_009dfbe0(const ReconUnionMemberFacts& facts) noexcept;

}  // namespace bsp
