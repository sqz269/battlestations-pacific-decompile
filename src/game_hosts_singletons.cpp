#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"

#include "bsp/native_gameplay_effect_construction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp::game {

GameSingletonHost::GameSingletonHost(GameHostLog& log)
    : log_(log), deletion_bindings_{&effect_publication_00f87664_, nullptr},
      observers_(std::make_unique<GameObserverRuntime>(*this, log)) {
    // Admit factory+4 before startup can register that exact subobject.
    deletion_bindings_.game_resource_factory = &game_resource_factory_context_;
}

GameSingletonHost::~GameSingletonHost() {
    // Normal WinMain shutdown has already drained this owner. All publication
    // cells and source bindings remain valid through this fallback call.
    shutdown();
}

SoundLifetimeAccess GameSingletonHost::sound_lifetime() noexcept {
    return SoundLifetimeAccess(manager_publication_01090aa0_);
}
void GameSingletonHost::bind_sound_runtime(GameSoundRuntime* runtime) noexcept {
    deletion_bindings_.sound_runtime = runtime;
}
void GameSingletonHost::bind_xlive_owner(XLiveOwnerAllocation* owner) noexcept {
    deletion_bindings_.xlive_owner = owner;
}
void GameSingletonHost::bind_input_backend(NativeInputBackendOwnerContext* context) noexcept {
    deletion_bindings_.input_backend = context;
}
void GameSingletonHost::bind_input_actions(NativeInputActionOwnerContext* context) noexcept {
    deletion_bindings_.input_actions = context;
}
void GameSingletonHost::bind_input_settings(NativeInputSettingsLifetimeContext* context) noexcept {
    deletion_bindings_.input_settings = context;
}
void GameSingletonHost::bind_observer_lifetime(NativeObserverLifetime* lifetime) noexcept {
    deletion_bindings_.observer_lifetime = lifetime;
}

void GameSingletonHost::bind_observer_dispatch_owner(
    NativeObserverDispatchOwner* volatile* publication) noexcept {
    deletion_bindings_.actual_observer_dispatch_owner_00e198dc = publication;
}

void GameSingletonHost::publish_game_resource_factory_008f840b() {
    game_resource_factory_alias_00f8d31c_ =
        get_native_game_resource_factory_007175d0(game_resource_factory_context_);
    log_.notef("game resource factory published: owner=%p alias=%p storage=raw8h",
        game_resource_factory_publication_00e19b90_, game_resource_factory_alias_00f8d31c_);
}

void GameSingletonHost::probe_gameplay_effect_memory(const char* label) {
    void* const owner = get_native_gameplay_effect_manager_004c1650(
        manager_publication_01090aa0_, effect_publication_00f87664_);
    probe_native_gameplay_effect_registry_0086b0b0(owner, nullptr, label);
}

void GameSingletonHost::shutdown() {
    void* const manager = manager_publication_01090aa0_;
    if (manager == nullptr) return;
    const std::uint32_t registered = count_native_singleton_slots_00bcf910(manager, nullptr);
    destroy_native_singleton_manager_00bd0400(manager, deletion_bindings_);
    observers_->record_after_singleton_drain();
    singleton_lifetime_free(manager);
    manager_publication_01090aa0_ = nullptr;
    log_.notef("singleton lifetime drained: registered_slots=%u effect_publication=%s "
        "manager_publication=%s factory_publication=%s factory_alias=%p "
        "storage=raw14h/raw10h/raw8h", registered,
        effect_publication_00f87664_ == nullptr ? "null" : "non-null",
        manager_publication_01090aa0_ == nullptr ? "null" : "non-null",
        game_resource_factory_publication_00e19b90_ == nullptr ? "null" : "non-null",
        game_resource_factory_alias_00f8d31c_);
}
} // namespace bsp::game
