#pragma once

// cc13_sweepB9: ninth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB9, docs/CC13_SWEEPB9.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the
// game.
//
// Same conventions as cc13_sweepB..B8 and cc13_pkt642: native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named host method at the same point with
// the same arguments. Cc13SweepB9Host extends Cc13SweepB8Host, so the earlier
// hooks are reused as they are.

#include "bsp/cc13_sweepB8.hpp"

#include <cstdint>

namespace bsp {

class Cc13SweepB9Host : public Cc13SweepB8Host {
public:
    // ---- session browser registration (segment 19, 0056F320) ----
    // Screen vtable +14h, __thiscall(ECX = screen), no arguments.
    virtual void screen_vcall_14(void* screen) = 0;
    // 00467CF0 BSP_NativeString_ReverseFindHeader, __thiscall(ECX = string;
    // const NativeString* needle, unsigned from), RET 8; EAX = position or -1.
    virtual std::int32_t native_string_rfind_00467cf0(void* string, const void* needle, std::uint32_t from) = 0;
    // Vtable slot +A4h of the screen's texture owner +A0h, __thiscall(ECX =
    // owner; NativeString* name, const float* uv_rect (four floats), 0, 0);
    // the four pushes are the callee's to clean.
    virtual void texture_owner_vcall_a4(void* owner, void* name, const float* uv_rect, std::int32_t a,
                                        std::int32_t b) = 0;
    // 005684D0 BSP_MultiSessionBrowserScreen_BuildCreateOptions, __fastcall(ECX
    // = screen), RET (reconstructed in frontend_seg12_17_cc13 under another host).
    virtual void session_browser_build_create_options_005684d0(void* screen) = 0;

    // ---- unit library enter (segment 27, 0067F4D0) ----
    // 004CB420 BSP_SceneEnvironmentBlock_ConstructDefault, __fastcall(ECX =
    // 288h-byte block); EAX = block.
    virtual void* scene_environment_block_construct_004cb420(void* block) = 0;
    // 004C6520 BSP_SceneRecordHeader_Destruct, __fastcall(ECX = block).
    virtual void scene_environment_block_destruct_004c6520(void* block) = 0;
    // 0078C9B0 (applies an authored environment block to the ocean
    // environment), __thiscall(ECX = environment; const block*), RET 4.
    virtual void ocean_environment_apply_block_0078c9b0(void* environment, const void* block) = 0;

    // ---- lobby player rows (segment 22, 005D73A0) ----
    // 00786010 BSP_GamePeerRows_GetRowQuality, __thiscall(ECX = [00E188A8] +
    // 1EF8h; peer id), RET 4; result in ST0.
    virtual float game_peer_rows_get_row_quality_00786010(void* rows, std::uint32_t peer) = 0;
    // 00517CB0 BSP_FrontEnd_SetPlayerRankIconFrame, register ABI: ECX = icon
    // widget, EDX = rank (zero-extended byte); RET.
    virtual void frontend_set_player_rank_icon_frame_00517cb0(void* icon, std::uint32_t rank) = 0;
    // 007F74B0 BSP_PlayerRecord_IsTalking, __thiscall(ECX = player record); AL.
    virtual bool player_record_is_talking_007f74b0(void* record) = 0;
    // 00778740 BSP_VoiceMuteTable_IsMutedOneWay, __thiscall(ECX = [00E188A8] +
    // 2080h; four dwords: the local record's +80h/+84h, then the row
    // record's +80h/+84h), RET 10h; AL.
    virtual bool voice_mute_is_muted_one_way_00778740(void* table, std::uint32_t local_lo, std::uint32_t local_hi,
                                                      std::uint32_t remote_lo, std::uint32_t remote_hi) = 0;

    // ---- packet statistics CSV row (segment 44, 007884D0) ----
    // 0043BDF0 BSP_NativeString_ConstructFromFloat ("%f"), __thiscall(ECX =
    // out; float), RET 4; EAX = out.
    virtual void* native_string_from_float_0043bdf0(void* out, float value) = 0;
    // 00788380 BSP_NetworkPacketStatistics_FormatField, __thiscall(ECX =
    // left; NativeString* out, unsigned value), RET 8; EAX = out
    // (reconstructed in segs_13_15_22_44_cc13 under another host).
    virtual void* packet_stats_format_field_00788380(const void* left, void* out, std::uint32_t value) = 0;
    // 00788250 BSP_NetworkPacketStatistics_FormatTypeColumn, __thiscall(ECX =
    // 16-byte type entry; int type, log file), RET 8 (writes the column).
    virtual void packet_stats_format_type_column_00788250(void* entry, std::int32_t type, void* file) = 0;
    // CSV log file (+7649A0h / +7649A4h) vtable +5Ch(const NativeString*
    // text, 0): writes the text.
    virtual void log_file_vcall_5c_write(void* file, const void* text, std::int32_t flags) = 0;

    // ---- HUD formation menu (segment 15, 00536A90) ----
    // Command director (unit vtable +114h result) vtable +50h, no
    // arguments; EAX (the order stance, 0 = defensive).
    virtual std::int32_t director_vcall_50_stance(void* director) = 0;
    // Command director vtable +54h(int stance), RET 4.
    virtual void director_vcall_54_set_stance(void* director, std::int32_t stance) = 0;
    // 0070D060 BSP_UnitGroup_MemberAt, __thiscall(ECX = group; int), RET 4; EAX.
    virtual void* unit_group_member_at_0070d060(void* group, std::int32_t index) = 0;
    // 0070EFD0 BSP_UnitGroup_ApplyFormationShape, __thiscall(ECX = group;
    // int shape), RET 4.
    virtual void unit_group_apply_formation_shape_0070efd0(void* group, std::int32_t shape) = 0;
    // 0077A080 BSP_SessionMessage_FORMATION_SET_Construct, __thiscall(ECX =
    // message storage; group), RET 4; EAX = message. Host call only (cc9
    // keep-out address).
    virtual void* session_message_formation_set_construct_0077a080(void* message, void* group) = 0;
    // 0077C880 BSP_UnitGroup_BroadcastFormation, __thiscall(ECX = group
    // leader [group+14h]; message), RET 4. Host call only (cc9 range).
    virtual void unit_group_broadcast_formation_0077c880(void* leader, void* message) = 0;
    // 005347C0 BSP_SessionMessageFormation_Destruct, __fastcall(ECX = message).
    virtual void session_message_formation_destruct_005347c0(void* message) = 0;
    // 00533A30 BSP_HudFormationScreen_HideAllWidgets, __fastcall(ECX = screen).
    virtual void formation_screen_hide_all_widgets_00533a30(void* screen) = 0;
    // 00AA85B0 (GUI widget colour setter), __thiscall(ECX = widget; const
    // float rgba[4]), RET 4.
    virtual void gui_widget_set_colour_00aa85b0(void* widget, const float* rgba) = 0;

    // ---- HUD map player list layout (segment 20, 005B3470) ----
    // 005A0AC0 STL_inst_005a0ac0 (std::vector<T*>::erase(first, last) with
    // checked iterators), __thiscall(ECX = vector; iterator* out, {container,
    // first}, {container, last}), RET 14h; EAX = out.
    virtual void* pointer_vector_erase_range_005a0ac0(void* vector, void* out_iterator, void* first_container,
                                                      void* first, void* last_container, void* last) = 0;
    // 00644230 TRIV_body_00644230, __fastcall(ECX = [[00E198C4]+40h]); EAX
    // (the camera-like object whose vtable +C4h gives a position).
    virtual void* trivial_body_00644230(void* self) = 0;
    // 00AD56D0 TRIV_body_00AD56D0, __fastcall(ECX = [00F8C210]); EAX (the
    // scene root walked for "Terrain Visibility Group").
    virtual void* trivial_body_00ad56d0(void* self) = 0;
    // 005293B0 BSP_HudCounters_HideUnderScreen54, __fastcall(ECX = counters).
    virtual void hud_counters_hide_under_screen54_005293b0(void* counters) = 0;
    // 0068E420 BSP_AwardTracker_FinishActiveHint, __fastcall(ECX = tracker).
    virtual void award_tracker_finish_active_hint_0068e420(void* tracker) = 0;
    // 0068E600 BSP_AwardTracker_HasHintBeenShown, __thiscall(ECX = tracker;
    // const NativeString*), RET 4; AL.
    virtual bool award_tracker_has_hint_been_shown_0068e600(void* tracker, const void* name) = 0;
    // 00690FD0 BSP_AwardTracker_TriggerHint, __thiscall(ECX = tracker; const
    // NativeString*, int forced), RET 8.
    virtual void award_tracker_trigger_hint_00690fd0(void* tracker, const void* name, std::int32_t forced) = 0;
};

// 006179D0 BSP_HudPowerupCarousel_RebuildSlots, __fastcall(ECX = carousel,
// the powerup screen + 100h per hud_widgets.hpp), RET; caller 00619C50.
// Scrolls the first powerup column of the support block [[00E198C4]+C0h]:
// shifts the four clone / icon / highlight / section slots one place in the
// direction of byte +3Ch, re-finds the three kept "circle_full_N_Section"
// children, clones a new slot from the template [[00E198C4]+BCh]+138h and
// fills its icon states, its frame from the record selected by +4h, its
// position and its section, then +40h = 1.0f.
void cc13_sweepB9_powerup_carousel_rebuild_slots_006179d0(void* carousel, Cc13SweepB9Host& host);

// 0056F320 BSP_MultiSessionBrowserScreen_Register (vtable 00CEF134 slot
// +10h), __thiscall(ECX = screen), RET. Chains 004F71D0, clears +50h, calls
// its own vtable +14h; for every 434h-byte scene record of the vector
// [[00E198AC]+5Ch]+24h preloads eight "gui/radarshots/<name>" textures
// (name = the file name after the last '/', cut before ".scn", first 14h
// characters; suffixes _C, _D, _E, _ICH, _ICL, _ICM, _ICS, _S .tga) through
// +A0h vtable +A4h; then sets the "no server found" text, caches the widget
// positions and gaps, builds the filter rows (0056D9A0) and create options
// (005684D0), copies the session settings from [00E188A8] and clears the
// selection state.
void cc13_sweepB9_session_browser_register_0056f320(void* screen, Cc13SweepB9Host& host);

// 0067F4D0 BSP_UnitLibraryScreen_Enter (vtable 00CF6E50 slot +18h),
// __thiscall(ECX = screen), RET. Creates the ocean environment +E0h on first
// entry and applies a default environment block to it, selects the
// "UNITLIB" front-end frame and help line, then binds the unit library's
// widgets under +6Ch / +78h / +7Ch / +70h into +70h..+DCh (the main list box
// gets the listener +64h, the seven picture / arrow icons the listener +68h)
// and clears +E8h.
void cc13_sweepB9_unit_library_enter_0067f4d0(void* screen, Cc13SweepB9Host& host);

// 005D73A0 BSP_MultiGameLobbyScreen_RefreshPlayerRows, __fastcall(ECX =
// lobby screen), RET; caller 005E5B30. Shows +148h / +14Ch by the session
// mode, sets the front-end flag for modes 0 and 1, then for the eight rows
// +250h.. and the player records [00E188A8]+18CCh.. fills the flags, rank,
// talk, check, drop-rate and connection icons, the ping icons (00578580),
// the name and tag texts and their colours.
void cc13_sweepB9_lobby_refresh_player_rows_005d73a0(void* screen, Cc13SweepB9Host& host);

// 007884D0 BSP_NetworkPacketStatistics_WriteCsvRow, __fastcall(ECX = stats),
// RET; callers 00789560, 0078A430. Samples the clock into +764930h, writes
// "Time;<previous>;<now>\n" to both CSV files, then for each pending
// statistics row ([+764994h] % 1001 up to [+764998h] % 1001) sums the 234
// type entries of each direction and, when the direction saw packets,
// writes "<min>-<max>;<bytes>;<bytes/count>;" plus the 234 type columns
// and a newline, resetting each entry; finally advances both cursors by
// +76499Ch, keeps the time in +76492Ch and clears +7649A8h.
void cc13_sweepB9_packet_stats_write_csv_row_007884d0(void* stats, Cc13SweepB9Host& host);

// 00536A90 BSP_HudFormationScreen_UpdateOrderLabels, __thiscall(ECX =
// formation screen; float dt), RET 4; caller 0053A8E0. In the order (ECh)
// and formation (EFh) states it steers the four-way selector from the two
// analogue sticks (+144h..+150h), picks the highlighted entry +16Ch (0
// cancel, 1 line, 3 diamond / stance toggle, 4 column / disband), shows
// its label, and on the confirm button applies the stance or formation
// (or raises the disband prompt); while open it fades the frame +A8h and
// pulses the option highlight alpha, closing the menu (state 128h) when
// the frame alpha reaches zero.
void cc13_sweepB9_formation_update_order_labels_00536a90(void* screen, float dt, Cc13SweepB9Host& host);

// 005B3470 BSP_HudMapScreen_BuildPlayerList (vtable 00CF0118 slot 13),
// __fastcall(ECX = HUD map screen), RET. Ducks speech, hides the map panel
// for a live local player, binds All_Group (+94h) and 23 more named children
// (templates and icons hidden, popup_FrameBox into a fresh 11-entry
// vector +D4h), caches the camera position, centres the cursor, resets the
// map state and reloads the background (sweepB5's 005B2D70), dismisses the
// hints, triggers MAP1STUSE / MAPPERMANENT, hides every non-bush child of
// each "Terrain Visibility Group" node and re-registers those nodes under
// the screen's root, then clears the pick state.
void cc13_sweepB9_hud_map_build_player_list_005b3470(void* screen, Cc13SweepB9Host& host);

}  // namespace bsp
