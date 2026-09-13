#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Actual checked tree headers, native node/key/mapped storage and source pooled
// string services. Original map-index ABI: ECX tree, stack key, EAX mapped, RET4.
// Explicit preimages represent native private-stack bytes left uninitialized by
// the default constructors. Existing-key calls leave the mapped value unchanged.
void* index_native_input_sensitivity_tree_0055a9a0(void* tree, const NativeString* key,
    std::uint32_t default_scalar_preimage, NativeStringStorage&);
void* index_native_input_preset_tree_006a1e70(void* tree, const NativeString* key,
    std::uint32_t default_flag_word_preimage, NativeStringStorage&);
void* index_native_input_controller_tree_006a6900(void* tree, const NativeString* key,
    NativeStringStorage&);
void* index_native_input_controller_name_006a1f80(void* tree, const std::int32_t* key,
    NativeStringStorage&);

// Original ECX tree; stack output/key; EAX output; RET8. Writes owner, node,
// then only the low byte of the flag at output+8; its three padding bytes survive.
void* insert_native_input_integer_key_0069fa40(void* tree, void* output,
    const std::int32_t* key);

// New source ABIs, requiring consistent owned trees, aligned storage and stable
// ordering. Allocation/release callbacks must not mutate topology, keys or private
// defaults. Original STL/CRT/exception ABI, FH3/private-stack aliasing, malformed
// storage and hardware-fault behavior remain outside these contracts.
} // namespace bsp
