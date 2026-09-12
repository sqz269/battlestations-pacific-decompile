#pragma once
// bsp_game.exe milestone 2m: the per-step and per-load routines whose
// reconstructions had landed on main and that the executable was still
// recording.
//
// Addresses: 0098bdb0 (the spatial re-bucket, fan-out row 3+4 at 00875e33 /
// 00875e3a), 00926700 (the deferred entity event drain, rows 6 and 14 at
// 00875e44 and 00875ec4), 00888230 (the queued Lua call drain, row 7 at
// 00875e55), 00929460 (the due entity think, row 8 at 00875e64), 009273a0 (the
// pending entity queues, row 15 at 00875ec9), 00903610 (the world expiry, row
// 16 at 00875eda); 004dfc13 with 004cec60 and 004bb160 (the load's session-slot
// reset), 004e0754 with 004218e0 and 00424d00 (the avoid-zone rebuild) and
// 004d30f0 with 00b66200 (the scripted-name baseline).
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::SpatialRefreshHost, bsp::DeferredEntityEventHost,
// bsp::MissionNamedCallHost, bsp::EntityThinkHost, bsp::PendingEntityQueueHost,
// bsp::WorldExpiryHost, bsp::NetworkSlotResetHost, bsp::AvoidZoneResetHost or
// bsp::ScriptedNameListHost, satisfied either by state this process owns or by
// the explicit unimplemented policy in GameHostLog.
//
// What a run can show is bounded by what this process holds, and each bound is
// reported rather than worked around:
//   - the spatial index has no node. 0098a310's callers are the scene graph's
//     and nothing in the reconstructed load registers one, so the re-bucket
//     walk visits an empty root list.
//   - the deferred event queue, the pending destroy and kill lists, the live
//     and pending think lists and the deferred Lua call queue are all empty,
//     because their producers (00922e60, 00922fd0, 0088a240, 00887560) are
//     records or bindings this process does not run.
//   - the world expiry walk does have something to walk: the 32 created
//     instances of the chain 009037f0 allocated. Every counter at entity+6Ch is
//     zero, which is 00903625's skip, because 00922fd0 is the only writer and
//     nothing marks an entity for release here.
//
// Evidence: docs/FIXED_STEP_FANOUT.md, docs/SPATIAL_INDEX.md,
// docs/ENTITY_EVENT_QUEUES.md, docs/ENTITY_THINK_DISPATCH.md,
// docs/MISSION_NAMED_CALL_ARGS.md, docs/MISSION_LOAD_HOSTS.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "bsp/entity_think_dispatch.hpp"
#include "bsp/fixed_step_callbacks.hpp"
#include "bsp/mission_named_call_args.hpp"

namespace bsp::game {

class GameHostLog;
class GameMissionLuaHost;
class GameUnitsHost;

struct GameStepSubsystemsSummary {
    unsigned long long spatial_passes{0};
    std::size_t spatial_nodes{0};
    unsigned long long callback_passes{0};
    std::size_t callbacks_run{0};
    unsigned long long deferred_event_passes{0};
    std::size_t deferred_events{0};
    unsigned long long lua_call_passes{0};
    std::size_t lua_calls_drained{0};
    unsigned long long think_passes{0};
    std::size_t thinks_run{0};
    std::size_t think_collections{0};
    unsigned long long pending_queue_passes{0};
    std::size_t entities_destroyed{0};
    std::size_t entities_killed{0};
    unsigned long long expiry_passes{0};
    unsigned long long expiry_entities{0};
    std::size_t expiry_released{0};
};

// The six fan-out rows whose reconstructions are on main, as one owner. Held for
// the whole run because every one of them keeps a container across steps.
class GameStepSubsystemsHost {
public:
    explicit GameStepSubsystemsHost(GameHostLog& log);

    // The world entity chain 009037f0 allocated, which row 16 walks. Attached on
    // the load's own load_scene_contents row, with the created instances.
    void attach_units(GameUnitsHost* units) noexcept;

    void refresh_moved_spatial_nodes_0098bdb0(float step);
    // Fan-out row 5, 00874de0 at 00875e3f. Packet cc2_fixed_step_callbacks
    // landed while this packet was open.
    void run_fixed_step_callbacks_00874de0(float step);
    void drain_deferred_entity_events_00926700();
    void drain_queued_lua_calls_00888230();
    void run_due_entity_think_00929460(float step);
    void flush_pending_entity_queues_009273a0();
    void release_expired_world_objects_00903610();

    void report();
    const GameStepSubsystemsSummary& summary() const noexcept { return summary_; }

private:
    GameHostLog& log_;
    GameUnitsHost* units_{nullptr};
    GameStepSubsystemsSummary summary_{};
    // 00f89a04, the think countdown the pass ages, and the two lists at
    // 00f89ab0 / 00f89abc.
    float think_countdown_{0.0f};
    bsp::EntityThinkList think_live_{};
    bsp::EntityThinkList think_pending_{};
    // The deferred call list at [game+1A08h]+8h.
    bsp::MissionLuaCallQueue lua_queue_{};
    // The per-step callback list at 00F87680. Its only registrant is 00875a80,
    // whose callers are the engine's own subsystems, so the list stays empty.
    bsp::FixedStepCallbackList callbacks_{};
    // entity+6Ch per created instance, the expiry counter 00922fd0 writes.
    std::vector<int> expiry_counters_{};
    bool logged_empty_{false};
};

// 004dfc13's own branch, the load's session-slot reset. Returns true when the
// networked arm ran. In a local session the routine takes 004dfd18 and calls
// 004bb160, which is the reset the executable already performs, so the step is
// the recovered decision rather than a skipped arm.
bool run_load_session_slot_reset_004dfc13(GameHostLog& log, int session_mode,
    bool& single_player_reset);

// 004e0754..004e07c2: the two clears, the tree erase and the avoid-zone rebuild
// 00424d00 the singleton 004218e0 hands it.
void run_load_avoid_zone_state_004e0754(GameHostLog& log,
    const std::function<void()>& rebuild_geometry);

// 004d30f0: the name of every Lua global that is a function at the moment the
// scene reaches state 0Ch, which is the baseline teardown nils against.
std::size_t run_load_scripted_name_baseline_004d30f0(GameHostLog& log,
    GameMissionLuaHost& lua, std::vector<std::string>& names);

}  // namespace bsp::game
