#pragma once

#include "bsp/native_mpak_container_storage.hpp"
#include "bsp/native_mpak_lookup_storage.hpp"
#include "bsp/native_mpak_offset_copy_storage.hpp"
#include "bsp/native_mpak_runtime.hpp"

namespace bsp {
class NativeVfsRuntimeBindings;

// The non-storage owners required by NativeMpakRuntime. All references must
// remain live through provider drain and runtime destruction.
struct NativeMpakStorageRuntimeDependencies {
    NativeVfsLookupRouteContext& lookup;
    const void* actual_device_profile_00d683f4;
    NativePakRegistryContext& registry;
    NativeStoredStreamConversionContext& conversion;
    void* volatile& actual_manager_publication_0109ceec;
    void* volatile& actual_lock_publication_010904e0;
    void* volatile& actual_cached_provider_010904dc;
    const char* null_pattern_00e17bf0;
};

// One retained storage graph. No pool, stream, manager or provider is created.
class NativeMpakStorageServices final {
public:
    NativeMpakStorageServices(ActualNativeStringPoolStorage& strings,
        NativeVfsRuntimeBindings& streams,
        NativePathCanonicalizerServices& allocation_and_pool,
        const SingletonLifetimeCallbacks& invalid_parameters) noexcept;
    NativeMpakStorageServices(const NativeMpakStorageServices&) = delete;
    NativeMpakStorageServices& operator=(const NativeMpakStorageServices&) = delete;
    NativeMpakStorageServices(NativeMpakStorageServices&&) = delete;
    NativeMpakStorageServices& operator=(NativeMpakStorageServices&&) = delete;

    NativeMpakLookupStorage& lookup() noexcept { return lookup_; }
    NativeMpakOffsetCopyStorage& offsets() noexcept { return offsets_; }
    NativeMpakContainerStorage& containers() noexcept { return containers_; }
    NativeVfsRuntimeBindings& streams() noexcept { return streams_; }
    NativeMpakRuntimeInputs runtime_inputs(
        const NativeMpakStorageRuntimeDependencies& dependencies) noexcept;
private:
    ActualNativeStringPoolStorage& strings_;
    NativeVfsRuntimeBindings& streams_;
    NativePathCanonicalizerServices& allocation_and_pool_;
    const SingletonLifetimeCallbacks& invalid_parameters_;
    NativeMpakLookupStorage lookup_;
    NativeMpakOffsetCopyStorage offsets_;
    NativeMpakContainerStorage containers_;
};
} // namespace bsp
