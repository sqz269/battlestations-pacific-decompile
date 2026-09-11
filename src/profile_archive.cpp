#include "bsp/profile_archive.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace bsp {
namespace {

using Field = GuiLuaFieldType;

void read_field(ProfileArchiveReader& reader, const char* key, Field type, void* dest) {
    reader.read_00bd6830(gui_lua_key_by_name(key), gui_lua_field(type, dest));
}

void read_int_or_default(
    ProfileArchiveReader& reader, const char* key, int& dest, int fallback) {
    reader.read_or_default_00bd68d0(gui_lua_key_by_name(key),
        gui_lua_field(Field::Int, &dest), gui_lua_key_by_index(fallback));
}

bool has_section(ProfileArchiveReader& reader, const char* key) {
    return reader.has_key_00bd5eb0(gui_lua_key_by_name(key));
}

void enter(ProfileArchiveReader& reader, const char* key) {
    reader.enter_00bd8e20(gui_lua_key_by_name(key));
}

template <typename Append>
void read_strings(ProfileArchiveReader& reader, Append append) {
    for (std::int32_t index = 0;
         reader.has_key_00bd5eb0(gui_lua_key_by_index(index)); ++index) {
        std::string value;
        reader.read_00bd6830(gui_lua_key_by_index(index), gui_lua_field(Field::String, &value));
        append(std::move(value));
    }
}

void write(ProfileArchiveWriter& writer, const char* key, const SettingsValue& value) {
    writer.write_field(gui_lua_key_by_name(key), value);
}

template <typename Strings>
void write_strings(ProfileArchiveWriter& writer, const char* section, const Strings& strings) {
    writer.begin_section(section);
    std::int32_t index = 0;
    for (const auto& value : strings) {
        writer.write_field(gui_lua_key_by_index(index++), SettingsValue::from_text(value.c_str()));
    }
    writer.end_section();
}

} // namespace

void read_profile_archive_007fdf00(
    ProfileResetState& profile, ProfileArchiveReader& reader, ProfileArchiveHost& host) {
    enter(reader, "PlayerProfile");
    int version = 0;
    read_int_or_default(reader, "Version", version, 0);
    profile.version_e8 = static_cast<std::uint32_t>(version);
    read_field(reader, "Voice", Field::Bool, &profile.voice_59);
    read_field(reader, "Difficulty", Field::Int, &profile.difficulty_5c);
    read_int_or_default(reader, "SelectedDifficulty", profile.selected_difficulty_60,
                        profile.difficulty_5c);
    read_int_or_default(reader, "JapanNoseArt", profile.japan_nose_art_e0, 1);
    read_int_or_default(reader, "AlliedNoseArt", profile.allied_nose_art_e4, 1);
    read_field(reader, "SelectedMissionID", Field::String, &profile.selected_mission_id_68);
    if (has_section(reader, "DropRateTC")) {
        read_field(reader, "DropRateTC", Field::Int, &profile.drop_rate_tc_ec);
    }
    if (has_section(reader, "DropRateDC")) {
        read_field(reader, "DropRateDC", Field::Int, &profile.drop_rate_dc_f0);
    }
    if (has_section(reader, "Bonus")) {
        enter(reader, "Bonus");
        profile.bonus_ac.clear();
        read_strings(reader, [&](std::string value) { profile.bonus_ac.push_back(std::move(value)); });
        reader.leave_00bd7a20();
    }

    enter(reader, "Unlocks");
    profile.unlock_state.unlocks.clear();
    read_strings(reader, [&](std::string value) { profile.unlock_state.unlocks.insert(std::move(value)); });
    reader.leave_00bd7a20();
    profile.seen_unlocks_88.clear();

    // 007fe3c4..007fe3d6: this clear precedes the AllSeenHints presence test.
    if (host.hints_owner_field_08_004c1e90() != 0) {
        profile.string_list_08.clear();
    }
    if (has_section(reader, "AllSeenHints")) {
        enter(reader, "AllSeenHints");
        read_strings(reader, [&](std::string value) { profile.string_list_08.push_back(std::move(value)); });
        if (profile.string_list_08.size() > 1000) {
            profile.string_list_08.clear();
        }
        reader.leave_00bd7a20();
    }

    auto& filters = profile.lobby_filters;
    if (!has_section(reader, "SavedLobbyFilters")) {
        filters.player_count_20 = 0;
        filters.free_slots_24 = 0;
        filters.latency_28 = 0;
        filters.array_size_2c = 9;
        filters.entries_14.clear();
        for (int i = 0; i < 9; ++i) {
            filters.entries_14.push_back(static_cast<std::uint8_t>(i < 5));
        }
    } else {
        filters.entries_14.clear();
        enter(reader, "SavedLobbyFilters");
        const auto read_filter = [&](const char* name, int& field) {
            if (has_section(reader, name)) {
                read_field(reader, name, Field::Int, &field);
            } else {
                field = 0;
            }
        };
        read_filter("filterPlayerCount", filters.player_count_20);
        read_filter("filterFreeSlots", filters.free_slots_24);
        read_filter("filterLatency", filters.latency_28);
        read_filter("arraySize", filters.array_size_2c);
        if (filters.array_size_2c != 0) {
            // Native order queries existence before comparing the signed bound.
            for (std::int32_t index = 0;
                 reader.has_key_00bd5eb0(gui_lua_key_by_index(index)) &&
                 index < filters.array_size_2c; ++index) {
                bool value = false;
                reader.read_00bd6830(gui_lua_key_by_index(index), gui_lua_field(Field::Bool, &value));
                filters.entries_14.push_back(static_cast<std::uint8_t>(value));
            }
        }
        reader.leave_00bd7a20();
    }
    if (has_section(reader, "SeenUnlocks")) {
        enter(reader, "SeenUnlocks");
        read_strings(reader, [&](std::string value) { profile.seen_unlocks_88.insert(std::move(value)); });
        reader.leave_00bd7a20();
    }

    auto& counters = profile.unlock_state.named_counters;
    counters.clear();
    if (has_section(reader, "Achievements")) {
        enter(reader, "Achievements");
        GuiLuaKeyArray keys;
        gui_lua_clear_key_array_00aaa710(keys);
        reader.enumerate_keys_00bd5f50(keys);
        std::int32_t legacy_index = 0;
        for (std::int32_t i = 0; i < keys.count; ++i) {
            const auto& key = keys.keys[i];
            if (key.tag == static_cast<int>(GuiLuaKeyKind::Name)) {
                auto& value = counters[key.value.text];
                reader.read_00bd6830(key, gui_lua_field(Field::Int, &value));
            } else if (key.tag == static_cast<int>(GuiLuaKeyKind::Index)) {
                // 007fecc6..007fed17 ignores the enumerated numeric key value.
                std::string name;
                reader.read_00bd6830(gui_lua_key_by_index(legacy_index++),
                    gui_lua_field(Field::String, &name));
                counters[name] = 1;
            }
        }
        reader.leave_00bd7a20();
    }

    if (profile.mission_progress_present) {
        host.destroy_mission_progress_007fd780();
        profile.mission_progress_present = false;
    }
    profile.unlock_state.mission_completion.clear();
    profile.mission_progress_present = host.construct_mission_progress_00920e10();
    enter(reader, "Score");
    host.read_mission_progress_00920000(profile, reader);
    reader.leave_00bd7a20();
    reader.leave_00bd7a20();
    profile.score_total_30 = host.sum_mission_progress_0090bf50();
    profile.transient_records_94.clear();
    host.refresh_profile_manager_004374f0(profile);
}

void write_profile_archive_007f9540(
    ProfileResetState& profile, ProfileArchiveWriter& writer, ProfileArchiveHost& host) {
    writer.begin_section("PlayerProfile");
    write(writer, "Name", SettingsValue::from_text(profile.player_name_3c.c_str()));
    // 007f95a2 uses signed JGE. High-bit versions are negative native ints.
    if (profile.version_e8 < 2 || profile.version_e8 >= 0x80000000u) {
        profile.version_e8 = 2;
    }
    write(writer, "Version", SettingsValue::from_int(static_cast<int>(profile.version_e8)));
    write(writer, "Voice", SettingsValue::from_bool(profile.voice_59));
    write(writer, "Difficulty", SettingsValue::from_int(profile.difficulty_5c));
    write(writer, "SelectedDifficulty", SettingsValue::from_int(profile.selected_difficulty_60));
    write(writer, "JapanNoseArt", SettingsValue::from_int(profile.japan_nose_art_e0));
    write(writer, "AlliedNoseArt", SettingsValue::from_int(profile.allied_nose_art_e4));
    write(writer, "DropRateTC", SettingsValue::from_int(profile.drop_rate_tc_ec));
    write(writer, "DropRateDC", SettingsValue::from_int(profile.drop_rate_dc_f0));
    write(writer, "SelectedMissionID", SettingsValue::from_text(profile.selected_mission_id_68.c_str()));
    write_strings(writer, "Bonus", profile.bonus_ac);
    write_strings(writer, "Unlocks", profile.unlock_state.unlocks);
    if (!profile.seen_unlocks_88.empty()) {
        write_strings(writer, "SeenUnlocks", profile.seen_unlocks_88);
    }
    if (!profile.string_list_08.empty()) {
        write_strings(writer, "AllSeenHints", profile.string_list_08);
    }
    const auto& filters = profile.lobby_filters;
    if (filters.array_size_2c != 0) {
        writer.begin_section("SavedLobbyFilters");
        write(writer, "filterPlayerCount", SettingsValue::from_int(filters.player_count_20));
        write(writer, "filterFreeSlots", SettingsValue::from_int(filters.free_slots_24));
        write(writer, "filterLatency", SettingsValue::from_int(filters.latency_28));
        write(writer, "arraySize", SettingsValue::from_int(filters.array_size_2c));
        std::int32_t index = 0;
        for (const auto value : filters.entries_14) {
            writer.write_field(gui_lua_key_by_index(index++), SettingsValue::from_bool(value != 0));
        }
        writer.end_section();
    }
    writer.begin_section("Achievements");
    if (profile.unlock_state.named_counters.empty()) {
        write(writer, "RANK", SettingsValue::from_int(1));
    } else {
        for (const auto& entry : profile.unlock_state.named_counters) {
            write(writer, entry.first.c_str(), SettingsValue::from_int(entry.second));
        }
    }
    writer.end_section();
    writer.begin_section("Score");
    host.write_mission_progress_0090cb40(profile, writer);
    writer.end_section();
    writer.end_section();
}

} // namespace bsp
