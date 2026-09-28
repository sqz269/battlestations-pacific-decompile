#include "bsp/ai_command_tick.hpp"
#include <cmath>

namespace bsp {
namespace {

void copy3(float out[3], const float in[3]) noexcept {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

}  // namespace

bool ai_order_bridge_accepts_00a02020(bool is_plane_squadron, bool squadron_excluded,
                                      bool is_ship_base) noexcept {
    // 00A02026 PUSH 18h through vtable[+5Ch]; the squadron arm calls 007EDA90
    // and negates it at 00A0203A, the other arm asks PUSH 6.
    if (is_plane_squadron) return !squadron_excluded;
    return is_ship_base;
}

AiOrderBridgePoint ai_order_bridge_point_00a02020(bool is_ship_base) noexcept {
    // 00A02089 asks vtable[+5Ch](6) again; only the ship answer takes the
    // 00417B10 branch at 00A020E8.
    return is_ship_base ? AiOrderBridgePoint::AvoidZoneOffset : AiOrderBridgePoint::Direct;
}

AiOrderBridgeResult ai_order_bridge_00a02020(bool is_plane_squadron, bool squadron_excluded,
                                             bool is_ship_base, const float member_position[3],
                                             const float target[3],
                                             const float avoid_zone_point[2]) noexcept {
    AiOrderBridgeResult result;
    if (!ai_order_bridge_accepts_00a02020(is_plane_squadron, squadron_excluded, is_ship_base)) {
        return result;
    }
    // 00A0205C-00A0206E: the squared planar separation of the member's pose
    // (+FCh, +104h) from the requested point, against the DOUBLE at 00D21530,
    // which is 6400.0 and not the 0.0f the dword reading gave. See the header.
    const float dx = member_position[0] - target[0];
    const float dz = member_position[2] - target[2];
    if (dx * dx + dz * dz < kAiOrderIssueDistanceSquared) return result;

    result.point_source = ai_order_bridge_point_00a02020(is_ship_base);
    if (result.point_source == AiOrderBridgePoint::AvoidZoneOffset) {
        // 00A020F5-00A02103: the returned pair lands in x and z, y is zeroed.
        result.position[0] = avoid_zone_point[0];
        result.position[1] = 0.0f;
        result.position[2] = avoid_zone_point[1];
    } else {
        copy3(result.position, target);
    }
    result.issued = true;
    return result;
}

std::uint32_t ai_entity_class_weight_offset_009fdf30(int class_id) noexcept {
    // The jump table at 009FDFF0, one entry per class id 6..1Ch, resolved arm
    // by arm from the bodies at 009FDF43..009FDFE4. 14h, 18h, 19h and 1Ah point
    // at the FLD1 default, as does any id outside the range.
    switch (class_id) {
    case 0x06: return 0x044u;  // OtherShip,       arm 009FDFD2
    case 0x07: return 0x014u;  // Destroyer,       arm 009FDF66
    case 0x08: return 0x018u;  // Submarine,       arm 009FDF6F
    case 0x09: return 0x000u;  // MotherShip,      arm 009FDF43
    case 0x0A: return 0x010u;  // Cruiser,         arm 009FDF5D
    case 0x0B: return 0x020u;  // Cargo,           arm 009FDF81
    case 0x0C: return 0x01Cu;  // LandingShip,     arm 009FDF78
    case 0x0D: return 0x004u;  // BattleShip,      arm 009FDF4B
    case 0x0E: return 0x024u;  // TBoat,           arm 009FDF8A
    case 0x0F: return 0x048u;  // OtherPlane,      arm 009FDFDB
    case 0x10: return 0x028u;  // LevelBomber,     arm 009FDF93
    case 0x11: return 0x030u;  // TorpedoBomber,   arm 009FDFA5
    case 0x12: return 0x034u;  // DiveBomber,      arm 009FDFAE
    case 0x13: return 0x038u;  // Fighter,         arm 009FDFB7
    case 0x15: return 0x03Cu;  // ReconPlaneSmall, arm 009FDFC0
    case 0x16: return 0x040u;  // ReconPlaneLarge, arm 009FDFC9
    case 0x17: return 0x02Cu;  // KamikazePlane,   arm 009FDF9C
    case 0x1B: return 0x00Cu;  // Landfort,        arm 009FDFE4
    case 0x1C: return 0x008u;  // CommandBuilding, arm 009FDF54
    default: return 0xFFFFFFFFu;  // 009FDFED, FLD1
    }
}

float ai_entity_leader_weight_009ffd80(float class_weight, bool is_ship_base,
                                       bool is_one_of_three_group_classes) noexcept {
    // 009FFD81 loads the 1.0f default, 009FFD9F replaces it with 100.0f on the
    // IsKindOf(6) arm, 009FFDD6 with 0.01f on the 1Bh/45h/46h arm, and 009FFDEF
    // multiplies the class weight by whichever survived.
    float multiplier = kAiLeaderWeightDefaultMultiplier;
    if (is_ship_base) {
        multiplier = kAiLeaderWeightShipMultiplier;
    } else if (is_one_of_three_group_classes) {
        multiplier = kAiLeaderWeightGroupClassMultiplier;
    }
    return class_weight * multiplier;
}

bool ai_group_member_sorts_before_00a2d8e0(float candidate_weight,
                                           float existing_weight) noexcept {
    // 00A2D94A FCOMPI with the candidate in ST0, 00A2D94E JA. JA needs both CF
    // and ZF clear, so an equal weight does not stop the walk.
    return candidate_weight > existing_weight;
}

bool ai_group_leader_point_is_origin_00a10c20(std::uint32_t population) noexcept {
    // 00A10C20 CMP [ECX+5644h],0 / JNE, and the zero arm returns 00F87574.
    return population == 0u;
}

AiFollowerAction ai_command_follower_action_00a10dc0(bool is_ship_base, bool is_plane_squadron,
                                                     bool squadron_excluded_009ffeb0) noexcept {
    // 00A10E3E PUSH 6 first: the ship arm wins outright.
    if (is_ship_base) return AiFollowerAction::JoinLeaderFormation;
    if (is_plane_squadron && !squadron_excluded_009ffeb0) {
        return AiFollowerAction::MoveToLeaderPoint;
    }
    return AiFollowerAction::None;
}

bool ai_command_follower_pass_runs_00a10dc0(std::uint32_t population) noexcept {
    // 00A10DCB CMP [group+5644h],1 / JBE: strictly more than one member.
    return population > 1u;
}

bool ai_moveto_attack_keeps_closing(bool leader_is_groupable_combatant,
                                    float leader_distance,
                                    float close_attack_collect_dist) noexcept {
    // 00A12B93 CALL 009FE080 then 00A12BA0-00A12BA8 FLD the tuning field,
    // FCOMPI against the distance, JBE past the order arm. Both must hold.
    return leader_is_groupable_combatant && leader_distance > close_attack_collect_dist;
}

AiCommandTickResult ai_command_follower_pass_00a10dc0(AiCommandTickHost& host, void* group) {
    AiCommandTickResult result;
    if (group == nullptr) return result;
    const std::uint32_t population = host.tick_group_population(group);
    if (!ai_command_follower_pass_runs_00a10dc0(population)) return result;

    float leader_point[3] = {0.0f, 0.0f, 0.0f};
    host.tick_leader_point(group, leader_point);
    const std::size_t count = host.tick_member_count(group);
    if (count == 0u) return result;
    void* leader = host.tick_member_at(group, 0);

    // 00A10DE8 starts at the node after the head, so the leader is skipped.
    for (std::size_t i = 1; i < count; ++i) {
        void* member = host.tick_member_at(group, i);
        if (member == nullptr) continue;
        ++result.followers_walked;
        const AiFollowerAction action = ai_command_follower_action_00a10dc0(
            host.tick_member_is_ship_base(member),
            host.tick_member_is_plane_squadron(member),
            host.tick_squadron_excluded_009ffeb0(member));
        if (action == AiFollowerAction::JoinLeaderFormation) {
            if (host.tick_request_join_formation(member, leader)) ++result.formation_requests;
        } else if (action == AiFollowerAction::MoveToLeaderPoint) {
            float position[3] = {0.0f, 0.0f, 0.0f};
            if (!host.tick_member_position(member, position)) continue;
            float avoid[2] = {leader_point[0], leader_point[2]};
            host.tick_avoid_zone_point(member, leader_point, avoid);
            const AiOrderBridgeResult bridge = ai_order_bridge_00a02020(
                true, host.tick_squadron_excluded_007eda90(member),
                host.tick_member_is_ship_base(member), position, leader_point, avoid);
            if (bridge.issued && host.tick_issue_moveto(member, bridge.position)) {
                ++result.orders_issued;
            }
        }
    }
    return result;
}

namespace {

// The leader arm 00A124E0, 00A12A90 and 00A15490 share: the group's first
// member is handed 00A02020 with the point the class chose.
bool order_leader(AiCommandTickHost& host, void* group, const float point[3],
                  AiCommandTickResult& result) {
    if (host.tick_group_population(group) == 0u) return false;
    void* leader = host.tick_member_at(group, 0);
    if (leader == nullptr) return false;
    float position[3] = {0.0f, 0.0f, 0.0f};
    if (!host.tick_member_position(leader, position)) return false;
    float avoid[2] = {point[0], point[2]};
    host.tick_avoid_zone_point(leader, point, avoid);
    const AiOrderBridgeResult bridge = ai_order_bridge_00a02020(
        host.tick_member_is_plane_squadron(leader),
        host.tick_squadron_excluded_007eda90(leader),
        host.tick_member_is_ship_base(leader), position, point, avoid);
    if (!bridge.issued) return false;
    if (!host.tick_issue_moveto(leader, bridge.position)) return false;
    ++result.orders_issued;
    result.leader_ordered = true;
    return true;
}

void add(AiCommandTickResult& into, const AiCommandTickResult& from) {
    into.orders_issued += from.orders_issued;
    into.formation_requests += from.formation_requests;
    into.followers_walked += from.followers_walked;
}

}  // namespace

AiCommandTickResult ai_command_tick_vt000c(AiCommandTickHost& host,
                                           const AiCommandObject& command) {
    AiCommandTickResult result;
    void* group = command.owner_group;
    if (group == nullptr) return result;

    switch (command.type) {
    case AiCommandType::MoveTo: {
        // 00A124E0: the leader must pass 009FE080, then 00A02020 with the
        // command's own +8h destination, then 00A10DC0 and 00A11070.
        if (host.tick_group_population(group) == 0u) return result;
        void* leader = host.tick_member_at(group, 0);
        if (leader == nullptr || !host.tick_member_is_groupable_combatant(leader)) return result;
        order_leader(host, group, command.target_position, result);
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        return result;
    }
    case AiCommandType::CautiousMove:
        // 00A152B0 forwards the group and the +8h destination to 00A14DD0 on
        // the +14h sub-object, then runs 00A10DC0 and 00A11690. 00A14DD0 was
        // not read, so only the follower pass is reproduced here.
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        return result;
    case AiCommandType::MoveToAttack: {
        // 00A12A90. Refresh both leaders, take the horizontal distance, and
        // while the leader is a groupable combatant farther than
        // CloseAttack_CollectDist, order it at the target group's leader and
        // run the follower and formation passes. Otherwise promote.
        void* target_group = command.target_group;
        float target_point[3] = {0.0f, 0.0f, 0.0f};
        if (target_group != nullptr) host.tick_leader_point(target_group, target_point);
        float own_point[3] = {0.0f, 0.0f, 0.0f};
        host.tick_leader_point(group, own_point);
        const float distance = host.tick_horizontal_distance(own_point, target_point);
        const float collect = host.tick_tuning_field(kAiTuningCloseAttackCollectDist);
        bool groupable = false;
        if (host.tick_group_population(group) != 0u) {
            void* leader = host.tick_member_at(group, 0);
            groupable = leader != nullptr && host.tick_member_is_groupable_combatant(leader);
        }
        if (ai_moveto_attack_keeps_closing(groupable, distance, collect)) {
            order_leader(host, group, target_point, result);
            add(result, ai_command_follower_pass_00a10dc0(host, group));
            return result;
        }
        host.tick_replace_command(group, AiCommandType::CloseAttack);
        result.promoted = true;
        result.promoted_to = AiCommandType::CloseAttack;
        return result;
    }
    case AiCommandType::CloseAttack:
        // 00A15490 hands the target group's leader point, the target group and
        // the constant 1.5f at 00CE380C to 00A13B60, then 00A11B80 and
        // 00A11AF0. 00A13B60 was not read; nothing is issued here.
        return result;
    case AiCommandType::DefendPosition:
        // 00A15500 runs 00A10DC0 and 00A11690 first, then hands its OWN
        // leader point, a null target and 1.0f to 00A13B60.
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        return result;
    case AiCommandType::CautiousAttack: {
        if constexpr (kCautiousAttackTickBound) {
        // 00A152FC-00A15339: the target group's (+1Ch) first member's +FCh,
        // 00F87574 when it is empty.
        float target_point[3] = {0.0f, 0.0f, 0.0f};
        if (command.target_group != nullptr) {
            host.tick_leader_point(command.target_group, target_point);
        }
        // 00A15342: 00A14DD0(+20h)(owner, point). Its gate is the owner's first
        // member under 009FE080. The route object is the leader's director
        // slot 0 (00778860 -> vt[+114h] -> 0071BFC0(0), +1A4h); with that
        // slot's vt[+4h] false, or the +28h counter (4 from 00A109B0) below 1,
        // it issues 00A02020 with the point. LABELLED: the director slot
        // objects and their vt[+4h]/[+0Ch]/[+10h]/[+18h] have no host
        // counterpart, so the no-route arm is taken every tick; the waypoint
        // build (three candidates per leg scored by 00A010F0, pushed into the
        // slot through vt[+18h]) and the `clearorders` arm (00E08F08 at
        // 00A14EA7) are not issued.
        AiCommandObject* state = kCautiousRouteBound ? host.tick_command_state(group) : nullptr;
        if (state != nullptr) {
            result.route = ai_cautious_approach_pass_00a14dd0(host, group, target_point, *state,
                                                             result);
            result.route_ran = true;
        } else if (host.tick_group_population(group) != 0u) {
            void* leader = host.tick_member_at(group, 0);
            if (leader != nullptr && host.tick_member_is_groupable_combatant(leader)) {
                order_leader(host, group, target_point, result);
            }
        }
        // 00A15349 00A10DC0, then 00A11690 (not read).
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        // 00A15355-00A15429: collect = tuning+1F4h; d2 = the x/z squared
        // distance between the two leaders; FCOMPI of collect^2 against d2,
        // JBE skips, so collect^2 > d2 promotes: new(20h), 00A10710(owner,
        // target), vtables 00D22C44 / 00D22C2C (CLOSEATTACK), 00A2BD00.
        float own_point[3] = {0.0f, 0.0f, 0.0f};
        host.tick_leader_point(group, own_point);
        float tgt_point[3] = {0.0f, 0.0f, 0.0f};
        if (command.target_group != nullptr) host.tick_leader_point(command.target_group, tgt_point);
        const float dx = tgt_point[0] - own_point[0];
        const float dz = tgt_point[2] - own_point[2];
        const float d2 = dx * dx + dz * dz;
        const float collect = host.tick_tuning_field(kAiTuningCloseAttackCollectDist);
        const float collect2 = collect * collect;
        if (collect2 > d2) {
            host.tick_replace_command(group, AiCommandType::CloseAttack);
            result.promoted = true;
            result.promoted_to = AiCommandType::CloseAttack;
        }
        }
        return result;
    }
    case AiCommandType::PatrolTo: {
        // 00A15570-00A1566A. The leader (the first +563Ch member) must pass
        // 009FE080; then the squared x/z distance from its +FCh (00F87574 for an
        // empty group) to the command's +8h/+10h decides `far` (JA against the
        // float 202500.0 at 00D22C98) and `near` ((tuning+1F4h * 1.5)^2 above
        // it, FCOMIP / JA), and 00A10DC0 and 00A11070 run. 00A11070 was not
        // read. Without a groupable leader, far stays 0 and near stays 1.
        if (host.tick_group_population(group) == 0u) return result;
        void* leader = host.tick_member_at(group, 0);
        if (leader == nullptr || !host.tick_member_is_groupable_combatant(leader)) return result;
        float own_point[3] = {0.0f, 0.0f, 0.0f};
        host.tick_leader_point(group, own_point);
        const float dx = own_point[0] - command.target_position[0];
        const float dz = own_point[2] - command.target_position[2];
        const float d2 = dz * dz + dx * dx;
        result.patrol_far = d2 > 202500.0f;
        const float collect = static_cast<float>(
            static_cast<double>(host.tick_tuning_field(kAiTuningCloseAttackCollectDist)) * 1.5);
        const float collect2 = collect * collect;
        result.patrol_near = collect2 > d2;
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        return result;
    }
    case AiCommandType::Idle:
        // 00A12430: CALL 00A10EC0 at 00A12433, CALL 00A10DC0 at 00A1243A, then
        // JMP 00A11070 at 00A12442. The follower pass is the middle call;
        // 00A10EC0 and 00A11070 were not read.
        add(result, ai_command_follower_pass_00a10dc0(host, group));
        return result;
    default:
        // NONCONTROL is a bare JMP to 00A10EC0, which was not read; the other
        // classes' bodies were read only to their first dispatch.
        return result;
    }
}

AiCommandTickResult ai_command_order_leader_00a02020(AiCommandTickHost& host, void* group,
                                                     const float point[3]) {
    AiCommandTickResult result;
    if (group != nullptr) order_leader(host, group, point, result);
    return result;
}

AiCommandTickResult ai_command_patrol_to_tail_00a15695(AiCommandTickHost& host,
                                                       const AiCommandObject& command,
                                                       bool patrol_far,
                                                       bool close_pass_found_target) {
    AiCommandTickResult result;
    if (!patrol_far || close_pass_found_target) return result;
    if (command.owner_group == nullptr) return result;
    order_leader(host, command.owner_group, command.target_position, result);
    return result;
}

}  // namespace bsp

namespace bsp {

AiCautiousRouteResult ai_cautious_approach_pass_00a14dd0(AiCommandTickHost& host,
    void* group, const float point[3], AiCommandObject& state, AiCommandTickResult& result) {
    AiCautiousRouteResult route;
    // 00A14DF4..00A14E16: the first member of +5640h must pass 009FE080. An
    // empty list is the CRT's invalid-iterator abort (00BF6713) in the image.
    if (host.tick_group_population(group) == 0u) return route;
    void* leader = host.tick_member_at(group, 0);
    if (leader == nullptr || !host.tick_member_is_groupable_combatant(leader)) return route;
    // 00A14E30 00778860: the leader's director slot 0; its vt[+4h] 0071FC30 is
    // `MOV AL,1`. 00A14E4C: the counter below 1 takes the moveto.
    if (!host.tick_route_slot_present(leader) || state.route_counter_28 < 1) {
        route.moveto = true;
        const float target[3] = {point[0], point[1], point[2]};
        order_leader(host, group, target, result);          // 00A15289 00A02020
        return route;
    }
    if (!host.tick_route_attached_0071fc40(leader)) state.route_flag_24 = false;  // 00A14E63
    if (state.route_flag_24 && host.tick_route_remaining_0071d2a0(leader) <= 0) {  // 00A14E77
        route.waited = true;
        return route;
    }
    if (host.tick_route_attached_0071fc40(leader)) {           // 00A14E84
        host.tick_issue_clearorders(leader);                   // 00A14EA7
        state.route_flag_24 = false;                           // 00A14EAC
        route.cleared = true;
        return route;
    }
    // 00A14EB5: EDX for 00A010F0 is (group+5638h == 0).
    const int team = host.tick_group_team(group) == 0 ? 1 : 0;
    float lead[3] = {0.0f, 0.0f, 0.0f};
    host.tick_leader_point(group, lead);                       // 00A14EC9 00A10C20
    const float d[3] = {static_cast<float>(static_cast<double>(point[0]) - lead[0]),
                        static_cast<float>(static_cast<double>(point[1]) - lead[1]),
                        static_cast<float>(static_cast<double>(point[2]) - lead[2])};
    // 00A14F1F..00A14F39: 0042B490 builds BSP_Matrix_BuildRotationY(-0.0 - pi/2)
    // ([cos 0 -sin][0 1 0][sin 0 cos]) and 00439820 transforms d by it (w = 1).
    const float angle = -0.0f - kCautiousRouteTurn;
    const double c = static_cast<float>(std::cos(static_cast<double>(angle)));
    const double s = static_cast<float>(std::sin(static_cast<double>(angle)));
    const float turned[3] = {static_cast<float>(d[0] * c + d[2] * s), d[1],
                             static_cast<float>(-(d[0] * s) + d[2] * c)};
    // 00A14FBC..00A1500D: times the double 0.3.
    const float off[3] = {static_cast<float>(turned[0] * kCautiousRouteOffset),
                          static_cast<float>(turned[1] * kCautiousRouteOffset),
                          static_cast<float>(turned[2] * kCautiousRouteOffset)};
    const int counter = state.route_counter_28;                // 00A14F3E
    state.route_counter_28 = counter - 1;                      // 00A14F48
    const float n = static_cast<float>(counter);               // 00A14F90 FILD
    const float step[3] = {static_cast<float>(d[0] / static_cast<double>(n)),
                           static_cast<float>(d[1] / static_cast<double>(n)),
                           static_cast<float>(d[2] / static_cast<double>(n))};
    float chosen[8][3]{};
    int built = 0;
    for (int k = counter - 1; k >= 1 && built < 8; --k) {      // 00A15011..00A151C7
        const float kf = static_cast<float>(k);
        float base[3];
        for (int i = 0; i < 3; ++i) {
            const float scaled = static_cast<float>(static_cast<double>(step[i]) * kf);
            base[i] = static_cast<float>(static_cast<double>(lead[i]) + scaled);
        }
        float candidate[3][3];
        for (int i = 0; i < 3; ++i) {
            candidate[0][i] = base[i];
            candidate[1][i] = static_cast<float>(static_cast<double>(off[i]) + base[i]);
            candidate[2][i] = static_cast<float>(static_cast<double>(base[i]) - off[i]);
        }
        int best = 0;
        float best_cost = kCautiousRouteNoCost;                // 00A15015
        for (int j = 0; j < 3; ++j) {
            const float cost = host.tick_danger_cost_00a010f0(candidate[j], team);
            if (best_cost > cost) {                            // 00A15157 JBE skips
                best = j;
                best_cost = cost;
            }
        }
        for (int i = 0; i < 3; ++i) chosen[built][i] = candidate[best][i];
        ++built;                                               // 00A151B4 push_back
    }
    // 00A151DD..00A15244: the vector is sent from its last element down.
    for (int i = built - 1; i >= 0; --i) host.tick_route_send_point_0071d340(leader, chosen[i]);
    host.tick_route_send_point_0071d340(leader, point);        // 00A15254
    state.route_flag_24 = true;                                // 00A1525C
    route.built = true;
    route.points = built;
    return route;
}

}  // namespace bsp
