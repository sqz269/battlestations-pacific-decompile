#pragma once

// cc13_front2: callee functions that block the wiring of already-reconstructed
// callers (packet cc13-front2, docs/CC13_FRONT2.md). Every name is a
// hypothesis, not a recovered symbol. These are new C++ interfaces over the
// native records, not drop-in replacements of the original ABI. Nothing here
// is wired into the game.
//
// Conventions as in the cc13 sweeps: native objects are raw byte blocks touched
// only at the offsets the image reads or writes; every call the image makes is
// either a direct call of an existing reconstruction or a named host method,
// invoked at the same point with the same arguments.

#include <cstdint>

namespace bsp {

class Cc13Front2Host {
public:
    virtual ~Cc13Front2Host() = default;

    // 00BF6713, the CRT checked-iterator failure (_invalid_parameter_noinfo).
    // The image continues after the call when it returns, so the code here
    // does too.
    virtual void invalid_parameter_00bf6713() = 0;

    // ---- camera mover (0042DCD0) ----
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity), RET.
    // Called only when the entity's pose byte +C8h is clear (the caller tests it).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00BF701A, the CRT _CIatan2: ST1 = y, ST0 = x, result in ST0.
    virtual double ci_atan2_00bf701a(double y, double x) = 0;

    // ---- input (00A91020, 00A95CB0) ----
    // The dword global 00F8BBF4: the input device table (backend).
    virtual std::uint8_t* input_device_table_00f8bbf4() = 0;
    // Device vtable +2Ch: __thiscall(ECX = device), no stack arguments, AL.
    virtual bool input_device_vcall_2c(void* device) = 0;
    // 00A95890 BSP_GamepadForce_ClearRequests: __thiscall(ECX = channel), RET.
    // 00A95CB0 can pass a null ECX (see there).
    virtual void gamepad_force_clear_requests_00a95890(void* channel) = 0;

    // ---- session transport (00784790) ----
    // The kernel32 imports through 00CE2218 / 00CE2210 (stdcall, one argument).
    virtual void enter_critical_section_ce2218(void* section) = 0;
    virtual void leave_critical_section_ce2210(void* section) = 0;
    // 00783C80 BSP_SessionTransport_SerializeMessage: __thiscall(ECX =
    // transport, peer, message), RET 8.
    virtual void session_transport_serialize_message_00783c80(void* transport, void* peer,
                                                              void* message) = 0;

    // ---- settings apply (008D5B50) ----
    // The dword stored at an image global (00F8D394 renderer, 00E188A8 game,
    // 00F8A304 download manager, 00F8ABE8 Xenon manager, 00F88974 language
    // table, 00F8BC4C localization, 00F8BBF0 shadow service, 00F8D39C render
    // service, 0109CF04 platform, 00E198C4), read at each point the image reads it.
    virtual std::uint8_t* load_global_pointer(std::uint32_t address) = 0;
    // The address of an image global block (00F88A3C latch block and empty
    // text, 0108FF24 the geography-region NativeString).
    virtual std::uint8_t* global_address(std::uint32_t address) = 0;
    // NativeString helpers on an 8-byte {length, data} block.
    // 0041E870 BSP_NativeString_Assign: __thiscall(ECX = string, const char*), RET 4.
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    // 0041DD40 BSP_NativeString_Resize: __thiscall(ECX = string, length, keep), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint8_t keep) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments, EAX.
    virtual void* sized_storage_pool_get_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool, data, size, 1), RET 0Ch.
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
                                                          std::uint32_t one) = 0;
    // 008D5430 BSP_Settings_ApplyAudio: __thiscall(ECX = settings), RET.
    virtual void settings_apply_audio_008d5430(std::uint8_t* settings) = 0;
    // 00B1FFB0 BSP_Renderer_SetTextureDetailBiasWord: __thiscall(ECX = renderer, int), RET 4.
    virtual void renderer_set_texture_detail_bias_00b1ffb0(void* renderer, std::int32_t bias) = 0;
    // Renderer vtable +F0h: __thiscall(ECX = renderer, float), RET 4 (gamma).
    virtual void renderer_vcall_f0(void* renderer, float value) = 0;
    // Renderer vtable +80h: __thiscall(ECX = renderer, out), EAX = a {width,
    // height} pair (the out buffer or another).
    virtual const std::uint32_t* renderer_vcall_80(void* renderer, std::uint32_t* out) = 0;
    // 004B46E0 (adjustor thunk): __thiscall(ECX = game, four bytes), RET 10h.
    virtual void game_set_stick_options_004b46e0(void* game, std::uint8_t a, std::uint8_t b, std::uint8_t c,
                                                 std::uint8_t d) = 0;
    // 004DCDF0 BSP_Game_ApplyCameraDisplaySettings: __thiscall(ECX = game), RET.
    virtual void game_apply_camera_display_settings_004dcdf0(void* game) = 0;
    // 00A3E510 BSP_XenonSystemManager_HasSelectedUser: __thiscall(ECX = manager), AL.
    virtual bool xenon_manager_has_selected_user_00a3e510(void* manager) = 0;
    // 00BF681B operator new(size), cdecl.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 009955F0 BSP_DownloadManager_Construct: __thiscall(ECX = storage), RET.
    virtual void download_manager_construct_009955f0(void* storage) = 0;
    // 004C1710 BSP_MediaManager_GetOrCreate: no arguments, EAX.
    virtual std::uint8_t* media_manager_get_or_create_004c1710() = 0;
    // 007371D0 BSP_FontSystem_GetRegistry: no arguments, EAX.
    virtual void* font_system_get_registry_007371d0() = 0;
    // 00AC3610 BSP_FontRegistry_ReloadPaths: __thiscall(ECX = registry, root
    // string*, language string*), RET 8.
    virtual void font_registry_reload_paths_00ac3610(void* registry, void* root, void* language_path) = 0;
    // 00AA09D0 BSP_Localization_SetLanguage: __thiscall(ECX = localization, string*), RET 4.
    virtual void localization_set_language_00aa09d0(void* localization, void* lanfile) = 0;
    // 00A94C50 BSP_GamepadForce_SetEnabled: the byte in CL.
    virtual void gamepad_force_set_enabled_00a94c50(std::uint8_t enabled) = 0;
    // Download manager vtable +24h / +20h: __thiscall(ECX = manager), RET;
    // vtable +14h: __thiscall(ECX = manager, list*), RET 4.
    virtual void download_manager_vcall_24(void* manager) = 0;
    virtual void download_manager_vcall_14(void* manager, void* names) = 0;
    virtual void download_manager_vcall_20(void* manager) = 0;
    // 0043EA80 BSP_DownloadManager_FilterKnownNames: __thiscall(ECX = manager, list*), RET 4.
    virtual void download_manager_filter_known_names_0043ea80(void* manager, void* names) = 0;
    // 00AA0D30 BSP_Localization_RegisterTableName: __thiscall(ECX, string*), RET 4.
    virtual void localization_register_table_name_00aa0d30(void* localization, void* name) = 0;
    // 00AA06D0 BSP_Localization_ReloadTables: __thiscall(ECX, dword), RET 4.
    virtual void localization_reload_tables_00aa06d0(void* localization, std::uint32_t value) = 0;
    // 00B72270 BSP_Scene_FindNodeByName: __thiscall(ECX = scene, string*, 0), RET 8, EAX.
    virtual void* scene_find_node_by_name_00b72270(void* scene, void* name, std::uint32_t zero) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor: __thiscall(ECX = node, float, dword), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor, std::uint32_t flag) = 0;
    // 00BBDDF0 BSP_Atmosphere_GetCloudManager: __thiscall(ECX), EAX = [ECX+3Ch].
    virtual void* atmosphere_get_cloud_manager_00bbddf0(void* atmosphere) = 0;
    // 00BBCFE0 BSP_CloudManager_SetEnabled: __thiscall(ECX = clouds, byte), RET 4.
    virtual void cloud_manager_set_enabled_00bbcfe0(void* clouds, std::uint8_t enabled) = 0;
    // Ocean-side object [[game+19F0h]+A8h] vtable +10h: __thiscall(ECX, dword), RET 4.
    virtual void ocean_effect_vcall_10(void* object, std::uint32_t value) = 0;
    // [00F8BBF0] vtable +8: __thiscall(ECX, byte widened to a dword), RET 4.
    virtual void shadow_service_vcall_08(void* service, std::uint32_t value) = 0;
    // 00B0D580 BSP_RenderService_SetMotionBlurAndSamples: __thiscall(ECX, byte, dword), RET 8.
    virtual void render_service_set_motion_blur_and_samples_00b0d580(void* service, std::uint8_t blur,
                                                                     std::uint32_t samples) = 0;
    // 00B0FC00 BSP_RenderResources_ReleaseMembers (a JMP to 00B0F6E0): __thiscall(ECX).
    virtual void render_resources_release_members_00b0fc00(void* service) = 0;
    // 00B29E60 BSP_D3D9Renderer_ChangePresentationMode: __thiscall(ECX = renderer,
    // width, height, byte, dword, byte, flag dword), RET 18h; AL not read here.
    virtual void renderer_change_presentation_mode_00b29e60(void* renderer, std::uint32_t width,
                                                            std::uint32_t height, std::uint8_t windowed,
                                                            std::uint32_t samples, std::uint8_t flag_60,
                                                            std::uint8_t changed) = 0;
    // [0109CF04] vtable +0Ch: __thiscall(ECX = platform, byte, width, height), RET 0Ch.
    virtual void platform_vcall_0c(void* platform, std::uint8_t windowed, std::uint32_t width,
                                   std::uint32_t height) = 0;
    // 00A4D46A XLiveOnResetDevice (import thunk), stdcall(one pointer).
    virtual void xlive_on_reset_device_00a4d46a(void* parameters) = 0;
    // 00B107F0 BSP_RenderResources_InitializeMembers_PartialEntry: __thiscall(ECX, byte, byte, dword), RET 0Ch.
    virtual void render_resources_initialize_members_00b107f0(void* service, std::uint8_t a, std::uint8_t b,
                                                              std::uint32_t samples) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate: no arguments, EAX.
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    // 00AA4EF0 BSP_GuiManager_RelayoutAllPagesWideScreen: __thiscall(ECX), RET.
    virtual void gui_manager_relayout_all_pages_wide_screen_00aa4ef0(void* gui) = 0;
    // 00B0D080 BSP_RenderService_SetFilmEffect: __thiscall(ECX, dword), RET 4.
    virtual void render_service_set_film_effect_00b0d080(void* service, std::uint32_t value) = 0;
    // 00439100 BSP_Win32_GetUserGeographyRegion: no arguments, EAX = text or null.
    virtual const char* win32_get_user_geography_region_00439100() = 0;

    // ---- entity replication state (0077FAD0) ----
    // 009295B0 SEntity_DefaultInitSlotA4: __thiscall(ECX = entity), RET.
    virtual void sentity_default_init_slot_a4_009295b0(void* entity) = 0;
    // 0070DB20 BSP_UnitGroup_CreateForLeader: __thiscall(ECX = entity), RET.
    virtual void unit_group_create_for_leader_0070db20(void* entity) = 0;
    // 0070EFD0 BSP_UnitGroup_ApplyFormationShape: __thiscall(ECX = group, dword), RET 4.
    virtual void unit_group_apply_formation_shape_0070efd0(void* group, std::uint32_t shape) = 0;
    // 00521E30 BSP_EntityHandleTable_Resolve: the handle word in CX, EAX.
    virtual void* entity_handle_table_resolve_00521e30(std::uint16_t handle) = 0;
    // 0077F940 BSP_UnitGroup_JoinOrMerge: __thiscall(ECX = entity, other), RET 4.
    virtual void unit_group_join_or_merge_0077f940(void* entity, void* other) = 0;
    // 0070D080 BSP_UnitGroup_FindMemberRecord: __thiscall(ECX = group, entity), RET 4, EAX.
    virtual std::uint8_t* unit_group_find_member_record_0070d080(void* group, void* entity) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData: __thiscall(ECX = entity, out), RET 4;
    // out is the 14h-byte block the caller then passes by value to 004425C0.
    virtual void sentity_get_saved_entity_lua_data_009238a0(void* entity, void* out) = 0;
    // 004425C0 BSP_LuaReader_Construct: __thiscall(ECX = reader, 14h bytes by value), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, const void* saved_data) = 0;
    // The reader methods take a Lua key by value as two dwords {type, value}.
    // 00BD8E20 BSP_LuaReader_EnterKey: __thiscall(ECX = reader, key), RET 8.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, std::uint32_t key_type, const void* key) = 0;
    // 00BD7A20 BSP_LuaReader_LeaveKeyThunk: __thiscall(ECX = reader), RET.
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    // 00BD6830 BSP_LuaReader_ReadField: __thiscall(ECX = reader, key, {value
    // type, value pointer}), RET 10h.
    virtual void lua_reader_read_field_00bd6830(void* reader, std::uint32_t key_type, const void* key,
                                                std::uint32_t value_type, void* value) = 0;
    // 00BD5EB0 BSP_LuaReader_HasKey: __thiscall(ECX = reader, key), RET 8, AL.
    virtual bool lua_reader_has_key_00bd5eb0(void* reader, std::uint32_t key_type, const void* key) = 0;
    // Reader vtable +4 (key by value, RET 8) and +8 (no arguments).
    virtual void lua_reader_vcall_04(void* reader, std::uint32_t key_type, const void* key) = 0;
    virtual void lua_reader_vcall_08(void* reader) = 0;
    // 00441A20 BSP_LuaReader_DestroyStackInstance: __thiscall(ECX = reader), RET.
    virtual void lua_reader_destroy_stack_instance_00441a20(void* reader) = 0;
    // 00804F10 BSP_ReconDetectionSet_ReadArchive: __thiscall(ECX = set, reader), RET 4.
    virtual void recon_detection_set_read_archive_00804f10(void* set, void* reader) = 0;
    // 0070ECF0 BSP_UnitGroup_CreateFromScene: __thiscall(ECX = reader), EAX.
    virtual void* unit_group_create_from_scene_0070ecf0(void* reader) = 0;
    // 006E6B10 BSP_SaveReader_ReadUnitHandle: __thiscall(ECX = slot, reader), RET 4.
    virtual void save_reader_read_unit_handle_006e6b10(void* slot, void* reader) = 0;
    // 00927B40 BSP_MissionEntity_LuaSelfObject: __thiscall(ECX = entity, out), RET 4, EAX.
    virtual void* mission_entity_lua_self_object_00927b40(void* entity, void* out) = 0;
    // 00B67580 BSP_LuaObject_SetNewTable: __thiscall(ECX = object, string*), RET 4.
    virtual void lua_object_set_new_table_00b67580(void* object, void* name) = 0;
    // 00B67700 BSP_LuaObject_Destruct: __thiscall(ECX = object), RET.
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // Entity vtable +5Ch: IsKindOf(class id), RET 4, AL.
    virtual bool entity_vcall_5c_is_kind_of(void* entity, std::uint32_t class_id) = 0;
    // 00985590 BSP_WarningManager_DispatchGenerateChannel: __thiscall(ECX =
    // [00F8A0C4], entity), RET 4.
    virtual void warning_manager_dispatch_generate_00985590(void* manager, void* entity) = 0;
    // Entity vtable +0Ch: __thiscall(ECX = entity, pointer, dword), RET 8.
    virtual void entity_vcall_0c(void* entity, void* slot, std::uint32_t zero) = 0;
};

// 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize: __fastcall(ECX =
// destination, EDX = source, stack matrix, normalize byte in a dword slot),
// RET 8, EAX = destination. The 3x3 rotation of source by the row-major matrix
// (no translation), computed in x87 with float spills of all three components
// before any destination write; normalize != 0 then hands the spilled vector to
// 00419510 (ECX = destination, EDX = the spill), otherwise the spill is copied.
float* vector3f_transform_direction_optional_normalize_0042d0d0(float* destination,
                                                                const float* source,
                                                                const float* matrix,
                                                                std::uint8_t normalize);

// 0042DCD0 BSP_CameraMover_UpdateFocus: __thiscall(ECX = mover, float dt, byte
// add_parent_yaw), RET 8, AL. mover is the raw camera-mover block. Returns 1
// when it steered (0042C750) toward the focus point, 0 when it cleared the
// focus offsets +3E0h/+3E4h instead.
bool camera_mover_update_focus_0042dcd0(std::uint8_t* mover, float dt, std::uint8_t add_parent_yaw,
                                        Cc13Front2Host& host);

// 00A91020 BSP_InputDeviceTable_AnyDynamicDeviceButtonDown: __fastcall(ECX =
// table), RET, AL. Any device in the three checked vectors at table+6Ch +
// class*24h whose vtable +2Ch answers nonzero.
bool input_device_table_any_dynamic_device_button_down_00a91020(std::uint8_t* table,
                                                                Cc13Front2Host& host);

// 00A95CB0 BSP_GamepadForce_ClearAllChannels: no arguments, RET. 00A95890 on
// each pointer of the vector at [00F8BBF4]+B8h..+BCh.
void gamepad_force_clear_all_channels_00a95cb0(Cc13Front2Host& host);

// 00784790 BSP_SessionTransport_BroadcastExcept: __thiscall(ECX = transport,
// message, exclude_peer), RET 8. Serializes the message to every peer of the
// list at transport+8h after the first, except exclude_peer, each under the
// peer's lock.
void session_transport_broadcast_except_00784790(std::uint8_t* transport, void* message,
                                                 void* exclude_peer, Cc13Front2Host& host);

// 008D5B50 BSP_Settings_ApplyAll: __thiscall(ECX = settings, the block at
// 00F88980), RET. The single commit of the whole settings block to audio,
// renderer, input, download content, fonts, localization, the scene and the
// render service. The SEH frame's state stores only select which temporary
// NativeString an unwind destroys; they are not modelled.
void settings_apply_all_008d5b50(std::uint8_t* settings, Cc13Front2Host& host);

// 0077FAD0 BSP_NetEntity_ReadReplicationState: __thiscall(ECX = entity), RET.
// 009295B0 first; with an init record at +C0h of kind 2 whose data (+8) has
// +C5h set, the formation group is created or joined and its member record
// filled; with kind 3, the saved "_netEntity"/"_reconData" tables are read
// (hasPlrCmd, formation, lastBanto, lastBantoPlayerIndex); then the Lua self
// table "reconlevel", the generate channel for kinds 5/18h, and vtable +0Ch
// when +15Ch is nonzero. The SEH state stores are not modelled.
void net_entity_read_replication_state_0077fad0(std::uint8_t* entity, Cc13Front2Host& host);

} // namespace bsp
