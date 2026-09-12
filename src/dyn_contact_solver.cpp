#include "bsp/dyn_contact_solver.hpp"

#include <vector>

// The collision, group and LCP phases of one Dyn substep. docs/DYN_CONTACT_SOLVER.md
// carries the addresses, the original ABI and the uncertainty. The float expressions
// follow the x87 listing operation for operation at the game's 24-bit precision
// (docs/X87_CONTROL_WORD.md), so every intermediate below is a float.

namespace bsp {
namespace {

// p' = px*row0 + py*row1 + pz*row2 + position, in the order 00C5C1D6..00C5C24D forms
// it: the row-1 term first, then row 0, then row 2, then the translation. Float
// addition is commutative, so the reordering the decompiler shows is not a divergence;
// the grouping is what matters and it is (a + b) + c + d in both readings.
void transform_point(const DynBody& body, const float p[3], float out[3]) noexcept {
    const float px = p[0];
    const float py = p[1];
    const float pz = p[2];
    for (int i = 0; i < 3; ++i) {
        out[i] = ((py * body.row1[i] + px * body.row0[i]) + pz * body.row2[i]) +
                 body.position[i];
    }
}

}  // namespace

void dyn_build_contact_report_record(const DynContactPoint& point, const DynBody& body_a,
                                     const DynBody& body_b,
                                     DynContactReportRecord& out) noexcept {
    // record+00h and +0Ch: the two local points in world space, through
    // [manifold+0CCh] and [manifold+0D0h].
    transform_point(body_a, point.local_point_a, out.world_point_a);
    transform_point(body_b, point.local_point_b, out.world_point_b);

    // record+18h: the normal copied unchanged, three dword moves with no arithmetic.
    for (int i = 0; i < 3; ++i) {
        out.normal[i] = point.normal[i];
    }

    // record+24h and +30h: the normal scaled by +s and by -s. The listing forms the
    // negation of point+24h once and multiplies again, so the second triple is (-s)*n
    // and not -(s*n); at 24-bit precision those agree bit for bit.
    const float s = point.normal_scale;
    const float negated = -point.normal_scale;
    for (int i = 0; i < 3; ++i) {
        out.impulse_on_a[i] = s * point.normal[i];
        out.impulse_on_b[i] = negated * point.normal[i];
    }

    // record+3Ch and +48h: six zeroed dwords. record+54h is never written.
    for (int i = 0; i < 3; ++i) {
        out.zeroed_3c[i] = 0.0f;
        out.zeroed_48[i] = 0.0f;
    }
}

bool dyn_solver_mode_is_known(std::int32_t world_solver_mode) noexcept {
    // 00C5BBF5 SUB ECX,0 / JE (mode 0), 00C5BBFE SUB ECX,EBX / JNE 0x00C5C456 (mode 1
    // falls through, anything else skips the whole block).
    return world_solver_mode == 0 || world_solver_mode == 1;
}

std::int32_t dyn_solver_task_count(std::int32_t group_count, std::int32_t capacity) noexcept {
    // 00C5BC37 CMP ESI,EAX / JGE, 00C5C05A the same: the smaller of the two.
    return (capacity < group_count) ? capacity : group_count;
}

DynSolverTaskRange dyn_solver_task_range(std::int32_t group_count, std::int32_t task_count,
                                         std::int32_t task_index, float dt) noexcept {
    DynSolverTaskRange range{};
    range.dt = dt;
    if (task_count <= 0) {
        return range;
    }
    // 00C5BC4D CDQ / 00C5BC53 IDIV ESI: a signed integer division done once, before the
    // loop, and reused for every task.
    const std::int32_t per_task = group_count / task_count;
    range.first_group = per_task * task_index;
    if (task_index < task_count - 1) {
        // 00C5BC76 LEA EBX,[EAX+ECX-1]: start + per_task - 1.
        range.last_group = range.first_group + per_task - 1;
    } else {
        // 00C5BCAA ADD EDX,-1 on the group count: the last task absorbs the remainder.
        range.last_group = group_count - 1;
    }
    return range;
}

// ---------------------------------------------------------------------------
// 00C4B610, CreateGroups
// ---------------------------------------------------------------------------
std::int32_t dyn_create_contact_groups_00c4b610(DynGroupFormationHost& host) {
    host.clear_groups_00c3f410();
    host.reset_marks_00c36ac0();

    // 00C4B668..00C4B674: the stack is sized to the scene's manifold count and zero-filled. It is
    // the flood fill's work list; the fill can never hold more entries than there are
    // manifolds because every push marks its manifold assigned first.
    const std::int32_t manifold_count = host.scene_manifold_count();
    std::vector<DynHandle> stack;
    if (manifold_count != 0) {
        stack.assign(static_cast<std::size_t>(manifold_count), DynHandle{0});
    }

    const DynHandle sentinel = host.manifold_list_sentinel();
    std::int32_t groups = 0;
    std::int32_t top = -1;

    // The native writes straight into the buffer. The bound below can only be reached
    // if the scene's count at +1D0h disagrees with its list, which the native would
    // answer with a heap overrun; here it drops the push instead.
    const auto push = [&stack, &top](DynHandle handle) noexcept -> bool {
        const std::size_t slot = static_cast<std::size_t>(top + 1);
        if (slot >= stack.size()) {
            return false;
        }
        stack[slot] = handle;
        top = static_cast<std::int32_t>(slot);
        return true;
    };

    for (DynHandle node = host.manifold_list_head(); node != sentinel;
         node = host.manifold_next(node)) {
        // 00C4B6C9..00C4B6FA: three tests in this order. A manifold with no contact points, or
        // one already assigned, is skipped; and at least one of the two bodies must be
        // neither static nor asleep, so an island of settled bodies forms no group.
        if (host.manifold_point_count(node) == 0) {
            continue;
        }
        if (host.manifold_group_mark(node) != kDynGroupMarkUnassigned) {
            continue;
        }
        const std::uint32_t flags_a = host.body_flags(host.manifold_body_a(node));
        const std::uint32_t flags_b = host.body_flags(host.manifold_body_b(node));
        if ((flags_a & kDynBodyStaticOrAsleepMask) != 0 &&
            (flags_b & kDynBodyStaticOrAsleepMask) != 0) {
            continue;
        }

        // The group is opened before the fill runs: the native increments world+458h at
        // 00C4B700, grows the vector of 0Ch-byte group entries to that size and stores the
        // same number into world+450h.
        const std::int32_t group_index = groups;
        groups += 1;
        host.set_group_count(groups);

        if (!push(node)) {
            continue;
        }
        host.set_manifold_group_mark(node, kDynGroupMarkAssigned);

        while (top >= 0) {
            const DynHandle manifold = stack[static_cast<std::size_t>(top)];
            top -= 1;
            host.append_to_group_00c36b60(group_index, manifold);

            const DynHandle bodies[2] = {host.manifold_body_a(manifold),
                                         host.manifold_body_b(manifold)};
            for (const DynHandle body : bodies) {
                // A static body is not expanded through, so an island stops
                // at the world geometry instead of merging every body touching it.
                if ((host.body_flags(body) & kDynBodyFlagStatic) != 0) {
                    continue;
                }
                if (host.body_group_mark(body) != kDynGroupMarkUnassigned) {
                    continue;
                }
                host.set_body_group_mark(body, kDynGroupMarkAssigned);
                const std::int32_t contacts = host.body_contact_count(body);
                for (std::int32_t i = 0; i < contacts; ++i) {
                    const DynHandle other = host.body_contact(body, i);
                    if (other == 0) {
                        continue;
                    }
                    if (host.manifold_point_count(other) == 0) {
                        continue;
                    }
                    if (host.manifold_group_mark(other) != kDynGroupMarkUnassigned) {
                        continue;
                    }
                    if (!push(other)) {
                        continue;
                    }
                    host.set_manifold_group_mark(other, kDynGroupMarkAssigned);
                    // The wake is applied to the manifold the fill reaches, not to the
                    // one it came from, and it is the same AND 0FFFFFFEDh every body
                    // mutator uses: a body dragged into a live island stops being
                    // asleep and stops being excluded from integration.
                    host.wake_body(host.manifold_body_a(other));
                    host.wake_body(host.manifold_body_b(other));
                }
            }
        }
    }

    return groups;
}

// ---------------------------------------------------------------------------
// 00C4B550, SleepGroups
// ---------------------------------------------------------------------------
std::int32_t dyn_sleep_contact_groups_00c4b550(DynContactGroupHost& host) {
    std::int32_t slept = 0;
    const std::int32_t groups = host.group_count();
    for (std::int32_t g = 0; g < groups; ++g) {
        const std::int32_t contacts = host.group_contact_count(g);

        // 00C4B583..00C4B5A5: scan until a contact is found with either body neither
        // static nor asleep. The loop counter is what the test below reads, so an empty
        // group leaves it at zero and fails the `contacts > 0` half.
        std::int32_t scanned = 0;
        while (scanned < contacts) {
            const DynHandle manifold = host.group_contact(g, scanned);
            const std::uint32_t flags_a = host.body_flags(host.manifold_body_a(manifold));
            const std::uint32_t flags_b = host.body_flags(host.manifold_body_b(manifold));
            if ((flags_a & kDynBodyStaticOrAsleepMask) == 0 ||
                (flags_b & kDynBodyStaticOrAsleepMask) == 0) {
                break;
            }
            scanned += 1;
        }

        if (scanned < contacts || contacts <= 0) {
            continue;
        }

        // 00C4B5C0..00C4B5E4 (EBX = 10h from 00C4B56D): bit 4 on both bodies of every contact. That is the bit both
        // integration phases test first, so the whole island stops being integrated
        // until something clears it -- which the group fill above does, through
        // wake_body, the moment a live manifold reaches one of these bodies again.
        for (std::int32_t i = 0; i < contacts; ++i) {
            const DynHandle manifold = host.group_contact(g, i);
            const DynHandle body_a = host.manifold_body_a(manifold);
            const DynHandle body_b = host.manifold_body_b(manifold);
            host.set_body_flags(body_a, host.body_flags(body_a) | kDynBodyFlagNoIntegrate);
            host.set_body_flags(body_b, host.body_flags(body_b) | kDynBodyFlagNoIntegrate);
        }
        slept += 1;
    }

    // 00C4B5FF: the same outlined reset CreateGroups opens with. The group vector does
    // not survive the substep; every substep rebuilds it from the manifold list.
    host.clear_groups_00c3f410();
    return slept;
}

}  // namespace bsp
