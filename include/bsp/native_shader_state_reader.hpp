#pragma once
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_shader_state_definitions.hpp"
namespace bsp {
// Full B579B0, three native stack arguments (output, registry, LuaObject*),
// RET0C; ECX unused. Actual storage throughout:16-byte definitions,20-byte
// Lua objects tracked in a4C8h owner, and8-byte state rows. No GuiLuaRef handles.
// Exact NUMBER gate on the first lookup, release it, then fresh lookup and
// numeric coercion. Tag0 uses float32 then native integer mode; tag1 float bits.
// Other tags reuse the previous payload, initially the source LuaObject address.
// Walk live unsigned registry count, copying each name/ID/tag before Lua calls.
// Existing output survives; B567B0 keeps the first matching state ID.
// Lua API errors remain unprotected native-style; native EH ABI is not claimed.
void read_native_shader_state_table_00b579b0(NativeShaderStateListStorage& output,
    NativeShaderStateDefinitionArray& definitions,NativeLuaObjectStorage& table,
    NativeStringStorage&,const bool& crt_sse2_conversion);
} // namespace bsp
