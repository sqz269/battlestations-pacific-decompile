#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// New Win32 source interfaces over actual twelve-byte map headers. The caller
// owns destination construction/destruction and the borrowed returned cells.
// 0055B400: ECX destination, stack source, EAX destination, RET4.
void* copy_native_input_scale_map_0055b400(void* destination, const void* source);
// 0055B490: ECX map, no stack arguments, RET. Complete populated cleanup.
void destroy_native_input_scale_map_0055b490(void* map) noexcept;
// 006A5AA0: ECX map, stack signed key*, EAX borrowed inner map, RET4.
void* lookup_or_insert_native_input_scalar_tree_006a5aa0(void* map,
    const std::int32_t* key);
// 00444BE0: ECX raw string-CI map, stack native 8-byte name header,
// EAX borrowed float cell, RET4. Storage is the explicit native pool boundary.
float* lookup_or_insert_native_input_float_00444be0(void* map,
    const void* native_name, NativeStringStorage& strings);
} // namespace bsp
