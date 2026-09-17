#pragma once

#include "bsp/native_weak_owner.hpp"
#include <mutex>

namespace bsp::game {

// Process storage for the neighboring 0109CE90 publication and 0109CE94 pool.
// Every consumer borrows the common E188B4 allocator list. Native CE1040 owns
// pool destruction; this companion's C++ destructor performs bookkeeping only.
class GameNativeWeakPoolProcess final {
public:
    GameNativeWeakPoolProcess(const GameNativeWeakPoolProcess&) = delete;
    GameNativeWeakPoolProcess& operator=(const GameNativeWeakPoolProcess&) = delete;

    // Explicit startup, once per process. Preserve the native atexit result;
    // a registration failure leaves the initialized pool without a callback.
    // A throwing first attempt is not retried.
    int initialize_once_00cd8a60();
    NativeWeakHandlePool& pool_0109ce94();
    NativeWeakMutexOwner* volatile& lock_publication_0109ce90() noexcept {
        return lock_publication_0109ce90_;
    }

private:
    friend GameNativeWeakPoolProcess& game_native_weak_pool_process();
    GameNativeWeakPoolProcess();
    ~GameNativeWeakPoolProcess() = default;
    enum class StartupState { unattempted, returned, threw };
    NativeWeakMutexOwner* volatile lock_publication_0109ce90_{};
    NativeWeakPoolStorage storage_0109ce94_{};
    NativeWeakHandlePool pool_;
    std::mutex startup_mutex_;
    StartupState startup_state_{StartupState::unattempted};
    int registration_status_{};
};

// Complete static construction before registering CE1040, so that callback
// runs while both this companion and the shared allocator-list owner exist.
GameNativeWeakPoolProcess& game_native_weak_pool_process();

} // namespace bsp::game
