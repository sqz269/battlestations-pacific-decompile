#pragma once

// Segment 6 (00465610..0047C680): engine-movie item constructors and the action
// base destructor, the scene database (00E18680) with its class map, name hash,
// hidden-entity record map, pending-object vector and command list, the cloud
// entity's visual slots, and the path entity constructor and destructor.
// Evidence and per-function notes: docs/CC13_SEG6.md. Every name is a
// hypothesis, not a recovered symbol.
//
// These bodies are written against the image's object layout: an object is a
// byte pointer and every field is read or written at the image's offset
// (native_at below). Every call the image makes, virtual or direct, goes
// through a named Seg6Host method at the same point with the same arguments;
// globals are read through the host each time the image loads them.
//
// Scene database (170h bytes, 0046F160):
//   +00h vtable 00CE59FC, +04h byte (1 after construction),
//   +08h vector<void*> {+0Ch first, +10h last, +14h end} (00442190 push_back),
//   +18h map<NativeString, record*> {+1Ch head, +20h size} (hidden records),
//   +24h vector<object*> {+28h first, +2Ch last, +30h end} (pending objects),
//   +34h class map {vtable 00CE55B0, +38h count, +3Ch 64 bucket heads},
//   +13Ch and +144h NativeStrings, +14Ch command list {count, head, tail},
//   +158h and +164h set<int> {head +4, size +8}.
// NativeString: {+0 size, +4 data}; data == 0 means empty. Hash nodes:
//   {+0 key size, +4 key data, +8 value, +0Ch next}.

#include <cstdint>

namespace bsp::cc13_seg6 {

static_assert(sizeof(void*) == 4, "segment 6 bodies use the Win32 object layout");

using Object = std::uint8_t*;

template <class T>
T& native_at(Object base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

inline Object native_ptr(Object base, std::uint32_t offset) {
    return reinterpret_cast<Object>(native_at<std::uintptr_t>(base, offset));
}

struct Seg6Host {
    virtual ~Seg6Host() = default;

    // Image addresses and globals, read where the image reads them.
    virtual Object image_address(std::uint32_t va) = 0;     // a literal address operand
    virtual Object game_00e188a8() = 0;                     // [00E188A8]
    virtual Object scene_database_00e18680() = 0;           // [00E18680]
    virtual void set_scene_database_00e18680(Object value) = 0;

    // CRT and allocators.
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* memory) = 0;
    virtual void free_00bf6989(void* memory) = 0;
    virtual void crt_memcpy_00bf7680(void* dest, const void* src, std::uint32_t bytes) = 0;
    // 00BF67A7 memmove_s(dest, dest_size, src, count), cdecl.
    virtual void crt_memmove_s_00bf67a7(void* dest, std::uint32_t dest_size, const void* src,
                                        std::uint32_t count) = 0;
    virtual std::int32_t crt_toupper_00bf924e(std::int32_t c) = 0;
    virtual std::int32_t crt_stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00BF6713, the checked-iterator report, at every point it can fire.
    virtual void checked_iterator_report_00bf6713() = 0;

    // Sized string pool: 00419CC0 returns the pool (its stack arguments stay
    // for 00BD1510), 00BD1510 (ECX = pool) takes the block back, RET 0Ch.
    virtual Object sized_pool_singleton_00419cc0(void* data, std::uint32_t bytes, std::int32_t flag) = 0;
    virtual void sized_pool_return_block_00bd1510(Object pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;

    // NativeString helpers.
    virtual void native_string_resize_0041dd40(Object string, std::uint32_t size, std::int32_t flag) = 0;
    virtual Object native_string_construct_0041e870(Object string, const char* text) = 0;  // RET 4
    virtual const char* native_string_c_str_00419ca0(Object string) = 0;                   // fastcall
    virtual std::int32_t string_compare_00438e10(const char* a, const char* b) = 0;        // ECX, EDX
    virtual char* string_duplicate_00438e40(const char* text) = 0;                          // fastcall
    virtual bool native_string_equals_00425850(Object string, const char* text) = 0;       // RET 4

    // Engine-movie items.
    virtual void movie_timed_item_construct_00465850(Object item, float time) = 0;  // RET 4

    // Scene name hash iterator {+0 table, +4 node, +8 bucket}, both fastcall.
    virtual void scene_name_hash_iterator_begin_00466bb0(Object iterator) = 0;
    virtual bool scene_name_hash_iterator_next_00466c00(Object iterator) = 0;

    // Trees, lists and vectors.
    virtual void tree_iterator_increment_00467980(Object iterator) = 0;                      // fastcall
    // 00468CD0 __thiscall(map, out iterator {cont, node}, const NativeString* key), RET 8.
    virtual Object hidden_record_map_find_00468cd0(Object map, Object out_iterator, Object key) = 0;
    virtual void hidden_record_tree_erase_0046ad10(Object map, Object root) = 0;              // RET 4
    virtual Object hidden_record_map_buy_head_node_00468260(Object map) = 0;
    virtual Object int_set_buy_head_node_004682b0(Object set) = 0;
    // 0046AC50 __thiscall(set, out pair, const int* key), RET 8.
    virtual void int_set_insert_0046ac50(Object set, Object out_pair, const std::int32_t* key) = 0;
    // 0046B170 / 0046B570 __thiscall(tree, out iterator, first {cont, node}, last {cont, node}), RET 14h.
    virtual void int_set_erase_range_0046b170(Object set, Object out_iterator, Object first_cont,
                                              Object first_node, Object last_cont, Object last_node) = 0;
    virtual void hidden_record_map_erase_range_0046b570(Object map, Object out_iterator, Object first_cont,
                                                        Object first_node, Object last_cont,
                                                        Object last_node) = 0;
    virtual void raw_list_clear_storage_004683b0(Object list) = 0;                            // fastcall
    virtual void pointer_vector_push_back_00442190(Object vector, const void* value) = 0;    // RET 4
    // 0043B0A0 __thiscall(ECX node pool, node), RET 4. 0046B240 carries its body inline.
    virtual void enum_node_pool_return_0043b0a0(Object pool, Object node) = 0;

    // Scene records and properties.
    virtual Object scene_command_record_construct_004690d0(Object record, const char* a, const char* b,
                                                           const char* c) = 0;              // RET 0Ch
    virtual Object property_bag_clone_008f41f0(Object bag) = 0;                            // fastcall
    virtual Object property_bag_find_008f2260(Object bag, const char* name) = 0;           // RET 4
    virtual std::uint32_t scene_property_array_element_count_008ef7f0(Object property) = 0;  // fastcall
    virtual void matrix_copy_4x4_004134f0(Object dest, Object source) = 0;                  // RET 4
    virtual void sentity_init_all_00925f20(std::uint8_t flag) = 0;                          // CL
    virtual void writer_vcall_0c(Object writer, std::int32_t key_zero, const char* key,
                                 std::int32_t value_zero, const char* value) = 0;           // vtable +0Ch

    // Virtual calls, named by vtable offset.
    virtual void object_deleting_destructor_vcall_00(Object object, std::int32_t flag) = 0;  // vtable +0
    virtual bool entity_is_kind_vcall_5c(Object entity, std::int32_t kind) = 0;             // vtable +5Ch
    virtual Object model_source_vcall_08(Object source, std::int32_t kind, float scale) = 0; // vtable +8
    virtual void cloud_instance_vcall_34(Object instance, Object transform) = 0;            // vtable +34h
    virtual void scene_node_vcall_38(Object node, Object transform) = 0;                    // vtable +38h

    // Entities, nodes and the cloud manager.
    virtual void sentity_init_base_0048dbd0(Object entity) = 0;
    virtual void class_entity37_destruct_base_00487380(Object entity) = 0;
    virtual Object atmosphere_cloud_manager_00bbddf0(Object atmosphere) = 0;
    virtual void cloud_manager_release_instance_00bbd910(Object manager, Object instance) = 0;  // RET 4
    virtual Object cloud_manager_create_instance_00bbd770(Object manager, Object cloud_class_28) = 0;
    virtual void cloud_instance_invalidate_lighting_00bbe760(Object instance, std::int32_t arg) = 0;
    virtual Object unit_part_instance_construct_007135c0(Object block, Object entity, Object model) = 0;
    virtual void node_set_parent_00b6e680(Object node, Object parent) = 0;                   // RET 4
    virtual void node_or_flags_recursive_00475e60(Object node, std::uint32_t flags) = 0;     // RET 4
    virtual void node_propagate_root_registration_00b6d890(Object node, Object root) = 0;    // RET 4
    virtual void scene_node_set_visibility_factor_00b6da70(Object node, float factor,
                                                           std::int32_t flag) = 0;          // RET 8
    virtual void sentity_write_scene_properties_00924d90(Object entity, Object writer) = 0;  // RET 4
    virtual void game_entity_construct_00928630(Object entity, std::int32_t zero, std::int32_t id) = 0;
    virtual void game_entity_destruct_009287b0(Object entity) = 0;
    virtual void path_component_construct_007b28c0(Object component, Object entity) = 0;    // RET 4
    virtual void path_component_destruct_007b1bd0(Object component) = 0;
};

// Engine-movie items.
void movie_action_destruct_base_00465610(Seg6Host& host, Object action);
Object movie_timed_flag_construct_004658d0(Seg6Host& host, Object item, float time);
Object movie_timed_box_construct_00465950(Seg6Host& host, Object item, float x, float y, float z, float time);
// 00465A40: the trailing NativeString is passed by value (size, data) and its
// block is released before the return.
Object movie_timed_record_construct_00465a40(Seg6Host& host, Object item, float a0, float a1, float a2,
                                             float time, const float b[6], std::int32_t value_34,
                                             float value_18, std::uint32_t text_size, char* text_data);

// Matrix and small value helpers.
void matrix_set_rotation_from_euler_00467050(float* matrix, float a, float b, float c);
void command_target_init_position_00468560(Object descriptor, const float* position, std::uint32_t extra);

// Scene database.
void scene_database_init_entities_once_00467370(Seg6Host& host, Object flag);
void pod_pointer_array_reserve_004674f0(Seg6Host& host, Object array, std::int32_t count);
Object scene_database_find_class_row_by_id_00467ee0(Seg6Host& host, Object database, std::int32_t id);
void scene_command_list_push_back_00468350(Seg6Host& host, Object list, Object record);
void scene_property_collect_array_elements_00468600(Seg6Host& host, Object property, Object array);
Object scene_database_class_id_to_name_00468660(Seg6Host& host, Object database, std::int32_t id);
std::uint32_t scene_name_hash_bucket_004689d0(Seg6Host& host, Object table, Object key);
std::uint32_t scene_class_map_bucket_00468a90(Seg6Host& host, Object table, Object key);
Object scene_name_hash_find_node_00468ed0(Seg6Host& host, Object table, Object key, std::uint32_t* bucket);
Object scene_class_map_find_node_00468fb0(Seg6Host& host, Object table, Object key, std::uint32_t* bucket);
void scene_record_destruct_fields_00469010(Seg6Host& host, Object record);
Object scene_database_find_hidden_entity_record_004691b0(Seg6Host& host, Object database, const char* name);
Object scene_hidden_entity_record_copy_construct_00469240(Seg6Host& host, Object record, Object source);
void scene_hidden_entity_record_destruct_00469300(Seg6Host& host, Object record);
Object scene_hidden_entity_record_deleting_destructor_004693a0(Seg6Host& host, Object record,
                                                               std::uint8_t flags);
Object scene_hidden_entity_record_construct_004693c0(Seg6Host& host, Object record, Object bag,
                                                     const char* class_name, const char* name,
                                                     std::int32_t party, Object frame,
                                                     Object parent_name);
bool scene_database_has_hidden_entity_record_00469480(Seg6Host& host, Object database, const char* name);
void scene_database_queue_entity_command_00469610(Seg6Host& host, Object database, const char* a,
                                                  const char* b, const char* c);
std::int32_t scene_database_class_name_to_id_00469690(Seg6Host& host, Object database,
                                                      const char* class_name);
void scene_database_destroy_pending_objects_004697b0(Seg6Host& host, Object database);
std::uint32_t scene_name_table_lookup_value_00469b60(Seg6Host& host, Object owner, const char* name);
std::uint32_t scene_database_find_class_value_by_name_00469eb0(Seg6Host& host, Object database,
                                                               const char* class_name);
void scene_record_ptr_delete_0046a090(Seg6Host& host, Object slot);
void scene_command_list_clear_0046a9f0(Seg6Host& host, Object list);
void named_vector_destruct_0046aa60(Seg6Host& host, Object object);
void scene_class_map_destruct_0046b240(Seg6Host& host, Object map);
void scene_database_clear_hidden_records_0046b680(Seg6Host& host, Object database);
void scene_database_release_records_and_pending_0046bd10(Seg6Host& host, Object database);
void scene_stock_push_positive_counters_0046be50(Seg6Host& host, Object owner);
void scene_stock_register_unit_class_0046be90(Seg6Host& host, Object owner, std::int32_t unit_class);
void scene_stock_register_from_type_property_0046bf20(Seg6Host& host, Object owner, Object bag,
                                                      const char* class_name);
void scene_database_destruct_0046efb0(Seg6Host& host, Object database);
Object scene_database_construct_0046f160(Seg6Host& host, Object database);
void scene_database_create_singleton_0046f350(Seg6Host& host);
void scene_database_destroy_singleton_0046f3e0(Seg6Host& host);

// Visual class entities, cloud entity and path entity.
void class_entity37_sentity_init_00472530(Seg6Host& host, Object entity);
void class_entity37_sentity_init_00473e30(Seg6Host& host, Object entity);
void cloud_entity_destruct_00475990(Seg6Host& host, Object entity);
void cloud_particle_array_reserve_00476550(Seg6Host& host, Object array, std::int32_t count);
void cloud_entity_create_visual_00476bd0(Seg6Host& host, Object entity);
void cloud_entity_update_visibility_00476d80(Seg6Host& host, Object entity);
void cloud_entity_write_scene_properties_00477180(Seg6Host& host, Object entity, Object writer);
Object property_bag_find_by_cstring_004772c0(Seg6Host& host, Object bag, const char* name);
Object path_entity_construct_0047b660(Seg6Host& host, Object entity, std::int32_t id);
void path_entity_destruct_0047b700(Seg6Host& host, Object entity);

}  // namespace bsp::cc13_seg6
