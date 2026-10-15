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

    // Independent explicit startup for the DISTINCT E17540 outer library-node
    // owner. Same one-attempt Source policy as E175B0: cache real registration
    // status, including nonzero; no retry/reconstruction after a thrown attempt.
    // This never initializes the inner E175B0 owner or wires application startup.
    // Externally serialize all owners' mutations of the common E188B4 list;
    // their separate startup mutexes are not a shared-list concurrency contract.
    int initialize_property_library_node_pool_once_00cc8a70();

    // Borrow the same retained actual38h cell ONLY after startup returned.
    // Nonzero registration still leaves an initialized owner without callback.
    // Mapped bags/inner records+nodes, pooled keys and outer nodes must all
    // retire before either pool's pages. No populated-node/profile provenance
    // or payload/class lifetime follows from obtaining this mutable pointer.
    void* property_library_node_pool_storage_00e17540();

    // Independent explicit startups for DISTINCT actual38h enum table E175E8
    // and symbol E17578 owners on the SAME canonical E188B4 list. One-attempt
    // Source policy: cache real atexit status (including nonzero); a thrown
    // attempt never retries or rolls back. This is not an Original once guard.
    // No constructor/application autostart or CEnum/global registry is wired.
    int initialize_enum_table_pool_once_00cc8a10();
    int initialize_enum_symbol_pool_once_00cc8a50();

    // SAME private cells, accessible only after their respective startup
    // returned; nonzero status still leaves an initialized owner. Externally
    // serialize common-list mutations across all owners; separate mutexes do
    // not supply that contract. Retire actual enum payloads/pooled keys/table
    // and symbol slots before page teardown. Keep this retained process/list
    // and every binding live through callbacks. No payload provenance follows
    // merely from a numeric phase word or obtaining these borrowed pointers.
    void* enum_table_pool_storage_00e175e8();
    void* enum_symbol_pool_storage_00e17578();

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

    // Append only: preserve every old field's position and owner identity.
    // Loader-zero Source storage is not a constructed pool until CC8A70 returns.
    alignas(std::max_align_t) std::byte property_library_pool_00e17540_[0x38]{};
    std::mutex property_library_startup_mutex_;
    StartupState property_library_startup_state_{StartupState::unattempted};
    int property_library_registration_status_{};

    // Append only: retain every established field's offset and owner meaning.
    // Neither loader-zero Source cell is a constructed pool before startup.
    alignas(std::max_align_t) std::byte enum_table_pool_00e175e8_[0x38]{};
    std::mutex enum_table_startup_mutex_;
    StartupState enum_table_startup_state_{StartupState::unattempted};
    int enum_table_registration_status_{};

    alignas(std::max_align_t) std::byte enum_symbol_pool_00e17578_[0x38]{};
    std::mutex enum_symbol_startup_mutex_;
    StartupState enum_symbol_startup_state_{StartupState::unattempted};
    int enum_symbol_registration_status_{};
};

// Function-local static construction finishes (and its C++ destructor is
// registered) BEFORE any explicit pool startup registers its CRT callback.
// Reverse CRT order tears those pools down while their process/list still live.
GameNativePhysicalPoolProcess& game_native_physical_pool_process();

} // namespace bsp::game
