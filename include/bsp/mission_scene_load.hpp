#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/frontend_entry.hpp"

// The mission scene load: BSP_Game_LoadMissionScene (004dfb70), the handler the
// state-request drain of docs/GAME_FRAME_CONTROL.md selects for requests 0Ah and
// 0Bh, and the only path in the image that raises the loading screen for a
// mission. Evidence and uncertainties: docs/MISSION_SCENE_LOAD.md.
//
// The whole load is synchronous inside this one call. Nothing is posted to the
// loading queue that docs/APP_RUN_FRAME.md drives (004fde20 / 00509190); the
// loading screen is animated by the render worker that begin_loading_screen
// starts, and the caller blocks until end_loading_screen at 004e185c.
namespace bsp {
struct GlobalSubsystemInvocation;

// Values of game+5D4h that reach 004dfb70. The drain writes the dequeued request
// into game+5D4h before dispatch, so the handler reads its own request there.
// These complement GameStateId in bsp/game_frame_control.hpp, which does not
// name the load requests.
inline constexpr std::uint32_t kMissionSceneLoadRequest = 0x0A;
inline constexpr std::uint32_t kMissionSceneReloadRequest = 0x0B;

// game+5D4h on return, written at 004e086b, before the mission scripts run. It
// is the device/profile wait state BSP_Game_UpdateDeviceWaitScreen (004db920)
// services; that handler ends in 004da6c0, which writes GameStateId::kInMission.
inline constexpr std::uint32_t kGameStateSceneReady = 0x0C;

// Label of the VFS file block opened around the loading-screen bring-up, chosen
// from game+5D4h at 004dfdd5. The third value is reached when the drain routes a
// request other than 0Ah or 0Bh here, which the dispatch table never does.
const char* mission_scene_load_block_label(std::uint32_t request) noexcept;

// The five numbered VFS file blocks 004dfb70 opens around the load phases. Each
// name is the prefix below concatenated with derive_scene_short_name of the
// scene path. 004dfb70 itself opens no "2_" block; the "2_" block is opened
// inside 004d4df0 (load_scene_contents) around scene-file passes 2 and 3
// (docs/MISSION_SCENE_CONTENTS.md), and the GvSpace_Init block sits between
// the "1_" and "3_" blocks of this routine.
enum class MissionLoadPhase {
    WorldConstruction = 1, // "1_", 004dff44; scene file pass 1 and the world
    MissionScript = 3, // "3_", 004e0971; Scripts/missions/<name>.lua
    StageInit = 4, // "4_", 004e0b6c; luaStageInitMulti then luaStageInit
    EngineMovie = 5, // "5_", 004e0d33; luaEngineMovieInit
    RendererHandoff = 6, // "6_", 004e0e8b; [00F8D394] vtable +E4h
};
const char* mission_load_phase_prefix(MissionLoadPhase phase) noexcept;
std::string mission_load_phase_block_name(MissionLoadPhase phase, const std::string& short_name);

// The literal file block opened at 004e04b9 around the in-mission HUD manager
// bring-up. Unlike the numbered blocks it carries no mission name.
inline constexpr const char* kGvSpaceInitBlock = "GvSpace_Init"; // 00CE7E94

// 004cd7f0. Derives the short mission name every numbered block is keyed on from
// the scene path at [game+5FCh]+90Ch. In native order: normalise '\' to '/', keep
// the text after the last '/', truncate at the SECOND '_' when there is one, then
// drop a trailing ".scn" (case-insensitive). Both of the last two steps always
// run; the ".scn" strip is a no-op once the underscore truncation has fired.
std::string derive_scene_short_name(const std::string& scene_path);

// 008860b0. The mission script name held in the record table is resolved to
// "Scripts/missions/" + name + ".lua" and handed to 00885fb0 with 1.
inline constexpr const char* kMissionScriptDirectory = "Scripts/missions/"; // 008860f8
inline constexpr const char* kMissionScriptExtension = ".lua"; // 00CFD2C8
std::string mission_script_path(const std::string& script_name);

// Offset of the script-name table inside the scene record at game+5FCh. Entries
// are the native NativeString {int length; char* data;} pair, stride 8, indexed
// by the slot normalise_mission_script_slot returns (004e08aa).
inline constexpr std::size_t kSceneRecordScriptTableOffset = 0x928;
inline constexpr std::size_t kSceneRecordScriptTableStride = 8;
// Other scene-record fields 004dfb70 reads.
inline constexpr std::size_t kSceneRecordScenePathOffset = 0x90C; // to 0046df00
inline constexpr std::size_t kSceneRecordLocaleTableListOffset = 0x980; // comma separated
inline constexpr std::size_t kSceneRecordSideBlockCountOffset = 0x988;
inline constexpr std::size_t kSceneRecordMissionIdOffset = 0x1098; // to [00F8A2FC]+48h

// 004e087b. Which entry of the script table the load runs. Slot 9 always falls
// back to 8; outside network play and without the force byte at game+61Ch every
// slot other than 8 also falls back to 8, so single player has exactly one
// script slot. The raw slot is re-read afterwards for the arm decision below.
std::int32_t normalise_mission_script_slot(
    std::int32_t script_slot, bool script_slot_forced, std::int32_t session_mode) noexcept;

// 004e0a50. The raw game+614h value, not the normalised one, selects the arm:
// slot 9 runs only the engine-movie Lua entry point, every other slot runs the
// precache and stage-init sequence. The nested compare at 004e0a5f..004e0a75 is
// the compiler sharing the "== 9" test between the two guard arms; the guard
// bytes do not change the outcome.
bool mission_uses_engine_movie(std::int32_t script_slot) noexcept;

// Named Lua entry points 004dfb70 invokes, in call order. luaPrecacheUnits and
// luaStageInitMulti go through 0045f520; luaStageInit and luaEngineMovieInit go
// through 0045f440. The two dispatchers differ only in the callee they reach
// (00887b30 versus 00887e50).
inline constexpr const char* kLuaPrecacheUnits = "luaPrecacheUnits";
inline constexpr const char* kLuaStageInitMulti = "luaStageInitMulti";
inline constexpr const char* kLuaStageInit = "luaStageInit";
inline constexpr const char* kLuaEngineMovieInit = "luaEngineMovieInit";

// Suffix appended to a spoken-warning bank name, chosen at 004e10e6, 004e105f
// and 004e1150 from the local participant's side selector. Zero selects "_Ally".
const char* mission_side_suffix(std::int32_t side_selector) noexcept;

// The audio bank names built at 004e11c3, 004e129c, 004e14d3 and 004e16cc.
inline constexpr const char* kWarningsBankPrefix = "Warnings_";
inline constexpr const char* kWarningsAuthenticEnglishBank = "Warnings_englishauthentic";
inline constexpr const char* kWarningsPlaceholderBank = "dummy DO NOT USE";
inline constexpr const char* kAuthenticEnglishContentId = "DL_Content_0000062";
std::string warnings_bank_name(const std::string& language, std::int32_t side_selector);
std::string authentic_warnings_bank_name(std::int32_t side_selector);

// Fields of the game object (00E188A8) that 004dfb70 reads or writes. Offsets are
// from the listing; see docs/MISSION_SCENE_LOAD.md for the store sites.
struct MissionSceneLoadState {
    std::uint32_t state{kMissionSceneLoadRequest}; // +5D4h, read at 004dfdd5, written at 004e086b
    std::int32_t session_mode{0}; // +1FE4h: 0 single player, 1 and 2 network
    bool script_slot_forced{false}; // +61Ch
    std::int32_t script_slot{0}; // +614h
    std::int32_t local_slot{0}; // +18ECh, cleared at 004dfd77 in single player
    std::int32_t side_selector{0}; // [game+18CCh + local_slot*4]+28h
    bool scene_reload_latch{false}; // +1EE7h, cleared first at 004dfb9d
    bool hud_suppressed{false}; // +193Ch, cleared at 004e0360 and 004e05a2
    std::int32_t objective_counter{0}; // +648h, cleared at 004e075a
    float objective_timer{0.0f}; // +64Ch, cleared at 004e0764
    bool scene_resident{false}; // 00E0AF20, cleared at 004e0150, set at 004e0865
    std::string scene_path; // [game+5FCh]+90Ch
    std::string scene_override; // the string at game+600h/604h
    std::string locale_table_list; // [game+5FCh]+980h, comma separated
    std::string script_name; // [game+5FCh]+928h + slot*8
    std::int32_t resolved_script_slot{0}; // what normalise_mission_script_slot returned
    std::int32_t mission_id{0}; // [game+5FCh]+1098h, published to [00F8A2FC]+48h
    bool have_scene_record{true}; // game+5FCh non-null; every string above is empty when false
};

// 004e10c0..004e10e6. The listing takes the bank-loading arm when the installed
// entry does NOT match the current language, so the loop is an eviction pass over
// the other languages. An empty name on either side alone counts as a mismatch;
// two empty names count as a match.
bool language_entry_matches(const std::string& current, const std::string& entry) noexcept;

// 0094ec70 with the literal "," at 00CE4BFC. Splits the scene record's locale
// table list; each element is registered through BSP_Localization_RegisterTableName
// (00aa0d30) and the set is committed once with BSP_Localization_ReloadTables (00aa06d0).
inline constexpr char kLocaleTableListSeparator = ',';
std::vector<std::string> split_locale_table_list(const std::string& list);

// Integration boundary. One method per native call site of 004dfb70, in the order
// the listing reaches them. Nothing here has a default implementation: none of it
// stands in for unrecovered game behaviour.
struct MissionSceneLoadHost {
    virtual ~MissionSceneLoadHost() = default;

    // Prologue, 004dfba3..004dfc0e.
    virtual float pending_time_scale() = 0; // [00F8BBD8]+6Ch
    virtual void clear_pending_time_scale() = 0; // [00F8BBD8]+6Ch = 0
    virtual void apply_audio_time_scale(float scale) = 0; // 00a7a440 with [00F8BBD8]+4Ch
    virtual void set_cinematic_mode(int a, int b, int c) = 0; // 004cd0f0, ECX = game
    virtual void reset_frame_pacing_globals() = 0; // 00F874FD, 00E08178, 00E0E35C, 00E0E2FC, game+610h
    virtual void session_reset() = 0; // 0076da60, ECX = game+1EF0h

    // Participant tables, 004dfc13..004dfd80. Exactly one of these runs.
    virtual void reset_single_player_slots() = 0; // 004bb160 then 004bb440, sets game+18CCh/18ECh
    // The 004dfc13..004dfd18 network arm (docs/MISSION_LOAD_HOSTS.md, packet
    // cc2_mission_load_hosts): erases the two native-string sets 00E18A60/00E18A6C
    // through 004cec60 (an MSVC tree erase, not a slot reset), clears the eight
    // headers at game+758h, 00626930. A local session takes reset_single_player_slots
    // instead, so this step leaves nothing behind there.
    virtual void reset_network_slots() = 0;

    // Front-end teardown, 004dfd90..004dfdd0. The two managers are released
    // through virtual +0h with 1 and the globals nulled.
    virtual void release_main_menu_manager() = 0; // 00E198AC
    virtual void release_secondary_menu_manager() = 0; // 00E198B4, single player only
    virtual void detach_network_menu_manager() = 0; // 006878f0, network only

    // Loading screen, 004dfe3f..004dfe7e.
    virtual void enter_file_block(const std::string& name) = 0; // 00be0a30
    virtual void leave_file_block() = 0; // 00bdcb30
    virtual void publish_loading_screen_config() = 0; // 0057d0c0 -> 0057cff0
    virtual void begin_loading_screen(LoadingScreenMode mode) = 0; // 0057cb60, ECX = mode
    virtual void end_loading_screen() = 0; // 0057c250

    // Previous mission result, 004dfe83..004dfeb3. Freed, not reused.
    virtual void release_mission_result() = 0; // game+7188h

    // Phase 1, inside the "1_" block.
    // Supply current owners/services; the caller runs004DC6A0 directly.
    virtual GlobalSubsystemInvocation global_subsystems() = 0;
    virtual void reset_render_scene() = 0; // 00874640(0)
    virtual void reset_effect_atlas() = 0; // 006ad600, ECX = game+21D8h
    // 0046df00, ECX = [00E18680]. Three passes run per load; this one is
    // (path, 0, 0, record, override, 0). The other two are inside 004d4df0.
    virtual void load_scene_file(const std::string& path, const std::string& override_name) = 0;
    virtual void construct_world() = 0; // 004de610: game+19CCh, game+21D4h, ocean and sky
    virtual void reset_shader_globals() = 0; // 00951560 on 00F89A08 and 00F89A5C
    virtual bool lua_global_exists(const char* name) = 0; // 00b65fb0 on "thisTable"
    // 00b67580: globals[name] = {} (a fresh empty table). 004e021a..004e029b reads
    // globals.thisTable, tests it with 00b65fb0 and creates the table ONLY when it
    // was nil; a non-nil thisTable is kept across the load (docs/MISSION_LUA_TEARDOWN.md,
    // packet cc2_lobby_settings). Formerly misnamed lua_clear_global.
    virtual void lua_set_global_empty_table(const char* name) = 0; // 00b67580
    virtual void sync_lobby_settings_from_lua() = 0; // 005e2f00 BSP_Game_SyncLobbySettingsFromLua: opens the LobbySettings global and walks the thirteen slots of 00e08908 (docs/MISSION_LUA_MACHINE.md); it resets no Lua state (2f correction 3)
    virtual void lua_declare_global(const char* name) = 0; // 00b67350 on "recon"
    virtual void resolve_named_scene_objects() = 0; // 004f2800

    // Phase 2, 004e0378..004e0463.
    virtual void reset_slot_cameras() = 0; // game+1910h..192Ch = -1, then 0095ba60 in network play
    virtual void load_scene_contents() = 0; // 004d4df0, scene file passes 2 and 3
    virtual bool slot_present(int slot) = 0; // game+18CCh + slot*4 non-null
    virtual void activate_slot(int slot) = 0; // 007fa2d0 per non-null game+18CCh entry
    virtual void assign_party_player_slots() = 0; // 004c3840(0), session_mode 1 only

    // In-mission HUD manager, 004e0463..004e05c0.
    virtual void create_hud_manager() = 0; // operator new(0x108) then 0068a990 into 00E198C4
    virtual void reset_hud_layout() = 0; // 004c9680
    virtual void select_front_end_layout(int layout) = 0; // 004c1ac0(3,0) then 00518250(3,0)
    virtual void hud_manager_init() = 0; // 00E198C4 vtable +4h
    virtual void hud_manager_start() = 0; // 00E198C4 vtable +8h
    virtual void publish_side_to_renderer(std::int32_t side_selector) = 0; // [00F8BBCC]+210h
    virtual void prime_view(int index) = 0; // 008053c0 three times then 00807a50
    virtual void bind_local_view() = 0; // slot +19h/+30h, 008073c0, 004c3cb0, 006485a0

    // Localisation, 004e05c8..004e0750.
    virtual void register_locale_table(const std::string& name) = 0; // 00aa0d30
    virtual void reload_locale_tables() = 0; // 00aa06d0(0)

    // Input and session handover, 004e0750..004e0870.
    // 004e0754..004e07c2: clears an unidentified tree at game+5C8h and rebuilds the
    // avoid-zone table through 004218e0 BSP_AvoidZoneManager_GetSingleton and
    // 00424d00 (docs/MISSION_LOAD_HOSTS.md). The objective sets live at
    // game+21A4h + slot*4; the old name reset_objective_list was wrong.
    virtual void reset_avoid_zone_state() = 0;
    virtual void set_input_capture(bool capture) = 0; // [00F8BBF4]+64h
    virtual void input_update(float seconds) = 0; // 004bec00 then 00a92c40
    virtual void dispatch_session_ready_event() = 0; // 0075b430(0Ch) then 00770af0, mode 2
    virtual void mark_local_slot_ready() = 0; // slot +0Eh = 1, +10h = FFFDh, +18h = 0, mode 1
    virtual void commit_scene_ready() = 0; // 0077f5e0

    // Mission scripts, 004e0870..004e0f60.
    // 004d30f0, ECX = game: fills the set at game+1930h with the name of every Lua
    // global whose value is a function, the pre-script baseline that 004d32a0 nils
    // against at teardown (docs/MISSION_LOAD_HOSTS.md).
    virtual void rebuild_scripted_name_list() = 0;
    virtual void run_mission_script(const std::string& path) = 0; // 008860b0 -> 00885fb0(path, 1)
    virtual void lua_collect_garbage() = 0; // 006b8ad0("collectgarbage(\"collect\")", 0, 0, 2)
    virtual bool lua_script_mode_enabled() = 0; // [game+1A08h]+4h
    virtual void push_script_reentry_guard(int delta) = 0; // [00E188A8]+644h += delta
    virtual void lua_call_entry_point_a(const char* name) = 0; // 0045f520 -> 00887b30
    virtual void lua_call_entry_point_b(const char* name) = 0; // 0045f440 -> 00887e50
    virtual void precache_units() = 0; // the eleven calls at 004e0a9b..004e0acd
    virtual bool lua_entry_point_exists(const char* name) = 0; // 00b66200 on "luaEngineMovieInit"
    virtual void renderer_scene_ready() = 0; // [00F8D394] vtable +E4h

    // Spoken warnings, 004e0f60..004e1840.
    virtual int spoken_language_setting() = 0; // [004c1e90()]+8h, 2 selects the enumerated path
    virtual int spoken_language_index() = 0; // 004c1e90(), the per-side flag index
    virtual bool warnings_loaded_for_side(std::int32_t side_selector, int language_index) = 0;
    virtual void mark_warnings_loaded_for_side(std::int32_t side_selector, int language_index) = 0;
    virtual std::string current_language_name() = 0; // 008d4870, ECX = 00F88980
    virtual std::vector<std::string> installed_languages() = 0; // 008d76c0
    virtual void select_voice_language(const std::string& name) = 0; // 008d56c0
    virtual bool content_installed(const char* content_id) = 0; // 007f8890, ECX = 00E188A8
    virtual void load_warning_bank(int mode) = 0; // 007065e0, ECX = game+21DCh
    virtual void notify_slots_ready() = 0; // 00a32350, when a slot has +8h/+9h/+0Ah set
    virtual void publish_mission_id(std::int32_t mission_id) = 0; // [00F8A2FC]+48h
    virtual void clear_render_gate() = 0; // 00F1B038

    // Epilogue, 004e185c..004e18a2.
    virtual void gui_set_enabled(bool enabled) = 0; // 004c12b0 then 00aa0e20
    virtual void apply_in_game_interface() = 0; // 004c9ca0(1)
};

// 004dfb70. Native __thiscall(GGame* this) with ECX holding the game object, no
// stack arguments, RET at 004e18d9, no return value. The whole load runs to
// completion before the call returns and the state left behind is
// kGameStateSceneReady.
void run_mission_scene_load(MissionSceneLoadState& state, MissionSceneLoadHost& host);
}
