#include "bsp/native_node_destruction.hpp"
#include "bsp/point_effect_matrix_setters.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <algorithm>
#include <stdexcept>

namespace bsp {
void shrink_native_node_point_lights_to_zero_00b6ec70(NativeNodePointLightArray& array) noexcept {
    while (array.count > 0) --array.count;
    array.count = 0;
}
namespace {
void unregister_current_attachment(NativeNodeDestructionRuntime& runtime,
    CameraTransform& node) {
    if (void* identity = node.notification_context) {
        unregister_generated_model_attachment_00b8f4c0(runtime.attachments.attachment(identity), node);
        node.notification_context = nullptr; // native callers repeat this store
        node.notify_changed = nullptr; // host projection of absent A0 virtual+3C
    }
}
void destroy_point_light_array(NativeNodeStorage& node) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(node.point_lights_164);
    singleton_lifetime_free(node.point_lights_164.begin);
    // Native leaves the freed pointer and capacity words unchanged.
}
void destroy_name(NativeStringStorage& strings, NativeNodeStorage& node) noexcept {
    destroy_native_string_header_0041dd20(&node.name_54, strings);
}
void destroy_reference_base(NativeNodeStorage& node) noexcept {
    node.vtable_00 = 0x00d5c104u;
    node.vtable_00 = 0x00ceb130u; // BD30F0, no reference-count modification
    node.~NativeNodeStorage();
}
}

NativeNodeDestructionRuntime::NativeNodeDestructionRuntime(SceneAttachmentRuntime& scene_runtime,
    GeneratedModelLifetimeRuntime& actual_attachments, SizedStoragePool& string_pool,
    SceneTypePredicate actual_node_virtual_0c)
    : scenes(scene_runtime), attachments(actual_attachments), strings(string_pool),
      node_virtual_0c(actual_node_virtual_0c) {
    if (&attachments.scenes != &scenes || !node_virtual_0c)
        throw std::invalid_argument("native node destruction requires one scene runtime and actual node type predicate");
}
void NativeNodeDestructionRuntime::bind_retained_owner(NativeNodeRetainedOwnerBinding& owner) {
    if (!owner.identity || !owner.destroy_on_zero ||
        reinterpret_cast<const std::byte*>(&owner.references_04) !=
            static_cast<const std::byte*>(owner.identity) + 4)
        throw std::invalid_argument("retained owner identity, actual count and virtual+00 are required");
    for (const auto* old : retained_owners_) {
        if (old == &owner) return;
        if (old->identity == owner.identity)
            throw std::invalid_argument("retained owner identity must have one actual binding");
    }
    retained_owners_.push_back(&owner);
}
void NativeNodeDestructionRuntime::unbind_retained_owner(NativeNodeRetainedOwnerBinding& owner) noexcept {
    retained_owners_.erase(std::remove(retained_owners_.begin(), retained_owners_.end(), &owner), retained_owners_.end());
}
void NativeNodeDestructionRuntime::release_retained_owner(void* captured_owner) {
    for (auto* owner : retained_owners_) {
        if (owner->identity != captured_owner) continue;
        if (owner->references_04.fetch_sub(1, std::memory_order_seq_cst) == 1) {
            const auto invoke = owner->destroy_on_zero;
            invoke(captured_owner); // no owner/binding access after its terminal callback
        }
        return;
    }
    throw std::logic_error("node+130 has no actual retained-owner binding");
}

void unlink_native_node_child_00b6d940(CameraTransform& parent, CameraTransform& child) noexcept {
    if (child.parent != &parent) return;
    auto* previous = child.previous_sibling;
    child.parent = nullptr;
    if (previous) previous->next_sibling = child.next_sibling;
    if (child.next_sibling) child.next_sibling->previous_sibling = child.previous_sibling;
    if (parent.first_child == &child) parent.first_child = parent.first_child->next_sibling;
    --parent.child_count;
}
void prepend_native_node_root_00b721f0(RenderNodeRootList& root, CameraTransform& node) noexcept {
    node.next_sibling = root.first;
    node.previous_sibling = nullptr;
    if (root.first) root.first->previous_sibling = &node;
    root.first = &node;
}
void unregister_native_node_attachments_00b6d850(NativeNodeDestructionRuntime& runtime,
    CameraTransform& node) {
    unregister_current_attachment(runtime, node);
    auto* child = node.first_child;
    while (child) {
        unregister_native_node_attachments_00b6d850(runtime, *child);
        child = child->next_sibling;
    }
}
void propagate_native_node_root_00b6d890(NativeNodeDestructionRuntime& runtime,
    CameraTransform& node, RenderNodeRootList* requested_root) {
    if (node.root_list == requested_root && node.parent) return;
    if (node.root_list && !node.parent) unlink_render_root_node_00b72220(*node.root_list, node);
    node.root_list = requested_root;
    if (requested_root) {
        if (!node.parent) {
            prepend_native_node_root_00b721f0(*requested_root, node);
            auto& binding = runtime.scenes.resolve(node);
            if (!binding.scene) {
                const auto invoke = binding.attach_scene; // native captures vtable+50 before B72110
                SceneResource* scene = node.root_list->scene_resource_1c;
                invoke(runtime.scenes, binding, scene, false);
            }
        } else {
            auto& binding = runtime.scenes.resolve(node);
            if (!binding.scene) {
                const auto invoke = binding.attach_scene;
                SceneResource* scene = runtime.scenes.resolve(*node.parent).scene;
                invoke(runtime.scenes, binding, scene, false);
            }
        }
    }
    auto* child = node.first_child;
    while (child) {
        propagate_native_node_root_00b6d890(runtime, *child, requested_root);
        child = child->next_sibling;
    }
}
void set_native_node_parent_null_00b6e680(NativeNodeDestructionRuntime& runtime, CameraTransform& node) {
    auto* parent = node.parent;
    if (!parent) return;
    unlink_native_node_child_00b6d940(*parent, node);
    unregister_current_attachment(runtime, node);
    auto* previous_root = node.root_list;
    node.parent = nullptr;
    node.root_list = nullptr;
    node.notification_context = nullptr;
    node.notify_changed = nullptr;
    propagate_native_node_root_00b6d890(runtime, node, previous_root);
    unregister_native_node_attachments_00b6d850(runtime, node);
    if (node.valid_flags & 2u) {
        node.auxiliary_flags &= 0xffffffcfu;
        node.valid_flags &= 0xfffffff5u;
        if (node.first_child) invalidate_camera_descendants_00b6da30(node);
    }
    auto& binding = runtime.scenes.resolve(node);
    const auto invoke = binding.world_changed;
    if (!invoke) throw std::logic_error("node virtual+40 has no actual binding");
    invoke(runtime.scenes, binding);
}
void native_node_world_changed_00b6dbe0(SceneAttachmentRuntime&, SceneNodeAttachment& binding) {
    auto& node = binding.transform;
    node.auxiliary_flags &= 0xffffffcfu;
    if (void* attachment = node.notification_context) {
        const auto invoke = node.notify_changed;
        if (!invoke) throw std::logic_error("node+A0 virtual+3C has no actual binding");
        invoke(attachment);
    }
}
void remove_native_node_scene_00b6ee10(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& node, SceneResource* expected_scene, bool recurse) {
    if (node.scene == expected_scene) {
        if (node.scene) remove_scene_node_if_type_00b83ec0(runtime, *node.scene, node);
        SceneResource* current = node.scene; // reload after type/registry callback
        if (current) {
            node.scene = nullptr;
            if (current->references.fetch_sub(1, std::memory_order_seq_cst) == 1)
                current->destroy_on_zero(*current);
        }
    }
    if (recurse) {
        auto* child = node.transform.first_child;
        while (child) {
            auto& binding = runtime.resolve(*child);
            const auto invoke = binding.remove_scene;
            if (!invoke) throw std::logic_error("child virtual+54 has no actual binding");
            invoke(runtime, binding, expected_scene, true);
            child = child->next_sibling;
        }
    }
}
void destroy_native_node_00b6f440(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding,
    NativeStringStorage& strings) {
    if (&runtime.scenes.resolve(binding.transform) != &binding.scene_attachment)
        throw std::logic_error("native node destructor requires its existing scene dispatch binding");
    auto& node = binding.storage;
    auto& transform = binding.transform;
    node.vtable_00 = 0x00d62c88u;
    binding.scene_attachment.is_type = runtime.node_virtual_0c;
    binding.scene_attachment.set_world_matrix = set_native_node_world_matrix_00b6e870;
    binding.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
    binding.scene_attachment.attach_scene = set_node_scene_00b6ed80;
    binding.scene_attachment.remove_scene = remove_native_node_scene_00b6ee10;
    try {
        unregister_current_attachment(runtime, transform);
        set_native_node_parent_null_00b6e680(runtime, transform);
        // A reentrant current virtual40 may rebuild a parent/root. Preserve the
        // native asymmetric condition and reload both fields after that call.
        if (transform.root_list || !transform.parent) {
            if (transform.root_list && !transform.parent)
                unlink_render_root_node_00b72220(*transform.root_list, transform);
            auto* child = transform.first_child;
            transform.root_list = nullptr;
            while (child) {
                propagate_native_node_root_00b6d890(runtime, *child, nullptr);
                child = child->next_sibling;
            }
        }
        if (void* owner = node.retained_130) {
            runtime.release_retained_owner(owner);
            node.retained_130 = nullptr;
        }
        SceneResource* scene = node.scene_170;
        node.retained_130 = nullptr; // second native clear precedes B6EE10
        remove_native_node_scene_00b6ee10(runtime.scenes, binding.scene_attachment, scene, true);
    } catch (...) {
        // CC1A01 / DFA900 / DFA8E8 state 2 ->1 ->0: B6F3E0 array,
        // 41DD20 name, AA6E10/BD30F0 reference base. No physical slot return.
        destroy_point_light_array(node);
        destroy_name(strings, node);
        destroy_reference_base(node);
        throw;
    }
    // These three recovered cleanup operations cannot throw in this interface.
    destroy_point_light_array(node);
    destroy_name(strings, node);
    destroy_reference_base(node);
}

void destroy_native_node_00b6f440(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding) {
    PooledStringStorage strings(runtime.strings);
    destroy_native_node_00b6f440(runtime, binding, strings);
}

} // namespace bsp
