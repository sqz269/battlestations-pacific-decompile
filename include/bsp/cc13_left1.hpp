// Leftover game bodies with no host (packet cc13-left1, docs/CC13_LEFT1.md).
//
// 005B8810 builds the 30h-byte ambient-message voice object and starts its sound; 00605630 is
// the plane HUD's copy of the direction angle deltas (the same arithmetic as the path camera's
// 00794750, src/cc13_camera_controllers.cpp, with a different ABI); 006D5220 is the airfield's
// hangar and marker property reader (scene bag, compiled table or saved Lua data).
//
// Model: the bodies work on the original objects as raw bytes (Win32 layout, offsets as in the
// image) and reach the engine through Left1Host, whose methods name the image routine they stand
// for. New C++ interfaces, not drop-in binary replacements; nothing here is bound into bsp_game.
// Names are hypotheses, not recovered symbols. Marker: cc13_left1.

#ifndef BSP_CC13_LEFT1_HPP
#define BSP_CC13_LEFT1_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_left1 {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;          // [00E188A8], the game
inline constexpr std::uint32_t kAmbientObserverVtable = 0x00CE75CCu;
inline constexpr std::uint32_t kEmptyText = 0x00E1952Eu;           // the image's "" (a dword arg)
inline constexpr double kAmbientFadeDistance = 2000.0;             // double 00CF0DD8
inline constexpr float kAmbientMinimumVolume = 0.25f;              // float 00CE3868

// The 0Ch clip element passed by value (VoiceClipRef in engine_seg49_64_78_cc13.hpp: the
// element's vtable word, the clip record, the line index). Only the record and the index are
// read here.
struct AmbientClip12 {
    std::uint32_t vtable_word;
    void* record;            // record+4: the clip's text pointer (null: the image prints "")
    std::int32_t index;
};

struct Left1Host {
    virtual ~Left1Host() = default;
    virtual void* game_global() = 0;  // [00E188A8]
    // 006952A0 BSP_Observer_UnregisterPair / 00694A60 BSP_Observer_RegisterPair,
    // __fastcall(ECX = observed entity, EDX = observer).
    virtual void observer_unregister_pair_006952a0(void* target, void* observer) = 0;
    virtual void observer_register_pair_00694a60(void* target, void* observer) = 0;
    // 00702CC0 BSP_VoiceSlot_ConstructAndStart, __thiscall(ECX = fresh 18h slot; clip*, owned
    // resource by value), RET 8; EAX = slot.
    virtual void* voice_slot_construct_and_start_00702cc0(void* slot, const AmbientClip12* clip,
                                                          std::uint32_t owned_resource) = 0;
    // 005B82C0 BSP_VoiceSlot_Assign, __thiscall(ECX = destination; source*), RET 4.
    virtual void voice_slot_assign_005b82c0(void* destination, void* source) = 0;
    // 005B7FC0 BSP_VoiceSlot_Destroy, __fastcall(ECX = slot).
    virtual void voice_slot_destroy_005b7fc0(void* slot) = 0;
    // 007027B0 BSP_VoiceSlot_PollState, __fastcall(ECX = slot); EAX.
    virtual std::int32_t voice_slot_poll_state_007027b0(void* slot) = 0;
    // 00702130 BSP_VoiceSlot_SetVolume, __thiscall(ECX = slot; float), RET 4.
    virtual void voice_slot_set_volume_00702130(void* slot, float volume) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __fastcall(ECX = transform).
    virtual void transform_refresh_world_matrix_00b6db70(void* transform) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __fastcall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __thiscall(ECX = float[3]); ST0.
    virtual float vector3_length_threshold_0042b2f0(const float* v) = 0;
    // 004254B0, __cdecl(format, ...): a trivial body (the release build's compiled-out trace).
    // The pushed argument dwords are handed over in stack order.
    virtual void trace_printf_004254b0(const char* format, const std::uint32_t* args,
                                       std::uint32_t count) = 0;

    // --- 006D5220 ---
    virtual std::uint32_t image_dword(std::uint32_t address) = 0;
    virtual void invalid_parameter_00bf6713() = 0;  // returns here
    // 0095E5B0 BSP_Unit_ReadCommonLuaProperties, __fastcall(ECX = unit).
    virtual void unit_read_common_lua_properties_0095e5b0(void* unit) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(ECX = entity; out 14h LuaObject), RET 4.
    virtual void saved_entity_lua_data_009238a0(void* entity, void* out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(ECX = 14h reader; LuaObject by value), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_object_by_value) = 0;
    // The reader's {tag, word} variants: tag 0 = C string, 1 = int, 2 = float*, 4 = object ref*.
    // 00BD8E20 BSP_LuaReader_EnterKey (key), RET 8; 00BD7A20 BSP_LuaReader_LeaveKeyThunk, RET;
    // 00BD5EB0 BSP_LuaReader_HasKey (key), RET 8, AL; 00BD6830 BSP_LuaReader_ReadField (key,
    // field), RET 10h. All __thiscall(ECX = reader).
    virtual void lua_reader_enter_key_00bd8e20(void* reader, std::uint32_t tag, std::uint32_t word) = 0;
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    virtual bool lua_reader_has_key_00bd5eb0(void* reader, std::uint32_t tag, std::uint32_t word) = 0;
    virtual void lua_reader_read_field_00bd6830(void* reader, std::uint32_t key_tag, std::uint32_t key_word,
                                                std::uint32_t field_tag, std::uint32_t field_word) = 0;
    // The reader's virtual slots called through [reader]: +4h enter (key), RET 8; +8h leave, RET;
    // +10h read (key, field), RET 10h (AL unused here).
    virtual void reader_vslot_04(void* reader, std::uint32_t tag, std::uint32_t word) = 0;
    virtual void reader_vslot_08(void* reader) = 0;
    virtual void reader_vslot_10(void* reader, std::uint32_t key_tag, std::uint32_t key_word,
                                 std::uint32_t field_tag, std::uint32_t field_word) = 0;
    // 00441210 BSP_LuaObjectVector_Tidy, __fastcall(ECX = reader+4).
    virtual void lua_object_vector_tidy_00441210(void* vector) = 0;
    // 006BF0D0 BSP_AirOpsBlock_SetRunwayDimensions, __thiscall(ECX = block; width, length), RET 8.
    virtual void air_ops_block_set_runway_dimensions_006bf0d0(void* block, float width, float length) = 0;
    // 00BF55BE operator new, __cdecl(bytes); 00BF6989 _free, __cdecl(block).
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // 007AC9D0 BSP_Entity_PathInterfaceForKind, __fastcall(ECX = entity); EAX.
    virtual void* entity_path_interface_007ac9d0(void* entity) = 0;
    // 00836510 BSP_WeaponDirector_VisitEnables, __thiscall(ECX = director; reader), RET 4.
    virtual void weapon_director_visit_enables_00836510(void* director, void* reader) = 0;
    // 006CFA80 BSP_AirFieldSite_Create, __fastcall(ECX = airfield); EAX.
    virtual void* airfield_site_create_006cfa80(void* airfield) = 0;
    // 006BA690 TRIV_body_006ba690, __thiscall(ECX = block; site), RET 4.
    virtual void air_ops_block_trivial_006ba690(void* block, void* site) = 0;
    // 006BEF30 BSP_AirOpsBlock_ReadTrafficTable, __thiscall(ECX = block; reader), RET 4.
    virtual void air_ops_block_read_traffic_table_006bef30(void* block, void* reader) = 0;
    // 006BA670 BSP_AirOpsBlock_ForwardToTraffic08, __thiscall(ECX = block; float dt), RET 4.
    virtual void air_ops_block_tick_006ba670(void* block, float dt) = 0;
    // 006D2980 BSP_AirField_RebuildAimPoints, __fastcall(ECX = airfield).
    virtual void airfield_rebuild_aim_points_006d2980(void* airfield) = 0;
    // 006BEE40 BSP_AirOpsHolder_CopyOwnerFrame, __thiscall(ECX = holder; float dt), RET 4.
    virtual void air_ops_holder_copy_owner_frame_006bee40(void* holder, float dt) = 0;
    // 006D2190 BSP_ScenePosition_FromNamedEntity, __fastcall(ECX = out vec3, EDX = bag; name,
    // const float* fallback), RET 8; EAX = out.
    virtual float* scene_position_from_named_entity_006d2190(float* out, void* bag, const char* name,
                                                             const float* fallback) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX = in; out, matrix), RET 8.
    virtual void transform_affine_point_004142e0(const float* in, float* out, const void* matrix) = 0;
    // 00414E10 BSP_EntityPose_GetDerivedAffineInverse, __fastcall(ECX = entity); EAX = matrix.
    virtual const void* entity_pose_derived_affine_inverse_00414e10(void* entity) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(ECX = destination; source), RET 4.
    virtual void matrix_copy_4x4_004134f0(void* destination, const float* source) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, __fastcall(ECX = matrix).
    virtual void matrix_orthonormalize_basis_rows_0085dc80(void* matrix) = 0;
    // 00467310 BSP_FormatToBuffer32, __cdecl(buffer, format, ...): here one int.
    virtual void format_to_buffer32_00467310(char* buffer, const char* format, std::int32_t value) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = bag; name), RET 4; EAX (null when absent).
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* name) = 0;
    // 008F0DF0 BSP_SceneProperty_GetStringValue, __fastcall(ECX = property); EAX.
    virtual const char* scene_property_string_value_008f0df0(void* property) = 0;
    // 006D2120 BSP_Entity_NthPartOfType0C, __thiscall(ECX = entity; n), RET 4; EAX.
    virtual void* entity_nth_part_of_type_0c_006d2120(void* entity, std::int32_t n) = 0;
    // 00925A90 BSP_EntityRegistry_FindEntityByName, __thiscall(ECX = registry; name), RET 4; EAX.
    virtual void* entity_registry_find_by_name_00925a90(void* registry, const char* name) = 0;
    // [[entity]+5Ch], __thiscall(ECX = entity; kind), RET 4; AL.
    virtual bool entity_vslot_5c_is_kind(void* entity, std::uint32_t kind) = 0;
    // 006D3440 BSP_Entity_AsKind1B, __fastcall(ECX = entity); EAX.
    virtual void* entity_as_kind1b_006d3440(void* entity) = 0;
    // 00744A20 BSP_LandFort_SetMaster, __thiscall(ECX = fort; master, part reference), RET 8.
    virtual void land_fort_set_master_00744a20(void* fort, void* master, void* part) = 0;
    // NativeString {length +0h, data +4h}: 0041E870 Assign, __thiscall(ECX = string; text), RET 4,
    // EAX = string; 004261A0 Concat, __thiscall(ECX = lhs; dst, rhs), RET 8, EAX = dst; 004263B0
    // ConcatInt, __thiscall(ECX = source; out, value), RET 8, EAX = out.
    virtual void* native_string_assign_0041e870(void* string, const char* text) = 0;
    virtual void* native_string_concat_004261a0(void* lhs, void* dst, void* rhs) = 0;
    virtual void* native_string_concat_int_004263b0(void* source, void* out, std::int32_t value) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton (data, bytes, flag pushed for 00BD1510);
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool, data, bytes, flag), RET 0Ch.
    virtual void* sized_pool_singleton_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::uint32_t flag) = 0;
};

// 005B8810 BSP_AmbientMessage_PlaySound, __thiscall(ECX = object; AmbientClip12 by value,
// entity*), RET 10h; EAX = object. SEH frame, handler 00C7215B.
void* ambient_message_voice_construct_005b8810(Left1Host& host, void* object, AmbientClip12 clip,
                                               void* entity);

// 00605630 BSP_Math_DirectionAngleDeltas, __fastcall(ECX = float* yaw, EDX = float* pitch;
// a.xyz, b.xyz by value), RET 18h.
void direction_angle_deltas_00605630(float* yaw, float* pitch, const float a[3], const float b[3]);

// 006D5220 BSP_AirField_ReadHangarAndMarkerProperties, __fastcall(ECX = airfield), RET; SEH
// handler 00C81EEF. MAirfield vtable slot +0A4h. The scene record [airfield+0C0h] kind (+4h)
// picks the source: 1 the scene property bag (markers, "Hangar %d", the part sections), 2 the
// compiled table at [record+8]+138h/+13Ch, 3 the saved Lua data ("_airField", "hangars",
// "gameUnit"); then the common tail (site, "_airBase" for kind 3, the air-ops ticks).
void airfield_read_hangar_and_marker_properties_006d5220(Left1Host& host, void* airfield);

}  // namespace bsp::cc13_left1

#endif  // BSP_CC13_LEFT1_HPP
