#pragma once

// cc13_seg52b: the second pass over segment 52 (008288D0-00851CB0: ship units, shipyard,
// stationaries) after cc13_seg52 (docs/CC13_SEG52B.md), excluding the struck cc9 ranges
// 00836000-00837FFF and 0084E000-0084EFFF. New C++ interfaces over the image's Win32 layouts;
// nothing here is wired into the game. Names are hypotheses, not recovered symbols. Every call
// the image makes is a named host hook invoked at the same point with the same arguments.

#include "bsp/cc13_seg52.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg52b {

using cc13_seg52::Object;
using cc13_seg52::field;

// Seg52bHost extends the first pass's host so this file can call cc13_seg52 functions and its
// hooks (NativeString resize, the sized pool, Interlocked*, the refcounted vtable[0] slot).
struct Seg52bHost : cc13_seg52::Seg52Host {
    // 00BF7289 _vsprintf_s(char* dest, size_t size, const char* format, va_list).
    virtual int vsprintf_s_00bf7289(char* dest, std::size_t size, const char* format,
                                    std::va_list args) = 0;
    // 00829180 BSP_ShipIndexedPointArray_ReserveCapacity, __thiscall(ECX = array; int count),
    // RET 4.
    virtual void ship_indexed_point_array_reserve_00829180(Object array, std::int32_t count) = 0;
    // 008484A0, the stock record's name vector copy construct, __thiscall(ECX = dest vector;
    // const source vector*), RET 4.
    virtual void stock_name_vector_copy_008484a0(Object dest, Object source) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report (does not return normally).
    virtual void invalid_parameter_00bf6713() = 0;
    // [object]->vtable[0B0h](ECX = object) -> EAX.
    virtual Object object_vslot_b0(Object object) = 0;
    // 0042E630 BSP_SpatialIndex_GetSingleton() -> EAX; 0098B370 BSP_SpatialIndex_SweepSegments,
    // __thiscall(ECX = index; segments, a, b), RET 0Ch -> EAX.
    virtual Object spatial_index_get_singleton_0042e630() = 0;
    virtual std::uint32_t spatial_index_sweep_segments_0098b370(Object index, Object segments,
                                                                Object a, Object b) = 0;
    // 0082CAD0, the vector tidy of the 0082DC30 holder's +8 member, __thiscall(ECX = vector);
    // 00BD30F0 BSP_RefCounted_DestroyBase, __fastcall(ECX = object).
    virtual void holder_vector_tidy_0082cad0(Object vector) = 0;
    virtual void refcounted_destroy_base_00bd30f0(Object object) = 0;
    // 00438E10 BSP_CString_CompareInsensitive, __fastcall(ECX = a, EDX = b) -> EAX (0 = equal).
    virtual std::int32_t cstring_compare_insensitive_00438e10(const char* a, const char* b) = 0;
    // 00779D50 BSP_Entity_MayFollowTarget, __thiscall(ECX = entity; token, target), RET 8 -> AL.
    virtual std::uint8_t entity_may_follow_target_00779d50(Object entity, const char* token,
                                                           Object target) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject, __fastcall(ECX = target) -> EAX.
    virtual Object command_target_resolve_object_00521ea0(Object target) = 0;
    // 0071EDD0 BSP_WeaponDirector_TargetStillValid, __thiscall(ECX = director; target, byte
    // flag), RET 8 -> AL.
    virtual std::uint8_t weapon_director_target_still_valid_0071edd0(Object director,
                                                                     Object target,
                                                                     std::uint8_t flag) = 0;
    // [00E188A8], the game session (mode at +1FE4h).
    virtual Object game_session_00e188a8() = 0;
    // [unit]->vtable[5Ch](ECX = unit; int class id), RET 4 -> AL (IsKindOf).
    virtual std::uint8_t unit_is_kind_of_vslot_5c(Object unit, std::int32_t class_id) = 0;
    // [visitor]->vtable[0](ECX = visitor; object), RET 4.
    virtual void visitor_vslot_00(Object visitor, Object object) = 0;
    // 0077E830 BSP_MissionEntity_AttachLuaSelfWithResourceUsage, __thiscall(ECX = entity).
    virtual void mission_entity_attach_lua_self_0077e830(Object entity) = 0;
    // 00BF681B operator new(size) -> EAX (cdecl).
    virtual Object operator_new_00bf681b(std::uint32_t size) = 0;
    // 008363E0 BSP_WeaponDirectorBase_Construct, __thiscall(ECX = memory; endpoint, byte),
    // RET 8 -> EAX.
    virtual Object weapon_director_base_construct_008363e0(Object memory, Object endpoint,
                                                           std::uint8_t flag) = 0;
    // 0095A880 BSP_Unit_InstantiateClassDevices, __thiscall(ECX = unit; list, flag, arg),
    // RET 0Ch.
    virtual void unit_instantiate_class_devices_0095a880(Object unit, Object list,
                                                         std::uint32_t flag,
                                                         std::uint32_t arg) = 0;
    // 00408120 BSP_LegacySboString_AssignSubstring, __thiscall(ECX = string; source, offset,
    // count), RET 0Ch -> EAX.
    virtual Object sbo_string_assign_substring_00408120(Object string, Object source,
                                                        std::uint32_t offset,
                                                        std::uint32_t count) = 0;
    // 00796BD0 BSP_EntityObserver_CopyConstructBase, __thiscall(ECX = observer; source),
    // RET 4.
    virtual void entity_observer_copy_construct_base_00796bd0(Object observer, Object source) = 0;
    // 00408720 BSP_CharacterString_AssignCounted, __thiscall(ECX = string; const char*,
    // count), RET 8 -> EAX.
    virtual Object character_string_assign_counted_00408720(Object string, const char* text,
                                                             std::uint32_t count) = 0;
    // 0094EDC0, the name list split, __fastcall(ECX = vector, EDX = const string*;
    // const char* separators), RET 4.
    virtual void string_vector_split_0094edc0(Object vector, Object text,
                                              const char* separators) = 0;
    // 00BF65AC _free.
    virtual void free_00bf65ac(void* block) = 0;
    // 00B63F10 BSP_Matrix_BuildLookAt, __fastcall(ECX = out matrix, EDX = eye; const float3*
    // target, up.x, up.y, up.z), RET 10h -> EAX; 004134F0 BSP_Matrix_Copy4x4X87,
    // __thiscall(ECX = dest; const float* source), RET 4.
    virtual float* matrix_build_look_at_00b63f10(float* out, const float* eye, const float* target,
                                                 float up_x, float up_y, float up_z) = 0;
    virtual void matrix_copy_4x4_004134f0(float* dest, const float* source) = 0;
    // 00868420 BSP_PointEffect_CreateFromMatrix, __fastcall(ECX = out RefPtr slot, EDX =
    // registry; RefPtr definition by value (consumed), const float* matrix, 0, 0), RET 10h ->
    // EAX out slot.
    virtual Object* point_effect_create_from_matrix_00868420(Object* out, Object registry,
                                                             Object definition,
                                                             const float* matrix,
                                                             std::uint32_t a, std::uint32_t b) = 0;
    // 0084BF00 BSP_Projectile_TraceSegmentAndImpact, __fastcall(CL = replay flag, EDX = flags;
    // segment list*, a6, a7, a8, a9, a10), RET 18h -> EAX.
    virtual std::uint32_t projectile_trace_segment_and_impact_0084bf00(
        std::uint8_t replay_flag, std::uint32_t flags, Object segment_list, std::uint32_t a6,
        std::uint32_t a7, std::uint32_t a8, std::uint32_t a9, std::uint32_t a10) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __fastcall(ECX = entity); 007AF800
    // BSP_ScenePath_TransformPointToWorld, __thiscall(ECX = path; out, index), RET 8 -> EAX.
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    virtual float* scene_path_transform_point_to_world_007af800(Object path, float* out,
                                                                std::uint32_t index) = 0;
    // 0095CC90 BSP_UnitGameObject_Construct, __thiscall(ECX = unit; flag, 0), RET 8.
    virtual void unit_game_object_construct_0095cc90(Object unit, std::uint32_t flag,
                                                     std::uint32_t zero) = 0;
    // 00BF79F0 _memset (cdecl).
    virtual void memset_00bf79f0(Object dest, int value, std::uint32_t size) = 0;
    // 0084FA30, the checked vector<8-byte> erase, __thiscall(ECX = vector; out iterator*,
    // owner, where), RET 0Ch -> EAX out.
    virtual Object pair_vector_erase_0084fa30(Object vector, Object out_iterator, Object owner,
                                              Object where) = 0;
    // [object]->vtable[4](ECX = object; 1), RET 4: the scalar deleting destructor.
    virtual void object_delete_vslot_04(Object object, std::uint32_t flag) = 0;
    // 00923060 TRIV_body_00923060, the entity base destructor body, __fastcall(ECX = entity),
    // reached by tail jump.
    virtual void entity_base_destruct_00923060(Object entity) = 0;
    // The entity handle tables 008437D0 resolves inline: [00F89A10] split, [00F89A0C] / [00F89A60]
    // bases, [00F89A54] / [00F89AA8] 10h-byte entry tables (object at +0Ch).
    virtual std::int32_t handle_split_00f89a10() = 0;
    virtual std::int32_t handle_base_a_00f89a0c() = 0;
    virtual std::int32_t handle_base_b_00f89a60() = 0;
    virtual Object handle_table_a_00f89a54() = 0;
    virtual Object handle_table_b_00f89aa8() = 0;
    // 00694A60 BSP_Observer_RegisterPair, __fastcall(ECX = entity, EDX = observer);
    // [object]->vtable[0D8h](ECX = object).
    virtual void observer_register_pair_00694a60(Object entity, Object observer) = 0;
    virtual void object_vslot_d8(Object object) = 0;
    // 00851B40 STL_Map_Subscript, __thiscall(ECX = map 00F87550; const NativeString* key),
    // RET 4 -> EAX value slot.
    virtual Object* stationary_class_map_subscript_00851b40(Object map, Object key) = 0;
    // 00B67980 BSP_LuaStateOwner_GetGlobals, __thiscall(ECX = owner; out), RET 4; 00B67800
    // BSP_LuaObject_GetByName, __thiscall(ECX = object; out, const char*), RET 8; 00B68100
    // BSP_LuaObject_GetByNativeString, __thiscall(ECX = object; out, const NativeString*),
    // RET 8.
    virtual void lua_state_owner_get_globals_00b67980(Object owner, cc13_seg52::LuaObject* out) = 0;
    virtual void lua_object_get_by_name_00b67800(cc13_seg52::LuaObject* object,
                                                 cc13_seg52::LuaObject* out,
                                                 const char* name) = 0;
    virtual void lua_object_get_by_native_string_00b68100(cc13_seg52::LuaObject* object,
                                                          cc13_seg52::LuaObject* out,
                                                          Object name) = 0;
    // [class]->vtable[8](ECX = class; const LuaObject* row), RET 4 (00851580 for 00D0BE38);
    // [class]->vtable[0Ch](ECX = class) (0084E640 for 00D0BE38).
    virtual void stationary_class_vslot_08(Object stationary_class, cc13_seg52::LuaObject* row) = 0;
    virtual void stationary_class_vslot_0c(Object stationary_class) = 0;
    // --- 008509F0's scene and body set-up ---
    // 00923840 BSP_Entity_InheritRaceAndActivate, __fastcall(ECX = entity).
    virtual void entity_inherit_race_and_activate_00923840(Object entity) = 0;
    // [model]->vtable[8](ECX = model; int kind, float scale), RET 8 -> EAX.
    virtual Object model_vslot_08(Object model, std::int32_t kind, float scale) = 0;
    // 007135C0 BSP_UnitPartInstance_Construct, __thiscall(ECX = memory; unit, part set), RET 8
    // -> EAX.
    virtual Object unit_part_instance_construct_007135c0(Object memory, Object unit,
                                                         Object part_set) = 0;
    // 0048E9F0 BSP_ScenePropertyBag_Contains / 008F2260 BSP_ScenePropertyBag_Find,
    // __thiscall(ECX = bag; const char* key), RET 4 -> AL / EAX.
    virtual std::uint8_t property_bag_contains_0048e9f0(Object bag, const char* key) = 0;
    virtual Object property_bag_find_008f2260(Object bag, const char* key) = 0;
    // 00710BB0 BSP_ModelGroups_SelectVisibleGroup, __thiscall(ECX = part; group), RET 4.
    virtual void model_groups_select_visible_group_00710bb0(Object part, std::int32_t group) = 0;
    // [child]->vtable[1B4h](ECX = child; level), RET 4; [node]->vtable[38h](ECX = node; arg),
    // RET 4; [object]->vtable[40h](ECX = object) -> EAX.
    virtual void child_vslot_1b4(Object child, std::int32_t level) = 0;
    virtual void scene_node_vslot_38(Object node, Object arg) = 0;
    virtual Object object_vslot_40(Object object) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration / 00B6E680 BSP_Node_SetParent,
    // __thiscall(ECX = node; arg), RET 4.
    virtual void node_propagate_root_registration_00b6d890(Object node, Object root) = 0;
    virtual void node_set_parent_00b6e680(Object node, Object parent) = 0;
    // 00475E60 BSP_Node_OrFlagsRecursive / 00728340 BSP_SceneNode_ClearFlagsRecursive,
    // __thiscall(ECX = node; mask), RET 4.
    virtual void node_or_flags_recursive_00475e60(Object node, std::uint32_t mask) = 0;
    virtual void scene_node_clear_flags_recursive_00728340(Object node, std::uint32_t mask) = 0;
    // 00747560 BSP_GeneratedModel_RegisterShadowFactor, __fastcall(ECX = node, EDX = const
    // float* factor).
    virtual void generated_model_register_shadow_factor_00747560(Object node,
                                                                 const float* factor) = 0;
    // [00E1AEA0], the decal manager; 00740FE0 BSP_DecalList_RegisterOwner, __thiscall(ECX =
    // manager) -> EAX.
    virtual Object decal_manager_00e1aea0() = 0;
    virtual Object decal_list_register_owner_00740fe0(Object manager) = 0;
    // 00747CC0 / 00748150, the 48h-byte shape descriptor vector reserve / push_back,
    // __thiscall(ECX = vector; count / const element*), RET 4.
    virtual void shape_vector_reserve_00747cc0(Object vector, std::uint32_t count) = 0;
    virtual void shape_vector_push_back_00748150(Object vector, Object element) = 0;
    // 0084EFE0, __fastcall(ECX = stationary) -> EAX [[s+19Ch]+160h] (0 without a part).
    virtual Object stationary_part_scene_0084efe0(Object stationary) = 0;
    // 00C336C0, __thiscall(ECX = dest matrix; const source*), RET 4; 00C5D580
    // Dyn_World_CreateBody, __thiscall(ECX = world; const descriptor*), RET 4 -> EAX.
    virtual void matrix_copy_00c336c0(Object dest, Object source) = 0;
    virtual Object dyn_world_create_body_00c5d580(Object world, Object descriptor) = 0;
    // 00BF6989 operator delete (cdecl).
    virtual void operator_delete_00bf6989(void* block) = 0;
    // 0095BB10 BSP_VehicleUnit_LoadArchiveCommon, __thiscall(ECX = unit; writer), RET 4;
    // 008360C0 BSP_WeaponDirector_VisitProperties, __thiscall(ECX = director; writer), RET 4.
    virtual void vehicle_unit_load_archive_common_0095bb10(Object unit, Object writer) = 0;
    virtual void weapon_director_visit_properties_008360c0(Object director, Object writer) = 0;
    // 00F87541, an empty text global; 00BD5680 BSP_TaggedValue_Equals, __thiscall(ECX = const
    // tagged*; const tagged*), RET 4 -> AL.
    virtual const char* default_empty_text_00f87541() = 0;
    virtual std::uint8_t tagged_value_equals_00bd5680(const cc13_seg52::TaggedValue* a,
                                                      const cc13_seg52::TaggedValue* b) = 0;
    // 00851230, the registry vector push_back, __thiscall(ECX = vector; const 8-byte entry*),
    // RET 4; 00923040 TRIV_body_00923040, the entity OnWrecked base, __fastcall(ECX = entity).
    virtual void registry_pair_vector_push_back_00851230(Object vector, Object entry) = 0;
    virtual void entity_on_wrecked_base_00923040(Object entity) = 0;
    // byte [00E0AF20] (mission_scene_load.hpp: scene_resident).
    virtual std::uint8_t scene_resident_00e0af20() = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message; type), RET 4;
    // 0076A9F0, __thiscall(ECX = [00E188A8]+1EF0h; message*, 0), RET 8.
    virtual void session_message_construct_base_0075b430(Object message, std::int32_t type) = 0;
    virtual void session_message_send_0076a9f0(Object sink, Object message, std::uint32_t flag) = 0;
    // 00B62D10 BSP_Vector4f_Transform, __thiscall(ECX = const float4*; float4* out, const
    // matrix*), RET 8 -> EAX out.
    virtual const float* vector4f_transform_00b62d10(const float* vector, float* out,
                                                     Object matrix) = 0;
    // 008685E0 BSP_PointEffect_CreateFromPosition, __fastcall(ECX = out RefPtr slot, EDX =
    // registry; RefPtr definition by value (consumed), const float3*, 0, 0), RET 10h -> EAX.
    virtual Object* point_effect_create_from_position_008685e0(Object* out, Object registry,
                                                               Object definition,
                                                               const float* position,
                                                               std::uint32_t a,
                                                               std::uint32_t b) = 0;
    // 0049C940 BSP_PointAnnouncement_Create_Provisional, __thiscall(ECX = announcer; const
    // float3* position, const float3* offset, int index), RET 0Ch -> EAX.
    virtual Object point_announcement_create_0049c940(Object announcer, const float* position,
                                                      const float* offset,
                                                      std::int32_t index) = 0;
    // 00B66FA0 BSP_LuaObject_CopyConstruct, __thiscall(ECX = dest; const LuaObject* source),
    // RET 4.
    virtual void lua_object_copy_construct_00b66fa0(cc13_seg52::LuaObject* dest, Object source) = 0;
    // 00BD68D0 BSP_LuaReader_ReadFieldOrDefault, __thiscall(ECX = reader; key, value, default
    // as three by-value tagged pairs), RET 18h.
    virtual void lua_reader_read_field_or_default_00bd68d0(Object reader,
                                                           cc13_seg52::TaggedValue key,
                                                           cc13_seg52::TaggedValue value,
                                                           cc13_seg52::TaggedValue fallback) = 0;
    // 00871A90 BSP_GameplayEffect_FindIdByName, __fastcall(ECX = const NativeString*) -> EAX.
    virtual std::int32_t gameplay_effect_find_id_by_name_00871a90(Object name) = 0;
    // 00870CD0 BSP_EffectHandle_Acquire, __fastcall(ECX = out RefPtr slot, EDX = id; 1), RET 4
    // -> EAX out slot.
    virtual Object* effect_handle_acquire_00870cd0(Object* out, std::int32_t id,
                                                   std::uint32_t flag) = 0;
    // 00441210 BSP_LuaObjectVector_Tidy, __thiscall(ECX = reader+4).
    virtual void lua_object_vector_tidy_00441210(Object vector) = 0;
    // 00779BB0 BSP_Unit_GetPropertyReconLevel, __thiscall(ECX = unit; LuaObject* lua, const
    // NativeString* name), RET 8; 00BF7FBF _stricmp.
    virtual void unit_get_property_recon_level_00779bb0(Object unit, Object lua, Object name) = 0;
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // The Lua table helpers: 00B67930 BSP_LuaObject_NewTable, __thiscall(ECX = lua; out), RET
    // 4; 00B66670 BSP_LuaObject_SetNewTableAtIndex, __thiscall(ECX = table; index), RET 4;
    // 00B67720 BSP_LuaObject_GetByIndex, __thiscall(ECX = table; out, index), RET 8; 00B67460
    // BSP_LuaObject_SetNativeStringInteger, __thiscall(ECX = table; const NativeString* key,
    // value), RET 8; 00B67700 BSP_LuaObject_Destruct, __thiscall(ECX = object).
    virtual void lua_object_new_table_00b67930(Object lua, cc13_seg52::LuaObject* out) = 0;
    virtual void lua_set_new_table_at_index_00b66670(cc13_seg52::LuaObject* table,
                                                     std::int32_t index) = 0;
    virtual void lua_get_by_index_00b67720(cc13_seg52::LuaObject* table,
                                           cc13_seg52::LuaObject* out, std::int32_t index) = 0;
    virtual void lua_set_native_string_integer_00b67460(cc13_seg52::LuaObject* table, Object key,
                                                        std::int32_t value) = 0;
    virtual void lua_object_destruct_00b67700(cc13_seg52::LuaObject* object) = 0;
    // [object]->vtable[0](ECX = object; flag), RET 4: the scalar deleting destructor in slot 0.
    virtual void object_delete_vslot_00(Object object, std::uint32_t flag) = 0;
    // The image globals: 00F87550 the stationary class map {+4h head, +8h size}; 00F87548 the
    // default name text; 00F87574 the default float3.
    virtual Object stationary_class_map_00f87550() = 0;
    virtual const char* default_name_text_00f87548() = 0;
    virtual const float* default_vector_00f87574() = 0;
    // 0084EE80 STL_TreeIter_Inc, __thiscall(ECX = iterator {owner, node}); 008501D0
    // STL_Tree_Erase, __thiscall(ECX = map; root), RET 4.
    virtual void tree_iterator_increment_0084ee80(Object iterator) = 0;
    virtual void tree_erase_008501d0(Object map, Object root) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string; const char*), RET 4 -> EAX.
    virtual Object native_string_assign_0041e870(Object string, const char* text) = 0;
    // 007188A0 BSP_Resource_LoadWithGameFactory, __thiscall(ECX = const NativeString* name) ->
    // EAX resource.
    virtual Object resource_load_with_game_factory_007188a0(Object name) = 0;
    // 00718870 BSP_GameResource_LookupNamedPointGroup, __thiscall(ECX = resource; const
    // string*, index), RET 8 -> EAX group.
    virtual Object resource_lookup_named_point_group_00718870(Object resource, Object name,
                                                              std::uint32_t index) = 0;
    // 00484270, the point group's checked point read, __thiscall(ECX = group; float3* out,
    // index), RET 8 -> EAX out.
    virtual float* point_group_point_00484270(Object group, float* out, std::uint32_t index) = 0;
};

// The image's 1Ch-byte MSVC string {+0 allocator, +4 inline buffer / pointer, +14h size,
// +18h capacity}; inline while capacity < 10h.
struct StdString1C {
    alignas(4) std::uint8_t bytes[0x1C];
};

// ---- small helpers -------------------------------------------------------------------------
int format_to_buffer16_00843000(Seg52bHost& host, char* dest, const char* format, ...);
Object ref_ptr_assign_0082a160(Seg52bHost& host, Object slot, Object value);
void ship_indexed_point_array_push_back_0082a590(Seg52bHost& host, Object array,
                                                 const Object element);
std::uint32_t los_sweep_with_ignore_0084b6b0(Seg52bHost& host, Object segments, Object b,
                                             Object ignore_object);
Object vector_holder_construct_0082dc30(Object holder);
void vector_holder_destruct_0082dc80(Seg52bHost& host, Object holder);
bool shipyard_accepts_command_token_00843460(Seg52bHost& host, Object shipyard,
                                             const char* token, Object target);
Object shipyard_weapon_director_00842a80(Object shipyard);
void shipyard_create_weapon_director_008429e0(Seg52bHost& host, Object shipyard);

// ---- settings records -----------------------------------------------------------------------
Object settings_keyed_string_copy_construct_008397a0(Seg52bHost& host, Object dest, Object source);
Object settings_record30_copy_construct_00839800(Seg52bHost& host, Object dest, Object source);
Object settings_record30_assign_00839490(Seg52bHost& host, Object dest, Object source);

// ---- shipyard stock / slots -----------------------------------------------------------------
Object shipyard_stock_record_copy_construct_008486b0(Seg52bHost& host, Object dest, Object source);
void shipyard_stock_record_construct_at_00848d00(Seg52bHost& host, Object where, Object source);
Object shipyard_copy_stock_record_00848d70(Seg52bHost& host, Object shipyard, Object out,
                                           std::uint32_t index);
Object shipyard_stock_record_construct_008485f0(Seg52bHost& host, Object record,
                                                Object vehicle_class, std::uint32_t count,
                                                const char* names);
Object shipyard_slot_copy_construct_00846970(Seg52bHost& host, Object dest, Object source);

// ---- plane squadron command block (vtable 00D0BDD0..) ----------------------------------------
void squadron_block_accept_visitor_0084da20(Seg52bHost& host, Object block, Object visitor);
void squadron_block_set_fire_and_move_0084d980(Seg52bHost& host, Object block,
                                               std::uint32_t mode);
bool squadron_block_target_still_valid_0084da60(Seg52bHost& host, Object block, Object target,
                                                std::uint8_t flag);

// ---- projectiles / effects ------------------------------------------------------------------
Object spawn_oriented_point_effect_0084b6f0(Seg52bHost& host, Object definition_slot,
                                            const float* position, const float* direction);
std::uint32_t projectile_sweep_step_segment_0084c430(Seg52bHost& host, std::uint8_t replay_flag,
                                                     std::uint32_t flags, float from_x,
                                                     float from_y, float from_z, float to_x,
                                                     float to_y, float to_z, std::uint32_t a6,
                                                     std::uint32_t a7, std::uint32_t a8,
                                                     std::uint32_t a9, std::uint32_t a10);

// ---- shipyard lifetime ----------------------------------------------------------------------
float* shipyard_exit_point_00845670(Seg52bHost& host, Object shipyard, float* out);
Object shipyard_construct_00848080(Seg52bHost& host, Object shipyard, std::uint32_t flag);
Object shipyard_allocate_00848380(Seg52bHost& host, Object vehicle_class, std::uint32_t flag);

// ---- stationaries ---------------------------------------------------------------------------
void stationary_class_destruct_0084f670(Seg52bHost& host, Object stationary_class);
void stationary_destruct_0084fb80(Seg52bHost& host, Object stationary);

void stationary_apply_fort_class_0084f7f0(Seg52bHost& host, Object entity, Object vehicle_class);
void stationary_class_map_destroy_all_00850930(Seg52bHost& host);
void stationary_class_load_explosion_resource_0084fce0(Seg52bHost& host, Object stationary_class);

void shipyard_get_property_stock_00846640(Seg52bHost& host, Object shipyard, Object lua,
                                          Object name);

void stationary_class_read_lua_fields_00851580(Seg52bHost& host, Object stationary_class,
                                               Object row);

void stationary_on_wrecked_00851850(Seg52bHost& host, Object stationary);

void shipyard_describe_state_00845c10(Seg52bHost& host, Object shipyard, Object writer);

void stationary_sentity_init_008509f0(Seg52bHost& host, Object stationary);

Object stationary_class_get_or_create_00851cb0(Seg52bHost& host, Object name);

void shipyard_apply_spawn_message_008437d0(Seg52bHost& host, Object shipyard, Object message);

}  // namespace bsp::cc13_seg52b
