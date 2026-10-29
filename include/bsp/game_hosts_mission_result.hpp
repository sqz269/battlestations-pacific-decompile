#pragma once
// bsp_game.exe milestone 2g: the mission result object and the way out of game
// state 0Dh.
//
// Addresses: 0089a480 (the `PlayBinkMovie` binding), 0089a390 (the "movies/"
// prefix, the 004cd390 call and the synchronous 004d7970 it makes when the
// script asks for the debrief), 004cd390 (the 24h result object at game+7188h),
// 004d7ea0 (the completion poll), 004d7970 (GGame::EndScene) with 009205e0 (the
// record commit), the drain arm 004e458a for request 10h with 004da780,
// 004cd610 and 004cc510, the movie pair 004f8a20 / 004f8970 and the default
// completion 004f89d0 -> 004c7ed0, and 004e504b (the state 11h arm that
// enqueues request 04h).
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::MissionResultHost, bsp::MoviePlayerHost or
// bsp::GameMovieCompletionHost, satisfied either by a reconstruction already on
// main (bsp/mission_result.hpp, bsp/game_frame_control.hpp, bsp/movie_player.hpp)
// or by the explicit unimplemented policy in GameHostLog.
//
// Evidence: docs/MISSION_RESULT_DECISION.md, docs/GAME_FRAME_CONTROL.md,
// docs/GAME_MOVIE_PLAYER.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "bsp/game_frame_control.hpp"
#include "bsp/mission_progress.hpp"
#include "bsp/mission_result.hpp"
#include "bsp/movie_player.hpp"

namespace bsp {
class LocaleTables;
}

namespace bsp::game {

class GameHostLog;
class GameProfileOwner;
class GameVfsHost;

// Packet cc13-fix20. Scoring_SetMissionCompleted, the Lua row 008B8AD0
// (registered at 00E0C380). OFF keeps the row UNIMPLEMENTED. ON runs the
// reconstruction run_set_mission_completed_008b8ad0: the slot is game+18ECh in
// mode 0 (game+1FE4h) and the first argument otherwise, the flag is the
// argument at that position as lua_toboolean (00B66250) and defaults to true,
// and 00906460 stores it at record+0h with the clock 00F876A4 at record+10h on
// the 0 -> 1 edge only. The record is scoring_.slots[slot], the one 004D7970's
// commit 009205E0 copies into progress_. Divergence, labelled: 00906460's walk
// of [[game+19CCh]+28h] calling 00927F60 per entity with +180h < 8 (stamp the
// clock at entity+1D0h, give five flight members the same value, and pass the
// elapsed time clock - old +1D0h to 0090ED40, which does nothing when
// [game]+1FE4h == 2 (0090ED45 CMP / 0090ED4F JE 0090ED8C) and otherwise adds it into slot
// entity+1ACh's saved unit_usage map, record+114h keyed by entity+170h's
// vtable[0]() (0090ED74 LEA ECX,[slot*284h + scoring + 118h], the records
// starting at scoring+4h; 0090E740 subscript, float add); 0091A080 merges that
// map into the progress record (0091AB5E ADD ECX,114h; 0090E740; float add),
// which the archive writes as section
// "unit_usage") is one recorded step; the host has no entity list and does not
// write the unit_usage map.
// The binding returns 0 results (00B66400). No 00BD2F10 draw.
// ON (cc9-lua67, SQUADRON_LAND_TASK 5ik).
inline constexpr bool kLuaScoringSetMissionCompletedBound = true;

// Packet cc13-fix20. PlayBinkMovie, the Lua row 0089A480 (registered at
// 00E0BA60). OFF keeps the row UNIMPLEMENTED. ON runs what 0089A480 does after
// reading arg 0 as the name (00B662B0) and, only when there are more than two
// arguments, arg 2 as the debrief byte (default false): 0089A390 prefixes
// "movies/", 004CD390 replaces the result object at game+7188h, and with the
// byte set 004D7970(0) runs before the binding returns. The script does not
// wait: the call is synchronous and returns 0 results. No clip is played here,
// as before: the poll 004D7EA0 and the request-10h arm record 004F8A20 and the
// executable runs the completion 004F89D0 itself (milestone 2g). No 00BD2F10 draw.
// ON (cc9-lua67, SQUADRON_LAND_TASK 5ik).
inline constexpr bool kLuaPlayBinkMovieBound = true;

// What the exit path did, for the report.
struct GameMissionExitSummary {
    bool movie_requested{false};      // 0089a480 -> 004cd390 ran
    std::string movie_name;           // the "movies/" prefixed name at result+14h
    bool requests_debrief{false};     // result+21h
    bool end_scene_ran{false};        // 004d7970's body
    bool committed_record{false};     // 009205e0
    bool enqueued_teardown{false};    // request 10h
    bool poll_ran{false};             // 004d7ea0 got past its two guards
    bool poll_enqueued_debrief{false};// the poll's own request 0Fh
    bool teardown_arm_ran{false};     // the drain's 10h arm
    bool movie_completion_ran{false}; // 004f89d0 -> 004c7ed0
    bool mission_end_wait_ran{false}; // 004e504b cleared the suspension
    bool front_end_requested{false};  // request 04h enqueued
    bool front_end_entered{false};    // request 04h dispatched to 004e4000
    std::uint32_t state_after{0};     // game+5D4h when the path finished
    std::size_t steps{0};             // host steps logged on the path
};

// The producer half of docs/MISSION_RESULT_DECISION.md, owned by the process for
// the whole mission: the eight 284h scoring records at [game+21A0h]+4h, the
// mission progress object at game+6B4h, the eight objective sets at game+21A4h
// and the 24h end-of-mission request at game+7188h.
class GameMissionResultHost final : public bsp::MissionResultHost {
public:
    // Borrow the mission's same VFS and live locale owners; both must outlive
    // this result and every movie callback. No provider or snapshot is created.
    GameMissionResultHost(GameHostLog& log, bsp::GameFrameControlState& control,
        GameVfsHost& vfs, const bsp::LocaleTables& locale);
    ~GameMissionResultHost() override;
    GameMissionResultHost(const GameMissionResultHost&) = delete;
    GameMissionResultHost& operator=(const GameMissionResultHost&) = delete;

    // What the mission start wrote: the key 009205e0 commits under (game+2198h)
    // and the session mode game+1FE4h reads.
    void set_mission_key(std::string key);
    void set_session_mode(std::uint32_t mode) noexcept;
    void set_mission_clock(float seconds) noexcept;
    // game+1EE3h, written by the state entry 004da71e. The drain's 10h arm
    // tests it at 004e4715 and takes the early-return sub-path when it is set.
    void set_session_networked(bool networked) noexcept;

    // 0089a480 -> 0089a390 -> 004cd390, the path a mission script takes when it
    // calls PlayBinkMovie(name, goToDebrief). When `go_to_debrief` is set,
    // 0089a390 also calls 004d7970(0) before it returns, which is why the poll
    // 004d7ea0 never sees an empty request queue on that frame.
    // This entry is the --mission-complete-frame injection: the binding itself is
    // recorded, because no script made the call.
    void play_bink_movie_0089a480(const std::string& script_movie_name, bool go_to_debrief);

    // Packet cc13-fix20, behind kLuaPlayBinkMovieBound: the same path entered
    // from a script's PlayBinkMovie row, with the arguments 0089a480 read.
    void lua_play_bink_movie_0089a480(const std::string& script_movie_name, bool go_to_debrief);

    // Packet cc13-fix20, behind kLuaScoringSetMissionCompletedBound: a script's
    // Scoring_SetMissionCompleted row. The caller passes the argument count
    // (00b663f0), arg 0 as an integer (00b66290) and args 0 and 1 as
    // lua_toboolean (00b66250); which of them 008b8ad0 uses depends on the mode
    // game+1FE4h, which is this object's, as is the local slot game+18ECh.
    void lua_scoring_set_mission_completed_008b8ad0(int argument_count, int argument0_integer,
        bool argument0_boolean, bool argument1_boolean);

    // 004d7970 on its own, for the drain's request 0Fh.
    void run_end_scene_004d7970(bool aborted);

    // The drain's request 10h arm, 004e458a. Returns true when the native takes
    // the sub-path at 004e48d7 that returns without the drain tail.
    bool run_teardown_arm_004e458a(bsp::GameFrameControlState& state);

    // 004f89d0. The movie player is another owner's, so no clip ever ends and
    // nothing would call the completion the teardown arm registered; the
    // executable runs it itself, which is the substitution milestone 2e already
    // makes for the mission-detail movie.
    void run_movie_completion_004f89d0();

    // game+7188h as bsp::MissionResult, what the frame's poll reads.
    bsp::MissionResult result_projection() const noexcept;
    // 004cd610(1) then 004cc510, the poll's own release.
    void release_result_004cc510();

    // The two steps the frame bookkeeping owns: 004e504b clearing the drain
    // suspension and enqueuing request 04h, and the drain dispatching it.
    void note_mission_end_wait(bool enqueued_front_end) noexcept;
    void note_front_end_entered() noexcept;

    void report();
    const GameMissionExitSummary& summary() const noexcept { return exit_; }
    // cc15 lane L2 (kProfileOwnerCc15Bound): read [game+6B4h] through the
    // profile owner at each use instead of progress_. The owner outlives this.
    void bind_profile(GameProfileOwner* profile) noexcept { profile_ = profile; }

private:
    // --- bsp::MissionResultHost -------------------------------------------
    bsp::MissionScoreRecord& scoring_slot(int slot) override;
    float mission_clock() override;
    void notify_scoring_entities_00927f60() override;
    std::optional<bsp::MissionEndMovieRequest>& end_movie_slot() override;
    std::size_t pending_state_requests() override;
    void tick_world_zero_delta() override;
    void flush_entity_activations_004c3cb0() override;
    void play_end_movie(const bsp::MissionEndMovieRequest& request) override;
    void enqueue_state_request(std::uint32_t request) override;
    void set_drain_suspended(bool suspended) override;
    void release_end_movie_004cc510() override;
    void set_scene_ended_by_abort(bool aborted) override;
    void debrief_bringup_00920a20(bool aborted) override;
    bsp::MissionProgress& mission_progress() override;
    const std::string& current_mission_key() override;
    int commit_slot() override;
    bool multiplayer_score_accumulates() override;
    void adjust_mission_start_counters_004bcaa0() override;
    void flush_storage_007fa1b0() override;
    void reset_player_records_00916980() override;
    void broadcast_end_scene() override;
    void send_secondary_end_message() override;
    void record_metrics_007556a0() override;
    void set_end_scene_body_done() override;
    bsp::MissionObjectiveSet& objective_set(std::size_t index) override;
    int game_local_player_slot() override;
    std::uint32_t local_player_mode() override;
    bool slot_has_peer(int slot) override;
    void announce_objective_sound(int sound) override;
    void refresh_objective_markers_008dfe50(const bsp::MissionObjectiveEntry& entry) override;
    void replicate_objective_state(int slot, const bsp::MissionObjectiveEntry& entry,
        bsp::MissionObjectiveStatus status) override;
    void clear_hud_objective_dirty_byte() override;

    // 0089a390: the prefix, 004cd390 and, with the byte set, 004d7970(0).
    void play_named_movie_0089a390(const std::string& script_movie_name, bool go_to_debrief);

    void record(const char* method, std::uint32_t address);
    void done(const char* method, std::uint32_t address);

    struct MovieBridge;  // bsp::MoviePlayerHost, defined in the source
    struct CompletionBridge;  // bsp::GameMovieCompletionHost

    GameHostLog& log_;
    bsp::GameFrameControlState& control_;
    std::unique_ptr<MovieBridge> movie_bridge_;
    std::unique_ptr<CompletionBridge> completion_bridge_;

    bsp::MissionScoringManagerState scoring_{};            // [game+21A0h]
    bsp::MissionProgress progress_{};                      // game+6B4h stand-in
    GameProfileOwner* profile_{nullptr};                   // game+650h when bound
    bsp::MissionObjectiveSet objectives_[bsp::kObjectiveSetCount]{};  // game+21A4h
    std::optional<bsp::MissionEndMovieRequest> end_movie_{};  // game+7188h
    bsp::MoviePlayer movie_player_{};                      // 00e18d48
    bsp::GameMovieCompletionState movie_state_{};          // game+7184h, game+5ECh
    std::string mission_key_{};                            // game+2198h
    std::uint32_t session_mode_{0};                        // game+1FE4h
    float mission_clock_{0.0f};                            // 00f876a4
    bool scene_ended_by_abort_{false};                     // game+1EE1h
    bool end_scene_body_done_{false};                      // game+1EE2h
    bool session_networked_{false};                        // game+1EE3h
    GameMissionExitSummary exit_{};
};

}  // namespace bsp::game
