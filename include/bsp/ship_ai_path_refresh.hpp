// 009ED3E0's plan-refresh arm: the sequencing that turns 009E3780's two-node
// seed into a plan the follower can read.
//
// Addresses: 009ED3E0 (BSP_ShipAi_RefreshPathPlan, arm 009ED4E4..009ED69E),
// 009D9DE0 (the corridor-width latch), 009E3780, 009EC680, 009D9D40.
//
// Packet cc_exe_2q. Every descriptive name here is a hypothesis, not a
// recovered symbol. docs/SHIP_AI_PATH_SEARCH.md established what the two
// 009EC680 call sites do and that the plan is usable after two ticks;
// docs/SHIP_AI_PATH_PLANNER.md established the block and the node layout.
// Nothing here redefines either.
//
// The navigator holds two 68h-byte plan blocks, nav+224h and nav+28Ch, and two
// pointers to them: nav+2F4h is the plan in use and nav+2F8h the plan being
// computed. nav+2FCh is the "a plan is being computed" byte. One pass of the
// arm does exactly one of two things:
//
//   nav+2FCh set    009ED540..009ED5CE  revalidate and search the BACK plan,
//                                       and swap it into use once its state
//                                       passes 3 (009ED5A6 CMP [EAX+1Ch],3).
//   nav+2FCh clear  009ED5D1..009ED69E  revalidate and search the FRONT plan;
//                                       when the revalidation refuses, mark the
//                                       front plan Failed, reset the back plan
//                                       and seed it, and raise nav+2FCh again.
//
// Coverage: this file projects 009ED4E4..009ED69E and the whole of 009D9DE0.
// The head 009ED3E0..009ED4E2, which computes the two corridor widths (the
// literal 20.0f at 00CE3930, replaced from the unit's group through 00778890,
// 0070D400 and 0070D5D0 when the unit belongs to one), is NOT projected; the
// caller supplies the widths. That head belongs to ship_ai_order_consumer.
#pragma once

#include <array>
#include <cstdint>

#include "bsp/ship_ai_path_planner.hpp"
#include "bsp/ship_ai_path_search.hpp"

namespace bsp {

// 00CE3930, the corridor width the head seeds both slots with before the group
// arm can replace them.
inline constexpr float kShipAiPathCorridorWidthDefault = 20.0f;
// 00CE3880, the double 009D9DE0 compares both absolute differences against.
inline constexpr double kShipAiPathCorridorWidthEpsilon = 25.0;

// 009D9DE0, __thiscall(plan)(float width_a, float width_b) -> bool in AL,
// RET 8, body 009D9DE0-009D9E4B, complete. It is a latch, not a test: it
// answers "one of the two corridor widths moved by more than 25 since the last
// pass", and then stores both widths on the plan whatever it answered. A plan
// whose search_state is still 0 answers false without comparing (009D9DE3
// CMP [ECX+1Ch],0 / JLE), so the first pass over a fresh plan only records the
// widths. plan+4h and plan+8h are the two slots; docs/SHIP_AI_PATH_PLANNER.md
// names the second `published_width`.
bool ship_ai_path_corridor_width_changed_009d9de0(ShipAiPathPlanBlock& plan,
                                                  float width_a, float width_b) noexcept;

// The navigator half the arm reads and writes. The three fields are nav+2F4h,
// nav+2F8h and nav+2FCh; `front`/`back` are which of the caller's two blocks
// each pointer currently names.
struct ShipAiPathRefreshState {
    ShipAiPathPlanBlock* in_use{nullptr};    // nav+2F4h
    ShipAiPathPlanBlock* computing{nullptr}; // nav+2F8h
    bool computing_flag_2fc{false};          // nav+2FCh
};

// What one pass did, so a caller can count without re-deriving it.
struct ShipAiPathRefreshResult {
    bool corridor_changed{false};   // either 009D9DE0 answered true
    bool seeded_fresh{false};       // the 009ED523 seed ran
    bool requested_back{false};     // the 009ED588 revalidation ran
    bool requested_front{false};    // the 009ED5FD revalidation ran
    bool front_accepted{false};     // 009ED602's AL
    bool reset_back{false};         // the 009ED635..009ED66A reset ran
    bool reseeded_back{false};      // the 009ED692 seed ran
    bool swapped{false};            // 009ED5BD / 009ED5C3
    int searched_plan_state{0};     // the plan state after the tick that ran
    bool searched_back{false};      // which plan 009EC680 ran on
    bool searched_front{false};
    ShipAiPathSearchTickResult tick{};
};

// 009ED4E4..009ED69E. `pose` is nav+184h, `goal` nav+1DCh, `zone_layer`
// nav+30Ch and `owner_radius` [nav+3FCh]+9C8h - the four arguments all four
// 009E3780 call sites push. `seconds` is the frame delta 009EC680 takes.
//
// Deviation, stated rather than hidden: 009ED528 and 009ED63B raise nav+2FCh
// and 009ED5B6 clears it, and the head's own write at 009ED4DE only ever
// raises it (BL is loaded from the byte at 009ED49A and the two 009D9DE0 arms
// can only set it). A caller that does not project the head therefore carries
// the byte forward unchanged, which is what this function expects.
ShipAiPathRefreshResult ship_ai_path_refresh_arm_009ed4e4(
    ShipAiPathRefreshState& state,
    float seconds,
    const std::array<float, 2>& pose,
    const std::array<float, 2>& goal,
    std::uint32_t zone_layer,
    float owner_radius,
    float corridor_width_a,
    float corridor_width_b,
    ShipAiPathPlannerHost& planner,
    ShipAiPathSearchHost& search);

}  // namespace bsp
