#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::powerup_seg58_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

inline Raw raw(const void* ptr) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(ptr))};
}

inline void* add_bytes(const void* ptr, std::ptrdiff_t off) noexcept {
  return const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(ptr)) + off;
}

struct Seg58Host {
  virtual ~Seg58Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  virtual void* award_tracker_get_singleton_004e1ca0() = 0;
  virtual std::uint8_t award_tracker_has_hint_been_shown_0068e600( void* tracker, const void* name_str) = 0;
  virtual void award_tracker_trigger_hint_00690fd0(void* tracker, const void* name_str, std::uint8_t forced) = 0;
  virtual void callback_owner_destroy_00695870(void* owner) = 0;
  virtual void cg_adjustor_thunk_0076a9f0(void* session, void* msg, std::int32_t except_peer) = 0;
  virtual void eh_vector_destructor_iterator_00bf7c6e(void* base, std::uint32_t stride, std::int32_t count, void* dtor) = 0;
  virtual const float* entity_pose_get_world_position_refreshed_00427eb0(void* entity) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void free_00bf6989(void* block) = 0;
  virtual void* fun_008e6cb0(void* manager, const void* name_str) = 0;
  virtual void fun_0090f5a0(void* obj, std::int32_t player_slot) = 0;
  virtual void fun_0094bff0(void* spawner, std::int32_t player_slot, std::uint32_t cfg_20, void* target_unit1, std::uint32_t cfg_1c, std::int32_t zero_arg, void* target_unit2, void* empty_str, void* callback, void* pending_pair) = 0;
  virtual void hud_powerup_manager_screen_rebuild_lists_0061b0f0(void* screen) = 0;
  virtual void invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* lua_object_argument_at_00b677e0(void* call_frame, void* out_obj, std::int32_t index) = 0;
  virtual void* lua_object_copy_construct_00b66fa0(void* dst, const void* src) = 0;
  virtual void lua_object_destruct_00b67700(void* lua_obj) = 0;
  virtual std::int32_t lua_object_get_integer_00b66290(void* lua_obj) = 0;
  virtual void lua_object_new_table_00b67930(void* call_frame, void* out_table) = 0;
  virtual void* lua_object_open_call_frame_00b679b0(void* state_owner, void* out_frame) = 0;
  virtual std::int32_t lua_object_result_count_00b66400(void* call_frame) = 0;
  virtual void lua_object_set_lstring_at_index_00b672f0(void* table_obj, std::int32_t index, const char* text) = 0;
  virtual void lua_object_vector_tidy_00441210(void* vec_proxy) = 0;
  virtual void* lua_reader_construct_004425c0(void* reader, void* root_lua_obj_by_val) = 0;
  virtual void lua_reader_destroy_stack_instance_00441a20(void* reader) = 0;
  virtual void lua_state_owner_close_00b669a0(void* state_owner) = 0;
  virtual void* lua_state_owner_construct_borrowed_00b66c00( void* state_owner, void* lua_state) = 0;
  virtual void* memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) = 0;
  virtual void* memmove_00bf87e0(void* dst, const void* src, std::uint32_t bytes) = 0;
  virtual void* native_string_assign_0041e870(void* str, const char* text) = 0;
  virtual void* native_string_assign_header_00425f40(void* dst_str, const void* src_str) = 0;
  virtual void* native_string_copy_construct_00426060(void* dst_str, const void* src_str) = 0;
  virtual void native_string_destroy_storage_0041dd20(void* str) = 0;
  virtual std::uint8_t native_string_equals_insensitive_00435c40( const void* lhs_str, const void* rhs_str) = 0;
  virtual void* native_string_from_int_004260b0(void* str, std::int32_t value) = 0;
  virtual bool native_string_not_equal_case_insensitive_00449af0( const void* lhs, const void* rhs) = 0;
  virtual void native_string_resize_0041dd40(void* str, std::uint32_t new_len, std::uint8_t preserve) = 0;
  virtual void* native_string_substring_00469840(const void* src_str, void* out_str, std::uint32_t start, std::uint32_t count) = 0;
  virtual void native_string_trim_left_set_00585eb0(void* str, const char* set) = 0;
  virtual void native_string_trim_right_set_00584110(void* str, const char* set) = 0;
  virtual void* observed_event_get_subject_vslot_04(void* event_obj) = 0;
  virtual void observer_register_pair_00694a60(void* subject, void* observer) = 0;
  virtual void observer_unregister_pair_if_registered_006956a0( void* subject, void* observer) = 0;
  virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
  virtual void power_up_manager_expire_modifier_008e8c30(void* manager, void* modifier) = 0;
  virtual void* power_up_manager_find_class_008e6cb0( void* manager, const void* class_name_str) = 0;
  virtual void power_up_manager_list_ready_items_008ea0c0( void* manager, std::int32_t player_slot, void* out_vec) = 0;
  virtual void power_up_manager_prepare_from_lua_008e9fd0(void* manager, void* lua_obj) = 0;
  virtual void power_up_manager_refresh_hud_if_shown_008e4660( void* manager) = 0;
  virtual void power_up_manager_remove_item_008e87c0(void* manager, void* item) = 0;
  virtual void power_up_manager_remove_items_bound_to_008e8a30( void* manager, void* binding) = 0;
  virtual void powerup_class_resolve_air_support_vehicle_classes_008e34f0(void* powerup_class) = 0;
  virtual void* powerup_class_resolve_local_use_target_008e45d0( void* manager, void* powerup_class) = 0;
  virtual std::uint8_t powerup_class_scope_accepts_unit_008e35f0( void* unit, const void* powerup_class) = 0;
  virtual bool powerup_class_unit_kind_mask_accepts_008e3680( void* unit, const void* powerup_class) = 0;
  virtual void* powerup_item_construct_008e6a80(void* item) = 0;
  virtual void powerup_item_init_from_message67_008e7930( void* item, const void* msg67) = 0;
  virtual void powerup_item_load_lua_008e7820(void* item, void* lua_reader) = 0;
  virtual void powerup_item_release_modifiers_008e6230(void* item) = 0;
  virtual void* powerup_modifier_construct_008e6ac0(void* modifier) = 0;
  virtual void powerup_modifier_init_from_message68_008e6b10( void* modifier, const void* msg68) = 0;
  virtual std::uint8_t powerup_modifier_matches_message68_008e3570( const void* modifier, const void* msg68) = 0;
  virtual void* powerup_pick_from_bracket_list_provisional_008ec650( void* manager, void* out_str, void* bracket_str_by_val) = 0;
  virtual void powerup_slot_record_start_cooldown_008e4b00( void* record, void* powerup_class) = 0;
  virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
  virtual const char* scene_property_get_string_value_008f0df0( const void* prop) = 0;
  virtual void* session_message67_construct_from_class_008e6e00( void* msg67, std::int32_t player_slot, const void* item, std::int32_t op, void* unit1, void* unit2) = 0;
  virtual void* session_message67_construct_from_name_008e6ed0( void* msg67, std::int32_t player_slot, const void* name_str, std::int32_t uses_left, std::int32_t op, void* unit1, void* unit2) = 0;
  virtual void session_message67_destroy_007657d0(void* msg) = 0;
  virtual void* session_message68_construct_from_modifier_008e6fb0( void* msg68, const void* modifier, std::int32_t op) = 0;
  virtual void session_message68_destroy_007658c0(void* msg68) = 0;
  virtual void* session_message69_construct_00767570(void* msg69) = 0;
  virtual void session_message69_destroy_00767610(void* msg69) = 0;
  virtual void* session_message_construct_base_0075b430( void* msg, std::uint8_t msg_type) = 0;
  virtual void session_send_message_to_secondary_first_peer_00770af0( void* session, void* msg) = 0;
  virtual void session_transport_broadcast_except_0076a9f0( void* transport_adj, const void* msg, void* except_peer) = 0;
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual void sized_storage_pool_return_block_00bd1510( void* pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
  virtual void* stl_checked_float_vector_insert_one_0045a120( void* vec, void* out_iter, void* owner, void* pos, const void* val_ptr) = 0;
  virtual void stl_checked_native_string_vector_append_00450540( void* vec, const void* str) = 0;
  virtual void* stl_inst_008e57c0(void* iter_pair) = 0;
  virtual void* stl_inst_008e57f0(void* iter_pair) = 0;
  virtual void* stl_inst_008e5ed0(void* list) = 0;
  virtual void stl_inst_008e7da0(void* list) = 0;
  virtual void* stl_inst_008ebbe0(void* next, void* prev, const void* val) = 0;
  virtual void stl_inst_008ec420(void* list, const void* val) = 0;
  virtual void* stl_list_buy_head_node_008e58c0() = 0;
  virtual void* stl_list_buy_node_008e5aa0(void* next, void* prev, const void* val) = 0;
  virtual void* stl_list_buy_node_008e8480(void* next, void* prev, const void* val) = 0;
  virtual void stl_list_clear_008e58e0(void* list) = 0;
  virtual void* stl_list_copy_construct_008ed660(void* dst_list, const void* src_list) = 0;
  virtual void* stl_list_erase_008e5610(void* list, void* out_iter, void* iter_owner, void* node) = 0;
  virtual void stl_list_push_back_008e9680(void* list, const void* val) = 0;
  virtual void* stl_map_subscript_00617030(void* map, const void* key_ptr) = 0;
  virtual void stl_powerup_class_pair_destroy_008e70b0(void* pair) = 0;
  virtual void stl_powerup_random_name_map_value_destroy_008e9700( void* pair_val) = 0;
  virtual void stl_tree_erase_range_008ea990(void* tree, void* out_iter, void* first_owner, void* first_node, void* last_owner, void* last_node) = 0;
  virtual void stl_tree_erase_range_008ec550(void* tree, void* out_iter, void* first_owner, void* first_node, void* last_owner, void* last_node) = 0;
  virtual void stl_tree_find_008e6530(void* tree, void* out_iter, const void* key_str) = 0;
  virtual void stl_tree_iter_inc_00613760(void* iter_pair) = 0;
  virtual void* stl_vector_erase_range_004685a0(void* vec, void* out_iter, void* first_owner, void* first, void* last_owner, void* last) = 0;
  virtual void* stl_vector_erase_range_00488bf0(void* vec, void* out_iter, void* first_owner, void* first, void* last_owner, void* last) = 0;
  virtual void* stl_vector_erase_range_005a0be0(void* vec, void* out_iter, void* first_owner, void* first, void* last_owner, void* last) = 0;
  virtual void* stl_vector_insert_one_00441f30(void* vec, void* out_iter, void* owner, void* pos, const void* val_ptr) = 0;
  virtual void* stl_vector_insert_one_005a6580(void* vec, void* out_iter, void* owner, void* pos, const void* val_ptr) = 0;
  virtual void stl_vector_push_back_00442190(void* vec, const void* val_ptr) = 0;
  virtual void stl_vector_push_back_0045afd0(void* vec, const void* val_ptr) = 0;
  virtual void stl_vector_push_back_005a7960(void* vec, const void* val_ptr) = 0;
  virtual void stl_vector_push_back_007265e0(void* vec, const std::uint16_t* val) = 0;
  virtual std::uint32_t stl_vector_size_0059ae20(const void* vec) = 0;
  virtual void stl_xlen_throw_008e7d00(void* list, std::uint32_t inc) = 0;
  virtual void stl_xlen_throw_008e7e10(void* list, std::uint32_t inc) = 0;
  virtual void stl_xlen_throw_008e7f10(void* list, std::uint32_t inc) = 0;
  virtual std::uint32_t strcspn_00bf9a50(const char* str, const char* control) = 0;
  virtual std::int32_t stricmp_00bf7fbf(const char* lhs, const char* rhs) = 0;
  virtual const char* strstr_00bf9440(const char* haystack, const char* needle) = 0;
  virtual void tmpl_array12_push_back_008e5670(void* arr, const void* elem12) = 0;
  virtual void tmpl_array12_resize_008e5170(void* arr, std::int32_t new_count) = 0;
  virtual std::int32_t triv_body_00b663f0(const void* lua_obj) = 0;
  virtual void* unit_get_powerup_binding_vslot_64(void* subject_unit) = 0;
  virtual std::int32_t unit_loss_counting_slot_009ffd20(void* unit) = 0;
  virtual double vcall_0000_f64(void* obj) = 0;
  virtual bool vcall_005c_is_kind_of(void* obj, std::int32_t kind_id) = 0;
  virtual void* vcall_0064_get_subobject(void* obj) = 0;
  virtual void* vcall_0140_get_unit(void* obj) = 0;
  virtual void* vcall_entity_get_powerup_binding_slot64(void* entity) = 0;
  virtual std::uint8_t vcall_entity_is_kind_of_slot5c( void* entity, std::uint32_t kind_id) = 0;
  virtual void vcall_lua_reader_read_int_slot0c( void* reader, std::uint32_t key_tag, const void* key_name, std::uint32_t out_tag, void* out_value, std::uint32_t default_tag, std::uint32_t default_value) = 0;
  virtual void vcall_lua_reader_read_string_slot10(void* reader, std::uint32_t key_tag, const void* key_name, std::uint32_t out_tag, void* out_string) = 0;
  virtual void* vcall_observer_event_get_subject_slot04(void* event) = 0;
  virtual float vector3f_length_squared_00427e30(const float* v) = 0;
  virtual float* vector3f_subtract_to_00427c90(const float* lhs, float* out, const float* rhs) = 0;
  virtual void warning_manager_report_powerup_009789a0( void* warning_mgr, std::int32_t player_slot, void* item, const void* sound_str) = 0;
};

// Part 1 functions
void* powerup_class_resolve_local_use_target_008e45d0(
    Seg58Host& host, const void* powerup_class);
void power_up_manager_refresh_hud_if_shown_008e4660(Seg58Host& host);
bool powerup_target_applies_to_unit_008e4680(
    Seg58Host& host, const void* target_record, void* unit);
std::uint32_t powerup_slot_record_is_cooldown_expired_008e4b40(
    Seg58Host& host, const void* record);
float powerup_slot_record_get_ready_time_008e4b60(Seg58Host& host, const void* record);
float powerup_slot_record_get_end_time_008e4b80(Seg58Host& host, const void* record);
void tmpl_array12_reserve_008e4e10(Seg58Host& host, void* array, std::int32_t new_cap);
void tmpl_array12_resize_008e5170(Seg58Host& host, void* array, std::int32_t new_size);
void tmpl_array12_push_back_008e5670(Seg58Host& host, void* array, const void* elem);
void tmpl_array12_destroy_008e5f60(Seg58Host& host, void* array);
void cg_adjustor_thunk_008e6220(Seg58Host& host, void* obj);
void powerup_item_release_modifiers_008e6230(Seg58Host& host, void* item);
bool power_up_applies_to_unit_008e62a0(Seg58Host& host, void* entry, void* unit);
void native_string_release_storage_008e6400(Seg58Host& host, void* modifier);
void native_string_insert_at_008e64a0(
    Seg58Host& host, void* dst_str, const void* src_str, std::uint32_t pos);
void* powerup_item_construct_008e6a80(Seg58Host& host, void* item);
void* powerup_modifier_construct_008e6ac0(Seg58Host& host, void* modifier);
void powerup_modifier_init_from_message68_008e6b10(
    Seg58Host& host, void* modifier, const void* msg68);
void* fun_008e6cb0(Seg58Host& host, void* manager, const void* name_str);
void power_up_manager_on_powerup_binding_message_008e6d30(
    Seg58Host& host, void* manager, const void* msg);

// Part 2 function declarations (008e6e00 .. 008ea0c0)
void* session_message67_construct_from_class_008e6e00(
    Seg58Host& host, void* self, std::uint32_t player_slot, const void* item,
    std::uint32_t op, const void* unit_a, const void* unit_b);

void* session_message67_construct_from_name_008e6ed0(
    Seg58Host& host, void* self, std::uint32_t player_slot,
    const void* name_str, std::uint32_t use_limit, std::uint32_t op,
    const void* unit_a, const void* unit_b);

void* session_message68_construct_from_modifier_008e6fb0(Seg58Host& host,
                                                         void* self,
                                                         const void* modifier,
                                                         std::uint32_t op);

void* cg_scalar_deleting_dtor_008e7240(Seg58Host& host, void* self,
                                       std::uint8_t flags);

void* cg_scalar_deleting_dtor_008e7270(Seg58Host& host, void* self,
                                       std::uint8_t flags);

void* powerup_modifier_copy_construct_008e72b0(Seg58Host& host, void* self,
                                               const void* src);

void powerup_item_load_lua_008e7820(Seg58Host& host, void* self, void* reader);

void powerup_item_init_from_message67_008e7930(Seg58Host& host, void* self,
                                               const void* msg67);

void* cg_scalar_deleting_dtor_008e7c10(Seg58Host& host, void* self,
                                       std::uint8_t flags);

void* stl_list_erase_modifier_008e8370(Seg58Host& host, void* self,
                                       void* out_iter, void* iter_owner,
                                       void* node);

void power_up_manager_remove_item_008e87c0(Seg58Host& host, void* self,
                                           void* item);

void power_up_manager_remove_items_bound_to_008e8a30(Seg58Host& host,
                                                     void* self, void* binding);

void power_up_manager_expire_modifier_008e8c30(Seg58Host& host, void* self,
                                               void* modifier);

void command_building_powerup_binding_release_008e94d0(Seg58Host& host,
                                                       void* self);

void power_up_manager_on_observed_destroyed_008e9510(Seg58Host& host,
                                                     void* self, void* event);

void power_up_manager_on_observed_released_008e9630(Seg58Host& host, void* self,
                                                    void* event);

void command_building_powerup_binding_destruct_008e9a70(Seg58Host& host,
                                                        void* self);

void power_up_manager_collect_active_008e9af0(
    Seg58Host& host, void* self, void* out_vec_type3, void* out_vec_items,
    void* out_vec_counts, void* out_vec_end_times, void* out_vec_start_times,
    std::uint8_t include_all_players, std::uint8_t include_zero_limit);

void power_up_manager_prepare_from_lua_008e9fd0(Seg58Host& host,
                                                const void* lua_table);

void power_up_manager_list_ready_items_008ea0c0(Seg58Host& host, void* self,
                                                std::int32_t slot,
                                                void* out_vec);

// Part 3 functions (008ea250 .. 008ee410)
void power_up_manager_insert_modifier_008ea250(
    Seg58Host& host, void* manager, std::int32_t category, float multiplier,
    void* powerup_class, void* item, std::int32_t player_slot,
    void* target_unit);

void power_up_manager_on_powerup_effect_message_008ea4e0(Seg58Host& host,
                                                         void* manager,
                                                         void* msg68);

std::int32_t lua_binding_prepare_powerup_008ea610(Seg58Host& host,
                                                  void* lua_state);

void* cg_scalar_deleting_dtor_008eaad0(Seg58Host& host, void* self,
                                       std::uint8_t flags);

void power_up_manager_build_available_names_table_008eab00(
    Seg58Host& host, void* manager, std::int32_t player_slot, void* call_frame);

void power_up_manager_on_air_support_flight_spawned_008eabb0(
    Seg58Host& host, std::int32_t player_slot, void* flight_unit,
    void* pending_pair);

void power_up_manager_bind_pending_flights_008eac80(Seg58Host& host,
                                                    void* manager);

void power_up_manager_use_item_008eada0(Seg58Host& host, void* manager,
                                        void* item, std::int32_t player_slot,
                                        void* target_unit);

std::int32_t lua_binding_get_available_powerups_008eb350(Seg58Host& host,
                                                         void* lua_state);

void power_up_manager_use_item_local_008eb6e0(Seg58Host& host, void* manager,
                                              void* item);

void power_up_manager_broadcast_command_building_bindings_008eb710(
    Seg58Host& host);

void command_building_powerup_binding_grant_to_owner_008ed4b0(Seg58Host& host,
                                                              void* manager,
                                                              void* binding);

void power_up_manager_on_observed_event_008ed620(Seg58Host& host, void* manager,
                                                 void* event_obj,
                                                 std::int32_t event_code,
                                                 void* unused_arg3);

void command_building_powerup_binding_init_from_properties_008ed720(
    Seg58Host& host, void* binding, void* prop_container);

std::uint8_t power_up_manager_local_player_has_usable_item_008ed9c0(
    Seg58Host& host, void* manager);

void cg_vector_deleting_dtor_008edad0(Seg58Host& host, void* manager);

void* cg_scalar_deleting_dtor_008edda0(Seg58Host& host, void* self,
                                       std::uint8_t flags);

void power_up_manager_grant_item_008eddc0(Seg58Host& host, void* manager,
                                          std::int32_t player_slot,
                                          void* lua_obj);

void power_up_manager_on_powerup_message_008ee020(Seg58Host& host,
                                                  void* manager, void* msg67);

std::int32_t lua_binding_add_powerup_008ee410(Seg58Host& host, void* lua_state);

}  // namespace bsp::powerup_seg58_argon
