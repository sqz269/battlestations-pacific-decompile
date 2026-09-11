#include "bsp/weather_config.hpp"

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/native_pooled_string_substring.hpp"

#include <cstring>
#include <new>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Weather configuration reconstruction requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
void seed(float& field, std::uint32_t bits) noexcept {
    std::memcpy(&field, &bits, sizeof bits);
}
void copy_float(float& destination, const float& source) noexcept {
    auto* target = &destination;
    auto* input = &source;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov eax, target
        fstp dword ptr [eax]
    }
}
//004456A7..004456F9 keeps24.0 double on x87 throughout both loops.
// Preserve ordered COMISS branches, per-iteration float spills, inclusive24,
// signed zero and native nontermination when +/-24 cannot change the float.
void normalize_day_time(float& field) noexcept {
    const double period = 24.0;
    const float maximum = 24.0f;
    auto* destination = &field;
    __asm {
        mov eax, destination
        fld period
        xorps xmm0, xmm0
        comiss xmm0, dword ptr [eax]
        jbe above_negative
    negative_day:
        fld dword ptr [eax]
        fadd st(0), st(1)
        fstp dword ptr [eax]
        comiss xmm0, dword ptr [eax]
        ja negative_day
    above_negative:
        movss xmm1, dword ptr [eax]
        movss xmm0, maximum
        comiss xmm1, xmm0
        jbe normalized
    high_day:
        fld dword ptr [eax]
        fsub st(0), st(1)
        fstp dword ptr [eax]
        movss xmm1, dword ptr [eax]
        comiss xmm1, xmm0
        ja high_day
    normalized:
        fstp st(0)
    }
}
struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef ref;
    ~OwnedRef() { host.release(ref); }
    void assign(const GuiLuaRef& source) {
        host.release(ref);
        ref = {};
        ref = host.copy_ref_00b66fa0(source);
    }
};
struct Iteration {
    GuiLua51Host& host;
    GuiLuaRef table;
    GuiLuaRef key{};
    GuiLuaRef value{};
    ~Iteration() { host.release(value); host.release(key); }
    bool next(bool restart) {
        host.release(value); value = {};
        host.release(key); key = {};
        return host.next(table, key, value, restart);
    }
};
// Each loop saves its string data pointer BEFORE map insertion/number access.
// Recon also saves length there; the two amplitude loops reload length on free.
void read_map(WeatherFloatMap& map, Iteration& iterator,
    NativeStringStorage& strings, bool capture_length)
{
    for (bool more = iterator.next(true); more; more = iterator.next(false)) {
        NativeString key;
        key.assign_0041e870(strings, iterator.host.to_string(iterator.key));
        auto* captured_data = key.data();
        const auto captured_size = key.length() + 1u;
        try {
            auto& destination = lookup_weather_float_00444be0(map, key, strings);
            destination = lua_object_number_00b66270(iterator.host, iterator.value);
        } catch (...) {
            if (captured_data)
                strings.release(captured_data, capture_length ? captured_size : key.length() + 1u);
            throw;
        }
        if (captured_data)
            strings.release(captured_data, capture_length ? captured_size : key.length() + 1u);
    }
}
void read_vector(std::array<float, 3>& destination, GuiLua51Host& host,
    GuiLuaRef table)
{
    // Lookups3,2,1; conversions1,2,3; publish all three; releases1,2,3.
    OwnedRef third{host, host.get_by_index(table, 3)};
    OwnedRef second{host, host.get_by_index(table, 2)};
    OwnedRef first{host, host.get_by_index(table, 1)};
    const float x = lua_object_number_00b66270(host, first.ref);
    const float y = lua_object_number_00b66270(host, second.ref);
    const float z = lua_object_number_00b66270(host, third.ref);
    copy_float(destination[0], x);
    copy_float(destination[1], y);
    copy_float(destination[2], z);
}
void read_configuration(WeatherConfig& config, GuiLua51Host& host,
    NativeStringStorage& strings)
{
    GuiLuaRef globals_ref;
    {
        OwnedRef root{host, host.globals()};
        globals_ref = host.get_by_name(root.ref, "Globals");
    }
    OwnedRef globals{host, globals_ref};
    OwnedRef day_time{host, host.get_by_name(globals.ref, "DayTime")};
    const char* names[] = {"StartDayTimeDefault", "MorningStart", "MorningEnd",
        "EveningStart", "EveningEnd", "NightVisibility", "FireMaxModifier",
        "SmokeMaxModifier", "FireSmokeModifierDuration", "HighNoonMaxVisionRange"};
    float* destinations[] = {&config.start_day_time_04, &config.morning_start_08,
        &config.morning_end_0c, &config.evening_start_10, &config.evening_end_14,
        &config.night_visibility_18, &config.fire_max_modifier_1c,
        &config.smoke_max_modifier_20, &config.fire_smoke_modifier_duration_24,
        &config.high_noon_max_vision_range_28};
    for (std::size_t i = 0; i != 10; ++i) {
        OwnedRef value{host, host.get_by_name(day_time.ref, names[i])};
        *destinations[i] = lua_object_number_00b66270(host, value.ref);
    }
    OwnedRef current_table{host, host.get_by_name(globals.ref, "WeatherReconModifiers")};
    Iteration iterator{host, current_table.ref};
    read_map(config.weather_recon_modifiers_38.value(), iterator, strings, true);

    // Inlined assignment at44517E: no old-byte preserve copy. Equal length
    // does not allocate or write a terminator, even if its pointer is null.
    config.current_weather_2c.resize_0041dd40(strings, 7, false);
    if (config.current_weather_2c.data())
        std::memcpy(config.current_weather_2c.data(), "default", config.current_weather_2c.length());
    config.current_weather_modifier_34 = 1.0f; //00D7A24C

    GuiLuaRef camera_ref;
    {
        OwnedRef root{host, host.globals()};
        OwnedRef table{host, host.get_by_name(root.ref, "Globals")};
        camera_ref = host.get_by_name(table.ref, "CameraHimbilimbi");
    } // second Globals then globals pseudo-object, before Frequencies lookup
    OwnedRef camera{host, camera_ref};
    {
        OwnedRef frequencies{host, host.get_by_name(camera.ref, "Frequencies")};
        current_table.assign(frequencies.ref);
    }
    read_vector(config.frequencies_50, host, current_table.ref);
    {
        OwnedRef amplitudes{host, host.get_by_name(camera.ref, "BaseAmplitudes")};
        current_table.assign(amplitudes.ref);
    }
    read_vector(config.base_amplitudes_44, host, current_table.ref);
    {
        OwnedRef view_modes{host, host.get_by_name(camera.ref, "ViewModeAmpMultipliers")};
        current_table.assign(view_modes.ref);
        iterator.table = current_table.ref;
    }
    read_map(config.view_mode_amp_multipliers_5c.value(), iterator, strings, false);
    {
        OwnedRef weather{host, host.get_by_name(camera.ref, "WeatherAmpMultipliers")};
        current_table.assign(weather.ref);
        iterator.table = current_table.ref;
    }
    read_map(config.weather_amp_multipliers_68.value(), iterator, strings, false);
    config.current_view_mode_amp_multiplier_74 = 1.0f;
    config.current_weather_amp_multiplier_78 = 1.0f;
    normalize_day_time(config.start_day_time_04);
    // camera, iteration value/key, current table, DayTime, Globals, then owner.
}
void clear_map(std::optional<WeatherFloatMap>& optional, NativeStringStorage& strings) {
    if (!optional) return;
    auto& map = *optional;
    while (!map.empty()) {
        auto node = map.extract(map.begin());
        destroy_native_string_header_0041dd20(&node.key(), strings);
    }
    optional.reset();
}
} // namespace

WeatherConfig::WeatherConfig(const WeatherConfigAllocationWords& words) noexcept {
    float* fields[] = {&start_day_time_04, &morning_start_08, &morning_end_0c,
        &evening_start_10, &evening_end_14, &night_visibility_18,
        &fire_max_modifier_1c, &smoke_max_modifier_20, &fire_smoke_modifier_duration_24,
        &high_noon_max_vision_range_28, &current_weather_modifier_34,
        &base_amplitudes_44[0], &base_amplitudes_44[1], &base_amplitudes_44[2],
        &frequencies_50[0], &frequencies_50[1], &frequencies_50[2],
        &current_view_mode_amp_multiplier_74, &current_weather_amp_multiplier_78};
    for (std::size_t i = 0; i != 19; ++i) seed(*fields[i], words.scalar_words[i]);
}
WeatherConfig& construct_weather_config_00445b10(WeatherConfig& config) {
    config.native_vtable_00 = 0x00ce4934;
    new (&config.current_weather_2c) NativeString;
    config.weather_recon_modifiers_38.emplace();
    config.view_mode_amp_multipliers_5c.emplace();
    config.weather_amp_multipliers_68.emplace();
    return config;
}
WeatherFloatMap::iterator lower_bound_weather_float_00443d60(
    WeatherFloatMap& map, const NativeString& name)
{
    return map.lower_bound(name);
}
float& lookup_weather_float_00444be0(WeatherFloatMap& map,
    const NativeString& name, NativeStringStorage& strings)
{
    const auto lower = lower_bound_weather_float_00443d60(map, name);
    if (lower != map.end()
        && !native_string_less_case_insensitive_00443d00(name, lower->first))
        return lower->second;
    // Pair+8 is explicitly XORPS/MOVSS +0, unlike the palette's scratch words.
    NativeString pair_key;
    copy_construct_native_string_header_00426060(&pair_key, &name, strings);
    auto* captured_pair_data = pair_key.data();
    try {
        OwnedString node_key{strings, {}};
        copy_construct_native_string_header_00426060(&node_key.value, &pair_key, strings);
        auto where = map.emplace_hint(lower, std::move(node_key.value), 0.0f);
        if (captured_pair_data) strings.release(captured_pair_data, pair_key.length() + 1u);
        return where->second;
    } catch (...) {
        if (captured_pair_data) strings.release(captured_pair_data, pair_key.length() + 1u);
        throw;
    }
}
void load_weather_config_00444d20(WeatherConfig& config, LuaStateOwnerEnvironment environment,
    LuaScriptRuntime& scripts, NativeStringStorage& strings)
{
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(1);
    {
        OwnedString path{strings, {}};
        path.value.assign_0041e870(strings, "Scripts\\datatables\\Globals.lua");
        (void)scripts.run_file(lua.storage_lua_38(), path.value.data(), false);
    }
    {
        GuiLua51Host host(*lua.storage_lua_38());
        read_configuration(config, host, strings);
    }
    lua.close_storage_archive_00b65e80();
}
void release_weather_config_storage(WeatherConfig& config, NativeStringStorage& strings) {
    clear_map(config.weather_amp_multipliers_68, strings);
    clear_map(config.view_mode_amp_multipliers_5c, strings);
    clear_map(config.weather_recon_modifiers_38, strings);
    destroy_native_string_header_0041dd20(&config.current_weather_2c, strings);
    new (&config.current_weather_2c) NativeString;
}
} // namespace bsp
