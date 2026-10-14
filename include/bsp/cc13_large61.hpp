#pragma once
// Packet cc13-large61: large segment 61 functions the earlier waves (cc13-left61,
// cc13-avoid1) did not reach. docs/CC13_LARGE61.md lists every function with its
// ABI, the uncertainty, and the ones skipped with the reason.
//
// Objects are reached through Raw (byte offsets into the Win32 layout, shared with
// cc13_left61). Every call the image makes goes through a Large61Host method named
// for its target, invoked at the point the image makes it. Lua values the image
// keeps as LuaObject temporaries are opaque `void*` handles owned by the host.
// Descriptive names are hypotheses, not recovered symbols.
//
// Status: reconstructed, build-tested only; new C++ interfaces, not ABI-compatible,
// not fixture-tested, not game-validated.

#include <cstdint>

#include "bsp/cc13_left61.hpp"

namespace bsp::cc13_large61 {

using bsp::cc13_left61::Raw;
using bsp::cc13_left61::kVecFirst;
using bsp::cc13_left61::kVecLast;

struct Large61Host {
    virtual ~Large61Host() = default;

    // --- CRT / MSVC9 containers -------------------------------------------
    // 00BF6713, the checked-iterator report. The native carries on after it returns.
    virtual void checked_iterator_report_00bf6713() = 0;
    // size() of a dword vector, __thiscall(ECX = vector), EAX = count. Two identical
    // instantiations are called: 00415450 and 009413B0 (`target`).
    virtual std::uint32_t vector_size(std::uint32_t target, Raw vector) = 0;
    // 00492210, __thiscall(ECX = dword vector; count, fill), RET 8: resize.
    virtual void dword_vector_resize_00492210(Raw vector, std::uint32_t count, std::uint32_t fill) = 0;

    // --- session messages -------------------------------------------------
    // 0075B430, __thiscall(ECX = message; type), RET 4: the message base (+00h..+17h).
    virtual void session_message_construct_base_0075b430(Raw message, std::uint32_t type) = 0;
    // 0095B9C0, __thiscall(ECX = class descriptor), EAX: the vehicle class registry id.
    virtual std::uint32_t vehicle_class_registry_id_0095b9c0(void* descriptor) = 0;

    // --- NativeString / pool ----------------------------------------------
    // 0041E870, __thiscall(ECX = NativeString {size +0, data +4}; const char*), RET 4.
    virtual void native_string_assign_0041e870(std::uint32_t* string, const char* text) = 0;
    // 00419CC0 (the pool singleton) then 00BD1510 __thiscall(pool; data, size + 1, 1), RET 0Ch:
    // the release of a NativeString buffer. The image always makes the two calls back to back.
    virtual void sized_pool_return_00419cc0_00bd1510(void* data, std::uint32_t bytes,
                                                     std::uint32_t one) = 0;

    // --- Lua (opaque handles) ---------------------------------------------
    // 00B66C00, __thiscall(ECX = state owner; lua_State*), RET 4: wrap a borrowed state.
    virtual void* lua_state_owner_construct_borrowed_00b66c00(void* lua_state) = 0;
    // 00B679B0, __thiscall(ECX = state owner; LuaObject* out), RET 4: the call frame.
    virtual void* lua_open_call_frame_00b679b0(void* owner) = 0;
    // 00B677E0, __thiscall(ECX = frame; LuaObject* out, int index), RET 8: an argument.
    virtual void* lua_argument_at_00b677e0(void* frame, std::int32_t index) = 0;
    // 00B67700, __thiscall(ECX = LuaObject): the destructor.
    virtual void lua_object_destroy_00b67700(void* object) = 0;
    // 00B663F0, __thiscall(ECX = frame), EAX: the argument count.
    virtual std::int32_t lua_frame_argument_count_00b663f0(void* frame) = 0;
    // 00888760, __fastcall(ECX = float out[3], EDX = object), EAX = out: fields x, y, z.
    virtual float* lua_object_read_vector3_00888760(float* out, void* object) = 0;
    // 00B662B0, __thiscall(ECX = object), EAX: the borrowed string.
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    // 00B66400, __thiscall(ECX = frame), EAX: the result count.
    virtual std::int32_t lua_frame_result_count_00b66400(void* frame) = 0;
    // 00B669A0, __thiscall(ECX = state owner): close.
    virtual void lua_state_owner_close_00b669a0(void* owner) = 0;

    // --- game / scene / matrices ------------------------------------------
    // 004C6BA0 BSP_Game_SpawnEntityByName, __thiscall(ECX = [00E188A8]; const char* name,
    // const char* second name, int), RET 0Ch, EAX = entity (may be null).
    virtual void* game_spawn_entity_by_name_004c6ba0(void* game, const char* name,
                                                     const char* second_name, std::int32_t zero) = 0;
    // 00B64640 / 00B646E0 / 00B64780, __fastcall(ECX = out 4x4, EDX = const float* angle),
    // EAX = out: rotation about X / Y / Z.
    virtual float* matrix_rotation_x_00b64640(float* out, const float* angle) = 0;
    virtual float* matrix_rotation_y_00b646e0(float* out, const float* angle) = 0;
    virtual float* matrix_rotation_z_00b64780(float* out, const float* angle) = 0;
    // 004134F0, __thiscall(ECX = destination 4x4; source), RET 4: x87 copy.
    virtual void matrix_copy_004134f0(float* destination, const float* source) = 0;
    // 00413920, __thiscall(ECX = left 4x4; out, right), RET 8, EAX = out.
    virtual float* matrix_multiply_00413920(const float* left, float* out, const float* right) = 0;
    // Entity vtable+88h, __thiscall(entity; const float matrix[16]), RET 4.
    virtual void entity_vcall_88_set_matrix(void* entity, const float* matrix) = 0;
    // Entity vtable+D8h, __thiscall(entity).
    virtual void entity_vcall_d8(void* entity) = 0;
    // 0046AAB0 BSP_SceneDatabase_ResolveDeferredReferences, ECX = [00E18680].
    virtual void scene_resolve_deferred_references_0046aab0(void* scene_database) = 0;

    // --- spawn requests (0094BFF0) -----------------------------------------
    // 00414DB0, __thiscall(ECX = entity): refresh the world pose (+FCh..).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 004C7AA0, __thiscall(ECX = game; out (28h bytes), const float* position, float, int),
    // RET 10h. Body not read; 0094BFF0 reads out+10h and out+1Ch as two xyz points.
    virtual void game_query_004c7aa0(void* game, void* out, const float* position, float radius,
                                     std::int32_t minus_one) = 0;
    // 00944130, __thiscall(ECX = member (10h bytes); party, vehicle class, NativeString by
    // value {size, data}), RET 10h: reconstructed as cc13_left61::spawn_member_construct_00944130.
    virtual void spawn_member_construct_00944130(void* member, std::uint32_t party, void* vehicle_class,
                                                 std::uint32_t name_size, std::uint32_t name_data) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = bag; const char*), RET 4, EAX.
    virtual void* property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 008EF7C0, __thiscall(ECX = property; dword), RET 4: property+0Ch = value, EAX = property.
    virtual void* property_set_field0c_008ef7c0(void* property, std::uint32_t value) = 0;
    // Class descriptor vtable+18h, __thiscall(descriptor; int class id), RET 4, AL.
    virtual bool vehicle_class_vcall_18(void* descriptor, std::int32_t class_id) = 0;
    // 008F3710 / 008F3770 / 008F3940, __thiscall(ECX = bag; const char*, value), RET 8.
    virtual void property_bag_add_int_008f3710(void* bag, const char* key, std::int32_t value) = 0;
    virtual void property_bag_add_float_008f3770(void* bag, const char* key, float value) = 0;
    virtual void property_bag_add_bool_008f3940(void* bag, const char* key, std::uint32_t value) = 0;
    // 00949610, __thiscall(ECX = member vector; const member*), RET 4: push_back.
    virtual void spawn_member_vector_push_back_00949610(Raw vector, const void* member) = 0;
    // 008F84D0, __thiscall(ECX = reference frame; const float matrix[16]), RET 4, EAX.
    virtual void* spawn_ref_frame_construct_008f84d0(void* frame, const float* matrix) = 0;
    // 008F8680, __thiscall(ECX = reference frame), EAX = const float[16].
    virtual const float* spawn_ref_frame_get_matrix_008f8680(void* frame) = 0;
    // 00695870 BSP_CallbackOwner_Destroy, ECX = object (the reference frame here).
    virtual void callback_owner_destroy_00695870(void* object) = 0;
    // 0085DC80, __thiscall(ECX = matrix 4x4): orthonormalise the basis rows.
    virtual void matrix_orthonormalize_rows_0085dc80(float* matrix) = 0;
    // 0041DD40, __thiscall(ECX = NativeString; size, flag), RET 8.
    virtual void native_string_resize_0041dd40(std::uint32_t* string, std::uint32_t size,
                                               std::uint32_t flag) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void memcpy_00bf7680(void* destination, const void* source, std::uint32_t bytes) = 0;
    // 00949530 BSP_SpawnManager_EnqueueRequest, __thiscall(ECX = manager; 14 dwords), RET 38h.
    virtual void spawn_manager_enqueue_00949530(Raw manager, Raw members, std::uint32_t id, void* ref_frame,
                                                float f0, float f1, float f2, float f3, std::uint32_t party,
                                                std::int32_t player_slot, std::uint32_t a6,
                                                const float* limits, const std::uint32_t* name,
                                                std::uint32_t a7, std::uint32_t a8) = 0;
    // 00946D70, __fastcall(ECX = first, EDX = last; allocator, tag), RET 8: destroy members.
    virtual void member_destroy_range_00946d70(void* first, void* last, void* allocator,
                                               std::uint32_t tag) = 0;
    // 00BF65AC operator delete.
    virtual void operator_delete_00bf65ac(void* block) = 0;

    // --- spawn message handler (0094B600) ----------------------------------
    // 00964790 BSP_VehicleClass_GetOrCreate, __fastcall(ECX = type id, DL = flag), EAX.
    virtual void* vehicle_class_get_or_create_00964790(std::uint32_t type_id, bool flag) = 0;
    // 00943E40, __thiscall(ECX = manager; id), RET 4: unlinks and frees the queued request
    // whose +7Ch equals id (body read for the name only).
    virtual void spawn_manager_remove_request_00943e40(Raw manager, std::uint32_t id) = 0;
    // 004BCA50 BSP_Game_GetEffectiveGameMode, EAX.
    virtual std::uint32_t game_effective_mode_004bca50() = 0;
    // 00946970, __thiscall(ECX = manager; player), RET 4, ST0 (kept as double here).
    virtual double spawn_manager_player_usage_00946970(Raw manager, std::uint32_t player) = 0;
    // 00946F00, __fastcall(ECX = player), ST0 (kept as double here).
    virtual double player_support_points_00946f00(std::uint32_t player) = 0;
    // 009FFD20 BSP_Unit_LossCountingSlot, __thiscall(ECX = unit), EAX.
    virtual std::uint32_t unit_loss_counting_slot_009ffd20(void* unit) = 0;
    // Unit vtable+5Ch, __thiscall(unit; int class id), RET 4, AL.
    virtual bool entity_vcall_5c(void* unit, std::int32_t class_id) = 0;
    // 00770B50 BSP_Session_SendMessageToNonlocalPeer, __thiscall(ECX = game+1EF0h; peer,
    // message), RET 8.
    virtual void session_send_to_nonlocal_peer_00770b50(void* session, void* peer, Raw message) = 0;
    // 00951F10, __thiscall(ECX = class descriptor), EAX: the Equipments count (class +128h).
    virtual std::int32_t vehicle_class_equipment_count_00951f10(void* descriptor) = 0;
    // 008F8530 BSP_SpawnRefFrame_ConstructFromEntity, __thiscall(ECX = frame; entity), RET 4, EAX.
    virtual void* spawn_ref_frame_construct_from_entity_008f8530(void* frame, void* entity) = 0;

    // --- map scene ocean materials (0093D2D0) ------------------------------
    // 00B74310 BSP_GeneratedModel_GetCurrentType, EAX (ECX still holds the scene; not read
    // here whether the callee uses it).
    virtual std::uint32_t generated_model_type_00b74310() = 0;
    // Node vtable+0Ch, __thiscall(node; type), RET 4, AL: is-a test.
    virtual bool node_vcall_0c(void* node, std::uint32_t type) = 0;
    // 00B74650, __thiscall(ECX = node), EAX: the geometry count.
    virtual std::int32_t node_geometry_count_00b74650(void* node) = 0;
    // 00B74640, __thiscall(ECX = node; index), RET 4, EAX: a geometry.
    virtual void* node_geometry_00b74640(void* node, std::int32_t index) = 0;
    // 00B72B40, __thiscall(ECX = geometry), EAX: the element count.
    virtual std::int32_t geometry_element_count_00b72b40(void* geometry) = 0;
    // 00B732C0, __thiscall(ECX = geometry; index), RET 4, EAX: an element (+20h = material).
    virtual void* geometry_element_00b732c0(void* geometry, std::int32_t index) = 0;
    // 00B172D0 BSP_Effect_GetName, __thiscall(ECX = effect), EAX = const NativeString*.
    virtual const std::uint32_t* effect_name_00b172d0(void* effect) = 0;
    // 00BF7FBF _stricmp (cdecl).
    virtual std::int32_t stricmp_00bf7fbf(const char* left, const char* right) = 0;
    // [00F8D394] vtable+48h, __thiscall(manager; const NativeString*), RET 4, EAX: the shader
    // (reference counted at +4).
    virtual void* shader_manager_vcall_48(void* manager, const std::uint32_t* name) = 0;
    // 00B19210 BSP_Material_SetShaderAndClearParameters, __thiscall(ECX = material; shader), RET 4.
    virtual void material_set_shader_00b19210(void* material, void* shader) = 0;
    // Material parameter registration, __thiscall(ECX = material; const NativeString* name,
    // value pointer[, count]): 00B18B20 float, 00B18B00 float2, 00B18AE0 float3, 00B18AA0
    // float4 (RET 8), 00B18AC0 float4 array (RET 0Ch).
    virtual void material_register_float_00b18b20(void* material, const std::uint32_t* name, float* value) = 0;
    virtual void material_register_float2_00b18b00(void* material, const std::uint32_t* name, float* value) = 0;
    virtual void material_register_float3_00b18ae0(void* material, const std::uint32_t* name, float* value) = 0;
    virtual void material_register_float4_00b18aa0(void* material, const std::uint32_t* name, float* value) = 0;
    virtual void material_register_float4_array_00b18ac0(void* material, const std::uint32_t* name, float* value,
                                                         std::int32_t count) = 0;
    // [00CE2220] InterlockedDecrement.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    // Object vtable slot 0, __thiscall(ECX = object), no stack argument: the final release.
    virtual void object_vcall_00(void* object) = 0;

    // --- sound debug overlay (0093FF10) ------------------------------------
    // 0093F840, __thiscall(ECX = 0Ch-element array {data, count, capacity}; n), RET 4: resize.
    virtual void array12_resize_0093f840(void* array, std::int32_t n) = 0;
    // 0093F610, __thiscall(ECX = dword array {data, count, capacity}; n), RET 4: reserve.
    virtual void array4_reserve_0093f610(void* array, std::int32_t n) = 0;
    // 00BFBF10 qsort_s (cdecl); `comparator` is the image address (0093F4B0 here).
    virtual void qsort_s_00bfbf10(void* base, std::uint32_t count, std::uint32_t width, std::uint32_t comparator,
                                  void* context) = 0;
    // 00A777A0 BSP_StreamedDialogManager_DrawDebug, ECX = [00F8BBCC].
    virtual void streamed_dialog_draw_debug_00a777a0(void* manager) = 0;
    // Sound vtable+14h, __thiscall(sound).
    virtual void sound_vcall_14(void* sound) = 0;
    // Sound vtable+1Ch, __thiscall(sound), ST0 (stored to float by every caller here).
    virtual float sound_vcall_1c(void* sound) = 0;
    // Sound vtable+38h, __thiscall(sound; float out[3]), RET 4, EAX = const float[3].
    virtual const float* sound_vcall_38(void* sound, float* out) = 0;
    // 00A81860 BSP_SoundSample_GetOptions, __thiscall(ECX = sound+4Ch), EAX = options (+0 type).
    virtual const std::int32_t* sound_sample_options_00a81860(void* sample) = 0;
    // 0043A870 BSP_Camera_ProjectWorldToScreenXY, __fastcall(ECX = const float world[3],
    // EDX = float out[2]; int, int), RET 8, EAX (7 = on screen).
    virtual std::int32_t camera_project_0043a870(const float* world, float* out, std::int32_t a,
                                                 std::int32_t b) = 0;
    // 0093FA70, __thiscall(ECX = overlay; float out[2], x, y, dword), RET 10h, EAX = out:
    // cc13_left61::sound_overlay_place_label_0093fa70 (its float `z` receives the dword bits).
    virtual float* overlay_place_label_0093fa70(void* overlay, float* out, float x, float y, void* tag) = 0;
    // 0093F790, __thiscall(ECX = overlay; float out[2], key), RET 8, EAX = out:
    // support_seg61_cc13 array12_find_pair_0093f790.
    virtual float* overlay_find_pair_0093f790(void* overlay, float* out, void* key) = 0;
    // 00BF7030 LIBCRT_sqrt, ST0 in and out.
    virtual double sqrt_00bf7030(double value) = 0;
    // 00467CF0 BSP_NativeString_ReverseFindHeader, __thiscall(ECX = string; const NativeString*
    // needle, from), RET 8, EAX = position or -1.
    virtual std::int32_t native_string_rfind_00467cf0(const std::uint32_t* string, const std::uint32_t* needle,
                                                      std::uint32_t from) = 0;
    // 00469840 BSP_NativeString_Substring, __thiscall(ECX = string; NativeString* out, pos,
    // count), RET 0Ch, EAX = out.
    virtual std::uint32_t* native_string_substr_00469840(const std::uint32_t* string, std::uint32_t* out,
                                                         std::uint32_t pos, std::uint32_t count) = 0;
    // 0093FDF0 BSP_SoundSampleOptions_CopyConstruct, __thiscall(ECX = memory (48h); source), RET 4.
    virtual void sound_options_copy_construct_0093fdf0(void* memory, const void* source) = 0;
    // 0093F950 BSP_SoundSampleParameters_Resize, __thiscall(ECX = options+3Ch; count), RET 4.
    virtual void sound_parameters_resize_0093f950(void* parameters, std::int32_t count) = 0;
    // 00BF6989 free.
    virtual void free_00bf6989(void* block) = 0;
    // 00BF55BE operator new.
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    // 00BF7A6A sprintf with the format at 00D19850, "%s(%d%%%%) %s:%d/%d/%dm:%d".
    virtual void sprintf_sound_label_00bf7a6a(char* buffer, const char* format, const char* name,
                                              std::int32_t volume_percent, const char* scale, std::int32_t min_distance,
                                              std::int32_t max_distance, std::int32_t distance,
                                              std::int32_t level_percent) = 0;
    // 004C70C0 BSP_Diagnostic_FormatLargeBuffer, cdecl(game, float x, float y, colour, text).
    virtual void diagnostic_text_coloured_004c70c0(void* game, float x, float y, std::uint32_t colour,
                                                   const char* text) = 0;
    // 004C7070 BSP_DiagnosticPrintf, cdecl(game, float x, float y, text).
    virtual void diagnostic_text_004c7070(void* game, float x, float y, const char* text) = 0;
    // 004C14C0 BSP_DiagnosticSink_GetOrCreate, called with no arguments; the result is unused.
    virtual void diagnostic_sink_004c14c0() = 0;
    // 00A4C296 getChannelsPlaying(system, int*) and 00A4C290 getCPUUsage(system, dsp, stream,
    // update, total): the FMOD Ex stdcall exports.
    virtual void fmod_get_channels_playing_00a4c296(void* system, std::int32_t* channels) = 0;
    virtual void fmod_get_cpu_usage_00a4c290(void* system, float* dsp, float* stream, float* update,
                                             float* total) = 0;
    // 004D92B0 BSP_FrontEnd_ActionEdgeOrRepeat, __thiscall(ECX = game; action), RET 4, AL.
    virtual bool frontend_action_edge_004d92b0(void* game, std::int32_t action) = 0;
    // 004BEC00 BSP_InputManager_GetSingleton, EAX.
    virtual void* input_manager_004bec00() = 0;
    // 0093F280 BSP_SoundOverlay_AxisStep, CL = round_up, stack float, RET 4, ST0:
    // cc13_left61::sound_overlay_axis_step_0093f280.
    virtual float sound_overlay_axis_step_0093f280(char round_up, float value) = 0;
    // 0093FE90 BSP_SoundOverlay_ApplySampleSettings, __thiscall(ECX = sound; 48h-byte options
    // by value, owned and destroyed by the callee), RET 48h.
    virtual void sound_apply_settings_0093fe90(void* sound, void* options) = 0;
    // 00A7A3F0 BSP_SoundManager_DirtyClassEntries, __thiscall(ECX = [00F8BBD8]; mask), RET 4.
    virtual void sound_manager_dirty_class_entries_00a7a3f0(void* manager, std::uint32_t mask) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, ECX = node.
    virtual void transform_refresh_world_00b6db70(void* node) = 0;
    // [game+1ED4h] vtable+120h, __thiscall(object; out), RET 4, EAX = const float[16].
    virtual const float* object_vcall_120(void* object, void* out) = 0;
    // 0093EFC0 BSP_Matrix44_NotEqual, __thiscall(ECX = matrix; other), RET 4, AL.
    virtual bool matrix44_not_equal_0093efc0(const float* matrix, const float* other) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker, __fastcall(ECX = centre, EDX = colour; float
    // radius, const float* colour2, int), RET 0Ch.
    virtual void debug_wire_sphere_00860be0(const float* centre, const float* colour, float radius,
                                            const float* colour2, std::int32_t flag) = 0;
};

// The image globals the functions here read or write. Storage belongs to the caller.
struct Large61Globals {
    std::uint32_t* luakod_guard_00f89b58;   // bit 0: the one-time `luakod` static
    std::uint32_t* luakod_static_00f89b54;  // set to 0 by that one-time block
    const float* zero_vector_00f87574;      // 3 floats (BSS), the default rotation
    void* game_00e188a8;                    // [00E188A8]
    void* scene_database_00e18680;          // [00E18680]
    std::uint32_t* spawn_request_counter_00e0cf74;  // wraps to 1 above 16000
    // The two-bank entity handle table: id < [00F89A10] reads bank 0 ([00F89A54] records,
    // base id [00F89A0C]), else bank 1 ([00F89AA8], base [00F89A60]); 10h-byte records, the
    // entity at +0Ch.
    const std::int32_t* handle_split_00f89a10;
    const std::int32_t* bank0_base_00f89a0c;
    std::uint8_t* const* bank0_records_00f89a54;
    const std::int32_t* bank1_base_00f89a60;
    std::uint8_t* const* bank1_records_00f89aa8;
};

// Session message 42h (5Ch bytes), the spawn request message built by 00947BC0.
// +00h vtable 00D03740; +04h = 1; +08h..+17h base; +18h a0; +1Ch a2; +20h u16 id of a3
// (+174h) or 0; +24h dword vector of class registry ids from a1; +34h copy of a4;
// +44h copy of a5; +54h u16 id of a6 (+174h, only when a6 is non-null); +56h byte a6 != 0;
// +58h dword 0.
constexpr std::uint32_t kSpawnMessageType = 0x42;
constexpr std::uint32_t kSpawnMessageVtable = 0x00D03740;
constexpr std::size_t kSpawnMessageSize = 0x5C;

// 00947BC0, __thiscall(ECX = message; a0..a6), RET 1Ch, EAX = message.
// a1, a4, a5 are MSVC9 dword vectors (+4 first, +8 last); a3 and a6 are units (or null).
Raw spawn_message_construct_00947bc0(Raw message, std::uint32_t a0, Raw a1, std::uint32_t a2,
                                     void* a3, Raw a4, Raw a5, void* a6, Large61Host& host);

// 00945450, the SpawnLight mission Lua binding: int __fastcall(ECX = lua_State*), plain RET.
// Lua args: 0 class name, 1 position {x,y,z}, 2 optional rotation {x,y,z}, 3 optional
// second name (else argument 0 again).
std::int32_t lua_spawn_light_00945450(void* lua_state, const Large61Globals& globals,
                                      Large61Host& host);

// 0094BFF0, __thiscall(ECX = spawn manager; a0..a8), RET 24h. The power-up spawn request
// (caller 008EADA0 BSP_PowerUpManager_UseItem): a0 player slot, a1 vehicle class descriptor,
// a2 the unit the flight starts near, a3 Equipment, a4 ReloadEnabled, a5 AutoAttackTarget unit,
// a6..a8 passed through to 00949530.
void powerup_spawn_request_0094bff0(Raw manager, std::int32_t player_slot, void* vehicle_class, void* unit,
                                    std::uint32_t equipment, std::uint32_t reload_enabled, void* attack_target,
                                    std::uint32_t a6, std::uint32_t a7, std::uint32_t a8,
                                    const Large61Globals& globals, Large61Host& host);

// 0094B600, __thiscall(ECX = spawn manager; message 42h, a1), RET 8: the handler of the spawn
// request message (callers 00777850 BSP_Session_PumpReceiveQueue, 0094C830). a1 is passed through
// to 00949530. Writes message+58h (the request id) on the non-host path.
void spawn_manager_handle_message_0094b600(Raw manager, Raw message, std::uint32_t a1,
                                           const Large61Globals& globals, Large61Host& host);

// The image globals 0093D2D0 reads or writes. Storage belongs to the caller.
struct OceanMaterialGlobals {
    std::uint32_t* guard_00f89b20;            // bits 0..2: one-time blocks
    void* game_00e188a8;                      // [00E188A8]: +711Ch..+7130h the ocean bounds
    void* shader_manager_00f8d394;            // [00F8D394]
    float* static_shot_00e1891c;              // float3
    float* basic_00e18928;                    // float4 array, 1 element
    float* snoise_00e18938;                   // float3
    float* hnoise_00e18944;                   // float2
    const float* ground_seed_x_00e1894c;      // copied to 00F89B04 once
    const float* ground_seed_z_00e18954;      // copied to 00F89B0C once
    float* ground_00f89b04;                   // float3; +8 is multiplied by 3 on every pass
    float* uw_slope_00e18958;                 // float2
    float* uw_color0_00e18960;                // float4
    float* uw_color1_00e18970;                // float4
    float* ocean_size_map_00f89af4;           // float4
    float* ocean_size_underwater_00f89b10;    // float4
};

// 0093D2D0 BSP_MapScene_SetupOceanMaterials, __thiscall(ECX = scene; node, flag), RET 8
// (EBP frame, ESP aligned to 8). For each element of each geometry of a node of the
// generated-model type whose material effect name is none of the three ocean shaders, the
// flag's low byte picks the underwater shader set-up (non-zero) or the map parameter set
// (zero); then recurses over the child (+34h) and sibling (+3Ch) links with the same flag.
// Callers 00538F80 BSP_HudFormationScreen_Register, 005AB190 BSP_HudMapScreen_Register.
void map_scene_setup_ocean_materials_0093d2d0(Raw scene, void* node, std::uint32_t flag,
                                              const OceanMaterialGlobals& globals, Large61Host& host);

// The image globals 0093FF10 reads or writes. Slots are re-read where the image re-reads them.
struct SoundOverlayGlobals {
    void* const* sound_manager_slot_00f8bbd8;    // &[00F8BBD8]: +44h FMOD system, +8Ch/+90h sounds,
                                                 // +F4h listener, +160h..+168h cleared
    void* const* dialog_manager_slot_00f8bbcc;   // &[00F8BBCC]
    void* const* control_manager_slot_00f8bbf4;  // &[00F8BBF4]: +6Ch vector
    void* game_00e188a8;                         // [00E188A8]
    std::uint32_t* colour_guard_00f89b2c;        // bit 0: the one-time colour
    std::uint8_t* colour_00f89b28;               // 4 bytes, set to 00 00 FF FF once
    const char* empty_string_00f89b30;           // "" (BSS)
};

// 0093FF10 BSP_SoundOverlay_Draw, __thiscall(ECX = overlay; flag), RET 4 (EBP frame). No rel32
// or absolute-dword reference (no live caller). Labels each audible sound, prints the per-sound
// "%s(%d%%%%) %s:%d/%d/%dm:%d" line when the flag's low byte is set, lets the action pair 20h/21h
// edit the volume / min / max distance (and toggle Lin/Log) of the sound nearest the screen
// centre, and marks the camera target with a wire sphere when the camera and view matrices differ.
void sound_overlay_draw_0093ff10(Raw overlay, std::uint32_t flag, const SoundOverlayGlobals& globals,
                                 Large61Host& host);

}  // namespace bsp::cc13_large61
