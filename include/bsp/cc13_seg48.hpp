#pragma once

// cc13_seg48: segment 48 (007D1E50-007F8390) leftovers after cc13-gpr48:
// plane-unit list removal, plane neighbour and effects-component plumbing,
// squadron member broadcasts, the gun-platform destructor and the small
// session messages (packet cc13-seg48, docs/CC13_SEG48.md). Struck sub-ranges
// 007DF000-007DFFFF and 007EE000-007F2FFF are not touched.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the
// original ABI; each function's comment gives the native ABI. Native objects
// are raw byte blocks touched only at the offsets the image reads or writes,
// and every call the image makes goes through a named Cc13Seg48Host method at
// the same point with the same arguments. Nothing here is wired into the game.
// Build-tested only.

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_seg48_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

class Cc13Seg48Host {
public:
    virtual ~Cc13Seg48Host() = default;

    // ---- CRT and Win32 ----
    // 00BF6989 and 00BF65AC are two _free thunks; the image uses both.
    virtual void crt_free_00bf6989(void* block) = 0;
    virtual void crt_free_00bf65ac(void* block) = 0;
    // 00BF55BE and 00BF681B operator new (cdecl, size).
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // [00CE2220] InterlockedDecrement / [00CE221C] InterlockedIncrement.
    virtual std::int32_t interlocked_decrement(volatile std::int32_t* value) = 0;
    virtual std::int32_t interlocked_increment(volatile std::int32_t* value) = 0;

    // ---- virtual slots (ECX = the object) ----
    // Reference-counted object slot +0h with no arguments (the release /
    // self-delete the image calls when the count at +4h reaches zero).
    virtual void vcall_00_release(void* object) = 0;
    // Slot +0h with one dword (scalar deleting destructor, flag 1).
    virtual void vcall_00_delete(void* object, std::uint32_t flags) = 0;
    // Unit slot +5Ch(kind), AL (the ordnance-kind query).
    virtual bool vcall_5c_has_ordnance_kind(void* unit, std::int32_t kind) = 0;
    // Squadron member slots +148h(a, b), +14Ch(a, b, c), +40h(a).
    virtual void vcall_148(void* member, std::uint32_t a, std::uint32_t b) = 0;
    virtual void vcall_14c(void* member, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void vcall_40(void* member, std::uint32_t a) = 0;

    // ---- units and lists ----
    // 0074E610 BSP_PlaneUnit_DetachFromWorldLists: __thiscall(ECX = unit).
    virtual void plane_unit_detach_from_world_lists_0074e610(void* unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode: __thiscall(ECX = list, node), RET 4.
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;
    // 00B6DB60 BSP_Node_GetLocalMatrix: __thiscall(ECX = node), EAX = float[16].
    virtual const float* node_get_local_matrix_00b6db60(void* node) = 0;

    // ---- callback buffers and observers ----
    // 00695870 BSP_CallbackOwner_Destroy: __thiscall(ECX = owner).
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 00695830 BSP_CallbackOwner_DetachAllIfBound: __thiscall(ECX = owner).
    virtual void callback_owner_detach_all_if_bound_00695830(void* owner) = 0;
    // 00694A60 BSP_Observer_RegisterPair: __fastcall(ECX = observed, EDX = owner).
    virtual void observer_register_pair_00694a60(void* observed, void* owner) = 0;
    // 006952A0 BSP_Observer_UnregisterPair: __fastcall(ECX = observed, EDX = owner).
    virtual void observer_unregister_pair_006952a0(void* observed, void* owner) = 0;
    // 006CDD70 BSP_FollowObserverMember_Destroy: __thiscall(ECX = member).
    virtual void follow_observer_member_destroy_006cdd70(void* member) = 0;
    // 00875B30 BSP_TickSubnode_BaseCleanup: __thiscall(ECX = node).
    virtual void tick_subnode_base_cleanup_00875b30(void* node) = 0;

    // ---- effects ----
    // 00867B10 BSP_PointEffect_StopChildren: __thiscall(ECX = effect).
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // 004C9550 BSP_LiveEffectReferences_Resize: __thiscall(ECX = vector, int), RET 4.
    virtual void live_effect_references_resize_004c9550(void* vector, std::int32_t size) = 0;
    // 007C3000 (unnamed): __thiscall(ECX = pod array {data, count, cap}, int), RET 4.
    virtual void fun_007c3000(void* array, std::int32_t value) = 0;
    // 007EAE30 BSP_EffectHolder_EnsureStarted_007eae30: __thiscall(ECX = holder, char restart), RET 4.
    virtual void effect_holder_ensure_started_007eae30(void* holder, std::uint8_t restart) = 0;
    // 007EB160 BSP_PlaneFxComponent_StartClassLoops: __thiscall(ECX = component).
    virtual void plane_fx_component_start_class_loops_007eb160(void* component) = 0;

    // ---- arrays and lists (engine templates) ----
    // 007BD940 BSP_PodArray4_Reserve_007bd940: __thiscall(ECX = {data, count, cap}, int), RET 4.
    virtual void pod_array4_reserve_007bd940(void* array, std::int32_t capacity) = 0;
    // 007F7070 BSP_List_Clear_007f7070 (STL list clear): __thiscall(ECX = list).
    virtual void list_clear_007f7070(void* list) = 0;
    // 00487110 BSP_AwardIndexList_DestroyActualStorage: __thiscall(ECX = list).
    virtual void award_index_list_destroy_actual_storage_00487110(void* list) = 0;
    // 00BD30F0 BSP_RefCounted_DestroyBase: __thiscall(ECX = object).
    virtual void refcounted_destroy_base_00bd30f0(void* object) = 0;

    // ---- squadrons and weapons ----
    // 00721070 BSP_CommandTargetDescriptor_Init: __thiscall(ECX = descriptor).
    virtual void command_target_descriptor_init_00721070(void* descriptor) = 0;
    // 00927D20 (unnamed, the squadron's base for slot +148h): __thiscall(ECX = squadron, a, b), RET 8.
    virtual void fun_00927d20(void* squadron, std::uint32_t a, std::uint32_t b) = 0;
    // 00927E50 (unnamed, the base for slot +14Ch): __thiscall(ECX = squadron, a, b, c), RET 0Ch.
    virtual void fun_00927e50(void* squadron, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    // 007C3F60 BSP_WeaponController_FirstSlotOrdnance: __thiscall(ECX = unit), EAX.
    virtual std::uint32_t weapon_controller_first_slot_ordnance_007c3f60(void* unit) = 0;
    // 007B9410 BSP_WeaponController_FindReadyRocket: __thiscall(ECX = unit, arg), RET 4, AL.
    virtual bool weapon_controller_find_ready_rocket_007b9410(void* unit, std::uint32_t arg) = 0;

    // ---- session messages ----
    // 00766F60 BSP_NetEntityCreateMessage_ConstructForRead: __thiscall(ECX = message).
    virtual void net_entity_create_message_construct_for_read_00766f60(void* message) = 0;
    // 007668E0 BSP_NetEntityCreateMessage_Destruct: __thiscall(ECX = message).
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase: __thiscall(ECX = message, int type), RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::int32_t type) = 0;
    // 00428FF0 BSP_BitCursor_WriteByteBits: __thiscall(ECX = cursor, byte value, int bits), RET 8.
    virtual void bit_cursor_write_byte_bits_00428ff0(void* cursor, std::uint32_t value, std::int32_t bits) = 0;
    // 00429090 BSP_BitCursor_WriteSignedDwordBits: __thiscall(ECX = cursor, dword value, int bits), RET 8.
    virtual void bit_cursor_write_signed_dword_bits_00429090(void* cursor, std::uint32_t value,
                                                             std::int32_t bits) = 0;
    // 00428C70 BSP_BitCursor_ReadUnsignedByte: __thiscall(ECX = cursor, byte* out, int bits), RET 8.
    virtual void bit_cursor_read_unsigned_byte_00428c70(void* cursor, std::uint8_t* out, std::int32_t bits) = 0;
    // 00428D30 BSP_BitCursor_ReadSignedDword: __thiscall(ECX = cursor, dword* out, int bits), RET 8.
    virtual void bit_cursor_read_signed_dword_00428d30(void* cursor, std::uint32_t* out, std::int32_t bits) = 0;

    // ---- strings ----
    // 00467310 BSP_FormatToBuffer32: cdecl(char* buffer, const char* format, ...).
    virtual void format_to_buffer32_00467310(char* buffer, const char* format, std::int32_t value) = 0;
    // 00408720 BSP_CharacterString_AssignCounted: __thiscall(ECX = string, const char*, count), RET 8.
    virtual void character_string_assign_counted_00408720(void* string, const char* text, std::uint32_t count) = 0;

    // ---- save-game archives ----
    // Archive writer slot +0Ch(name word 0, const char* key, int type, dword
    // value): type 2 = float bits, 3 = byte (the image pushes a dword whose
    // upper 24 bits are stale stack; the low byte is the value), 5 = pointer
    // to a float[3].
    virtual void archive_vcall_0c_put(void* writer, std::uint32_t name_word0, const char* key, std::int32_t type,
                                      std::uint32_t value) = 0;
    // Archive reader slot +10h(name word 0, const char* key, int type, void* out).
    virtual void archive_vcall_10_get(void* reader, std::uint32_t name_word0, const char* key, std::int32_t type,
                                      void* out) = 0;
    // Archive slot +4h(name word 0, const char* key) and slot +8h(), the pair
    // the plane effects archive issues for "engines".
    virtual void archive_vcall_04(void* archive, std::uint32_t name_word0, const char* key) = 0;
    virtual void archive_vcall_08(void* archive) = 0;
    // 00BD4FE0 / 00BD5010: __thiscall(ECX = parent ref, out ref*, name word 0,
    // const char* key), RET 0Ch, EAX = out: the child reader / writer reference.
    virtual void* archive_reader_child_00bd4fe0(void* parent_ref, void* out_ref, std::uint32_t name_word0,
                                                const char* key) = 0;
    virtual void* archive_writer_child_00bd5010(void* parent_ref, void* out_ref, std::uint32_t name_word0,
                                                const char* key) = 0;
    // 00410630 BSP_ArchiveReaderRef_Destroy / 004107B0
    // BSP_ArchiveWriterRef_Destroy: __thiscall(ECX = ref).
    virtual void archive_reader_ref_destroy_00410630(void* ref) = 0;
    virtual void archive_writer_ref_destroy_004107b0(void* ref) = 0;
    // 007D9C10 BSP_PlaneFlightController_RefreshBodyFrame and 007D9C80
    // BSP_PlaneFlightController_BodyToWorldVelocity: __thiscall(ECX = controller).
    virtual void plane_flight_controller_refresh_body_frame_007d9c10(void* controller) = 0;
    virtual void plane_flight_controller_body_to_world_velocity_007d9c80(void* controller) = 0;
    // Sound element slot +18h(dword a, float b), RET 8.
    virtual void vcall_18_sound(void* element, std::uint32_t a, float b) = 0;

    // ---- per-member ordnance predicates: __thiscall(ECX = member unit, int
    // loadout), RET 4, AL. The 007ED790 family asks them of every member. ----
    virtual bool unit_can_drop_ordnance_007b9140(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_general_bomb_ordnance_007b9320(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_drop_kamikaze_ordnance_007b93e0(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_torpedo_ordnance_007b93f0(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_rocket_kind33_007b9400(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_rocket_ordnance_007b9480(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_depth_charge_ordnance_007b94f0(void* member, std::uint32_t loadout) = 0;
    virtual bool weapon_controller_has_level_bomb_ordnance_007b9500(void* member, std::uint32_t loadout) = 0;

    // ---- plane unit classes ----
    // 007CFD20 BSP_PlaneUnitInstance_Construct: __thiscall(ECX = unit, arg, int), RET 8.
    virtual void plane_unit_instance_construct_007cfd20(void* unit, std::uint32_t arg, std::int32_t flag) = 0;
    // 00BF79F0 memset (cdecl).
    virtual void crt_memset_00bf79f0(void* block, std::int32_t value, std::uint32_t size) = 0;
    // 009553D0 BSP_Unit_SetVehicleClass: __thiscall(ECX = unit, class), RET 4.
    virtual void unit_set_vehicle_class_009553d0(void* unit, void* vehicle_class) = 0;

    // ---- squadron members and command queue ----
    // 0071BE40 BSP_WeaponDirector_CurrentCommand: __thiscall(ECX = command block), EAX.
    virtual std::uint32_t weapon_director_current_command_0071be40(void* block) = 0;
    // 007BCB30 BSP_Plane_ReEquip: __thiscall(ECX = plane, int ammo type), RET 4.
    virtual void plane_re_equip_007bcb30(void* plane, std::uint32_t ammo_type) = 0;
    // 007C2130 BSP_Plane_RequestFlightStateOne: __thiscall(ECX = plane).
    virtual void plane_request_flight_state_one_007c2130(void* plane) = 0;
    // 007B8BD0 BSP_Plane_IsFreeFlightOrParked: __thiscall(ECX = plane), AL.
    virtual bool plane_is_free_flight_or_parked_007b8bd0(void* plane) = 0;

    // ---- squadron entity, game globals, front end ----
    // The image's global dword at `address` ([00E188D8] controlled unit,
    // [00E188A8] game, [00E198C4] front-end manager).
    virtual std::uint32_t* image_global(std::uint32_t address) = 0;
    // 007F2C60 BSP_PlaneSquadronTickableEntity_Construct: __thiscall(ECX =
    // block, zero-extended word id), RET 4, EAX = entity.
    virtual void* plane_squadron_construct_007f2c60(void* block, std::uint32_t id) = 0;
    // 0041DD40 BSP_NativeString_Resize: __thiscall(ECX = {size, data}, length, int), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t keep) = 0;
    // 00BF7680 memcpy (cdecl).
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t size) = 0;
    // 00565FB0 BSP_HudLimboScreen_SetState: __thiscall(ECX = screen, state), RET 4.
    virtual void hud_limbo_screen_set_state_00565fb0(void* screen, std::uint32_t state) = 0;
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest: __thiscall(ECX = manager, int, int), RET 8.
    virtual void front_end_push_interface_request_004cc460(void* manager, std::int32_t request, std::int32_t arg) = 0;
    // Front-end manager slot +10h(a, b), RET 8.
    virtual void vcall_10_front_end(void* manager, std::uint32_t a, std::uint32_t b) = 0;
    // 004C0890 BSP_Game_SetControlledUnit: __fastcall(ECX = unit or null).
    virtual void game_set_controlled_unit_004c0890(void* unit) = 0;
    // 0077D270 BSP_UnitInstance_OnKilledLua: __thiscall(ECX = unit).
    virtual void unit_instance_on_killed_lua_0077d270(void* unit) = 0;

    // ---- gun platforms, nose art ----
    // 004870F0 STL_List_BuyHeadNode_004870f0 / 007F6D30 BSP_List_BuyHeadNode_007f6d30
    // (STL list head nodes): __thiscall(ECX = list), EAX = head node.
    virtual void* list_buy_head_node_004870f0(void* list) = 0;
    virtual void* list_buy_head_node_007f6d30(void* list) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87: __thiscall(ECX = destination, const float[16]), RET 4.
    virtual void matrix_copy4x4_004134f0(void* destination, const float* source) = 0;
    // 0076A9F0 CG_adjustor_thunk_0076a9f0: __thiscall(ECX = game+1EF0h, message, int), RET 8.
    virtual void session_forward_message_0076a9f0(void* receiver, void* message, std::int32_t flag) = 0;

    // ---- plane neighbours and effects (more) ----
    // 007E11D0 (the neighbour refresh): __thiscall(ECX = neighbours).
    virtual void plane_neighbours_refresh_007e11d0(void* neighbours) = 0;
    // Engine record slot +4h(dword state), RET 4.
    virtual void vcall_04_engine(void* engine, std::uint32_t state) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix: __fastcall(ECX = out
    // ref*, EDX = parent, definition ref by value (consumed: the caller has
    // already added its reference), const matrix*, transform, option, tail),
    // RET 14h, EAX = out.
    virtual void** point_effect_create_with_parent_matrix_008687c0(void** out, void* parent, void* definition,
                                                                   const void* matrix, std::uint32_t transform,
                                                                   std::uint32_t option, std::uint32_t tail) = 0;
    // 0074D780 BSP_LiveEffectReferences_Append: __thiscall(ECX = vector, const ref*), RET 4.
    virtual void live_effect_references_append_0074d780(void* vector, void* const* ref) = 0;
    // 00866B70 BSP_EffectHandle_RequestStop: __thiscall(ECX = effect).
    virtual void effect_handle_request_stop_00866b70(void* effect) = 0;
    // 004845D0 BSP_RetainedReference_Assign_004845D0: __thiscall(ECX = slot, const ref*), RET 4.
    virtual void retained_reference_assign_004845d0(void* slot, void* const* ref) = 0;
    // 0041E870 BSP_NativeString_Assign: __thiscall(ECX = {size, data}, const char*), RET 4.
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    // 00866CD0 BSP_PointEffect_FindOrCreateNamedChild: __thiscall(ECX = effect,
    // ref* out, const native string* name), RET 8, EAX = out.
    virtual void** point_effect_find_or_create_named_child_00866cd0(void* effect, void** out, const void* name) = 0;
    // 006FBEB0 BSP_RetainedReference_AssignFromSlot: __thiscall(ECX = slot, ref* source), RET 4.
    virtual void retained_reference_assign_from_slot_006fbeb0(void* slot, void** source) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton (no arguments, EAX) and
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool, data,
    // size, int), RET 0Ch.
    virtual void* sized_pool_get_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size, std::int32_t flag) = 0;
    // 00876020 BSP_TickSubnode_Reparent: __thiscall(ECX = owner+310h or null, node), RET 4.
    virtual void tick_subnode_reparent_00876020(void* parent, void* node) = 0;
    // 00412E20 BSP_Math_TangentX87Float: (float) RET 4, ST0.
    virtual float math_tangent_00412e20(float angle) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange: __fastcall(ECX = stream, float
    // lo, float hi), RET 8, ST0. The process-wide generator (one shared state).
    virtual float random_uniform_float_range_00bd2f10(std::uint32_t stream, float lo, float hi) = 0;
};

// A save-game archive reference as the image passes it by value (three
// dwords; the reader or writer object is the third). Functions that take one
// destroy it before returning, as MSVC callee-destroyed by-value parameters do.
// The image builds a child reference directly in the argument slot; here it is
// built in a local and copied into the parameter, which the host cannot tell
// apart unless it tracks the reference's address.
struct Cc13Seg48ArchiveRef {
    std::uint32_t words[3];
};

// The image's 1.0f at 00D7A24C (bytes 00 00 80 3F).
constexpr float kCc13Seg48One = 1.0f;

// ---- plane units ----
void cc13_seg48_mplane_bomber_unregister_from_world_lists_007d7930(void* unit, Cc13Seg48Host& host);
void cc13_seg48_mplane_fighter_unregister_from_world_lists_007ddbc0(void* unit, Cc13Seg48Host& host);
void cc13_seg48_owned_ptr_holder_destruct_007d7a10(void* holder, Cc13Seg48Host& host);
std::int32_t cc13_seg48_plane_kamikaze_gate_predicate_007ddcb0(void* unit, Cc13Seg48Host& host);
void* cc13_seg48_part_record_ref_construct_007ddd60(void* record, std::uint32_t owner, void* node, float weight,
                                                    Cc13Seg48Host& host);
void cc13_seg48_vector3_fill_one_007ddde0(float* v);
void cc13_seg48_plane_sub_record_init_defaults_007dde00(void* record);
void cc13_seg48_singleton_f87440_destruct_unwind_007ddfb0(void* object, std::uint32_t* global_00f87440);
void* cc13_seg48_plane_record_init_zero_007e2510(void* record, std::uint32_t value);
void* cc13_seg48_mplane_bomber_construct_007d7720(void* unit, std::uint32_t arg, Cc13Seg48Host& host);
std::uint32_t cc13_seg48_mplane_bomber_slot0_return7_007d7790();
void* cc13_seg48_mplane_bomber_class_create_007d7850(void* vehicle_class, std::uint32_t arg, Cc13Seg48Host& host);
void* cc13_seg48_mplane_fighter_construct_007dd9b0(void* unit, std::uint32_t arg, Cc13Seg48Host& host);
std::uint32_t cc13_seg48_mplane_fighter_slot0_return0a_007dda20();
void* cc13_seg48_mplane_fighter_class_create_007ddae0(void* vehicle_class, std::uint32_t arg, Cc13Seg48Host& host);
void cc13_seg48_pilot_plan_reset_command_timer_007d7a40(void* plan);
void cc13_seg48_plane_sub_block_reset_timer40_007ddea0(void* block);

// ---- engine pod arrays {data, count, capacity} ----
void cc13_seg48_pod_array_free_buffer_007de030(void* array, Cc13Seg48Host& host);
void cc13_seg48_ptr_array_grow_007de0b0(void* array, Cc13Seg48Host& host);
void cc13_seg48_ptr_array_push_back_007de6b0(void* array, const std::uint32_t* value, Cc13Seg48Host& host);
void cc13_seg48_pod_array4_push_back_007e0fe0(void* array, const std::uint32_t* value, Cc13Seg48Host& host);
void cc13_seg48_pod_array4_push_back_007e1cf0(void* array, const std::uint32_t* value, Cc13Seg48Host& host);
void cc13_seg48_pod_array_free_buffer_unwind_007edd00(void* array, Cc13Seg48Host& host);
void cc13_seg48_ptr_array_reserve_007edd70(void* array, std::uint32_t capacity, Cc13Seg48Host& host);
void cc13_seg48_ptr_array_grow_007edde0(void* array, Cc13Seg48Host& host);
void cc13_seg48_pod_array_free_buffer_unwind_007f5570(void* array, Cc13Seg48Host& host);

// ---- callback buffers and plane neighbours ----
void cc13_seg48_callback_buffer_destruct_007de420(void* buffer, Cc13Seg48Host& host);
void cc13_seg48_callback_owner_reset_007de690(void* owner, Cc13Seg48Host& host);
void cc13_seg48_callback_buffer_destruct_007dec80(void* buffer, Cc13Seg48Host& host);
void cc13_seg48_plane_neighbours_destruct_007decf0(void* neighbours, Cc13Seg48Host& host);
void cc13_seg48_plane_neighbours_append_observed_007e0f20(void* buffer, void* observed, Cc13Seg48Host& host);
void cc13_seg48_plane_neighbours_clear_target_observer_007e2070(void* neighbours, Cc13Seg48Host& host);
void* cc13_seg48_plane_neighbours_construct_007e1e20(void* neighbours, void* owner, Cc13Seg48Host& host);
void cc13_seg48_plane_neighbours_advance_timers_007e2010(void* neighbours, float dt, Cc13Seg48Host& host);
void cc13_seg48_plane_fx_component_start_class_loops_007eb160(void* component, Cc13Seg48Host& host);
void cc13_seg48_effect_holder_ensure_started_007eae30(void* holder, std::uint8_t restart, Cc13Seg48Host& host);
void cc13_seg48_plane_fx_component_on_flight_state_007eb270(void* component, std::uint8_t flying,
                                                            Cc13Seg48Host& host);

// ---- reference pointers and effect holders ----
void* cc13_seg48_ref_ptr_assign_007e1020(void* slot, void* value, Cc13Seg48Host& host);
void cc13_seg48_ref_ptr_release_at4_007e2360(void* holder, Cc13Seg48Host& host);
void cc13_seg48_ref_ptr_release_at8_007f6fa0(void* holder, Cc13Seg48Host& host);
void cc13_seg48_ref_ptr_release_at8_007f7010(void* holder, Cc13Seg48Host& host);
void cc13_seg48_effect_holder_stop_effect_c_007e1b30(void* holder, Cc13Seg48Host& host);
void cc13_seg48_effect_vector_destruct_007e2240(void* holder, Cc13Seg48Host& host);
void cc13_seg48_plane_engine_base_bind_007e2300(void* engine, std::uint32_t owner, void* const* definition,
                                                std::uint32_t value_24, std::uint32_t value_18,
                                                Cc13Seg48Host& host);
void cc13_seg48_plane_effect_slot_destruct_007e2550(void* slot, Cc13Seg48Host& host);
void cc13_seg48_plane_fx_component_stop_all_effects_007eb0e0(void* component, Cc13Seg48Host& host);
void cc13_seg48_plane_fx_component_restart_007eb350(void* component, Cc13Seg48Host& host);
void cc13_seg48_plane_fx_component_destruct_007ec490(void* component, Cc13Seg48Host& host);

// ---- squadrons ----
void* cc13_seg48_command_target_pair_construct_007ecf00(void* pair, Cc13Seg48Host& host);
void* cc13_seg48_command_target_pair_d086f4_construct_007ecf20(void* pair, Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_broadcast_slot148_007ed020(void* squadron, std::uint32_t a, std::uint32_t b,
                                                          Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_broadcast_slot14c_007ed070(void* squadron, std::uint32_t a, std::uint32_t b,
                                                          std::uint32_t c, Cc13Seg48Host& host);
std::int32_t cc13_seg48_plane_squadron_member_index_of_007ed160(void* squadron, std::uint32_t member);
void cc13_seg48_plane_squadron_broadcast_slot40_007ed1d0(void* squadron, std::uint32_t a, Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_set_flag361_007ed3e0(void* squadron);
std::uint32_t cc13_seg48_plane_squadron_leader_first_ordnance_007ed500(void* squadron, Cc13Seg48Host& host);
bool cc13_seg48_plane_squadron_any_ready_rocket_007ed9c0(void* squadron, std::uint32_t arg, Cc13Seg48Host& host);
std::uint32_t cc13_seg48_plane_squadron_get_ai_command_block_007ecfd0(void* squadron);
std::uint32_t cc13_seg48_plane_squadron_current_command_token_007ed580(void* squadron, Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_reset_travel_speed_007ed590(void* squadron);
void cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(void* squadron);
void cc13_seg48_plane_squadron_re_equip_all_007ed690(void* squadron, std::uint32_t ammo_type, Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_stow_members_inside_007ed6e0(void* squadron, Cc13Seg48Host& host);
bool cc13_seg48_plane_squadron_all_free_flight_or_parked_007ed740(void* squadron, Cc13Seg48Host& host);
std::uint8_t cc13_seg48_plane_squadron_leader_flag_c24_007edac0(void* squadron);
void* cc13_seg48_session_message_entity_relocate_construct_heading_007edeb0(void* message, float heading,
                                                                            std::uint32_t value_28,
                                                                            Cc13Seg48Host& host);
void* cc13_seg48_session_message_entity_relocate_construct_position_007edf00(void* message, const float* position,
                                                                             std::uint32_t value_28,
                                                                             Cc13Seg48Host& host);
void* cc13_seg48_plane_squadron_clone_from_007f3500(void* source, Cc13Seg48Host& host);
void cc13_seg48_plane_squadron_release_controlled_unit_007f3a60(void* squadron, Cc13Seg48Host& host);
// The "any member" ordnance loops: __thiscall(ECX = squadron, int loadout),
// RET 4, AL; true at the first member (ECX = [squadron+3D0h+4i]) whose
// predicate answers. Each member is asked, not only the slot-0 plane.
bool cc13_seg48_plane_squadron_any_can_drop_ordnance_007ed790(void* squadron, std::uint32_t loadout,
                                                              Cc13Seg48Host& host);
bool cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_unit_has_level_bomb_ordnance_007ed830(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_unit_has_drop_kamikaze_ordnance_007ed880(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_plane_squadron_any_has_rockets_007ed920(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_unit_has_rocket_ordnance_007ed970(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);
bool cc13_seg48_unit_has_depth_charge_ordnance_007eda10(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host);

// ---- session messages ----
void* cc13_seg48_session_message_planesquadron_create_construct_for_read_007edb80(void* message,
                                                                                  Cc13Seg48Host& host);
void cc13_seg48_session_message_planesquadron_create_destruct_007edc10(void* message, Cc13Seg48Host& host);
void* cc13_seg48_session_message_unitexited_construct_007edf50(void* message, std::uint32_t unit_id,
                                                               Cc13Seg48Host& host);
void cc13_seg48_session_message_byte_vec3_word_write_bits_007f7cf0(void* message, void* cursor,
                                                                   Cc13Seg48Host& host);
void cc13_seg48_session_message_byte_vec3_word_read_bits_007f7d40(void* message, void* stream,
                                                                  Cc13Seg48Host& host);

// ---- strings and gun platforms ----
void* cc13_seg48_debug_string_from_int_007f3df0(void* out, std::int32_t value, Cc13Seg48Host& host);
void cc13_seg48_gun_platform_destruct_007f7300(void* platform, Cc13Seg48Host& host);
void* cc13_seg48_gun_platform_construct_007f7110(void* platform, Cc13Seg48Host& host);
std::uint8_t cc13_seg48_gun_platform_has_only_seed_window_007f6ca0(void* platform);
void cc13_seg48_pod_array20_grow_007f5800(void* array, Cc13Seg48Host& host);
void* cc13_seg48_pod_array20_insert_007f5580(void* array, void* where, const std::uint32_t* value,
                                             Cc13Seg48Host& host);
void cc13_seg48_pod_array20_push_back_007f6a50(void* array, const std::uint32_t* element, Cc13Seg48Host& host);
bool cc13_seg48_player_record_is_talking_007f74b0(void* record, Cc13Seg48Host& host);
void cc13_seg48_nose_art_set_selection_007f81c0(void* record, const std::uint32_t* selection, Cc13Seg48Host& host);
void cc13_seg48_session_receive_message_e3_007f8210(void* message, Cc13Seg48Host& host);

// ---- save-game archives (keys are the image's strings) ----
void cc13_seg48_actuator_channel_load_007de880(void* channel, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_actuator_channel_save_007de900(void* channel, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_lerp_value_load_007e0740(void* value, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_lerp_value_save_007e07e0(void* value, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_lerp_pair_load_007e08b0(void* pair, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_lerp_pair_save_007e0960(void* pair, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_timed_flag_load_007e0a10(void* flag, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_timed_flag_save_007e0ab0(void* flag, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_state_block_load_007e0b80(void* block, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_state_block_save_007e0d20(void* block, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host);
void cc13_seg48_plane_flight_controller_save_007d9b70(void* controller, void* writer, Cc13Seg48Host& host);
void cc13_seg48_plane_flight_controller_load_007db430(void* controller, void* reader, Cc13Seg48Host& host);

// ---- plane sound set ----
void cc13_seg48_plane_sound_set_for_each_slot18_007de760(void* set, std::uint32_t a, float b, Cc13Seg48Host& host);

// 007EB380..007EBF03, complete ordinary FX step. Native ECX = component
// (plane+C98h), one stack float dt, RET 4 at 007EBF01; no native EH frame.
// Image constants remain explicit inputs at their original widths/addresses.
// This interface does not alter the older Cc13Seg48Host implementations.
struct PlaneFxStepGlobals {
    std::uint8_t* const& game_00e188a8;
    std::uint8_t* const& sound_00f8bbd8;
    const double& c_00ce3d40;
    const double& c_00d086a0;
    const float& c_00d08698;
    const double& c_00ce3d48;
    const float& c_00d06bb4;
    const double& c_00d08690;
    const double& c_00d7a210;
    const double& c_00ce3820;
    const double& c_00ce4bd8;
    const double& c_00d04368;
    const float& c_00ce54a0;
    const double& c_00ce7630;
    const float& c_00ce3958;
    const float& c_00ce3d30;
    const float& c_00ce38c8;
    const void* rpm_key_00d08688;
    const void* load_key_00d08680;
    const void* pass_key_00d08678;
};

struct PlaneFxStepHost {
    virtual ~PlaneFxStepHost() = default;
    virtual void call_0042ac60(float* current, float target, float max_step) = 0;
    virtual std::uint8_t* call_0042e740() = 0;
    // Five stack floats, RET 14h, ST0 result; keep the unrounded result until
    // the caller's explicit float store or following x87 expression.
    virtual double call_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    virtual std::uint8_t vcall_controller_38(void* controller) = 0;
    virtual bool vcall_plane_5c(void* plane, std::uint32_t kind) = 0;
    virtual void vcall_effect_10(void* effect, float value, float dt) = 0;
    virtual const float* vcall_plane_34(void* plane, float* out3) = 0;
    virtual double vcall_plane_38(void* plane) = 0;
    virtual void vcall_sound_1c(void* sound, const void* key, float value) = 0;
    virtual void call_007e1b30(void* holder) = 0;
    virtual void call_007eb0e0(void* component) = 0;
    virtual void call_007eae30(void* holder, std::uint8_t restart) = 0;
    virtual void call_007eb160(void* component) = 0;
    virtual void call_00414db0(void* entity) = 0;
    virtual float* call_00413920(const float* left16, float* out16, const float* right16) = 0;
    virtual void call_004134f0(float* out16, const float* source16) = 0;
    virtual void call_00b6db70(void* node) = 0;
    virtual void call_007de490(void* holder, float dt) = 0;
};

void plane_fx_component_step_007eb380(std::uint8_t* component, float dt,
                                    const PlaneFxStepGlobals& globals, PlaneFxStepHost& host);

}  // namespace bsp
