// Packet cc2_scoring_bodies. See include/bsp/scoring_bodies.hpp and docs/SCORING_BODIES.md.
// Ghidra was read-only for this packet; every name is a hypothesis.

#include "bsp/scoring_bodies.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"

#include <cstring>
#if !defined(_MSC_VER)
#include <strings.h>
#endif

namespace bsp {

// scripts/datatables/globals.lua, Globals.Difficulty.ScoreMultipliers = { 1/4, 1/2, 1, }.
// 0087D7B0 push_backs these in index order into the vector at GlobalConfig+2Ch.
const float kScoringDifficultyMultipliers[kScoringMultiplierVectorCount] = {0.25f, 0.5f, 1.0f};

// 00803510, __fastcall(ECX, EDX), byte result. The three branches in listing order.
int scoring_relative_party_00803510(int subject_side, int reference_side) noexcept
{
    if (subject_side == kScoringPartyNeutral) {
        // `return -(uint)(reference != 2) & 2`: NEUTRAL unless both sides are neutral.
        return reference_side != kScoringPartyNeutral ? kScoringPartyNeutral : kScoringPartyOwn;
    }
    if (subject_side == reference_side) {
        return kScoringPartyOwn;
    }
    // `return (reference == 2) + 1`: NEUTRAL against a neutral reference, else ENEMY.
    return reference_side == kScoringPartyNeutral ? kScoringPartyNeutral : kScoringPartyEnemy;
}

ScoringKillCredit scoring_kill_credit_0091bda0(const ScoringKillAttribution& in) noexcept
{
    ScoringKillCredit credit{};
    // 0091BF86 `CMP EAX,0x7` then 0091BF97 `JA`: an unsigned test, so a negative slot is
    // above 7 as well and records nothing.
    if (in.credited_slot < 0 || in.credited_slot > 7) {
        return credit;
    }
    credit.recorded = true;
    credit.record_slot = in.credited_slot;
    // 0091BFA9 `CMP EAX,EDI` then 0091BFB9 `JNZ`: equal falls through to the +B4h branch.
    credit.player_tree = in.credited_slot == in.originating_slot;
    credit.key.party = scoring_relative_party_00803510(in.attacker_side, in.victim_side);
    credit.key.attacker_class = in.attacker_class;
    credit.key.victim_class = in.victim_class;
    return credit;
}

void scoring_record_kill_0091bda0(ScoringBodiesHost& host, const ScoringKillAttribution& in)
{
    if (host.session_mode() == 2) {
        return;
    }
    const ScoringKillCredit credit = scoring_kill_credit_0091bda0(in);
    if (!credit.recorded) {
        return;
    }
    int& leaf = host.kill_tree_leaf(credit.record_slot,
                                    scoring_kill_tree_offset(credit.player_tree), credit.key);
    leaf += 1;
}

int scoring_sum_shot_down_008bc9b0(const MissionEnumCounterTriples& kills, int party) noexcept
{
    const auto party_it = kills.find(party);
    if (party_it == kills.end()) {
        return 0;
    }
    int total = 0;
    for (const auto& attacker : party_it->second) {
        for (const auto& victim : attacker.second) {
            if (scoring_is_shot_down_class(victim.first)) {
                total += victim.second;
            }
        }
    }
    return total;
}

int scoring_unit_type_shot_down_008d0140(const UnlockCounterMap& losses, const std::string& name)
{
    const auto it = losses.find(name);
    return it == losses.end() ? 0 : it->second;
}

void grant_bonus_value_007fc9f0(ScoringBonusProfile profile, const std::string& key,
    const std::string& text_a, const std::string& text_b, int value)
{
    // bsp.py show 007FC9F0 --asm: 007FCA61..007FCA97 compares lengths first,
    // treats two empty strings as equal, otherwise calls __stricmp. The former
    // record-only helper omitted this body's profile writes and also handled
    // the distinct 007FCBC0 text form. Keep one complete body per address.
    for (const std::string& granted : profile.granted_ac) {
        if (granted.size() != key.size()) continue;
        if (granted.empty()) return;
#if defined(_MSC_VER)
        if (_stricmp(granted.c_str(), key.c_str()) == 0) return;
#else
        if (strcasecmp(granted.c_str(), key.c_str()) == 0) return;
#endif
    }
    // 007FCAB9 appends the key BEFORE constructing/appending the record.
    // 007FCABE supplies the integer; 007FCACC..007FCAF8 produces the empty
    // third string; 007FCB7E calls 007F8B40 and 007FCB96 calls 007FC940.
    profile.granted_ac.push_back(key);
    profile.records_bc.push_back(cc13_seg49_64_78::construct_profile_bonus_record_007f8b40(
        text_a, text_b, "", value));
}

ScoringClearedSlotState scoring_clear_player_score_008bc540(int difficulty_before,
                                                            int used_slot_before) noexcept
{
    ScoringClearedSlotState state{};
    state.difficulty = difficulty_before;
    state.used_slot = used_slot_before;
    state.cleared_flag = true;
    return state;
}

void scoring_binding_clear_player_score_008bc540(ScoringBodiesHost& host, int slot)
{
    const int difficulty = host.slot_difficulty(slot);
    const int used_slot = host.slot_used_slot(slot);
    host.clear_slot_record(slot);
    const ScoringClearedSlotState state =
        scoring_clear_player_score_008bc540(difficulty, used_slot);
    host.set_slot_difficulty(slot, state.difficulty);
    host.set_slot_used_slot(slot, state.used_slot);
    host.set_slot_cleared_flag(slot, state.cleared_flag);
}

void scoring_binding_clear_all_missions_score_008d2d60(ScoringBodiesHost& host)
{
    host.clear_all_mission_records();
    host.reset_all_player_records();
}

void scoring_binding_set_condition_message_008bbe00(ScoringBodiesHost& host, int slot,
                                                    bool is_local_target, const std::string& text)
{
    (void)slot;
    if (!is_local_target) {
        return; // the remote branch sends the session message; not this packet's contract
    }
    host.set_condition_message(host.commit_slot(), text);
}

void scoring_binding_set_victory_message_008bc0c0(ScoringBodiesHost& host, int slot,
                                                  bool is_local_target, const std::string& text)
{
    (void)slot;
    if (!is_local_target) {
        return;
    }
    host.set_victory_message(host.commit_slot(), text);
}

void scoring_binding_grant_bonus_008bb770(ScoringBonusProfile profile,
                                          const ScoringGrantBonusArguments& args)
{
    // bsp.py show 008BB770: the Lua argument-3 string test selects the actual
    // profile body, 007FCBC0 at 008BB982 or 007FC9F0 at 008BBB34. Dedupe and
    // appends belong to those callees; do not run a second copy in this binding.
    if (args.third_is_string) {
        cc13_seg49_64_78::grant_bonus_text_007fcbc0(profile.granted_ac, profile.records_bc,
            args.key, args.text_a, args.text_b, args.text_c);
    } else {
        grant_bonus_value_007fc9f0(profile, args.key, args.text_a, args.text_b, args.value);
    }
}

} // namespace bsp
