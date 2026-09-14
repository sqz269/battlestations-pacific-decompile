#include "bsp/native_vfs_provider_enumeration_bindings.hpp"

#include "bsp/native_filestore_enumeration.hpp"
#include "bsp/native_mpak_runtime.hpp"
#include "bsp/native_mpkg_enumeration.hpp"
#include "bsp/native_physical_enumeration.hpp"

#include <stdexcept>

namespace bsp {
void NativeVfsProviderEnumerationBindings::enumerate_entry(
    std::uintptr_t captured_entry, void* actual_provider,
    const void* actual_suffix, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& actual_output) {
    switch (captured_entry) {
    case 0x00bf47e0:
        enumerate_native_physical_names_00bf47e0(actual_provider,
            actual_suffix, actual_extension, flags, actual_output, inputs_.physical);
        return;
    case 0x00be6480:
        enumerate_native_filestore_names_00be6480(actual_provider,
            actual_suffix, actual_extension, flags, actual_output,
            inputs_.filestore_strings, inputs_.filestore_invalid_parameters);
        return;
    case 0x00bb98f0:
        enumerate_native_mpkg_names_00bb98f0(actual_provider,
            actual_suffix, actual_extension, flags, actual_output,
            inputs_.mpkg, inputs_.null_pattern_00e17bf0);
        return;
    case 0x00bb5f40:
        inputs_.mpak.enumerate_entry(captured_entry, actual_provider,
            actual_suffix, actual_extension, flags, actual_output);
        return;
    default:
        throw std::invalid_argument("unsupported native VFS enumeration provider entry");
    }
}
} // namespace bsp
