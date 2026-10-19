#pragma once

#include <memory>
#include <string>

namespace bsp {
struct WarningOwner;
struct TrafficConfig;
struct VoicePanelState;
struct WeatherConfig;
}

namespace bsp::game {
class GameHostLog;
class GameVfsHost;

// cc15-ob1 (docs/CC15_OBJECT_GLOBALSUBSYSTEMS.md): four of the five owners
// 004DC6A0 builds, each through its typed constructor and loader. The power-up
// manager 008EDC60 [00F88C30] is cc13-powerup's and is not built here, and nor
// are 004DC6A0's script folders, races, robots, recon values, marker classes or
// global effects. The Lua loaders run in their own states through the
// repository's Lua 5.1.1 (PcStorageLuaOwner), as the image's 00B6A020 owners do,
// with the scripts read out of the VFS (VfsLuaScriptFiles).
struct GlobalSubsystemsSummary {
    unsigned long long builds{0}, teardowns{0};
    unsigned long long traffic_loaded{0}, panel_loaded{0}, weather_loaded{0};
    unsigned long long load_failures{0};
    std::string last_error;
};

class GameGlobalSubsystemsHost {
public:
    GameGlobalSubsystemsHost(GameHostLog&, GameVfsHost&);
    ~GameGlobalSubsystemsHost();  // tears the owners down in 004D2BB0's order
    GameGlobalSubsystemsHost(const GameGlobalSubsystemsHost&) = delete;
    GameGlobalSubsystemsHost& operator=(const GameGlobalSubsystemsHost&) = delete;

    // 004DC6A0's owners in the image's order, called one by one so the caller
    // can run the power-up manager between panel and warnings. Traffic 004A43C0 +
    // 0049D690 (+21D0) and panel 00452660 + 0044FA30 (+21E4); a previous set is
    // torn down first.
    void build_traffic_and_panel();
    // The warning owner 0098A020 (+21E0) and 009870A0's scalar resets. The caller
    // runs 009870A0's table-load fragment into warnings()->tables_108.
    void build_warnings();
    // 00445B10 + 00444D20 (+21C4), the last owner 004DC6A0 builds.
    void build_weather();
    // 004D2BB0's deletes: +21D0 (004D2C65), +21C4 (004D2CBC), +21E0 (004D2D1C),
    // +21E4 (004D2D34). Each is logged. +21DC, the MessageManager, falls between
    // +21C4 and +21E0 in the image and is torn down by its own host.
    void teardown_004d2bb0();

    WarningOwner* warnings() noexcept;
    TrafficConfig* traffic() noexcept;
    VoicePanelState* panel() noexcept;
    WeatherConfig* weather() noexcept;
    const GlobalSubsystemsSummary& summary() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
