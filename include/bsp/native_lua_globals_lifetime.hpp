#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
struct NativeLuaRegionLifetimeContext {
    NativeString& actual_region_0108ff24;
    NativeStringRawPoolContext& actual_strings;
};
// Bind once to stable process cells before registering the source callback.
// A different subsequent binding is rejected; no replacement or unbind exists.
void bind_static_native_lua_region_0108ff24(NativeLuaRegionLifetimeContext&);
// Full CD7CE0[12]: register CE0D60 through the real source CRT and retain EAX
// status. No store to the loader-zero Xbox byte or region header occurs.
int initialize_static_native_lua_region_00cd7ce0();
// Full CE0D60[35]: capture nonnull data, then current length+1, getter00419CC0,
// returnBD1510 with alignment1. Null skips the getter; header stays untouched.
void destroy_static_native_lua_region_00ce0d60();
// Explicit-context body for source composition/differential checking. Same
// raw getter/return sequence, including disabled-small and large return paths.
void destroy_native_lua_region_00ce0d60(NativeLuaRegionLifetimeContext&);
} // namespace bsp
