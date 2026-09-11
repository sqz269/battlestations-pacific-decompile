#include "bsp/mission_result.hpp"

#include <algorithm>
#include <cctype>

namespace bsp {
namespace {

// The __stricmp 008e20d0 reaches at 008e2141. Kept local: profile_unlock.hpp's
// comparator orders, it does not answer equality for an already length-matched
// pair, and the native order (length, then emptiness, then case) is observable.
int case_insensitive_compare(const std::string& left, const std::string& right) noexcept
{
    const std::size_t count = std::min(left.size(), right.size());
    for (std::size_t i = 0; i < count; ++i) {
        const int a = std::tolower(static_cast<unsigned char>(left[i]));
        const int b = std::tolower(static_cast<unsigned char>(right[i]));
        if (a != b) {
            return a < b ? -1 : 1;
        }
    }
    if (left.size() == right.size()) {
        return 0;
    }
    return left.size() < right.size() ? -1 : 1;
}

} // namespace

// ---------------------------------------------------------------------------
// Objectives
// ---------------------------------------------------------------------------

bool objective_key_matches_008e20f6(const std::string& record_id, const std::string& key) noexcept
{
    // 008e2118: the lengths are compared first and a mismatch skips the entry.
    if (record_id.size() != key.size()) {
        return false;
    }
    // 008e212e..008e2137: a zero record length matches only a zero key length,
    // and a zero key length against a non-zero record never matches. With the
    // length test above both collapse to "both empty".
    if (record_id.empty()) {
        return key.empty();
    }
    return case_insensitive_compare(record_id, key) == 0;
}

std::optional<std::size_t> find_objective_008e20f6(const MissionObjectiveSet& set,
                                                   const std::string& key) noexcept
{
    for (std::size_t i = 0; i < set.entries.size(); ++i) {
        if (objective_key_matches_008e20f6(set.entries[i].id, key)) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<int> objective_announcement_sound_008e1d30(
    int set_local_player_slot, int game_local_slot_18ec, MissionObjectiveKind kind,
    bool completed, bool silent) noexcept
{
    // 008e1d47: -1 < game+18ECh < 8, then set+14h == game+18ECh.
    if (game_local_slot_18ec < 0
        || game_local_slot_18ec >= static_cast<int>(kMissionScoringSlotCount)) {
        return std::nullopt;
    }
    if (set_local_player_slot != game_local_slot_18ec) {
        return std::nullopt;
    }
    // The objective's kind must be primary or secondary.
    if (kind != MissionObjectiveKind::Primary && kind != MissionObjectiveKind::Secondary) {
        return std::nullopt;
    }
    if (silent) {
        return std::nullopt;
    }
    // 008e1d4f: 2 - (completed != 0).
    return 2 - (completed ? 1 : 0);
}

bool objective_refreshes_markers_008dfe50(MissionObjectiveKind kind) noexcept
{
    return kind != MissionObjectiveKind::Hidden; // 008dfe63, objective+18h != 2
}

bool objective_replicates_008e20d0(std::uint32_t local_player_mode, bool slot_has_peer) noexcept
{
    return local_player_mode == 1 && slot_has_peer;
}

std::optional<std::size_t> set_objective_status_008e20d0(
    MissionObjectiveSet& set, const std::string& key, MissionObjectiveStatus status) noexcept
{
    const std::optional<std::size_t> index = find_objective_008e20f6(set, key);
    if (!index) {
        return std::nullopt;
    }
    set.entries[*index].status = status; // objective+1Ch
    return index;
}

// ---------------------------------------------------------------------------
// The per-slot scoring record
// ---------------------------------------------------------------------------

bool set_slot_mission_completed_00906460(MissionScoreRecord& record, bool completed,
                                         float mission_clock) noexcept
{
    // 0090646a: the edge test reads the old flag before the store.
    const bool edge = record.mission_completed_00 == 0 && completed;
    if (edge) {
        record.completion_time_10 = mission_clock; // MOVSS at 00906485
    }
    record.mission_completed_00 = completed ? 1 : 0; // MOVZX at 0090648a
    return edge;
}

ScoringSetMissionCompletedCall resolve_set_mission_completed_008b8ad0(
    const ScoringSetMissionCompletedArguments& arguments) noexcept
{
    ScoringSetMissionCompletedCall call{};
    call.slot = arguments.local_player_slot; // MOV EDI,[game+18ECh] at 008b8bb8
    int boolean_position = 0;
    if (arguments.local_player_mode != 0) { // JZ at 008b8bc7 skips the integer read
        if (arguments.first_integer) {
            call.slot = *arguments.first_integer;
        }
        boolean_position = 1; // MOV ESI,1 at 008b8bf9
    }
    // 008b8c0c: the boolean is read only when the argument count exceeds its
    // position; otherwise the byte written at 008b8c02 stays 1.
    call.completed = true;
    if (arguments.lua_argument_count > boolean_position && arguments.boolean_argument) {
        call.completed = *arguments.boolean_argument;
    }
    return call;
}

bool scoring_notifies_entity_00906460(std::uint32_t entity_field_180) noexcept
{
    return entity_field_180 <= 7; // CMP ..,0x7 / JA at 009064aa
}

// ---------------------------------------------------------------------------
// End-of-mission movie and the poll
// ---------------------------------------------------------------------------

void set_end_of_mission_movie_004cd390(std::optional<MissionEndMovieRequest>& slot,
                                       const std::string& name, bool go_to_debrief,
                                       float parameter_08) noexcept
{
    slot.reset(); // 004cd3b4..004cd3d2, the free branch falls through
    MissionEndMovieRequest request{};
    request.name = name;
    request.parameter_08 = parameter_08; // MOVSS from 00ce77e4 at 004cd420
    request.requests_debrief = go_to_debrief; // 004cd430
    slot = request;
}

MissionCompletionPollDecision mission_completion_poll_004d7ea0(
    std::size_t pending_state_requests,
    const std::optional<MissionEndMovieRequest>& request) noexcept
{
    MissionCompletionPollDecision decision{};
    if (pending_state_requests != 0 || !request) {
        return decision; // both guards at 004d7eae
    }
    decision.runs = true;
    if (request->requests_debrief) { // result+21h
        decision.enqueues_debrief = true;
        decision.suspends_drain = true; // game+5ECh = 1
    }
    return decision;
}

// ---------------------------------------------------------------------------
// GGame::EndScene and the commit
// ---------------------------------------------------------------------------

void commit_mission_record_009205e0(MissionProgress& progress, const std::string& mission_key,
                                    const MissionScoreRecord& record,
                                    std::uint32_t local_player_mode,
                                    bool multiplayer_accumulates)
{
    MissionScoreRecord& stored = mission_record_00594a70(progress, mission_key);
    const int play_count = stored.count_284; // 0091a080 overwrites the record
    stored = record;
    stored.count_284 = play_count + 1; // the increment at 00920604

    // totals_1c8[6] is record+1E0h, the total the two maps take.
    const int total = record.totals_1c8[6];
    if (local_player_mode == 0) {
        int& best = progress.single_best_scores_0c[mission_key];
        best = std::max(best, total); // the CMP/CMOV pair at 0092062a
    } else if (multiplayer_accumulates) {
        int& accumulated = progress.multi_scores_18[mission_key];
        accumulated = accumulated + total; // 0092066b
    }
}

EndSceneDecision end_scene_004d7970(const EndSceneInputs& inputs) noexcept
{
    EndSceneDecision decision{};
    if (!inputs.already_run) { // game+1EE2h at 004d79a6
        if (!inputs.aborted) {
            decision.commits_record = true;
            decision.adjusts_start_counters = inputs.local_player_mode != 0
                                              && !inputs.session_selector_218c
                                              && inputs.game_field_624 == 0;
        } else {
            decision.flushes_storage = true;
            decision.resets_player_records = inputs.local_player_mode == 0;
        }
    }
    if (inputs.current_request == kGameStateInMission
        || inputs.current_request == kGameStateEndScene) { // 004d7a4e
        if (inputs.local_player_mode == 1 && !inputs.aborted) {
            decision.broadcasts_end_scene = true;
            return decision; // the broadcast arm returns without the enqueue
        }
        if (inputs.local_player_mode == 2 && inputs.aborted) {
            decision.sends_secondary_message = true;
        }
        decision.enqueues_teardown = true;
    }
    return decision;
}

// ---------------------------------------------------------------------------
// The loss path
// ---------------------------------------------------------------------------

UnitDeathDecision unit_death_00959450(const UnitDeathInputs& inputs) noexcept
{
    UnitDeathDecision decision{};
    // 00959468: COMISS clock, 1.0f with JBE skipping, so strictly greater.
    const bool reports = inputs.mission_clock > kUnitDeathReportGraceSeconds
                         && inputs.unit_party_70 == 1 && inputs.world_gate_4ac;
    if (reports) {
        if (inputs.owner_allows_alternate_report) {
            decision.reports_alternate = true; // 0091bda0
        } else {
            decision.reports_kill = true; // 009813a0
        }
    }
    if (!inputs.is_controlled_unit) {
        return decision;
    }
    bool suppressed_state = false;
    for (const int state : kFrontEndStatesSuppressingLimbo) {
        if (inputs.front_end_state == state) {
            suppressed_state = true;
            break;
        }
    }
    if (!inputs.world_gate_4ac || suppressed_state) {
        return decision;
    }
    decision.registers_limbo_page = true; // 00565fb0
    if (inputs.limbo_suppressed_0068a120) {
        decision.raises_limbo_notice_byte = !inputs.limbo_notice_byte_fd;
        return decision;
    }
    if (inputs.front_end_idle) {
        decision.pushes_limbo_interface = true; // PushInterfaceRequest(34h, 0)
    } else {
        decision.retargets_front_end = true; // [00e198c4] vtable +10h
    }
    return decision;
}

float component_failure_probability_0093bed0(float rate, float period, float delta,
                                             float fallback_rate,
                                             float fallback_period) noexcept
{
    if (rate < 0.0f || period < 0.0f) {
        return (fallback_rate * delta) / fallback_period;
    }
    return (rate * delta) / period;
}

bool component_failure_fires_0093bed0(float probability, float uniform_draw) noexcept
{
    return uniform_draw <= probability; // the COMISS/JA pair at 0093bf95
}

// ---------------------------------------------------------------------------
// Sequences
// ---------------------------------------------------------------------------

void run_set_mission_completed_008b8ad0(MissionResultHost& host,
                                        const ScoringSetMissionCompletedArguments& arguments)
{
    const ScoringSetMissionCompletedCall call =
        resolve_set_mission_completed_008b8ad0(arguments);
    MissionScoreRecord& record = host.scoring_slot(call.slot);
    set_slot_mission_completed_00906460(record, call.completed, host.mission_clock());
    host.notify_scoring_entities_00927f60();
}

bool run_set_objective_status_008bd340(MissionResultHost& host, std::size_t set_index,
                                       const std::string& key, MissionObjectiveStatus status,
                                       bool silent)
{
    MissionObjectiveSet& set = host.objective_set(set_index);
    const std::optional<std::size_t> index = find_objective_008e20f6(set, key);
    if (!index) {
        return false;
    }
    MissionObjectiveEntry& entry = set.entries[*index];
    const bool completed = status == MissionObjectiveStatus::Completed;
    // 008e216f: the announcement runs before the marker refresh and before the
    // state store at 008e2181.
    if (const std::optional<int> sound = objective_announcement_sound_008e1d30(
            set.local_player_slot, host.game_local_player_slot(), entry.kind, completed,
            silent)) {
        host.announce_objective_sound(*sound);
    }
    if (objective_refreshes_markers_008dfe50(entry.kind)) {
        host.refresh_objective_markers_008dfe50(entry);
    }
    entry.status = status;
    if (objective_replicates_008e20d0(host.local_player_mode(),
                                      host.slot_has_peer(set.local_player_slot))) {
        host.replicate_objective_state(set.local_player_slot, entry, status);
    }
    return true;
}

MissionCompletionPollDecision run_mission_completion_poll_004d7ea0(MissionResultHost& host)
{
    std::optional<MissionEndMovieRequest>& slot = host.end_movie_slot();
    const MissionCompletionPollDecision decision =
        mission_completion_poll_004d7ea0(host.pending_state_requests(), slot);
    if (!decision.runs) {
        return decision;
    }
    // 004d7ed6..004d7f0c: tick, flush, tick, flush, tick, then 004c3cb0.
    host.tick_world_zero_delta();
    host.flush_entity_activations_004c3cb0();
    host.tick_world_zero_delta();
    host.flush_entity_activations_004c3cb0();
    host.tick_world_zero_delta();
    host.play_end_movie(*slot);
    if (decision.enqueues_debrief) {
        host.enqueue_state_request(kStateRequestDebrief);
        host.set_drain_suspended(true);
    }
    host.release_end_movie_004cc510();
    slot.reset();
    return decision;
}

EndSceneDecision run_end_scene_004d7970(MissionResultHost& host, const EndSceneInputs& inputs)
{
    const EndSceneDecision decision = end_scene_004d7970(inputs);
    host.set_scene_ended_by_abort(inputs.aborted);
    if (!inputs.already_run) {
        host.debrief_bringup_00920a20(inputs.aborted);
        if (decision.commits_record) {
            commit_mission_record_009205e0(host.mission_progress(), host.current_mission_key(),
                                           host.scoring_slot(host.commit_slot()),
                                           inputs.local_player_mode,
                                           host.multiplayer_score_accumulates());
        }
        if (decision.adjusts_start_counters) {
            host.adjust_mission_start_counters_004bcaa0();
        }
        if (decision.flushes_storage) {
            host.flush_storage_007fa1b0();
        }
        if (decision.resets_player_records) {
            host.reset_player_records_00916980();
        }
        host.record_metrics_007556a0();
        host.set_end_scene_body_done();
    }
    if (decision.broadcasts_end_scene) {
        host.broadcast_end_scene();
        return decision;
    }
    if (decision.sends_secondary_message) {
        host.send_secondary_end_message();
    }
    if (decision.enqueues_teardown) {
        host.enqueue_state_request(kStateRequestTeardown);
    }
    return decision;
}

} // namespace bsp
