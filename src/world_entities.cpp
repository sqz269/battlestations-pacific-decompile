#include "bsp/world_entities.hpp"

#include <cmath>

namespace bsp {
namespace {
// 004e4e50 and 004e5019 both compare against 00d7a218, which holds 0.0f.
constexpr float kPositiveDeltaThreshold = 0.0f;
} // namespace

bool world_tick_runs(const WorldTickGameFields& game) noexcept {
    // 004e50b0..004e50c0 then 004e5108..004e5118: the cinematic byte can be
    // overridden, the state and the suspend byte cannot.
    if (game.cinematic && !game.cinematic_override) {
        return false;
    }
    return game.state == 0x0Du && !game.simulation_suspended;
}

bool mission_events_run(const WorldTickGameFields& game) noexcept {
    // 004e4df4 (state Dh), 004e4e31 (+634h clear, with no override on this
    // path), 004e4e3e (+21E0h non-null) and 004e4e50 (delta above 0.0f).
    return game.state == 0x0Du && !game.cinematic && game.mission_events_present
        && game.scaled_delta > kPositiveDeltaThreshold;
}

// ---------------------------------------------------------------------------
// 00914ef0
// ---------------------------------------------------------------------------

void update_bot_scheduler_00914ef0(WorldTickState& state, WorldTickHost& host) {
    BotSchedulerState& bots = state.bots;
    const float scaled_delta = state.game.scaled_delta;
    const std::uint32_t mode = state.game.local_player_mode;

    // 00914f0d..00914f2f. Both countdowns are skipped outside single-view play.
    if ((mode == 0u || mode == 1u) && bots.enabled) {
        // 00914f36..00914f4c, a subtraction rather than an accumulate.
        bots.retarget_countdown -= scaled_delta;
        if (bots.retarget_countdown <= 0.0f) {
            host.bot_retarget_begin_0075b430(kBotRetargetEventId);
            for (std::size_t slot = 0; slot < kBotSlotCount; ++slot) {
                // 00914f90: the same byte gates every slot, re-read each pass.
                if (bots.slots_active) {
                    host.bot_slot_prepare_00914390(slot);
                }
                // 00914fa1: the dispatch is single-view only.
                if (mode == 1u) {
                    const std::uint32_t token
                        = slot < bots.slot_tokens.size() ? bots.slot_tokens[slot] : 0u;
                    host.bot_slot_dispatch_0076a9f0(token, kBotSlotDispatchCode, slot);
                }
            }
            // 00914fe4..00914ff5, a reload rather than a wrap-around.
            bots.retarget_countdown = bots.retarget_period;
        }

        // 00915003..00915018.
        bots.think_countdown -= scaled_delta;
        if (bots.think_countdown <= 0.0f) {
            for (std::size_t slot = 0; slot < kBotSlotCount; ++slot) {
                host.bot_think_pass_a_00911e80(slot);
                host.bot_think_pass_b_00912a60(slot);
            }
            bots.think_countdown = bots.think_period;
        }
    }

    // 00915058..0091507a, outside the block above and with its own mode test.
    if (mode != 2u && bots.slots_active) {
        bots.active_time += scaled_delta;
    }
}

// ---------------------------------------------------------------------------
// 006dc1a0
// ---------------------------------------------------------------------------

float marker_pulse_magnitude(float elapsed, float pulse_epoch) noexcept {
    // 006dc3e6..006dc415. The phase is doubled with FADD ST0,ST0 and the
    // cosine is taken at float precision before the sign is masked off.
    const float phase = (elapsed - pulse_epoch) + (elapsed - pulse_epoch);
    return std::fabs(std::cos(phase));
}

void update_markers_006dc1a0(WorldTickState& state, WorldTickHost& host) {
    MarkerManagerState& markers = state.markers;
    const float scaled_delta = state.game.scaled_delta;

    // Walks 1 and 2, 006dc1c3..006dc39c. Two levels of std::list; the marker
    // pointer sits at the inner element and a null one is skipped.
    for (const MarkerGroup& group : markers.entity_marker_groups) {
        for (void* marker : group.markers) {
            if (marker != nullptr) {
                host.marker_update(marker, scaled_delta);
            }
        }
    }
    for (const MarkerGroup& group : markers.position_marker_groups) {
        for (void* marker : group.markers) {
            if (marker != nullptr) {
                host.marker_update(marker, scaled_delta);
            }
        }
    }

    // Walk 3, 006dc3a1..006dc439. No null test here: the native body loads the
    // target and stores through it unconditionally.
    const float magnitude = marker_pulse_magnitude(state.game.elapsed, state.marker_pulse_epoch);
    for (const MarkerEntry& entry : markers.gui_highlights) {
        const float level
            = static_cast<float>(static_cast<double>(magnitude) * kMarkerHighlightPulseScale);
        host.marker_set_highlight_level(entry.target, level);
    }

    // Walk 4, 006dc445..006dc4ab. The pulse is not used here.
    for (const MarkerEntry& entry : markers.recursive_gui_highlights) {
        host.marker_set_scale_006dbac0(
            entry.target, kMarkerScaleModeArg, kMarkerScaleFlagArg, kMarkerScaleValueArg);
    }

    // Walk 5, 006dc4b8..006dc581. The getter fills all four floats and only the
    // fourth is rewritten before the setter runs.
    for (const MarkerEntry& entry : markers.tinted_highlights) {
        MarkerColor color = host.marker_get_color(entry.target);
        color.rgba[3] = static_cast<float>(
            static_cast<double>(magnitude) * kMarkerAlphaPulseScale + kMarkerAlphaPulseBias);
        host.marker_set_color(entry.target, color);
    }
}

// ---------------------------------------------------------------------------
// 00481640
// ---------------------------------------------------------------------------

void update_entity_manager_00481640(WorldTickState& state, WorldTickHost& host) {
    // 00481640..00481652. The gate is the world byte, reached through the game
    // singleton, and it is tested before the entity manager is touched at all.
    if (!state.world_gate.enabled) {
        return;
    }
    host.entity_manager_update(state.game.scaled_delta);
}

// ---------------------------------------------------------------------------
// 00740e10
// ---------------------------------------------------------------------------

std::size_t update_decal_manager_00740e10(
    DecalManagerState& decals, float scaled_delta) noexcept {
    static_cast<void>(scaled_delta); // 00740e10 never reads its argument.
    // 00740e28..00740e75, the one-shot profiler label registration.
    decals.profiler_label_registered = true;
    // 00740e7a..00740e96. The loop body is empty in the shipped image.
    return decals.decals.size();
}

// ---------------------------------------------------------------------------
// 008eb110
// ---------------------------------------------------------------------------

void update_power_ups_008eb110(WorldTickState& state, WorldTickHost& host) {
    PowerUpManagerState& power_ups = state.power_ups;
    host.power_ups_pre_pass_008eac80();

    // 008eb146..008eb18f. Sixteen channels; the sweep notifies and never erases.
    const float now = host.world_clock();
    for (std::size_t channel = 0; channel < kPowerUpChannelCount; ++channel) {
        for (const PowerUpTimer& timer : power_ups.channels[channel]) {
            if (timer.expires_at < now) {
                host.power_up_expire_008e8c30(timer.payload);
            }
        }
    }

    // 008eb195..008eb2e4, over the list the active local-player slot selects.
    const std::size_t slot = state.game.active_local_player_slot;
    for (std::size_t index = 0; index < power_ups.player_slots.size(); ++index) {
        PowerUpSlot& entry = power_ups.player_slots[index];
        // 008eb1fa..008eb219: a set flag re-derives the deadline from the
        // source every frame, so it only fires once the source offset is
        // negative rather than at a fixed time.
        const float deadline
            = entry.relative_deadline ? entry.source_offset + now : entry.absolute_expiry;
        if (deadline >= now) {
            continue;
        }
        if (entry.notified) {
            continue;
        }
        entry.notified = true;
        if (power_ups.player_notify_enabled) {
            host.power_up_notify_ready_009789a0(slot, index);
        }
    }
    host.power_ups_post_pass_00613760();
}

// ---------------------------------------------------------------------------
// 00903670
// ---------------------------------------------------------------------------

bool activation_pending(const std::vector<ActivationNode>& nodes, std::size_t index) noexcept {
    if (index >= nodes.size()) {
        return false;
    }
    const ActivationNode& node = nodes[index];
    // 00903680..00903697.
    if (!node.spawned || node.activated) {
        return false;
    }
    if (node.parent == kNoActivationParent) {
        return true;
    }
    return node.parent < nodes.size() && !nodes[node.parent].spawned;
}

void flush_entity_activations_00903670(WorldTickState& state, WorldTickHost& host) {
    // 00903671..009036a5. The chain is walked once; 00922fd0 latches the
    // activated byte on the node and its children, so a node reached as a
    // child is skipped when the walk arrives at it.
    for (std::size_t index = 0; index < state.activation_nodes.size(); ++index) {
        if (activation_pending(state.activation_nodes, index)) {
            host.activate_entity_subtree_00922fd0(index);
        }
    }
}

// ---------------------------------------------------------------------------
// 00987590
// ---------------------------------------------------------------------------

MissionEventTickResult update_mission_events_00987590(
    WorldTickState& state, WorldTickHost& host) {
    MissionEventQueueState& queue = state.mission_events;
    MissionEventTickResult result{};
    result.retired = kNoMissionEvent;
    result.selected = kNoMissionEvent;

    host.mission_events_pre_pass_00982540();

    // 0098759e..009875c7. The accumulator is reset to zero, not decremented,
    // so the period is a floor rather than an exact interval.
    queue.tick_accumulator += state.game.scaled_delta;
    if (queue.tick_accumulator > kMissionEventTickPeriod) {
        queue.tick_accumulator = 0.0f;
        result.periodic_fired = true;
        host.mission_events_periodic_00977990();
    }
    host.mission_events_poll_0096d540();
    host.mission_events_poll_00968550();

    if (queue.events.empty()) {
        return result;
    }

    const float now = host.world_clock();
    for (std::size_t index = 0; index < queue.events.size(); ++index) {
        const MissionEvent& event = queue.events[index];
        const float start = host.mission_event_start_time(index);
        if (now - start >= event.duration) {
            // 0098765c..00987697. The node is destroyed, unlinked and freed,
            // and the routine returns without applying anything, so at most one
            // event is retired per frame and the pending selection is dropped.
            host.mission_event_destroy(index);
            queue.events.erase(queue.events.begin() + static_cast<std::ptrdiff_t>(index));
            result.retired = index;
            result.selected = kNoMissionEvent;
            return result;
        }
        if (!host.mission_event_ready_005b71d0(index)) {
            continue;
        }
        // 009876a1..009876d5. Strictly greater wins, so the first of several
        // equally rated events is kept.
        if (result.selected == kNoMissionEvent
            || host.mission_event_priority(result.selected)
                < host.mission_event_priority(index)) {
            result.selected = index;
        }
    }

    if (result.selected != kNoMissionEvent) {
        host.mission_event_apply_00974070(result.selected);
    }
    return result;
}

// ---------------------------------------------------------------------------
// 004d8cd0
// ---------------------------------------------------------------------------

void record_action_deadlines_004d8cd0(WorldTickState& state, WorldTickHost& host) {
    for (std::size_t i = 0; i < kActionDeadlineCount; ++i) {
        const int action = kActionDeadlineIds[i];
        if (!host.input_action_pressed(action)) {
            continue;
        }
        // 004d8d02..004d8d2d: the deadline is computed in double and stored
        // back as a float through the map's mapped reference.
        state.action_deadlines[action] = static_cast<float>(
            static_cast<double>(state.game.elapsed) + kActionDeadlineDelay);
    }
}

// ---------------------------------------------------------------------------
// Sequence
// ---------------------------------------------------------------------------

void run_world_tick(WorldTickState& state, WorldTickHost& host) {
    // 004e4df5..004e4e6c. The effect lists that follow the mission-event update
    // in the native body belong to game_world_ocean_effects.
    if (mission_events_run(state.game)) {
        update_mission_events_00987590(state, host);
    }

    // 004e4feb and 004e4ff5. Both run on every path, including the one that
    // failed the state test above.
    record_action_deadlines_004d8cd0(state, host);
    state.frame_counter += 1u;

    if (!world_tick_runs(state.game)) {
        return;
    }

    // 004e52ba..004e5389, native order. The award trackers ahead of this block
    // belong to game_award_trackers; the ocean pair at 004e52f9, the trivial
    // 0094c8f0, the 004d1100/00867ee0 effect pair and the mission-completion
    // poll at 004e5389 belong to other packets and are not stubbed here.
    update_bot_scheduler_00914ef0(state, host);
    update_markers_006dc1a0(state, host);
    update_entity_manager_00481640(state, host);
    update_decal_manager_00740e10(state.decals, state.game.scaled_delta);
    update_power_ups_008eb110(state, host);
    flush_entity_activations_00903670(state, host);
}
}
