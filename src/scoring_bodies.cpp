// Packet cc2_scoring_bodies. See include/bsp/scoring_bodies.hpp and docs/SCORING_BODIES.md.
// Ghidra was read-only for this packet; every name is a hypothesis.

#include "bsp/scoring_bodies.hpp"

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

ScoringBonusRecord scoring_bonus_record_007fc9f0(const ScoringGrantBonusArguments& args)
{
    ScoringBonusRecord record{};
    // Both callees copy stack argument 2 then argument 3 into the record's first two
    // strings; only the third slot and the int differ between them.
    record.text_00 = args.text_a;
    record.text_08 = args.text_b;
    if (args.third_is_string) {
        record.text_10 = args.text_c;
        record.value_18 = 0;
    } else {
        record.text_10.clear();
        record.value_18 = args.value;
    }
    return record;
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

void scoring_binding_grant_bonus_008bb770(ScoringBodiesHost& host,
                                          const ScoringGrantBonusArguments& args)
{
    // 007FCA32..007FCAA9 walks the granted-name vector and 007FCAAB skips everything when a
    // case-insensitive match was found. The comparison is the native NativeString rule:
    // equal lengths, then __stricmp; an empty stored name matches only an empty key.
    if (host.profile_has_granted_bonus(args.key)) {
        return;
    }
    host.profile_record_granted_bonus(args.key);
    host.profile_append_bonus(scoring_bonus_record_007fc9f0(args));
}

} // namespace bsp
