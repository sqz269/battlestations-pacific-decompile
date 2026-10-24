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

namespace {

template <typename Map>
void add_counters(Map& destination, const Map& source)
{
    for (const auto& [key, value] : source) {
        destination[key] += value;
    }
}

template <typename Outer>
void add_counter_pairs(Outer& destination, const Outer& source)
{
    for (const auto& [key, inner] : source) {
        add_counters(destination[key], inner);
    }
}

} // namespace

// 0091A080 (listing 0091A080..0091B2B1), following cc13_seg59b's
// score_record_merge_0091a080 field by field. Offsets are the image's.
// Image fields the host struct lacks, NOT MODELLED (the destination keeps
// nothing for them and the source's values are dropped): +18Ch (summed when
// src+0 != 0), and the ints +178h, +17Ch, +180h, +184h, +188h (summed at the
// tail). checkpoint_counters_168 (+168h..+174h), checkpoint_pending_24c,
// used_slot_250 and count_284 are not touched by the image; the destination
// keeps its own. The inlined 00BF6713 iterator checks have no host analogue.
void merge_mission_score_record_0091a080(MissionScoreRecord& destination,
                                         const MissionScoreRecord& source)
{
    // 0091A080..: i = src+8. Both tests are signed (JGE skips). The image does
    // not bound i; an index outside 0..2 would touch neighbouring fields and is
    // not modelled.
    const int i = source.difficulty_08;
    if (i >= 0 && i < 3
        && destination.completed_by_difficulty_26c[i] < source.mission_completed_00
        && destination.scores_by_difficulty_278[i] < source.totals_1c8[6]) {
        destination.scores_by_difficulty_278[i] = source.totals_1c8[6];  // +278h <- src+1E0h
        destination.completed_by_difficulty_26c[i] = source.mission_completed_00;  // +26Ch <- src+0
    }
    if (source.mission_completed_00 != 0) {
        destination.ranking_04 = std::max(destination.ranking_04, source.ranking_04);  // +4, JL
        destination.winner_mode_194 = static_cast<int>(
            static_cast<std::uint32_t>(destination.winner_mode_194)
            | static_cast<std::uint32_t>(source.winner_mode_194));  // +194h OR
        // +18Ch += src+18Ch: not modelled (see above).
    }
    destination.mission_completed_00 += source.mission_completed_00;  // +0 summed
    destination.difficulty_08 = source.difficulty_08;                  // +8 overwritten
    destination.play_time_0c = source.play_time_0c + destination.play_time_0c;  // FLD src, FADD dst
    destination.completion_time_10 = source.completion_time_10 + destination.completion_time_10;

    // String-keyed int maps (005070C0). The image folds the +24h, +3Ch, +30h
    // and +48h sources ALL into dst+24h (ECX = dst+24h at 0091A257, 0091A2D7,
    // 0091A357), in that order; an image quirk, kept. dst+30h/+3Ch/+48h are
    // therefore never merged into.
    auto& maps = destination.score_maps;
    const auto& from = source.score_maps;
    const auto at = [](auto& array, MissionScoreMap map) -> auto& {
        return array[static_cast<std::size_t>(map)];
    };
    using M = MissionScoreMap;
    add_counters(at(maps, M::Mission), at(from, M::Mission));             // +18h
    add_counters(at(maps, M::Action), at(from, M::Action));               // +24h
    add_counters(at(maps, M::Action), at(from, M::Plane));                // +3Ch -> +24h
    add_counters(at(maps, M::Action), at(from, M::Ship));                 // +30h -> +24h
    add_counters(at(maps, M::Action), at(from, M::Command));              // +48h -> +24h
    add_counters(at(maps, M::MissionMedals), at(from, M::MissionMedals)); // +54h
    add_counters(at(maps, M::ActionMedals), at(from, M::ActionMedals));   // +60h

    // Three-level trees, leaf ints summed: +B4h, +C0h (0062C170 / 0062BB60 /
    // 00625900), then +9Ch, +A8h (00915610 / 00911890 / 00625900).
    const auto add_triples = [](MissionEnumCounterTriples& to,
                                const MissionEnumCounterTriples& add) {
        for (const auto& [key, inner] : add) {
            add_counter_pairs(to[key], inner);
        }
    };
    add_triples(destination.player_kills_b4, source.player_kills_b4);
    add_triples(destination.party_kills_c0, source.party_kills_c0);
    add_triples(destination.player_damages_9c, source.player_damages_9c);
    add_triples(destination.party_damages_a8, source.party_damages_a8);
    // 0091AAFD..0091AB90: +114h int -> float (0090E740), *slot = src + *slot.
    for (const auto& [key, value] : source.unit_usage_114) {
        float& slot = destination.unit_usage_114[key];
        slot = value + slot;
    }
    add_counters(destination.unit_suicide_d8, source.unit_suicide_d8);          // 0091AB92, +D8h
    add_counter_pairs(destination.unit_remaining_cc, source.unit_remaining_cc); // 0091AC12, +CCh

    destination.map_usage_14 = source.map_usage_14 + destination.map_usage_14;  // +14h
    // Ints +1C8h..+1E8h summed: the seven totals, then +1E4h and +1E8h.
    for (std::size_t k = 0; k < destination.totals_1c8.size(); ++k) {
        destination.totals_1c8[k] += source.totals_1c8[k];
    }
    destination.allied_party_1e4 += source.allied_party_1e4;
    destination.japanese_party_1e8 += source.japanese_party_1e8;

    add_counters(destination.losses_1ec[0], source.losses_1ec[0]);  // +1ECh
    add_counters(destination.losses_1ec[1], source.losses_1ec[1]);  // +1F8h

    // 0091AEDF..0091B268: 00625F50 on each of +204h..+240h, [result+8] +=
    // node+1Ch, i.e. the pair's int. Uncertain: whether 00625F50 copies the
    // source's name into a new entry; here a new entry's name stays empty.
    for (std::size_t k = 0; k < destination.objectives_204.size(); ++k) {
        for (const auto& [key, value] : source.objectives_204[k]) {
            destination.objectives_204[k][key].value += value.value;
        }
    }
    // The tail adds +178h, +17Ch, +180h, +188h, +184h: not modelled (see above).
}

void commit_mission_record_009205e0(MissionProgress& progress, const std::string& mission_key,
                                    const MissionScoreRecord& record,
                                    std::uint32_t local_player_mode,
                                    bool multiplayer_accumulates)
{
    MissionScoreRecord& stored = mission_record_00594a70(progress, mission_key);
    if constexpr (kMissionRecordMergeBound) {
        merge_mission_score_record_0091a080(stored, record); // CALL 0091A080 at 009205F9
        stored.count_284 += 1;                               // ADD [stored+284h],1 at 009205FE
    } else {
        // The historical copy model: the record replaces the stored one.
        const int play_count = stored.count_284;
        stored = record;
        stored.count_284 = play_count + 1; // the increment at 009205FE
    }

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
        // 009594C4..0095951E (packet cc9_construct_world_p8): 009813A0 is skipped
        // when the controller at unit+538h is kind 17h with +C41h set or kind 6
        // with +100Ah set; 0091BDA0 follows in every case (00959519).
        decision.reports_kill = !inputs.owner_allows_alternate_report; // 009813a0
        decision.reports_alternate = true;                            // 0091bda0
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
    // 009595A4..009595B8: when idle, PushInterfaceRequest(34h, 0). Its store of the
    // pending pair makes 009595C3..009595D1's re-test fail the idle comparison, so
    // the [00e198c4] vtable +10h at 009595DD runs as well (packet cc13-limbo2). The
    // exception is 004CC460's refusal (004CC478 [00E19894] set and 004CC48A
    // 00683E90(34h) true), which leaves the front end idle and skips the +10h.
    // SUBSTITUTION: the refusal is not an input here, so the push is taken as
    // accepted. When not idle, only the +10h runs.
    if (inputs.front_end_idle) {
        decision.pushes_limbo_interface = true; // PushInterfaceRequest(34h, 0)
    }
    decision.retargets_front_end = true; // [00e198c4] vtable +10h
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
