// Packet cc2_mission_load_hosts. See include/bsp/mission_load_hosts.hpp and
// docs/MISSION_LOAD_HOSTS.md for the evidence behind every address cited here.

#include "bsp/mission_load_hosts.hpp"

#include <algorithm>

namespace bsp {

// ---------------------------------------------------------------------------
// Step 1. 004C3840
// ---------------------------------------------------------------------------

bool mission_slot_is_bound(const MissionSlotBinding& slot) noexcept {
    // 004C38A2 CMP byte [EAX+8],0 / JZ skip
    // 004C38A8 CMP byte [EAX+9],0 / JZ enroll
    // 004C38AE CMP byte [EAX+0Ah],0 / JZ skip
    if (!slot.present_08) {
        return false;
    }
    return !slot.flag_09 || slot.flag_0a;
}

MissionPartyRoster build_mission_party_roster(const MissionSlotBinding* slots,
                                              std::size_t count) noexcept {
    MissionPartyRoster roster{};
    if (slots == nullptr) {
        return roster;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const MissionSlotBinding& slot = slots[index];
        if (!mission_slot_is_bound(slot)) {
            continue;
        }
        // 004C38B4 MOV EAX,[EAX+28h]: the party indexes the frame unchecked.
        if (slot.party_28 < 0 || static_cast<std::size_t>(slot.party_28) >= kPartyCount) {
            continue;
        }
        const std::size_t party = static_cast<std::size_t>(slot.party_28);
        int& member_count = roster.member_count[party];
        if (static_cast<std::size_t>(member_count) >= kPartyRosterStride) {
            continue;
        }
        // 004C38C5 MOV [ESP + EAX*4 + 5Ch],ESI then 004C38C9 the count bump.
        roster.members[party][member_count] = static_cast<int>(index);
        member_count += 1;
    }
    return roster;
}

bool candidate_is_reassignable(const PartyOwnerCandidate& candidate) noexcept {
    // 004C3931, 004C393B, 004C3945, 004C394F, then 004C395F CMP ECX,7 / JA.
    if (!candidate.active_5c) {
        return false;
    }
    if (candidate.flag_5d || candidate.flag_60 || candidate.dead_5e) {
        return false;
    }
    return candidate.owner_slot_188 < kPlayerSlotLimit;
}

PartyReassignDecision decide_party_reassignment(const PartyOwnerCandidate& candidate,
                                                const MissionSlotBinding* slots,
                                                std::size_t slot_count,
                                                MissionPartyRoster& roster) noexcept {
    PartyReassignDecision decision{};
    if (!candidate_is_reassignable(candidate)) {
        return decision;
    }
    // 004C3968 indexes game+18CCh with the owner slot without consulting the
    // scene record's count, so a slot past slot_count is not reachable here.
    if (slots == nullptr || candidate.owner_slot_188 >= slot_count) {
        return decision;
    }
    const MissionSlotBinding& owner = slots[candidate.owner_slot_188];
    // 004C396F..004C3983, the exact negation of mission_slot_is_bound: only a
    // unit whose owning slot is vacant is redistributed.
    if (mission_slot_is_bound(owner)) {
        return decision;
    }
    if (owner.party_28 < 0 || static_cast<std::size_t>(owner.party_28) >= kPartyCount) {
        return decision;
    }
    const std::size_t party = static_cast<std::size_t>(owner.party_28);
    decision.party = owner.party_28;
    decision.from_slot = candidate.owner_slot_188;

    const int member_count = roster.member_count[party];
    if (member_count == 0) {
        // 004C3996 JNZ not taken: the party has no bound player at all.
        decision.action = PartyReassignAction::kUnbindOwner;
        return decision;
    }
    // 004C39B0 the cursor, 004C39BC the roster entry, 004C3A1D..004C3A3C the
    // signed remainder that advances it.
    const int cursor = roster.cursor[party];
    decision.to_slot = roster.members[party][cursor];
    roster.cursor[party] = (cursor + 1) % member_count;
    decision.action = PartyReassignAction::kReassign;
    return decision;
}

std::size_t run_assign_party_player_slots_004c3840(bool skip_marked_candidates,
                                                   AssignPartyPlayerSlotsHost& host) {
    const std::size_t slot_count = host.scene_slot_count();
    std::vector<MissionSlotBinding> slots;
    slots.reserve(slot_count);
    for (std::size_t index = 0; index < slot_count; ++index) {
        slots.push_back(host.slot(index));
    }
    MissionPartyRoster roster = build_mission_party_roster(slots.data(), slots.size());

    std::size_t reassigned = 0;
    const std::size_t candidates = host.candidate_count();
    for (std::size_t index = 0; index < candidates; ++index) {
        // 004C38ED..004C392B. The marker skip only exists when the argument is
        // set, and IsKindOf(1Ch) or IsKindOf(9) puts the candidate back in.
        if (skip_marked_candidates && host.candidate_marker_matches(index) &&
            !host.candidate_is_kind(index, kPartyReassignExemptKindA) &&
            !host.candidate_is_kind(index, kPartyReassignExemptKindB)) {
            continue;
        }
        const PartyOwnerCandidate candidate = host.candidate(index);
        const PartyReassignDecision decision =
            decide_party_reassignment(candidate, slots.data(), slots.size(), roster);
        switch (decision.action) {
            case PartyReassignAction::kSkip:
                break;
            case PartyReassignAction::kUnbindOwner:
                host.unbind_candidate_owner(index, kUnboundOwnerTokenA, kUnboundOwnerTokenB);
                break;
            case PartyReassignAction::kReassign:
                host.log_reassignment(kPartyReassignLogFormat, host.candidate_name(index),
                                      decision.party, decision.from_slot, decision.to_slot);
                host.route_owner_change(index, decision.to_slot);
                reassigned += 1;
                break;
        }
    }
    return reassigned;
}

// ---------------------------------------------------------------------------
// Step 2. the 004DFC13 branch
// ---------------------------------------------------------------------------

SessionSlotHeader reset_session_slot_header() noexcept {
    SessionSlotHeader header{};
    header.ready_0e = false;
    header.peer_id_10 = kSessionSlotUnassignedPeer;
    header.flag_18 = false;
    return header;
}

bool run_reset_network_slots_004dfc13(std::int32_t session_mode, NetworkSlotResetHost& host) {
    if (session_mode == 0) {
        // 004DFC19 JZ 004DFD16: a local session takes the participant-table arm
        // and none of the network reset below.
        host.reset_single_player_slots_004bb160();
        return false;
    }
    host.clear_string_tree_00e18a60();
    host.clear_record_tree_00e18a6c();
    host.clear_side_balance_latch_00e188bd();

    const SessionSlotHeader header = reset_session_slot_header();
    for (std::size_t index = 0; index < kSessionSlotCount; ++index) {
        host.write_slot_header(index, header);
    }

    const int local_slot = host.local_slot_index();
    const int party = host.slot_party(local_slot);
    const int record = host.select_menu_record_005d7070();
    // 004DFCF5 JC: the index must be strictly below the vector's element count,
    // and the native code traps through 00BF6713 when it is not.
    if (record >= 0 && static_cast<std::size_t>(record) < host.menu_record_count()) {
        host.apply_menu_record_00626930(record, party);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Step 4. the 004E0754 block
// ---------------------------------------------------------------------------

void run_reset_avoid_zone_state_004e0754(AvoidZoneResetHost& host) {
    host.clear_avoid_zone_counter_648h();
    host.clear_avoid_zone_clock_64ch();
    host.clear_scene_tree_5c8h();
    host.rebuild_avoid_zones_00424d00();
}

// ---------------------------------------------------------------------------
// Step 5. 004D30F0 and its teardown reader 004D32A0
// ---------------------------------------------------------------------------

namespace {

bool snapshot_contains(const std::vector<std::string>& snapshot, const std::string& name) {
    return std::find(snapshot.begin(), snapshot.end(), name) != snapshot.end();
}

}  // namespace

std::vector<std::string> scripted_name_snapshot(const LuaGlobalEntry* globals,
                                                std::size_t count) {
    std::vector<std::string> names;
    if (globals == nullptr) {
        return names;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const LuaGlobalEntry& entry = globals[index];
        // 004D31A4 00B66200 is lua_type(value) == LUA_TFUNCTION; a non-function
        // global never reaches the insert at 004D320B.
        if (!entry.is_function) {
            continue;
        }
        // The container is a set, so a repeated name is stored once.
        if (snapshot_contains(names, entry.name)) {
            continue;
        }
        names.push_back(entry.name);
    }
    return names;
}

std::vector<std::string> scripted_globals_to_nil(const LuaGlobalEntry* globals,
                                                 std::size_t count,
                                                 const std::vector<std::string>& snapshot) {
    std::vector<std::string> statements;
    if (globals == nullptr || snapshot.empty()) {
        // 004D32D2: an empty set ends 004D32A0 before the walk.
        return statements;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const LuaGlobalEntry& entry = globals[index];
        if (!entry.is_function) {
            continue;
        }
        // 004D33DB: the iterator whose node equals the tree head is end(), so a name
        // the snapshot does not carry is the one that gets nilled.
        if (snapshot_contains(snapshot, entry.name)) {
            continue;
        }
        statements.push_back(entry.name + kLuaNilAssignmentSuffix);
    }
    return statements;
}

std::size_t run_rebuild_scripted_name_list_004d30f0(ScriptedNameListHost& host) {
    host.clear_scripted_name_set();
    std::vector<LuaGlobalEntry> globals;
    const std::size_t count = host.lua_global_count();
    globals.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        globals.push_back(host.lua_global(index));
    }
    const std::vector<std::string> names = scripted_name_snapshot(globals.data(), globals.size());
    for (const std::string& name : names) {
        host.insert_scripted_name(name);
    }
    return names.size();
}

// ---------------------------------------------------------------------------
// Step 7. the 004C9CCD load arm
// ---------------------------------------------------------------------------

bool run_apply_in_game_interface_load_arm_004c9ccd(std::int32_t session_mode,
                                                   bool suppress_flag_218c,
                                                   LoadingElementHost& host) {
    if (!host.loading_element_present()) {
        // 004C9CE9: a null allocation stores null and skips the init, but the
        // active byte is written unconditionally at 004C9D1F.
        if (host.create_loading_element_00636d90()) {
            host.loading_element_init_00636f30();
        }
    }
    host.set_loading_element_active();
    // 004C9D23 / 004C9D2F, the two exits to the epilogue at 004C9EA3.
    if (session_mode == 0 || suppress_flag_218c) {
        return false;
    }
    return true;
}

}  // namespace bsp
