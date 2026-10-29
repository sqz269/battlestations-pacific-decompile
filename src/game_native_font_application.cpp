#include "bsp/game_native_font_application.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/game_native_raw_default_text_owner.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_name(const std::string& name) {
    if (name.find('\0') != std::string::npos ||
        name.size() > (std::numeric_limits<std::uint32_t>::max)())
        throw std::invalid_argument("Native font names require null-free DWORD-sized strings");
}
std::uint32_t image_profile(void* image) noexcept {
    std::uint32_t profile{};
    if (image) std::memcpy(&profile, image, sizeof(profile));
    return profile;
}
} // namespace

struct GameNativeFontApplication::Impl {
    enum class Phase { fresh, loading, ready, retiring, retired, failed };
    enum class RawBinding { none, bind_entered, bound, detach_entered, detached, failed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    FontRegistryStartup& registry;
    GameNativeVfsRawServices raw;
    NativeTextureCacheContext& cache;
    NativeGameGridContext& grid;
    NativeFontResourceOwners fonts;
    std::vector<std::unique_ptr<GameNativeFontLoadOperation>> operations;
    GuiTextRuntimeFactory* text_factory{}; // SAME canonical borrower only.
    RawBinding raw_binding{RawBinding::none};
    GameNativeRawDefaultTextOwner* raw_owner{}; // held through explicit post-parent detach
    const void* raw_registry_image{}; // identity ONLY after actual retirement
    bool raw_retirement_entered{};
    bool fonts_exported{};
    bool native_images{};
    Phase phase{Phase::fresh};
    std::exception_ptr failed;

    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& files,
        FontRegistryStartup& target)
        : log(l), renderer(r), vfs(files), registry(target), raw(vfs.borrow_raw_services()),
          cache(renderer.texture_cache()), grid(renderer.game_grid_context()) {
        require(registry.fonts().empty(), "Native font production requires its empty stable registry");
        validate_domain();
    }
    void validate_domain() {
        (void)renderer.section_layout_services(); // Existing ready/native-acquisition gate.
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
            "Native font production requires the same ready renderer, VFS, strings and actual owners");
        require_gui_text_native_renderer_domain(grid.graphics, renderer.publication_00f8d394(),
            raw.strings, grid.graphics.streams.geometry.actual_owners());
        if (native_images) {
            // VFS owns an adapter over these cells; adapter object identity is
            // not the lifetime domain. The raw constructor uses this SAME
            // process's publications and current small-return gate.
            auto& source_strings = vfs.raw_strings();
            auto& image_strings = game_native_string_process().raw_context();
            require(&source_strings.actual_published_01090aa8 == &image_strings.actual_published_01090aa8 &&
                &source_strings.actual_small_returns_disabled_01090aa4 ==
                    &image_strings.actual_small_returns_disabled_01090aa4 &&
                &source_strings.actual_manager_publication_01090aa0 ==
                    &image_strings.actual_manager_publication_01090aa0,
                "Native Font image requires the same actual string-pool process cells");
        }
    }
    void* load_image(NativeString& name, bool& name_live, const std::string& path,
        NativeTextureCacheAcquired& acquired) {
        name_live = true; // Record constructor entry even if allocation throws.
        name.assign_0041e870(raw.strings, path.c_str());
        void* receiver = renderer.publication_00f8d394(); // Original current64 capture.
        std::uint32_t profile{};
        if (receiver) std::memcpy(&profile, receiver, sizeof(profile));
        const auto* table = grid.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
        require(receiver && profile == 0x00d5f0a8 && table && table[0x64 / 4] == 0x00b319b0,
            "Native font image requires the observed current renderer64=B319B0 profile");
        // Numeric token selects the concrete existing body, never a host call.
        return load_native_renderer_texture_00b319b0(receiver, &name, 0, cache, &acquired);
    }
    bool produce(const FontDescriptor& descriptor, const std::string& root,
        const std::string& language, NativeFontResources*& output, std::string& error) {
        require(phase == Phase::loading, "Native font producer cannot reenter or replay");
        require_name(root); require_name(language); require_name(descriptor.data_file);
        require_name(descriptor.gfx_file); require_name(descriptor.alpha_texture);
        auto frame = std::make_unique<GameNativeFontLoadOperation>();
        frame->descriptor = &descriptor;
        frame->gfx_path = root + language + descriptor.gfx_file;
        frame->alpha_path = descriptor.alpha_texture.empty() ? "white.tga" : root + descriptor.alpha_texture;
        frame->data_path = root + language + descriptor.data_file;
        require_name(frame->gfx_path); require_name(frame->alpha_path); require_name(frame->data_path);
        auto& op = *frame;
        operations.push_back(std::move(frame)); // Stable frame BEFORE native effects.
        try {
            validate_domain();
            if (native_images) {
                op.phase = GameNativeFontLoadPhase::adoption;
                op.native_site = 0x00ad55c0;
                NativeFontResources::construct_image_00ad55c0(descriptor,
                    grid.graphics.streams.geometry.actual_owners(), game_native_string_process(), op.unpublished);
                op.native_site = 0x00ac3ba3;
                op.published = &fonts.adopt_image_before_load_00ac3910(op.unpublished);
            }
            op.phase = GameNativeFontLoadPhase::gfx;
            op.native_site = 0x00ad4cd0;
            op.gfx = load_image(op.gfx_name, op.gfx_name_live, op.gfx_path, op.gfx_cache);
            op.gfx_result = op.gfx;
            destroy_native_string_header_0041dd20(&op.gfx_name, raw.strings);
            op.gfx_name_live = false;
            if (native_images) op.published->save_image_paths_00ad4c30_fragment(root, language);
            op.phase = GameNativeFontLoadPhase::alpha;
            op.native_site = descriptor.alpha_texture.empty() ? 0x00ad4e6e : 0x00ad4e29;
            op.alpha = load_image(op.alpha_name, op.alpha_name_live, op.alpha_path, op.alpha_cache);
            op.alpha_result = op.alpha;
            destroy_native_string_header_0041dd20(&op.alpha_name, raw.strings);
            op.alpha_name_live = false;
            op.phase = GameNativeFontLoadPhase::data;
            op.native_site = 0x00ad4c30; // Composition entry; exact raw DAT-open site unclaimed.
            auto source = vfs.open(op.data_path, 2);
            require(source.provider_opened && source.stream && source.stream->fully_initialized(),
                "Native font DAT could not be read completely through the actual VFS");
            op.phase = GameNativeFontLoadPhase::decode;
            op.native_site = 0x00ad4f3a;
            if (native_images) {
                op.published->populate_image_00ad4c30_fragment(*source.stream, op.gfx, op.alpha);
            } else {
                op.decoded = std::make_unique<FontData>();
                if (!decode_font_data_00ad4c30_fragment(*source.stream, descriptor.scale_ratio, *op.decoded, error))
                    throw std::runtime_error(error);
                op.phase = GameNativeFontLoadPhase::adoption;
                op.native_site = 0x00ad50cb;
                op.unpublished = NativeFontResources::adopt_00ad4c30_fragment(descriptor,
                    op.decoded, op.gfx, op.alpha, grid.graphics.streams.geometry.actual_owners());
            }
            source.stream.reset();
            if (native_images) {
                op.phase = GameNativeFontLoadPhase::adoption;
                op.native_site = 0x00ad508f;
                op.published->finish_image_00ad4c30_fragment(op.gfx, op.alpha);
            }
            op.phase = GameNativeFontLoadPhase::publication;
            if (!native_images) op.published = &fonts.publish(op.unpublished);
            output = op.published;
            op.phase = GameNativeFontLoadPhase::complete;
            log.notef("native font %s glyphs=%zu gfx=%p profile=%08x alpha=%p profile=%08x",
                descriptor.name.c_str(), op.published->glyph_count(), op.gfx_result,
                image_profile(op.gfx_result), op.alpha_result, image_profile(op.alpha_result));
            return true;
        } catch (...) {
            op.phase = GameNativeFontLoadPhase::failed;
            op.failure = std::current_exception();
            throw;
        }
    }
};

GameNativeFontApplication::GameNativeFontApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, FontRegistryStartup& registry)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, registry)) {}
GameNativeFontApplication::~GameNativeFontApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}
void GameNativeFontApplication::enable_native_font_images() {
    require(impl_->phase == Impl::Phase::fresh && impl_->operations.empty(),
        "Native Font image selection requires fresh startup");
    impl_->native_images = true;
}
void GameNativeFontApplication::load_descriptors_00ac3910(VfsLuaScriptFiles& files,
    const LuaRuntimeGlobals& globals, const std::string& root,
    const std::string& descriptor, const std::string& language) {
    auto& app = *impl_;
    require(app.phase == Impl::Phase::fresh, "Native font startup is one attempt, with no reload/replay");
    require_name(root); require_name(descriptor); require_name(language);
    app.validate_domain();
    app.phase = Impl::Phase::loading;
    try {
        if (app.native_images) app.fonts.construct_image_registry_00ac3690();
        std::string error;
        const FontRegistryNativeResourceLoader loader = [&](const FontDescriptor& font,
            const std::string& prefix, const std::string& extra,
            NativeFontResources*& result, std::string& failure) {
            return app.produce(font, prefix, extra, result, failure);
        };
        if (!app.registry.load_lua_native_descriptors_00ac3910(files, globals, loader,
                root, descriptor, language, error)) throw std::runtime_error(error);
        if (app.native_images) {
            app.fonts.complete_image_registry();
            app.registry.use_native_image_lookup_00ac3570(app.fonts);
        }
        app.phase = Impl::Phase::ready;
        app.log.implemented("NativeFontRegistry::initial_resources", "00ad4c30");
    } catch (...) {
        app.failed = std::current_exception();
        app.phase = Impl::Phase::failed;
        throw;
    }
}
NativeFontResourceOwners& GameNativeFontApplication::native_fonts() {
    require(ready(), "Native font association export requires complete production");
    require(impl_->raw_binding == Impl::RawBinding::none && !impl_->raw_owner,
        "Native fonts cannot mix legacy export with the exclusive raw owner arm");
    impl_->fonts_exported = true;
    return impl_->fonts;
}
void GameNativeFontApplication::bind_text_factory(GuiTextRuntimeFactory& factory) {
    auto& app = *impl_;
    require(app.raw_binding == Impl::RawBinding::none && !app.raw_owner,
        "Native fonts cannot mix legacy factory binding with raw owner custody");
    require(ready() && app.fonts_exported && &factory.native_font_resources() == &app.fonts &&
        (app.native_images ? factory.native_font_registry_image() == app.fonts.native_registry_image()
            : factory.native_font_registry() == &app.registry) &&
        (!app.text_factory || app.text_factory == &factory) && factory.retained_allocation_count() == 0,
        "Native font borrowers require the same registry, resources and empty canonical Text factory before construction");
    app.text_factory = &factory;
}
void GameNativeFontApplication::fail_raw_text_owner_link(GameNativeRawDefaultTextOwner& attempted) noexcept {
    auto& app = *impl_;
    if (!app.failed) app.failed = std::current_exception();
    app.raw_binding = Impl::RawBinding::failed;
    app.phase = Impl::Phase::failed;
    if (!app.raw_owner) app.raw_owner = &attempted;
    app.raw_owner->fail_raw_font_link(*this);
    if (app.raw_owner != &attempted) attempted.fail_raw_font_link(*this);
}
bool GameNativeFontApplication::raw_text_owner_awaits_detach(const GameNativeRawDefaultTextOwner* owner) const noexcept {
    const auto& app = *impl_;
    return owner && app.raw_owner == owner && app.phase == Impl::Phase::retired && !app.failed &&
        app.raw_binding == Impl::RawBinding::bound && app.raw_retirement_entered;
}
NativeFontResourceOwners& GameNativeFontApplication::bind_raw_text_owner(GameNativeRawDefaultTextOwner& owner) {
    auto& app = *impl_;
    try {
        require(ready() && !app.failed && !app.fonts_exported && !app.text_factory &&
            app.raw_binding == Impl::RawBinding::none && !app.raw_owner,
            "Native raw Font export needs one unexported ready producer, never a rebind or legacy borrower");
        app.raw_binding = Impl::RawBinding::bind_entered; // BEFORE any export/admission
        app.raw_owner = &owner;
        owner.begin_raw_font_link(*this, app.renderer, app.vfs);
        require(app.native_images && app.fonts.native_registry_image(),
            "Native raw Font export requires the actual ready image registry, never typed FontData");
        app.validate_domain();
        app.fonts_exported = true; // irreversible actual collection export
        app.raw_registry_image = app.fonts.native_registry_image();
        owner.complete_raw_font_link(*this, app.fonts, app.raw_registry_image);
        require(app.raw_binding == Impl::RawBinding::bind_entered && app.raw_owner == &owner &&
            app.phase == Impl::Phase::ready && !app.failed,
            "Native raw Font binding cannot resume after a caught nested failure");
        app.raw_binding = Impl::RawBinding::bound;
        return app.fonts;
    } catch (...) { fail_raw_text_owner_link(owner); throw; }
}
void GameNativeFontApplication::detach_raw_text_owner_after_parent_drain(GameNativeRawDefaultTextOwner& owner) {
    auto& app = *impl_;
    try {
        require(app.phase == Impl::Phase::retired && !app.failed && app.raw_retirement_entered &&
            app.raw_binding == Impl::RawBinding::bound && app.raw_owner == &owner &&
            app.fonts_exported && !app.text_factory && app.raw_registry_image,
            "Native raw Font detach requires SAME bound owner and completed actual retirement");
        app.raw_binding = Impl::RawBinding::detach_entered;
        // No native_registry_image()/Font/header read after retirement.
        owner.detach_raw_font_link_after_parent_drain(*this, app.fonts, app.raw_registry_image);
        require(app.raw_binding == Impl::RawBinding::detach_entered && app.raw_owner == &owner &&
            app.phase == Impl::Phase::retired && !app.failed,
            "Native raw Font detach cannot resume after a caught nested failure");
        app.raw_owner = nullptr;
        app.raw_registry_image = nullptr;
        app.raw_binding = Impl::RawBinding::detached;
    } catch (...) { fail_raw_text_owner_link(owner); throw; }
}
void GameNativeFontApplication::retire_fonts() {
    auto& app = *impl_;
    require(app.phase == Impl::Phase::fresh || app.phase == Impl::Phase::ready,
        "Native fonts cannot replay incomplete or completed retirement");
    const bool raw = app.raw_binding != Impl::RawBinding::none;
    if (raw) {
        try {
            require(app.raw_binding == Impl::RawBinding::bound && app.raw_owner && !app.raw_retirement_entered &&
                !app.failed && app.fonts_exported && !app.text_factory && app.native_images &&
                app.raw_registry_image && app.fonts.native_registry_image() == app.raw_registry_image,
                "Native raw Font retirement needs SAME current ready image/owner and one completed link");
            app.raw_retirement_entered = true; // BEFORE terminal-owner admission
            app.raw_owner->require_font_retirement(*this, app.fonts, app.raw_registry_image);
            app.validate_domain();
        } catch (...) {
            if (app.raw_owner) fail_raw_text_owner_link(*app.raw_owner);
            throw;
        }
    } else {
        require((!app.fonts_exported || app.text_factory) &&
            (!app.text_factory || app.text_factory->retained_allocation_count() == 0),
            "Native font retirement requires quiescent same-factory Text borrowers");
        app.validate_domain();
    }
    app.phase = Impl::Phase::retiring;
    try {
        if (app.native_images) app.fonts.retire_image_registry_00ac36f0();
        for (auto& record : app.operations) {
            require(record->phase == GameNativeFontLoadPhase::complete && record->published,
                "Native font retirement requires completed canonical productions");
            if (!app.native_images) record->published->destroy_00ad53a0_fragment();
            app.registry.clear_retired_native_binding(*record->descriptor, *record->published);
        }
        if (app.native_images) app.registry.clear_retired_image_lookup(app.fonts);
        if (raw) require(app.phase == Impl::Phase::retiring && app.raw_binding == Impl::RawBinding::bound && !app.failed,
            "Native raw Font retirement cannot resume after a caught nested link failure");
        app.phase = Impl::Phase::retired;
    } catch (...) {
        if (!raw || !app.failed) app.failed = std::current_exception();
        app.phase = Impl::Phase::failed;
        if (raw && app.raw_owner) fail_raw_text_owner_link(*app.raw_owner);
        throw;
    }
}
const GameNativeFontLoadOperation& GameNativeFontApplication::operation(std::size_t index) const {
    return *impl_->operations.at(index);
}
std::size_t GameNativeFontApplication::produced_fonts() const noexcept {
    std::size_t completed{};
    for (const auto& op : impl_->operations) if (op->phase == GameNativeFontLoadPhase::complete) ++completed;
    return completed;
}
bool GameNativeFontApplication::ready() const noexcept { return impl_->phase == Impl::Phase::ready; }
bool GameNativeFontApplication::requires_process_retention() const noexcept {
    if (impl_->raw_binding == Impl::RawBinding::bind_entered || impl_->raw_binding == Impl::RawBinding::detach_entered ||
        impl_->raw_binding == Impl::RawBinding::failed ||
        (impl_->phase == Impl::Phase::retired && impl_->raw_binding == Impl::RawBinding::bound)) return true;
    return impl_->phase != Impl::Phase::retired &&
        (impl_->phase != Impl::Phase::fresh || !impl_->operations.empty());
}
const std::exception_ptr& GameNativeFontApplication::failure() const noexcept { return impl_->failed; }
} // namespace bsp::game
