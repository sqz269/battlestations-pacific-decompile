// Projections of the candidate-order instruction ranges inside 00864FE0 and of
// the three gates its walk applies. Evidence: docs/GUNNERY_CANDIDATE_ORDER.md.

#include "bsp/gunnery_candidate_order.hpp"

#include "bsp/unit_gunnery_pass.hpp"

namespace bsp {

float gunnery_candidate_distance_key_00863a21(float straight_distance,
                                              bool contact_is_class_0f,
                                              bool contact_slot_9d8_null) noexcept
{
    // 00863A32 stores the raw length first, so a contact that fails either test
    // keeps the unbiased distance. 00863A6B adds the double at 00D7A220.
    if (contact_is_class_0f && contact_slot_9d8_null) {
        return straight_distance + kUnitGunneryLoiteringPlanePenalty;
    }
    return straight_distance;
}

bool gunnery_category_range_gate_00863a34(float distance_key,
                                          float category_range) noexcept
{
    // 00863A3D FCOMIP ST0(distance), ST1(range); 00863A41 JC takes the accept
    // path only on CF, that is a strictly smaller distance. Equal or NaN falls
    // through to 00863A46 XOR AL,AL.
    return distance_key < category_range;
}

int gunnery_order_insert_position_00865284(const int* ranks,
                                           const float* distances,
                                           const int* order,
                                           int count,
                                           int new_rank,
                                           float new_distance) noexcept
{
    // 00865291..008653BD. The native scan is unrolled four ways with a scalar
    // tail; the predicate is identical at all five copies.
    int at = 0;
    while (at < count) {
        const int other = order[at];
        if (new_rank > ranks[other]) {
            break;                      // 008652B2 JG
        }
        if (new_rank == ranks[other] && new_distance > distances[other]) {
            break;                      // 008652C5 JA
        }
        ++at;                           // 008653BF ADD ECX,1
    }
    return at;
}

int gunnery_order_insert_00865284(int* order,
                                  int count,
                                  int at,
                                  int new_index) noexcept
{
    // 008653F9 JLE skips the shift when the insert lands at the tail.
    for (int i = count; i > at; --i) {
        order[i] = order[i - 1];        // 00865400..00865413
    }
    order[at] = new_index;              // 00865421
    return count + 1;                   // 00865428
}

int gunnery_order_append_unsorted_008655e1(int* order,
                                           int count,
                                           int new_index) noexcept
{
    order[count] = new_index;           // 008655E1 / 0086574B, both MOV [..],EBX
    return count + 1;                   // 008655E8 / 00865752
}

int gunnery_order_walk_index_008657a3(int count, int step) noexcept
{
    const int cursor = count - 1 - step;
    return cursor < 0 ? -1 : cursor;
}

bool gunnery_min_range_skip_008657d8(bool gun_kind_is_5_or_6,
                                     bool candidate_is_class_0f,
                                     float min_range,
                                     float distance) noexcept
{
    // 008657C7 and 008657D6 both jump past the test to 008657F2, so a gun of any
    // other kind and any non-0Fh candidate is never skipped here.
    if (!gun_kind_is_5_or_6 || !candidate_is_class_0f) {
        return false;
    }
    // 008657EC FCOMIP ST0(min_range), ST1(distance); 008657F0 JA skips.
    return min_range > distance;
}

bool gunnery_weapon_range_gate_00729c81(float distance,
                                        float weapon_max_range) noexcept
{
    // 00729C86 FCOMIP ST0(distance), ST1(max); 00729C8A JA goes to 00729D0A,
    // the XOR AL,AL return. Equal passes.
    return !(distance > weapon_max_range);
}

bool gunnery_torpedo_designated_skip_00865809(int category,
                                              bool candidate_is_arm_a_target,
                                              bool node_flag_7d) noexcept
{
    // 00865812 JNZ, 00865816 JZ and 00865820 JNZ each jump to 00865829, the
    // accept path; only the fallthrough reaches 00865822, the skip.
    return category == kUnitGunneryTorpedoCategory
        && candidate_is_arm_a_target
        && !node_flag_7d;
}

int gunnery_walk_pick_00865773(const GunneryWalkCandidate* candidates,
                               const int* order,
                               int count,
                               int category,
                               bool node_flag_7d) noexcept
{
    for (int step = 0;; ++step) {
        const int cursor = gunnery_order_walk_index_008657a3(count, step);
        if (cursor < 0) {
            return kGunneryWalkNoTarget;        // 008657A3 JL / 00865827 JMP
        }
        const int index = order[cursor];        // 008657B0
        const GunneryWalkCandidate& candidate = candidates[index];
        if (candidate.min_range_skip) {
            continue;                           // 008657F0 JA -> 00865822
        }
        if (!candidate.accepted) {
            continue;                           // 008657FE JZ -> 00865822
        }
        if (gunnery_torpedo_designated_skip_00865809(
                category, candidate.is_arm_a_target, node_flag_7d)) {
            continue;                           // 00865820 fallthrough
        }
        // 00865833 CALL 00727F10 and then 00865871, the next gun. Nothing
        // returns to 008657B0, so this candidate is final for this gun.
        return index;
    }
}

}  // namespace bsp
