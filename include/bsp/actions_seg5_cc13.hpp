#pragma once

// Segment 5 (00454A50..004654D0): engine-movie (.ema) action run bodies, the
// movie player's track lookups and the small session-message and unit-cast
// helpers that sit among them. Evidence and per-function notes:
// docs/CC13_SEG5.md. Every name is a hypothesis, not a recovered symbol.
//
// These bodies are written against the image's object layout: an object is a
// byte pointer and every field is read or written at the image's offset
// (native_at below). Every call the image makes, virtual or direct, goes
// through a named Seg5Host method at the same point with the same arguments;
// globals are read through the host each time the image loads them.
//
// Engine-movie action (44h bytes, see include/bsp/engine_movie_actions.hpp):
//   +04h float ActivateTime, +10h type, +14h the resolved ReferPath entity,
//   +20h/+24h/+28h Num1..Num3, +2Ch NativeString Enum {size +2Ch, data +30h},
//   +34h OffsetX, +3Ch OffsetY, +38h OffsetZ.
// Run bodies are the action vtable slot 0, __thiscall(ECX action,
// [ESP+4] entity), RET 4, called by 004560E0.

#include <cstdint>

namespace bsp::cc13_seg5 {

static_assert(sizeof(void*) == 4, "segment 5 bodies use the Win32 object layout");

using Object = std::uint8_t*;

template <class T>
T& native_at(Object base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct Seg5Host {
    virtual ~Seg5Host() = default;

    // Globals, read where the image reads them.
    virtual Object game_00e188a8() = 0;                    // [00E188A8]; +38h is the movie player
    virtual Object hud_00e198c4() = 0;                     // [00E198C4]
    virtual std::uint16_t local_slot_00f876b8() = 0;       // word [00F876B8]
    virtual const float* static_vector_00f87574() = 0;     // three floats 00F87574..7C
    virtual Object command_00e08f78() = 0;                 // the command object at 00E08F78
    virtual Object command_00e08ef8() = 0;                 // the command object at 00E08EF8

    // Virtual calls, named by vtable offset.
    virtual bool is_kind_vcall_5c(Object object, std::int32_t kind) = 0;            // vtable +5Ch
    virtual void view_set_active_vcall_34(Object view, std::int32_t active) = 0;    // vtable +34h
    virtual void weapon_fire_vcall_1d8(Object weapon, std::int32_t a, float b, float c) = 0;  // +1D8h
    virtual void weapon_fire_vcall_1f0(Object weapon) = 0;                           // +1F0h

    // Direct calls.
    virtual bool native_string_equals_ci_00425850(Object native_string, const char* text) = 0;
    virtual void entity_vcall_ac(Object entity, std::int32_t arg) = 0;      // entity vtable +ACh
    virtual const float* track_eval_position_004581e0(Object track, float* out, float time,
                                                      std::uint8_t flag) = 0;
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    virtual void hud_movie_screen_show_black_bars_005cb0f0(Object screen) = 0;
    virtual void hud_movie_screen_hide_black_bars_005cb130(Object screen) = 0;
    virtual void gun_set_barrel_reload_timer_0072cf00(Object gun, std::int32_t barrel, float time,
                                                      std::int32_t flag) = 0;
    virtual void session_message_construct_base_0075b430(Object message, std::int32_t type) = 0;
    virtual void session_message_write_extended_header_0075b480(Object message, Object stream) = 0;
    virtual void session_message_read_extended_header_0075b4c0(Object message, Object stream) = 0;
    virtual void bit_cursor_write_boolean_004290b0(Object cursor, std::uint8_t value) = 0;
    virtual void bit_cursor_read_boolean_00428d70(Object cursor, Object out_byte) = 0;
    virtual void session_route_message_0077c2a0(Object entity, Object message, std::int32_t a,
                                                std::int32_t b) = 0;
    virtual void entity_issue_command_0077d600(Object entity, Object command, Object target,
                                               std::int32_t flags) = 0;
    virtual void unit_request_ordnance_release_007bbba0(Object unit) = 0;
    virtual void turning_gun_set_target_angles_0085aba0(Object gun, float a, float b) = 0;

    // Movie track calls (batch 2).
    virtual void action_stop_vcall_04(Object action) = 0;                 // action vtable +4
    virtual void track_reset_vcall_04(Object track) = 0;                  // track vtable +4
    virtual void track_update_vcall_0c(Object track, float time) = 0;     // track vtable +0Ch
    virtual void crt_free_00bf65ac(void* memory) = 0;
    virtual Object sized_pool_singleton_00419cc0(void* data, std::uint32_t bytes, std::int32_t flag) = 0;
    virtual void sized_pool_return_block_00bd1510(Object pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    virtual void native_string_resize_0041dd40(Object native_string, std::uint32_t size,
                                               std::int32_t flag) = 0;
    virtual void crt_memcpy_00bf7680(void* dest, const void* src, std::uint32_t bytes) = 0;
    virtual void movie_track_bind_entities_00456dd0(Object track) = 0;
    virtual void game_build_local_player_unit_lists_004c3cb0(Object game) = 0;
    virtual void entity_track_update_00459020(Object track, float time) = 0;
    virtual void camera_track_update_shakes_0045acc0(Object track) = 0;
    virtual void shake_list_add_0045a320(Object list, float time, float x, float y, float z,
                                         float amplitude) = 0;
    virtual void list_node_value_construct_00455880(Object place, Object source) = 0;
    virtual bool win32_file_writer_open_009501b0(Object writer, const char* path) = 0;
    virtual void close_handle_import_00ce225c(void* handle) = 0;          // CALL [00CE225C]

    // Track construction and movie start (batch 3).
    virtual Object list_buy_head_node_00455520(Object list) = 0;
    virtual Object list_buy_head_node_00455540(Object list) = 0;
    virtual Object list_buy_head_node_004555d0(Object list) = 0;
    virtual void timed_value_construct_004658d0(Object object, float value) = 0;
    virtual void game_set_cinematic_mode_004cd0f0(Object game, std::int32_t a, std::int32_t b,
                                                  std::int32_t c) = 0;
    virtual void narrative_arm_blackout_fade_005b9ba0(Object screen, float level, float duration,
                                                      Object callback_name) = 0;
    virtual void narrative_begin_sound_fade_005b9760(Object screen, float a, float b,
                                                     Object callback_name) = 0;
    virtual void hud_movie_screen_005cb160(Object screen) = 0;

    // Batch 4.
    virtual void action_destroy_00465610(Object action) = 0;
    virtual void float_vector_insert_one_0045a120(Object vector, Object result_iterator,
                                                  Object where_container, float* where,
                                                  const float* value) = 0;
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual bool native_string_equals_insensitive_00435c40(Object string, Object other) = 0;
};

// ---------------------------------------------------------------------------
// Movie player lookups (the 74h-byte player at [00E188A8]+38h keeps its
// tracks in a std::vector<Track*> at +68h/+6Ch; a track's entity is +11Ch and
// its name NativeString is +120h {size, data +124h}).
// ---------------------------------------------------------------------------

// 0045C690 __thiscall(ECX player, [ESP+4] entity), RET 4: the first track
// whose +11Ch equals `entity`, else null.
Object movie_find_track_by_entity_0045c690(Object player, Object entity);

// 0045DCA0 __fastcall(ECX player), RET: the first track whose name (copied by
// 0045D840) is "CameraPosition 01" (00CE4DA4, _stricmp), else null.
Object movie_find_camera_track_0045dca0(Seg5Host& host, Object player);

// 0045D840 __thiscall(ECX track, [ESP+4] out NativeString), RET 4: out =
// {0, 0}; unless out is track+120h, 0041DD40(out, size, 1) and, for a
// non-empty name, memcpy(out.data, name.data, out.size). Returns out.
Object movie_track_copy_name_0045d840(Seg5Host& host, Object track, Object out);

// 0045C560 __fastcall(ECX player), RET: every track's vtable+0Ch(player
// time +4), in vector order (the time is re-read for each track).
void movie_update_tracks_0045c560(Seg5Host& host, Object player);

// ---------------------------------------------------------------------------
// Entity track (vtable 00CE4DB8 family). Lists are MSVC std::list objects
// {+0 allocator/proxy, +4 head node, +8 size}; a node is {next, prev, value}.
//   +0F4h list of offset samples (value +0Ch..+14h three floats, i.e. node
//         +14h..+1Ch), iterator {container +100h, node +104h}
//   +108h list<Action*>, scratch iterator {+114h, +118h}
//   +11Ch entity, +120h name NativeString
//   +140h/+144h the run cursor (container, node), +14Ch parent state
// ---------------------------------------------------------------------------

// 004561E0 __fastcall(ECX track), RET: walks the action list with the
// scratch iterator from the first node and calls each action's vtable+4
// (stop). The iterator is left at the head.
void entity_track_stop_actions_004561e0(Seg5Host& host, Object track);

// 00456290 __fastcall(ECX track), RET: cursor = (track+108h, first node),
// the track's vtable+4, then +14Ch = 0.
void entity_track_rewind_00456290(Seg5Host& host, Object track);

// 004562D0 __thiscall(ECX track, [ESP+4] out float[3]), RET 4: the mean of
// the three floats of every +0F4h node (sums accumulated as float), divided
// by float(unsigned size +0FCh).
void entity_track_mean_offset_004562d0(Object track, float* out);

// 004567D0 __thiscall(ECX track, [ESP+4] action), RET 4: finds the first
// node holding `action`; erases it (unless it is the head) and sets the run
// cursor to (track+108h, the node after it). The scratch iterator keeps
// pointing at the erased node. No match: nothing but the scratch walk.
void entity_track_remove_action_004567d0(Seg5Host& host, Object track, Object action);

// 004569A0 __fastcall(ECX list), RET: clears the list (frees every node) and
// tail-jumps to 004555F0, which clears again and frees the head (+4 = 0).
void action_list_destroy_004569a0(Seg5Host& host, Object list);
void action_list_tidy_004555f0(Seg5Host& host, Object list);

// 00459F40 __fastcall(ECX list), RET: clears a list whose node value
// carries a NativeString at node +0Ch {size} / +10h {data}: the data goes back
// to the sized pool (00419CC0, then 00BD1510 with size+1, 1) before the node
// is freed.
void named_list_clear_00459f40(Seg5Host& host, Object list);

// 00458050 __thiscall(ECX dest, [ESP+4] source), RET 4: copies a 38h-byte
// record: +0 dword, the NativeString at +4 (0041DD40 + memcpy, skipped when
// dest == source string), then the dwords +0Ch..+34h. Returns ECX.
Object record38_assign_00458050(Seg5Host& host, Object dest, Object source);

// 00456710 __fastcall(ECX place, EDX source), RET: when `place` is non-null,
// 00455880(ECX place, source) (the allocator's construct).
void list_value_construct_00456710(Seg5Host& host, Object place, Object source);

// 00458A70 __thiscall(ECX track, [ESP+4] float duration, [ESP+8] float
// movie time), RET 8 (cUnParent). With a parent track (+154h): the entity's
// vtable+ACh(0), +14Ch = 0, then a blend from the parent-relative pose back to
// the track's own path:
//   start (+164h) = 004581E0(parent, movie time) + the parent offset +1C8h,
//   +158h = movie time, start velocity (+174h, copied to +180h) =
//   200 * (004581E0(parent, float(movie time - 0.2)) - start);
//   t2 = duration + movie time (double), end (+19Ch) = 004581E0(track,
//   float(t2)), +190h = float(t2), end velocity (+1ACh, copied to +1B8h) =
//   200 * (004581E0(track, float(t2 - 0.2)) - end);
//   +150h = duration, the offset +1C8h..+1D0h = 0, +14Ch = 3.
// 0.2 is the double 0.20000000298023224 (00CE3D10), 200 the double 00CE4D70.
void entity_track_unparent_00458a70(Seg5Host& host, Object track, float duration, float movie_time);

// 00458D50 __thiscall(ECX track, float duration, parent track, const float*
// offset, float movie time), RET 10h (cSetParent). Refreshes the entity pose
// (00414DB0) when entity+C8h is clear, then blends from the entity's world
// position (entity +FCh..+104h, start +164h; +158h = movie time; start
// velocity from the track's own path at movie time - 0.2) to the parent path
// plus the offset at duration + movie time (end +19Ch, +190h, end velocity
// from the parent at that time - 0.2). Then +154h = parent, +150h = duration,
// +1C8h..+1D0h = offset, +14Ch = 1.
void entity_track_set_parent_00458d50(Seg5Host& host, Object track, float duration, Object parent,
                                      const float* offset, float movie_time);

// 0045B100 __fastcall(ECX track), RET (returns ECX): vtable 00CE4DB8; the
// +0F4h list (head from 00455520, size 0), iterator +100h/+104h = 0; the
// +108h list (head from 00455540, size 0), +114h/+118h = 0, name +120h/+124h
// = 0, cursor +140h/+144h = 0; 004658D0(0.0f) on +158h and on +190h; then
// cursor = (+108h, head), +14Ch = 0, +148h = 0.0f, and the 3Ch floats at
// +4..+0F0h = 0.0f.
Object entity_track_construct_0045b100(Seg5Host& host, Object track);

// 0045B220 __fastcall(ECX track), RET: the entity-track destructor body.
// vtable 00CE4DB8; 00459F40 on the +0F4h list; for every action in the +108h
// list (scratch iterator) a non-null action gets 00465610 and is freed; the
// +108h nodes are freed; the NativeStrings at +194h/+198h, +15Ch/+160h and
// the name +120h/+124h go back to the pool in that order; the +108h list is
// cleared again and its head freed (+10Ch = 0); 00459F40 on +0F4h again and
// its head freed (+0F8h = 0).
void entity_track_destroy_0045b220(Seg5Host& host, Object track);

// 0045BAD0 __thiscall(ECX track, [ESP+4] std::vector<float>* out), RET 4:
// appends the leading float (node +8) of every +0F4h sample to `out`
// ({+4 first, +8 last, +0Ch end}): in place when first is non-null and the
// size is below the capacity, otherwise through 0045A120(ECX out, &result,
// out, last, &value).
void entity_track_collect_sample_times_0045bad0(Seg5Host& host, Object track, Object out);

// 0045BD00 __fastcall(ECX track), RET (returns ECX): 0045B100, vtable
// 00CE4DCC, the +1DCh and +1E8h lists (heads from 004555D0, size 0), +1D4h = 0.
Object camera_track_construct_0045bd00(Seg5Host& host, Object track);

// 0045D8A0 __thiscall(ECX track, NativeString name by value {[ESP+4] size,
// [ESP+8] data}), RET 8: assigns the name to +120h (0041DD40 + memcpy of the
// new size), then destroys the by-value argument (pool release of a non-null
// data with size + 1).
void entity_track_set_name_0045d8a0(Seg5Host& host, Object track, std::uint32_t size, void* data);

// 0045EFB0 (ECX unused, NativeString name by value {[ESP+4] size, [ESP+8]
// data}), RET 8: builds "CameraPosition 01" (0041DD40(11h, 1), memcpy of
// size + 1 bytes from 00CE4DA4), compares it with the name through
// 00435C40(ECX literal, &name), releases the literal, then allocates 1F4h
// bytes and runs 0045BD00 (a match) or 1D4h bytes and 0045B100 (a null
// allocation yields null), then releases the argument's storage. Returns the
// track.
Object engine_movie_create_track_0045efb0(Seg5Host& host, std::uint32_t size, void* data);

// 0045DB40 __fastcall(ECX player), RET: +60h = 1; 004CD0F0(ECX [00E188A8],
// 0, 0, 1); an empty NativeString (0041DD40(0), memcpy of the "" at
// 00CE3A0C, size + 1 bytes) for 005B9BA0(ECX [[00E198C4]+A4h], 0.99f
// (00CE4E0C), -0.01f (00D7A300), &string), released; another for
// 005B9760(same ECX, 1.0f, 1.0f, &string), released; then
// 005CB160(ECX [[00E198C4]+A0h]).
void engine_movie_start_0045db40(Seg5Host& host, Object player);

// 00457160 __fastcall(ECX track), RET: track+1D4h = [[00E188A8]+19FCh], then
// tail-jumps to 00456DD0 (bind entities).
void entity_track_bind_00457160(Seg5Host& host, Object track);

// ---------------------------------------------------------------------------
// Camera track (an entity track with shake lists at +1DCh and +1E8h).
// ---------------------------------------------------------------------------

// 0045AF70 __thiscall(ECX track, time, x, y, z, amplitude, char camera),
// RET 18h: 0045A320(ECX track+1DCh when camera is non-zero, else track+1E8h,
// time, x, y, z, amplitude).
void camera_track_add_shake_0045af70(Seg5Host& host, Object track, float time, float x, float y,
                                     float z, float amplitude, std::uint8_t camera);

// 0045BD90 __thiscall(ECX track, float time), RET 4: 00459020(time), then
// 0045ACC0().
void camera_track_update_0045bd90(Seg5Host& host, Object track, float time);

// 0045BE40 __fastcall(ECX track), RET: destroys the lists at +1E8h and
// +1DCh (004569A0), then 0045B220 (the entity-track part).
void camera_track_destroy_0045be40(Seg5Host& host, Object track);

// ---------------------------------------------------------------------------
// Other bodies in the range.
// ---------------------------------------------------------------------------

// 0045C470 __fastcall(ECX game), RET: game+193Ch = 0, then tail-jumps to
// 004C3CB0 (BSP_Game_BuildLocalPlayerUnitLists) with the same ECX.
void game_rebuild_local_player_units_0045c470(Seg5Host& host, Object game);

// Win32 file writer (18h bytes: vtable, +4 byte, +8, +0Ch, +10h byte, +14h
// handle). 0045C510 __fastcall: closes a non-null handle through the import
// at 00CE225C and clears +14h. 0045CA40 (destructor body): vtable 00CE4E04,
// the same close, vtable 00CE4DFC, +14h = 0.
void win32_file_writer_close_handle_0045c510(Seg5Host& host, Object writer);
void win32_file_writer_destroy_0045ca40(Seg5Host& host, Object writer);

// 0045CAC0 (ECX unused, [ESP+4] NativeString path), RET 4: builds a writer
// on the stack (vtable 00CE4E04, +4 byte 0, +8 = 0, +0Ch = 2, +10h byte 20h,
// +14h = 0), opens path.data (or "" at 00E18511 when null) with 009501B0,
// closes the handle when the open succeeded, then runs the inlined
// destructor (vtable 00CE4E04, close a remaining handle).
void win32_touch_file_0045cac0(Seg5Host& host, Object path);

// ---------------------------------------------------------------------------
// Action run bodies.
// ---------------------------------------------------------------------------

void run_set_target_004650f0(Seg5Host& host, Object action, Object entity);
void run_set_parent_00464870(Seg5Host& host, Object action, Object entity);
void run_unparent_00464350(Seg5Host& host, Object action, Object entity);
void run_force_release_004648f0(Seg5Host& host, Object action, Object entity);
void run_force_depth_charge_00465180(Seg5Host& host, Object action, Object entity);
void run_set_gun_fire_00464910(Seg5Host& host, Object action, Object entity);
void run_toggle_periscope_004649b0(Object action, Object entity);
void run_force_artillery_angle_00464390(Object action, Object entity);
void run_force_torpedo_00465210(Seg5Host& host, Object action, Object entity);
void run_set_gun_fire_target_only_00465410(Seg5Host& host, Object action, Object entity);
void run_turn_gun_00465450(Seg5Host& host, Object action, Object entity);
void run_fov_004643d0(Object action, Object entity);
void run_fov_now_004643e0(Object action, Object entity);
void run_dont_move_004649f0(Object action, Object entity);
void run_camera_shake_00464ab0(Seg5Host& host, Object action, Object entity);
void run_binocular_00464c40(Seg5Host& host, Object action, Object entity);
void run_take_off_hack_00464d70(Object action, Object entity);
void run_wing_tip_effect_00464d80(Object action, Object entity);
void run_move_to_point_00464dc0(Object action, Object entity);
void run_wide_black_bars_00464e00(Seg5Host& host, Object action, Object entity);
void run_use_catapult_00464e50(Seg5Host& host, Object action, Object entity);
void run_hide_entity_00464ee0(Seg5Host& host, Object action, Object entity);

// ---------------------------------------------------------------------------
// Helpers in the same range.
// ---------------------------------------------------------------------------

// 004646B0 __fastcall(ECX gun), RET: 0072CF00(i, 0.0f, 0) for every barrel
// i < [gun+448h].
void gun_reset_all_barrel_reload_timers_004646b0(Seg5Host& host, Object gun);

// 00465000 / 00465020 / 00465040 / 00465060, __fastcall(ECX object), RET:
// the object when it is non-null and answers vtable+5Ch(kind) with true,
// else null. Kinds 24h, 20h, 5 and 8.
Object unit_cast_to_kind24_00465000(Seg5Host& host, Object object);
Object unit_cast_to_kind20_00465020(Seg5Host& host, Object object);
Object unit_cast_to_vehicle_00465040(Seg5Host& host, Object object);
Object unit_cast_to_submarine_00465060(Seg5Host& host, Object object);

// 00465080 and its identical twin 00464F70, __thiscall(ECX target, [ESP+4]
// entity, [ESP+8] float), RET 8. Fills the 18h-byte command target: +8..+10h
// from 00F87574, +0 word 0, +4 entity; with an entity +0 byte 1 and +2 word =
// entity+174h, else +0 byte 0 and +2 word 0; +14h the float. Returns ECX.
Object command_target_from_entity_00465080(Seg5Host& host, Object target, Object entity, float value);
Object command_target_from_entity_00464f70(Seg5Host& host, Object target, Object entity, float value);

// Session message IsKindOf answers, __thiscall (ECX unused), [ESP+4] type,
// RET 4.
bool session_message46_is_kind_of_00464500(std::int32_t type);
bool session_message49_is_kind_of_00464590(std::int32_t type);
bool session_message7b_is_kind_of_004647b0(std::int32_t type);

// 004645B0 / 004645D0, __thiscall(ECX message, [ESP+4] stream), RET 4: the
// extended header, then the bool at message+1Ch (written to the stream itself,
// read through the cursor at stream+4).
void session_bool_message_write_004645b0(Seg5Host& host, Object message, Object stream);
void session_bool_message_read_004645d0(Seg5Host& host, Object message, Object stream);

// 00464710 __thiscall(ECX message, [ESP+4] float), RET 4: 0075B430(7Bh),
// then +18h word 0, +1Ah byte 0, +1Ch byte 0, +4 = 1, vtable 00CE53AC,
// +20h = the float. Returns ECX.
Object session_message7b_construct_00464710(Seg5Host& host, Object message, float value);

} // namespace bsp::cc13_seg5
