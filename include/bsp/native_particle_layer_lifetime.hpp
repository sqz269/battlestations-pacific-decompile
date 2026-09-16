#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete AFAB90..AFAC46. Original: ECX raw40h Layer, stack borrowed native8h
// name and a DWORD stored at +10; EAX same Layer, RET8. Only established fields
// are written; +1C..+2B and +2D..+2F remain untouched. The source name can alias
// Layer+8 (which is cleared before identity is tested).
void* construct_native_particle_layer_00afab90(void* layer, const void* name,
    std::uint32_t value_10, NativeStringRawPoolContext& strings);

// Complete AFAC50..AFACD8, original ECX Layer/RET. Stamp D5DC38; release +14,
// then +8, then destroy the reference-counted base. Normal releases capture
// data before state changes; unwind destroys current headers. Headers persist.
void destroy_native_particle_layer_00afac50(void* layer,
    NativeStringRawPoolContext& strings);

// Complete AFACE0..AFACFD, original ECX Layer, flags on stack, EAX Layer/RET4.
void* scalar_delete_native_particle_layer_00aface0(void* layer,
    std::uint32_t flags, NativeStringRawPoolContext& strings);

// Owning C++ interfaces with a concrete raw pool context, not binary thunks.
// C++ unwind cleanup is supported; another exception during cleanup terminates.
// Original FH3/SEH/fault handling and gameplay are not established.
} // namespace bsp
