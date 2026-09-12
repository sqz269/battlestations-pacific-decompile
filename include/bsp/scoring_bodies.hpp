// The scoring routines that write outside the keyed maps: the bonus grant, the two clears,
// the two mission-message setters, the kill trees and the difficulty multiplier vector.
//
// Packet cc2_scoring_bodies, worktree agent/cc2-scoring-bodies. Ghidra was read-only for this
// packet. Every name here is a hypothesis, not a recovered symbol.
//
// docs/SCORING_BODIES.md carries the evidence. The 284h record, its offsets and the persisted
// containers come from docs/MISSION_PROGRESS_ARCHIVE.md and include/bsp/mission_progress.hpp;
// the manager arithmetic from include/bsp/mission_result.hpp; the keyed maps from
// include/bsp/scoring_bindings.hpp. All three are reused, not redone.
#pragma once

#include <cstddef>
#include <string>

#include "bsp/mission_progress.hpp"
#include "bsp/mission_result.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Record offsets this packet establishes
// ---------------------------------------------------------------------------
// Record-relative, like scoring_bindings.hpp's map offsets. The manager expression is
// `manager + slot*284h + offset + 4`; mission_scoring_record_offset holds the +4h base.
// Evidence is the writer in each case, never a consumer (checklist rule 4).

// The two three-level kill trees. 0091BFF0 `LEA ECX,[EBP+0xb4]` and 0091C02D
// `LEA ECX,[EBP+0xc0]`, EBP being the record the writer computed at 0091BFAB.
inline constexpr std::size_t kScoringPlayerKillsOffset = 0xb4;
inline constexpr std::size_t kScoringPartyKillsOffset = 0xc0;

// The two per-slot fields Scoring_ClearPlayerScore saves across the record clear: 008BC540
// reads `manager + slot*284h + 0Ch` and `+254h` before 008BC6B4 and writes both back after.
// 0091C560 is the independent producer of both: it seeds `+254h` with the slot index and
// `+0Ch` with the difficulty for all eight slots at world construction.
inline constexpr std::size_t kScoringDifficultyOffset = 0x08;
inline constexpr std::size_t kScoringUsedSlotOffset = 0x250;

// The two runtime-only NativeStrings the mission-message setters write. 0090BDC0
// `LEA ... [param+254h]` with the text pointer at +258h; 0090BE50 the same at +25Ch/+260h.
// Neither appears in the archive's field table, so neither is persisted.
inline constexpr std::size_t kScoringConditionMessageOffset = 0x254;
inline constexpr std::size_t kScoringVictoryMessageOffset = 0x25c;

// The per-slot byte array Scoring_ClearPlayerScore stamps after clearing a record. 008BC540
// writes `manager + 14B0h + slot` = 1; 0091C560 clears the same byte for all eight slots.
// No reader of the array was found; see the doc's open questions.
inline constexpr std::size_t kScoringSlotClearedFlagBase = 0x14b0;

// ---------------------------------------------------------------------------
// The difficulty multiplier vector at GlobalConfig+2Ch
// ---------------------------------------------------------------------------
// 0087D7B0 reads `Globals.Difficulty.ScoreMultipliers` and push_backs one float per index
// into the vector at GlobalConfig+2Ch (0087DC08 `LEA EBP,[ESI+0x2c]`; _Myfirst +30h read at
// 0087DC03, _Mylast +34h, _Myend +38h). The name string is at 00D0E50C. The installed
// scripts/datatables/globals.lua line 11 has `{ 1/4, 1/2, 1, }`, so the vector 009103F0 and
// 00910480 bounds-check has length three.
inline constexpr std::size_t kScoringMultiplierVectorCount = 3;
extern const float kScoringDifficultyMultipliers[kScoringMultiplierVectorCount];

// The bounds check at 00910436: `CMP EDI,EAX; JC`, else 00BF6713. An out-of-range difficulty
// is a hard failure in the native build, not a clamp.
constexpr bool scoring_multiplier_index_in_range(int difficulty_index,
                                                 std::size_t vector_length) noexcept
{
    return difficulty_index >= 0 && static_cast<std::size_t>(difficulty_index) < vector_length;
}

// ---------------------------------------------------------------------------
// The relative-party rule
// ---------------------------------------------------------------------------
// 00803510, __fastcall(ECX = subject side, EDX = reference side), returns a byte. The result
// is a party index in the archive's OWN/ENEMY/NEUTRAL/UNKNOWN space (0..3), and it is the
// level-1 key of both kill trees.
inline constexpr int kScoringPartyOwn = 0;
inline constexpr int kScoringPartyEnemy = 1;
inline constexpr int kScoringPartyNeutral = 2;

int scoring_relative_party_00803510(int subject_side, int reference_side) noexcept;

// ---------------------------------------------------------------------------
// The kill trees
// ---------------------------------------------------------------------------
// `MissionEnumCounterTriples` in mission_progress.hpp already models the shape
// `party -> unit class -> unit class -> int`. This packet settles which key is which:
// level 1 is the relative party, level 2 the attacker's unit class, level 3 the victim's.

struct ScoringKillKey {
    int party{0};          // 00803510(victim+2D0h, victim+54h), pushed last at 0091BFEF
    int attacker_class{0}; // victim+2CCh, the attribution field 0077CE60 wrote
    int victim_class{0};   // the victim's own class, vtable[0] on victim+170h
};

// What 0091BDA0 does with one destroyed unit's attribution block.
struct ScoringKillAttribution {
    int credited_slot{0};   // victim+2D8h, `MOV EAX,[ESI+0x2d8]` at 0091BF80
    int originating_slot{0};// victim+2DCh, `MOV EDI,[ESI+0x2dc]` at 0091BF89
    int attacker_class{0};  // victim+2CCh
    int attacker_side{0};   // victim+2D0h
    int victim_side{0};     // victim+54h
    int victim_class{0};    // vtable[0] on victim+170h
};

struct ScoringKillCredit {
    bool recorded{false};   // false when the credited slot is above 7 (0091BF97 JA)
    int record_slot{0};     // always the credited slot
    bool player_tree{false};// +B4h when the two slots match, +C0h otherwise (0091BFB9 JNZ)
    ScoringKillKey key{};
};

// 0091BDA0's decision, with no host: which record, which tree, which three keys.
ScoringKillCredit scoring_kill_credit_0091bda0(const ScoringKillAttribution& in) noexcept;

// The record offset the credit names, for callers that index the native record directly.
constexpr std::size_t scoring_kill_tree_offset(bool player_tree) noexcept
{
    return player_tree ? kScoringPlayerKillsOffset : kScoringPartyKillsOffset;
}

// Scoring_GetPlayerShotDown (008BC9B0) sums every leaf whose level-3 key is one of the six
// aircraft classes, over every level-2 key, under level-1 key 1. The six compares test 7, 9,
// 8, 0Ah, 0Bh and 0Ch in that decompiled order and all fall into one accumulate at
// LAB_008BCCBF, `local += leaf`.
constexpr bool scoring_is_shot_down_class(int unit_class) noexcept
{
    return unit_class >= 7 && unit_class <= 12;
}

// The sum itself, over the tree the caller supplies. Level 2 is not filtered.
int scoring_sum_shot_down_008bc9b0(const MissionEnumCounterTriples& kills, int party) noexcept;

// Scoring_GetUnitTypeShotDown (008D0140) does not read a kill tree at all: it copies one of
// the two `name -> int` loss maps of the commit-slot record and looks one name up in it.
// Argument 0 true selects +1ECh (allied losses), false +1F8h (Japanese); both branches join
// at LAB_008D02F0. Argument 1 is the name. A miss returns 0 (004C8B80 find, no insert).
inline constexpr std::size_t kScoringAlliedLossesOffset = 0x1ec;
inline constexpr std::size_t kScoringJapaneseLossesOffset = 0x1f8;

constexpr std::size_t scoring_losses_offset(bool allied) noexcept
{
    return allied ? kScoringAlliedLossesOffset : kScoringJapaneseLossesOffset;
}

int scoring_unit_type_shot_down_008d0140(const UnlockCounterMap& losses, const std::string& name);

// ---------------------------------------------------------------------------
// The bonus list on the player profile
// ---------------------------------------------------------------------------
// Scoring_GrantBonus (008BB770) does not touch the scoring manager. Both of its callees take
// ECX = `[00e188a8]+650h`, the player profile (008BBB07 `MOV ESI,[00e188a8]` then
// 008BBB17 `ADD ESI,0x650`; 008BB964/008BB974 the same for the other branch).
inline constexpr std::size_t kScoringProfileOffsetInGame = 0x650;

// The dedupe vector of granted names, profile+ACh (007FCA18 `LEA EDI,[ECX+0xac]`; _Myfirst
// +B0h, _Mylast +B4h, _Myend +B8h; 8-byte NativeString elements, stepped `+8` at 007FCAA6).
inline constexpr std::size_t kScoringGrantedNamesVectorOffset = 0xac;
// The record vector, profile+BCh (007FCB88 `ADD ECX,0xbc`), stride 1Ch (007FC940's divisor).
inline constexpr std::size_t kScoringBonusVectorOffset = 0xbc;
inline constexpr std::size_t kScoringBonusRecordStride = 0x1c;

// The 1Ch record 007F8B40 builds: three NativeStrings then an int.
struct ScoringBonusRecord {
    std::string text_00;  // +00h/+04h
    std::string text_08;  // +08h/+0Ch
    std::string text_10;  // +10h/+14h
    int value_18{0};      // +18h
};

// Both callees are __thiscall(profile, NativeString* a, NativeString* b, NativeString* c,
// <int|NativeString*> d) with RET 10h (007FCBBA, 007FCD8A), so four stack arguments.
// Argument a is the dedupe key only; it never reaches the record.
struct ScoringGrantBonusArguments {
    std::string key;        // Lua argument 0
    std::string text_a;     // Lua argument 1
    std::string text_b;     // Lua argument 2
    std::string text_c;     // Lua argument 3 when it is a string
    int value{0};           // Lua argument 3 when it is not
    bool third_is_string{false}; // 008BB770's BSP_LuaObject_IsString on argument 3
};

// 007FC9F0 (integer form) and 007FCBC0 (four-string form) differ only in the record's third
// string and its int: the string form stores argument 3 and a zero value (007FCC92 PUSH EBX
// with EBX zeroed at 007FCC90), the integer form an empty string and argument 3.
ScoringBonusRecord scoring_bonus_record_007fc9f0(const ScoringGrantBonusArguments& args);

// ---------------------------------------------------------------------------
// The two clears
// ---------------------------------------------------------------------------
// 00915760 zeroes every scalar of one 284h record and empties every owned container; it does
// not touch +08h or +250h. Scoring_ClearPlayerScore (008BC540) saves those two itself, calls
// the clear, writes them back and stamps the per-slot flag.
struct ScoringClearedSlotState {
    int difficulty{0};
    int used_slot{0};
    bool cleared_flag{false};
};

ScoringClearedSlotState scoring_clear_player_score_008bc540(int difficulty_before,
                                                            int used_slot_before) noexcept;

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
// One virtual per native call site the sequences below reach. No defaults.
struct ScoringBodiesHost {
    virtual ~ScoringBodiesHost() = default;

    // [00e188a8]+1FE4h. 0091BDA0's first test returns at once when this is 2.
    virtual int session_mode() = 0;

    // 00432650 then the bounds-checked vector at GlobalConfig+2Ch. Length and contents are
    // kScoringDifficultyMultipliers for the installed globals.lua; the host still supplies
    // them because the loader runs before this code and can be re-authored.
    virtual float difficulty_score_multiplier(int difficulty_index) = 0;

    // 0062C170 then 0062BB60 then 00625900, the three `operator[]` of the nested int maps,
    // chained `MOV ECX,EAX` at 0091C038 and 0091C03F. Inserts on a miss at every level, so
    // the returned reference is always valid. `ADD dword ptr [EAX],0x1` at 0091C046.
    virtual int& kill_tree_leaf(int slot, std::size_t tree_offset, const ScoringKillKey& key) = 0;

    // 00915760 on `manager + slot*284h + 4`, called at 008BC6B4 with that record in ECX.
    virtual void clear_slot_record(int slot) = 0;
    virtual int slot_difficulty(int slot) = 0;
    virtual int slot_used_slot(int slot) = 0;
    virtual void set_slot_difficulty(int slot, int difficulty) = 0;
    virtual void set_slot_used_slot(int slot, int used_slot) = 0;
    virtual void set_slot_cleared_flag(int slot, bool cleared) = 0;

    // 004B4750, `manager + [manager+1424h]*284h + 4`: the commit-slot record every message
    // setter and 008D0140 use. Returned as a slot so the host stays pointer-free.
    virtual int commit_slot() = 0;

    // 0090BDA0 and 0090BE30, NativeString assign into the commit record at +254h and +25Ch.
    virtual void set_condition_message(int slot, const std::string& text) = 0;
    virtual void set_victory_message(int slot, const std::string& text) = 0;

    // 007FC9F0/007FCBC0's two containers on the profile at [00e188a8]+650h. The first
    // answers the dedupe walk at 007FCA32..007FCAA9, the second is 00450540/007FC940.
    virtual bool profile_has_granted_bonus(const std::string& key) = 0;
    virtual void profile_record_granted_bonus(const std::string& key) = 0;
    virtual void profile_append_bonus(const ScoringBonusRecord& record) = 0;

    // 007FD510 over the root of the map at MissionProgress+0h, then the three self-pointers
    // and the size (008D2E60..008D2E7x), then 00916980 for the eight live records.
    virtual void clear_all_mission_records() = 0;
    virtual void reset_all_player_records() = 0;
};

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------
// 0091BDA0's scoring tail: the slot test, the tree choice and the one increment. The rest of
// 0091BDA0 (the per-slot string counters, the award tokens) is outside this packet; see the
// doc's coverage row.
void scoring_record_kill_0091bda0(ScoringBodiesHost& host, const ScoringKillAttribution& in);

// 008BC540 Scoring_ClearPlayerScore.
void scoring_binding_clear_player_score_008bc540(ScoringBodiesHost& host, int slot);

// 008D2D60 Scoring_ClearAllMissionsScore. No arguments.
void scoring_binding_clear_all_missions_score_008d2d60(ScoringBodiesHost& host);

// 008BBE00 and 008BC0C0. The local branch writes the commit-slot record whatever slot the
// script names; the remote branch sends the session message instead. `is_local_target` is
// the `[game+18CCh + slot*4] + 0Bh == 0` test that guards 008BC015..008BC032.
void scoring_binding_set_condition_message_008bbe00(ScoringBodiesHost& host, int slot,
                                                    bool is_local_target, const std::string& text);
void scoring_binding_set_victory_message_008bc0c0(ScoringBodiesHost& host, int slot,
                                                  bool is_local_target, const std::string& text);

// 008BB770 Scoring_GrantBonus.
void scoring_binding_grant_bonus_008bb770(ScoringBodiesHost& host,
                                          const ScoringGrantBonusArguments& args);

} // namespace bsp
