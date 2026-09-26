#pragma once
#include "bsp/native_procedural_factory_storage.hpp"
#include "bsp/native_procedural_factory_table_storage.hpp"
#include <cstdint>

namespace bsp {
struct NativeProceduralFactoryTableConstructionContext {
    NativeProceduralFactoryStorageContext& factories;
    const NativeResourceRegistryLookupContext& profiles;
    const char* const names[4]; // CE7DEC, CE7DC8, CE7DA4, CE7D80, in that order
};

// Five contiguous, initialized, genuinely live DWORDs: entry S-20 mask,
// S-1C cleanup self, S-18 current allocation, S-14/-10 raw name length/data.
// Their preimages remain until the corresponding native stores. No owning
// NativeString overlay or automatic header destructor is installed.
struct NativeProceduralFactoryTableConstructionFrame {
    volatile std::uint32_t (&locals)[5];
    volatile std::uint32_t& eh_state;
    const NativeProceduralFactoryStorageFrame& factory; // one prepared live frame
    const NativeProceduralFactorySlotFrame (&cleanup)[4]; // +0,+4,+8,+C
};
struct NativeProceduralFactoryTableConstructionAcquired {
    bool started{};
    void* captured_table{};
    int native_eh_state{-1};
    std::uint32_t captured_ebx_mask{};
    unsigned stage{};
    void* captured_allocation[4]{};
    bool allocation_entered[4]{};
    bool allocation_returned[4]{};
    bool name_entered[4]{};
    bool name_returned[4]{};
    bool name_credit_outstanding[4]{};
    bool final_profile_written[4]{};
    bool slot_published[4]{};
    std::uint32_t normal_name_data[4]{};
    std::uint32_t normal_name_size[4]{};
    bool normal_getter_entered[4]{};
    bool normal_getter_returned[4]{};
    bool normal_name_returned[4]{};
    NativeProceduralFactoryStorageAcquired factory[4]; // four fresh invocations
    bool completed{};
    bool source_failed{};
    bool partial_name_unarmed{};
    bool cleanup_started{};
    bool cleanup_completed{};
    bool cleanup_failed{};
    int cleanup_action{-1};
    void* cleanup_receiver[4]{};
    NativeProceduralFactorySlotAcquired cleanup[4];
    void* current_allocation_freed[4]{};
    bool mask_cleanup_entered[4]{};
    bool mask_cleanup_returned[4]{};
};

// BBC900[573], ECX actual live10h table, RET/EAX captured receiver. Exactly
// four4B allocations, raw41E870 names, genuine BBC5F0/BBC740 registration,
// derived D644E8/F0 stamps and slot publication. Normal cleanup captures
// CURRENT data/length before real419CC0/BD1510; only EBX bits0..2 are cleared.
// Full16-state source unwind consumes state before every genuine action,
// rereads cleanup self per slot, and uses CURRENT mask/allocation/raw header.
void* construct_native_procedural_factory_table_00bbc900(void* actual_table,
    const NativeProceduralFactoryTableConstructionFrame&,
    NativeProceduralFactoryTableConstructionContext&,
    NativeProceduralFactoryTableConstructionAcquired&);

struct NativeProceduralFactoryTablePublicationFrame {
    const std::uint32_t incoming_ecx;
    volatile std::uint32_t& cleanup_allocation; // entry S-10, seeded by PUSH ECX
    volatile std::uint32_t& eh_state;
    const NativeProceduralFactoryTableConstructionFrame& constructor;
};
struct NativeProceduralFactoryTablePublicationAcquired {
    bool started{};
    int native_eh_state{-1};
    bool allocation_entered{};
    bool allocation_returned{};
    void* captured_allocation{};
    bool constructor_entered{};
    bool constructor_returned{};
    void* captured_return{};
    bool publication_written{};
    bool completed{};
    bool source_failed{};
    bool cleanup_started{};
    bool cleanup_completed{};
    void* current_allocation_freed{};
    NativeProceduralFactoryTableConstructionAcquired constructor;
};

// BBCB40[96]. Seed incoming ECX BEFORE BF681B(10h), capture allocation/state0,
// publish constructor return or zero to actual01090900. No old-table release
// or initialization guard. Failure runs inner cleanup then frees CURRENT
// wrapper pointer; no second table destructor or publication write.
void* publish_native_procedural_factory_table_00bbcb40(
    void* volatile& actual_publication_01090900,
    const NativeProceduralFactoryTablePublicationFrame&,
    NativeProceduralFactoryTableConstructionContext&,
    NativeProceduralFactoryTablePublicationAcquired&);

// Admission: exact live payload/cells, same genuine CRT/manager/registry/pool
// domain, current native profile views and pinned live C strings. All reached
// acquisitions are fresh, persistent and disjoint from backing; frames survive
// failure disposition. Registry node+14 BORROWS factory storage. A successful
// registration can remain dangling after subsequent cleanup; quiesce lookup
// and dispose/reconstruct that real registry explicitly. No retain, rollback,
// unregister, success callbacks or fallback profile dispatch is added.
// Stop on a second source cleanup failure with residual diagnostics. Partial
// name construction before mask arming and post-slot getter failures retain
// name credits. New C++ interfaces do not reproduce original ABI, native FH3/
// SEH, saved-register/private-stack coincidence, startup or game behavior.
} // namespace bsp
