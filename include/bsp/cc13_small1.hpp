#pragma once

// cc13_small1: the ten bodies cc9 released to cc13 under the gameplay terms
// (packet cc13-small1, docs/CC13_SMALL1.md):
// - the in-game movie-screen wiring 005CD0F0, 005CD9C0, 005CDB00, 005CDC50,
//   005CDD70, 005CDF30, 005CDF40;
// - the surrender prompt result 005D36B0;
// - the lobby map refresh 005DECA0;
// - the limbo respawn poll 00565D80.
// Names are hypotheses, not recovered symbols. These are new C++ interfaces
// over the native records, not drop-in replacements. Nothing is wired. Same
// conventions as cc13_sweepB..B11: raw byte blocks touched only at the offsets
// the image uses, and one named host hook per native call, at the same point
// with the same arguments.

#include "bsp/cc13_sweepB11.hpp"

#include <cstdint>

namespace bsp {

class Cc13Small1Host : public Cc13SweepB11Host {
public:
    // 005CD1A0 BSP_HudMovieScreen_EngageMovieInterface, __thiscall(ECX = screen, [00E188D8]), RET 4.
    virtual void hud_movie_engage_interface_005cd1a0(void* screen, void* arg) = 0;
    // 005CC170 BSP_HudMovieScreen_EnsureCamera, __thiscall(ECX = screen), RET.
    virtual void hud_movie_ensure_camera_005cc170(void* screen) = 0;
    // 005CC000 BSP_HudMovieScreen_EnsureMovCam, __thiscall(ECX = screen), RET.
    virtual void hud_movie_ensure_movcam_005cc000(void* screen) = 0;
    // 007A6910 BSP_MovCam_SetTarget, __thiscall(ECX = movcam, target), RET 4.
    virtual void movcam_set_target_007a6910(void* movcam, void* target) = 0;
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest, __thiscall(ECX = [00E198C4], id, arg), RET 8.
    virtual void frontend_push_interface_request_004cc460(void* manager, std::int32_t id, void* arg) = 0;
    // 004D6790 STL_Vector_PushBack_004d6790, __thiscall(ECX = vector, const value*), RET 4.
    virtual void pointer_vector_push_back_004d6790(void* vector, const void* value) = 0;
    // 00B0D0B0 BSP_RenderService_SetValue224, __thiscall(ECX = [00F8D39C], float), RET 4.
    virtual void render_service_set_value224_00b0d0b0(void* service, float value) = 0;
    // 00A933F0 BSP_InputManager_SetContextLevel, __thiscall(ECX = manager, context, level), RET 8.
    virtual void input_set_context_level_00a933f0(void* manager, std::int32_t context, std::int32_t level) = 0;
    // 0045D930 cutscene player destructor (cc13_sweepA2), __thiscall(ECX = player), RET.
    virtual void cutscene_player_destruct_0045d930(void* player) = 0;
    // 005D8650 (the lobby's mission record lookup), no register input; EAX = record.
    virtual void* lobby_mission_record_005d8650() = 0;
    // 004C89A0 BSP_Game_AssignConsoleString2198, __thiscall(ECX = game, record), RET 4.
    virtual void game_assign_console_string2198_004c89a0(void* game, void* record) = 0;
    // 00626930 BSP_MissionStats_ResetForMission, __fastcall(ECX = record); EDX = team is
    // loaded by the caller and not read (include/bsp/mission_briefing_start.hpp).
    virtual void mission_stats_reset_00626930(void* record, std::int32_t edx_team) = 0;
    // 00425F40 (native_string_assign_header_00425f40, sweepB8), 00506C80
    // (string_vector_assign_00506c80, sweepB2) and 004C43C0
    // (input_action_was_pressed_004c43c0, sweepB4) come from the chain.
    // 0057D060 BSP_LoadingScreen_SetMissionText, __thiscall(ECX = text block), RET.
    virtual void loading_screen_set_mission_text_0057d060(void* block) = 0;
    // 004D2A80 (ledger BSP_Game_ClearPendingMissionName), __thiscall(ECX = text block), RET:
    // here it releases the local text block.
    virtual void text_block_release_004d2a80(void* block) = 0;
    // 006485A0 BSP_InGameHudRoot_ForceSelectUnit, __thiscall(ECX = [[00E198C4]+40h]), RET.
    virtual void hud_force_select_unit_006485a0(void* hud_root) = 0;
};

// 005CD0F0 BSP_MovCam_SpectateUnit, __thiscall(ECX = screen, unit), RET 4.
void cc13_small1_movcam_spectate_unit_005cd0f0(void* screen, void* unit, Cc13Small1Host& host);
// 005CD9C0 HudMovieScreen_Enter (vtable slot), __thiscall(ECX = screen), RET.
void cc13_small1_hud_movie_screen_enter_005cd9c0(void* screen, Cc13Small1Host& host);
// 005CDB00 HudMovieScreen_Exit, __thiscall(ECX = screen, not read), RET.
void cc13_small1_hud_movie_screen_exit_005cdb00(Cc13Small1Host& host);
// 005CDC50 HudScreen38_ClearCameraMover, __thiscall(ECX = screen, not read), RET.
void cc13_small1_hud_screen38_clear_camera_mover_005cdc50(Cc13Small1Host& host);
// 005CDD70 BSP_HudEngineMovieScreen_Exit, __thiscall(ECX = screen, not read), RET.
void cc13_small1_hud_engine_movie_screen_exit_005cdd70(Cc13Small1Host& host);
// 005CDF30 / 005CDF40 CollectChildren, __thiscall(ECX = screen, vector), RET 4:
// push_back of the field at screen+30h / +18h.
void cc13_small1_hud_movie_screen_collect_children_005cdf30(void* screen, void* vector, Cc13Small1Host& host);
void cc13_small1_hud_engine_movie_screen_collect_children_005cdf40(void* screen, void* vector, Cc13Small1Host& host);
// 005D36B0 BSP_PromptResult_Surrender, __fastcall(ECX = result), RET.
void cc13_small1_prompt_result_surrender_005d36b0(std::int32_t result, Cc13Small1Host& host);
// 005DECA0 BSP_MultiGameLobby_RefreshMapIfChanged, __thiscall(ECX = lobby screen), RET.
void cc13_small1_lobby_refresh_map_if_changed_005deca0(void* screen, Cc13Small1Host& host);
// 00565D80 BSP_HudLimboScreen_PollRespawnInput, ECX not read, RET.
void cc13_small1_limbo_poll_respawn_input_00565d80(Cc13Small1Host& host);

}  // namespace bsp
