#pragma once
// bsp_game.exe milestone 2f: the rest of the mission load, the mission state
// entry and the in-mission frame, as process bindings.
//
// Addresses: 004dfb70 with the ordered host inventory of
// bsp/mission_load_path.hpp (004dc6a0, 00874640, 006ad600, 0046df00, 004de610,
// 00951560, 005e2f00, 004f2800, 004d4df0, 0068a990, 00aa0d30, 00a92c40,
// 008860b0, 0045f440 / 0045f520, 00f8d394, 007065e0 and the rest), 004db920
// (BSP_Game_UpdateDeviceWaitScreen) and 004da6c0 (BSP_Game_EnterMissionState)
// with 00447060 and 004d87b0, and the in-mission branch of BSP_Game_OnMove
// 004e4a40 with the 57 call sites bsp::mission_frame_step() carries.
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::MissionSceneLoadHost, bsp::MissionStateEntryHost,
// bsp::MissionDeviceWaitHost, bsp::MissionFrameHost, bsp::WorldTickHost,
// bsp::InputTickHost, bsp::GameFrameControlHost or bsp::MenuRequestServicer,
// satisfied either by a reconstruction already on main or by the explicit
// unimplemented policy in GameHostLog.
//
// Evidence: docs/MISSION_LOAD_PATH.md, docs/MISSION_SCENE_LOAD.md,
// docs/MISSION_STATE_ENTRY.md, docs/MISSION_STATE_FRAME.md,
// docs/MISSION_LUA_MACHINE.md, docs/MISSION_RESULT_DECISION.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/game_hosts_scene_contents.hpp"

namespace bsp {
class SessionParticipantPools;
struct SceneRecord;
}

namespace bsp::game {

class GameHostLog;
class GameVfsHost;
class GameMissionLuaHost;
class GameFrameProfiler;
class GameHudHost;

// What the walk of the load inventory did. `concrete` counts the steps a
// reconstruction performed in process; `records` counts the steps that took the
// unimplemented policy because their owner area has no reconstruction here.
struct GameMissionLoadRunSummary {
    bool ran{false};
    std::size_t steps{0};
    std::size_t concrete{0};
    std::size_t records{0};
    std::size_t skipped_arms{0};      // steps whose native arm is not taken here
    std::uint32_t state_after{0};     // game+5D4h, 0Ch once the load finishes
    std::string short_name;           // 004cd7f0
    std::vector<std::string> blocks;  // the five numbered VFS file blocks
    std::string script_name;          // record+928h + slot*8
    std::int32_t script_slot{0};      // what normalise_mission_script_slot chose
    bool engine_movie_arm{false};     // 004e0a50
    std::size_t locale_tables{0};     // names split off record+980h
    std::size_t slots_reset{0};       // the eight records at game+1008h
    bool participant_scene_present{false};
    bool participant_count_available{false};
    std::int32_t participant_scene_slots{0}; // actual record+988h, only when available
    bool participant_local_claimed{false};  // current[0] selects player record 0
    // Milestone 2m: the function-valued Lua globals 004d30f0 recorded into the
    // set at game+1930h, the baseline teardown nils the mission's own additions
    // against.
    std::size_t scripted_names{0};
    std::int32_t mission_id{0};       // record+1098h, published to [00f8a2fc]+48h
};

// The 0Ch handler and the entry itself.
struct GameMissionEntrySummary {
    bool device_wait_ran{false};
    bool participant_view_available{false};
    bool player_count_arm{false};
    std::size_t unbound_player_slots{0}; // meaningful only when view is available
    bool entered{false};             // 004da6c0 wrote 0Dh
    std::uint32_t state{0};          // game+5D4h afterwards
    std::size_t dynamics_released{0};// 00447060
    std::size_t managers_created{0}; // the load's own count, for the log line
    std::string interface_request;   // what 004c9ca0 was asked for
    bool cinematic_cleared{false};   // 004cd0f0(0, 0, 1)
};

// One headless mission frame, and the totals over the run.
struct GameMissionFrameRunSummary {
    long requested{0};
    unsigned long long frames{0};
    unsigned long long simulated{0};
    unsigned long long paused{0};
    unsigned long long units_ticked{0};
    unsigned long long mission_events_applied{0};
    unsigned long long script_calls{0};
    unsigned long long input_entries_erased{0};
    unsigned long long menu_drain_iterations{0};
    unsigned long long interface_updates{0};
    bool completion_requested{false}; // 004d7ea0 enqueued request 0Fh
    bool exit_reachable{false};       // the debrief path could be entered
    std::string exit_note;
    // Milestone 2g. `exit_frames` are the frames run after the mission left
    // game state 0Dh; `exit_completed` says the drain reached request 04h.
    unsigned long long exit_frames{0};
    bool exit_completed{false};
    bool complete_injected{false};    // --mission-complete-frame fired
    // Milestone 2i: the world walk and the units under it.
    unsigned long long world_walks{0};
    unsigned long long entities_walked{0};
    unsigned long long entities_updated{0};
    unsigned long long unit_motion_ticks{0};
    std::size_t units{0};
    std::size_t merged_unit_list{0};
    float simulated_seconds{0.0f};
    float controlled_distance{0.0f};
    float total_path_length{0.0f};
    std::string controlled_unit;
    bool player_order_issued{false};
};

// Everything milestone 2f adds behind the mission load request. Owned for the
// whole run, because the Lua machine 004dd627 builds is per process and the
// slot records the load fills are read again by the entry and the frame.
class GameMissionFrameHost {
public:
    GameMissionFrameHost(GameHostLog& log, GameVfsHost& vfs, GameMissionLuaHost& lua,
        bsp::SessionParticipantPools& participants, GameFrameProfiler* profiler,
        std::string language, GameHudHost* hud = nullptr);
    ~GameMissionFrameHost();
    GameMissionFrameHost(const GameMissionFrameHost&) = delete;
    GameMissionFrameHost& operator=(const GameMissionFrameHost&) = delete;

    // Walks the recovered load inventory in order from the step the mission
    // path's request 0Ah dispatches, performing every step whose owner area has
    // a reconstruction this process can drive and recording the rest.
    // `scene_path` is the record's +90Ch, `script_name` the record's +928h entry
    // and `locale_tables` the comma separated list at +980h. `scene_record`
    // supplies actual presence. `participant_count` is the separately parsed
    // +988h scalar for that record's path; null means unavailable. A present
    // scene requires that scalar (the still-unfilled side_blocks vector does
    // not establish zero). The
    // borrowed pools supplied to the constructor must outlive this host/ShipAI.
    void run_scene_load_004dfb70(const std::string& scene_path, const std::string& script_name,
        const std::string& locale_tables, std::int32_t mission_id,
        const bsp::SceneRecord* scene_record, const std::int32_t* participant_count);

    // 004db920 for game state 0Ch, which tails into 004da6c0.
    bool enter_mission_state_004da6c0();

    // One frame of 004e4a40: the request drain, the state 11h arm, and then
    // either the in-mission branch or, once the mission has left game state
    // 0Dh, one frame of the exit path. Returns false when the path is over.
    bool run_mission_frame_004e4a40(float raw_delta);

    // Milestone 2g, --mission-complete-frame N: the in-mission frame on which
    // the executable makes the call a mission script's end-movie binding makes,
    // 0089a480 -> 0089a390 -> 004cd390 with the debrief byte set. Negative or
    // zero injects nothing and the run ends on the frame count as 2f did.
    void set_mission_complete_frame(long frame) noexcept;

    // Milestone 2i, --order-frame N with --order throttle=<f>,rudder=<f>: the
    // in-mission frame on which one player order is issued to the controlled
    // unit, through the same 00816a40 the authored command takes. A negative
    // frame issues nothing.
    void set_player_order(long frame, float throttle, float rudder) noexcept;
    // Milestone 2l: --order <command>[:<entity>] on the same --order-frame,
    // issued through 0046aab0 -> 0077d600 instead of through the order ring.
    void set_player_command(std::string token, std::string target, std::string unit = {});
    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>: the labelled
    // diagnostic stand-in for the eight state steps that have no body. Engaged
    // on the same --order-frame as the player order.
    void set_ai_drive(std::string unit, float throttle, float rudder);
    // Milestone 2m: --order speed=<m/s> on the same --order-frame, the store
    // luaMW_SetShipSpeed 00890d30 makes on *(unit+73Ch) +24h / +28h.
    void set_player_commanded_speed(float speed) noexcept;
    // Milestone 2i, --mission-frame-seconds S: a fixed in-mission frame delta
    // instead of the wall clock. Zero keeps the wall clock.
    void set_mission_frame_seconds(float seconds) noexcept;
    // Milestone 2j, --trajectory-csv <path>: one row per unit per fixed
    // simulation step. An empty path writes nothing. The file is opened on the
    // first step that has units and closed when the run reports.
    void set_trajectory_csv(std::string path);

    // game+2198h, the mission key the record commit 009205e0 writes under.
    void set_mission_key(std::string key);

    // True while game+5D4h still holds 0Dh.
    bool in_mission_phase() const noexcept;
    // True once the exit path has run to the front-end request or its bound.
    bool exit_path_finished() const noexcept;

    // Logs the per-frame totals and the exit disposition.
    void report(long requested_frames);

    const GameMissionLoadRunSummary& load_summary() const noexcept;
    const GameMissionEntrySummary& entry_summary() const noexcept;
    const GameMissionFrameRunSummary& frame_summary() const noexcept;
    // Milestone 2h: what the two scene-file passes of 004d4df0 did. Null when
    // the load has not reached that row.
    const GameSceneContentsSummary* scene_contents_summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
