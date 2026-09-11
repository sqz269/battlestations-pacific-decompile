#include "bsp/global_subsystems.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <bit>
#include <cstring>
#include <new>

namespace bsp {
namespace {
template<class T> T read(const void* base, std::size_t offset) {
    T result;
    std::memcpy(&result, static_cast<const unsigned char*>(base) + offset, sizeof result);
    return result;
}
std::uint32_t name_count(void* config) {
    const auto begin = read<std::uint32_t>(config, 0x10);
    if (!begin) return 0;
    const auto difference = read<std::uint32_t>(config, 0x14) - begin;
    return static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(difference) >> 3);
}
struct FileBlockCleanup {
    GlobalSubsystemHost& host;
    void* block;
    ~FileBlockCleanup() { host.destroy_file_block_00bdcb30(block); }
};
struct StringCleanup {
    NativeString& value;
    NativeStringStorage& storage;
    ~StringCleanup() { destroy_native_string_header_0041dd20(&value, storage); }
};
struct EffectCleanup {
    void*& value;
    GlobalSubsystemHost& host;
    ~EffectCleanup() {
        void* const captured = value;
        if (!captured) return;
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                static_cast<unsigned char*>(captured) + 4)) == 0)
            host.effect_zero_references(captured);
        value = nullptr; // after the virtual call, even if it changed the local
    }
};
template<class Constructor>
void* construct_opaque(std::size_t bytes, Constructor constructor) {
    void* const storage = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    if (!storage) return nullptr;
    try { return constructor(storage); }
    catch (...) { singleton_lifetime_free(storage); throw; }
}
} // namespace

void construct_global_subsystems_004dc6a0(GlobalSubsystemState game,
    GlobalSubsystemHost& host, GlobalSubsystemContext& context) {
    static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);
    void* file_block;
    {
        NativeString name;
        StringCleanup cleanup{name, context.strings};
        name.resize_0041dd40(context.strings, 11, true);
        if (name.data()) std::memcpy(name.data(), "Game_Global", name.length() + 1u);
        file_block = host.construct_file_block_00be0a30(name, 1);
    } // temporary name is released before the first script call
    FileBlockCleanup block_cleanup{host, file_block};

    host.load_global_scripts_00886900(game.mission_lua_1a08);
    host.load_races_00800160();
    host.load_robots_00901610();
    // 006F7B50 is one RET, verified from the listing.
    host.install_recon_values_00803a40();
    host.load_marker_classes_006dbeb0();
    void* const lua = read<void*>(game.mission_lua_1a08, 4);
    if (lua) host.run_string_006b8ad0(lua, "collectgarbage(\"collect\")", 0, 0, 2);

    for (std::uint32_t index = 0; index < name_count(host.current_config_00432650()); ++index) {
        void* const selected_config = host.current_config_00432650();
        if (index >= name_count(selected_config))
            context.validation.invalid_parameter(context.validation.context);
        // The validation callback may return after repairing this same owner.
        auto* const begin = read<NativeString*>(selected_config, 0x10);
        void* effect;
        host.acquire_effect_00871ba0(effect, begin + index, 1);
        EffectCleanup cleanup{effect, host};
        host.append_effect_004d9c00(game.effects_vector_718c, &effect);
    }
    void* const config = host.current_config_00432650();
    const float zero = 0.0f;
    std::memcpy(static_cast<unsigned char*>(config) + 0x2d8, &zero, sizeof zero);

    void* const traffic = construct_opaque(0x58,
        [&](void* storage) { return host.construct_traffic_004a43c0(storage); });
    game.traffic_21d0 = traffic;
    host.load_traffic_0049d690(traffic);

    void* const panel_storage = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x38, sizeof(VoicePanelState)});
    VoicePanelState* panel = nullptr;
    if (panel_storage) {
        try {
            panel = ::new (panel_storage) VoicePanelState;
            prepare_panel_owner_allocation(*panel, context.panel_allocation);
            construct_panel_owner_00452660(*panel);
        } catch (...) {
            if (panel) panel->~VoicePanelState();
            singleton_lifetime_free(panel_storage);
            throw;
        }
    }
    game.panel_21e4 = panel;
    load_dialog_config_0044fa30(voice_panel_dialog_config(*panel),
        context.lua_environment, context.scripts, context.dialog);

    void* const powerup = construct_opaque(0x1c4,
        [&](void* storage) { return host.construct_powerup_008edc60(storage); });
    game.powerup_00f88c30 = powerup;
    host.initialize_powerup_008ecec0(powerup);

    void* const warnings = construct_opaque(0x1b0,
        [&](void* storage) { return host.construct_warnings_0098a020(storage); });
    game.warnings_21e0 = warnings;
    host.initialize_warnings_009870a0(warnings);

    void* const weather_storage = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x7c, sizeof(WeatherConfig)});
    WeatherConfig* weather = nullptr;
    if (weather_storage) {
        try {
            weather = ::new (weather_storage) WeatherConfig(context.weather_allocation);
            construct_weather_config_00445b10(*weather);
        } catch (...) {
            if (weather) weather->~WeatherConfig();
            singleton_lifetime_free(weather_storage);
            throw;
        }
    }
    game.weather_21c4 = weather;
    load_weather_config_00444d20(*weather, context.lua_environment,
        context.scripts, context.strings);
}
} // namespace bsp
