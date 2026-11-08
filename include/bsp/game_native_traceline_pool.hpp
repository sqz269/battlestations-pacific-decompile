#pragma once

#include "bsp/native_traceline_pool.hpp"

#include <mutex>

namespace bsp::game {

// Canonical Source storage for F8C288, the 1BCh Traceline pool. The 7B0h
// Tracer owner is distinct. CD77F0 uses the application's E188B4 list and
// registers the actual CE0B70 callback; C++ destruction performs no drain.
class GameNativeTracelinePoolProcess final {
public:
    GameNativeTracelinePoolProcess(const GameNativeTracelinePoolProcess&) = delete;
    GameNativeTracelinePoolProcess& operator=(const GameNativeTracelinePoolProcess&) = delete;

    // Attempt native startup once. Preserve atexit's return value, including
    // failure, and do not retry an attempt that threw after partial publication.
    int initialize_once();
    // Borrow only after explicit startup returned; finish payloads and return
    // every native slot before the real CRT exit callback destroys this pool.
    void* pool_storage();

private:
    friend GameNativeTracelinePoolProcess& game_native_traceline_pool_process();
    GameNativeTracelinePoolProcess();
    ~GameNativeTracelinePoolProcess() = default;

    enum class StartupState { unattempted, returned, threw };
    AllocatorListDomain& list_;
    alignas(4) std::byte storage_00f8c288_[native_traceline_pool_bytes]{};
    std::mutex startup_mutex_;
    StartupState state_{StartupState::unattempted};
    int registration_status_{};
};

// Construct the shared allocator domain and stable storage before registering
// native cleanup, so both outlive CE0B70 under the CRT's reverse exit order.
GameNativeTracelinePoolProcess& game_native_traceline_pool_process();

} // namespace bsp::game
