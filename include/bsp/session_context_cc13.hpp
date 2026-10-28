#pragma once

// Packet cc13-mp1 (MP2a): the game object's session and game-mode words in one
// place, so the host's single-player constants can read them. Header-only.

#include <cstdint>

namespace bsp {

// kSkirmishSessionCc13Bound (OFF). It gates the non-campaign values only. ON: a
// --skirmish run (packet cc13-mp1) publishes the skirmish context below at the point
// the image's Session_SetMode(1) runs. OFF: every run keeps the campaign context, so a
// --skirmish run loads its map as a single-player scene, as MP1 alone does. A campaign
// run never publishes anything and reads the campaign context either way.
inline constexpr bool kSkirmishSessionCc13Bound = false;

// The five game fields. Descriptive names are hypotheses.
struct SessionContext {
    // game+1FE4h, session+F4h (session at game+1EF0h): 0 local, 1 host, 2 client.
    // Its only writer is 0076FAD0 Session_SetMode (0076FE85).
    std::int32_t session_word_1fe4{0};
    // game+218Ch, session+29Ch: the skirmish byte. 005E6F70 (the main menu's skirmish
    // item) sets it to 1 at 005E6F87; 004D8041, 0059882D and 005ECB33 clear it.
    bool skirmish_218c{false};
    // game+618h and game+614h, both stored by 004BC890 (004BC8A7 / 004BC8AD).
    std::int32_t raw_game_mode_618{8};
    std::int32_t game_mode_614{8};
    // game+61Ch, the forced-mode byte, 004BC890's second argument (004BC894).
    bool forced_61c{false};
};

// The campaign: no session, no skirmish, and mode 8 with nothing forced, which is
// what every mission's stage init leaves (0046B99B SetGameMode(8, 0)).
inline constexpr SessionContext campaign_session_context() noexcept { return {}; }

// 004BC890 BSP_Game_SetGameMode(mode, forced), the three stores at 004BC894,
// 004BC8A7 and 004BC8AD. Not modelled: the view loop that follows when +1FE4h and
// the record's +5FCh are set (004BCA0E..004BCA48).
inline void set_game_mode_004bc890(SessionContext& context, std::int32_t mode,
                                   bool forced) noexcept {
    context.forced_61c = forced;
    context.raw_game_mode_618 = mode;
    context.game_mode_614 = mode;
}

// 004BCA50 BSP_Game_GetEffectiveGameMode: +614h when +61Ch is set, +1FE4h is
// non-zero, or +614h is 8 or 9; otherwise 8 (004BCA72).
inline constexpr std::int32_t effective_game_mode_004bca50(
    const SessionContext& context) noexcept {
    return context.forced_61c || context.session_word_1fe4 != 0
            || context.game_mode_614 == 9 || context.game_mode_614 == 8
        ? context.game_mode_614 : 8;
}

// A front-end skirmish: 005E6F87 sets +218Ch; 0076FAD0 stores the session word 1
// (0076FE85) and ends with SetGameMode(game+618h, 0) (0076FEA4), +618h being the
// lobby's mode. LABELLED: 0046B99B's stage-init SetGameMode(8, 0) is not applied
// to a skirmish; whether it runs on a multiplayer stage init is open (MP4).
inline SessionContext skirmish_session_context(std::int32_t mode) noexcept {
    SessionContext context;
    context.session_word_1fe4 = 1;
    context.skirmish_218c = true;
    set_game_mode_004bc890(context, mode, false);
    return context;
}

// The process's context, the game object's fields. A campaign run never writes it.
inline SessionContext& session_context() noexcept {
    static SessionContext context;
    return context;
}

}  // namespace bsp
