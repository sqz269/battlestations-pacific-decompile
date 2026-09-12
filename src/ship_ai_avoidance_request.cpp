#include "bsp/ship_ai_avoidance_request.hpp"

// The ship AI avoidance request block, blk+3ECh..blk+3F5h. Every rule here is
// read from the listing; the addresses in the comments are the sites.
//
// Frame order, from the chain table of docs/SHIP_AI_STATES.md: on a re-plan tick
// 009F1420 (step 3) rewrites all four bytes, then the state step (step 4)
// overrides the ones it owns. On every other frame neither runs and the block
// keeps its last value while the consumers at steps 9, 10, 12, 13 and 14 read it.

namespace bsp {

ShipAiAvoidanceRequestBlock ship_ai_avoidance_request_constructed_009e468b() noexcept
{
    ShipAiAvoidanceRequestBlock block{};
    block.request.enable_3f4 = true;                          // 009E4695, byte 1
    block.request.side_filter_3f8 = kShipAvoidanceSideAll;    // 009E468B, dword 3
    block.request.flag_3fc = true;                            // 009E469C, byte 1
    block.early_out_3f5 = false;                              // 009E46A3, BL = 0
    return block;
}

ShipAiAvoidanceRequestBlock ship_ai_avoidance_request_prepass_009f1b7b(
    bool avoid_all_ship_collision) noexcept
{
    ShipAiAvoidanceRequestBlock block{};
    block.request.enable_3f4 = true;  // 009F1B7B, brain+3F4h = 1
    block.request.flag_3fc = true;    // 009F1B85, brain+3FCh = 1
    block.early_out_3f5 = false;      // 009F1B8C, brain+3FDh = 0
    // 009F1B78..009F1B9A: SETNZ on the settings byte, then LEA ECX,[ECX*4-1].
    block.request.side_filter_3f8 =
        ship_avoidance_side_filter_009f1b78(avoid_all_ship_collision);
    return block;
}

void ship_ai_stop_step_request_009e15bb(ShipAiAvoidanceRequest& request,
                                        bool making_way) noexcept
{
    if (making_way) {
        request.flag_3fc = true;                           // 009E15E4, AL = 1
        request.side_filter_3f8 = kShipAvoidanceSideAll;   // 009E15ED, 3
        request.enable_3f4 = true;                         // 009E15FB, AL = 1
    } else {
        request.flag_3fc = false;                            // 009E15BB, BL = 0
        request.side_filter_3f8 = kShipAvoidanceSideDisabled; // 009E15C4, -1
        request.enable_3f4 = true;                           // 009E15D2, 1
    }
}

void ship_ai_attack_step_request_009e2588(ShipAiAvoidanceRequest& request, int own_party,
                                          bool run_mode) noexcept
{
    // 009E2588 / 009E21F0: the side filter is the owning unit's own Party, read
    // as `[[brain+0AA8h]+54h]`. Both arms of both steps write it before the
    // latch is tested.
    request.side_filter_3f8 = own_party;
    // 009E25BC (from the CMP at 009E258E) and 009E2224 (from 009E21F6), the
    // `sub+8h` latch. Close mode, the zero side, takes the goal arm.
    request.flag_3fc = !run_mode; // 009E2669 writes 1, 009E25CC writes 0
    // blk+3ECh is not written by either step; it keeps the pre-pass value.
}

void ship_ai_land_step_request_009e1c28(ShipAiAvoidanceRequest& request,
                                        bool navigating) noexcept
{
    if (navigating) {
        request.flag_3fc = true;                          // 009E1EA2, byte 1
        request.side_filter_3f8 = kShipAvoidanceSideAll;  // 009E1EC2, 3
    } else {
        request.flag_3fc = false;                             // 009E1C28, byte 0
        request.side_filter_3f8 = kShipAvoidanceSideDisabled; // 009E1C32, -1
    }
    // blk+3ECh is not written by this step either.
}

ShipAiCruiseAvoidanceArm ship_ai_cruise_step_arm_009e11a5(
    const ShipAiCruiseAvoidanceInputs& in) noexcept
{
    // 009E11A5 `CMP ECX,8` short-circuits the AI-held test, exactly as the
    // out-of-line 00521E70 does with its `if (slot != 8)`.
    if (!in.group_slot_unassigned && !in.group_slot_ai_held) {
        return ShipAiCruiseAvoidanceArm::HelmHeldByPlayer; // 009E11B4 JZ 009E13B4
    }
    if (in.unit_player_controlled) {
        return ShipAiCruiseAvoidanceArm::UnitPlayerFlag;   // 009E11C8 JZ 009E1265
    }
    return ShipAiCruiseAvoidanceArm::CruiseRule;
}

ShipAiCruiseAvoidanceArm ship_ai_cruise_step_request_009e11d6(
    ShipAiAvoidanceRequestBlock& block, const ShipAiCruiseAvoidanceInputs& in) noexcept
{
    const ShipAiCruiseAvoidanceArm arm = ship_ai_cruise_step_arm_009e11a5(in);
    switch (arm) {
    case ShipAiCruiseAvoidanceArm::HelmHeldByPlayer:
        block.request.enable_3f4 = false;                            // 009E13B6, BL = 0
        block.request.side_filter_3f8 = kShipAvoidanceSideDisabled;  // 009E13C6, -1
        block.request.flag_3fc = false;                              // 009E13D2, BL = 0
        block.early_out_3f5 = true;                                  // 009E13DC, AL = 1
        break;
    case ShipAiCruiseAvoidanceArm::UnitPlayerFlag:
        block.request.enable_3f4 = false;                            // 009E11D6, BL = 0
        block.request.side_filter_3f8 = kShipAvoidanceSideDisabled;  // 009E11DE, -1
        block.request.flag_3fc = false;                              // 009E11EA, BL = 0
        break;
    case ShipAiCruiseAvoidanceArm::CruiseRule:
        // 009E12E4, AL = 00521E70(unit, 0) on `[unit+1ACh]`.
        block.request.enable_3f4 = in.own_slot_unassigned || in.own_slot_ai_held;
        block.request.side_filter_3f8 = kShipAvoidanceSideAll;       // 009E12F3, 3
        block.request.flag_3fc = true;                               // 009E12FF, 1
        break;
    }
    return arm;
}

bool ship_ai_avoidance_party_accepted_009ec770(
    int party_filter, int other_party, bool director_ship_collision_avoidance,
    bool avoid_all_ship_collision) noexcept
{
    // 009EC773 / 009F1052 / 009EF368: `CMP dword [blk+3F0h],0` then JL.
    if (party_filter < 0) {
        return false;
    }
    // 009EC787 / 009F106C / 009EF37B: the director byte at +241h.
    if (!director_ship_collision_avoidance) {
        return false;
    }
    // 009EC795 / 009F107A / 009EF389: settings+4h, AvoidAllShipCollision.
    if (!avoid_all_ship_collision) {
        return false;
    }
    // 009EC79F..009EC7B1, projected already by
    // ship_avoidance_side_accepted_009ec79b in src/ship_ai_settings_block.cpp.
    return ship_avoidance_side_accepted_009ec79b(other_party, party_filter);
}

bool ship_ai_neighbour_excluded_009eafde(bool accepted) noexcept
{
    return !accepted; // 009EAFDE, node+69h = (the accept byte == 0)
}

bool ship_ai_avoidance_steer_gate_009da1d0(const ShipAiAvoidanceRequest& request,
                                           ShipAiAvoidanceSteerGateHost& host)
{
    // 009DA1D3 tests blk+3FCh, the unit pointer, before the virtual call; a null
    // unit skips the kind test and falls into the pose refresh, which the image
    // then performs on the same null pointer. The projection keeps the order but
    // cannot model the null dereference, so the host owns that case.
    if (host.unit_is_kind_vtable_005c(kShipAiAvoidanceExcludedKind)) {
        return false; // 009DA1EB TEST AL,AL; 009DA1EF XOR AL,AL
    }
    if (!host.unit_pose_valid_00c8()) {
        host.refresh_unit_pose_00414db0(); // 009DA201 JNZ skips this
    }
    // 009DA20A..009DA21B: FLD [unit+100h]; FLD double [00CE3D58]; FCOMIP ST0,ST1;
    // JA. ST0 is the constant, so the taken branch is `-15.0 > depth` and an
    // unordered compare is not taken.
    if (kShipAiAvoidanceMinimumDepth > static_cast<double>(host.unit_world_y_0100())) {
        return false;
    }
    if (!request.enable_3f4) {
        return false; // 009DA21D CMP byte [blk+3ECh],0; JZ 009DA241
    }
    if (!host.director_torpedo_avoidance_0080e160_240()) {
        return false; // 009DA231 CMP byte [director+240h],0; JZ 009DA241
    }
    return true; // 009DA23A MOV EAX,1
}

float ship_ai_avoid_zone_query_half_extent_009da7eb(float look_ahead) noexcept
{
    // 009DA7EB FLD float [blk+3C8h]; 009DA7F1 FMUL double [00D7A2B0];
    // 009DA7F7 FSTP float [ESP+8]: one rounding to float32 at the store.
    const float span =
        static_cast<float>(static_cast<double>(look_ahead) * kShipAiAvoidZoneQuerySpan);
    // 009DA7FF FLD double [00CE3CA8]; FCOMIP ST0,ST1; 009DA809 JBE keeps `span`.
    // The fall-through at 009DA80B loads 300.0f, so the substitution happens only
    // when 300.0 is strictly greater, which a NaN compare is not.
    if (kShipAiAvoidZoneQueryFloorCompare > static_cast<double>(span)) {
        return kShipAiAvoidZoneQueryFloor;
    }
    return span;
}

void ship_ai_refresh_avoid_zone_searchers_009da6e0(
    ShipAiAvoidZoneSearcherSet& searchers, const ShipAiAvoidanceRequest& request,
    const ShipAiAvoidZoneSearcherInputs& inputs, ShipAiAvoidZoneSearcherHost& host)
{
    // Three identical blocks at 009DA6E6, 009DA730 and 009DA77D. Each re-reads
    // the request byte and re-asks the director, so the host sees four calls.
    for (std::size_t i = 0; i < kShipAiAvoidZoneSearcherCount; ++i) {
        const bool want =
            request.flag_3fc && host.director_land_avoidance_0080e160_242();
        ShipAiAvoidZoneSearcher& searcher = searchers.searchers[i];
        if (searcher.enabled == want) {
            continue; // 009DA719 / 009DA75C / 009DA7A9, the JZ past the whole block
        }
        searcher.enabled = want; // 009DA71D / 009DA760 / 009DA7AD
        if (want) {
            continue; // 009DA71F / 009DA766 / 009DA7B3, TEST AL,AL; JNZ
        }
        // The clear runs on the enabled-to-disabled edge only.
        host.avoid_zone_segment_list_clear_004158a0(i); // 009DA724 / 76E / 7BB
        searcher.layer_key = -1;                        // 009DA729 / 773 / 7C0
    }

    // 009DA7CA, the fourth evaluation of the same pair. Only searcher 0 is
    // refreshed; nothing read here fills searcher 1 or searcher 2.
    if (!request.flag_3fc || !host.director_land_avoidance_0080e160_242()) {
        return;
    }
    const float half = ship_ai_avoid_zone_query_half_extent_009da7eb(inputs.look_ahead_3c8);
    ShipAiAvoidZoneQuery query{};
    query.x = inputs.hull_x;         // 009DA81B, blk+184h
    query.z = inputs.hull_z;         // 009DA833, blk+188h
    query.half_width = half;         // 009DA844
    query.half_height = half;        // 009DA84A, the same value
    query.layer_key = inputs.layer_key_168; // 009DA850, blk+168h
    host.avoid_zone_query_refresh_009d7050(0, query); // 009DA854, ECX = blk+0A24h
}

} // namespace bsp
