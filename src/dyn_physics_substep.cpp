#include "bsp/dyn_physics_substep.hpp"

// 00C5C540, the Dynamics world's step, and 00C5BB30, the substep it drives.
// docs/DYN_PHYSICS_SUBSTEP.md carries the addresses, the original ABI and the
// uncertainty.

namespace bsp {
namespace {

// The seven profiler scopes the step opens, in the order the listing reaches
// them. `profiler_slot` is the byte offset of the cached record inside the profiler at
// [0109E9F8]; the id is the second argument 00C50390 takes.
constexpr DynProfilerScopeSlot kScopes[kDynProfilerScopeCount] = {
    {"Simulate", 0x01, 0x10, 0x00c5c5c5},       // 00C5C540, slot tested at 00C5C5B3
    {"CreateGroups", 0x16, 0x64, 0x00c5bb83},   // 00C5BB30, slot tested at 00C5BB70
    // The "Solve" scope goes through 00C57020, which caches the record in a slot this
    // packet did not read; the label (00D79F34) and the id come from the caller, and
    // both 00C5BC12/00C5BC1A and 00C5C037/00C5C03C push the same pair.
    {"Solve", 0x18, 0x00, 0x00c5703c},
    {"SolverPreStep", 0x19, 0x70, 0x00c5c7d3},     // inside 00C5C7A0, the solver task
    {"SolveConstraints", 0x1a, 0x74, 0x00c5c72f},  // inside 00C5C710, the solver task
    {"UpdatePosition", 0x1b, 0x78, 0x00c5c469},    // 00C5BB30, slot tested at 00C5C456
    {"SleepGroups", 0x1c, 0x7c, 0x00c5c4ce},       // 00C5BB30, label pushed at 00C5C4C9
};

}  // namespace

const DynProfilerScopeSlot& dyn_profiler_scope(std::size_t index) noexcept {
    if (index >= kDynProfilerScopeCount) {
        index = 0;
    }
    return kScopes[index];
}

void dyn_copy_previous_transform(const DynBody& body, DynPreviousTransform& previous) noexcept {
    // 00C5C614..00C5C62F: ESI = B+08h, EDI = [B+04h]+84h, ECX = 0Ch, REP MOVSD. Twelve
    // dwords, which is the 3x4 transform whole; the rows and the position keep their
    // order because the copy is a straight block move.
    for (int i = 0; i < 3; ++i) {
        previous.row0[i] = body.row0[i];
        previous.row1[i] = body.row1[i];
        previous.row2[i] = body.row2[i];
        previous.position[i] = body.position[i];
    }
}

DynSimulateResult dyn_physics_world_simulate_00c5c540(const DynWorldSettings& settings,
                                                      float& accumulator,
                                                      std::int32_t& step_counter, float dt,
                                                      DynSimulateHost& host) {
    DynSimulateResult result{};

    // 00C5C55E..00C5C5AA, before the profiler scope opens: the counter tree is reset
    // for the new frame and the frame counter bumped.
    host.reset_profiler_counter_tree_00c321b0();

    const DynProfilerScopeSlot& simulate = kScopes[0];
    host.push_profiler_scope(simulate);

    // 00C5C5F5, between the two RDTSCs: an int increment on world+2Ch. It counts calls
    // to Simulate, not substeps, so a step whose budget is exhausted still counts.
    step_counter += 1;
    result.step_counter = step_counter;

    // 00C5C5FF. Bodies queued for removal leave the world before their transforms are
    // published, so a removed body never contributes a previous transform.
    host.flush_pending_body_removals_00c4d980();

    // 00C5C604..00C5C62F. Every body on world+204h, the sentinel being world+208h and
    // the next pointer B+84h. No flag is tested: a static or sleeping body's previous
    // transform is refreshed too.
    for (DynRegisteredBody entry = host.first_registered_body(); entry.body != nullptr;
         entry = host.next_registered_body(entry.body)) {
        if (entry.previous != nullptr) {
            dyn_copy_previous_transform(*entry.body, *entry.previous);
        }
        result.previous_transforms_copied += 1;
    }

    // 00C5C639..00C5C6C9. The accumulator rule already lives in
    // bsp/dyn_world_settings.hpp, which is where the budget's integer type was settled;
    // this routine executes the plan it returns rather than restating it.
    result.plan = dyn_world_substep_plan(settings, accumulator, dt);
    for (int i = 0; i < result.plan.full_substeps; ++i) {
        host.run_substep_00c5bb30(result.plan.substep_dt);  // 00C5C66D
    }
    if (result.plan.remainder_substep) {
        host.run_substep_00c5bb30(result.plan.remainder_dt);  // 00C5C6BD
    }

    host.pop_profiler_scope(simulate);
    return result;
}

DynSubstepResult dyn_world_substep_00c5bb30(float dt, DynContactReportRecord* records,
                                            std::size_t record_capacity,
                                            DynSubstepHost& host) {
    DynSubstepResult result{};

    // 00C5BB5A. The first phase runs before any profiler scope opens and before the
    // collision pass, so the velocities the narrow phase sees are this substep's.
    host.integrate_velocities_00c41550(dt);

    // 00C5BB66. The collision pass, on the scene at world+444h.
    host.run_collision_pass_00c57070();

    // 00C5BB6B..00C5BBF1, the "CreateGroups" scope around 00C4B610.
    const DynProfilerScopeSlot& create_groups = kScopes[1];
    host.push_profiler_scope(create_groups);
    host.create_contact_groups_00c4b610();
    host.pop_profiler_scope(create_groups);

    const std::int32_t mode_value = host.solver_mode();
    if (dyn_solver_mode_is_known(mode_value)) {
        const DynSolverTaskKind kind = (mode_value == 0) ? DynSolverTaskKind::kLcpSolverTask
                                                         : DynSolverTaskKind::kLcpSolver2Task;

        // 00C5BC06 / 00C5C02A: with no groups there is nothing to solve, and the
        // contact report below still runs.
        const std::int32_t group_count = host.contact_group_count();
        if (group_count != 0) {
            const DynProfilerScopeSlot& solve = kScopes[2];
            host.push_profiler_scope(solve);
            const std::int32_t task_count =
                dyn_solver_task_count(group_count, host.solver_task_capacity(kind));
            for (std::int32_t i = 0; i < task_count; ++i) {
                host.set_solver_task_range(kind, i,
                                           dyn_solver_task_range(group_count, task_count, i, dt));
            }
            host.dispatch_solver_tasks_00c33140(kind, task_count);
            host.pop_profiler_scope(solve);
            result.solve_ran = true;
            result.solver_task_count = task_count;
        }

        // 00C5C124 / 00C5C431. A world with no listener skips the whole record build,
        // including the allocation.
        if (host.has_contact_listener()) {
            // 00C5C149..00C5C179: manifoldCount * 4 records of 58h bytes, allocated
            // once and freed at 00C5C449. Four is the assumed cap on points per
            // manifold; nothing in this routine enforces it, so a manifold reporting
            // more would overrun the native buffer.
            const std::size_t wanted = static_cast<std::size_t>(host.scene_manifold_count()) *
                                       kDynContactReportRecordsPerManifold;
            const std::size_t limit = (wanted < record_capacity) ? wanted : record_capacity;

            std::int32_t written = 0;
            const DynHandle sentinel = host.manifold_list_sentinel();
            for (DynHandle manifold = host.manifold_list_head(); manifold != sentinel;
                 manifold = host.manifold_next(manifold)) {
                const std::int32_t points = host.manifold_point_count(manifold);
                if (points <= 0) {
                    continue;
                }
                const DynBody& body_a = host.manifold_body_a(manifold);
                const DynBody& body_b = host.manifold_body_b(manifold);
                for (std::int32_t p = 0; p < points; ++p) {
                    if (records == nullptr || static_cast<std::size_t>(written) >= limit) {
                        break;
                    }
                    dyn_build_contact_report_record(host.manifold_point(manifold, p), body_a,
                                                    body_b, records[written]);
                    written += 1;
                }
            }

            // 00C5C431..00C5C43A. The count is the running index across every manifold,
            // not a per-manifold count, so the listener gets one flat array.
            host.report_contacts_world_24h(records, written);
            result.contacts_reported = true;
            result.contact_record_count = written;
        }
    }

    // 00C5C456 onward, the "UpdatePosition" scope. Reached whatever the solver mode
    // was: 00C5BBF8 and 00C5C000 both jump straight here, and so does the fall-through
    // after the contact report is freed at 00C5C449.
    const DynProfilerScopeSlot& update_position = kScopes[5];
    host.push_profiler_scope(update_position);
    host.integrate_positions_00c5b1b0(dt);
    host.pop_profiler_scope(update_position);

    // 00C5C4C9..00C5C53E, the "SleepGroups" scope. It is the last thing the substep
    // does, so a group that sleeps here is excluded from the NEXT substep's integration
    // and not from this one's.
    const DynProfilerScopeSlot& sleep_groups = kScopes[6];
    host.push_profiler_scope(sleep_groups);
    host.sleep_contact_groups_00c4b550();
    host.pop_profiler_scope(sleep_groups);

    return result;
}

}  // namespace bsp
