#include "bsp/gui_native_scene.hpp"
#include <stdexcept>

namespace bsp {
namespace {
GuiNativeScene* process_notification_binding;
const volatile std::uint32_t* current_model_table(NativeModelOwner& owner) {
    if (owner.storage.node.vtable_00 == 0x00d62de8u) return owner.environment.vtable_00d62de8;
    if (owner.storage.node.vtable_00 == 0x00d62c88u) return owner.environment.vtable_00d62c88;
    throw std::logic_error("GUI model dispatch requires its current actual table");
}
const volatile std::uint32_t* current_group_table(NativeGroupOwner& owner) {
    if (owner.storage.node.vtable_00 == 0x00d634f8u) return owner.environment.vtable_00d634f8;
    if (owner.storage.node.vtable_00 == 0x00d62c88u) return owner.environment.vtable_00d62c88;
    throw std::logic_error("GUI group dispatch requires its current actual table");
}
}
struct GuiNativeScene::GroupRecord {
    std::unique_ptr<NativeGroupOwner> owner;
    std::unique_ptr<NativeGroupReference> reference;
};

GuiNativeScene::GuiNativeScene(NativeGroupEnvironment& environment)
    : environment_(environment), parenting_{environment.nodes,
        environment.types.storage().group_0109032c.own_id, set_attachment, notify_group} {
    if (process_notification_binding)
        throw std::logic_error("GUI native notification runtime already has a process binding");
    process_notification_binding = this;
}
GuiNativeScene::~GuiNativeScene() {
    // Page owners perform actual logical release. Queued references can keep
    // these companions alive afterward; all must drain before this binding dies.
    if (!groups_.empty() || process_notification_binding != this) std::terminate();
    process_notification_binding = nullptr;
}
NativeNodeBinding& GuiNativeScene::resolve_node(CameraTransform& node) {
    auto& reference = environment_.nodes.attachments.resolve(node);
    if (auto* model = dynamic_cast<NativeModelReference*>(&reference))
        return model->model_owner().node;
    if (auto* group = dynamic_cast<NativeGroupReference*>(&reference))
        return group->group_owner().node;
    throw std::logic_error("GUI native node has no supported actual model/group companion");
}
void GuiNativeScene::set_parent_00b6e680(NativeNodeBinding& child, NativeNodeBinding* parent) {
    if (&resolve_node(child.transform) != &child ||
        (parent && &resolve_node(parent->transform) != parent))
        throw std::invalid_argument("GUI parenting requires the canonical node companions");
    set_native_node_parent_00b6e680(parenting_, child.transform,
        parent ? &parent->transform : nullptr);
}
void GuiNativeScene::set_attachment(NativeNodeParentingRuntime& runtime,
    CameraTransform& node, void* group) {
    auto& reference = runtime.nodes.attachments.resolve(node);
    if (auto* model = dynamic_cast<NativeModelReference*>(&reference)) {
        if (current_model_table(model->model_owner())[0x1c / 4] != 0x00b6d7b0u)
            throw std::logic_error("GUI model current virtual1C is unsupported");
        set_native_node_attachment_00b6d7b0(runtime, node, group);
        return;
    }
    if (auto* retained = dynamic_cast<NativeGroupReference*>(&reference)) {
        auto& owner = retained->group_owner();
        const auto target = current_group_table(owner)[0x1c / 4];
        if (target == 0x00b8f4f0u) native_group_attachment_virtual1c(owner, runtime, group);
        else if (target == 0x00b6d7b0u) set_native_node_attachment_00b6d7b0(runtime, node, group);
        else throw std::logic_error("GUI group current virtual1C is unsupported");
        return;
    }
    throw std::logic_error("GUI attachment has no actual model/group virtual1C binding");
}
void GuiNativeScene::notify_group(void* identity) {
    if (!process_notification_binding || !identity)
        throw std::logic_error("GUI group notification has no live process binding");
    const auto key = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(identity));
    auto* retained = process_notification_binding->environment_.nodes.attachments.find_actual_node(key);
    auto* reference = dynamic_cast<NativeGroupReference*>(retained);
    if (!reference)
        throw std::logic_error("GUI group notification requires an actual group companion");
    auto& group = reference->group_owner();
    if (&group.storage.node != identity)
        throw std::logic_error("GUI group notification identity is not canonical");
    notify_native_group_bounds_00b6dbc0(group);
}
NativeGroupReference& GuiNativeScene::create_group(const std::string& name) {
    auto record = std::make_unique<GroupRecord>();
    void* slot = environment_.pool_010902f4.allocate_raw_slot_00b8f310();
    if (!slot) throw std::runtime_error("GUI plain page root allocation returned null");
    // An invalid allocator result must not return another live owner's slot.
    if (groups_.count(slot))
        throw std::logic_error("group pool returned an occupied GUI root slot");
    NativeString native_name;
    PooledStringStorage strings(environment_.nodes.strings);
    bool inserted = false;
    try {
        groups_.emplace(slot, nullptr);
        inserted = true;
        record->owner = std::make_unique<NativeGroupOwner>(slot, NativeGroupPool::slot_bytes, environment_);
        native_name.assign_0041e870(strings, name.c_str());
        construct_native_group_00b8f5e0(*record->owner, native_name);
        native_name.release_to(strings);
        groups_.at(slot) = std::move(record);
        auto& current = *groups_.at(slot);
        try {
            current.reference = std::make_unique<NativeGroupReference>(*current.owner,
                NativeGroupCompanionDisposal{this, retire_group});
        } catch (...) {
            destroy_native_group_00b8f680(*current.owner);
            groups_.erase(slot);
            throw;
        }
        return *current.reference;
    } catch (...) {
        native_name.release_to(strings);
        if (record && record->owner && record->owner->phase == NativeGroupOwner::Phase::live)
            destroy_native_group_00b8f680(*record->owner);
        record.reset();
        if (inserted) groups_.erase(slot);
        environment_.pool_010902f4.return_raw_slot_00b8ed40(slot);
        throw;
    }
}
GuiWidgetOwner& GuiNativeScene::create_plain_page_root_00aa5840_fragment(
    GuiWidgetOwnerRuntime& widgets, GuiLayoutWidget& layout) {
    if (&widgets.environment().models.nodes != &environment_.nodes ||
        &widgets.environment().native != this || layout.type != GuiWidgetType::Screen)
        throw std::invalid_argument("GUI page root requires one canonical node runtime and a Screen layout");
    auto& reference = create_group(layout.key);
    try { return widgets.construct_root(layout, reference.group_owner().node); }
    catch (...) { unlink_and_release_render_model_00b6dfa0(reference); throw; }
}
void GuiNativeScene::clear_page_root_registration_00b6d890(GuiWidgetOwner& widget) {
    auto* binding = widget.node_binding();
    if (!binding || &resolve_node(binding->transform) != binding)
        throw std::logic_error("GUI page completion requires its actual root binding");
    propagate_native_node_root_00b6d890(environment_.nodes, binding->transform, nullptr);
}
void GuiNativeScene::retire_group(void* context, NativeGroupReference& reference) noexcept {
    auto& scene = *static_cast<GuiNativeScene*>(context);
    scene.groups_.erase(&reference.group_owner().storage.node);
}
} // namespace bsp
