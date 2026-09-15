#include "bsp/native_resource_graph_builtin_dispatch.hpp"
#include "bsp/native_resource_builtin_factories.hpp"
#include "bsp/native_plain_node.hpp"
#include "bsp/point_effect_matrix_setters.hpp"
#include <exception>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

namespace bsp {
struct NativeResourceGraphBuiltinDispatch::Record {
    explicit Record(NativeResourceGraphBuiltinDispatch& dispatch) noexcept : dispatch(dispatch) {}
    NativeResourceGraphBuiltinDispatch& dispatch;
    std::list<std::unique_ptr<Record>>::iterator position;
    NativeNodeStorage* native{};
    std::optional<NativeNodeBinding> node;
    std::optional<NativeGroupOwner> group;
    std::optional<NativePlainNodeReference> plain_reference;
    std::optional<NativeModelBaseReference> model_reference;
    std::optional<NativeGroupReference> group_reference;
    static void retire_plain(void* context, NativePlainNodeReference&) noexcept {
        auto& record = *static_cast<Record*>(context);record.dispatch.retire(record);
    }
    static void retire_model(void* context, NativeModelBaseReference&) noexcept {
        auto& record = *static_cast<Record*>(context);record.dispatch.retire(record);
    }
    static void retire_group(void* context, NativeGroupReference&) noexcept {
        auto& record = *static_cast<Record*>(context);record.dispatch.retire(record);
    }
};
NativeResourceGraphBuiltinDispatch::NativeResourceGraphBuiltinDispatch(
    NativeResourceGraphBuiltinEnvironment environment, NativeResourceGraphCalls& external)
    : environment_(environment), external_(external),
      raw_names_(environment.groups.nodes.require_raw_name_pool()),
      graph_names_(raw_names_.actual_published_01090aa8,
          raw_names_.actual_small_returns_disabled_01090aa4, raw_names_.actual_manager_publication_01090aa0) {
    if (!environment.actual_plain_pool_0108ff58 || !environment.actual_model_base_pool_0109008c ||
        environment.actual_plain_pool_0108ff58 == environment.actual_model_base_pool_0109008c ||
        !environment.plain_factory_00d63210 || !environment.model_factory_00d63218 ||
        !environment.group_factory_00d63220 || !environment.model_base_00d62d78 ||
        !environment.groups.vtable_00d62c88 || !environment.groups.vtable_00d634f8)
        throw std::invalid_argument("resource builtins require distinct actual pools and current profiles");
    bind_static_native_group_pool_010902f4(environment.groups.pool_010902f4);
}
NativeResourceGraphBuiltinDispatch::~NativeResourceGraphBuiltinDispatch() {
    // The owning resources must finish native logical/reference release first.
    // A provider destructor is not a substitute graph destructor.
    if (!records_.empty()) std::terminate();
}
void NativeResourceGraphBuiltinDispatch::retire(Record& record) noexcept {
    records_.erase(record.position); // native terminal callback makes no later access
}
std::size_t NativeResourceGraphBuiltinDispatch::live_owned_nodes() const noexcept {
    std::size_t count = 0;
    for (const auto& record : records_)
        if (record->plain_reference || record->model_reference || record->group_reference) ++count;
    return count;
}
const volatile std::uint32_t* NativeResourceGraphBuiltinDispatch::table(std::uint32_t profile) noexcept {
    switch (profile) {
    case 0x00d63210:return environment_.plain_factory_00d63210;
    case 0x00d63218:return environment_.model_factory_00d63218;
    case 0x00d63220:return environment_.group_factory_00d63220;
    case 0x00d62c88:return environment_.groups.vtable_00d62c88;
    case 0x00d62d78:return environment_.model_base_00d62d78;
    case 0x00d634f8:return environment_.groups.vtable_00d634f8;
    default:return external_.table(profile);
    }
}
void* NativeResourceGraphBuiltinDispatch::create_instance(std::uint32_t target, void* resource) {
    return external_.create_instance(target, resource);
}
void* NativeResourceGraphBuiltinDispatch::item_factory(std::uint32_t target, void* item) {
    return external_.item_factory(target, item);
}
void NativeResourceGraphBuiltinDispatch::attach_item(std::uint32_t target, void* item,
    void* instance, void* record, void* node, std::uint32_t word) {
    external_.attach_item(target, item, instance, record, node, word);
}
void* NativeResourceGraphBuiltinDispatch::create_node(std::uint32_t target, void* factory, const void* name) {
    if (target != 0x00b866c0 && target != 0x00b86720 && target != 0x00b86780)
        return external_.create_node(target, factory, name);
    auto& nodes = environment_.groups.nodes;
    auto record = std::make_unique<Record>(*this);
    auto scene_admission = nodes.scenes.reserve_binding();
    auto lifetime_admission = nodes.attachments.reserve_binding();
    auto attachment_admission = target == 0x00b86780
        ? nodes.attachments.reserve_attachment_binding()
        : GeneratedModelLifetimeRuntime::BindingAdmission{};
    records_.push_back(std::move(record));
    auto position = std::prev(records_.end());auto& owner = **position;owner.position = position;
    try {
        if (target == 0x00b866c0)
            owner.native = create_native_plain_node_00b866c0(environment_.actual_plain_pool_0108ff58,
                name, raw_names_, environment_.constants);
        else if (target == 0x00b86720)
            owner.native = create_native_model_base_00b86720(environment_.actual_model_base_pool_0109008c,
                name, raw_names_, environment_.constants);
        else
            owner.native = create_native_resource_group_00b86780(name, raw_names_, environment_.constants);
    } catch (...) {
        records_.erase(position); // complete native factory already handled its failed slot
        throw;
    }
    if (!owner.native) {records_.erase(position);return nullptr;}
    // Every allocation for companions and admission happened before the native
    // factory. Valid fresh owners/current canonical profiles cannot fail below.
    // Profile corruption or duplicate native identities are adapter-contract
    // violations, not new recoverable native constructor failures.
    try {
        if (target == 0x00b86780) {
            auto& tail = *reinterpret_cast<NativeGroupTailStorage*>(
                reinterpret_cast<std::byte*>(owner.native) + 0x174);
            owner.group.emplace(NativeGroupStorageView{*owner.native, tail}, environment_.groups,
                std::move(scene_admission), std::move(attachment_admission));
            // No callbacks or allocations intervene between returning this
            // credit and the reference constructor's existing bind operation.
            lifetime_admission.cancel();
            owner.group_reference.emplace(*owner.group,
                NativeGroupCompanionDisposal{&owner, Record::retire_group});
        } else {
            owner.node.emplace(*owner.native, nodes.node_virtual_0c, set_node_scene_00b6ed80, nullptr);
            owner.node->scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
            nodes.scenes.bind(owner.node->scene_attachment, std::move(scene_admission));
            lifetime_admission.cancel();
            if (target == 0x00b866c0)
                owner.plain_reference.emplace(*owner.node, nodes, raw_names_, environment_.actual_plain_pool_0108ff58,
                    environment_.groups.vtable_00d62c88, NativePlainNodeCompanionDisposal{&owner, Record::retire_plain});
            else
                owner.model_reference.emplace(*owner.node, nodes, raw_names_, environment_.actual_model_base_pool_0109008c,
                    environment_.model_base_00d62d78, environment_.model_base_types,
                    NativeModelBaseCompanionDisposal{&owner, Record::retire_model});
        }
    } catch (...) {std::terminate();}
    return owner.native;
}
NativeNodeBinding& NativeResourceGraphBuiltinDispatch::node_binding(void* node) noexcept {
    auto* reference = environment_.groups.nodes.attachments.find_actual_node(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node)));
    if (auto* plain = dynamic_cast<NativePlainNodeReference*>(reference)) return plain->node_binding();
    if (auto* model = dynamic_cast<NativeModelBaseReference*>(reference)) return model->node_binding();
    if (auto* group = dynamic_cast<NativeGroupReference*>(reference)) return group->group_owner().node;
    return external_.node_binding(node);
}
NativeGroupOwner& NativeResourceGraphBuiltinDispatch::group_owner(void* node) noexcept {
    auto* reference = environment_.groups.nodes.attachments.find_actual_node(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node)));
    if (auto* group = dynamic_cast<NativeGroupReference*>(reference)) return group->group_owner();
    return external_.group_owner(node);
}
void NativeResourceGraphBuiltinDispatch::set_matrix(std::uint32_t target, void* node, const void* matrix) {
    if (target == 0x00b6db10)
        set_transform_local_matrix_00b6db10(node_binding(node).transform, *static_cast<const CameraMatrix*>(matrix));
    else external_.set_matrix(target, node, matrix);
}
std::uint8_t NativeResourceGraphBuiltinDispatch::is_type(std::uint32_t target, void* node, std::uint32_t token) {
    switch (target) {
    case 0x00b6f570:
        return static_cast<std::uint8_t>(environment_.groups.nodes.node_virtual_0c(
            environment_.groups.nodes.scenes, node_binding(node).scene_attachment, token));
    case 0x00b743e0:return static_cast<std::uint8_t>(native_model_base_is_type_00b743e0(environment_.model_base_types, token));
    case 0x00b8f650:return static_cast<std::uint8_t>(environment_.groups.types.is_type_00b8f650(token));
    default:return external_.is_type(target, node, token);
    }
}
} // namespace bsp
