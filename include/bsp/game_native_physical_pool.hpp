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

    // Explicit startup for the distinct actual 00E175B0 property-node pool.
    // One Source process attempt; cache the real CC8A30 std::atexit status,
    // including nonzero. A thrown attempt is never retried or rolled back.
    // Serialize this with other owners' shared 00E188B4 list mutations. The
    // unchanged Native bookend has no once guard. No app startup is wired here.
    int initialize_property_node_pool_once_00cc8a30();

    // SAME private 38h process cell, available only after startup returned.
    // A nonzero status still leaves it initialized without its CRT callback.
    // Its borrowers use allocator_list_domain_00e188b4(), never a second list.
    // Finish all key/record/map payload lifetimes before CD9260 releases pages;
    // keep this canonical process/list alive through that callback. No record
    // profile/class lifetime or free permission follows from this raw pointer.
    void* property_node_pool_storage_00e175b0();

private:
    friend GameNativePhysicalPoolProcess& game_native_physical_pool_process();
    GameNativePhysicalPoolProcess() noexcept;
    ~GameNativePhysicalPoolProcess() = default; // bookkeeping; CRT owns pool callbacks

    enum class StartupState { unattempted, returned, threw };
    alignas(std::max_align_t) std::byte pool_0109dbf0_[0x38]{};
    AllocatorListElement* list_head_00e188b4_{};
    AllocatorListDomain list_;
    NativePhysicalProviderPoolContext context_;
    std::mutex startup_mutex_;
    StartupState startup_state_{StartupState::unattempted};
    int registration_status_{};

    // Append after every old field so existing member offsets remain intact.
    // Original E175B0 is a distinct loader-zero 38h physical global. Its real
    // CC8A30/411050 startup constructs this Source projection; not 0109DBF0.
    alignas(std::max_align_t) std::byte property_pool_00e175b0_[0x38]{};
    std::mutex property_startup_mutex_;
    StartupState property_startup_state_{StartupState::unattempted};
    int property_registration_status_{};
};

// Function-local static construction finishes (and its C++ destructor is
// registered) BEFORE either explicit pool startup registers its CRT callback.
// Reverse CRT order tears those pools down while their process/list still live.
GameNativePhysicalPoolProcess& game_native_physical_pool_process();

} // namespace bsp::game
