#pragma once
#include "bsp/game_sound_platform.hpp"

namespace bsp { struct NativeOnlineManagerStorage; }
namespace bsp::game {
class GameInputRuntime;

// Shared load-time cursor service. Borrows the application's actual raw input
// publication and its source runtime; no typed backend/device vectors exist.
// Before native online/input construction, the original null guards apply.
class GamePlatformServices final : public GameSoundCursorCalls {
public:
    GamePlatformServices(void* volatile& input_00f8bbf4,
        GameInputRuntime* volatile& input_runtime,
        NativeOnlineManagerStorage* volatile& online_00f8abe8, XLiveLibrary&) noexcept;
    GamePlatformServices(const GamePlatformServices&) = delete;
    GamePlatformServices& operator=(const GamePlatformServices&) = delete;
    GameSoundLoadEvents& load_events() noexcept { return load_events_; }
    void update_cursor_focus_00becb20(bool loading) override;
private:
    void* volatile& input_;
    GameInputRuntime* volatile& runtime_;
    NativeOnlineManagerStorage* volatile& online_;
    GameSoundLoadEvents load_events_;
};
} // namespace bsp::game
