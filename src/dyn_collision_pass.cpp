#include "bsp/dyn_collision_pass.hpp"

#include <cmath>

// Reconstruction of the Dyn scene's collision pass, 00C57070.
// docs/DYN_COLLISION_PASS.md carries the addresses, the original ABI and the
// uncertainty. Semantic C++ for MSVC Win32, not a drop-in binary replacement.

namespace bsp {
namespace {

constexpr float kNegativeZero = -0.0f;

inline float abs_float(float v) noexcept { return std::fabs(v); }  // BSP_Math_AbsFloat

// p' = px*row0 + py*row1 + pz*row2 + position, the same 3x4 the solver uses.
void transform_point(const DynSolverBodyInput& body, const float local[3],
                     float world[3]) noexcept {
    world[0] = local[1] * body.row1[0] + local[0] * body.row0[0] +
               local[2] * body.row2[0] + body.position[0];
    world[1] = body.row2[1] * local[2] + body.row0[1] * local[0] +
               body.row1[1] * local[1] + body.position[1];
    world[2] = local[2] * body.row2[2] + body.row0[2] * local[0] +
               body.row1[2] * local[1] + body.position[2];
}

float distance_squared(const float a[3], const float b[3]) noexcept {
    // 00C3F66B..00C3F6A?: z first, then x, then y, the order 00C3F650 accumulates in.
    const float dz = a[2] - b[2];
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    return dz * dz + dx * dx + dy * dy;
}

}  // namespace

DynWorldBounds dyn_body_world_bounds_00c5715c(const DynSolverBodyInput& body,
                                              const float local_min[3],
                                              const float local_max[3]) noexcept {
    // 00C5715C..00C5719?: centre and half extent, both scaled by the 0.5 at 00D7A280.
    const float centre[3] = {(local_min[0] + local_max[0]) * kDynAabbHalf,
                             (local_min[1] + local_max[1]) * kDynAabbHalf,
                             (local_min[2] + local_max[2]) * kDynAabbHalf};
    const float half[3] = {(local_max[0] - local_min[0]) * kDynAabbHalf,
                           (local_max[1] - local_min[1]) * kDynAabbHalf,
                           kDynAabbHalf * (local_max[2] - local_min[2])};

    float world_centre[3];
    transform_point(body, centre, world_centre);

    // 00C571??..00C573??: the same matrix with every element replaced by its magnitude,
    // which is the conservative rotation of a box. Nine BSP_Math_AbsFloat calls.
    const float world_half[3] = {
        half[0] * abs_float(body.row0[0]) + half[1] * abs_float(body.row1[0]) +
            half[2] * abs_float(body.row2[0]),
        abs_float(body.row2[1]) * half[2] + abs_float(body.row0[1]) * half[0] +
            abs_float(body.row1[1]) * half[1],
        half[0] * abs_float(body.row0[2]) + abs_float(body.row1[2]) * half[1] +
            abs_float(body.row2[2]) * half[2]};

    DynWorldBounds bounds;
    // 00C57404..00C5745E, min first then max.
    bounds.min[0] = world_centre[0] - world_half[0];
    bounds.min[1] = world_centre[1] - world_half[1];
    bounds.min[2] = world_centre[2] - world_half[2];
    bounds.max[0] = world_half[0] + world_centre[0];
    bounds.max[1] = world_half[1] + world_centre[1];
    bounds.max[2] = world_half[2] + world_centre[2];
    return bounds;
}

bool dyn_pair_survives_sleep_filter(std::uint32_t flags_a,
                                    std::uint32_t flags_b) noexcept {
    return (flags_a & flags_b & kDynBodyFlagNoIntegrate) == 0u;
}

bool dyn_shapes_overlap_filter(std::uint32_t group_a, std::uint32_t mask_a,
                               std::uint32_t group_b, std::uint32_t mask_b) noexcept {
    // 00C44104..00C44110. Either direction is enough, so one shape can pull another into
    // the narrow phase without the second opting in.
    return (mask_b & group_a) != 0u || (mask_a & group_b) != 0u;
}

std::int32_t dyn_narrow_phase_dispatch_index(std::int32_t type_a,
                                             std::int32_t type_b) noexcept {
    return type_a * kDynShapeTypeCount + type_b;  // 00C44121
}

bool dyn_contact_normal_is_unit(const float normal[3]) noexcept {
    // 00C3F767..00C3F790: |1 - |n|^2| must not exceed the tolerance. A candidate that
    // fails is dropped silently, points and all.
    const float length_sq = normal[2] * normal[2] + normal[0] * normal[0] +
                            normal[1] * normal[1];
    return abs_float(1.0f - length_sq) <= kDynContactNormalUnitTolerance;
}

std::int32_t dyn_match_contact_point_00c3f650(const DynSolverContactPoint* points,
                                              std::int32_t point_count,
                                              const float local_a[3],
                                              const float local_b[3]) noexcept {
    for (std::int32_t i = 0; i < point_count; ++i) {
        if (distance_squared(points[i].local_point_a, local_a) <
                kDynContactPointMatchDistanceSq ||
            distance_squared(points[i].local_point_b, local_b) <
                kDynContactPointMatchDistanceSq) {
            return i;
        }
    }
    return point_count;
}

float dyn_contact_depth_00c3f93c(const DynSolverBodyInput& body_a,
                                 const DynSolverBodyInput& body_b,
                                 const DynSolverContactPoint& point) noexcept {
    float world_a[3];
    float world_b[3];
    transform_point(body_a, point.local_point_a, world_a);
    transform_point(body_b, point.local_point_b, world_b);
    // 00C3F8B0..00C3F93C, z term first, then x, then y.
    return point.normal[2] * (world_a[2] - world_b[2]) +
           point.normal[0] * (world_a[0] - world_b[0]) +
           point.normal[1] * (world_a[1] - world_b[1]);
}

DynContactInsertResult dyn_insert_contact_point_00c3f760(
    DynSolverContactPoint* points, std::int32_t& point_count,
    const DynSolverBodyInput& body_a, const DynSolverBodyInput& body_b,
    const float local_a[3], const float local_b[3], const float normal[3]) noexcept {
    DynContactInsertResult result;
    if (!dyn_contact_normal_is_unit(normal)) {
        result.rejected_normal = true;
        return result;
    }

    const std::int32_t match =
        dyn_match_contact_point_00c3f650(points, point_count, local_a, local_b);
    if (match < point_count) {
        // 00C3F7C?..00C3F93C: the geometry is replaced and the two accumulated impulses
        // at +24h and +28h are left alone. That is the whole of the warm start's
        // frame-to-frame persistence.
        DynSolverContactPoint& point = points[match];
        for (int i = 0; i < 3; ++i) {
            point.local_point_a[i] = local_a[i];
            point.local_point_b[i] = local_b[i];
            point.normal[i] = normal[i];
        }
        point.depth = dyn_contact_depth_00c3f93c(body_a, body_b, point);
        result.matched_existing = true;
        result.index = match;
        return result;
    }

    if (point_count < kDynManifoldMaxPoints) {
        DynSolverContactPoint& point = points[point_count];
        for (int i = 0; i < 3; ++i) {
            point.local_point_a[i] = local_a[i];
            point.local_point_b[i] = local_b[i];
            point.normal[i] = normal[i];
        }
        // 00C3F9BD and 00C3F9D2: a brand new point starts cold.
        point.normal_impulse = 0.0f;
        point.bias_impulse = 0.0f;
        point.depth = dyn_contact_depth_00c3f93c(body_a, body_b, point);
        result.appended = true;
        result.index = point_count;
        ++point_count;  // 00C3FA4?
        return result;
    }

    // 00C3FA46..00C3FFD5, the four-point reduction this packet did not read.
    result.hit_full_manifold = true;
    return result;
}

std::int32_t dyn_run_collision_pass_00c57070(DynCollisionPassHost& host) {
    // Step 1, the "BroadPhase" scope, 00C5712A..00C5747A.
    const std::int32_t bodies = host.body_count();
    for (std::int32_t body = 0; body < bodies; ++body) {
        const std::uint32_t flags = host.body_flags(body);
        if ((flags & kDynBodyFlagBroadPhaseActive) == 0u) continue;
        if ((flags & kDynBodyFlagNoIntegrate) != 0u) continue;
        float local_min[3];
        float local_max[3];
        host.body_local_bounds(body, local_min, local_max);
        host.set_body_world_bounds(
            body, dyn_body_world_bounds_00c5715c(host.body_transform(body), local_min,
                                                 local_max));
    }

    // Step 2, the "BroadPhaseUpdate" scope.
    host.broad_phase_update_vslot3();

    // Step 3, the "ManifoldUpdate" scope. It runs before the pair set is read, so a
    // manifold whose points all separated this frame is already off the list by the
    // time the narrow phase asks for one.
    host.manifold_update_00c549d0();

    // Step 4. 00C575AF clears the contact-event count whatever the pair count is, so a
    // frame with no pairs still runs step 6 against an empty event array.
    const std::uint32_t pair_count = host.broad_phase_pair_count_vslot4();
    host.clear_contact_events();
    std::int32_t kept = 0;
    if (pair_count == 0u) {
        host.dispatch_contact_events_00c35480();
        return kept;
    }

    // Step 5, the "IntersectLoop" scope with "GetManifold" nested inside it.
    std::int32_t pair = host.first_pair_vslot5();
    for (std::uint32_t i = 0; i < pair_count; ++i) {
        if (dyn_pair_survives_sleep_filter(host.pair_body_flags(pair, 0),
                                           host.pair_body_flags(pair, 1))) {
            host.keep_pair(kept, pair);
            ++kept;
        }
        pair = host.next_pair_vslot6(pair);
    }

    if (kept != 0) {
        // 00C57777..00C577EE. The same partition rule the solver split uses
        // (bsp/dyn_contact_solver.hpp, dyn_solver_task_count and dyn_solver_task_range):
        // integer division, and the last task absorbs the remainder.
        std::int32_t task_count = kept;
        const std::int32_t capacity = host.narrow_task_capacity();
        if (capacity < task_count) task_count = capacity;
        const std::int32_t per_task = kept / task_count;
        std::int32_t start = 0;
        std::int32_t task = 0;
        for (; task < task_count - 1; ++task) {
            host.set_narrow_task_range(task, start, per_task - 1 + start);
            start += per_task;
        }
        host.set_narrow_task_range(task, start, kept - 1);
        host.run_narrow_phase_batch_00c33140(task_count);
    }

    // Step 6, outside the IntersectLoop scope and outside the pair-count guard.
    host.dispatch_contact_events_00c35480();
    return kept;
}

}  // namespace bsp
