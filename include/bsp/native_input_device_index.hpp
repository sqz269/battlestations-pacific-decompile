#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Partial source-ABI projection: the reached fresh-empty default path of
// 0055C110. Opaque bytes untouched by the native constructors remain untouched.
// Returns node+0x14.
// Original: ECX tree, stack pooled-string key, EAX mapped device, RET 4.
void* index_native_input_device_tree_0055c110(void* tree, const NativeString* key,
    NativeStringStorage& strings);
}
