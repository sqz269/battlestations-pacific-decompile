#pragma once

#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/panel_sequence_types.hpp"

#include <array>
#include <map>
#include <optional>

namespace bsp {
// All nineteen numeric words are untouched by the native constructor.
// Order:04..28 (ten),34,44..4C (three),50..58 (three),74,78.
struct WeatherConfigAllocationWords {
    std::array<std::uint32_t, 19> scalar_words;
};
using WeatherFloatMap = std::map<NativeString, float, PanelSequenceNameLess>;

// Canonical semantic owner for native7Ch allocation published at game+21C4.
// Optional maps defer library initialization until the recovered constructor
// reaches each map. They are engaged after successful construction. Native
// map nodes, allocator words and this C++ object do not have matching ABIs.
struct WeatherConfig {
    explicit WeatherConfig(const WeatherConfigAllocationWords&) noexcept;
    std::uint32_t native_vtable_00{};
    float start_day_time_04;
    float morning_start_08;
    float morning_end_0c;
    float evening_start_10;
    float evening_end_14;
    float night_visibility_18;
    float fire_max_modifier_1c;
    float smoke_max_modifier_20;
    float fire_smoke_modifier_duration_24;
    float high_noon_max_vision_range_28;
    NativeString current_weather_2c;
    float current_weather_modifier_34;
    std::optional<WeatherFloatMap> weather_recon_modifiers_38;
    std::array<float, 3> base_amplitudes_44;
    std::array<float, 3> frequencies_50;
    std::optional<WeatherFloatMap> view_mode_amp_multipliers_5c;
    std::optional<WeatherFloatMap> weather_amp_multipliers_68;
    float current_view_mode_amp_multiplier_74;
    float current_weather_amp_multiplier_78;
};

// Native ECX=fresh7Ch owner; EAX=this; RET. Does not initialize scalar words.
WeatherConfig& construct_weather_config_00445b10(WeatherConfig&);
// Native ECX=map; name* stack; EAX=node; RET4. Semantic library lower-bound.
WeatherFloatMap::iterator lower_bound_weather_float_00443d60(
    WeatherFloatMap&, const NativeString&);
// Native ECX=map; name* stack; EAX=borrowed node+14 float; RET4.
// Missing values explicitly start at +0.0f. Existing values keep identity.
float& lookup_weather_float_00444be0(WeatherFloatMap&, const NativeString&,
    NativeStringStorage&);
// Native ECX=owner; RET. Creates an actual Lua5.1 state, opens mask1, runs
// Scripts\datatables\Globals.lua and its real VFS overrides. Updates the
// existing three maps without clearing them, preserving omitted entries.
void load_weather_config_00444d20(WeatherConfig&, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, NativeStringStorage&);
// Explicit host lifetime convenience, not a recovered native destructor.
// Required before the shell is discarded: NativeString has no implicit free.
void release_weather_config_storage(WeatherConfig&, NativeStringStorage&);
} // namespace bsp
