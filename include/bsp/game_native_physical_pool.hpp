#pragma once

#include "bsp/allocator_list.hpp"
#include "bsp/native_physical_provider_pool.hpp"

#include <cstddef>
#include <mutex>

namespace bsp::game {

// Process owner for the source projection of 0109DBF0 and the common 00E188B4
// allocator list. Construct it before CD9010 registration; keep every borrower
// (including GameNativeVfsRuntime) within this process lifetime.
class GameNativePhysicalPoolProcess final {
public:
    GameNativePhysicalPoolProcess(const GameNativePhysicalPoolProcess&) = delete;
    GameNativePhysicalPoolProcess& operator=(const GameNativePhysicalPoolProcess&) = delete;

    // Explicit CD9010 startup. One attempt per process; later calls return the
    // original CRT registration status. A thrown first attempt is never retried.
    // Registration failure retains the initialized native pool without rollback.
    int initialize_once_00cd9010();

    // Common application domain, also available before pool startup so other
    // allocator owners can use the same 00E188B4 list.
    AllocatorListDomain& allocator_list_domain_00e188b4() noexcept { return list_; }

    // Retained source context for GameNativeVfsRuntimeInputs. Requires the
    // explicit startup call to have returned. A nonzero registration status
    // still leaves BF3250's pool initialized, without a CRT cleanup callback.
    NativePhysicalProviderPoolContext& physical_provider_pool_context_0109dbf0();

private:
    friend GameNativePhysicalPoolProcess& game_native_physical_pool_process();
    GameNativePhysicalPoolProcess() noexcept;
    ~GameNativePhysicalPoolProcess() = default; // bookkeeping only; CRT owns BF33A0

    enum class StartupState { unattempted, returned, threw };
    alignas(std::max_align_t) std::byte pool_0109dbf0_[0x38]{};
    AllocatorListElement* list_head_00e188b4_{};
    AllocatorListDomain list_;
    NativePhysicalProviderPoolContext context_;
    std::mutex startup_mutex_;
    StartupState startup_state_{StartupState::unattempted};
    int registration_status_{};
};

// Function-local static construction finishes (and its C++ destructor is
// registered) BEFORE initialize_once_00cd9010 registers native CE10F0.
GameNativePhysicalPoolProcess& game_native_physical_pool_process();

} // namespace bsp::game
