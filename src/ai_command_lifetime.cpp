// The four rules between a planner's target choice and the command that lands
// on the group. docs/AI_COMMAND_LIFETIME.md carries the listing.

#include "bsp/ai_command_lifetime.hpp"

namespace bsp {

bool ai_squadron_excluded_007eda90(const AiGroupableCombatantFacts& facts) noexcept {
    // 007EDA91 MOV ESI,[ECX+3D0h]; 007EDA99 JE ends false.
    if (!facts.squadron_has_carrier) return false;
    // 007EDAA0 PUSH 17h through vtable[+5Ch]; 007EDAA8 JE ends false.
    if (!facts.squadron_carrier_is_kind_17) return false;
    // 007EDAAA CMP byte [ESI+C24h],0; 007EDAB1 JNE ends false.
    if (facts.squadron_carrier_flag_0c24) return false;
    // 007EDAB3 MOV AL,1.
    return true;
}

bool ai_entity_is_groupable_combatant_009fe080(
    const AiGroupableCombatantFacts& facts) noexcept {
    if (facts.is_plane_squadron) {
        // 009FE092 CALL 007EDA90 then 009FE097 NEG AL / SBB EAX,EAX / ADD EAX,1,
        // which is a logical not of the byte in AL.
        return !ai_squadron_excluded_007eda90(facts);
    }
    // 009FE0A0..009FE0AA: the tail is the ship-base test alone.
    return facts.is_ship_base;
}

bool ai_group_has_groupable_combatant_00a2c5a0(const bool* member_is_groupable,
                                               std::size_t member_count) noexcept {
    if (member_is_groupable == nullptr) return false;
    for (std::size_t i = 0; i < member_count; ++i) {
        // 00A2C5CE CALL 009FE080 on the node's +8h; 00A2C5D5 JNE leaves true.
        if (member_is_groupable[i]) return true;
    }
    return false;
}

bool ai_group_split_runs_00a2e260(std::size_t groupable_count,
                                  std::size_t population) noexcept {
    // 00A2E334 JBE on the subset size against zero and 00A2E342 JNC against the
    // population, both unsigned: non-empty and strictly smaller.
    return groupable_count != 0u && groupable_count < population;
}

AiAttackOrderOutcome ai_attack_order_outcome_00a2cbd0(std::size_t population,
                                                      bool has_attack_command,
                                                      const void* current_target,
                                                      const void* requested_target) noexcept {
    // if (group+5644h == 0) return
    if (population == 0u) return AiAttackOrderOutcome::SkippedEmptyGroup;
    // if (command->IsType(ATTACK) && command+1Ch == target) return
    if (has_attack_command && current_target == requested_target) {
        return AiAttackOrderOutcome::SkippedSameTarget;
    }
    // Otherwise the old command is deleted through its vtable slot 0 with flag
    // 1 and the new one takes group+564Ch. No separate clear is needed.
    return AiAttackOrderOutcome::Issued;
}

AiAttackArmChoice ai_attack_arm_00a2cbd0(bool leader_passes_kind_6, bool gate,
                                         float draw, float aggressive) noexcept {
    // member->vtable[+5Ch](6) == 0 && 00A2C9F0(00A10C20()) && 00BD2F40() > aggressive
    if (leader_passes_kind_6) return AiAttackArmChoice::MoveToAttack;
    if (!gate) return AiAttackArmChoice::MoveToAttack;
    return draw > aggressive ? AiAttackArmChoice::CautiousAttack
                             : AiAttackArmChoice::MoveToAttack;
}

}  // namespace bsp
