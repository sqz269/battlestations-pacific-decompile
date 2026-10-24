#include "bsp/cc13_release_timer.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/gunnery_tables.hpp"
#include "bsp/torpedo_release_spawn.hpp"

#include <cstdio>
#include <map>
#include <string>

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
    // The flags are a property of the class, so they are read once per class
    // and mission: the chunk below then runs once per class instead of on
    // every refused tick, and touches the host's last_status_/last_chunk_ only
    // that once. Cached only once BSPGun exists (n > 0, i.e. after the
    // gunnery flatten) and the chunk succeeded; the host is single-threaded
    // here. Packet cc13-baystale: the cache is the Lua host's own member, which
    // dies with the host at the next mission load. It was a function-local
    // static keyed by the host's address, so a later mission's host at a
    // reused address got the previous mission's flags.
    std::map<int, std::vector<bool>>& cache = lua.bomb_rack_bay_doors_cache();
    if (const auto it = cache.find(type_id); it != cache.end()) return it->second;

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
    if (ok && platforms > 0) cache.emplace(type_id, out);
    return out;
}

std::uint8_t pilot_step_flag_df0_0099b055(int mode, bool has_leader, bool leader_9c1,
                                          bool own_9c3) noexcept {
    // 0099B006 CMP EBP,2 / SETE DL -> [ESP+10h], the slot 0099B04D reads as
    // [ESP+0Ch] after 0099B019 POP EBP.
    bool flag = mode == 2;
    // 0099B012 CMP EBP,1 / JNE: only mode 1 takes the leader's 9C1h
    // (0099B03B-0099B049).
    if (mode == 1 && has_leader) flag = leader_9c1;
    // 007BB154 TEST AL,AL / JZ: +DF0h = flag ? 9C3h : 0.
    return flag && own_9c3 ? 1 : 0;
}

bool step_byte_9c1_007cdc84(bool channel_c_enabled, float channel_c_value) noexcept {
    // 007CDC7E CMP byte [EAX+60h],0 / JZ -> 1; 007CDC89 UCOMISS / JP 007CDC9A
    // (0) unless ordered equal.
    if (!channel_c_enabled) return true;
    return channel_c_value == kReseedHigh_00d7a24c;
}

bool plane_bay_target_007c6a8d(bsp::PlaneActuatorBlock& block, bool suppress_520,
                               bool drop_kamikaze_2f, bool df0, bool c25, bool c20,
                               bool own_9c3) noexcept {
    // 007C6A8D CMP byte [EBP+520h],0 / JNE 007C6C23 skips the whole arm.
    if (suppress_520) return false;
    bsp::PlaneActuatorChannel& c = block.channel_c;
    bool wrote = false;
    // 007C6AC8-007C6AD5 007B91C0(plane; 2Fh, 0).
    if (drop_kamikaze_2f) {
        // 007C6ADD CMP byte [ECX+61h],1 / JNE write; 007C6AE8 UCOMISS +64h,
        // 1.0f / JNP skip (ordered equal skips).
        if (!c.target || !(c.value == kReseedHigh_00d7a24c)) {
            c.value = kReseedHigh_00d7a24c;  // 007C6AFD
            c.target = true;                 // 007C6B02
            c.moving = true;                 // 007C6B06
            wrote = true;
        }
    } else {
        // 007C6A9A-007C6AC1: BL = (+DF0h || +C25h || +C20h) ? 9C3h : 0.
        const bool bl = (df0 || c25 || c20) && own_9c3;
        if (c.target != bl) {                // 007C6B1C / 007C6B1F
            c.target = bl;                   // 007C6B21
            c.moving = true;                 // 007C6B24
            wrote = true;
        }
    }
    // 007C6B0A / 007C6B28 CMP byte [+68h],0 / JE; 007C6B10 / 007C6B2E +11h = 1.
    if (c.moving) block.moving_any = true;
    return wrote;
}

}  // namespace bsp::cc13_release_timer
