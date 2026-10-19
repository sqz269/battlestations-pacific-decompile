// cc13_small1: the movie-screen wiring, surrender prompt, lobby map refresh and
// limbo respawn poll that cc9 released to cc13 (packet cc13-small1,
// docs/CC13_SMALL1.md). Names are hypotheses; see the header for the hooks. No x87
// apart from the 0.0f constants (FLDZ, FSTP float).

#include "bsp/cc13_small1.hpp"

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

void* game_pointer(Cc13Small1Host& host) {
    return *static_cast<void**>(host.image_global(0x00E188A8));
}

// The screen-id list the four movie-screen handlers build in a local
// std::vector<int> {allocator byte, first, last, end}: first/last/end zeroed,
// eight push_backs through 00442190, 004D6480(&vector, park) (ECX = [00E188A8],
// not read), then the storage released with 00BF65AC when first is non-null.
void park_movie_screens(std::uint8_t park, Cc13Small1Host& host) {
    alignas(4) std::uint8_t vector[0x10] = {};
    std::int32_t value = 0;
    constexpr std::int32_t kScreens[] = {0x36, 0x37, 0x38, 0x53, 0x33, 0x55, 0x47, 0x5A};
    for (const std::int32_t id : kScreens) {
        value = id;
        host.dword_vector_push_back_00442190(vector, &value);
    }
    host.gui_screens_park_widgets_004d6480(vector, park);
    if (void* const first = at<void*>(vector, 0x4); first != nullptr) host.crt_operator_delete_00bf65ac(first);
}

}  // namespace

// ---------------------------------------------------------------------------
// 005CD0F0 BSP_MovCam_SpectateUnit. __thiscall(ECX = screen, unit), RET 4. No SEH.
// Evidence: export listing 005CD0F0..005CD118.
// ---------------------------------------------------------------------------
void cc13_small1_movcam_spectate_unit_005cd0f0(void* screen, void* unit, Cc13Small1Host& host) {
    host.frontend_push_interface_request_004cc460(*static_cast<void**>(host.image_global(0x00E198C4)), 0x2B,
                                                  unit);       // 005CD101
    host.hud_movie_ensure_movcam_005cc000(screen);             // 005CD108
    host.movcam_set_target_007a6910(at<void*>(screen, 0x30), unit);  // 005CD111
}

// ---------------------------------------------------------------------------
// 005CD9C0 HudMovieScreen_Enter. __thiscall(ECX = screen), RET. SEH 00C73738
// (state 0 releases the local vector; not modelled). Evidence: export listing
// 005CD9C0..005CDAF5.
// ---------------------------------------------------------------------------
void cc13_small1_hud_movie_screen_enter_005cd9c0(void* screen, Cc13Small1Host& host) {
    if (at<std::uint8_t>(screen, 0x20) == 0) {
        host.hud_movie_engage_interface_005cd1a0(screen, *static_cast<void**>(host.image_global(0x00E188D8)));  // 005CD9E9
    }
    host.hud_movie_ensure_camera_005cc170(screen);  // 005CD9F0
    at<std::uint8_t>(game_pointer(host), 0x61D) = 1;
    park_movie_screens(1, host);  // 005CDA23..005CDAE4
}

// ---------------------------------------------------------------------------
// 005CDB00 HudMovieScreen_Exit. ECX is not read; RET. SEH 00C73758 (not modelled).
// Evidence: export listing 005CDB00..005CDC46; game+61Dh = 0 lies between the two
// context-level calls.
// ---------------------------------------------------------------------------
void cc13_small1_hud_movie_screen_exit_005cdb00(Cc13Small1Host& host) {
    host.render_service_set_value224_00b0d0b0(*static_cast<void**>(host.image_global(0x00F8D39C)), 0.0f);  // 005CDB25
    host.input_set_context_level_00a933f0(host.input_manager_singleton_004bec00(), 0x1E, 0);  // 005CDB2F, 005CDB36
    at<std::uint8_t>(game_pointer(host), 0x61D) = 0;                                          // 005CDB43
    host.input_set_context_level_00a933f0(host.input_manager_singleton_004bec00(), 3, 0);     // 005CDB49, 005CDB50
    park_movie_screens(0, host);  // 005CDB76..005CDC30
}

// ---------------------------------------------------------------------------
// 005CDC50 HudScreen38_ClearCameraMover. ECX is not read; RET. SEH 00C73778 (not
// modelled). Evidence: export listing 005CDC50..005CDD69.
// ---------------------------------------------------------------------------
void cc13_small1_hud_screen38_clear_camera_mover_005cdc50(Cc13Small1Host& host) {
    host.game_set_camera_mover_004bc410(game_pointer(host), nullptr);  // 005CDC72
    park_movie_screens(1, host);                                       // 005CDC98..005CDD53
}

// ---------------------------------------------------------------------------
// 005CDD70 BSP_HudEngineMovieScreen_Exit. ECX is not read; RET. SEH 00C73798 (not
// modelled). Evidence: export listing 005CDD70..005CDEBE.
// ---------------------------------------------------------------------------
void cc13_small1_hud_engine_movie_screen_exit_005cdd70(Cc13Small1Host& host) {
    host.render_service_set_value224_00b0d0b0(*static_cast<void**>(host.image_global(0x00F8D39C)), 0.0f);  // 005CDD96
    // 005CDD9B..005CDDC9: a cutscene player of kind 4 at game+38h is destroyed and
    // freed and the slot cleared (the image tests the pointer twice).
    void*& player = at<void*>(game_pointer(host), 0x38);
    if (player != nullptr && at<std::int32_t>(player, 0x60) == 4) {
        void* const doomed = player;
        if (doomed != nullptr) {
            host.cutscene_player_destruct_0045d930(doomed);  // 005CDDBB
            host.crt_operator_delete_00bf65ac(doomed);       // 005CDDC1
            player = nullptr;
        }
    }
    park_movie_screens(0, host);  // 005CDDED..005CDEA8
}

// ---------------------------------------------------------------------------
// 005CDF30 / 005CDF40: the vector receives &screen+30h / &screen+18h (the child
// pointer fields) through 004D6790. Evidence: export listings.
// ---------------------------------------------------------------------------
void cc13_small1_hud_movie_screen_collect_children_005cdf30(void* screen, void* vector, Cc13Small1Host& host) {
    host.pointer_vector_push_back_004d6790(vector, static_cast<std::uint8_t*>(screen) + 0x30);
}

void cc13_small1_hud_engine_movie_screen_collect_children_005cdf40(void* screen, void* vector, Cc13Small1Host& host) {
    host.pointer_vector_push_back_004d6790(vector, static_cast<std::uint8_t*>(screen) + 0x18);
}

// ---------------------------------------------------------------------------
// 005D36B0 BSP_PromptResult_Surrender. __fastcall(ECX = result), RET. Evidence:
// export listing 005D36B0..005D36D8. Result 1: 00771F20(game+1EF0h, [game+18C8h], 1),
// then a tail jump to 004DB030 with ECX = [00E188A8] reloaded.
// ---------------------------------------------------------------------------
void cc13_small1_prompt_result_surrender_005d36b0(std::int32_t result, Cc13Small1Host& host) {
    if (result != 1) return;
    void* const game = game_pointer(host);
    host.session_request_player_surrender_00771f20(static_cast<std::uint8_t*>(game) + 0x1EF0,
                                                   at<std::int32_t>(game, 0x18C8), result);  // 005D36C8
    host.game_toggle_pause_menu_004db030(game_pointer(host));                                  // 005D36D3
}

// ---------------------------------------------------------------------------
// 005DECA0 BSP_MultiGameLobby_RefreshMapIfChanged. __thiscall(ECX = lobby screen), RET.
// SEH 00C74988 (state 0 releases the local text block; not modelled). Evidence:
// export listing 005DECA0..005DEDF3.
// ---------------------------------------------------------------------------
void cc13_small1_lobby_refresh_map_if_changed_005deca0(void* screen, Cc13Small1Host& host) {
    void* const game = game_pointer(host);
    if (at<std::int32_t>(game, 0x18C8) < 0) return;  // 005DECCB JL
    const std::int32_t slot = at<std::int32_t>(game, 0x18EC);
    if (slot < 0 || slot > 7) return;  // JL / JG (signed)
    void* const mission = at<void*>(game, 0x5FC);
    if (mission == nullptr || at<std::uint8_t>(mission, 0x914) == 0) return;
    auto party_of_slot = [](void* g) {
        void* const player = at<void*>(g, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(g, 0x18EC)) * 4);
        return at<std::int32_t>(player, 0x28);
    };
    if (at<void*>(screen, 0x2D4) == mission && at<std::int32_t>(screen, 0x2D8) == party_of_slot(game)) return;

    // 005DED23..005DED41: remember the mission and the player's party.
    at<void*>(screen, 0x2D4) = mission;
    at<std::int32_t>(screen, 0x2D8) = party_of_slot(game_pointer(host));

    void* const record = host.lobby_mission_record_005d8650();  // 005DED47
    const std::int32_t party = at<std::int32_t>(screen, 0x2D8);  // 005DED4C, read before the calls
    host.game_assign_console_string2198_004c89a0(game_pointer(host), record);  // 005DED68
    host.mission_stats_reset_00626930(record, at<std::int32_t>(screen, 0x2D8));  // 005DED75, EDX = party (unread)

    // 005DED7A..005DEDDC: a local text block {+04h..+14h zero, +18h byte 1}: +10h the
    // mission's +920h string, the block itself the record's per-party string vector at
    // +154h*party + B8h + 64h, +18h = (party == 1); published, then released.
    alignas(4) std::uint8_t block[0x1C] = {};
    at<std::uint8_t>(block, 0x18) = 1;
    host.native_string_assign_header_00425f40(block + 0x10,
                                              static_cast<std::uint8_t*>(at<void*>(screen, 0x2D4)) + 0x920);  // 005DEDA7
    host.string_vector_assign_00506c80(block, static_cast<std::uint8_t*>(record) + party * 0x154 + 0xB8 + 0x64);  // 005DEDB4
    at<std::uint8_t>(block, 0x18) = at<std::int32_t>(screen, 0x2D8) == 1 ? 1 : 0;  // 005DEDC0 SETZ
    host.loading_screen_set_mission_text_0057d060(block);  // 005DEDCB
    host.text_block_release_004d2a80(block);              // 005DEDDC
}

// ---------------------------------------------------------------------------
// 00565D80 BSP_HudLimboScreen_PollRespawnInput. ECX is not read; RET. No SEH.
// Evidence: export listing 00565D80..00565DFA. Unless game+5D4h == 0Ch, the first of
// the actions E1h, 8Dh, 8Ch, 8Eh, 8Fh (in that order; ECX = [00E188A8] reloaded
// for each) pressed this frame tail-jumps to 006485A0 with ECX = [[00E198C4]+40h].
// No direct RNG draw (see docs/CC13_SMALL1.md).
// ---------------------------------------------------------------------------
void cc13_small1_limbo_poll_respawn_input_00565d80(Cc13Small1Host& host) {
    if (at<std::int32_t>(game_pointer(host), 0x5D4) == 0x0C) return;
    constexpr std::int32_t kActions[] = {0xE1, 0x8D, 0x8C, 0x8E, 0x8F};
    for (const std::int32_t action : kActions) {
        if (host.input_action_was_pressed_004c43c0(game_pointer(host), action)) {
            host.hud_force_select_unit_006485a0(at<void*>(*static_cast<void**>(host.image_global(0x00E198C4)), 0x40));
            return;
        }
    }
}

}  // namespace bsp
