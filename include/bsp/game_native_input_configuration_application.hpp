#pragma once
#include "bsp/game_input_actions.hpp"
#include <cstdint>
#include <memory>

namespace bsp {
class NativeStringStorage;
struct NativeGameSettingsStorage;
struct CameraAxesCrtAccess;
}
namespace bsp::game {
class GameHostLog;
class GameNativeLuaServices;
class GameNativeReadOnlyData;
class GameNativeInputSettingsApplication;

// Goal 4 chain 302. The game object's embedded input configuration: the
// constructor 004DDB90 builds it at game+3Ch (004DDBE2 -> 00698680) and the
// next member starts at game+560h, so it is 524h bytes with its Lua owner at
// offset zero (include/bsp/native_input_configuration_owner.hpp). This host
// builds no native 71A0h game, so it owns that one span itself; everything the
// loader reaches is the application's existing input runtime, action owner
// [00F8BBF8], backend [00F8BBF4], input settings [00E198E8], Lua/VFS/string
// services and raw settings block 00F88980. Hypothesis names; see
// docs/NATIVE_INPUT_CONFIGURATION_LOAD.md for the loader contract.
struct GameNativeInputConfigurationBindings {
    GameHostLog& log;
    GameInputRuntime& input;
    GameNativeInputSettingsApplication& settings;
    GameNativeLuaServices& lua;
    NativeStringStorage& strings;
    const GameNativeReadOnlyData& data;
    NativeGameSettingsStorage& game_settings;
    void* volatile& backend_00f8bbf4;
    const bool& crt_sse2_conversion;
    const CameraAxesCrtAccess& crt;
    NativeInputActionBindingConstants binding_constants;
    NativeInputActionTimingGlobals timing;
    const volatile float& zero_00d7a218;
};

class GameNativeInputConfigurationApplication final {
public:
    // 00698680 on the owned span (004DDBE2's call); no Lua state is opened.
    explicit GameNativeInputConfigurationApplication(GameNativeInputConfigurationBindings);
    ~GameNativeInputConfigurationApplication();
    GameNativeInputConfigurationApplication(const GameNativeInputConfigurationApplication&) = delete;
    GameNativeInputConfigurationApplication& operator=(const GameNativeInputConfigurationApplication&) = delete;

    // 004DD739..004DD745: CMP byte [game+55Ch],0 (configuration+520h); a clear
    // byte runs 00698A10 on game+3Ch. Returns whether the load ran.
    bool load_if_unloaded();
    // 00698A10 unconditionally, as the press-start commits 0067CEB9 and
    // 0067D50F call it. Exceptions propagate after marking the owner failed.
    void load();
    // 004E4A63..72: whole native action tick over the configured shared owner.
    // It includes the backend refresh; callers must not also update_backend.
    void update_actions(float seconds);
    // 008D5BB6 -> 004B46E0 (ADD ECX,3Ch; JMP 00699B80): store the four device
    // bytes +4C9h..+4CCh from settings +41h..+44h; when they change and +520h
    // is set, 00698730 then 00698A10 rebuild the actions with the new swaps.
    void set_device_flags_004b46e0(bool swap_sticks_41, bool invert_camera_y_42,
        bool invert_plane_y_43, bool swap_map_sticks_44);
    // 004DCEB0 on the same span: flat buffers, rows, then the Lua owner close.
    // Once only. After a failed or interrupted load it runs nothing and returns
    // false: the span must be retained (not freed) and the process must not go
    // on into the shared drain (no invented rollback).
    [[nodiscard]] bool retire();

    bool loaded_520() const noexcept;
    std::uint32_t loads() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
