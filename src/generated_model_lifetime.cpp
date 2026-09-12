#include "bsp/generated_model_lifetime.hpp"
#include <algorithm>
#include <exception>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
template<class Reference>
void release_then_clear(Reference*& field) noexcept {
    Reference* old = field;
    if (old) {
        release_render_command_reference(*old);
        field = nullptr;
    }
}
void unregister_current_attachment(GeneratedModelLifetime& model) noexcept {
    CameraTransform& node = model.transform();
    if (void* attached = node.notification_context) {
        unregister_generated_model_attachment_00b8f4c0(model.runtime.attachment(attached), node);
        node.notification_context = nullptr;
        node.notify_changed = nullptr;
    }
}
bool cleared_terminal_hierarchy(const CameraTransform& node) noexcept {
    return !node.parent && !node.root_list && !node.first_child;
}
using DiagnosticPointLights = std::vector<GeneratedModelPointLightLinks*>;
std::uint32_t diagnostic_light_count(void* context) noexcept {
    return static_cast<std::uint32_t>(static_cast<DiagnosticPointLights*>(context)->size());
}
void remove_diagnostic_light_backlink(void* context,
    std::uint32_t index, CameraTransform& node) noexcept {
    remove_point_light_model_link_00b7c1a0(
        *(*static_cast<DiagnosticPointLights*>(context))[index], node);
}
void shrink_diagnostic_lights_to_zero(void* context) noexcept {
    resize_generated_model_point_lights_00b6ec70(*static_cast<DiagnosticPointLights*>(context), 0);
}
}

void GeneratedModelLifetimeRuntime::bind(GeneratedModelNodeLifetime& node) {
    if (&node.scene_attachment().transform != &node.transform())
        throw std::invalid_argument("Generated model scene and hierarchy identities differ");
    for (auto* existing : nodes_)
        if (&existing->transform() == &node.transform())
            throw std::invalid_argument("Duplicate generated model lifetime binding");
    nodes_.push_back(&node);
}
void GeneratedModelLifetimeRuntime::unbind(GeneratedModelNodeLifetime& node) noexcept {
    const auto found = std::find(nodes_.begin(), nodes_.end(), &node);
    if (found == nodes_.end()) std::terminate();
    nodes_.erase(found);
}
GeneratedModelNodeLifetime& GeneratedModelLifetimeRuntime::resolve(CameraTransform& node) const noexcept {
    for (auto* lifetime : nodes_)
        if (&lifetime->transform() == &node) return *lifetime;
    std::terminate();
}
GeneratedModelNodeLifetime* GeneratedModelLifetimeRuntime::find_actual_node(std::uint32_t key) const noexcept {
    for (auto* lifetime : nodes_)
        if (lifetime->scene_attachment().pointer_key == key) return lifetime;
    return nullptr;
}
void GeneratedModelLifetimeRuntime::bind_attachment(GeneratedModelAttachmentLinks& attachment) {
    if (!attachment.identity) throw std::invalid_argument("Null generated model attachment identity");
    const auto& view = attachment.native_array;
    if ((view.context || view.append || view.erase) &&
        (!view.context || !view.append || !view.erase || !attachment.models.empty()))
        throw std::invalid_argument("Actual attachment array requires complete callbacks and no copied vector");
    for (auto* existing : attachments_)
        if (existing->identity == attachment.identity)
            throw std::invalid_argument("Duplicate generated model attachment binding");
    attachments_.push_back(&attachment);
}
void GeneratedModelLifetimeRuntime::unbind_attachment(GeneratedModelAttachmentLinks& attachment) noexcept {
    const auto found = std::find(attachments_.begin(), attachments_.end(), &attachment);
    if (found == attachments_.end()) std::terminate();
    attachments_.erase(found);
}
GeneratedModelAttachmentLinks& GeneratedModelLifetimeRuntime::attachment(void* identity) const noexcept {
    for (auto* binding : attachments_)
        if (binding->identity == identity) return *binding;
    std::terminate();
}

GeneratedModelGeometryReference::GeneratedModelGeometryReference(
    std::shared_ptr<GeneratedInstanceGeometry> owner) : geometry(std::move(owner)) {
    if (!geometry) throw std::invalid_argument("Generated model requires its actual geometry owner");
}
void GeneratedModelGeometryReference::release_zero_references() noexcept { delete this; }

GeneratedModelLifetime::GeneratedModelLifetime(GeneratedModelLifetimeRuntime& lifetime_runtime,
    SceneNodeAttachment& attachment, GeneratedModelConstructionState initial,
    GeneratedModelStorageOwner& owner)
    : runtime(lifetime_runtime), attachment_state(attachment), storage_owner(owner),
      released_byte_44(initial.released_byte_44), retained_174(initial.retained_174),
      geometry_180(initial.geometry_180), retained_130(initial.retained_130),
      point_lights_164(std::move(initial.point_lights_164)), name_54_58(std::move(initial.name_54_58)) {
    if (initial.references <= 0)
        throw std::invalid_argument("Generated model construction requires an actual positive reference count");
    reference_count.store(initial.references, std::memory_order_relaxed);
}
void GeneratedModelLifetime::release_model_virtual18_00b6f310() noexcept { release_generated_model_00b6f310(*this); }
void GeneratedModelLifetime::remove_scene_virtual54(SceneResource* scene, bool recurse) noexcept {
    remove_generated_model_scene_00b6ee10(runtime, *this, scene, recurse);
}
void GeneratedModelLifetime::release_zero_references() noexcept {
    destroy_generated_model_after_release_00b750c0(*this);
}

bool erase_generated_model_pointer_00b7bed0(std::vector<CameraTransform*>& models,
    CameraTransform* node) noexcept {
    for (std::size_t i = 0; i < models.size(); ++i) {
        if (models[i] == node) {
            if (i != models.size() - 1) models[i] = models.back();
            models.pop_back();
            return true;
        }
    }
    return false;
}
void remove_point_light_model_link_00b7c1a0(GeneratedModelPointLightLinks& light,
    CameraTransform& node) noexcept {
    erase_generated_model_pointer_00b7bed0(light.models, &node);
}
void unregister_generated_model_attachment_00b8f4c0(GeneratedModelAttachmentLinks& owner,
    CameraTransform& node) noexcept {
    if (node.notification_context == owner.identity) {
        if (owner.native_array.context)
            owner.native_array.erase(owner.native_array.context, &node);
        else erase_generated_model_pointer_00b7bed0(owner.models, &node);
        node.notification_context = nullptr;
        node.notify_changed = nullptr;
    }
}
void append_generated_model_attachment(GeneratedModelAttachmentLinks& owner, CameraTransform& node) {
    if (owner.native_array.context) {
        owner.native_array.append(owner.native_array.context, node);
        return;
    }
    if (owner.models.size() == owner.models.capacity())
        owner.models.reserve((std::max)(std::size_t{1}, owner.models.capacity() * 2));
    owner.models.push_back(&node);
}
void reserve_generated_model_point_lights_00b6e500(
    std::vector<GeneratedModelPointLightLinks*>& links, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (static_cast<std::uint32_t>(capacity) >
        (std::numeric_limits<std::uint32_t>::max)() / sizeof(GeneratedModelPointLightLinks*))
        throw std::length_error("Generated model point-light link byte extent overflows");
    if (static_cast<std::size_t>(capacity) > links.capacity()) links.reserve(capacity);
}
void resize_generated_model_point_lights_00b6ec70(
    std::vector<GeneratedModelPointLightLinks*>& links, std::int32_t count) {
    if (count < 0) throw std::invalid_argument("Negative generated model point-light link count");
    if (static_cast<std::size_t>(count) > links.capacity())
        reserve_generated_model_point_lights_00b6e500(links, count);
    links.resize(static_cast<std::size_t>(count), nullptr);
}

void remove_generated_model_scene_00b6ee10(GeneratedModelLifetimeRuntime& runtime,
    GeneratedModelNodeLifetime& lifetime, SceneResource* expected_scene, bool recurse) noexcept {
    SceneNodeAttachment& node = lifetime.scene_attachment();
    if (node.scene == expected_scene) {
        if (node.scene) remove_scene_node_if_type_00b83ec0(runtime.scenes, *node.scene, node);
        SceneResource* current = node.scene; // reload after registry/type callback
        if (current) {
            node.scene = nullptr;
            if (current->references.fetch_sub(1, std::memory_order_seq_cst) == 1)
                current->destroy_on_zero(*current);
        }
    }
    if (recurse) {
        CameraTransform* child = node.transform.first_child;
        while (child) {
            runtime.resolve(*child).remove_scene_virtual54(expected_scene, true);
            child = child->next_sibling;
        }
    }
}

void release_node_logical_00b6f310(NodeLogicalReleaseState state) noexcept {
    CameraTransform& node = state.transform;
    std::uint32_t index = 0;
    while (index < state.point_lights.live_count(state.point_lights.context)) {
        state.point_lights.remove_live_backlink(state.point_lights.context, index, node);
        ++index;
    }
    state.point_lights.shrink_to_zero(state.point_lights.context);
    while (CameraTransform* child = node.first_child) {
        node.first_child = child->next_sibling;
        if (node.first_child) node.first_child->previous_sibling = nullptr;
        child->parent = nullptr;
        child->next_sibling = nullptr;
        state.runtime.resolve(*child).release_model_virtual18_00b6f310();
    }
    if (state.released_byte_44 == 0) {
        void* attached = node.notification_context; // native F377, before link stores
        node.root_list = nullptr;
        node.parent = nullptr;
        node.first_child = nullptr;
        node.previous_sibling = nullptr;
        node.next_sibling = nullptr;
        state.released_byte_44 = 1;
        if (attached) {
            unregister_generated_model_attachment_00b8f4c0(state.runtime.attachment(attached), node);
            node.notification_context = nullptr;
            node.notify_changed = nullptr;
        }
        release_render_command_reference(state.self); // may end backing/companion lifetimes
    }
}

void release_generated_model_00b6f310(GeneratedModelLifetime& model) noexcept {
    release_node_logical_00b6f310({model.runtime, model.transform(), model.released_byte_44,
        model, {&model.point_lights_164, diagnostic_light_count, remove_diagnostic_light_backlink,
            shrink_diagnostic_lights_to_zero}});
}

void destroy_generated_model_after_release_00b750c0(GeneratedModelLifetime& model) noexcept {
    // 00B6F310 has already cleared parent/root/children before terminal release.
    // This closes that concrete path, not arbitrary direct c3dNode destruction.
    if (!model.released_byte_44 || !cleared_terminal_hierarchy(model.transform())) std::terminate();
    release_then_clear(model.retained_174);
    release_then_clear(model.geometry_180);
    unregister_current_attachment(model);
    // Native00B6E680(0) returns immediately for the already-null parent. The
    // subsequent root/child registration block also has no work on this path.
    if (!cleared_terminal_hierarchy(model.transform())) std::terminate();
    release_then_clear(model.retained_130);
    model.retained_130 = nullptr; // second native store before scene detach
    remove_generated_model_scene_00b6ee10(model.runtime, model,
        model.attachment_state.scene, true);
    resize_generated_model_point_lights_00b6ec70(model.point_lights_164, 0);
    std::vector<GeneratedModelPointLightLinks*>().swap(model.point_lights_164);
    if (model.name_54_58) model.name_54_58->return_storage();
    // Host association teardown happens after recovered observable cleanup.
    model.runtime.scenes.unbind(model.attachment_state);
    model.runtime.unbind(model);
    model.storage_owner.dispose_model_storage(model);
}
}
