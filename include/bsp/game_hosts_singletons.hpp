#pragma once
#include "bsp/sound_lifetime_access.hpp"

#include "bsp/native_singleton_destruction.hpp"

namespace bsp::game {
class GameHostLog;

// One application-owned raw14h manager and raw10h gameplay-effect owner.
// Private publications outlive the menu and all shutdown calls. Admitted
// registrations use recovered fixed deletion profiles. Sound binds its
// concrete runtime before registering and keeps it alive through the drain.
class GameSingletonHost final {
public:
    explicit GameSingletonHost(GameHostLog&);
    ~GameSingletonHost();
    GameSingletonHost(const GameSingletonHost&) = delete;
    GameSingletonHost& operator=(const GameSingletonHost&) = delete;

    void probe_gameplay_effect_memory(const char* label);
    SoundLifetimeAccess sound_lifetime() noexcept;
    void bind_sound_runtime(GameSoundRuntime*) noexcept;
    // Install stable borrowed services before registration. They remain valid
    // until shutdown returns; scalar flags1 consumes the online allocation.
    // Input callers retain SDK references until all raw device owners finish.
    void bind_xlive_owner(XLiveOwnerAllocation*) noexcept;
    void bind_input_backend(NativeInputBackendOwnerContext*) noexcept;
    void bind_input_actions(NativeInputActionOwnerContext*) noexcept;
    // Borrow the observer lifetime before its lock registers. It must use this
    // host's actual publication access and remain alive until shutdown returns.
    void bind_observer_lifetime(NativeObserverLifetime*) noexcept;
    // Borrow the dispatch owner's actualE198DC cell before its CRT initializer
    // registers. Observer cleanup must finish before shutdown frees the owner;
    // its nativeE198E4 alias remains dangling after the drain.
    void bind_observer_dispatch_owner(NativeObserverDispatchOwner* volatile*) noexcept;
    // 008F8449: capture current manager, rawBD0400 drain, free captured manager,
    // then clear its actual publication, while all bindings remain alive.
    void shutdown();

private:
    GameHostLog& log_;
    void* volatile manager_publication_01090aa0_{nullptr};
    void* volatile effect_publication_00f87664_{nullptr};
    NativeSingletonDeletionBindings deletion_bindings_;
};
} // namespace bsp::game
