// The keyed `Scoring_*` Lua bindings and the mission-scoring manager methods behind them.
//
// Packet cc2_mission_objectives, worktree agent/cc2-mission-objectives. Ghidra was read-only
// for this packet. Every name here is a hypothesis, not a recovered symbol.
//
// docs/SCORING_BINDING_TABLE.md carries the evidence. docs/MISSION_RESULT_DECISION.md
// established the 284h record, the manager at [00e188a8]+21A0h holding eight records at +4h,
// and the persisted shape in include/bsp/mission_progress.hpp; include/bsp/mission_result.hpp
// already declares the stride and the slot arithmetic and both are reused here.
//
// This header adds what that packet left open: the record offset every keyed setter writes,
// the two prologue variants that pick the player slot, and the value rule of each accessor.
#pragma once

#include <cstddef>
#include <string>

#include "bsp/lua_binding_core.hpp"
#include "bsp/mission_result.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The keyed maps of the 284h record
// ---------------------------------------------------------------------------
// Record-relative offsets, in MissionScoreMap order. The manager methods index as
// `manager + slot*284h + (offset + 4)` because the record array starts at manager+4h; the
// LEA of each accessor is in the doc's offset table. These are evidence labels for the
// native record, not offsets into MissionScoreRecord, which is a C++ projection.
inline constexpr std::size_t kScoringMapOffsetMission = 0x18;       // 00910461, 009104a1
inline constexpr std::size_t kScoringMapOffsetAction = 0x24;        // 0090f12e, 0090f15e
inline constexpr std::size_t kScoringMapOffsetShip = 0x30;          // 0090f18e, 0090f1be
inline constexpr std::size_t kScoringMapOffsetPlane = 0x3c;         // 0090f1ee, 0090f21e
inline constexpr std::size_t kScoringMapOffsetCommand = 0x48;       // 0090f24e, 0090f27e
inline constexpr std::size_t kScoringMapOffsetMissionMedals = 0x54; // 0090f0ef
inline constexpr std::size_t kScoringMapOffsetActionMedals = 0x60;  // 0090f29f
// The unkeyed total the getter at 008b8ff0 reads inline (008b912a).
inline constexpr std::size_t kScoringTotalOffset = 0x1e0;

// Record offset of one keyed map, for the doc's table and for asserting the projection.
constexpr std::size_t scoring_map_record_offset(MissionScoreMap map) noexcept
{
    switch (map) {
    case MissionScoreMap::Mission: return kScoringMapOffsetMission;
    case MissionScoreMap::Action: return kScoringMapOffsetAction;
    case MissionScoreMap::Ship: return kScoringMapOffsetShip;
    case MissionScoreMap::Plane: return kScoringMapOffsetPlane;
    case MissionScoreMap::Command: return kScoringMapOffsetCommand;
    case MissionScoreMap::MissionMedals: return kScoringMapOffsetMissionMedals;
    case MissionScoreMap::ActionMedals: return kScoringMapOffsetActionMedals;
    }
    return kScoringMapOffsetMission;
}

// The address the manager methods compute, `this + slot*284h + offset + 4`. Reusing
// mission_scoring_record_offset keeps the +4h base in one place.
constexpr std::size_t scoring_manager_field_offset(int slot, std::size_t record_offset) noexcept
{
    return mission_scoring_record_offset(static_cast<std::size_t>(slot)) + record_offset;
}

// The difficulty index 009103f0/00910480 use to pick the multiplier. Multiplayer forces 2
// (00910410 MOV EDI,0x2); single player takes game+6ACh (00910417).
inline constexpr int kScoringMultiplayerDifficultyIndex = 2;

// ---------------------------------------------------------------------------
// The value rules
// ---------------------------------------------------------------------------
// Pure functions with explicit inputs. The float multiplier comes from the host because
// only a bounds-checked GlobalConfig vector supplies it; the truncation is __ftol's, which
// rounds toward zero.

// 009103f0: FILD value; FMUL multiplier; __ftol; store.
int scoring_scaled_set_009103f0(int value, float multiplier) noexcept;

// 00910480: FLD multiplier; FIMUL value; FIADD current; __ftol; store. The current value is
// added before the single truncation, so this is not `current + scaled(delta)`.
int scoring_scaled_add_00910480(int current, int delta, float multiplier) noexcept;

// 0090f110/0090f170/0090f1d0/0090f230: MOV dword ptr [eax], ecx.
constexpr int scoring_plain_set(int value) noexcept { return value; }

// 0090f140/0090f1a0/0090f200/0090f260: ADD dword ptr [eax], ecx.
constexpr int scoring_plain_add(int current, int delta) noexcept { return current + delta; }

// 0090f0e0/0090f290: MOVZX ECX, byte ptr [esp+0Ch], so only the low byte of the Lua value
// reaches the map and a boolean stores 0 or 1.
constexpr int scoring_medal_value(bool granted) noexcept { return granted ? 1 : 0; }

// ---------------------------------------------------------------------------
// The binding prologue
// ---------------------------------------------------------------------------
// Which record slot a keyed binding writes and where its key argument starts.
struct ScoringSlotSelection {
    int slot{0};        // EDI at the accessor call
    int first_key_arg{0}; // ESI, the index of the key argument
};

// Variant A, 008b9273..008b92bc: no argument-count test. In single player the slot is the
// local player and the key is argument 0; in multiplayer argument 0 is the slot.
ScoringSlotSelection scoring_select_slot_variant_a(bool multiplayer, int local_player_slot,
                                                   int argument0_integer) noexcept;

// Variant B, 008b9b4e..008b9b8a: in single player a three-argument call has its slot
// argument read and discarded, yet still shifts the key to argument 1. Only the multiplayer
// branch does MOV EDI,EAX (008b9b4a).
ScoringSlotSelection scoring_select_slot_variant_b(bool multiplayer, int local_player_slot,
                                                   int argument0_integer,
                                                   int argument_count) noexcept;

// The count a variant B binding must see before it consumes an explicit slot (008b9b57).
inline constexpr int kScoringVariantBSlotArgumentCount = 3;

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
// One virtual per native call site the handlers reach. No defaults: nothing here stands in
// for unrecovered game behaviour.
struct ScoringBindingHost {
    virtual ~ScoringBindingHost() = default;

    // [00e188a8]+1FE4h, the flag every prologue tests (008b927d).
    virtual bool is_multiplayer() = 0;
    // [00e188a8]+18ECh, the local player slot (008b9273).
    virtual int local_player_slot() = 0;
    // [00e188a8]+6ACh, the campaign difficulty (00910417).
    virtual int campaign_difficulty() = 0;

    // 00432650 then the bounds-checked std::vector<float> at GlobalConfig+2Ch, indexed by the
    // difficulty (0091043f..00910449). The body of 00432650 was not read; the vector's
    // producer and length are unread, so the host supplies the value.
    virtual float difficulty_score_multiplier(int difficulty_index) = 0;

    // [[00e198c4]+B8h]+14h = 0, the front-end cache byte the eight score accessors clear
    // (009103fe). The two medal setters do not call this.
    virtual void invalidate_front_end_score_cache() = 0;

    // 005070c0, std::map<NativeString,int>::operator[] on the map at the given record offset
    // of the given slot. Inserts a zero-valued entry on a miss, so the reference is always
    // valid. Called at 00910465, 009104a5, 0090f0f3 and the rest of the setter family.
    virtual int& score_map_entry(int slot, std::size_t record_offset, const std::string& key) = 0;

    // 004c8b80, std::map::find on the same map. Returns 0 without inserting when the key is
    // absent (0090c7d6 XOR EAX,EAX). Body not read; named by its effect at the call site.
    virtual int score_map_find(int slot, std::size_t record_offset, const std::string& key) = 0;

    // [manager + slot*284h + 1E4h], the total 008b8ff0 reads inline.
    virtual int slot_total_score(int slot) = 0;
};

// ---------------------------------------------------------------------------
// The manager methods
// ---------------------------------------------------------------------------
// Each is the sequence of one native accessor over the host.
void scoring_set_slot_mission_score_009103f0(ScoringBindingHost& host, int slot,
                                             const std::string& key, int value);
void scoring_add_slot_mission_score_00910480(ScoringBindingHost& host, int slot,
                                             const std::string& key, int delta);
void scoring_set_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                  const std::string& key, int value);
void scoring_add_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                  const std::string& key, int delta);
void scoring_set_slot_medal(ScoringBindingHost& host, MissionScoreMap map, int slot,
                            const std::string& key, bool granted);
int scoring_get_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                 const std::string& key);

// ---------------------------------------------------------------------------
// The bindings
// ---------------------------------------------------------------------------
// Each returns the binding's own result count, as the native tail does through 00B66400.

// 008b9190 Scoring_SetMissionScore, 008b95c0 Scoring_AddMissionScore, 008b93c0
// Scoring_GetMissionScore. Variant A.
int lua_binding_scoring_set_mission_score_008b9190(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host);
int lua_binding_scoring_add_mission_score_008b95c0(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host);
int lua_binding_scoring_get_mission_score_008b93c0(LuaBindingArgumentReader& args,
                                                   LuaBindingResultWriter& results,
                                                   ScoringBindingHost& host);

// 008b8ff0 Scoring_GetTotalMissionScore. Variant A with no key argument.
int lua_binding_scoring_get_total_mission_score_008b8ff0(LuaBindingArgumentReader& args,
                                                          LuaBindingResultWriter& results,
                                                          ScoringBindingHost& host);

// 008b97f0 Scoring_SetMissionMedal and 008bb530 Scoring_SetActionMedal. Variant A plus the
// optional trailing boolean that defaults to true (008b9961).
int lua_binding_scoring_set_medal_008b97f0(LuaBindingArgumentReader& args,
                                            ScoringBindingHost& host, MissionScoreMap map);

// 008b9a30 / 008ba0f0 / 008ba7b0 / 008bae70 and their Add twins. Variant B.
int lua_binding_scoring_set_keyed_score_variant_b(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map);
int lua_binding_scoring_add_keyed_score_variant_b(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map);

// 008b9c90 / 008ba350 / 008baa10 / 008bb0d0. Variant A.
int lua_binding_scoring_get_keyed_score_variant_a(LuaBindingArgumentReader& args,
                                                   LuaBindingResultWriter& results,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map);

} // namespace bsp
