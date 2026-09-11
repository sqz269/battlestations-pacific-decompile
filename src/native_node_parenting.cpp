#include "bsp/native_node_parenting.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void invalidate_and_notify(NativeNodeDestructionRuntime& runtime, CameraTransform& node) {
    if (node.valid_flags & 2u) {
        node.auxiliary_flags &= 0xffffffcfu;
        node.valid_flags &= 0xfffffff5u;
        if (node.first_child) invalidate_camera_descendants_00b6da30(node);
    }
    auto& binding = runtime.scenes.resolve(node);
    const auto invoke = binding.world_changed;
    if (!invoke) throw std::logic_error("native reparent requires current node virtual40");
    invoke(runtime.scenes, binding);
}
void unregister_current(NativeNodeDestructionRuntime& runtime, CameraTransform& node) {
    if (void* current = node.notification_context) {
        unregister_generated_model_attachment_00b8f4c0(runtime.attachments.attachment(current), node);
        node.notification_context = nullptr;
        node.notify_changed = nullptr;
    }
}
void publish_attachment(NativeNodeParentingRuntime& runtime, CameraTransform& node, void* group) {
    if (group && !runtime.notify_attached_group_virtual3c)
        throw std::logic_error("attached group requires actual virtual3C notification");
    node.notification_context = group;
    node.notify_changed = group ? runtime.notify_attached_group_virtual3c : nullptr;
}
void invoke_attachment(NativeNodeParentingRuntime& runtime, CameraTransform& node, void* group) {
    const auto invoke = runtime.set_attachment_virtual1c;
    if (!invoke) throw std::logic_error("native reparent requires current node virtual1C");
    invoke(runtime, node, group);
}
}

void prepend_native_node_child_00b6e010(NativeNodeDestructionRuntime& runtime,
    CameraTransform& parent, CameraTransform& child) {
    if (child.parent == &parent) return;
    child.parent = &parent;
    propagate_native_node_root_00b6d890(runtime, child, parent.root_list);
    auto& binding = runtime.scenes.resolve(child);
    if (!binding.scene) {
        const auto invoke = binding.attach_scene;
        if (!invoke) throw std::logic_error("native child prepend requires current virtual50");
        SceneResource* scene = runtime.scenes.resolve(parent).scene;
        invoke(runtime.scenes, binding, scene, true);
    }
    child.next_sibling = parent.first_child;
    child.previous_sibling = nullptr;
    if (parent.first_child) parent.first_child->previous_sibling = &child;
    ++parent.child_count;
    parent.first_child = &child;
    invalidate_and_notify(runtime, child);
}

void set_native_node_parent_00b6e680(NativeNodeParentingRuntime& runtime,
    CameraTransform& child, CameraTransform* requested_parent) {
    if (child.parent == requested_parent) return;
    if (!requested_parent) {
        set_native_node_parent_null_00b6e680(runtime.nodes, child);
        return;
    }
    if (child.parent) {
        unlink_native_node_child_00b6d940(*child.parent, child);
        unregister_current(runtime.nodes, child);
    } else {
        propagate_native_node_root_00b6d890(runtime.nodes, child, nullptr);
    }
    auto& parent_binding = runtime.nodes.scenes.resolve(*requested_parent);
    const auto predicate = parent_binding.is_type;
    if (!predicate) throw std::logic_error("native reparent requires current parent virtual0C");
    const auto group_token = runtime.group_type_0109032c;
    const bool is_group = predicate(runtime.nodes.scenes, parent_binding, group_token);
    // The native selects the owner before B6E010 and retains that captured
    // identity through its reentrant root/scene/world-change callbacks.
    void* group = is_group
        ? reinterpret_cast<void*>(static_cast<std::uintptr_t>(parent_binding.pointer_key))
        : requested_parent->notification_context;
    prepend_native_node_child_00b6e010(runtime.nodes, *requested_parent, child);
    if (group) invoke_attachment(runtime, child, group);
    else unregister_native_node_attachments_00b6d850(runtime.nodes, child);
    invalidate_and_notify(runtime.nodes, child);
}

void register_native_node_attachment_00b8f460(NativeNodeParentingRuntime& runtime,
    GeneratedModelAttachmentLinks& group, CameraTransform& node) {
    if (node.notification_context == group.identity) return;
    append_generated_model_attachment(group, node);
    publish_attachment(runtime, node, group.identity);
}
void set_native_node_attachment_00b6d7b0(NativeNodeParentingRuntime& runtime,
    CameraTransform& node, void* group) {
    unregister_current(runtime.nodes, node);
    publish_attachment(runtime, node, group);
    if (!group) return;
    register_native_node_attachment_00b8f460(runtime, runtime.nodes.attachments.attachment(group), node);
    auto* child = node.first_child.get();
    while (child) {
        invoke_attachment(runtime, *child, group);
        child = child->next_sibling;
    }
}
void set_native_group_attachment_00b8f4f0(NativeNodeParentingRuntime& runtime,
    CameraTransform& node, void* group) {
    unregister_current(runtime.nodes, node);
    publish_attachment(runtime, node, group);
    if (group)
        register_native_node_attachment_00b8f460(runtime, runtime.nodes.attachments.attachment(group), node);
}
} // namespace bsp
