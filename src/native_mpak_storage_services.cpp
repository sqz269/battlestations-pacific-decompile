#include "bsp/native_mpak_storage_services.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"

namespace bsp {
NativeMpakStorageServices::NativeMpakStorageServices(
    ActualNativeStringPoolStorage& strings, NativeVfsRuntimeBindings& streams,
    NativePathCanonicalizerServices& allocation_and_pool,
    const SingletonLifetimeCallbacks& invalid_parameters) noexcept
    : strings_(strings), streams_(streams), allocation_and_pool_(allocation_and_pool),
      invalid_parameters_(invalid_parameters), lookup_(invalid_parameters),
      offsets_(), containers_(strings, streams, allocation_and_pool, offsets_) {}

NativeMpakRuntimeInputs NativeMpakStorageServices::runtime_inputs(
    const NativeMpakStorageRuntimeDependencies& dependencies) noexcept {
    return {dependencies.lookup, dependencies.actual_device_profile_00d683f4,
        strings_, allocation_and_pool_, containers_, lookup_, lookup_,
        dependencies.registry, dependencies.conversion, invalid_parameters_,
        dependencies.actual_manager_publication_0109ceec,
        dependencies.actual_lock_publication_010904e0,
        dependencies.actual_cached_provider_010904dc,
        dependencies.null_pattern_00e17bf0};
}
} // namespace bsp
