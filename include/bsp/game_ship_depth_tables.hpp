#pragma once
#include "bsp/gameplay_tuning_lua_rows.hpp"

#include <array>
#include <cstring>

namespace bsp::game {
namespace ship_depth_tables_detail {
// 00841B7B..008425D8, the two depth-table passes of the existing 0083B5E0
// loader. This fills its previously unrepresented +80..15F bytes, not another
// 0083B5E0 body. The table is captured with the load-time ShipGlobals owner;
// class readers must not reread mutable Lua tables after this publication.
// Native store order starts with BattleShip (00841BEE/00841C4A), then +00..24,
// +30..44, +48..58 and +5C..6C. Each element repeats GetByName/GetByIndex /
// bare GetInteger / destroy element / destroy field; no fallback is supplied.
struct Entry { const char* key; std::size_t offset; int count; };
inline constexpr Entry entries[] = {
    {"BattleShip", 0x28, 2}, {"MotherShip", 0x00, 2},
    {"Destroyer", 0x08, 2}, {"TBoat", 0x10, 2},
    {"SmallLandingShip", 0x18, 2}, {"LargeLandingShip", 0x20, 2},
    {"CargoShip", 0x30, 2}, {"LightCruiser", 0x38, 2},
    {"HeavyCruiser", 0x40, 2}, {"MiniSub", 0x48, 5},
    {"Submarine", 0x5c, 5},
};
struct ReadContext {
    gameplay_tuning_lua_detail::Rows& rows;
    std::array<std::byte, 2 * kAvoidZoneDepthsRecordSize> bytes;
};
inline int read_protected(lua_State* state) {
    auto& context = *static_cast<ReadContext*>(lua_touserdata(state, lua_upvalueindex(1)));
    context.rows.start(state);
    for (std::size_t pass = 0; pass != 2; ++pass) {
        context.rows.enter(pass == 0 ? "AvoidZoneDepthsSingle" : "AvoidZoneDepthsMulti");
        for (const auto& entry : entries) {
            for (int i = 0; i != entry.count; ++i) {
                const std::int32_t value = context.rows.integer(entry.key, i + 1);
                std::memcpy(context.bytes.data() + pass * kAvoidZoneDepthsRecordSize
                    + entry.offset + static_cast<std::size_t>(i) * 4, &value, 4);
            }
        }
    }
    return 0;
}
} // namespace ship_depth_tables_detail

// Same protected publication policy as the keyed settings projection: keep
// every partial table private on Lua failure, and restore the caller's stack.
inline bool read_ship_depth_tables_lua(lua_State& state, GameplayTuningSettings& settings,
    bool crt_sse2_conversion, std::string& error) {
    gameplay_tuning_lua_detail::Rows rows(crt_sse2_conversion);
    ship_depth_tables_detail::ReadContext context{rows, {}};
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &context);
    lua_pushcclosure(&state, &ship_depth_tables_detail::read_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message ? message : "ShipGlobals depth-table read raised a non-string Lua error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    static_assert(offsetof(GameplayTuningSettings, gap_080h) == kAvoidZoneDepthsSingleOffset);
    static_assert(sizeof(settings.gap_080h) == sizeof(context.bytes));
    std::memcpy(settings.gap_080h, context.bytes.data(), sizeof(context.bytes));
    error.clear();
    return true;
}

struct LoadedShipDepthTable {
    const std::byte* bytes;
    std::uint32_t settings_offset;
    std::uint32_t word(std::uint32_t offset) const noexcept {
        std::uint32_t value;
        std::memcpy(&value, bytes + offset, sizeof(value));
        return value;
    }
};

LoadedShipDepthTable select_loaded_ship_depth_table(GameplayTuningSettings& settings,
    std::int32_t session_mode) noexcept;
} // namespace bsp::game
