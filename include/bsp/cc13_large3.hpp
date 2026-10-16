// cc13_large3 (packet cc13-large3): large bodies the cc13 sweeps deferred (docs/CC13_SWEEPC.md,
// docs/CC13_SWEEPC2.md "not reached"). Evidence and per-function ABI notes: docs/CC13_LARGE3.md.
//
// Model as in cc13_sweepC.hpp: every routine works on the original object as raw bytes (Win32
// layout, offsets as in the image) and reaches the engine through Large3Host, whose methods name
// the image routine or virtual slot they stand for and give the register inputs. Tagged archive
// values use cc13_sweepC::ArchiveArg {kind, value}; pointer values are truncated to 32 bits as
// in the Win32 image. Constants carry their image address and width. New C++ interfaces, not
// drop-in binary replacements; nothing here is bound into bsp_game. Names are hypotheses, not
// recovered symbols.

#ifndef BSP_CC13_LARGE3_HPP
#define BSP_CC13_LARGE3_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_sweepC.hpp"

namespace bsp::cc13_large3 {

using cc13_sweepC::add;
using cc13_sweepC::ArchiveArg;
using cc13_sweepC::at;
using cc13_sweepC::ptr;

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr float kUnitScale = 1.0f;                  // 00D7A24C, float
inline constexpr float kFrameSizesXDefault = 0x1.DDDD2Ap-5f;  // 00D5D11C, float bits 3D6EEE95
inline constexpr float kFrameSizesYDefault = 0x1.3E9318p-4f;  // 00D5D10C, float bits 3D9F498C
inline constexpr std::size_t kFrameBoxStateStride = 0x2C;     // 00AD0290 / 00AD1A80 records
inline constexpr float kShadowElevationAngle = 0x1.BECDE8p-2f;  // 00D5B564, float bits 3EDF66F4
inline constexpr double kShadowEyeDistance = 100000.0;          // 00CF81F0, double
inline constexpr float kBodyDescDefault68 = 1000.0f;            // 00CE3804, float (D+68h, D+6Ch)
inline constexpr double kVelocityScale = 2.0;                   // 00D7A308, double
inline constexpr double kMassScale = 0.5;                       // 00D7A280, double
inline constexpr double kInertiaDivisor = 5.0;                  // 00D7A370, double
inline constexpr int kShipRepairPreferenceCount = 6;          // 008186D1
inline constexpr int kShipPathPointCount = 0x28;              // 00818892
inline constexpr std::size_t kShipPathPointStride = 0x18;     // 0081888F

// The engine's NativeString as laid out in the image: {size, char* data}, 8 bytes on Win32.
struct NativeStringRaw {
    std::uint32_t size;
    char* data;
};

// ---------------------------------------------------------------------------
// Engine boundary.
// ---------------------------------------------------------------------------
struct Large3Host {
    virtual ~Large3Host() = default;

    // 00BF6713, the checked-iterator failure report. The native carries on after it returns.
    virtual void invalid_iterator_00bf6713() = 0;
    // [00CE2220] InterlockedDecrement(&counter) -> the new count.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* counter) = 0;
    // Refcounted object vtable +0 (ECX = object, no stack arguments): the destroy the release
    // makes when the count reaches zero.
    virtual void refcounted_destroy_vslot_00(void* object) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, __thiscall(ECX = effect).
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;

    // Archive / property sink virtuals (ECX = sink): +04h opens a block with key {kind, value}
    // (two dwords, RET 8); +08h closes it; +0Ch writes key {kind, value} and value {kind, value}
    // (four dwords, RET 10h). Key kind 0 carries a const char* name, kind 1 an index.
    virtual void sink_begin_vslot_04(void* sink, ArchiveArg key) = 0;
    virtual void sink_end_vslot_08(void* sink) = 0;
    virtual void sink_write_vslot_0c(void* sink, ArchiveArg key, ArchiveArg value) = 0;
    // 00BD5680 BSP_TaggedValue_Equals, __thiscall(ECX = &left, &right), RET 4 -> AL.
    virtual bool tagged_value_equals_00bd5680(const ArchiveArg* left, const ArchiveArg* right) = 0;

    // --- 00818340 callees --------------------------------------------------------------------
    // 0095BB10 BSP_VehicleUnit_LoadArchiveCommon, __thiscall(ECX = unit, archive), RET 4
    // (reconstructed as cc13_seg62 vehicle_unit_write_state_0095bb10 behind its own host).
    virtual void vehicle_unit_write_state_0095bb10(void* unit, void* archive) = 0;
    // 0092D730 BSP_UnitController_GetBodyAxisSpeed, __thiscall(ECX = [ship+1018h]) -> ST0.
    virtual double motion_body_axis_speed_0092d730(void* controller) = 0;
    // 00778890 BSP_UnitInstance_IsOccupantOwner, __thiscall(ECX = unit) -> AL.
    virtual bool unit_is_occupant_owner_00778890(void* unit) = 0;
    // 008362A0 BSP_WeaponDirector_DumpProperties, __thiscall(ECX = [ship+738h], archive), RET 4.
    virtual void weapon_director_dump_properties_008362a0(void* director, void* archive) = 0;
    // 009D5E00, __thiscall(ECX = [ship+73Ch], archive), RET 4: the "navigatorParams" body.
    virtual void navigator_dump_properties_009d5e00(void* navigator, void* archive) = 0;
    // 0074EF20 BSP_LeakManager_WriteZones, __thiscall(ECX = ship+10D4h, archive), RET 4.
    virtual void leak_manager_write_zones_0074ef20(void* leak_manager, void* archive) = 0;

    // --- 00AD0290 / 00AD1A80 callees -----------------------------------------------------------
    // 00AAAED0 BSP_GuiWidget_DescribeProperties, __thiscall(ECX = widget, sink), RET 4.
    virtual void gui_widget_describe_properties_00aaaed0(void* widget, void* sink) = 0;
    // 00ACF0C0, __thiscall(ECX = frame box, float out[2]*, index), RET 8 -> the pointer it
    // returns (read as two floats).
    virtual const float* frame_box_state_size_00acf0c0(void* frame_box, void* out,
                                                       std::uint32_t index) = 0;
    // 00B33E40 BSP_LogicalTextureBase_GetNameAddress, __thiscall(ECX = texture) -> the address
    // of its NativeString name {size, char* data}.
    virtual const void* texture_name_address_00b33e40(void* texture) = 0;
    // 00AAA0F0 BSP_GuiWidget_VisitChildPaths, __thiscall(ECX = widget, const NativeString*
    // prefix), RET 4.
    virtual void gui_widget_visit_child_paths_00aaa0f0(void* widget, const void* prefix) = 0;

    // --- NativeString {size, char* data} (00AD1A80, 00A96750) ----------------------------------
    // 0041DD40, __thiscall(ECX = string, length, keep), RET 8: resize.
    virtual void native_string_resize_0041dd40(NativeStringRaw* s, std::uint32_t length,
                                               std::uint32_t keep) = 0;
    // 0041DD20, __thiscall(ECX = string): the out-of-line destroy.
    virtual void native_string_destroy_0041dd20(NativeStringRaw* s) = 0;
    // The pooled release every inline NativeString destructor makes: 00419CC0(data, size, 1)
    // answers the pool, then 00BD1510(ECX = pool) returns the block with the same three stack
    // arguments. `size` is the string size + 1, as pushed.
    virtual void string_pool_release_00419cc0_00bd1510(void* data, std::uint32_t size) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(ECX = left, out, const right*), RET 8 -> out.
    virtual NativeStringRaw* native_string_concat_004261a0(const void* left, NativeStringRaw* out,
                                                           const void* right) = 0;
    // 004263B0 BSP_NativeString_ConcatInt, __thiscall(ECX = left, out, int), RET 8 -> out.
    virtual NativeStringRaw* native_string_concat_int_004263b0(const void* left,
                                                               NativeStringRaw* out,
                                                               std::int32_t value) = 0;
    // 00469840 BSP_NativeString_Substring, __thiscall(ECX = source, out, pos, count), RET 0Ch
    // -> out.
    virtual NativeStringRaw* native_string_substring_00469840(const void* source,
                                                              NativeStringRaw* out,
                                                              std::uint32_t pos,
                                                              std::uint32_t count) = 0;
    // 00425F40 BSP_NativeString_AssignHeader, __thiscall(ECX = destination, const source*), RET 4.
    virtual void native_string_assign_00425f40(void* destination, const NativeStringRaw* source) = 0;
    // 00531030 BSP_NativeString_ConstructByte, __thiscall(ECX = out, dword whose low byte is the
    // character), RET 4 -> out.
    virtual NativeStringRaw* native_string_construct_byte_00531030(NativeStringRaw* out,
                                                                   std::uint32_t value) = 0;

    // --- 00A96750 callees --------------------------------------------------------------------
    // 00AB6D30 BSP_TextContext_AcceptsInputChar, ECX = [owner+24h] (set by the caller; the body
    // is reconstructed as text_accepts_input_char_00ab6d30, which reads only the stack byte),
    // one stack dword, RET 4 -> AL.
    virtual bool text_accepts_input_char_00ab6d30(void* context, std::uint32_t value) = 0;
    // 00A96450 BSP_KeyCharMap_Translate, __thiscall(ECX = owner, dword), RET 4 -> AL.
    virtual std::uint8_t key_char_map_translate_00a96450(void* owner, std::uint32_t value) = 0;
    // Text owner vtable slot at `slot_offset` (+4h changed, +8h enter, +0Ch escape, +10h up,
    // +14h down, +18h tab, +1Ch page down, +20h page up), ECX = owner, no stack arguments.
    virtual void text_owner_virtual(void* owner, std::uint32_t slot_offset) = 0;
    // Image byte globals: 00E12F34 (insert mode, read and written), 00F8BC01 (read).
    virtual std::uint8_t* global_byte(std::uint32_t address) = 0;

    // --- 00A8F3B0 callees --------------------------------------------------------------------
    // Image pointer globals read by value: 00F8BBF0 (render settings), 00F8C210 (foliage).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // [00CE221C] InterlockedIncrement(&counter) -> the new count.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* counter) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(ECX = destination, const source*), RET 4.
    virtual void matrix_copy_4x4_004134f0(void* destination, const void* source) = 0;
    // 00B71990 BSP_Camera_SetViewport, __thiscall(ECX = camera, viewport), RET 4.
    virtual void camera_set_viewport_00b71990(void* camera, void* viewport) = 0;
    // 00B7AAE0 BSP_Render_GetOverrideContextWord, called with ECX = [owner+0Ch] -> EAX.
    virtual std::uint32_t render_override_context_word_00b7aae0(void* ecx) = 0;
    // [owner+0Ch] vtable+0Ch, __thiscall(ECX = [owner+0Ch], word), RET 4 -> AL.
    virtual bool shadow_context_vslot_0c(void* context, std::uint32_t word) = 0;
    // 00419510 BSP_Vector3f_Normalize, __fastcall(ECX = out, EDX = in) -> EAX (read as 3 floats).
    virtual const float* vector3_normalize_00419510(float* out, const float* in) = 0;
    // 00412E20 BSP_Math_TangentX87Float, one stack float, RET 4 -> ST0.
    virtual double tangent_00412e20(float angle) = 0;
    // 00415550 BSP_Math_MaxFloatByRef / 00415510 BSP_Math_MinFloatByRef, __fastcall(ECX = &a,
    // EDX = &b) -> ST0.
    virtual double max_float_by_ref_00415550(const float* a, const float* b) = 0;
    virtual double min_float_by_ref_00415510(const float* a, const float* b) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __thiscall(ECX = node).
    virtual void transform_refresh_world_matrix_00b6db70(void* node) = 0;
    // 00B63F10 BSP_Matrix_BuildLookAt: ECX out, EDX eye, stack (target*, up.x, up.y, up.z),
    // RET 10h, returns ECX.
    virtual void* matrix_build_look_at_00b63f10(void* out, const float* eye, const float* target,
                                                float up_x, float up_y, float up_z) = 0;
    // 00B63B30 BSP_Matrix_InvertOrthogonalScaledAffine, ECX = destination, EDX = source.
    virtual void* matrix_invert_affine_00b63b30(void* destination, const void* source) = 0;
    // 00AD7A30, __thiscall(ECX = foliage system, camera, float), RET 8.
    virtual void foliage_render_for_camera_00ad7a30(void* foliage, void* camera, float scale) = 0;
    // 00A8EA20 BSP_DirectionalShadowOwner_SubmitCascades, __thiscall(ECX = owner).
    virtual void shadow_owner_submit_cascades_00a8ea20(void* owner) = 0;

    // --- 0092AAE0 callees --------------------------------------------------------------------
    // 00424C40 BSP_GameSettings_GetSingleton() -> EAX.
    virtual void* game_settings_singleton_00424c40() = 0;
    // 00923840 BSP_Entity_InheritRaceAndActivate, __fastcall(ECX = entity).
    virtual void entity_inherit_race_and_activate_00923840(void* entity) = 0;
    // 00B6E0D0 BSP_Node_GetInverseWorldMatrix, __thiscall(ECX = node) -> EAX.
    virtual const void* node_inverse_world_matrix_00b6e0d0(void* node) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX = point, out, matrix), RET 8
    // -> EAX (read as 3 floats).
    virtual const float* vector3_transform_affine_point_004142e0(const float* point, float* out,
                                                                 const void* matrix) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00C336C0, __thiscall(ECX = out 3x3 + translation, const float m[16]), RET 4.
    virtual void physics_pose_from_matrix_00c336c0(void* out, const void* matrix) = 0;
    // [unit]->vtable+34h, __thiscall(ECX = unit, const float v[3]*), RET 4 -> EAX (3 floats).
    virtual const float* unit_vslot_34(void* unit, const float* v) = 0;
    // 00BF55BE operator new(bytes) / 00BF6989 free(block), __cdecl.
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // 00C5D580 Dyn_World_CreateBody, __thiscall(ECX = world, const descriptor*), RET 4 -> body.
    virtual void* physics_world_create_body_00c5d580(void* world, const void* descriptor) = 0;
    // 00C31F90, __thiscall(ECX = body, float lo[3]*, float hi[3]*), RET 8: the body bounds.
    virtual void physics_body_bounds_00c31f90(void* body, float* lo, float* hi) = 0;
    // 00C37F40 Dyn_Body_SetMass, __thiscall(ECX = body, float), RET 4.
    virtual void physics_body_set_mass_00c37f40(void* body, float mass) = 0;
    // 00C37E70 Dyn_Body_SetInertia, __thiscall(ECX = body, const float[3]*), RET 4.
    virtual void physics_body_set_inertia_00c37e70(void* body, const float* inertia) = 0;
    // 0098A920 BSP_SpatialNode_SetLocalBounds, __thiscall(ECX = node, const min*, const max*),
    // RET 8.
    virtual void spatial_node_set_local_bounds_0098a920(void* node, const float* lo,
                                                        const float* hi) = 0;
    // 0042E630 BSP_SpatialIndex_GetSingleton() -> EAX.
    virtual void* spatial_index_singleton_0042e630() = 0;
    // 0098BA10 BSP_SpatialIndex_AttachNode, __thiscall(ECX = index, node, a, b, c), RET 10h.
    virtual void spatial_index_attach_node_0098ba10(void* index, void* node, std::uint32_t a,
                                                    std::uint32_t b, std::uint32_t c) = 0;

    // --- 008EC650 callees (STL instantiations are vendor code, called, not reconstructed) ------
    // 00585EB0 BSP_NativeString_TrimLeftSet / 00584110 BSP_NativeString_TrimRightSet,
    // __thiscall(ECX = string, const char* set), RET 4.
    virtual void native_string_trim_left_set_00585eb0(NativeStringRaw* s, const char* set) = 0;
    virtual void native_string_trim_right_set_00584110(NativeStringRaw* s, const char* set) = 0;
    // 008E64A0 BSP_NativeString_InsertAt, __thiscall(ECX = string, const NativeString* insert,
    // pos), RET 8.
    virtual void native_string_insert_at_008e64a0(NativeStringRaw* s, const NativeStringRaw* insert,
                                                  std::uint32_t pos) = 0;
    // 004954F0 vector<NativeString>::erase(first, last), __thiscall(ECX = vector, out iterator*,
    // {container, first}, {container, last}) (five dwords), RET 14h.
    virtual void string_vector_erase_range_004954f0(void* vector, void* out_iterator,
                                                    void* first_container, void* first,
                                                    void* last_container, void* last) = 0;
    // 00589690 vector<NativeString>::erase(where), __thiscall(ECX = vector, out iterator*,
    // {container, where}), RET 0Ch.
    virtual void string_vector_erase_00589690(void* vector, void* out_iterator, void* container,
                                              void* where) = 0;
    // 008E65E0 tree find, __thiscall(ECX = tree, out iterator* {container, node}, const key*),
    // RET 8 -> out.
    virtual const std::uint32_t* string_tree_find_008e65e0(void* tree, void* out_iterator,
                                                           const NativeStringRaw* key) = 0;
    // 008E5BB0 tree lower_bound, __thiscall(ECX = tree, const key*), RET 4 -> node.
    virtual void* name_vector_map_lower_bound_008e5bb0(void* tree, const NativeStringRaw* key) = 0;
    // 008EC460 BSP_PowerupRandomNameMap_LookupOrInsert, __thiscall(ECX = manager+1ACh map,
    // const key*), RET 4 -> the mapped vector<NativeString> {proxy, first, last, end}.
    virtual void* powerup_random_name_map_lookup_or_insert_008ec460(void* map,
                                                                   const NativeStringRaw* key) = 0;
    // 00450540 vector<NativeString>::push_back, __thiscall(ECX = vector, const element*), RET 4.
    virtual void string_vector_push_back_00450540(void* vector, const void* element) = 0;
    // 00506C80 vector<NativeString>::operator=, __thiscall(ECX = destination, const source*),
    // RET 4.
    virtual void string_vector_assign_00506c80(void* destination, const void* source) = 0;
    // 00432050, ECX = first, EDX = last, stack (allocator*, extra dword), the element destroy
    // loop; the image pushes the out pointer as the extra dword.
    virtual void string_range_destroy_00432050(void* first, void* last, void* allocator,
                                               void* extra) = 0;
    // 00BF65AC free(block), __cdecl.
    virtual void free_00bf65ac(void* block) = 0;
    // 00BD2FC0 BSP_RandomThreads_NextU32, ECX = stream selector -> EAX (random-thread state
    // through 00BD2ED0, the module 00BD2F10 also draws from).
    virtual std::uint32_t random_threads_next_u32_00bd2fc0(std::uint32_t stream) = 0;
};

// 00818970, __thiscall(ECX = ship), plain RET.
void ship_stop_wreck_effects_00818970(Large3Host& host, void* ship);

// 00818340, __thiscall(ECX = ship, archive), RET 4.
void ship_describe_state_00818340(Large3Host& host, void* ship, void* archive);

// 00AD0290, __thiscall(ECX = frame box, sink), RET 4; vtable 00D5D130 slot 7.
void gui_frame_box_describe_properties_00ad0290(Large3Host& host, void* frame_box, void* sink);

// 00AD1A80, __thiscall(ECX = frame box, const NativeString* prefix), RET 4; vtable 00D5D130
// slot 27. Builds and destroys one path string per state record, then the base visit.
void gui_frame_box_visit_child_paths_00ad1a80(Large3Host& host, void* frame_box,
                                              const void* prefix);

// 0092AAE0, __thiscall(ECX = tickable entity, const source*), RET 4: builds the physics body
// descriptor, creates the body and publishes the entity's collision node.
void tickable_entity_install_collision_0092aae0(Large3Host& host, void* entity, void* source);

// 008EC650, __thiscall(ECX = powerup manager, NativeString* out, NativeString key by value),
// RET 0Ch -> out. `key` points at the by-value copy, which this function destroys.
NativeStringRaw* powerup_pick_from_bracket_list_008ec650(Large3Host& host, void* manager,
                                                         NativeStringRaw* out,
                                                         NativeStringRaw* key);

// 00A8F3B0, __thiscall(ECX = shadow owner, camera), RET 4.
void shadow_map_update_view_from_camera_00a8f3b0(Large3Host& host, void* owner, void* camera);

// 00A96750, __thiscall(ECX = text owner, event, category), RET 8: the editing fallback after
// the owner's vtable+0 declines a queued key event.
void text_owner_handle_key_fallback_00a96750(Large3Host& host, void* owner, std::uint32_t event,
                                             std::uint32_t category);

}  // namespace bsp::cc13_large3

#endif  // BSP_CC13_LARGE3_HPP
