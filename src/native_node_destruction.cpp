#include "bsp/native_node_destruction.hpp"
#include "bsp/point_effect_matrix_setters.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string_pool_storage.hpp"
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
void destroy_point_light_array(NativeNodeDestructionRuntime& runtime, NativeNodeStorage& node) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(node.point_lights_164);
    runtime.point_lights.free_backing(node.point_lights_164.begin);
    // Native leaves the freed pointer and capacity words unchanged.
}
void destroy_name(NativeStringStorage& strings, NativeNodeStorage& node) noexcept {
    destroy_native_string_header_0041dd20(&node.name_54, strings);
}
void destroy_name(NativeStringRawPoolContext& strings, NativeNodeStorage& node) {
    destroy_native_string_header_0041dd20(&node.name_54, strings);
}
void* current_name_data(NativeNodeStorage& node) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        reinterpret_cast<unsigned char*>(&node.name_54) + 4);
}
std::uint32_t current_name_size(NativeNodeStorage& node) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(&node.name_54) + 1u;
}
void return_captured_name(NativeStringStorage& strings, NativeNodeStorage& node,
    void* captured_data) noexcept {
    strings.release(static_cast<char*>(captured_data), current_name_size(node));
}
void return_captured_name(NativeStringRawPoolContext& strings, NativeNodeStorage& node,
    void* captured_data) {
    const auto size = current_name_size(node); // B6F52C/B6F531, before getter.
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, captured_data, size,
        strings.actual_small_returns_disabled_01090aa4);
}
void destroy_reference_base(NativeNodeStorage& node) noexcept {
    *static_cast<volatile std::uint32_t*>(&node.vtable_00) = 0x00d5c104u;
    destroy_native_ref_counted_base_00bd30f0(&node); // No count modification.
    node.~NativeNodeStorage();
}
}

NativeNodeDestructionRuntime::NativeNodeDestructionRuntime(SceneAttachmentRuntime& scene_runtime,
    GeneratedModelLifetimeRuntime& actual_attachments, SizedStoragePool& string_pool,
    SceneTypePredicate actual_node_virtual_0c)
    : scenes(scene_runtime), attachments(actual_attachments), node_virtual_0c(actual_node_virtual_0c),
      semantic_name_pool_(&string_pool), raw_name_pool_(nullptr) {
    if (&attachments.scenes != &scenes || !node_virtual_0c)
        throw std::invalid_argument("native node destruction requires one scene runtime and actual node type predicate");
}
NativeNodeDestructionRuntime::NativeNodeDestructionRuntime(SceneAttachmentRuntime& scene_runtime,
    GeneratedModelLifetimeRuntime& actual_attachments, NativeStringRawPoolContext& string_pool,
    SceneTypePredicate actual_node_virtual_0c)
    : scenes(scene_runtime), attachments(actual_attachments), node_virtual_0c(actual_node_virtual_0c),
      semantic_name_pool_(nullptr), raw_name_pool_(&string_pool) {
    if (&attachments.scenes != &scenes || !node_virtual_0c)
        throw std::invalid_argument("native node destruction requires one scene runtime and actual node type predicate");
}
SizedStoragePool& NativeNodeDestructionRuntime::require_semantic_name_pool() const {
    if (!semantic_name_pool_) throw std::logic_error("native node runtime requires semantic name domain");
    return *semantic_name_pool_;
}
NativeStringRawPoolContext& NativeNodeDestructionRuntime::require_raw_name_pool() const {
    if (!raw_name_pool_) throw std::logic_error("native node runtime requires raw name domain");
    return *raw_name_pool_;
}
bool NativeNodeDestructionRuntime::uses_raw_name_pool() const noexcept { return raw_name_pool_ != nullptr; }
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
    auto* previous = child.previous_sibling.get();
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
    auto* child = node.first_child.get();
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
    auto* child = node.first_child.get();
    while (child) {
        propagate_native_node_root_00b6d890(runtime, *child, requested_root);
        child = child->next_sibling;
    }
}
void set_native_node_parent_null_00b6e680(NativeNodeDestructionRuntime& runtime, CameraTransform& node) {
    auto* parent = node.parent.get();
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
        auto* child = node.transform.first_child.get();
        while (child) {
            auto& binding = runtime.resolve(*child);
            const auto invoke = binding.remove_scene;
            if (!invoke) throw std::logic_error("child virtual+54 has no actual binding");
            invoke(runtime, binding, expected_scene, true);
            child = child->next_sibling;
        }
    }
}
namespace {
template<class Strings>
void destroy_node(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding,
    Strings& strings) {
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
    int state = 2;
    try {
        unregister_current_attachment(runtime, transform);
        set_native_node_parent_null_00b6e680(runtime, transform);
        // A reentrant current virtual40 may rebuild a parent/root. Preserve the
        // native asymmetric condition and reload both fields after that call.
        if (transform.root_list || !transform.parent) {
            if (transform.root_list && !transform.parent)
                unlink_render_root_node_00b72220(*transform.root_list, transform);
            auto* child = transform.first_child.get();
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
        state = 1; // B6F50C consumes array cleanup BEFORE its resize/free.
        destroy_point_light_array(runtime, node);
        void* const captured_data = current_name_data(node); // B6F51E.
        state = 0; // B6F526 consumes name cleanup BEFORE getter/return.
        if (captured_data) return_captured_name(strings, node, captured_data);
        state = -1; // B6F544 consumes base cleanup before both profile stores.
        destroy_reference_base(node);
    } catch (...) {
        // CC1A01 / DFA900 / DFA8E8. Consume each reached stage before invoking
        // it; a normal name-getter failure therefore reaches ONLY state0.
        // A second cleanup exception terminates this source projection;
        // original FH3 second-exception search ordering is not certified.
        try {
            while (state >= 0) {
                const int current = state--;
                if (current == 2) destroy_point_light_array(runtime, node);
                else if (current == 1) destroy_name(strings, node);
                else destroy_reference_base(node);
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace

void destroy_native_node_00b6f440(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding,
    NativeStringStorage& strings) { destroy_node(runtime, binding, strings); }

void destroy_native_node_00b6f440(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding,
    NativeStringRawPoolContext& strings) { destroy_node(runtime, binding, strings); }

void destroy_native_node_00b6f440(NativeNodeDestructionRuntime& runtime, NativeNodeBinding& binding) {
    PooledStringStorage strings(runtime.require_semantic_name_pool());
    destroy_native_node_00b6f440(runtime, binding, strings);
}

} // namespace bsp
