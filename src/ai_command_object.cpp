#include "bsp/ai_command_object.hpp"

#include "bsp/ai_planner_tails.hpp"

namespace bsp {

AiCommandType ai_command_initial_type(int ai_party_slot, bool party_slot_admitted) noexcept {
    // docs/AI_GROUP_THINK.md's constructor table for group+564Ch: NONCONTROL
    // when +5634h is negative, IDLE when 009FFE50 admits the slot, NONCONTROL
    // otherwise. The eight-byte allocation carries only the vtable and the
    // back-pointer, which is why neither class can hold a target.
    if (ai_party_slot < 0) return AiCommandType::NonControl;
    return party_slot_admitted ? AiCommandType::Idle : AiCommandType::NonControl;
}

bool ai_command_install_deletes_previous(bool group_has_command) noexcept {
    // 00A2BD00: the branch at 00A2BD09 tests group+564Ch and only the non-null
    // arm reaches the vtable[+0h](1) call at 00A2BD13.
    return group_has_command;
}

bool ai_command_notification_is_discarded(AiCommandType type) noexcept {
    // The +24h slot of all sixteen vtables between 00D22968 and 00D22C68 is
    // 00A0FC90, which is `RET 8`. No id is an exception.
    (void)type;
    return true;
}

bool ai_entity_class_matches_009fe0b0(bool is_class_1b, bool is_class_45,
                                      bool is_class_46) noexcept {
    // 009FE0B0: three calls to entity->vtable[+5Ch] with 0x1B, 0x45 and 0x46,
    // each jumping to the `MOV EAX,1` tail on the first non-zero answer.
    return is_class_1b || is_class_45 || is_class_46;
}

bool ai_group_any_member_matches_009fe0b0(const bool* member_matches,
                                          std::size_t member_count) noexcept {
    // 00A2C600: the loop at 00A2C611-00A2C643 leaves with AL = 1 at the first
    // accepted member and falls out of the list with AL = 0.
    if (member_matches == nullptr) return false;
    for (std::size_t i = 0; i < member_count; ++i) {
        if (member_matches[i]) return true;
    }
    return false;
}

bool ai_command_can_merge_with(const AiCommandMergeFacts& facts) noexcept {
    // 00A11F80 and 00A12450 share their first four tests and their last two;
    // only the IDLE body inserts the 00A2C600 equality at 00A12490-00A124A3.
    AiCommandMergeInputs in;
    in.type = facts.type;
    in.other_group_present = facts.other_group_present;
    in.other_population = facts.other_population;
    in.own_population = facts.own_population;
    in.same_grouping_answer =
        facts.own_has_groupable_combatant == facts.other_has_groupable_combatant;
    in.same_secondary_answer = facts.own_matches_009fe0b0 == facts.other_matches_009fe0b0;
    // 00A10D50 at 00A11FBC / 00A124A8, then 00A10C60 at 00A11FC8 / 00A124B4.
    in.extra_test_a = ai_tail_merge_leader_strength_ok(facts.own_leader_weight,
                                                       facts.other_leader_weight);
    in.extra_test_b = ai_tail_merge_leader_distance_ok(facts.own_leader_position,
                                                       facts.other_leader_position,
                                                       facts.auto_merge_dist);
    return ai_command_can_merge(in);
}

AiCommandMemberPassResult ai_command_member_pass_00a2c790(AiCommandMemberPassHost& host,
                                                          void* group) {
    AiCommandMemberPassResult result;
    if (group == nullptr) return result;
    // 00A2C796: CMP [EBP+5644h],0 / JZ the epilogue. An empty group neither
    // reports nor reschedules.
    if (host.group_population(group) == 0u) {
        result.next_due = host.member_pass_due(group);
        return result;
    }

    void* command = host.group_command(group);
    const std::size_t count = host.group_member_count(group);
    for (std::size_t i = 0; i < count; ++i) {
        void* member = host.group_member_at(group, i);
        ++result.members_walked;
        if (member == nullptr) continue;
        // 00A2C7F3-00A2C801: the first vt+114h call is the gate.
        void* director = host.member_weapon_director(member);
        if (director == nullptr) continue;
        ++result.directors_found;
        // 00A2C80D-00A2C826: the slot is called again for each reader.
        void* descriptor = host.director_target_descriptor(director);
        void* current = host.director_current_command(director);
        // 00A2C82B: the notification only runs while group+564Ch is non-null.
        // PUSH EDI then PUSH EAX puts the current command first.
        if (command != nullptr) {
            host.command_notify_member(command, current, descriptor);
            ++result.notifications_sent;
        }
    }

    // 00A2C859-00A2C86C: FLD the clock, FLD group+5650h, FCOMIP, JA past the
    // tick. The tick runs while the due time is at or below the clock.
    const float now = host.fixed_step_clock();
    const float due = host.member_pass_due(group);
    result.next_due = due;
    if (due > now || command == nullptr) return result;

    host.command_tick(command);            // vt+0Ch at 00A2C879
    float low = kAiCommandMemberPassIntervalLow;
    float high = kAiCommandMemberPassIntervalHigh;
    host.command_pass_interval(command, &low, &high);  // vt+10h at 00A2C890
    // 00A2C8B3: the draw, then FADD the clock value stashed at 00A2C898.
    result.next_due = now + host.random_interval(low, high);
    host.store_member_pass_due(group, result.next_due);
    result.ticked = true;
    return result;
}

}  // namespace bsp
