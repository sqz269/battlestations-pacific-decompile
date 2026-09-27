#pragma once
// bsp_game.exe milestone 2g: the body of one fixed simulation step.
//
// Addresses: 00875e0c..00875edf (the sixteen per-step subsystem calls and their
// world gate 00875e69..00875e7f), 00875cc0..00875dfc (the three job waves over
// the five 68h groups at 00f876c0), 00875f32..00875fcc (the interpolation wave)
// and 00875fd1..00875ff7 (the tail hook). Milestone 2f ran the driver 00875bb0
// and recorded all four of those blocks as opaque host methods; this file runs
// the reconstructions bsp/fixed_step_fanout.hpp and bsp/fixed_step_job_waves.hpp
// put behind them.
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::FixedStepFanoutHost, bsp::FixedStepJobWaveHost or
// bsp::GameDynamicsBuoyancyHost, satisfied either by a reconstruction already on
// main or by the explicit unimplemented policy in GameHostLog.
//
// Evidence: docs/FIXED_STEP_FANOUT.md, docs/FIXED_STEP_JOB_WAVES.md,
// docs/IN_MISSION_SUBSYSTEM_TICK.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>

#include "bsp/fixed_step_fanout.hpp"
#include "bsp/fixed_step_job_waves.hpp"
#include "bsp/game_dynamics_list.hpp"

namespace bsp::game {

class GameHostLog;
class GameStepSubsystemsHost;
class GameNativeGameRuntime;

// Packet cc9_sentity_init_all (docs/CONSTRUCT_WORLD.md section 17). True: every
// route this process creates a mid-mission entity on feeds the pending list the
// image keeps at 00F899D0 (the 00926BE0 push_back 00928760 makes from the base
// constructor) and runs 00925F20 BSP_SEntity_InitAll where the image does: the
// fixed-step row 12 (00875EA2), GenerateObject's creator (0046DBE8) and its
// RunExtraFixedStep (00874D79 from 00945311), SpawnNew's CreateMembers
// (0094879A) and LaunchSquadron (0089E613). Pass A is the existing `thisTable`
// attach, run in the image's list order; passes B, C and E are named records
// (their per-class bodies are done piecemeal at creation by other hosts); the
// start branch and pass D are exact for single player. False: each route
// attaches at creation (attach_created_entity_00928a00 and
// attach_wing_member_tables), and row 12 is the named record. ON by the
// verdict: USN04, E2 and USN02 pairs moved only the predicted rows
// (docs/CONSTRUCT_WORLD.md section 17).
inline constexpr bool kSEntityInitAllBound = true;

// Packet cc9_init_attach_order (docs/SENTITY_INIT_ATTACH_ORDER.md). Needs
// kSEntityInitAllBound. True: the passes B and C of the InitAll walk run the
// steps that read the `thisTable` slot pass A made, after every pass A of the
// walk, as 00925F20 orders them:
//   - pass B, 009292B0 BSP_Unit_BindLuaClass on every node: `ClassID` (integer),
//     `Name` (the vt+10h name) and `Class` = VehicleClass[ClassID]. The
//     squadron reaches it at 007F218E; ships, carriers, planes and airfields
//     through 00955420 at 00955498;
//   - pass C, the plane's `thisTable.SquadronID` = the squadron's +174h id
//     (007C97E3..007C9805), for a plane whose +9D4h holds its squadron.
// The attach then seeds only `ID`, `Dead` and `Ptr`, as 00928A00 does. The
// default pass C 009295B0 reads a think-script name only from a kind-3 holder
// (the saved-game `_savedata._entities` data), so that step is not taken on a
// fresh mission start, and no node this process pushes uses 009295B0.
// False: pass A's attach writes `Class` (the stand-in for 009292B0's third
// field), and no `ClassID`, `Name` or `SquadronID` is written. ON by the
// verdict: the USN04, E2 and USN02 pairs moved only the predicted rows
// (docs/SENTITY_INIT_ATTACH_ORDER.md section 6).
inline constexpr bool kSEntityInitThisTableStepsBound = true;

// Packet cc9_init_pass_e_property_bag (docs/SENTITY_INIT_ATTACH_ORDER.md section
// 7). Pass E of 00925F20 releases the holder at entity+C0h (00926317
// CALL [vtable+0] with 1, then 00926319 stores 0). The holder owns a CLONE of
// the authored bag (00922E2D CALL 008F41F0), so the scene database's own bag
// outlives it. This process's per-entity copy of the authored values is the
// GameSceneEntityRecord its creator passes to create_units, a temporary that
// no reader holds past the creator call; the spawn pool and the scene
// entities stand for the scene database. True: the release is exact, logged as
// implemented, and counted. False: the named record. ON by the verdict: the
// USN04 and USN02 pairs moved only the row's status and the pass E summary line.
inline constexpr bool kSEntityInitPassEReleaseBound = true;

// Packet cc9_init_identity_gaps (docs/SENTITY_INIT_ATTACH_ORDER.md section 8).
// The scene read's InitAll calls (0046EB4B, 0046EB88, 0046EBC6, 0046ED0F) run
// passes B and C over the load-time instances too; this process attaches them
// in the mission frame's attach_scene_entities_00928a00 instead. True: after
// that attach loop (pass A over all), each scene unit gets 009292B0's
// `ClassID`, `Name` and `Class` (its pass B reaches 009292B0, section 1.1),
// and each marker of a class whose pass C is the default 009295B0 (NavPoint
// 41h, MovieCamPos 42h, MovieCamLookat 43h, Path 47h, CameraPath 4Ah,
// SimpleEffect 5Bh, PeriodicEffect 5Ch) gets 00928100's `Party` (+54h: the
// authored Party, else 00925E1D's 2), `Name` (vt+10h when +154h, the name
// length, is not 0) and `Type` (00E0CD80[+C4h], +C4h = the class id). The
// attach then seeds units without its `Class` stand-in. `Race` (+58h) is not
// written: the scene record does not carry an authored Race (contract,
// section 8). False: today's load attach. ON by the verdict: the USN04 and
// USN02 pairs moved only the two new rows and the summary line (section 8.6).
inline constexpr bool kSceneLoadThisTableIdentityBound = true;

// Packet cc9_pending_list_dedup (docs/SENTITY_INIT_ATTACH_ORDER.md section 9).
// The image pushes each entity once, from its base constructor (00928760 CALL
// 00926BE0). This process has two pushers for one construction: the Lua
// routes (GenerateObject, SpawnNew, LaunchSquadron, air-ops creation) and,
// once it lands, create_units. True, the list keeps one node per entity id:
//   - a plain push is skipped when the id is pending or already attached
//     (by a pass A, or by the load attach);
//   - a squadron push upgrades a pending plain node of the same id in place,
//     and drops the pending plain nodes of its own wing, so its pass A appends
//     them at the tail, which is where 007F4580's constructions push them;
//   - pass A's wing append skips an id already pending;
//   - the load attach drops pending nodes of the ids it attaches.
// With the Lua routes the only pusher (today), none of these fires. False: the
// list takes every push. ON by the verdict: identity on USN04 and USN02, no
// rule fired (section 9.5); the create_units push is what exercises it.
inline constexpr bool kPendingListDedupBound = true;

// Packet cc9_load_time_init_all (docs/SENTITY_INIT_ATTACH_ORDER.md section 10).
// BSP_SceneFile_Read 0046DF00 runs InitAll once per instantiate pass, at the
// first of 0046EB4B / 0046EB88 / 0046EBC6 / 0046ED0F it reaches (the latch
// [ESP+13h], set at 0046EB50 and 0046EB8D after the call), after the entity
// blocks (0046CF40) and before the traffic, groups and browser-groups blocks.
// True: the mission frame pushes every load-time instance (the units
// create_units made, then the scene markers) on the pending list, standing in
// for their constructors' 00928760 pushes, and runs one InitAll walk at
// 0046EB4B in place of attach_scene_entities_00928a00, so passes A..E and the
// section 8 identity writes come from the walk. Needs kSEntityInitAllBound.
// False: the load attach and its section 8 patch. ON by the verdict: the USN04
// and USN02 pairs moved only the predicted rows (section 10.4).
inline constexpr bool kLoadTimeInitAllBound = true;

// Packet cc9_run_extra_fixed_step (docs/SENTITY_INIT_ATTACH_ORDER.md section
// 11). 00874D00 BSP_Game_RunExtraFixedStep(CL flag), which GenerateObject
// (00945311) and Spawn (00944D93) call with CL = 1: the queued-call drain
// 00888230 (00874D0F); with CL = 0 only, the think pass 00929460 with the
// fixed step [00D0DE84] (00874D22); while the world is active (00874D2E..
// 00874D41), the session pump 00778450 with 0.0 for CL = 1 or the step
// (00874D68), 0077EC20, 00874C90, InitAll 00925F20 with CL = 0 (00874D79) and
// the outbound flush 0076FFC0 (00874DAF); with CL = 0 only, 00926700,
// 009273A0 and the tail jump to 00903610. True: the GenerateObject route runs
// the whole body through the fixed-step host's own row methods. False: the
// route runs the InitAll row alone. ON by the verdict (section 11.4).
inline constexpr bool kRunExtraFixedStepBound = true;

// Packet cc9_deck_tick_in_step (docs/SENTITY_INIT_ATTACH_ORDER.md section 12).
// The image ticks the air-ops decks (006CDC70) from the owner's motion virtual:
// 0075828E in 00758270 BSP_MotherShipUnit_UpdateMotion for a carrier and
// 006D254B in 006D2510 BSP_AirField_TickAdvance for an airfield. Unit motion
// runs in the step's job waves (00875CDD..00875DBD), before the fan-out whose
// row 12 (00875EA2) runs InitAll, so a launch the deck tick starts is attached
// in the same step. True: the deck tick runs once per fixed step right after
// the job waves, and the per-frame script timers no longer run it. False: the
// script timers run it after the frame's fixed step, and such a launch
// attaches one step later.
inline constexpr bool kDeckTickInFixedStepBound = false;

// The fixed-step host's 00874D00, for the Lua routes that call it.
class GameExtraFixedStepRunner {
public:
    virtual ~GameExtraFixedStepRunner() = default;
    virtual void run_extra_fixed_step_00874d00(bool flag, std::uint32_t call_site) = 0;
};

// The owner of the pending list and of 00925F20's per-entity work. The Lua host
// is the one, because pass A is its `thisTable` attach.
class GameEntityInitAllRunner {
public:
    virtual ~GameEntityInitAllRunner() = default;
    // 00925F20 at one native call site; `flag` is the CL byte the site passes
    // and `call_site` the CALL's address, for the log.
    virtual void run_sentity_init_all_00925f20(bool flag, std::uint32_t call_site) = 0;
};

// What the run's fixed steps did.
struct GameFixedStepSummary {
    unsigned long long steps{0};             // fixed steps whose body ran
    unsigned long long fanout_calls{0};      // sites reached, gated ones excluded
    unsigned long long fanout_concrete{0};   // of those, run by a reconstruction
    unsigned long long fanout_records{0};    // of those, the unimplemented policy
    unsigned long long gate_closed_steps{0}; // steps whose world gate was closed
    unsigned long long gated_calls_skipped{0};
    unsigned long long wave_groups{0};       // wave x group visits inside the loop
    unsigned long long interpolation_groups{0};
    unsigned long long elements_seen{0};     // elements the group walk yielded
    unsigned long long jobs_queued{0};
    unsigned long long group_dispatches{0};
    unsigned long long tail_gate_open{0};
    unsigned long long tail_gate_closed{0};
    std::size_t buoyancy_records{0};         // records the last 004462d0 walked
    unsigned long long native_physics_steps{0};
};

// The sixteen per-step calls, the four waves and the tail hook, as one owner.
// Held for the whole run because the counters are per run and the dynamics list
// behind call 2 is the mission frame host's.
class GameFixedStepHost final : public bsp::FixedStepFanoutHost,
                                public GameExtraFixedStepRunner,
                                public bsp::FixedStepJobWaveHost,
                                private bsp::GameDynamicsBuoyancyHost {
public:
    GameFixedStepHost(GameHostLog& log, bsp::GameDynamicsState& dynamics);

    // Milestone 2m: the six rows whose reconstructions are on main. Attached on
    // the load's own load_scene_contents row, because row 16 walks the entity
    // chain that step creates.
    void attach_subsystems(GameStepSubsystemsHost* subsystems) noexcept;
    // Borrow the completed actual game owner through its last fixed step.
    void attach_native_game(GameNativeGameRuntime* game) noexcept;
    // Packet cc9_sentity_init_all: row 12's owner. Attached for the whole run.
    void attach_entity_init(GameEntityInitAllRunner* runner) noexcept;
    // Packet cc9_run_extra_fixed_step.
    void run_extra_fixed_step_00874d00(bool flag, std::uint32_t call_site) override;

    // 00875cc0..00875dfc, waves 1..3 over the five groups, inside the step loop.
    void run_job_waves_00875cc0(std::uint8_t run_pass);
    // 00875e0c..00875edf, the sixteen subsystem calls with the world gate.
    void run_subsystems_00875e0c(float step, bool world_active);
    // 00875f32..00875fcc, the interpolation wave, once per frame after the loop.
    void run_interpolation_wave_00875670(float leftover, std::uint8_t run_pass);
    // 00875fd1..00875ff7, the four-test tail gate and its hook.
    void run_tail_00875fd1();

    void report();
    const GameFixedStepSummary& summary() const noexcept { return summary_; }

private:
    // --- bsp::FixedStepFanoutHost, one method per callee -------------------
    void simulate_physics_world_00c5c540(float step) override;
    void apply_dynamics_buoyancy_004462d0(float step) override;
    void refresh_moved_spatial_nodes_0098bdb0(float step) override;
    void run_fixed_step_callbacks_00874de0(float step) override;
    void drain_deferred_entity_events_00926700() override;
    void drain_queued_lua_calls_00888230() override;
    void run_due_entity_think_00929460(float step) override;
    void pump_session_00778450(float step) override;
    void apply_pending_entity_creates_0077ec20() override;
    void flush_pending_tick_registrations_00874c90() override;
    void init_pending_entities_00925f20(bool flag) override;
    void flush_outbound_session_0076ffc0(float step, std::int32_t mode) override;
    void flush_pending_entity_queues_009273a0() override;
    void release_expired_world_objects_00903610() override;
    void run_tail_hook_00a317f0() override;

    // --- bsp::FixedStepJobWaveHost ----------------------------------------
    bool next_element(std::size_t group, std::size_t position,
        bsp::JobWaveElementView& view) override;
    void queue_job(bsp::JobWavePhase phase, std::size_t group,
        std::size_t position) override;
    void dispatch_group(bsp::JobWavePhase phase, std::size_t group,
        std::uint8_t run_pass) override;
    void run_wave1_job_008750a0(std::size_t group, std::size_t position,
        float step) override;
    void run_wave2_job_00875b90(std::size_t group, std::size_t position,
        float step) override;
    void run_wave3_job_00874fe0(std::size_t group, std::size_t position,
        float step) override;
    void run_interpolation_job_00875160(std::size_t group, std::size_t position,
        float leftover) override;

    // --- bsp::GameDynamicsBuoyancyHost, behind call 2 ----------------------
    void body_world_transform_00c32000(std::uint32_t body, bsp::DynamicsVec3& axis,
        bsp::DynamicsVec3& center) override;
    bsp::DynamicsVec3 body_box_extent_00c31f90(std::uint32_t body) override;
    float water_height_0078cf20(float x, float z) override;
    float body_buoyancy_scalar_00c31fc0(std::uint32_t body) override;
    void apply_buoyancy_force_00c32050(std::uint32_t body, float force) override;
    void set_body_damping_00c37de0(std::uint32_t body, float damping) override;

    void record(const char* method, std::uint32_t address);
    void done(const char* method, std::uint32_t address);

    GameHostLog& log_;
    bsp::GameDynamicsState& dynamics_;
    GameStepSubsystemsHost* subsystems_{nullptr};
    GameNativeGameRuntime* native_game_{nullptr};
    GameEntityInitAllRunner* entity_init_{nullptr};
    // Packet cc9_run_extra_fixed_step: the world gate the last fixed step saw
    // ([[game+19CCh]+4ACh], 00874D3A reads the same byte), and the extra steps.
    bool last_world_active_{false};
    unsigned long long extra_steps_{0};
    GameFixedStepSummary summary_{};
    // session+278h (game+2168h), 00778450's countdown; -1.0 from 0076EE67.
    float session_countdown_278_{-1.0f};
    bool first_step_reported_{false};
    bool groups_reported_{false};
};

}  // namespace bsp::game
