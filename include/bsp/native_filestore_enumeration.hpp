#pragma once

#include <cstdint>
#include "bsp/native_string_vector.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore enumeration requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Actual FileStore provider vtable 00D689E8 slot +14 -> 00BE6480.
// Inputs are borrowed eight-byte counted-string headers; output is the actual
// 0Ch pooled-string vector, appended in resident tree order. Flags is unread.
// Original ECX provider, stack directory,extension,flags,output; RET10h.
void enumerate_native_filestore_names_00be6480(void* actual_provider,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, NativeStringVectorStorage& output,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks);
}
