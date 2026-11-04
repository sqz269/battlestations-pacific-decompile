// Focused, offline Lua adapter check. Run from the installation directory so
// ShipGlobals' own DoFile("scripts/datatables/ScriptOptions.lua") reads it.
// Link bsp_core/bsp_lua511/bsp_zlib121 and /MANIFEST:EMBED; never launches game.
#include "bsp/gameplay_tuning_lua_rows.hpp"
#include "bsp/game_ship_avoidance_tuning_lua.hpp"
#include "bsp/lua_binding_core.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <float.h>

namespace {
int deg(lua_State* state) {
    lua_pushnumber(state, bsp::lua_binding_degrees_to_radians(lua_tonumber(state, 1)));
    return 1;
}
int do_file(lua_State* state) {
    const char* name = luaL_checkstring(state, 1);
    if (luaL_loadfile(state, name) != 0 || lua_pcall(state, 0, 0, 0) != 0)
        return lua_error(state);
    return 0;
}
bool check(bool ok, const char* what) {
    if (!ok) std::fprintf(stderr, "FAIL: %s\n", what);
    return ok;
}
}

int main() {
    unsigned control;
    _controlfp_s(&control, _PC_24, _MCW_PC);
    lua_State* state = luaL_newstate();
    if (!state) return 2;
    lua_register(state, "DEG", deg);
    lua_register(state, "DoFile", do_file);
    if (luaL_dofile(state, "scripts/datatables/shipglobals.lua") != 0) {
        std::fprintf(stderr, "actual ShipGlobals: %s\n", lua_tostring(state, -1));
        return 2;
    }
    lua_pushliteral(state, "stack sentinel");
    const int top = lua_gettop(state);
    bsp::GameplayTuningSettings settings{};
    bsp::apply_constructor_defaults_00424a10(settings);
    std::string error;
    const bool loaded = bsp::read_gameplay_tuning_lua(*state, settings, true, error);
    bool ok = check(loaded, error.c_str());
    ok &= check(lua_gettop(state) == top, "successful read preserves caller stack");

    // The existing offsets reader is an independent, established producer of
    // every float consumed by the neighbour and avoidance-tuning paths.
    std::array<std::uint32_t, 24> offsets;
    for (std::uint32_t i = 0; i < 19; ++i) offsets[i] = 0x190 + 4 * i;
    const std::uint32_t selected[] = {0x194, 0x1d4, 0x1d8, 0x214, 0x218};
    for (std::size_t i = 0; i < 5; ++i) offsets[19 + i] = selected[i];
    std::array<float, 24> legacy{};
    const bool legacy_loaded = bsp::game::read_ship_ai_settings_offsets_lua(*state, offsets.data(),
        legacy.data(), legacy.size(), error);
    ok &= check(legacy_loaded, error.c_str());
    const auto* bytes = reinterpret_cast<const unsigned char*>(&settings);
    for (std::size_t i = 0; i < offsets.size(); ++i)
        ok &= check(std::memcmp(bytes + offsets[i], &legacy[i], sizeof(float)) == 0,
            "live-consumer field differs from established reader");

    // Fail after earlier sections were read: the output must remain whole.
    const auto saved = settings;
    if (luaL_dostring(state, "ShipGlobals.Navigator=nil") != 0) return 2;
    ok &= check(!bsp::read_gameplay_tuning_lua(*state, settings, true, error),
        "missing section must report a protected Lua error");
    ok &= check(std::memcmp(&settings, &saved, sizeof settings) == 0,
        "failed read publishes no partial settings");
    ok &= check(lua_gettop(state) == top, "failed read preserves caller stack");
    std::printf("actual ShipGlobals: 24 consumed fields bit-identical; stack and failed-read preservation %s\n",
        ok ? "PASS" : "FAIL");
    lua_close(state);
    return ok ? 0 : 1;
}
