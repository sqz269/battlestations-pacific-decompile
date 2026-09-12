// docs/SCORING_BINDING_TABLE.md. Packet cc2_mission_objectives. Ghidra was read-only.
#include "bsp/scoring_bindings.hpp"

#include <cmath>

namespace bsp {
namespace {

// __ftol (00bf7420) truncates toward zero. std::trunc keeps that shape explicit; the native
// routine also saturates on overflow, which this projection does not model.
int ftol_00bf7420(double value) noexcept
{
    return static_cast<int>(std::trunc(value));
}

// The difficulty index both mission-score accessors compute: multiplayer forces 2 and single
// player reads game+6ACh (00910410..00910417).
int scoring_difficulty_index(ScoringBindingHost& host)
{
    return host.is_multiplayer() ? kScoringMultiplayerDifficultyIndex
                                 : host.campaign_difficulty();
}

} // namespace

int scoring_scaled_set_009103f0(int value, float multiplier) noexcept
{
    // FILD dword [value]; FMUL float [multiplier]; __ftol.
    return ftol_00bf7420(static_cast<double>(value) * static_cast<double>(multiplier));
}

int scoring_scaled_add_00910480(int current, int delta, float multiplier) noexcept
{
    // FLD float [multiplier]; FIMUL dword [delta]; FIADD dword [current]; __ftol. One
    // truncation over the whole expression, not two.
    const double scaled = static_cast<double>(multiplier) * static_cast<double>(delta);
    return ftol_00bf7420(scaled + static_cast<double>(current));
}

ScoringSlotSelection scoring_select_slot_variant_a(bool multiplayer, int local_player_slot,
                                                   int argument0_integer) noexcept
{
    ScoringSlotSelection selection{};
    selection.slot = local_player_slot;
    selection.first_key_arg = 0;
    if (multiplayer) {
        selection.slot = argument0_integer;
        selection.first_key_arg = 1;
    }
    return selection;
}

ScoringSlotSelection scoring_select_slot_variant_b(bool multiplayer, int local_player_slot,
                                                   int argument0_integer,
                                                   int argument_count) noexcept
{
    ScoringSlotSelection selection{};
    selection.slot = local_player_slot;
    selection.first_key_arg = 0;
    if (multiplayer) {
        selection.slot = argument0_integer;
        selection.first_key_arg = 1;
    } else if (argument_count == kScoringVariantBSlotArgumentCount) {
        // 008b9b75 reads the integer and 008b9b7a drops it: the slot stays the local player
        // but the key and value still shift by one.
        selection.first_key_arg = 1;
    }
    return selection;
}

void scoring_set_slot_mission_score_009103f0(ScoringBindingHost& host, int slot,
                                             const std::string& key, int value)
{
    host.invalidate_front_end_score_cache();
    const float multiplier = host.difficulty_score_multiplier(scoring_difficulty_index(host));
    int& entry = host.score_map_entry(slot, kScoringMapOffsetMission, key);
    entry = scoring_scaled_set_009103f0(value, multiplier);
}

void scoring_add_slot_mission_score_00910480(ScoringBindingHost& host, int slot,
                                             const std::string& key, int delta)
{
    host.invalidate_front_end_score_cache();
    // 00910480 indexes the map before it reads the multiplier; 009103f0 does it after. Both
    // orders reach the same entry because operator[] inserts a zero on a miss.
    int& entry = host.score_map_entry(slot, kScoringMapOffsetMission, key);
    const float multiplier = host.difficulty_score_multiplier(scoring_difficulty_index(host));
    entry = scoring_scaled_add_00910480(entry, delta, multiplier);
}

void scoring_set_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                  const std::string& key, int value)
{
    if (map == MissionScoreMap::Mission) {
        scoring_set_slot_mission_score_009103f0(host, slot, key, value);
        return;
    }
    host.invalidate_front_end_score_cache();
    int& entry = host.score_map_entry(slot, scoring_map_record_offset(map), key);
    entry = scoring_plain_set(value);
}

void scoring_add_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                  const std::string& key, int delta)
{
    if (map == MissionScoreMap::Mission) {
        scoring_add_slot_mission_score_00910480(host, slot, key, delta);
        return;
    }
    host.invalidate_front_end_score_cache();
    int& entry = host.score_map_entry(slot, scoring_map_record_offset(map), key);
    entry = scoring_plain_add(entry, delta);
}

void scoring_set_slot_medal(ScoringBindingHost& host, MissionScoreMap map, int slot,
                            const std::string& key, bool granted)
{
    // 0090f0e0 and 0090f290 do not clear the front-end cache byte; that is the only
    // structural difference between them and the eight score accessors.
    int& entry = host.score_map_entry(slot, scoring_map_record_offset(map), key);
    entry = scoring_medal_value(granted);
}

int scoring_get_slot_keyed_score(ScoringBindingHost& host, MissionScoreMap map, int slot,
                                 const std::string& key)
{
    return host.score_map_find(slot, scoring_map_record_offset(map), key);
}

namespace {

// The tail every keyed binding shares: key at `first_key_arg`, value at the next index.
struct KeyedArguments {
    int slot{0};
    int first_key_arg{0};
    std::string key;
    int value{0};
};

KeyedArguments read_keyed_arguments(LuaBindingArgumentReader& args, ScoringBindingHost& host,
                                    bool variant_b, bool read_value)
{
    const bool multiplayer = host.is_multiplayer();
    const int local_slot = host.local_player_slot();
    // Argument 0 is only read when a branch actually consumes it; the native code reads it in
    // the multiplayer branch of both variants and in variant B's three-argument branch.
    const int count = args.count();
    const bool reads_argument0 =
        multiplayer || (variant_b && count == kScoringVariantBSlotArgumentCount);
    const int argument0 = reads_argument0 ? args.get_integer(0) : 0;

    const ScoringSlotSelection selection =
        variant_b ? scoring_select_slot_variant_b(multiplayer, local_slot, argument0, count)
                  : scoring_select_slot_variant_a(multiplayer, local_slot, argument0);

    KeyedArguments out{};
    out.slot = selection.slot;
    out.first_key_arg = selection.first_key_arg;
    out.key = args.get_string(selection.first_key_arg);
    if (read_value) {
        out.value = args.get_integer(selection.first_key_arg + 1);
    }
    return out;
}

} // namespace

int lua_binding_scoring_set_mission_score_008b9190(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host)
{
    const KeyedArguments read = read_keyed_arguments(args, host, false, true);
    scoring_set_slot_mission_score_009103f0(host, read.slot, read.key, read.value);
    return 0;
}

int lua_binding_scoring_add_mission_score_008b95c0(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host)
{
    const KeyedArguments read = read_keyed_arguments(args, host, false, true);
    scoring_add_slot_mission_score_00910480(host, read.slot, read.key, read.value);
    return 0;
}

int lua_binding_scoring_get_mission_score_008b93c0(LuaBindingArgumentReader& args,
                                                   LuaBindingResultWriter& results,
                                                   ScoringBindingHost& host)
{
    const KeyedArguments read = read_keyed_arguments(args, host, false, false);
    results.push_number(
        scoring_get_slot_keyed_score(host, MissionScoreMap::Mission, read.slot, read.key));
    return 1;
}

int lua_binding_scoring_get_total_mission_score_008b8ff0(LuaBindingArgumentReader& args,
                                                          LuaBindingResultWriter& results,
                                                          ScoringBindingHost& host)
{
    // 008b90d8 takes the local slot, 008b90e9 replaces it from argument 0 in multiplayer, and
    // 008b912a reads manager+slot*284h+1E4h with no key at all.
    const bool multiplayer = host.is_multiplayer();
    const int slot = multiplayer ? args.get_integer(0) : host.local_player_slot();
    results.push_number(host.slot_total_score(slot));
    return 1;
}

int lua_binding_scoring_set_medal_008b97f0(LuaBindingArgumentReader& args,
                                            ScoringBindingHost& host, MissionScoreMap map)
{
    const KeyedArguments read = read_keyed_arguments(args, host, false, false);
    // 008b9961 seeds the byte with 1 and 008b996b only overwrites it when the count exceeds
    // the key's index, so an omitted argument grants the medal.
    bool granted = true;
    const int optional_index = read.first_key_arg + 1;
    if (args.count() > optional_index) {
        granted = args.get_boolean(optional_index);
    }
    scoring_set_slot_medal(host, map, read.slot, read.key, granted);
    return 0;
}

int lua_binding_scoring_set_keyed_score_variant_b(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map)
{
    const KeyedArguments read = read_keyed_arguments(args, host, true, true);
    scoring_set_slot_keyed_score(host, map, read.slot, read.key, read.value);
    return 0;
}

int lua_binding_scoring_add_keyed_score_variant_b(LuaBindingArgumentReader& args,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map)
{
    const KeyedArguments read = read_keyed_arguments(args, host, true, true);
    scoring_add_slot_keyed_score(host, map, read.slot, read.key, read.value);
    return 0;
}

int lua_binding_scoring_get_keyed_score_variant_a(LuaBindingArgumentReader& args,
                                                   LuaBindingResultWriter& results,
                                                   ScoringBindingHost& host,
                                                   MissionScoreMap map)
{
    const KeyedArguments read = read_keyed_arguments(args, host, false, false);
    results.push_number(scoring_get_slot_keyed_score(host, map, read.slot, read.key));
    return 1;
}

} // namespace bsp
