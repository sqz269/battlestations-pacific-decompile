#pragma once

#include "bsp/native_weak_owner.hpp"

#include <cstddef>
#include <mutex>

namespace bsp::game {

// Process storage for the source projections of F8D2D0 and F8D344. Both use
// the application's existing E188B4 allocator list. Each explicit native
// startup registers its own real CRT callback; C++ destruction does no drain.
class GameNativeParticlePoolProcess final {
public:
    GameNativeParticlePoolProcess(const GameNativeParticlePoolProcess&) = delete;
    GameNativeParticlePoolProcess& operator=(const GameNativeParticlePoolProcess&) = delete;

    // One attempt per owner. Return the original atexit status on subsequent
    // calls; a thrown attempt cannot be retried. Registration failure retains
    // the initialized native pool and does not install substitute cleanup.
    int initialize_model_once_00cd7830();
    int initialize_parameters_once_00cd78b0();

    // Require that the corresponding explicit startup returned. Keep all
    // native clients inside this process lifetime and drain them before exit.
    void* model_pool_00f8d2d0();
    NativeWeakHandlePool& parameter_pool_00f8d344();

private:
    friend GameNativeParticlePoolProcess& game_native_particle_pool_process();
    GameNativeParticlePoolProcess();
    ~GameNativeParticlePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    AllocatorListDomain& list_;
    alignas(NativeWeakPoolStorage) std::byte model_storage_00f8d2d0_[0x38]{};
    NativeWeakPoolStorage parameter_storage_00f8d344_{};
    NativeWeakHandlePool parameters_;
    std::mutex startup_mutex_;
    StartupState model_state_{StartupState::unattempted};
    StartupState parameter_state_{StartupState::unattempted};
    int model_registration_status_{};
    int parameter_registration_status_{};
};

// Finish constructing the shared allocator-list owner and this bookkeeping
// object before either native startup registers its atexit callback.
GameNativeParticlePoolProcess& game_native_particle_pool_process();

} // namespace bsp::game
