#include "bsp/ai_group_think.hpp"

// Packet cc2_ai_group_think. Read-only analysis; see docs/AI_GROUP_THINK.md for
// the address evidence behind every rule below.
namespace bsp {
namespace {

// 00A184F0 computes the record offset as LEA EAX,[ECX*8]; SUB EAX,ECX then
// scales by four, i.e. party*1Ch. Kept as a named helper so the stride has one
// producer in this file.
constexpr std::uint32_t party_record_offset(int party_slot) noexcept {
    return static_cast<std::uint32_t>(party_slot) * kAiGroupPartyRecordStride;
}

} // namespace

float ai_defend_resource_percent(const float* table, int difficulty) noexcept {
    // 00A18510: FLD [ECX*4 + 0xF8A8BC]; RET. The native routine indexes without
    // a bound check and the array is three wide, so the caller owns the range.
    if (table == nullptr || difficulty < 0 || difficulty >= kAiGroupDefendResourcePercentCount) {
        return 0.0f;
    }
    return table[difficulty];
}

float ai_attack_resource_percent(const float* table, int difficulty) noexcept {
    // 00A18520: FLD1; FSUBRP; FSTP [ESP]; FLD [ESP]. The store and reload round
    // the x87 subtraction to float, which this expression reproduces.
    return 1.0f - ai_defend_resource_percent(table, difficulty);
}

bool ai_party_think_due(float now, float next_think_time) noexcept {
    // 00A18329: FCOMIP of next against now, JA skips. The party thinks when the
    // scheduled time is not in the future.
    return !(next_think_time > now);
}

float ai_party_next_think_time(float now, float random_interval) noexcept {
    // 00A18352: FADD [ESP+10h] where the local holds the clock sampled at entry.
    return now + random_interval;
}

bool ai_party_ai_enabled(int game_mode, int party_slot) noexcept {
    // 009FFE62..009FFE83, the world+61Ch-set arm.
    if (game_mode > 3) {
        return true;
    }
    return party_slot == 0 || party_slot == 4;
}

AiPartyThinkMode ai_party_think_mode(int game_mode) noexcept {
    switch (game_mode) {
    case 4: return AiPartyThinkMode::ModeSpecific4;
    case 5: return AiPartyThinkMode::ModeSpecific5;
    case 6: return AiPartyThinkMode::ModeSpecific6;
    case 7: return AiPartyThinkMode::ModeSpecific7;
    default: return AiPartyThinkMode::GroupWalk;
    }
}

AiPlannerChoice ai_planner_for_group(bool has_groupable_combatant,
                                     bool has_member_in_world_set) noexcept {
    // 00A18258: JZ to the first-planner arm when 00A2C5A0 is false. When it is
    // true, 00A2C450 decides, and only its false arm reaches brain+0Ch.
    if (!has_groupable_combatant) {
        return AiPlannerChoice::FirstPlanner;
    }
    return has_member_in_world_set ? AiPlannerChoice::FirstPlanner
                                   : AiPlannerChoice::FourthPlanner;
}

bool ai_group_families_may_merge(bool into_has_ship, bool into_has_air,
                                 bool from_has_ship, bool from_has_air) noexcept {
    // 00A2C904..00A2C933. The two rejecting arms are (HasShip(into) and
    // HasAir(from)) and (not HasShip(into) and HasAir(into) and HasShip(from)).
    if (into_has_ship) {
        return !from_has_air;
    }
    if (into_has_air) {
        return !from_has_ship;
    }
    return true;
}

bool ai_group_within_auto_merge_distance(const float leader_a[3], const float leader_b[3],
                                         float auto_merge_dist) noexcept {
    // 00A2EDB6..00A2EE11: dx from +0h and dz from +8h only, then r*r > d2.
    if (leader_a == nullptr || leader_b == nullptr) {
        return false;
    }
    const float dx = leader_a[0] - leader_b[0];
    const float dz = leader_a[2] - leader_b[2];
    const float squared = dx * dx + dz * dz;
    return auto_merge_dist * auto_merge_dist > squared;
}

bool ai_group_split_applies(std::uint32_t detached_count, std::uint32_t population) noexcept {
    // 00A2E334: JBE on zero, then JNC on CMP against +5644h, both unsigned.
    return detached_count != 0u && detached_count < population;
}

bool ai_group_entity_flags_ok(const AiGroupCandidateFlags& flags) noexcept {
    // 00A2E838..00A2E84E and the identical block at 00A2DE26..00A2DE3C.
    return flags.active && !flags.flag_5d && !flags.flag_5e && !flags.flag_60;
}

bool ai_group_seed_candidate(const AiGroupCandidateFlags& flags, bool already_grouped,
                             int team_id) noexcept {
    // 00A2E850 adds the ungrouped test and 00A2E859 the JGE against 2.
    return ai_group_entity_flags_ok(flags) && !already_grouped && team_id < kAiGroupMaxSeedTeam;
}

bool ai_group_member_still_belongs(const AiGroupCandidateFlags& flags, int entity_party_slot,
                                   int group_party_slot, int entity_team, int group_team) noexcept {
    // 00A2DE40..00A2DE56: the member keeps its place only while its AI party
    // slot and its team both still match the group's.
    return ai_group_entity_flags_ok(flags) && entity_party_slot == group_party_slot &&
           entity_team == group_team;
}

AiCommandRetarget ai_command_retarget_for_class(int command_class_id) noexcept {
    switch (command_class_id) {
    case 7: return AiCommandRetarget::Class7;
    case 8: return AiCommandRetarget::Class8;
    case 9: return AiCommandRetarget::Class9;
    default: return AiCommandRetarget::None;
    }
}

void ai_coordinator_fixed_step_00a32d50(AiGroupThinkHost& host) {
    // CMP byte ptr [00E0E34C],0; JE past both calls. The flag has no writer in
    // the image and its static value is 1, so the gate is never taken in the
    // shipped build, but the test is in the instruction stream and is modelled.
    if (!host.high_level_ai_enabled()) {
        return;
    }
    // 00A32D59 then 00A32D5E. The composition pass runs first, so the party
    // pass sees the groups it seeded and merged on this same step.
    ai_groups_compose_00a2e720(host);
    ai_parties_think_00a182c0(host);
}

void ai_groups_compose_00a2e720(AiGroupThinkHost& host) {
    // Phase 1, 00A2E741..00A2E7E2. Drain the emptied-group list: every other
    // group drops its reference, the node is freed, then the group is deleted.
    while (host.emptied_group_count() > 0u) {
        void* empty = host.first_emptied_group();
        for (void* cursor = host.first_group_of_global_registry(); cursor != nullptr;
             cursor = host.next_group(cursor)) {
            if (cursor != empty) {
                host.release_group_reference(cursor, empty);
            }
        }
        host.unlink_and_free_emptied_node(empty);
        if (empty != nullptr) {
            host.destroy_group(empty);
        }
    }

    // Phase 2, 00A2E7E8..00A2E818. Eviction before the split, for every group.
    for (void* cursor = host.first_group_of_global_registry(); cursor != nullptr;
         cursor = host.next_group(cursor)) {
        host.evict_invalid_members(cursor);
        host.split_detached_members(cursor);
    }

    // Phase 3, 00A2E835..00A2EA5A. Five world collections, one new group per
    // collection holding every candidate that collection yields.
    for (int collection = 0; collection < kAiGroupSeedCollectionCount; ++collection) {
        void* seeded = nullptr;
        for (void* cursor = host.first_seed_candidate(collection); cursor != nullptr;
             cursor = host.next_seed_candidate(cursor)) {
            void* entity = host.seed_candidate_entity(cursor);
            if (!ai_group_seed_candidate(host.entity_flags(entity), host.entity_has_group(entity),
                                         host.entity_team(entity))) {
                continue;
            }
            if (seeded == nullptr) {
                seeded = host.create_group(entity);
            } else {
                host.add_group_member(seeded, entity);
            }
        }
    }

    // Phase 4, 00A2EA5C..00A2EAF7. Per-party auto-merge, restarting the party
    // scan whenever a merge lands (00A2EAE1 jumps back to the list head).
    for (int party = 0; party < kAiGroupPartySlotCount; ++party) {
        bool merged = true;
        while (merged) {
            merged = false;
            for (void* a = host.first_group_of_party(party); a != nullptr && !merged;
                 a = host.next_group(a)) {
                if (host.group_population(a) == 0u) {
                    continue;
                }
                for (void* b = host.first_group_of_party(party); b != nullptr; b = host.next_group(b)) {
                    if (host.group_population(b) == 0u || !host.can_auto_merge(a, b)) {
                        continue;
                    }
                    host.merge_group(a, b);
                    merged = true;
                    break;
                }
            }
        }
    }

    // Phase 5, 00A2EAFD..00A2EE7E. Only modes 0 through 3 merge by distance.
    if (host.game_mode() <= 3) {
        const float merge_dist = host.auto_merge_dist();
        bool merged = true;
        while (merged) {
            merged = false;
            for (void* a = host.first_group_of_proximity_list(); a != nullptr && !merged;
                 a = host.next_group(a)) {
                for (void* b = host.first_group_of_proximity_list(); b != nullptr;
                     b = host.next_group(b)) {
                    if (a == b || host.group_population(a) == 0u || host.group_population(b) == 0u) {
                        continue;
                    }
                    // 00A2EBAF: the pair is considered in one direction only.
                    if (host.group_leader_order_key(a) > host.group_leader_order_key(b)) {
                        continue;
                    }
                    if (!ai_group_within_auto_merge_distance(host.group_leader_position(a),
                                                             host.group_leader_position(b),
                                                             merge_dist)) {
                        continue;
                    }
                    host.merge_group(a, b);
                    merged = true;
                    break;
                }
            }
        }
    }

    // Phase 6, 00A2EE7E..00A2EEB5. The per-member pass, with the party published
    // for the duration and restored to -1 on the way out.
    for (int party = 0; party < kAiGroupPartySlotCount; ++party) {
        host.set_current_party(party);
        for (void* cursor = host.first_group_of_party(party); cursor != nullptr;
             cursor = host.next_group(cursor)) {
            host.group_member_pass(cursor);
        }
    }
    host.set_current_party(-1);
}

int ai_parties_think_00a182c0(AiGroupThinkHost& host) {
    const float now = host.fixed_step_clock();
    int thought = 0;
    for (int party = 0; party < kAiGroupPartySlotCount; ++party) {
        // 00A18309 publishes the party before anything else looks at a record.
        host.set_current_party(party);
        void* brain = host.party_brain(party);
        const bool immediate = brain != nullptr && host.brain_wants_immediate_think(brain);
        if (!immediate && !ai_party_think_due(now, host.next_think_time(party))) {
            continue;
        }
        // The reschedule happens before the enable test, so a disabled party
        // still burns its slot's timer.
        const float interval =
            host.random_think_interval(kAiGroupThinkIntervalMin, kAiGroupThinkIntervalMax);
        host.store_next_think_time(party, ai_party_next_think_time(now, interval));

        const bool enabled = host.party_record_enabled(party) &&
                             ai_party_ai_enabled(host.game_mode(), party);
        if (!enabled) {
            // 00A183B2..00A183CD: destroy and clear. The clear at 00A183CD sits
            // in a run Ghidra's listing omits after the free.
            if (brain != nullptr) {
                host.destroy_party_brain(brain);
                host.store_party_brain(party, nullptr);
            }
            continue;
        }
        if (brain == nullptr) {
            brain = host.create_party_brain(party);
            host.store_party_brain(party, brain);
        }
        host.party_brain_think(brain);
        ++thought;
    }
    host.set_current_party(-1);
    return thought;
}

void ai_party_brain_think_00a181a0(AiGroupThinkHost& host, void* brain) {
    const AiPartyThinkMode mode = ai_party_think_mode(host.game_mode());
    if (mode != AiPartyThinkMode::GroupWalk) {
        // Modes 4..7 tick exactly one planner and never touch the groups.
        const int slot = 4 + (static_cast<int>(mode) - static_cast<int>(AiPartyThinkMode::ModeSpecific4));
        host.planner_tick(host.brain_planner(brain, slot));
        host.party_brain_plan_tail(brain);
        return;
    }

    const int party = host.brain_party_slot(brain);
    const int world_set = host.brain_world_set_index(brain);
    for (void* cursor = host.first_group_of_party(party); cursor != nullptr;
         cursor = host.next_group(cursor)) {
        void* group = cursor;
        if (host.group_population(group) == 0u) {
            continue;
        }
        // Unconditional: the split runs even for a group another planner owns.
        host.split_detached_members(group);
        if (host.group_claiming_planner(group) != nullptr) {
            continue;
        }
        const bool combatant = host.group_has_groupable_combatant(group);
        const bool in_set = combatant && host.group_has_member_in_world_set(group, world_set);
        const int slot = ai_planner_for_group(combatant, in_set) == AiPlannerChoice::FirstPlanner
                             ? 0
                             : 3;
        host.planner_claim_group(host.brain_planner(brain, slot), group);
    }

    // 00A18288..00A182AD: the first four planners always tick, in slot order.
    for (int slot = 0; slot < 4; ++slot) {
        host.planner_tick(host.brain_planner(brain, slot));
    }
    host.party_brain_plan_tail(brain);
}

} // namespace bsp
