#include "bsp/mission_tree_data.hpp"

#include <algorithm>
#include <cctype>
#include <string>

// docs/MISSION_TREE_LUA_READER.md. Every rule here is one branch of 005C6A70,
// 005C5DA0, 005C9F70 or 005CAAF0; the addresses are in the comments.

namespace bsp {
namespace {

bool equal_ignoring_case(std::string_view left, std::string_view right) noexcept {
    // 005C34E7's __stricmp, restricted to the ASCII range the ids use.
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto a = static_cast<unsigned char>(left[i]);
        const auto b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return true;
}

// 005C5DA0's list reads are all "if the key is present, replace the vector".
// An absent key leaves the destination alone, which for a freshly constructed
// block is the empty vector.
void read_list_if_present(MissionTreeLuaView& view, std::string_view key,
                          std::vector<std::string>& out) {
    if (view.has_name(key)) out = view.read_string_array(key);
}

// 005C6D03 and the twelve sites like it in 005C5DA0 test the same key twice
// before reading. The second test cannot fail once the first passed, so the
// behaviour is one guard; it is reproduced as one call and recorded in the doc.
void read_int_list_if_present(MissionTreeLuaView& view, std::string_view key,
                              std::vector<std::int32_t>& out) {
    if (view.has_name(key)) out = view.read_int_array(key);
}

std::string mode_key(std::string_view mode, const char* suffix) {
    std::string key(mode);
    key += suffix;
    return key;
}

}  // namespace

// ---------------------------------------------------------------------------
// 005C5DA0, one 154h side block
// ---------------------------------------------------------------------------

void read_mission_side_block_005c5da0(MissionTreeLuaView& view, MissionSideBlockData& out) {
    // 005C5DA5: the block's first dword becomes 1 before anything is read. The
    // reader is only reached when the side key exists, so "block 0 enabled"
    // and "the mission has an allied side" are the same fact.
    out.screen.enabled = 1;

    out.screen.briefing_key = view.read_string("briefingGuiLayer", "");

    read_list_if_present(view, "primaryObjectives", out.screen.objectives_a);
    read_list_if_present(view, "secondaryObjectives", out.screen.objectives_b);
    read_list_if_present(view, "hiddenObjectives", out.extra.hidden_objectives);
    read_list_if_present(view, "hiddenHints", out.extra.hidden_hints);
    read_list_if_present(view, "loadingBackgrounds", out.extra.loading_backgrounds);
    read_list_if_present(view, "hints", out.screen.loading_text);
    read_list_if_present(view, "allunitsid", out.extra.unit_ids);
    read_list_if_present(view, "allunitsnum", out.extra.unit_counts);
    read_list_if_present(view, "allunitslockid", out.extra.locked_unit_ids);
    read_list_if_present(view, "allunitslocknum", out.extra.locked_unit_counts);
    read_list_if_present(view, "allunitslockhint", out.extra.locked_unit_hints);
    read_list_if_present(view, "changeables", out.extra.changeables);

    for (std::size_t i = 0; i < kMissionMultiplayerModeCount; ++i) {
        const std::string key = std::string("multiunitsid") + std::string(kMissionSideUnitListModes[i]);
        read_list_if_present(view, key, out.extra.multiplayer_unit_ids[i]);
    }
}

// ---------------------------------------------------------------------------
// 005C6DBE, MultiPlayMapSizes
// ---------------------------------------------------------------------------

std::array<MissionMapCorners, kMissionMultiplayerModeCount>
default_mission_map_sizes() noexcept {
    std::array<MissionMapCorners, kMissionMultiplayerModeCount> corners{};
    for (auto& mode : corners) {
        mode.north_west = kMissionMapDefaultNorthWest;
        mode.south_east = kMissionMapDefaultSouthEast;
    }
    return corners;
}

std::array<MissionMapCorners, kMissionMultiplayerModeCount>
read_mission_map_sizes_005c6dbe(MissionTreeLuaView& view) {
    std::array<MissionMapCorners, kMissionMultiplayerModeCount> corners{};
    // 005C6DDF: the table is entered once and every corner is a keyed read
    // inside it, with the origin as the default. A mode the table omits is
    // therefore {0,0,0} and not the +/-15000 box the missing-table arm writes.
    view.enter_by_name("MultiPlayMapSizes");
    for (std::size_t i = 0; i < kMissionMultiplayerModeCount; ++i) {
        const auto mode = kMissionMultiplayerModes[i];
        corners[i].north_west = view.read_vec3(mode_key(mode, "_nw"), kMissionMapMissingCorner);
        corners[i].south_east = view.read_vec3(mode_key(mode, "_se"), kMissionMapMissingCorner);
    }
    view.leave();
    return corners;
}

// ---------------------------------------------------------------------------
// 005C6A70, one 434h mission record
// ---------------------------------------------------------------------------

void read_mission_record_005c6a70(MissionTreeLuaView& view, MissionRecordData& out) {
    out.screen.name = view.read_string("id", "");
    out.screen.title = view.read_string("name", "");
    out.extra.content_id = view.read_string("contentID", "");
    out.extra.helpline = view.read_string("helpline", "");
    out.screen.scene = view.read_string("sceneFile", "");
    out.extra.movie_name = view.read_string("MovieName", "");

    // 005C6C3D: date arrives as three floats and is stored as three ints by
    // CVTTSS2SI, which truncates toward zero.
    const std::array<float, 3> date = view.read_vec3("date", {0.0f, 0.0f, 0.0f});
    out.screen.briefing_word0 = static_cast<std::uint32_t>(static_cast<std::int32_t>(date[0]));
    out.screen.briefing_word1 = static_cast<std::uint32_t>(static_cast<std::int32_t>(date[1]));
    out.screen.briefing_word2 = static_cast<std::uint32_t>(static_cast<std::int32_t>(date[2]));

    // 005C6C78: Pos is guarded, and the absent arm writes three zeroes with
    // XORPS rather than taking a reader default.
    out.extra.position = view.has_name("Pos") ? view.read_vec3("Pos", kMissionMapMissingCorner)
                                              : std::array<float, 3>{0.0f, 0.0f, 0.0f};

    read_list_if_present(view, "prerequisites", out.screen.unlock_requirements);
    read_int_list_if_present(view, "navalacademy", out.extra.naval_academy);

    out.extra.description = view.read_string("description", "");
    // 005C6D8E reads `picture` into a frame temporary; only a non-empty string
    // reaches 00AA2660, whose result is the texture at +0A0h and whose atlas
    // rectangle is +0A4h. An empty string releases the old texture and leaves
    // the field null.
    out.extra.picture = view.read_string("picture", "");

    out.extra.map_sizes = view.has_name("MultiPlayMapSizes")
                              ? read_mission_map_sizes_005c6dbe(view)
                              : default_mission_map_sizes();

    // 005C7641: the default pair is (Int, 3), the same value
    // kMissionDifficultyFromPlayer names on the consuming side.
    out.screen.difficulty = static_cast<std::uint32_t>(
        view.read_int("forcedDifficultyLevel",
                      static_cast<std::int32_t>(kMissionDifficultyFromPlayer)));

    for (std::size_t side = 0; side < kMissionSideBlockCount; ++side) {
        const auto key = kMissionSideKeys[side];
        if (!view.has_name(key)) continue;
        view.enter_by_name(key);
        read_mission_side_block_005c5da0(view, out.sides[side]);
        view.leave();
    }

    out.extra.side_mission = view.has_name(kMissionSideMissionKey)
                                 ? view.read_bool(kMissionSideMissionKey, false)
                                 : false;

    out.extra.background = view.read_string("background", "");
    out.extra.background_voice = view.read_string("backgroundVoice", "");
    out.extra.background_movie = view.read_string("backgroundMovie", "");
    out.extra.debriefing_text = view.read_string("debriefingText", "");
    out.extra.debriefing_voice = view.read_string("debriefingVoice", "");

    // 005C78E5 clears the settings container before the walk.
    out.extra.unique_multi_settings.clear();
    out.extra.has_unique_multi_settings = view.has_name("UniqueMultiSettings");
    if (!out.extra.has_unique_multi_settings) return;

    // 005C7944..005C7ADF: two nested key enumerations. The outer keys are the
    // multiplayer modes, the inner keys the parameters, and the only value
    // read is the MenuDIS boolean, stored inverted (005C7A87's SETZ on the
    // byte the read produced).
    view.enter_by_name("UniqueMultiSettings");
    for (const std::string& mode : view.string_keys()) {
        view.enter_by_name(mode);
        for (const std::string& parameter : view.string_keys()) {
            view.enter_by_name(parameter);
            const bool menu_dis = view.read_bool(kMissionUniqueMultiSettingKey, false);
            out.extra.unique_multi_settings.push_back({{mode, parameter}, !menu_dis});
            view.leave();
        }
        view.leave();
    }
    view.leave();
}

// ---------------------------------------------------------------------------
// 005C9F70, one 34h group entry
// ---------------------------------------------------------------------------

void read_mission_group_005c9f70(MissionTreeLuaView& view, MissionGroupData& out) {
    out.extra.group_name = view.read_string("groupName", "");
    out.extra.help_line = view.read_string("helpLine", "");
    out.extra.grat_msg = view.read_string("gratMsg", "");

    const std::array<float, 3> grat = view.read_vec3("gratDate", {0.0f, 0.0f, 0.0f});
    for (std::size_t i = 0; i < grat.size(); ++i) {
        out.extra.grat_date[i] = static_cast<std::int32_t>(grat[i]);
    }

    // 005CA0B0: the mission vector is emptied, then walked with integer keys
    // from 1. Every element is default constructed inside the vector and read
    // in place, so a partially read record keeps its defaults.
    out.missions.clear();
    view.enter_by_name("missions");
    for (std::int32_t index = 1; view.has_index(index); ++index) {
        view.enter_by_index(index);
        out.missions.emplace_back();
        read_mission_record_005c6a70(view, out.missions.back());
        view.leave();
    }
    view.leave();
}

// ---------------------------------------------------------------------------
// 005CAAF0, the whole load
// ---------------------------------------------------------------------------

float mission_tree_load_progress(std::int32_t one_based_index) noexcept {
    return static_cast<float>(static_cast<double>(one_based_index) * kMissionTreeProgressStep +
                              kMissionTreeProgressBase);
}

MissionTreeIndex find_mission_by_id_005c3470(const std::vector<MissionGroupData>& groups,
                                             std::string_view id) noexcept {
    MissionTreeIndex found{};
    for (std::size_t group = 0; group < groups.size(); ++group) {
        const auto& missions = groups[group].missions;
        for (std::size_t mission = 0; mission < missions.size(); ++mission) {
            if (equal_ignoring_case(missions[mission].screen.name, id)) {
                found.mission = static_cast<std::uint32_t>(mission);
                found.group = static_cast<std::uint32_t>(group);
                return found;
            }
        }
    }
    return found;
}

MissionTreeTables load_mission_tree_005caaf0(MissionTreeScriptHost& host) {
    MissionTreeTables tables{};

    host.open_state(kMissionTreeLuaLibraryMask);
    host.run_script(kMissionTreeScriptPath);
    MissionTreeLuaView& view = host.open_table(kMissionTreeGlobalTable);

    // 005CAC6F: the group loop. Every entry is push_backed first and filled in
    // place afterwards, and the progress report is the loop's last statement.
    view.enter_by_name(kMissionTreeGroupsKey);
    for (std::int32_t index = 1; view.has_index(index); ++index) {
        tables.groups.emplace_back();
        view.enter_by_index(index);
        read_mission_group_005c9f70(view, tables.groups.back());
        view.leave();
        host.report_progress(mission_tree_load_progress(index));
    }
    view.leave();

    // 005CAD95: the flat multiplayer list. No progress is reported here.
    view.enter_by_name(kMissionTreeMultiKey);
    for (std::int32_t index = 1; view.has_index(index); ++index) {
        tables.multi.emplace_back();
        view.enter_by_index(index);
        read_mission_record_005c6a70(view, tables.multi.back());
        view.leave();
    }
    view.leave();

    // 005CAE84: the shell's requested id, then the clamp at 005CAF0C. A
    // negative mission index -- the {FFFFFFFFh, 0} miss -- becomes (0, 0).
    tables.selection = find_mission_by_id_005c3470(tables.groups, host.requested_mission_id());
    if (static_cast<std::int32_t>(tables.selection.mission) < 0) {
        tables.selection.mission = 0;
        tables.selection.group = 0;
    }

    host.close_state();
    return tables;
}

}  // namespace bsp
