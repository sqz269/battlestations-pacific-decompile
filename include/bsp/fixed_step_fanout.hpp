#pragma once
#include <cstddef>
#include <cstdint>

// The sixteen per-step subsystem calls of the fixed simulation step,
// 00875E0C..00875EDF, and the tail hook at 00875FD1..00875FF7.
//
// bsp/in_mission_subsystem_tick.hpp reconstructed the driver 00875BB0 as a clock
// rule plus four host methods and left FixedStepHost::run_step_subsystems with
// every callee unread. This header is that method's body: the call order, the
// world gate that skips five of the sixteen, and one pure-virtual per callee.
// docs/FIXED_STEP_FANOUT.md carries the evidence for each contract; the job
// waves of the same step are bsp/fixed_step_job_waves.hpp.
//
// Every name is a hypothesis, not a recovered symbol. The two exceptions are
// noted in the doc: the image spells "Simulate" inside 00C5C540 and
// "SEntity::InitAll" inside 00925F20.
//
// The float every call receives is 00D0DE84 = 0.05f (kFixedSimulationStepFloat
// in bsp/in_mission_subsystem_tick.hpp), never the frame delta.

namespace bsp {

// ---------------------------------------------------------------------------
// Owner areas
//
// MissionFrameOwner in bsp/mission_state_frame.hpp has no value for physics,
// the entity registry or the world object, and that header is not this packet's
// to edit, so the fan-out carries its own enum. The mapping is in the doc.
// ---------------------------------------------------------------------------
enum class FixedStepFanoutOwner : std::uint8_t {
    kPhysics,   // 00C5C540, the mitengine Dynamics block
    kUnit,      // 004462D0, the dynamics list
    kRegistry,  // 0042E630 / 0098BDB0, the singleton whose nodes hold entity poses
    kEngine,    // 00874DE0 / 00874C90, the per-step callback and tick registries
    kEntity,    // 00926700 / 00925F20 / 009273A0
    kScript,    // 00888230 / 00929460
    kSession,   // 00778450 / 0077EC20 / 0076FFC0
    kWorld,     // 00903610
};

// ---------------------------------------------------------------------------
// The world gate, 00875E69..00875E7F
//
// Rows 9..13 run only when the game singleton exists and the world object's
// +4ACh byte is set. A closed gate jumps straight to row 14 (JE 0x875EC4).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kFanoutWorldObjectOffset = 0x19cc;  // 00875E72
inline constexpr std::size_t kFanoutWorldActiveOffset = 0x4ac;   // 00875E78

struct FixedStepWorldGate {
    bool game_present{};  // [00E188A8] != 0,            00875E6E
    bool world_active{};  // [[game+19CCh]+4ACh] != 0,   00875E78
};

bool fixed_step_world_gate_open(const FixedStepWorldGate& gate) noexcept;

// ---------------------------------------------------------------------------
// The tail hook, 00875FD1..00875FF5
//
// Four tests in listing order, all of which must hold before 00875FF7 calls
// 00A317F0. That callee is a single RET in this build, so the hook is
// observably a no-op; the rule is kept because the gate is real.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kTailManagerSubObjectOffset = 0x54;  // 00875FE8
inline constexpr std::size_t kTailSubObjectFlagOffset = 0x05;     // 00875FF1

struct FixedStepTailGate {
    bool enabled_00f8ab04{};       // 00F8AB04 != 0,              00875FD7
    bool interface_manager{};      // 00E198C4 != 0,              00875FE4
    bool manager_sub_object{};     // [00E198C4+54h] != 0,        00875FEC
    bool sub_object_flag_05{};     // [[00E198C4+54h]+5h] != 0,   00875FF1
};

bool fixed_step_tail_hook_runs(const FixedStepTailGate& gate) noexcept;

// ---------------------------------------------------------------------------
// The call table
//
// One row per call site, in listing order. `callee_this` is the expression the
// site puts in ECX, written as it appears in the listing; it is empty for the
// calls that pass no this. `takes_step` is true when the callee's RET form
// shows it consumes the pushed 0.05f.
// ---------------------------------------------------------------------------
struct FixedStepFanoutStep {
    const char* host_method;
    std::uint32_t call_site;
    std::uint32_t callee;
    const char* callee_this;  // "" when the site sets no this
    bool takes_step;
    bool gated;  // true for the five rows behind the world gate
    FixedStepFanoutOwner owner;
    const char* reconstruction;  // "" when nothing on main implements the callee
};

inline constexpr std::size_t kFixedStepFanoutStepCount = 16;

std::size_t fixed_step_fanout_step_count() noexcept;
const FixedStepFanoutStep& fixed_step_fanout_step(std::size_t index) noexcept;

// ---------------------------------------------------------------------------
// The host
//
// Fifteen methods for sixteen call sites: drain_deferred_entity_events_00926700
// is called twice, at 00875E44 and 00875EC4, with the whole gated block between
// them. Every method is named for its callee's address because the names are
// hypotheses and the address is the evidence.
// ---------------------------------------------------------------------------
struct FixedStepFanoutHost {
    virtual ~FixedStepFanoutHost() = default;

    // 00875E0C, 00C5C540, ECX = [game+18h], RET 4. The Dynamics world's
    // Simulate: its own substep loop bounded by the budget in world[0Dh].
    virtual void simulate_physics_world_00c5c540(float step) = 0;

    // 00875E24, 004462D0, ECX = [game+30h], RET 4. Reconstructed in
    // bsp/game_dynamics_list.hpp as apply_dynamics_buoyancy_004462d0.
    virtual void apply_dynamics_buoyancy_004462d0(float step) = 0;

    // 00875E33 then 00875E3A. One statement: the getter 0042E630 (__cdecl, RET,
    // no arguments) supplies the this for 0098BDB0 (__thiscall, RET 4), which
    // consumes the float pushed at 00875E2F. The host takes them together
    // because splitting them would invite a caller to pass the step to the
    // getter, which is the error this packet corrected.
    virtual void refresh_moved_spatial_nodes_0098bdb0(float step) = 0;

    // 00875E3F, 00874DE0, RET. The site pushes nothing: the callee loads
    // 00D0DE84 itself (00874DF2) and calls vtable[+10h](0.05f) on every node of
    // the list at 00E0B748, unlinking the ones whose +19h repeating byte is
    // clear. The parameter is here because the callee uses the step, not
    // because the site passes it; `takes_step` is false in the table.
    virtual void run_fixed_step_callbacks_00874de0(float step) = 0;

    // 00875E44 and 00875EC4, 00926700, RET. Drains the queue at 00F899C4 and
    // clears the pending byte 00E18684.
    virtual void drain_deferred_entity_events_00926700() = 0;

    // 00875E55, 00888230, ECX = [game+1A08h], RET. Drains the mission Lua
    // host's queued named calls; docs/MISSION_LUA_HOST.md.
    virtual void drain_queued_lua_calls_00888230() = 0;

    // 00875E64, 00929460, RET 4. The think list at 00F89AB4 plus the
    // collectgarbage() countdown at 00F89A04.
    virtual void run_due_entity_think_00929460(float step) = 0;

    // 00875E91, 00778450, ECX = game+1EF0h, RET 4. Gated.
    virtual void pump_session_00778450(float step) = 0;

    // 00875E96, 0077EC20, RET. Gated.
    virtual void apply_pending_entity_creates_0077ec20() = 0;

    // 00875E9B, 00874C90, RET. Gated. Splices the pending tick registrations
    // into the five groups of bsp/fixed_step_job_waves.hpp.
    virtual void flush_pending_tick_registrations_00874c90() = 0;

    // 00875EA2, 00925F20, RET. Gated. SEntity::InitAll; the site zeroes only
    // CL (00875EA0 XOR CL,CL), so the argument is false.
    virtual void init_pending_entities_00925f20(bool flag) = 0;

    // 00875EBF, 0076FFC0, ECX = game+1EF0h, RET 8. Gated. The mode is the
    // literal 1 pushed at 00875EAD; BSP_Multiplayer_Tick passes 0.
    virtual void flush_outbound_session_0076ffc0(float step, std::int32_t mode) = 0;

    // 00875EC9, 009273A0, RET.
    virtual void flush_pending_entity_queues_009273a0() = 0;

    // 00875EDA, 00903610, ECX = [game+19CCh], RET. Releases objects two fixed
    // steps after they are marked.
    virtual void release_expired_world_objects_00903610() = 0;

    // 00875FF7, 00A317F0, RET. A single RET in this build: implementing it as
    // a no-op is faithful. It is here so a host can observe that the tail gate
    // opened.
    virtual void run_tail_hook_00a317f0() = 0;
};

// 00875EAD. The literal mode the fan-out passes to 0076FFC0; the frame-level
// caller 00778560 passes 0 (docs/GAME_SESSION_POLLS.md).
inline constexpr std::int32_t kFanoutOutboundSessionMode = 1;

// 00875EA0 XOR CL,CL. The only input 00925F20 reads from the site.
inline constexpr bool kFanoutInitAllFlag = false;

// 00875E0C..00875EDF as one sequence. `step` is 0.05f at every site.
void run_fixed_step_subsystem_fanout_00875e0c(
    float step, const FixedStepWorldGate& gate, FixedStepFanoutHost& host);

// 00875FD1..00875FF7. Calls the hook when the four tests hold, and reports
// whether it did.
bool run_fixed_step_tail_00875fd1(
    const FixedStepTailGate& gate, FixedStepFanoutHost& host);

} // namespace bsp
