#include "bsp/game_platform_services.hpp"
#include "bsp/game_input_runtime.hpp"
#include <stdexcept>

namespace bsp::game {
GamePlatformServices::GamePlatformServices(void* volatile& input,
    GameInputRuntime* volatile& runtime, XLiveManagerOwner* volatile& online,
    XLiveLibrary& library) noexcept
    : input_(input), runtime_(runtime), online_(online), load_events_(*this, library) {}

void GamePlatformServices::update_cursor_focus_00becb20(bool loading) {
    if (auto* runtime = runtime_) {
        runtime->update_cursor(loading);
        return;
    }
    // The source service exists before native online/input initialization.
    // These are BECB20's real initial guards, not fabricated manager objects.
    if (!online_ || !input_) return;
    throw std::logic_error("published raw input requires its application cursor runtime");
}
} // namespace bsp::game
