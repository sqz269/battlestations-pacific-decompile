// bsp_game.exe milestone 2g: the mission result object and the exit from game
// state 0Dh.
//
// Milestone 2f ended with `mission exit reachable=0`: 004dfe83 released the
// previous result during the load, nothing built a new one, and the poll
// 004d7ea0 therefore returned on its first test every frame. This file builds
// the result the way the game does, through the routine the mission script's
// `PlayBinkMovie` binding reaches, and runs the recovered path out of the
// mission over the reconstructions on main.
//
// Three things here are the executable's own and are marked where they are done:
//
// 1. The call itself. No script on this installation calls PlayBinkMovie in a
//    headless run with no enemy contact, so --mission-complete-frame N stands in
//    for the script's call, the same substitution --press-start-frame makes for
//    the title page. Everything after the call is recovered behaviour.
// 2. The movie completion. 004f8a20 needs the GUI movie widget, which is the
//    front-end owner's, so no clip ever plays and nothing would call the
//    completion 004f8970 registered. The executable runs 004f89d0 itself, which
//    is what milestone 2e already does for the mission-detail preview movie.
// 3. The scoring record is whatever the mission left behind. Nothing here calls
//    Scoring_SetMissionCompleted 008b8ad0: that is the script's, and this
//    mission's stage init never reached it, so the record 009205e0 commits is
//    the zeroed one the load produced.
//
// See include/bsp/game_hosts_mission_result.hpp for the address list.

#include "bsp/game_hosts_mission_result.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/native_string.hpp"

#include <cstdio>

namespace bsp::game {
namespace {

// 00ce77e4, the float 004cd390 stores at result+8h (MOVSS at 004cd420). The
// image holds 00 00 48 C3 = -200.0f, and 004d7f1d hands it to 004f8a20 as the
// local Z of the movie widget; the drain's own teardown arm loads 1000.0f from
// 00ce3804 for the same argument. See the corrections in docs/GAME_EXECUTABLE.md.
constexpr float kEndMovieLocalZ = -200.0f;

// 0089a390's prefix: BSP_NativeString_Resize(7, 1) at 0089a3c7 then the concat
// at 0089a3fc, which is seven characters plus the script's name.
constexpr const char* kMoviePrefix = "movies/";

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

}  // namespace

// ---------------------------------------------------------------------------
// The two movie boundaries. Both exist so the recovered completion 004f89d0 can
// run; neither stands in for a movie player.
// ---------------------------------------------------------------------------

struct GameMissionResultHost::MovieBridge final : public bsp::MoviePlayerHost {
    explicit MovieBridge(GameHostLog& log) : log_(log) {}

    // --- bsp::MovieWidgetHost ---------------------------------------------
    bsp::NativeStringStorage& movie_string_storage() override {
        return bsp::crt_string_storage();
    }
    void ensure_movie_resources(bsp::MovieWidgetState&) override {
        log_.unimplemented("MoviePlayer::ensure_resources", "00aae770");
    }
    void prepare_movie_subtitles(bsp::MovieWidgetState&, const bsp::NativeString&) override {
        log_.unimplemented("MoviePlayer::prepare_subtitles", "00ab0520");
    }
    bool decoder_open(void*, const bsp::NativeString&, bool, int) override {
        log_.unimplemented("MoviePlayer::decoder_open", "00aae770+vtable0c");
        return false;
    }
    int missing_movie_retry_dialog() override {
        log_.unimplemented("MoviePlayer::missing_movie_dialog", "00ab0eb0");
        return 0;
    }
    void decoder_set_volume_immediate(void*, float) override {
        log_.unimplemented("MoviePlayer::decoder_volume", "00a4cbd0");
    }
    void decoder_set_loop(void*, std::uint8_t) override {
        log_.unimplemented("MoviePlayer::decoder_loop", "00aae770+vtable28");
    }
    void decoder_prepare_frame(void*) override {
        log_.unimplemented("MoviePlayer::decoder_prepare_frame", "00aae770+vtable34");
    }
    void bind_movie_textures(bsp::MovieWidgetState&) override {
        log_.unimplemented("MoviePlayer::bind_textures", "00aac820");
    }
    void decoder_set_running(void*, bool) override {
        log_.unimplemented("MoviePlayer::decoder_running", "00aae770+vtable1c");
    }
    void decoder_close(void*) override {
        log_.unimplemented("MoviePlayer::decoder_close", "00aae770+vtable10");
    }
    bool decoder_completed(void*) override {
        log_.unimplemented("MoviePlayer::decoder_completed", "00aae770+vtable20");
        return true;
    }
    void decoder_mark_completed(void*, bool) override {
        log_.unimplemented("MoviePlayer::decoder_mark_completed", "00aae770+vtable24");
    }
    void decoder_update(void*) override {
        log_.unimplemented("MoviePlayer::decoder_update", "00aae770+vtable2c");
    }
    bool decoder_has_open_handle(void*) override { return false; }
    void update_gui_widget(bsp::MovieWidgetState&, float) override {
        log_.unimplemented("MoviePlayer::update_widget", "00aa87b0");
    }
    void update_movie_subtitles(bsp::MovieWidgetState&) override {
        log_.unimplemented("MoviePlayer::update_subtitles", "00aadba0");
    }
    void clear_movie_subtitles(bsp::MovieWidgetState&) override {
        log_.unimplemented("MoviePlayer::clear_subtitles", "00ab00c0");
    }

    // --- bsp::MoviePlayerHost ---------------------------------------------
    void* load_movie_page(const char*, int, int) override {
        log_.unimplemented("MoviePlayer::load_page", "00aa5840");
        return nullptr;
    }
    void* find_movie_page_child(void*, const char*, bool) override {
        log_.unimplemented("MoviePlayer::find_page_child", "00aa7e00");
        return nullptr;
    }
    void set_widget_visible(void*, bool) override {
        // 004f8ac0's own second statement, reached by the completion below with
        // a null backdrop handle because no movie page was ever loaded.
        log_.unimplemented("MoviePlayer::set_widget_visible", "00aa8530");
    }
    void set_widget_local_z(void*, float) override {
        log_.unimplemented("MoviePlayer::set_widget_local_z", "00aa7910");
    }
    bsp::MovieWidgetState& movie_widget_state(void*) override {
        log_.unimplemented("MoviePlayer::widget_state", "00e18d48+20");
        return widget_;
    }
    bool platform_flag_0d() override { return false; }
    bool mission_context_present() override { return false; }
    bool movie_input_action_pressed(int) override { return false; }
    void notify_movie_frame(int) override {
        log_.unimplemented("MoviePlayer::notify_frame", "004c1e90");
    }
    void release_movie_page(void*) override {
        log_.unimplemented("MoviePlayer::release_page", "00aa31f0");
    }
    void destroy_movie_callback_owner(bsp::MoviePlayer&) override {
        log_.unimplemented("MoviePlayer::destroy_callback_owner", "00695870");
    }

    GameHostLog& log_;
    bsp::MovieWidgetState widget_{};
};

struct GameMissionResultHost::CompletionBridge final : public bsp::GameMovieCompletionHost {
    CompletionBridge(GameHostLog& log, bsp::GameFrameControlState& control)
        : log_(log), control_(control) {}

    int current_game_state() override {
        // 004f89d6 and again at 004f89f6: [game+5D4h], reread after the stop.
        return static_cast<int>(control_.state);
    }
    void set_movie_input_context(int context, bool enabled) override {
        // 00a933f0 with context 10h and 0, inside 004c7ed0 at 004c7ee5.
        static_cast<void>(context);
        static_cast<void>(enabled);
        log_.unimplemented("MovieCompletion::set_input_context", "00a933f0");
    }
    void set_movie_cinematic_mode(bool hide, std::uint8_t allow_simulation,
        bool argument_3) override {
        static_cast<void>(hide);
        static_cast<void>(allow_simulation);
        static_cast<void>(argument_3);
        log_.unimplemented("MovieCompletion::set_cinematic_mode", "004cd0f0");
    }

    GameHostLog& log_;
    bsp::GameFrameControlState& control_;
};

// ---------------------------------------------------------------------------
// GameMissionResultHost
// ---------------------------------------------------------------------------

GameMissionResultHost::GameMissionResultHost(GameHostLog& log,
    bsp::GameFrameControlState& control)
    : log_(log), control_(control),
      movie_bridge_(std::make_unique<MovieBridge>(log)),
      completion_bridge_(std::make_unique<CompletionBridge>(log, control)) {}

GameMissionResultHost::~GameMissionResultHost() = default;

void GameMissionResultHost::record(const char* method, std::uint32_t address) {
    char text[16];
    format_address(address, text);
    log_.unimplemented(method, text);
    ++exit_.steps;
}

void GameMissionResultHost::done(const char* method, std::uint32_t address) {
    char text[16];
    format_address(address, text);
    log_.implemented(method, text);
    ++exit_.steps;
}

void GameMissionResultHost::set_mission_key(std::string key) {
    mission_key_ = std::move(key);
}

void GameMissionResultHost::set_session_mode(std::uint32_t mode) noexcept {
    session_mode_ = mode;
}

void GameMissionResultHost::set_mission_clock(float seconds) noexcept {
    mission_clock_ = seconds;
}

void GameMissionResultHost::set_session_networked(bool networked) noexcept {
    session_networked_ = networked;
}

// --- the script's call, 0089a480 -> 0089a390 -------------------------------

void GameMissionResultHost::play_bink_movie_0089a480(const std::string& script_movie_name,
    bool go_to_debrief) {
    // 0089a480 is the binding registered at 00e0ba60 as {"PlayBinkMovie",
    // 0089a480}; it reads the two Lua arguments and tails into 0089a390. The
    // arguments here are the executable's, not a script's.
    record("MissionEndMovie::lua_binding_play_bink_movie", 0x0089a480u);

    // 0089a3c7 and 0089a3fc: a seven-character native string and the concat that
    // prefixes it onto the script's name.
    const std::string name = std::string(kMoviePrefix) + script_movie_name;
    done("MissionEndMovie::prefix_movie_name", 0x0089a3fcu);

    // 0089a40e, CALL 004cd390 with the concatenated name and the debrief byte.
    bsp::set_end_of_mission_movie_004cd390(end_movie_, name, go_to_debrief, kEndMovieLocalZ);
    done("MissionEndMovie::set_end_of_mission_movie", 0x0089a40eu);
    exit_.movie_requested = true;
    exit_.movie_name = name;
    exit_.requests_debrief = go_to_debrief;
    log_.notef("mission end movie requested: name=\"%s\" go_to_debrief=%d "
        "result+8h=%.1f (00ce77e4, the movie's local Z at 004f8a20)", name.c_str(),
        go_to_debrief ? 1 : 0, static_cast<double>(kEndMovieLocalZ));

    // 0089a45c..0089a469: with the debrief byte set, 0089a390 calls EndScene
    // itself, before the poll ever sees the request. That is why a mission that
    // ends through PlayBinkMovie(name, true) in state 0Dh enqueues 10h here and
    // never reaches the poll's own 0Fh.
    if (go_to_debrief) {
        run_end_scene_004d7970(false);
    }
}

void GameMissionResultHost::run_end_scene_004d7970(bool aborted) {
    bsp::EndSceneInputs inputs{};
    inputs.aborted = aborted;
    inputs.already_run = end_scene_body_done_;  // game+1EE2h
    inputs.local_player_mode = session_mode_;   // game+1FE4h
    inputs.current_request = control_.state;    // game+5D4h
    inputs.session_selector_218c = false;       // game+218Ch, unset in this process
    inputs.game_field_624 = 0;                  // game+624h

    const bool first = !end_scene_body_done_;
    const bsp::EndSceneDecision decision = bsp::run_end_scene_004d7970(*this, inputs);
    done("MissionEndScene::end_scene", 0x004d7970u);
    exit_.end_scene_ran = exit_.end_scene_ran || first;
    exit_.committed_record = exit_.committed_record || decision.commits_record;
    exit_.enqueued_teardown = exit_.enqueued_teardown || decision.enqueues_teardown;
    log_.notef("GGame::EndScene aborted=%d first_run=%d commit=%d teardown=%d "
        "broadcast=%d state=0x%02X", aborted ? 1 : 0, first ? 1 : 0,
        decision.commits_record ? 1 : 0, decision.enqueues_teardown ? 1 : 0,
        decision.broadcasts_end_scene ? 1 : 0, control_.state);
}

// --- the drain's request 10h, 004e458a -------------------------------------

bool GameMissionResultHost::run_teardown_arm_004e458a(bsp::GameFrameControlState& state) {
    exit_.teardown_arm_ran = true;
    // 004e4710, CALL 004da780. The mission teardown itself is the world and
    // front-end owners'; nothing of it is reconstructed.
    record("MissionTeardown::tear_down_mission", 0x004e4710u);

    // 004e4715: game+1EE3h, the networked byte the entry 004da71e wrote. A
    // networked session takes the sub-path at 004e48d7 that returns without the
    // drain tail; a single-player one does not.
    if (session_networked_) {
        record("MissionTeardown::networked_sub_path", 0x004e48d7u);
        return true;
    }

    // 004e4722: with a result object at game+7188h the arm plays its movie and
    // waits in state 11h; without one it asks for the front end directly.
    if (end_movie_.has_value()) {
        record("MissionTeardown::play_end_movie", 0x004e474au);   // 004f8a20, local Z 1000.0f
        movie_player_.allow_skip = true;                          // 004e4754, player+30h
        record("MissionTeardown::set_movie_completion", 0x004e4763u);  // 004f8970(004f89d0)
        record("MissionTeardown::close_result_gui", 0x004e476cu);      // 004cd610(1)
        end_movie_.reset();                                            // 004e4773, 004cc510
        done("MissionTeardown::release_result", 0x004e4773u);
        state.state = 0x11;            // 004e4778
        state.drain_suspended = true;  // 004e4782
        movie_state_.requests_held = true;
        movie_state_.movie_request_active = true;
        log_.notef("mission teardown: the end movie was handed to 004f8a20, the completion "
            "004f89d0 was registered, the result object was released and the game left "
            "state 0Dh for 11h with the drain suspended at game+5ECh");
        return false;
    }

    // 004e478b..004e47a7: no result object, so the arm enqueues request 04h when
    // the queue is empty.
    if (state.requests.count == 0) {
        bsp::enqueue_state_request_004d3ed0(state.requests, bsp::kStateRequestFrontEnd);
        exit_.front_end_requested = true;
        done("MissionTeardown::request_front_end", 0x004e47a7u);
    }
    return false;
}

void GameMissionResultHost::run_movie_completion_004f89d0() {
    // The completion the teardown arm registered. No clip is playing, so nothing
    // in this process would call it; running it here is the executable's own
    // substitution and is what lifts the drain suspension the arm raised.
    bsp::default_movie_completion_004f89d0(movie_state_, movie_player_, *movie_bridge_,
        *completion_bridge_);
    control_.drain_suspended = movie_state_.requests_held;
    exit_.movie_completion_ran = true;
    done("MissionTeardown::movie_completion", 0x004f89d0u);
    log_.notef("movie completion 004f89d0 -> 004c7ed0 ran: game+7184h=%d game+5ECh=%d "
        "(no movie player, so the executable runs the completion the arm registered)",
        movie_state_.movie_request_active ? 1 : 0, movie_state_.requests_held ? 1 : 0);
}

void GameMissionResultHost::note_mission_end_wait(bool enqueued_front_end) noexcept {
    exit_.mission_end_wait_ran = true;
    if (enqueued_front_end) exit_.front_end_requested = true;
}

void GameMissionResultHost::note_front_end_entered() noexcept {
    exit_.front_end_entered = true;
}

bsp::MissionResult GameMissionResultHost::result_projection() const noexcept {
    bsp::MissionResult projection{};
    if (end_movie_.has_value()) {
        projection.present = true;
        projection.requests_debrief = end_movie_->requests_debrief;
        projection.score = end_movie_->parameter_08;
    }
    return projection;
}

void GameMissionResultHost::release_result_004cc510() {
    if (end_movie_.has_value()) {
        end_movie_.reset();
        done("MissionCompletion::release_result", 0x004cc510u);
    }
}

// ---------------------------------------------------------------------------
// bsp::MissionResultHost
// ---------------------------------------------------------------------------

bsp::MissionScoreRecord& GameMissionResultHost::scoring_slot(int slot) {
    const std::size_t index = (slot >= 0 && static_cast<std::size_t>(slot)
        < bsp::kMissionScoringSlotCount) ? static_cast<std::size_t>(slot) : 0;
    return scoring_.slots[index];
}

float GameMissionResultHost::mission_clock() { return mission_clock_; }

void GameMissionResultHost::notify_scoring_entities_00927f60() {
    record("MissionScoring::notify_entities", 0x009064b5u);
}

std::optional<bsp::MissionEndMovieRequest>& GameMissionResultHost::end_movie_slot() {
    return end_movie_;
}

std::size_t GameMissionResultHost::pending_state_requests() {
    return control_.requests.count;
}

void GameMissionResultHost::tick_world_zero_delta() {
    record("MissionCompletion::world_zero_delta_tick", 0x00904bf0u);
}

void GameMissionResultHost::flush_entity_activations_004c3cb0() {
    record("MissionCompletion::flush_activations", 0x004c3cb0u);
}

void GameMissionResultHost::play_end_movie(const bsp::MissionEndMovieRequest& request) {
    static_cast<void>(request);
    record("MissionCompletion::play_end_movie", 0x004d7f32u);
}

void GameMissionResultHost::enqueue_state_request(std::uint32_t request) {
    bsp::enqueue_state_request_004d3ed0(control_.requests, request);
    done("MissionResult::enqueue_state_request", 0x004d3ed0u);
    log_.notef("state request 0x%02X enqueued, queue=%zu", request, control_.requests.count);
}

void GameMissionResultHost::set_drain_suspended(bool suspended) {
    control_.drain_suspended = suspended;
    movie_state_.requests_held = suspended;
}

void GameMissionResultHost::release_end_movie_004cc510() {
    record("MissionCompletion::close_result_gui", 0x004cc510u);
}

void GameMissionResultHost::set_scene_ended_by_abort(bool aborted) {
    scene_ended_by_abort_ = aborted;  // game+1EE1h
}

void GameMissionResultHost::debrief_bringup_00920a20(bool aborted) {
    static_cast<void>(aborted);
    // The debrief bring-up and its GUI_scoring page are packet
    // mission_debrief_bringup; nothing of either is reconstructed.
    record("MissionEndScene::debrief_bringup", 0x004d79b7u);
}

bsp::MissionProgress& GameMissionResultHost::mission_progress() { return progress_; }

const std::string& GameMissionResultHost::current_mission_key() { return mission_key_; }

int GameMissionResultHost::commit_slot() { return scoring_.commit_slot; }

bool GameMissionResultHost::multiplayer_score_accumulates() {
    // The 00f8a2fc virtual +198h query and the byte at [00f8a2fc]+4Dh. This
    // process publishes a mission id into that object and owns nothing else of
    // it, and the session is single player, so the answer is never read.
    log_.unimplemented("MissionEndScene::multiplayer_score_query", "00f8a2fc+vtable198");
    ++exit_.steps;
    return false;
}

void GameMissionResultHost::adjust_mission_start_counters_004bcaa0() {
    record("MissionEndScene::adjust_start_counters", 0x004d7a0fu);
}

void GameMissionResultHost::flush_storage_007fa1b0() {
    record("MissionEndScene::flush_storage", 0x004d7a23u);
}

void GameMissionResultHost::reset_player_records_00916980() {
    record("MissionEndScene::reset_player_records", 0x004d7a37u);
}

void GameMissionResultHost::broadcast_end_scene() {
    record("MissionEndScene::broadcast_end_scene", 0x004d7a78u);
}

void GameMissionResultHost::send_secondary_end_message() {
    record("MissionEndScene::send_secondary_message", 0x004d7accu);
}

void GameMissionResultHost::record_metrics_007556a0() {
    record("MissionEndScene::record_metrics", 0x004d7a42u);
}

void GameMissionResultHost::set_end_scene_body_done() {
    end_scene_body_done_ = true;  // game+1EE2h
}

bsp::MissionObjectiveSet& GameMissionResultHost::objective_set(std::size_t index) {
    return objectives_[index < bsp::kObjectiveSetCount ? index : 0];
}

int GameMissionResultHost::game_local_player_slot() { return 0; }

std::uint32_t GameMissionResultHost::local_player_mode() { return session_mode_; }

bool GameMissionResultHost::slot_has_peer(int slot) {
    static_cast<void>(slot);
    return false;
}

void GameMissionResultHost::announce_objective_sound(int sound) {
    static_cast<void>(sound);
    record("MissionObjectives::announce_sound", 0x008dd460u);
}

void GameMissionResultHost::refresh_objective_markers_008dfe50(
    const bsp::MissionObjectiveEntry& entry) {
    static_cast<void>(entry);
    record("MissionObjectives::refresh_markers", 0x008dfe50u);
}

void GameMissionResultHost::replicate_objective_state(int slot,
    const bsp::MissionObjectiveEntry& entry, bsp::MissionObjectiveStatus status) {
    static_cast<void>(slot);
    static_cast<void>(entry);
    static_cast<void>(status);
    record("MissionObjectives::replicate_state", 0x008dda20u);
}

void GameMissionResultHost::clear_hud_objective_dirty_byte() {
    record("MissionObjectives::clear_hud_dirty_byte", 0x008e1f80u);
}

void GameMissionResultHost::report() {
    exit_.state_after = control_.state;
    log_.notef("summary mission exit movie=%d name=%s debrief=%d end_scene=%d commit=%d "
        "teardown_request=%d teardown_arm=%d completion=%d end_wait=%d front_end=%d/%d "
        "state=0x%02X steps=%zu",
        exit_.movie_requested ? 1 : 0,
        exit_.movie_name.empty() ? "(none)" : exit_.movie_name.c_str(),
        exit_.requests_debrief ? 1 : 0, exit_.end_scene_ran ? 1 : 0,
        exit_.committed_record ? 1 : 0, exit_.enqueued_teardown ? 1 : 0,
        exit_.teardown_arm_ran ? 1 : 0, exit_.movie_completion_ran ? 1 : 0,
        exit_.mission_end_wait_ran ? 1 : 0, exit_.front_end_requested ? 1 : 0,
        exit_.front_end_entered ? 1 : 0, exit_.state_after, exit_.steps);
}

}  // namespace bsp::game
