#pragma once

// cc13_seg53: segment 53 (00851E10-008740E0: torpedo, turning gun, gunnery AI, effects) functions
// that had no reconstruction (docs/CC13_SEG53.md). New C++ interfaces over the image's Win32
// layouts; nothing here is wired into the game. Names are hypotheses, not recovered symbols.
// Every call the image makes is a named host hook invoked at the same point with the same
// arguments; the hook comment gives the callee's original ABI.

#include <cstddef>
#include <cstdint>
#include <cstdarg>

namespace bsp::cc13_seg53 {

using Object = std::uint8_t*;

// Raw field access over an image object (unaligned-safe).
template <typename T>
T& field(Object base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

// A tagged value as the Lua reader takes it by value (8 bytes): {tag, value}; tag 0 is a
// string pointer, 1 an int pointer, 10 (0Ah) the angle-float pointer used by 0085BB50.
struct TaggedValue {
    std::uint32_t tag;
    std::uintptr_t value;
};

// The 14h-byte LuaObject the saved-data getter fills; opaque here.
struct LuaObject {
    alignas(4) std::uint8_t bytes[0x14];
};

struct Seg53Host {
    virtual ~Seg53Host() = default;

    // --- shared runtime ---
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message, type), RET 4.
    virtual void session_message_construct_base_0075b430(Object message, std::uint32_t type) = 0;
    // 0075B480 / 0075B4C0 write / read the extended header, __thiscall(ECX = message, stream), RET 4.
    virtual void session_message_write_header_0075b480(Object message, Object stream) = 0;
    virtual void session_message_read_header_0075b4c0(Object message, Object stream) = 0;
    // 004290B0 BSP_BitCursor_WriteBoolean, __thiscall(ECX = stream, byte), RET 4.
    virtual void bit_write_bool_004290b0(Object stream, std::uint8_t value) = 0;
    // 00428D70 BSP_BitCursor_ReadBoolean, __thiscall(ECX = stream+4, byte* out), RET 4.
    virtual void bit_read_bool_00428d70(Object cursor, std::uint8_t* out) = 0;
    // 004295C0 BSP_BitCursor_WriteNumericFloat, __thiscall(ECX = stream, float value, a, b,
    // float range, bits), RET 14h.
    virtual void bit_write_float_004295c0(Object stream, float value, std::uint32_t a, std::uint32_t b,
                                          float range, std::uint32_t bits) = 0;
    // 004293F0 BSP_BitCursor_ReadNumericFloat, __thiscall(ECX = stream+4, float* out, a, b,
    // float range, bits), RET 14h.
    virtual void bit_read_float_004293f0(Object cursor, float* out, std::uint32_t a, std::uint32_t b,
                                         float range, std::uint32_t bits) = 0;
    // 00429090 BSP_BitCursor_WriteSignedDword, __thiscall(ECX = stream, value, bits), RET 8.
    virtual void bit_write_signed_00429090(Object stream, std::int32_t value, std::uint32_t bits) = 0;
    // 00428D30 BSP_BitCursor_ReadSignedDword, __thiscall(ECX = stream+4, int* out, bits), RET 8.
    virtual void bit_read_signed_00428d30(Object cursor, std::int32_t* out, std::uint32_t bits) = 0;
    // [message]->vtable[14h] / [18h](ECX = message, stream), RET 4: the message's body writer /
    // reader (00859CC0 / 00859FE0 for vtable 00D032B4).
    virtual void message_vslot_14(Object message, Object stream) = 0;
    virtual void message_vslot_18(Object message, Object stream) = 0;
    // float [00F876A8], the mission clock (read with MOVSS).
    virtual float mission_clock_00f876a8() = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle / 00438AA0 BSP_Math_AddWrappedAngle,
    // __stdcall(float a, float b), RET 8, result in ST0 (the callers FSTP it to a float).
    virtual float math_subtract_wrapped_angle_00438b10(float a, float b) = 0;
    virtual float math_add_wrapped_angle_00438aa0(float a, float b) = 0;
    // 00605070 BSP_Math_WrapAngleInPlace_Provisional, __thiscall(ECX = float*), RET 0.
    virtual void math_wrap_angle_in_place_00605070(float* angle) = 0;
    // 004B4D80 BSP_Lighting_DirectionFromAngles, __thiscall(ECX = float out[3], float a, float b),
    // RET 8 -> EAX = out.
    virtual const float* lighting_direction_from_angles_004b4d80(float* out, float a, float b) = 0;
    // 00927F30 BSP_UnitInstance_IsLocalPlayerRole, __thiscall(ECX = unit, int role), RET 4 -> AL.
    virtual bool unit_is_local_player_role_00927f30(Object unit, std::int32_t role) = 0;
    // 0085B0F0 BSP_TurningGun_SetAnglesImmediate, __thiscall(ECX = gun, float, float, float,
    // float), RET 10h.
    virtual void turning_gun_set_angles_immediate_0085b0f0(Object gun, float a, float b, float c,
                                                           float d) = 0;
    // 006952A0 BSP_Observer_UnregisterPair / 00694A60 BSP_Observer_RegisterPair,
    // __fastcall(ECX = observee, EDX = observer).
    virtual void observer_unregister_pair_006952a0(Object observee, Object observer) = 0;
    virtual void observer_register_pair_00694a60(Object observee, Object observer) = 0;
    // 00B6DFA0 BSP_Node_UnlinkAndRelease, __fastcall(ECX = node).
    virtual void node_unlink_and_release_00b6dfa0(Object node) = 0;
    // 00AF1980 BSP_NativeTraceline_Destroy, __thiscall(ECX = traceline).
    virtual void native_traceline_destroy_00af1980(Object traceline) = 0;
    // 00B748C0 BSP_NativeModelPool_ReturnCanonicalSlot, __thiscall(ECX = object).
    virtual void native_model_pool_return_slot_00b748c0(Object object) = 0;
    // 00B63F10 BSP_Matrix_BuildLookAt: ECX out, EDX eye, stack (target*, up.x, up.y, up.z),
    // RET 10h -> EAX = out.
    virtual Object matrix_build_look_at_00b63f10(Object out, const float* eye, const float* target,
                                                 float up_x, float up_y, float up_z) = 0;
    // 0047C2A0 BSP_Matrix_BuildFromEulerYXZ, __fastcall(ECX = out, EDX = float angles[3])
    // -> EAX = out.
    virtual Object matrix_build_from_euler_yxz_0047c2a0(Object out, const float* angles) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(ECX = destination, const source*), RET 4 -> EAX.
    virtual Object matrix_copy_4x4_004134f0(Object destination, Object source) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, __fastcall(ECX = float m[16]), in place.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float* m) = 0;
    // 00860140 BSP_DebugDraw_ArcInFrame, __fastcall(ECX = float frame[16], EDX = const float
    // rgba[4], read as four floats at 008602AB..; float radius, float start, float sweep,
    // segment parameter), RET 10h.
    virtual void debug_draw_arc_in_frame_00860140(const float* frame, const float* rgba,
                                                  float radius, float start, float sweep,
                                                  std::uint32_t segments) = 0;
    // Object [00E19994]: the ArtillerySubDirectorBot config table (28h-byte level rows).
    virtual Object artillery_bot_config_00e19994() = 0;
    // 00695870 BSP_CallbackOwner_Destroy, __fastcall(ECX = owner) (reached by a tail JMP).
    virtual void callback_owner_destroy_00695870(Object owner) = 0;
    // 0085C3F0 BSP_Vector3f_RotateAboutAxis, __thiscall(ECX = float v[3]; const float* axis,
    // float angle), RET 8: rotates v in place.
    virtual void vector3_rotate_about_axis_0085c3f0(float* v, const float* axis, float angle) = 0;
    // [entity]->vtable[5Ch](ECX = entity, kind), RET 4 -> AL: the entity kind test.
    virtual bool entity_is_kind_vslot_5c(Object entity, std::int32_t kind) = 0;
    // [object]->vtable[4](ECX = object, flags), RET 4: the scalar deleting destructor.
    virtual void object_delete_vslot_04(Object object, std::uint32_t flags) = 0;
    // 0041CC80 BSP_CriticalSection_DestroyOwned, __thiscall(ECX = section).
    virtual void critical_section_destroy_owned_0041cc80(Object section) = 0;
    // Pointer globals [00F87654] (effect deletion lock) / [00F87650] (effect manager) cleared
    // by the singleton holders' destructors.
    virtual void set_global_dword(std::uint32_t address, std::uint32_t value) = 0;
    // 0086B7E0 BSP_EffectComponent_Destruct / 0086AD50 BSP_NativeTracelinePayload_Destroy,
    // __thiscall(ECX = object).
    virtual void effect_component_destruct_0086b7e0(Object component) = 0;
    virtual void native_traceline_payload_destroy_0086ad50(Object payload) = 0;
    // 0086CF70, the scalar deleting destructor reached by the adjustor thunk 0086B9A0,
    // __thiscall(ECX = object, flags), RET 4 -> EAX.
    virtual Object scalar_deleting_dtor_0086cf70(Object object, std::uint32_t flags) = 0;
    // 00AF4130 BSP_ParticleSystem_ComputeLodFade, __thiscall(ECX = emitter; float x, y, z,
    // radius (raw dword), void* scratch), RET 14h -> ST0 (FSTP to float by the caller).
    virtual float particle_compute_lod_fade_00af4130(Object emitter, float x, float y, float z,
                                                     std::uint32_t radius_bits, void* scratch) = 0;
    // 00AF3F80 BSP_ParticleSystem_IsSphereVisible, __thiscall(ECX = emitter; float x, y, z,
    // radius (raw dword)), RET 10h -> AL.
    virtual bool particle_is_sphere_visible_00af3f80(Object emitter, float x, float y, float z,
                                                     std::uint32_t radius_bits) = 0;
    // 0086FCF0 BSP_ParticleResourceRecord_Destroy, __thiscall(ECX = record).
    virtual void particle_resource_record_destroy_0086fcf0(Object record) = 0;
    // 004C1650 BSP_GameplayEffectManager_GetSingleton() -> EAX; 00871750
    // BSP_GameplayEffectNameIndex_FindId, __thiscall(ECX = manager, name), RET 4 -> EAX.
    virtual Object gameplay_effect_manager_get_singleton_004c1650() = 0;
    virtual std::uint32_t gameplay_effect_name_index_find_id_00871750(Object manager,
                                                                      Object name) = 0;
    // 00BA9960 BSP_SkinedWaterTracer_SetAlpha, __thiscall(ECX = tracer, float), RET 4.
    virtual void skinned_water_tracer_set_alpha_00ba9960(Object tracer, float alpha) = 0;
    // 00BC04C0 BSP_LightningStorm_Destroy, __thiscall(ECX = storm); 00BF65AC CRT free (cdecl).
    virtual void lightning_storm_destroy_00bc04c0(Object storm) = 0;
    virtual void crt_free_00bf65ac(void* memory) = 0;
    // 004C3020 BSP_PooledStringList_AllocateSentinel, __thiscall(ECX = list) -> EAX.
    virtual Object pooled_string_list_allocate_sentinel_004c3020(Object list) = 0;

    // --- torpedo boat / torpedo ---
    // 00BF55BE operator new(size) (cdecl) -> EAX.
    virtual Object operator_new_00bf55be(std::uint32_t size) = 0;
    // 00857CD0 BSP_TorpedoBoat_Construct, __thiscall(ECX = memory, flag), RET 4 -> EAX.
    virtual Object torpedo_boat_construct_00857cd0(Object memory, std::uint32_t flag) = 0;
    // 009553D0 BSP_Unit_SetVehicleClass, __thiscall(ECX = unit, class), RET 4.
    virtual void unit_set_vehicle_class_009553d0(Object unit, Object vehicle_class) = 0;
    // 00B6DA70 scene node visibility factor, __thiscall(ECX = node, float value, byte recurse),
    // RET 8.
    virtual void scene_node_set_visibility_00b6da70(Object node, float value,
                                                    std::uint8_t recurse) = 0;
    // The two-bank net entity table globals (dword reads).
    virtual std::int32_t net_handle_split_00f89a10() = 0;
    virtual std::int32_t net_bank0_base_00f89a0c() = 0;
    virtual Object net_bank0_records_00f89a54() = 0;
    virtual std::int32_t net_bank1_base_00f89a60() = 0;
    virtual Object net_bank1_records_00f89aa8() = 0;

    // --- turning gun replication ---
    // 00779FC0 BSP_Entity_SendMessageToPeer, __thiscall(ECX = entity; peer, message*), RET 8.
    virtual void entity_send_message_to_peer_00779fc0(Object entity, Object peer,
                                                      Object message) = 0;
    // 00779F80 BSP_NetEntityHolder_StampSyncTick, __thiscall(ECX = entity; holder*), RET 4.
    virtual void net_entity_stamp_sync_tick_00779f80(Object entity, Object holder) = 0;
    // [object]->vtable[5Ch](ECX = object, 6) on gun+3Ch: entity_is_kind_vslot_5c above.

    // --- turning gun aiming ---
    // 005471B0 BSP_RefCountedPointerArray_Resize, __thiscall(ECX = {data, count, capacity},
    // int count), RET 4.
    virtual void ref_counted_pointer_array_resize_005471b0(Object array, std::int32_t count) = 0;
    // 007F6190 BSP_GunPlatform_SnapHeadingToFireWindow, __thiscall(ECX = platform; float heading,
    // float limit), RET 8 -> ST0 (kept as double: the caller returns it unrounded).
    virtual double gun_platform_snap_heading_007f6190(Object platform, float heading,
                                                      float limit) = 0;
    // 0085ABA0 BSP_TurningGun_SetTargetAngles, __thiscall(ECX = gun; float horz, float vert),
    // RET 8 -> AL (ignored by these callers).
    virtual bool turning_gun_set_target_angles_0085aba0(Object gun, float horz, float vert) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = pose).
    virtual void entity_pose_refresh_world_00414db0(Object pose) = 0;
    // 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse, __fastcall(ECX = out, EDX = src).
    virtual void matrix_build_affine_inverse_00b63d50(Object out, Object source) = 0;
    // 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize, ECX = out, EDX = source; stack
    // (matrix*, normalize byte), RET 8 -> EAX = out.
    virtual float* vector3_transform_direction_0042d0d0(float* out, const float* source,
                                                        Object matrix, std::uint8_t normalize) = 0;
    // 0042B260 BSP_Geometry_NormalizeVectorWithFloor, __fastcall(ECX = float v[3]), in place.
    virtual void geometry_normalize_with_floor_0042b260(float* v) = 0;
    // 0042CF10 BSP_Geometry_AsinClamped, __stdcall(float), RET 4 -> ST0 (FSTP to float).
    virtual float geometry_asin_clamped_0042cf10(float value) = 0;

    // --- saved-state Lua reader ---
    // 0072F060 BSP_Gun_LoadSavedState, __thiscall(ECX = gun).
    virtual void gun_load_saved_state_0072f060(Object gun) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(ECX = entity; LuaObject* out), RET 4.
    virtual void entity_get_saved_lua_data_009238a0(Object entity, LuaObject* out) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(ECX = reader; LuaObject by value), RET 14h.
    virtual void lua_reader_construct_004425c0(Object reader, LuaObject* data) = 0;
    // 00BD8E20 enter key / 00BD6830 read field, __thiscall(ECX = reader; tagged values by value);
    // 00BD7A20 leave key and 00441A20 destroy, __thiscall(ECX = reader).
    virtual void lua_reader_enter_key_00bd8e20(Object reader, TaggedValue key) = 0;
    virtual void lua_reader_read_field_00bd6830(Object reader, TaggedValue key,
                                                TaggedValue value) = 0;
    virtual void lua_reader_leave_key_00bd7a20(Object reader) = 0;
    virtual void lua_reader_destroy_00441a20(Object reader) = 0;
    // 0072ADC0 BSP_Gun_DebugDumpFields, __thiscall(ECX = gun; writer), RET 4.
    virtual void gun_debug_dump_fields_0072adc0(Object gun, Object writer) = 0;
    // Field writer vtable: [4](key) opens a section, [0Ch](key, value) writes one field (both
    // tagged values by value), [8]() closes the section; ECX = writer.
    virtual void writer_open_vslot_04(Object writer, TaggedValue key) = 0;
    virtual void writer_field_vslot_0c(Object writer, TaggedValue key, TaggedValue value) = 0;
    virtual void writer_close_vslot_08(Object writer) = 0;

    // --- weather / effects singletons ---
    // A writable image global (the function-static guard [00F875FC]).
    virtual std::uint32_t* global_dword_ptr(std::uint32_t address) = 0;
    // The image object at a fixed address (the rain descriptor 00F875C0).
    virtual Object global_object(std::uint32_t address) = 0;
    // 00BF6FF5 CRT atexit(function address) (cdecl).
    virtual void crt_atexit_00bf6ff5(std::uint32_t function) = 0;
    // 0041DD40 NativeString resize, __thiscall(ECX = string; size, flag), RET 8.
    virtual void native_string_resize_0041dd40(Object native_string, std::uint32_t size,
                                               std::int32_t flag) = 0;
    // 004C9550 BSP_LiveEffectReferences_Resize, __thiscall(ECX = vector; count), RET 4.
    virtual void live_effect_references_resize_004c9550(Object vector, std::uint32_t count) = 0;
    // 008671A0 BSP_EffectManager_FlushPendingDeletes, __thiscall(ECX = manager) (tail JMP).
    virtual void effect_manager_flush_pending_deletes_008671a0(Object manager) = 0;
    // [[[00E188A8]+1A08h]+4]: the Lua machine; 006B8AD0 BSP_LuaMachine_RunString,
    // __thiscall(ECX = machine; const char*, a, b, c), RET 10h.
    virtual Object lua_machine_00e188a8_1a08_4() = 0;
    virtual void lua_machine_run_string_006b8ad0(Object machine, const char* text, std::uint32_t a,
                                                 std::uint32_t b, std::uint32_t c) = 0;

    // --- gunnery director bridge ---
    // [entity]->vtable[140h](ECX = entity) -> EAX; [object]->vtable[114h](ECX = object) -> EAX.
    virtual Object entity_vslot_140(Object entity) = 0;
    virtual Object object_vslot_114(Object object) = 0;
    // 00BF681B operator new(size) (cdecl, ADD ESP,4) -> EAX.
    virtual Object operator_new_00bf681b(std::uint32_t size) = 0;
    // 008624C0 BSP_UnitGunneryDirectorBridge_Apply, __thiscall(ECX = bridge, byte force), RET 4.
    virtual void director_bridge_apply_008624c0(Object bridge, std::uint8_t force) = 0;

    // --- engine arrays {data, count, capacity} ---
    // 0086CE10 BSP_EffectNamedRecord14Array_Reserve / 0086E6F0 _Resize, __thiscall(ECX = array,
    // int), RET 4.
    virtual void named_record14_array_reserve_0086ce10(Object array, std::int32_t capacity) = 0;
    virtual void named_record14_array_resize_0086e6f0(Object array, std::int32_t count) = 0;
    // 00BF6989 CRT free variant (cdecl).
    virtual void crt_free_00bf6989(void* memory) = 0;
    // 00870C10 BSP_ParticleResourceRecord_Assign, __thiscall(ECX = destination; source), RET 4.
    virtual void particle_resource_record_assign_00870c10(Object destination, Object source) = 0;
    // 00AF6790 BSP_ParticleModel_SetEmitterValue, __thiscall(ECX = model; a, float value), RET 8.
    virtual void particle_model_set_emitter_value_00af6790(Object model, std::uint32_t a,
                                                           float value) = 0;

    // --- recon category names ---
    // char* [00E0B590 + 4*index], the 19-entry recon category name table ("mothership", ...).
    virtual const char* recon_category_name_00e0b590(std::int32_t index) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive, __thiscall(ECX = string; const char*),
    // RET 4 -> AL.
    virtual bool native_string_equals_insensitive_00425850(Object native_string,
                                                           const char* text) = 0;
    // [00CE2220] InterlockedDecrement (IAT, stdcall) -> the new count; [object]->vtable[0](ECX =
    // object): the refcounted object's release-to-zero slot.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    virtual void refcounted_vslot_00(Object object) = 0;

    // --- gun fire window ---
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __thiscall(ECX = node).
    virtual void transform_refresh_world_matrix_00b6db70(Object node) = 0;
    // 00521370 BSP_Direction_ToPitchYaw, __fastcall(ECX = const float* direction, EDX = float*
    // pitch; float* yaw), RET 4.
    virtual void direction_to_pitch_yaw_00521370(const float* direction, float* pitch,
                                                 float* yaw) = 0;
    // 007F60A0 BSP_GunPlatform_AnglesInFireWindow, __thiscall(ECX = platform; float horz,
    // float vert), RET 8 -> AL.
    virtual bool gun_platform_angles_in_fire_window_007f60a0(Object platform, float horz,
                                                             float vert) = 0;

    // --- debug renderer [00F8D394] ---
    // [r]->vtable[B4h](ECX = r; const float* p0, const float* p1, ARGB c0, ARGB c1), RET 10h.
    virtual void debug_renderer_line_vslot_b4(const float* p0, const float* p1, std::uint32_t c0,
                                              std::uint32_t c1) = 0;
    // [r]->vtable[C0h](ECX = r; const float* a (2 floats), const float* b, ARGB, ARGB), RET 10h.
    virtual void debug_renderer_screen_line_vslot_c0(const float* a, const float* b,
                                                     std::uint32_t c0, std::uint32_t c1) = 0;
};

// --- 00851EE0 / 00852140: the fixed string builder (+4 buffer, +8 capacity, +0Ch length) ---
void fixed_string_builder_append_bytes_00851ee0(Seg53Host& host, Object builder, const void* src,
                                                std::size_t n);
Object fixed_string_builder_append_native_string_00852140(Seg53Host& host, Object builder,
                                                          const std::uint32_t* native_string);

// 00855FE0 observer link at +14h; returns the link.
Object observer_link14_assign_00855fe0(Seg53Host& host, Object link, Object target);
// 008580C0 Node** handle: release and clear.
void node_handle_unlink_and_clear_008580c0(Seg53Host& host, Object* handle);
// 00858330 the traceline scalar-deleting destructor that returns the slot to the model pool.
Object native_traceline_scalar_delete_to_pool_00858330(Seg53Host& host, Object traceline,
                                                       std::uint8_t flags);
// 00858180 / 008581B0 / 00858200 bare RET 4 / RET 8 / RET slots; 008581E0 returns false.
void noop_ret4_00858180(std::uint32_t);
void noop_ret8_008581b0(std::uint32_t, std::uint32_t);
void noop_00858200();
bool return_false_008581e0();

// --- T_GUN_TURN_SYNC (session message 0AEh, vtable 00D032B4) ---
Object gun_turn_sync_construct_00859410(Seg53Host& host, Object message, Object record,
                                        std::uint8_t mode, float a28, float a2c, float a20,
                                        float a24, std::int32_t a30);
void gun_turn_sync_write_00859c80(Seg53Host& host, Object message, Object stream);
void gun_turn_sync_write_angle_deltas_00859cc0(Seg53Host& host, Object message, Object stream);
void gun_turn_sync_read_00859fa0(Seg53Host& host, Object message, Object stream);
void gun_turn_sync_read_angle_deltas_00859fe0(Seg53Host& host, Object message, Object stream);

// --- turning gun / unit aim state ---
void unit_set_gun_aim_record_00859830(Seg53Host& host, Object unit, float a4a4, float a4a8,
                                      float a4ac, float angle_b, float angle_a);
void turning_gun_commit_step_angles_00859920(Seg53Host& host, Object gun);
void turning_gun_apply_snapshot_fields_0085ba30(Seg53Host& host, Object gun, Object message);

// --- matrix wrappers ---
void matrix_set_look_at_copy_0085c0f0(Seg53Host& host, Object matrix, const float* eye,
                                      const float* target, const float* up);
Object matrix_set_from_euler_yxz_copy_0085c220(Seg53Host& host, Object matrix,
                                               const float* angles);

// --- debug draw ---
// 008520D0 ECX text buffer ({vtable,data+4,capacity+8,length+C}), stack
// format/va_list, RET8. Current linked CRT is an explicit vendor boundary.
// Buffer allocation must include the trailing NUL beyond native capacity.
void text_buffer_append_format_008520d0(void* buffer, const char* format, va_list arguments);
struct GeometrySquaredDistanceCalls {
    virtual ~GeometrySquaredDistanceCalls() = default;
    virtual double sphere_triangle_distance_0085df60(const float* center, const float* a,
        const float* b, const float* c) = 0;
};
// 0085E4B0 ECX center, EDX a, stack b/c, RET8 -> ST0 after FSTP float.
float sphere_triangle_distance_root_0085e4b0(const float* center, const float* a,
    const float* b, const float* c, GeometrySquaredDistanceCalls&);
struct DebugArcArithmeticCalls {
    virtual ~DebugArcArithmeticCalls() = default;
    // Exact original BF7420 low-EAX conversion and FSIN/FCOS + FSTP float
    // are provider contracts. No substitute numeric library is given ABI credit.
    virtual std::int32_t truncate_low_dword_00bf7420(double value) = 0;
    virtual float sine_x87(float value) = 0;
    virtual float cosine_x87(float value) = 0;
};
// 00860340 ECX center, EDX sine basis; stack cosine basis,start,sweep,rgba,
// signed density; RET14h. Every original float spill is retained in source.
void debug_draw_basis_arc_00860340(Seg53Host&, const float* center, const float* sine_basis,
    const float* cosine_basis, float start, float sweep, const float* rgba,
    std::int32_t density, DebugArcArithmeticCalls&);
// 00865F50 ECX owner+90h -> 004134F0, stack matrix, RET4 -> EAX.
Object effect_copy_matrix_00865f50(Seg53Host&, Object owner, Object matrix);
void debug_draw_line_rgba_floats_0085f990(Seg53Host& host, const float* p0, const float* p1,
                                          float r0, float g0, float b0, float a0, float r1,
                                          float g1, float b1, float a1);
void debug_draw_line_rgba_floats_one_colour_0085fb30(Seg53Host& host, const float* p0,
                                                     const float* p1, float r, float g, float b,
                                                     float a);
void debug_draw_screen_line_rgba_floats_00860030(Seg53Host& host, const float* rgba, float x0,
                                                 float y0, float x1, float y1);
void debug_draw_arc_around_axis_00860ad0(Seg53Host& host, const float* centre, const float* axis,
                                         float radius, float start, float sweep,
                                         const float* rgba, std::uint32_t segments);
void debug_draw_circle_around_axis_00860bb0(Seg53Host& host, const float* centre,
                                            const float* axis, float radius, const float* rgba,
                                            std::uint32_t segments);

// --- the gunnery director bridge (+4 = weapon director) byte getters ---
std::uint8_t director_bridge_get_allow_fire_00861970(Object bridge);       // director+3Ch
std::uint8_t director_bridge_get_torpedo_enabled_00861980(Object bridge);  // director+222h
std::uint8_t director_bridge_get_artillery_enabled_00861990(Object bridge);  // +220h
std::uint8_t director_bridge_get_aa_enabled_008619a0(Object bridge);       // director+221h
std::uint8_t director_bridge_get_depth_charge_enabled_008619b0(Object bridge);  // +223h
std::uint8_t director_bridge_get_allow_move_008619c0(Object bridge);       // director+3Dh

// --- ArtillerySubDirectorBot level-row float getters ([00E19994] + 28h*[bot+14h] + offset) ---
float artillery_bot_get_max_error_radius_00861a10(Seg53Host& host, Object bot);        // +0Ch
float artillery_bot_get_error_range_mul_00861a30(Seg53Host& host, Object bot);         // +10h
float artillery_bot_get_min_error_mul_00861a50(Seg53Host& host, Object bot);           // +14h
float artillery_bot_get_approach_mul_min_00861a70(Seg53Host& host, Object bot);        // +18h
float artillery_bot_get_approach_mul_max_00861a90(Seg53Host& host, Object bot);        // +1Ch
float artillery_bot_get_deviation_mul_00861ab0(Seg53Host& host, Object bot);           // +20h
float artillery_bot_get_angle_change_00861ad0(Seg53Host& host, Object bot);            // +24h
float artillery_bot_get_error_dist_inc_start_time_00861af0(Seg53Host& host, Object bot);  // +28h
float artillery_bot_get_error_dist_inc_full_time_00861b10(Seg53Host& host, Object bot);   // +2Ch

// --- gunnery target record ---
void gunnery_target_record_destruct_00861b30(Seg53Host& host, Object record);
void gunnery_target_record_on_observee_destroyed_00861b40(Object record, Object observee);
void gunnery_target_record_reset_aim_offset_008626d0(Seg53Host& host, Object record);
Object entity_as_kind02_or_null_00863620(Seg53Host& host, Object entity);
void unit_gunnery_ai_vslot60_noop_00864510(std::uint32_t);

// --- effects ---
void node_handle_release_008658c0(Seg53Host& host, Object* handle);
void owned_object_delete_if_set_00865e30(Seg53Host& host, Object* owner);
void effect_deletion_lock_holder_dtor_00866760(Seg53Host& host, Object holder);
void effect_manager_holder_dtor_008669a0(Seg53Host& host, Object holder);
bool particle_admit_at_position_00869d70(Seg53Host& host, Object system, const float* position,
                                         std::uint32_t radius_bits);
bool effect_component_slot0c_false_0086b7c0();
Object effect_tracer_adjustor_thunk_0086b9a0(Seg53Host& host, Object sub_object,
                                             std::uint32_t flags);
void effect_tracer_destruct_0086b9b0(Seg53Host& host, Object tracer);
Object pooled_string_list_owner_construct_0086fc90(Seg53Host& host, Object owner);
void particle_resource_record_destroy_if_set_0086fd90(Seg53Host& host, Object records);
std::uint32_t gameplay_effect_find_id_by_name_00871a90(Seg53Host& host, Object name);
void registered_type4_effect_set_alpha_00872080(Seg53Host& host, Object effect, float alpha);
std::uint32_t effect_node_is_nonpositive20_008721a0(Object node);
void registered_model_effect_set_value1dc_00872770(Object effect, float value);
void lightning_storm_owner_reset_00872e80(Seg53Host& host, Object* owner);

// --- torpedo boat / torpedo projectile ---
std::uint32_t torpedo_boat_class_recon_category_00857d60();
Object torpedo_boat_class_create_entity_00857e20(Seg53Host& host, Object vehicle_class,
                                                 std::uint32_t flag);
void torpedo_projectile_show_node_00855b90(Seg53Host& host, Object torpedo, std::uint32_t unused);
Object torpedo_create_message_resolve_target_00855bd0(Seg53Host& host, Object message);

// --- turning gun replication ---
void turning_gun_read_recent_shot_008598b0(Seg53Host& host, Object gun, Object out);
void turning_gun_write_aim_snapshot_fields_00859a20(Seg53Host& host, Object gun, Object* holder,
                                                    std::int32_t count);
void turning_gun_write_snapshot_fields_00859b10(Seg53Host& host, Object gun, Object message,
                                                Object writer);

// --- turning gun aiming ---
double turning_gun_filter_heading_to_traverse_0085ab50(Seg53Host& host, Object gun, float heading,
                                                       float limit);
void turning_gun_aim_to_rest_angles_0085ad00(Seg53Host& host, Object gun);
bool gun_target_in_fire_window_0085a9a0(Seg53Host& host, Object gun, Object target,
                                        float* direction, float* yaw, float* pitch);
void turning_gun_aim_at_world_direction_0085b980(Seg53Host& host, Object gun,
                                                 const float* direction);

void turning_gun_load_instance_state_0085bb50(Seg53Host& host, Object gun);
void turning_gun_debug_dump_fields_0085a130(Seg53Host& host, Object gun, Object writer);

// --- weather / effects ---
void world_atmosphere_release_node_008658e0(Seg53Host& host, Object descriptor);
Object rain_descriptor_construct_00865b80(Seg53Host& host, Object descriptor);
Object rain_descriptor_get_singleton_00865cf0(Seg53Host& host);
void effect_manager_clear_live_references_008674a0(Seg53Host& host, Object manager);
void effects_clear_lua_table_00869d20(Seg53Host& host);
void flag_grid_set_cell_00861c20(Object grid, std::int32_t column, std::int32_t row,
                                 std::uint8_t value);

void unit_gunnery_ai_create_director_bridge_00863a80(Seg53Host& host, Object ai);

// --- effect component category getters (vtable slots; MOV EAX,n; RET) ---
std::uint32_t effect_tracer_get_category_0086b990();          // 3
std::uint32_t effect_particle_get_category_0086bc50();        // 1
std::uint32_t effect_shake_get_category_0086bd60();           // 6
std::uint32_t effect_const_rumble_get_category_0086cfc0();    // 6
std::uint32_t effect_slope_rumble_get_category_0086d010();    // 6
std::uint32_t effect_square_rumble_get_category_0086d060();   // 6
void effect_named_record14_array_append_0086e670(Seg53Host& host, Object array, Object source);
void effect_named_record14_array_destroy_0086f900(Seg53Host& host, Object array);
void particle_resource_records_erase_unordered_00871390(Seg53Host& host, Object array,
                                                        const Object* it);
void registered_model_effect_set_emitter_value_00871ff0(Seg53Host& host, Object effect,
                                                        std::uint32_t a, float value);
// 0086CDB0 the 14h-byte named record {float, float, NativeString, dword} copy constructor.
Object effect_named_record14_copy_construct_0086cdb0(Seg53Host& host, Object record,
                                                     Object source);

// --- the engine's intrusive RefPtr<T> (refcount at object+4) and node handles ---
// 00866730, 0086AC50, 0086AC80, 0086ACB0, 0086ACE0, 00873AB0 (one body): destruct -> this.
Object ref_ptr_destruct(Seg53Host& host, Object* ref);
// 00866970, 00866C30, 0086B080, 0086B750 (one body): release, no return value.
void ref_ptr_release(Seg53Host& host, Object* ref);
// 0086AFC0: release the old referent and store a raw pointer without adding a reference.
Object* ref_ptr_attach_0086afc0(Seg53Host& host, Object* ref, Object raw);
// 00872220, 00872240, 00872260 (one body, = 008658C0): node handle release.
void node_ref_release(Seg53Host& host, Object* handle);

// --- recon categories (NativeString {size, ptr}) ---
std::int32_t recon_category_index_from_name_0085bce0(Seg53Host& host, Object name);
std::int32_t recon_category_from_class_type_name_0085bd50(Seg53Host& host, Object name);

// --- unit gunnery AI group switches (category mask words at +80h + 4*category) ---
void unit_gunnery_ai_set_artillery_group_allowed_00861cd0(Seg53Host& host, Object ai, std::uint8_t allowed);
void unit_gunnery_ai_set_aa_group_allowed_00861d20(Seg53Host& host, Object ai, std::uint8_t allowed);
void unit_gunnery_ai_set_torpedo_group_allowed_00861d70(Seg53Host& host, Object ai, std::uint8_t allowed);
void unit_gunnery_ai_set_depth_charge_group_allowed_00861dc0(Seg53Host& host, Object ai,
                                                             std::uint8_t allowed);

}  // namespace bsp::cc13_seg53
