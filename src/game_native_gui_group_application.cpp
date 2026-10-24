#include "bsp/game_native_gui_group_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
const volatile std::uint32_t* table(GameNativeReadOnlyData& data, std::uint32_t address) {
    return static_cast<const volatile std::uint32_t*>(data.data_at(address, 0x58));
}
} // namespace

struct GameNativeGuiGroupApplication::Impl {
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameNativeVfsRawServices raw;
    NativeNodeParentingRuntime& parenting;
    LightTypeBootstrap node_types;
    NativeGroupTypes group_types;
    NativeGroupEnvironment environment;
    void* actual{}; // Same physical pool identity, including interrupted allocation.
    std::unique_ptr<NativeGroupOwner> owner;
    std::unique_ptr<NativeGroupReference> reference;
    NativeRenderActualOwnerRegistry::BindingAdmission admission;
    NativeString name;
    NativeModelReference* child{}; // Borrow only: no native credit or second owner.
    bool name_entered{}, name_live{}, name_release_entered{}, registered{}, retired{};
    bool active{};
    std::uint32_t failed_entry{};
    std::exception_ptr failed;

    Impl(GameHostLog& output, GameNativeRendererApplication& app, GameVfsHost& vfs,
        GameNativeReadOnlyData& data, NativeNodeParentingRuntime& same_parenting)
        : log(output), renderer(app), files(vfs), raw(vfs.borrow_raw_services()),
          parenting(same_parenting),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          environment{game_native_resource_pool_process().group_pool_010902f4(),
              app.camera_environment().nodes, group_types,
              {app.node_constants().one_00d7a24c, app.node_constants().positive_bound_00ce4970},
              table(data, 0x00d634f8), table(data, 0x00d62c88), &app.node_constants()} {
        auto& node_names = environment.nodes.require_raw_name_pool();
        auto& vfs_names = vfs.raw_strings();
        require(&parenting.nodes == &environment.nodes &&
            &parenting.group_type_0109032c == &group_types.storage().group_0109032c.own_id &&
            &node_names.actual_published_01090aa8 == &vfs_names.actual_published_01090aa8 &&
            &node_names.actual_small_returns_disabled_01090aa4 ==
                &vfs_names.actual_small_returns_disabled_01090aa4 &&
            &node_names.actual_manager_publication_01090aa0 ==
                &vfs_names.actual_manager_publication_01090aa0 &&
            environment.vtable_00d62c88 == app.camera_environment().vtable_00d62c88,
            "native GUI Group requires the same canonical parenting/node/type/string/table domains");
    }
    void require_entry() const {
        require(!active && !failed, "native GUI Group cannot reenter or replay interrupted work");
    }
    struct Entry {
        Impl& app;
        explicit Entry(Impl& value) : app(value) { app.require_entry(); app.active = true; }
        ~Entry() { app.active = false; }
    };
    void fail(std::uint32_t entry) noexcept { failed_entry = entry; failed = std::current_exception(); }
    static void retire(void* context, NativeGroupReference& same_reference) noexcept {
        auto& app = *static_cast<Impl*>(context);
        if (!app.registered || app.reference.get() != &same_reference ||
            app.actual != &same_reference.group_owner().storage.node || app.child)
            std::terminate();
        app.renderer.actual_owners().unbind(app.actual, same_reference);
        app.registered = false;
        app.retired = true;
        // Companions remain stable until the nonthrowing native callback returns.
    }
};

GameNativeGuiGroupApplication::GameNativeGuiGroupApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& files,
    GameNativeReadOnlyData& data, NativeNodeParentingRuntime& parenting)
    : impl_(std::make_unique<Impl>(log, renderer, files, data, parenting)) {}
GameNativeGuiGroupApplication::~GameNativeGuiGroupApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
NativeGroupEnvironment& GameNativeGuiGroupApplication::groups() noexcept { return impl_->environment; }
NativeGroupReference& GameNativeGuiGroupApplication::root() {
    auto& p = *impl_;
    p.require_entry();
    require(p.reference && p.registered && !p.retired, "native GUI Group has no live canonical root");
    return *p.reference;
}
NativeGroupReference& GameNativeGuiGroupApplication::create_root(const char* name,
    NativeModelReference& text_model) {
    auto& p = *impl_;
    p.require_entry();
    require(!p.actual && !p.owner && !p.reference && !p.retired && name && *name &&
        std::strlen(name) < (std::numeric_limits<std::uint32_t>::max)(),
        "native GUI Group requires one fresh root and an unchanged native-length name");
    auto& model = text_model.model_owner();
    require(model.phase == NativeModelOwner::Phase::live &&
        &model.environment.nodes == &p.environment.nodes &&
        &model.environment.retained_owners == &p.renderer.actual_owners() &&
        model.environment.vtable_00d62c88 == p.environment.vtable_00d62c88 &&
        !model.node.transform.parent && !model.node.transform.root_list &&
        !text_model.scene_attachment().scene &&
        p.renderer.actual_owners().find(&model.storage.node) == &text_model &&
        &p.environment.nodes.attachments.resolve(model.node.transform) == &text_model,
        "native GUI Group requires its same live unattached canonical Text Model");
    Impl::Entry entered(p);
    try {
        p.actual = p.environment.pool_010902f4.allocate_raw_slot_00b8f310();
        require(p.actual != nullptr, "native GUI Group pool returned null");
        p.admission = p.renderer.actual_owners().prepare_binding(p.actual);
        p.owner = std::make_unique<NativeGroupOwner>(p.actual, NativeGroupPool::slot_bytes, p.environment);
        p.name_entered = true;
        p.name.assign_0041e870(p.environment.nodes.require_raw_name_pool(), name);
        p.name_entered = false; p.name_live = true;
        construct_native_group_00b8f5e0(*p.owner, &p.name, *p.environment.raw_node_constants);
        p.name_live = false; p.name_release_entered = true;
        destroy_native_string_header_0041dd20(&p.name, p.environment.nodes.require_raw_name_pool());
        p.name_release_entered = false;
        p.reference = std::make_unique<NativeGroupReference>(*p.owner,
            NativeGroupCompanionDisposal{&p, Impl::retire});
        p.renderer.actual_owners().publish(std::move(p.admission), *p.reference);
        p.registered = true;
        p.child = &text_model; // Persist BEFORE the entered parenting call.
        set_native_node_parent_00b6e680(p.parenting, model.node.transform, &p.owner->node.transform);
        p.log.notef("native GUI Group ready: group=%p text_model=%p pool=010902F4 constructor=B8F5E0",
            p.actual, &model.storage.node);
        return *p.reference;
    } catch (...) { p.fail(p.child ? 0x00b6e680 : 0x00b8f5e0); throw; }
}
void GameNativeGuiGroupApplication::detach_text() {
    auto& p = *impl_;
    p.require_entry();
    require(p.reference && p.registered && !p.retired && p.child &&
        !p.owner->node.transform.root_list && !p.reference->scene_attachment().scene &&
        p.reference->reference_count.load(std::memory_order_relaxed) == 1,
        "native GUI Group detachment requires retired command/scene references");
    require(p.child->transform().parent == &p.owner->node.transform,
        "native GUI Group lost its sole same-owner Text parent relation");
    Impl::Entry entered(p);
    try {
        set_native_node_parent_00b6e680(p.parenting, p.child->transform(), nullptr);
        p.child = nullptr;
    } catch (...) { p.fail(0x00b6e680); throw; }
}
void GameNativeGuiGroupApplication::retire_root() {
    auto& p = *impl_;
    p.require_entry();
    if (p.retired) return;
    if (!p.actual) { p.retired = true; return; }
    require(p.reference && p.registered && !p.child &&
        !p.owner->node.transform.parent && !p.owner->node.transform.first_child &&
        !p.owner->node.transform.root_list && !p.reference->scene_attachment().scene &&
        p.reference->reference_count.load(std::memory_order_relaxed) == 1,
        "native GUI Group retirement requires detached Text and its sole creator reference");
    Impl::Entry entered(p);
    try {
        p.reference->release_model_virtual18_00b6f310();
        require(p.retired && !p.registered && p.owner->phase == NativeGroupOwner::Phase::dead,
            "native GUI Group did not finish native destruction/pool return");
        p.reference.reset(); p.owner.reset(); p.actual = nullptr;
    } catch (...) { p.fail(0x00b8eec0); throw; }
}
bool GameNativeGuiGroupApplication::requires_process_retention() const noexcept {
    const auto& p = *impl_;
    return p.active || p.failed || p.actual || p.owner || p.reference || p.child ||
        p.admission || p.name_entered || p.name_live || p.name_release_entered || p.registered;
}
std::uint32_t GameNativeGuiGroupApplication::failure_entry() const noexcept { return impl_->failed_entry; }
const std::exception_ptr& GameNativeGuiGroupApplication::failure() const noexcept { return impl_->failed; }
} // namespace bsp::game
