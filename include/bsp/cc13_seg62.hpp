#pragma once
// Packet cc13-seg62: the game functions of segments 62 (00951F40..00966890) and
// 65 (0097AB70..0098C510) that had no reconstruction record. Segment 62 holds
// unit and vehicle-class helpers (device walks, catapult slots, gun ranges,
// class tables and their destructors); segment 65 holds the mission-event
// subscriptions, the warnings and the warning manager. docs/CC13_SEG62.md lists
// every function with its ABI, evidence and uncertainty, the functions found
// already implemented elsewhere (ledger-only) and the ones skipped.
//
// Objects are reached through byte offsets in their Win32 layout (Object plus
// at<T>). Every call the image makes goes through a Seg62Host method named for
// its target address (virtual calls by their vtable offset), at the same point
// and with the same arguments. Names are hypotheses, not recovered symbols.
// These are new C++ interfaces over an injected host: build-tested only, not
// wired into a game host, not ABI-compatible replacements.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg62 {

static_assert(sizeof(void*) == 4, "segment 62/65 objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}
inline Object obj(std::uint32_t value) noexcept {
    return reinterpret_cast<Object>(static_cast<std::uintptr_t>(value));
}
inline std::uint32_t addr(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

// The iterator 00952080 / 009520E0 fill: {position in the slot table, list node}.
struct DeviceListIterator {
    std::int32_t index;
    std::uint32_t node;
};

// A tagged value as the archive and Lua readers take it by value: {tag, value};
// tag 0 a string pointer, 1 an int, 2 a float, 3 a bool (low byte; the image
// leaves the other three bytes as stack contents, modelled as zero).
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t value;
};

class Seg62Host {
public:
    virtual ~Seg62Host() = default;

    // Globals (image addresses).
    virtual std::uint32_t global_u32(std::uint32_t address) = 0;

    // Virtual calls, by receiver and vtable offset (all __thiscall).
    virtual bool vt5c_is_kind_of(Object entity, std::int32_t kind) = 0;  // vtable[5Ch](kind) -> AL
    virtual void vt1ec(Object device) = 0;                                // vtable[1ECh]()

    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(dest, const float src[16]), RET 4.
    virtual void matrix_copy_004134f0(Object dest, const float* source) = 0;
    // 0043F080 BSP_UnitInstance_IsAliveAndVisible, __thiscall(instance) -> AL.
    virtual bool unit_instance_alive_visible_0043f080(Object instance) = 0;
    // 00647300 BSP_InGameHudRoot_SetSpectatedUnit, __thiscall(hud root, unit), RET 4.
    virtual void hud_set_spectated_unit_00647300(Object hud_root, Object unit) = 0;

    virtual float global_f32(std::uint32_t address) = 0;
    // CRT: 00BF681B operator new(size) -> EAX; 00BF65AC _free(p).
    virtual Object operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void free_00bf65ac(Object memory) = 0;

    // More virtual calls.
    virtual std::uint32_t vt0(Object receiver) = 0;          // vtable[0]() -> EAX
    virtual Object vt114(Object entity) = 0;                 // vtable[114h]() -> EAX

    // 006E6BB0 BSP_Projectile_BindOwner, __thiscall(shot, owner), RET 4.
    virtual void projectile_bind_owner_006e6bb0(Object shot, Object owner) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __thiscall(transform).
    virtual void transform_refresh_world_00b6db70(Object transform) = 0;
    // 0042B260 BSP_Geometry_NormalizeVectorWithFloor, ECX = float[3] in place.
    virtual void normalize_with_floor_0042b260(float* vector) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(entity).
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    // 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse, ECX = out, EDX = source.
    virtual void affine_inverse_00b63d50(Object out, Object source) = 0;
    // 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize, ECX = out,
    // EDX = direction, stack (matrix, normalize), RET 8, EAX = out.
    virtual float* transform_direction_0042d0d0(float* out, const float* direction, Object matrix,
                                                std::int32_t normalize) = 0;
    // 00521370 BSP_Direction_ToPitchYaw, ECX = direction, EDX = out a, stack out b, RET 4.
    virtual void direction_to_pitch_yaw_00521370(const float* direction, float* out_a, float* out_b) = 0;
    // 00605070 BSP_Math_WrapAngleInPlace_Provisional, ECX = float*.
    virtual void wrap_angle_00605070(float* angle) = 0;
    // 00954F90 STL list erase, __thiscall(list, node), RET 4, EAX = next node.
    virtual Object list_erase_00954f90(Object list, Object node) = 0;

    // Kernel32 through the IAT: [00CE221C] InterlockedIncrement, [00CE2220]
    // InterlockedDecrement (-> the new count).
    virtual std::int32_t interlocked_increment(Object counter) = 0;
    virtual std::int32_t interlocked_decrement(Object counter) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix: ECX = out slot, EDX =
    // parent node, stack (definition reference consumed by the callee, matrix,
    // transform word, option word, tail word), RET 14h, EAX = out slot.
    virtual Object* point_effect_create_008687c0(Object* out, Object parent_node, Object definition,
                                                 const float* matrix, std::uint32_t transform_word,
                                                 std::uint32_t option_word, std::uint32_t tail_word) = 0;
    // 004845D0 BSP_RetainedReference_Assign_004845D0, __thiscall(slot, const Object* source), RET 4.
    virtual void retained_assign_004845d0(Object slot, const Object* source) = 0;
    // 00484620, __thiscall(slot, value), RET 4: adopt without a retain.
    virtual void reference_adopt_00484620(Object slot, Object value) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, __thiscall(effect).
    virtual void point_effect_stop_children_00867b10(Object effect) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, ECX = point, stack (out, matrix), RET 8.
    virtual void transform_affine_point_004142e0(const float* point, float* out, Object matrix) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose, __thiscall(node).
    virtual void scene_node_invalidate_pose_0042ed50(Object node) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(string, size, fill), RET 8.
    virtual void string_resize_0041dd40(Object string, std::uint32_t size, std::uint32_t fill) = 0;
    // 0041E350 BSP_NativeString_AssignCString, __thiscall(string, text), RET 4.
    virtual void string_assign_0041e350(Object string, const char* text) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void memcpy_00bf7680(Object dest, const void* source, std::uint32_t size) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report (no return in the image).
    virtual void invalid_parameter_00bf6713() = 0;
    // 00415450 STL_Vector_Size_00415450, __thiscall(vector) -> EAX.
    virtual std::uint32_t vector_size_00415450(Object vector) = 0;
    // 00727E10 BSP_Gun_SetPair43C, __thiscall(gun, a, b), RET 8.
    virtual void gun_set_pair_00727e10(Object gun, std::uint32_t a, std::uint32_t b) = 0;
    // 0042D2E0 BSP_Matrix_ExtractNormalizedBasisAngles: ECX = matrix, EDX = out a,
    // stack (out b, out c), RET 8.
    virtual void extract_basis_angles_0042d2e0(Object matrix, float* a, float* b, float* c) = 0;
    // 00438AA0 BSP_Math_AddWrappedAngle, __stdcall(float, float) -> ST0, RET 8.
    virtual float add_wrapped_angle_00438aa0(float angle, float delta) = 0;
    // 00467050 BSP_Matrix_SetRotationFromEuler, __thiscall(matrix, a, b, c), RET 0Ch.
    virtual void matrix_set_rotation_euler_00467050(float* matrix, float a, float b, float c) = 0;
    // 00741E90 BSP_Entity_SetWorldPoseMatrix, __thiscall(entity, const float* matrix), RET 4.
    virtual void entity_set_world_pose_00741e90(Object entity, const float* matrix) = 0;

    // Native strings {size +0, data +4} (8 bytes).
    // 0041E870 BSP_NativeString_Assign: constructor, __thiscall(string, text), RET 4, EAX = string.
    virtual Object string_construct_0041e870(Object string, const char* text) = 0;
    // 004263B0 BSP_NativeString_ConcatInt, __thiscall(source, out, int), RET 8, EAX = out.
    virtual Object string_concat_int_004263b0(Object source, Object out, std::int32_t value) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(lhs, out, const rhs*), RET 8, EAX = out.
    virtual Object string_concat_004261a0(Object lhs, Object out, Object rhs) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(dest, const source*), RET 4.
    virtual void string_assign_header_00425f40(Object dest, Object source) = 0;
    // The inlined string destructor: 00419CC0 BSP_SizedStoragePool_GetSingleton()
    // -> EAX (the arguments pushed before it belong to the next call), then
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(pool, block, size, unused), RET 0Ch.
    virtual Object pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(Object pool, Object block, std::uint32_t size, std::uint32_t unused) = 0;
    // 00443490 BSP_DeviceClass_ResolveAndActivate, ECX = id, DL = flag -> EAX.
    virtual Object device_class_resolve_00443490(std::uint32_t id, std::uint8_t flag) = 0;

    // The state archive writer (vtable calls on the archive, __thiscall):
    // vtable[4](key) RET 8 opens a table, vtable[8]() closes it, vtable[0Ch](key,
    // value) RET 10h writes a field. Keys and values are TaggedValue by value.
    virtual void archive_vt4_open(Object archive, TaggedValue key) = 0;
    virtual void archive_vt8_close(Object archive) = 0;
    virtual void archive_vt0c_write(Object archive, TaggedValue key, TaggedValue value) = 0;
    // 008776E0 BSP_UnitTickableEntity_WriteState, __thiscall(unit, archive), RET 4.
    virtual void unit_write_state_008776e0(Object unit, Object archive) = 0;
    // 00437F50 BSP_VehicleClassRegistry_GetSingleton() -> EAX.
    virtual Object class_registry_00437f50() = 0;
    // 006E63A0 BSP_SkillLevel_Write, __thiscall(skill, archive), RET 4.
    virtual void skill_level_write_006e63a0(Object skill, Object archive) = 0;
    // 00BD5680 BSP_TaggedValue_Equals, __thiscall(const a*, const b*), RET 4 -> AL.
    virtual bool tagged_value_equals_00bd5680(const TaggedValue* a, const TaggedValue* b) = 0;
    // 005CD640 BSP_VehicleCameraArray_Resize, __thiscall(array, count), RET 4.
    virtual void camera_array_resize_005cd640(Object array, std::uint32_t count) = 0;
    // 00BF6989 (_free; a second CRT free entry), cdecl.
    virtual void free_00bf6989(Object memory) = 0;

    // Lua object wrappers (14h-byte objects, __thiscall; "out" is constructed).
    // 00B67980 BSP_LuaStateOwner_GetGlobals(owner, out), RET 4, EAX = out.
    virtual Object lua_globals_00b67980(Object owner, Object out) = 0;
    // 00B67800 BSP_LuaObject_GetByName(table, out, name), RET 8, EAX = out.
    virtual Object lua_get_by_name_00b67800(Object table, Object out, const char* name) = 0;
    // 00B67720 BSP_LuaObject_GetByIndex(table, out, index), RET 8, EAX = out.
    virtual Object lua_get_by_index_00b67720(Object table, Object out, std::int32_t index) = 0;
    // 00B67700 BSP_LuaObject_Destruct(object).
    virtual void lua_destruct_00b67700(Object object) = 0;
    // 00B662B0 BSP_LuaObject_GetString(object) -> EAX.
    virtual const char* lua_get_string_00b662b0(Object object) = 0;
    // 00B66380 BSP_LuaReference_GetIntegerOrDefault(object, default), RET 4.
    virtual std::int32_t lua_get_int_or_default_00b66380(Object object, std::int32_t fallback) = 0;
    // 00BF7FBF _stricmp (cdecl).
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive(string, text), RET 4 -> AL.
    virtual bool string_equals_ci_00425850(Object string, const char* text) = 0;
    // 004359E0 BSP_VehicleClassPartyRows_Resize(rows, count), RET 4.
    virtual void party_rows_resize_004359e0(Object rows, std::uint32_t count) = 0;
    // 0095BAB0 BSP_VehicleClass_IsEnemyOfLocalParty, ECX = class entry -> AL.
    virtual bool class_is_enemy_0095bab0(std::uint32_t entry) = 0;
    // 004CACA0 BSP_NativeString_AppendInt(string, value), RET 4.
    virtual void string_append_int_004caca0(Object string, std::int32_t value) = 0;
    // 00B65F50 BSP_LuaObject_Construct(object).
    virtual void lua_construct_00b65f50(Object object) = 0;
    // 00B67080 / 00B67190 BSP_LuaObject_IterateFirst / IterateNext(table, key*, value*), RET 8.
    virtual void lua_iterate_first_00b67080(Object table, Object key, Object value) = 0;
    virtual void lua_iterate_next_00b67190(Object table, Object key, Object value) = 0;
    // 00B66420 (named BSP_LuaObject_IsUnbound), __thiscall(table, key*), RET 4 -> AL:
    // true ends the iteration.
    virtual bool lua_iterate_done_00b66420(Object table, Object key) = 0;
    // 00B66000 (named BSP_LuaObject_IsBoolean), __thiscall(object) -> AL.
    virtual bool lua_test_00b66000(Object object) = 0;
    // 00B66290 BSP_LuaObject_GetInteger(object) -> EAX.
    virtual std::int32_t lua_get_integer_00b66290(Object object) = 0;
    // 00442190 STL_Vector_PushBack_00442190, __thiscall(vector<int>, const int*), RET 4.
    virtual void int_vector_push_back_00442190(Object vector, const std::int32_t* value) = 0;
    // 0043EE60 BSP_String_FormatBuffer2A(buffer[2Ah], format, ...) cdecl.
    virtual void format_buffer_0043ee60(char* buffer, const char* format, std::int32_t value) = 0;
    // 006B8AD0 BSP_LuaMachine_RunString, __thiscall(machine, source, sink, error, mode), RET 10h.
    virtual void lua_run_string_006b8ad0(Object machine, const char* source, Object sink, Object error,
                                         std::int32_t mode) = 0;
    // 00441E50 std::_Tree erase(first, last), __thiscall(tree, out iterator*,
    // first {container, node}, last {container, node}), RET 14h.
    virtual void tree_erase_range_00441e50(Object tree, Object out_iterator, Object first_container,
                                           Object first_node, Object last_container, Object last_node) = 0;
    // 00440D90 STL_Tree_CopyTree_00440d90, __thiscall(tree, const source tree*), RET 4.
    virtual void tree_copy_00440d90(Object tree, Object source) = 0;
    // 0095BE70 STL_Vector_Resize_0095be70, __thiscall(vector<44h-byte>, count,
    // element by value (11h dwords on the stack)).
    virtual void vector44_resize_0095be70(Object vector, std::uint32_t count, const std::uint32_t* element) = 0;
    // 008797B0 BSP_UnitInstance_ReleaseDamageStateInstance, __thiscall(unit).
    virtual void release_damage_state_008797b0(Object unit) = 0;
    // 0095D2C0 STL_Tree_EraseRange_0095d2c0, same shape as 00441E50.
    virtual void tree_erase_range_0095d2c0(Object tree, Object out_iterator, Object first_container,
                                           Object first_node, Object last_container, Object last_node) = 0;
    // 005471B0 BSP_RefCountedPointerArray_Resize, __thiscall(array, count), RET 4.
    virtual void ref_pointer_array_resize_005471b0(Object array, std::uint32_t count) = 0;
    // 00955180, __thiscall(object) (an STL_inst body; destroys the list object
    // 0095F1C0 frees afterwards).
    virtual void destroy_00955180(Object object) = 0;
    // 00745200 BSP_IntrusiveList_ClearNodes_00745200, __thiscall(list).
    virtual void list_clear_nodes_00745200(Object list) = 0;
    // 007480A0, ECX = first, EDX = last, stack (vector, owner): destroys the
    // 14h-byte elements of [first, last).
    virtual void destroy_range_007480a0(Object first, Object last, Object vector, Object owner) = 0;
    // 00746A00 BSP_RefMemberSet_Release_00746a00, __thiscall(set).
    virtual void ref_member_set_release_00746a00(Object set) = 0;
    // 0087C700 BSP_DamageableClass_Destruct, __thiscall(class).
    virtual void damageable_class_destruct_0087c700(Object vehicle_class) = 0;
    // 0074D190 BSP_PointsArray_ReserveCapacity, __thiscall(array, capacity), RET 4.
    virtual void points_array_reserve_0074d190(Object array, std::uint32_t capacity) = 0;
    // 00963600 (named CG_vector_deleting_dtor_00963600), __thiscall(class): the
    // ship class destructor body the submarine class chains to.
    virtual void ship_class_destruct_00963600(Object vehicle_class) = 0;

    // --- segment 65 ---
    // Tree head-node allocation for the subscription sets: __thiscall(tree) ->
    // EAX = the new head (its left, parent and right are set by the caller).
    virtual Object tree_buy_head_0096ba00(Object tree) = 0;
    virtual Object tree_buy_head_0096ba50(Object tree) = 0;
    virtual Object tree_buy_head_0096baa0(Object tree) = 0;
    virtual Object tree_buy_head_004c26b0(Object tree) = 0;  // BSP_RawTreeNode_Allocate18_004c26b0
    // Tree erase(first, last) for the same sets, __thiscall(tree, out iterator*,
    // first {container, node}, last {container, node}), RET 14h.
    virtual void tree_erase_00971840(Object tree, Object out, Object c1, Object n1, Object c2, Object n2) = 0;
    virtual void tree_erase_00971910(Object tree, Object out, Object c1, Object n1, Object c2, Object n2) = 0;
    virtual void tree_erase_009719e0(Object tree, Object out, Object c1, Object n1, Object c2, Object n2) = 0;
    virtual void tree_erase_004d1a50(Object tree, Object out, Object c1, Object n1, Object c2, Object n2) = 0;

    // The state archive reader (vtable calls on the reader, __thiscall):
    // vtable[4](key) RET 8 enters a table, vtable[8]() leaves it, vtable[10h](key,
    // value) RET 10h reads into the value's pointer, vtable[14h](key) RET 8 -> AL
    // tests for the key.
    virtual void reader_vt4_enter(Object reader, TaggedValue key) = 0;
    virtual void reader_vt8_leave(Object reader) = 0;
    virtual void reader_vt10_read(Object reader, TaggedValue key, TaggedValue value) = 0;
    virtual bool reader_vt14_has(Object reader, TaggedValue key) = 0;
    // 009768F0 BSP_Warning_LoadArchive, __thiscall(warning, reader), RET 4.
    virtual void warning_load_archive_009768f0(Object warning, Object reader) = 0;
    // 00450540 STL_CheckedNativeStringVector_Append_00450540, __thiscall(vector, const string*), RET 4.
    virtual void string_vector_append_00450540(Object vector, Object string) = 0;
    // 00976CB0 BSP_WarningManager_ReportFirstString, __thiscall(manager, const
    // vector<string>*, a, b), RET 0Ch.
    virtual void warning_report_first_string_00976cb0(Object manager, Object strings, std::uint32_t a,
                                                      std::uint32_t b) = 0;
    // 00432050 STL_NativeStringRange_Destroy_00432050: ECX = first, EDX = last,
    // two unused stack words, RET 8.
    virtual void string_range_destroy_00432050(Object first, Object last, std::uint32_t unused_a,
                                               std::uint32_t unused_b) = 0;
    // 008EC460 BSP_PowerupRandomNameMap_LookupOrInsert, __thiscall(map, const key string*), RET 4 -> EAX.
    virtual std::uint32_t name_map_lookup_008ec460(Object map, Object key) = 0;
    // 00979B50 BSP_WarningMessageTable_PickVariant, __thiscall(table, entry), RET 4
    // -> EAX = string*. Draws from BSP_RandomThreads_NextU32 (00BD2FC0), not the
    // gameplay stream 00BD2F10.
    virtual Object pick_variant_00979b50(Object table, std::uint32_t entry) = 0;
    // 00976B00 BSP_Warning_AddMessage, __thiscall(warning, const string*), RET 4.
    virtual void warning_add_message_00976b00(Object warning, Object text) = 0;
    // 0097D650 BSP_Warning_ResolveEntitySoundKeys, __thiscall(manager, string*, entity), RET 8.
    virtual void resolve_entity_sound_keys_0097d650(Object manager, Object text, Object entity) = 0;
    // 00803CE0 BSP_Recon_RelationIfIdentified, ECX = party object, EDX = unit -> EAX.
    virtual std::int32_t recon_relation_00803ce0(Object party, Object unit) = 0;

    // Channel dispatch (the warning manager's event channels).
    // [00CE2218] EnterCriticalSection / [00CE2210] LeaveCriticalSection.
    virtual void enter_critical_section(Object section) = 0;
    virtual void leave_critical_section(Object section) = 0;
    // 00980150 BSP_WarningManager_ChannelIndex, __thiscall(channel map, const name*), RET 4 -> EAX.
    virtual Object channel_index_00980150(Object channel_map, Object name) = 0;
    // 00975EA0, __thiscall(vector<arg*>, arg* const*), RET 4: push_back.
    virtual void arg_vector_push_00975ea0(Object vector, const Object* argument) = 0;
    // 0097B8C0 BSP_WarningChannel_Evaluate, __thiscall(manager, channel, const
    // vector<arg*>*, vector<string>* callbacks out), RET 0Ch.
    virtual void channel_evaluate_0097b8c0(Object manager, Object channel, Object arguments, Object callbacks) = 0;
    // The string argument object: 00426060 BSP_NativeString_CopyConstruct,
    // __thiscall(string, const source*), RET 4, builds the by-value copy on the
    // stack; 0096DC00 BSP_WarningEventRecord_Construct, __thiscall(object,
    // string by value), RET 8, EAX = object.
    virtual void string_copy_construct_00426060(Object string, Object source) = 0;
    virtual Object string_arg_construct_0096dc00(Object object, Object string_by_value) = 0;
    // Lua variants (__thiscall(variant, value), RET 4): 00884A90 entity,
    // 00884A00 int, 00884FC0 string, 00884A30 bool word, 008849D0 (the float
    // kind of 00D1AF3C); 00884AF0 the destructor; 006EDF00 push_back(vector, variant*).
    virtual void lua_variant_entity_00884a90(Object variant, Object entity) = 0;
    virtual void lua_variant_int_00884a00(Object variant, std::uint32_t value) = 0;
    virtual void lua_variant_string_00884fc0(Object variant, Object string) = 0;
    virtual void lua_variant_bool_00884a30(Object variant, std::uint32_t word) = 0;
    virtual void lua_variant_008849d0(Object variant, std::uint32_t word) = 0;
    virtual void lua_variant_destruct_00884af0(Object variant) = 0;
    virtual void lua_variant_vector_push_006edf00(Object vector, Object variant) = 0;
    // 006B88E0 BSP_LuaVariantVector_Resize, __thiscall(vector, count), RET 4.
    virtual void lua_variant_vector_resize_006b88e0(Object vector, std::uint32_t count) = 0;
    // 00887E50 BSP_MissionLuaHost_CallNamedThreadSafe, __thiscall(host
    // [game+1A08h], 0, const name*, variants, 0, -1), RET 14h.
    virtual void lua_call_named_00887e50(Object lua_host, std::uint32_t a, Object name, Object variants,
                                         std::uint32_t b, std::int32_t c) = 0;
    // vtable[0](1) on an argument object: its scalar deleting destructor.
    virtual void vt0_delete(Object object, std::uint32_t flags) = 0;
    // 007EDAD0 BSP_PlaneSquadron_AmmoType, __thiscall(squadron) -> EAX.
    virtual std::uint32_t squadron_ammo_type_007edad0(Object squadron) = 0;
    // vtable[10h]() on a generated object -> EAX = const char* (its name).
    virtual const char* vt10_name(Object object) = 0;
    // 0077EDF0 BSP_MessageSystem_EntitySuppressSlot, __thiscall(map +D4h, const
    // entity* key), RET 4 -> EAX = byte*.
    virtual std::uint8_t* entity_suppress_slot_0077edf0(Object map, const Object* entity) = 0;
    // 00974150 BSP_EntityWarning_ConstructWithId, __thiscall(warning, entity,
    // const string* id), RET 8, EAX = warning.
    virtual Object entity_warning_construct_00974150(Object warning, Object entity, Object id) = 0;
    // 009763E0 BSP_WarningManager_Report, __thiscall(manager, warning), RET 4.
    virtual void warning_report_009763e0(Object manager, Object warning) = 0;
    // 008E65E0 STL_Tree_Find_008e65e0, __thiscall(map, iterator* out {container,
    // node}, const key*), RET 8, EAX = out.
    virtual Object tree_find_008e65e0(Object map, Object out_iterator, Object key) = 0;
    // 006E0E00 BSP_ObserverMember9348_Construct, __thiscall(observer, target), RET 4.
    virtual void observer_construct_006e0e00(Object observer, Object target) = 0;
    // 006E0860 BSP_ObserverMember9348_Destruct, __thiscall(observer).
    virtual void observer_destruct_006e0860(Object observer) = 0;
    // 00923810, __thiscall(ship, 1), RET 4 -> EAX (the landed-on object, or 0).
    virtual Object ship_landing_target_00923810(Object ship, std::uint32_t flag) = 0;
    // 00BD2FC0 BSP_RandomThreads_NextU32, ECX = stream -> EAX (not the gameplay stream 00BD2F10).
    virtual std::uint32_t random_threads_next_00bd2fc0(std::uint32_t stream) = 0;
    // 0085CAD0 BSP_Geometry_SegmentOverlapsAabb: ECX = box min, EDX = box max,
    // stack (a, b), RET 8 -> AL.
    virtual bool segment_overlaps_aabb_0085cad0(Object box_min, Object box_max, std::uint32_t a, std::uint32_t b) = 0;
    // 0098AC20 BSP_CollisionNode_TraceSegment, __thiscall(node, a, b, c, d, e), RET 14h.
    virtual void collision_trace_segment_0098ac20(Object node, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                                  std::uint32_t d, std::uint32_t e) = 0;
    // 0098ADD0 BSP_SpatialIndex_QuerySegment, __thiscall(index, from*, to*, a, b, c), RET 14h -> AL.
    virtual bool spatial_query_segment_0098add0(Object index, const float* from, const float* to, std::uint32_t a,
                                                std::uint32_t b, std::uint32_t c) = 0;
    // 0098AD60 BSP_SpatialIndex_CellOfPoint: ECX = out cell {row, column}, EDX = point.
    virtual void spatial_cell_of_point_0098ad60(std::int32_t* cell, const float* point) = 0;
    // 0085BF20 BSP_Aabb_Overlaps: ECX = a min, EDX = a max, stack (b min, b max), RET 8 -> AL.
    virtual bool aabb_overlaps_0085bf20(const float* a_min, const float* a_max, Object b_min, Object b_max) = 0;
    // 0098A920 BSP_SpatialNode_SetLocalBounds, __thiscall(node, const min*, const max*), RET 8.
    virtual void spatial_node_set_bounds_0098a920(Object node, const float* box_min, const float* box_max) = 0;

    // 0095E5B0's callees.
    virtual std::uint8_t global_u8(std::uint32_t address) = 0;
    virtual void vt0_with(Object receiver, std::uint32_t argument) = 0;   // vtable[0](argument), RET 4
    virtual void vt4_with(Object receiver, Object argument) = 0;          // vtable[4](argument), RET 4
    virtual void vt0c(Object receiver) = 0;                                // vtable[0Ch]()
    virtual void vtd8(Object receiver) = 0;                                // vtable[0D8h]()
    virtual void vt1bc_with(Object receiver, std::uint32_t argument) = 0; // vtable[1BCh](argument), RET 4
    // 0095DE00 BSP_Unit_AttachFlagNode, __thiscall(unit) (not reconstructed here).
    virtual void attach_flag_node_0095de00(Object unit) = 0;
    // 0087BF80 BSP_UnitTickableEntity_SEntityInitSlotA4, __thiscall(unit).
    virtual void entity_init_slot_a4_0087bf80(Object unit) = 0;
    // 00956C20 BSP_Unit_RebuildWeaponCategoryIndex, __thiscall(unit).
    virtual void rebuild_weapon_category_index_00956c20(Object unit) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData, __thiscall(unit, out LuaObject*), RET 4.
    virtual void saved_entity_lua_data_009238a0(Object unit, Object out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(reader, LuaObject by value (14h)), RET 14h.
    virtual void lua_reader_construct_004425c0(Object reader, Object lua_object_by_value) = 0;
    // Reader helpers, __thiscall(reader, ...): 00BD8E20 EnterKey(key) RET 8;
    // 00BD7A20 LeaveKey(); 00BD68D0 ReadFieldOrDefault(key, value, default) RET 18h;
    // 00BD6830 ReadField(key, value) RET 10h; 00BD5EB0 HasKey(key) RET 8 -> AL;
    // 00441A20 the destructor.
    virtual void lua_reader_enter_00bd8e20(Object reader, TaggedValue key) = 0;
    virtual void lua_reader_leave_00bd7a20(Object reader) = 0;
    virtual void lua_reader_read_or_default_00bd68d0(Object reader, TaggedValue key, TaggedValue value,
                                                     TaggedValue fallback) = 0;
    virtual void lua_reader_read_00bd6830(Object reader, TaggedValue key, TaggedValue value) = 0;
    virtual bool lua_reader_has_00bd5eb0(Object reader, TaggedValue key) = 0;
    virtual void lua_reader_destruct_00441a20(Object reader) = 0;
    // 006E6360 BSP_SkillLevel_Read, __thiscall(skill, reader), RET 4.
    virtual void skill_level_read_006e6360(Object skill, Object reader) = 0;
    // 00864580 BSP_UnitGunneryAi_Construct, __thiscall(memory) -> EAX.
    virtual Object gunnery_ai_construct_00864580(Object memory) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(bag, name), RET 4 -> EAX = entry.
    virtual Object property_bag_find_008f2260(Object bag, const char* name) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(node, float, flag), RET 8.
    virtual void scene_node_set_visibility_00b6da70(Object node, float factor, std::uint32_t flag) = 0;
};

// --- segment 62 -----------------------------------------------------------

void devices_kind20_call_1ec_00951f80(Seg62Host& host, Object unit);
void device_list_first_00952080(Object unit, const std::int32_t* slot_table, DeviceListIterator* it);
void device_list_next_009520e0(Object unit, const std::int32_t* slot_table, DeviceListIterator* it);
void entity_decay_impulse_00952150(Object entity, float dt);
Object matrix_scale_00952950(Seg62Host& host, Object out, float x, float y, float z);
std::uint32_t* copy_44_00952df0(const std::uint32_t* first, const std::uint32_t* last, std::uint32_t* dest);
std::uint32_t* copy_backward_44_00952f30(const std::uint32_t* first, const std::uint32_t* last, std::uint32_t* dest_end);
bool device_is_gun_kind0_00953170(Seg62Host& host, Object device);
bool device_is_gun_kind2346_009531a0(Seg62Host& host, Object device);
std::uint32_t ship_count_catapulted_00953920(Object ship);
bool ship_spectate_last_catapulted_009539e0(Seg62Host& host, Object ship);
std::uint32_t ship_last_catapulted_00953a60(Object ship);
void unit_gather_gun_ranges_00953b80(Seg62Host& host, Object unit);
void entity_reset_impulse_00954580(Seg62Host& host, Object entity);
std::uint32_t list_value_at_00954df0(Object list, std::int32_t index);
void fill_44_00955090(std::uint32_t* first, std::uint32_t* last, const std::uint32_t* value);
std::uint32_t class_loadout_record_009552e0(Object vehicle_class, std::int32_t loadout);
void owner_register_projectile_009555a0(Seg62Host& host, Object owner, Object shot);
void device_aim_direction_to_local_pair_00955830(Seg62Host& host, Object device, const float* target,
                                                 float* out_a, float* out_b);
void list_push_back_00955c90(Seg62Host& host, Object list, std::uint32_t value);
void list_push_back_00955d00(Seg62Host& host, Object list, std::uint32_t value);
void list_push_back_00955e20(Seg62Host& host, Object list, std::uint32_t value);
void list_clear_free_values_00955d60(Seg62Host& host, Object list);
void list_clear_00955eb0(Seg62Host& host, Object list);
void unit_copy_gun_list_00956b40(Seg62Host& host, Object unit, Object out_list);
void unit_remove_gun_00956ba0(Seg62Host& host, Object unit, Object gun);
void plane_set_gun_fire_effect_00956ee0(Seg62Host& host, Object plane, bool firing);
void entity_set_world_position_00957170(Seg62Host& host, Object entity, const float* world_point);
void flag_select_nation_texture_009574d0(Seg62Host& host, Object unit, Object flag);
void unit_bind_gun_groups_009575d0(Seg62Host& host, Object unit);

void entity_apply_impulse_pose_009580e0(Seg62Host& host, Object entity);
Object copy_14_009588a0(Seg62Host& host, Object first, Object last, Object dest);
Object copy_backward_14_00958910(Seg62Host& host, Object first, Object last, Object dest_end);
Object new_class_entry_00958e90(Seg62Host& host, std::uint32_t a, std::uint32_t b, const Object* reference);

Object record_display_key_00959230(Seg62Host& host, Object out, Object record);

Object uninitialized_copy_14_009597e0(Seg62Host& host, Object first, Object last, Object dest);
void uninitialized_fill_n_14_0095a740(Seg62Host& host, Object dest, std::uint32_t count, Object value);

void vehicle_unit_write_state_0095bb10(Seg62Host& host, Object unit, Object archive);

void camera_array_tidy_0095b8d0(Seg62Host& host, Object array);
void camera_array_tidy_0095c170(Seg62Host& host, Object array);
void camera_array_tidy_0095ca50(Seg62Host& host, Object array);

Object class_preload_alias_0095c1c0(Seg62Host& host, Object out, std::int32_t class_id);

void class_registry_clear_lua_tables_0095ca70(Seg62Host& host);

Object ammo_record_assign_0095cf80(Seg62Host& host, Object record, Object source);

void vector44_resize_default_0095d120(Seg62Host& host, Object vector, std::uint32_t count);
void unit_clear_vector660_0095d3c0(Seg62Host& host, Object unit);
void unit_release_effects_0095d400(Seg62Host& host, Object unit);

void class_model_data_destruct_0095db40(Seg62Host& host, Object data);

void vehicle_class_destruct_base_0095f1c0(Seg62Host& host, Object vehicle_class);
void vehicle_class_destruct_009600e0(Seg62Host& host, Object vehicle_class);
void vehicle_class_destruct_009601c0(Seg62Host& host, Object vehicle_class);
void submarine_class_destruct_00963f90(Seg62Host& host, Object vehicle_class);

void fire_subscription_destruct_0097ab70(Seg62Host& host, Object s);
Object repair_subscription_construct_0097ac10(Seg62Host& host, Object s);
void repair_subscription_destruct_0097acc0(Seg62Host& host, Object s);
Object target_subscription_construct_0097ad80(Seg62Host& host, Object s);
void target_subscription_destruct_0097ae30(Seg62Host& host, Object s);
Object ammo_type_subscription_construct_0097aef0(Seg62Host& host, Object s);
void ammo_type_subscription_destruct_0097afa0(Seg62Host& host, Object s);
Object stock_subscription_construct_0097b060(Seg62Host& host, Object s);
void stock_subscription_destruct_0097b140(Seg62Host& host, Object s);
Object ship_landed_subscription_construct_0097b230(Seg62Host& host, Object s);
void ship_landed_subscription_destruct_0097b2b0(Seg62Host& host, Object s);
Object hit_subscription_construct_0097c2c0(Seg62Host& host, Object s);
void hit_subscription_destruct_0097c410(Seg62Host& host, Object s);
Object failure_subscription_construct_0097c560(Seg62Host& host, Object s);
void failure_subscription_destruct_0097c610(Seg62Host& host, Object s);
Object zone_subscription_construct_0097c6d0(Seg62Host& host, Object s);
void zone_subscription_destruct_0097c7b0(Seg62Host& host, Object s);
Object command_event_subscription_construct_0097c8a0(Seg62Host& host, Object s);
void command_event_subscription_destruct_0097c9b0(Seg62Host& host, Object s);
Object gui_subscription_construct_0097cad0(Seg62Host& host, Object s);
void gui_subscription_destruct_0097cb80(Seg62Host& host, Object s);
Object generate_subscription_construct_0097cc40(Seg62Host& host, Object s);
void generate_subscription_destruct_0097ccf0(Seg62Host& host, Object s);
Object player_subscription_construct_0097cdb0(Seg62Host& host, Object s);
void player_subscription_destruct_0097ce60(Seg62Host& host, Object s);
Object music_over_subscription_construct_0097cf20(Seg62Host& host, Object s);
void music_over_subscription_destruct_0097cfa0(Seg62Host& host, Object s);
Object chat_subscription_construct_0097d040(Seg62Host& host, Object s);
void chat_subscription_destruct_0097d120(Seg62Host& host, Object s);
Object entity_killed_subscription_construct_0097d210(Seg62Host& host, Object s);
void entity_killed_subscription_destruct_0097d2c0(Seg62Host& host, Object s);
Object hp_event_subscription_construct_0097d380(Seg62Host& host, Object s);
void hp_event_subscription_destruct_0097d430(Seg62Host& host, Object s);

void string_list_warning_load_0097b4d0(Seg62Host& host, Object warning, Object reader);

void session_warning_string_message_0097b960(Seg62Host& host, Object manager, Object message);

void entity_warning_activate_0097f490(Seg62Host& host, Object warning);
void recon_warning_activate_0097f7e0(Seg62Host& host, Object warning);
void kill_warning_activate_0097f970(Seg62Host& host, Object warning);
void hit_warning_activate_0097fa90(Seg62Host& host, Object warning);
void exit_zone_warning_activate_0097fbb0(Seg62Host& host, Object warning);
void confirm_warning_activate_0097fcb0(Seg62Host& host, Object warning);
void new_unit_warning_activate_0097fd90(Seg62Host& host, Object warning);
void type9_warning_activate_0097fe70(Seg62Host& host, Object warning);
void powerup_warning_activate_0097ff90(Seg62Host& host, Object warning);
void warning_activate_00980070(Seg62Host& host, Object warning);

void dispatch_surrender_00982990(Seg62Host& host, Object manager, std::uint32_t value);
void dispatch_repair_00983c10(Seg62Host& host, Object manager, Object entity, std::uint32_t setting);
void dispatch_zone_00983f50(Seg62Host& host, Object manager, Object entity, Object zone, std::uint32_t flag);
void dispatch_stock_00984eb0(Seg62Host& host, Object manager, Object entity, std::uint32_t a, std::uint32_t b);
void dispatch_gui_00985250(Seg62Host& host, Object manager, Object text, std::uint32_t flag);
void dispatch_player_00985920(Seg62Host& host, Object manager, std::uint32_t player, Object text);
void dispatch_music_over_00985c50(Seg62Host& host, Object manager, Object text);

void dispatch_ammo_type_00984ba0(Seg62Host& host, Object manager, Object squadron);
void dispatch_generate_00985590(Seg62Host& host, Object manager, Object generated);

void dispatch_leak_009832f0(Seg62Host& host, Object manager, Object entity, float seconds);
void dispatch_fire_00983780(Seg62Host& host, Object manager, Object entity, float seconds);

void dispatch_failure_00982c50(Seg62Host& host, Object manager, Object entity, Object kind, float seconds);

void report_target_00984800(Seg62Host& host, Object manager, Object entity, Object target);
void report_ship_landed_00986820(Seg62Host& host, Object manager, Object ship);

Object pick_sound_key_0097dc80(Seg62Host& host, Object manager, Object out, Object keys, Object entity);
void node_bounds_0098a710(Object node, float* box_min, float* box_max);

void collision_trace_segment_gated_0098b9c0(Seg62Host& host, Object node, std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d);

bool spatial_sweep_segments_0098b370(Seg62Host& host, Object index, Object segments, std::uint32_t a, std::uint32_t b);
bool spatial_any_ship_in_box_0098b450(Seg62Host& host, Object index, const float* box_min, const float* box_max);

void spatial_node_union_shape_bounds_0098b7d0(Seg62Host& host, Object node);

void unit_read_common_lua_properties_0095e5b0(Seg62Host& host, Object unit);

}  // namespace bsp::cc13_seg62
