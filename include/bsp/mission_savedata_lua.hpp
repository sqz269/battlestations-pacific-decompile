#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

struct lua_State;

namespace bsp {

// Mission Lua-state pickler, 006ADDC0..006B68C0, driven by the mission save
// entry 006B7710 and the mission load tail 006B5960.
//
// Saving rewrites the live mission Lua state in place so that it can be
// printed as Lua source: every table reachable from a saved root gets a path
// string in its "__" field (for example "|thisTable|units|#3"), a second
// reference to an already named table becomes the string "_t_<path>", a
// global function becomes "_f_<global name>", and a light userdata entity
// pointer becomes "_p_<16-bit entity handle>" ("_p_x" for null). After the
// roots are printed, the restore pass walks every global table again and turns
// those strings back into values.
//
// Original ABI: every native routine below is a __thiscall/__fastcall body
// taking 14h-byte LuaObject wrappers and 8-byte NativeString values by value;
// the per-routine comments in src/mission_savedata_lua.cpp give the register
// and stack layout. This host interface works on a stock Lua 5.1 lua_State
// with the globals table at LUA_GLOBALSINDEX. It is not ABI compatible.
// Number keys are narrowed through float32 exactly as the 00B66270/00B66290/
// 00B66A60 wrappers do. Evidence: docs/CC13_SETTINGS_SAVEDATA.md.
//
// Reachability: no absolute dword, RVA or rel32 reference to 006B7710,
// 006B5960, 006AFB00 or the unnamed load body 006AF8B0 exists in the image,
// so this whole path may be retained but unreachable code in this build.

// The singleton at 00E198F0 (created by 006B17B0, constructor 006AFF20,
// vtable 00CF82E8): a std::map at +4 keyed by the signed int value of
// lua_topointer, mapping a global function to its global name. Only
// operator[] (005FF5D0) is used, so a missing function inserts "".
using MissionLuaFunctionNames = std::map<std::intptr_t, std::string>;

// Host-only guard for the recursive walkers (not a native constant).
inline constexpr int kMissionLuaWalkDepthLimit = 4096;

// Entity-pointer hooks for "_p_". Native encode reads the WORD at
// pointer+174h (006AEC46); native decode is the 006AD080 two-bank handle
// table lookup (006AE33D..006AE36F) without a null or range check.
struct MissionLuaEntityHooks {
    std::function<std::uint16_t(const void* entity)> handle_of;
    std::function<void*(std::uint16_t handle)> resolve;
};

// Key of one table slot in the walkers. Native keeps three element vectors
// per value kind: NativeString for string keys, int for keys whose float32
// value equals its truncation (00B66A60), float32 for every other number.
struct MissionLuaKey {
    enum class Kind { String, Integer, Number };
    Kind kind{Kind::String};
    std::string text;      // String
    std::int32_t integer{0}; // Integer
    float number{0.0f};    // Number
};

// 006B3DD0, __cdecl, no stack arguments: clears the registry and records
// every function-valued global under its key text.
void register_global_functions_006b3dd0(lua_State* L,
                                        MissionLuaFunctionNames& names);

// 006B1870 (string key, DL = string flag), 006B0650 (EDX = int key,
// stack flag) and 006B1E40 (DL = flag, float key): the naming pass.
// The table at stack index `parent` must be a table.
void name_table_tree(lua_State* L, const std::string& prefix, int parent,
                     const MissionLuaKey& key, bool string_key_segment);

// 006B3FD0 (string key), 006B3130 (int key), 006B4CB0 (float key) and
// 006B5BF0 (C-string key used for the global roots): the encode pass.
// `names` is mutable because the "_f_" encoders use operator[] and insert ""
// for a function that is not a global.
void encode_table_tree(lua_State* L, const std::string& prefix, int parent,
                       const MissionLuaKey& key, bool string_key_segment,
                       bool force, MissionLuaFunctionNames& names,
                       const MissionLuaEntityHooks& entities);

// 006B1190, one LuaObject by value on the stack, callee pops 14h: clears
// "__" unconditionally and recurses into every table-valued slot. Native has
// no visit guard, so a cycle recurses until the stack is exhausted; the host
// throws std::length_error past kMissionLuaWalkDepthLimit instead.
void strip_table_names_006b1190(lua_State* L, int table);

// 006B68C0, __cdecl, no arguments: the whole save-side rewrite of the
// mission state (naming, thisTable, Mission/MissionGlobals/MessageSender,
// then name stripping).
void encode_mission_lua_state_006b68c0(lua_State* L,
                                       MissionLuaFunctionNames& names,
                                       const MissionLuaEntityHooks& entities);

// 006B0020 (string key) and 006B0BE0 (float key): the restore walk.
void decode_table_tree(lua_State* L, int parent, const MissionLuaKey& key,
                       const MissionLuaEntityHooks& entities);

// 006AEA00 (string key) and 006AF120 (float key): turns one "_t_", "_f_" or
// "_p_" string slot back into a value.
void decode_slot(lua_State* L, int parent, const MissionLuaKey& key,
                 const MissionLuaEntityHooks& entities);

// 006B1490: clears the "_R_" visited marks set by the restore walk.
void strip_visit_marks_006b1490(lua_State* L, int table);

// 006B23E0, __cdecl, no arguments: the whole restore pass.
void decode_mission_lua_state_006b23e0(lua_State* L,
                                       const MissionLuaEntityHooks& entities);

// 006B7AA2..006B7BBE inside 006B7710: register functions, encode, print the
// four roots through `emit_global` (006AD8D0 per name, in the order
// Mission, MissionGlobals, MessageSender, thisTable), restore, clear the
// registry. The archive header written before this point and the garbage
// collection/continuation after it stay with the caller.
void save_mission_lua_globals_006b7710(
    lua_State* L, MissionLuaFunctionNames& names,
    const MissionLuaEntityHooks& entities,
    const std::function<void(const char* global_name)>& emit_global);

} // namespace bsp
