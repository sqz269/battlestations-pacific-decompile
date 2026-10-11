#include "bsp/mission_savedata_lua.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

// Literals, all .rdata.
constexpr const char* kNameField = "__";   // 00CF82F0
constexpr const char* kVisitField = "_R_"; // 00CF82EC
constexpr std::string_view kTableRef = "_t_";    // 00CF8298
constexpr std::string_view kFunctionRef = "_f_"; // 00CF8294
constexpr std::string_view kEntityRef = "_p_";   // 00CF8290
constexpr const char* kThisTable = "thisTable";  // 00CE7494
// 00E08DF0..00E08DFC: three pointers, 00CF5148 / 00CF8240 / 00CF8230.
constexpr const char* kMissionRoots[] = {"Mission", "MissionGlobals", "MessageSender"};

int abs_index(lua_State* L, int index) {
    return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

void enter(lua_State* L, int depth) {
    if (depth > kMissionLuaWalkDepthLimit)
        throw std::length_error("mission savedata walk deeper than the host limit");
    if (!lua_checkstack(L, 8)) throw std::length_error("Lua stack exhausted");
}

// _stricmp in the "C" locale.
bool same_name(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

// CVTTSS2SI: truncation; NaN and out-of-range give 80000000h.
std::int32_t truncate_float32(float value) {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

// 00B66270: lua_tonumber narrowed through FSTP float.
float number_float32(lua_State* L, int index) {
    return static_cast<float>(lua_tonumber(L, index));
}

// 00B66A60: LUA_TNUMBER whose float32 equals its CVTTSS2SI truncation.
bool is_integer_number(lua_State* L, int index) {
    if (lua_type(L, index) != LUA_TNUMBER) return false;
    const float value = number_float32(L, index);
    return static_cast<double>(value) == static_cast<double>(truncate_float32(value));
}

// 00B662B0 (lua_tostring) followed by NativeString assign through strlen.
// The copy keeps lua_tostring from converting a lua_next key in place.
std::string native_text(lua_State* L, int index) {
    lua_pushvalue(L, index);
    const char* text = lua_tostring(L, -1);
    std::string result = text ? std::string(text) : std::string();
    lua_pop(L, 1);
    return result;
}

[[noreturn]] void not_indexable(const char* site) {
    // Native: lua_gettable/lua_settable on a non-table raises a Lua error that
    // unwinds to the enclosing protected call. The host refuses first.
    throw std::runtime_error(std::string(site) + ": slot owner is not a table");
}

void require_table(lua_State* L, int index, const char* site) {
    if (!lua_istable(L, index)) not_indexable(site);
}

// 00B68100 for string keys (C-string semantics, so text stops at a NUL),
// 00B67720 for integer keys, CVTTSS2SI then 00B67720 for float keys.
void push_key(lua_State* L, const MissionLuaKey& key) {
    switch (key.kind) {
    case MissionLuaKey::Kind::String: lua_pushstring(L, key.text.c_str()); break;
    case MissionLuaKey::Kind::Integer: lua_pushnumber(L, key.integer); break;
    case MissionLuaKey::Kind::Number: lua_pushnumber(L, truncate_float32(key.number)); break;
    }
}

void push_slot(lua_State* L, int table, const MissionLuaKey& key) {
    require_table(L, table, "slot read");
    push_key(L, key);
    lua_gettable(L, table);
}

// 00B674C0 (string key) and 00B672F0 (index key) with a NativeString value.
void set_slot_text(lua_State* L, int table, const MissionLuaKey& key, const std::string& value) {
    require_table(L, table, "slot write");
    push_key(L, key);
    lua_pushlstring(L, value.data(), value.size());
    lua_settable(L, table);
}

// 00B675D0 (string key) and 00B666C0 (index key) with a LuaObject value.
void set_slot_value(lua_State* L, int table, const MissionLuaKey& key, int value) {
    require_table(L, table, "slot write");
    value = abs_index(L, value);
    push_key(L, key);
    lua_pushvalue(L, value);
    lua_settable(L, table);
}

bool field_is_nil(lua_State* L, int table, const char* field) {
    require_table(L, table, "field read");
    lua_getfield(L, table, field);
    const bool nil = lua_isnil(L, -1);
    lua_pop(L, 1);
    return nil;
}

std::string field_text(lua_State* L, int table, const char* field) {
    require_table(L, table, "field read");
    lua_getfield(L, table, field);
    std::string text = native_text(L, -1);
    lua_pop(L, 1);
    return text;
}

// 00B67630 (named string), 00B66790 (named C string).
void set_field_text(lua_State* L, int table, const char* field, const std::string& value) {
    require_table(L, table, "field write");
    lua_pushlstring(L, value.data(), value.size());
    lua_setfield(L, table, field);
}

MissionLuaKey string_key(std::string text) {
    MissionLuaKey key;
    key.text = std::move(text);
    return key;
}

MissionLuaKey integer_key(std::int32_t value) {
    MissionLuaKey key;
    key.kind = MissionLuaKey::Kind::Integer;
    key.integer = value;
    return key;
}

MissionLuaKey number_key(float value) {
    MissionLuaKey key;
    key.kind = MissionLuaKey::Kind::Number;
    key.number = value;
    return key;
}

// The path segment after "|": the key itself for string keys, otherwise "#"
// and 004263B0 "%d" (integer) or 006AF2B0/0043BDF0 "%f" (float).
std::string key_segment(const MissionLuaKey& key, bool string_key_segment) {
    std::string segment = string_key_segment ? "" : "#";
    char buffer[400];
    switch (key.kind) {
    case MissionLuaKey::Kind::String: segment += key.text; break;
    case MissionLuaKey::Kind::Integer:
        std::snprintf(buffer, sizeof buffer, "%d", key.integer);
        segment += buffer;
        break;
    case MissionLuaKey::Kind::Number:
        std::snprintf(buffer, sizeof buffer, "%f", static_cast<double>(key.number));
        segment += buffer;
        break;
    }
    return segment;
}

// Three key vectors per value class, in lua_next order: IsString (type 4),
// then IsInteger (00B66A60, stored truncated), then IsNumber (float32).
// Keys of any other type are dropped.
struct ThreeWayKeys {
    std::vector<std::string> strings;
    std::vector<std::int32_t> integers;
    std::vector<float> numbers;

    void add(lua_State* L, int key) {
        if (lua_type(L, key) == LUA_TSTRING) strings.push_back(native_text(L, key));
        else if (is_integer_number(L, key)) integers.push_back(truncate_float32(number_float32(L, key)));
        else if (lua_type(L, key) == LUA_TNUMBER) numbers.push_back(number_float32(L, key));
    }
};

// The restore-side walkers keep two vectors: IsString, then IsNumber as float32.
struct TwoWayKeys {
    std::vector<std::string> strings;
    std::vector<float> numbers;

    void add(lua_State* L, int key) {
        if (lua_type(L, key) == LUA_TSTRING) strings.push_back(native_text(L, key));
        else if (lua_type(L, key) == LUA_TNUMBER) numbers.push_back(number_float32(L, key));
    }
};

// 00B67080/00B67190/00B66420 iteration.
template <class Visit>
void for_each_slot(lua_State* L, int table, Visit visit) {
    require_table(L, table, "table walk");
    lua_pushnil(L);
    while (lua_next(L, table)) {
        const int value = lua_gettop(L);
        visit(value - 1, value);
        lua_pop(L, 1);
    }
}

void name_tree(lua_State* L, const std::string& prefix, int parent, const MissionLuaKey& key,
               bool string_key_segment, int depth) {
    enter(L, depth);
    push_slot(L, parent, key);
    const int child = lua_gettop(L);
    // 006B18B9..006B18FE: a table that already carries "__" is left alone.
    if (field_is_nil(L, child, kNameField)) {
        const std::string name = prefix + "|" + key_segment(key, string_key_segment);
        set_field_text(L, child, kNameField, name);
        ThreeWayKeys tables;
        for_each_slot(L, child, [&](int k, int v) {
            if (lua_istable(L, v)) tables.add(L, k);
        });
        // 006B1BB0 (DL=1), 006B1C50 (stack flag 0), 006B1C95 (DL=0).
        for (const auto& text : tables.strings) name_tree(L, name, child, string_key(text), true, depth + 1);
        for (auto value : tables.integers) name_tree(L, name, child, integer_key(value), false, depth + 1);
        for (auto value : tables.numbers) name_tree(L, name, child, number_key(value), false, depth + 1);
    }
    lua_pop(L, 1);
}

// 006AEBF0 / 006AEDC0 / 006AEF70.
void encode_entity_slot(lua_State* L, int parent, const MissionLuaKey& key,
                        const MissionLuaEntityHooks& entities) {
    push_slot(L, parent, key);
    const void* entity = lua_touserdata(L, -1); // 00B662D0
    lua_pop(L, 1);
    std::string text(kEntityRef);
    if (entity) {
        if (!entities.handle_of) throw std::invalid_argument("entity handle hook is required");
        text += std::to_string(static_cast<unsigned>(entities.handle_of(entity)));
    } else {
        text += "x"; // 00CEB488
    }
    set_slot_text(L, parent, key, text);
}

// 006B2D30 / 006B2E90 / 006B2FC0.
void encode_function_slot(lua_State* L, int parent, const MissionLuaKey& key,
                          MissionLuaFunctionNames& names) {
    push_slot(L, parent, key);
    const auto pointer = reinterpret_cast<std::intptr_t>(lua_topointer(L, -1)); // 00B662E0
    lua_pop(L, 1);
    set_slot_text(L, parent, key, std::string(kFunctionRef) + names[pointer]);
}

void encode_tree(lua_State* L, const std::string& prefix, int parent, const MissionLuaKey& key,
                 bool string_key_segment, bool force, MissionLuaFunctionNames& names,
                 const MissionLuaEntityHooks& entities, int depth) {
    enter(L, depth);
    push_slot(L, parent, key);
    const int child = lua_gettop(L);
    // 006B4021..006B408F: "__" is read only when the force byte is zero.
    if (force || field_is_nil(L, child, kNameField)) {
        const std::string name = prefix + "|" + key_segment(key, string_key_segment);
        set_field_text(L, child, kNameField, name);
        ThreeWayKeys tables, entity_slots, function_slots;
        for_each_slot(L, child, [&](int k, int v) {
            if (lua_istable(L, v)) tables.add(L, k);                              // 00B661B0
            else if (lua_type(L, v) == LUA_TLIGHTUSERDATA) entity_slots.add(L, k); // 00B66160
            else if (lua_type(L, v) == LUA_TFUNCTION) function_slots.add(L, k);    // 00B66200
        });
        // 006B4540 (DL=1, force 0), 006B45E0 (flag 0, force 0), 006B4630 (DL=0, force 0).
        for (const auto& text : tables.strings)
            encode_tree(L, name, child, string_key(text), true, false, names, entities, depth + 1);
        for (auto value : tables.integers)
            encode_tree(L, name, child, integer_key(value), false, false, names, entities, depth + 1);
        for (auto value : tables.numbers)
            encode_tree(L, name, child, number_key(value), false, false, names, entities, depth + 1);
        for (const auto& text : entity_slots.strings) encode_entity_slot(L, child, string_key(text), entities);
        for (auto value : entity_slots.integers) encode_entity_slot(L, child, integer_key(value), entities);
        for (auto value : entity_slots.numbers) encode_entity_slot(L, child, number_key(value), entities);
        for (const auto& text : function_slots.strings) encode_function_slot(L, child, string_key(text), names);
        for (auto value : function_slots.integers) encode_function_slot(L, child, integer_key(value), names);
        for (auto value : function_slots.numbers) encode_function_slot(L, child, number_key(value), names);
    } else {
        // 006B4B8C..006B4C09: a second reference becomes "_t_" + its path.
        set_slot_text(L, parent, key, std::string(kTableRef) + field_text(L, child, kNameField));
    }
    lua_pop(L, 1);
}

void strip_names(lua_State* L, int table, int depth) {
    enter(L, depth);
    table = abs_index(L, table);
    // 006B11D7..006B11F3: no "__" test before clearing it.
    require_table(L, table, "name strip");
    lua_pushnil(L);
    lua_setfield(L, table, kNameField);
    TwoWayKeys tables;
    for_each_slot(L, table, [&](int k, int v) {
        if (lua_istable(L, v)) tables.add(L, k);
    });
    for (const auto& text : tables.strings) {
        push_slot(L, table, string_key(text));
        strip_names(L, -1, depth + 1);
        lua_pop(L, 1);
    }
    for (auto value : tables.numbers) {
        push_slot(L, table, number_key(value));
        strip_names(L, -1, depth + 1);
        lua_pop(L, 1);
    }
}

// 006ADDC0 / 006AE400: walk "|seg|#n|..." from the globals table.
void decode_table_ref(lua_State* L, int parent, const MissionLuaKey& key, std::string path) {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    int current = lua_gettop(L);
    while (!path.empty()) {
        // 006ADE91: Substring(1, 1) compared with "#".
        const bool numeric = path.size() >= 2 && path[1] == '#';
        const std::size_t start = numeric ? 2 : 1;
        // 006ADEF4..006ADF17: end = 1 + strcspn(data + 1, "|"), the first '|'
        // after position 0 or the length. numeric implies path[1] == '#', so
        // start <= end and both substrings are in range.
        const std::size_t bar = path.find('|', 1);
        const std::size_t end = bar == std::string::npos ? path.size() : bar;
        const std::string segment = path.substr(start, end - start);
        path = path.substr(end);
        if (numeric) {
            // _atof then _ftol: truncation of the double to 64 bits, low half kept.
            const double value = std::atof(segment.c_str());
            const auto wide = value > -9.2233720368547758e18 && value < 9.2233720368547758e18
                ? static_cast<std::int64_t>(value) : std::numeric_limits<std::int64_t>::min();
            push_slot(L, current, integer_key(static_cast<std::int32_t>(static_cast<std::uint32_t>(wide))));
        } else {
            push_slot(L, current, string_key(segment));
        }
        lua_replace(L, current);
    }
    set_slot_value(L, parent, key, current);
    lua_pop(L, 1);
}

void decode_tree(lua_State* L, int parent, const MissionLuaKey& key,
                 const MissionLuaEntityHooks& entities, int depth) {
    enter(L, depth);
    push_slot(L, parent, key);
    const int child = lua_gettop(L);
    // 006B0063..006B0120: a table already marked "_R_" is skipped, otherwise marked true.
    if (field_is_nil(L, child, kVisitField)) {
        lua_pushboolean(L, 1);
        lua_setfield(L, child, kVisitField);
        TwoWayKeys tables, strings;
        for_each_slot(L, child, [&](int k, int v) {
            if (lua_istable(L, v)) tables.add(L, k);
            else if (lua_type(L, v) == LUA_TSTRING) strings.add(L, k);
        });
        for (const auto& text : tables.strings) decode_tree(L, child, string_key(text), entities, depth + 1);
        for (auto value : tables.numbers) decode_tree(L, child, number_key(value), entities, depth + 1);
        for (const auto& text : strings.strings) decode_slot(L, child, string_key(text), entities);
        for (auto value : strings.numbers) decode_slot(L, child, number_key(value), entities);
    }
    lua_pop(L, 1);
}

void strip_visits(lua_State* L, int table, int depth) {
    enter(L, depth);
    table = abs_index(L, table);
    // 006B14AC..006B1517: only a marked table is cleared and walked.
    if (field_is_nil(L, table, kVisitField)) return;
    lua_pushnil(L);
    lua_setfield(L, table, kVisitField);
    TwoWayKeys tables;
    for_each_slot(L, table, [&](int k, int v) {
        if (lua_istable(L, v)) tables.add(L, k);
    });
    for (const auto& text : tables.strings) {
        push_slot(L, table, string_key(text));
        strip_visits(L, -1, depth + 1);
        lua_pop(L, 1);
    }
    for (auto value : tables.numbers) {
        push_slot(L, table, number_key(value));
        strip_visits(L, -1, depth + 1);
        lua_pop(L, 1);
    }
}

bool skipped_by_restore(const std::string& name) {
    return same_name(name, "recon") || same_name(name, "_G") || same_name(name, "_savedata");
}

} // namespace

// 006B3DD0: ECX/EDX unused, RET. Clears the 006B17B0 map, then for every
// function-valued global stores its key text (00B662B0) under lua_topointer.
void register_global_functions_006b3dd0(lua_State* L, MissionLuaFunctionNames& names) {
    names.clear();
    for_each_slot(L, LUA_GLOBALSINDEX, [&](int k, int v) {
        if (lua_type(L, v) == LUA_TFUNCTION)
            names[reinterpret_cast<std::intptr_t>(lua_topointer(L, v))] = native_text(L, k);
    });
}

// 006B1870: ECX=prefix NativeString*, DL=string-segment flag, [ESP+4]=parent
// LuaObject (14h), [ESP+18h]=key NativeString (8), RET 1Ch.
// 006B0650: ECX=prefix, EDX=int key, [ESP+4]=parent, [ESP+18h]=flag, RET 18h.
// 006B1E40: ECX=prefix, DL=flag, [ESP+4]=parent, [ESP+18h]=float key, RET 18h.
void name_table_tree(lua_State* L, const std::string& prefix, int parent,
                     const MissionLuaKey& key, bool string_key_segment) {
    name_tree(L, prefix, abs_index(L, parent), key, string_key_segment, 0);
}

// 006B3FD0: ECX=prefix, DL=flag, [ESP+4]=parent, [ESP+18h]=key NativeString,
// [ESP+20h]=force byte, RET 20h. 006B3130: EDX=int key, flag and force on the
// stack. 006B4CB0: DL=flag, float key and force on the stack. 006B5BF0:
// EDX=C-string key, [ESP+18h]=flag, [ESP+1Ch]=force, RET 1Ch.
void encode_table_tree(lua_State* L, const std::string& prefix, int parent,
                       const MissionLuaKey& key, bool string_key_segment, bool force,
                       MissionLuaFunctionNames& names, const MissionLuaEntityHooks& entities) {
    encode_tree(L, prefix, abs_index(L, parent), key, string_key_segment, force, names, entities, 0);
}

void strip_table_names_006b1190(lua_State* L, int table) {
    strip_names(L, table, 0);
}

void encode_mission_lua_state_006b68c0(lua_State* L, MissionLuaFunctionNames& names,
                                       const MissionLuaEntityHooks& entities) {
    const int globals = LUA_GLOBALSINDEX;
    // 006B6904..006B691C: globals.__ = "" so a reference to _G encodes as "_t_".
    set_field_text(L, globals, kNameField, "");
    std::vector<std::string> table_globals;
    for_each_slot(L, globals, [&](int k, int v) {
        if (lua_istable(L, v)) table_globals.push_back(native_text(L, k));
    });
    // 006B6A4E..006B6E07: name every other global table "|<key>" first, so a
    // saved root that points at one stores a reference instead of a copy.
    for (const auto& name : table_globals) {
        if (same_name(name, kThisTable) || same_name(name, "recon") || same_name(name, "_G")) continue;
        bool root = false;
        for (const char* mission_root : kMissionRoots) root = root || same_name(name, mission_root);
        if (root) continue;
        name_tree(L, "", globals, string_key(name), true, 0);
    }
    // 006B6E18..006B6F22: every key of thisTable, whatever its value type.
    lua_getfield(L, globals, kThisTable);
    const int this_table = lua_gettop(L);
    std::vector<std::string> this_keys;
    for_each_slot(L, this_table, [&](int k, int) { this_keys.push_back(native_text(L, k)); });
    // 006B6F28..006B7057: thisTable.__ = "" + "|" + "thisTable".
    const std::string this_name = std::string("|") + kThisTable;
    set_field_text(L, this_table, kNameField, this_name);
    // 006B7060..006B71E2: thisTable[k].__ = "|thisTable|k" before any encoding,
    // so siblings reference each other by their own paths.
    for (const auto& name : this_keys) {
        push_slot(L, this_table, string_key(name));
        set_field_text(L, lua_gettop(L), kNameField, this_name + "|" + name);
        lua_pop(L, 1);
    }
    // 006B71F0..006B7292: forced encode of each thisTable member.
    for (const auto& name : this_keys)
        encode_tree(L, this_name, this_table, string_key(name), true, true, names, entities, 0);
    lua_pop(L, 1);
    // 006B72A0..006B7380: Mission, MissionGlobals, MessageSender when not nil,
    // through 006B5BF0 with flag 1 and force 0.
    for (const char* mission_root : kMissionRoots) {
        lua_getfield(L, globals, mission_root);
        const bool present = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (present) encode_tree(L, "", globals, string_key(mission_root), true, false, names, entities, 0);
    }
    // 006B7396..006B75C6: strip "__" from every table global except recon and _G.
    for (const auto& name : table_globals) {
        if (same_name(name, "recon") || same_name(name, "_G")) continue;
        push_slot(L, globals, string_key(name));
        if (lua_istable(L, -1)) strip_names(L, -1, 0);
        lua_pop(L, 1);
    }
}

void decode_table_tree(lua_State* L, int parent, const MissionLuaKey& key,
                       const MissionLuaEntityHooks& entities) {
    decode_tree(L, abs_index(L, parent), key, entities, 0);
}

// 006AEA00 / 006AF120: [ESP+4]=parent LuaObject, then key NativeString or
// float, RET 1Ch / RET 18h.
void decode_slot(lua_State* L, int parent, const MissionLuaKey& key,
                 const MissionLuaEntityHooks& entities) {
    parent = abs_index(L, parent);
    push_slot(L, parent, key);
    const std::string text = native_text(L, -1);
    lua_pop(L, 1);
    // 006AEA51: only strings of length 3 or more are examined.
    if (text.size() < 3) return;
    const std::string_view head(text.data(), 3);
    const std::string tail = text.substr(3);
    if (same_name(head, kTableRef)) {
        decode_table_ref(L, parent, key, tail);
    } else if (same_name(head, kFunctionRef)) {
        // 006AE16B..006AE1EE: parent[key] = globals[tail].
        push_slot(L, LUA_GLOBALSINDEX, string_key(tail));
        set_slot_value(L, parent, key, -1);
        lua_pop(L, 1);
    } else if (same_name(head, kEntityRef)) {
        // 006AE307..006AE37D: "x" is a null pointer; anything else is a
        // 16-bit handle from _atol. The result is a light userdata either way.
        void* entity = nullptr;
        if (tail.empty() || !same_name(tail, "x")) {
            if (!entities.resolve) throw std::invalid_argument("entity resolve hook is required");
            entity = entities.resolve(static_cast<std::uint16_t>(std::atol(tail.c_str())));
        }
        lua_pushlightuserdata(L, entity);
        set_slot_value(L, parent, key, -1);
        lua_pop(L, 1);
    }
}

void strip_visit_marks_006b1490(lua_State* L, int table) {
    strip_visits(L, table, 0);
}

void decode_mission_lua_state_006b23e0(lua_State* L, const MissionLuaEntityHooks& entities) {
    const int globals = LUA_GLOBALSINDEX;
    // 006B2421..006B2436: globals.__ = "" again; nothing removes it afterwards.
    set_field_text(L, globals, kNameField, "");
    std::vector<std::string> table_globals, string_globals;
    for_each_slot(L, globals, [&](int k, int v) {
        if (lua_istable(L, v)) table_globals.push_back(native_text(L, k));
        else if (lua_type(L, v) == LUA_TSTRING) string_globals.push_back(native_text(L, k));
    });
    // 006B25D8..006B28D2: walk every table global but recon, _G and _savedata.
    for (const auto& name : table_globals)
        if (!skipped_by_restore(name)) decode_tree(L, globals, string_key(name), entities, 0);
    // 006B28E0..006B2962: every string global, with no exclusions; this is how
    // a root saved as "_t_..." (for example Mission) comes back.
    for (const auto& name : string_globals) decode_slot(L, globals, string_key(name), entities);
    // 006B2970..006B2C0D: clear the visit marks under the same exclusions.
    for (const auto& name : table_globals) {
        if (skipped_by_restore(name)) continue;
        push_slot(L, globals, string_key(name));
        strip_visits(L, -1, 0);
        lua_pop(L, 1);
    }
}

void save_mission_lua_globals_006b7710(
    lua_State* L, MissionLuaFunctionNames& names, const MissionLuaEntityHooks& entities,
    const std::function<void(const char* global_name)>& emit_global) {
    register_global_functions_006b3dd0(L, names);       // 006B7AB2
    encode_mission_lua_state_006b68c0(L, names, entities); // 006B7AB7
    for (const char* mission_root : kMissionRoots) emit_global(mission_root); // 006B7ABC..006B7B4A
    emit_global(kThisTable);                             // 006B7B79
    decode_mission_lua_state_006b23e0(L, entities);      // 006B7BA1
    names.clear();                                       // 006B7BA6..006B7BCF
}

} // namespace bsp
