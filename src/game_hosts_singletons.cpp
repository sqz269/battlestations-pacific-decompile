#include "bsp/game_hosts_singletons.hpp"
#include <stdexcept>
#include "bsp/native_resource_registry_scalar_delete.hpp"
#include "bsp/game_native_input_settings_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_locale_manager.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_weak_pool.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_native_vfs_runtime.hpp"

#include "bsp/native_gameplay_effect_construction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_texture_atlas_manager_delete_attempt.hpp"
#include "bsp/native_texture_atlas_deletion_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp::game {

GameSingletonHost::GameSingletonHost(GameHostLog& log)
    : log_(log), manager_publication_01090aa0_(game_native_string_process().manager_01090aa0()),
      input_settings_publication_00e198e8_(game_native_input_settings_process().publication_00e198e8()),
      deletion_bindings_{&effect_publication_00f87664_, nullptr},
      observers_(std::make_unique<GameObserverRuntime>(*this, log)),
      pending_registry_publication_00f878cc_(game_native_string_process().pending_registry_00f878cc()),
      allocation_stats_context_{manager_publication_01090aa0_,
          game_native_string_process().allocation_stats_0109cefc()} {
    // Admit factory+4 before startup can register that exact subobject.
    deletion_bindings_.game_resource_factory = &game_resource_factory_context_;
    deletion_bindings_.resource_support = &resource_support_context_;
    deletion_bindings_.actual_diagnostic_publication_0109cf14 = &diagnostic_publication_0109cf14_;
    deletion_bindings_.actual_pending_registry_publication_00f878cc =
        &pending_registry_publication_00f878cc_;
    deletion_bindings_.allocation_stats = &allocation_stats_context_;
    deletion_bindings_.actual_loading_queue_publication_00e18d4c =
        &loading_queue_publication_00e18d4c_;
    // Bind before the fixed-step getter registers CE3CEC. The permanent
    // process cell and this dispatch table both survive normal/fallback drain.
    deletion_bindings_.actual_spatial_index_publication_00f8a0d8 =
        &game_native_string_process().spatial_index_00f8a0d8();
    deletion_bindings_.actual_path_leg_manager_publication_00f8a720 =
        &game_native_string_process().path_leg_manager_00f8a720();
}

GameSingletonHost::~GameSingletonHost() {
    // Normal WinMain shutdown has already drained this owner. All publication
    // cells and source bindings remain valid through this fallback call.
    shutdown();
}

SoundLifetimeAccess GameSingletonHost::sound_lifetime() noexcept {
    return SoundLifetimeAccess(manager_publication_01090aa0_);
}
void* volatile& GameSingletonHost::movie_factory_publication_00f8aefc() noexcept {
    return game_native_string_process().movie_factory_00f8aefc();
}
NativeAllocationStatsConstructorContext& GameSingletonHost::allocation_stats_context() noexcept {
    return allocation_stats_context_;
}
void GameSingletonHost::bind_particle_manager_domain(GameNativeReadOnlyData& data) {
    auto& process = game_native_string_process();
    auto& publication = process.particle_manager_00f8c274();
    const auto* base_profile = static_cast<const volatile std::uint32_t*>(
        data.data_at(0x00d5d7ecu, sizeof(std::uint32_t)));
    const auto* derived_profile = static_cast<const volatile std::uint32_t*>(
        data.data_at(0x00d5d7f8u, sizeof(std::uint32_t)));
    const auto& one = *static_cast<const volatile std::uint32_t*>(
        data.data_at(0x00d7a24cu, sizeof(std::uint32_t)));
    if (*base_profile != 0x00af0870u || *derived_profile != 0x00af1080u)
        throw std::logic_error("particle manager deletion profiles do not match the verified domain");
    if (particle_manager_context_) {
        auto& context = *particle_manager_context_;
        if (&context.manager_00f8c274 != &publication ||
            !context.lifetime_01090aa0.uses_actual_storage() ||
            !context.lifetime_01090aa0.borrows_same_domain(manager_publication_01090aa0_) ||
            &context.one_00d7a24c != &one || deletion_bindings_.particle_manager != &context)
            throw std::logic_error("particle manager is already bound to another application domain");
        return;
    }
    // These rejection checks do not prove the external valid/quiescent/no-prior-
    // attempt precondition and do not inspect or repair an unknown registry.
    if (publication != nullptr || deletion_bindings_.particle_manager != nullptr)
        throw std::logic_error("particle manager deletion must bind before publication or registration");
    if (!process.claim_particle_manager_domain())
        throw std::logic_error("particle manager process domain was already claimed by a host");
    // The raw Access constructor is noexcept, stores references only, and does
    // not observe one. Inline emplace and pointer binding cannot allocate an owner.
    particle_manager_context_.emplace(publication, manager_publication_01090aa0_, one);
    deletion_bindings_.particle_manager = &*particle_manager_context_;
}
NativeParticleModelManagerAccess& GameSingletonHost::particle_manager_context() {
    if (!particle_manager_context_ || deletion_bindings_.particle_manager != &*particle_manager_context_)
        throw std::logic_error("particle manager context requires its retained deletion domain");
    return *particle_manager_context_;
}
NativeWeakOwnerDomain& GameSingletonHost::weak_owners() {
    if (!weak_owners_) {
        auto& process = game_native_weak_pool_process();
        weak_owners_ = std::make_unique<NativeWeakOwnerDomain>(sound_lifetime(),
            process.lock_publication_0109ce90(), process.pool_0109ce94());
        deletion_bindings_.weak_owner_domain = weak_owners_.get();
    }
    return *weak_owners_;
}
void GameSingletonHost::bind_resource_registry_domain(
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& validation) {
    if (resource_registry_bindings_) {
        if (&resource_registry_bindings_->strings != &strings ||
            &resource_registry_bindings_->invalid_parameters != &validation ||
            deletion_bindings_.resource_registry != resource_registry_bindings_.get())
            throw std::logic_error("resource registry is already bound to another application domain");
        return;
    }
    if (resource_registry_publication_00f8d41c_ || deletion_bindings_.resource_registry)
        throw std::logic_error("resource registry deletion must bind before publication or registration");
    resource_registry_bindings_ = std::make_unique<NativeResourceRegistryDeleteBindings>(
        NativeResourceRegistryDeleteBindings{resource_registry_publication_00f8d41c_, strings, validation});
    deletion_bindings_.resource_registry = resource_registry_bindings_.get();
}
void* volatile& GameSingletonHost::resource_registry_publication_00f8d41c() {
    if (!resource_registry_bindings_ ||
        deletion_bindings_.resource_registry != resource_registry_bindings_.get())
        throw std::logic_error("resource registry publication requires its retained deletion domain");
    return resource_registry_publication_00f8d41c_;
}
void* volatile& GameSingletonHost::particle_clock_publication_00f8d420() noexcept {
    return particle_clock_publication_00f8d420_;
}
void GameSingletonHost::bind_sound_runtime(GameSoundRuntime* runtime) noexcept {
    deletion_bindings_.sound_runtime = runtime;
}
void GameSingletonHost::bind_vfs_runtime(GameNativeVfsRuntime* runtime) noexcept {
    vfs_runtime_ = runtime;
}
void GameSingletonHost::retire_vfs_after_drain() noexcept {
    auto* const runtime = vfs_runtime_;
    vfs_runtime_ = nullptr;
    if (runtime) runtime->retire_after_shared_drain();
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
void GameSingletonHost::bind_input_settings(NativeInputSettingsLifetimeContext* context) {
    game_native_input_settings_process().bind_context(context);
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

GameNativeLocaleManagerProcess& GameSingletonHost::prepare_locale_manager() {
    auto& owner = game_native_locale_manager_process();
    locale_manager_process_ = &owner;
    owner.bind_once(*this);
    return owner;
}

void GameSingletonHost::shutdown() {
    auto* const atlas = pending_texture_atlas_attempt_; // capture before callbacks
    const bool atlas_fresh = !atlas || !atlas->host_entered_;
    if (atlas) atlas->host_entered_ = true; // BEFORE all admission, callbacks and Native effects
    try {
    if (atlas && (!atlas_fresh || atlas->unknown_ || !atlas->cold_bound_ || atlas->host_ != this ||
        atlas->bindings_ != &deletion_bindings_ ||
        deletion_bindings_.texture_atlas != atlas->atlas_))
        throw std::logic_error("atlas shutdown requires the SAME retained actual Host attempt");
    auto* const selected = pending_selected_gui_attempt_; // capture before callbacks
    if (selected) {
        shutdown_selected(*selected, atlas); // separate attempts, SAME immutable Host capture
        if (atlas) atlas->host_returned_ = true;
        return;
    }
    auto* const locale = locale_manager_process_;
    if (locale != nullptr) locale->before_shared_drain(*this);
    void* const manager = manager_publication_01090aa0_;
    if (atlas && manager == nullptr && atlas->registered_receiver_bound_ && !atlas->scalar_returned_)
        throw std::logic_error("atlas Host cannot retire providers without its real registered scalar return");
    if (manager == nullptr) {
        retire_vfs_after_drain();
        if (locale != nullptr) {
            locale->after_shared_drain(*this);
            locale_manager_process_ = nullptr;
        }
        if (atlas) atlas->host_returned_ = true;
        return;
    }
    if (atlas) {
        atlas->outer_receiver_ = manager;
        atlas->receiver_bound_ = true; // actual reached manager, BEFORE count/core effects
    }
    const std::uint32_t registered = count_native_singleton_slots_00bcf910(manager, nullptr);
    if (atlas) destroy_native_singleton_manager_00bd0400(manager, deletion_bindings_, *atlas);
    else destroy_native_singleton_manager_00bd0400(manager, deletion_bindings_);
    retire_vfs_after_drain();
    observers_->record_after_singleton_drain();
    singleton_lifetime_free(manager);
    manager_publication_01090aa0_ = nullptr;
    if (locale != nullptr) {
        locale->after_shared_drain(*this);
        locale_manager_process_ = nullptr;
    }
    log_.notef("singleton lifetime drained: registered_slots=%u effect_publication=%s "
        "manager_publication=%s factory_publication=%s factory_alias=%p "
        "storage=raw14h/raw10h/raw8h", registered,
        effect_publication_00f87664_ == nullptr ? "null" : "non-null",
        manager_publication_01090aa0_ == nullptr ? "null" : "non-null",
        game_resource_factory_publication_00e19b90_ == nullptr ? "null" : "non-null",
        game_resource_factory_alias_00f8d31c_);
    if (atlas) atlas->host_returned_ = true; // ALL actual shutdown work/logging returned
    } catch (...) {
        if (atlas) atlas->unknown_ = true;
        throw; // real StartupHost/WinMain catch retains heap graph; no cleanup authority
    }
}

NativeTextureAtlasManagerDeleteAttempt& GameSingletonHost::prepare_texture_atlas_delete_attempt(
    NativeTextureAtlasDeletionLifetime& application) {
    if (texture_atlas_attempt_) {
        texture_atlas_attempt_->unknown_ = true;
        throw std::logic_error("atlas manager attempt cannot be replaced or replayed");
    }
    texture_atlas_attempt_.reset(new NativeTextureAtlasManagerDeleteAttempt(
        *this, application, deletion_bindings_)); // adopt BEFORE throwing binding admission
    auto& atlas = *texture_atlas_attempt_;
    try {
        if (pending_texture_atlas_attempt_ ||
            deletion_bindings_.texture_atlas != &application)
            throw std::logic_error("atlas manager attempt requires its actual cold application binding");
        atlas.cold_bound_ = true;
        pending_texture_atlas_attempt_ = &atlas;
        return atlas;
    } catch (...) {
        atlas.unknown_ = true;
        throw;
    }
}

void GameSingletonHost::finish_texture_atlas_delete_attempt(NativeTextureAtlasManagerDeleteAttempt& atlas) {
    try {
        if (pending_texture_atlas_attempt_ != &atlas || atlas.host_ != this ||
            atlas.bindings_ != &deletion_bindings_ || atlas.unknown_ || !atlas.host_returned_ ||
            (atlas.receiver_bound_ && !atlas.core_returned_) ||
            (atlas.registered_receiver_bound_ && (!atlas.scalar_entered_ || !atlas.scalar_returned_)))
            throw std::logic_error("atlas source fence requires complete actual shutdown and registered scalar return");
        atlas.source_fence_returned_ = true; // Source observation only, no Native origin credit
        pending_texture_atlas_attempt_ = nullptr;
    } catch (...) {
        atlas.unknown_ = true;
        throw;
    }
}
NativeGuiSelectedManagerDeleteAttempt& GameSingletonHost::prepare_gui_manager_delete_attempt(
    NativeGuiMediaFocusLifetimeContext& context) {
    auto owned = std::unique_ptr<NativeGuiSelectedManagerDeleteAttempt>(
        new NativeGuiSelectedManagerDeleteAttempt(*this, context, deletion_bindings_));
    auto* const selected = owned.get();
    selected_gui_attempts_.push_back(std::move(owned)); // actual stable adoption BEFORE bind
    if (pending_selected_gui_attempt_ || deletion_bindings_.native_gui != &context)
        throw std::logic_error("selected GUI drain requires its unchanged actual singleton host/context");
    selected->cold_bound_ = true;
    pending_selected_gui_attempt_ = selected; // publish only the retained record
    return *selected;
}

void GameSingletonHost::shutdown_selected(NativeGuiSelectedManagerDeleteAttempt& selected,
    NativeTextureAtlasManagerDeleteAttempt* const atlas) {
    const bool fresh = !selected.host_entered_;
    selected.host_entered_ = true; // before admission, callbacks and Native effects
    try {
        if (!fresh || selected.unknown_ || !selected.cold_bound_ || selected.host_ != this ||
            selected.bindings_ != &deletion_bindings_ || pending_selected_gui_attempt_ != &selected)
            throw std::logic_error("selected singleton shutdown requires its actual fresh retained attempt");

    auto* const locale = locale_manager_process_;
    if (locale != nullptr) locale->before_shared_drain(*this);
    void* const manager = manager_publication_01090aa0_;
    if (atlas && manager == nullptr && atlas->registered_receiver_bound_ && !atlas->scalar_returned_)
        throw std::logic_error("atlas Host cannot retire providers without its real registered scalar return");
    if (manager == nullptr) {
        retire_vfs_after_drain();
        if (locale != nullptr) {
            locale->after_shared_drain(*this);
            locale_manager_process_ = nullptr;
        }
        selected.host_returned_ = true;
        return;
    }
    selected.outer_receiver_ = manager;
    selected.receiver_bound_ = true;
    if (atlas) {
        atlas->outer_receiver_ = manager;
        atlas->receiver_bound_ = true; // same actual capture; no GUI attempt repurposing
    }
    const std::uint32_t registered = count_native_singleton_slots_00bcf910(manager, nullptr);
    if (atlas) destroy_native_singleton_manager_00bd0400(manager, deletion_bindings_, selected, *atlas);
    else destroy_native_singleton_manager_00bd0400(manager, deletion_bindings_, selected);
    retire_vfs_after_drain();
    observers_->record_after_singleton_drain();
    singleton_lifetime_free(manager);
    manager_publication_01090aa0_ = nullptr;
    if (locale != nullptr) {
        locale->after_shared_drain(*this);
        locale_manager_process_ = nullptr;
    }
    log_.notef("singleton lifetime drained: registered_slots=%u effect_publication=%s "
        "manager_publication=%s factory_publication=%s factory_alias=%p "
        "storage=raw14h/raw10h/raw8h", registered,
        effect_publication_00f87664_ == nullptr ? "null" : "non-null",
        manager_publication_01090aa0_ == nullptr ? "null" : "non-null",
        game_resource_factory_publication_00e19b90_ == nullptr ? "null" : "non-null",
        game_resource_factory_alias_00f8d31c_);
    selected.host_returned_ = true; // only after ALL actual shutdown work/logging returns
    } catch (...) {
        selected.unknown_ = true;
        throw; // no new cleanup/terminal policy; real Host catch retains/exits
    }
}

void GameSingletonHost::finish_gui_manager_delete_attempt(NativeGuiSelectedManagerDeleteAttempt& selected) {
    if (pending_selected_gui_attempt_ != &selected || selected.host_ != this ||
        selected.unknown_ || !selected.host_returned_ ||
        (selected.receiver_bound_ && !selected.core_returned_))
        throw std::logic_error("selected manager normal mark requires complete shutdown and real after-drain fence");
    selected.source_fence_returned_ = true; // Source observation, never Native custody credit
    pending_selected_gui_attempt_ = nullptr; // private consumed routing borrow only
}

} // namespace bsp::game
