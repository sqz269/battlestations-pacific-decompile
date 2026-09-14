// The gunnery candidate order: how 00864FE0 orders the per-category candidate
// list and which end of it a gun consumes first.
//
// Evidence: docs/GUNNERY_CANDIDATE_ORDER.md. Every rule here is a projection of
// one instruction range inside BSP_UnitGunneryAi_Tick (body 00864FE0-008658B8)
// or of one of its small callees. Names are hypotheses, not recovered symbols.
//
// Reused rather than redeclared: unit_gunnery_pass.hpp already declares
// kUnitGunneryCandidateCapacity (50h), kUnitGunneryLoiteringPlanePenalty
// (100.0f), kUnitGunneryTorpedoCategory (7), GunneryCandidate and the pair
// insert_ranked_candidate_00865284 / candidate_walk_order_008657a3. This header
// does not redefine those; it publishes the gates and the first-accepted-wins
// walk rule that module does not carry, and the 8-byte native record shape that
// its 12-byte GunneryCandidate abstracts over.
#ifndef BSP_GUNNERY_CANDIDATE_ORDER_HPP
#define BSP_GUNNERY_CANDIDATE_ORDER_HPP

#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGunneryOrderInsertAddress = 0x00865284u;
inline constexpr std::uint32_t kGunneryOrderAppendArmBAddress = 0x0086558Du;
inline constexpr std::uint32_t kGunneryOrderAppendArmAAddress = 0x008656EEu;
inline constexpr std::uint32_t kGunneryOrderWalkAddress = 0x008657A3u;
inline constexpr std::uint32_t kGunneryOrderScoreCandidateAddress = 0x00863990u;
inline constexpr std::uint32_t kGunneryOrderAcceptTargetAddress = 0x00729BC0u;
inline constexpr std::uint32_t kGunneryOrderMinRangeAddress = 0x00729B90u;

// The candidate record the native frame actually holds: 8 bytes at ESP+1D0h,
// stride 8 (00865284 indexes it as [ESP + idx*8 + 1D0h] for the pointer and
// + 1D4h for the float). There is no rank field; 008652A5 recomputes the rank
// from a global table on every comparison. See the doc's correction section.
struct GunneryOrderEntry {
    void* entity = nullptr;   // ESP + idx*8 + 1D0h
    float distance = 0.0f;    // ESP + idx*8 + 1D4h, the biased key below
};

// The per-frame frame arrays 00864FE0 carries, both bounded by the 444h frame:
// the order array is 140h bytes at ESP+90h and the record array is 280h bytes at
// ESP+1D0h, so both hold exactly 50h entries. No instruction in the body
// compares the count against that bound.
inline constexpr int kGunneryOrderCapacity = 0x50;

// ---------------------------------------------------------------------------
// The sort key
// ---------------------------------------------------------------------------

// 00863A21..00863A71. The float stored in a recon candidate is the straight-line
// distance from the owner to the contact, plus a flat 100.0 when the contact
// answers vfn 5Ch(0Fh) and 007B8AD0 finds its +9D8h slot null. The value is a
// de-prioritising bias, not a measured distance.
float gunnery_candidate_distance_key_00863a21(float straight_distance,
                                              bool contact_is_class_0f,
                                              bool contact_slot_9d8_null) noexcept;

// 00863A34..00863A41. FCOMIP with JC accepts only a strictly smaller distance,
// so a contact exactly at the per-category range is rejected. The range is read
// from owner + category*4 + 430h.
bool gunnery_category_range_gate_00863a34(float distance_key,
                                          float category_range) noexcept;

// ---------------------------------------------------------------------------
// The order array
// ---------------------------------------------------------------------------

// 00865284..008653C6. Returns the index in `order` the new candidate takes. The
// scan runs front to back and stops at the first slot the new candidate beats:
// a strictly greater rank (008652B2 JG), or an equal rank with a strictly
// greater distance (008652C5 JA). `ranks[i]` and `distances[i]` are indexed by
// record index, `order` by position. The array that results is descending in
// (rank, distance); the walk below reverses it.
int gunnery_order_insert_position_00865284(const int* ranks,
                                           const float* distances,
                                           const int* order,
                                           int count,
                                           int new_rank,
                                           float new_distance) noexcept;

// 008653D5..0086542B. Shifts `order[at..count-1]` up one and writes the new
// record index at `at`. Returns the new count. The record itself is always
// appended at `count`; only the order array is sorted.
int gunnery_order_insert_00865284(int* order,
                                  int count,
                                  int at,
                                  int new_index) noexcept;

// 008655E1 and 0086574B, the two step-8.7 arms. Both write order[count] = count
// and stop; no rank is read and no shift is performed, so the entry lands at the
// far end of the order array.
int gunnery_order_append_unsorted_008655e1(int* order,
                                           int count,
                                           int new_index) noexcept;

// 00865789 sets the cursor to count-1 and 00865822 decrements it, so step index
// 0 of the walk is order[count-1]. Returns -1 once the walk is exhausted
// (008657A3 TEST/JL also takes this path immediately when count is 0).
int gunnery_order_walk_index_008657a3(int count, int step) noexcept;

// ---------------------------------------------------------------------------
// The per-gun walk, 00865773..00865871
// ---------------------------------------------------------------------------

// 008657C0..008657F0. The minimum-range skip only runs when 005459E0 finds the
// gun's kind ([gun+3F4h]+80h) in {5, 6} and the candidate answers vfn 5Ch(0Fh);
// the range is the float at +58h of the record 00729B90 picks. Skip when the
// range is strictly greater than the candidate distance (008657F0 JA).
bool gunnery_min_range_skip_008657d8(bool gun_kind_is_5_or_6,
                                     bool candidate_is_class_0f,
                                     float min_range,
                                     float distance) noexcept;

// 00729C81..00729C8A, inside the accept gate. Reject when the straight-line
// distance is strictly greater than +60h of the same selected record. This is
// the only maximum-range test a step-8.7 candidate ever faces.
bool gunnery_weapon_range_gate_00729c81(float distance,
                                        float weapon_max_range) noexcept;

// 00865809..00865820. Category 7 alone refuses the candidate that is the arm-A
// target unless the owning node's +7Dh byte is set.
bool gunnery_torpedo_designated_skip_00865809(int category,
                                              bool candidate_is_arm_a_target,
                                              bool node_flag_7d) noexcept;

// One candidate as the walk sees it: the gate outcomes already evaluated.
struct GunneryWalkCandidate {
    bool min_range_skip = false;      // 008657F0 took the skip branch
    bool accepted = false;            // 00729BC0 returned true
    bool is_arm_a_target = false;     // 00865804 CMP [ESI], [ESP+18h] was equal
};

// The result of the walk for one gun: the record index the gun locks onto, or
// kGunneryWalkNoTarget when every candidate was skipped. A gun that ends with no
// target takes 0086586A, the ClearBotFireTarget call.
inline constexpr int kGunneryWalkNoTarget = -1;

// 008657A3..00865833. Walks the order array back to front and returns the first
// candidate that survives every gate. There is no loop back edge after 00865833,
// so the first survivor is the gun's target and no later candidate is examined.
int gunnery_walk_pick_00865773(const GunneryWalkCandidate* candidates,
                               const int* order,
                               int count,
                               int category,
                               bool node_flag_7d) noexcept;

}  // namespace bsp

#endif  // BSP_GUNNERY_CANDIDATE_ORDER_HPP
