#include "bsp/ship_ai_path_turn_ramp.hpp"

#include "bsp/gameplay_settings.hpp"
#include "bsp/native_lua_objects.hpp"

#include <cstdint>
#include <cstring>
extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {
float float_bits(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

int read_ramp_protected(lua_State* state) {
    auto& output = *static_cast<ShipAiPathSearchTurnRamp*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    // This fresh C frame begins with no arguments. At most four tracked stack
    // positions are live, within the native owner's50-slot/5-reference bounds.
    // Every local here is trivial: a Lua longjmp crosses no C++ destructor.
    NativeLuaStateStorage owner;
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state; // Explicit borrowed adapter binding, no DoFile write.
    NativeLuaObjectStorage globals, ship_globals, navigator, path_finder;
    native_lua_globals_00b67980(owner, &globals);
    native_lua_get_by_name_00b67800(globals, &ship_globals, "ShipGlobals");
    native_lua_get_by_name_00b67800(ship_globals, &navigator, "Navigator");
    native_lua_get_by_name_00b67800(navigator, &path_finder, "PathFinderParams");
    load_ship_ai_path_turn_ramp_0083d492(path_finder, output);
    destroy_native_lua_object_00b67700(path_finder);
    destroy_native_lua_object_00b67700(navigator);
    destroy_native_lua_object_00b67700(ship_globals);
    destroy_native_lua_object_00b67700(globals);
    return 0;
}
} // namespace

void load_ship_ai_path_turn_ramp_0083d492(NativeLuaObjectStorage& path_finder,
    ShipAiPathSearchTurnRamp& output) {
    NativeLuaObjectStorage value;
    native_lua_get_by_name_00b67800(path_finder, &value,
        "LengthModifier_DirDiffMin"); //0083D4A6; RET8.
    output.knee_x = native_lua_number_or_00b66330(value,
        float_bits(0x3e860a92u)); //0083D4BF/C4;00D05AA8.
    destroy_native_lua_object_00b67700(value); //0083D4D9.

    native_lua_get_by_name_00b67800(path_finder, &value,
        "LengthModifier_DirDiffMax"); //0083D4F2.
    output.limit_x = native_lua_number_or_00b66330(value,
        float_bits(0x3fc90fdbu)); //0083D50B/510;00CE3C64.
    destroy_native_lua_object_00b67700(value); //0083D525.

    native_lua_get_by_name_00b67800(path_finder, &value,
        "LengthModifier_LengthAddon"); //0083D53E.
    output.limit_y = native_lua_number_or_00b66330(value,
        float_bits(0x44bb8000u)); //0083D557/55C;00CED724.
    destroy_native_lua_object_00b67700(value); //0083D571.
}

bool read_ship_ai_path_turn_ramp_lua(lua_State& state,
    ShipAiPathSearchTurnRamp& output, std::string& error) {
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &output);
    lua_pushcclosure(&state, &read_ramp_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message ? message : "path turn ramp Lua lookup raised a non-string error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    error.clear();
    return true;
}

ShipAiPathSearchTurnRamp ship_ai_path_turn_ramp_from_settings(
    const GameplayTuningSettings& settings) noexcept {
    return {
        settings.navigator_path_finder_params_length_modifier_dir_diff_min,
        settings.navigator_path_finder_params_length_modifier_dir_diff_max,
        settings.navigator_path_finder_params_length_modifier_length_addon};
}
} // namespace bsp
