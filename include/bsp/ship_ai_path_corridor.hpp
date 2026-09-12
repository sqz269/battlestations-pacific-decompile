// The head of 009ED3E0: where a ship's unit group becomes the two corridor
// widths a path plan is latched to, and the only thing that can throw a plan
// away while its search is still running.
//
// Addresses: 009ED3E0 (head 009ED3E0-009ED498), 00778890, 0070D400, 0070D5D0,
// 009D9DE0 (the latch, reconstructed by cc_exe_2q in bsp/ship_ai_path_refresh.hpp).
//
// Packet cc_ai_corridor. Every descriptive name here is a hypothesis, not a
// recovered symbol.
//
// One pass of 009ED3E0 is three pieces:
//
//   009ED3E0-009ED498  this file: seed both widths with the 20.0f at 00CE3930,
//                      and when 00778890 says the unit leads its group,
//                      replace them with the group's two lateral extents
//                      (0070D400 and 0070D5D0) plus the 20.0 at 00CE3D88,
//                      capped at the 600.0 at 00D20198/00CE4BC4.
//   009ED49A-009ED4E3  bsp/ship_ai_path_refresh.hpp: hand both widths to
//                      009D9DE0 on each of the two plan blocks and OR both
//                      answers into nav+2FCh.
//   009ED4E4-009ED69E  bsp/ship_ai_path_refresh.hpp: the seed / revalidate /
//                      search / swap arm.
//
// The widths live on the plan block, not on the navigator: 009D9DE0 stores
// them into plan+4h and plan+8h on every call (docs/SHIP_AI_PATH_PLANNER.md
// calls the second `published_width`, and 009EE61E publishes max(30.0f, +8h)
// for the point the follower steers to).
//
// docs/SHIP_AI_PATH_CORRIDOR.md carries the evidence for every line here.
#pragma once

#include <array>
#include <cstdint>

#include "bsp/ship_ai_path_planner.hpp"
#include "bsp/ship_ai_path_refresh.hpp"
#include "bsp/ship_ai_path_search.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The unit group, as the three callees read it
// ---------------------------------------------------------------------------
// 00778890 reads entity+284h; 0070D400, 0070D5D0 and 0070E450 walk the record
// array at group+18h with the count at group+4F8h, and 0070D060
// BSP_UnitGroup_MemberAt returns [group + 34h*index + 18h].

inline constexpr std::uint32_t kShipAiEntityUnitGroup = 0x0284u;      // 00778890, 009ED414
inline constexpr std::uint32_t kShipAiUnitGroupLeader = 0x0014u;      // 0077889F, written by 0070D0C0
inline constexpr std::uint32_t kShipAiUnitGroupMembers = 0x0018u;     // 0070D070, 0070D060
inline constexpr std::uint32_t kShipAiUnitGroupMemberStride = 0x0034u;  // 0070D579 ADD ECX,34h
inline constexpr std::uint32_t kShipAiUnitGroupMemberCount = 0x04F8u; // 0070D418 MOV ESI,[ECX+4F8h]
inline constexpr std::uint32_t kShipAiUnitGroupColumn = 0x0500u;      // 0070D42F, 0070D533

// 0070D290 is the only routine read for this packet that touches both float
// columns of a member record: it takes `slot+10h + col*4` as the offset it
// multiplies by one scale and `slot+20h + col*4` as the offset it multiplies
// by another, then rotates the first into the leader's frame. So the column
// 0070D400 and 0070D5D0 reduce is the member's across-axis formation offset
// and the second column is its along-axis offset. 0070D290 is a consumer, not
// the producer: nothing read for this packet writes either column, so the two
// meanings are provisional and which sign is port is still open.
inline constexpr std::uint32_t kShipAiUnitGroupMemberLateral = 0x0010u;  // 0070D2BD, 0070D540
inline constexpr std::uint32_t kShipAiUnitGroupMemberAxial = 0x0020u;    // 0070D2CC

// The record is 34h bytes and the two column arrays start at +10h and +20h, so
// at most four columns fit in each. The image never bounds the index at
// group+500h; four is this file's hypothesis from the record size alone.
inline constexpr std::size_t kShipAiUnitGroupColumnCount = 4u;

struct ShipAiUnitGroupMember {
    std::uint32_t entity{0};                      // +00h, 0070D030 compares it
    std::array<std::uint32_t, 3> field_04{};      // +04h..+0Ch, unread here
    std::array<float, kShipAiUnitGroupColumnCount> lateral{};  // +10h..+1Ch
    std::array<float, kShipAiUnitGroupColumnCount> axial{};    // +20h..+2Ch
    std::uint32_t field_30{0};                    // +30h, unread here
};
static_assert(sizeof(ShipAiUnitGroupMember) == 0x34u,
              "the member record is 34h bytes: 0070D070 returns index*34h + 18h + group");

// What 0070D400 and 0070D5D0 need from the group object.
struct ShipAiUnitGroupView {
    const ShipAiUnitGroupMember* members{nullptr};  // group+18h
    std::int32_t count{0};                          // group+4F8h
    std::int32_t column{0};                         // group+500h
};

// ---------------------------------------------------------------------------
// Literals, all read from the image at the addresses given
// ---------------------------------------------------------------------------

// 0070D403 / 0070D5D3, the float at 00D7A24C: the seed both extent reductions
// start from, below the floor at 00CE3850 and therefore never returned.
inline constexpr float kShipAiGroupExtentSeed = 1.0f;
// 0070D40E / 0070D5E6, the float at 00CFD714: every slot value is clamped into
// [0, 1200] before it joins the maximum.
inline constexpr float kShipAiGroupExtentSlotCap = 1200.0f;
// 0070D581 / 0070D762, the float at 00CE3850: the floor under the result, so
// an empty group and a group with no offset both answer 5.
inline constexpr float kShipAiGroupExtentFloor = 5.0f;
// 0070D5A0 / 0070D781, the float at 00CFD710, compared as the double at
// 00CE3D90 (0070D590 / 0070D771): the cap on the result.
inline constexpr float kShipAiGroupExtentCap = 400.0f;
inline constexpr double kShipAiGroupExtentCapCompare = 400.0;

// 009ED41F / 009ED460, the double at 00CE3D88: the margin added to each extent
// before it becomes a corridor width. The sum is rounded to float32 by the
// FSTP at 009ED425 / 009ED466 before it is compared or stored.
inline constexpr double kShipAiGroupCorridorMargin = 20.0;
// 009ED429 / 009ED46A, the double at 00D20198, and the float at 00CE4BC4 that
// replaces the width when it is exceeded. 0070D400 already caps its answer at
// 400, so a width can never reach 620 and this cap is unreachable in the image
// as it stands.
inline constexpr double kShipAiGroupCorridorCapCompare = 600.0;
inline constexpr float kShipAiGroupCorridorCap = 600.0f;

// `kShipAiPathCorridorWidthDefault` (20.0f, 00CE3930) and
// `kShipAiPathCorridorWidthEpsilon` (25.0, 00CE3880) come from
// bsp/ship_ai_path_refresh.hpp; this file never redefines them.

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00778890, __fastcall(entity) -> bool in AL, RET 0, body 00778890-007788A7,
// complete. `group` is entity+284h and `group_leader` is [group+14h] when the
// group is non-null; the routine answers "this entity is its group's leader".
// 007788B0-007788C7 is its exact complement and belongs to
// docs/SHIP_AI_ARM_FINAL_STEP.md.
bool ship_ai_unit_group_leads_00778890(std::uint32_t entity,
                                       std::uint32_t group,
                                       std::uint32_t group_leader) noexcept;

// 0070D400, __fastcall(group) -> float in ST0, RET 0, body 0070D400-0070D5C3,
// complete (the four-way unrolled body 0070D450-0070D52E and the remainder
// loop 0070D546-0070D57F are the same step). The largest across-axis offset in
// the selected column, each slot clamped into [0, 1200], the maximum seeded at
// 1 and the answer clamped into [5, 400].
float ship_ai_unit_group_extent_positive_0070d400(const ShipAiUnitGroupView& group) noexcept;

// 0070D5D0, __fastcall(group) -> float in ST0, RET 0, body 0070D5D0-0070D7A4,
// complete. The same reduction over `-0.0f - offset` (0070D5DB loads the
// -0.0f at 00D7A208, 0070D625 / 0070D727 subtract), so it answers the largest
// offset on the other side of the axis as a positive number.
float ship_ai_unit_group_extent_negative_0070d5d0(const ShipAiUnitGroupView& group) noexcept;

// 009ED41F-009ED447, and identically 009ED460-009ED488: an extent becomes a
// corridor width. `extent + 20.0` in double, rounded to float32, replaced by
// 600.0f when it exceeds 600.0.
float ship_ai_group_corridor_width_009ed41f(float extent) noexcept;

// Derived, not a routine in the image: the executable-facing form of "when is
// a running plan thrown away". Two extents on the same side, one pass apart,
// invalidate a plan whose search has started exactly when the widths they
// produce differ by more than kShipAiPathCorridorWidthEpsilon - that is what
// 009D9DE0 compares. Both arguments are extents as 0070D400 / 0070D5D0 return
// them, not widths.
bool ship_ai_group_corridor_extent_change_invalidates(float extent_before,
                                                      float extent_after) noexcept;

// ---------------------------------------------------------------------------
// The host: one pure-virtual method per native call site in 009ED3E0's head
// ---------------------------------------------------------------------------

struct ShipAiPathCorridorHost {
    virtual ~ShipAiPathCorridorHost() = default;

    // 009ED401 CALL 00778890, ECX = [nav+3FCh] (009ED3EF): does this unit lead
    // its group? Everything below runs only when it answers true (009ED408 JZ
    // 009ED490).
    virtual bool unit_leads_group_00778890() = 0;

    // 009ED41A CALL 0070D400, ECX = [[nav+3FCh]+284h] (009ED40E, 009ED414).
    virtual float group_extent_positive_0070d400() = 0;

    // 009ED45B CALL 0070D5D0, ECX reloaded the same way (009ED449, 009ED44F).
    virtual float group_extent_negative_0070d5d0() = 0;
};

// What the head produced, in the order 009ED4AE and 009ED4D1 push it.
struct ShipAiPathCorridorWidths {
    // The 0070D400 side. 009ED455 stores it in the slot the first argument is
    // taken from, so 009D9DE0 lands it in plan+4h.
    float width_a{kShipAiPathCorridorWidthDefault};
    // The 0070D5D0 side, 009ED48A, second argument, plan+8h.
    float width_b{kShipAiPathCorridorWidthDefault};
    // 00778890 answered true and both widths came from the group.
    bool from_group{false};
};

// 009ED3E0-009ED498, __thiscall(nav)(float seconds) as far as the widths go.
// The pair is not swapped when the ship's latch is astern: 009DEEF8 swaps the
// same two extents into the free-bearing query on `blk+35Ch == 2`, and this
// head has no such test.
ShipAiPathCorridorWidths ship_ai_path_corridor_widths_009ed3e0(ShipAiPathCorridorHost& host);

// The whole of 009ED3E0: the head above, then cc_exe_2q's projection of
// 009ED49A-009ED69E with the widths it computed. Arguments after `host` are
// the ones ship_ai_path_refresh_arm_009ed4e4 already documents.
struct ShipAiPathCorridorRefreshResult {
    ShipAiPathCorridorWidths widths{};
    ShipAiPathRefreshResult refresh{};
};

ShipAiPathCorridorRefreshResult ship_ai_path_refresh_plan_009ed3e0(
    ShipAiPathRefreshState& state,
    float seconds,
    const std::array<float, 2>& pose,
    const std::array<float, 2>& goal,
    std::uint32_t zone_layer,
    float owner_radius,
    ShipAiPathCorridorHost& corridor,
    ShipAiPathPlannerHost& planner,
    ShipAiPathSearchHost& search);

}  // namespace bsp
