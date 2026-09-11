#include "bsp/lua_runtime_globals.hpp"
#include "bsp/game_settings.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>

namespace bsp {

LuaRuntimeGlobals make_initial_lua_runtime_globals_0108ff20() {
    // PE zero-fill; 00cd7ce0 only registers destructor 00ce0d60 with atexit.
    return LuaRuntimeGlobals{false, std::string{}};
}

const char* user_geography_region_00439100() noexcept {
    const GEOID location = GetUserGeoID(GEOCLASS_NATION); // 00439103, literal 10h
    if (location != GEOID_NOT_AVAILABLE) {
        char name[100];
        if (GetGeoInfoA(location, GEO_FRIENDLYNAME, name, 100, 0) != 0) {
            if (_stricmp(name, "United States") == 0 || _stricmp(name, "Canada") == 0) {
                return "USA"; // 0043916e -> 00ce4284
            }
            if (_stricmp(name, "Japan") == 0) {
                return "JAP"; // 00439165 -> 00ce4288
            }
        }
    }
    return "EU"; // 00439177 -> 00ce4280
}

const char* publish_lua_xbox_compatibility_008d44c0(
    GameSettingsBlock& settings, LuaRuntimeGlobals& globals, bool enabled,
    bool command_sink_present) noexcept {
    const char* command = set_xbox_compatibility_008d44c0(
        settings, enabled, command_sink_present);
    globals.x360comp = enabled; // native 008d44cc, before optional command delivery
    return command;
}

void publish_lua_region_008d6132(LuaRuntimeGlobals& globals, const std::string& region) {
    globals.region.assign(region.c_str()); // native strlen/resize/memcpy
}

} // namespace bsp
