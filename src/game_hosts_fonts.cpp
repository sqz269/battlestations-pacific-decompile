#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_font_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/vfs_candidates.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp::game {
struct GameFontHost::Impl {
    enum class TextBinding { unexported, export_pending, bind_entered, bound };
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    FontRegistryStartup fonts;
    GameNativeFontApplication native;
    bool native_retirement_attempted{};
    bool native_retired{};
    bool native_failed{};
    TextBinding text_binding{TextBinding::unexported};
    NativeFontResourceOwners* exported_fonts{};
    FingerprintPayload fingerprint;
    std::size_t opens{};
    bool initialized{};

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        GameNativeRendererApplication& renderer)
        : log(log_in), vfs(vfs_in), scripts(scripts_in),
          native(log_in, renderer, vfs_in, fonts) {}

    bool read(const std::string& requested, std::shared_ptr<MemoryStream>& stream,
        std::string& error) {
        auto* manager = vfs.ready() ? &vfs.context() : nullptr;
        if (!manager) { error = "Font VFS manager missing"; return false; }
        auto& mounts = *manager;
        std::string resolved = requested;
        if (!resolve_existing_resource_00bdf4c0_fragment(mounts,
                vfs.search_registrations(), resolved)) {
            error = "Font VFS cannot resolve " + requested;
            return false;
        }
        auto opened = open_resource_memory_00bdf310_fragment(mounts, resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            error = "Font VFS cannot fully read " + resolved + ": " + opened.error;
            return false;
        }
        stream = std::move(opened.stream);
        ++opens;
        log.notef("font resource %s -> %s bytes=%lld", requested.c_str(),
            resolved.c_str(), static_cast<long long>(stream->size_00bef600()));
        return true;
    }
};

GameFontHost::GameFontHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
    GameNativeRendererApplication& renderer)
    : impl_(std::make_unique<Impl>(log, vfs, scripts, renderer)) {}
GameFontHost::~GameFontHost() = default;

void GameFontHost::initialize(const std::string& language_font_path) {
    load_descriptors_00ac3910(std::string(kFontRootPrefix), std::string(kFontDescriptorPath),
        language_font_path);
    preload_fingerprint_payload_00be9760();
}

void GameFontHost::enable_native_font_images() {
    auto& host = *impl_;
    if (host.initialized || host.native_failed || host.native_retirement_attempted)
        throw std::logic_error("Native Font image selection must precede descriptor loading");
    host.native.enable_native_font_images();
}

void GameFontHost::load_descriptors_00ac3910(const std::string& root,
    const std::string& descriptor, const std::string& language_font_path) {
    auto& host = *impl_;
    if (host.initialized) throw std::logic_error("Application font startup already completed");
    if (host.native_failed || host.native_retirement_attempted || host.native_retired)
        throw std::logic_error("Native font host cannot load after an interrupted or retired operation");
    try {
        host.native.load_descriptors_00ac3910(host.scripts.files(), host.scripts.globals(),
            root, descriptor, language_font_path);
        const auto& rows = host.fonts.fonts();
        if (host.native.produced_fonts() != rows.size())
            throw std::runtime_error("Native font producer/registry count mismatch");
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const auto& row = *rows[i];
            const auto& op = host.native.operation(i);
            if (row.resources || !row.native_resources || !row.native_resources->live() ||
                op.phase != GameNativeFontLoadPhase::complete ||
                op.descriptor != &row.descriptor || op.published != row.native_resources ||
                op.decoded || op.unpublished || op.gfx || op.alpha ||
                op.gfx_name_live || op.alpha_name_live)
                throw std::runtime_error("Native font registry must borrow its sole completed producer");
        }
        host.initialized = true;
        host.log.implemented("Phase 7 font registry resources", "00ac3910");
        host.log.notef("font descriptors loaded: %zu native resources; same_registry=1 "
            "same_descriptors=1 semantic_resources=0 image_refs=transferred dat_allocations=sole",
            rows.size());
    } catch (...) {
        // Also latch preflight and post-production failures outside the producer.
        // Neither may authorize replay or rollback of acquired native state.
        host.native_failed = true;
        throw;
    }
}

void GameFontHost::preload_fingerprint_payload_00be9760() {
    auto& host = *impl_;
    //0073bc0d..bc17 forces the payload preload, discarding its data pointer.
    //This is application RAII, not the original global singleton allocator ABI.
    std::shared_ptr<MemoryStream> payload;
    std::string error;
    if (!host.read("fonts/arial19.dat", payload, error))
        throw std::runtime_error("Fingerprint preload: " + error);
    host.fingerprint.load_00be9760(*payload);
    host.log.implemented("Phase 7 fingerprint payload preload", "00be9760");
    host.log.notef("fingerprint_defined_bytes=%zu decoded=%d", host.fingerprint.defined_size(),
        host.fingerprint.decoded() ? 1 : 0);
}

NativeFontResourceOwners& GameFontHost::native_fonts() {
    auto& host = *impl_;
    if (!host.initialized || !host.native.ready() || host.native_failed ||
        host.native_retirement_attempted || host.native_retired ||
        host.text_binding != Impl::TextBinding::unexported)
        throw std::logic_error("Native font host exports its complete resources only once before retirement");
    // Before the producer's irreversible export, and before the caller can
    // construct any factory. A constructor failure cannot lose this gap state.
    host.text_binding = Impl::TextBinding::export_pending;
    try {
        host.exported_fonts = &host.native.native_fonts();
        return *host.exported_fonts;
    } catch (...) {
        host.native_failed = true;
        throw;
    }
}

void GameFontHost::bind_text_factory(GuiTextRuntimeFactory& factory) {
    auto& host = *impl_;
    if (host.native_failed || host.native_retirement_attempted || host.native_retired ||
        !host.initialized || !host.native.ready() ||
        host.text_binding != Impl::TextBinding::export_pending)
        throw std::logic_error("Native font host binds one factory only after its sole export");
    // The first admitted attempt includes identity/emptiness preflight. Even
    // its rejection cannot be retried or authorize cleanup of exported fonts.
    host.text_binding = Impl::TextBinding::bind_entered;
    try {
        if (!host.exported_fonts || &factory.native_font_resources() != host.exported_fonts ||
            factory.retained_allocation_count() != 0)
            throw std::invalid_argument("Native font host requires the same empty Text factory resource domain");
        host.native.bind_text_factory(factory);
        host.text_binding = Impl::TextBinding::bound; // Only after exact-factory success.
    } catch (...) {
        host.native_failed = true;
        throw;
    }
}

void GameFontHost::retire_native_fonts() {
    auto& host = *impl_;
    if (host.native_retired) return;
    if (host.native_failed || host.native_retirement_attempted ||
        host.text_binding == Impl::TextBinding::export_pending ||
        host.text_binding == Impl::TextBinding::bind_entered)
        throw std::logic_error("Native font host cannot replay an interrupted operation");
    host.native_retirement_attempted = true; // Before every callee preflight.
    try {
        const auto count = host.native.produced_fonts();
        host.native.retire_fonts();
        for (const auto& row : host.fonts.fonts()) {
            if (row->native_resources || row->resources)
                throw std::runtime_error("Native font retirement left a registry resource binding");
        }
        host.native_retired = true;
        host.log.notef("native font host retired: fonts=%zu registry_bindings=cleared "
            "sprite_borrowers=closed before_device_release=1 before_shared_drain=1", count);
    } catch (...) {
        host.native_failed = true;
        throw;
    }
}
bool GameFontHost::native_operation_interrupted() const noexcept {
    const auto& host = *impl_;
    return host.native_failed ||
        host.text_binding == Impl::TextBinding::export_pending ||
        host.text_binding == Impl::TextBinding::bind_entered ||
        (host.native_retirement_attempted && !host.native_retired) ||
        (!host.native.ready() && host.native.requires_process_retention());
}
FontRegistryStartup& GameFontHost::registry() noexcept { return impl_->fonts; }
const FingerprintPayload& GameFontHost::fingerprint() const noexcept { return impl_->fingerprint; }
std::size_t GameFontHost::resource_opens() const noexcept { return impl_->opens + impl_->native.produced_fonts(); }
}
