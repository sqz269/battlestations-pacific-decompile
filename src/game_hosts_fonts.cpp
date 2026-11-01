#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_font_application.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_raw_default_text_owner.hpp"
#include "bsp/game_native_gui_page_application.hpp"
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
    enum class TextBinding { unexported, export_pending, bind_entered, bound, detach_entered, detached };
    enum class PageBinding { none, bind_entered, bound, prepare_entered, prepared,
        retire_entered, retired_await_parent, detach_entered, detached, failed };
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    GameNativeRendererApplication& renderer;
    FontRegistryStartup fonts;
    GameNativeFontApplication native;
    bool native_retirement_attempted{};
    bool native_retired{};
    bool native_failed{};
    TextBinding text_binding{TextBinding::unexported};
    NativeFontResourceOwners* exported_fonts{};
    GameNativeRawDefaultTextOwner* raw_owner{}; // exclusive typed alternative, never a legacy stand-in
    const void* raw_registry_image{}; // saved identity only after actual Font retirement
    PageBinding page_binding{PageBinding::none};
    GameNativeGuiPageApplication* page_owner{};
    NativeFontResourceOwners* page_fonts{};
    const void* page_registry_image{};
    std::exception_ptr page_failure;
    FingerprintPayload fingerprint;
    std::size_t opens{};
    bool initialized{};

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        GameNativeRendererApplication& renderer_in)
        : log(log_in), vfs(vfs_in), scripts(scripts_in), renderer(renderer_in),
          native(log_in, renderer_in, vfs_in, fonts) {}

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
        host.text_binding != Impl::TextBinding::unexported || host.page_owner ||
        host.page_binding != Impl::PageBinding::none)
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
        host.text_binding != Impl::TextBinding::export_pending || host.page_owner ||
        host.page_binding != Impl::PageBinding::none)
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

void GameFontHost::bind_raw_text_owner(GameNativeRawDefaultTextOwner& owner) {
    auto& host = *impl_;
    try {
        if (!host.initialized || !host.native.ready() || host.native_failed ||
            host.native_retirement_attempted || host.native_retired ||
            host.text_binding != Impl::TextBinding::unexported || host.raw_owner || host.page_owner ||
            host.page_binding != Impl::PageBinding::none)
            throw std::logic_error("Native Font raw binding needs one unexported ready host, never mixed/replayed borrowers");
        host.text_binding = Impl::TextBinding::bind_entered; // BEFORE producer admission/export
        host.raw_owner = &owner;
        host.exported_fonts = &host.native.bind_raw_text_owner(owner);
        host.raw_registry_image = host.exported_fonts->native_registry_image(); // STILL ready/live, no lookup
        if (host.native_failed || host.text_binding != Impl::TextBinding::bind_entered ||
            host.raw_owner != &owner || !host.raw_registry_image)
            throw std::logic_error("Native Font raw host cannot resume after a caught nested binding failure");
        host.text_binding = Impl::TextBinding::bound;
    } catch (...) {
        host.native_failed = true;
        host.native.fail_raw_text_owner_link(owner); // BOTH layers and actual/attempted owners remain retained
        throw;
    }
}

void GameFontHost::detach_raw_text_owner_after_parent_drain(GameNativeRawDefaultTextOwner& owner) {
    auto& host = *impl_;
    try {
        if (host.native_failed || !host.native_retired || !host.native_retirement_attempted ||
            host.text_binding != Impl::TextBinding::bound || host.raw_owner != &owner ||
            !host.exported_fonts || !host.raw_registry_image)
            throw std::logic_error("Native Font raw host detach needs SAME owner and completed Font retirement");
        host.text_binding = Impl::TextBinding::detach_entered;
        // Producer compares saved identities and calls the typed owner's closed/drained guard.
        // Do not query a retired image or dereference any Font/name/header payload.
        host.native.detach_raw_text_owner_after_parent_drain(owner);
        if (host.native_failed || host.text_binding != Impl::TextBinding::detach_entered || host.raw_owner != &owner)
            throw std::logic_error("Native Font raw host detach cannot resume after a caught nested failure");
        host.raw_owner = nullptr;
        host.raw_registry_image = nullptr;
        host.exported_fonts = nullptr;
        host.text_binding = Impl::TextBinding::detached;
    } catch (...) {
        host.native_failed = true;
        host.native.fail_raw_text_owner_link(owner);
        throw;
    }
}

bool GameFontHost::owns_page_font_producer(const GameNativeFontApplication& producer) const noexcept {
    return &impl_->native == &producer;
}

void GameFontHost::latch_page_text_owner_failure(GameNativeGuiPageApplication& attempted) noexcept {
    auto& host = *impl_;
    if (!host.page_failure) host.page_failure = std::current_exception();
    host.native_failed = true;
    host.page_binding = Impl::PageBinding::failed;
    if (!host.page_owner) host.page_owner = &attempted;
}

void GameFontHost::fail_page_text_owner_link(GameNativeGuiPageApplication& page) noexcept {
    latch_page_text_owner_failure(page);
    impl_->native.fail_page_text_owner_link(*this, page);
}

GameNativeGuiPageApplication* GameFontHost::page_text_owner() const noexcept {
    return impl_->page_owner; // Identity only, including an interrupted attempted link.
}

void GameFontHost::bind_page_text_owner(GameNativeGuiPageApplication& page) {
    auto& host = *impl_;
    const bool fresh_link = host.page_binding == Impl::PageBinding::none && !host.page_owner;
    if (!host.page_owner) host.page_owner = &page;
    host.page_binding = Impl::PageBinding::bind_entered; // Before App/admission/export.
    try {
        if (!fresh_link || !host.initialized || !host.native.ready() || host.native_failed ||
            host.native_retirement_attempted || host.native_retired || host.raw_owner ||
            host.text_binding != Impl::TextBinding::unexported || host.exported_fonts)
            throw std::logic_error("Page Font host needs one unexported ready actual producer and exclusive Page");
        host.page_fonts = &host.native.bind_page_text_owner(*this, page);
        host.page_registry_image = host.page_fonts->native_registry_image(); // Still ready; no lookup.
        if (host.native_failed || host.page_binding != Impl::PageBinding::bind_entered ||
            host.page_owner != &page || !host.page_registry_image)
            throw std::logic_error("Page Font host binding cannot resume after interruption");
        host.page_binding = Impl::PageBinding::bound;
    } catch (...) { fail_page_text_owner_link(page); throw; }
}

void GameFontHost::prepare_page_fonts_for_drain(GameNativeGuiPageApplication& page) {
    auto& host = *impl_;
    try {
        if (host.native_failed || host.page_binding != Impl::PageBinding::bound || host.page_owner != &page ||
            !host.page_fonts || !host.page_registry_image || !host.initialized || !host.native.ready() ||
            host.native_retirement_attempted || host.native_retired)
            throw std::logic_error("Page Font host preparation needs its exact completed link once");
        host.page_binding = Impl::PageBinding::prepare_entered;
        host.native.prepare_page_fonts_for_drain(*this, page);
        if (host.native_failed || host.page_binding != Impl::PageBinding::prepare_entered || host.page_owner != &page)
            throw std::logic_error("Page Font host preparation cannot resume after interruption");
        host.page_binding = Impl::PageBinding::prepared;
    } catch (...) { fail_page_text_owner_link(page); throw; }
}

void GameFontHost::retire_prepared_page_fonts(GameNativeGuiPageApplication& page,
    const GameNativeGuiPageFontRetirementCut& cut) {
    auto& host = *impl_;
    try {
        if (host.native_failed || host.page_binding != Impl::PageBinding::prepared || host.page_owner != &page ||
            !host.page_fonts || !host.page_registry_image || host.native_retirement_attempted || host.native_retired)
            throw std::logic_error("Prepared Page Font host consumption requires its exact scheduled role");
        page.require_page_font_retirement_cut(*this, host.native, cut);
        page.require_actual_page_font_last_use_closed(cut); // UNBOUND: rejects before Native retirement.
        host.page_binding = Impl::PageBinding::retire_entered;
        host.native_retirement_attempted = true;
        const auto count = host.native.produced_fonts(); // Source production history, not closure.
        host.native.retire_prepared_page_fonts(*this, page, cut);
        for (const auto& row : host.fonts.fonts())
            if (row->native_resources || row->resources)
                throw std::runtime_error("Prepared Page Font retirement left a descriptor binding");
        if (host.native_failed || host.page_binding != Impl::PageBinding::retire_entered || host.page_owner != &page)
            throw std::logic_error("Prepared Page Font host cannot resume after interruption");
        host.native_retired = true;
        host.page_binding = Impl::PageBinding::retired_await_parent;
        host.log.notef("native Page fonts retired: fonts=%zu registry_bindings=cleared in_actual_gui_cut=1", count);
    } catch (...) { fail_page_text_owner_link(page); throw; }
}

bool GameFontHost::page_font_retired_awaiting_parent(const GameNativeGuiPageApplication& page) const noexcept {
    const auto& host = *impl_;
    return !host.native_failed && host.page_owner == &page && host.page_fonts && host.page_registry_image &&
        host.page_binding == Impl::PageBinding::retired_await_parent && host.native_retired &&
        host.native_retirement_attempted && host.native.page_text_owner_awaits_detach(*this, page);
}

void GameFontHost::detach_page_text_owner_after_parent_drain(GameNativeGuiPageApplication& page) {
    auto& host = *impl_;
    try {
        if (!page_font_retired_awaiting_parent(page))
            throw std::logic_error("Page Font host detach requires exact completed retirement and parent closure");
        host.page_binding = Impl::PageBinding::detach_entered;
        host.native.detach_page_text_owner_after_parent_drain(*this, page);
        if (host.native_failed || host.page_binding != Impl::PageBinding::detach_entered || host.page_owner != &page)
            throw std::logic_error("Page Font host detach cannot resume after interruption");
        host.page_owner = nullptr; host.page_fonts = nullptr; host.page_registry_image = nullptr;
        host.page_binding = Impl::PageBinding::detached;
    } catch (...) { fail_page_text_owner_link(page); throw; }
}

void GameFontHost::retire_native_fonts() {
    auto& host = *impl_;
    if (host.page_owner) {
        try { throw std::logic_error("Bound Page Fonts must await the prepared actual GUI retirement cut"); }
        catch (...) { fail_page_text_owner_link(*host.page_owner); throw; }
    }
    if (host.native_retired) return;
    if (host.raw_owner) {
        try {
            if (host.native_failed || host.native_retirement_attempted ||
                host.text_binding == Impl::TextBinding::export_pending ||
                host.text_binding == Impl::TextBinding::bind_entered ||
                host.text_binding == Impl::TextBinding::detach_entered || host.text_binding == Impl::TextBinding::detached)
                throw std::logic_error("Native font host cannot replay an interrupted operation");
        } catch (...) {
            host.native_failed = true;
            host.native.fail_raw_text_owner_link(*host.raw_owner);
            throw;
        }
    } else {
        if (host.native_failed || host.native_retirement_attempted ||
            host.text_binding == Impl::TextBinding::export_pending ||
            host.text_binding == Impl::TextBinding::bind_entered ||
            host.text_binding == Impl::TextBinding::detach_entered || host.text_binding == Impl::TextBinding::detached)
            throw std::logic_error("Native font host cannot replay an interrupted operation");
    }
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
        if (host.raw_owner) host.native.fail_raw_text_owner_link(*host.raw_owner);
        throw;
    }
}
bool GameFontHost::native_operation_interrupted() const noexcept {
    const auto& host = *impl_;
    const bool page_interrupted = host.page_binding == Impl::PageBinding::bind_entered ||
        host.page_binding == Impl::PageBinding::prepare_entered || host.page_binding == Impl::PageBinding::retire_entered ||
        host.page_binding == Impl::PageBinding::detach_entered || host.page_binding == Impl::PageBinding::failed;
    const bool page_awaiting_detach = host.page_owner && page_font_retired_awaiting_parent(*host.page_owner);
    const bool raw_awaiting_detach = host.native_retired && host.raw_owner &&
        host.text_binding == Impl::TextBinding::bound && host.native.raw_text_owner_awaits_detach(host.raw_owner);
    return host.native_failed || page_interrupted ||
        host.text_binding == Impl::TextBinding::export_pending ||
        host.text_binding == Impl::TextBinding::bind_entered ||
        host.text_binding == Impl::TextBinding::detach_entered ||
        (host.native_retirement_attempted && !host.native_retired) ||
        (!raw_awaiting_detach && !page_awaiting_detach && !host.native.ready() && host.native.requires_process_retention());
}
FontRegistryStartup& GameFontHost::registry() noexcept { return impl_->fonts; }
const FingerprintPayload& GameFontHost::fingerprint() const noexcept { return impl_->fingerprint; }
std::size_t GameFontHost::resource_opens() const noexcept { return impl_->opens + impl_->native.produced_fonts(); }
}
