#pragma once
#include "bsp/native_lua_objects.hpp"
namespace bsp {
// Native B437F0 leaves mode unwritten if ordinal0 has no integral-number key.
// The caller supplies its original stack input. Every selected slots+mode*8
// must address a valid native string; the native routine does no bounds check.
struct NativeShaderCombinerStackInputs {
    void* context;
    std::int32_t (*mode_for_entry)(void*,NativeLuaObjectStorage&);
};
// ECX unused; stack destination strings/actual Lua entry; RET8. Ordinal advances
// for EVERY Lua entry, including noninteger keys. At ordinal0 exact NUMBER else
// mode13; ordinal1 exact STRING elseempty. Duplicates overwrite the same slot.
void read_native_shader_combiner_00b437f0(NativeString* slots,NativeLuaObjectStorage&,
    NativeStringStorage&,const bool& crt_sse2_conversion,std::int32_t mode_preimage);
// ECX descriptor passthrough (unused by inner routine), stack slots/Shader/name,
// RET0C. Fresh table lookup after gate; outer keys must pass integral-number
// predicate. Preserve prior slots and Lua order; no clearing or deduplication.
void read_native_shader_combiner_table_00b439c0(NativeString* slots,NativeLuaObjectStorage& shader,
    const NativeString& field,NativeStringStorage&,const bool& crt_sse2_conversion,
    const NativeShaderCombinerStackInputs&);
} // namespace bsp
