// Packet cc_ai_corridor. docs/SHIP_AI_PATH_CORRIDOR.md carries the evidence
// for every line here; each block cites the instruction it projects.

#include "bsp/ship_ai_path_corridor.hpp"

#include <cmath>

namespace bsp {
namespace {

// 0070D450-0070D57F and 0070D724-0070D760 are the same step: the unrolled
// body runs ((count-4)>>2 + 1)*4 slots and the remainder loop the rest, both
// in index order, so one loop is the whole reduction.
//
// `negate` picks 0070D5D0, which spells its negation `-0.0f - offset`
// (0070D5DB loads the -0.0f at 00D7A208 into XMM3, 0070D625 and 0070D727
// subtract the slot from it).
float unit_group_extent(const ShipAiUnitGroupView& group, bool negate) noexcept
{
    // 0070D403 / 0070D5D3: the maximum is seeded at 1.0f, and 0070D423 /
    // 0070D5FB parks that seed in the slot the tail reads back.
    float largest = kShipAiGroupExtentSeed;

    // Deviation, stated rather than hidden: the image indexes the column
    // without a bound and would read past the record for a column index the
    // record cannot hold. A view with no members or an unmodelled column index
    // answers the floor here, which is what an all-zero column answers anyway.
    const bool readable = group.members != nullptr && group.column >= 0 &&
                          static_cast<std::size_t>(group.column) < kShipAiUnitGroupColumnCount;

    if (readable) {
        // 0070D418 / 0070D5F0 MOV ESI,[ECX+4F8h]; both loops are skipped when
        // the count is not positive.
        for (std::int32_t i = 0; i < group.count; ++i) {
            float value = group.members[static_cast<std::size_t>(i)]
                              .lateral[static_cast<std::size_t>(group.column)];
            if (negate) {
                value = -0.0f - value;
            }
            // 0070D455 COMISS XMM1,XMM0 / JBE: a negative slot contributes
            // zero. The test is written against `value < 0` so that an
            // unordered compare takes the same branch the image takes.
            if (!(value < 0.0f)) {
                // 0070D45F COMISS XMM0,XMM2 / JBE.
                if (value > kShipAiGroupExtentSlotCap) {
                    value = kShipAiGroupExtentSlotCap;
                }
            } else {
                value = 0.0f;
            }
            // 0070D475 FCOMIP / JBE: a strictly larger value replaces the
            // running maximum.
            if (largest < value) {
                largest = value;
            }
        }
    }

    // 0070D581 / 0070D762 MOVSS XMM0,[00CE3850] then COMISS XMM0,max / JA:
    // below the floor the floor is the answer.
    if (kShipAiGroupExtentFloor > largest) {
        return kShipAiGroupExtentFloor;
    }
    // 0070D590 / 0070D771 FLD double [00CE3D90], FCOMIP / JBE: above the cap
    // the cap is the answer (0070D5A0 / 0070D781 load the float at 00CFD710).
    if (static_cast<double>(largest) > kShipAiGroupExtentCapCompare) {
        return kShipAiGroupExtentCap;
    }
    return largest;
}

}  // namespace

bool ship_ai_unit_group_leads_00778890(std::uint32_t entity,
                                       std::uint32_t group,
                                       std::uint32_t group_leader) noexcept
{
    // 00778890 MOV EAX,[ECX+284h] / 00778896 TEST EAX,EAX / 0077889A XOR AL,AL,
    // then 0077889F CMP [EAX+14h],ECX / SETZ DL.
    if (group == 0u) {
        return false;
    }
    return group_leader == entity;
}

float ship_ai_unit_group_extent_positive_0070d400(const ShipAiUnitGroupView& group) noexcept
{
    return unit_group_extent(group, false);
}

float ship_ai_unit_group_extent_negative_0070d5d0(const ShipAiUnitGroupView& group) noexcept
{
    return unit_group_extent(group, true);
}

float ship_ai_group_corridor_width_009ed41f(float extent) noexcept
{
    // 009ED41F FADD double [00CE3D88] on the x87 return value, then 009ED425
    // FSTP float: the sum is rounded to float32 before anything reads it. The
    // double sum below rounds identically - a float32 plus 20.0 needs far
    // fewer than 53 bits - so there is no double-rounding step here.
    const float sum = static_cast<float>(static_cast<double>(extent) + kShipAiGroupCorridorMargin);
    // 009ED429 FLD double [00D20198], 009ED433 FCOMIP, 009ED437 JBE: only a
    // width strictly above 600 is replaced, by the float at 00CE4BC4.
    if (static_cast<double>(sum) > kShipAiGroupCorridorCapCompare) {
        return kShipAiGroupCorridorCap;
    }
    return sum;
}

bool ship_ai_group_corridor_extent_change_invalidates(float extent_before,
                                                      float extent_after) noexcept
{
    const float before = ship_ai_group_corridor_width_009ed41f(extent_before);
    const float after = ship_ai_group_corridor_width_009ed41f(extent_after);
    // 009D9DE9-009D9E09: the float32 difference, its absolute value, compared
    // against the double at 00CE3880 with a strict JA.
    return static_cast<double>(std::fabs(before - after)) > kShipAiPathCorridorWidthEpsilon;
}

ShipAiPathCorridorWidths ship_ai_path_corridor_widths_009ed3e0(ShipAiPathCorridorHost& host)
{
    // 009ED3E3 MOVSS XMM0,[00CE3930] then 009ED3F5 / 009ED3FB: both slots
    // start at the same 20.0f the plan constructor 009D9CFB / 009D9D00 writes.
    ShipAiPathCorridorWidths out{};

    // 009ED3EF MOV ECX,[ESI+3FCh], 009ED401 CALL 00778890, 009ED408 JZ
    // 009ED490: with no group, or in a group this unit does not lead, both
    // widths stay at the default and no extent is read.
    if (!host.unit_leads_group_00778890()) {
        return out;
    }
    out.from_group = true;

    // 009ED40E / 009ED414 reload ECX = [[nav+3FCh]+284h] for 009ED41A, and
    // 009ED449 / 009ED44F reload it again for 009ED45B: the group pointer is
    // read from the unit twice, not cached.
    //
    // 009ED455 stores the 0070D400 width in the slot 009ED4A7 pushes first and
    // 009ED48A the 0070D5D0 width in the slot 009ED4A3 pushes second, so
    // 009D9DE0 lands them in plan+4h and plan+8h in that order. The pair is
    // not swapped for an astern latch here; only the free-bearing query at
    // 009DEEF8 does that.
    out.width_a = ship_ai_group_corridor_width_009ed41f(host.group_extent_positive_0070d400());
    out.width_b = ship_ai_group_corridor_width_009ed41f(host.group_extent_negative_0070d5d0());
    return out;
}

ShipAiPathCorridorRefreshResult ship_ai_path_refresh_plan_009ed3e0(
    ShipAiPathRefreshState& state,
    float seconds,
    const std::array<float, 2>& pose,
    const std::array<float, 2>& goal,
    std::uint32_t zone_layer,
    float owner_radius,
    ShipAiPathCorridorHost& corridor,
    ShipAiPathPlannerHost& planner,
    ShipAiPathSearchHost& search)
{
    ShipAiPathCorridorRefreshResult out{};
    out.widths = ship_ai_path_corridor_widths_009ed3e0(corridor);
    // 009ED490 onwards is cc_exe_2q's projection; it performs the two 009D9DE0
    // calls at 009ED4AE and 009ED4D1 and everything after them.
    out.refresh = ship_ai_path_refresh_arm_009ed4e4(state, seconds, pose, goal, zone_layer,
                                                    owner_radius, out.widths.width_a,
                                                    out.widths.width_b, planner, search);
    return out;
}

}  // namespace bsp
