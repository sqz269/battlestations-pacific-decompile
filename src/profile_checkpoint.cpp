#include "bsp/profile_checkpoint.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "bsp/cc13_seg87.hpp"                  // 00B65F80, 00B66530, 00B665D0, 00B66670, 00B66740
#include "bsp/game_native_string_process.hpp"  // the live 01090AA8 raw string-pool context
#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_lua_class_setters.hpp"     // 00B67460, 00B66790
#include "bsp/native_lua_field_setters.hpp"     // 00B67580
#include "bsp/native_lua_number_field.hpp"      // 00B67400
#include "bsp/native_lua_numeric_element.hpp"   // 00B66580
#include "bsp/native_lua_object_boolean.hpp"    // 00B673A0
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_string.hpp"
#include "bsp/oct18_tail_a.hpp"                 // 00436C50
#include "bsp/wire_goal4_chain274_switches.hpp"
#include "bsp/wire_goal4_chain299_switches.hpp"
extern "C" {
#include <lua.h>
}
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>  // IsProcessorFeaturePresent: the CRT's __sse2_available for 00B66290

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

// --- Goal 4 chain 299: the copies over the native 14h LuaObjects -----------

// 00B66290's CRT conversion selector (__sse2_available), as the other live
// LuaObject integer readers take it (src/game_hosts_lua.cpp).
const bool& crt_sse2_conversion() noexcept {
    static const bool flag = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    return flag;
}

// A borrowed owner over a live state (00B66BD0 then the state word), and an
// untracked kind-2 object over an existing absolute stack index: the value a
// native caller's LuaObject argument holds when it names a stack slot.
NativeLuaStateStorage& bind_owner(NativeLuaStateStorage& owner, lua_State* L) noexcept {
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = L;
    return owner;
}
NativeLuaObjectStorage stack_object(NativeLuaStateStorage& owner, int index) noexcept {
    NativeLuaObjectStorage object{};
    object.owner_00 = &owner;
    object.kind_04 = 2;
    object.index_08 = index;
    return object;
}

// 00B67700 on scope exit (kind 0 is a no-op), in the native reverse order.
struct NativeObjectCleanup {
    NativeLuaObjectStorage& object;
    ~NativeObjectCleanup() { destroy_native_lua_object_00b67700(object); }
};
// 0041E870 into a local 8-byte header, 0041DD20 on scope exit.
struct NativeKeyText {
    NativeString header;
    NativeStringRawPoolContext& strings;
    NativeKeyText(NativeStringRawPoolContext& pool, const char* text) : strings(pool) {
        construct_native_string_header_0041e870(&header, strings, text);
    }
    ~NativeKeyText() { destroy_native_string_header_0041dd20(&header, strings); }
    NativeKeyText(const NativeKeyText&) = delete;
    NativeKeyText& operator=(const NativeKeyText&) = delete;
};

// Host admission, not native. Each nested table level of either native copy
// holds three tracked LuaObjects (child table, iteration key, value) at
// consecutive absolute stack slots, and 00B67800/00B67720/00B67080 register
// them in owner slots_14[stack index] with no bound check; the owner has 50.
// The native caller's shallow profile tables meet that contract. Refuse a
// value nested deeper than the slots left (a cycle counts as unbounded) before
// any copy starts, instead of overrunning the owner.
constexpr int kNativeLuaTrackedSlots = 50;
int table_depth(lua_State* L, int index, int cap) {
    index = abs_index(L, index);
    if (lua_type(L, index) != LUA_TTABLE) return 0;
    if (cap <= 0) return 1;
    if (!lua_checkstack(L, 3)) throw std::length_error("Lua stack exhausted");
    int deepest = 0;
    lua_pushnil(L);
    while (lua_next(L, index)) {
        const int child = table_depth(L, -1, cap - 1);
        if (child > deepest) deepest = child;
        lua_pop(L, 1);
        if (deepest >= cap) {
            lua_pop(L, 1);
            break;
        }
    }
    return 1 + deepest;
}
void admit_native_copy(lua_State* L, int value, int highest_held_slot) {
    const int levels = (kNativeLuaTrackedSlots - 1 - highest_held_slot) / 3;
    if (levels < 0 || table_depth(L, value, levels) > levels)
        throw std::length_error("checkpoint table nesting exceeds the native LuaObject tracking slots");
}

struct CheckpointSlotContext {
    NativeStringRawPoolContext& strings;
    const bool& crt_sse2_conversion;
};

void copy_checkpoint_children(NativeLuaObjectStorage& child,
    NativeLuaObjectStorage& source, CheckpointSlotContext& context);

// 00436730 over the native objects: ECX (forwarded only to the recursion and
// never read) is dropped; destination/key/value as the three stacked
// LuaObjects. Listing: key type 4 (0043675D..00436765) takes the jump table
// 00436C20 on the value type; otherwise only key type 3 (00436A0B..00436A13)
// takes the table 00436C34, and every other key copies nothing. Values that
// are not boolean, number, string or table are skipped (00436C09).
void copy_checkpoint_slot_objects(NativeLuaObjectStorage& destination,
    NativeLuaObjectStorage& key, NativeLuaObjectStorage& value, CheckpointSlotContext& context) {
    auto integer = [&](const NativeLuaObjectStorage& object) {
        return native_lua_integer_00b66290(object, context.crt_sse2_conversion);
    };
    if (native_lua_type_00b65f80(key) == LUA_TSTRING) {
        switch (native_lua_type_00b65f80(value)) {
        case LUA_TBOOLEAN: {   // 00436788: 0041E870 name, 00B66250, 00B673A0
            NativeKeyText name(context.strings, native_lua_string_00b662b0(key));
            native_lua_set_boolean_00b673a0(destination, name.header,
                native_lua_boolean_00b66250(value) ? 1u : 0u);
            return;
        }
        case LUA_TNUMBER: {    // 00B66270, then the 00BF85B0 floor test
            const float number = native_lua_number_00b66270(value);
            if (!float32_is_whole(number)) {
                NativeKeyText name(context.strings, native_lua_string_00b662b0(key));
                native_lua_set_number_field_00b67400(destination, name.header,
                    native_lua_number_00b66270(value));
            } else {
                NativeKeyText name(context.strings, native_lua_string_00b662b0(key));
                native_lua_set_integer_00b67460(destination, name.header, integer(value));
            }
            return;
        }
        case LUA_TSTRING:      // 00B66790 with both C-strings
            native_lua_set_cstring_00b66790(destination, native_lua_string_00b662b0(key),
                native_lua_string_00b662b0(value));
            return;
        case LUA_TTABLE: {     // 00B67580 then 00B67800 by the C-string key
            {
                NativeKeyText name(context.strings, native_lua_string_00b662b0(key));
                native_lua_set_new_table_00b67580(destination, name.header);
            }
            NativeLuaObjectStorage child;
            native_lua_get_by_name_00b67800(destination, &child, native_lua_string_00b662b0(key));
            NativeObjectCleanup child_cleanup{child};
            copy_checkpoint_children(child, value, context);
            return;
        }
        default:
            return;
        }
    }
    if (native_lua_type_00b65f80(key) != LUA_TNUMBER) return;   // 00436A13
    switch (native_lua_type_00b65f80(value)) {
    case LUA_TBOOLEAN: {       // 00B66250, 00B66290(key), 00B66530
        const bool flag = native_lua_boolean_00b66250(value);
        native_lua_set_boolean_at_00b66530(destination, integer(key), flag ? 1u : 0u);
        return;
    }
    case LUA_TNUMBER: {        // float32 spill, 004355C0 floor, then 00B66580 or 00B665D0
        const float number = native_lua_number_00b66270(value);
        if (!float32_is_whole(number)) {
            const float element = native_lua_number_00b66270(value);
            set_native_lua_numeric_element_00b66580(&destination, nullptr, integer(key), element);
        } else {
            const std::int32_t n = integer(value);
            native_lua_set_integer_at_00b665d0(destination, integer(key), n);
        }
        return;
    }
    case LUA_TSTRING: {        // 00B662B0, 00B66290(key), 00B66740
        const char* const text = native_lua_string_00b662b0(value);
        native_lua_set_string_at_00b66740(destination, integer(key), text);
        return;
    }
    case LUA_TTABLE: {         // 00436A36: 00B66670, then 00B67720 by the same key
        native_lua_set_new_table_at_00b66670(destination, integer(key));
        NativeLuaObjectStorage child;
        native_lua_get_by_index_00b67720(destination, &child, integer(key));
        NativeObjectCleanup child_cleanup{child};
        copy_checkpoint_children(child, value, context);
        return;
    }
    default:
        return;
    }
}

// The 00436730 table arm's loop: 00B65F50 twice, 00B67080, then 00436730 per
// slot while 00B66420 reports the key bound, 00B67190 between; the value and
// key objects are destroyed before the caller's child.
void copy_checkpoint_children(NativeLuaObjectStorage& child,
    NativeLuaObjectStorage& source, CheckpointSlotContext& context) {
    NativeLuaObjectStorage key;
    construct_native_lua_object_00b65f50(&key);
    NativeObjectCleanup key_cleanup{key};
    NativeLuaObjectStorage value;
    construct_native_lua_object_00b65f50(&value);
    NativeObjectCleanup value_cleanup{value};
    native_lua_iterate_first_00b67080(source, key, value);
    while (!native_lua_is_unbound_00b66420(key)) {
        copy_checkpoint_slot_objects(child, key, value, context);
        native_lua_iterate_next_00b67190(source, key, value);
    }
}

// The ProfileLuaServices 00436C50 calls, bound to the existing bodies.
class CheckpointProfileLuaServices final : public oct18_tail_a::ProfileLuaServices {
public:
    std::int32_t type_00b65f80(const NativeLuaObjectStorage& object) override {
        return native_lua_type_00b65f80(object);
    }
    double floor_00bf85b0(double value) override { return std::floor(value); }
    void boolean_index_00b66530(NativeLuaObjectStorage& object, std::int32_t key,
                                std::uint32_t value) override {
        native_lua_set_boolean_at_00b66530(object, key, static_cast<std::uint8_t>(value));
    }
    void integer_index_00b665d0(NativeLuaObjectStorage& object, std::int32_t key,
                                std::int32_t value) override {
        native_lua_set_integer_at_00b665d0(object, key, value);
    }
    void string_index_00b66740(NativeLuaObjectStorage& object, std::int32_t key,
                               const char* value) override {
        native_lua_set_string_at_00b66740(object, key, value);
    }
    void table_index_00b66670(NativeLuaObjectStorage& object, std::int32_t key) override {
        native_lua_set_new_table_at_00b66670(object, key);
    }
};

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
        if (lua_istable(L, value) && !same_name(lua_text(L, value - 1), mission_id)) {
            if constexpr (kWireGoal4Chain274Bound) {
                // 004388C6 (00438020) and 00437405 (00437250): CALL 00435D30.
                write_lua_slot_archive_00435d30(L, value - 1, value, writer);
            } else {
            write_slot(L, value - 1, value, writer, 0);
            }
        }
        lua_pop(L, 1);
    }
}

} // namespace

void write_lua_slot_archive_00435d30(lua_State* L, int key, int value, SettingsWriter& writer) {
    write_slot(L, key, value, writer, 0);
}

void copy_lua_slot_00436730(lua_State* source, int key, int value, lua_State* destination, int table) {
    if constexpr (kWireGoal4Chain299Bound) {
        // The three stacked LuaObjects name existing stack slots; one owner
        // when both sides are the same state, so its tracked indices stay
        // consistent across the copy's pushes and removals.
        key = abs_index(source, key);
        value = abs_index(source, value);
        table = abs_index(destination, table);
        const int held = lua_gettop(source) > lua_gettop(destination)
            ? lua_gettop(source) : lua_gettop(destination);
        admit_native_copy(source, value, held);
        NativeLuaStateStorage source_owner;
        NativeLuaStateStorage destination_owner;
        bind_owner(source_owner, source);
        NativeLuaStateStorage& to = destination == source
            ? source_owner : bind_owner(destination_owner, destination);
        NativeLuaObjectStorage destination_object = stack_object(to, table);
        NativeLuaObjectStorage key_object = stack_object(source_owner, key);
        NativeLuaObjectStorage value_object = stack_object(source_owner, value);
        CheckpointSlotContext context{game::game_native_string_process().raw_context(),
                                      crt_sse2_conversion()};
        copy_checkpoint_slot_objects(destination_object, key_object, value_object, context);
    } else {
        copy_slot(source, key, value, destination, table, 0);
    }
}

void save_checkpoint_00438020(lua_State* mission, const std::string& mission_id,
                              const CheckpointRecordFields& record, float play_time,
                              lua_State* script, int data_table, SettingsWriter* writer,
                              bool include_other_missions, const char* empty_0108ff2c) {
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
    if constexpr (kWireGoal4Chain299Bound) {
        // 0043844D..004384DB: key [ESP+64h] and value [ESP+B4h] from 00B65F50,
        // 00B67080 over the data table, then per slot 00436C50(ECX manager,
        // destination entry, key, value) while 00B66420(key) reports bound,
        // 00B67190 between. The manager ECX (EBP, 00438045) is only forwarded
        // by 00436C50 to its own recursion; this host has no profile-manager
        // object, so nothing is passed for it. One owner when the script table
        // and the checkpoint entry share the state, as at MW_SaveCheckpoint.
        if (empty_0108ff2c == nullptr)
            throw std::logic_error("00436C50 needs the 0108FF2C empty-string authority");
        const int held = (lua_gettop(script) > lua_gettop(mission)
            ? lua_gettop(script) : lua_gettop(mission)) + 2;   // the iteration key and value
        lua_pushnil(script);
        while (lua_next(script, data_table)) {
            admit_native_copy(script, -1, held);
            lua_pop(script, 1);
        }
        NativeLuaStateStorage script_owner;
        NativeLuaStateStorage mission_owner;
        bind_owner(script_owner, script);
        NativeLuaStateStorage& to = mission == script
            ? script_owner : bind_owner(mission_owner, mission);
        NativeLuaObjectStorage data = stack_object(script_owner, data_table);
        NativeLuaObjectStorage destination = stack_object(to, entry);
        NativeLuaObjectStorage key;
        construct_native_lua_object_00b65f50(&key);
        NativeObjectCleanup key_cleanup{key};
        NativeLuaObjectStorage value;
        construct_native_lua_object_00b65f50(&value);
        NativeObjectCleanup value_cleanup{value};
        CheckpointProfileLuaServices services;
        oct18_tail_a::ProfileLuaContext context{game::game_native_string_process().raw_context(),
            crt_sse2_conversion(), empty_0108ff2c, services};
        native_lua_iterate_first_00b67080(data, key, value);
        while (!native_lua_is_unbound_00b66420(key)) {
            oct18_tail_a::copy_profile_lua_value_00436c50(nullptr, destination, key, value, context);
            native_lua_iterate_next_00b67190(data, key, value);
        }
    } else {
    lua_pushnil(script);
    while (lua_next(script, data_table)) {
        const int value = lua_gettop(script);
        copy_slot(script, value - 1, value, mission, entry, 0);
        lua_pop(script, 1);
    }
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
            if constexpr (kWireGoal4Chain274Bound) {
                // 00438747: CALL 00435D30 per script slot.
                write_lua_slot_archive_00435d30(script, value - 1, value, *writer);
            } else {
            write_slot(script, value - 1, value, *writer, 0);
            }
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
        if constexpr (kWireGoal4Chain274Bound) {
            // 00437B21: CALL 00436730 per entry slot.
            copy_lua_slot_00436730(mission, value - 1, value, destination, copy);
        } else {
        copy_slot(mission, value - 1, value, destination, copy, 0);
        }
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
