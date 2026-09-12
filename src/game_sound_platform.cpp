#include "bsp/game_sound_platform.hpp"

namespace bsp::game {
GameSoundLoadEvents::GameSoundLoadEvents(Win32PlatformState& platform,
    PlatformCursorGlobals globals, PlatformCursorHost& cursor, XLiveLibrary& xlive) noexcept
    : platform_(platform), globals_(globals), cursor_(cursor), xlive_(xlive) {}

bool GameSoundLoadEvents::pretranslate(MSG& message) {
    ++pretranslation_calls_;
    return xlive_.pretranslate(message);
}
void GameSoundLoadEvents::update_cursor_focus_00becb20(bool loading) {
    ++focus_calls_;
    update_platform_cursor_focus_00becb20(platform_, loading, globals_, cursor_);
}
} // namespace bsp::game
