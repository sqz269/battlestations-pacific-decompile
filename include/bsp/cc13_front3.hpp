#pragma once

// cc13_front3: flight and gun functions in segments 40, 47 and 48 that block
// the wiring of already-reconstructed callers (packet cc13-front3,
// docs/CC13_FRONT3.md). Struck sub-ranges 007DF000-007DFFFF and
// 007EE000-007F2FFF are not touched.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the
// original ABI; each function's comment gives the native ABI. Native objects
// are raw byte blocks touched only at the offsets the image reads or writes,
// and every call the image makes goes through a named Cc13Front3Host method at
// the same point with the same arguments. Nothing here is wired into the game.
// Build-tested only.

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_front3_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

class Cc13Front3Host {
public:
    virtual ~Cc13Front3Host() = default;

    // ---- CRT, Win32 and globals ----
    // 00BF65AC and 00BF6989 are two _free thunks; the image uses both.
    virtual void crt_free_00bf65ac(void* block) = 0;
    virtual void crt_free_00bf6989(void* block) = 0;
    // [00CE2220] InterlockedDecrement.
    virtual std::int32_t interlocked_decrement(volatile std::int32_t* value) = 0;
    // [00E188A8], the game object.
    virtual void* game_00e188a8() = 0;
    // 00F87574..00F8757C, the static zero vector (BSS; three floats).
    virtual const float* static_vector_00f87574() = 0;

    // ---- native strings ----
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = 8-byte {length, data}; const char*), RET 4.
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments, EAX = pool. The three
    // dwords the image pushes before it are 00BD1510's arguments.
    virtual void* sized_pool_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool; data, bytes, flag), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;

    // ---- virtual slots (ECX = the object) ----
    // Reference-counted object slot +0h, no arguments (release when the count at +4h hits 0).
    virtual void vcall_00_release(void* object) = 0;
    // Unit slot +5Ch(kind), AL.
    virtual bool vcall_5c_has_ordnance_kind(void* unit, std::int32_t kind) = 0;
    // Plane slot +194h(const NativeString* name, 0): the named effect/state hook ("powerlost").
    virtual void vcall_194_named(void* plane, void* name, std::int32_t zero) = 0;
    // Plane slot +204h(), float in ST0.
    virtual float vcall_204_float(void* plane) = 0;
    // Plane sub-object +72Ch slot +38h(), EAX (ECX = plane+72Ch).
    virtual std::uint32_t vcall_38_dword(void* object) = 0;
    // Gun slot +154h(a, b).
    virtual void vcall_154(void* gun, std::int32_t a, std::int32_t b) = 0;
    // Gun slot +1E8h(a).
    virtual void vcall_1e8(void* gun, std::int32_t a) = 0;
    // Gun slots +1C8h(message) and +1CCh(message) (opcodes 68h / 69h).
    virtual void vcall_1c8(void* gun, void* message) = 0;
    virtual void vcall_1cc(void* gun, void* message) = 0;
    // Gun slot +1DCh() (opcode ADh, the fire-if-ready slot).
    virtual void vcall_1dc(void* gun) = 0;
    // Path object slot +8h(const float* xyz): appends one point (007B34F0).
    virtual void vcall_08_append_point(void* path, const float* xyz) = 0;

    // ---- segment 3 / gun helpers ----
    // 00442A30 gun_records_resize, __thiscall(ECX = record array; count), RET 4.
    virtual void gun_records_resize_00442a30(void* records, std::uint32_t count) = 0;
    // 004420D0 device_class_destruct, __thiscall(ECX = class).
    virtual void device_class_destruct_004420d0(void* object) = 0;
    // 0043F860 BSP_Entity_HandleMessages4B4C, __thiscall(ECX = entity; message), RET 4, AL.
    virtual bool entity_handle_messages_4b4c_0043f860(void* entity, void* message) = 0;
    // 0043F080 BSP_UnitInstance_IsAliveAndVisible, __thiscall(ECX = unit), AL.
    virtual bool unit_is_alive_and_visible_0043f080(void* unit) = 0;
    // 0072AA10 BSP_GunEffectSet_SetMix, __thiscall(ECX = effect set; float), RET 4.
    virtual void gun_effect_set_set_mix_0072aa10(void* effects, float mix) = 0;
    // 0072D5B0 BSP_Gun_SelectFireMode, __thiscall(ECX = gun; mode, flag), RET 8.
    virtual void gun_select_fire_mode_0072d5b0(void* gun, std::uint32_t mode, std::uint32_t flag) = 0;
    // 00728000 BSP_Gun_ClearBotFireTarget, __thiscall(ECX = gun) (a tail jump in 00729F70).
    virtual void gun_clear_bot_fire_target_00728000(void* gun) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject, __thiscall(ECX = target record), EAX.
    virtual void* command_target_resolve_object_00521ea0(void* record) = 0;
    // 006952A0 BSP_Observer_UnregisterPair, __fastcall(ECX = observed, EDX = owner).
    virtual void observer_unregister_pair_006952a0(void* observed, void* owner) = 0;
    // 00734870 (unnamed narrative-list append), __thiscall(ECX = list; entry, NativeString*, 0), RET 0Ch.
    virtual void narrative_list_append_00734870(void* list, void* entry, void* text,
                                                std::uint32_t zero) = 0;

    // ---- session messages ----
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message; kind), RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t kind) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = entity; message, flags, arg), RET 0Ch.
    virtual void session_route_message_0077c2a0(void* entity, void* message, std::int32_t flags,
                                                std::int32_t arg) = 0;

    // ---- planes ----
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 007BC5B0 BSP_Plane_IsDownOrHumanHeld, __thiscall(ECX = plane), AL.
    virtual bool plane_is_down_or_human_held_007bc5b0(void* plane) = 0;
    // 007BC550 BSP_Plane_SyncElevatorAttachmentForState, __thiscall(ECX = plane).
    virtual void plane_sync_elevator_attachment_007bc550(void* plane) = 0;
    // 007C1430 BSP_Plane_SetFlightState, __thiscall(ECX = plane; state), RET 4.
    virtual void plane_set_flight_state_007c1430(void* plane, std::int32_t state) = 0;
    // 007D9C80 BSP_PlaneFlightController_BodyToWorldVelocity, __thiscall(ECX = plane+AB0h).
    virtual void flight_controller_body_to_world_velocity_007d9c80(void* controller) = 0;
    // 007DDEA0 BSP_PlaneSubBlock_ResetTimer40, __thiscall(ECX = plane+C98h).
    virtual void plane_sub_block_reset_timer40_007ddea0(void* block) = 0;
    // 007EB270 BSP_PlaneFxComponent_OnFlightState, __thiscall(ECX = plane+C98h; dword), RET 4.
    virtual void plane_fx_on_flight_state_007eb270(void* fx, std::uint32_t value) = 0;
    // 007EB380 BSP_PlaneFxComponent_Step, __thiscall(ECX = plane+C98h; float), RET 4.
    virtual void plane_fx_step_007eb380(void* fx, float step) = 0;
    // 0090F6C0 (unnamed; strings SA_OC / SA_IM, the achievement recorder),
    // __thiscall(ECX = [game+21A0h]; plane, kind), RET 8.
    virtual void achievement_record_0090f6c0(void* recorder, void* plane, std::int32_t kind) = 0;

    // ---- scene paths (007B34F0) ----
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = bag; const char* key), RET 4, EAX = entry.
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 00415870 STL_Vector_Size_00415870, __thiscall(ECX = path), EAX = point count.
    virtual std::int32_t vector_size_00415870(void* path) = 0;
    // 004F9A80 BSP_FormatString128, __cdecl(char* dst, const char* fmt, ...).
    virtual void format_string128_004f9a80(char* dst, const char* fmt, std::int32_t value) = 0;
    // 00BF6713 (the CRT invalid-parameter report behind the checked vector index).
    virtual void crt_invalid_parameter_00bf6713() = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(ECX = entity; out 14h LuaObject), RET 4.
    virtual void saved_entity_lua_data_009238a0(void* entity, void* out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(ECX = 14h reader; LuaObject by value), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_object_by_value) = 0;
    // 00BD8E20 BSP_LuaReader_EnterKey, __thiscall(ECX = reader; 8-byte {tag, text} by value), RET 8.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, std::uint32_t tag, const char* text) = 0;
    // 00BD7A20 BSP_LuaReader_LeaveKeyThunk, __thiscall(ECX = reader).
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    // 00441A20 BSP_LuaReader_DestroyStackInstance, __thiscall(ECX = reader).
    virtual void lua_reader_destroy_00441a20(void* reader) = 0;
    // Path slot +18h(reader*): loads the path from a saved-entity reader.
    virtual void vcall_18_load_from_reader(void* path, void* reader) = 0;
    // 007AF150 BSP_CameraPath_DeriveKnots, __fastcall(ECX = path).
    virtual void camera_path_derive_knots_007af150(void* path) = 0;

    // ---- plane flight controller (007DC830) ----
    // The float at an address of the Dynamics tuning mirror 00F872F0..00F873FC
    // (zero-filled .data past the raw size; filled at run time from Dynamics/*,
    // docs/PLANE_FLIGHT.md). `address` is the image address read.
    virtual float dynamics_mirror_float(std::uint32_t address) = 0;
    // 007DB680 BSP_PlaneFlight_CoreLaw, __thiscall(ECX = controller; float step,
    // float, float, int), RET 10h.
    virtual void plane_flight_core_law_007db680(void* controller, float step, float a, float b,
                                                std::int32_t c) = 0;
    // 007D99C0 BSP_PlaneFlight_ForwardSpeed, __thiscall(ECX = controller), ST0.
    virtual float plane_flight_forward_speed_007d99c0(void* controller) = 0;
    // 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), five stack floats, RET 14h, ST0.
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    // 007DAFD0 BSP_PlaneFlightController_ApplyForceAtPoint, __thiscall(ECX = controller;
    // const float* point, const float* direction, float force, float step, float k), RET 14h.
    virtual void flight_controller_apply_force_at_point_007dafd0(void* controller, const float* point,
                                                                 const float* direction, float force,
                                                                 float step, float k) = 0;
    // Plane slot +34h(float* out3), RET 4 (ECX = [controller+8h]).
    virtual void vcall_34_vector(void* plane, float* out) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __thiscall(ECX = float[3]), ST0.
    virtual float vector3_length_threshold_0042b2f0(const float* v) = 0;
};

// Segment 40 (guns).
void cc13_front3_gun_class_base_destruct_00732f50(void* gun_class, Cc13Front3Host& host);
void cc13_front3_mission_narrative_apply_mission_info_00734db0(void* message, Cc13Front3Host& host);
void cc13_front3_gun_set_controlled_audio_mix_0072b540(void* gun, bool controlled, Cc13Front3Host& host);
bool cc13_front3_gun_handle_network_message_0072d830(void* gun, void* message, Cc13Front3Host& host);
void cc13_front3_gun_bot_clear_fire_target_0072bd50(void* bot, Cc13Front3Host& host);
void cc13_front3_gun_release_player_control_00729f70(void* gun, Cc13Front3Host& host);

// Segment 47 (planes and scene paths).
void cc13_front3_scene_path_load_source_holder_007b34f0(void* path, void* holder, Cc13Front3Host& host);
void cc13_front3_plane_notify_flight_state_007c11e0(void* plane, bool immediate, Cc13Front3Host& host);
void cc13_front3_plane_on_water_contact_007cb7f0(void* plane, Cc13Front3Host& host);

// Segment 48 (plane flight controller).
void cc13_front3_plane_flight_free_flight_step_007dc830(void* controller, float step, Cc13Front3Host& host);

}  // namespace bsp
