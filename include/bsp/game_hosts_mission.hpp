#pragma once
// bsp_game.exe milestone 2e: from the main menu to the mission load request,
// as process bindings.
//
// Addresses: 005caaf0 (the mission-tree screen's register virtual, which reads
// Scripts/datatables/MissionTree.lua), 005c3470 (the id lookup its tail runs),
// 00586150 (the mission id the shell asks for), 005861b0 (the main-menu
// screen's bind-layout virtual and the three page roots it keeps), 00584ae0
// (the seven top-level list entries), 00580940 (the mission-selection publish
// and the page pairing), 0058c010 (the mission-detail page builder), 00599db0
// (the per-frame page machine), 00598b60 (the page dispatcher whose campaign
// arm at 00599318 calls the builder), 005922f0 (the play action), 0058bdf0
// (the start), 00626930 (the per-mission statistics reset), 00439020 (the two
// state requests), 004e2770 / 004e1d70 (the pending scene and the scene
// record), 0046df00 (the .scn header pass) and 004c6890 (the scene-record
// selection).
//
// Nothing in this file is a reconstruction of native code. Every type here is
// an integration binding that satisfies one of the host interfaces in
// bsp/mission_tree_data.hpp, bsp/main_menu_screen.hpp,
// bsp/main_menu_mission_detail.hpp, bsp/mission_briefing_start.hpp,
// bsp/mission_load_path.hpp and bsp/scene_file.hpp with either a concrete
// implementation over an already reconstructed routine or the explicit
// unimplemented policy in GameHostLog.
//
// Evidence: docs/MISSION_TREE_LUA_READER.md, docs/MAIN_MENU_SCREENS.md,
// docs/MAIN_MENU_SCREEN_UPDATE.md, docs/MAIN_MENU_MISSION_DETAIL.md,
// docs/MISSION_BRIEFING_START.md, docs/MISSION_LOAD_PATH.md,
// docs/SCENE_FILE_READER.md and docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp {
class LocaleTables;
struct GuiLayoutPage;
struct MissionPictureTextureServices;
}  // namespace bsp

namespace bsp::game {

class GameHostLog;
class GameVfsHost;
class GameScriptHost;
class GameFrontendHost;
class GameFrameProfiler;
class GameMissionLuaHost;
class GameMissionFrameHost;
class GameHudHost;

// One class token of the selected mission's `.scn`, as the reconstructed
// reader counted it. `registered` is bsp::scene_entity_class_is_registered.
struct GameSceneClassCount {
    std::string name;
    std::size_t count{0};
    bool registered{false};
    bool registration_pass{false};
};

// One top-level menu item, with the locale string its label id resolves to.
struct GameMenuItemLabel {
    int index{0};
    std::string label_id;   // 00e087b8[index]
    std::string text;       // what 00a9ec70 returned for it
    bool resolved{false};   // the id is in the loaded table
    bool enabled{true};     // 00cef77c[index]
};

// Where the scripted selection has got to. Each value is one frame's work.
enum class GameMissionStep {
    Idle,          // no --menu-select, or the main menu is not up yet
    PublishList,   // 00580940 published the tree selection and chose the page
    MissionList,   // 00599db0 ran the mission-list arm at least once
    DetailPage,    // 0058c010 built page 9
    BriefingStart, // 005922f0 then 0058bdf0 ran
    LoadRequested, // 00439020's two requests are on the queue
    SceneRecord,   // 004e2770 built the record and the .scn was read
    // Milestone 2f. The load no longer stops in front of the first
    // renderer-owner host: it walks the recovered inventory to its end, the
    // state 0Ch handler enters the mission and the in-mission branch of
    // 004e4a40 runs headless.
    SceneLoaded,   // the load ran to game state 0Ch
    InMission,     // 004da6c0 wrote game state 0Dh
    MissionFrames, // --mission-frames N frames of 004e4a40 ran
    Stopped,       // nothing left to do
};
const char* game_mission_step_name(GameMissionStep step) noexcept;

struct GameMissionSummary {
    std::string requested_id;          // --menu-select
    // 005caaf0
    bool tree_loaded{false};
    bool tree_pictures_loaded{false}; // actual resource reader, separate from metadata
    bool tree_script_ran{false};
    std::string tree_error;
    std::size_t tree_groups{0};
    std::size_t tree_missions{0};
    std::size_t tree_multi{0};
    bool requested_found{false};
    std::uint32_t selected_group{0};   // the tree screen's +10h
    std::uint32_t selected_mission{0}; // the tree screen's +0Ch
    std::string selected_id;           // the record's Lua `id`
    std::string selected_title;        // the record's Lua `name`
    std::string selected_scene;        // the record's `sceneFile`
    // 005861b0
    std::size_t detail_pages_loaded{0};
    std::size_t detail_widgets_bound{0};
    std::size_t detail_widgets_missing{0};
    // 00584ae0
    std::vector<GameMenuItemLabel> menu_items;
    std::size_t menu_items_resolved{0};
    // 00580940 and 0058c010
    int published_group{0};            // 00e194d8
    int published_mission{0};          // 00e194dc
    int list_page{0};                  // 00e08874 after the publish
    bool detail_built{false};
    int detail_page{0};
    std::size_t detail_missions_visited{0};
    std::size_t detail_map_flags{0};
    std::size_t detail_map_points{0};
    std::string detail_briefing_text;  // the record's `background` key
    // 005922f0 / 0058bdf0 / 00439020
    bool briefing_started{false};
    bool movie_arm{false};
    std::string movie_name;
    bool load_requested{false};
    std::size_t requests_queued{0};
    // 004e2770 through the .scn
    bool scene_record_built{false};
    std::string scene_path;
    std::string scene_short_name;
    std::string scene_stem;
    std::int32_t scene_mission_id{0};
    bool scene_file_read{false};
    std::size_t scene_bytes{0};
    std::size_t scene_entities{0};
    std::size_t scene_distinct_classes{0};
    std::size_t scene_unregistered{0};
    std::size_t scene_groups{0};
    std::vector<GameSceneClassCount> scene_classes;
    std::vector<std::string> scene_block_names;  // the five numbered VFS blocks
    std::string mission_script_path;
    // The load inventory
    std::size_t load_host_steps{0};
    std::size_t load_host_external{0};
    std::string load_stopped_at;
    // Milestone 2f
    long mission_frames_requested{0};
    unsigned long long mission_frames_run{0};
    unsigned long long mission_frames_simulated{0};
    bool mission_load_finished{false};
    bool mission_entered{false};
    int mission_game_state{0};
    std::size_t mission_load_concrete{0};
    std::size_t mission_load_records{0};
    std::size_t lua_bindings{0};
    std::size_t lua_natives{0};
    unsigned long long lua_native_calls{0};
    std::string lua_script_path;
    std::string mission_exit_note;
    // Milestone 2g: the exit path out of game state 0Dh.
    unsigned long long mission_exit_frames{0};
    bool mission_exit_completed{false};
    bool mission_complete_injected{false};
    GameMissionStep step{GameMissionStep::Idle};
};

// Everything milestone 2e adds behind the main menu, owned for the whole run.
//
// The order is the recovered one: the mission-tree screen's register virtual
// loads the tree, the main-menu screen's bind-layout virtual loads the three
// page roots the detail page drives, and once the shell has published the
// main-menu screen the scripted selection walks 00580940, 00599db0, 0058c010,
// 005922f0, 0058bdf0 and 004e2770 one step per frame.
class GameMissionHost {
public:
    GameMissionHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
        GameFrontendHost& frontend, LocaleTables& locale, std::string requested_mission_id,
        long mission_frames = 0, GameFrameProfiler* profiler = nullptr,
        std::string language = {}, long mission_complete_frame = -1,
        GameHudHost* hud = nullptr, long order_frame = -1, float order_throttle = 0.0f,
        float order_rudder = 0.0f, float mission_frame_seconds = 0.0f,
        std::string trajectory_csv = {}, std::string order_command = {},
        std::string order_command_target = {}, float order_speed = 0.0f,
        bool order_speed_set = false);
    ~GameMissionHost();
    GameMissionHost(const GameMissionHost&) = delete;
    GameMissionHost& operator=(const GameMissionHost&) = delete;

    // Milestone 2n, --order-unit <name>: the created instance the command form
    // of --order is issued to, instead of the controlled unit.
    void set_order_unit(std::string unit);

    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>: the labelled
    // diagnostic stand-in for the state step, engaged on --order-frame.
    void set_ai_drive(std::string unit, float throttle, float rudder);

    // True when --menu-select named a mission, so the run drives the path.
    bool requested() const noexcept;

    // 005caaf0, the mission-tree screen's slot +10h override (screen id 2).
    // Bind before any load. Services and native-owner backing must outlive
    // this host and all its retained records. COM sprite textures do not
    // satisfy the native +04/current0 texture contract.
    void bind_mission_picture_services(const MissionPictureTextureServices&);
    void load_mission_tree_005caaf0();
    // 005861b0, the main-menu screen's slot +14h override: FE_worldmap_historical,
    // FE_briefing_grid and FE_briefing, and the fourteen widget handles 0058c010
    // drives. `page` is the screen's own FE_main, already loaded by the caller.
    void bind_main_menu_layout_005861b0(GuiLayoutPage* main_page);
    // 00584ae0's seven list entries, with the locale text of each label id.
    void build_top_level_page_00584ae0();

    // One frame of the scripted selection, run after the main-menu path has
    // published the screen. Returns true while there is still work to do.
    bool advance(float seconds);

    const GameMissionSummary& summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
