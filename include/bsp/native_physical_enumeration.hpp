#pragma once

#include <cstdint>
#include "bsp/native_string_vector.hpp"

namespace bsp {
struct NativePhysicalFileDateContext;
struct NativePathCanonicalizerContext;

struct NativePhysicalEnumerationContext {
    NativePhysicalFileDateContext& physical;
    NativePathCanonicalizerContext& canonicalizer;
};

// BEE520: ECX output actual8h header, EDX directory actual8h header,
// stack child actual8h header; EAX output, RET4. New C++ service ABI.
void* join_native_physical_enumerated_name_00bee520(void* output,
    const void* directory, const void* child,
    NativePhysicalEnumerationContext&);

// BF47E0: ECX actual D69168 physical provider; stack directory and extension
// actual8h headers, DWORD flags and actual0Ch string vector; RET10h, no result.
// Appends in Win32 discovery order, recursing only when flags' low byte is set.
void enumerate_native_physical_names_00bf47e0(void* actual_provider,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& output,
    NativePhysicalEnumerationContext&);
} // namespace bsp
