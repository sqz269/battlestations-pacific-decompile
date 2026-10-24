#include "bsp/game_native_gui_resource_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
constexpr const char* kWhiteGui = "data/interface/textures/whiteGui.tga";
constexpr const char* kTransparent = "interface/textures/common/transparent.tga";
} // namespace

struct GameNativeGuiResourceApplication::Impl {
    enum class Phase { active, loading, retiring, retired, failed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameNativeVfsRawServices raw;
    NativeTextureCacheContext& cache;
    NativeGameGridContext& grid;
    std::vector<std::unique_ptr<GameNativeGuiResourceLoadOperation>> operations;
    Phase phase{Phase::active};
    std::exception_ptr failed;

    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& files)
        : log(l), renderer(r), vfs(files), raw(vfs.borrow_raw_services()),
          cache(renderer.texture_cache()), grid(renderer.game_grid_context()) {
        validate_domain();
    }
    void validate_domain() {
        (void)renderer.section_layout_services(); // Existing ready/native admission.
        auto current = vfs.borrow_raw_services();
        require(&renderer.texture_cache() == &cache && &renderer.game_grid_context() == &grid &&
            &current.bindings == &raw.bindings && &current.strings == &raw.strings &&
            &current.opens == &raw.opens && &current.conversion == &raw.conversion &&
            &current.actual_vfs_publication_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &cache.strings == &raw.strings && &grid.strings == &raw.strings &&
            &cache.textures.strings == &raw.strings && &cache.textures.streams == &raw.bindings &&
            &cache.textures.opens == &raw.opens && &cache.textures.conversion == &raw.conversion &&
            static_cast<const volatile void*>(&cache.textures.current_vfs_0109ceec) ==
                static_cast<const volatile void*>(&raw.actual_vfs_publication_0109ceec) &&
            static_cast<const volatile void*>(&cache.textures.current_renderer_00f8d394) ==
                static_cast<const volatile void*>(&renderer.publication_00f8d394()) &&
            &grid.graphics.streams.geometry.actual_owners() == &renderer.actual_owners() &&
            cache.textures.current_vfs_0109ceec == raw.actual_vfs_publication_0109ceec &&
            cache.textures.current_renderer_00f8d394 == renderer.publication_00f8d394(),
            "Native GUI resources require the same ready renderer, VFS, strings and actual owners");
        require_gui_text_native_renderer_domain(grid.graphics, renderer.publication_00f8d394(),
            raw.strings, grid.graphics.streams.geometry.actual_owners());
    }
};

bool GameNativeGuiResourceApplication::requested() noexcept {
    static const bool enabled = [] {
        char text[2]{};
        return GetEnvironmentVariableA("BSP_NATIVE_GUI_RESOURCES", text, sizeof text) == 1 && text[0] == '1';
    }();
    return enabled;
}
GameNativeGuiResourceApplication::GameNativeGuiResourceApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs)
    : impl_(std::make_unique<Impl>(log, renderer, vfs)) {}
GameNativeGuiResourceApplication::~GameNativeGuiResourceApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
void* GameNativeGuiResourceApplication::load_manager_texture_00aa5e20(
    std::uint16_t offset, std::string_view path) {
    auto& app = *impl_;
    require(app.phase == Impl::Phase::active, "Native GUI texture load cannot reenter or replay");
    const bool white = app.operations.empty();
    require((white && offset == 0x28 && path == kWhiteGui) ||
        (!white && app.operations.size() == 1 && offset == 0x2c && path == kTransparent),
        "Native GUI resource binding supports only the ordered initial AA5E20 texture entries");
    app.validate_domain();
    auto frame = std::make_unique<GameNativeGuiResourceLoadOperation>();
    frame->manager_offset = offset;
    frame->requested_path = std::string(path);
    auto& op = *frame;
    app.operations.push_back(std::move(frame)); // Publish BEFORE native effects.
    app.phase = Impl::Phase::loading;
    try {
        op.native_site = white ? 0x00aa5e56 : 0x00aa5eb7;
        op.name_live = true;
        op.name.assign_0041e870(app.raw.strings, op.requested_path.c_str());
        if (white) {
            op.phase = GameNativeGuiResourceLoadOperation::Phase::resolving;
            op.native_site = 0x00aa5e88;
            op.resolved = op.resolution.invoke(&op.name, app.raw.actual_vfs_publication_0109ceec,
                app.cache.textures.resolution_context, app.cache.textures.make_resolution_operation);
        } else {
            op.resolved = true; // The native transparent entry has no outer guard.
        }
        if (op.resolved) {
            op.phase = GameNativeGuiResourceLoadOperation::Phase::loading;
            op.native_site = white ? 0x00aa5ea2 : 0x00aa5ef0;
            void* const receiver = app.renderer.publication_00f8d394();
            std::uint32_t profile{};
            if (receiver) std::memcpy(&profile, receiver, sizeof(profile));
            const auto* table = app.grid.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
            require(receiver && profile == 0x00d5f0a8 && table && table[0x64 / 4] == 0x00b319b0,
                "Native GUI texture requires the observed renderer64=B319B0 profile");
            op.result = load_native_renderer_texture_00b319b0(receiver, &op.name, 0,
                app.cache, &op.cache);
            op.reference_owned = op.result != nullptr; // Exactly the returned reference.
        }
        destroy_native_string_header_0041dd20(&op.name, app.raw.strings);
        op.name_live = false;
        op.phase = GameNativeGuiResourceLoadOperation::Phase::complete;
        app.phase = Impl::Phase::active;
        app.log.notef("native GUI resource offset=%02x requested=%s resolved=%d actual=%p renderer64=00b319b0",
            static_cast<unsigned>(offset), op.requested_path.c_str(), op.resolved ? 1 : 0, op.result);
        app.log.implemented("GuiManagerResources::renderer_load_texture", "00aa5e60");
        return op.result;
    } catch (...) {
        op.failure = app.failed = std::current_exception();
        op.phase = GameNativeGuiResourceLoadOperation::Phase::failed;
        app.phase = Impl::Phase::failed;
        throw;
    }
}
void GameNativeGuiResourceApplication::retire_textures() {
    auto& app = *impl_;
    require(app.phase == Impl::Phase::active, "Native GUI texture retirement cannot replay failed or completed work");
    app.validate_domain();
    app.phase = Impl::Phase::retiring;
    try {
        for (auto& frame : app.operations) {
            auto& op = *frame;
            require(op.phase == GameNativeGuiResourceLoadOperation::Phase::complete && !op.name_live,
                "Native GUI resource retirement requires completed load frames");
            if (op.reference_owned) {
                void* const captured = op.result;
                op.reference_owned = false;
                op.release_started = true;
                release_native_render_actual_owner_observed(app.renderer.actual_owners(), captured, op.release);
            }
        }
        app.phase = Impl::Phase::retired;
        app.log.note("native GUI resource references retired before renderer drain");
    } catch (...) {
        app.failed = std::current_exception();
        app.phase = Impl::Phase::failed;
        throw;
    }
}
const GameNativeGuiResourceLoadOperation& GameNativeGuiResourceApplication::operation(std::size_t index) const {
    return *impl_->operations.at(index);
}
std::size_t GameNativeGuiResourceApplication::operation_count() const noexcept {
    return impl_->operations.size();
}
bool GameNativeGuiResourceApplication::requires_process_retention() const noexcept {
    return impl_->phase != Impl::Phase::retired &&
        (impl_->phase != Impl::Phase::active || !impl_->operations.empty());
}
const std::exception_ptr& GameNativeGuiResourceApplication::failure() const noexcept {
    return impl_->failed;
}
} // namespace bsp::game
