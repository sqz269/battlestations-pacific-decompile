#include "bsp/game_ship_avoidance_tuning_lua.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/ship_ai_settings_block.hpp"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <cstring>
#include <type_traits>

namespace bsp::game {
namespace {
constexpr std::uint32_t offsets[] = {0x194, 0x1d4, 0x1d8, 0x214, 0x218};
struct ReadContext { std::array<float, 5> values; };
static_assert(std::is_trivially_destructible_v<ReadContext>);
static_assert(std::is_trivially_destructible_v<NativeLuaStateStorage>);
static_assert(std::is_trivially_destructible_v<NativeLuaObjectStorage>);

int read_protected(lua_State* state) {
    auto& context = *static_cast<ReadContext*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    // This protected C frame starts with an empty Lua stack. Only trivial
    // objects cross Lua longjmp; native tracked slots are local to this owner.
    NativeLuaStateStorage owner;
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    NativeLuaObjectStorage globals, ship_globals, section;
    native_lua_globals_00b67980(owner, &globals);
    native_lua_get_by_name_00b67800(globals, &ship_globals, "ShipGlobals");
    std::size_t count = 0;
    const auto* records = ship_ai_settings_keys(count);
    const char* current_section = nullptr;
    std::size_t current_length = 0;
    for (std::size_t i = 0; i < context.values.size(); ++i) {
        const ShipAiSettingsKeyRecord* record = nullptr;
        for (std::size_t j = 0; j < count; ++j)
            if (records[j].settings_offset == offsets[i]) { record = &records[j]; break; }
        if (!record) return luaL_error(state, "avoidance tuning has no source for %d", static_cast<int>(offsets[i]));
        const char* separator = std::strchr(record->lua_path, '.');
        if (!separator || separator == record->lua_path || separator[1] == '\0' ||
            std::strchr(separator + 1, '.'))
            return luaL_error(state, "avoidance tuning has an unsupported source path");
        const std::size_t section_length = static_cast<std::size_t>(separator - record->lua_path);
        if (!current_section || section_length != current_length ||
            std::strncmp(current_section, record->lua_path, current_length) != 0) {
            if (current_section) destroy_native_lua_object_00b67700(section);
            char name[32];
            if (section_length >= sizeof name)
                return luaL_error(state, "avoidance tuning source section exceeds its buffer");
            std::memcpy(name, record->lua_path, section_length);
            name[section_length] = '\0';
            native_lua_get_by_name_00b67800(ship_globals, &section, name);
            current_section = record->lua_path;
            current_length = section_length;
        }
        NativeLuaObjectStorage field, element;
        native_lua_get_by_name_00b67800(section, &field, separator + 1);
        NativeLuaObjectStorage* value = &field;
        if (record->array_index != 0) {
            native_lua_get_by_index_00b67720(field, &element, record->array_index);
            value = &element;
        }
        if (record->getter == ShipAiSettingsGetter::kNumber)
            context.values[i] = native_lua_number_00b66270(*value);
        else if (record->getter == ShipAiSettingsGetter::kFloatOrDefault)
            context.values[i] = native_lua_number_or_00b66330(*value, record->loader_default);
        else return luaL_error(state, "avoidance tuning has an unsupported getter");
        if (record->array_index != 0) destroy_native_lua_object_00b67700(element);
        destroy_native_lua_object_00b67700(field);
    }
    if (current_section) destroy_native_lua_object_00b67700(section);
    destroy_native_lua_object_00b67700(ship_globals);
    destroy_native_lua_object_00b67700(globals);
    return 0;
}
} // namespace

bool read_ship_avoidance_tuning_lua(lua_State& state,
    std::array<float, 5>& output, std::string& error) {
    ReadContext context{};
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &context);
    lua_pushcclosure(&state, &read_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message ? message : "avoidance tuning raised a non-string Lua error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    output = context.values;
    error.clear();
    return true;
}
} // namespace bsp::game
