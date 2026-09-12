// The scoring tails of the AI commander's Defend and Capture planners, the AI
// command factory and the script group spawner. See docs/AI_PLANNER_TAILS.md
// for the address evidence behind every rule. Read-only analysis; the names are
// hypotheses and nothing here is ABI-compatible.
#include "bsp/ai_planner_tails.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x). Local to this file:
// the projection has no shared math header, and the AI callers are the only
// ones this packet read.
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x <= x0) {
        return y0;
    }
    if (x >= x1) {
        return y1;
    }
    const float span = x1 - x0;
    if (span == 0.0F) {
        return y0;
    }
    return y0 + (y1 - y0) * ((x - x0) / span);
}

char ascii_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// 00425850 BSP_NativeString_EqualsCStringInsensitive, restricted to the ASCII
// class names and prefixes the two routines compare.
bool equals_insensitive(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a != '\0' && *b != '\0') {
        if (ascii_lower(*a) != ascii_lower(*b)) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

// 00469840 BSP_NativeString_Substring(s, 0, n) followed by the comparison: a
// string shorter than n cannot match an n-character prefix.
bool has_prefix_insensitive(const char* s, const char* prefix) noexcept {
    if (s == nullptr || prefix == nullptr) {
        return false;
    }
    for (std::size_t i = 0; prefix[i] != '\0'; ++i) {
        if (s[i] == '\0' || ascii_lower(s[i]) != ascii_lower(prefix[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// 009FFC10 and the team flip
// ---------------------------------------------------------------------------

float ai_tail_horizontal_length(const float v[3]) noexcept {
    if (v == nullptr) {
        return 0.0F;
    }
    // 009FFC1F-009FFC5F: x and z only, and exactly zero at or below the epsilon.
    const double squared = static_cast<double>(v[0]) * v[0] + static_cast<double>(v[2]) * v[2];
    if (squared <= kAiTailLengthEpsilon) {
        return 0.0F;
    }
    return static_cast<float>(std::sqrt(squared));
}

int ai_tail_enemy_team(int own_team) noexcept {
    // 00A1E27D: SETZ on TEST EBX,EBX. Team 2 maps onto team 0.
    return own_team == 0 ? 1 : 0;
}

// ---------------------------------------------------------------------------
// 00A03760
// ---------------------------------------------------------------------------

float ai_tail_unit_arrival_value(const AiTailArrivalValueInputs& in) noexcept {
    // 00A037E5: inside the capture radius the unit counts in full.
    if (in.distance <= in.capture_radius) {
        return in.capture_weight;
    }
    // 00A03840: a unit that is neither IsType(6) nor IsType(18h) has no speed.
    if (in.speed <= kAiTailMinArrivalSpeed) {
        return 0.0F;
    }
    const float time_to_arrive = (in.distance - in.capture_radius) / in.speed;
    // 00A03864-00A03880: the tuning value is floored at one second.
    const float limit =
        in.arrive_to_range_time < 1.0F ? 1.0F : in.arrive_to_range_time;
    if (time_to_arrive >= limit) {
        return 0.0F;
    }
    // 00A038A0-00A038A4.
    return (1.0F - time_to_arrive / limit) * in.capture_weight;
}

// ---------------------------------------------------------------------------
// 00A1E250
// ---------------------------------------------------------------------------

float ai_tail_reach_factor(float nearest_distance) noexcept {
    // 00A1E664-00A1E692: the distance is floored at the 1000 reference before
    // the division, so the factor never exceeds one.
    const float floored = nearest_distance < kAiTailDistanceReference
                              ? kAiTailDistanceReference
                              : nearest_distance;
    return kAiTailDistanceReference / floored;
}

AiTailCaptureScoreTerms ai_tail_capture_score(const AiTailCaptureScoreInputs& in) noexcept {
    AiTailCaptureScoreTerms terms;
    terms.own_value = in.own_value;
    terms.enemy_value = in.enemy_value;

    const float point_value =
        in.capture_point_value == 0.0F ? kAiTailDefaultCapturePointValue : in.capture_point_value;
    terms.own_reach =
        in.own_resources * ai_tail_reach_factor(in.own_nearest_distance) / point_value;
    terms.enemy_reach =
        in.enemy_resources * ai_tail_reach_factor(in.enemy_nearest_distance) / point_value;
    terms.strategic_gain = in.strategic_gain;

    // 00A1E3E2-00A1E3FB.
    const float value_term = terms.own_value - terms.enemy_value * kAiTailEnemyHalfWeight;
    // 00A1E70E-00A1E739: the resource term is clamped at zero.
    float reach_term = terms.own_reach - terms.enemy_reach * kAiTailEnemyHalfWeight;
    if (reach_term < 0.0F) {
        reach_term = 0.0F;
    }
    // 00A1E74B-00A1E773: the sum is floored before StrategicGain is added, so
    // the printed decomposition can disagree with the printed total.
    float subtotal = value_term + reach_term;
    if (subtotal < in.min_cb_target_weight) {
        subtotal = in.min_cb_target_weight;
    }
    // 00A1E81D-00A1E830.
    terms.total = subtotal + terms.strategic_gain;
    return terms;
}

float ai_tail_capture_record_price(const AiTailCaptureScoreTerms& terms,
                                   float target_capture_weight, float capture_point_value,
                                   float min_resource) noexcept {
    // 00A2A18B-00A2A1E2: the enemy's investment (b + d) priced at the value of a
    // capture point, floored by the point's own weight.
    const float enemy_investment =
        capture_point_value * (terms.enemy_reach + terms.enemy_value);
    const float weight_floor = min_resource * target_capture_weight;
    return enemy_investment >= weight_floor ? enemy_investment : weight_floor;
}

// ---------------------------------------------------------------------------
// The Defend merge pass
// ---------------------------------------------------------------------------

namespace {

float planar_distance_squared(const float a[3], const float b[3]) noexcept {
    const float dx = a[0] - b[0];
    const float dz = a[2] - b[2];
    return dx * dx + dz * dz;
}

}  // namespace

bool ai_tail_defend_group_near_target(const float leader[3], const float defended[3],
                                      float merge_target_dist) noexcept {
    if (leader == nullptr || defended == nullptr) {
        return false;
    }
    // 00A29A4A-00A29A56: JA skips the group when the squared distance exceeds
    // the squared tuning value, so equality still passes.
    const float limit = merge_target_dist * merge_target_dist;
    return planar_distance_squared(leader, defended) <= limit;
}

bool ai_tail_defend_groups_mergeable(const float leader_a[3], const float leader_b[3],
                                     float merge_groups_dist) noexcept {
    if (leader_a == nullptr || leader_b == nullptr) {
        return false;
    }
    // 00A29B73-00A29B7F: JBE skips, so the merge needs a strict inequality.
    const float limit = merge_groups_dist * merge_groups_dist;
    return limit > planar_distance_squared(leader_a, leader_b);
}

// ---------------------------------------------------------------------------
// The Capture think's scoring blocks
// ---------------------------------------------------------------------------

float ai_tail_capture_spawn_delay(float planner_age, float delay_start, float delay_end,
                                  float delay_time) noexcept {
    // 00A2A09A-00A2A0A6: InterpolateClamped(0, SpawnDelay[1], SpawnDelayTime,
    // SpawnDelay[2], plannerAge).
    return interpolate_clamped(0.0F, delay_start, delay_time, delay_end, planner_age);
}

bool ai_tail_capture_spawn_due(float planner_age, float since_last_spawn, float delay_start,
                               float delay_end, float delay_time) noexcept {
    // 00A2A0AF-00A2A0C1: JA keeps the flag set, so the comparison is strict.
    return since_last_spawn >
           ai_tail_capture_spawn_delay(planner_age, delay_start, delay_end, delay_time);
}

float ai_tail_capture_attack_weight(float distance, float near_dist, float near_mul,
                                    float far_dist, float far_mul) noexcept {
    // 00A2A5FA-00A2A646: the pushes give the argument order
    // (Dist[1], Mul[1], Dist[2], Mul[2], distance).
    return interpolate_clamped(near_dist, near_mul, far_dist, far_mul, distance);
}

float ai_tail_capture_blend(float own_weight, float strategic_weight,
                            float command_building_mul) noexcept {
    // 00A2AC11-00A2AC3E.
    return (1.0F - command_building_mul) * own_weight + command_building_mul * strategic_weight;
}

float ai_tail_capture_spawn_budget(float party_resource_cap, float defend_resource_percent,
                                   float already_committed) noexcept {
    // 00A2B4AB-00A2B4E0: the Defend reserve is taken off the top.
    return party_resource_cap * (1.0F - defend_resource_percent) - already_committed;
}

bool ai_tail_capture_may_compose(float budget) noexcept {
    // 00A2B4E4-00A2B4EE: JBE skips when the budget is at or below one.
    return budget > 1.0F;
}

// ---------------------------------------------------------------------------
// 00A10D50 and 00A10C60
// ---------------------------------------------------------------------------

bool ai_tail_merge_leader_strength_ok(float own_leader_weight,
                                      float other_leader_weight) noexcept {
    // 00A10D9F-00A10DA7: FXCH then FCOMIP, and JC returns false, so the other
    // group's lead unit must be at least as heavy as ours.
    return other_leader_weight >= own_leader_weight;
}

bool ai_tail_merge_leader_distance_ok(const float own_leader[3], const float other_leader[3],
                                      float auto_merge_dist) noexcept {
    if (own_leader == nullptr || other_leader == nullptr) {
        return false;
    }
    // 00A10CE3-00A10D3A, the same rule as ai_group_within_auto_merge_distance.
    const float limit = auto_merge_dist * auto_merge_dist;
    return limit > planar_distance_squared(own_leader, other_leader);
}

// ---------------------------------------------------------------------------
// 00A13340
// ---------------------------------------------------------------------------

AiTailCommandRecipe ai_tail_command_recipe(const char* command_type_name) noexcept {
    AiTailCommandRecipe recipe;
    // 00A133B2-00A1341F: no key, an empty name, or a name equal to the id-0
    // entry all build NONCONTROL.
    if (command_type_name == nullptr || command_type_name[0] == '\0') {
        recipe.type = AiCommandType::NonControl;
        recipe.instance_size = 0x08u;
        return recipe;
    }
    struct Row {
        const char* name;
        AiCommandType type;
        std::uint32_t size;
        AiTailCommandArgument argument;
    };
    // The chain in source order, 00A13400 then 00A133CD onward.
    static const Row kRows[] = {
        {"NONCONTROL", AiCommandType::NonControl, 0x08u, AiTailCommandArgument::None},
        {"IDLE", AiCommandType::Idle, 0x08u, AiTailCommandArgument::None},
        {"MOVETO", AiCommandType::MoveTo, 0x14u, AiTailCommandArgument::TargetPosition},
        {"CAUTIOUSMOVE", AiCommandType::CautiousMove, 0x20u, AiTailCommandArgument::TargetPosition},
        {"REGROUPINGMOVE", AiCommandType::RegroupingMove, 0x14u,
         AiTailCommandArgument::TargetPosition},
        {"MOVETOATTACK", AiCommandType::MoveToAttack, 0x20u, AiTailCommandArgument::TargetEntity},
        {"CAUTIOUSATTACK", AiCommandType::CautiousAttack, 0x2Cu,
         AiTailCommandArgument::TargetEntity},
        {"CLOSEATTACK", AiCommandType::CloseAttack, 0x20u, AiTailCommandArgument::TargetEntity},
        {"DEFENDPOSITION", AiCommandType::DefendPosition, 0x08u, AiTailCommandArgument::None},
        {"RETREAT", AiCommandType::Retreat, 0x08u, AiTailCommandArgument::None},
    };
    for (const Row& row : kRows) {
        if (equals_insensitive(command_type_name, row.name)) {
            recipe.type = row.type;
            recipe.instance_size = row.size;
            recipe.argument = row.argument;
            return recipe;
        }
    }
    // 00A13787: an unmatched name falls back to IDLE.
    recipe.type = AiCommandType::Idle;
    recipe.instance_size = 0x08u;
    return recipe;
}

// ---------------------------------------------------------------------------
// 00A16EF0
// ---------------------------------------------------------------------------

AiTailSpawnRouting ai_tail_spawn_routing(int game_mode, const char* group_name) noexcept {
    AiTailSpawnRouting routing;
    // 00A16FF2-00A1706C: modes 4..7 go straight to the one mode planner and the
    // group name is never looked at.
    switch (game_mode) {
        case 4:
            routing.route = AiTailSpawnRoute::Duel;
            routing.brain_planner_offset = 0x10u;
            return routing;
        case 5:
            routing.route = AiTailSpawnRoute::Escort;
            routing.brain_planner_offset = 0x14u;
            return routing;
        case 6:
            routing.route = AiTailSpawnRoute::Siege;
            routing.brain_planner_offset = 0x18u;
            return routing;
        case 7:
            routing.route = AiTailSpawnRoute::Competitive;
            routing.brain_planner_offset = 0x1Cu;
            return routing;
        default:
            break;
    }
    // 00A1708E-00A170CD, then 00A1715E-00A1719D.
    if (has_prefix_insensitive(group_name, kAiTailCapturePrefix)) {
        routing.route = AiTailSpawnRoute::Capture;
        routing.brain_planner_offset = 0x0Cu;
        routing.handle_text_offset = std::strlen(kAiTailCapturePrefix);
        return routing;
    }
    if (has_prefix_insensitive(group_name, kAiTailDefendPrefix)) {
        routing.route = AiTailSpawnRoute::Defend;
        routing.brain_planner_offset = 0x00u;
        routing.handle_text_offset = std::strlen(kAiTailDefendPrefix);
        return routing;
    }
    return routing;
}

// ---------------------------------------------------------------------------
// The sequence routine over the host
// ---------------------------------------------------------------------------

AiTailCaptureScoreTerms ai_capture_target_score(AiCaptureScoreHost& host, void* target,
                                                int own_team) {
    const int enemy_team = ai_tail_enemy_team(own_team);

    AiTailCaptureScoreInputs in;
    // 00A1E2A0-00A1E3CF: four world lists, one accumulator per side.
    const int unit_count = host.world_unit_count();
    for (int i = 0; i < unit_count; ++i) {
        const int team = host.world_unit_team(i);
        if (team != own_team && team != enemy_team) {
            continue;
        }
        const float value = host.unit_arrival_value(i, target);
        if (team == own_team) {
            in.own_value += value;
        } else {
            in.enemy_value += value;
        }
    }

    // 00A1E3D5-00A1E64E: a per-team minimum seeded at 1e10.
    float nearest[kAiTailTeamCount];
    for (float& slot : nearest) {
        slot = kAiTailNoUnitDistance;
    }
    const int near_count = host.near_unit_count();
    for (int i = 0; i < near_count; ++i) {
        const int team = host.near_unit_team(i);
        if (team < 0 || team >= kAiTailTeamCount) {
            continue;
        }
        const float distance = host.near_unit_distance(i, target);
        if (distance < nearest[team]) {
            nearest[team] = distance;
        }
    }
    in.own_nearest_distance =
        (own_team >= 0 && own_team < kAiTailTeamCount) ? nearest[own_team] : kAiTailNoUnitDistance;
    in.enemy_nearest_distance = (enemy_team >= 0 && enemy_team < kAiTailTeamCount)
                                    ? nearest[enemy_team]
                                    : kAiTailNoUnitDistance;

    // 00A1E6C0-00A1E70A.
    in.own_resources = host.available_resources(own_team);
    in.enemy_resources = host.available_resources(enemy_team);
    in.capture_point_value = host.tuning(kAiTailTuningCapturePointValue);
    // 00A1E74B.
    in.min_cb_target_weight = host.tuning(kAiTailTuningMinCbTargetWeight);
    // 00A1E7A5-00A1E7DA.
    in.strategic_gain = host.strategic_gain(target);

    return ai_tail_capture_score(in);
}

}  // namespace bsp
