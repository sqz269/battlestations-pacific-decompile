// The mission-tree table readers: the Lua schema behind the 34h group entries,
// the 434h mission records and their two 154h side blocks.
//
// Addresses: 005CAAF0 (register / table load), 005C9F70 (group reader),
//            005C6A70 (mission-record reader), 005C5DA0 (side-block reader),
//            005C5860 (string-array reader), 005C4AD0 (int-array reader),
//            005CAA40 (group push_back), 005C9E30 (record push_back),
//            005CA300 (group range copy), 005C9830 (group copy ctor),
//            005C9B60 (group dtor), 005C6850 / 005C49C0 (settings map insert).
//
// docs/MISSION_TREE_LUA_READER.md carries the evidence. This header names the
// bytes of each record from the Lua key that fills it; the screen-facing subset
// of the same records stays in bsp/mission_tree_screens.hpp and is aggregated
// here rather than restated. The reader virtuals, the key kinds and the field
// type tags are bsp/gui_lua_reader.hpp's: the mission tree drives the same
// 00CE44FC reader the GUI pages use.
//
// None of these structs is binary compatible; the offsets are comments.

#ifndef BSP_MISSION_TREE_DATA_HPP
#define BSP_MISSION_TREE_DATA_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/mission_tree_screens.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The script the screen loads
// ---------------------------------------------------------------------------

// 005CAB3F: BSP_NativeString_Resize(path, 22h, 1) then a copy of the 34-byte
// literal at 00CF176C. 005CAB85: 00B69D40(path, 0), so the VFS override search
// of docs/GUI_LUA_READER.md applies and a mounted package may replace the file.
inline constexpr std::string_view kMissionTreeScriptPath =
    "Scripts/datatables/MissionTree.lua";

// 005CABC4: 00B67800 on the globals object with the literal at 00CF1760.
inline constexpr std::string_view kMissionTreeGlobalTable = "MissionTree";

// 005CAC68 and 005CAD8E, the two top-level keys, in the order the reader walks
// them. Each is entered once and iterated with integer keys from 1.
inline constexpr std::string_view kMissionTreeGroupsKey = "missionGroups";
inline constexpr std::string_view kMissionTreeMultiKey = "multiMissionInfos";

// 005CAB28 pushes 4 into 00B6A020, so this interpreter opens the `table`
// library alone -- not the 65h mask the GUI pages get. Base, `string` and
// `math` are absent unless the state owner's own bootstrap installs them.
inline constexpr std::uint32_t kMissionTreeLuaLibraryMask = 0x04;

// 005CAD49..005CAD65: progress = index * 0.02 + 0.45, computed on the x87 stack
// from the 1-based group index and narrowed to a float before
// BSP_LoadingScreen_ReportProgress (0057BEC0). Only the group loop reports.
inline constexpr float kMissionTreeProgressBase = 0.45f;
inline constexpr float kMissionTreeProgressStep = 0.02f;

// ---------------------------------------------------------------------------
// The schema, as data
// ---------------------------------------------------------------------------

// Which reader virtual a field uses. The mission tree never calls the key
// enumerator except inside UniqueMultiSettings.
enum class MissionTreeReadSlot : std::int32_t {
    Read = 0x10,           // 00BD6830, no presence test
    ReadOrDefault = 0x0C,  // 00BD68D0, a nil takes the default
    GuardedRead = 0x14,    // 00BD5EB0 first, then 00BD6830 or a literal default
    StringArray,           // 005C5860, guarded by 00BD5EB0
    IntArray,              // 005C4AD0, guarded by 00BD5EB0 twice
    Nested,                // enter, a sub-reader, leave
};

// One key of one record. `offset` is the byte the native reader writes, so the
// table doubles as the layout of the record.
struct MissionTreeFieldSpec {
    std::string_view key;
    GuiLuaFieldType type;
    std::uint32_t offset;
    MissionTreeReadSlot slot;
    std::string_view default_text;  // GuiLuaFieldType::String defaults
    std::int32_t default_int;       // GuiLuaFieldType::Int / Bool defaults
};

// 005C9F70, the 34h group entry. gratDate is read as three floats into a frame
// temporary and stored as three ints by CVTTSS2SI (005C6C3D's pattern).
inline constexpr std::array<MissionTreeFieldSpec, 5> kMissionGroupSchema{{
    {"groupName", GuiLuaFieldType::String, 0x00, MissionTreeReadSlot::Read, "", 0},
    {"helpLine", GuiLuaFieldType::String, 0x08, MissionTreeReadSlot::Read, "", 0},
    {"gratMsg", GuiLuaFieldType::String, 0x10, MissionTreeReadSlot::Read, "", 0},
    {"gratDate", GuiLuaFieldType::Vec3, 0x18, MissionTreeReadSlot::Read, "", 0},
    {"missions", GuiLuaFieldType::Unhandled9, 0x24, MissionTreeReadSlot::Nested, "", 0},
}};

// 005C6A70, the 434h mission record, in call order.
inline constexpr std::array<MissionTreeFieldSpec, 24> kMissionRecordSchema{{
    {"id", GuiLuaFieldType::String, 0x000, MissionTreeReadSlot::Read, "", 0},
    {"name", GuiLuaFieldType::String, 0x008, MissionTreeReadSlot::Read, "", 0},
    {"contentID", GuiLuaFieldType::String, 0x010, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"helpline", GuiLuaFieldType::String, 0x018, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"sceneFile", GuiLuaFieldType::String, 0x020, MissionTreeReadSlot::Read, "", 0},
    {"MovieName", GuiLuaFieldType::String, 0x058, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"date", GuiLuaFieldType::Vec3, 0x060, MissionTreeReadSlot::Read, "", 0},
    {"Pos", GuiLuaFieldType::Vec3, 0x06C, MissionTreeReadSlot::GuardedRead, "", 0},
    {"prerequisites", GuiLuaFieldType::String, 0x078, MissionTreeReadSlot::StringArray, "", 0},
    {"navalacademy", GuiLuaFieldType::Int, 0x088, MissionTreeReadSlot::IntArray, "", 0},
    {"description", GuiLuaFieldType::String, 0x098, MissionTreeReadSlot::Read, "", 0},
    {"picture", GuiLuaFieldType::String, 0x0A0, MissionTreeReadSlot::Read, "", 0},
    {"MultiPlayMapSizes", GuiLuaFieldType::Vec3, 0x364, MissionTreeReadSlot::Nested, "", 0},
    {"forcedDifficultyLevel", GuiLuaFieldType::Int, 0x0B4, MissionTreeReadSlot::ReadOrDefault, "", 3},
    {"allied", GuiLuaFieldType::Unhandled9, 0x0B8, MissionTreeReadSlot::Nested, "", 0},
    {"japanese", GuiLuaFieldType::Unhandled9, 0x20C, MissionTreeReadSlot::Nested, "", 0},
    {"sideMission", GuiLuaFieldType::Bool, 0x360, MissionTreeReadSlot::GuardedRead, "", 0},
    {"background", GuiLuaFieldType::String, 0x030, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"backgroundVoice", GuiLuaFieldType::String, 0x048, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"backgroundMovie", GuiLuaFieldType::String, 0x040, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"debriefingText", GuiLuaFieldType::String, 0x038, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"debriefingVoice", GuiLuaFieldType::String, 0x050, MissionTreeReadSlot::ReadOrDefault, "", 0},
    {"UniqueMultiSettings", GuiLuaFieldType::Bool, 0x424, MissionTreeReadSlot::GuardedRead, "", 0},
    {"MenuDIS", GuiLuaFieldType::Bool, 0x42C, MissionTreeReadSlot::Nested, "", 0},
}};

// 005C5DA0, the 154h side block, in call order. Every list is a string array;
// the block's first dword is set to 1 before any key is read, which is what
// makes "block 0 present" the side selector at 005C572A.
inline constexpr std::array<MissionTreeFieldSpec, 21> kMissionSideBlockSchema{{
    {"briefingGuiLayer", GuiLuaFieldType::String, 0x004, MissionTreeReadSlot::Read, "", 0},
    {"primaryObjectives", GuiLuaFieldType::String, 0x014, MissionTreeReadSlot::StringArray, "", 0},
    {"secondaryObjectives", GuiLuaFieldType::String, 0x024, MissionTreeReadSlot::StringArray, "", 0},
    {"hiddenObjectives", GuiLuaFieldType::String, 0x034, MissionTreeReadSlot::StringArray, "", 0},
    {"hiddenHints", GuiLuaFieldType::String, 0x044, MissionTreeReadSlot::StringArray, "", 0},
    {"loadingBackgrounds", GuiLuaFieldType::String, 0x054, MissionTreeReadSlot::StringArray, "", 0},
    {"hints", GuiLuaFieldType::String, 0x064, MissionTreeReadSlot::StringArray, "", 0},
    {"allunitsid", GuiLuaFieldType::String, 0x074, MissionTreeReadSlot::StringArray, "", 0},
    {"allunitsnum", GuiLuaFieldType::String, 0x084, MissionTreeReadSlot::StringArray, "", 0},
    {"allunitslockid", GuiLuaFieldType::String, 0x0A4, MissionTreeReadSlot::StringArray, "", 0},
    {"allunitslocknum", GuiLuaFieldType::String, 0x0B4, MissionTreeReadSlot::StringArray, "", 0},
    {"allunitslockhint", GuiLuaFieldType::String, 0x0C4, MissionTreeReadSlot::StringArray, "", 0},
    {"changeables", GuiLuaFieldType::String, 0x094, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidIC1v1", GuiLuaFieldType::String, 0x114, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidIC2v2", GuiLuaFieldType::String, 0x124, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidIC3v3", GuiLuaFieldType::String, 0x134, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidIC4v4", GuiLuaFieldType::String, 0x144, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidDuel", GuiLuaFieldType::String, 0x0D4, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidSiege", GuiLuaFieldType::String, 0x0E4, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidEscort", GuiLuaFieldType::String, 0x0F4, MissionTreeReadSlot::StringArray, "", 0},
    {"multiunitsidCompetitive", GuiLuaFieldType::String, 0x104, MissionTreeReadSlot::StringArray, "", 0},
}};

// The two side keys, indexed by the block the reader fills: block 0 is +0B8h
// and block 1 is +20Ch (005C76B2 and 005C76FC).
inline constexpr std::array<std::string_view, kMissionSideBlockCount> kMissionSideKeys{
    "allied", "japanese"};

// ---------------------------------------------------------------------------
// Multiplayer map sizes
// ---------------------------------------------------------------------------

// The eight modes of MultiPlayMapSizes, in **record-offset** order. Each
// contributes a `<mode>_nw` and a `<mode>_se` float[3] corner, so the sixteen
// entries fill +364h..+423h at a 0Ch stride. The reader's call order is a
// different one -- Competitive, Duel, Escort, then the four Island Capture
// sizes, then Siege -- and only the offsets decide the layout.
inline constexpr std::size_t kMissionMultiplayerModeCount = 8;
inline constexpr std::array<std::string_view, kMissionMultiplayerModeCount>
    kMissionMultiplayerModes{"IslandCapture1v1", "IslandCapture2v2",
                             "IslandCapture3v3", "IslandCapture4v4",
                             "Siege",           "Competitive",
                             "Duel",            "Escort"};

// The same eight modes in the order the side block's `multiunitsid<Mode>`
// lists occupy, +0D4h..+144h. It is **not** kMissionMultiplayerModes' order,
// so the two tables are kept apart rather than shared.
inline constexpr std::array<std::string_view, kMissionMultiplayerModeCount>
    kMissionSideUnitListModes{"Duel",  "Siege",            "Escort",
                              "Competitive",              "IslandCapture1v1",
                              "IslandCapture2v2",         "IslandCapture3v3",
                              "IslandCapture4v4"};

// 005C6DE1's else arm writes the corners at this stride, mode-major, in the
// kMissionMultiplayerModes order: +364h is IslandCapture1v1_nw.
inline constexpr std::uint32_t kMissionMapSizeTableOffset = 0x364;
inline constexpr std::uint32_t kMissionMapSizeCornerStride = 0x0C;

// DAT_00CF1458 and DAT_00CF1464. The whole-table default is a +/-15000 box per
// mode; the per-corner default, once the table exists, is the origin. That
// asymmetry is the reason the two constants are separate.
inline constexpr float kMissionMapDefaultExtent = 15000.0f;
inline constexpr std::array<float, 3> kMissionMapDefaultNorthWest{
    -kMissionMapDefaultExtent, 0.0f, kMissionMapDefaultExtent};
inline constexpr std::array<float, 3> kMissionMapDefaultSouthEast{
    kMissionMapDefaultExtent, 0.0f, -kMissionMapDefaultExtent};
inline constexpr std::array<float, 3> kMissionMapMissingCorner{0.0f, 0.0f, 0.0f};

struct MissionMapCorners {
    std::array<float, 3> north_west{};
    std::array<float, 3> south_east{};
};

// 005C7767's key, read as one byte with GuiLuaFieldType::Bool.
inline constexpr std::string_view kMissionSideMissionKey = "sideMission";

// 005C7A5B: the only key read inside a UniqueMultiSettings leaf table.
inline constexpr std::string_view kMissionUniqueMultiSettingKey = "MenuDIS";

// ---------------------------------------------------------------------------
// The records, as the table fills them
// ---------------------------------------------------------------------------

// The 12Ch of a 154h side block that MissionSideBlock does not name. Held
// beside it rather than inside it so no field has two homes:
// MissionSideBlock::enabled is +00h, ::briefing_key is +04h briefingGuiLayer,
// ::objectives_a is +14h primaryObjectives, ::objectives_b is +24h
// secondaryObjectives and ::loading_text is +64h hints.
struct MissionSideBlockExtra {
    std::vector<std::string> hidden_objectives;    // +034h hiddenObjectives
    std::vector<std::string> hidden_hints;         // +044h hiddenHints
    std::vector<std::string> loading_backgrounds;  // +054h loadingBackgrounds
    std::vector<std::string> unit_ids;             // +074h allunitsid
    std::vector<std::string> unit_counts;          // +084h allunitsnum
    std::vector<std::string> changeables;          // +094h changeables
    std::vector<std::string> locked_unit_ids;      // +0A4h allunitslockid
    std::vector<std::string> locked_unit_counts;   // +0B4h allunitslocknum
    std::vector<std::string> locked_unit_hints;    // +0C4h allunitslockhint
    // +0D4h..+144h, in kMissionSideUnitListModes order.
    std::array<std::vector<std::string>, kMissionMultiplayerModeCount> multiplayer_unit_ids{};
};

// One whole side block: the screen-facing part and the rest.
struct MissionSideBlockData {
    MissionSideBlock screen{};
    MissionSideBlockExtra extra{};
};

// The bytes of a 434h record that MissionRecord does not name.
// MissionRecord::name is +00h `id`, ::title is +08h `name`, ::scene is +20h
// `sceneFile`, ::briefing_word0..2 are the +60h `date` triple,
// ::unlock_requirements is +78h `prerequisites` and ::difficulty is +0B4h
// `forcedDifficultyLevel`.
struct MissionRecordExtra {
    std::string content_id;       // +010h contentID, default ""
    std::string helpline;         // +018h helpline, default ""
    std::string background;       // +030h background, default ""
    std::string debriefing_text;  // +038h debriefingText, default ""
    std::string background_movie; // +040h backgroundMovie, default ""
    std::string background_voice; // +048h backgroundVoice, default ""
    std::string debriefing_voice; // +050h debriefingVoice, default ""
    std::string movie_name;       // +058h MovieName, default ""
    std::array<float, 3> position{};          // +06Ch Pos, default {0,0,0}
    std::vector<std::int32_t> naval_academy;  // +088h navalacademy
    std::string description;                  // +098h description
    std::string picture;                      // consumed into +0A0h / +0A4h
    bool side_mission{false};                 // +360h sideMission, default false
    // +364h..+423h, in kMissionMultiplayerModes order.
    std::array<MissionMapCorners, kMissionMultiplayerModeCount> map_sizes{};
    bool has_unique_multi_settings{false};    // +424h, the presence byte
    // +42Ch, an ordered container keyed by (mode, parameter).
    std::vector<std::pair<std::pair<std::string, std::string>, bool>> unique_multi_settings;
};

struct MissionRecordData {
    MissionRecord screen{};
    MissionRecordExtra extra{};
    std::array<MissionSideBlockData, kMissionSideBlockCount> sides{};
};

// The 24h of a 34h group entry that precedes its mission vector.
struct MissionGroupExtra {
    std::string group_name;  // +00h groupName
    std::string help_line;   // +08h helpLine
    std::string grat_msg;    // +10h gratMsg
    // +18h, +1Ch, +20h: gratDate read as float[3] and truncated to int.
    std::array<std::int32_t, 3> grat_date{};
};

struct MissionGroupData {
    MissionGroupExtra extra{};
    std::vector<MissionRecordData> missions;  // group +24h..+30h, stride 434h
};

// What 005CAAF0 leaves in the screen: the two vectors and the resolved indices.
struct MissionTreeTables {
    std::vector<MissionGroupData> groups;      // +14h..+20h, stride 34h
    std::vector<MissionRecordData> multi;      // +24h..+30h, stride 434h
    MissionTreeIndex selection{};              // +0Ch, +10h
};

// ---------------------------------------------------------------------------
// The abstract Lua-value view
// ---------------------------------------------------------------------------

// One method per reader operation the mission tree actually uses, stated in
// terms of values rather than destination addresses. `enter`/`leave` are the
// reader's +4h/+8h; `has` is +14h; the typed reads are +10h and +0Ch; the two
// array reads are 005C5860 and 005C4AD0, which are loops over +14h and +10h
// with integer keys from 1. `keys` is +18h, used only inside
// UniqueMultiSettings.
struct MissionTreeLuaView {
    virtual ~MissionTreeLuaView() = default;

    virtual void enter_by_name(std::string_view key) = 0;
    virtual void enter_by_index(std::int32_t index) = 0;
    virtual void leave() = 0;

    virtual bool has_name(std::string_view key) = 0;
    virtual bool has_index(std::int32_t index) = 0;

    virtual std::string read_string(std::string_view key, std::string_view fallback) = 0;
    virtual std::int32_t read_int(std::string_view key, std::int32_t fallback) = 0;
    virtual bool read_bool(std::string_view key, bool fallback) = 0;
    virtual std::array<float, 3> read_vec3(std::string_view key,
                                           const std::array<float, 3>& fallback) = 0;

    // 005C5860 / 005C4AD0: clear the destination, enter the key, read 1..n
    // until has_index fails, leave. An absent key is never entered, so the
    // destination keeps whatever it held; both call sites guard with has_name.
    virtual std::vector<std::string> read_string_array(std::string_view key) = 0;
    virtual std::vector<std::int32_t> read_int_array(std::string_view key) = 0;

    // 00BD5F50, capped at kGuiLuaKeyArrayCapacity string keys.
    virtual std::vector<std::string> string_keys() = 0;
};

// ---------------------------------------------------------------------------
// The record-filling rules
// ---------------------------------------------------------------------------

// Each of these is called with the view positioned on the table it reads and
// leaves it there. They are the bodies of 005C5DA0, 005C6A70 and 005C9F70.
void read_mission_side_block_005c5da0(MissionTreeLuaView& view, MissionSideBlockData& out);
void read_mission_record_005c6a70(MissionTreeLuaView& view, MissionRecordData& out);
void read_mission_group_005c9f70(MissionTreeLuaView& view, MissionGroupData& out);

// 005C6DBE's two arms. A missing MultiPlayMapSizes gives every mode the
// +/-15000 box; a present one gives every missing corner the origin instead.
std::array<MissionMapCorners, kMissionMultiplayerModeCount>
default_mission_map_sizes() noexcept;
std::array<MissionMapCorners, kMissionMultiplayerModeCount>
read_mission_map_sizes_005c6dbe(MissionTreeLuaView& view);

// 005CAD49: the value 0057BEC0 receives for the 1-based group index.
float mission_tree_load_progress(std::int32_t one_based_index) noexcept;

// ---------------------------------------------------------------------------
// The reader sequence
// ---------------------------------------------------------------------------

// One method per native call site of 005CAAF0 outside the reader itself.
struct MissionTreeScriptHost {
    virtual ~MissionTreeScriptHost() = default;

    // 00B66BD0 then 00B6A020(mask): construct the interpreter owner on the
    // frame and create the state with the library mask.
    virtual void open_state(std::uint32_t library_mask) = 0;

    // 00B69D40(path, 0): run the file and every VFS override of it, unprotected.
    virtual void run_script(std::string_view path) = 0;

    // 00B67980 (globals), 00B67800 (the named global), 00B66FA0 and 004425C0
    // (the reader over it). The view is owned by the host and stays valid
    // until close_state.
    virtual MissionTreeLuaView& open_table(std::string_view global_name) = 0;

    // 0057BEC0, once per group entry.
    virtual void report_progress(float fraction) = 0;

    // 00586150: the mission id the shell wants selected.
    virtual std::string requested_mission_id() = 0;

    // 00B669A0 through the LuaStateOwner destructor.
    virtual void close_state() = 0;
};

// 005CAAF0's body after BSP_FrontEndScreen_Register. The selection tail is
// 005C3470 followed by the negative-index clamp, which is why an unknown id
// falls back to the first mission of the first group.
MissionTreeTables load_mission_tree_005caaf0(MissionTreeScriptHost& host);

// 005C3470: walk every group and every mission and compare the record's id
// case-insensitively. The default is {FFFFFFFFh, 0}.
MissionTreeIndex find_mission_by_id_005c3470(const std::vector<MissionGroupData>& groups,
                                             std::string_view id) noexcept;

}  // namespace bsp

#endif  // BSP_MISSION_TREE_DATA_HPP
