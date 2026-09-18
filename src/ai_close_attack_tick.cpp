#include "bsp/ai_close_attack_tick.hpp"

#include <cmath>

namespace bsp {

std::uint32_t ai_close_attack_order_class(bool member_is_ship_base) noexcept {
    // 00A149FC MOV EAX,0xE08F78 then 00A14A01 JNE keeps it on the ship answer;
    // the fall-through at 00A14A03 replaces it with 0xE08EF8.
    return member_is_ship_base ? kAiSceneCommandAttackMove : kAiSceneCommandSetTarget;
}

bool ai_close_attack_member_served(bool is_plane_squadron, bool squadron_carrier_excluded,
                                   bool is_ship_base, bool controller_busy) noexcept {
    // 00A143ED PUSH 18h picks the squadron arm, whose 00A14402 PUSH 17h and
    // 00A1440C byte test are 007EDA90's shape inline; 00A14413 JE skips the
    // member. The other arm is 00A14427 PUSH 6 and the 00A1443D vtable[+2Ch].
    if (is_plane_squadron) return !squadron_carrier_excluded;
    if (!is_ship_base) return false;
    return !controller_busy;
}

float ai_close_attack_collect_radius(float collect_dist, float radius_argument) noexcept {
    // 00A13B8A FLD [EAX+1F4h], 00A13BA1 FMUL the caller's radius.
    return collect_dist * radius_argument;
}

bool ai_close_attack_candidate_admitted(float target_weight, bool in_target_group) noexcept {
    // 00A146CD FLD the weight, FLDZ, FCOMPI, JB proceeds when 0 < weight;
    // otherwise 00A146D9 TEST BL,BL keeps only a target-group member.
    if (target_weight > 0.0f) return true;
    return in_target_group;
}

float ai_close_attack_range_factor(float distance, float near_dist, float far_dist) noexcept {
    // 00419010(x0 = near, y0 = 1.0f, x1 = far, y1 = 0.0f, x = distance). The
    // degenerate arm at 00419026 returns y0 when the two x-endpoints are equal.
    if (far_dist == near_dist) return 1.0f;
    if (distance <= near_dist) return 1.0f;
    if (distance >= far_dist) return 0.0f;
    return 1.0f - (distance - near_dist) / (far_dist - near_dist);
}

float ai_close_attack_score(const AiCloseAttackScoreInputs& in) noexcept {
    // 00A14942 FMUL folds the range factor into the weight; 00A1494C picks the
    // target-group multiplier; 00A14965 picks the existing-target multiplier;
    // 00A14990-00A1499C multiplies the three.
    float score = ai_close_attack_range_factor(in.distance, in.near_dist, in.far_dist)
                * in.target_weight;
    score *= in.in_target_group ? in.target_group_member_mul : 1.0f;
    score *= in.is_existing_target ? in.existing_target_mul : 1.0f;
    return score;
}

bool ai_close_attack_candidate_wins(float score, float best) noexcept {
    // 00A149A8 FCOMPI with the candidate's score in ST0 and 00A149AC JBE skips,
    // so only a strictly greater score takes the slot.
    return score > best;
}

namespace {

float planar_distance(const float a[3], const float b[3]) noexcept {
    const float dx = a[0] - b[0];
    const float dz = a[2] - b[2];
    return std::sqrt(dx * dx + dz * dz);
}

}  // namespace

AiCloseAttackTickResult ai_close_attack_tick_00a13b60(AiCloseAttackTickHost& host,
                                                      const AiCommandObject& command,
                                                      float radius_argument,
                                                      const float centre[3]) {
    AiCloseAttackTickResult result;
    void* group = command.owner_group;
    if (group == nullptr) return result;

    const float collect = ai_close_attack_collect_radius(
        host.close_tuning_field(kAiTuningCloseAttackCollectDist), radius_argument);
    const float collect_squared = collect * collect;
    const float near_dist = host.close_tuning_field(kAiTuningCloseAttackNearDist);
    const float far_dist = host.close_tuning_field(kAiTuningCloseAttackFarDist);
    const float group_mul = host.close_tuning_field(kAiTuningCloseAttackTargetGroupMemberMul);
    const float sticky_mul = host.close_tuning_field(kAiTuningCloseAttackExistingTargetMul);
    const int own_team = host.close_own_team(group);

    const std::size_t member_count = host.close_member_count(group);
    for (std::size_t i = 0; i < member_count; ++i) {
        void* member = host.close_member_at(group, i);
        if (member == nullptr) continue;
        if (!ai_close_attack_member_served(host.close_member_is_plane_squadron(member),
                                           host.close_member_squadron_excluded(member),
                                           host.close_member_is_ship_base(member),
                                           host.close_member_controller_busy(member))) {
            continue;
        }
        ++result.members_served;

        float member_position[3] = {0.0f, 0.0f, 0.0f};
        if (!host.close_member_position(member, member_position)) continue;
        void* existing = host.close_member_current_target(member);

        void* chosen = nullptr;
        float best = 0.0f;  // the 00CE4ADC seed, which every positive score beats
        const std::size_t candidate_count = host.close_candidate_count();
        for (std::size_t c = 0; c < candidate_count; ++c) {
            void* candidate = host.close_candidate_at(c);
            if (candidate == nullptr || candidate == member) continue;
            if (host.close_candidate_team(candidate) == own_team) continue;
            if (!host.close_candidate_alive(candidate)) continue;
            float candidate_position[3] = {0.0f, 0.0f, 0.0f};
            if (!host.close_candidate_position(candidate, candidate_position)) continue;
            // 00A13BE6's collection walk keeps only what is inside the squared
            // collect radius of the centre the caller passed.
            const float from_centre = planar_distance(centre, candidate_position);
            if (from_centre * from_centre > collect_squared) continue;

            AiCloseAttackScoreInputs in;
            in.target_weight = host.close_target_weight(member, candidate);
            in.in_target_group =
                host.close_in_target_group(command.target_group, candidate);
            if (!ai_close_attack_candidate_admitted(in.target_weight, in.in_target_group)) {
                continue;
            }
            in.distance = planar_distance(member_position, candidate_position);
            in.near_dist = near_dist;
            in.far_dist = far_dist;
            in.is_existing_target = existing != nullptr && existing == candidate;
            in.target_group_member_mul = group_mul;
            in.existing_target_mul = sticky_mul;
            ++result.candidates_scored;
            const float score = ai_close_attack_score(in);
            if (ai_close_attack_candidate_wins(score, best)) {
                best = score;
                chosen = candidate;
            }
        }

        if (chosen != nullptr) {
            // 00A149DF CMP [ESP+40h],0 then the 0077D600 at 00A14A6E.
            const std::uint32_t command_class =
                ai_close_attack_order_class(host.close_member_is_ship_base(member));
            if (host.close_issue_order(member, command_class, chosen)) {
                if (command_class == kAiSceneCommandAttackMove) ++result.attack_move_orders;
                else ++result.set_target_orders;
            }
        } else if (host.close_issue_moveto(member, centre)) {
            // 00A14A78-00A14D4C builds an offset point and hands it to
            // 00A02020. The offset itself was not traced, so the centre stands
            // in for it; labelled substitution.
            ++result.fallback_movetos;
        }
    }
    return result;
}

float ai_candidate_target_weight_00a0f810(
    const AiCandidateTargetWeightInputs& in) noexcept {
    // Slot A, 00A0F848: the raw weight, zeroed at 00A0F861/00A0F864 only when
    // BOTH the attacker record's +1Ch is set and its vtable[+18h](1Ch) holds.
    float weight = in.base_weight;
    if (in.attacker_record_flag_1c && in.attacker_is_command_building) {
        weight = 0.0f;
    }
    // Slot B, 00A0F86A..00A0F889: the two record factors multiplied.
    const float factors = in.attacker_factor * in.target_factor;
    // Slot C, 00A0F88D..00A0F89E: that product, the weight and target+14h.
    const float scaled = factors * weight * in.target_scale;
    // Slot B is now reused as the objective multiplier: 1.0f at 00A0F891,
    // 10.0f at 00A0F8C6 when 008DDF90 answered true.
    const float objective = in.target_is_objective
        ? kAiCandidateWeightObjectiveMultiplier
        : kAiCandidateWeightDefaultMultiplier;
    // Slot D, 00A0F8CE..00A0F8E6.
    const float term = kAiCandidateWeightTermBase - in.target_term;
    // Slot A is now reused as the class multiplier: 1.0f at 00A0F8EE, 0.1f at
    // 00A0F929 for the trio, and 0.01f at 00A0F93D when the trio target is not
    // also a command building. The 0.01f OVERWRITES the 0.1f, same slot.
    float class_multiplier = kAiCandidateWeightDefaultMultiplier;
    if (in.target_matches_009fe0b0) {
        class_multiplier = kAiCandidateWeightFortMultiplier;
        if (!in.target_is_command_building) {
            class_multiplier = kAiCandidateWeightNonCommandWeight;
        }
    }
    // 00A0F943..00A0F956: B * C * D * A.
    return objective * scaled * term * class_multiplier;
}

}  // namespace bsp
