// cc15-ob1: four of 004DC6A0's owners. See include/bsp/game_hosts_global_subsystems.hpp
// and docs/CC15_OBJECT_GLOBALSUBSYSTEMS.md.

#include "bsp/game_hosts_global_subsystems.hpp"

#include "bsp/dialog_config.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/native_string.hpp"
#include "bsp/panel_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/traffic_config.hpp"
#include "bsp/vfs_lua_scripts.hpp"
#include "bsp/warning_owner.hpp"
#include "bsp/weather_config.hpp"

#include <windows.h>

#include <cstdio>
#include <exception>
#include <new>
#include <stdexcept>
#include <vector>

namespace bsp::game {

namespace {

// 00BF6713 can return; no validation site in these four owners is expected to
// reach it with the owners this host builds, so a call is an error.
void invalid_parameter_unreached(void*) {
    throw std::logic_error("global subsystems: native invalid-parameter path reached");
}
void destroy_registered_unreached(void*, void*, std::uint32_t) noexcept { std::terminate(); }

// The warning owner's services. 009870A0's roster (00973F20), the two effects
// (00871BA0) and the hooks (00985C50 / 00985250) are not run (see the doc), so
// none of these is reached; release_warning_owner_host_storage only releases
// effects that were acquired.
class WarningOwnerServicesSeam final : public WarningOwnerServices {
public:
    void increment_outer_00581290(WarningRosterIterator&) override { unreached(); }
    void increment_inner_0057f510(WarningRosterIterator&) override { unreached(); }
    void acquire_effect_00871ba0(void*&, const NativeString&, int) override { unreached(); }
    void effect_zero_references(void*) override { unreached(); }
    void call_00985c50(WarningOwner&, std::uint32_t) override { unreached(); }
    void call_00985250(WarningOwner&, std::uint32_t, std::uint32_t) override { unreached(); }

private:
    [[noreturn]] static void unreached() {
        throw std::logic_error("global subsystems: warning owner service not hosted");
    }
};

// operator new's words that each constructor leaves alone. The image's heap
// leaves them unspecified; the host zeroes them, so a null traffic group and
// zero scalars until the loaders write them.
template <class T, class Words>
T* allocate_owner(std::size_t native_bytes, const Words& words) {
    void* const storage = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, native_bytes, sizeof(T)});
    if (storage == nullptr) throw std::bad_alloc();
    try {
        return ::new (storage) T(words);
    } catch (...) {
        singleton_lifetime_free(storage);
        throw;
    }
}
template <class T> void free_owner(T*& owner) noexcept {
    if (owner == nullptr) return;
    owner->~T();
    singleton_lifetime_free(owner);
    owner = nullptr;
}

}  // namespace

struct GameGlobalSubsystemsHost::Impl {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in)
        : log(log_in), vfs(vfs_in), files(vfs_in.context(), suffixes), runtime(files) {
        sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0;
    }

    GameHostLog& log;
    GameVfsHost& vfs;
    // manager+48h/+4Ch: the host registers no content suffixes (as GameMissionLuaHost).
    const std::vector<std::string> suffixes{};
    VfsLuaScriptFiles files;
    LuaScriptRuntime runtime;
    // 0108FF20 / 0108FF24: the host keeps x360comp 0 and an empty region.
    LuaRuntimeGlobals globals{};
    bool sse2{false};

    SingletonLifetimeCallbacks callbacks{nullptr, &destroy_registered_unreached,
        &invalid_parameter_unreached};
    WarningOwnerServicesSeam warning_services;
    WarningOwner* warning_singleton_f8a0c4{nullptr};
    void* world_e188a8{nullptr};
    void* hook_owner_f8bbcc{nullptr};
    std::uint32_t hook_f8bf4c{0};
    WarningOwnerContext warning_context{crt_string_storage(), warning_services,
        warning_singleton_f8a0c4, world_e188a8, hook_owner_f8bbcc, hook_f8bf4c, callbacks};

    TrafficConfig* traffic_21d0{nullptr};
    VoicePanelState* panel_21e4{nullptr};
    WarningOwner* warnings_21e0{nullptr};
    WeatherConfig* weather_21c4{nullptr};
    GlobalSubsystemsSummary summary;

    LuaStateOwnerEnvironment environment() { return files.owner_environment(runtime, globals); }

    // Each loader runs in a Lua state of its own; a failure leaves the owner as
    // the constructor made it and is logged, as a record.
    template <class Load> bool load(const char* method, const char* address, Load&& body) {
        try {
            body();
            log.implemented(method, address);
            return true;
        } catch (const std::exception& failure) {
            ++summary.load_failures;
            summary.last_error = failure.what();
            log.unimplemented(method, address);
            log.notef("global subsystems: %s (%s) failed: %s", method, address, failure.what());
            return false;
        }
    }
};

GameGlobalSubsystemsHost::GameGlobalSubsystemsHost(GameHostLog& log, GameVfsHost& vfs)
    : impl_(std::make_unique<Impl>(log, vfs)) {}

GameGlobalSubsystemsHost::~GameGlobalSubsystemsHost() {
    try {
        teardown_004d2bb0();
    } catch (...) {
    }
}

void GameGlobalSubsystemsHost::build_traffic_and_panel() {
    Impl& m = *impl_;
    if (m.traffic_21d0 || m.panel_21e4 || m.warnings_21e0 || m.weather_21c4) teardown_004d2bb0();
    ++m.summary.builds;

    // 004DC7xx: operator new(58h), 004A43C0, the store to game+21D0, 0049D690.
    static const TrafficConfigAllocationWords traffic_words{};
    static const TrafficGroupAllocationWords group_words{};
    m.traffic_21d0 = allocate_owner<TrafficConfig>(0x58, traffic_words);
    construct_traffic_config_004a43c0(*m.traffic_21d0);
    m.log.implemented("GlobalSubsystems::construct_traffic", "004a43c0");
    TrafficConfigContext traffic_context{crt_string_storage(), group_words};
    if (m.load("GlobalSubsystems::load_traffic", "0049d690", [&] {
            load_traffic_config_0049d690(*m.traffic_21d0, m.environment(), m.runtime,
                traffic_context);
        })) {
        ++m.summary.traffic_loaded;
    }

    // operator new(38h), 00452660, the store to game+21E4, 0044FA30.
    static const PanelOwnerAllocationWords panel_words{};
    {
        void* const storage = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x38, sizeof(VoicePanelState)});
        if (storage == nullptr) throw std::bad_alloc();
        VoicePanelState* panel = nullptr;
        try {
            panel = ::new (storage) VoicePanelState;
            prepare_panel_owner_allocation(*panel, panel_words);
            construct_panel_owner_00452660(*panel);
        } catch (...) {
            if (panel) panel->~VoicePanelState();
            singleton_lifetime_free(storage);
            throw;
        }
        m.panel_21e4 = panel;
    }
    m.log.implemented("GlobalSubsystems::construct_panel", "00452660");
    static const PanelPaletteValueWords missing_palette_words{};
    DialogConfigContext dialog_context{crt_string_storage(), missing_palette_words, m.callbacks,
        m.sse2};
    if (m.load("GlobalSubsystems::load_dialog_config", "0044fa30", [&] {
            load_dialog_config_0044fa30(voice_panel_dialog_config(*m.panel_21e4),
                m.environment(), m.runtime, dialog_context);
        })) {
        ++m.summary.panel_loaded;
    }
}

void GameGlobalSubsystemsHost::build_warnings() {
    Impl& m = *impl_;
    // operator new(1B0h), 0098A020, the store to game+21E0. 009870A0 follows in
    // the image; its table-load fragment is the caller's, and its scalar resets
    // (00987xxx: +174h..+180h, +D0h, +198h, the clocks +4h..+20h) run here. The
    // roster 00973F20, the effects and the hooks are not run.
    static const WarningOwnerAllocationWords warning_words{};
    m.warnings_21e0 = allocate_owner<WarningOwner>(0x1b0, warning_words);
    construct_warning_owner_0098a020(*m.warnings_21e0);
    m.log.implemented("GlobalSubsystems::construct_warnings", "0098a020");
    WarningOwner& owner = *m.warnings_21e0;
    owner.state.air_raid_deadline = 0.0f;
    owner.state.air_raid_active = false;
    owner.state.collision_deadline = 0.0f;
    owner.state.collision_active = false;
    owner.state.suppressed = false;
    owner.state.periodic_accumulator = 0.0f;
    for (float& clock : owner.clocks_04_20) clock = 0.0f;
}

void GameGlobalSubsystemsHost::build_weather() {
    Impl& m = *impl_;
    // operator new(7Ch), 00445B10, the store to game+21C4, 00444D20.
    static const WeatherConfigAllocationWords weather_words{};
    m.weather_21c4 = allocate_owner<WeatherConfig>(0x7c, weather_words);
    construct_weather_config_00445b10(*m.weather_21c4);
    m.log.implemented("GlobalSubsystems::construct_weather", "00445b10");
    if (m.load("GlobalSubsystems::load_weather", "00444d20", [&] {
            load_weather_config_00444d20(*m.weather_21c4, m.environment(), m.runtime,
                crt_string_storage());
        })) {
        ++m.summary.weather_loaded;
    }
}

void GameGlobalSubsystemsHost::teardown_004d2bb0() {
    Impl& m = *impl_;
    if (!(m.traffic_21d0 || m.panel_21e4 || m.warnings_21e0 || m.weather_21c4)) return;
    ++m.summary.teardowns;
    // The image calls each owner's vtable slot 0 with 1. The host releases the
    // storage the typed bodies made, which is all these owners hold here.
    if (m.traffic_21d0) {  // 004D2C65
        release_traffic_config_startup_storage(*m.traffic_21d0, crt_string_storage());
        free_owner(m.traffic_21d0);
        m.log.note("global subsystems teardown: +21D0 traffic (004D2C65)");
    }
    if (m.weather_21c4) {  // 004D2CBC
        release_weather_config_storage(*m.weather_21c4, crt_string_storage());
        free_owner(m.weather_21c4);
        m.log.note("global subsystems teardown: +21C4 weather (004D2CBC)");
    }
    if (m.warnings_21e0) {  // 004D2D1C
        release_warning_owner_host_storage(*m.warnings_21e0, m.warning_context);
        free_owner(m.warnings_21e0);
        m.log.note("global subsystems teardown: +21E0 warnings (004D2D1C)");
    }
    if (m.panel_21e4) {  // 004D2D34, 00452720 with 1
        scalar_delete_panel_owner_00452720(m.panel_21e4, 1, crt_string_storage());
        m.panel_21e4 = nullptr;
        m.log.note("global subsystems teardown: +21E4 panel (004D2D34)");
    }
}

WarningOwner* GameGlobalSubsystemsHost::warnings() noexcept { return impl_->warnings_21e0; }
TrafficConfig* GameGlobalSubsystemsHost::traffic() noexcept { return impl_->traffic_21d0; }
VoicePanelState* GameGlobalSubsystemsHost::panel() noexcept { return impl_->panel_21e4; }
WeatherConfig* GameGlobalSubsystemsHost::weather() noexcept { return impl_->weather_21c4; }
const GlobalSubsystemsSummary& GameGlobalSubsystemsHost::summary() const noexcept {
    return impl_->summary;
}

std::string GameGlobalSubsystemsHost::describe_contents() const {
    const Impl& m = *impl_;
    char text[512];
    const auto size_of = [](const auto& map) { return map ? map->size() : std::size_t{0}; };
    std::snprintf(text, sizeof text,
        "traffic soldiers=%zu vehicles=%zu; panel characters=%zu default_pause=%.3f; "
        "warnings tables=%d; weather start_day_time=%.3f morning=%.3f..%.3f "
        "evening=%.3f..%.3f recon_modifiers=%zu view_mode_amps=%zu weather_amps=%zu",
        m.traffic_21d0 ? size_of(m.traffic_21d0->soldiers_40) : 0u,
        m.traffic_21d0 ? size_of(m.traffic_21d0->vehicles_4c) : 0u,
        m.panel_21e4 ? m.panel_21e4->characters_04.size() : 0u,
        m.panel_21e4 ? static_cast<double>(m.panel_21e4->default_pause_30) : 0.0,
        m.warnings_21e0 && m.warnings_21e0->tables_108 ? 1 : 0,
        m.weather_21c4 ? static_cast<double>(m.weather_21c4->start_day_time_04) : 0.0,
        m.weather_21c4 ? static_cast<double>(m.weather_21c4->morning_start_08) : 0.0,
        m.weather_21c4 ? static_cast<double>(m.weather_21c4->morning_end_0c) : 0.0,
        m.weather_21c4 ? static_cast<double>(m.weather_21c4->evening_start_10) : 0.0,
        m.weather_21c4 ? static_cast<double>(m.weather_21c4->evening_end_14) : 0.0,
        m.weather_21c4 ? size_of(m.weather_21c4->weather_recon_modifiers_38) : 0u,
        m.weather_21c4 ? size_of(m.weather_21c4->view_mode_amp_multipliers_5c) : 0u,
        m.weather_21c4 ? size_of(m.weather_21c4->weather_amp_multipliers_68) : 0u);
    return text;
}

}  // namespace bsp::game
