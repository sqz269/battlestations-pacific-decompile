#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"

#include <cstdlib>
#include <stdexcept>

namespace bsp::game {
namespace {
// Reuse the current CRT validation service. Keep the source call capable of
// returning when a configured handler returns, as the recovered callers do.
__declspec(naked) void __cdecl observer_crt_invalid_parameter() {
    __asm { jmp _invalid_parameter_noinfo }
}
}

GameObserverRuntime::GameObserverRuntime(GameSingletonHost& singletons,
    GameHostLog& log) noexcept
    : log_(log), manager_(singletons.sound_lifetime()),
      lifetime_(manager_, lock_00e198e0_, dispatch_00e198e4_, *this) {
    singletons.bind_observer_lifetime(&lifetime_);
    singletons.bind_observer_dispatch_owner(&owner_00e198dc_);
}

void GameObserverRuntime::initialize_dispatch_00ccd6a0() {
    publish_observer_dispatch_storage_00ccd6a0(
        manager_, owner_00e198dc_, dispatch_00e198e4_);
    initialized_alias_ = reinterpret_cast<std::uintptr_t>(dispatch_00e198e4_);
    log_.implemented("ObserverRuntime::initialize_dispatch", "00ccd6a0");
    log_.notef("observer runtime initialized: dispatch_owner=%s alias=%s lock=%s "
        "manager=application_raw14h storage=raw14h/raw8h",
        owner_00e198dc_ != nullptr ? "present" : "null",
        initialized_alias_ == reinterpret_cast<std::uintptr_t>(owner_00e198dc_) + 4u
            ? "owner+4" : "unexpected",
        lock_00e198e0_ != nullptr ? "present" : "lazy");
}

void GameObserverRuntime::record_after_singleton_drain() {
    log_.notef("observer runtime after raw singleton drain: dispatch_owner=%s "
        "lock_owner=%s alias=%s",
        owner_00e198dc_ == nullptr ? "null" : "non-null",
        lock_00e198e0_ == nullptr ? "null" : "non-null",
        reinterpret_cast<std::uintptr_t>(dispatch_00e198e4_) == initialized_alias_
            ? "retained" : "changed");
}

void GameObserverRuntime::delete_edge_virtual_00(NativeObserverEdgeStorage&,
    std::uint32_t, std::uint32_t) {
    // CF7E64 uses the concrete recovered path inside NativeObserverLifetime.
    // Other edge producers/slot-zero targets are not admitted by this binding.
    throw std::logic_error("observer edge deleting profile is unavailable");
}

void GameObserverRuntime::invalid_parameter_00bf6713() {
    observer_crt_invalid_parameter();
}
} // namespace bsp::game
