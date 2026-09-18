#pragma once
// Why a group's attack command is issued once and never again: the four rules
// between a planner's target choice and the command that lands on the group.
//
// Addresses: 009FE080 BSP_Entity_IsGroupableCombatant (the predicate that both
// splits a group and admits it to a planner), 00A2E260 (the split's own
// decision, 00A2E334 and 00A2E342), 00A2CBD0 (the attack order's redundancy
// test and its arm choice) and 00A13340 (the command factory, not read).
//
// Packet cc8_ai_command_lifetime. docs/AI_COMMAND_LIFETIME.md.
//
// This header adds no type from bsp/ai_group_think.hpp or bsp/ai_planners.hpp
// and no top-level name that collides with them; every name is prefixed
// `ai_command_` or `ai_entity_` or `ai_group_split_`. `AiCommandType` and
// `ai_command_is_type` stay where they are, in bsp/ai_planners.hpp.
//
// The finding these rules carry: the reason packet cc8_ai_coordinator_tick
// measured exactly one attack order per mission is NOT a missing AI command
// object. 00A2CBD0 replaces the old command whenever the target differs, so a
// second target would land a second order. It is that the host's target set
// holds one group per team, because 00A2E260's split is a no-op there, and
// 009FE080 is the predicate that split turns on.

#include <cstddef>

namespace bsp {

// ---------------------------------------------------------------------------
// 009FE080, the groupable-combatant predicate
// ---------------------------------------------------------------------------

// The entity facts 009FE080 reads, all through vtable[+5Ch] IsKindOf except the
// last two, which are the squadron arm's own reads.
struct AiGroupableCombatantFacts {
    // 009FE088 PUSH 18h: the plane squadron. When this holds, the answer is the
    // NEGATION of the squadron arm below (009FE097 NEG AL / SBB / ADD 1 is a
    // logical not), and the ship test is never reached.
    bool is_plane_squadron{false};

    // 009FE0A5 PUSH 6: the ship base. The whole answer when the entity is not a
    // plane squadron.
    bool is_ship_base{false};

    // 007EDA90's three reads, taken on the squadron only.
    // 007EDA91 [squadron+3D0h]; a null pointer ends the routine false.
    bool squadron_has_carrier{false};
    // 007EDAA0 PUSH 17h, IsKindOf on that +3D0h object.
    bool squadron_carrier_is_kind_17{false};
    // 007EDAAA the byte at carrier+C24h; a set byte ends the routine false.
    bool squadron_carrier_flag_0c24{false};
};

// 007EDA90, __thiscall(squadron) -> bool. True only when the squadron's +3D0h
// object exists, is IsKindOf(17h), and its +C24h byte is clear.
bool ai_squadron_excluded_007eda90(const AiGroupableCombatantFacts& facts) noexcept;

// 009FE080 itself. A plane squadron is groupable exactly when
// ai_squadron_excluded_007eda90 is false; anything else is groupable exactly
// when it is a ship base.
//
// Note what this does NOT admit: the plane base 0Fh. An individual aircraft is
// not a groupable combatant, only a squadron is, and only a squadron whose
// carrier link fails the 007EDA90 test.
bool ai_entity_is_groupable_combatant_009fe080(
    const AiGroupableCombatantFacts& facts) noexcept;

// 00A2C5A0 BSP_AiGroup_HasGroupableCombatant walks the member list at
// group+563Ch and calls 009FE080 on each member's +8h (00A2C5CE). The group
// answer is therefore the disjunction over its members, and it is the SAME
// predicate the split uses, not a second rule.
bool ai_group_has_groupable_combatant_00a2c5a0(const bool* member_is_groupable,
                                               std::size_t member_count) noexcept;

// ---------------------------------------------------------------------------
// 00A2E260, the split's own decision
// ---------------------------------------------------------------------------

// 00A2E260 builds the subset of members for which 009FE080 holds and splits it
// into a new group only when the subset is non-empty AND strictly smaller than
// the population (00A2E334 JBE and 00A2E342 JNC, both unsigned). A group whose
// members all pass, or none of which pass, is left alone.
//
// This is the group multiplier. A mixed group of ships and aircraft splits in
// two on the first pass it is seen; a group that is all ships or all aircraft
// never splits, which is why a host whose seed phase files one group per team
// keeps exactly one group per team forever.
bool ai_group_split_runs_00a2e260(std::size_t groupable_count,
                                  std::size_t population) noexcept;

// ---------------------------------------------------------------------------
// 00A2CBD0, the attack order
// ---------------------------------------------------------------------------

// Why an attack order is skipped, so a caller can tell a no-op apart from a
// refusal. The order of the arms is 00A2CBD0's own.
enum class AiAttackOrderOutcome {
    SkippedEmptyGroup,   // group+5644h == 0
    SkippedSameTarget,   // the command IsType(ATTACK) and command+1Ch == target
    Issued,
};

// The two tests at the head of 00A2CBD0. `has_attack_command` is the command's
// IsType(ATTACK) answer, which bsp::ai_command_is_type already computes, and
// `current_target` is command+1Ch.
//
// The second test is the one that matters for a repeated think: it skips only
// when the target is the SAME. A different target falls through and replaces
// the command, so nothing has to clear the old one first.
AiAttackOrderOutcome ai_attack_order_outcome_00a2cbd0(std::size_t population,
                                                      bool has_attack_command,
                                                      const void* current_target,
                                                      const void* requested_target) noexcept;

// The arm 00A2CBD0 takes once the order is issued. `draw` is 00BD2F40's uniform
// value and `aggressive` the planner's ratio, compared at the JBE that follows:
// CAUTIOUSATTACK needs `draw > aggressive`, so a HIGHER ratio makes the cautious
// arm LESS likely. `leader_passes_kind_6` is the first member's
// vtable[+5Ch](6) answer, which must be false, and `gate` is the
// 00A2C9F0(00A10C20()) pair, which must be true.
enum class AiAttackArmChoice { MoveToAttack, CautiousAttack };

AiAttackArmChoice ai_attack_arm_00a2cbd0(bool leader_passes_kind_6, bool gate,
                                         float draw, float aggressive) noexcept;

}  // namespace bsp
