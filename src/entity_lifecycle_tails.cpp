// Packet cc2_entity_lifecycle_tails. See include/bsp/entity_lifecycle_tails.hpp and
// docs/ENTITY_LIFECYCLE_TAILS.md. Ghidra was read-only for this packet; every name is
// a hypothesis, not a recovered symbol.
#include "bsp/entity_lifecycle_tails.hpp"

namespace bsp {

// 009272EF MOV byte ptr [ESI+0x5d],1 and 009272F3 MOV dword ptr [ESI+0x70],1, taken
// only when the "deadMeat" field read by 009272E3 came back non-zero. The routine is
// the spawn-descriptor apply, so this is a creation-time write, not a teardown.
bool spawn_apply_dead_meat_009272ef(SceneNodeFlags& node, bool dead_meat) noexcept {
    if (!dead_meat) {
        return false;
    }
    node.torn_down = true;
    return true;  // the value stored into entity+70h
}

// 009274CE..009274DA. Unlike scene_node_kill_00922fd0 this also sets the removed byte
// and leaves the expiry counter at +6Ch alone.
void pending_flush_mark_teardown_009274ce(SceneNodeFlags& node) noexcept {
    node.torn_down = true;  // 009274CE
    node.destroyed = true;  // 009274D2
    node.removed = true;    // 009274D6
    node.active = false;    // 009274DA
}

// 00927080 TEST, then 00927086 CMP ECX,1 and 0092721B CMP ECX,3.
SpawnDescriptorKind spawn_descriptor_kind_00927086(bool descriptor_present,
                                                   std::int32_t kind) noexcept {
    if (!descriptor_present) {
        return SpawnDescriptorKind::none;
    }
    if (kind == 1) {
        return SpawnDescriptorKind::property_bag;
    }
    if (kind == 3) {
        return SpawnDescriptorKind::lua_table;
    }
    return SpawnDescriptorKind::other;
}

// 009287F5 CMP ECX,[00F89A10] / 00928809 JL: the low registry when id < split.
bool entity_id_uses_low_registry_009287f5(std::uint16_t id, std::uint32_t split) noexcept {
    return static_cast<std::uint32_t>(id) < split;
}

std::vector<EntityReleaseStep> entity_release_steps_006fe570(const EntityReleaseState& state) {
    std::vector<EntityReleaseStep> steps;

    // 009287B0, the GameEntity level. A class that does not derive from it skips both.
    if (state.is_game_entity) {
        steps.push_back(EntityReleaseStep::release_entity_id);  // 00928810, always
        if (state.has_name_string) {
            steps.push_back(EntityReleaseStep::free_name_string);  // 0092881B gate
        }
    }

    // 00925780, the entity root.
    if (state.has_owned_ref) {
        steps.push_back(EntityReleaseStep::release_owned_ref);  // 009257CF
    }
    if (state.has_children) {
        steps.push_back(EntityReleaseStep::destroy_children);  // the 009257D4 loop
    }
    steps.push_back(EntityReleaseStep::unlink_update_chain);  // 009257FC, unconditional
    if (state.has_parent) {
        steps.push_back(EntityReleaseStep::unlink_from_parent);  // 0092580C
    } else {
        steps.push_back(EntityReleaseStep::unlink_from_world_roots);  // 00925820
        // 00925825: the world unit list is only left from the parentless branch.
        if (state.world_list_counter != 0) {
            steps.push_back(EntityReleaseStep::remove_from_world_unit_list);  // 00925835
        }
    }
    if (state.has_pooled_string_160) {
        steps.push_back(EntityReleaseStep::free_pooled_string_160);  // 00925856
    }
    if (state.has_pooled_string_158) {
        steps.push_back(EntityReleaseStep::free_pooled_string_158);  // 0092587E
    }
    steps.push_back(EntityReleaseStep::destroy_weak_owner);      // 00925891
    steps.push_back(EntityReleaseStep::destroy_callback_owner);  // 0092589D

    // 00695760, the observer endpoint base.
    if (state.has_observer_list) {
        steps.push_back(EntityReleaseStep::detach_observer_edges);
    }
    if (state.has_observer_edge_array) {
        steps.push_back(EntityReleaseStep::free_observer_edge_array);
    }

    if (state.free_flag) {
        steps.push_back(EntityReleaseStep::free_instance);  // 006FE57F
    }
    return steps;
}

// 00903F30 and 00924710 are the same body over two link pairs. The early-out at
// 00903F40 only fires when the node is already detached on both sides AND the chain
// holds at most one entry, so it is a guard on the count, not on the node.
void chain_unlink_00903f30(IntrusiveChain& chain, std::size_t node,
                           std::vector<IntrusiveChainNode>& nodes) noexcept {
    if (node == 0 || node >= nodes.size()) {
        return;
    }
    IntrusiveChainNode& self = nodes[node];
    if (self.prev == 0 && self.next == 0 && chain.count <= 1) {
        return;  // 00903F39 / 00903F3E / 00903F44
    }
    if (self.prev != 0) {
        nodes[self.prev].next = self.next;  // 00903F4E
    } else {
        chain.first = self.next;  // 00903F56
    }
    if (self.next != 0) {
        nodes[self.next].prev = self.prev;  // 00903F62
    } else {
        chain.last = self.prev;  // 00903F6A
    }
    self.next = 0;  // 00903F6D
    self.prev = 0;  // 00903F74
    chain.count -= 1;  // 00903F7B
}

MarkerDropOutcome marker_drop_unit_006de3f0(bool slot_has_object,
                                            const std::vector<bool>& entry_keep_flags,
                                            bool pair_registered) noexcept {
    MarkerDropOutcome out;
    if (slot_has_object) {
        out.destroyed_slot_object = true;  // 006DE424, slot->vtable[0](1)
        out.cleared_slot = true;           // 006DE426, *slot = 0
    }
    for (bool keep : entry_keep_flags) {
        if (keep) {
            return out;  // 006DE47E jumps straight to the epilogue
        }
    }
    if (pair_registered) {
        out.unregistered_pair = true;  // 006DE4AC, 006952A0
    }
    return out;
}

ObjectiveEraseResult objective_erase_unit_entries_008dc3e0(
    const std::vector<ObjectiveUnitEntryHandle>& entries, std::size_t unit,
    std::int32_t size_before) {
    ObjectiveEraseResult result;
    result.size_after = size_before;
    for (const ObjectiveUnitEntryHandle& entry : entries) {
        if (entry.unit == unit) {  // 008DC423 CMP dword ptr [ECX],EDX
            result.erased += 1;
            result.size_after -= 1;  // 008DC473
            continue;                // the walk resumes at the saved successor
        }
        result.remaining.push_back(entry);
    }
    return result;
}

void entity_release_006fe570(EntityReleaseHost& host, const EntityReleaseState& state,
                             std::uint16_t entity_id, std::uint32_t id_split,
                             std::int32_t child_count) {
    for (EntityReleaseStep step : entity_release_steps_006fe570(state)) {
        switch (step) {
            case EntityReleaseStep::release_entity_id:
                host.release_entity_id(entity_id,
                                       entity_id_uses_low_registry_009287f5(entity_id, id_split));
                break;
            case EntityReleaseStep::free_name_string:
                host.free_pooled_string(kEntityOffNamePointer);
                break;
            case EntityReleaseStep::release_owned_ref:
                host.release_owned_ref();
                break;
            case EntityReleaseStep::destroy_children:
                for (std::int32_t i = 0; i < child_count; ++i) {
                    host.destroy_next_child();
                }
                break;
            case EntityReleaseStep::unlink_update_chain:
                host.unlink_update_chain();
                break;
            case EntityReleaseStep::unlink_from_parent:
                host.unlink_sibling_chain(true);
                host.clear_parent();  // 00925811
                break;
            case EntityReleaseStep::unlink_from_world_roots:
                host.unlink_sibling_chain(false);
                break;
            case EntityReleaseStep::remove_from_world_unit_list:
                host.remove_from_world_unit_list();
                break;
            case EntityReleaseStep::free_pooled_string_160:
                host.free_pooled_string(0x160);
                break;
            case EntityReleaseStep::free_pooled_string_158:
                host.free_pooled_string(0x158);
                break;
            case EntityReleaseStep::destroy_weak_owner:
                host.destroy_weak_owner();
                break;
            case EntityReleaseStep::destroy_callback_owner:
                host.destroy_callback_owner();
                break;
            case EntityReleaseStep::detach_observer_edges:
                host.detach_observer_edges();
                break;
            case EntityReleaseStep::free_observer_edge_array:
                host.free_observer_edge_array();
                break;
            case EntityReleaseStep::free_instance:
                host.free_instance();
                break;
        }
    }
}

}  // namespace bsp
