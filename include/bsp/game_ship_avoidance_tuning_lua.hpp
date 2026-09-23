#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
struct lua_State;

namespace bsp::game {
// Bounded projection of0083B5E0 using already compiled native Lua wrappers and
// ship_ai_settings_keys metadata. Output order is settings offsets
// +194,+1D4,+1D8,+214,+218, matching the five native getter/store sites.
// +194 uses bare Number; the others use exact-number-type FloatOrDefault.
// Reads the borrowed state without running scripts. Output is unchanged on a
// Lua error; the original stack top is restored on either result.
//
// Capture once after the represented settings-script load and retain the five
// values in the settings owner. Calling again rereads current Lua tables; that
// is an explicit host reload, not the native singleton's ordinary read path.
// This does not reproduce the native loader's private interpreter/lifetime or
// whole76Ch object. See docs/SHIP_AI_AVOIDANCE_TUNING.md.
bool read_ship_avoidance_tuning_lua(lua_State&,
    std::array<float, 5>& output, std::string& error);
// The same table-driven read for any settings offsets ship_ai_settings_keys
// carries (at most 32), in the order given. Packet cc9_ship_neighbour_list reads
// the ShipAvoidance block +190h..+1D8h through it. Output is unchanged on error.
bool read_ship_ai_settings_offsets_lua(lua_State&, const std::uint32_t* offsets,
    float* output, std::size_t count, std::string& error);
} // namespace bsp::game
