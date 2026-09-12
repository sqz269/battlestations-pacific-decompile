// Packet cc_exe_2q. See docs/GAME_EXECUTABLE.md milestone 2q and
// docs/SHIP_AI_PATH_SEARCH.md for the evidence behind every line here.

#include "bsp/ship_ai_path_refresh.hpp"

#include <cmath>

namespace bsp {

bool ship_ai_path_corridor_width_changed_009d9de0(ShipAiPathPlanBlock& plan,
                                                  float width_a, float width_b) noexcept
{
    bool changed = false;  // 009D9DE1 XOR AL,AL
    // 009D9DE3 CMP dword ptr [ECX+1Ch],0 / 009D9DE7 JLE 009D9E32: a plan that
    // has not been seeded skips the comparison entirely and only stores.
    if (plan.search_state > 0) {
        // 009D9DE9..009D9E01: float32 difference, absolute value by AND 7FFFFFFFh.
        const float diff_a = plan.field_04 - width_a;
        const float abs_a = std::fabs(diff_a);
        // 009D9DF6 FLD double [00CE3880], 009D9E07 FCOMIP, 009D9E09 JA: strictly
        // greater than the epsilon is the change.
        if (static_cast<double>(abs_a) > kShipAiPathCorridorWidthEpsilon) {
            changed = true;  // 009D9E30 MOV AL,1
        } else {
            // 009D9E0B..009D9E1E, the same shape on plan+8h.
            const float diff_b = plan.published_width - width_b;
            const float abs_b = std::fabs(diff_b);
            changed = static_cast<double>(abs_b) > kShipAiPathCorridorWidthEpsilon;
        }
    }
    // 009D9E32..009D9E43: both slots are stored on every call, whatever the answer.
    plan.field_04 = width_a;
    plan.published_width = width_b;
    return changed;
}

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
    ShipAiPathSearchHost& search)
{
    ShipAiPathRefreshResult out{};
    if (state.in_use == nullptr || state.computing == nullptr) {
        return out;
    }

    // 009ED49A MOV BL,[ESI+2FCh] then the two 009D9DE0 calls, each of which can
    // only raise BL (009ED4B5 / 009ED4D8 are both JZ past a MOV BL,1), then
    // 009ED4DE MOV [ESI+2FCh],BL.
    bool flag = state.computing_flag_2fc;
    if (ship_ai_path_corridor_width_changed_009d9de0(*state.in_use, corridor_width_a,
                                                     corridor_width_b)) {
        flag = true;  // 009ED4B7
    }
    if (ship_ai_path_corridor_width_changed_009d9de0(*state.computing, corridor_width_a,
                                                     corridor_width_b)) {
        flag = true;  // 009ED4DA
    }
    out.corridor_changed = flag && !state.computing_flag_2fc;
    state.computing_flag_2fc = flag;

    // 009ED4E4 JNZ 009ED531: the fresh seed runs only while nothing is being
    // computed and both blocks are still Empty.
    if (!flag && state.in_use->search_state == 0 && state.computing->search_state == 0) {
        const ShipAiPathPlanRequestResult seed = ship_ai_path_plan_request_009e3780(
            *state.computing, pose, goal, zone_layer, owner_radius, planner);  // 009ED523
        out.seeded_fresh = seed.seeded;
        state.computing_flag_2fc = true;  // 009ED528 MOV byte ptr [ESI+2FCh],1
    }

    // 009ED533 CMP byte ptr [ESI+2FCh],0 / 009ED53A JZ 009ED5D1.
    if (state.computing_flag_2fc) {
        ShipAiPathPlanBlock& back = *state.computing;  // 009ED540
        const int st = back.search_state;              // 009ED546
        // 009ED549 CMP EAX,1 / JZ and 009ED54E CMP EAX,3 / JG both skip to the
        // search, so the revalidation runs for states 0, 2 and 3 only.
        if (st != 1 && st <= 3) {
            if (st == 2) {
                ship_ai_path_plan_reset_009d9d40(back, planner);  // 009ED558
            }
            const ShipAiPathPlanRequestResult again = ship_ai_path_plan_request_009e3780(
                back, pose, goal, zone_layer, owner_radius, planner);  // 009ED588
            out.requested_back = true;
            out.seeded_fresh = out.seeded_fresh || again.seeded;
        }
        out.tick = ship_ai_path_search_tick_009ec680(back, seconds, search);  // 009ED59B
        out.searched_back = true;
        out.searched_plan_state = back.search_state;
        // 009ED5A0/009ED5A6 re-read the pointer and compare the state against 3;
        // 009ED5AA JLE leaves without swapping.
        if (back.search_state > 3) {
            ShipAiPathPlanBlock* const old_front = state.in_use;  // 009ED5B0
            state.computing_flag_2fc = false;                     // 009ED5B6
            state.in_use = &back;                                 // 009ED5BD
            state.computing = old_front;                          // 009ED5C3
            out.swapped = true;
        }
        return out;
    }

    // 009ED5D1: nothing is being computed, so the plan in use is revalidated.
    ShipAiPathPlanBlock& front = *state.in_use;
    const ShipAiPathPlanRequestResult live = ship_ai_path_plan_request_009e3780(
        front, pose, goal, zone_layer, owner_radius, planner);  // 009ED5FD
    out.requested_front = true;
    out.front_accepted = live.accepted;
    if (live.accepted) {  // 009ED602 TEST AL,AL / 009ED604 JZ 009ED622
        out.tick = ship_ai_path_search_tick_009ec680(front, seconds, search);  // 009ED614
        out.searched_front = true;
        out.searched_plan_state = front.search_state;
        return out;
    }

    // 009ED622: the plan in use no longer answers for this goal.
    if (front.search_state != 0) {
        front.search_state = 2;  // 009ED62D, Failed
    }
    ShipAiPathPlanBlock& back = *state.computing;  // 009ED635
    state.computing_flag_2fc = true;               // 009ED63B
    if (back.head != nullptr) {                    // 009ED642 / 009ED645
        planner.release_node_list_vtable0(back.head);  // 009ED64F CALL EDX, slot 0 with 1
    }
    back.goal_node = nullptr;                      // 009ED659
    back.head = nullptr;                           // 009ED65C
    back.node_count = 0;                           // 009ED65F
    back.zone_layer = 0;                           // 009ED662
    back.goal_clearance = kShipAiPathNoClearance;  // 009ED665, MOVSS from 00CF58EC
    back.search_state = 0;                         // 009ED66A
    out.reset_back = true;
    const ShipAiPathPlanRequestResult reseed = ship_ai_path_plan_request_009e3780(
        back, pose, goal, zone_layer, owner_radius, planner);  // 009ED692
    out.reseeded_back = reseed.seeded;
    return out;
}

}  // namespace bsp
