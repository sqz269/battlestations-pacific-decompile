#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::objectives_seg57_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct NativeStringView {
  std::uint32_t length{0};
  void* data{nullptr};
};

struct Seg57Host {
  virtual ~Seg57Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // External direct callees (verified from callee prologues/RETs)
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual void native_string_destroy_storage_0041dd20(void* str) = 0;
  virtual void native_string_resize_0041dd40(
      void* str, std::uint32_t new_len, std::uint8_t preserve) = 0;
  virtual void* native_string_assign_0041e870(void* str, const char* text) = 0;
  virtual void* native_string_concat_004261a0(
      const void* lhs, void* dst, const void* rhs) = 0;
  virtual void* global_config_get_singleton_00432650() = 0;
  virtual std::uint8_t native_string_not_equal_case_insensitive_00449af0(
      const void* lhs, const void* rhs) = 0;
  virtual void scene_entity_register_hidden_stock_unit_0046be90(
      void* manager, std::int32_t unit_type_id) = 0;
  virtual void* sound_request_queue_get_singleton_004c1b90() = 0;
  virtual void observer_register_pair_00694a60(
      void* subject, void* callback_owner) = 0;
  virtual std::uint8_t observer_is_pair_registered_00694af0(
      const void* subject, const void* callback_owner) = 0;
  virtual void observer_unregister_pair_006952a0(
      void* subject, void* callback_owner) = 0;
  virtual void callback_owner_destroy_00695870(void* self) = 0;
  virtual void marker_group_vector_clear_006d6010(
      void* entity, void* out_vec) = 0;
  virtual void marker_manager_drop_unit_marker_006de3f0(
      void* marker_mgr, const void* marker_name, void* unit) = 0;
  virtual void marker_manager_clear_position_marker_006de4c0(
      void* marker_mgr, const void* marker_name, float x, float y, float z) = 0;
  virtual void flag_cloth_lock_vertex_buffer_0070a710(void* flag_model) = 0;
  virtual void flag_model_update_wave_0070aa70(void* flag_model, float dt) = 0;
  virtual void hud_announce_objective_text_00734870(
      void* hud_mgr, const void* text_str, const void* sub_str, std::int32_t mode) = 0;
  virtual void* session_message_construct_base_0075b430(
      void* self, std::uint8_t msg_type) = 0;
  virtual void session_broadcast_except_thunk_0076a9f0(
      void* session, void* msg, void* excluded_peer) = 0;
  virtual void session_send_message_to_nonlocal_peer_00770b50(
      void* session, void* peer, void* msg) = 0;
  virtual void shipyard_collect_live_hangar_objects_00848410(
      void* shipyard, void* out_vec) = 0;
  virtual void stl_tree_iter_inc_008da530(void* iter_pair) = 0;
  virtual void* stl_pointer_list_allocate_sentinel_008db4f0() = 0;
  virtual void* stl_pointer_list_allocate_sentinel_008db560() = 0;
  virtual void stl_pointer_list_destroy_008db580(void* list) = 0;
  virtual void* stl_list_buy_node4_008db650(
      void* list, void* next_node, void* prev_node, const void* val_ptr) = 0;
  virtual void* stl_list_buy_node4_008db6a0(
      void* list, void* next_node, void* prev_node, const void* val_ptr) = 0;
  virtual void* stl_tree_buy_head_node_008db7b0() = 0;
  virtual void stl_tree_distance_008db950(
      void* first_tree,
      void* first_node,
      void* last_tree,
      void* last_node,
      std::uint32_t* out_count,
      std::uint32_t unused_flag) = 0;
  virtual void* objective_entry_select_entity_or_pos_key_008db990(
      void* entry, void* archive_reader) = 0;
  virtual void stl_tree_equal_range_008dbba0(
      void* tree, void* out_range, const void* key_ptr) = 0;
  virtual void map_ping_entry_describe_properties_008dc210(
      void* entry, void* archive_writer) = 0;
  virtual void objective_erase_unit_entries_008dc3e0(
      void* objective, void* unit) = 0;
  virtual std::uint8_t objective_contains_unit_008dc4a0(
      const void* objective, const void* unit) = 0;
  virtual void objective_erase_position_entries_008dc500(
      void* objective, const float* pos3) = 0;
  virtual void session_message56_destroy_008dc7a0(void* msg) = 0;
  virtual void session_message57_destroy_008dc980(void* msg) = 0;
  virtual void session_message58_destroy_008dcb40(void* msg) = 0;
  virtual void session_message60_destroy_008dce40(void* msg) = 0;
  virtual void session_message62_destroy_008dd120(void* msg) = 0;
  virtual void objective_set_destroy_objectives_008dd1c0(void* self) = 0;
  virtual void stl_xlen_throw_008de200(void* list, std::uint32_t inc) = 0;
  virtual void stl_list_checked_size_add_008de2a0(
      void* list, std::uint32_t inc) = 0;
  virtual void stl_receive28_range_destroy_008de660(
      void* first, void* last, void* owner1, void* owner2) = 0;
  virtual void stl_set_ptr_insert_008de6a0(
      void* tree, void* out_pair, const void* key_ptr) = 0;
  virtual void stl_tree_erase_range_008de7e0(
      void* tree,
      void* out_iter,
      void* first_tree,
      void* first_node,
      void* last_tree,
      void* last_node) = 0;
  virtual void objective_set_add_unit_to_named_objective_008df2b0(
      void* self, const void* name_str, void* unit) = 0;
  virtual void objective_set_add_position_to_named_objective_008df5d0(
      void* self, const void* name_str, float x, float y, float z) = 0;
  virtual void stl_list_assign_range_008df8a0(
      void* dst_list,
      void* first_list,
      void* first_node,
      void* last_list,
      void* last_node,
      void* owner) = 0;
  virtual void objective_clear_unit_markers_and_empty_008dfe50(
      void* self, void* objective) = 0;
  virtual void session_message55_destroy_008e1480(void* msg) = 0;
  virtual void objective_set_set_completed_008e20d0(
      void* self,
      const void* id_str,
      const void* comment_str,
      std::uint8_t silent_flag) = 0;
  virtual void* stl_list_buy_head_node_008e28f0() = 0;
  virtual void* stl_list_buy_node_008e2b40(
      void* list, void* next_node, void* prev_node, const void* val_ptr) = 0;
  virtual void stl_xlen_throw_008e2c80(void* list, std::uint32_t inc) = 0;
  virtual void stl_list_push_back_008e2d60(
      void* list, const void* val_ptr) = 0;
  virtual void vehicle_class_mark_party_required_0095ba60(
      std::int32_t type_id, std::int32_t party) = 0;
  virtual void* vehicle_class_get_or_create_00964790(
      std::int32_t type_id, std::uint8_t read_race) = 0;
  virtual std::uint8_t entity_is_static_installation_009fe0b0(
      const void* entity) = 0;
  virtual std::int32_t sound_manager_find_class_by_name_00a7acf0(
      void* sound_mgr, const void* name_str) = 0;
  virtual std::int32_t sound_manager_find_type_by_name_00a7b0a0(
      void* sound_mgr, const void* name_str) = 0;
  virtual void* sound_manager_create_tracked_sound_00a7e490(
      void* sound_mgr,
      void** out_tracked,
      void* sample,
      std::int32_t class_index,
      std::int32_t type_index,
      std::uint8_t flag) = 0;
  virtual void sized_storage_pool_return_block_00bd1510(
      void* pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void invalid_parameter_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
  virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;

  // Win32 import slots used in this segment
  virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* addend) = 0;
  virtual void raise_exception_00ce2280(
      std::uint32_t code,
      std::uint32_t flags,
      std::uint32_t arg_count,
      const std::uintptr_t* args) = 0;

  // Virtual table dispatch hooks
  virtual void vcall_tracked_sound_destroy_00(void* tracked_sound) = 0;
  virtual void vcall_archive_enter_block_04(
      void* archive, std::uint32_t key_kind, const void* key_or_index) = 0;
  virtual void vcall_archive_leave_block_08(void* archive) = 0;
  virtual void vcall_archive_write_property_0c(
      void* archive,
      std::uint32_t key_kind,
      std::int32_t key_index,
      std::uint32_t val_kind,
      const void* val_ptr) = 0;
  virtual void vcall_archive_read_property_10(
      void* archive,
      std::uint32_t key_kind,
      const void* key_or_index,
      std::uint32_t val_kind,
      void* val_target) = 0;
  virtual std::uint8_t vcall_archive_has_entry_14(
      void* archive, std::uint32_t key_kind, std::int32_t key_index) = 0;
  virtual std::uint8_t vcall_entity_is_kind_of_5c(
      const void* entity, std::int32_t kind_id) = 0;
  virtual std::uint8_t vcall_session_message_is_type_0c(
      const void* msg, std::int32_t msg_type) = 0;
};

// 51 reconstructed Segment 57 functions (008dd380 through 008e3710)
void native_string_release_storage_008dd380(Seg57Host& host, void* self);
void objective_sounds_play_throttled_008dd460(
    Seg57Host& host, void* self, std::int32_t sample_index);
void* objective_construct_008dd5c0(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* text_str,
    std::int32_t kind,
    std::uint8_t quiet_flag);
void* session_message56_construct_obj_add_008dd6a0(
    Seg57Host& host, void* self, const void* objective);
void* session_message57_construct_obj_add_entity_008dd780(
    Seg57Host& host, void* self, const void* name_str, const void* entity);
void* session_message58_construct_obj_remove_entity_008dd820(
    Seg57Host& host, void* self, const void* name_str, const void* entity);
void* session_message59_construct_obj_add_position_008dd8c0(
    Seg57Host& host, void* self, const void* name_str, const float* pos3);
void* session_message60_construct_obj_remove_position_008dd970(
    Seg57Host& host, void* self, const void* name_str, const float* pos3);
void* session_message61_construct_obj_completed_008dda20(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* comment_str,
    std::uint8_t silent_flag);
void* session_message62_construct_obj_failed_008ddb00(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* comment_str,
    std::uint8_t silent_flag);
void objective_sync_describe_objectives_008ddbe0(
    Seg57Host& host, void* self, void* archive_writer);
void objective_set_remove_position_from_named_objective_008ddd30(
    Seg57Host& host,
    void* self,
    const void* name_str,
    float x,
    float y,
    float z);
void objective_sync_apply_obj_remove_position_008ddf40(
    Seg57Host& host, void* self, const void* msg);
std::uint32_t objective_set_erase_named_entries_008deb40(
    Seg57Host& host, void* tree_self, const void* key_ptr);
void receive28_range_destroy_forward_008debc0(
    Seg57Host& host, void* self, void* first, void* last);
void* session_message55_entry_read_properties_008dec10(
    Seg57Host& host, void* self, void* archive_reader);
void objective_sync_destruct_008def60(Seg57Host& host, void* self);
void objective_sync_read_objectives_008df000(
    Seg57Host& host, void* self, void* archive_reader);
void objective_sync_detach_observers_008df1e0(
    Seg57Host& host, void* self, void* entity);
void objective_set_remove_unit_from_named_objective_008df400(
    Seg57Host& host, void* self, const void* name_str, void* entity);
void objective_sync_apply_obj_add_position_008df800(
    Seg57Host& host, void* self, const void* msg);
void receive28_vector_tidy_008df860(Seg57Host& host, void* self);
void* objective_set_construct_008df900(
    Seg57Host& host, void* self, std::int32_t slot_index);
void objective_set_add_unit_expanding_groups_008df9b0(
    Seg57Host& host, void* self, const void* name_str, void* unit);
void objective_set_remove_unit_expanding_groups_008dfc00(
    Seg57Host& host, void* self, const void* name_str, void* unit);
void objective_sync_apply_obj_add_entity_008e0060(
    Seg57Host& host, void* self, const void* msg);
void objective_sync_apply_obj_remove_entity_008e00b0(
    Seg57Host& host, void* self, const void* msg);
void receive28_vector_tidy_008e0100(Seg57Host& host, void* self);
void* session_message55_construct_obj_create_snapshot_008e1380(
    Seg57Host& host, void* self, const void* objective_set);
void objective_set_send_create_snapshot_to_peer_008e1720(
    Seg57Host& host, void* self, const void* peer_owner);
void objective_set_announce_added_to_local_party_008e1b90(
    Seg57Host& host, void* self, void* objective);
void objective_set_announce_state_change_008e1d30(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* text_str,
    std::uint8_t completed_flag,
    std::uint8_t silent_flag);
void objective_sync_apply_obj_create_008e1e60(
    Seg57Host& host, void* self, void* msg);
void objective_set_add_008e1f80(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* text_str,
    std::int32_t kind,
    std::uint8_t quiet_flag);
void objective_set_set_failed_008e2200(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* comment_str,
    std::uint8_t silent_flag);
void objective_sync_apply_obj_completed_008e2350(
    Seg57Host& host, void* self, const void* msg);
void objective_sync_apply_obj_failed_008e2370(
    Seg57Host& host, void* self, const void* msg);
void objective_sync_dispatch_received_008e2390(Seg57Host& host, void* msg);
void thread_set_debug_name_008e24f0(
    Seg57Host& host, std::uint32_t thread_id, const char* name);
void flag_cloth_process_frame_008e2560(Seg57Host& host, float dt);
void map_ping_reset_cooldown_008e25f0(Seg57Host& host);
void* map_ping_list_construct_008e2b90(Seg57Host& host, void* self);
void map_ping_list_prune_expired_008e2bf0(Seg57Host& host, void* self);
void map_ping_handle_received_008e2da0(Seg57Host& host, void* msg);
void map_ping_add_local_008e2e50(Seg57Host& host, float x, float y);
void powerup_class_resolve_air_support_vehicle_classes_008e34f0(
    Seg57Host& host, void* powerup_class);
bool powerup_modifier_matches_message68_008e3570(
    const void* modifier, const void* msg68);
void* command_building_powerup_binding_construct_008e35d0(
    Seg57Host& host, void* self, void* owner);
bool powerup_class_scope_accepts_unit_008e35f0(
    Seg57Host& host, void* unit, const void* powerup_class);
bool powerup_class_unit_kind_mask_accepts_008e3680(
    Seg57Host& host, void* unit, const void* powerup_class);
void powerup_slot_record_reset_008e3710(void* self);

}  // namespace bsp::objectives_seg57_argon
