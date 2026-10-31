#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp::game {
namespace {
std::uint32_t word_at(const void* p, std::size_t offset) noexcept {
    std::uint32_t value; std::memcpy(&value, static_cast<const char*>(p) + offset, sizeof value); return value;
}
// The manager's page vector (+14h) stays empty while the bridge owns every
// page. A later native page application may borrow these two dispatch sites;
// until it binds, reaching either retains the original NoPageOwner rejection.
struct NoPageOwner final : NativeGuiPageLifetimeHost {
    NativeGuiPageLifetimeHost* delegate{};
    void call_current_page_virtual_20(void* page) override {
        if (delegate) { delegate->call_current_page_virtual_20(page); return; }
        throw std::logic_error("native GUI manager drain reached page virtual +20h without a page owner");
    }
    void call_current_page_virtual_04(void* page, std::uint32_t flags) override {
        if (delegate) { delegate->call_current_page_virtual_04(page, flags); return; }
        throw std::logic_error("native GUI manager drain reached page virtual +04h without a page owner");
    }
};
} // namespace

struct GameNativeGuiManagerApplication::Impl {
    enum class Phase { prepared, constructing, published, drained, failed };
    GameHostLog& log;
    GameSingletonHost& singletons;
    // Process cells owned here and kept through the shared drain. Only the GUI
    // cell is written; 004C1710's media cell is part of the same context type.
    void* volatile gui_00f8bc5c{nullptr};
    void* volatile media_00f8aef8{nullptr};
    NativePlatformFocusOwnersContext owners;
    NoPageOwner pages;
    NativeGuiMediaFocusLifetimeContext lifetime;
    Phase phase{Phase::prepared};
    PageLifetimeBinding* page_binding{};
    bool page_binding_used{};
    NativeGuiSelectedManagerDeleteAttempt* selected_manager_delete{}; // owned by real singleton host
    bool selected_preparation_started{};
    bool selected_preparation_failed{};
    Impl(GameHostLog& l, GameSingletonHost& s, GameNativeReadOnlyData& data,
        NativeRenderActualOwners& resources, NativeStringRawPoolContext& strings)
        : log(l), singletons(s),
          owners{s.manager_publication_01090aa0(), gui_00f8bc5c, media_00f8aef8,
              *static_cast<const volatile std::uint32_t*>(data.data_at(0xce3800, 4)),
              *static_cast<const volatile std::uint32_t*>(data.data_at(0xd7a24c, 4)), resources},
          lifetime{owners, strings, pages} {
        if (s.native_deletion_bindings().native_gui)
            throw std::logic_error("native GUI manager deletion arm already has an owner");
        s.native_deletion_bindings().native_gui = &lifetime;
    }
};

GameNativeGuiManagerApplication::GameNativeGuiManagerApplication(GameHostLog& log,
    GameSingletonHost& singletons, GameNativeReadOnlyData& data,
    NativeRenderActualOwners& resources, NativeStringRawPoolContext& strings)
    : impl_(std::make_unique<Impl>(log, singletons, data, resources, strings)) {}
GameNativeGuiManagerApplication::~GameNativeGuiManagerApplication() {
    auto& p = *impl_;
    if (p.page_binding || p.pages.delegate) std::terminate();
    if (p.singletons.native_deletion_bindings().native_gui == &p.lifetime)
        p.singletons.native_deletion_bindings().native_gui = nullptr;
}

void* GameNativeGuiManagerApplication::get_004c12b0() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::published) return get_native_gui_manager_004c12b0(p.owners);
    if (p.phase != Impl::Phase::prepared)
        throw std::logic_error("native GUI manager is not available in this phase");
    p.phase = Impl::Phase::constructing;
    try {
        void* const manager = get_native_gui_manager_004c12b0(p.owners);
        if (!manager || manager != p.gui_00f8bc5c)
            throw std::logic_error("004C12B0 did not publish its constructed manager");
        p.phase = Impl::Phase::published;
        p.log.notef("native GUI manager published: 00f8bc5c=%p vtable=%08x registered=01090aa0 "
            "pages=%u deletion=00aa6540", manager, static_cast<unsigned>(word_at(manager, 0)),
            static_cast<unsigned>((word_at(manager, 0x1c) - word_at(manager, 0x18)) / 4));
        return manager;
    } catch (...) { p.phase = Impl::Phase::failed; throw; }
}

NativePlatformFocusOwnersContext& GameNativeGuiManagerApplication::context() noexcept {
    return impl_->owners;
}

void GameNativeGuiManagerApplication::prepare_cold_selected_manager_delete() {
    auto& p = *impl_; // Impl is already completely constructed and owned.
    const bool fresh = !p.selected_preparation_started;
    p.selected_preparation_started = true; // sticky before throwing cold admission
    try {
        if (!fresh || p.selected_manager_delete || p.phase != Impl::Phase::published ||
            p.singletons.native_deletion_bindings().native_gui != &p.lifetime)
            throw std::logic_error("GUI selected drain prepares once against the genuine published context");
        p.selected_manager_delete = &p.singletons.prepare_gui_manager_delete_attempt(p.lifetime);
    } catch (...) {
        p.selected_preparation_failed = true;
        throw; // actual Host pre-drain retention fence, no cold Native cleanup
    }
}

std::unique_ptr<GameNativeGuiManagerApplication::PageLifetimeBinding>
GameNativeGuiManagerApplication::bind_page_lifetime_host(NativeGuiPageLifetimeHost& provider) {
    auto& p = *impl_;
    // AA5D70 initializes the +14h vector's begin/end/capacity at +18/+1C/+20.
    // Requiring all three null also rejects binding after an emptied page set.
    if (p.phase != Impl::Phase::published || !p.gui_00f8bc5c ||
        p.page_binding_used || p.page_binding || p.pages.delegate || &provider == &p.pages ||
        word_at(p.gui_00f8bc5c, 0x18) || word_at(p.gui_00f8bc5c, 0x1c) ||
        word_at(p.gui_00f8bc5c, 0x20))
        throw std::logic_error("native GUI page lifetime binds once to the warm manager before page allocation");
    // Allocate the host token before publishing its borrowed callback. Failure
    // leaves the original rejection intact and changes no native field.
    auto binding = std::unique_ptr<PageLifetimeBinding>(new PageLifetimeBinding(*this));
    p.page_binding = binding.get();
    p.page_binding_used = true;
    p.pages.delegate = &provider;
    return binding;
}

GameNativeGuiManagerApplication::PageLifetimeBinding::~PageLifetimeBinding() noexcept {
    owner_->release_page_lifetime_binding(*this);
}

void GameNativeGuiManagerApplication::release_page_lifetime_binding(PageLifetimeBinding& binding) noexcept {
    auto& p = *impl_;
    if (binding.owner_ != this || p.page_binding != &binding || !p.pages.delegate ||
        p.phase != Impl::Phase::drained || p.gui_00f8bc5c)
        std::terminate();
    p.pages.delegate = nullptr;
    p.page_binding = nullptr;
    binding.owner_ = nullptr;
}

bool GameNativeGuiManagerApplication::requires_process_retention() const noexcept {
    if (impl_->selected_preparation_failed) return true;
    if (impl_->selected_manager_delete &&
        impl_->selected_manager_delete->requires_source_retention()) return true;
    return impl_->phase == Impl::Phase::constructing || impl_->phase == Impl::Phase::failed;
}

void GameNativeGuiManagerApplication::after_singleton_drain() {
    auto& p = *impl_;
    if (p.selected_manager_delete && p.phase == Impl::Phase::drained &&
        p.selected_manager_delete->requires_source_retention())
        throw std::logic_error("selected GUI drain cannot replay an incomplete Source fence");
    if (p.phase == Impl::Phase::prepared || p.phase == Impl::Phase::drained) return;
    if (p.phase != Impl::Phase::published || p.gui_00f8bc5c)
        throw std::logic_error("native GUI manager survived or interrupted the shared drain");
    p.phase = Impl::Phase::drained;
    p.log.note("native GUI manager drained: 00aa6540/00aa6340 cleared 00f8bc5c");
    if (p.selected_manager_delete)
        p.singletons.finish_gui_manager_delete_attempt(*p.selected_manager_delete);
}
} // namespace bsp::game
