#pragma once
#include "bsp/native_resource_record_vector.hpp"
#include "bsp/native_resource_registry_lookup.hpp"
#include <cstddef>

namespace bsp {
// Actual 1Ch F8D420 owner view. The established native particle-clock provider
// produces this same storage; this module does not implement 4DE4B0 a second
// time. B1A4F0 consumes the record vector through owner+4 and B19A10 writes
// float18. No constructor initializes18 or constructs a projected clock.
struct NativeSamplerLoaderSingletonStorage {
    std::uint32_t vtable_00, cache_vtable_04;
    NativeResourceRecordVectorStorage records_08;
    std::uint32_t word_14;
    float time_18;
};
static_assert(sizeof(NativeSamplerLoaderSingletonStorage) == 0x1c);
static_assert(offsetof(NativeSamplerLoaderSingletonStorage, records_08) == 8);
static_assert(offsetof(NativeSamplerLoaderSingletonStorage, time_18) == 0x18);

struct NativeSamplerLoaderOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{};
    void* volatile* manager_publication{};
    void* volatile* owner_publication{};
    void* registry{};
    void* result{};
    const void* source_name{};
    const NativeResourceRegistryLookupContext* lookup_context{};
    NativeSamplerLoaderOperation() = default;
    ~NativeSamplerLoaderOperation();
    NativeSamplerLoaderOperation(const NativeSamplerLoaderOperation&) = delete;
    NativeSamplerLoaderOperation& operator=(const NativeSamplerLoaderOperation&) = delete;
    // Caller first resolves retained native storage/publication obligations.
    // Frees nothing, unregisters nothing, and never retries the operation.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Operation-frame adapter for the complete established 004DE4B0 provider.
// Original entry has no input, returns the actual 1Ch owner in EAX and uses a
// plain RET. The delegated provider owns its precise captured-manager, guard,
// recheck, publication, registration and final-reload schedule. This frame
// records only the call boundary, borrowed cell identities, result and
// success/failure; it does not invent internal native-site receipts.
NativeSamplerLoaderSingletonStorage* get_native_sampler_loader_singleton_004de4b0(
    void* volatile& actual_manager_01090aa0, void* volatile& actual_owner_00f8d420,
    NativeSamplerLoaderOperation&);

// Full 20-byte B1B810: incoming ECX and second stacked options are unused;
// first stacked actual name is forwarded, EAX is the creator result, RET8.
// Obtain the
// actual F8D41C registry through existing B1B730, invoke existing B19E90 and
// return unchanged. No added retain, fallback, null result or factory map.
void* create_native_sampler_factory_resource_00b1b810(const void* actual_name,
    const void* unused_options, void* volatile& actual_manager_01090aa0,
    void* volatile& actual_registry_00f8d41c, const NativeResourceRegistryLookupContext&,
    NativeSamplerLoaderOperation&);

// Actual borrowed globals and canonical provider domains; new explicit C++ ABI.
// Numeric profiles are original identities, not callable host vtables. Raw
// singleton terminal dispatch and application admission remain outside this
// module. Current direct scalar providers are available to an explicit owner,
// but no host deletion binding is installed here. Full B1A4F0/B1B4D0 loading
// remains separate. Descriptive names are hypotheses. Native FH3/hardware
// fault/provider exception identities and game behavior are not reproduced.
// Keep cells, owners and contexts alive; exclude retirement while running or
// failed. The persistent frame does not intercept external owner retirement.
} // namespace bsp
