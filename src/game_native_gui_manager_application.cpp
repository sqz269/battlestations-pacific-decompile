#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace bsp::game {
namespace {
std::uint32_t word_at(const void* p, std::size_t offset) noexcept {
    std::uint32_t value; std::memcpy(&value, static_cast<const char*>(p) + offset, sizeof value); return value;
}
// The manager's page vector (+14h) stays empty while the bridge owns every
// page, so 00AA6340 never reaches a page virtual. Reaching one means a page
// entered the vector without a page owner; there is no substitute body.
struct NoPageOwner final : NativeGuiPageLifetimeHost {
    void call_current_page_virtual_20(void*) override {
        throw std::logic_error("native GUI manager drain reached page virtual +20h without a page owner");
    }
    void call_current_page_virtual_04(void*, std::uint32_t) override {
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

bool GameNativeGuiManagerApplication::requires_process_retention() const noexcept {
    return impl_->phase == Impl::Phase::constructing || impl_->phase == Impl::Phase::failed;
}

void GameNativeGuiManagerApplication::after_singleton_drain() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::prepared || p.phase == Impl::Phase::drained) return;
    if (p.phase != Impl::Phase::published || p.gui_00f8bc5c)
        throw std::logic_error("native GUI manager survived or interrupted the shared drain");
    p.phase = Impl::Phase::drained;
    p.log.note("native GUI manager drained: 00aa6540/00aa6340 cleared 00f8bc5c");
}
} // namespace bsp::game
