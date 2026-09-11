#include "bsp/settings_initial_state.hpp"

namespace bsp {

bool initialize_game_settings_008d7710(
    GameSettingsBlock& settings, bool xenon_state_is_two,
    bool xenon_user_selected) noexcept
{
    settings.gameplay.language_index_04 = 0;       // 008d7733
    settings.audio.enabled_24 = true;             // 008d7736
    settings.gameplay.target_indicator_4a = true; // 008d7747
    settings.options_file.firewall_94 = false;    // 008d7759
    settings.presentation.hardware_reported_95 = false; // 008d775f
    settings.downloaded_content.clear();          // +98/+9c/+a0 zeroed
    settings.control.xbox_compatibility_b0 = false; // 008d77a1
    settings.gameplay.show_safe_area_b1 = false;   // 008d77a7
    settings.gameplay.cockpit_mode_b2 = true;      // 008d77ad
    settings.gameplay.clan_text_b4.clear();        // 008d77b4, empty 00ce3a0c

    // The constructor inlines these same stores. No whole-object assignment:
    // fields outside the native store set must retain their incoming values.
    reset_audio_defaults_008d41f0(settings);        // 008d77b9..008d77d3
    reset_game_defaults_008d41c0(settings);         // 008d77e0..008d77f9 (+7c below)
    reset_video_defaults_008d4520(settings, true, false); // 008d7801..008d7871
    return reset_control_defaults_008d4820(
        settings, xenon_state_is_two, xenon_user_selected); // 008d787b..008d78b5
}

void initialize_static_game_settings_00cd2d80(GameSettingsBlock& settings) noexcept
{
    (void)initialize_game_settings_008d7710(settings, false, false);
}

} // namespace bsp
