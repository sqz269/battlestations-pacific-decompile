#pragma once

// Segment 7 (0047D250..0048F670): soldier types and their Lua loaders, the
// SoldierGlobals singleton, scene-entity view checks, quad-tree cell resets,
// scene-property-bag iterators, the small pointer-array instantiations and
// observer slots, and the record copies that sit among them. Evidence and
// per-function notes: docs/CC13_SEG7.md. Every name is a hypothesis.
//
// Same model as segment 5 (include/bsp/actions_seg5_cc13.hpp): an object is a
// byte pointer, every field is read or written at the image's offset, every
// call the image makes goes through a named Seg7Host method at the same point
// with the same arguments, and globals are read through the host.

#include <cstdint>

namespace bsp::cc13_seg7 {

static_assert(sizeof(void*) == 4, "segment 7 bodies use the Win32 object layout");

using Object = std::uint8_t*;

template <class T>
T& native_at(Object base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct Seg7Host {
    virtual ~Seg7Host() = default;

    // Globals.
    virtual Object game_00e188a8() = 0;                  // [00E188A8]
    virtual Object soldier_globals_00f87544() = 0;       // [00F87544]
    virtual void set_soldier_globals_00f87544(Object value) = 0;
    virtual std::uint8_t traffic_debug_00e081c0() = 0;   // byte [00E081C0]
    virtual void set_traffic_debug_00e081c0(std::uint8_t value) = 0;

    // CRT and imports.
    virtual void invalid_parameter_00bf6713() = 0;       // checked-iterator / range report
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void crt_free_00bf65ac(void* memory) = 0;
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;  // the pointer-array allocator
    virtual void crt_free_00bf6989(void* memory) = 0;
    virtual void crt_memcpy_00bf7680(void* dest, const void* src, std::uint32_t bytes) = 0;
    virtual void crt_memmove_s_00bf67a7(void* dest, std::uint32_t dest_size, const void* src,
                                        std::uint32_t count) = 0;
    virtual void enter_critical_section_import_00ce2218(Object section) = 0;
    virtual void leave_critical_section_import_00ce2210(Object section) = 0;
    virtual std::int32_t interlocked_increment_import_00ce221c(void* value) = 0;
    virtual std::int32_t interlocked_decrement_import_00ce2220(void* value) = 0;

    // Virtual calls, named by vtable offset.
    virtual void object_vcall_00(Object object) = 0;                              // ref-counted release
    virtual void notify_vcall_08(Object object, Object argument) = 0;             // +8 with one argument
    virtual void delete_vcall_04(Object object, std::int32_t flags) = 0;          // +4 (deleting dtor)
    virtual std::uint32_t subject_id_vcall_04(Object subject) = 0;                // +4, no argument
    virtual Object create_vcall_08(Object factory) = 0;                           // +8, no argument
    virtual Object class_instantiate_vcall_08(Object factory, std::int32_t a, float b) = 0;  // +8
    virtual void node_update_vcall_38(Object visual, Object source) = 0;          // +38h
    virtual Object parent_node_vcall_40(Object object) = 0;                       // +40h
    virtual bool is_kind_vcall_5c(Object object, std::int32_t kind) = 0;          // +5Ch
    virtual void set_pose_vcall_98(Object object, std::int32_t a, Object b, const float* matrix) = 0;
    virtual void vcall_a0(Object object) = 0;                                     // +A0h
    virtual void vcall_d8(Object object) = 0;                                     // +D8h
    virtual void attach_vcall_114(Object object, Object owner) = 0;               // +114h
    virtual void blast_vcall_1ac(Object object, float value) = 0;                 // +1ACh

    // Direct calls.
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    virtual bool game_is_aabb_in_view_004b48e0(Object game, const float* box) = 0;
    virtual void mission_entity_kill_00926d90(Object entity, std::int32_t reason) = 0;
    virtual void scene_entity_destroyed_base_00923040(Object entity) = 0;
    virtual void point_effect_stop_children_00867b10(Object effect) = 0;
    virtual float* point3_array_at_007ad030(Object array, std::uint32_t index) = 0;
    virtual void point3ptr_array_insert_at_007b2850(Object array, std::int32_t index, Object value) = 0;
    virtual void point3ptr_array_erase_at_007b1b40(Object array, std::uint32_t index) = 0;
    virtual void observer_unregister_pair_006952a0(Object subject, Object slot) = 0;  // ECX, EDX
    virtual void observer_register_pair_00694a60(Object subject, Object slot) = 0;    // ECX, EDX
    virtual void soldier_globals_load_lua_0084cb70(Object globals) = 0;
    virtual Object singleton_manager_00415350() = 0;
    virtual void singleton_register_00bd0c30(Object manager, Object instance) = 0;
    virtual void scene_node_set_visibility_factor_00b6da70(Object node, float factor, std::int32_t flag) = 0;
    virtual void scene_node_disable_00922f80(Object node, std::int32_t flag) = 0;
    virtual Object unit_part_instance_construct_007135c0(Object memory, Object owner, Object visual) = 0;
    virtual void node_set_parent_00b6e680(Object node, Object parent) = 0;
    virtual void node_propagate_root_registration_00b6d890(Object node, Object root) = 0;
    virtual void quad_cell_range_0047ba40(Object tree, const float* box, std::int32_t* i0, std::int32_t* i1,
                                          std::int32_t* j0, std::int32_t* j1) = 0;
    virtual void traffic_visible_cells_00498c40(Object tree, Object node_slot, const float* box) = 0;
    virtual void matrix_copy_004134f0(Object dest, Object source) = 0;
    virtual void scene_proxy_base_destroy_00489dd0(Object proxy) = 0;
    virtual void path_unregister_base_00928570(Object path) = 0;
    virtual void unit_list_erase_node_004837d0(Object list, Object node) = 0;
    virtual void scene_node_invalidate_subtree_pose_0042ed50(Object node) = 0;
    virtual void landing_troop_kill_all_member_entities_0048c090(Object troop) = 0;
    virtual void tree_iterator_increment_00427fe0(Object iterator) = 0;
    virtual Object sized_pool_singleton_00419cc0(void* data, std::uint32_t bytes, std::int32_t flag) = 0;
    virtual void sized_pool_return_block_00bd1510(Object pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    virtual void native_string_resize_0041dd40(Object native_string, std::uint32_t size, std::int32_t flag) = 0;
    virtual std::uint32_t* bit_word_0048e330(Object owner) = 0;
    virtual bool bit_source_0048f230(Object source) = 0;
    virtual Object property_bag_find_008f2260(Object bag, const char* name) = 0;
    virtual Object effect_handle_acquire_00870cd0(Object out_slot, std::int32_t id, std::int32_t flag) = 0;
    virtual Object matrix_build_rotation_y_00b646e0(Object out, const float* angle) = 0;  // ECX out, EDX angle
    virtual Object matrix_multiply_00413920(Object lhs, Object out, Object rhs) = 0;
    virtual double random_uniform_float_range_00bd2f10(Object ecx, float lo, float hi) = 0;
    virtual void entity_destruct_base_00925780(Object entity) = 0;
    virtual void scene_node_construct_00925ce0(Object node) = 0;
    virtual void entity_inherit_race_and_activate_00923840(Object entity) = 0;
    virtual void native_string_assign_0041e870(Object native_string, const char* text) = 0;
    virtual Object property_map_find_borrowed_key_node_0043b8b0(Object map, Object key, Object out_slot) = 0;
    virtual void game_entity_register_in_parent_00928560(Object entity) = 0;
    virtual void unit_list_push_back_00484540(Object list, Object entity) = 0;
    virtual void entity_update_vcall_04(Object entity, float delta) = 0;        // +4 with a float
    virtual void sentity_init_vcall_120(Object entity) = 0;                     // +120h

    // Lua object calls (ECX = the Lua object).
    virtual Object lua_get_by_name_00b67800(Object lua, Object out, const char* key) = 0;
    virtual double lua_get_number_00b66270(Object ref) = 0;
    virtual double lua_get_float_or_default_00b66330(Object ref, float fallback) = 0;
    virtual std::int32_t lua_get_integer_or_default_00b66380(Object ref, std::int32_t fallback) = 0;
    virtual std::uint8_t lua_get_boolean_or_default_00b662f0(Object ref, std::int32_t fallback) = 0;
    virtual void lua_destruct_00b67700(Object ref) = 0;
    virtual bool lua_is_nil_00b65fb0(Object ref) = 0;
    virtual bool lua_is_table_00b661b0(Object ref) = 0;
    virtual void lua_construct_00b65f50(Object ref) = 0;
    virtual void lua_iterate_first_00b67080(Object table, Object key, Object value) = 0;
    virtual bool lua_iterate_done_00b66420(Object table, Object value) = 0;
    virtual void lua_iterate_next_00b67190(Object table, Object key, Object value) = 0;
};

// ---------------------------------------------------------------------------
// Record copies, __thiscall(ECX dest, [ESP+4] source), RET 4, returning ECX.
// Field order and form from the listing: d = dword MOV, f = float FLD/FSTP
// (a signalling NaN would come out quiet in the image), b = byte MOV.
// ---------------------------------------------------------------------------
Object record54_copy_0047d250(Object dest, Object source);        // d0 d4 f8..f48 b4C d50
Object record4c_copy_0047d2e0(Object dest, Object source);        // d0 d4 f8..f44 b48
Object record40_copy_0047f0c0(Object dest, Object source);        // d0 d4 f8 fC d10 f14..f28 d2C f30..f3C
Object record54_copy_assign_0047f930(Object dest, Object source); // as 0047D250
Object record4c_copy_assign_0047f9c0(Object dest, Object source); // as 0047D2E0

// 0047E540 __fastcall(ECX matrix, EDX const float* translation), RET (EAX =
// matrix): a 4x4 identity (1.0f from 00D7A24C) with row 3 = translation.
Object matrix_init_translation_0047e540(Object matrix, const float* translation);

// 00481510 __thiscall(ECX box, [ESP+4] const float bounds[6] {min, max}),
// RET 4: +60h..+6Ch = 0, then 24 floats built from the corners and the centre
// c = float(float(min + max) * 0.5) (double 00D7A280; the x product uses
// FMUL ST(1),ST(0), bytes checked).
void box_corners_from_bounds_00481510(Object box, const float* bounds);

// ---------------------------------------------------------------------------
// Scene property bag iterator {bag +0, node +4, bucket +8} over a 64-bucket
// table at bag+8.
// ---------------------------------------------------------------------------
bool property_bag_iterator_next_0047e480(Object iterator);   // __fastcall, RET
void property_bag_iterator_begin_00480690(Object iterator);  // __fastcall, RET
const char* property_bag_iterator_key_00484d20(Object iterator);  // node+4 or "" (00E186ED, .bss)
Object property_bag_create_iterator_00487040(Seg7Host& host, Object bag);  // new(0Ch) over bag+4
// 0048F5D0 __thiscall(bag, name, float fallback), RET 8: fallback when
// 0048E9F0 fails; else 008F2260's record +0Ch as float when +4 == 1, else the
// int converted (CVTSI2SS).
float property_bag_get_float_or_default_0048f5d0(Seg7Host& host, Object bag, const char* name,
                                                 float fallback);

// ---------------------------------------------------------------------------
// Scene entities, points and proxies.
// ---------------------------------------------------------------------------
void point_effect_stop_and_mark_004802f0(Seg7Host& host, Object effect);
void point3_array_set_at_00480810(Seg7Host& host, Object array, std::uint32_t index, const float* value);
void point3_array_set_at_dirty_00480860(Seg7Host& host, Object array, std::uint32_t index,
                                        const float* value);
void point3ptr_array_insert_at_dirty_004808a0(Seg7Host& host, Object array, std::int32_t index, Object value);
void point3ptr_array_erase_at_dirty_004808b0(Seg7Host& host, Object array, std::uint32_t index);
Object scene_path_record_create_004808f0(Seg7Host& host);
std::uint16_t scene_path_source_word174_00480960(Object source);
void scene_entity_on_destroyed_release_links_00480ba0(Seg7Host& host, Object entity);
void scene_entity_stop_blast_effect_00480bf0(Seg7Host& host, Object entity);
void scene_entity_forward_blast_notify_00480c20(Seg7Host& host, Object entity, float value);
bool entity_view_check_with_fade_00481090(Seg7Host& host, Object check);
bool entity_view_check_004812f0(Seg7Host& host, Object check);
void scene_entity_layer_reset_child_00481430(Seg7Host& host, Object entity, std::int32_t layer,
                                             std::uint32_t index);
void scene_entity_layers_reset_all_004814c0(Seg7Host& host, Object entity);
void quad_cell_reset_00484a10(Seg7Host& host, Object tree, std::uint32_t i, std::uint32_t j);
void quad_tree_prune_00484ae0(Seg7Host& host, Object tree, Object node_slot, const float* box);
void traffic_bounds_update_004879b0(Seg7Host& host, Object tree, Object node_slot, const float* box);
void traffic_proxy_update_0048c900(Seg7Host& host, Object proxy, std::uint32_t unused);
void scene_node_hide_and_maybe_disable_004847a0(Seg7Host& host, Object entity);
void anim_class_create_instance_004847d0(Seg7Host& host, Object entity);
void scene_entity_refresh_visibility_factor_00484940(Seg7Host& host, Object entity);
void scene_proxy_update_when_visible_00484bc0(Seg7Host& host, Object proxy, std::uint32_t unused);
void scene_proxy_destruct_0048c920(Seg7Host& host, Object proxy);
void path_unregister_from_world_lists_00487230(Seg7Host& host, Object path);
void traffic_group_flush_pending_visibility_00487940(Seg7Host& host, Object group);
void scene_node_set_matrix_and_invalidate_004897b0(Seg7Host& host, Object node, Object matrix);
std::uint8_t traffic_toggle_debug_and_refresh_0048cc60(Seg7Host& host, Object traffic);
void scene_entity_set_name_0048d6c0(Seg7Host& host, Object entity, const char* name);

// ---------------------------------------------------------------------------
// Pointer arrays {data +0, count +4, capacity +8} and observer slots (+14h).
// ---------------------------------------------------------------------------
void pod_pointer_array_reserve_00482510(Seg7Host& host, Object array, std::int32_t capacity);
void pod_pointer_array_reserve_00482570(Seg7Host& host, Object array, std::int32_t capacity);
void pod_pointer_array_push_back_00485390(Seg7Host& host, Object array, const std::uint32_t* value);
void pod_pointer_array_push_back_004853d0(Seg7Host& host, Object array, const std::uint32_t* value);
void pod_pointer_array_push_back_004883b0(Seg7Host& host, Object array, const std::uint32_t* value);
void pod_pointer_array_push_back_004883f0(Seg7Host& host, Object array, const std::uint32_t* value);
void pod_pointer_array_resize_00485990(Seg7Host& host, Object array, std::int32_t count);
void pod_pointer_array_resize_004859e0(Seg7Host& host, Object array, std::int32_t count);
void pod_pointer_array_tidy_0048ce30(Seg7Host& host, Object array);
void pod_pointer_array_tidy_0048ce50(Seg7Host& host, Object array);
void observer_slot_set_00483910(Seg7Host& host, Object slot, Object subject);
Object observer_slot_set_00486b10(Seg7Host& host, Object slot, Object subject);
Object observer_slot_set_004893f0(Seg7Host& host, Object slot, Object subject);
void observer_slot_release_if_subject_004884f0(Seg7Host& host, Object slot, Object subject);
void observer_slot_release_if_subject_00488520(Seg7Host& host, Object slot, Object subject);
// memmove_s copies of a dword range: returns dest + bytes (__stdcall-like, RET 0Ch).
Object pointer_vector_copy_range_00486f80(Seg7Host& host, Object first, Object last, Object dest);
Object pointer_vector_copy_range_0048d740(Seg7Host& host, Object first, Object last, Object dest);
Object pointer_vector_copy_range_0048d830(Seg7Host& host, Object first, Object last, Object dest);

// ---------------------------------------------------------------------------
// Strings, buffers, holders and bit flags.
// ---------------------------------------------------------------------------
void native_string_destroy_storage_0048cfc0(Seg7Host& host, Object native_string);
void named_buffer_clear_0048e0a0(Seg7Host& host, Object buffer);
Object native_string_pair_copy_construct_0048e3b0(Seg7Host& host, Object dest, Object source,
                                                  const std::uint32_t* third);
void ref_counted_ptr_holder_delete_0048e630(Seg7Host& host, Object holder);
Object bit_flag_set_flag_0048f200(Seg7Host& host, Object owner, std::uint8_t on);
Object bit_flag_sync_flag_0048f550(Seg7Host& host, Object owner, Object source);

// ---------------------------------------------------------------------------
// Scene-node and class-entity bodies (batch 2).
// ---------------------------------------------------------------------------
// 00480930 __thiscall(object, id), RET 4 (not a Ghidra function): 1 for ids
// 47h, 1, 0 or object+C4h, else 0.
bool path_is_kind_of_00480930(Object object, std::int32_t id);
// 004845D0 __thiscall(RefPtr* dest, const RefPtr* source), RET 4 (EAX =
// dest): a changed pointer is stored, the new object gains a reference
// (import 00CE221C) and the old one loses one (00CE2220; vtable slot 0 at 0).
Object retained_reference_assign_004845d0(Seg7Host& host, Object dest, Object source);
// 00487210 __fastcall(entity), RET: 00928560, then 00484540(ECX
// [entity+30h]+36Ch, entity).
void scene_node_attach_to_kind41_lists_00487210(Seg7Host& host, Object entity);
// 00487270 __thiscall(manager, float delta), RET 4: each value of the list
// at +0Ch (head +10h) gets vtable+4(delta); the next node is read first.
void entity_list_update_all_00487270(Seg7Host& host, Object manager, float delta);
// 00489760 __thiscall(node, const float* position), RET 4: +A4h..+ACh =
// position (FLD/FSTP), +C8h = 0, +10Ch = 0, 0042ED50 on each child.
void scene_node_set_position_and_invalidate_00489760(Seg7Host& host, Object node, const float* position);
// 0048E9F0 __thiscall(bag, const char* name), RET 4: a NativeString key
// (0041E870), 0043B8B0(ECX bag+4, &key, &argument slot) != 0, the key's
// storage released.
bool property_bag_contains_0048e9f0(Seg7Host& host, Object bag, const char* name);
// 0048DBD0 __fastcall(entity), RET: 00923840; the class (+170h) name data
// (class+8, or "" at 00E186ED) copied into entity+154h (0041DD40 flag 0 +
// memcpy); tail jump to vtable+120h.
void class_entity37_sentity_init_base_0048dbd0(Seg7Host& host, Object entity);
// 00487380 __fastcall(entity), RET: base destructor (vtables 00CE6490,
// 00CE6474 at +10h, 00CE646C at +24h); stop the effect +180h (00867B10,
// +9 = 1); +178h vtable+4(1) and clear; release +180h and clear; 00925780.
void class_entity37_destruct_base_00487380(Seg7Host& host, Object entity);
// 00489F30 __fastcall(entity), RET (EAX = entity): 00925CE0, the three
// vtables, +180h = 0, class id +C4h = 37h, +17Ch = 00BD2F10(ECX 0,
// lo = float(0.25 * -clip), hi = float(clip * 0.25)) with clip the
// SoldierGlobals +0Ch read through two 00485520 calls, +184h = 0.
Object class_entity37_construct_base_00489f30(Seg7Host& host, Object entity);
// 0047F200 __thiscall(node, float heading), RET 4: node+74h = 00413920(
// rotation-Y(-0.0f - heading) (00B646E0, copied with 004134F0), translation
// matrix of node+A4h..+ACh), copied with 004134F0.
void scene_node_set_heading_0047f200(Seg7Host& host, Object node, float heading);

// ---------------------------------------------------------------------------
// Soldier globals and soldier types.
// ---------------------------------------------------------------------------
Object soldier_globals_construct_00484700(Seg7Host& host, Object globals);
Object soldier_globals_get_singleton_00485520(Seg7Host& host);
// 00487770 __thiscall(ECX row, [ESP+4] LuaObject*), RET 4 (14h-byte FakeEffects row).
Object soldier_fake_effect_construct_00487770(Seg7Host& host, Object row, Object lua);
// 0048DC40 __thiscall(ECX type, [ESP+4] class, [ESP+8] LuaObject*), RET 8.
Object soldier_type_construct_0048dc40(Seg7Host& host, Object type, Object klass, Object lua);

} // namespace bsp::cc13_seg7
