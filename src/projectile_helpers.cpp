// Reconstruction of the projectile flight-and-impact leaves.
// Evidence, ABI and coverage: docs/PROJECTILE_HELPERS.md.
#include "bsp/projectile_helpers.hpp"

#include <array>

#include "bsp/gamepad_force_events.hpp"

namespace bsp {
namespace {

TickPoint3 midpoint_of(const TickPoint3& a, const TickPoint3& b) noexcept {
    // 0078D1CA / 0078D1DC / 0078D1EE and 0070C4B9 / 0070C4C6 / 0070C4D3 all
    // FSTP the componentwise sum to a float before reloading it and scaling by
    // the double 0.5 at 00D7A280, so the sum rounds once and the halving is
    // exact. A double-precision sum here would differ in the last bit.
    TickPoint3 out;
    out.x = (a.x + b.x) * 0.5f;
    out.y = (a.y + b.y) * 0.5f;
    out.z = (a.z + b.z) * 0.5f;
    return out;
}

float dot_of(const TickPoint3& a, const TickPoint3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float segment_length_0042b2f0(const TickPoint3& from, const TickPoint3& to) noexcept {
    const std::array<float, 3> delta{to.x - from.x, to.y - from.y, to.z - from.z};
    return force_event_vector_length_0042b2f0(delta);
}

}  // namespace

// ---------------------------------------------------------------------------
// The static traces
// ---------------------------------------------------------------------------

StaticTraceVariant static_trace_variant_0084c11c(int shot_medium,
                                                 bool class_prefers_entry) noexcept {
    // 0084C12F TEST/JZ then 0084C133 CMP byte [classDesc+74h]/JNZ: the exit
    // trace runs only when the shot is already in a medium and the class does
    // not override it.
    if (shot_medium == 0 || class_prefers_entry) {
        return StaticTraceVariant::kSurfaceEntry;
    }
    return StaticTraceVariant::kSurfaceExit;
}

TickPoint3 water_crossing_point_0078d1b0(WaterSurfaceSampler& water,
                                         const TickPoint3& from,
                                         const TickPoint3& to) noexcept {
    TickPoint3 near_point = from;
    TickPoint3 far_point = to;
    TickPoint3 mid = midpoint_of(near_point, far_point);

    // 0078D21C..0078D257: |to.y - from.y| through the -0.0f fold, compared as a
    // double against 00CE6638. A segment that is level in y is never bisected.
    float height_delta = far_point.y - near_point.y;
    if (!(height_delta > 0.0f)) {
        height_delta = kWaterCrossingNegativeZero - height_delta;
    }
    if (!(static_cast<double>(height_delta) > kWaterCrossingMinHeightDelta)) {
        return mid;
    }

    float working_length = segment_length_0042b2f0(near_point, far_point);
    if (!(working_length > kWaterCrossingTargetLength)) {
        return mid;
    }

    bool midpoint_submerged = false;
    do {
        // 0078D2A8 and 0078D2C2, inlined from 0078CF20: the wave height times
        // the coverage mask. The arguments are the midpoint's x and *y*; see
        // the axis defect in docs/PROJECTILE_HELPERS.md.
        midpoint_submerged = mid.y <= water.water_height_0078cf20(mid.x, mid.y);
        if (midpoint_submerged) {
            far_point = mid;
        } else {
            near_point = mid;
        }
        mid = midpoint_of(near_point, far_point);
        working_length = static_cast<float>(working_length * kProjectileHalf);
    } while (working_length > kWaterCrossingTargetLength);

    // 0078D3A3..0078D3AF: the fresh midpoint when the last probe was under the
    // surface, otherwise the far endpoint, which is the deepest known point.
    return midpoint_submerged ? mid : far_point;
}

namespace {

StaticTraceResult walk_segments(WaterSurfaceSampler& water, const TraceSegmentList& list,
                                bool want_submerged_end) noexcept {
    StaticTraceResult result;
    const int count = list.count;
    for (int i = 0; i < count && i < static_cast<int>(kTraceSegmentCapacity); ++i) {
        const TraceSegment& segment = list.segments[i];
        // 0084B3F8 / 0084B528: the sampler takes the end point's first two
        // components and the comparison uses the second one.
        const float surface = water.water_height_0078cf20(segment.to.x, segment.to.y);
        const bool end_is_submerged = segment.to.y <= surface;
        if (end_is_submerged != want_submerged_end) {
            continue;
        }
        // 0084B485 passes (from, to); 0084B5B5 passes (to, from).
        result.point = want_submerged_end
                           ? water_crossing_point_0078d1b0(water, segment.from, segment.to)
                           : water_crossing_point_0078d1b0(water, segment.to, segment.from);
        result.hit = true;
        return result;
    }
    // 0084B41D / 0084B54D: the flag byte at +0Ch is cleared and the three
    // floats are left untouched.
    result.hit = false;
    return result;
}

}  // namespace

StaticTraceResult static_trace_0084b380(WaterSurfaceSampler& water,
                                        const TraceSegmentList& list) noexcept {
    return walk_segments(water, list, true);
}

StaticTraceResult static_trace_0084b4b0(WaterSurfaceSampler& water,
                                        const TraceSegmentList& list) noexcept {
    return walk_segments(water, list, false);
}

// ---------------------------------------------------------------------------
// 0084AFD0
// ---------------------------------------------------------------------------

void* notify_entity_hit_chain_0084afd0(EntityHitChainHost& host, void* entity,
                                       const EntityHitNotice& notice) noexcept {
    // 0084AFD3: a null receiver returns immediately, before the first call.
    while (entity != nullptr) {
        if (host.offer_hit_notice(entity, notice)) {
            return entity;
        }
        entity = host.parent_of(entity);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 007BC4E0
// ---------------------------------------------------------------------------

void bind_plane_crash_effect_007bc4e0(PlaneCrashEffectHost& host, void* plane,
                                      const TickPoint3& world_position) noexcept {
    // 007BC4E6 and 007BC4EF: both the handle and the flag must be clear.
    if (host.crash_effect_already_bound(plane)) {
        return;
    }
    host.refresh_world_pose(plane);
    const void* direction = host.plane_effect_direction(plane);
    void* effect = host.spawn_point_effect_0084b6f0(plane, world_position, direction);
    // 007BC52B stores the handle whether or not it is null; only a non-null
    // one takes the reference and raises the flag.
    host.store_crash_effect(plane, effect);
    if (effect != nullptr) {
        host.add_effect_reference(effect);
    }
}

// ---------------------------------------------------------------------------
// 0084B000 and 0084B650
// ---------------------------------------------------------------------------

ProjectileHitMessage build_hit_message_0084b000(ProjectileImpactMode mode, void* source,
                                                const TickPoint3& hit_position,
                                                const TickPoint3& direction) noexcept {
    ProjectileHitMessage message;
    message.routing_field = 1;
    message.pad_18 = 0;
    message.pad_1a = 0;
    message.mode = static_cast<int>(mode);
    message.source = source;
    message.hit_position = hit_position;
    message.direction = direction;
    return message;
}

void* checked_cast_to_shot_owner_0084b650(void* object, bool passes_kind_2a) noexcept {
    return (object != nullptr && passes_kind_2a) ? object : nullptr;
}

// ---------------------------------------------------------------------------
// 0084B8C0
// ---------------------------------------------------------------------------

ImpactEffectSlot impact_effect_slot_0084b8c0(ProjectileImpactMode mode, int medium) noexcept {
    ImpactEffectSlot slot;
    switch (mode) {
        case ProjectileImpactMode::kStatic:
            // 0084B918..0084B92E, no medium test at all.
            slot.valid = true;
            slot.class_desc_offset = kWeaponClassOffEffectStatic;
            return slot;
        case ProjectileImpactMode::kUnit:
            // 0084B953..0084B964, likewise unconditional.
            slot.valid = true;
            slot.class_desc_offset = kWeaponClassOffEffectPlane;
            return slot;
        case ProjectileImpactMode::kEntity:
            if (medium == 0) {
                slot.valid = true;
                slot.class_desc_offset = kWeaponClassOffEffectEntityAir;  // 0084B908
                return slot;
            }
            break;
        case ProjectileImpactMode::kScoringTarget:
            if (medium == 0) {
                slot.valid = true;
                slot.class_desc_offset = kWeaponClassOffEffectLandscape;  // 0084B943
                return slot;
            }
            break;
        default:
            // 0084B969: anything outside 1..4 returns without an effect.
            return slot;
    }
    // 0084B93D jumps into the mode-1 chain, so modes 1 and 3 share these two.
    if (medium == 1) {
        slot.valid = true;
        slot.class_desc_offset = kWeaponClassOffEffectMedium1;  // 0084B8F5
    } else if (medium == 2) {
        slot.valid = true;
        slot.class_desc_offset = kWeaponClassOffEffectMedium2;  // 0084B8E0
    }
    return slot;
}

// ---------------------------------------------------------------------------
// The flak proximity search
// ---------------------------------------------------------------------------

float flak_search_radius_squared_0070c50f(float step_length, float blast_radius) noexcept {
    // 0070C518 FADD ST0,ST0 with the result stored to a float at 0070C51A,
    // then 0070C51E FMULP by the 0.5 still on the x87 stack, 0070C520 FADD and
    // 0070C524 FMUL ST0,ST0: only the square is rounded back to a float.
    const float doubled_radius = blast_radius + blast_radius;
    const double reach = static_cast<double>(step_length) * kProjectileHalf + doubled_radius;
    return static_cast<float>(reach * reach);
}

bool flak_candidate_kind_passes(bool is_kind_5, bool is_kind_f, bool is_kind_e,
                                bool is_kind_c) noexcept {
    // 0070C553 gates everything; then 0070C566 / 0070C575 / 0070C586 are an
    // early-out chain, so any one of the three is enough.
    return is_kind_5 && (is_kind_f || is_kind_e || is_kind_c);
}

void flak_proximity_search_0070c4a4(FlakProximityHost& host, FlakProximityState& state,
                                    const TickPoint3& midpoint,
                                    const TickPoint3& previous_position,
                                    const TickPoint3& step_direction,
                                    float search_radius_squared,
                                    float muzzle_speed) noexcept {
    // 0070C4B1 and 0070C4F7 seed the tracker with FLT_MAX every tick.
    state.nearest_squared = kFlakNearestSeed;

    // 0070C5A8..0070C61F: the radius is clamped per candidate, but the clamp
    // has no per-candidate input, so it is hoisted here.
    const float capped = (kFlakSearchRadiusSquaredCap <= search_radius_squared)
                             ? kFlakSearchRadiusSquaredCap
                             : search_radius_squared;

    for (void* cursor = host.first_candidate(); cursor != nullptr;
         cursor = host.next_candidate(cursor)) {
        void* entity = host.entity_of(cursor);
        if (entity == nullptr) {
            continue;  // 0070C546
        }
        if (!flak_candidate_kind_passes(host.candidate_is_kind(entity, kFlakCandidateRequiredKind),
                                        host.candidate_is_kind(entity, kFlakCandidatePlaneKind),
                                        host.candidate_is_kind(entity, kFlakCandidateAltKindE),
                                        host.candidate_is_kind(entity, kFlakCandidateAltKindC))) {
            continue;
        }

        const TickPoint3 position = host.candidate_world_position(entity);
        const TickPoint3 offset{position.x - midpoint.x, position.y - midpoint.y,
                                position.z - midpoint.z};
        const float distance_squared = dot_of(offset, offset);

        // 0070C62D then 0070C639, both strict and both skipping on JBE.
        if (!(capped > distance_squared)) {
            continue;
        }
        if (!(state.nearest_squared > distance_squared)) {
            continue;
        }

        // 0070C661 and 0070C665, before the solver runs.
        state.target_latched = true;
        state.target = entity;
        state.nearest_squared = distance_squared;

        const TickPoint3 lead = host.lead_solution_00901c20(entity, muzzle_speed);
        const TickPoint3 travel{lead.x - previous_position.x, lead.y - previous_position.y,
                                lead.z - previous_position.z};
        // 0070C6A6..0070C6C2: the dot with the normalised step direction, then
        // the running bias at projectile+290h, then a clamp at zero.
        const float along = dot_of(step_direction, travel);
        const float remaining = state.burst_distance_bias + along;
        state.burst_distance = (remaining < 0.0f) ? 0.0f : remaining;
    }
}

bool flak_should_detonate_0070c716(const FlakProximityState& state,
                                   float step_length) noexcept {
    // 0070C709: nothing latched, nothing to burst against.
    if (!state.target_latched) {
        return false;
    }
    // 0070C71C FCOMI then JB: a step shorter than the remaining distance only
    // shortens it (0070C7AD), it does not fire.
    return !(step_length < state.burst_distance);
}

bool flak_airburst_conditions_0070c7b6(FlakProximityState& state,
                                       float nearest_squared) noexcept {
    // 0070C7BE: nothing was ever close enough this tick.
    if (!(kFlakSearchRadiusSquaredCap > nearest_squared)) {
        return false;
    }
    // 0070C7CA: while the miss distance is still shrinking, record it and wait.
    if (!(state.previous_nearest_squared < nearest_squared)) {
        state.previous_nearest_squared = nearest_squared;
        return false;
    }
    // 0070C7D7: a near miss is left to the impact path, not aired off.
    return nearest_squared > kFlakAirburstMinMissSquared;
}

}  // namespace bsp
