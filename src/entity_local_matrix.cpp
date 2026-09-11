#include "bsp/entity_local_matrix.hpp"

namespace bsp {
namespace {

// 006E00B1..006E00D2 and 0042142E..00431441, the identical child walk. The
// native loop reads the head once, tests it, then re-tests after each +44h
// step, so an entity with no children never enters the body.
std::size_t invalidate_children(EntityLocalMatrixHost& host, std::uint32_t entity)
{
    std::size_t visited = 0;
    std::uint32_t child = host.first_child(entity);
    while (child != 0) {
        host.invalidate_subtree_pose(child);
        child = host.next_sibling(child);
        ++visited;
    }
    return visited;
}

// The shared prefix of the two setters. Native order: the copy first, then the
// head load, then the two byte stores, then the walk. The head is loaded at
// 006E00B1 before the stores at 006E00B6 and 006E00BD, but neither store can
// alias the head field, so the observable order is copy, clear, walk.
EntityLocalMatrixSetResult set_local_matrix_common(EntityLocalMatrixHost& host,
                                                   std::uint32_t entity,
                                                   const float matrix[16])
{
    EntityLocalMatrixSetResult result{};
    host.copy_local_matrix(entity, matrix);
    host.clear_pose_cache_flags(entity);
    result.children_invalidated = invalidate_children(host, entity);
    return result;
}

} // namespace

EntityLocalMatrixSetResult set_entity_local_matrix_00431410(EntityLocalMatrixHost& host,
                                                            std::uint32_t entity,
                                                            const float matrix[16])
{
    return set_local_matrix_common(host, entity, matrix);
}

EntityLocalMatrixSetResult set_entity_local_matrix_006e00a0(EntityLocalMatrixHost& host,
                                                            std::uint32_t entity,
                                                            const float matrix[16])
{
    EntityLocalMatrixSetResult result = set_local_matrix_common(host, entity, matrix);
    // 006E00D4..006E00E3. Unconditional: the notification runs whether or not
    // the entity had children, and [entity+310h] is never null-checked.
    host.notify_local_matrix_owner(entity);
    result.owner_notified = true;
    return result;
}

EntityRefreshOutcome refresh_entity_00955970(EntityRefreshHost& host, std::uint32_t entity)
{
    // 00955979: a null scene node skips the whole body. The epilogue at
    // 00955A30 is shared with the kPlainLocal exit.
    const std::uint32_t node = host.scene_node(entity);
    if (node == 0) return EntityRefreshOutcome::kNoSceneNode;

    // 00955986: a parented entity publishes the cached world matrix.
    if (host.parent(entity) != 0) {
        // 0095598C..00955995. The guard is inverted relative to 00414DB0's own
        // head, which repeats the same test; a valid cache calls nothing.
        if (!host.world_matrix_valid(entity)) host.refresh_world_matrix(entity);
        // 009559A2..009559AC. LEA EAX,[EBX+0CCh]: the address of the cached
        // world matrix, not a copy of it.
        host.scene_node_set_pose(node, entity + kUnitOffPoseBlock);
        return EntityRefreshOutcome::kParentedPose;
    }

    // 009559B6..009559C2. A root entity asks whether it has a derived local
    // matrix. Every shipped implementation of slot 8Ch is 0042B9A0, which
    // returns false, so the kDerivedLocal branch below is unreachable in this
    // image; it is reconstructed because the listing was read, not because a
    // call reaches it.
    if (!host.has_derived_local_matrix(entity)) {
        // 00955A1F..00955A2E. ADD EBX,74h then PUSH EBX: the entity's own local
        // matrix goes straight to the node.
        float local[16];
        host.read_local_matrix(entity, local);
        host.scene_node_set_world_matrix(node, local);
        return EntityRefreshOutcome::kPlainLocal;
    }

    // 009559C4..00955A13. Three stack temporaries. The pushes interleave with
    // the two calls: &local and &product are pushed for 00413920 before
    // slot 90h is dispatched with &derived as its by-value return buffer, and
    // slot 90h's RET 4 pops only that one. The multiply then finds its own two
    // arguments already in place.
    float local[16];
    host.read_local_matrix(entity, local); // 009559D4, REP MOVSD of 16 dwords
    float derived[16];
    host.derived_local_matrix(entity, derived); // 009559F0, returns &derived
    float product[16];
    // 009559F4: ECX = the slot 90h result (left), stack (product, local).
    host.multiply_matrices(product, derived, local);
    // 009559FA..009559FE: the product is copied back over the local temporary,
    // and 00955A0E hands that same temporary to the node.
    for (std::size_t i = 0; i < kEntityMatrixFloatCount; ++i) local[i] = product[i];
    host.scene_node_set_world_matrix(node, local);
    return EntityRefreshOutcome::kDerivedLocal;
}

void write_identity_matrix_0042d700(float out[16]) noexcept
{
    // 0042D707..0042D759. XMM1 holds 1.0f from 00D7A24C, XMM0 is zeroed by
    // XORPS; the sixteen MOVSS stores run in ascending address order.
    for (std::size_t i = 0; i < kEntityMatrixFloatCount; ++i) out[i] = 0.0f;
    out[0] = 1.0f;  // 0042D70F
    out[5] = 1.0f;  // 0042D727
    out[10] = 1.0f; // 0042D740
    out[15] = 1.0f; // 0042D759
}

} // namespace bsp
