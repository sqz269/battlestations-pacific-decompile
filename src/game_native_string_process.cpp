#include "bsp/game_native_string_process.hpp"

namespace bsp::game {
void* volatile& GameNativeStringProcess::pending_registry_00f878cc() noexcept {
    return pending_registry_00f878cc_;
}

void* volatile& GameNativeStringProcess::allocation_stats_0109cefc() noexcept {
    return allocation_stats_0109cefc_;
}

NativeParticleModelManagerStorage* volatile&
GameNativeStringProcess::particle_manager_00f8c274() noexcept {
    return particle_manager_00f8c274_;
}

bool GameNativeStringProcess::claim_particle_manager_domain() noexcept {
    if (particle_manager_domain_claimed_) return false;
    particle_manager_domain_claimed_ = true;
    return true;
}

void* volatile& GameNativeStringProcess::application_00e1ae90() noexcept {
    return application_00e1ae90_;
}

bool GameNativeStringProcess::claim_application_domain() noexcept {
    if (application_domain_claimed_) return false;
    application_domain_claimed_ = true;
    return true;
}

void* volatile& GameNativeStringProcess::movie_factory_00f8aefc() noexcept {
    return movie_factory_00f8aefc_;
}

void* volatile& GameNativeStringProcess::locale_manager_00f8bc4c() noexcept {
    return locale_manager_00f8bc4c_;
}

bool GameNativeStringProcess::claim_locale_manager_domain() noexcept {
    if (locale_manager_domain_claimed_) return false;
    locale_manager_domain_claimed_ = true;
    return true;
}

const char* GameNativeStringProcess::property_empty_00e177e4() const noexcept {
    return &property_empty_00e177e4_;
}

const char* GameNativeStringProcess::query_empty_00e17654() const noexcept {
    return &query_empty_00e17654_;
}

const char* GameNativeStringProcess::clone_empty_00f89450() const noexcept {
    return &clone_empty_00f89450_;
}

const char* GameNativeStringProcess::library_empty_00e18560() const noexcept {
    return &library_empty_00e18560_;
}

const char* GameNativeStringProcess::enum_empty_00e186ed() const noexcept {
    return &enum_empty_00e186ed_;
}

GameNativeStringProcess& game_native_string_process() {
    static auto* const process = new GameNativeStringProcess;
    return *process;
}
} // namespace bsp::game
