// Explosion radial damage. Evidence: docs/EXPLOSION_RADIAL_DAMAGE.md.
// Every sequence below follows the native order of effects, because the record
// fields are overwritten more than once along the path: the gather writes the
// centre, radius and flag, and 0084BAD0 then writes the damage, the shot, the
// centre again over the shape's hit position, and the flag again.
#include "bsp/blast_damage.hpp"

namespace bsp {

float blast_falloff_fraction_004705c0(float distance, float radius,
                                      bool ignore_falloff) noexcept
{
    // 004705E7: FDIV without a zero guard, then FSUBR against 1.0f.
    float falloff = 1.0f - distance / radius;
    // 00470602: the flag only lifts a fraction that is already positive.
    if (ignore_falloff && falloff > 0.0f) {
        falloff = 1.0f;
    }
    return falloff;
}

HitQueryBounds blast_sphere_bounds(const HitQueryPoint& centre, float radius) noexcept
{
    // 0098AAF3..0098AB43 and the same shape at 0098C639..0098C6A8.
    HitQueryBounds bounds{};
    bounds.min.x = centre.x - radius;
    bounds.min.y = centre.y - radius;
    bounds.min.z = centre.z - radius;
    bounds.max.x = centre.x + radius;
    bounds.max.y = centre.y + radius;
    bounds.max.z = centre.z + radius;
    return bounds;
}

bool blast_sphere_overlaps_bounds_0098aae0(const HitQueryBounds& sphere,
                                           const HitQueryBounds& node) noexcept
{
    // The six comparisons in the native's order: 0098AB4B (node min x against
    // the sphere max x), 0098AB62 (node max x), 0098AB72/0098AB82 (the z pair),
    // 0098AB92/0098ABA2 (the y pair). Each JA/JBE rejects on a strict
    // separation, so a sphere that only touches the box still overlaps.
    if (node.min.x > sphere.max.x) { return false; }
    if (sphere.min.x > node.max.x) { return false; }
    if (node.min.z > sphere.max.z) { return false; }
    if (sphere.min.z > node.max.z) { return false; }
    if (node.min.y > sphere.max.y) { return false; }
    if (sphere.min.y > node.max.y) { return false; }
    return true;
}

void blast_gather_record_fill_00904470(HitRecordFill& record,
                                       const HitQueryPoint& centre, float radius,
                                       bool ignore_falloff) noexcept
{
    record.blast_centre = centre;   // +18h..+20h, 00904503..00904511
    record.falloff_range = radius;  // +24h, 00904524
    record.ignore_falloff = ignore_falloff; // +2Ch, 00904537
}

void blast_apply_record_fill_0084bad0(HitRecordFill& record,
                                      const HitQueryPoint& centre, float damage,
                                      bool ignore_falloff) noexcept
{
    record.part_damage_base = damage; // +28h, 0084BBAC
    record.position = centre;         // +8h..+10h, 0084BBBC..0084BBCC
    record.ignore_falloff = ignore_falloff; // +2Ch again, 0084BBD1
}

bool blast_record_is_queued_0084bad0(const HitRecordFill& record,
                                     const void* source_entity) noexcept
{
    return record.entity != source_entity; // 0084BBCF
}

bool collision_node_sphere_test_0098aae0(CollisionNodeSphereHost& host,
                                         const void* node,
                                         const HitQueryPoint& centre, float radius,
                                         HitRecordFill& record)
{
    const HitQueryBounds sphere = blast_sphere_bounds(centre, radius);
    if (!blast_sphere_overlaps_bounds_0098aae0(sphere, host.node_bounds(node))) {
        return false; // 0098AB57, AL cleared
    }

    // 0098ABB2: the inline shape array at node+D0h, count at node+F8h. The
    // return is the OR of the per-shape results, and an empty array falls out
    // at 0098AC03 with the flag still zero.
    bool hit = false;
    const int shapes = host.node_shape_count(node);
    for (int slot = 0; slot < shapes; ++slot) {
        if (host.shape_overlaps_sphere(host.node_shape(node, slot), centre, radius,
                                       record)) {
            hit = true; // 0098ABE9
        }
    }
    return hit;
}

bool collision_node_overlap_sphere_0098c510(SphereQueryHost& host,
                                            const SphereQueryArgs& args,
                                            const void* node)
{
    if (node == args.exclude) {
        return false; // 0098C535, the source entity's own node
    }

    HitRecordFill record{};
    host.record_reset(record);                    // 00470470 at 0098C55D
    record.entity = host.node_owner_entity(node); // 00470370 at 0098C570

    bool hit = false;
    if (host.node_overlaps_sphere(node, args.centre, args.radius, args.exclude,
                                  record)) {
        host.append_record(record); // 0098C460 at 0098C5A7
        hit = true;                 // 0098C5AC
    }

    // 0098C5C0: the children are walked whether or not this node hit, with the
    // same exclude and the same output vector.
    const int children = host.node_child_count(node);
    for (int slot = 0; slot < children; ++slot) {
        if (collision_node_overlap_sphere_0098c510(host, args,
                                                   host.node_child(node, slot))) {
            hit = true; // 0098C5EF
        }
    }

    host.destroy_record(record); // 004704B0 at 0098C606
    return hit;
}

bool spatial_index_query_sphere_0098c630(SphereQueryHost& host,
                                         const SphereQueryArgs& args)
{
    const HitQueryBounds sphere = blast_sphere_bounds(args.centre, args.radius);
    HitQueryCell min_cell = host.cell_of_point(sphere.min);
    HitQueryCell max_cell = host.cell_of_point(sphere.max);

    // 0098C6BE..0098C6F0. Only the low end is clamped up and only the high end
    // is clamped down, so a sphere entirely off the grid still walks one row.
    if (min_cell.x < 0) { min_cell.x = 0; }
    if (min_cell.z < 0) { min_cell.z = 0; }
    if (max_cell.x > kSpatialGridMaxIndex) { max_cell.x = kSpatialGridMaxIndex; }
    if (max_cell.z > kSpatialGridMaxIndex) { max_cell.z = kSpatialGridMaxIndex; }

    const int stamp = host.query_stamp(); // [world+648h] at 0098C653
    bool hit = false;
    for (int x = min_cell.x; x <= max_cell.x; ++x) {          // 0098C6F4
        for (int z = min_cell.z; z <= max_cell.z; ++z) {      // 0098C720
            for (const void* link = host.cell_first_link(x, z); link != nullptr;
                 link = host.cell_next_link(link)) {          // 0098C740/0098C77F
                const void* node = host.cell_link_node(link); // link+8h
                if (host.node_stamp(node) == stamp) {
                    continue; // 0098C753, already tested this query
                }
                host.set_node_stamp(node, stamp); // 0098C762
                if (collision_node_overlap_sphere_0098c510(host, args, node)) {
                    hit = true; // 0098C77B, the OR of every node's result
                }
            }
        }
    }
    return hit;
}

void blast_gather_hit_records_00904470(BlastGatherHost& host, const BlastArgs& args)
{
    SphereQueryArgs query{};
    query.centre = args.centre;
    query.radius = args.radius;
    // 00904476: a null source entity leaves the exclude null rather than
    // calling through a null vtable.
    query.exclude = args.source_entity != nullptr
                        ? host.entity_collision_node(args.source_entity)
                        : nullptr;

    host.query_sphere(host.spatial_index(), query); // 0042E630 then 0098C630

    // 009044B4: every gathered record, 54h apart. The shot and the three blast
    // fields are written here and nowhere else on the gather side.
    const int count = host.gathered_count();
    for (int slot = 0; slot < count; ++slot) {
        HitRecordFill& record = host.gathered_record(slot);
        host.record_set_shot(record, args.shot);
        blast_gather_record_fill_00904470(record, args.centre, args.radius,
                                          args.ignore_falloff);
    }
}

void blast_apply_radial_damage_0084bad0(BlastDamageHost& host, const BlastArgs& args)
{
    // 0084BB00, ahead of the world-mode test: this one runs even when the blast
    // itself is suppressed.
    host.call_00428800(args.radius, args.damage);
    if (host.world_blast_mode() == kWorldBlastSuppressedMode) {
        return; // 0084BB11
    }

    host.gather_hit_records(args); // 0084BB4A

    const int count = host.gathered_count();
    for (int slot = 0; slot < count; ++slot) {
        HitRecordFill& record = host.gathered_record(slot);
        // The native writes the damage at 0084BBAC, before the shot call at
        // 0084BBB1, and the centre and flag after it. 00470350 touches only +4h
        // and +14h, so doing the three field writes together is the same state.
        host.record_set_shot(record, args.shot);
        blast_apply_record_fill_0084bad0(record, args.centre, args.damage,
                                         args.ignore_falloff);
        if (blast_record_is_queued_0084bad0(record, args.source_entity)) {
            HitQueryPoint direction{};
            direction.y = kBlastQueueDirectionY; // (0, 1, 0) at 0084BBD6
            host.queue_hit(record, direction);   // 00926E80
        }
    }

    // 0084BC22: every record is destroyed and the buffer freed, queued or not.
    for (int slot = 0; slot < count; ++slot) {
        host.destroy_record(host.gathered_record(slot));
    }
    host.free_gathered();
}

} // namespace bsp
