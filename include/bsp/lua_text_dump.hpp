#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

struct lua_State;

namespace bsp {

// Lua value printers used by the mission save emitter 006AD8D0 and by
// 00920F00 / 00921360. They write Lua-like source text; strings are wrapped in
// double quotes with no escaping, so a string holding '"' or a newline does
// not read back. Host interface over a stock Lua 5.1 state, not ABI compatible.
// Evidence: docs/CC13_PROFILE_SAVE.md.

// WORD at pointer+174h, the entity handle printed for a light userdata.
using LuaTextEntityHandle = std::function<std::uint16_t(const void* entity)>;
// BSP_SaveStorage_AppendArchiveBytes (00BD49B0) on the storage singleton 0109CECC.
using LuaTextAppend = std::function<void(std::string_view bytes)>;

// 00B68460: type names indexed by lua_type + 1 ("none", "nil", "bool",
// "lightuserdata", "number", "string", "table", "function", "userdata",
// "thread"; anything else "unknown").
const char* lua_type_name_00b68460(int lua_type) noexcept;

// Address: 0043C1C0 (body 0043C1C0..0043C523).
// Original ABI: __fastcall, ECX = out NativeString*, EDX = LuaObject*, RET;
// returns ECX. nil/true/false; an integral float32 as "%d"; another number
// as "%f" of its float32 (0043BDF0); a string as "\"" + text + "\""; a light
// userdata as "\"_ptr_<handle>_\"" or "\"_ptr_x_\"" for null; a function as
// "\"_function_<lua_topointer as %d>_\""; anything else as
// "\"_other_type:_<type name>_\"".
std::string lua_value_text_0043c1c0(lua_State* L, int index, const LuaTextEntityHandle& handle_of);

// Address: 0043C530 (body 0043C530..0043CC17).
// Original ABI: ECX = table LuaObject*, EDX = indent (tab count), [ESP+4] =
// remaining depth, [ESP+8] = a word only forwarded to the recursion; RET 8.
// Emits "{\n", one "\t[<key>] =" line per slot in lua_next order followed by
// " <value>,\n" or by "\n" and the nested table at indent + 1, depth - 1, and
// finally "}\n" at indent 0 or "},\n" deeper, each line prefixed by the indent.
// At negative depth it emits only indent + "\tnil, -- too deep recursion",
// with no line break, so the next key line continues on the same line.
void dump_lua_table_text_0043c530(lua_State* L, int index, int indent, int depth,
                                  const LuaTextEntityHandle& handle_of, const LuaTextAppend& append);

// Address: 006AD8D0 (BSP_MissionSave_EmitLuaGlobal).
// Original ABI: ECX = NativeString* global name, RET. A table global is
// written as name + " =\n" and 0043C530(value, 0, 10h, 1); any other value as
// name + " = " + 0043C1C0(value) + "\n".
void emit_lua_global_text_006ad8d0(lua_State* L, const char* name,
                                   const LuaTextEntityHandle& handle_of, const LuaTextAppend& append);

} // namespace bsp
