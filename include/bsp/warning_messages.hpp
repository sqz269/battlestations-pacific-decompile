#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/profile_unlock.hpp"
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// New C++ projections, not the original MSVC Win32 tree/string/vector ABI.
// Names are hypotheses; evidence and preconditions: docs/WARNING_MESSAGE_TABLE.md.
using WarningMessageMap =
    std::map<std::string, std::vector<std::string>, NativeStringCaseInsensitiveLess>;
using WarningEscapeMap =
    std::map<std::string, std::string, NativeStringCaseInsensitiveLess>;

struct WarningEscapeTable {
    std::string prefixes; // native section+0: top-level keys concatenated in Lua order
    WarningEscapeMap replacements; // native section+8: flattened path -> scalar string
};

struct WarningMessageTables {
    WarningMessageMap messages; // manager+108h; vector value, not a WarningRecord
    std::string escape_prefixes; // manager+114h: both sections' top-level keys
    WarningEscapeTable entity; // manager+11Ch
    WarningEscapeTable playerunit_section; // manager+130h
};

inline constexpr const char* kWarningDataScript = "Scripts/datatables/Warnings.lua";

// 0097F570: ECX=manager, stack=(message map*, LuaObject*, NativeString* prefix), RET Ch.
// Recurse on table values using prefix+key, with no separator. Collect scalar
// values at this level in lua_next order; only a nonempty vector assigns this
// prefix in the destination map. The scalar keys are ignored. Existing entries
// survive unless assigned, and a colliding path replaces the entire vector.
void load_warning_message_tree_0097f570(WarningMessageMap& destination,
    GuiLuaHost& lua, const GuiLuaRef& table, const std::string& prefix);

// 00979730: ECX=manager, stack=(escape section*, LuaObject*, prefix*), RET Ch.
// Recurse on table values using prefix+key; assign every scalar as one string.
// lua_tolstring returning null becomes an empty replacement, as at 00979860.
void load_warning_escape_tree_00979730(WarningEscapeTable& destination,
    GuiLuaHost& lua, const GuiLuaRef& table, const std::string& prefix);

// 00979990: ECX=manager, stack=(escape section*, LuaObject*), RET 8.
// Append each top-level key to manager+114h and section+0, then recurse with
// that key as the initial prefix. Does not clear either string or map.
void load_warning_escape_table_00979990(WarningMessageTables& manager,
    WarningEscapeTable& destination, GuiLuaHost& lua, const GuiLuaRef& table);

// Only the table-loading fragment of 009870A0. Caller supplies the existing
// Warnings LuaObject after running kWarningDataScript and reaching 009871BB.
// Loads Warnings.escapecharacters.{entity,playerunit_section}, then
// Warnings.messages with an empty prefix. Does not model the rest of Init.
void load_warning_tables_009870a0(WarningMessageTables& destination,
    GuiLuaHost& lua, const GuiLuaRef& warnings);

// 0096DBB0: ECX=manager, stack=(NativeString* id), bool in AL, RET 4.
// Semantic adaptation of map.find(id) != end; native checked-iterator failure
// paths are library contracts. Connects to WarningManagerHost::message_id_known.
bool warning_message_id_known_0096dbb0(
    const WarningMessageTables& tables, std::string_view id);

} // namespace bsp
