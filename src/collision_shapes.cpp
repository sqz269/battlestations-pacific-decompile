#include "bsp/collision_shapes.hpp"

// Reconstruction of the collision shape classes, the node-level sphere reject,
// the unit part's sphere test and the two shape installers.
// Addresses and evidence: docs/COLLISION_SHAPES.md, reports/collision_shapes.json.

namespace bsp {

bool shape_class_answers_sphere(CollisionShapeClass shape_class) noexcept {
    // Four of the five classes put XOR AL,AL; RET 0Ch in slot 4 (004F13A0,
    // 006D3100, 00929FF0) or leave it __purecall (00BF698E). Only 00CFD768
    // supplies a body, 0070F720.
    return shape_class == CollisionShapeClass::UnitPart;
}

bool shape_class_from_vtable(std::uint32_t vtable, CollisionShapeClass& out) noexcept {
    switch (vtable) {
    case kVTableCollisionShapeBase:
        out = CollisionShapeClass::Base;
        return true;
    case kVTableSubobjectShape:
        out = CollisionShapeClass::Subobject;
        return true;
    case kVTableFactoryShape:
        out = CollisionShapeClass::Factory;
        return true;
    case kVTableUnitPartShape:
        out = CollisionShapeClass::UnitPart;
        return true;
    case kVTableTransformedBoxShape:
        out = CollisionShapeClass::TransformedBox;
        return true;
    default:
        return false;
    }
}

bool collision_node_sphere_aabb_overlap(const float node_min[3], const float node_max[3],
                                        const float centre[3], float radius) noexcept {
    // Sphere AABB, 0098AB01-0098AB43. The x87 sequence stores centre-radius into
    // the three min slots and centre+radius into the three max slots.
    const float sphere_min[3] = {centre[0] - radius, centre[1] - radius, centre[2] - radius};
    const float sphere_max[3] = {centre[0] + radius, centre[1] + radius, centre[2] + radius};

    // Six FCOMIP tests, in the native order x, x, z, z, y, y. Each rejects only
    // on a strict separation:
    //   0098AB4B  node_min.x >  sphere_max.x   (JBE continues, so > rejects)
    //   0098AB62  sphere_min.x > node_max.x    (JA rejects)
    //   0098AB72  node_min.z >  sphere_max.z
    //   0098AB82  sphere_min.z > node_max.z
    //   0098AB92  node_min.y >  sphere_max.y
    //   0098ABA2  sphere_min.y > node_max.y
    // Written as the negation so that a NaN operand takes the continue side on
    // every test, matching the unordered FCOMIP flags the native branches see.
    if (node_min[0] > sphere_max[0]) {
        return false;
    }
    if (sphere_min[0] > node_max[0]) {
        return false;
    }
    if (node_min[2] > sphere_max[2]) {
        return false;
    }
    if (sphere_min[2] > node_max[2]) {
        return false;
    }
    if (node_min[1] > sphere_max[1]) {
        return false;
    }
    if (sphere_min[1] > node_max[1]) {
        return false;
    }
    return true;
}

bool collision_body_needs_prepare(std::uint8_t body_flags) noexcept {
    // TEST byte ptr [EDI+5Ch],2; JNZ skips the prepare call.
    return (body_flags & kCollisionBodyPreparedBit) == 0;
}

bool unit_part_shape_test_sphere(const UnitPartCollisionShapeRecord& shape,
                                 const float centre[3], float radius, void* record,
                                 UnitPartShapeSphereHost& host) noexcept {
    // 0070F724: EDI = shape+20h, read once and reused for the tail argument.
    const std::uint32_t body = shape.body;
    if (collision_body_needs_prepare(host.body_flags(body))) {
        host.prepare_body(body); // 0070F72F
    }
    // 0070F746 reloads shape+20h for the query's `this`.
    const std::uint32_t query_result = host.body_sphere_query(body, centre, radius, record);
    // 0070F752: ADD EDI,0F0h, then this = shape+24h.
    return host.owner_accept_sphere_hit(shape.owner, body + 0xF0, query_result);
}

void collision_node_add_shape(CollisionNodeShapeArray& array, std::uint32_t shape) noexcept {
    // 006D1400: shapes[count] = shape; count += 1. The native store has no
    // bounds check, so the overflow is recorded rather than silently dropped.
    if (array.count >= 0 && static_cast<std::size_t>(array.count) < kCollisionNodeShapeSlots) {
        array.shapes[array.count] = shape;
    } else {
        array.overflowed += 1;
    }
    array.count += 1;
}

void collision_node_clear_shapes(CollisionNodeShapeArray& array) noexcept {
    // 006D1420 writes the count only; the slots keep their stale pointers.
    array.count = 0;
}

void install_tickable_entity_shape(std::uint32_t entity, CollisionNodeShapeArray& array,
                                   const float node_min[3], const float node_max[3],
                                   CollisionShapeInstallHost& host) noexcept {
    const std::uint32_t node = entity + static_cast<std::uint32_t>(kTickableEntityOffNode);
    const std::uint32_t box = entity + static_cast<std::uint32_t>(kTickableEntityOffBoxShape);

    // 0092B289-0092B2E1. Exactly one shape, and the count is read through the
    // entity as [entity+2BCh], which is the same field as node+F8h.
    collision_node_add_shape(array, box);

    host.set_node_bounds(node, node_min, node_max); // 0092B2F9
    const std::uint32_t index = host.spatial_index(); // 0092B305
    host.insert_node(index, node);                    // 0092B30C
}

int install_unit_part_shapes(std::uint32_t node, CollisionNodeShapeArray& array,
                             CollisionShapeInstallHost& host) noexcept {
    // 00712700-00712758. The node owns the shapes; the array holds their
    // addresses. Each element is given its bounds before it is published.
    std::uint32_t elements[kCollisionNodeShapeSlots]{};
    const int produced =
        host.shape_list(node, elements, static_cast<int>(kCollisionNodeShapeSlots));

    int published = 0;
    for (int i = 0; i < produced; ++i) {
        const CollisionShapeBounds bounds = host.shape_source_bounds(node, i);
        host.set_shape_bounds(elements[i], bounds.min, bounds.max); // 00712723
        collision_node_add_shape(array, elements[i]);               // 0071274A
        ++published;
    }
    return published;
}

int hit_record_vector_size(const HitRecordVectorState& vec) noexcept {
    // 0098C467-0098C486: a null first is size zero without a division.
    if (vec.first == 0) {
        return 0;
    }
    return static_cast<int>((vec.last - vec.first) / kHitRecordStride);
}

int hit_record_vector_capacity(const HitRecordVectorState& vec) noexcept {
    if (vec.first == 0) {
        return 0;
    }
    return static_cast<int>((vec.end - vec.first) / kHitRecordStride);
}

HitRecordAppendPath hit_record_append_path(const HitRecordVectorState& vec) noexcept {
    // 0098C488: a null first goes straight to the grow path. Otherwise
    // 0098C4A2 compares size against capacity with JNC, so equality grows.
    if (vec.first == 0) {
        return HitRecordAppendPath::Grow;
    }
    return hit_record_vector_size(vec) < hit_record_vector_capacity(vec)
               ? HitRecordAppendPath::ConstructInPlace
               : HitRecordAppendPath::Grow;
}

} // namespace bsp
