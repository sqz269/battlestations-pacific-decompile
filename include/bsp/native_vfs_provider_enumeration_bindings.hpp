#pragma once

#include "bsp/native_vfs_enumeration.hpp"

namespace bsp {
struct NativePhysicalEnumerationContext;
struct NativeMpkgDirectoryContext;
class NativeMpakRuntime;
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// All references are borrowed from the retained actual VFS owner graph.
// The four original numeric provider entries are compared with the value
// captured by BE1130; no projected provider or fallback route is accepted.
struct NativeVfsProviderEnumerationInputs {
    NativePhysicalEnumerationContext& physical;
    NativeStringStorage& filestore_strings;
    const SingletonLifetimeCallbacks& filestore_invalid_parameters;
    NativeMpkgDirectoryContext& mpkg;
    const char* null_pattern_00e17bf0;
    NativeMpakRuntime& mpak;
};

class NativeVfsProviderEnumerationBindings final :
    public NativeVfsProviderEnumerationDispatch {
public:
    explicit NativeVfsProviderEnumerationBindings(
        const NativeVfsProviderEnumerationInputs& inputs) noexcept : inputs_(inputs) {}
    void enumerate_entry(std::uintptr_t captured_entry, void* actual_provider,
        const void* actual_suffix, const void* actual_extension,
        std::uint32_t flags, NativeStringVectorStorage& actual_output) override;
private:
    NativeVfsProviderEnumerationInputs inputs_;
};
} // namespace bsp
