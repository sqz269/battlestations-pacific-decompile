#pragma once
#include "bsp/sound_lifetime_access.hpp"

#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_game_resource_factory.hpp"
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameObserverRuntime;

// One application-owned raw14h manager, raw10h effect owner and raw8h factory.
// Private publications outlive the menu and all shutdown calls. Admitted
// registrations use recovered fixed deletion profiles. Sound binds its
// concrete runtime before registering and keeps it alive through the drain.
class GameSingletonHost final {
public:
    explicit GameSingletonHost(GameHostLog&);
    ~GameSingletonHost();
    GameSingletonHost(const GameSingletonHost&) = delete;
    GameSingletonHost& operator=(const GameSingletonHost&) = delete;

    // 008F840B obtains the factory; 008F8414 stores its separate alias.
    // The owner context and both cells survive the complete raw drain.
    void publish_game_resource_factory_008f840b();
    void probe_gameplay_effect_memory(const char* label);
    SoundLifetimeAccess sound_lifetime() noexcept;
    void bind_sound_runtime(GameSoundRuntime*) noexcept;
    // Install stable borrowed services before registration. They remain valid
    // until shutdown returns; scalar flags1 consumes the online allocation.
    // Input callers retain SDK references until all raw device owners finish.
    void bind_xlive_owner(XLiveOwnerAllocation*) noexcept;
    void bind_input_backend(NativeInputBackendOwnerContext*) noexcept;
    void bind_input_actions(NativeInputActionOwnerContext*) noexcept;
    // Borrow these stable actual-publication cells for the settings context.
    // Both remain valid through shutdown's raw manager drain.
    void* volatile& input_settings_publication_00e198e8() noexcept {
        return input_settings_publication_00e198e8_;
    }
    void* volatile& manager_publication_01090aa0() noexcept {
        return manager_publication_01090aa0_;
    }
    // Install the borrowed settings context before CF81CC can be registered.
    // The context must remain alive until shutdown returns.
    void bind_input_settings(NativeInputSettingsLifetimeContext*) noexcept;
    // Borrow the observer lifetime before its lock registers. It must use this
    // host's actual publication access and remain alive until shutdown returns.
    void bind_observer_lifetime(NativeObserverLifetime*) noexcept;
    // Borrow the dispatch owner's actualE198DC cell before its CRT initializer
    // registers. Observer cleanup must finish before shutdown frees the owner;
    // its nativeE198E4 alias remains dangling after the drain.
    void bind_observer_dispatch_owner(NativeObserverDispatchOwner* volatile*) noexcept;
    // One application observer context, with stable cells through raw drain.
    GameObserverRuntime& observers() noexcept { return *observers_; }
    // 008F8449: capture current manager, rawBD0400 drain, free captured manager,
    // then clear its actual publication, while all bindings remain alive.
    void shutdown();

private:
    GameHostLog& log_;
    void* volatile manager_publication_01090aa0_{nullptr};
    void* volatile input_settings_publication_00e198e8_{nullptr};
    void* volatile effect_publication_00f87664_{nullptr};
    void* volatile game_resource_factory_publication_00e19b90_{nullptr};
    // Native owner deletion clears E19B90, leaving this WinMain alias intact.
    void* volatile game_resource_factory_alias_00f8d31c_{nullptr};
    NativeGameResourceFactoryContext game_resource_factory_context_{
        manager_publication_01090aa0_, game_resource_factory_publication_00e19b90_};
    NativeSingletonDeletionBindings deletion_bindings_;
    std::unique_ptr<GameObserverRuntime> observers_;
};
} // namespace bsp::game
