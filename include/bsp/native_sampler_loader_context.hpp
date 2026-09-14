#pragma once
#include "bsp/native_resource_record_vector.hpp"
#include "bsp/native_resource_registry_lookup.hpp"
#include <cstddef>

namespace bsp {
// Actual1Ch F8D420 owner. 4DE4B0 produces both profiles and clears08..14;
// B1A4F0 consumes the existing record vector via owner+4, and B19A10 writes
// the float18. No constructor initializes18 or constructs a projected clock.
struct NativeSamplerLoaderSingletonStorage {
    std::uint32_t vtable_00,cache_vtable_04;
    NativeResourceRecordVectorStorage records_08;
    std::uint32_t word_14;
    float time_18;
};
static_assert(sizeof(NativeSamplerLoaderSingletonStorage)==0x1c);
static_assert(offsetof(NativeSamplerLoaderSingletonStorage,records_08)==8);
static_assert(offsetof(NativeSamplerLoaderSingletonStorage,time_18)==0x18);

struct NativeSamplerLoaderOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    struct Guard {std::uint32_t profile_00{};void* section_04{};};
    Phase phase{Phase::fresh};
    std::uint32_t function{},native_site{};
    void* volatile* manager_publication{};
    void* volatile* owner_publication{};
    void* first_manager{};
    void* current_manager{};
    void* allocation{};
    void* registration_argument{};
    void* registry{};
    void* result{};
    const void* source_name{};
    const NativeResourceRegistryLookupContext* lookup_context{};
    Guard guard;
    bool guard_armed{},published{},registration_started{},registration_returned{};
    NativeSamplerLoaderOperation()=default;
    ~NativeSamplerLoaderOperation();
    NativeSamplerLoaderOperation(const NativeSamplerLoaderOperation&)=delete;
    NativeSamplerLoaderOperation& operator=(const NativeSamplerLoaderOperation&)=delete;
    // Caller first resolves retained native storage/publication obligations.
    // Frees nothing, unregisters nothing, and never retries the operation.
    void acknowledge_diagnostic_cleanup() noexcept;
};
static_assert(sizeof(NativeSamplerLoaderOperation::Guard)==8);

// Full4DE4B0: no native input; EAX actual1Ch owner, plainRET. Captured fast
// return; slow path captures the first actual manager+10 section, enters and
// increments raw+18, rechecks publication, allocates/initializes/publishes,
// obtains the current manager again, registers CURRENT publication, releases
// the captured section, then returns CURRENT publication. Guard unwind reuses
// actual411EE0. Registration failure keeps publication/allocation, not rollback.
NativeSamplerLoaderSingletonStorage* get_native_sampler_loader_singleton_004de4b0(
    void* volatile& actual_manager_01090aa0,void* volatile& actual_owner_00f8d420,
    NativeSamplerLoaderOperation&);

// Full20-byte B1B810: incoming ECX and second stacked options are unused;
// first stacked actual name forwarded, EAX creator result, RET8. Obtain the
// actual F8D41C registry through existing B1B730, invoke existing B19E90 and
// return unchanged. No added retain, fallback, null result or factory map.
void* create_native_sampler_factory_resource_00b1b810(const void* actual_name,
    const void* unused_options,void* volatile& actual_manager_01090aa0,
    void* volatile& actual_registry_00f8d41c,const NativeResourceRegistryLookupContext&,
    NativeSamplerLoaderOperation&);

// Actual borrowed globals and canonical provider domains; new explicit C++ ABI.
// Numeric profiles are original identities, not callable host vtables. Raw
// singleton terminal dispatch and complete B1A4F0/B1B4D0 cache loading remain
// unimplemented here. Descriptive names are hypotheses. Native FH3/hardware
// faults/provider exception identities and game behavior are not reproduced.
// Keep cells, owners and contexts alive; exclude retirement while running or
// failed. The persistent frame does not intercept external owner retirement.
} // namespace bsp
