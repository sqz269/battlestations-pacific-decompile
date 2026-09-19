#pragma once
#include "bsp/native_game_settings_owner.hpp"

namespace bsp::game {
// E198E8 has loader lifetime. The table/Lua services are borrowed only while
// the application owns an input settings instance; no service is fabricated
// for the later null-publication CRT shutdown path.
class GameNativeInputSettingsProcess final {
public:
    GameNativeInputSettingsProcess(const GameNativeInputSettingsProcess&)=delete;
    GameNativeInputSettingsProcess& operator=(const GameNativeInputSettingsProcess&)=delete;
    void* volatile& publication_00e198e8() noexcept {return publication_;}
    NativeGameSettingsInputReference settings_reference() noexcept;
    // Reject a different domain or replacing another live application binding.
    // Retirement requires the native publication to have been cleared first.
    void bind_context(NativeInputSettingsLifetimeContext*);
private:
    friend GameNativeInputSettingsProcess& game_native_input_settings_process();
    GameNativeInputSettingsProcess();
    void* volatile publication_{};
    void* volatile& manager_;
    NativeInputSettingsLifetimeContext* context_{};
};
// Intentionally retained through every CRT callback, like the string process.
GameNativeInputSettingsProcess& game_native_input_settings_process();
} // namespace bsp::game
