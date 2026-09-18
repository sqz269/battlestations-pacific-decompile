#pragma once
#include "bsp/game_input_settings_runtime.hpp"
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;
class GameNativeLuaServices;
class GameNativeReadOnlyData;
struct GameNativeVfsRawServices;
struct GameNativeInputSettingsSummary {
    std::uint32_t devices{},input_names{},controller_names{};
    bool tables_started{},runtime_settings_loaded{};
    void* presets_lua{};
};
// Application binding of the complete raw540h settings singleton. Borrows the
// canonical manager/input publication and existing VFS/Lua/pool services; no
// projected settings containers or second interpreter are created.
// Retain this object BEFORE load_tables and through the shared manager drain.
class GameNativeInputSettingsApplication final {
public:
    GameNativeInputSettingsApplication(GameHostLog&,GameSingletonHost&,
        GameNativeLuaServices&,GameNativeReadOnlyData&,
        const GameNativeVfsRawServices&,GameInputSettingsStackPolicy);
    ~GameNativeInputSettingsApplication();
    GameNativeInputSettingsApplication(const GameNativeInputSettingsApplication&)=delete;
    GameNativeInputSettingsApplication& operator=(const GameNativeInputSettingsApplication&)=delete;
    // 73DA94 getter, then73DA9B load on its captured result. The latter sees
    // the constructor's byte4 guard and preserves the persistent interpreter.
    void load_tables();
    GameNativeInputSettingsSummary summary() const;
    NativeInputSettingsLifetimeContext& context() noexcept;
    void* actual_settings() const;
    bool requires_process_retention() const noexcept;
    // Call while GameSingletonHost still exists, after successful raw drain.
    void after_singleton_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
