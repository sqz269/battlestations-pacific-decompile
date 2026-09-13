#pragma once

#include "bsp/observer_dispatch_owner.hpp"

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;

// Application publication cells for the actual observer owners. This source
// context is owned by GameSingletonHost and outlives its raw manager drain.
// Native owners are created/deleted only by the recovered routines; this
// wrapper has no independent lifetime domain or implicit native destructor.
class GameObserverRuntime final : private ObserverLifetimeServices {
public:
    GameObserverRuntime(GameSingletonHost&, GameHostLog&) noexcept;
    GameObserverRuntime(const GameObserverRuntime&) = delete;
    GameObserverRuntime& operator=(const GameObserverRuntime&) = delete;

    // Invoke once in the represented CRT sequence, before CD2D80 settings.
    // Actual table slots CE2BAC/CE3054 establish this relative order.
    void initialize_dispatch_00ccd6a0();
    NativeObserverLifetime& lifetime() noexcept { return lifetime_; }
    NativeObserverDispatchStorage* volatile& dispatch_publication() noexcept {
        return dispatch_00e198e4_;
    }
    // Diagnostic reads only. Do not dereference the alias after native drain.
    void record_after_singleton_drain();

private:
    void delete_edge_virtual_00(NativeObserverEdgeStorage&,
        std::uint32_t native_vtable, std::uint32_t flags) override;
    void invalid_parameter_00bf6713() override;

    GameHostLog& log_;
    SoundLifetimeAccess manager_;
    NativeObserverDispatchOwner* volatile owner_00e198dc_{nullptr};
    NativeObserverLockOwner* volatile lock_00e198e0_{nullptr};
    NativeObserverDispatchStorage* volatile dispatch_00e198e4_{nullptr};
    NativeObserverLifetime lifetime_;
    std::uintptr_t initialized_alias_{}; // diagnostics, not native state
};
} // namespace bsp::game
