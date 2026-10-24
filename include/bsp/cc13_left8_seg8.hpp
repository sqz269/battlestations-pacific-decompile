#pragma once
// Packet cc13-left8: the leftover game functions of segment 8 (00491170..004BA200)
// that the first segment-8 pass (classes_seg8_cc13) did not take. Scene traffic
// groups and their members, the ship idle actors, landing-troop managers, the
// soldier animation start and related class loaders. docs/CC13_LEFT8.md lists
// every function with its ABI, evidence and uncertainty, and the functions that
// were classified as STL instantiations or skipped.
//
// Objects are reached through byte offsets in their Win32 layout (Object plus
// at<T>). Every call the image makes goes through a Left8Host method named for
// its target address (virtual calls by their vtable offset), at the same point
// and with the same arguments. Checked-iterator traps (00BF6713, which does not
// return in the image) are host calls at the points the image tests. Names are
// hypotheses, not recovered symbols. These are new C++ interfaces over an
// injected host: build-tested only, not wired into a game host.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_left8 {

static_assert(sizeof(void*) == 4, "segment 8 objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}
inline Object obj(std::uint32_t value) noexcept {
    return reinterpret_cast<Object>(static_cast<std::uintptr_t>(value));
}

// A checked std::vector<bool> reference or iterator as the image builds it on
// the stack: {container, word pointer, bit}. The container is the 14h-byte
// vector<bool> {bit count +0, word vector: proxy +4, first +8, last +0Ch, end +10h}.
struct BitRef {
    Object container;
    std::uint32_t* word;
    std::uint32_t bit;
};

// A 14h-byte Lua object handle (opaque here).
struct LuaObject {
    std::uint8_t storage[0x14];
};

// The 14h-byte Lua reader (004425C0 constructs it; opaque here).
struct LuaReader {
    std::uint8_t storage[0x14];
};

// The scene property block 008F41A0 builds on the stack (opaque; the image's
// frame holds it below a 114h-byte local area).
struct PropertyBlock {
    std::uint8_t storage[0x110];
};

// A tagged value as the Lua reader takes it by value: {tag, value}; tag 0 is a
// string pointer, 1 an int pointer, 2 a float pointer.
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t value;
};

class Left8Host {
public:
    virtual ~Left8Host() = default;

    // Globals and the CRT.
    virtual std::uint32_t global_u32(std::uint32_t address) = 0;
    virtual float global_f32(std::uint32_t address) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void crt_free_00bf65ac(void* memory) = 0;
    virtual void enter_critical_section(void* section) = 0;   // [00CE2218]
    virtual void leave_critical_section(void* section) = 0;   // [00CE2210]

    // STL helpers the image calls (bodies are library instantiations).
    // 0048F930 __thiscall(vector<bool>, BitRef* out, index), RET 8, EAX = out.
    virtual BitRef* bit_vector_at_0048f930(Object bit_vector, BitRef* out, std::uint32_t index) = 0;
    // 0048F230 __thiscall(BitRef*), the checked read of the referenced bit.
    virtual bool bit_ref_get_0048f230(BitRef* ref) = 0;
    // 0048D3B0 __thiscall(BitRef* iterator, distance), RET 4: advance in place.
    virtual void bit_iterator_advance_0048d3b0(BitRef* iterator, std::int32_t distance) = 0;
    // 00488630 __thiscall(map, iterator* out {container, node}, const key*), RET 8.
    virtual void tree_find_00488630(Object map, Object out_iterator, const void* key) = 0;
    // 0048F9A0 __thiscall(map, iterator* out, container, node), RET 0Ch: erase.
    virtual void tree_erase_0048f9a0(Object map, Object out_iterator, Object container, Object node) = 0;
    // std::list helpers: the sentinel allocation (ECX = list, EAX = head), the
    // node buy (ECX = list; next, prev, const value*; EAX = node) and the
    // checked size increment (ECX = list, count).
    virtual Object list_allocate_sentinel_00485270(Object list) = 0;
    virtual Object list_buy_node_00485880(Object list, Object next, Object prev, const void* value) = 0;
    virtual void list_incsize_0048fd20(Object list, std::uint32_t count) = 0;
    virtual Object list_buy_node_004857f0(Object list, Object next, Object prev, const void* value) = 0;
    virtual void list_incsize_0048fc80(Object list, std::uint32_t count) = 0;

    // Virtual calls by vtable offset (ECX = object).
    virtual void vcall_deleting_dtor_00(Object object, std::uint32_t flags) = 0;     // +0
    virtual Object vcall_create_instance_08(Object class_object) = 0;                // +8
    virtual void vcall_attach_98(Object entity, Object owner, Object owner_node, Object pose) = 0;  // +98h
    virtual void vcall_a0(Object entity) = 0;                                        // +A0h
    virtual void vcall_d8(Object entity) = 0;                                        // +D8h
    virtual void vcall_set_group_114(Object entity, Object group) = 0;               // +114h
    virtual void vcall_11c(Object entity) = 0;                                       // +11Ch
    virtual bool vcall_group_is_visible_0c(Object group) = 0;                        // +0Ch
    // [[[ent+178h]+160h]+0Ch] vtable +24h(anim id, float start, 1).
    virtual void anim_node_play_24(Object anim_node, std::uint32_t anim_id, float start, std::uint32_t one) = 0;

    // Engine calls.
    virtual void mission_entity_kill_00926d90(Object entity, std::int32_t reason) = 0;
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    virtual float* vector4_transform_00b62d10(float* out, const float* point, const float* matrix) = 0;
    virtual bool game_is_aabb_in_view_004b48e0(Object game, const float* box) = 0;
    virtual void scene_entity_layer_reset_child_00481430(Object entity, std::int32_t layer, std::int32_t index) = 0;
    virtual Object scene_entity_layer_member_0048c6a0(Object entity, std::int32_t layer, std::int32_t index) = 0;
    virtual float game_world_sample_water_height_0078cf20(Object world, float x, float z) = 0;
    virtual void landing_troop_on_ship_left_0048c190(Object troop) = 0;
    virtual void traffic_group_flush_pending_visibility_00487940(Object group) = 0;
    // 00B78670 __thiscall(anim set, const void* name), RET 4: the animation id.
    virtual std::uint32_t anim_set_find_00b78670(Object anim_set, const void* name) = 0;
    // 00B6DC90 __thiscall(anim node, id), RET 4, ST0: the animation duration.
    virtual float anim_node_duration_00b6dc90(Object anim_node, std::uint32_t anim_id) = 0;
    // 00444BE0 __thiscall(map<string,float>, const key*), RET 4: the value slot.
    virtual float* string_float_map_lookup_00444be0(Object map, const void* key) = 0;
    // 00BD2F10 __thiscall(stream, lo, hi), RET 8, ST0. The image passes stream
    // ECX = 0 (the shared stream) at every site in this file.
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float lo, float hi) = 0;

    virtual void game_submit_main_scene_render_0068a0d0(Object game) = 0;          // ECX = [00E198C4]
    virtual std::uint32_t camera_classify_aabb_00b71590(Object frustum, const float* box) = 0;
    virtual void camera_set_near_plane_00b6fbf0(Object camera, float near_plane) = 0;
    // 004F3730 __fastcall(ECX a0, EDX a1, b0, b1, out), RET 0Ch, AL (float2 points).
    virtual bool math_segment_crossing_2d_004f3730(const float* a0, const float* a1, const float* b0,
                                                   const float* b1, float* out) = 0;
    virtual void frontend_screen_commit_visibility_004f83b0(Object screen) = 0;    // ECX = screen
    virtual void vcall_screen_refresh_18(Object screen) = 0;                         // +18h

    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 00442190 __thiscall(vector<int>, const int*), RET 4: push_back.
    virtual void int_vector_push_back_00442190(Object vector, const std::int32_t* value) = 0;
    // 00484660 __thiscall(vector<int>): the deterministic hash of the elements.
    virtual std::uint32_t int_vector_hash_00484660(Object vector) = 0;
    // 00480A90 __fastcall(ECX seed, EDX weights vector): weighted pick index.
    virtual std::uint32_t weighted_pick_00480a90(std::uint32_t seed, Object weights) = 0;
    virtual void matrix_copy_004134f0(Object destination, const float* source) = 0;   // ECX = destination
    // 00467050 __thiscall(matrix, x, y, z), RET 0Ch.
    virtual void matrix_set_rotation_from_euler_00467050(Object matrix, float x, float y, float z) = 0;
    // 0047BA40 __thiscall(tree, box, &i0, &i1, &j0, &j1), RET 14h.
    virtual void quad_cell_range_0047ba40(Object tree, const float* box, std::int32_t* i0, std::int32_t* i1,
                                          std::int32_t* j0, std::int32_t* j1) = 0;
    // 00481510 __thiscall(node, box), RET 4, EAX = node.
    virtual Object quad_node_construct_00481510(Object node, const float* box) = 0;
    // 004879B0 __thiscall(tree, Node** slot, const float* box), RET 8.
    virtual void traffic_bounds_update_004879b0(Object tree, Object slot, Object box) = 0;
    // 004722D0 __fastcall(ECX = NativeString* name), EAX = the anim class.
    virtual Object anim_class_get_or_create_004722d0(const void* name) = 0;
    // 006952A0 / 00694A60 __fastcall(ECX = target, EDX = observer).
    virtual void observer_unregister_pair_006952a0(Object target, Object observer) = 0;
    virtual void observer_register_pair_00694a60(Object target, Object observer) = 0;

    // Path runners (ECX = runtime).
    virtual void path_runner_update_members_00496bd0(Object runtime) = 0;
    virtual void path_runner_update_formation_00497bb0(Object runtime, float t, bool force_visible) = 0;  // RET 8
    virtual void traffic_runtime_kill_all_members_0048a5a0(Object runtime) = 0;
    virtual void landing_lane_rebuild_path_0048c230(Object lane) = 0;
    virtual void landing_ship_finish_landing_0074ad90(Object ship) = 0;

    // Destructor helpers.
    virtual void global_add_i32(std::uint32_t address, std::int32_t delta) = 0;
    virtual void record54_copy_assign_0047f930(Object destination, Object source) = 0;  // ECX = destination
    virtual void record54_vector_resize_00495d40(Object vector, std::uint32_t size) = 0;
    virtual void record4c_vector_resize_00495e20(Object vector, std::uint32_t size) = 0;
    virtual void callback_owner_destroy_00695870(Object observer) = 0;                   // ECX = observer
    virtual void list_node_destruct_00489dd0(Object node) = 0;                            // ECX = node
    // 00484AE0 __thiscall(tree, Node** slot, const float* box), RET 8.
    virtual void quad_tree_prune_00484ae0(Object tree, Object slot, const float* box) = 0;
    // 004914A0 (ECX = first, EDX = last, stack: vector proxy, owner): destroys the row vectors.
    virtual void row_vectors_destroy_004914a0(Object first, Object last, Object proxy, Object owner) = 0;

    // Native strings and Lua objects.
    virtual void* string_pool_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;           // ECX = string
    // 00B67800 __thiscall(source, out, name), RET 8, EAX = out.
    virtual LuaObject* lua_get_by_name_00b67800(LuaObject* source, LuaObject* out, const char* name) = 0;
    virtual const char* lua_get_string_00b662b0(LuaObject* object) = 0;                      // ECX = object
    virtual void lua_object_destruct_00b67700(LuaObject* object) = 0;                        // ECX = object
    virtual bool lua_is_nil_00b65fb0(LuaObject* object) = 0;                                 // ECX = object
    // 00888760 __fastcall(ECX = float3 out, EDX = Lua object).
    virtual void lua_read_vector3_00888760(float* out, LuaObject* object) = 0;
    // 0049CB60 __thiscall(map, const NativeString* key), RET 4: map<string, Object*>::operator[].
    virtual Object* temp_map_subscript_0049cb60(Object map, const void* key) = 0;

    // Traffic manager and record helpers.
    virtual void critical_section_destroy_owned_0041cc80(Object owner) = 0;                 // ECX = owner
    // Map range erasers __thiscall(map, &result, container, first, container, last), RET 14h.
    virtual void tree_erase_range_0049a230(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void tree_erase_range_00499b90(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void tree_erase_range_004906d0(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void tree_erase_range_00496390(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void tree_erase_range_00493d50(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void traffic_layers_rebuild_0049e560(Object object) = 0;                        // ECX = object
    virtual Object tree_buy_head_node_004858d0(Object map) = 0;                              // ECX = map
    virtual Object string_float_map_blank_node_00443e20(Object map) = 0;                    // ECX = map
    virtual void tree_copy_construct_004990b0(Object map, Object source) = 0;               // RET 4
    virtual void string_float_map_copy_construct_0049dec0(Object map, Object source) = 0;   // RET 4
    virtual void tree_erase_range_00444b10(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void tree_erase_range_00492a00(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    // 00432050 (ECX = first, EDX = last, stack: vector proxy, owner): destroys the elements.
    virtual void element_range_destroy_00432050(Object first, Object last, Object proxy, Object owner) = 0;
    virtual void list_destruct_00487110(Object list) = 0;                                    // ECX = list
    virtual void list_tidy_00485340(Object list) = 0;                                        // ECX = list
    virtual void traffic_path_set_destruct_0048ee90(Object object) = 0;                     // ECX = object
    // 004A5B30 __thiscall(registry, key, value), RET 8.
    virtual void traffic_registry_remove_004a5b30(Object registry, Object key, Object value) = 0;
    virtual Object path_object_get_entity_007aca30(Object path) = 0;                         // ECX = path
    // 004A0410 __thiscall(group, parent, pad, a3, ship), RET 10h, EAX = group.
    virtual Object landing_troop_group_construct_004a0410(Object group, Object parent, Object pad, std::uint32_t a3,
                                                          Object ship) = 0;
    // 004926A0 __thiscall(multimap, iterator* out, const pair* {key, value}), RET 8.
    virtual void tree_insert_equal_004926a0(Object map, Object out, const std::uint32_t* pair) = 0;
    // 004A5750 __thiscall(map, const key*), RET 4, EAX = the vector of groups for the key.
    virtual Object traffic_registry_groups_004a5750(Object map, const std::uint32_t* key) = 0;
    virtual void vcall_group_release_member_08(Object group, Object member) = 0;            // +8

    // Soldier and wreck classes.
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void class_entity_construct_base_00489f30(Object entity) = 0;                   // ECX = entity
    virtual void class_entity_set_class_0047b9e0(Object entity, Object entity_class) = 0;    // RET 4
    virtual void class_entity37_sentity_init_base_0048dbd0(Object entity) = 0;               // ECX = entity
    virtual std::uint32_t node_query_00b6d980(Object node) = 0;                              // ECX = node
    virtual Object soldier_globals_00485520() = 0;
    virtual void node_set_word50_00b6dca0(Object node, float value) = 0;                     // RET 4
    virtual void matrix_orthonormalize_0085dad0(Object matrix) = 0;                          // ECX = matrix
    virtual void vcall_class_activate_0c(Object entity_class) = 0;                           // +0Ch
    virtual float vector3_length_0042b2f0(const float* vector) = 0;                          // ECX = vector, ST0
    // 00BF857A, the CRT x87 fmod helper: ST1 mod ST0.
    virtual double crt_fmod_00bf857a(double x, double y) = 0;
    virtual void scene_entity_refresh_visibility_factor_00484940(Object entity) = 0;          // ECX = entity
    virtual void point_effect_stop_children_00867b10(Object effect) = 0;                      // ECX = effect
    virtual bool vcall_is_kind_of_5c(Object object, std::uint32_t kind) = 0;                  // +5Ch
    // 0087FA20 __thiscall(landscape, x, z), RET 8, ST0; 0087FB90 __thiscall(landscape, out, x, z), RET 0Ch, EAX = out.
    virtual float landscape_forward_local_height_0087fa20(Object landscape, float x, float z) = 0;
    virtual const float* landscape_forward_local_normal_0087fb90(Object landscape, float* out, float x, float z) = 0;
    virtual void scene_node_invalidate_subtree_pose_0042ed50(Object node) = 0;                 // ECX = node

    // Soldier class registry and Lua iteration.
    virtual void named_class_read_lua_0048f670(Object entity_class, LuaObject* table) = 0;   // RET 4
    virtual void lua_object_construct_00b65f50(LuaObject* object) = 0;                      // ECX = object
    // 00B67080 / 00B67190 __thiscall(table, key*, value*), RET 8; 00B66420 __thiscall(table, key*), RET 4.
    virtual void lua_iterate_first_00b67080(LuaObject* table, LuaObject* key, LuaObject* value) = 0;
    virtual void lua_iterate_next_00b67190(LuaObject* table, LuaObject* key, LuaObject* value) = 0;
    virtual bool lua_iterate_done_00b66420(LuaObject* table, LuaObject* key) = 0;
    virtual double lua_get_number_00b66270(LuaObject* object) = 0;                          // ECX = object, ST0
    // 0041DD40 __thiscall(NativeString*, length, 1), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t flag) = 0;
    virtual Object soldier_registry_004b1330() = 0;
    // 004B0CB0 __thiscall(map, const NativeString* key), RET 4, EAX = &mapped.
    virtual Object* soldier_registry_get_or_insert_004b0cb0(Object map, const void* key) = 0;
    virtual void lua_get_globals_00b67980(Object owner, LuaObject* out) = 0;                 // ECX = owner, RET 4
    // 00B68100 __thiscall(table, out, const NativeString* name), RET 8.
    virtual LuaObject* lua_get_by_native_string_00b68100(LuaObject* table, LuaObject* out, const void* name) = 0;
    virtual Object soldier_class_construct_004b12a0(Object memory) = 0;                      // ECX = memory
    virtual void vcall_class_read_lua_04(Object entity_class, LuaObject* table) = 0;         // +4
    virtual void soldier_registry_iterator_increment_004afb50(Object iterator) = 0;          // ECX = iterator
    // 004B0600 __thiscall(map, node), RET 4: frees the subtree.
    virtual void soldier_registry_destroy_subtree_004b0600(Object map, Object node) = 0;
    virtual Object wreck_registry_004b3c50() = 0;
    // 004B39B0 __thiscall(map, const NativeString* key), RET 4, EAX = &mapped.
    virtual Object* wreck_registry_lookup_004b39b0(Object map, const void* key) = 0;
    virtual void named_class_construct_base_00489e60(Object entity_class) = 0;               // ECX = class
    virtual void wreck_registry_iterator_increment_004b1b70(Object iterator) = 0;            // ECX = iterator
    virtual void wreck_registry_destroy_subtree_004b2dd0(Object map, Object node) = 0;       // RET 4
    virtual void mission_entity_register_00926be0(Object entity) = 0;                        // ECX = entity
    // 009039D0 __thiscall(world, const float* pos), RET 4, EAX = the containing object.
    virtual Object world_find_container_009039d0(Object world, const float* pos) = 0;
    virtual void matrix_inverse_00b63d50(Object destination, Object source) = 0;             // ECX, EDX
    // 00439820 __fastcall(ECX = out, EDX = point, matrix, flag), RET 8, EAX = out.
    virtual const float* matrix_transform_point_00439820(float* out, const float* point, Object matrix,
                                                         std::uint32_t flag) = 0;
    // Reference counting: [00CE221C] InterlockedIncrement, [00CE2220] InterlockedDecrement,
    // and vtable slot 0 called with no arguments.
    virtual long interlocked_increment_ce221c(long* addend) = 0;
    virtual long interlocked_decrement_ce2220(long* addend) = 0;
    virtual void object_slot_00(Object object) = 0;
    virtual std::int32_t lua_get_integer_00b66290(LuaObject* object) = 0;                    // ECX = object
    // 00870CD0 __fastcall(ECX = out handle slot, EDX = id, flag), RET 4, EAX = out.
    virtual Object* effect_handle_acquire_00870cd0(Object* out, std::int32_t id, std::uint32_t flag) = 0;

    // Wreck SEntityInit.
    // 008F2260 __thiscall(bag, const char* name), RET 4, EAX = the property entry {.., tag +4, value +0Ch}.
    virtual Object scene_property_bag_find_008f2260(Object bag, const char* name) = 0;
    virtual void sentity_get_saved_lua_data_009238a0(Object entity, LuaObject* out) = 0;     // RET 4
    // 004425C0 __thiscall(reader, LuaObject by value), the 14h-byte reader.
    virtual void lua_reader_construct_004425c0(LuaReader* reader, LuaObject* data) = 0;
    // 00BD8E20 __thiscall(reader, key by value); 00BD6830 (reader, key, value) by value.
    virtual void lua_reader_enter_key_00bd8e20(LuaReader* reader, TaggedValue key) = 0;
    virtual void lua_reader_read_field_00bd6830(LuaReader* reader, TaggedValue key, TaggedValue value) = 0;
    virtual void lua_reader_leave_key_00bd7a20(LuaReader* reader) = 0;
    virtual void lua_reader_destroy_00441a20(LuaReader* reader) = 0;
    // 008687C0 __fastcall(ECX = out, EDX = parent node, handle by value, const matrix*, 1, 0, 0), RET 14h, EAX = out.
    virtual Object* point_effect_create_with_parent_matrix_008687c0(Object* out, Object node, Object handle,
                                                                      const float* matrix, std::uint32_t a,
                                                                      std::uint32_t b, std::uint32_t c) = 0;

    // Ship part class registry.
    virtual void global_set_u32(std::uint32_t address, std::uint32_t value) = 0;
    virtual void ship_part_registry_iterator_increment_004abda0(Object iterator) = 0;        // ECX = iterator
    virtual void ship_part_registry_destroy_subtree_004adb80(Object map, Object node) = 0;   // RET 4
    virtual void ship_part_objects_delete_all_004ad2a0() = 0;
    virtual void tree_erase_range_004ae3e0(Object map, Object result, Object c0, Object first, Object c1, Object last) = 0;
    virtual void global_set_f32(std::uint32_t address, float value) = 0;
    // 00B67720 __thiscall(table, out, index), RET 8, EAX = out.
    virtual LuaObject* lua_get_by_index_00b67720(LuaObject* table, LuaObject* out, std::int32_t index) = 0;
    virtual Object image_address(std::uint32_t address) = 0;
    // 008F41A0 __thiscall(block, 0), RET 4; 0043D8F0 __fastcall(ECX = source, EDX = block);
    // 0049CF80 __thiscall(record, block), RET 4; 008F5410 __thiscall(block).
    virtual void property_block_construct_008f41a0(PropertyBlock* block, std::uint32_t flag) = 0;
    virtual void property_block_from_lua_0043d8f0(Object source, PropertyBlock* block) = 0;
    virtual void traffic_record_apply_properties_0049cf80(Object record, PropertyBlock* block) = 0;
    virtual void property_block_destruct_008f5410(PropertyBlock* block) = 0;
    // 00B66330 __thiscall(object, float default), RET 4, ST0; 00B66380 (object, int default), RET 4, EAX.
    virtual double lua_get_float_or_default_00b66330(LuaObject* object, float fallback) = 0;
    virtual std::int32_t lua_get_integer_or_default_00b66380(LuaObject* object, std::int32_t fallback) = 0;
    virtual bool lua_is_table_00b661b0(LuaObject* object) = 0;                               // ECX = object
    virtual void lua_object_assign_00b67690(LuaObject* destination, LuaObject* source) = 0; // RET 4
    // 004AEC00 __fastcall(ECX = const NativeString* name), EAX = the ship part class.
    virtual Object ship_part_class_get_or_load_004aec00(const void* name) = 0;
    // Pointer vector inserts __thiscall(vector, &result, container, where, const Object*), RET 10h.
    virtual void pointer_vector_insert_004ae230(Object vector, Object result, Object container, Object where,
                                                const Object* value) = 0;
    virtual void pointer_vector_insert_004ae1a0(Object vector, Object result, Object container, Object where,
                                                const Object* value) = 0;
};

// Ship idle actors (a traffic group, vtable 00CE69D4: 004A6B50, 00497B80,
// 00491860, 00481090).
void idle_group_release_member_00491860(Left8Host& host, Object group, Object member);
void ship_idle_actor_instantiate_00491950(Left8Host& host, Object group, std::uint32_t index);
void ship_idle_actors_update_00494f30(Left8Host& host, Object group);
void ship_idle_actors_tick_00497b80(Left8Host& host, Object group);

// The layered water entity tick (vtable 00CE67AC slot 1).
void layered_water_entity_update_00491ac0(Left8Host& host, Object entity);

// Landing-troop manager: remove the group keyed by a ship.
void landing_traffic_remove_ship_00491e70(Left8Host& host, Object manager, std::uint32_t ship, bool destroy);
void landing_troops_launch_004a4520(Left8Host& host, Object manager, Object config, std::uint32_t a3, Object ship);

// The list-linked child node base constructor and the list-group release.
Object list_node_construct_00496920(Left8Host& host, Object node, Object parent);
void list_group_release_member_004969d0(Left8Host& host, Object group, Object member);

// Scene traffic cells and the list-node objects built on 00496920.
Object traffic_cell_member_00498790(Left8Host& host, Object tree, std::uint32_t i, std::uint32_t j);
void traffic_visible_cells_00498c40(Left8Host& host, Object tree, Object slot, const float* box);
Object temp_anim_construct_00498d60(Left8Host& host, Object object, Object parent, const void* class_name,
                                    const float* matrix);
Object plane_debris_spawner_construct_00498e60(Left8Host& host, Object spawner, Object parent, Object observed);
Object plane_debris_spawner_create_00498f80(Left8Host& host, Object owner, Object observed);

// TempAddAnim / RemoveTempAddedStuff.
void temp_add_anim_0049dc00(Left8Host& host, Object manager, LuaObject* table);
void remove_temp_added_0049de90(Left8Host& host, Object manager, const void* id);

// Traffic manager and the 4Ch traffic record.
void traffic_manager_destruct_0049f0e0(Left8Host& host, Object manager);
void traffic_refresh_if_set_0049f260(Left8Host& host, Object object);
Object traffic_record_construct_0049f9b0(Left8Host& host, Object record);
void traffic_record_apply_from_lua_0049d4c0(Left8Host& host, Object record, Object source);
Object traffic_record_copy_0049fa70(Left8Host& host, Object record, Object source);
void traffic_record_destruct_0049f910(Left8Host& host, Object record);
void path_runtime_destruct_0049fb20(Left8Host& host, Object runtime);
void soldier_type_destruct_004a02e0(Left8Host& host, Object object);
void path_runtime_owned_destruct_004a6510(Left8Host& host, Object runtime);
void ship_idle_group_destruct_004a6580(Left8Host& host, Object group);
void landing_lane_destruct_004a67c0(Left8Host& host, Object lane);

// Destructors.
void plane_debris_spawner_destruct_0049a580(Left8Host& host, Object spawner);
void record4c_owner_destruct_0049c6e0(Left8Host& host, Object owner);
void traffic_tree_destruct_0049d540(Left8Host& host, Object tree);

// Path runner ticks.
void path_runner_tick_0049a360(Left8Host& host, Object runtime, float dt);
void landing_lane_tick_0049a410(Left8Host& host, Object lane, float dt);

void traffic_registry_kill_member_004a5b90(Left8Host& host, Object registry, std::uint32_t key, std::int32_t index);

// Weighted entry pick of the point-announcement constructor.
Object weighted_entry_pick_004ab9a0(Left8Host& host, Object table);

// Soldier and wreck class helpers.
Object soldier_class_create_instance_004af890(Left8Host& host, Object soldier_class);
void soldier_sentity_init_004affe0(Left8Host& host, Object soldier);
Object matrix_orthonormalize_return_004b1880(Left8Host& host, Object matrix);
Object wreck_class_get_or_create_activated_004b3f10(Left8Host& host, const void* name);
void soldier_advance_animation_phase_004afca0(Left8Host& host, Object soldier);
void soldier_class_read_lua_004b0de0(Left8Host& host, Object soldier_class, LuaObject* table);
Object soldier_class_get_or_load_004b1400(Left8Host& host, const void* name);
void soldier_registry_destroy_004b1540(Left8Host& host);
Object wreck_class_get_or_create_004b3d20(Left8Host& host, const void* name);
void wreck_class_registry_clear_004b3e60(Left8Host& host);
Object wreck_class_create_instance_004b2030(Left8Host& host, Object wreck_class, Object source);
void wreck_class_read_lua_fields_004b21e0(Left8Host& host, Object wreck_class, LuaObject* table);
void wreck_sentity_init_004b3480(Left8Host& host, Object wreck);
void ship_part_class_registry_destruct_004aea00(Left8Host& host, Object registry);
void ship_explosion_load_globals_004af290(Left8Host& host);
void ship_part_class_read_lua_fields_004acab0(Left8Host& host, Object part_class, LuaObject* table);
void plane_part_class_read_lua_fields_004a7d30(Left8Host& host, Object part_class, LuaObject* table);
Object ship_explosion_read_parts_table_004aed50(Left8Host& host, std::int32_t index, bool force);
void wreck_tick_phases_004b18b0(Left8Host& host, Object wreck, float dt);
void wreck_place_on_surface_004b2760(Left8Host& host, Object wreck);

// Soldier animation start.
void soldier_start_animation_004b0f80(Left8Host& host, Object soldier, const void* anim_name, bool random_start,
                                      bool force);

// Small accessors at the end of the segment.
Object mission_scoring_commit_record_004b4750(Object game);
void render_camera_update_job_run_004b4820(Left8Host& host, Object job, std::uint32_t unused);
bool game_is_aabb_in_view_004b48e0(Left8Host& host, Object game, const float* box);
void game_set_camera_near_plane_004b4940(Left8Host& host, Object game, float near_plane);
void segment_crossing_2d_float3_004b6720(Left8Host& host, const float* a0, const float* a1, const float* b0,
                                         const float* b1, float* out);
Object cloud_light_colours_construct_004b6980(Object colours);
void frontend_screen_mark_visible_004b6e30(Left8Host& host, Object screen);
int format_string_512_004b7f70(char* dst, const char* format, ...);
int format_string_255_004b8410(char* dst, const char* format, ...);
int format_string_4092_004b8450(char* dst, const char* format, ...);
int format_string_256_004b8490(char* dst, const char* format, ...);

}  // namespace bsp::cc13_left8
