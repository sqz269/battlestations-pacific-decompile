#include "bsp/cc13_release_timer.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/gunnery_tables.hpp"

#include <cstdio>
#include <map>
#include <string>
#include <utility>

// Packet cc13-reltimer. docs/CC13_RELEASE_TIMER.md.

namespace bsp::cc13_release_timer {

bool unit_can_release_007bb110(bool channel_c_enabled, float channel_c_value,
                               bool bay_rack_holds_round_9c3) noexcept {
    // 007BB119 CMP byte [EAX],0 / 007BB11C JZ 007BB130 (AL = 1).
    if (!channel_c_enabled) return true;
    // 007BB123 UCOMISS / LAHF / TEST AH,44h / 007BB12E JP: only an ordered
    // equal clears PF, so only exactly 1.0f returns 1 here; NaN falls through.
    if (channel_c_value == kReseedHigh_00d7a24c) return true;
    // 007BB133-007BB142 SETZ AL on the bay byte.
    return !bay_rack_holds_round_9c3;
}

std::vector<bool> bomb_rack_bay_doors(bsp::game::GameMissionLuaHost& lua, int type_id) {
    // The flags are a property of the class, so they are read once per (Lua
    // host, class): the chunk below then runs once per class instead of on
    // every refused tick, and touches the host's last_status_/last_chunk_ only
    // that once. Cached only once BSPGun exists (n > 0, i.e. after the
    // gunnery flatten) and the chunk succeeded; the host is single-threaded
    // here.
    static std::map<std::pair<const void*, int>, std::vector<bool>> cache;
    const std::pair<const void*, int> cache_key{&lua, type_id};
    if (const auto it = cache.find(cache_key); it != cache.end()) return it->second;

    std::vector<bool> out;
    // One character per flattened BSPGun row p1..pn: '1' when
    // Platforms[p<i>_key].UseBayDoor is the boolean true (0096127A reads it
    // through the boolean reader, so only `true` sets the byte).
    char chunk[768];
    std::snprintf(chunk, sizeof(chunk),
        "local row = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
        "local g = type(row) == 'table' and row.BSPGun or nil\n"
        "local s = ''\n"
        "if type(g) == 'table' then\n"
        "  local plats = row.Platforms\n"
        "  for p = 1, (tonumber(g.n) or 0) do\n"
        "    local k = g['p' .. p .. '_key']\n"
        "    local pl = type(plats) == 'table' and k ~= nil and plats[k] or nil\n"
        "    s = s .. ((type(pl) == 'table' and pl.UseBayDoor == true) and '1' or '0')\n"
        "  end\n"
        "end\n"
        "return s\n",
        type_id);
    const int top = lua.lua_gettop();
    std::string flags;
    // A failed load or call leaves every flag false and is not cached, so the
    // next refused tick tries again.
    const bool ok =
        lua.luaL_loadbuffer(chunk, static_cast<int>(std::char_traits<char>::length(chunk)),
                            "bsp_release_timer_bay_doors") == 0
        && lua.lua_pcall(0, 1, 0) == 0;
    if (ok) flags = lua.lua_tolstring_at_top();
    lua.lua_settop(top);

    // The rack filter of GameUnitsHost::Impl::ordnance_racks, in its order.
    const int platforms = lua.read_vehicle_class_integer(type_id, "BSPGun", "n", 0);
    for (int p = 1; p <= platforms && p <= 64; ++p) {
        char key[32];
        std::snprintf(key, sizeof(key), "p%d_cat", p);
        if (lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1)
            != static_cast<int>(bsp::GunneryCategory::kBombPlatform)) continue;
        std::snprintf(key, sizeof(key), "p%d_dev", p);
        const int dev = lua.read_vehicle_class_integer(type_id, "BSPGun", key, -1);
        const std::string type = dev >= 0 ? lua.read_device_class_string(dev, "Type")
                                          : std::string();
        if (type != "BombPlatform" && type != "MultiBombPlatform") continue;
        const std::size_t i = static_cast<std::size_t>(p - 1);
        out.push_back(i < flags.size() && flags[i] == '1');
    }
    if (ok && platforms > 0) cache.emplace(cache_key, out);
    return out;
}

}  // namespace bsp::cc13_release_timer
