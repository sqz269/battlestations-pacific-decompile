#pragma once

// 0070DB60 BSP_UnitGroup_SwapSlotsByDistance (a hypothesis), __fastcall(group),
// RET 0, body 0070DB60..0070E39A, reconstructed from the Ghidra listing (x87
// included) for packet cc9_ship_motion_tail part 8b. docs/SHIP_POST_MOTION.md
// sections 5 and 13.
//
// Coverage: complete. The world-matrix refreshes inlined before each position
// read (0070DC15..0070E13C, through 00414DB0 / 00413920) are the host's pose
// cache and are not modelled.

#include "bsp/ship_ai_path_corridor.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

struct UnitGroupSlotSwapHost {
    virtual ~UnitGroupSlotSwapHost() = default;
    // [entity+538h], the class descriptor, compared for equality at 0070DC06.
    virtual std::uint32_t member_class_0538(std::uint32_t entity) = 0;
    // 0070E066 / 0070E156: 00811180 with ECX = the leader [group+14h] and the
    // member's world position (entity+0FCh). Returns false when the trail has no
    // leg to measure along; the image would then read its uninitialised stack
    // slots, and a binder must decide (this reconstruction skips the pair).
    virtual bool decompose_against_leader_00811180(std::uint32_t entity, float& across,
                                                   float& along) = 0;
};

struct UnitGroupSlotSwapResult {
    bool gated{false};          // 0070DB70 (type 18h) or 0070DB92 (controlled unit)
    int pairs_compared{0};      // pairs that reached the distance test
    int pairs_skipped_invalid{0};
    int swaps{0};
};

// `members` is the record array at group+18h (count group+4F8h); `column` is
// group+500h; `type_04fc` is group+4FCh; `controlled_entity` is [00E188D8]'s
// entity value in the same encoding as ShipAiUnitGroupMember::entity (0 when
// there is none).
UnitGroupSlotSwapResult unit_group_swap_slots_by_distance_0070db60(
    std::vector<ShipAiUnitGroupMember>& members, std::int32_t column,
    std::int32_t type_04fc, std::uint32_t controlled_entity, UnitGroupSlotSwapHost& host);

}  // namespace bsp
