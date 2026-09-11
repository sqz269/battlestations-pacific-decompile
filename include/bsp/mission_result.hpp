#pragma once

// Mission result decision, packet `mission_result_decision`.
// docs/MISSION_RESULT_DECISION.md carries the evidence for every offset,
// constant and comparison spelled here. Descriptive names are hypotheses, not
// recovered symbols.
//
// The native image does not decide that a mission is won or lost. It offers the
// mission script four write paths and one poll that turns the script's last
// write into a state transition; this header models those paths and the poll.

#include "bsp/mission_progress.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The objective record, constructor 008dd5c0 (operator new 2Ch at 008e1f96)
// ---------------------------------------------------------------------------

// objective+18h. 008dfe50 skips the HUD marker refresh for 2 (008dfe63) and
// 008e1d30 announces only 0 and 1; mission_progress.hpp persists six trees as
// allied primary/secondary/hidden then Japanese primary/secondary/hidden.
enum class MissionObjectiveKind : int {
    Primary = 0,
    Secondary = 1,
    Hidden = 2,
};

// objective+1Ch, initialised 0 at 008dd687. 008e20d0 stores 1 at 008e2181 and
// 008e2200 stores 2 at the matching site. Nothing else writes the field.
enum class MissionObjectiveStatus : int {
    Active = 0,
    Completed = 1,
    Failed = 2,
};

// Projection of the 2Ch native record. Offsets are evidence labels, not this
// struct's layout. The vtable at +0h and the unit list at +20h..+28h are not
// modelled here; docs names `objective_unit_list` as the follow-up for them.
struct MissionObjectiveEntry {
    std::string id;   // +4h length, +8h pointer
    std::string text; // +0Ch length, +10h pointer
    // +14h, the constructor's fifth argument. No decoded reader; carried raw.
    std::uint8_t flag_14{0};
    MissionObjectiveKind kind{MissionObjectiveKind::Primary}; // +18h
    MissionObjectiveStatus status{MissionObjectiveStatus::Active}; // +1Ch
};

// The per-slot set: this+14h is the local player slot, this+28h an MSVC
// std::list<Objective*> (node +0h next, +8h value). 008cd440 reaches eight of
// them through game+21A4h + 4*slot.
inline constexpr std::size_t kObjectiveSetCount = 8;

struct MissionObjectiveSet {
    int local_player_slot{0};                   // +14h
    std::vector<MissionObjectiveEntry> entries; // +28h, in list order
};

// The key comparison 008e20d0 and 008e2200 share: lengths must match first,
// then a both-empty / one-empty test, then __stricmp. Reproduced rather than
// simplified because the native order is observable through the length test.
bool objective_key_matches_008e20f6(const std::string& record_id,
                                    const std::string& key) noexcept;

// First matching entry in list order, or no index. Native walks head->next and
// stops at the first hit.
std::optional<std::size_t> find_objective_008e20f6(const MissionObjectiveSet& set,
                                                   const std::string& key) noexcept;

// 008e1d30's announcement gate and sound selection. Returns the value the
// native passes to 00432650 and 008dd460: 1 for completed, 2 for failed
// (`2 - (completed != 0)` at 008e1d4f). No value means the gate rejected it.
// The gate is: 0 <= local_slot < 8, set slot == local_slot, kind in {0,1}, and
// the `silent` byte clear.
std::optional<int> objective_announcement_sound_008e1d30(
    int set_local_player_slot, int game_local_slot_18ec, MissionObjectiveKind kind,
    bool completed, bool silent) noexcept;

// 008dfe50's entry test: the HUD marker refresh runs unless the objective is
// hidden (objective+18h == 2 at 008dfe63).
bool objective_refreshes_markers_008dfe50(MissionObjectiveKind kind) noexcept;

// Replication gate shared by 008e1f80, 008e20d0 and 008e2200: game+1FE4h == 1
// and [game+18CCh + 4*set+14h]+50h non-null.
bool objective_replicates_008e20d0(std::uint32_t local_player_mode,
                                   bool slot_has_peer) noexcept;

// 008e20d0 / 008e2200 applied to the set. Returns the index it changed, if any.
// The caller is responsible for the announcement and the replication, which the
// host below exposes as separate call sites.
std::optional<std::size_t> set_objective_status_008e20d0(
    MissionObjectiveSet& set, const std::string& key, MissionObjectiveStatus status) noexcept;

// ---------------------------------------------------------------------------
// The per-slot scoring record, [game+21A0h] + 4h + slot*284h
// ---------------------------------------------------------------------------

inline constexpr std::size_t kMissionScoringSlotCount = 8; // 00916980, 004d79d3
inline constexpr std::size_t kMissionScoringRecordStride = 0x284; // IMUL at 00906464
inline constexpr std::size_t kMissionScoringArrayBase = 4;        // LEA at 0090646f
// 4 + 8*284h; 004d7970 reads the slot to commit from this offset (004d79d3).
inline constexpr std::size_t kMissionScoringCommitSlotOffset = 0x1424;

constexpr std::size_t mission_scoring_record_offset(std::size_t slot) noexcept
{
    return kMissionScoringArrayBase + slot * kMissionScoringRecordStride;
}

// The runtime half of the manager: the eight records plus the index 004d7970
// commits. The record itself is mission_progress.hpp's MissionScoreRecord;
// nothing is re-declared here.
struct MissionScoringManagerState {
    MissionScoreRecord slots[kMissionScoringSlotCount]{}; // +4h, stride 284h
    int commit_slot{0};                                   // +1424h
};

// 00906460. Writes the completion flag as a zero-extended byte and, only on the
// 0 -> set edge with the flag set, stamps `mission_clock` (00f876a4) into
// completion_time_10 (MOVSS at 00906485). Returns true when the edge fired, so
// a host knows whether the timestamp moved.
bool set_slot_mission_completed_00906460(MissionScoreRecord& record, bool completed,
                                         float mission_clock) noexcept;

// 008b8ad0's argument resolution, 008b8bad..008b8c42. In mode 0 there is no
// explicit slot argument and the local slot game+18ECh is used; otherwise the
// first Lua argument is the slot and the boolean shifts one position right.
// `completed` defaults to true when the boolean is absent.
struct ScoringSetMissionCompletedCall {
    int slot{0};
    bool completed{true};
};
struct ScoringSetMissionCompletedArguments {
    std::uint32_t local_player_mode{0};   // game+1FE4h
    int local_player_slot{0};             // game+18ECh
    int lua_argument_count{0};            // 00b663f0
    std::optional<int> first_integer{};   // arg 0 as an integer, mode != 0 only
    std::optional<bool> boolean_argument{};
};
ScoringSetMissionCompletedCall resolve_set_mission_completed_008b8ad0(
    const ScoringSetMissionCompletedArguments& arguments) noexcept;

// 00927f60's caller-side filter in 00906460: every value of the list at
// [[game+19CCh]+28h] whose +180h dword is <= 7 (CMP ..,7 / JA at 009064aa).
bool scoring_notifies_entity_00906460(std::uint32_t entity_field_180) noexcept;

// ---------------------------------------------------------------------------
// The end-of-mission movie, 004cd390, and the poll 004d7ea0
// ---------------------------------------------------------------------------

// The 24h object at game+7188h. game_frame_control.hpp already projects the
// polled side as bsp::MissionResult; this is the producer side, which also
// carries the name string 004cd390 stores at +14h/+18h.
struct MissionEndMovieRequest {
    std::string name;             // +14h length, +18h pointer, "movies/" + script name
    float parameter_08{0.0f};     // +8h, from the constant at 00ce77e4
    bool requests_debrief{false}; // +21h, PlayBinkMovie's second argument
};

// 004cd390: free any existing request, then construct a new one. The free
// branch falls through into the allocation; the decompiler's early return is
// the no-return annotation on 00bf65ac (see the flow gap note in the doc).
void set_end_of_mission_movie_004cd390(std::optional<MissionEndMovieRequest>& slot,
                                       const std::string& name, bool go_to_debrief,
                                       float parameter_08) noexcept;

// 004d7ea0's decision, without the world ticks and the movie call the host owns.
// The poll runs only with an empty state request queue (game+5E8h == 0) and a
// present request (game+7188h non-null); it enqueues 0Fh and raises the drain
// suspension only when the request asks for the debrief (result+21h).
inline constexpr std::uint32_t kStateRequestDebrief = 0x0F;   // 004d7f56
inline constexpr std::uint32_t kStateRequestTeardown = 0x10;  // 004d7ae0, 004d7a5d
inline constexpr std::uint32_t kStateRequestFrontEnd = 0x04;  // 004e47a7
inline constexpr std::uint32_t kGameStateInMission = 0x0D;    // 004d7a4e
inline constexpr std::uint32_t kGameStateEndScene = 0x0F;

struct MissionCompletionPollDecision {
    bool runs{false};            // the two entry guards
    bool enqueues_debrief{false};// result+21h
    bool suspends_drain{false};  // game+5ECh = 1, set with the enqueue
};
MissionCompletionPollDecision mission_completion_poll_004d7ea0(
    std::size_t pending_state_requests, const std::optional<MissionEndMovieRequest>& request) noexcept;

// ---------------------------------------------------------------------------
// GGame::EndScene, 004d7970, and the commit 009205e0
// ---------------------------------------------------------------------------

// 009205e0. Copies the record into the profile's per-mission map, bumps the
// play counter, and then either keeps the best total (single player) or adds it
// (multiplayer). `record_total` is record+1E0h, the last of totals_1c8.
// `multiplayer_accumulates` is the 00f8a2fc vtable+198h query AND the byte at
// [00f8a2fc]+4Dh; nothing is written when it is false in a multiplayer session.
void commit_mission_record_009205e0(MissionProgress& progress, const std::string& mission_key,
                                    const MissionScoreRecord& record,
                                    std::uint32_t local_player_mode,
                                    bool multiplayer_accumulates);

// 004d7970's decision shape. The body runs once per scene; `already_run` is the
// latch game+1EE2h that mission_state_entry.hpp carries as
// MissionOneShots::end_scene_body_done.
struct EndSceneInputs {
    bool aborted{false};                 // the char argument, stored to game+1EE1h
    bool already_run{false};             // game+1EE2h
    std::uint32_t local_player_mode{0};  // game+1FE4h
    std::uint32_t current_request{0};    // game+5D4h
    bool session_selector_218c{false};   // game+218Ch
    std::int32_t game_field_624{0};      // game+624h
};

struct EndSceneDecision {
    bool commits_record{false};        // 009205e0
    bool adjusts_start_counters{false};// 004bcaa0(0)
    bool resets_player_records{false}; // 00916980 on game+21A0h
    bool flushes_storage{false};       // 007fa1b0 on game+650h
    bool broadcasts_end_scene{false};  // mode 1, normal end
    bool sends_secondary_message{false};// mode 2, aborted
    bool enqueues_teardown{false};     // request 10h
};
EndSceneDecision end_scene_004d7970(const EndSceneInputs& inputs) noexcept;

// ---------------------------------------------------------------------------
// The loss path: unit destruction, 00959450
// ---------------------------------------------------------------------------

// 00959460..00959491, taken from the listing: COMISS against 1.0f at 00d7a24c
// with JBE skipping the report, so the clock must be strictly greater.
inline constexpr float kUnitDeathReportGraceSeconds = 1.0f; // 00d7a24c
// Front-end states that suppress the limbo screen (00959527..0095953f).
inline constexpr int kFrontEndStatesSuppressingLimbo[] = {0x2B, 0x2C, 0x2D, 0x29};
// BSP_FrontEndManager_PushInterfaceRequest id; raises HUD slot 32h, GUI_limbo.
inline constexpr int kInterfaceRequestLimbo = 0x34;

struct UnitDeathInputs {
    float mission_clock{0.0f};      // 00f876a4
    int unit_party_70{0};           // unit+70h; the report needs == 1
    bool world_gate_4ac{false};     // [game+19CCh]+4ACh
    bool is_controlled_unit{false}; // unit == 004b4b00()
    int front_end_state{0};         // [00e198c4]+20h
    bool front_end_idle{false};     // [+4h]==[+20h] && [+1Ch]==[+38h]
    bool limbo_suppressed_0068a120{false}; // 0068a120()
    bool limbo_notice_byte_fd{false};      // [00e198c4]+FDh
    bool owner_allows_alternate_report{false}; // the 17h / 6 virtual queries
};

struct UnitDeathDecision {
    bool reports_kill{false};        // 009813a0, the `kill` warning channel
    bool reports_alternate{false};   // 0091bda0
    bool registers_limbo_page{false};// 00565fb0(unit+70h)
    bool pushes_limbo_interface{false};   // PushInterfaceRequest(34h, 0)
    bool retargets_front_end{false};      // [00e198c4] vtable +10h
    bool raises_limbo_notice_byte{false}; // [00e198c4]+FDh = 1
};

// 00959450. Note what is absent: no objective write, no scoring write, no
// game+7188h write and no state request. A player death raises the limbo
// (respawn) screen; defeat is a mission-script reaction to the kill warning.
UnitDeathDecision unit_death_00959450(const UnitDeathInputs& inputs) noexcept;

// 0093bed0's hazard roll, 0093bf5a..0093bf95. `rate` is component+28h and
// `period` component+2Ch; when either is negative the native substitutes the
// globals at [00424c40()+3DCh] and [+3E0h]. The draw is 00bd2f10(0, 1.0f) and
// the failure fires on `draw <= p`, so p >= 1 always fires.
float component_failure_probability_0093bed0(float rate, float period, float delta,
                                             float fallback_rate,
                                             float fallback_period) noexcept;
bool component_failure_fires_0093bed0(float probability, float uniform_draw) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site of the two sequences this packet recovers.
// There are no default implementations: nothing here stands in for unrecovered
// game behaviour.
struct MissionResultHost {
    virtual ~MissionResultHost() = default;

    // -- Scoring_SetMissionCompleted, 008b8ad0 -> 00906460 --------------------
    virtual MissionScoreRecord& scoring_slot(int slot) = 0; // [game+21A0h]+4h+slot*284h
    virtual float mission_clock() = 0;                      // 00f876a4
    // 00906460's tail: every entity of [[game+19CCh]+28h] with +180h <= 7.
    virtual void notify_scoring_entities_00927f60() = 0;

    // -- PlayBinkMovie, 0089a480 -> 0089a390 -> 004cd390 ----------------------
    virtual std::optional<MissionEndMovieRequest>& end_movie_slot() = 0; // game+7188h

    // -- The poll, 004d7ea0 ---------------------------------------------------
    virtual std::size_t pending_state_requests() = 0;       // game+5E8h
    virtual void tick_world_zero_delta() = 0;               // [game+19CCh] vtable +Ch
    virtual void flush_entity_activations_004c3cb0() = 0;
    virtual void play_end_movie(const MissionEndMovieRequest& request) = 0; // 004f8a20
    virtual void enqueue_state_request(std::uint32_t request) = 0;          // 004d3ed0
    virtual void set_drain_suspended(bool suspended) = 0;                   // game+5ECh
    virtual void release_end_movie_004cc510() = 0;                          // game+7188h reset

    // -- GGame::EndScene, 004d7970 -------------------------------------------
    virtual void set_scene_ended_by_abort(bool aborted) = 0; // game+1EE1h
    virtual void debrief_bringup_00920a20(bool aborted) = 0;
    virtual MissionProgress& mission_progress() = 0;         // game+6B4h
    virtual const std::string& current_mission_key() = 0;    // game+2198h
    virtual int commit_slot() = 0;                           // [game+21A0h]+1424h
    virtual bool multiplayer_score_accumulates() = 0;        // 00f8a2fc queries
    virtual void adjust_mission_start_counters_004bcaa0() = 0;
    virtual void flush_storage_007fa1b0() = 0;
    virtual void reset_player_records_00916980() = 0;
    virtual void broadcast_end_scene() = 0;                  // mode 1
    virtual void send_secondary_end_message() = 0;           // mode 2, message 13h
    virtual void record_metrics_007556a0() = 0;
    virtual void set_end_scene_body_done() = 0;              // game+1EE2h

    // -- Objective state, 008bd340 / 008bd900 -> 008e20d0 / 008e2200 ---------
    virtual MissionObjectiveSet& objective_set(std::size_t index) = 0; // game+21A4h+4*i
    virtual int game_local_player_slot() = 0;                          // game+18ECh
    virtual std::uint32_t local_player_mode() = 0;                     // game+1FE4h
    virtual bool slot_has_peer(int slot) = 0;         // [game+18CCh+4*slot]+50h
    virtual void announce_objective_sound(int sound) = 0;   // 00432650 / 008dd460
    virtual void refresh_objective_markers_008dfe50(const MissionObjectiveEntry& entry) = 0;
    virtual void replicate_objective_state(int slot, const MissionObjectiveEntry& entry,
                                           MissionObjectiveStatus status) = 0; // 008dda20/008ddb00
    virtual void clear_hud_objective_dirty_byte() = 0;      // [[00e198c4]+60h]+65h
};

// Scoring_SetMissionCompleted end to end, 008b8ad0.
void run_set_mission_completed_008b8ad0(MissionResultHost& host,
                                        const ScoringSetMissionCompletedArguments& arguments);

// Objectives_Completed / Objectives_Failed end to end, 008bd340 / 008bd900.
// `silent` is the byte 008e1d30 checks before the announcement.
bool run_set_objective_status_008bd340(MissionResultHost& host, std::size_t set_index,
                                       const std::string& key, MissionObjectiveStatus status,
                                       bool silent);

// The completion poll, 004d7ea0, in native order: three zero-delta world ticks
// around two activation flushes, the movie, the conditional 0Fh, the release.
MissionCompletionPollDecision run_mission_completion_poll_004d7ea0(MissionResultHost& host);

// GGame::EndScene, 004d7970, in native order.
EndSceneDecision run_end_scene_004d7970(MissionResultHost& host, const EndSceneInputs& inputs);

} // namespace bsp
