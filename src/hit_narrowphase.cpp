// Reconstruction of the hit narrowphase. See docs/HIT_NARROWPHASE.md for the
// evidence behind every rule; the comments here only cite the site.
#include "bsp/hit_narrowphase.hpp"

#include <cmath>

namespace bsp {
namespace {

constexpr float kHalf = 0.5f; // the double at 00D7A280, used as a float everywhere

inline float abs_f(float v) noexcept { return v < 0.0f ? -v : v; }

} // namespace

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

HitQueryBounds segment_bounds_00722b20(const HitQueryPoint& from,
                                       const HitQueryPoint& to) noexcept
{
    HitQueryBounds out{};
    // 00722B20 tests x and y with `<=` on the second argument and z with `<` on
    // the first; the branch shape differs but the result is the same min/max.
    out.min.x = to.x <= from.x ? to.x : from.x;
    out.max.x = to.x <= from.x ? from.x : to.x;
    out.min.y = to.y <= from.y ? to.y : from.y;
    out.max.y = to.y <= from.y ? from.y : to.y;
    out.min.z = from.z < to.z ? from.z : to.z;
    out.max.z = from.z < to.z ? to.z : from.z;
    return out;
}

HitQueryCell cell_of_point_0098ad60(const HitQueryPoint& p, float cell_size) noexcept
{
    HitQueryCell cell{};
    cell.x = static_cast<int>(std::floor(p.x / cell_size)) + kSpatialGridHalf;
    cell.z = static_cast<int>(std::floor(p.z / cell_size)) + kSpatialGridHalf;
    return cell;
}

HitQueryCell clamp_cell_0098add0(const HitQueryCell& cell, bool clamp_low) noexcept
{
    HitQueryCell out = cell;
    if (clamp_low) {
        // 0098AE37 and 0098AE3D: only the low pair is raised to zero.
        if (out.x < 0) { out.x = 0; }
        if (out.z < 0) { out.z = 0; }
    } else {
        // 0098AE55 and 0098AE5A: only the high pair is lowered to 95h.
        if (out.x > kSpatialGridMaxIndex) { out.x = kSpatialGridMaxIndex; }
        if (out.z > kSpatialGridMaxIndex) { out.z = kSpatialGridMaxIndex; }
    }
    return out;
}

bool bounds_overlap_0098add0(const HitQueryBounds& segment,
                             const HitQueryBounds& box) noexcept
{
    // The six JA branches at 0098AEEE..0098AF60, in their listing order:
    // x low, x high, z low, z high, y low, y high.
    if (!(box.min.x <= segment.max.x)) { return false; }
    if (!(segment.min.x <= box.max.x)) { return false; }
    if (!(box.min.z <= segment.max.z)) { return false; }
    if (!(segment.min.z <= box.max.z)) { return false; }
    if (!(box.min.y <= segment.max.y)) { return false; }
    if (!(segment.min.y <= box.max.y)) { return false; }
    return true;
}

bool segment_overlaps_box_0085cad0(const HitQueryBounds& box,
                                   const HitQueryPoint& from,
                                   const HitQueryPoint& to) noexcept
{
    // Both operands are treated as boxes: half-extents and centre differences,
    // three face axes then three cross axes. x, then z, then y, matching the
    // order the native evaluates them in.
    const float seg_x = (to.x - from.x) * kHalf;
    const float seg_ax = abs_f(seg_x);
    const float box_x = (box.max.x - box.min.x) * kHalf;
    const float dx = ((from.x + to.x) - (box.min.x + box.max.x)) * kHalf;
    if (!(abs_f(dx) <= seg_ax + box_x)) { return false; }

    const float seg_z = (to.z - from.z) * kHalf;
    const float seg_az = abs_f(seg_z);
    const float box_z = (box.max.z - box.min.z) * kHalf;
    const float dz = ((from.z + to.z) - (box.min.z + box.max.z)) * kHalf;
    if (!(abs_f(dz) <= seg_az + box_z)) { return false; }

    const float seg_y = (to.y - from.y) * kHalf;
    const float seg_ay = abs_f(seg_y);
    const float box_y = (box.max.y - box.min.y) * kHalf;
    const float dy = ((from.y + to.y) - (box.min.y + box.max.y)) * kHalf;
    if (!(abs_f(dy) <= seg_ay + box_y)) { return false; }

    if (!(abs_f(seg_y * dz - dy * seg_z) <= seg_ay * box_z + box_y * seg_az)) { return false; }
    if (seg_ax * box_y + box_x * seg_ay < abs_f(seg_x * dy - dx * seg_y)) { return false; }
    return abs_f(seg_z * dx - dz * seg_x) <= box_z * seg_ax + seg_az * box_x;
}

// ---------------------------------------------------------------------------
// The hit record, producer side
// ---------------------------------------------------------------------------

void hit_record_reset_00470470(HitRecordFill& record) noexcept
{
    record.shape_kind = kHitRecordResetIndex;   // +30h
    record.hull_segment = kHitRecordResetIndex; // +34h
    record.source_record = nullptr;             // +38h
    record.part_hits = nullptr;                 // +3Ch
    record.part_hit_count = 0;                  // +40h
    record.part_capacity = 0;                   // +44h
    record.entity = nullptr;                    // +0h
    record.shot = nullptr;                      // +4h
    record.applied_damage = 0.0f;               // +48h
    record.ignore_falloff = false;              // the byte at +2Ch
    // +4Ch and +50h are also cleared; this projection does not carry them.
}

void hit_record_set_shot_00470350(HitRecordFill& record, const void* shot,
                                  float shot_hull_damage) noexcept
{
    record.shot = shot;
    if (shot != nullptr) {
        record.hull_damage_base = shot_hull_damage; // shot->vtable[54h]()
    }
}

void hit_record_set_entity_00470370(HitRecordFill& record, const void* owner_entity) noexcept
{
    record.entity = owner_entity;
}

void shape_hit_fill_0087fec0(HitRecordFill& record, const HitQueryPoint& hit_position,
                             const void* owner_entity) noexcept
{
    record.position = hit_position;
    hit_record_set_entity_00470370(record, owner_entity);
    record.shape_kind = kHitRecordShapeKindSimple;
    record.hull_segment = kHitRecordNoHullSegment;
}

void blast_record_prefill_00904470(HitRecordFill& record, const void* shot,
                                   float shot_hull_damage, const HitQueryPoint& centre,
                                   float radius, bool ignore_falloff) noexcept
{
    hit_record_set_shot_00470350(record, shot, shot_hull_damage);
    record.blast_centre = centre;
    record.falloff_range = radius;
    record.ignore_falloff = ignore_falloff;
}

void blast_record_finish_0084bad0(HitRecordFill& record, const void* shot,
                                  float shot_hull_damage, const HitQueryPoint& centre,
                                  float damage, bool ignore_falloff) noexcept
{
    record.part_damage_base = damage; // +28h, written before the second SetShot
    hit_record_set_shot_00470350(record, shot, shot_hull_damage);
    record.position = centre;         // +8h..+10h, the burst centre
    record.ignore_falloff = ignore_falloff;
}

bool blast_record_queues_0084bad0(const HitRecordFill& record, const void* source_entity) noexcept
{
    return record.entity != source_entity;
}

HitRecord consumer_view(const HitRecordFill& record, float weapon_scale,
                        float owner_modifier) noexcept
{
    HitRecord out{};
    out.hull_damage_base = record.hull_damage_base;
    out.part_damage_base = record.part_damage_base;
    out.falloff_range = record.falloff_range;
    out.ignore_falloff = record.ignore_falloff;
    // +0Ch is the hit position's y: the same dword, which is why 008777F8 reads
    // a negative value as "below the surface".
    out.armour_selector = record.position.y;
    out.hull_segment = record.hull_segment;
    out.weapon_scale = weapon_scale;
    out.owner_modifier = owner_modifier;
    out.part_hits = record.part_hits;
    out.part_hit_count = record.part_hit_count;
    return out;
}

// ---------------------------------------------------------------------------
// Part selection
// ---------------------------------------------------------------------------

WorstPartResult worst_part_004706d0(const HitRecord& hit, float armour_scaled) noexcept
{
    WorstPartResult out{};
    out.index = kHitPartEntryNoPart;
    out.damage = 0.0f;
    for (int i = 0; i < hit.part_hit_count; ++i) {
        const float damage = part_damage_004705c0(hit, armour_scaled, i);
        if (out.damage < damage) {
            out.index = i;
            out.damage = damage;
        }
    }
    return out;
}

float max_part_damage_00470740(const HitRecord& hit, float armour_scaled) noexcept
{
    float best = 0.0f;
    for (int i = 0; i < hit.part_hit_count; ++i) {
        const float damage = part_damage_004705c0(hit, armour_scaled, i);
        if (best < damage) { best = damage; }
    }
    return best;
}

// ---------------------------------------------------------------------------
// Scene-node flags
// ---------------------------------------------------------------------------

void scene_node_enable_00922f30(SceneNodeFlags& node) noexcept
{
    if (!node.destroyed && !node.active) { node.active = true; }
}

void scene_node_disable_00922f80(SceneNodeFlags& node) noexcept
{
    if (!node.destroyed && node.active) { node.active = false; }
}

void scene_node_kill_00922fd0(SceneNodeFlags& node) noexcept
{
    node.destroyed = true;
    node.torn_down = true;
    node.active = false;
}

void scene_node_remove_009263c0(SceneNodeFlags& node) noexcept
{
    if (node.destroyed) { return; } // the 009263E2 guard
    node.torn_down = true;
    node.destroyed = true;
    node.removed = true;
    node.active = false;
}

bool projectile_sweep_enabled_006e64ef(const SceneNodeFlags& node) noexcept
{
    return node.active;
}

bool projectile_may_bind_006e6bb0(const SceneNodeFlags& node) noexcept
{
    return !node.torn_down;
}

// ---------------------------------------------------------------------------
// The segment query
// ---------------------------------------------------------------------------

namespace {

// The kind filter and the exclude test both entity-level passes share.
bool entity_passes_filter(SegmentQueryHost& host, const void* entity,
                          const SegmentQueryArgs& args) noexcept
{
    if (args.kind_filter >= 1) {
        const void* owner = host.entity_owner(entity);
        if (owner != nullptr && !host.entity_is_kind(owner, args.kind_filter)) {
            return false;
        }
    }
    return args.exclude_entity != entity;
}

} // namespace

bool test_entity_0098ac20(SegmentQueryHost& host, const void* entity,
                          const SegmentQueryArgs& args, HitQueryPoint& to,
                          HitRecordFill& record) noexcept
{
    bool hit_any = false;
    const int shapes = host.shape_count(entity);
    for (int slot = 0; slot < shapes; ++slot) {
        if (!host.shape_trace_segment(entity, slot, args.from, to, record)) { continue; }
        to = record.position; // 0098AC88: shorten to the hit
        hit_any = true;
        hit_record_set_entity_00470370(record, host.entity_owner(entity));
    }

    // 0098ACC0: the children are only walked when there is no kind filter.
    if (args.kind_filter != 0) { return hit_any; }

    const int children = host.child_count(entity);
    for (int slot = 0; slot < children; ++slot) {
        const void* child = host.child_entity(entity, slot);
        if (args.exclude_entity == child) { continue; }
        if (!segment_overlaps_box_0085cad0(host.entity_bounds(child), args.from, to)) { continue; }
        if (!test_entity_0098ac20(host, child, args, to, record)) { continue; }
        to = record.position;
        hit_any = true;
    }
    return hit_any;
}

bool query_segment_0098add0(SegmentQueryHost& host, const SegmentQueryArgs& args,
                            HitRecordFill& record) noexcept
{
    HitQueryPoint to = args.to; // the mutable copy taken at 0098ADE5
    const HitQueryBounds segment = segment_bounds_00722b20(args.from, args.to);

    const float cell_size = host.grid_cell_size();
    const HitQueryCell low = clamp_cell_0098add0(cell_of_point_0098ad60(segment.min, cell_size), true);
    const HitQueryCell high = clamp_cell_0098add0(cell_of_point_0098ad60(segment.max, cell_size), false);

    bool hit_any = false;

    const auto consider = [&](const void* entity) {
        if (!entity_passes_filter(host, entity, args)) { return; }
        const HitQueryBounds box = host.entity_bounds(entity);
        if (!bounds_overlap_0098add0(segment, box)) { return; }
        if (!segment_overlaps_box_0085cad0(box, args.from, to)) { return; }
        if (!test_entity_0098ac20(host, entity, args, to, record)) { return; }
        to = record.position;
        hit_any = true;
    };

    // 0098AE64: the rectangle is walked only when the low cell is not past the
    // high one, and both bounds are inclusive.
    if (low.x <= high.x) {
        for (int x = low.x; x <= high.x; ++x) {
            for (int z = low.z; z <= high.z; ++z) {
                for (const void* node = host.cell_first_node(x, z); node != nullptr;
                     node = host.cell_next_node(node)) {
                    consider(host.cell_node_entity(node));
                }
            }
        }
    }

    // 0098AFDC: then the unbucketed array, with the same filter chain.
    const int loose = host.loose_entity_count();
    for (int slot = 0; slot < loose; ++slot) {
        consider(host.loose_entity(slot));
    }

    return hit_any;
}

// ---------------------------------------------------------------------------
// The flak burst
// ---------------------------------------------------------------------------

void flak_detonate_0070c210(FlakDetonationHost& host, const FlakBurstClass& desc,
                            const void* owner_entity, const void* projectile) noexcept
{
    if (!host.world_pose_valid()) { host.refresh_world_pose(); }
    const float damage = host.random_in_range(desc.damage_min, desc.damage_max);
    if (!host.world_pose_valid()) { host.refresh_world_pose(); }

    FlakBurstParameters burst{};
    burst.centre = host.world_translation();
    burst.radius = desc.radius;
    burst.damage = damage;
    burst.ignore_falloff = true;
    burst.excluded_entity = owner_entity;
    burst.shot = projectile;
    host.apply_explosion(burst);

    if (desc.burst_effect != nullptr) {
        if (!host.world_pose_valid()) { host.refresh_world_pose(); }
        host.spawn_burst_effect(desc.burst_effect, host.world_translation());
    }

    const void* attached = host.attached_effect();
    if (attached != nullptr) {
        if (!host.world_pose_valid()) { host.refresh_world_pose(); }
        host.move_attached_effect(attached, host.world_translation());
    }

    host.kill_projectile(kFlakKillMode);
}

} // namespace bsp
