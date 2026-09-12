#include "bsp/game_sound_platform.hpp"

namespace bsp::game {
GameSoundLoadEvents::GameSoundLoadEvents(GameSoundCursorCalls& cursor, XLiveLibrary& xlive) noexcept
    : cursor_(cursor), xlive_(xlive) {}

bool GameSoundLoadEvents::pretranslate(MSG& message) {
    ++pretranslation_calls_;
    return xlive_.pretranslate(message);
}
void GameSoundLoadEvents::update_cursor_focus_00becb20(bool loading) {
    ++focus_calls_;
    cursor_.update_cursor_focus_00becb20(loading);
}
} // namespace bsp::game
