#include "bsp/ai_group_think.hpp"
#include <cstdio>
#include <cstring>

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

template<class T> T ai_debug_read(const void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof value);
    return value;
}
template<class T> void ai_debug_write(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof value);
}
void ai_debug_colour(AiGroupDebugColour& colour, std::array<std::uint32_t, 4> words) {
    if ((colour.flags & 1u) == 0) {
        colour.flags |= 1u;
        colour.words = words;
    }
}
void ai_debug_refresh_first(void* group, AiGroupDebugHost& host) {
    if (ai_debug_read<std::int32_t>(group, 0x5644) == 0) return;
    void* const head = ai_debug_read<void*>(group, 0x5640);
    void* const first = ai_debug_read<void*>(head, 0);
    void* const entity = ai_debug_read<void*>(first, 8);
    if (ai_debug_read<std::uint8_t>(entity, 0xC8) == 0) host.call_00414db0(entity);
}
std::string ai_debug_native_string(const void* header) {
    const auto length = ai_debug_read<std::uint32_t>(header, 0);
    if (!length) return {};
    return std::string(ai_debug_read<const char*>(header, 4), length);
}

} // namespace

// 00A2CDD0..00A2D43C. Original ECX is the score-list subobject, not the
// containing group: caller 00A2D87A explicitly LEAs group+24h. Producer
// 00A2B950 establishes the ACh record layout (values +0, labels +40h/stride6,
// term count +A0h, chosen +A4h, target group +A8h). Complete normal value and
// host-effect algorithm from the whole pseudocode and register-sensitive listing.
// Native temporary text is discarded; this source returns it. Pool allocations,
// checked-iterator failure paths, FH3 unwinding and binary ABI are not claimed.
std::string ai_debug_group_target_text_00a2cdd0(void* score_list,
    AiGroupDebugState& state, AiGroupDebugHost& host) {
    void* owner = ai_debug_read<void*>(score_list, 0x560C);
    if (!owner || ai_debug_read<std::int32_t>(owner, 0x5644) == 0) return {};
    std::string text = " -- " + std::to_string(ai_debug_read<std::int32_t>(owner, 0x5658)) + " --\n";
    (void)host.call_00a10c20(owner);
    std::int32_t unchosen = 0;
    for (std::int32_t i = 0; i < ai_debug_read<std::int32_t>(score_list, 0x5600); ++i) {
        auto* const record = static_cast<unsigned char*>(score_list) + static_cast<std::size_t>(i) * 0xAC;
        void* const target = ai_debug_read<void*>(record, 0xA8);
        const bool chosen = ai_debug_read<std::uint8_t>(record, 0xA4) != 0;
        const auto mode = ai_debug_read<std::int32_t>(score_list, 0x5608);
        if (mode != 0 && target && (mode > 1 || chosen)) {
            host.call_00a2c050();
            (void)host.call_00a10c20(target);
        }
        const auto limit = ai_debug_read<std::int32_t>(score_list, 0x5604);
        if (limit > 0 && limit <= unchosen) break;
        if (!chosen) ++unchosen;
        char buffer[308];
        const auto id = target ? ai_debug_read<std::int32_t>(target, 0x5658) : 0;
        std::sprintf(buffer, "%s%d :  %.3f ", chosen ? "* " : "", id,
                     static_cast<double>(ai_debug_read<float>(record, 0)));
        std::string row = buffer;
        const auto terms = ai_debug_read<std::int32_t>(record, 0xA0);
        if (terms > 1) {
            row += reinterpret_cast<const char*>(record + 0x40);
            for (std::int32_t term = 1; term < terms; ++term) {
                std::sprintf(buffer, "%.3f%s", static_cast<double>(ai_debug_read<float>(record, term * 4)),
                             reinterpret_cast<const char*>(record + 0x40 + term * 6));
                row += buffer;
            }
        }
        text += row + "\n";
    }
    if (ai_debug_read<std::int32_t>(score_list, 0x5600) == 0)
        ai_debug_colour(state.yellow_00f8aad4, {state.one_00d7a24c, state.one_00d7a24c, 0, state.one_00d7a24c});
    else
        ai_debug_colour(state.white_00f8aa84, {state.one_00d7a24c, state.one_00d7a24c,
                                            state.one_00d7a24c, state.one_00d7a24c});
    owner = ai_debug_read<void*>(score_list, 0x560C);
    ai_debug_refresh_first(owner, host);
    return text;
}

// 00A2D580..00A2D8B2, original ECX group, plain RET. Complete normal
// value/host-effect algorithm. The first pass deliberately starts AFTER the
// first member. All three vslot+114 calls take no arguments: 00A2D723 pushes
// an outstanding output vector for the later 006F7DD0 RET4, not for that slot.
// The native countdown is signed char; the second command call is conditional.
void ai_debug_group_members_overlay_00a2d580(void* group,
    AiGroupDebugState& state, AiGroupDebugHost& host) {
    if (ai_debug_read<std::int32_t>(group, 0x5644) == 0) return;
    void* head = ai_debug_read<void*>(group, 0x5640);
    void* first = ai_debug_read<void*>(head, 0);
    void* node = ai_debug_read<void*>(first, 0);
    while (node != ai_debug_read<void*>(group, 0x5640)) {
        ai_debug_colour(state.grey_00f8aa98, {state.half_00ce3800, state.half_00ce3800,
                                            state.half_00ce3800, state.one_00d7a24c});
        ai_debug_refresh_first(group, host);
        void* const entity = ai_debug_read<void*>(node, 8);
        if (ai_debug_read<std::uint8_t>(entity, 0xC8) == 0) host.call_00414db0(entity);
        node = ai_debug_read<void*>(node, 0);
    }
    head = ai_debug_read<void*>(group, 0x5640);
    node = ai_debug_read<void*>(head, 0);
    while (node != ai_debug_read<void*>(group, 0x5640)) {
        void* const entity = ai_debug_read<void*>(node, 8);
        if (entity && !host.call_007788b0(entity) && host.entity_vslot_114(entity)) {
            void* const director = host.entity_vslot_114(entity);
            const void* const descriptor = host.call_0071eb60(director);
            if (ai_debug_read<std::uint8_t>(descriptor, 1) != 0 ||
                ai_debug_read<std::uint8_t>(descriptor, 0) != 0) {
                if (ai_debug_read<std::uint8_t>(entity, 0xC8) == 0) host.call_00414db0(entity);
                std::array<float, 3> point;
                void* const repeated_director = host.entity_vslot_114(entity);
                const void* const repeated_descriptor = host.call_0071eb60(repeated_director);
                host.call_006f7dd0(repeated_descriptor, point);
                ai_debug_colour(state.green_00f8aaac, {0, state.one_00d7a24c, 0, state.one_00d7a24c});
            }
        }
        node = ai_debug_read<void*>(node, 0);
    }
    const auto countdown = ai_debug_read<std::int8_t>(group, 0x565C);
    if (countdown > 0) {
        ai_debug_write(group, 0x565C, static_cast<std::int8_t>(countdown - 1));
    } else if (state.mode_00f8a9e0 == 0) {
        const auto temporary_id = std::to_string(ai_debug_read<std::int32_t>(group, 0x5658));
        (void)temporary_id; // original integer NativeString is discarded/freed
        if (ai_debug_read<std::int32_t>(group, 0x5658) != state.selected_id_00f8a9e4) {
            ai_debug_colour(state.yellow_00f8aad4, {state.one_00d7a24c, state.one_00d7a24c, 0, state.one_00d7a24c});
            ai_debug_refresh_first(group, host);
        }
    }
    host.command_vslot_1c(ai_debug_read<void*>(group, 0x564C));
    if (state.mode_00f8a9e0 != 0 && ai_debug_read<std::int8_t>(group, 0x565C) <= 0) {
        (void)ai_debug_group_target_text_00a2cdd0(static_cast<unsigned char*>(group) + 0x24, state, host);
        return;
    }
    host.command_vslot_1c(ai_debug_read<void*>(group, 0x564C));
}

// 00A2F6F0..00A30FFF, native ECX group, stack output NativeString*, RET4,
// EAX output. Complete normal value/host-effect algorithm from the full
// 1031-line export; pool/checked-STL and FH3 cleanup are storage boundaries.
// 00A303AD..00A303B8 is CALL ReconRatio; FMUL qword D7A220; CALL _ftol2.
// Preserve it in x87 between explicit native-register entry bindings. No CRT
// implementation is reconstructed. The literal D23138 is TWO percent signs
// followed by LF; no printf consumes it. Empty groups are outside native's
// valid domain (it dereferences the first member after its checked-list check).
std::string ai_group_describe_debug_00a2f6f0(void* group,
    const char* const* unit_type_names_00e0b590, const volatile double& scale_00d7a220,
    AiGroupDescribeDebugHost& host) {
    ai_debug_write(group, 0x565C, std::uint8_t{2});
    const auto objective = host.call_00a2def0(group, ai_debug_read<std::int32_t>(group, 0x5638));
    std::string objective_text = "objectiveMembers:";
    if (objective.empty()) objective_text += " -";
    else for (const void* member : objective)
        objective_text += " " + ai_debug_native_string(static_cast<const unsigned char*>(member) + 0x154);
    const auto types = host.call_00a2e1e0(group);
    std::string type_text = "unitTypes:";
    if (types.empty()) type_text += " -";
    else for (const auto& type : types)
        type_text += std::string(" ") + unit_type_names_00e0b590[type.first] + ":" + std::to_string(type.second);

    std::string prefix;
    if (void* const object = ai_debug_read<void*>(group, 0x5654))
        prefix = host.object_vslot_2c(object) + " : ";
    const bool auto_grouping = ai_debug_read<std::uint8_t>(group, 0x5648) != 0;
    const auto command_text = host.call_00a137d0(ai_debug_read<void*>(group, 0x564C));
    void* head = ai_debug_read<void*>(group, 0x5640);
    void* first = ai_debug_read<void*>(head, 0);
    void* const leader = ai_debug_read<void*>(first, 8);
    std::string text = "leader: " + ai_debug_native_string(static_cast<unsigned char*>(leader) + 0x154)
        + "\nplayer: " + std::to_string(ai_debug_read<std::int32_t>(group, 0x5634))
        + " (party:" + std::to_string(ai_debug_read<std::int32_t>(group, 0x5638)) + ")\n"
        + "members: " + std::to_string(ai_debug_read<std::int32_t>(group, 0x5644)) + "\n"
        + "autoGrouping: " + (auto_grouping ? "on" : "off") + "\nreconRatio: ";
    void* const ratio_entry = host.recon_ratio_entry_00a2c3d0();
    void* const truncate_entry = host.truncate_st0_entry_00bf7420();
    const volatile double* const scale = &scale_00d7a220;
    std::int32_t percent;
    __asm {
        mov ecx, group
        call ratio_entry
        mov eax, scale
        fmul qword ptr [eax]
        call truncate_entry
        mov percent, eax
    }
    text += std::to_string(percent) + "%%\n" + objective_text + "\n" + type_text
        + "\n" + prefix + command_text + "\n";
    head = ai_debug_read<void*>(group, 0x5640);
    first = ai_debug_read<void*>(head, 0);
    void* const entity = ai_debug_read<void*>(first, 8);
    if (entity && host.entity_vslot_114(entity)) {
        void* const director = host.entity_vslot_114(entity);
        if (host.call_0071be40(director)) {
            void* const repeated_director = host.entity_vslot_114(entity);
            void* const weapon_command = host.call_0071be40(repeated_director);
            std::string extra = host.weapon_command_vslot_04(weapon_command);
            if (host.entity_vslot_5c(entity, 0x18)) {
                const char* const equipment = host.call_007ed500(entity);
                if (equipment) extra += std::string(", ") + equipment;
                else extra += ", NoEquip";
            }
            text += extra;
        }
    }
    return text;
}

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

bool ai_party_ai_enabled_009ffe50(bool mode_forced, bool multiplayer, int game_mode,
                                  int party_slot) noexcept {
    if (mode_forced) return ai_party_ai_enabled(game_mode, party_slot);
    // 009FFE84 CMP [game+1FE4h],0 / JNE 009FFE96: the multiplayer arm.
    if (multiplayer) return false;
    // 009FFE8D XOR EAX,EAX / TEST ESI,ESI / SETNE AL.
    return party_slot != 0;
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

    // Phase 3, 00A2E835..00A2EA5A. Five world collections. The image builds one
    // group per admitted candidate (kAiGroupSeedPerEntityBound). The OFF arm,
    // one group per collection holding every candidate, was a misreading
    // (docs/AI_CAUTIOUS_ROUTE.md section 19).
    for (int collection = 0; collection < kAiGroupSeedCollectionCount; ++collection) {
        void* seeded = nullptr;
        for (void* cursor = host.first_seed_candidate(collection); cursor != nullptr;
             cursor = host.next_seed_candidate(cursor)) {
            void* entity = host.seed_candidate_entity(cursor);
            if (!ai_group_seed_candidate(host.entity_flags(entity), host.entity_has_group(entity),
                                         host.entity_team(entity))) {
                continue;
            }
            if (kAiGroupSeedPerEntityBound) {
                // 00A2E85E..00A2E881 (and the four later loops): new(5660h),
                // 00A2DFA0(entity), one group per candidate.
                host.create_group(entity);
            } else if (seeded == nullptr) {
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

        const bool enabled = host.party_record_enabled(party) && host.party_ai_enabled(party);
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
