#pragma once
// Packet cc13-seg47 (cc13_seg47 marker): segment 47 (007B2DD0-007D1DC0, flight)
// functions that had no reconstruction record. docs/CC13_SEG47.md lists every
// function with its outcome (reconstructed here / existing-and-recorded /
// skipped with the reason), its original ABI and its uncertainty.
//
// Objects are reached through Raw (byte offsets into the Win32 layout). Every
// call the image makes goes through a Seg47Host method named for its target and
// is invoked at the binary's point with the binary's arguments. Descriptive
// names are hypotheses, not recovered symbols. Nothing here is wired; nothing
// is ABI-compatible.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg47 {

struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
    std::uint8_t* field(std::size_t off) const noexcept { return p + off; }
    Raw sub(std::size_t off) const noexcept { return Raw{p + off}; }
    Raw deref(std::size_t off) const noexcept { return Raw{static_cast<std::uint8_t*>(ptr(off))}; }
    explicit operator bool() const noexcept { return p != nullptr; }
};
inline Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

// The archive key/value pairs every archive virtual takes by value (two
// dwords each): {kind, payload}. Key kind 0 = const char* name, 1 = index.
// Value kind 0 = NativeString*, 1 = int*, 3 = bool*, 5 = float[3]*.
struct ArchiveKey {
    std::int32_t kind;
    std::uintptr_t value;
};

struct Seg47Host {
    virtual ~Seg47Host() = default;

    // The address of the image global at `address`; the pointer stored there.
    virtual void* global_address(std::uint32_t address) = 0;
    virtual void* global_pointer(std::uint32_t address) = 0;

    // --- CRT --------------------------------------------------------------
    // 00BF681B operator new(size), __cdecl (the allocator the path sources use).
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 00BF55BE operator new(size), __cdecl.
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    // 00BF65AC operator delete(block), __cdecl.
    virtual void operator_delete_00bf65ac(void* block) = 0;
    // 00BF79F0 memset(dst, value, count), __cdecl.
    virtual void memset_00bf79f0(void* dst, int value, std::uint32_t count) = 0;
    // 00BF7FBF _stricmp(a, b), __cdecl, EAX.
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00BF6713 _invalid_parameter_noinfo (the checked-iterator failure), __cdecl.
    virtual void invalid_parameter_00bf6713() = 0;
    // 004F9A80 sprintf-style formatter, __cdecl(buffer, format, ...): one int argument here.
    virtual void format_int_004f9a80(char* buffer, const char* format, std::int32_t value) = 0;
    // InterlockedIncrement [00CE221C] / InterlockedDecrement [00CE2220], __stdcall(long*), EAX.
    virtual long interlocked_increment_ce221c(long* value) = 0;
    virtual long interlocked_decrement_ce2220(long* value) = 0;

    // --- strings and the sized pool -----------------------------------------
    // 00419CC0 BSP_SizedStoragePool_GetSingleton, EAX = pool (its stack words
    // are left for the next call).
    virtual void* sized_pool_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool; data, bytes, flag), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive, __thiscall(ECX = string; const char*), RET 4, AL.
    virtual bool native_string_equals_insensitive_00425850(Raw string, const char* text) = 0;

    // --- archive virtuals (vtable of the archive object) ----------------------
    // +04h enter(key), +08h leave(), +10h read(key, value), +14h has(key) -> AL.
    // Each is __thiscall with the pairs by value (RET 8 / RET 0 / RET 10h / RET 8).
    virtual void archive_enter_04(Raw archive, ArchiveKey key) = 0;
    virtual void archive_leave_08(Raw archive) = 0;
    virtual void archive_read_10(Raw archive, ArchiveKey key, ArchiveKey value) = 0;
    virtual bool archive_has_14(Raw archive, ArchiveKey key) = 0;

    // --- path sources and path components (segment 46) -------------------------
    // 004215D0 MSVC_Vector12_PushBack, __thiscall(ECX = vector; const float[3]*), RET 4.
    virtual void vector12_push_back_004215d0(Raw vector, const float* point) = 0;
    // 007B2AB0 BSP_OwnedPointsPathSource_Construct, __thiscall(ECX = memory; arg), RET 4, EAX.
    virtual void* owned_points_path_source_construct_007b2ab0(void* memory, void* arg) = 0;
    // 007B2D80 BSP_OwnedPointsPathSource_ConstructEmpty, __thiscall(ECX = memory), EAX.
    virtual void* owned_points_path_source_construct_empty_007b2d80(void* memory) = 0;
    // Path source vtable +1Ch: load(archive), __thiscall, RET 4. Also slot +00h
    // (called with no stack argument when the reference count reaches zero).
    virtual void path_source_vcall_load_1c(Raw source, Raw archive) = 0;
    virtual void path_source_vcall_destroy_00(Raw source) = 0;
    // Point array vtable +20h: allocate a point, __thiscall(ECX = array), EAX.
    virtual float* point_array_vcall_new_point_20(Raw array) = 0;
    // 007B3190 STL vector<void*>::push_back, __thiscall(ECX = vector; void* const*), RET 4.
    virtual void ptr_vector_push_back_007b3190(Raw vector, void* const* value) = 0;
    // 007B34F0 BSP_Point3PtrArray_LoadFromScene, __thiscall(ECX = array; property holder), RET 4.
    virtual void point_array_load_from_scene_007b34f0(Raw array, void* holder) = 0;
    // 00415870 vector size, __thiscall(ECX = vector owner), EAX.
    virtual std::int32_t vector_size_00415870(Raw owner) = 0;
    // 007B1A80 BSP_PathComponent_Destruct, __thiscall(ECX = component).
    virtual void path_component_destruct_007b1a80(Raw component) = 0;
    // 007B2950 PATH_CREATE message construct, __thiscall(ECX = message; entity), RET 4, EAX = message.
    virtual void* path_create_message_construct_007b2950(void* message, Raw entity) = 0;

    // --- scene property bag --------------------------------------------------
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = bag; const char*), RET 4,
    // EAX = the property {tag +4, value +0Ch}.
    virtual Raw property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 008F0DF0 BSP_SceneProperty_GetStringValue, __thiscall(ECX = property), EAX.
    virtual const char* property_string_value_008f0df0(Raw property) = 0;
    // 00925A90 entity registry find by name, __thiscall(ECX = registry; const char*), RET 4, EAX.
    virtual void* entity_registry_find_by_name_00925a90(void* registry, const char* name) = 0;

    // --- entity bases ----------------------------------------------------------
    // 0077EED0 unit-owner entity base constructor, __thiscall(ECX = entity; id), RET 4.
    virtual void unit_owner_entity_construct_0077eed0(Raw entity, std::uint32_t id) = 0;
    // 0077E380 unit-owner entity base destructor, __thiscall(ECX = entity) (tail jump).
    virtual void unit_owner_entity_destruct_0077e380(Raw entity) = 0;
    // 00928560 / 00928570 entity base register / unregister, __thiscall(ECX = entity).
    virtual void entity_register_00928560(Raw entity) = 0;
    virtual void entity_unregister_00928570(Raw entity) = 0;
    // 00484540 list push_back, __thiscall(ECX = list; value), RET 4.
    // 004837D0 list erase, __thiscall(ECX = list; node), RET 4.
    virtual void unit_list_push_back_00484540(Raw list, void* value) = 0;
    virtual void unit_list_erase_node_004837d0(Raw list, void* node) = 0;
    // 00923840 / 0077F0E0 / 009277E0 the scene-entity activate bases, __thiscall(ECX = entity).
    virtual void scene_entity_activate_00923840(Raw entity) = 0;
    virtual void net_entity_activate_0077f0e0(Raw entity) = 0;
    virtual void scene_entity_activate_009277e0(Raw entity) = 0;
    // 00779FC0, __thiscall(ECX = entity; peer, message), RET 8: send to a peer.
    virtual void entity_send_message_to_peer_00779fc0(Raw entity, void* peer, void* message) = 0;
    // 007668E0 entity-create message base destructor, __thiscall(ECX = message).
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;

    // --- bot tasks (rocket band) ------------------------------------------------
    // 00BD2F10 BSP_Random_UniformFloatRange, ECX = stream (1 here), stack (lo, hi), RET 8, ST0.
    virtual float random_uniform_00bd2f10(std::int32_t stream, float lo, float hi) = 0;
    // 00BF6989 _free(block), __cdecl.
    virtual void free_00bf6989(void* block) = 0;
    // Bot state virtuals: +04h enter(), +08h exit(), __thiscall(ECX = state).
    virtual void state_vcall_enter_04(Raw state) = 0;
    virtual void state_vcall_exit_08(Raw state) = 0;
    // Owned object vtable +08h (release), __thiscall(ECX = object) (tail jump in 007B3E20).
    virtual void owned_object_vcall_release_08(Raw object) = 0;
    // 007B8AD0, __thiscall(ECX = plane), AL.
    virtual bool plane_query_007b8ad0(Raw plane) = 0;
    // 006952A0 BSP_Observer_UnregisterPair, __fastcall(ECX = observed, EDX = observer handle).
    virtual void observer_unregister_pair_006952a0(void* observed, Raw handle) = 0;
    // 00695870 BSP_CallbackOwner_Destroy, __thiscall(ECX = handle).
    virtual void callback_owner_destroy_00695870(Raw handle) = 0;
    // 004B7EF0, __thiscall(ECX = holder): the second observer-holder destructor.
    virtual void observer_holder_destruct_004b7ef0(Raw holder) = 0;
    // 0042BCA0 handle destructor, __thiscall(ECX = handle).
    virtual void handle_destruct_0042bca0(Raw handle) = 0;
    // 004BEF00 handle constructor, __thiscall(ECX = handle; value), RET 4.
    virtual void handle_construct_004bef00(Raw handle, void* value) = 0;
    // 007B4500, __thiscall(ECX = vector; 0), RET 4: the element teardown before the free.
    virtual void observer_vector_tidy_007b4500(Raw vector, std::int32_t zero) = 0;
    // 007B45F0 bot state destructor, __thiscall(ECX = state).
    virtual void bot_state_destruct_007b45f0(Raw state) = 0;
    // 006CDD70, __thiscall(ECX = member): the moveto/follow states' inner destructor.
    virtual void state_member_destruct_006cdd70(Raw member) = 0;
    // 007B65E0, __thiscall(ECX = state): the +ECh state destructor.
    virtual void state_destruct_007b65e0(Raw state) = 0;
    // 004116D0 state machine destructor, __thiscall(ECX = machine, null for a null task).
    virtual void state_machine_destruct_004116d0(void* machine) = 0;
    // 0042E740, EAX = the AI tuning object (fields +664h, +668h read here).
    virtual Raw ai_tuning_0042e740() = 0;
    // 009F9CE0 approach head constructor, __thiscall(ECX = approach; arg, float), RET 8.
    virtual void approach_head_construct_009f9ce0(Raw approach, void* arg, float value) = 0;
    // 007B3C00, __thiscall(ECX = target kinds), EAX = the size index.
    virtual std::int32_t rocket_target_size_007b3c00(void* target_kinds) = 0;
    // 009FB200 approach target reference constructor, __thiscall(ECX = ref; arg), RET 4.
    virtual void approach_target_ref_construct_009fb200(Raw ref, void* arg) = 0;
    // 007B3EA0, __thiscall(ECX = approach): the glide seed.
    virtual void rocket_glide_seed_007b3ea0(Raw approach) = 0;
    // 00411E20 state machine constructor, __thiscall(ECX = machine).
    virtual void state_machine_construct_00411e20(Raw machine) = 0;
    // 00411E70 state machine register, __thiscall(ECX = machine; const char* name, state), RET 8.
    virtual void state_machine_register_00411e70(Raw machine, const char* name, Raw state) = 0;
    // 009C2AC0 moveto state constructor, __thiscall(ECX = state; owner, arg, f0, f1, f2), RET 14h.
    virtual void moveto_state_construct_009c2ac0(Raw state, Raw owner, void* arg, float f0, float f1,
                                                 float f2) = 0;
    // 009C2980 follow-style state constructor, __thiscall(ECX = state; owner, float), RET 8.
    virtual void follow_state_construct_009c2980(Raw state, Raw owner, float value) = 0;
    // 007B5780 rocket goaway state constructor, __thiscall(ECX = state; owner), RET 4.
    virtual void rocket_goaway_construct_007b5780(Raw state, Raw owner) = 0;
    // 007B4030 bot task base destructor, __thiscall(ECX = task).
    virtual void bot_task_base_destruct_007b4030(Raw task) = 0;
    // 007B6A40 rocket task constructor, __thiscall(ECX = memory; a, b), RET 8, EAX.
    virtual void* rocket_task_construct_007b6a40(void* memory, void* a, void* b) = 0;
    // std::string (MSVC 2005: +4 buffer/pointer, +14h size, +18h capacity).
    // 00408120 assign(ECX = string; const string&, pos, count), RET 0Ch, EAX.
    virtual Raw string_assign_00408120(Raw string, Raw source, std::uint32_t pos, std::uint32_t count) = 0;
    // 007B7CC0 append(ECX = string; const char*, count), RET 8, EAX = string.
    virtual Raw string_append_007b7cc0(Raw string, const char* text, std::uint32_t count) = 0;
    // 009A1A90 bot task debug name, __thiscall(ECX = task; string* result), RET 4, EAX = result.
    virtual Raw bot_task_debug_name_009a1a90(Raw task, Raw result) = 0;
    // 00411430 state machine name of a state, __thiscall(ECX = machine; state), RET 4, EAX.
    virtual const char* state_machine_state_name_00411430(Raw machine, void* state) = 0;

    // --- plane state and weapon racks ------------------------------------------------
    // (plane+72Ch)->vtable[38h], __thiscall(ECX = control mode), AL.
    virtual bool control_mode_vcall_38(Raw mode) = 0;
    // plane->vtable[38h], __thiscall(ECX = plane), ST0 = the speed (007B8E60 reads +B1Ch).
    virtual float plane_vcall_speed_38(Raw plane) = 0;
    // 007B8860 BSP_ActuatorBlock_SetGearChannelInstant, __thiscall(ECX = block; char down), RET 4.
    virtual void actuator_set_gear_instant_007b8860(Raw block, std::uint8_t down) = 0;
    // 007DE600, __thiscall(ECX = plane+C98h).
    virtual void plane_power_block_007de600(Raw block) = 0;
    // Rack virtuals: +218h(kind) -> EAX, +220h(loadout) -> EAX descriptor,
    // +20Ch() -> AL; descriptor +08h(kind) -> AL. All __thiscall, RET 4 / RET.
    virtual void* rack_vcall_218(Raw rack, std::int32_t kind) = 0;
    virtual Raw rack_vcall_descriptor_220(Raw rack, std::int32_t loadout) = 0;
    virtual bool rack_vcall_20c(Raw rack) = 0;
    virtual bool descriptor_vcall_is_kind_08(Raw descriptor, std::int32_t kind) = 0;

    // --- plane helpers 007B9590-007BB110 ---------------------------------------------
    // AI node vtable +10h (retire), __thiscall(ECX = node).
    virtual void ai_node_vcall_retire_10(Raw node) = 0;
    // 00951FB0 unit-instance retire (tail jump), __thiscall(ECX = unit).
    virtual void unit_instance_retire_00951fb0(Raw unit) = 0;
    // 00951F40, __thiscall(ECX = unit; int), RET 4.
    virtual void unit_instance_detach_00951f40(Raw unit, std::int32_t value) = 0;
    // 007E2090 gun target finder scan, __thiscall(ECX = finder; f0..f4, int, float), RET 1Ch, EAX.
    virtual void* gun_target_finder_scan_007e2090(Raw finder, float f0, float f1, float f2, float f3, float f4,
                                                  std::int32_t n, float f6) = 0;
    // 007E2070 gun target finder clear, __thiscall(ECX = finder) (tail jump).
    virtual void gun_target_finder_clear_007e2070(Raw finder) = 0;
    // 00413920 matrix product, __thiscall(ECX = left; out, right), RET 8, EAX = out.
    virtual void* matrix_multiply_00413920(Raw left, void* out, const void* right) = 0;
    // 004134F0 matrix assign, __thiscall(ECX = matrix; const matrix*), RET 4, EAX = matrix.
    virtual void* matrix_assign_004134f0(Raw matrix, const void* source) = 0;
    // 0075B430 session message base constructor, __thiscall(ECX = message; type), RET 4.
    virtual void session_message_construct_0075b430(Raw message, std::uint32_t type) = 0;
    // entity->vtable[5Ch](kind), __thiscall, RET 4, AL.
    virtual bool entity_vcall_is_kind_5c(Raw entity, std::int32_t kind) = 0;
    // 0087FA50 landscape height, __thiscall(ECX = landscape; x, z), RET 8, ST0.
    virtual float landscape_height_0087fa50(Raw landscape, float x, float z) = 0;
    // 0093E9A0 BSP_MapOverlay_DrawEllipse, __cdecl(overlay, const float[2]*).
    virtual void map_overlay_draw_ellipse_0093e9a0(void* overlay, const float* radii) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(ECX = node; float, int), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(Raw node, float factor, std::int32_t flag) = 0;
    // node->vtable[2Ch](const float[3]*), __thiscall, RET 4.
    virtual void scene_node_vcall_set_scale_2c(Raw node, const float* scale) = 0;
    // 00C32490, __thiscall(ECX = physics handle), EAX = the body record (+6Ch owner).
    virtual Raw physics_body_owner_00c32490(void* handle) = 0;
    // plane->vtable[1ACh](float), __thiscall, RET 4.
    virtual void plane_vcall_damage_1ac(Raw plane, float amount) = 0;
    // 00439950 BSP_Vector3f_SetRandomUnitDirection, __thiscall(ECX = vector), EAX = vector.
    virtual const float* vector3_set_random_unit_direction_00439950(float* vector) = 0;
    // 00415690 BSP_Math_ClampInPlace, __fastcall(ECX = value*, EDX = low*; high*), RET 4.
    virtual void math_clamp_in_place_00415690(void* value, void* low, void* high) = 0;
    // 0095BAB0, __thiscall(ECX = [class+70h]), AL.
    virtual bool resource_query_0095bab0(void* object) = 0;
    // 0043AB50 / 0043AB80 the resource scope constructor (char) / destructor, __thiscall(ECX = scope).
    virtual void resource_scope_construct_0043ab50(void* scope, std::uint8_t flag) = 0;
    virtual void resource_scope_destruct_0043ab80(void* scope) = 0;
    // 009598D0, __thiscall(ECX = class; char), RET 4.
    virtual void scene_class_activate_009598d0(Raw cls, std::uint8_t force) = 0;
    // 004B3F10, __thiscall(ECX = holder+4).
    virtual void resource_holder_activate_004b3f10(Raw holder) = 0;

    // --- plane helpers 007BB150-007BBC50 ---------------------------------------------
    // 00770AF0 BSP_Session_SendMessageToSecondaryFirstPeer, __thiscall(ECX = [game]+1EF0h; message), RET 4.
    virtual void session_send_to_secondary_first_peer_00770af0(Raw session, void* message) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(Raw entity) = 0;
    // 006BC530 BSP_AirOpsHolder_OverRunway, __thiscall(ECX = holder; const float*, out*), RET 8, AL.
    virtual bool air_ops_holder_over_runway_006bc530(Raw holder, void* position, void* out) = 0;
    // 004BFED0, __fastcall(ECX = plane), AL.
    virtual bool plane_cockpit_ready_004bfed0(Raw plane) = 0;
    // 00B646E0 / 00B64640 rotation Y / X, __fastcall(ECX = out matrix, EDX = const float* angle), EAX = out.
    virtual void* matrix_build_rotation_y_00b646e0(void* out, const float* angle) = 0;
    virtual void* matrix_build_rotation_x_00b64640(void* out, const float* angle) = 0;
    // 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse, __fastcall(ECX = out, EDX = source).
    virtual void matrix_build_affine_inverse_00b63d50(Raw out, Raw source) = 0;
    // 00955970 BSP_Entity_RefreshSceneNodeMatrix, __thiscall(ECX = entity) (tail jump).
    virtual void entity_refresh_scene_node_matrix_00955970(Raw entity) = 0;
    // 007D8180 BSP_PlaneFlightController_SetGravityTerm94, __thiscall(ECX = plane+AB0h; int), RET 4.
    virtual void plane_flight_set_gravity_term_007d8180(Raw controller, std::int32_t value) = 0;
    // plane->vtable[70h](int), __thiscall, RET 4.
    virtual void plane_vcall_70(Raw plane, std::int32_t value) = 0;
    // 0084B6F0 BSP_Effect_SpawnOrientedPointEffect, __fastcall(ECX = table, EDX = position; direction), RET 4.
    virtual void effect_spawn_oriented_point_0084b6f0(Raw table, const float* position, const float* direction) = 0;
    // 00926D90 BSP_MissionEntity_Kill, __thiscall(ECX = entity; int), RET 4.
    virtual void mission_entity_kill_00926d90(Raw entity, std::int32_t mode) = 0;
    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed, __thiscall(ECX = entity), RET, EAX = entity+FCh.
    virtual void* entity_pose_world_position_refreshed_00427eb0(Raw entity) = 0;
    // 00414D10 BSP_Vector3f_TransformAffinePointViaScratch, __thiscall(ECX = out, EDX = pose; point), RET 4.
    virtual void vector3_transform_affine_point_00414d10(float* out, void* pose, const void* point) = 0;

    // --- plane helpers 007BC550-007BD3E0 ---------------------------------------------
    // 00922F80 BSP_SceneNode_Disable / 00922F30 BSP_SceneNode_Enable, __thiscall(ECX = entity; int), RET 4.
    virtual void scene_node_disable_00922f80(Raw entity, std::int32_t value) = 0;
    virtual void scene_node_enable_00922f30(Raw entity, std::int32_t value) = 0;
    // 00779C60 BSP_Unit_UpdateOwnerHudState, __thiscall(ECX = unit; float), RET 4.
    virtual void unit_update_owner_hud_state_00779c60(Raw unit, float value) = 0;
    // 00B3C650 BSP_CockpitHelper_SetAttachedNode, __thiscall(ECX = helper; node), RET 4.
    virtual void cockpit_helper_set_attached_node_00b3c650(Raw helper, void* node) = 0;
    // 006E3500 BSP_MBombPlatform_RoundCount / 006E3520, __thiscall(ECX = rack), EAX.
    virtual std::int32_t bomb_platform_round_count_006e3500(Raw rack) = 0;
    virtual std::int32_t bomb_platform_capacity_006e3520(Raw rack) = 0;
    // 009552E0, __thiscall(ECX = [plane+538h]; id), RET 4, EAX.
    virtual void* plane_class_device_set_009552e0(Raw cls, std::int32_t id) = 0;
    // 0095A880 BSP_Unit_InstantiateClassDevices, __thiscall(ECX = unit; set, 0, 0), RET 0Ch.
    virtual void unit_instantiate_class_devices_0095a880(Raw unit, void* devices, std::int32_t a, std::int32_t b) = 0;
    // 00999AE0 BSP_PilotBot_FirstTaskWaitValue, __thiscall(ECX = bot), ST0 (tail jump).
    virtual float pilot_bot_first_task_wait_value_00999ae0(Raw bot) = 0;
    // 0078CF20 BSP_GameWorld_SampleWaterHeight, __thiscall(ECX = [game+19F0h]; x, z), RET 8, ST0.
    virtual float game_world_sample_water_height_0078cf20(Raw water, float x, float z) = 0;
    // 00809DE0 BSP_RocketClass_IgnitionLift, __thiscall(ECX = class), ST0 (tail jump).
    virtual float rocket_class_ignition_lift_00809de0(Raw cls) = 0;
    // 0072C6A0 BSP_Gun_CreateAiBots, __thiscall(ECX = gun).
    virtual void gun_create_ai_bots_0072c6a0(Raw gun) = 0;
    // 00927850, __thiscall(ECX = entity; key), RET 4, AL.
    virtual bool entity_replication_query_00927850(Raw entity, std::int32_t key) = 0;
    // 0077B280 BSP_Replication_BuildSyncDelta, __cdecl, seventeen stack words.
    virtual void replication_build_sync_delta_0077b280(Raw entity, void** arg, std::uint32_t flag, float f0,
                                                       std::int32_t i0, float f1, std::int32_t i1, float f2,
                                                       std::int32_t i2, float f3, std::int32_t i3, float f4,
                                                       std::int32_t i4, float f5, std::int32_t i5, float f6,
                                                       std::int32_t i6) = 0;
    // 00766FC0 BSP_NetEntityCreateMessage_Construct, __thiscall(ECX = message; type, a, entity, 1), RET 10h.
    virtual void net_entity_create_message_construct_00766fc0(Raw message, std::uint32_t type, std::uint32_t a,
                                                              Raw entity, std::int32_t flag) = 0;
    // (plane+170h)->vtable[0](), __thiscall, AL.
    virtual std::uint8_t replicator_vcall_00(Raw replicator) = 0;

    // --- pod arrays and small helpers ----------------------------------------------
    // A refcounted object's vtable slot 0 at a zero count, __thiscall(ECX = object), no stack argument.
    virtual void refcounted_vcall_destroy_00(Raw object) = 0;
    // 007C3000 pod array resize, __thiscall(ECX = array; count), RET 4.
    virtual void pod_array4_resize_007c3000(Raw array, std::int32_t count) = 0;
    // 00BD1860 BSP_CriticalSection_Create, EAX.
    virtual void* critical_section_create_00bd1860() = 0;
    // 00467050, __thiscall(ECX = matrix; three float angles), RET 0Ch.
    virtual void matrix_set_rotation_00467050(float* matrix, float a, float b, float c) = 0;

    // --- rack queries, registries --------------------------------------------------
    // Rack (bomb platform) virtuals: +200h reload(), +21Ch(kind) -> EAX rounds,
    // +210h(kind, flag) -> AL. __thiscall.
    virtual void rack_vcall_reload_200(Raw rack) = 0;
    virtual std::int32_t rack_vcall_round_count_21c(Raw rack, std::int32_t kind) = 0;
    virtual bool rack_vcall_holds_kind_210(Raw rack, std::int32_t kind, std::int32_t flag) = 0;
    // Bot task vtable +0Ch(record* out), __thiscall, RET 4.
    virtual void bot_task_vcall_0c(Raw task, Raw out) = 0;
    // 00415350, EAX = the lock manager (+10h the critical section).
    virtual void* lock_manager_00415350() = 0;
    // EnterCriticalSection [00CE2218] / LeaveCriticalSection [00CE2210], __stdcall.
    virtual void enter_critical_section_ce2218(void* cs) = 0;
    virtual void leave_critical_section_ce2210(void* cs) = 0;
    // 00BD0C30, __thiscall(ECX = manager; object), RET 4.
    virtual void lock_manager_register_00bd0c30(Raw manager, void* object) = 0;
    // 0041CC80, __thiscall(ECX = registry+4): the critical-section holder destructor.
    virtual void critical_section_destroy_0041cc80(Raw holder) = 0;
    // 007DDDE0, __thiscall(ECX = vector).
    virtual void vector3_fill_one_007ddde0(Raw vector) = 0;

    // --- plane flight-state helpers 007C0C60-007CD250 ----------------------------------
    // 007BE9B0 BSP_Plane_ApplyPointEventLocal, __thiscall(ECX = plane+84Ch; three float[3] and a float by value), RET 28h.
    virtual void plane_apply_point_event_local_007be9b0(Raw target, const float* a, const float* b, const float* c,
                                                        float d) = 0;
    // 00924610, __thiscall(ECX = entity; the same ten words), RET 28h.
    virtual void entity_point_event_00924610(Raw entity, const float* a, const float* b, const float* c, float d) = 0;
    // 007D99C0 BSP_PlaneFlight_ForwardSpeed, __thiscall(ECX = plane+AB0h), ST0.
    virtual float plane_flight_forward_speed_007d99c0(Raw controller) = 0;
    // 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), RET 14h, ST0.
    virtual float math_interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    // [[00E188A8]+1ED4h]->vtable[114h](out), __thiscall, RET 4, EAX = a matrix.
    virtual Raw camera_vcall_matrix_114(Raw camera, void* out) = 0;
    // 00903BC0, __thiscall(ECX = [game+19CCh]; const float* a, const float* b), RET 8, AL.
    virtual bool world_segment_blocked_00903bc0(Raw world, const float* a, const float* b) = 0;
    // 008685E0 BSP_PointEffect_CreateFromPosition, __fastcall(ECX = result slot, EDX = manager;
    // RefPtr effect by value, const float*, int, int), RET 10h.
    virtual void point_effect_create_from_position_008685e0(void** result, Raw manager, void* effect,
                                                            const float* position, std::int32_t a,
                                                            std::int32_t b) = 0;
    // 00440A30 BSP_RefPtrSlot_Release, __thiscall(ECX = slot).
    virtual void ref_ptr_slot_release_00440a30(void** slot) = 0;
    // 00465080 BSP_CommandTarget_FromEntity, __thiscall(ECX = descriptor; entity, float), RET 8, EAX.
    virtual void* command_target_from_entity_00465080(void* descriptor, void* entity, float value) = 0;
    // 0077D600 BSP_Entity_IssueCommand, __thiscall(ECX = entity; command, descriptor, flags), RET 0Ch.
    virtual void entity_issue_command_0077d600(Raw entity, std::uint32_t command, void* descriptor,
                                               std::uint32_t flags) = 0;
    // 0099A600 BSP_PilotBot_RetireFinishedTasks, __thiscall(ECX = bot).
    virtual void pilot_bot_retire_finished_tasks_0099a600(Raw bot) = 0;
    // 0077C470, __thiscall(ECX = entity; int, int), RET 8.
    virtual void entity_set_state_0077c470(Raw entity, std::int32_t a, std::int32_t b) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = entity; message, int, int), RET 0Ch.
    virtual void session_route_message_0077c2a0(Raw entity, void* message, std::int32_t a, std::int32_t b) = 0;
    // 007C71E0, __thiscall(ECX = plane): the touchdown arm.
    virtual void plane_touchdown_007c71e0(Raw plane) = 0;
    // 0042C1B0, __thiscall(ECX = camera+428h).
    virtual void camera_shake_start_0042c1b0(Raw shake) = 0;
    // 007C78A0 BSP_Plane_SetFlightStateUnguarded, __thiscall(ECX = plane; state, int), RET 8.
    virtual void plane_set_flight_state_unguarded_007c78a0(Raw plane, std::int32_t state, std::int32_t flag) = 0;
    // 007C11E0, __thiscall(ECX = plane; int), RET 4.
    virtual void plane_flight_state_entered_007c11e0(Raw plane, std::int32_t value) = 0;
    // 006E6450, __thiscall(ECX = control mode; arg), RET 4.
    virtual void control_mode_water_contact_006e6450(Raw mode, void* arg) = 0;
    // 007CB7F0 BSP_Plane_OnWaterContact, __thiscall(ECX = plane).
    virtual void plane_on_water_contact_007cb7f0(Raw plane) = 0;
    // 007CCB30, __thiscall(ECX = NativeString result), EAX = result.
    virtual void* flight_state_debug_text_007ccb30(Raw result) = 0;
    // 00BD1A20 NativeString append, __thiscall(ECX = string; const NativeString*), RET 4.
    virtual void native_string_append_00bd1a20(Raw string, void* other) = 0;

    // --- plane class and entity creation -----------------------------------------------
    // 00964790, __fastcall(ECX = class id, DL = flag), EAX = the entity class.
    virtual void* entity_class_by_id_00964790(std::uint16_t id, std::uint8_t flag) = 0;
    // class->vtable[28h](id), __thiscall, RET 4, EAX = the new entity.
    virtual Raw entity_class_vcall_create_28(Raw cls, std::uint16_t id) = 0;
    // 0041DD40 NativeString resize, __thiscall(ECX = {length, data}; length, char preserve), RET 8.
    virtual void native_string_resize_0041dd40(Raw string, std::uint32_t length, std::uint8_t preserve) = 0;
    // 00BF7680 memcpy(dst, src, count), __cdecl.
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t count) = 0;
    // 0075B500, __thiscall(ECX = message; out), RET 4, EAX.
    virtual void* session_message_entity_descriptor_0075b500(Raw message, void* out) = 0;
    // entity->vtable[88h](descriptor), __thiscall, RET 4.
    virtual void entity_vcall_apply_create_88(Raw entity, void* descriptor) = 0;
    // 005471B0, __thiscall(ECX = vector; size), RET 4.
    virtual void class_vector_grow_005471b0(Raw vector, std::int32_t size) = 0;
    // 007CFD20 plane unit-instance constructor, __thiscall(ECX = memory; arg, class), RET 8, EAX.
    virtual void* plane_unit_instance_construct_007cfd20(void* memory, void* arg, Raw cls) = 0;


    // 007DDE00, __thiscall(ECX = vector).
    virtual void vector3_construct_007dde00(Raw vector) = 0;
    // 007B9140, __thiscall(ECX = plane; int), RET 4, AL.
    virtual bool plane_query_007b9140(Raw plane, std::int32_t value) = 0;
};

// --- path band 007B2DD0-007B3A9B ---------------------------------------------
void owned_points_path_source_load_007b2dd0(Seg47Host& host, Raw source, Raw archive);
void** owned_points_path_source_new_007b2e40(Seg47Host& host, void** out, void* arg);
void** path_source_create_from_archive_kind_007b2ec0(Seg47Host& host, void** out, Raw archive);
void path_source_ref_load_from_archive_007b3000(Seg47Host& host, Raw holder, Raw archive);
Raw net_path_construct_007b3200(Seg47Host& host, Raw entity, std::uint16_t id);
const char* net_path_class_name_007b32b0();
std::int32_t net_path_sub170_slot0_007b32c0();
void net_path_slot180_noop_007b32d0(void* unused);
void net_path_slot18c_noop_007b32e0(void* unused);
std::uint16_t path_replicator_entity_net_id_007b3350(Raw replicator);
void net_path_register_in_world_lists_007b3370(Seg47Host& host, Raw entity);
void net_path_unregister_from_world_lists_007b33a0(Seg47Host& host, Raw entity);
void net_path_destruct_007b3400(Seg47Host& host, Raw entity);
void point3_ptr_array_add_point_007b34b0(Seg47Host& host, Raw array, const float* point);
void camera_path_load_path_points_007b3730(Seg47Host& host, Raw component, void* holder);
void scene_path_entity_load_holder_properties_007b38d0(Seg47Host& host, Raw entity);
void entity_path_activate_load_points_007b3920(Seg47Host& host, Raw entity);
void net_path_activate_load_points_007b3940(Seg47Host& host, Raw entity);
void net_path_send_create_message_007b3960(Seg47Host& host, Raw entity, void* peer);
void* path_create_message_create_entity_007b3a00(Seg47Host& host, Raw message);
void camera_path_activate_007b3a80(Seg47Host& host, Raw entity);

// --- bot task / rocket band 007B3BA0-007B7FD0 ----------------------------------
void pilot_plan_element_destruct_007b3ba0(Raw element);
Raw bot_rocket_task_state_block_for_role_007b3cb0(Seg47Host& host, Raw task);
void bot_observer_holder_destruct_007b3cd0(Seg47Host& host, Raw holder);
Raw random_timer_construct_007b3d60(Seg47Host& host, Raw timer, float lo, float hi);
void bot_task_state_base_exit_007b3dc0();
void empty_one_arg_virtual_007b3dd0(void* unused);
bool bot_task_state_base_predicate10_007b3de0();
void bot_task_state_base_slot14_007b3df0();
void owned_object_holder_destruct_007b3e20(Seg47Host& host, Raw holder);
void bot_observer_holder_destruct_full_007b3f70(Seg47Host& host, Raw holder);
std::int32_t bot_task_base_slot08_007b40c0();
const char* bot_task_base_name_007b40e0();
bool bot_task_base_slot20_007b40f0();
bool bot_task_base_slot28_007b4100();
bool bot_task_base_slot44_007b4120();
std::int32_t bot_task_base_slot48_007b4130();
std::int32_t bot_task_base_step_result_007b4140();
std::int32_t bot_task_base_slot5c_007b4150();
void bot_task_base_slot64_007b4160(void* unused);
Raw rocket_approach_construct_base_007b41e0(Seg47Host& host, Raw approach, void* arg, void* target_kinds);
void land_state_observer_vector_destruct_007b45a0(Seg47Host& host, Raw vector);
bool bot_task_state_base_predicate18_007b45e0();
void rocket_approach_redraw_run_speed_007b4890(Seg47Host& host, Raw state);
Raw rocket_attack_run_state_construct_007b4d90(Seg47Host& host, Raw state, void* owner);
float* rocket_approach_body_aim_offset_007b5400(Raw approach, float* out);
bool random_timer_step_007b5980(Seg47Host& host, Raw timer, float dt);
float unit_largest_extent_007b5be0(Raw unit);
void bot_condition_destruct_base_007b5c20(Seg47Host& host, Raw condition);
Raw bot_condition_construct_base_007b5c70(Seg47Host& host, Raw condition, void* arg);
Raw bot_condition_deleting_dtor_007b5cd0(Seg47Host& host, Raw condition, std::uint8_t flags);
Raw bot_condition_point_off_nose_construct_007b5d30(Seg47Host& host, Raw condition, void* arg, float a,
                                                    float b, float c);
void bot_condition_destruct_base_007b5d70(Seg47Host& host, Raw condition);
Raw bot_condition_point_off_nose_deleting_dtor_007b5dc0(Seg47Host& host, Raw condition, std::uint8_t flags);
Raw bot_condition_point_distance_construct_007b5e20(Seg47Host& host, Raw condition, void* arg,
                                                   const float* point, float distance);
void bot_condition_destruct_base_007b5e60(Seg47Host& host, Raw condition);
Raw bot_condition_point_distance_deleting_dtor_007b5eb0(Seg47Host& host, Raw condition, std::uint8_t flags);
void rocket_approach_destroy_states_007b6680(Seg47Host& host, Raw approach);
Raw rocket_approach_construct_007b6830(Seg47Host& host, Raw approach, Raw task, void* target_kinds);
std::uint32_t bot_task_rocket_command_class_007b6cd0();
const char* bot_task_rocket_name_007b6cf0();
void ref_holder_assign_007b6eb0(Seg47Host& host, Raw holder, Raw state);
void state_machine_set_current_state_007b6ee0(Seg47Host& host, Raw machine, Raw state);
void bot_task_rocket_set_state_007b6f50(Seg47Host& host, Raw task, Raw state);
void bot_task_rocket_stop_007b7ba0(Seg47Host& host, Raw task);
void bot_task_rocket_destruct_007b7e10(Seg47Host& host, Raw task);
Raw bot_task_build_debug_string_007b7e70(Seg47Host& host, Raw out, Raw base, const char* suffix);
Raw bot_task_rocket_debug_name_007b7f20(Seg47Host& host, Raw task, Raw out);
void* bot_task_make_rocket_007b7fd0(Seg47Host& host, void* a, void* b);


// --- plane state and weapon-controller band 007B80A0-007B9510 ----------------------
std::int32_t target_flag_e4_set_or_global_007b80a0(Seg47Host& host, Raw target);
std::int32_t target_flag_e4_clear_or_global_007b80c0(Seg47Host& host, Raw target);
void float_window_set_sample_and_sum_007b8140(Raw window, std::int32_t index, std::int32_t count, float sample);
void plane_clear_flag_904_007b8450(Raw plane);
void index_block_decrement_all_007b8550(Raw block);
void plane_set_turbo_byte_007b8aa0(Raw plane, std::uint8_t value);
bool plane_is_cockpit_view_active_007b8c20(Raw plane);
void plane_set_control_enabled_007b8c30(Seg47Host& host, Raw plane, bool enabled);
void plane_set_gear_mode_007b8ce0(Seg47Host& host, Raw plane, std::int32_t mode, std::uint8_t flag);
std::int32_t plane_gear_is_down_007b8d70(Raw plane);
float plane_get_float_b2c_007b8e70(Raw plane);
void plane_lose_power_007b8ef0(Seg47Host& host, Raw plane);
std::int32_t plane_control_mode_speed_above_class_19c_007b9080(Seg47Host& host, Raw mode);
void* weapon_controller_first_bomb_class_007b9100(Seg47Host& host, Raw plane);
void* weapon_controller_first_bomb_class_007b9510(Seg47Host& host, Raw plane);
bool weapon_controller_has_ordnance_of_kind_007b91c0(Seg47Host& host, Raw plane, std::int32_t kind,
                                                     std::int32_t loadout);
void* weapon_controller_first_slot_descriptor_007b9230(Seg47Host& host, Raw plane);
bool weapon_controller_any_slot_answers_20c_007b92e0(Seg47Host& host, Raw plane);
bool weapon_controller_has_general_bomb_ordnance_007b9320(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_drop_kamikaze_ordnance_007b93e0(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_torpedo_ordnance_007b93f0(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_rocket_kind33_007b9400(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_depth_charge_ordnance_007b94f0(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_level_bomb_ordnance_007b9500(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_find_ready_rocket_007b9410(Seg47Host& host, Raw plane, std::int32_t loadout);
bool weapon_controller_has_rocket_ordnance_007b9480(Seg47Host& host, Raw plane, std::int32_t loadout);


// --- plane helpers 007B9590-007BB110 ---------------------------------------------------
void unit_instance_retire_ai_sub_nodes_007b9590(Seg47Host& host, Raw plane);
bool plane_altitude_sink_predicate_007b9620(Raw plane);
void plane_detach_into_hangar_007b96c0(Seg47Host& host, Raw plane);
void* unit_find_gun_target_007b96f0(Seg47Host& host, Raw unit, float f0, float f1, float f2, float f3, float f4,
                                    std::int32_t n, float f6);
void plane_neighbours_clear_target_007b9740(Seg47Host& host, Raw unit);
void* plane_get_squadron_007b97e0(Raw plane);
void firing_gun_registry_destruct_unwind_007b98a0(Seg47Host& host, Raw registry);
Raw matrix_multiply_in_place_007ba020(Seg47Host& host, Raw matrix, const void* other);
Raw session_message_enginefire_construct_007ba170(Seg47Host& host, Raw message, std::int32_t index, float value);
Raw session_message_engineslost_construct_007ba1c0(Seg47Host& host, Raw message, float value);
Raw session_message_spinning_construct_007ba200(Seg47Host& host, Raw message, std::uint8_t side, float value);
float landscape_height_at_007ba340(Seg47Host& host, Raw game, float x, float z);
std::int32_t unit_is_gun_of_subtype156_007ba3e0(Seg47Host& host, Raw unit);
float max3_007ba450(const float* a, const float* b, const float* c);
void plane_store_message_vector_f0_007ba600(Raw plane, const float* v);
std::int32_t unit_gun_fire_blocked_007ba760(Raw unit);
void plane_state_record_init_defaults_007ba7b0(Seg47Host& host, Raw record);
void* session_message_resolve_entity_by_net_id_007ba890(Seg47Host& host, Raw message);
Raw session_message_plane_deadmeatrot_construct_007ba8d0(Seg47Host& host, Raw message, Raw plane);
Raw session_message_plane_controlfactor_construct_007ba950(Seg47Host& host, Raw message,
                                                           const std::uint32_t (&values)[7]);
Raw session_message_plane_hit_construct_007ba990(Seg47Host& host, Raw message, std::int32_t a, float b, float c,
                                                 float d, const float* v);
Raw plane_state_record_copy_007baa80(Raw destination, Raw source);
Raw plane_control_state_copy_007baaf0(Raw destination, Raw source);
void map_overlay_draw_ellipse_thunk_007bac50(Seg47Host& host, void* overlay, float r);
void plane_hide_model_007bac80(Seg47Host& host, Raw node);
void plane_contact_pair_ram_007bad00(Seg47Host& host, void* const* pair);
void vector3_place_at_unit_distance_007bad70(Seg47Host& host, float* a, const float* b);
void rate_limiter_step_007bae90(Seg47Host& host, Raw limiter, float target, float dt);
Raw plane_unit_instance_init_control_block_007baf90(Raw block, std::uint32_t arg);
void plane_class_activate_resources_007bb060(Seg47Host& host, Raw cls, std::uint8_t force);
bool unit_can_release_ordnance_007bb110(Seg47Host& host, Raw unit);


// --- plane helpers 007BB150-007BBC50 ----------------------------------------------------
void plane_publish_slot_byte_df0_007bb150(Seg47Host& host, Raw plane, std::uint8_t flag);
void plane_set_pilot_command_last_time_007bb170(Seg47Host& host, Raw plane, std::int32_t slot, float time);
void plane_refresh_ground_contact_007bb2c0(Seg47Host& host, Raw plane);
void plane_update_cockpit_transform_007bb310(Seg47Host& host, Raw plane);
float triangle_vertex_distance_sum_007bb4c0(const float* a, const float* b);
void plane_on_impact_message_007bba10(Seg47Host& host, Raw plane, Raw message);
void unit_copy_vector_ac8_007bbb70(Raw plane, float* out);
std::int32_t unit_pilot_target_is_007bbc10(Seg47Host& host, Raw plane, void* target);
bool plane_is_grounded_on_owned_site_007bbc50(Seg47Host& host, Raw plane);


// --- plane helpers 007BC550-007BD3E0, 007C7750 -------------------------------------------
void plane_sync_elevator_attachment_for_state_007bc550(Seg47Host& host, Raw plane);
void plane_enter_cockpit_view_007bc610(Seg47Host& host, Raw plane);
void plane_leave_cockpit_view_007bc690(Seg47Host& host, Raw plane);
float weapon_controller_total_bomb_rounds_007bc8c0(Seg47Host& host, Raw plane);
std::int32_t plane_ordnance_icon_kind_007bca50(Seg47Host& host, Raw plane);
void plane_re_equip_007bcb30(Seg47Host& host, Raw plane, std::int32_t id);
float plane_formation_wait_value_007bcc20(Seg47Host& host, Raw plane);
float* plane_ordnance_release_point_world_007bccf0(Seg47Host& host, Raw plane, float* out);
float weapon_controller_rocket_ignition_lift_007bd120(Seg47Host& host, Raw plane);
void plane_create_child_gun_ai_bots_007bd1a0(Seg47Host& host, Raw plane);
bool plane_has_child_gun_of_subtype156_007bd1f0(Seg47Host& host, Raw plane);
void plane_build_replication_delta_007bd240(Seg47Host& host, Raw plane, void** arg);
Raw session_message_plane_create_construct_007bd3e0(Seg47Host& host, Raw message, std::uint32_t a1, Raw plane,
                                                    Raw reference, std::uint32_t a4, std::uint32_t a5, float f6,
                                                    float f7, float f8, std::uint8_t c9, std::uint32_t a10);
bool plane_level_bomb_attitude_gate_007c7750(Seg47Host& host, Raw plane, std::uint8_t check_doors);


// --- engine pod arrays and small helpers -------------------------------------------------
void pod_array64_reserve_007bd600(Seg47Host& host, Raw array, std::int32_t count);
void pod_array4_reserve_007bd940(Seg47Host& host, Raw array, std::int32_t count);
void pod_array64_push_back_007c2db0(Seg47Host& host, Raw array, const void* element);
void pod_array64_clear_007c2f80(Seg47Host& host, Raw array);
void pod_array64_destruct_007c3fc0(Seg47Host& host, Raw array);
void ref_ptr_vector_pop_back_007c90d0(Seg47Host& host, Raw array);
void pod_array4_destruct_007c9150(Seg47Host& host, Raw array);
void plane_engine_effect_vector_destruct_007cd2c0(Seg47Host& host, Raw vector);
void* entity_cast_to_mrfs_gun_007bdb80(Seg47Host& host, Raw entity);
void* entity_world_matrix_refreshed_007bdba0(Seg47Host& host, Raw entity);
Raw firing_gun_registry_construct_007bdcb0(Seg47Host& host, Raw registry);
float* plane_control_state_init_identity_007bdf50(Seg47Host& host, Raw state, float* out);


// --- rack queries, registries and refcounted holders 007C1D80-007C9510 -------------------
void plane_reload_bomb_platforms_007c1d80(Seg47Host& host, Raw plane);
std::int32_t unit_count_remaining_ordnance_rounds_007c1db0(Seg47Host& host, Raw plane);
std::int32_t plane_rocket_round_count_007c1de0(Seg47Host& host, Raw plane);
std::int32_t unit_count_droppable_rounds_007c1f60(Seg47Host& host, Raw plane);
void* plane_slot_d4_forward_to_df4_007c2050(Seg47Host& host, Raw plane, Raw out);
void* plane_state_record_get_or_create_007c2810(Seg47Host& host, Raw holder);
float plane_get_heading_angle_007c2aa0(Seg47Host& host, Raw plane);
void* firing_gun_registry_instance_007c2ec0(Seg47Host& host);
void firing_gun_registry_destruct_007c33b0(Seg47Host& host, Raw registry);
Raw transform_record_init_identity_007c3420(Seg47Host& host, Raw record);
Raw plane_engine_effect_record_copy_construct_007c34b0(Seg47Host& host, Raw record, Raw source);
Raw plane_engine_effect_construct_007c3510(Seg47Host& host, Raw record, void* effect);
void ref_ptr_release_007c3600(Seg47Host& host, Raw holder);
void ref_ptr_release_007c3640(Seg47Host& host, Raw holder);
void ref_ptr_release_007c3680(Seg47Host& host, Raw holder);
void ref_ptr_release_007c94e0(Seg47Host& host, Raw holder);
void ref_ptr_release_007c9510(Seg47Host& host, Raw holder);
void* weapon_controller_first_slot_ordnance_007c3f60(Seg47Host& host, Raw plane);
Raw effect_vector_construct_007c4470(Seg47Host& host, Raw vector);
void record_destruct_string_3c_007c6f20(Seg47Host& host, Raw record);


// --- plane flight-state helpers 007C0C60-007CD250 -----------------------------------------
void plane_slot4c_point_event_007c0c60(Seg47Host& host, Raw plane, const float* a, const float* b, const float* c,
                                       float d);
void plane_step_countdown_964_007c3cb0(Seg47Host& host, Raw plane, float dt);
void plane_issue_land_on_touchdown_007c3ea0(Seg47Host& host, Raw plane);
void plane_remote_touchdown_to_state4_007c7430(Seg47Host& host, Raw plane);
void plane_sync_cockpit_camera_gun_shake_007c7800(Seg47Host& host, Raw plane, Raw camera);
void plane_remote_enter_flight_state6_007cb9e0(Seg47Host& host, Raw plane);
void plane_control_mode_on_water_contact_007cc2d0(Seg47Host& host, Raw mode, void* arg);
Raw plane_append_flight_state_debug_007cd250(Seg47Host& host, Raw string);


// --- plane class and entity creation 007CC9B0-007D1C90 -------------------------------------
void* session_message_plane_create_create_entity_007cc9b0(Seg47Host& host, Raw message);
bool plane_bay_door_rack_holds_bomb_007cd9a0(Seg47Host& host, Raw plane);
std::int32_t plane_class_slot1c_007cfcf0();
std::uint32_t plane_get_field_93c_007d0350(Raw plane);
void* plane_unit_instance_create_007d1c90(Seg47Host& host, Raw cls, void* arg);


// --- plane class speed and altitude gates ---------------------------------------------------
float plane_class_stall_range_speed_007c4830(Seg47Host& host, Raw cls);
bool plane_in_paratrooper_drop_altitude_007c4d90(Seg47Host& host, Raw plane);


// --- plane engine-effect vector -----------------------------------------------------------
void plane_engine_effect_vector_reserve_007c9170(Seg47Host& host, Raw vector, std::int32_t count);
void plane_engine_effect_vector_resize_007c9290(Seg47Host& host, Raw vector, std::int32_t count);
void plane_engine_effect_vector_push_back_007ccca0(Seg47Host& host, Raw vector, Raw record);

Raw plane_fx_component_construct_007cd520(Seg47Host& host, Raw component);

void plane_publish_step_record_007cdc70(Seg47Host& host, Raw plane);

}  // namespace bsp::cc13_seg47
