#pragma once

// cc13_sweepB2: continuation of the segment 15-51 leftover sweep (packet
// cc13-sweepB2, docs/CC13_SWEEPB2.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB: native objects are raw byte blocks touched
// only at the offsets the image reads or writes, and every call the image
// makes goes through a named host method at the same point with the same
// arguments. Cc13SweepB2Host extends Cc13SweepBHost, so the sweepB hooks
// (image_global, global_00e188a8, the CRT hooks, ...) are reused as they are.

#include "bsp/cc13_sweepB.hpp"

#include <cstdint>

namespace bsp {

// The 8-byte {type, pointer} pair the Lua property readers take by value
// (type 0 with a C name, 4 = int destination, 7 = 4x4 frame destination).
struct Cc13SweepB2Descriptor {
    std::uint32_t type;
    const void* pointer;
};

class Cc13SweepB2Host : public Cc13SweepBHost {
public:
    // ---- CRT and Win32 ----
    // 00BF6713 _invalid_parameter_noinfo (checked-iterator failure; does not return in the image).
    virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
    // 00BF9280 strncpy (cdecl), returns dest.
    virtual char* crt_strncpy_00bf9280(char* dest, const char* source, std::uint32_t count) = 0;
    // 00BFAEEF wcsncpy (cdecl), returns dest.
    virtual wchar_t* crt_wcsncpy_00bfaeef(wchar_t* dest, const wchar_t* source, std::uint32_t count) = 0;
    // IAT 00CE2218 EnterCriticalSection / 00CE2210 LeaveCriticalSection (stdcall).
    virtual void enter_critical_section_00ce2218(void* section) = 0;
    virtual void leave_critical_section_00ce2210(void* section) = 0;

    // ---- online session (00F8A2FC) and Xenon system manager (00F8ABE8) ----
    // Online-session vtable slot +198h, no stack arguments: AL = online.
    virtual bool online_session_vcall_198_is_online(void* online) = 0;
    // Online-session vtable slot +80h, no stack arguments: the member list
    // record (+4h list head node, +8h count).
    virtual void* online_session_vcall_80_members(void* online) = 0;
    // 00A3E510 BSP_XenonSystemManager_HasSelectedUser, ECX = manager, AL.
    virtual bool xenon_has_selected_user_00a3e510(void* manager) = 0;
    // 00A3EAE0 BSP_XenonSystemManager_GetSelectedUsername, ECX = manager, RET, EAX = char*.
    virtual const char* xenon_selected_username_00a3eae0(void* manager) = 0;

    // ---- multiplayer session ----
    // 0076F8A0 (first peer of the session peer list or null), ECX = list record.
    virtual void* session_peers_front_0076f8a0(void* peers) = 0;
    // 0042BE90 BSP_Math_AbsFloatByRef_Provisional, ECX = float*, ST0 = |*p|.
    virtual float math_abs_float_by_ref_0042be90(const float* value) = 0;
    // 00574050 BSP_SessionFilter_ReadRankSetting, ECX = game+650h, AL.
    virtual std::uint8_t session_filter_read_rank_00574050(void* filter) = 0;
    // 008D4870 BSP_GameSettings_GetLanguageName, ECX = 00F88980, RET, EAX = char*.
    virtual const char* game_settings_language_name_008d4870(void* settings) = 0;
    // Object vtable slot +20h with one stack argument (here the 98h host-info blob).
    virtual void object_vcall_20_fill(void* object, void* blob) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(message, byte type), RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::uint8_t type) = 0;
    // 00783C80 BSP_SessionTransport_SerializeMessage, __thiscall(transport, peer, message), RET 8.
    virtual void transport_serialize_message_00783c80(void* transport, void* peer, void* message) = 0;
    // 00783490 BSP_SessionTransport_FlushTargetBuffers, __thiscall(transport, peer), RET 4.
    virtual void transport_flush_target_buffers_00783490(void* transport, void* peer) = 0;
    // Transport vtable slot +2Ch(peer, const void* data, int size): raw send.
    virtual void transport_vcall_2c_send_raw(void* transport, void* peer, const void* data,
                                             std::int32_t size) = 0;

    // ---- session message queues (00776230) ----
    // Session handlers, __thiscall(session)(peer), RET 4: 00774AB0 OnPeerAdded, 007752D0
    // ServerAddPlayer, 007717A0 HostRefreshScene, 007702E0 HostRemovePeerPlayer, 00772EC0 OnPeerRemoved.
    virtual void session_on_peer_added_00774ab0(void* session, void* peer) = 0;
    virtual void session_server_add_player_007752d0(void* session, void* peer) = 0;
    virtual void session_host_refresh_scene_007717a0(void* session, void* peer) = 0;
    virtual void session_host_remove_peer_player_007702e0(void* session, void* peer) = 0;
    virtual void session_on_peer_removed_00772ec0(void* session, void* peer) = 0;
    // 0076E850 (std::list clear), ECX = list.
    virtual void stl_list_clear_0076e850(void* list) = 0;
    // Timer object [01090AB0] vtable slot +20h(out): EAX = {int64 counter, int64 frequency}.
    virtual const std::int64_t* timer_vcall_20_read(void* timer, void* out) = 0;
    // 00784C40 BSP_MNetworkServer_KickTimedOutPeers / 007850E0 BSP_MNetworkClient_CheckServerSilence,
    // __thiscall(net, float seconds), RET 4.
    virtual void network_server_kick_timed_out_00784c40(void* server, float seconds) = 0;
    virtual void network_client_check_server_silence_007850e0(void* client, float seconds) = 0;

    // ---- native wide strings ({u32 length, wchar_t* data}) ----
    // 004C53E0 BSP_WideString_Resize, __thiscall(string, length, int preserve), RET 8.
    virtual void wide_string_resize_004c53e0(void* string, std::uint32_t length, std::int32_t preserve) = 0;
    // 004C8DD0 BSP_WideString_CopyConstruct, __thiscall(dest, const source*), RET 4, EAX = dest.
    virtual void* wide_string_copy_construct_004c8dd0(void* dest, const void* source) = 0;

    // ---- ocean water particles ----
    // 00BA43B0 BSP_OceanWaterParticles_Construct, __thiscall(block 11Ch, const refptr*), RET 4, EAX.
    virtual void* ocean_water_particles_construct_00ba43b0(void* block, void* const* source) = 0;
    // 0078D940 BSP_OceanWaterParticles_AssignSettings, __thiscall(particles, settings), RET 4.
    virtual void ocean_water_particles_assign_settings_0078d940(void* particles, void* settings) = 0;
    // 00BA44F0 BSP_OceanWaterParticles_Initialize, ECX = particles.
    virtual void ocean_water_particles_initialize_00ba44f0(void* particles) = 0;
    // 00BA3A60 (ocean beams construct), __thiscall(block FCh)(const refptr* source, const refptr* env), RET 8, EAX.
    virtual void* ocean_beams_construct_00ba3a60(void* block, void* const* source_ref, void* const* env_ref) = 0;
    // 0078D9F0 (ocean beams assign settings), __thiscall(beams, settings), RET 4.
    virtual void ocean_beams_assign_settings_0078d9f0(void* beams, void* settings) = 0;
    // 00BA3BD0 (ocean beams initialise), ECX = beams.
    virtual void ocean_beams_initialize_00ba3bd0(void* beams) = 0;

    // ---- matrices and vectors ----
    // 0042D7E0 BSP_EntityPose_GetWorldMatrix, ECX = entity, EAX = const float[16].
    virtual const float* entity_pose_world_matrix_0042d7e0(void* entity) = 0;
    // 00414E10 BSP_EntityPose_GetDerivedAffineInverse, ECX = entity, EAX = const float[16].
    virtual const float* entity_pose_derived_affine_inverse_00414e10(void* entity) = 0;
    // 00414D10 BSP_Vector3f_TransformAffinePointViaScratch: ECX out, EDX point, stack matrix, RET 4, EAX = out.
    virtual float* vector3_transform_affine_point_via_scratch_00414d10(float* out, const float* point,
                                                                      const float* matrix) = 0;
    // 00427C90 BSP_Vector3f_SubtractTo, __thiscall(ECX a)(out, b), RET 8, EAX = out (a - b).
    virtual float* vector3_subtract_to_00427c90(const float* a, float* out, const float* b) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(ECX dest)(source), RET 4.
    virtual void matrix_copy_4x4_004134f0(float* dest, const float* source) = 0;

    // ---- movie camera ----
    // 0079F710 BSP_MovCam_GetVehicleCameraPose, __thiscall(camera)(float out[2], entity, index,
    // bool* inside), RET 10h.
    virtual void movcam_vehicle_camera_pose_0079f710(void* camera, float* out, void* entity, std::int32_t index,
                                                     std::uint8_t* inside) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange: ECX stream word, stack (lo, hi), RET 8, ST0.
    virtual float random_uniform_float_range_00bd2f10(std::int32_t ecx_stream, float lo, float hi) = 0;
    // 007A4590 (map subscript over the class camera lists), __thiscall(map = cameras+0Ch)(key), RET 4,
    // EAX = the list {int* data, int count}.
    virtual void* map_subscript_007a4590(void* map, void* key) = 0;
    // 00468050 BSP_PodPointerArray_Resize, __thiscall(array, count), RET 4.
    virtual void pod_pointer_array_resize_00468050(void* array, std::int32_t count) = 0;
    // 007A6710 BSP_MovCam_EnterVehicleCamera, __thiscall(camera, index), RET 4 (also reconstructed below).
    virtual void movcam_enter_vehicle_camera_007a6710(void* camera, std::int32_t index) = 0;
    // 007A4EF0 BSP_MovCam_SeedAnglesFromVehicleCamera, __thiscall(camera, target), RET 4.
    virtual void movcam_seed_angles_007a4ef0(void* camera, void* target) = 0;
    // 005CD640 BSP_VehicleCameraArray_Resize, __thiscall(array, count), RET 4.
    virtual void vehicle_camera_array_resize_005cd640(void* array, std::int32_t count) = 0;
    // 007A4E10 (map subscript), __thiscall(map, const key*), RET 4, EAX = the mapped record
    // (a vector of float pairs: +4h begin, +8h end).
    virtual void* map_subscript_007a4e10(void* map, void* const* key) = 0;
    // 007A0AC0 (map subscript), __thiscall(map, const key*), RET 4, EAX = int* mapped value.
    virtual std::int32_t* map_subscript_007a0ac0(void* map, void* const* key) = 0;
    // 005CC5E0 BSP_MovCam_ShowModeText, __thiscall(hud, NativeString by value), RET 8; consumes it.
    virtual void movcam_show_mode_text_005cc5e0(void* hud, Cc13SweepBNativeString text) = 0;
    // Object vtable slot +0Ch, no stack arguments.
    virtual void object_vcall_0c(void* object) = 0;
    // 004222C0 (vector of float pairs push_back), __thiscall(vector)(const float pair[2]*), RET 4.
    virtual void float_pair_vector_push_back_004222c0(void* vector, const float* pair) = 0;
    // 0042D2E0 BSP_Matrix_ExtractNormalizedBasisAngles: ECX matrix, EDX float* a, stack (float* b,
    // float* c), RET 8.
    virtual void matrix_extract_basis_angles_0042d2e0(const float* matrix, float* a, float* b, float* c) = 0;
    // 0079C830 BSP_MovCam_ClearPathRecords, ECX = camera.
    virtual void movcam_clear_path_records_0079c830(void* camera) = 0;
    // 0079EB10 (pointer vector resize), __thiscall(vector, count, value), RET 8.
    virtual void pointer_vector_resize_0079eb10(void* vector, std::uint32_t count, void* value) = 0;
    // 005CD160 BSP_MovCam_EnsureTarget, ECX = holder ([[00E198C4]+98h]), EAX = movie camera.
    virtual void* movcam_ensure_target_005cd160(void* holder) = 0;
    // 007A5A00 BSP_MovCam_BeginPathWalk, __thiscall(camera)(float, byte, float,
    // const NativeString*, int), RET 14h.
    virtual void movcam_begin_path_walk_007a5a00(void* camera, float a, std::uint8_t b, float c,
                                                 const Cc13SweepBNativeString* name, std::int32_t d) = 0;

    // ---- HUD markers ----
    // 00B6FCB0 BSP_Camera_GetViewMatrix, ECX = camera, EAX = const float[16].
    virtual const float* camera_view_matrix_00b6fcb0(void* camera) = 0;
    // 00BF701A _CIatan2: ST1 = y, ST0 = x in, ST0 = atan2(y, x) out.
    virtual double crt_atan2_00bf701a(double y, double x) = 0;
    // 00B6E0D0 BSP_Node_GetInverseWorldMatrix, ECX = node, EAX = const float[16].
    virtual const float* node_inverse_world_matrix_00b6e0d0(void* node) = 0;
    // 0042B260 BSP_Geometry_NormalizeVectorWithFloor, ECX = float[3] (normalised in place).
    virtual void geometry_normalize_with_floor_0042b260(float* xyz) = 0;
    // 0042CF10 BSP_Geometry_AsinClamped(float), stdcall RET 4, ST0.
    virtual float geometry_asin_clamped_0042cf10(float value) = 0;

    // ---- unit view cameras ----
    // 00BF79F0 memset (cdecl), returns dest.
    virtual void* crt_memset_00bf79f0(void* dest, std::int32_t value, std::uint32_t count) = 0;
    // 00519290 BSP_ShipCamera_ConstructBase, ECX = block, EAX = block.
    virtual void* ship_camera_construct_base_00519290(void* block) = 0;
    // 00923870 BSP_EntityPlaceAndCommit, __thiscall(entity)(a, b, const float matrix[16]), RET 0Ch.
    virtual void entity_place_and_commit_00923870(void* entity, std::uint32_t a, std::uint32_t b,
                                                  const float* matrix) = 0;
    // 00694A60 BSP_Observer_RegisterPair: ECX = endpoint, EDX = callback owner, RET.
    virtual void observer_register_pair_00694a60(void* endpoint, void* owner) = 0;
    // 0042F6C0 BSP_SubmarineCamera_Setup, __thiscall(camera)(target, node, const float offset[3]), RET 0Ch.
    virtual void submarine_camera_setup_0042f6c0(void* camera, void* target, void* node, const float* offset) = 0;
    // 004BC410 BSP_Game_SetCameraMover, __thiscall(game)(mover), RET 4.
    virtual void game_set_camera_mover_004bc410(void* game, void* mover) = 0;

    // ---- GUI widgets ----
    // Widget vtable slot +38h, no stack arguments, AL (shown).
    virtual bool widget_vcall_38_is_shown(void* widget) = 0;
    // 00AA6740 BSP_GuiWidget_GetSize, ECX = widget, EAX = const float[2].
    virtual const float* widget_get_size_00aa6740(void* widget) = 0;
    // 00AA78D0 BSP_GuiWidget_SetLocalX, __thiscall(widget, float), RET 4.
    virtual void widget_set_local_x_00aa78d0(void* widget, float x) = 0;

    // ---- in-game interface screens ----
    // 004F7180 BSP_FrontEndScreen_Construct, ECX = screen.
    virtual void frontend_screen_construct_004f7180(void* screen) = 0;
    // 0093CB10 BSP_Simple3dScene_Construct, ECX = scene.
    virtual void simple3d_scene_construct_0093cb10(void* scene) = 0;
    // STL head-node allocators (instantiations, called as they are), ECX = container, EAX = node:
    // 005A0220 (tree, isnil byte +11h), 005356C0 (list), 005359A0 and 005A0270 (trees, isnil +15h).
    virtual void* stl_buy_head_node_005a0220(void* container) = 0;
    virtual void* stl_buy_head_node_005356c0(void* container) = 0;
    virtual void* stl_buy_head_node_005359a0(void* container) = 0;
    virtual void* stl_buy_head_node_005a0270(void* container) = 0;

    // 004F71D0 BSP_FrontEndScreen_Register, ECX = screen.
    virtual void frontend_screen_register_004f71d0(void* screen) = 0;
    // 00AA5840 BSP_GuiManager_LoadPage, __thiscall(manager)(const NativeString*, int, int), RET 0Ch, EAX = page.
    virtual void* gui_manager_load_page_00aa5840(void* manager, const void* name, std::int32_t a,
                                                 std::int32_t b) = 0;

    // ---- main menu (00588A80) ----
    // 00A9C990 BSP_GuiListbox_GetSelectedData, ECX = list box, EAX.
    virtual std::int32_t listbox_selected_data_00a9c990(void* listbox) = 0;
    // 00584F50 BSP_MainMenu_BuildSinglePlayerPage / 005853C0 (multiplayer page) /
    // 005886F0 (tactical-library page), ECX = screen.
    virtual void main_menu_build_single_player_page_00584f50(void* screen) = 0;
    virtual void main_menu_build_multiplayer_page_005853c0(void* screen) = 0;
    virtual void main_menu_build_tactical_library_page_005886f0(void* screen) = 0;
    // 004D7920 BSP_Game_RequestState, __thiscall(game, int), RET 4.
    virtual void game_request_state_004d7920(void* game, std::int32_t state) = 0;
    // 006B8AD0 BSP_LuaMachine_RunString, __thiscall(machine)(const char*, int, int, int), RET 10h.
    virtual void lua_machine_run_string_006b8ad0(void* machine, const char* text, std::int32_t a, std::int32_t b,
                                                 std::int32_t c) = 0;
    // 00A4D44C XShowGuideUI(user) (import thunk, stdcall); result unused.
    virtual std::uint32_t x_show_guide_ui_00a4d44c(std::uint32_t user) = 0;
    // 00A4D446 XShowMarketplaceUI(user, entry, u64 offer, categories) (import thunk, stdcall); result unused.
    virtual std::uint32_t x_show_marketplace_ui_00a4d446(std::uint32_t user, std::uint32_t entry,
                                                         std::uint64_t offer, std::uint32_t categories) = 0;
    // 00585B40 BSP_MainMenu_CheckConnectionGate (no arguments), AL.
    virtual bool main_menu_check_connection_gate_00585b40() = 0;
    // 00585810 BSP_MainMenu_CheckAccountGate: CL = flag byte, AL.
    virtual bool main_menu_check_account_gate_00585810(std::uint8_t flag) = 0;
    // 00A3EAC0 (trivial Xenon manager read), ECX = manager, EAX (the user index passed on).
    virtual std::uint32_t xenon_trivial_00a3eac0(void* manager) = 0;
    // 00425D10 BSP_MenuCommandScreen_GetSingleton (cdecl), EAX.
    virtual void* menu_command_screen_get_00425d10() = 0;
    // 00531B00 BSP_MenuPromptScreen_Raise, __thiscall(menu)(int, const NativeString*, int,
    // callback image address, int, const NativeString*, float, int, NativeString by value, int), RET 2Ch.
    virtual void menu_prompt_raise_00531b00(void* menu, std::int32_t a, const void* title, std::int32_t b,
                                            std::uint32_t callback, std::int32_t c, const void* text, float d,
                                            std::int32_t e, Cc13SweepBNativeString owned, std::int32_t f) = 0;
    // 00530C20 BSP_FrontEndPrompt_FocusLastNavigation, ECX = screen.
    virtual void frontend_prompt_focus_last_navigation_00530c20(void* screen) = 0;
    // 0041DD20 BSP_NativeString_DestroyStorage, ECX = string.
    virtual void native_string_destroy_0041dd20(void* string) = 0;

    // ---- LuaObject (14h-byte value objects) and the input settings ----
    // 00B67980 (globals table), __thiscall(machine)(LuaObject* out), RET 4, EAX = out.
    virtual void* lua_globals_00b67980(void* machine, void* out) = 0;
    // 00B67800 (field by C name), __thiscall(table)(LuaObject* out, const char*), RET 8, EAX = out.
    virtual void* lua_object_get_field_00b67800(void* table, void* out, const char* key) = 0;
    // 00B68100 (field by native string), __thiscall(table)(LuaObject* out, const NativeString*), RET 8.
    virtual void* lua_object_get_field_by_string_00b68100(void* table, void* out, const void* key) = 0;
    // 00B67350 BSP_LuaObject_SetNil (field by native string), __thiscall(table)(const NativeString*), RET 4.
    virtual void lua_object_set_nil_00b67350(void* table, const void* key) = 0;
    // 00B662B0 BSP_LuaObject_GetString, ECX = object, EAX = const char*.
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    // 00B67700 (LuaObject destructor), ECX = object.
    virtual void lua_object_destroy_00b67700(void* object) = 0;
    // 00B66FA0 BSP_LuaObject_CopyConstruct, __thiscall(dest)(const source*), RET 4.
    virtual void lua_object_copy_construct_00b66fa0(void* dest, const void* source) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(reader)(LuaObject by value, 14h bytes), RET 14h,
    // EAX = reader. The image builds the by-value object in the argument area and the callee owns it;
    // here it is built in `object` and the hook places it.
    virtual void* lua_reader_construct_004425c0(void* reader, void* object) = 0;
    // 006ABA50 BSP_InputSettings_ReadKeyboardArchive, __thiscall(settings)(reader), RET 4.
    virtual void input_settings_read_keyboard_archive_006aba50(void* settings, void* reader) = 0;
    // 00441210 BSP_LuaObjectVector_Tidy, ECX = vector.
    virtual void lua_object_vector_tidy_00441210(void* vector) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(entity)(LuaObject* out), RET 4. In
    // 006AF780 `out` is the by-value argument area of the following 004425C0 call.
    virtual void entity_saved_lua_data_009238a0(void* entity, void* out) = 0;
    // LuaReader vtable slot +10h(key descriptor, value descriptor), both by value, RET 10h.
    virtual void lua_reader_vcall_10_read(void* reader, Cc13SweepB2Descriptor key, Cc13SweepB2Descriptor value) = 0;
    // LuaReader vtable slot +4h(int kind, const char* key): enter a table.
    virtual void lua_reader_vcall_04_enter(void* reader, std::int32_t kind, const char* key) = 0;
    // LuaReader vtable slot +14h(key descriptor by value), AL = the key exists.
    virtual bool lua_reader_vcall_14_has(void* reader, Cc13SweepB2Descriptor key) = 0;
    // LuaReader vtable slot +8h, no stack arguments: leave the table.
    virtual void lua_reader_vcall_08_leave(void* reader) = 0;
    // 004954F0 (checked NativeString vector erase range), __thiscall(vector)(iterator* result,
    // iterator first {container, pointer}, iterator last {container, pointer}), RET 14h.
    virtual void string_vector_erase_range_004954f0(void* vector, void* result, void* first_container,
                                                    void* first, void* last_container, void* last) = 0;
    // 00450540 (checked NativeString vector append), __thiscall(vector)(const NativeString*), RET 4.
    virtual void string_vector_append_00450540(void* vector, const void* value) = 0;
    // 005C3470 BSP_MissionTreeScreen_FindMissionByName, __thiscall(screen)(int out[2], const NativeString*), RET 8.
    virtual void mission_tree_find_mission_by_name_005c3470(void* screen, std::int32_t* out, const void* name) = 0;
    // 007FC4C0 BSP_Profile_IsUnlockSatisfied, __thiscall(profile)(const NativeString*), RET 4, AL.
    virtual bool profile_is_unlock_satisfied_007fc4c0(void* profile, const void* name) = 0;
    // 004D0640 BSP_NativeStringSet_Insert, __thiscall(set)(iterator* result, const NativeString*), RET 8.
    virtual void native_string_set_insert_004d0640(void* set, void* result, const void* name) = 0;
    // 00432050 (NativeString range destroy): ECX first, EDX last, two stack words, RET 8.
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* vector, std::uint32_t word) = 0;
    // 00BD6830 BSP_LuaReader_ReadField, __thiscall(reader)(name descriptor, field descriptor), RET 10h.
    virtual void lua_reader_read_field_00bd6830(void* reader, Cc13SweepB2Descriptor name,
                                                Cc13SweepB2Descriptor field) = 0;
    // Entity vtable slot +98h(parent id, int, const float* frame).
    virtual void entity_vcall_98_restore_parent(void* entity, std::uint32_t parent_id, std::uint32_t b,
                                                const float* frame) = 0;

    // 00557BC0 (checked NativeString vector copy-construct), __thiscall(dest)(const source*), RET 4.
    virtual void string_vector_copy_construct_00557bc0(void* dest, const void* source) = 0;
    // 00506C80 BSP_StringVector_Assign, __thiscall(dest vector)(const source vector*), RET 4.
    virtual void string_vector_assign_00506c80(void* dest, const void* source) = 0;

    // ---- session browser (screen 00CEF120 listener at screen+8h) ----
    // 00A9BE00 (list-box row widget at index), __thiscall(listbox)(int), RET 4, EAX.
    virtual void* listbox_row_00a9be00(void* listbox, std::int32_t index) = 0;
    // 00A9C920 BSP_GuiListbox_GetSelectedIndex, ECX = listbox, EAX.
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    // 0056AA70 BSP_MultiSessionBrowserScreen_FocusList, __thiscall(screen)(list), RET 4.
    virtual void session_browser_focus_list_0056aa70(void* screen, std::uint32_t list) = 0;
    // 00577650 BSP_MultiSessionBrowserScreen_RefreshRowDetails, ECX = screen.
    virtual void session_browser_refresh_row_details_00577650(void* screen) = 0;
    // 0056B610 BSP_MultiSessionBrowserScreen_MoveSelection, __thiscall(screen)(void*, int, int, int), RET 10h.
    virtual void session_browser_move_selection_0056b610(void* screen, void* a, std::uint32_t b, std::uint32_t c,
                                                         std::uint32_t d) = 0;

    // Widget vtable slot +60h(int): 1 = row fully inside the clip, 0 = not.
    virtual void widget_vcall_60(void* widget, std::int32_t value) = 0;
    // 00425E50 (list box: the selected row widget), ECX = listbox, EAX.
    virtual void* listbox_selected_row_00425e50(void* listbox) = 0;
    // 00683680 BSP_GuiScroller_ScrollTowardStart / 00683700 ..TowardEnd, __thiscall(scroller, float), RET 4.
    virtual void gui_scroller_scroll_toward_start_00683680(void* scroller, float delta) = 0;
    virtual void gui_scroller_scroll_toward_end_00683700(void* scroller, float delta) = 0;
    // 00A9C8F0 (list box select index), __thiscall(listbox, int), RET 4.
    virtual void listbox_select_00a9c8f0(void* listbox, std::int32_t index) = 0;

    // ---- HUD marker screen (4Dh) ----
    // 004B4B00 BSP_Game_GetControlledVehicle (no stack arguments), EAX.
    virtual void* game_controlled_vehicle_004b4b00() = 0;
    // 00470470 BSP_HitRecord_Reset / 004704B0 BSP_HitRecord_Destroy, ECX = record.
    virtual void hit_record_reset_00470470(void* record) = 0;
    virtual void hit_record_destroy_004704b0(void* record) = 0;
    // 00852860 BSP_Entity_IsBelowAltitudeThird, ECX = entity, AL.
    virtual bool entity_is_below_altitude_third_00852860(void* entity) = 0;
    // 00638E50 BSP_HudMarker_ProjectAndClip: ECX const float world[3], EDX float out[3], AL.
    virtual bool hud_marker_project_and_clip_00638e50(const float* world, float* screen) = 0;
    // 006430C0 BSP_InGameHudMarkers_AddUnitMarker, __thiscall(markers)(unit, byte, int, int), RET 10h.
    virtual void hud_markers_add_unit_marker_006430c0(void* markers, void* unit, std::uint8_t force,
                                                      std::int32_t kind_a, std::int32_t kind_b) = 0;
    // 00638C60 BSP_HudMarker_ProjectOffscreenArrow (reconstructed above), __fastcall(world, x*)(y*, angle*), RET 8.
    virtual void hud_marker_project_offscreen_arrow_00638c60(const float* world, float* x, float* y,
                                                             float* angle) = 0;
    // 005224C0 (squadron member index lookup), __thiscall([[00E198C4]+CCh])(squadron), RET 4, EAX.
    virtual std::int32_t squadron_selected_member_005224c0(void* owner, void* squadron) = 0;
    // 007B8AD0 BSP_Unit_IsSquadronFlightLeader, ECX = unit, AL.
    virtual bool unit_is_squadron_flight_leader_007b8ad0(void* unit) = 0;
    // 00642040 BSP_InGameHudMarkers_BuildUnitMarker, __thiscall(markers)(unit, byte, int, int, byte, int), RET 18h.
    virtual void hud_markers_build_unit_marker_00642040(void* markers, void* unit, std::uint8_t a, std::uint32_t b,
                                                        std::uint32_t c, std::uint8_t d, std::int32_t e) = 0;
    // 0043DF90 BSP_HudObserverSet_Construct_0043df90, __thiscall(block 1Ch)(a, b), RET 8, EAX = block.
    virtual void* hud_observer_set_construct_0043df90(void* block, void* a, void* b) = 0;
    // 0064AE20 BSP_HudShipScreen45_ApplyUnit, ECX = screen.
    virtual void hud_ship_screen45_apply_unit_0064ae20(void* screen) = 0;
    // ---- lead marker (00640A20) ----
    // 00927F10 BSP_PartySlot_IsAiHeld(slot), stdcall RET 4, AL.
    virtual bool party_slot_is_ai_held_00927f10(std::int32_t slot) = 0;
    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed, ECX = entity, EAX = const float[3].
    virtual const float* entity_world_position_00427eb0(void* entity) = 0;
    // 00427E30 BSP_Vector3f_LengthSquared, ECX = float[3], ST0.
    virtual float vector3_length_squared_00427e30(const float* xyz) = 0;
    // Unit vtable slot +34h(float out[3]), RET 4, EAX = out (the unit's velocity).
    virtual const float* unit_vcall_34_velocity(void* unit, float* out) = 0;
    // 00901C20 BSP_GunBot_InterceptSolution: ECX shooter position, EDX target, stack (float speed,
    // const float* shooter velocity, float* out aim point, float* out distance), RET 10h.
    virtual void gunbot_intercept_solution_00901c20(const float* shooter, void* target, float speed,
                                                    const float* shooter_velocity, float* aim, float* distance) = 0;
    // 00637790 (pointer vector size), ECX = vector, EAX.
    virtual std::uint32_t pointer_vector_size_00637790(void* vector) = 0;
    // 006377B0 (pointer vector element reference), __thiscall(vector, index), RET 4, EAX = T**.
    virtual void** pointer_vector_at_006377b0(void* vector, std::uint32_t index) = 0;
    // 00640340 (pointer vector push_back), __thiscall(vector, T* const*), RET 4.
    virtual void pointer_vector_push_back_00640340(void* vector, void* const* value) = 0;
    // 0063E950 BSP_HudBefogoMarker_Construct, __thiscall(block 8)(parent), RET 4, EAX = block.
    virtual void* hud_befogo_marker_construct_0063e950(void* block, void* parent) = 0;
    // Widget vtable slot +4Ch(float): the marker alpha.
    virtual void widget_vcall_4c_set_alpha(void* widget, float alpha) = 0;
    // 00640860 BSP_InGameHudMarkers_AcquireObjectiveMarker, ECX = markers, EAX = marker.
    virtual void* hud_markers_acquire_objective_marker_00640860(void* markers) = 0;
    // 00639230 BSP_HudColourIconSet_ShowForKind, __thiscall(set)(x, y, angle, objective,
    // record, int flag, int), RET 1Ch.
    virtual void hud_icon_set_show_for_kind_00639230(void* set, float x, float y, float angle, void* objective,
                                                     void* record, std::int32_t flag, std::uint32_t extra) = 0;

    // ---- shadows ----
    // 00B7AAB0 BSP_Light_GetShadowMapOwner, ECX = light, EAX = light+174h.
    virtual void* light_shadow_map_owner_00b7aab0(void* light) = 0;
    // 00A8E860 BSP_DirectionalShadowOwner_RenderGroundOrthoView, __thiscall(owner)(camera,
    // const float min[3], const float max[3]), RET 0Ch.
    virtual void shadow_owner_render_ground_ortho_00a8e860(void* owner, void* camera, const float* min_xyz,
                                                           const float* max_xyz) = 0;
};

// ---- segment 18 ----

// 0056B610 BSP_MultiSessionBrowserScreen_MoveSelection(screen, scroller, listbox, clip, byte keep).
void cc13_sweepB2_session_browser_move_selection_0056b610(void* screen, void* scroller, void* listbox, void* clip,
                                                          std::uint8_t keep, Cc13SweepB2Host& host);

// ---- segment 19 ----

// 005787E0 BSP_MultiSessionBrowserScreen_OnListboxSelectionChanged(listener = screen+8h, a1, list).
void cc13_sweepB2_session_browser_on_listbox_selection_changed_005787e0(void* listener, std::uint32_t unused,
                                                                        std::uint32_t list, Cc13SweepB2Host& host);

// ---- segment 20 ----

// 00588A80 BSP_MainMenu_ActivateTopLevelItem(screen).
void cc13_sweepB2_main_menu_activate_top_level_item_00588a80(void* screen, Cc13SweepB2Host& host);

// 005AAEF0 BSP_InGameInterfaceScreen_Construct_005aaef0(screen): returns screen.
void* cc13_sweepB2_ingame_interface_screen_construct_005aaef0(void* screen, Cc13SweepB2Host& host);

// 005C5470 BSP_MissionRecordHeader_Assign(dest, source): returns dest.
void* cc13_sweepB2_mission_record_header_assign_005c5470(void* dest, const void* source, Cc13SweepB2Host& host);

// 005C51F0 BSP_MissionSideBlock_CopyConstruct(dest, source): returns dest.
void* cc13_sweepB2_mission_side_block_copy_construct_005c51f0(void* dest, const void* source, Cc13SweepB2Host& host);

// 005C5860 BSP_MissionTree_ReadStringVector(reader, kind, key, out).
void cc13_sweepB2_mission_tree_read_string_vector_005c5860(void* reader, std::int32_t kind, const char* key,
                                                           void* out, Cc13SweepB2Host& host);

// 005C5990 BSP_MissionTreeScreen_CollectMissingPrerequisites(screen, name).
void cc13_sweepB2_mission_tree_collect_missing_prerequisites_005c5990(void* screen, const void* name,
                                                                      Cc13SweepB2Host& host);

// 005A4020 BSP_HudMapScreen_SetNodeVisibleByName(screen, name by value (consumed), show).
void cc13_sweepB2_hud_map_set_node_visible_by_name_005a4020(void* screen, Cc13SweepBNativeString name, bool show,
                                                            Cc13SweepB2Host& host);

// 0059D7B0 BSP_InterfaceView_DrawGroundOrtho(view = [00E198C4]+54h).
void cc13_sweepB2_interface_view_draw_ground_ortho_0059d7b0(void* view, Cc13SweepB2Host& host);

// ---- segment 21 ----

// 005CCE70 BSP_HudEngineMovieScreen_Register(screen).
void cc13_sweepB2_hud_engine_movie_screen_register_005cce70(void* screen, Cc13SweepB2Host& host);

// ---- segment 22 ----

// 005D20A0 BSP_MultiErrorScreen_LayoutButtons(screen).
void cc13_sweepB2_multi_error_layout_buttons_005d20a0(void* screen, Cc13SweepB2Host& host);

// ---- segment 26 ----

// 00638C60 BSP_HudMarker_ProjectOffscreenArrow(world, x_out, y_out, angle_out).
void cc13_sweepB2_hud_marker_project_offscreen_arrow_00638c60(const float* world, float* x_out, float* y_out,
                                                             float* angle_out, Cc13SweepB2Host& host);

// The three floats passed by value (the image may rewrite its copy).
struct Cc13SweepB2Vec3 {
    float x;
    float y;
    float z;
};

// 00640A20 BSP_InGameHudMarkers_UpdateLeadMarker(markers, target).
void cc13_sweepB2_hud_markers_update_lead_marker_00640a20(void* markers, void* target, Cc13SweepB2Host& host);

// 00642960 BSP_InGameHudMarkers_AddSquadronMemberMarkers(markers, squadron, flag, a3, a4, a5).
void cc13_sweepB2_hud_markers_add_squadron_member_markers_00642960(void* markers, void* squadron, std::uint8_t flag,
                                                                   std::uint32_t a3, std::uint32_t a4,
                                                                   std::uint32_t a5, Cc13SweepB2Host& host);

// 00643360 BSP_InGameHudMarkers_AddObjectiveMarker(markers, position, objective, record).
void cc13_sweepB2_hud_markers_add_objective_marker_00643360(void* markers, Cc13SweepB2Vec3 position,
                                                            void* objective, void* record, Cc13SweepB2Host& host);

// 0064B370 BSP_HudShipScreen45_Activate(screen).
void cc13_sweepB2_hud_ship_screen45_activate_0064b370(void* screen, Cc13SweepB2Host& host);

// 00651370 BSP_HudUnitView_EnsureSubmarineCamera(view).
void cc13_sweepB2_hud_unit_view_ensure_submarine_camera_00651370(void* view, Cc13SweepB2Host& host);

// 0064B170 BSP_HudDamageDirection_Register(widget, world_point).
void cc13_sweepB2_hud_damage_direction_register_0064b170(void* widget, const float* world_point,
                                                         Cc13SweepB2Host& host);

// ---- segment 29 ----

// 006AC030 BSP_InputSettings_LoadForCurrentProfile(settings).
void cc13_sweepB2_input_settings_load_for_current_profile_006ac030(void* settings, Cc13SweepB2Host& host);

// 006AD600 BSP_Game_ClearMissionLuaGlobals().
void cc13_sweepB2_game_clear_mission_lua_globals_006ad600(Cc13SweepB2Host& host);

// 006AF780 BSP_SceneEntity_RestoreFromSaveData(entity) (ECX unused).
void cc13_sweepB2_scene_entity_restore_from_save_data_006af780(void* entity, Cc13SweepB2Host& host);

// ---- segment 43 ----

// 0076F0E0 BSP_Session_RefreshFromOnlineSession(session).
void cc13_sweepB2_session_refresh_from_online_0076f0e0(void* session, Cc13SweepB2Host& host);

// 007700E0 BSP_Session_UpdateClientClockScale(session): writes the float 00E0B6C8.
void cc13_sweepB2_session_update_client_clock_scale_007700e0(void* session, Cc13SweepB2Host& host);

// 00770E50 BSP_WideString_Substring(source, out, pos, count): returns out.
void* cc13_sweepB2_wide_string_substring_00770e50(const void* source, void* out, std::int32_t pos,
                                                  std::uint32_t count, Cc13SweepB2Host& host);

// 00776230 BSP_PeerManager_PumpMessageQueues(session = game+1EF0h).
void cc13_sweepB2_peer_manager_pump_message_queues_00776230(void* session, Cc13SweepB2Host& host);

// 00775160 BSP_SessionHostInfo_BuildBlob(owner, out_blob, out_size).
void cc13_sweepB2_session_host_info_build_blob_00775160(void* owner, void** out_blob, std::uint32_t* out_size,
                                                       Cc13SweepB2Host& host);

// 00784B10 BSP_SessionTransport_KickPeer(transport, peer, flag).
void cc13_sweepB2_session_transport_kick_peer_00784b10(void* transport, void* peer, std::uint8_t flag,
                                                       Cc13SweepB2Host& host);

// ---- segments 45 and 46 ----

// 0078DBA0 BSP_OceanEnvironment_InitWaterParticles(environment, source, settings).
void cc13_sweepB2_ocean_init_water_particles_0078dba0(void* environment, void* source, void* settings,
                                                      Cc13SweepB2Host& host);

// 0078DCF0 BSP_OceanEnvironment_InitBeams(environment, source, settings).
void cc13_sweepB2_ocean_init_beams_0078dcf0(void* environment, void* source, void* settings, Cc13SweepB2Host& host);

// 00795D90 MovieCameraKeyframe_ConvertAnchorSpace(key, new_mode).
void cc13_sweepB2_movie_keyframe_convert_anchor_space_00795d90(void* key, std::int32_t new_mode,
                                                               Cc13SweepB2Host& host);

// 007A4EF0 BSP_MovCam_SeedAnglesFromVehicleCamera(camera, target).
void cc13_sweepB2_movcam_seed_angles_from_vehicle_camera_007a4ef0(void* camera, void* target, Cc13SweepB2Host& host);

// 007A49A0 BSP_MovCam_PickRandomDeckCamera(camera, target, look_target, key): the chosen
// camera index from the keyed list, or -1.
std::int32_t cc13_sweepB2_movcam_pick_random_deck_camera_007a49a0(void* camera, void* target, void* look_target,
                                                                  void* key, Cc13SweepB2Host& host);

// 007A8C60 BSP_MovCam_PickDeckPointScored(camera, entity): the chosen index or -1.
std::int32_t cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60(void* camera, void* entity,
                                                                 Cc13SweepB2Host& host);

// 007A6710 BSP_MovCam_EnterVehicleCamera(camera, index).
void cc13_sweepB2_movcam_enter_vehicle_camera_007a6710(void* camera, std::int32_t index, Cc13SweepB2Host& host);

// 007A7C60 BSP_MovCam_ReceivePathPoints(camera, message).
void cc13_sweepB2_movcam_receive_path_points_007a7c60(void* camera, const void* message, Cc13SweepB2Host& host);

}  // namespace bsp
