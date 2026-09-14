#pragma once

#include "bsp/native_mpkg_directory.hpp"
#include "bsp/native_string_vector.hpp"

namespace bsp {

// BB98F0: ECX actual MPKG provider, stack directory, extension, flags,
// output vector; tail-JMP BB97B0 and inherit its RET10h.
// The caller supplies the original readable null-pattern data for BEE340.
void enumerate_native_mpkg_names_00bb98f0(void* actual_provider,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& output,
    NativeMpkgDirectoryContext& context, const char* null_pattern_00e17bf0);

} // namespace bsp
