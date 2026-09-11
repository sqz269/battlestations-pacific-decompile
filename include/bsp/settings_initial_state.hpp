#pragma once

#include "bsp/game_settings.hpp"

namespace bsp {

// 008d7710..008d78cd: ECX settings, EAX settings, RET. Applies the constructor's
// stores to every represented native field. The non-native options_file.language
// and unknown_tokens metadata are left alone. Native unprojected storage is
// documented in SETTINGS_INITIAL_STATE.md; this is not an ABI replacement.
//
// A true return means 008d45d0 must import selected-user profile preferences;
// this function does not claim that platform operation has occurred. Both input
// flags are explicit so callers constructing later settings copies cannot
// silently omit that reachable native branch.
bool initialize_game_settings_008d7710(
    GameSettingsBlock& settings, bool xenon_state_is_two,
    bool xenon_user_selected) noexcept;

// Static initializer 00cd2d80..00cd2d95 calls the constructor on 00f88980.
// 00f8abe8 is initially null; its first construction is later, at 0073dc7c,
// after settings loading at 0073daa5. Consequently the selected-user tail is
// inactive here. C++ members retain ordinary lifetime management; this wrapper
// does not register the native atexit destructor 00cdeec0.
void initialize_static_game_settings_00cd2d80(GameSettingsBlock& settings) noexcept;

} // namespace bsp
