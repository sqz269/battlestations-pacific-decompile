#include "bsp/mission_progress.hpp"
#include "bsp/profile_archive.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {

using Field = GuiLuaFieldType;

struct NamedOffset {
    const char* name;
    std::uint16_t native_offset;
};
// Literal spellings and native offsets from 00908A30, cross-checked against
// destination LEA/ADD instructions in 00916A00. These tables drive traversal.
constexpr std::array<NamedOffset, 7> score_maps{{
    {"mission", 0x18}, {"action", 0x24}, {"ship", 0x30}, {"plane", 0x3c},
    {"command", 0x48}, {"missionMedals", 0x54}, {"actionMedals", 0x60}}};
constexpr std::array<NamedOffset, 7> totals{{
    {"mission", 0x1c8}, {"action", 0x1cc}, {"ship", 0x1d0}, {"plane", 0x1d4},
    {"command", 0x1d8}, {"badge", 0x1dc}, {"total", 0x1e0}}};
constexpr std::array<NamedOffset, 2> losses{{
    {"allied_losses", 0x1ec}, {"japanese_losses", 0x1f8}}};
constexpr std::array<NamedOffset, 6> objectives{{
    {"allied_primary_objectives", 0x204}, {"allied_secondary_objectives", 0x210},
    {"allied_hidden_objectives", 0x21c}, {"japanese_primary_objectives", 0x228},
    {"japanese_secondary_objectives", 0x234}, {"japanese_hidden_objectives", 0x240}}};
constexpr std::array<NamedOffset, 4> checkpoints{{
    {"repairuses", 0x168}, {"formationuses", 0x16c},
    {"shipvsShip", 0x170}, {"islandCapture", 0x174}}};
constexpr const char* parties[]{"OWN", "ENEMY", "NEUTRAL", "UNKNOWN"}; // 00E0B080
constexpr const char* unit_classes[]{ // 00E0B590
    "mothership", "destroyer", "torpedoboat", "battleship", "cruiser", "cargo",
    "landingship", "levelbomber", "divebomber", "torpedobomber", "fighter",
    "reconplane", "kamikaze", "submarine", "landvehicle", "landfort", "airfield",
    "shipyard", "path", "other"};

bool has(ProfileArchiveReader& reader, const char* name) {
    return reader.has_key_00bd5eb0(gui_lua_key_by_name(name));
}
void enter(ProfileArchiveReader& reader, const char* name) {
    reader.enter_00bd8e20(gui_lua_key_by_name(name));
}
void read(ProfileArchiveReader& reader, const char* name, int& value) {
    reader.read_00bd6830(gui_lua_key_by_name(name), gui_lua_field(Field::Int, &value));
}
void read(ProfileArchiveReader& reader, const char* name, float& value) {
    reader.read_00bd6830(gui_lua_key_by_name(name), gui_lua_field(Field::Float, &value));
}
template <typename T>
void read_if_present(ProfileArchiveReader& reader, const char* name, T& value) {
    if (has(reader, name)) read(reader, name, value);
}
void read_default_zero(ProfileArchiveReader& reader, const char* name, int& value) {
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(name),
        gui_lua_field(Field::Int, &value), gui_lua_key_by_index(0));
}
void write(ProfileArchiveWriter& writer, const char* name, int value) {
    writer.write_field(gui_lua_key_by_name(name), SettingsValue::from_int(value));
}
void write(ProfileArchiveWriter& writer, const char* name, float value) {
    writer.write_field(gui_lua_key_by_name(name), SettingsValue::from_float(value));
}

const char* text_key(const GuiLuaVariant& key) {
    // Native code dereferences this as a C string without checking its tag.
    // Numeric/null keys are outside that valid domain; do not reinterpret their
    // payload as an address in the new C++ interface.
    if (key.tag != 0 || key.value.text == nullptr)
        throw std::invalid_argument("mission progress requires string map keys");
    return key.value.text;
}
template <typename F>
void each_key(ProfileArchiveReader& reader, F visit) {
    GuiLuaKeyArray keys;
    reader.enumerate_keys_00bd5f50(keys);
    for (int index = 0; index < keys.count; ++index) visit(keys.keys[index]);
}
void read_named_counters(ProfileArchiveReader& reader, UnlockCounterMap& values) {
    each_key(reader, [&](const GuiLuaVariant& key) {
        auto& value = values[text_key(key)];
        reader.read_00bd6830(key, gui_lua_field(Field::Int, &value));
    });
}
void write_named_counters(ProfileArchiveWriter& writer, const char* section,
    const UnlockCounterMap& values, bool omit_zero, bool omit_empty) {
    if (omit_empty && values.empty()) return;
    writer.begin_section(section);
    for (const auto& [key, value] : values)
        if (!omit_zero || value != 0) write(writer, key.c_str(), value);
    writer.end_section();
}

enum class EnumNames { Party, Unit };
int enum_index(std::string_view name, EnumNames kind) {
    // 00805990 compares 4 party strings then returns 4 on a miss. 0085BCE0
    // compares only unit classes 0..18 then returns 19, the valid 'other' slot.
    const int count = kind == EnumNames::Party ? 4 : 19;
    const auto* names = kind == EnumNames::Party ? parties : unit_classes;
    NativeStringCaseInsensitiveLess less;
    for (int i = 0; i < count; ++i)
        if (!less(name, names[i]) && !less(names[i], name)) return i;
    return count;
}
const char* enum_name(int index, EnumNames kind) {
    const int count = kind == EnumNames::Party ? 4 : 20;
    // Native writer indexes unchecked; no valid archive name exists outside
    // these tables. Preserve the integer on read and fail explicitly on write.
    if (index < 0 || index >= count)
        throw std::out_of_range("mission trace enum has no native archive name");
    return (kind == EnumNames::Party ? parties : unit_classes)[index];
}
void read_triples(ProfileArchiveReader& reader, const char* section,
    MissionEnumCounterTriples& values, bool damage) {
    if (!has(reader, section)) return;
    enter(reader, section);
    each_key(reader, [&](const GuiLuaVariant& outer) {
        reader.enter_00bd8e20(outer);
        each_key(reader, [&](const GuiLuaVariant& middle) {
            reader.enter_00bd8e20(middle);
            each_key(reader, [&](const GuiLuaVariant& inner) {
                // Native map operator[] calls happen only at the leaf.
                const int a = enum_index(text_key(outer), damage ? EnumNames::Unit : EnumNames::Party);
                const int b = enum_index(text_key(middle), damage ? EnumNames::Party : EnumNames::Unit);
                const int c = enum_index(text_key(inner), EnumNames::Unit);
                reader.read_00bd6830(inner, gui_lua_field(Field::Int, &values[a][b][c]));
            });
            reader.leave_00bd7a20();
        });
        reader.leave_00bd7a20();
    });
    reader.leave_00bd7a20();
}
void write_triples(ProfileArchiveWriter& writer, const char* section,
    const MissionEnumCounterTriples& values, bool damage) {
    if (values.empty()) return;
    writer.begin_section(section);
    for (const auto& [a, middle] : values) {
        writer.begin_section(enum_name(a, damage ? EnumNames::Unit : EnumNames::Party));
        for (const auto& [b, inner] : middle) {
            writer.begin_section(enum_name(b, damage ? EnumNames::Party : EnumNames::Unit));
            for (const auto& [c, value] : inner) write(writer, enum_name(c, EnumNames::Unit), value);
            writer.end_section();
        }
        writer.end_section();
    }
    writer.end_section();
}

void read_objectives(ProfileArchiveReader& reader, MissionObjectiveMap& values) {
    each_key(reader, [&](const GuiLuaVariant& outer) {
        reader.enter_00bd8e20(outer);
        each_key(reader, [&](const GuiLuaVariant& inner) {
            auto& value = values[text_key(outer)];
            value.name = text_key(inner);
            reader.read_00bd6830(inner, gui_lua_field(Field::Int, &value.value));
        });
        reader.leave_00bd7a20();
    });
}

} // namespace

MissionScoreRecord& mission_record_00594a70(MissionProgress& progress, std::string_view key) {
    return progress.mission_scores_00[std::string(key)];
}

void read_mission_score_record_00916a00(
    MissionScoreRecord& record, ProfileArchiveReader& reader, std::uint32_t version) {
    // 00916A31/JL: high-bit native versions are negative signed ints.
    if (version < 2 || version >= 0x80000000u) return;
    read(reader, "play_time", record.play_time_0c);
    read_default_zero(reader, "mission_completed", record.mission_completed_00);
    read_default_zero(reader, "ranking", record.ranking_04);
    GuiLuaVariant zero_float;
    zero_float.tag = static_cast<int>(Field::Float);
    zero_float.value.number = 0.0f;
    reader.read_or_default_00bd68d0(gui_lua_key_by_name("completion_time"),
        gui_lua_field(Field::Float, &record.completion_time_10), zero_float);
    read_default_zero(reader, "difficulty", record.difficulty_08);
    if (has(reader, "TotalScores")) {
        enter(reader, "TotalScores");
        for (int i = 0; i < 3; ++i) {
            const auto key = gui_lua_key_by_index(i);
            if (!reader.has_key_00bd5eb0(key)) continue;
            reader.enter_00bd8e20(key);
            read_if_present(reader, "completed", record.completed_by_difficulty_26c[i]);
            read_if_present(reader, "score", record.scores_by_difficulty_278[i]);
            reader.leave_00bd7a20();
        }
        read_if_present(reader, "winnerMode", record.winner_mode_194);
        reader.leave_00bd7a20();
    }
    if (has(reader, "total")) {
        enter(reader, "total");
        for (std::size_t i = 0; i < totals.size(); ++i)
            read_if_present(reader, totals[i].name, record.totals_1c8[i]);
        reader.leave_00bd7a20();
    }
    if (has(reader, "ScoreMaps")) {
        enter(reader, "ScoreMaps");
        for (const std::size_t i : {0, 1, 2, 3, 4, 6, 5}) {
            if (!has(reader, score_maps[i].name)) continue;
            enter(reader, score_maps[i].name);
            read_named_counters(reader, record.score_maps[i]);
            reader.leave_00bd7a20();
        }
        reader.leave_00bd7a20();
    }
    enter(reader, "trace"); // native enters even when absent
    read_triples(reader, "player_kills", record.player_kills_b4, false);
    read_triples(reader, "party_kills", record.party_kills_c0, false);
    read_triples(reader, "player_damages", record.player_damages_9c, true);
    read_triples(reader, "party_damages", record.party_damages_a8, true);
    if (has(reader, "unit_remaining")) {
        enter(reader, "unit_remaining");
        each_key(reader, [&](const GuiLuaVariant& outer) {
            reader.enter_00bd8e20(outer);
            each_key(reader, [&](const GuiLuaVariant& inner) {
                const int a = enum_index(text_key(outer), EnumNames::Party);
                const int b = enum_index(text_key(inner), EnumNames::Unit);
                reader.read_00bd6830(inner,
                    gui_lua_field(Field::Int, &record.unit_remaining_cc[a][b]));
            });
            reader.leave_00bd7a20();
        });
        reader.leave_00bd7a20();
    }
    if (has(reader, "unit_suicide")) {
        enter(reader, "unit_suicide");
        each_key(reader, [&](const GuiLuaVariant& key) {
            const int i = enum_index(text_key(key), EnumNames::Unit);
            reader.read_00bd6830(key, gui_lua_field(Field::Int, &record.unit_suicide_d8[i]));
        });
        reader.leave_00bd7a20();
    }
    if (has(reader, "unit_usage")) {
        enter(reader, "unit_usage");
        each_key(reader, [&](const GuiLuaVariant& key) {
            const int i = enum_index(text_key(key), EnumNames::Unit);
            reader.read_00bd6830(key, gui_lua_field(Field::Float, &record.unit_usage_114[i]));
        });
        reader.leave_00bd7a20();
    }
    reader.leave_00bd7a20();
    if (has(reader, "objective")) {
        enter(reader, "objective");
        if (has(reader, "allied_party")) read_default_zero(reader, "allied_party", record.allied_party_1e4);
        if (has(reader, "japanese_party")) read_default_zero(reader, "japanese_party", record.japanese_party_1e8);
        for (std::size_t i = 0; i < losses.size(); ++i) {
            if (!has(reader, losses[i].name)) continue;
            enter(reader, losses[i].name);
            read_named_counters(reader, record.losses_1ec[i]);
            reader.leave_00bd7a20();
        }
        for (std::size_t i = 0; i < objectives.size(); ++i) {
            if (!has(reader, objectives[i].name)) continue;
            enter(reader, objectives[i].name);
            read_objectives(reader, record.objectives_204[i]);
            reader.leave_00bd7a20();
        }
        reader.leave_00bd7a20();
    }
    read_if_present(reader, "map_usage", record.map_usage_14);
    read_if_present(reader, "usedSlot", record.used_slot_250);
    // 00919EF1..0091A060 checks this spelling but DOES NOT enter the section.
    // It never changes +24Ch. Preserve the native asymmetry with the writer.
    if (has(reader, "CheckPointData"))
        for (std::size_t i = 0; i < checkpoints.size(); ++i)
            read_if_present(reader, checkpoints[i].name, record.checkpoint_counters_168[i]);
}

void write_mission_score_record_00908a30(MissionScoreRecord& record, ProfileArchiveWriter& writer) {
    write(writer, "play_time", record.play_time_0c);
    if (record.mission_completed_00 != 0) {
        write(writer, "mission_completed", record.mission_completed_00);
        if (record.ranking_04 != 0) write(writer, "ranking", record.ranking_04);
        if (record.completion_time_10 != 0.0f) write(writer, "completion_time", record.completion_time_10);
    }
    if (record.difficulty_08 != 0) write(writer, "difficulty", record.difficulty_08);
    writer.begin_section("TotalScores");
    for (int i = 0; i < 3; ++i) {
        writer.begin_section(gui_lua_key_by_index(i));
        write(writer, "completed", record.completed_by_difficulty_26c[i]);
        write(writer, "score", record.scores_by_difficulty_278[i]);
        writer.end_section();
    }
    write(writer, "winnerMode", record.winner_mode_194);
    writer.end_section();
    writer.begin_section("total");
    for (std::size_t i = 0; i < totals.size(); ++i) write(writer, totals[i].name, record.totals_1c8[i]);
    writer.end_section();
    writer.begin_section("ScoreMaps");
    for (const std::size_t i : {0, 1, 3, 2, 4, 6, 5})
        write_named_counters(writer, score_maps[i].name, record.score_maps[i], true, true);
    writer.end_section();
    writer.begin_section("trace");
    write_triples(writer, "player_kills", record.player_kills_b4, false);
    write_triples(writer, "player_damages", record.player_damages_9c, true);
    write_triples(writer, "party_kills", record.party_kills_c0, false);
    write_triples(writer, "party_damages", record.party_damages_a8, true);
    if (!record.unit_remaining_cc.empty()) {
        writer.begin_section("unit_remaining");
        for (const auto& [party, values] : record.unit_remaining_cc) {
            writer.begin_section(enum_name(party, EnumNames::Party));
            for (const auto& [unit, value] : values) write(writer, enum_name(unit, EnumNames::Unit), value);
            writer.end_section();
        }
        writer.end_section();
    }
    if (!record.unit_suicide_d8.empty()) {
        writer.begin_section("unit_suicide");
        for (const auto& [unit, value] : record.unit_suicide_d8) write(writer, enum_name(unit, EnumNames::Unit), value);
        writer.end_section();
    }
    if (!record.unit_usage_114.empty()) {
        writer.begin_section("unit_usage");
        for (const auto& [unit, value] : record.unit_usage_114)
            if (value != 0.0f) write(writer, enum_name(unit, EnumNames::Unit), value);
        writer.end_section();
    }
    writer.end_section();
    writer.begin_section("objective");
    if (record.allied_party_1e4 != 0) write(writer, "allied_party", record.allied_party_1e4);
    if (record.japanese_party_1e8 != 0) write(writer, "japanese_party", record.japanese_party_1e8);
    for (std::size_t i = 0; i < losses.size(); ++i)
        write_named_counters(writer, losses[i].name, record.losses_1ec[i], true, true);
    for (std::size_t i = 0; i < objectives.size(); ++i) {
        const auto& values = record.objectives_204[i];
        if (values.empty()) continue;
        writer.begin_section(objectives[i].name);
        for (const auto& [key, value] : values) {
            writer.begin_section(key.c_str());
            write(writer, value.name.c_str(), value.value);
            writer.end_section();
        }
        writer.end_section();
    }
    writer.end_section();
    write(writer, "map_usage", record.map_usage_14);
    write(writer, "usedSlot", record.used_slot_250);
    if (record.checkpoint_pending_24c) {
        writer.begin_section("ChackPointData"); // exact writer literal, 00D1850C
        for (std::size_t i = 0; i < checkpoints.size(); ++i)
            write(writer, checkpoints[i].name, record.checkpoint_counters_168[i]);
        writer.end_section();
        record.checkpoint_pending_24c = false; // 0090AB92, after successful normal flow
    }
}

void read_mission_progress_00920000(
    MissionProgress& progress, ProfileArchiveReader& reader, std::uint32_t version) {
    const bool wrapped = has(reader, "missionScores");
    if (wrapped) {
        enter(reader, "singleBestScores");
        read_named_counters(reader, progress.single_best_scores_0c);
        reader.leave_00bd7a20();
        if (has(reader, "MultiScores")) {
            enter(reader, "MultiScores");
            read_named_counters(reader, progress.multi_scores_18);
            reader.leave_00bd7a20();
        }
        enter(reader, "missionScores");
    }
    each_key(reader, [&](const GuiLuaVariant& key) {
        auto& record = mission_record_00594a70(progress, text_key(key));
        reader.enter_00bd8e20(key);
        enter(reader, "sum");
        read_mission_score_record_00916a00(record, reader, version);
        reader.leave_00bd7a20();
        read(reader, "count", record.count_284);
        reader.leave_00bd7a20();
    });
    if (wrapped) reader.leave_00bd7a20();
}

void write_mission_progress_0090cb40(MissionProgress& progress, ProfileArchiveWriter& writer) {
    write_named_counters(writer, "singleBestScores", progress.single_best_scores_0c, false, false);
    write_named_counters(writer, "MultiScores", progress.multi_scores_18, false, false);
    writer.begin_section("missionScores");
    for (auto& [key, record] : progress.mission_scores_00) {
        writer.begin_section(key.c_str());
        writer.begin_section("sum");
        write_mission_score_record_00908a30(record, writer);
        writer.end_section();
        write(writer, "count", record.count_284);
        writer.end_section();
    }
    writer.end_section();
}

int sum_mission_progress_0090bf50(const MissionProgress& progress) noexcept {
    // Native ADD wraps in 32 bits; unsigned accumulation avoids signed C++ UB.
    std::uint32_t sum = 0;
    for (const auto& item : progress.multi_scores_18) sum += static_cast<std::uint32_t>(item.second);
    int result;
    static_assert(sizeof result == sizeof sum);
    std::memcpy(&result, &sum, sizeof result);
    return result;
}

void sync_mission_completion(const MissionProgress& progress, ProfileUnlockState& unlocks) {
    unlocks.mission_completion.clear();
    for (const auto& [key, record] : progress.mission_scores_00)
        unlocks.mission_completion.emplace(key, record.mission_completed_00);
}

} // namespace bsp
