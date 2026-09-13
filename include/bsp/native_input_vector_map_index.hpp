#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Original ECX checked tree, stack NativeString key, EAX mapped native header,
// RET4. These source interfaces operate on the actual 32-bit tree/node storage.
// Constructors leave checked-vector opaque0 and packed-bit opaque4 unwritten.
// No default-stack preimage reaches a read or copy on these reached paths;
// callers must treat those words as indeterminate until a native producer sets them.
void* index_native_input_word_vector_006a44b0(void* tree, const NativeString* key,
    NativeStringStorage& strings);
void* index_native_input_descriptor_vector_006a45c0(void* tree, const NativeString* key,
    NativeStringStorage& strings);
void* index_native_input_bit_vector_006a4ca0(void* tree, const NativeString* key,
    NativeStringStorage& strings);
} // namespace bsp
