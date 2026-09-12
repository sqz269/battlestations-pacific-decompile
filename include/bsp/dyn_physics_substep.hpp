#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/dyn_contact_solver.hpp"
#include "bsp/dyn_world_settings.hpp"
#include "bsp/rigid_body_integration.hpp"

// The Dynamics world's step in full: 00C5C540 (the schedule row 1 of the fixed-step
// fan-out calls) and 00C5BB30 (the substep body it drives).
//
// docs/DYN_PHYSICS_SUBSTEP.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol.
//
// docs/RIGID_BODY_INTEGRATION.md reconstructed the two integration phases and left
// 00C5C540 "partial: the proxy transform copy and the profiler bookkeeping are
// described, not reconstructed", and 00C5BB30 "read for its phase order and call sites
// only". This header closes both: the copy is a pure rule, the profiler bookkeeping is
// two host methods, and the substep body is a sequence with one pure-virtual per call
// site. The contact phase's own rules are bsp/dyn_contact_solver.hpp.

namespace bsp {

// ---------------------------------------------------------------------------
// The world fields 00C5C540 and 00C5BB30 touch that
// bsp/dyn_world_settings.hpp does not already carry
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynWorldStepCounterOffset = 0x2c;         // 00C5C5F5, an int
inline constexpr std::size_t kDynWorldAccumulatorOffset = 0x48;         // 00C5C63F
inline constexpr std::size_t kDynWorldBodyListHeadOffset = 0x204;       // 00C5C604
inline constexpr std::size_t kDynWorldBodyListSentinelOffset = 0x208;   // 00C5C60A
inline constexpr std::size_t kDynBodyListNextOffset = 0x84;             // 00C5C627
inline constexpr std::size_t kDynWorldPendingRemovalsOffset = 0x438;    // 00C4D980
inline constexpr std::size_t kDynWorldPendingRemovalCountOffset = 0x43c;
inline constexpr std::size_t kDynWorldSceneOffset = 0x444;              // 00C5BB5F
inline constexpr std::size_t kDynWorldContactGroupsOffset = 0x448;      // 00C5BBAB
inline constexpr std::size_t kDynWorldContactListenerOffset = 0x24;     // 00C5C124, 00C5C431
inline constexpr std::size_t kDynWorldSolverModeOffset = 0x10;          // 00C5BBF2

// ---------------------------------------------------------------------------
// The proxy transform copy, 00C5C614..00C5C62F
//
// `REP MOVSD` of 12 dwords from B+08h to [B+04h]+84h, for every body on the world's
// list, before the substep loop and after the pending removals are flushed. It is the
// producer of M+84h..+B3h, which docs/RIGID_BODY_INTEGRATION.md records as "the
// previous 3x4 transform" and which 00C43EA0 interpolates against. It runs whatever
// B+50h says, so a static or sleeping body's previous transform is refreshed too.
// ---------------------------------------------------------------------------
struct DynPreviousTransform {
    float row0[3]{1.0f, 0.0f, 0.0f};  // M+84h
    float row1[3]{0.0f, 1.0f, 0.0f};  // M+90h
    float row2[3]{0.0f, 0.0f, 1.0f};  // M+9Ch
    float position[3]{};              // M+A8h
};

void dyn_copy_previous_transform(const DynBody& body, DynPreviousTransform& previous) noexcept;

// One element of the world's body list, as the copy loop sees it: the node itself and
// the previous-transform slot inside its motion state. A null `body` is the sentinel
// at world+208h.
struct DynRegisteredBody {
    DynBody* body{nullptr};
    DynPreviousTransform* previous{nullptr};
};

// ---------------------------------------------------------------------------
// The profiler scopes
//
// [0109E9F8] is the profiler. Its +4h is the top of the active scope stack and the
// slots below cache one record per label; 00C50390(label, id) creates the record the
// first time the slot is empty. A scope pushes the record, takes an rdtsc, and on pop
// writes the delta to record+30h/+34h, adds it into record+38h/+3Ch and bumps the call
// count at record+40h. Those are exactly the three dwords 00C321B0 clears.
// ---------------------------------------------------------------------------
struct DynProfilerScopeSlot {
    const char* label;
    std::int32_t id;
    std::uint32_t profiler_slot;  // the byte offset inside [0109E9F8]
    std::uint32_t create_site;    // the 00C50390 / 00C57020 call site
};

inline constexpr std::size_t kDynProfilerScopeCount = 7;
const DynProfilerScopeSlot& dyn_profiler_scope(std::size_t index) noexcept;

// ---------------------------------------------------------------------------
// 00C5C540, the schedule
//
// __thiscall void(world, float), RET 4 at 00C5C704, body 00C5C540..00C5C706. Sole
// caller 00875E0C with ECX = [[00E188A8]+18h] and the fixed step 0.05f.
// ---------------------------------------------------------------------------
struct DynSimulateHost {
    virtual ~DynSimulateHost() = default;

    // 00C5C586, 00C321B0, __fastcall(node). Called once per child of the counter tree
    // root at [[0109E9FC]+0Ch]; the callee recurses over its own children (+4h array,
    // +8h count) and zeroes +38h, +3Ch and +40h on every node it reaches. The wrapper
    // then zeroes the root's own three and bumps the frame counter at
    // [[0109E9FC]+0Ch]+8h. The host takes the whole reset because the native loop
    // bound is re-read from the root every iteration (00C5C58B).
    virtual void reset_profiler_counter_tree_00c321b0() = 0;

    // 00C5C5C5, 00C50390("Simulate", 1), and the push/pop around the body. One method
    // for the pair because the pop is the same inline code at 00C5C6CE.
    virtual void push_profiler_scope(const DynProfilerScopeSlot& scope) = 0;
    virtual void pop_profiler_scope(const DynProfilerScopeSlot& scope) = 0;

    // 00C5C5FF, 00C4D980, __thiscall(world). Drains the pending-removal array at
    // world+438h (count world+43Ch), then sets that count to 0. Per body: when B+70h
    // is non-zero it releases the proxy handle at B+60h through slot 1 of the vtable
    // of [[B+00h]+444h]+0ACh (B+00h is the owning world, so that is the scene's proxy
    // manager), clears B+50h bit 3 and zeroes B+60h (00C4D9A9..00C4D9CC); then it
    // destroys every node of the contact list at B+70h through the node's own vtable
    // slot 1 with the argument 1 and clears B+70h; then 00C43AA0 and 00C43C00; then it
    // unlinks the body (prev B+80h, next B+84h) and pushes it onto the pool at
    // world+4Ch when B+50h bit 0 is set and world+170h otherwise, decrementing that
    // pool's count at +120h. 004DA780 (BSP_Game_TeardownSessionState) calls it for the
    // same reason, which is what fixes it as a removal flush rather than a step pass.
    virtual void flush_pending_body_removals_00c4d980() = 0;

    // world+204h and B+84h. Field reads, not calls: the schedule walks the list itself.
    virtual DynRegisteredBody first_registered_body() = 0;
    virtual DynRegisteredBody next_registered_body(DynBody* body) = 0;

    // 00C5C66D and 00C5C6BD, 00C5BB30, __cdecl void(world, float) with the world
    // pushed second and the substep's own dt first.
    virtual void run_substep_00c5bb30(float dt) = 0;
};

struct DynSimulateResult {
    DynWorldSubstepPlan plan{};             // what the accumulator loop decided
    std::int32_t previous_transforms_copied{0};
    std::int32_t step_counter{0};           // world+2Ch after the increment
};

// 00C5C540 in full. `accumulator` is world+48h and is left at zero, which is what the
// XORPS/MOVSS pair at 00C5C6C6 does whichever way the loop exited. `step_counter` is
// world+2Ch, an int incremented once per call at 00C5C5F5 (before the removals flush,
// so a step that ends up taking no substep at all still counts).
DynSimulateResult dyn_physics_world_simulate_00c5c540(const DynWorldSettings& settings,
                                                      float& accumulator,
                                                      std::int32_t& step_counter, float dt,
                                                      DynSimulateHost& host);

// ---------------------------------------------------------------------------
// 00C5BB30, the substep body
//
// __cdecl void(world, float dt), body 00C5BB30..00C5C53E, called only from 00C5C540.
// Nine call sites in listing order; the two Solve blocks are the same code against two
// task arrays, so they are one host method with the mode as an argument.
// ---------------------------------------------------------------------------
struct DynSubstepHost {
    virtual ~DynSubstepHost() = default;

    // 00C5BB5A, 00C41550 with the world in ESI and dt on the stack. Reconstructed as
    // dyn_body_integrate_velocity_00c41550 in bsp/rigid_body_integration.hpp.
    virtual void integrate_velocities_00c41550(float dt) = 0;

    // 00C5BB66, 00C57070(scene) where scene is [world+444h]. The collision pass: its
    // body carries the image strings "BroadPhase", "BroadPhaseUpdate", "IntersectLoop",
    // "GetManifold", "ManifoldUpdate" and "Collide", and it dispatches its own task
    // batch through 00C33140 at 00C577EE. Contract partial: this packet read its call
    // site, its argument and its strings, not its body. It is the producer of the
    // manifold list the contact phase then reads.
    virtual void run_collision_pass_00c57070() = 0;

    // 00C5BBBE, 00C4B610(world+448h). dyn_create_contact_groups_00c4b610.
    virtual void create_contact_groups_00c4b610() = 0;

    virtual void push_profiler_scope(const DynProfilerScopeSlot& scope) = 0;
    virtual void pop_profiler_scope(const DynProfilerScopeSlot& scope) = 0;

    // world+10h at 00C5BBF2, world+458h at 00C5BC06 / 00C5C02A, world+460h at
    // 00C5C054 and world+478h at 00C5BC31. Field reads, not calls.
    virtual std::int32_t solver_mode() = 0;
    virtual std::int32_t contact_group_count() = 0;
    virtual std::int32_t solver_task_capacity(DynSolverTaskKind kind) = 0;

    // 00C5BC7A..00C5BC82 and 00C5C0CA..00C5C0D9: the three stores into task+0Ch,
    // +10h and +14h. Field writes, not calls.
    virtual void set_solver_task_range(DynSolverTaskKind kind, std::int32_t task_index,
                                       const DynSolverTaskRange& range) = 0;

    // 00C5BCCA and 00C5C0E8, 00C33140 with the task-pointer vector in EAX
    // (world+480h for kLcpSolver2Task, world+468h for kLcpSolverTask), the scheduler
    // [[0109E9FC]+10h] in ECX and the count pushed. The callee takes the scheduler's
    // critical section, appends the tasks, tags each with the batch index in task+4h,
    // releases the semaphore at scheduler+350h by the count, and blocks on the batch's
    // event at scheduler+1C0h + batch*4 until every task has run. A fork-join, so the
    // substep is single-threaded again when it returns.
    virtual void dispatch_solver_tasks_00c33140(DynSolverTaskKind kind,
                                                std::int32_t task_count) = 0;

    // world+24h at 00C5C124 / 00C5C431. Field read, not a call.
    virtual bool has_contact_listener() = 0;

    // The manifold walk the record builder runs, 00C5C13D..00C5C42D. Field reads.
    virtual std::int32_t scene_manifold_count() = 0;
    virtual DynHandle manifold_list_head() = 0;
    virtual DynHandle manifold_list_sentinel() = 0;
    virtual DynHandle manifold_next(DynHandle manifold) = 0;
    virtual std::int32_t manifold_point_count(DynHandle manifold) = 0;
    virtual DynContactPoint manifold_point(DynHandle manifold, std::int32_t index) = 0;
    virtual const DynBody& manifold_body_a(DynHandle manifold) = 0;
    virtual const DynBody& manifold_body_b(DynHandle manifold) = 0;

    // 00C5C43A, CALL EDX through [[world+24h]] slot 0, __thiscall RET 8 (no stack
    // cleanup at the site). ECX is the listener, the array is pushed at 00C5C439 and
    // the total point count at 00C5C438. The array is heap memory the substep frees at
    // 00C5C449, so the listener must not retain it.
    virtual void report_contacts_world_24h(const DynContactReportRecord* records,
                                           std::int32_t count) = 0;

    // 00C5C491, 00C5B1B0 with the world in EBX and dt on the stack. Reconstructed as
    // dyn_body_integrate_position_00c5b1b0 in bsp/rigid_body_integration.hpp.
    virtual void integrate_positions_00c5b1b0(float dt) = 0;

    // 00C5C503, 00C4B550(world+448h). dyn_sleep_contact_groups_00c4b550.
    virtual void sleep_contact_groups_00c4b550() = 0;
};

struct DynSubstepResult {
    bool solve_ran{false};             // world+458h != 0 and the mode was known
    std::int32_t solver_task_count{0};
    bool contacts_reported{false};
    std::int32_t contact_record_count{0};
};

// 00C5BB30 as one sequence. `records` is the caller's buffer for the contact report;
// the native allocates `scene_manifold_count() * 4` records of 58h bytes at 00C5C16D
// and frees them at 00C5C449, so a caller sizes it the same way.
DynSubstepResult dyn_world_substep_00c5bb30(float dt, DynContactReportRecord* records,
                                            std::size_t record_capacity, DynSubstepHost& host);

}  // namespace bsp
