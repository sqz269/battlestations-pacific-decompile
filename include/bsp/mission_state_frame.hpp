#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/simulation_gate.hpp"

// The mission state itself: what the two entry routines still leave opaque, and
// the in-mission branch of BSP_Game_OnMove (004e4a40) as one ordered sequence.
//
// docs/MISSION_STATE_FRAME.md carries the evidence. The entry decision itself is
// bsp/mission_state_entry.hpp (004db920, 004da6c0); the frame spine around this
// branch is bsp/game_frame_control.hpp (run_game_frame_control); the individual
// blocks this sequence dispatches into already have their own reconstructions,
// which mission_frame_step() names per call site.
//
// Every name below is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// Part 1: the two entry routines the mission-entry host still leaves opaque
// ---------------------------------------------------------------------------
// MissionStateEntryHost::release_deferred_dynamics and
// MissionStateEntryHost::check_multiplayer_player_count in
// bsp/mission_state_entry.hpp are the two native calls 004da6c0 makes that had
// no reconstruction. These are those two routines.

// 00447060 BSP_GameDynamicsList_ReleaseAll, __thiscall(sub), RET. ECX is
// game+30h (loaded at 004da6d1, dropped by the decompiler). The object holds
// two MSVC vectors: records of 20h bytes at +14h/+18h, and an 8h-stride range
// at +4h/+8h. Only the dword at record+0Ch is read.
inline constexpr std::size_t kDynamicsRecordStride = 0x20;       // 004470af
inline constexpr std::size_t kDynamicsRecordHandleOffset = 0x0c; // 004470a0
inline constexpr std::size_t kDynamicsPendingStride = 0x08;      // 00446f1b, inside 00446ef0
// The release target is captured once, at 00447060 itself: MOV EAX,[00e188a8]
// then MOV EBP,[EAX+18h]. It is a sub-object of the game singleton, not of the
// list, and the owning subsystem is not identified.
inline constexpr std::size_t kDynamicsOwnerOffset = 0x18; // 0044706a

// The handles the 20h records carry, and the element count of the 8h range.
// Neither element type was recovered: only record+0Ch is ever read, and the 8h
// range is copied wholesale by the erase, so its contents are opaque here.
struct GameDynamicsList {
    std::vector<std::uint32_t> record_handles{}; // +14h..+18h, one per 20h record
    std::size_t pending_count{};                 // (+8h - +4h) / 8h
};

struct DynamicsReleaseHost {
    virtual ~DynamicsReleaseHost() = default;
    // 004470a7: 00c34f70 with ECX = the owner captured at entry and the record's
    // +0Ch dword as the single stack argument. 00c34f70 reaches operator new and
    // _free, so it is a real release and not a handle table write.
    virtual void release_dynamics_handle_00c34f70(std::uint32_t handle) = 0;
};

// Releases every record handle, then clears both ranges, and returns the number
// of handles released. The native loop re-reads begin and end from +14h/+18h on
// every iteration (00447075..00447081) and stops as soon as the index reaches
// the current size, so a release that shrinks the vector ends the walk early;
// that is reproduced. Both erases are erase(begin, end), so both ranges end
// empty: the 20h vector inline at 004470b4..00447106, the 8h range through
// 00446ef0 (std::vector::erase with two checked iterators, RET 14h).
std::size_t release_all_dynamics_00447060(GameDynamicsList& list, DynamicsReleaseHost& host);

// ---------------------------------------------------------------------------
// 004d87b0 BSP_Game_CheckMultiplayerPlayerCount, __fastcall(GGame*), RET
// ---------------------------------------------------------------------------

// 004d87c1..004d87e6, three early returns in this order.
struct MissionPlayerCountInputs {
    std::int32_t local_view_mode{};  // +1FE4h, zero returns at 004d87c9
    bool scene_record_present{};     // +5FCh, null returns at 004d87d3
    std::uint32_t game_state{};      // +5D4h, anything but 0Dh returns at 004d87e0
    bool session_flag_29c{};         // +218Ch (= game+1EF0h + 29Ch), the arm selector
};

inline constexpr std::uint32_t kMissionGameState = 0x0D;

bool mission_player_count_check_runs(const MissionPlayerCountInputs& inputs) noexcept;

// 004d88c9..004d88e2. Effective game mode 7 (BSP_Game_GetEffectiveGameMode
// 004bca50) wants more than one participant in total; every other mode wants
// both sides non-empty.
inline constexpr int kMissionFreeForAllGameMode = 7; // 004d88c6
bool mission_player_counts_sufficient(int side0, int side1, int effective_game_mode) noexcept;

// 004d88b0: the retry deadline is the global clock at 00f876a4 plus the double
// at 00ce3db0, which holds 8.0.
inline constexpr float kMissionPlayerCountRetryDelaySeconds = 8.0f;
// 004d8905, the message key 004d87b0 raises through 00734e50 on a later failure.
inline constexpr const char* kMissionNotEnoughPlayersKey = "ingame.multi_notenoughplayer";
// 004d890d: the three side counters start at -99 so "never seen" is separable
// from "seen but empty". Only sides 0 and 1 are tested; the native code indexes
// the three-slot stack array with the slot's own +28h dword and never bounds
// checks it, so a slot claiming a side above 2 corrupts the frame. The
// reconstruction ignores such a slot instead, and that deviation is deliberate.
inline constexpr int kMissionSideCounterUnseen = -99;
inline constexpr std::size_t kMissionSideCounterCount = 3;

// One entry of the pointer array at game+18CCh as the balance walk reads it.
struct MissionSideSlot {
    bool excluded_09{};  // +9h, the slot exclusion byte; set skips the slot
    bool excluded_1b{};  // +1Bh, set marks the side seen but does not count it
    int side_28{};       // +28h, the side index
};

struct MissionSideBalance {
    int counts[kMissionSideCounterCount]{
        kMissionSideCounterUnseen, kMissionSideCounterUnseen, kMissionSideCounterUnseen};
};

// 004d8920..004d8960. A side first seen is pulled up from the sentinel to zero,
// then counted only when the slot's +1Bh byte is clear.
MissionSideBalance count_bound_side_entries(const MissionSideSlot* slots, std::size_t count) noexcept;

// 004d8962: exactly zero, so a side that stayed at the sentinel does not fire.
bool side_balance_broken(const MissionSideBalance& balance) noexcept;

enum class MissionPlayerCountOutcome : std::uint8_t {
    kNotRun,             // one of the three early returns
    kSufficient,         // the counts passed; the retry latch is cleared
    kFirstFailureArmed,  // first failure: deadline armed, +1EE5h set, returns
    kWarningShown,       // later failure: the message was raised
    kEndSceneRequested,  // mode 1 past the deadline: 004d7970(0) ran
    kSideBalanceOnly,    // session_flag_29c set: no counting, balance block only
};

struct MissionPlayerCountHost {
    virtual ~MissionPlayerCountHost() = default;

    // 004d8813: [00f8a2fc] and its +4Dh byte select the walk over the eight
    // 118h-stride participant blocks at game+748h instead of 004bb770.
    virtual bool participant_blocks_override_active() = 0;
    // 004d8850, 004bb770(&side0, &side1), the default participant count.
    virtual void count_participants_004bb770(int& side0, int& side1) = 0;
    // 004d8828, 004b5530 per block; the side is the block's +28h dword.
    virtual bool participant_block_active_004b5530(std::size_t block_index) = 0;
    virtual int participant_block_side(std::size_t block_index) = 0;
    // 004d8860, BSP_Game_GetEffectiveGameMode 004bca50.
    virtual int effective_game_mode_004bca50() = 0;

    // 00e18b44, the one-attempt latch, and 00e18b40, the deadline it arms.
    virtual bool retry_latch() = 0;
    virtual void set_retry_latch(bool value) = 0;
    virtual float retry_deadline() = 0;
    virtual void set_retry_deadline(float deadline) = 0;
    virtual float global_time_00f876a4() = 0;
    // 004d88b7, game+1EE5h = 1; the debrief 00920a20 reads it with game+2018h.
    virtual void set_mission_failure_byte() = 0;

    // 004d8905..004d8930, 00734e50 with the message key and the game object.
    virtual void show_message_00734e50(const char* text_key) = 0;
    // 004d8975, BSP_Game_EndScene 004d7970 with aborted = 0. This moves
    // game+5D4h, which is why 004da6c0 re-reads the state at 004da75a.
    virtual void end_scene_004d7970(bool aborted) = 0;

    // The side-balance continuation at 004d88eb. 00e188bd is its own one-shot.
    virtual bool side_balance_latch() = 0;
    virtual void set_side_balance_latch() = 0;
    // 004d8907, [[00e188a8]+5FCh]+988h: the scene record supplies the slot count.
    virtual std::size_t scene_slot_count() = 0;
    virtual MissionSideSlot scene_slot(std::size_t index) = 0;
    // 004d8974, 00982990 with (side0 != 0), then 00530650 through the menu
    // command screen singleton 00425d10.
    virtual void report_side_empty_00982990(bool side0_populated) = 0;
    virtual void dismiss_menu_prompts_00530650() = 0;
};

// The whole routine. Note the join: the participant-count arm's success path
// clears 00e18b44 at 004d88e4 and falls straight through into the side-balance
// block at 004d88eb, which the session_flag_29c arm jumps to. The balance check
// is therefore the shared continuation of both arms, not the alternative arm.
MissionPlayerCountOutcome run_mission_player_count_check_004d87b0(
    const MissionPlayerCountInputs& inputs, MissionPlayerCountHost& host);

// ---------------------------------------------------------------------------
// Part 2: the in-mission branch of BSP_Game_OnMove (004e4a40)
// ---------------------------------------------------------------------------

// Which area owns the native routine behind a step. The tags are the ones the
// packet brief asks for; "pure" means the step has no native call at all.
enum class MissionFrameOwner : std::uint8_t {
    kPure,
    kInput,
    kScript,
    kUnit,
    kHud,
    kRenderer,
    kSound,
    kSession,
    kProfiler,
};

// One row per host method, in native call order. `reconstruction` names the
// symbol on main that already implements the step, or is empty when none does;
// wiring a headless mission frame means implementing each host method by
// forwarding to that symbol.
struct MissionFrameStep {
    const char* host_method;      // the MissionFrameHost method
    std::uint32_t call_site;      // the address of the CALL inside 004e4a40
    std::uint32_t callee;         // the routine it enters, 0 for an inlined step
    MissionFrameOwner owner;
    const char* reconstruction;   // "" when nothing on main implements it yet
};

std::size_t mission_frame_step_count() noexcept;
const MissionFrameStep& mission_frame_step(std::size_t index) noexcept;

// The game fields this branch reads or writes, named by native offset. The
// request queue, the scaled delta and the state value itself are produced by
// the spine in bsp/game_frame_control.hpp; they are inputs here.
struct MissionFrameState {
    std::uint32_t state{};                 // +5D4h
    float raw_delta{};                     // the OnMove argument slot after 004c6e30 rewrote it
    float scaled_delta{};                  // +21F0h
    float global_time{};                   // 00f876a4, the erase and particle references
    bool cinematic_hidden{};               // +634h
    bool cinematic_allow_simulation{};     // +635h
    bool mission_end_suspended{};          // +7184h
    bool mission_start_latched{};          // +1EE7h, the one-shot at 004e4e00
    bool engine_movie_pending{};           // +1EE0h, written by 004c9ca0 at mission entry
    bool session_flag_624{};               // +624h, suppresses the counter bump
    bool warning_manager_present{};        // +21E0h
    bool ocean_owner_present{};            // +19E8h
    bool max_step_clamp_disabled{};        // 00f876b0 > 0
    bool game_block_label_registered{};    // bit 1 of 00e18b58
    std::uint32_t frame_counter{};         // +648h, incremented at 004e4ff5
    float scene_shader_time{};             // [[+5FCh]+1054h], the 00af0450 argument
};

// One method per native call site of the in-mission branch, in call order.
// Nothing has a default implementation: none of these stands in for native
// behaviour that was not recovered.
struct MissionFrameHost {
    virtual ~MissionFrameHost() = default;

    // --- the mission-start one-shot, 004e4e00..004e4e2a ------------------
    // 004e4e0f, ECX = game+1EF0h (the session sub-object, not the game).
    virtual bool session_counts_mission_start_004b6260() = 0;
    // 004e4e25, argument 1. Bumps the two counters at game+740h/+73Ch.
    virtual void adjust_mission_start_counters_004bcaa0(bool add) = 0;

    // --- the warning director and the input effect sets ------------------
    // 004e4e67, ECX = game+21E0h, one float. Runs only with a positive scaled
    // delta (COMISS against the 0.0f at 00d7a218, JBE skips).
    virtual void update_warning_manager_00987590(float scaled_delta) = 0;
    // 004e4e6c..004e4fe4 as one unit: the suppression set at game+5B0h, the
    // record-1 clear when game+5C4h is non-zero, and the timed set at +5BCh
    // with its start, continue and erase arms. Returns entries erased.
    virtual int run_input_effect_sets_004e4e6c(float expiry_reference) = 0;

    // --- frame bookkeeping, 004e4feb..004e50ab ---------------------------
    // 004e4feb, ECX = game. On the join every path reaches, before +648h += 1.
    virtual void record_action_deadlines_004d8cd0() = 0;
    // 004e5036, ECX = [00e188a8]+1EF0h, the raw delta. Skipped in the mission
    // state when the clamp is disabled and the scaled delta is positive.
    virtual void multiplayer_tick_00778560(float raw_delta) = 0;
    virtual void begin_game_profile_block() = 0; // 004e509d getter, 004e50ab, counter 0109db08
    virtual void end_game_profile_block() = 0;   // 004e54a2 getter, 004e54b0

    // --- the simulation gate, 004e50b0..004e5259 -------------------------
    // 004e50c6..004e5110, the one-shot label "GGame::OnMove::game" at 00ce82e4.
    virtual void register_game_block_label() = 0;
    // 004e5133, ECX = game. A fixed four-call sequence, each with the scaled
    // delta; it tests nothing of its own.
    virtual void update_in_mission_subsystems_004c40a0() = 0;
    // 004e5147, behind the +1EE0h latch the gate clears at 004e514c. Starts the
    // <mission>.ema in-engine movie and switches to INTF_ENGINEMOVIE.
    virtual void begin_engine_movie_004cce50() = 0;
    // 004e5153..004e5211 as one decision; decide_simulation_gate_branch in
    // bsp/simulation_gate.hpp is the recovered rule. Only kSimulationOnly,
    // kPauseMenuOpened and kInterfaceOnly occur here: kGateClosed belongs to
    // the outer gate, which run_mission_frame has already passed.
    virtual SimulationGateBranch pause_gate_branch_004e5153() = 0;
    // 004e521c..004e522b: the tutorial arm needs the hint object at
    // [[00e198c4]+A8h] to report its +8h byte set.
    virtual bool tutorial_hint_step_available() = 0;
    // 004e522d, ECX = [00e198c4]+A8h. Advances the basic-training hint one step.
    virtual void advance_tutorial_hint_0054e440() = 0;
    // 004e5234 and 004e523b, ECX = game. The tutorial arm falls through into
    // the second call site, so that arm invokes the routine twice in a row.
    virtual void toggle_pause_menu_004db030() = 0;
    virtual bool in_game_interface_active() = 0; // 00e198c4 and its +3Ch
    // 004e5252, ECX = 00e198c4. More than an audio-environment switch.
    virtual void update_in_game_interface_0068c1f0() = 0;
    // 004e5259, and again at the gate fallback 004e53b6 and inside the menu
    // drain at 004e5469. ECX = game.
    virtual void update_interface_only_004c40f0() = 0;

    // --- the seven hint passes, 004e525e..004e52b5 -----------------------
    // Each is 004e1ca0 (the 0A0h hint singleton) then one update; the getter is
    // the host's business, the seven updates are these.
    virtual void hint_tick_cooldowns_0068ec10(float raw_delta) = 0; // 004e526d
    virtual void hint_drain_queued_00692b00() = 0;                  // 004e5279
    virtual void hint_unit_class_00692b60() = 0;                    // 004e5285
    virtual void hint_weapon_006926f0() = 0;                        // 004e5291
    virtual void hint_environment_00692580() = 0;                   // 004e529d
    virtual void hint_zone_first_get_00692fd0() = 0;                // 004e52a9
    virtual void hint_strategic_map_00692960() = 0;                 // 004e52b5

    // --- the world tick, 004e52ba..004e5389 ------------------------------
    virtual void update_bot_scheduler_00914ef0(float scaled_delta) = 0;  // 004e52ca, ECX +21A0h
    virtual void update_markers_006dc1a0(float scaled_delta) = 0;        // 004e52df, ECX +21D4h
    virtual void update_entity_manager_00481640(float scaled_delta) = 0; // 004e52f4, ECX +21D0h
    // 004e5302 getter then 004e5309; runs only while +19E8h is set.
    virtual void update_rain_descriptor_00865ab0() = 0;
    // 004e5325, ECX = +19E8h, arguments (+19FCh, scaled delta).
    virtual void update_ocean_00bbddd0(float scaled_delta) = 0;
    virtual void update_decals_00740e10(float scaled_delta) = 0;      // 004e533a, ECX 00e1aea0
    // 004e534f, ECX = 00f89b3c. The shipped body is trivial; the call is kept
    // because the object and the argument are real.
    virtual void update_00f89b3c_0094c8f0(float scaled_delta) = 0;
    virtual void update_power_ups_008eb110() = 0;                     // 004e535a, ECX 00f88c30
    // 004e5370 getter (004d1100) then 004e5377 with (scaled delta, +19FCh).
    virtual void update_effect_manager_00867ee0(float scaled_delta) = 0;
    virtual void flush_entity_activations_00903670() = 0;             // 004e5382, ECX +19CCh
    // 004e5389, ECX = game. True when it enqueued the debrief request 0Fh;
    // bsp/game_frame_control.hpp check_mission_completion_004d7ea0 is the rule.
    virtual bool check_mission_completion_004d7ea0() = 0;

    // --- the particle clock, 004e538e..004e53ad --------------------------
    // 004e53a6 getter then 004e53ad. The argument is the global time in
    // milliseconds, not a delta: 00f876a4 * the double 1000.0 at 00ce47a0.
    virtual void set_particle_clock_time_00b19a10(float time_milliseconds) = 0;

    // --- post-simulation, 004e53bb..004e549d -----------------------------
    // 004e53d9, ECX = 00f8c274, argument [[game+5FCh]+1054h].
    virtual void set_foliage_shader_time_00af0450(float scene_time) = 0;
    // 004e540f, ECX = 00f8c274, arguments (+19FCh, the camera chosen from
    // [[+19F0h]+A8h] by its +CCh byte: +D0h when set, +C8h when clear).
    virtual void build_foliage_visible_set_00af0c50() = 0;
    virtual void apply_gui_visibility_004c6c70() = 0;   // 004e5416, ECX = game
    virtual bool interface_manager_present() = 0;       // 00e198ac
    // 004e542d, ECX = 00e198ac, the raw delta. Re-applies the music volume at
    // 00f889a8 to the two stream objects at +50h/+54h and advances their fades.
    virtual void update_interface_music_00685c80(float raw_delta) = 0;
    virtual void update_multiplayer_interface_004d80d0() = 0; // 004e5434, ECX = game
    // 004e5442 and 004e5477. Returns the out byte at [esp+1Bh]: true when a
    // menu channel actually needed servicing, which drives the drain loop.
    virtual bool service_pending_menu_requests_006840f0() = 0;
    // 004e5462, ECX = game+1EF0h. Only reachable from the drain loop Ghidra
    // drops from the pseudocode.
    virtual void pump_peer_queues_00776230() = 0;
    // 004e548f getter then 004e5496, the sound cue request queue.
    virtual void apply_sound_requests_00941140() = 0;
    virtual void update_front_end_screens_004d8620() = 0; // 004e549d, ECX = game

    // --- render, 004e54a2..004e54eb --------------------------------------
    virtual void begin_render_profile_block() = 0; // 004e54bc getter, 004e54ca, counter 0109db14
    virtual void render_004ca440() = 0;            // 004e54d1
    virtual void end_render_profile_block() = 0;   // 004e54d6 getter, 004e54e4
    virtual void finish_render_frame_004ca1f0() = 0; // 004e54eb

    // --- tail, 004e5512..004e551e ----------------------------------------
    virtual bool frame_metrics_enabled() = 0;                       // 00e1aed4
    virtual void submit_frame_metrics_00757ce0(float raw_delta) = 0; // 004e551e
};

struct MissionFrameResult {
    bool simulated{};                    // the gate at 004e50b0/004e5118 opened
    bool paused{};                       // the pause branch ran
    bool mission_completion_requested{}; // 004d7ea0 enqueued request 0Fh
    int input_entries_erased{};          // from the timed set walk
    int menu_drain_iterations{};         // passes of the 004e5455 loop
};

// The in-mission branch in native order, starting at the state test at
// 004e4df4 and ending at the metrics call at 004e551e. The spine before it
// (pre-tick, input poll, window close, the render-queue open, the request
// drain, the delta scale and the frame clock) is run_game_frame_control in
// bsp/game_frame_control.hpp; this routine assumes it already ran and that
// state.state, state.raw_delta and state.scaled_delta carry its results.
MissionFrameResult run_mission_frame(MissionFrameState& state, MissionFrameHost& host);

// ---------------------------------------------------------------------------
// Part 3: the exit
// ---------------------------------------------------------------------------
// The state request that leaves 0Dh. 004d7ea0 is the only producer inside the
// frame; the drain dispatches it to BSP_Game_EndScene 004d7970, which commits
// or discards the mission record and enqueues request 10h. What 004d7970 does
// is docs/MISSION_RESULT_DECISION.md and is not re-derived here.
inline constexpr std::uint32_t kMissionDebriefRequest = 0x0F;  // 004d7ea0
inline constexpr std::uint32_t kMissionTeardownRequest = 0x10; // 004d7970
// The five one-shot bytes 004da6c0 clears on entry are read again on the exit
// path: +1EE1h takes the aborted flag, +1EE2h latches the once-per-scene body.
// bsp/mission_state_entry.hpp MissionOneShots is the entry-side projection.
inline constexpr std::size_t kMissionAbortedByteOffset = 0x1ee1; // 004d79a6
inline constexpr std::size_t kMissionEndLatchOffset = 0x1ee2;    // 004d799f, 004d7a49
}
