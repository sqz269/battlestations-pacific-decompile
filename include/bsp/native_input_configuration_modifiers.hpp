#pragma once
#include "bsp/native_input_configuration_load.hpp"

namespace bsp {
// Stateless source storage adapters for the checked10h vector push_back calls
// 697F40 (DWORD) and698980 (checked DWORD-vector row). They mutate the actual
// {opaque,begin,end,capacity} headers, use the existing CRT allocation domain,
// preserve opaque words and deep-copy owning rows. These are library contracts,
// not a reimplementation of the original STL ABI or its FH3 machinery.
void append_input_checked_word_storage(void* actual_header, const void* value);
void append_input_checked_row_storage(void* actual_header, const void* row);

// Contiguous698B64..699039 fragment, with the globals object already obtained
// by698B5F. Load all five InputModifiers groups only when the outer pair vector
// has zero rows. Populate pairs first, then General, Map, CameraY and PlaneY.
// Iterate actual Lua key/value objects and preserve lookup/destruction order.
void populate_native_input_configuration_modifiers_00698b64(
    void* actual_configuration, NativeLuaObjectStorage& globals,
    const bool& crt_sse2_conversion);

// Compose the established script prefix, globals acquisition and modifier
// fragment. On success the caller owns the returned object at fresh_globals
// and must eventually call destroy_native_lua_object_00b67700. On failure it
// is released here. This stops BEFORE69903A (input manager/Inputs parsing),
// so it MUST NOT substitute for a complete native698A10 call.
NativeLuaObjectStorage* load_native_input_configuration_modifiers_prefix_00698a10(
    void* actual_configuration, void* fresh_globals,
    NativeInputConfigurationLoadServices& services, const bool& crt_sse2_conversion);

// Valid separately owning ranges, compatible CRT allocations, and returning
// parameter handlers are supported. Malformed ranges, hardware faults,
// arbitrary stack aliases and original exception ABI are not claimed.
} // namespace bsp
