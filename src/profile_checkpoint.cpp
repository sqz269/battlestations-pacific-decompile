#include "bsp/profile_checkpoint.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "bsp/gui_lua_reader.hpp"
extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

constexpr const char* kCheckpoints = "BSP_Chk_Save"; // 00CE4124
constexpr int kDepthLimit = 4096;                    // host guard, not native

int abs_index(lua_State* L, int index) {
    return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

// 00B66290: float32 then __ftol; 80000000h outside the 32-bit range.
std::int32_t lua_integer_00b66290(lua_State* L, int index) {
    const float value = static_cast<float>(lua_tonumber(L, index));
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

// 00BF85B0 / 004355C0 floor test on the float32 value: x - floor(x) == 0.
// Infinity and NaN give NaN, which compares unordered and takes the float arm.
bool float32_is_whole(float value) {
    const double wide = value;
    return wide - std::floor(wide) == 0.0;
}

bool same_name(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

std::string lua_text(lua_State* L, int index) {
    lua_pushvalue(L, index);
    const char* text = lua_tostring(L, -1);
    std::string result = text ? text : "";
    lua_pop(L, 1);
    return result;
}

GuiLuaVariant text_key(const char* text) {
    GuiLuaVariant key;
    key.tag = 0;
    key.value.text = text;
    return key;
}

void write_slot(lua_State* L, int key, int value, SettingsWriter& writer, int depth) {
    if (depth > kDepthLimit) throw std::length_error("checkpoint table nesting exceeds the host limit");
    if (!lua_checkstack(L, 6)) throw std::length_error("Lua stack exhausted");
    key = abs_index(L, key);
    value = abs_index(L, value);
    // 00435D5D..00435D7F: number -> integer key, string -> string key, else integer 0.
    GuiLuaVariant variant;
    std::string key_text;
    switch (lua_type(L, key)) {
    case LUA_TNUMBER:
        variant.tag = 1;
        variant.value.integer = lua_integer_00b66290(L, key);
        break;
    case LUA_TSTRING:
        key_text = lua_text(L, key);
        variant = text_key(key_text.c_str());
        break;
    default:
        variant.tag = 1;
        variant.value.integer = 0;
        break;
    }
    // 00435D85..00435D96: switch on the value type, jump table 00435F28.
    switch (lua_type(L, value)) {
    case LUA_TBOOLEAN:
        writer.write_field(variant, SettingsValue::from_bool(lua_toboolean(L, value) != 0));
        break;
    case LUA_TNUMBER: {
        const float number = static_cast<float>(lua_tonumber(L, value));
        if (float32_is_whole(number))
            writer.write_field(variant, SettingsValue::from_int(lua_integer_00b66290(L, value)));
        else
            writer.write_field(variant, SettingsValue::from_float(number));
        break;
    }
    case LUA_TSTRING: {
        const std::string text = lua_text(L, value);
        writer.write_field(variant, SettingsValue::from_text(text.c_str()));
        break;
    }
    case LUA_TTABLE:
        writer.begin_section(variant);
        lua_pushnil(L);
        while (lua_next(L, value)) {
            const int child = lua_gettop(L);
            write_slot(L, child - 1, child, writer, depth + 1);
            lua_pop(L, 1);
        }
        writer.end_section();
        break;
    default: // nil, light userdata, function, userdata, thread
        break;
    }
}

void copy_slot(lua_State* source, int key, int value, lua_State* destination, int table, int depth) {
    if (depth > kDepthLimit) throw std::length_error("checkpoint table nesting exceeds the host limit");
    if (!lua_checkstack(source, 4) || !lua_checkstack(destination, 6))
        throw std::length_error("Lua stack exhausted");
    key = abs_index(source, key);
    value = abs_index(source, value);
    table = abs_index(destination, table);
    const int type = lua_type(source, value);
    if (type != LUA_TBOOLEAN && type != LUA_TNUMBER && type != LUA_TSTRING && type != LUA_TTABLE)
        return; // jump table 00436C20: light userdata and the rest are skipped
    // 0043675D..00436765: string keys by C-string prefix, everything else
    // through 00B66290.
    if (lua_type(source, key) == LUA_TSTRING) lua_pushstring(destination, lua_text(source, key).c_str());
    else lua_pushnumber(destination, lua_integer_00b66290(source, key));
    switch (type) {
    case LUA_TBOOLEAN:
        lua_pushboolean(destination, lua_toboolean(source, value));
        break;
    case LUA_TNUMBER: {
        const float number = static_cast<float>(lua_tonumber(source, value));
        if (float32_is_whole(number)) lua_pushnumber(destination, lua_integer_00b66290(source, value));
        else lua_pushnumber(destination, number);
        break;
    }
    case LUA_TSTRING:
        lua_pushstring(destination, lua_text(source, value).c_str());
        break;
    case LUA_TTABLE: {
        // 004367AE then 004367D6: create and assign the child, then fill it.
        lua_newtable(destination);
        lua_pushvalue(destination, -2);
        lua_pushvalue(destination, -2);
        lua_settable(destination, table);
        const int child = lua_gettop(destination);
        lua_pushnil(source);
        while (lua_next(source, value)) {
            const int slot = lua_gettop(source);
            copy_slot(source, slot - 1, slot, destination, child, depth + 1);
            lua_pop(source, 1);
        }
        lua_pop(destination, 2);
        return;
    }
    }
    lua_settable(destination, table);
}

// Pushes BSP_Chk_Save from the mission globals.
void push_checkpoints(lua_State* L) {
    lua_getfield(L, LUA_GLOBALSINDEX, kCheckpoints);
}

void write_other_entries(lua_State* L, int checkpoints, const std::string& mission_id,
                         SettingsWriter& writer) {
    if (!lua_istable(L, checkpoints)) throw std::runtime_error("BSP_Chk_Save is not a table");
    lua_pushnil(L);
    while (lua_next(L, checkpoints)) {
        const int value = lua_gettop(L);
        if (lua_istable(L, value) && !same_name(lua_text(L, value - 1), mission_id))
            write_slot(L, value - 1, value, writer, 0);
        lua_pop(L, 1);
    }
}

} // namespace

void write_lua_slot_archive_00435d30(lua_State* L, int key, int value, SettingsWriter& writer) {
    write_slot(L, key, value, writer, 0);
}

void copy_lua_slot_00436730(lua_State* source, int key, int value, lua_State* destination, int table) {
    copy_slot(source, key, value, destination, table, 0);
}

void save_checkpoint_00438020(lua_State* mission, const std::string& mission_id,
                              const CheckpointRecordFields& record, float play_time,
                              lua_State* script, int data_table, SettingsWriter* writer,
                              bool include_other_missions) {
    data_table = abs_index(script, data_table);
    push_checkpoints(mission);
    const int checkpoints = lua_gettop(mission);
    if (!lua_istable(mission, checkpoints)) throw std::runtime_error("BSP_Chk_Save is not a table");
    // 0043821C..00438246: a fresh table under the mission id, then read back.
    lua_pushstring(mission, mission_id.c_str());
    lua_newtable(mission);
    lua_settable(mission, checkpoints);
    lua_getfield(mission, checkpoints, mission_id.c_str());
    const int entry = lua_gettop(mission);
    // 00438278..00438426: the five fields.
    lua_pushnumber(mission, record.difficulty);
    lua_setfield(mission, entry, "__difficulty");
    lua_pushnumber(mission, record.unlock_from);
    lua_setfield(mission, entry, "__unlockFrom");
    lua_pushnumber(mission, record.unlock_to);
    lua_setfield(mission, entry, "__unlockTo");
    lua_pushlstring(mission, record.unlock_name.data(), record.unlock_name.size());
    lua_setfield(mission, entry, "__unlockName");
    lua_pushnumber(mission, play_time);
    lua_setfield(mission, entry, "__playTime");
    // 0043846F..004384DB: copy the script table through 00436C50.
    lua_pushnil(script);
    while (lua_next(script, data_table)) {
        const int value = lua_gettop(script);
        copy_slot(script, value - 1, value, mission, entry, 0);
        lua_pop(script, 1);
    }
    if (writer) {
        // 0043856B..004386D0.
        writer->begin_section(kCheckpoints);
        writer->begin_section(mission_id.c_str());
        writer->write_field("__difficulty", SettingsValue::from_int(record.difficulty));
        writer->write_field("__unlockFrom", SettingsValue::from_int(record.unlock_from));
        writer->write_field("__unlockTo", SettingsValue::from_int(record.unlock_to));
        writer->write_field("__unlockName", SettingsValue::from_text(record.unlock_name.c_str()));
        writer->write_field("__playTime", SettingsValue::from_float(play_time));
        // 004386F5..00438774: the script table slots, from the script state.
        lua_pushnil(script);
        while (lua_next(script, data_table)) {
            const int value = lua_gettop(script);
            write_slot(script, value - 1, value, *writer, 0);
            lua_pop(script, 1);
        }
        writer->end_section();
        // 0043877F..004388F3: the other missions at BSP_Chk_Save level.
        if (include_other_missions) write_other_entries(mission, checkpoints, mission_id, *writer);
        writer->end_section();
    }
    lua_pop(mission, 2);
}

std::optional<float> load_checkpoint_004379c0(lua_State* mission, const std::string& mission_id,
                                              lua_State* destination) {
    push_checkpoints(mission);
    if (!lua_istable(mission, -1)) throw std::runtime_error("BSP_Chk_Save is not a table");
    lua_getfield(mission, -1, mission_id.c_str());
    const int entry = lua_gettop(mission);
    if (!lua_istable(mission, entry)) {
        lua_pop(mission, 2);
        lua_pushnil(destination); // 00437B87
        return std::nullopt;
    }
    // 00437A53..00437A99: 00B66330 with default 0.0, actual numbers only.
    lua_getfield(mission, entry, "__playTime");
    const float play_time = lua_type(mission, -1) == LUA_TNUMBER
        ? static_cast<float>(lua_tonumber(mission, -1)) : 0.0f;
    lua_pop(mission, 1);
    // 00437AB8..00437B49: a new table on the destination, filled by 00436730.
    lua_newtable(destination);
    const int copy = lua_gettop(destination);
    lua_pushnil(mission);
    while (lua_next(mission, entry)) {
        const int value = lua_gettop(mission);
        copy_slot(mission, value - 1, value, destination, copy, 0);
        lua_pop(mission, 1);
    }
    lua_pop(mission, 2);
    return play_time;
}

void clear_checkpoint_00437c70(lua_State* mission, const std::string& mission_id) {
    push_checkpoints(mission);
    if (!lua_istable(mission, -1)) throw std::runtime_error("BSP_Chk_Save is not a table");
    lua_pushnil(mission);
    lua_setfield(mission, -2, mission_id.c_str()); // 00437CD7
    lua_pop(mission, 1);
}

void write_other_checkpoints_00437250(lua_State* mission, const std::string& mission_id,
                                      SettingsWriter& writer) {
    writer.begin_section(kCheckpoints); // 004372C1
    push_checkpoints(mission);
    // 0043730A: 00B65F70 tests only that the wrapper is bound, which a named
    // lookup always is; a non-table BSP_Chk_Save is then walked natively, and
    // the host throws instead.
    write_other_entries(mission, lua_gettop(mission), mission_id, writer);
    lua_pop(mission, 1);
    writer.end_section(); // 00437459
}

std::string format_play_time_0043cc20(float seconds) {
    // 0043CC5B: CVTTSS2SI, 80000000h when out of range.
    const std::int32_t total = !(seconds >= -2147483648.0f && seconds < 2147483648.0f)
        ? std::numeric_limits<std::int32_t>::min() : static_cast<std::int32_t>(seconds);
    std::string out;
    const bool hours = total >= 3600;                               // 0043CC60
    if (hours) out += "." + std::to_string(total / 3600) + "|.:|";  // 00CE3A70, 00CE43C0
    const std::int32_t minutes = (total / 60) % 60;                 // 0043CCA5..0043CCC0
    out += "." + std::string(hours && minutes < 10 ? "0" : "") + std::to_string(minutes) + "|.:|";
    const std::int32_t secs = total - (total / 60) * 60;            // 0043CDEF..0043CDF7
    out += "." + std::string(secs < 10 ? "0" : "") + std::to_string(secs);  // 00CE43BC
    return out;
}

} // namespace bsp
