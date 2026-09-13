#pragma once

#include "bsp/native_mpak_enumeration.hpp"
#include "bsp/native_mpak_entry.hpp"
#include "bsp/native_mpak_open.hpp"
#include "bsp/native_vfs_device_route.hpp"

namespace bsp {
class NativeVfsRuntimeBindings;

// Borrowed provider operations consumed by the numeric VFS dispatcher.
// This is source composition, not a new native object or reconstructed body.
struct NativeMpakRuntimeContext {
    NativeMpakCreateContext& create;
    NativeMpakProviderContext& provider;
    NativeMpakOpenContext& open;
};

struct NativeMpakRuntimeInputs {
    NativeVfsLookupRouteContext& lookup;
    const void* actual_device_profile_00d683f4;
    ActualNativeStringPoolStorage& strings;
    NativePathCanonicalizerServices& allocation_and_pool;
    NativeMpakContainerLibrary& containers;
    NativeMpakOpenLibrary& open_library;
    NativeMpakDirectorySearchLibrary& directory_search;
    NativePakRegistryContext& registry;
    NativeStoredStreamConversionContext& conversion;
    const SingletonLifetimeCallbacks& invalid_parameters;
    void* volatile& actual_manager_publication_0109ceec;
    void* volatile& actual_lock_publication_010904e0;
    void* volatile& actual_cached_provider_010904dc;
    const char* null_pattern_00e17bf0;
};

// Connects existing actual owners using one borrowed pool/publication/lifetime
// domain. Constructor installs only source-context pointers; destructor restores
// previous pointers only while they still identify this binding. It never
// clears a cache, closes a provider, changes a native table or creates a second
// manager. Keep this graph alive while its owned provider operations are used.
class NativeMpakRuntime final : public NativeMpakProviderDispatch,
    public NativeMpakOpenDispatch, public NativeMpakEntryPositionDispatch,
    public NativeVfsProviderResolveDispatch {
public:
    NativeMpakRuntime(NativeVfsRuntimeBindings&, const NativeMpakRuntimeInputs&);
    ~NativeMpakRuntime() override;
    NativeMpakRuntime(const NativeMpakRuntime&) = delete;
    NativeMpakRuntime& operator=(const NativeMpakRuntime&) = delete;
    NativeMpakRuntimeContext& context() noexcept { return binding_; }
    NativeVfsDeviceRouteContext& device_context() noexcept { return device_; }

    void* open_manager_00bb82c8(std::uintptr_t, void*, const void*, std::uint32_t) override;
    std::int32_t select_device_00bdd850(void*, const void*, const void*) override;
    void* materialize_entry_00bb5080(void*, std::int32_t) override;
    std::uint64_t source_position(std::uintptr_t, void*) override;
    std::uint8_t invoke_resolve(std::uintptr_t captured_entry, void* provider,
        const void* suffix, void* output) override;

    // Explicit captured D641F8 methods beyond the existing open/lookup routes.
    // Unknown entries are source boundaries; no original numeric code is called.
    void enumerate_entry(std::uintptr_t captured_entry, void* provider,
        const void* prefix, const void* extension, std::uint32_t flags,
        NativeStringVectorStorage& output);
    bool reject_operation_entry(std::uintptr_t captured_entry, std::uint32_t,
        std::uint32_t, std::uint32_t, std::uint32_t);
    void* clear_result_entry(std::uintptr_t captured_entry, void* output,
        std::uint32_t ignored);
    void noop_entry(std::uintptr_t captured_entry);
private:
    NativeVfsRuntimeBindings& vfs_;
    NativeVfsLookupRouteContext& lookup_;
    NativeStoredStreamConversionContext& conversion_;
    NativeMpakDirectorySearchLibrary& directory_search_;
    const char* null_pattern_;
    NativeMpakDirectoryContext directory_;
    NativeMpakProviderContext provider_;
    NativeMpakCacheContext cache_;
    NativeMpakRuntimeProviderOperations provider_operations_;
    NativeMpakCreateContext create_;
    NativeMpakEntryContext entry_;
    NativeMpakOpenContext open_;
    NativeVfsDeviceRouteContext device_;
    NativeMpakRuntimeContext binding_;
    NativeMpakRuntimeContext* previous_mpak_;
    NativeVfsDeviceRouteContext* previous_device_;
    NativeAdoptedSubstreamDispatch* previous_substreams_;
};

// Full actual-storage position leaves required by BB5080's source slot20.
// BEF580[11]: ECX memory stream; cursor+10 minus backing+8/data+8, DWORD
// wrap then CDQ; EDX:EAX, RET. BF4F40[7]: ECX physical stream; current
// position low+10 then high+14; EDX:EAX, RET. New source APIs, no null guard.
std::uint64_t position_native_memory_stream_00bef580(const void*) noexcept;
std::uint64_t position_native_physical_stream_00bf4f40(const void*) noexcept;
} // namespace bsp
