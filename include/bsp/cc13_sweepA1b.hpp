#pragma once
// Packet cc13-sweepA1 (worktree cc13-sweepA1b): the remaining segment 0..14
// leftovers of docs/CC13_SWEEPA.md after sweepA2..A4.
// docs/CC13_SWEEPA1.md lists every function with its outcome, ABI and
// uncertainty, and the ones skipped with the reason.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a SweepA1bHost method named
// for its target; SweepA1bHost extends SweepA4Host (and so SweepA3Host,
// SweepA2Host and SweepAHost), whose hooks are reused. Descriptive names are
// hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_sweepA4.hpp"

#include <cstdint>

namespace bsp::cc13_sweepA1b {

using cc13_sweepA::Raw;

struct SweepA1bHost : cc13_sweepA4::SweepA4Host {
    // --- multiplayer interface update (004D80D0) --------------------------
    // [[00F8A2FC]]+0Ch, __thiscall(ECX = network session, float), RET 4.
    virtual void session_vcall_0c(void* session, float value) = 0;
    // [[00F8A2FC]]+19Ch and +40h, __thiscall(ECX = network session), AL.
    virtual bool session_vcall_19c(void* session) = 0;
    virtual bool session_vcall_40(void* session) = 0;
    // 006890B0 BSP_MultiMenu_UpdateOnlineInterface, __thiscall(ECX = [00E198B4]).
    virtual void multi_menu_update_online_interface_006890b0(void* multi_menu) = 0;
    // 0076A750 (session poll, an empty body in the image), __thiscall(ECX = game+1EF0h).
    virtual void session_poll_0076a750(Raw session) = 0;
    // 004B61C0 BSP_Session_IsClientStage3Or4, __thiscall(ECX = game+1EF0h), AL.
    virtual bool session_is_client_stage3_or4_004b61c0(Raw session) = 0;
    // 00688C70 BSP_MultiMenu_LeaveToMainInterface, __thiscall(ECX = [00E198B4]).
    virtual void multi_menu_leave_to_main_interface_00688c70(void* multi_menu) = 0;
    // 00688AD0 BSP_MultiMenu_PushRequestInterfaceLogged, __thiscall(ECX =
    // [00E198B4], interface id), RET 4.
    virtual void multi_menu_push_request_interface_logged_00688ad0(void* multi_menu, std::int32_t id) = 0;
    // 005D2E70 BSP_MultiErrorScreen_SetErrorCode, __thiscall(ECX = [[00E198B4]+50h], code), RET 4.
    virtual void multi_error_screen_set_error_code_005d2e70(void* screen, std::int32_t code) = 0;
    // 004BDF50 BSP_MultiplayerInterface_ApplyPendingSize, __thiscall(ECX = [00E198B4]).
    virtual void multiplayer_interface_apply_pending_size_004bdf50(void* multi_menu) = 0;
    // 004D7BA0 BSP_Game_ShowSessionLostMessage, __thiscall(ECX = game, the
    // flag byte in a dword whose upper bytes are a stale stack slot), RET 4
    // (reconstructed as cc13_sweepA2::game_show_session_lost_message_004d7ba0).
    virtual void game_show_session_lost_message_004d7ba0(Raw game, std::uint8_t kicked) = 0;
    // 004254B0 with the format only, __cdecl (empty body).
    virtual void debug_print0_004254b0(const char* format) = 0;
    // 006529E0 (an image AL query), __thiscall(ECX = [[00E198C4]+BCh]).
    virtual bool hud_query_006529e0(void* object) = 0;
    // 004C3A80 (controlled-unit selection, cc9 lane), __thiscall(ECX = game).
    virtual void game_select_controlled_unit_004c3a80(Raw game) = 0;
    // 0076C500 BSP_Session_BroadcastQueuedMessages, __thiscall(ECX = game+1EF0h).
    virtual void session_broadcast_queued_messages_0076c500(Raw session) = 0;

    // --- scene database kill pass (0046B730) --------------------------------
    // 004881F0 (an STL-shaped list walk), __thiscall(ECX = [[00E188A8]+21D0h]).
    virtual void game_object_list_walk_004881f0(void* object) = 0;
    // 00468DB0 std::map find, __thiscall(ECX = database+158h; iterator* out
    // ({container, node}), const key*), RET 8; EAX = out.
    virtual void* database_map_find_00468db0(Raw map, Raw out, const void* key) = 0;
    // 004323D0 BSP_PointerVector_PushBack, __thiscall(ECX = vector, const
    // void* const* value), RET 4.
    virtual void pointer_vector_push_back_004323d0(Raw vector, void* const* value) = 0;
    // 004322D0 STL_Vector_InsertOne, __thiscall(ECX = vector; iterator* out,
    // container, where, void* const* value), RET 10h.
    virtual void pointer_vector_insert_one_004322d0(Raw vector, Raw out, Raw container, void** where,
                                                    void* const* value) = 0;
    // 00874D00 BSP_Game_RunExtraFixedStep, mode byte in CL.
    virtual void game_run_extra_fixed_step_00874d00(std::uint8_t mode) = 0;
    // 00903670 BSP_EntityWorld_FlushActivations, __thiscall(ECX = [[00E188A8]+19CCh]).
    virtual void entity_world_flush_activations_00903670(void* world) = 0;
    // [[world]]+0Ch, __thiscall(ECX = world, float dt), RET 4.
    virtual void world_vcall_0c(void* world, float dt) = 0;
    // 004BC890 BSP_Game_SetGameMode, __thiscall(ECX = [00E188A8]; mode, arg), RET 8.
    virtual void game_set_game_mode_004bc890(void* game, std::int32_t mode, std::int32_t arg) = 0;
    // 00468FB0 BSP_SceneClassMap_FindNode, __thiscall(ECX = database+34h;
    // const NativeString* key, int* out_hash), RET 8; EAX = node (the class
    // record at node+8h).
    virtual void* scene_class_map_find_node_00468fb0(Raw map, Raw key, std::int32_t* out_hash) = 0;
    // 00BF7FBF _stricmp, __cdecl.
    virtual std::int32_t crt_stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00925A90 BSP_EntityRegistry_FindEntityByName, __thiscall(ECX = world, const char*), RET 4.
    virtual void* entity_registry_find_by_name_00925a90(void* world, const char* name) = 0;
    // The class record's create function [class+4h]: ECX = [class+0h], EDX =
    // [record+8h], stack (entity, record+14h, [record+4h], 0); EAX = the new
    // object (callee cleanup assumed from the caller not adjusting ESP).
    virtual void* scene_class_create(void* function, std::uint32_t ecx, std::uint32_t edx, void* entity,
                                     void* properties, std::uint32_t bag, std::int32_t zero) = 0;
    // 00922E20 BSP_ScenePropertyBagRef_Construct, __thiscall(ECX = 0Ch-byte
    // block, bag), RET 4; EAX = block.
    virtual void* scene_property_bag_ref_construct_00922e20(void* block, std::uint32_t bag) = 0;
    // 00925F20 BSP_SEntity_InitAll, mode byte in CL.
    virtual void sentity_init_all_00925f20(std::uint8_t mode) = 0;
    // 0046AAB0 BSP_SceneDatabase_ResolveDeferredReferences, __thiscall(ECX = database).
    virtual void scene_resolve_deferred_references_0046aab0(void* database) = 0;
    // 00BF67A7 memmove_s, __cdecl(dest, dest size, source, count).
    virtual void crt_memmove_s_00bf67a7(void* dest, std::uint32_t size, const void* source, std::uint32_t count) = 0;

    // --- 2D geometry (004F4880) ---------------------------------------------
    // 00419260 BSP_Vector2f_ReciprocalLength, __thiscall(ECX = float[2]); ST0.
    virtual float vector2f_reciprocal_length_00419260(const float* v) = 0;
    // 00414C60 BSP_Vector2f_LengthWithCutoff, __thiscall(ECX = float[2]); ST0.
    virtual float vector2f_length_with_cutoff_00414c60(const float* v) = 0;
    // 004F3630 BSP_Math_SegmentParameters2D, __fastcall(ECX = a0, EDX = da;
    // b0, db, float* t, float* u), RET 10h; AL (the caller ignores it).
    // Reconstructed as native_segment_parameters_004f3630.
    virtual bool math_segment_parameters_2d_004f3630(const float* a0, const float* da, const float* b0,
                                                     const float* db, float* t, float* u) = 0;
    // 00BF7030 _CIsqrt: ST0 in, ST0 out.
    virtual float crt_cisqrt_00bf7030(float value) = 0;
};

// 004D80D0 BSP_Game_UpdateMultiplayerInterface, __fastcall(ECX = game), RET;
// SEH frame (00C66498); sole caller BSP_Game_OnMove (004E5434). Session
// polls, the termination / lost-session prompts and error codes by the
// interface state +624h and game mode +5D4h, the controlled-unit selection in
// mode 0Dh, and the queued-message broadcast in modes 0Ch, 0Dh and 12h.
void game_update_multiplayer_interface_004d80d0(Raw game, SweepA1bHost& host);

// 0046B730 (scene database kill pass), __fastcall(ECX = database, [00E18680]),
// RET; sole caller BSP_EngineMovie_Stop (0045F600). Kills (cause 2) every
// world entity, and every child of a registered one, whose +C4h key is not in
// the database map +158h; runs the fixed-step / flush / world-tick sequence;
// sets game mode 8; recreates through the class map +34h each database record
// (vector +24h) whose class key is not in the map, binding the named entity;
// InitAll twice, resolves deferred references, runs the sequence again, then
// deletes every record and empties the vector.
void scene_database_kill_pass_0046b730(Raw database, SweepA1bHost& host);

// 004F3DA0 BSP_Triangle2f_ContainsPoint, __fastcall(ECX = point float[2],
// EDX = triangle float[6] a, b, c), RET; AL. Solves p - a = u (b - a) +
// t (c - a) and answers 0 <= t <= 1, u >= 0, t + u <= 1 (NaN fails every
// test). Only caller: the undefined wrapper 004F3EE0 (no references).
bool triangle2f_contains_point_004f3da0(const float* point, const float* triangle);

// 004F4880 BSP_Segment2f_ClipAgainstLine, __fastcall(ECX = point P, EDX =
// segment start A; float radius, const float* B, float* out0, float* out1),
// RET 10h; EAX = 0..2. With n the unit direction of B - A and m = (-n.y,
// n.x), 004F3630 crosses A..B with the line through P - m along m, giving t
// on A..B and s along m; when -r <= s <= r, the parameters
// t -+ sqrt(r^2 - s^2) / |B - A| that lie in [0, 1] are written to out0, then
// out0 or out1 (the second goes to out1 only when the first was written).
// Only caller: the undefined
// function 004F4AE0 (no references).
std::int32_t segment2f_clip_against_line_004f4880(const float* point, const float* a, float radius,
                                                  const float* b, float* out0, float* out1, SweepA1bHost& host);

}  // namespace bsp::cc13_sweepA1b
