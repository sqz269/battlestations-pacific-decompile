#include "bsp/game_native_particle_pools.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_particle_model_pool_owner.hpp"
#include "bsp/native_particle_parameter_pool_owner.hpp"
#include "bsp/native_tracer_pool.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native particle pools require MSVC Win32.
#endif

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeWeakPoolStorage) == 0x38);
static_assert(alignof(std::max_align_t) >= alignof(NativeWeakPoolStorage));
}

GameNativeParticlePoolProcess::GameNativeParticlePoolProcess()
    : list_(game_native_physical_pool_process().allocator_list_domain_00e188b4()),
      parameters_(list_, parameter_storage_00f8d344_) {}

GameNativeParticlePoolProcess& game_native_particle_pool_process() {
    static GameNativeParticlePoolProcess process;
    return process;
}

int GameNativeParticlePoolProcess::initialize_model_once_00cd7830() {
    std::lock_guard lock(startup_mutex_);
    if (model_state_ == StartupState::returned) return model_registration_status_;
    if (model_state_ == StartupState::threw)
        throw std::logic_error("particle model pool startup previously threw");

    model_state_ = StartupState::threw;
    bind_static_native_particle_model_pool_00f8d2d0(model_storage_00f8d2d0_, list_);
    model_registration_status_ = initialize_static_native_particle_model_pool_00cd7830();
    model_state_ = StartupState::returned;
    return model_registration_status_;
}

int GameNativeParticlePoolProcess::initialize_parameters_once_00cd78b0() {
    std::lock_guard lock(startup_mutex_);
    if (parameter_state_ == StartupState::returned) return parameter_registration_status_;
    if (parameter_state_ == StartupState::threw)
        throw std::logic_error("particle parameter pool startup previously threw");

    parameter_state_ = StartupState::threw;
    bind_static_native_particle_parameter_pool_00f8d344(parameters_);
    parameter_registration_status_ = initialize_static_native_particle_parameter_pool_00cd78b0();
    parameter_state_ = StartupState::returned;
    return parameter_registration_status_;
}

int GameNativeParticlePoolProcess::initialize_tracer_once_00cd88d0() {
    std::lock_guard lock(startup_mutex_);
    if (tracer_state_ == StartupState::returned) return tracer_registration_status_;
    if (tracer_state_ == StartupState::threw)
        throw std::logic_error("tracer pool startup previously threw");

    // This process and the shared E188B4 list already have C++ exit callbacks.
    // Register the native callback later so it drains this same owner first.
    // Commit the attempt before binding/construction: never recreate a live CS.
    tracer_state_ = StartupState::threw;
    bind_static_native_tracer_pool_0109049c(tracer_storage_0109049c_, list_);
    tracer_registration_status_ = initialize_static_native_tracer_pool_00cd88d0();
    tracer_state_ = StartupState::returned;
    return tracer_registration_status_;
}

void* GameNativeParticlePoolProcess::model_pool_00f8d2d0() {
    std::lock_guard lock(startup_mutex_);
    if (model_state_ != StartupState::returned)
        throw std::logic_error("particle model pool requires completed explicit startup");
    return model_storage_00f8d2d0_;
}

NativeWeakHandlePool& GameNativeParticlePoolProcess::parameter_pool_00f8d344() {
    std::lock_guard lock(startup_mutex_);
    if (parameter_state_ != StartupState::returned)
        throw std::logic_error("particle parameter pool requires completed explicit startup");
    return parameters_;
}

void* GameNativeParticlePoolProcess::tracer_pool_0109049c() {
    std::lock_guard lock(startup_mutex_);
    if (tracer_state_ != StartupState::returned)
        throw std::logic_error("tracer pool requires completed explicit startup");
    return tracer_storage_0109049c_;
}

} // namespace bsp::game
