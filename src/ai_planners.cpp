#include <bsp/ai_planners.hpp>

#include <cmath>

namespace bsp {
namespace {

// 00A15A70's eight allocation arms, in brain-slot order. The names, the two
// vtable-adjacent name strings and the sizes all come from the constructors.
constexpr AiPlannerClass kPlannerClasses[kAiPlannerKindCount] = {
    {AiPlannerKind::Defend, "Defend", 0x00u, 0x00D22D44u, 0x00A1EFC0u, 0x00A28A60u,
     0x00A18480u, 0x38u, -1},
    {AiPlannerKind::Attack, "Attack", 0x04u, 0x00D22D94u, 0x00A1F0F0u, 0x00A1CF90u,
     0x00A18480u, 0x38u, -1},
    {AiPlannerKind::Sell, "Sell", 0x08u, 0x00D22DE4u, 0x00A1F220u, 0x00A22800u,
     0x00A18480u, 0x38u, -1},
    {AiPlannerKind::Capture, "Capture", 0x0Cu, 0x00D22E34u, 0x00A1F390u, 0x00A29FD0u,
     0x00A18480u, 0x44u, -1},
    {AiPlannerKind::Duel, "Duel", 0x10u, 0x00D22E7Cu, 0x00A1F500u, 0x00A25F70u,
     0x00A1D110u, 0x38u, 4},
    {AiPlannerKind::Escort, "Escort", 0x14u, 0x00D22EC4u, 0x00A1F630u, 0x00A26210u,
     0x00A1D140u, 0x38u, 5},
    {AiPlannerKind::Siege, "Siege", 0x18u, 0x00D22F0Cu, 0x00A1F760u, 0x00A26510u,
     0x00A1D170u, 0x38u, 6},
    {AiPlannerKind::Competitive, "Competitive", 0x1Cu, 0x00D22F54u, 0x00A1F890u,
     0x00A265F0u, 0x00A1D1A0u, 0x38u, 7},
};

// The table at 00E0E308, index 0..14.
constexpr const char* kCommandTypeNames[kAiCommandTypeCount] = {
    "NONCONTROL", "IDLE", "MOVE", "MOVETO", "CAUTIOUSMOVE", "REGROUPINGMOVE",
    "ATTACK", "MOVETOATTACK", "CAUTIOUSATTACK", "CLOSEATTACK",
    "DEFEND", "DEFENDPOSITION", "PATROLTO", "RETREAT", "SELLING",
};

int kind_index(AiPlannerKind kind) noexcept { return static_cast<int>(kind); }

} // namespace

const AiPlannerClass* ai_planner_class_table() noexcept { return kPlannerClasses; }

const AiPlannerClass& ai_planner_class(AiPlannerKind kind) noexcept {
    return kPlannerClasses[kind_index(kind)];
}

bool ai_planner_slot_constructed(AiPlannerKind kind, int game_mode) noexcept {
    const bool mode_specific = kind_index(kind) >= 4;
    if (game_mode >= 4 && game_mode <= 7) {
        return mode_specific && ai_planner_class(kind).game_mode == game_mode;
    }
    return !mode_specific;
}

const char* ai_command_type_name(AiCommandType type) noexcept {
    const int id = static_cast<int>(type);
    if (id < 0 || id >= kAiCommandTypeCount) {
        return nullptr;
    }
    return kCommandTypeNames[id];
}

AiCommandType ai_command_family(AiCommandType type) noexcept {
    switch (type) {
    case AiCommandType::MoveTo:
    case AiCommandType::CautiousMove:
    case AiCommandType::RegroupingMove:
        return AiCommandType::Move;
    case AiCommandType::MoveToAttack:
    case AiCommandType::CautiousAttack:
    case AiCommandType::CloseAttack:
        return AiCommandType::Attack;
    case AiCommandType::DefendPosition:
    case AiCommandType::PatrolTo:
    case AiCommandType::Retreat:
        return AiCommandType::Defend;
    default:
        return type;
    }
}

bool ai_command_is_type(AiCommandType self, AiCommandType query) noexcept {
    return self == query || ai_command_family(self) == query;
}

bool ai_command_overrides_merge(AiCommandType type) noexcept {
    return type == AiCommandType::NonControl || type == AiCommandType::Idle;
}

bool ai_command_can_merge(const AiCommandMergeInputs& in) noexcept {
    if (!ai_command_overrides_merge(in.type)) {
        return false; // 00A0FC80, XOR AL,AL
    }
    if (!in.other_group_present || in.other_population == 0 || in.own_population == 0) {
        return false;
    }
    if (!in.same_grouping_answer) {
        return false;
    }
    if (in.type == AiCommandType::Idle && !in.same_secondary_answer) {
        return false; // the extra equality the 00A12450 arm adds
    }
    return in.extra_test_a && in.extra_test_b;
}

bool ai_group_command_is_engaged(AiCommandType type, float move_target_distance_squared,
                                 float move_radius_squared) noexcept {
    if (ai_command_is_type(type, AiCommandType::Attack)) {
        return true;
    }
    if (ai_command_is_type(type, AiCommandType::Defend)) {
        return true;
    }
    if (!ai_command_is_type(type, AiCommandType::Move)) {
        return false;
    }
    return move_target_distance_squared > move_radius_squared;
}

bool ai_planner_needs_replan_base(const AiPlannerReplanInputs& in) noexcept {
    return in.flag_set;
}

bool ai_planner_needs_replan_mode(const AiPlannerReplanInputs& in) noexcept {
    if (in.owned_group_count == 0) {
        return in.flag_set;
    }
    return in.first_group_engaged ? in.flag_set : true;
}

bool ai_planner_needs_replan(AiPlannerKind kind, const AiPlannerReplanInputs& in) noexcept {
    return kind_index(kind) >= 4 ? ai_planner_needs_replan_mode(in)
                                 : ai_planner_needs_replan_base(in);
}

AiAttackOrder ai_group_attack_order(const AiAttackOrderInputs& in) noexcept {
    if (in.own_population == 0) {
        return AiAttackOrder::None;
    }
    if (ai_command_is_type(in.current_type, AiCommandType::Attack) &&
        in.current_target_is_chosen) {
        return AiAttackOrder::KeepCurrent;
    }
    if (!in.member_prefers_direct_attack && in.target_position_accepted &&
        in.caution_roll > in.aggressive) {
        return AiAttackOrder::CautiousAttack;
    }
    return AiAttackOrder::MoveToAttack;
}

AiCommandType ai_attack_order_command_type(AiAttackOrder order) noexcept {
    switch (order) {
    case AiAttackOrder::MoveToAttack:
        return AiCommandType::MoveToAttack;
    case AiAttackOrder::CautiousAttack:
        return AiCommandType::CautiousAttack;
    default:
        return AiCommandType::NonControl;
    }
}

float ai_target_score(const AiTargetScoreInputs& in) noexcept {
    return in.sticky * in.own_set * in.base * in.range;
}

float ai_target_scoring_distance(float squared_planar_distance,
                                 float near_radius_squared) noexcept {
    if (squared_planar_distance <= near_radius_squared) {
        return 0.0F;
    }
    return std::sqrt(squared_planar_distance);
}

bool ai_engagement_pair_accepted(const AiEngagementPairInputs& in) noexcept {
    if (in.own_population == 0 || in.enemy_population == 0) {
        return false;
    }
    if (in.own_group_party != in.brain_party) {
        return false;
    }
    if (in.squared_planar_distance > in.radius_squared) {
        return false;
    }
    if (in.own_strength < in.min_strength) {
        return false;
    }
    return in.own_strength * in.strength_factor <= in.enemy_strength &&
           in.strength_factor * in.enemy_strength <= in.own_strength;
}

int ai_enemy_team_index(int own_team) noexcept { return own_team == 0 ? 1 : 0; }

const char* ai_planner_spawn_tag(AiPlannerKind kind) noexcept {
    switch (kind) {
    case AiPlannerKind::Capture:
        return "[capture]"; // 00D22CA8, used at 00A2B6CE
    case AiPlannerKind::Duel:
        return "[duel]"; // 00D23000, used at 00A25F91
    case AiPlannerKind::Escort:
        return "[escort]"; // 00D23008, used at 00A26237
    case AiPlannerKind::Siege:
        return "[siege]"; // 00D23014, used at 00A26531
    case AiPlannerKind::Competitive:
        return "[competitive]"; // 00D2301C, used at 00A26611
    default:
        return nullptr; // Defend, Attack and Sell do not spawn from their think
    }
}

void* ai_planner_choose_attack_target(AiPlannerHost& host, void* group, int own_team,
                                     int enemy_team, float aggressive, bool reset_target) {
    if (reset_target) {
        host.clear_group_target_cache(group); // 00A1CB9C, 00A1CBA6
    }

    void* current_target = nullptr;
    if (ai_command_is_type(host.group_command_type(group), AiCommandType::Attack)) {
        current_target = host.group_command_target(group);
    }

    if (host.enemy_team_group_count(enemy_team) == 0) {
        return nullptr; // 00A1CBFB guards the whole loop
    }

    const float near_radius_squared = host.near_radius_squared();
    const float own_set_factor = host.tuning_field(kAiPlannerTuningOwnSetFactor);
    const float range_near = host.tuning_field(kAiPlannerTuningRangeNear);
    const float range_far = host.tuning_field(kAiPlannerTuningRangeFar);
    const float sticky_factor = host.tuning_field(kAiPlannerTuningStickyFactor);

    void* best = nullptr;
    float best_score = kAiPlannerScoreFloor; // 00A1CC4B seeds it from 00D22CC4

    for (void* node = host.first_enemy_team_group(enemy_team);
         !host.group_is_end(enemy_team, node); node = host.next_group(node)) {
        if (host.group_population(node) == 0) {
            continue; // 00A1CCC0
        }

        AiTargetScoreInputs score{};
        score.base = host.candidate_base_weight(node);
        const float distance = ai_target_scoring_distance(
            host.squared_planar_distance(group, node), near_radius_squared);
        score.range = host.range_interpolation(range_near, range_far, distance);
        if (host.candidate_has_member_in_own_set(node, own_team)) {
            score.own_set = own_set_factor;
        }
        if (node == current_target) {
            score.sticky = sticky_factor;
        }

        const float value = ai_target_score(score);
        if (value > best_score) { // 00A1CEAE, a strict compare against the running max
            best_score = value;
            best = node;
        }
    }

    if (best != nullptr) {
        host.order_attack(group, best, aggressive); // 00A1CEEF
    }
    return best;
}

void ai_mode_planner_tick(AiPlannerHost& host, const AiModePlannerTickInputs& in) {
    if (in.owned_group_count == 0) {
        const char* tag = ai_planner_spawn_tag(in.kind);
        if (tag != nullptr) {
            host.quick_spawn_named_group(tag);
        }
        host.publish_spawn_bias(kAiPlannerSpawnBias); // 00A26580, FLD [00CE3800]
        return;
    }

    ai_planner_choose_attack_target(host, in.first_owned_group, in.own_team,
                                    ai_enemy_team_index(in.own_team),
                                    kAiPlannerDefaultWeightFactor,
                                    host.reset_target_flag());
}

} // namespace bsp
