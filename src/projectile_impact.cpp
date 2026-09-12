// Reconstruction of the projectile's spawn, flight and impact. Evidence per
// claim is in docs/PROJECTILE_IMPACT.md; the call sites the host methods stand
// for are listed in reports/projectile_impact.json.
//
// Coverage is not uniform. The four integration bodies (006E65C0, 006E65F0,
// 006E7670, 006E7760), 0072BF10, 0084BC60 and 009239A0 are complete. 006E8430
// is modelled only for its velocity and record initialisation; its matrix,
// node and effect work stays with the host. 0084BF00 is modelled for the
// projectile caller only. The flak variant 0070C370 and the narrowphase that
// fills the rest of the hit record are not modelled at all.
#include "bsp/projectile_impact.hpp"

namespace bsp {

namespace {

TickPoint3 scaled(const TickPoint3& v, float s) {
    TickPoint3 out;
    out.x = v.x * s;
    out.y = v.y * s;
    out.z = v.z * s;
    return out;
}

TickPoint3 added(const TickPoint3& a, const TickPoint3& b) {
    TickPoint3 out;
    out.x = a.x + b.x;
    out.y = a.y + b.y;
    out.z = a.z + b.z;
    return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

bool projectile_sub_type_takes_launch_bias(int sub_type) {
    // 006E856x: the four compares that guard the vertical bias.
    return sub_type == 4 || sub_type == 5 || sub_type == 6 || sub_type == 7;
}

TickPoint3 projectile_launch_velocity_006e8430(float speed, const TickPoint3& direction,
                                               int sub_type) {
    TickPoint3 v = scaled(direction, speed);
    if (projectile_sub_type_takes_launch_bias(sub_type)) {
        // FSUB of 00D0DE84 * 00CF9058, computed in single precision after the
        // product, matching the FMUL / FSUBP pair in the listing.
        v.y -= kProjectileLaunchBiasStep * static_cast<float>(kProjectileGravity);
    }
    return v;
}

TickPoint3 projectile_integrate_position_006e7670(const ProjectileFlightState& state,
                                                  float scaled_step) {
    // Both slots start from the snapshot the previous commit wrote, not from
    // the position they last produced.
    TickPoint3 position = added(state.snapshot_current, scaled(state.velocity, scaled_step));
    const bool gravity = state.mode == ProjectileMotionMode::kBallistic &&
                         !state.class_disables_gravity;
    if (gravity) {
        // 006E7704..006E771C evaluates (scaled * g) * scaled * 0.5 in that
        // order, so the intermediate rounding is reproduced here.
        const float step_gravity = scaled_step * static_cast<float>(kProjectileGravity);
        position.y -= step_gravity * scaled_step * static_cast<float>(kProjectileHalf);
    }
    return position;
}

TickPoint3 projectile_integrate_velocity_006e65c0(const ProjectileFlightState& state,
                                                  float scaled_step) {
    TickPoint3 v = state.velocity;
    if (state.mode == ProjectileMotionMode::kDamped) {
        // 006E65F0: each component takes (-component) * step added back, which
        // is v *= (1 - step) with no tunable coefficient.
        v.x += -v.x * scaled_step;
        v.y += -v.y * scaled_step;
        v.z += -v.z * scaled_step;
        return v;
    }
    if (!state.class_disables_gravity) {
        v.y -= scaled_step * static_cast<float>(kProjectileGravity);
    }
    return v;
}

ProjectileFlightState projectile_flight_step(const ProjectileFlightState& state,
                                             float scaled_step) {
    ProjectileFlightState next = state;
    // 006E6750 runs the position slot first, with the velocity the previous
    // step left.
    next.local_position = projectile_integrate_position_006e7670(state, scaled_step);
    // 006E64BF: the flight time accumulates before the mode query.
    next.flight_time = state.flight_time + scaled_step;
    next.velocity = projectile_integrate_velocity_006e65c0(state, scaled_step);
    return next;
}

bool projectile_has_expired(float flight_time, float class_max_life) {
    // 006E6592 FCOMIP then JBE: the expiry needs a strict greater-than.
    return flight_time > class_max_life;
}

bool projectile_fuse_is_armed(float flight_time, float class_fuse_time) {
    // 0070C3E2 FCOMPI then JBE, the same shape.
    return flight_time > class_fuse_time;
}

bool projectile_segment_is_sweepable(const TickPoint3& from, const TickPoint3& to) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const float dz = to.z - from.z;
    // 0084BF00 compares the squared length against the double at 00D0B9C0
    // without narrowing it, so the comparison is done in double here too.
    const double squared = static_cast<double>(dx * dx + dy * dy + dz * dz);
    return squared > kProjectileMinSweepLengthSquared;
}

ProjectileImpactMode projectile_refine_impact_mode(ProjectileImpactMode mode,
                                                   bool target_is_landscape_kind,
                                                   bool target_is_plane_kind) {
    // 0084BC8x: only mode 1 is refined, and the 44h test wins over the 0Fh one.
    if (mode != ProjectileImpactMode::kEntity) {
        return mode;
    }
    if (target_is_landscape_kind) {
        return ProjectileImpactMode::kLandscape;
    }
    if (target_is_plane_kind) {
        return ProjectileImpactMode::kPlane;
    }
    return mode;
}

// ---------------------------------------------------------------------------
// Spawn
// ---------------------------------------------------------------------------

void* projectile_spawn_0072bf10(const ProjectileSpawnRequest& request, int sub_type,
                                ProjectileSpawnHost& host) {
    void* shot = host.create_projectile_006e8430(request);
    if (shot == nullptr) {
        return nullptr;
    }

    // 006E8430 step 8, inside the factory: the medium is decided once, at the
    // muzzle, and it is what picks the motion mode for the whole flight.
    const float surface = host.water_height(request.position.x, request.position.z);
    host.shot_set_medium(shot, request.position.y <= surface, 0);

    host.store_ids(shot, request.gun_id, request.owner_id);
    host.gun_on_projectile_created(shot);
    host.stamp_fire_time();

    if (request.inherit_platform_velocity) {
        host.add_platform_velocity(shot, host.owner_velocity());
    }
    if (request.shot_scale != 1.0f) {
        host.shot_set_scale(shot, request.shot_scale);
    }
    host.shot_set_appearance(shot);
    host.shot_set_team(shot, request.team_id);

    // 0072C155 -> 009555A0: the owner binding and the director timestamp are
    // the last thing the spawn does.
    host.bind_owner_006e6bb0(shot);
    host.stamp_director_fire_time(sub_type);
    return shot;
}

// ---------------------------------------------------------------------------
// Sweep and impact
// ---------------------------------------------------------------------------

ProjectileImpactMode projectile_sweep_segment_0084bf00(const TickPoint3& from,
                                                       const TickPoint3& to,
                                                       ProjectileSweepStaging& staging,
                                                       bool& impacted,
                                                       ProjectileImpactHost& host) {
    impacted = false;
    if (!projectile_segment_is_sweepable(from, to)) {
        return ProjectileImpactMode::kStatic;
    }

    TickPoint3 delta;
    delta.x = to.x - from.x;
    delta.y = to.y - from.y;
    delta.z = to.z - from.z;
    // BSP_Vector3f_Length, then the reciprocal only when the length is
    // positive; a zero length leaves the direction at zero.
    const float length = host.segment_length(delta);
    const float inverse = length > 0.0f ? 1.0f / length : 0.0f;
    staging.direction = scaled(delta, inverse);

    // Step 3: the static trace runs first and its hit point is kept aside.
    // 0084C13F takes 0084B4B0 only when the shot's vtable[2Ch] is set and the
    // class does not ask for the plain trace.
    TickPoint3 static_hit;
    const bool use_alternate =
        host.shot_motion_mode_2c() && !host.class_prefers_alternate_trace();
    const bool static_hit_found = host.trace_static(from, to, use_alternate, static_hit);

    // Step 5: the entity sweep fills the hit record in place.
    bool entity_hit = host.sweep_entities_0098b370(from, to, staging);
    if (entity_hit && host.entity_hit_is_friendly_exempt(staging.hit_entity)) {
        entity_hit = false;
    }
    const bool replay_static = host.replay_forces_static_path();

    if (entity_hit && !replay_static) {
        host.on_entity_hit_0084afd0(staging.hit_entity);
        const ProjectileImpactMode mode = ProjectileImpactMode::kEntity;
        impacted = true;
        projectile_on_impact_0084bc60(mode, staging, host);
        return mode;
    }

    if (!static_hit_found) {
        return ProjectileImpactMode::kStatic;
    }

    host.on_static_hit_007bc4e0();
    if (host.shot_motion_mode_2c()) {
        // Step 9's else branch: the shot switches medium and keeps flying.
        host.shot_switch_medium(true);
        return ProjectileImpactMode::kStatic;
    }
    staging.hit_position = static_hit;
    impacted = true;
    projectile_on_impact_0084bc60(ProjectileImpactMode::kStatic, staging, host);
    return ProjectileImpactMode::kStatic;
}

void projectile_on_impact_0084bc60(ProjectileImpactMode mode,
                                   ProjectileSweepStaging& staging,
                                   ProjectileImpactHost& host) {
    // Step 2 is unconditional and comes before every effect: the projectile is
    // moved onto the impact point so that everything downstream reads it there.
    host.teleport_projectile_to_impact(staging.hit_position);

    if (host.session_should_route_hit()) {
        host.route_hit_message(mode, staging.hit_position);
    }
    if (host.shot_leaves_decal()) {
        host.place_decal_0084b8c0(staging.hit_position);
    }
    if (host.impact_kills_projectile()) {
        host.kill_projectile();
    }
    if (staging.hit_entity != nullptr) {
        // The hit is queued, not applied: 00926E80 copies the record under a
        // lock and the drain applies it later in the same fixed step.
        host.queue_hit_00926e80(staging.hit_entity, staging.direction);
    }
    if (host.class_has_explosion()) {
        const float back_off = host.explosion_back_off();
        TickPoint3 origin;
        origin.x = staging.hit_position.x - staging.direction.x * back_off;
        origin.y = staging.hit_position.y - staging.direction.y * back_off;
        origin.z = staging.hit_position.z - staging.direction.z * back_off;
        host.spawn_explosion_0084bad0(origin, false);
    }
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

void* projectile_dispatch_queued_hit_009239a0(const ProjectileQueuedHit& hit,
                                              ProjectileHitDispatchHost& host) {
    if (hit.hit_entity == nullptr || !host.entity_accepts_events(hit.hit_entity)) {
        return nullptr;
    }

    const bool is_projectile = hit.shot != nullptr &&
                               host.shot_is_kind(hit.shot, kProjectileClassId);
    const bool is_alternate = hit.shot != nullptr &&
                              host.shot_is_kind(hit.shot, kProjectileAltShotClassId);
    if (is_projectile || is_alternate) {
        void* source = host.shot_damage_source(hit.shot, !is_projectile);
        if (source != nullptr &&
            host.entity_is_kind(hit.hit_entity, kProjectileScoringTargetKind)) {
            const int scoring_id = host.source_scoring_id(source);
            if (scoring_id >= 0) {
                host.credit_hit(hit.hit_entity, scoring_id, hit.hit_position, hit.direction);
            }
        }
    }

    // The hit walks up the hierarchy until a handler accepts it. vtable[ECh] is
    // 007BBCF0 BSP_UnitInstance_OnHit; a false answer passes the record on.
    void* entity = hit.hit_entity;
    while (!host.deliver_hit_007bbcf0(entity, hit.record)) {
        entity = host.entity_parent(entity);
        if (entity == nullptr) {
            return nullptr;
        }
    }
    if (host.entity_is_kind(entity, kProjectileHitListenerKind)) {
        host.notify_hit_listener_0077ce60(hit.record);
    }
    return entity;
}

}  // namespace bsp
