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

namespace bsp::game {

class GameHostLog;

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
    GameMissionResultHost(GameHostLog& log, bsp::GameFrameControlState& control);
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
    void play_bink_movie_0089a480(const std::string& script_movie_name, bool go_to_debrief);

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

    void record(const char* method, std::uint32_t address);
    void done(const char* method, std::uint32_t address);

    struct MovieBridge;  // bsp::MoviePlayerHost, defined in the source
    struct CompletionBridge;  // bsp::GameMovieCompletionHost

    GameHostLog& log_;
    bsp::GameFrameControlState& control_;
    std::unique_ptr<MovieBridge> movie_bridge_;
    std::unique_ptr<CompletionBridge> completion_bridge_;

    bsp::MissionScoringManagerState scoring_{};            // [game+21A0h]
    bsp::MissionProgress progress_{};                      // game+6B4h
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
