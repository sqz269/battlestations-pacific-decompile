#include "bsp/game_native_gui_node_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
template<class T> const volatile T& cell(GameNativeReadOnlyData& data, std::uint32_t address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
const volatile std::uint32_t* table(GameNativeReadOnlyData& data,
    std::uint32_t address, std::size_t bytes) {
    return static_cast<const volatile std::uint32_t*>(data.data_at(address, bytes));
}
void require_fresh_layout(const GuiLayoutWidget& layout) {
    require(!layout.parent && !layout.transform.parent && !layout.before_destroy &&
        layout.children.empty() && layout.transform.children.empty() && !layout.node_id,
        "native GUI construction requires its sole fresh unattached layout");
    require(layout.key.find('\0') == std::string::npos &&
        layout.key.size() < (std::numeric_limits<std::uint32_t>::max)(),
        "native GUI node name must fit its unchanged native string length");
}
} // namespace

struct GameNativeGuiNodeApplication::Impl {
    struct Standalone {
        std::unique_ptr<GuiLayoutWidget> layout;
        GuiWidgetOwner* owner{}; // Borrow of widgets' sole canonical owner.
    };
    // Host lifetime token only: neither field represents a native reference.
    struct WidgetToken {
        std::shared_ptr<Impl> application;
        std::shared_ptr<Standalone> storage;
    };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameNativeVfsRawServices raw;
    LightTypeBootstrap node_types;
    ModelTypeBootstrap model_types;
    NativeGroupTypes group_types;
    NativeModelRawNameServices model_names;
    NativeModelEnvironment models;
    NativeGroupEnvironment groups;
    GuiNativeScene scene;
    GuiWidgetOwnerRuntime widgets;
    std::unordered_map<GuiLayoutWidget*, std::shared_ptr<Standalone>> standalone;
    // Borrow associations only, not another tree or layout owner.
    std::unordered_set<GuiLayoutWidget*> borrowed_roots;
    bool active{};
    std::uint32_t failed_entry{};
    std::exception_ptr failed;

    Impl(GameHostLog& output, GameNativeRendererApplication& app, GameVfsHost& vfs,
        GameNativeReadOnlyData& data, GuiWidgetImplementationFactory factory)
        : log(output), renderer(app), files(vfs), raw(vfs.borrow_raw_services()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          model_types(vfs.native_owners().types(), node_types, vfs.native_types().model_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          model_names{app.node_constants(), raw.strings},
          models{game_native_resource_pool_process().model_pool_01090054(),
              app.camera_environment().nodes, model_types, app.actual_owners(),
              {cell<std::uint32_t>(data, 0x00ce4adc), cell<std::uint32_t>(data, 0x00ce4970),
               cell<std::uint32_t>(data, 0x00d7a208), cell<std::uint32_t>(data, 0x00d7a260)},
              table(data, 0x00d62de8, 0x5c), table(data, 0x00d62c88, 0x58), nullptr, &model_names},
          groups{game_native_resource_pool_process().group_pool_010902f4(),
              app.camera_environment().nodes, group_types,
              {app.node_constants().one_00d7a24c, app.node_constants().positive_bound_00ce4970},
              table(data, 0x00d634f8, 0x58), models.vtable_00d62c88, &app.node_constants()},
          scene(groups), widgets({models, scene, std::move(factory), nullptr, &app.actual_owners()}) {
        auto& node_strings = models.nodes.require_raw_name_pool();
        auto& vfs_strings = vfs.raw_strings();
        require(&node_strings.actual_published_01090aa8 == &vfs_strings.actual_published_01090aa8 &&
            &node_strings.actual_small_returns_disabled_01090aa4 ==
                &vfs_strings.actual_small_returns_disabled_01090aa4 &&
            &node_strings.actual_manager_publication_01090aa0 ==
                &vfs_strings.actual_manager_publication_01090aa0 &&
            &app.game_grid_context().strings == &raw.strings &&
            &models.constants.minimum_00ce4adc == &app.node_constants().negative_bound_00ce4adc &&
            &models.constants.maximum_00ce4970 == &app.node_constants().positive_bound_00ce4970,
            "GUI nodes require the renderer's same actual node/string/constant domains");
    }
    void require_entry() const {
        require(!active && !failed && !scene.native_operation_interrupted(),
            "native GUI nodes cannot reenter or replay an interrupted operation");
    }
    void record_failure(std::uint32_t entry) noexcept {
        failed_entry = entry;
        failed = std::current_exception();
    }
    struct Entry {
        Impl& app;
        explicit Entry(Impl& value) : app(value) { app.require_entry(); app.active = true; }
        ~Entry() { app.active = false; }
    };
    Standalone& find(GuiWidgetOwner& owner) const {
        const auto found = standalone.find(&owner.layout());
        require(found != standalone.end() && found->second->layout.get() == &owner.layout() &&
            found->second->owner == &owner && &owner.runtime() == &widgets &&
            &widgets.owner(owner.layout()) == &owner,
            "native GUI lifetime token requires its same app-owned canonical Text");
        return *found->second;
    }
};

GameNativeGuiNodeApplication::GameNativeGuiNodeApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& files, GameNativeReadOnlyData& data,
    GuiWidgetImplementationFactory factory)
    : impl_(std::make_shared<Impl>(log, renderer, files, data, std::move(factory))) {}
GameNativeGuiNodeApplication::~GameNativeGuiNodeApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
GuiWidgetOwnerRuntime& GameNativeGuiNodeApplication::widgets() noexcept { return impl_->widgets; }
NativeNodeParentingRuntime& GameNativeGuiNodeApplication::parenting() noexcept {
    return impl_->scene.parenting();
}
GuiNativeScene& GameNativeGuiNodeApplication::scene() noexcept { return impl_->scene; }
NativeModelEnvironment& GameNativeGuiNodeApplication::models() noexcept { return impl_->models; }

GuiWidgetOwner& GameNativeGuiNodeApplication::create_standalone_text(
    std::unique_ptr<GuiLayoutWidget> layout) {
    auto& app = *impl_;
    app.require_entry();
    require(layout && layout->type == GuiWidgetType::Text,
        "standalone native Text requires its sole authored type3 layout");
    require_fresh_layout(*layout);
    auto storage = std::make_shared<Impl::Standalone>();
    storage->layout = std::move(layout);
    auto* const identity = storage->layout.get();
    require(app.standalone.emplace(identity, std::move(storage)).second,
        "standalone native Text already has an owning layout record");
    Impl::Entry guard(app);
    try {
        auto& owner = app.widgets.create_with_scene_00aa6640(*identity);
        app.standalone.at(identity)->owner = &owner;
        require(owner.node_binding() && owner.model_reference(),
            "standalone native Text requires its actual canonical Model allocation");
        app.log.implemented("NativeGuiNode::standalone_text", "00aa6640");
        return owner;
    } catch (...) {
        app.record_failure(0x00aa6640);
        throw;
    }
}
std::shared_ptr<const void> GameNativeGuiNodeApplication::retain_widget(GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_entry();
    (void)app.find(owner);
    auto token = std::make_shared<Impl::WidgetToken>(
        Impl::WidgetToken{impl_, app.standalone.at(&owner.layout())});
    return std::shared_ptr<const void>(std::move(token), &owner);
}
std::unique_ptr<GuiLayoutWidget> GameNativeGuiNodeApplication::take_standalone_for_retirement(
    GuiWidgetOwner& owner) {
    auto& app = *impl_;
    app.require_entry();
    auto& current = app.find(owner);
    owner.require_no_active_owned_operation();
    require(!owner.layout().parent && !owner.layout().transform.parent,
        "standalone Text retirement requires its same detached layout");
    auto found = app.standalone.find(&owner.layout());
    require(found->second.use_count() == 1,
        "release retained widget/material lifetime tokens before native Text retirement");
    auto result = std::move(current.layout);
    app.standalone.erase(found);
    return result;
}
GuiWidgetOwner& GameNativeGuiNodeApplication::create_plain_page_root_00aa5840_fragment(
    GuiLayoutWidget& layout) {
    auto& app = *impl_;
    app.require_entry();
    require(layout.type == GuiWidgetType::Screen, "native page root requires its Screen layout");
    require_fresh_layout(layout);
    require(app.borrowed_roots.emplace(&layout).second,
        "native page root already has a borrowed layout association");
    Impl::Entry guard(app);
    try {
        auto& owner = app.scene.create_plain_page_root_00aa5840_fragment(app.widgets, layout);
        app.log.implemented("NativeGuiNode::plain_page_root", "00aa5840");
        return owner;
    } catch (...) {
        app.record_failure(0x00aa5840);
        throw;
    }
}
void GameNativeGuiNodeApplication::retire_borrowed_page_tree(GuiLayoutWidget& layout) {
    auto& app = *impl_;
    app.require_entry();
    require(app.borrowed_roots.count(&layout) != 0,
        "native page retirement requires its same borrowed root association");
    Impl::Entry guard(app);
    try {
        app.widgets.retire_tree(layout);
        app.borrowed_roots.erase(&layout);
    } catch (...) {
        app.record_failure(0x00aa31f0);
        throw;
    }
}
bool GameNativeGuiNodeApplication::requires_process_retention() const noexcept {
    return impl_->active || impl_->failed || !impl_->standalone.empty() ||
        !impl_->borrowed_roots.empty() || impl_->scene.native_operation_interrupted() ||
        impl_->widgets.retained_model_count() != 0 || impl_->scene.retained_group_count() != 0;
}
std::uint32_t GameNativeGuiNodeApplication::failure_entry() const noexcept { return impl_->failed_entry; }
const std::exception_ptr& GameNativeGuiNodeApplication::failure() const noexcept { return impl_->failed; }
} // namespace bsp::game
