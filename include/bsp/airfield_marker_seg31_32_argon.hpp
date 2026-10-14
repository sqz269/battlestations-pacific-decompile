#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::airfield_marker_seg31_32_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct Seg31_32Host {
  virtual ~Seg31_32Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // External callees (verified from callee bodies and RET immediates)
  virtual void* matrix_copy_4x4_x87_004134f0(void* dst, const void* src) = 0;
  virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual void native_string_resize_0041dd40(
      void* self, std::uint32_t new_length, std::uint8_t preserve) = 0;
  virtual void* native_string_concat_004261a0(
      const void* lhs, void* dst, const void* rhs) = 0;
  virtual void bit_cursor_read_unsigned_dword_00428d10(
      void* cursor, std::uint32_t* out_value, std::uint32_t bit_count) = 0;
  virtual void bit_cursor_read_word_bits_00428e30(
      void* cursor, std::uint16_t* out_value, std::uint32_t bit_count) = 0;
  virtual void bit_cursor_write_unsigned_dword_bits_00429070(
      void* cursor, std::uint32_t value, std::uint32_t bit_count) = 0;
  virtual void bit_cursor_write_word_bits_00429120(
      void* cursor, std::uint16_t value, std::uint32_t bit_count) = 0;
  virtual void bit_cursor_read_numeric_float_004293f0(
      void* cursor,
      float* out_value,
      std::uint8_t zero_flag,
      std::uint8_t signed_flag,
      float scale,
      std::int32_t bit_count) = 0;
  virtual void bit_cursor_write_numeric_float_004295c0(
      void* cursor,
      float value,
      std::uint8_t zero_flag,
      std::uint8_t signed_flag,
      float scale,
      std::int32_t bit_count) = 0;
  virtual void* matrix_set_rotation_z_negated_0042b4d0(
      void* dst_matrix, float angle) = 0;
  virtual void pointer_vector_push_back_004323d0(
      void* vec, void* const* element_ptr) = 0;
  virtual void* native_string_substring_00469840(
      const void* src,
      void* out_string,
      std::int32_t start,
      std::int32_t count) = 0;
  virtual std::uint32_t stl_vector_size_00519d50(const void* vec) = 0;
  virtual void* stl_vector_insert_one_0052eb90(
      void* vec,
      void* out_iter,
      void* pos_owner,
      void* pos_ptr,
      const void* value_ptr) = 0;
  virtual void credits_screen_push_widget_0052ed20(
      void* vec, void* const* widget_ptr) = 0;
  virtual void callback_owner_destroy_00695870(void* self) = 0;
  virtual void lua_machine_run_string_006b8ad0(
      void* lua_machine,
      const char* chunk,
      void* result_sink,
      void* error_out,
      std::int32_t mode) = 0;
  virtual void stl_vector_resize_006ca010(
      void* vec,
      std::uint32_t count,
      std::uint32_t fill0,
      std::uint32_t fill1,
      std::uint32_t fill2) = 0;
  virtual void stl_vector_resize_006ca0f0(
      void* vec,
      std::uint32_t count,
      std::uint32_t fill0,
      std::uint32_t fill1,
      std::uint32_t fill2,
      std::uint32_t fill3,
      std::uint32_t fill4) = 0;
  virtual void air_ops_stock_snapshot_capture_006cc800(
      void* snapshot, const void* source_stock) = 0;
  virtual void* stl_vector_erase_range_006d3a50(
      void* vec,
      void* out_iter,
      void* first_owner,
      void* first_ptr,
      void* last_owner,
      void* last_ptr) = 0;
  virtual void stl_vector_push_back_006d4e40(
      void* vec, const void* element_ptr) = 0;
  virtual void stl_tree_iter_inc_006d7d20(void* iter_pair) = 0;
  virtual void* stl_tree_find_006d8870(
      void* tree, void* out_iter_pair, const void* key_ptr) = 0;
  virtual void* stl_tree_find_006d8d10(
      void* tree, void* out_iter_pair, const void* key_ptr) = 0;
  virtual void* stl_tree_erase_006d9bf0(
      void* tree, void* out_iter_pair, void* iter_owner, void* iter_node) = 0;
  virtual void* stl_tree_erase_range_006da090(
      void* tree,
      void* out_iter_pair,
      void* first_owner,
      void* first_node,
      void* last_owner,
      void* last_node) = 0;
  virtual void* stl_tree_insert_006da5b0(
      void* tree, void* out_result, const void* key_ptr) = 0;
  virtual void* stl_map_subscript_006dbd40(void* map, const void* key_name) = 0;
  virtual void* stl_tree_erase_006dd1f0(
      void* tree, void* out_iter_pair, void* iter_owner, void* iter_node) = 0;
  virtual void* stl_tree_erase_range_006ddf00(
      void* tree,
      void* out_iter_pair,
      void* first_owner,
      void* first_node,
      void* last_owner,
      void* last_node) = 0;
  virtual void* stl_map_subscript_006de150(void* map, const void* key_vec3) = 0;
  virtual void* stl_tree_erase_range_006de300(
      void* tree,
      void* out_iter_pair,
      void* first_owner,
      void* first_node,
      void* last_owner,
      void* last_node) = 0;
  virtual void net_entity_create_message_write_00765000(
      void* self, void* cursor) = 0;
  virtual void net_entity_create_message_destruct_007668e0(void* self) = 0;
  virtual void* net_entity_create_message_construct_00766fc0(
      void* self,
      std::uint32_t msg_type,
      std::uint32_t class_id,
      void* entity,
      std::uint8_t flag) = 0;
  virtual void net_entity_create_message_read_00767e30(
      void* self, void* stream) = 0;
  virtual void* matrix_build_rotation_x_00b64640(
      void* dst_matrix, const float* angle) = 0;
  virtual void* matrix_build_rotation_y_00b646e0(
      void* dst_matrix, const float* angle) = 0;
  virtual void node_propagate_root_registration_00b6d890(
      void* node, void* root) = 0;
  virtual void sized_storage_pool_return_block_00bd1510(
      void* pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
  virtual float random_uniform_float_range_00bd2f10(
      std::uint32_t stream_id, float lo, float hi) = 0;
  virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
  virtual void free_00bf65ac(void* block) = 0;
  virtual void invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
  virtual void free_00bf6989(void* block) = 0;

  // Virtual table slot calls
  virtual void vcall_scalar_deleting_dtor_vslot00(
      void* obj, std::uint32_t flags) = 0;
  virtual void vcall_scalar_deleting_dtor_vslot04(
      void* obj, std::uint32_t flags) = 0;
  virtual void vcall_marker_instance_vslot08(void* self) = 0;
  virtual void* vcall_resource_instantiate_vslot08(
      void* resource, std::int32_t arg0, float scale) = 0;
  virtual std::uint16_t vcall_path_get_id_vslot28(void* path_obj) = 0;
  virtual const void* vcall_gui_element_get_name_vslot28(void* element) = 0;
  virtual void vcall_node_set_transform_vslot34(
      void* node, const void* matrix_4x4) = 0;
  virtual void vcall_gui_element_set_color_vslot50(
      void* element, const float* rgba4) = 0;
  virtual void vcall_gui_element_get_color_vslot54(
      void* element, float* out_rgba4) = 0;
};

// Reconstructed functions (39 functions in 006d4370 through 006deac0)
void air_ops_stock_snapshot_destruct_006d4370(Seg31_32Host& host, void* self);
void session_message_airfield_create_destruct_006d4420(
    Seg31_32Host& host, void* self);
void* session_message_airfield_create_stock_snapshot_006d44b0(void* self);
std::uint32_t net_entity_create_message_vslot18_return_zero_006d44c0(
    void* self);
void air_ops_stock_snapshot_write_006d44f0(
    Seg31_32Host& host, void* self, void* cursor);
void session_message_airfield_create_write_006d47a0(
    Seg31_32Host& host, void* self, void* cursor);
void* air_ops_stock_snapshot_construct_006d4c20(
    Seg31_32Host& host, void* self);
void* session_message_airfield_create_construct_006d4c70(
    Seg31_32Host& host,
    void* self,
    void* airfield,
    float field_140,
    float field_144);
void air_field_append_intact_hangars_as_sub_entities_006d4dd0(
    Seg31_32Host& host, void* self, void* out_pointer_vec);
void air_ops_stock_snapshot_read_006d4eb0(
    Seg31_32Host& host, void* self, void* stream);
void session_message_airfield_create_read_006d5130(
    Seg31_32Host& host, void* self, void* stream);
void marker_group_vector_clear_006d6010(
    Seg31_32Host& host, void* self, void* out_vec);
bool vec3_lexicographic_less_006d6170(const float* lhs, const float* rhs);
void marker_instance_tick_006d61f0(
    Seg31_32Host& host, void* self, float delta);
void triv_body_006d6200();
void tmpl_owned_ptr_delete_reset_006d7100(
    Seg31_32Host& host, void** self_slot);
void owned_object_ptr_delete_006d7120(Seg31_32Host& host, void** self_slot);
void tmpl_owned_ptr_delete_reset_006d7140(
    Seg31_32Host& host, void** self_slot);
void marker_classes_reset_lua_table_006d74b0(Seg31_32Host& host);
void marker_instance_destruct_006d74d0(Seg31_32Host& host, void* self);
void marker_instance_update_pose_006d7fc0(Seg31_32Host& host, void* self);
void* marker_instance_construct_006d8750(
    Seg31_32Host& host,
    void* self,
    void* descriptor,
    void* owner,
    float x,
    float y,
    float z);
void* marker_class_create_instance_006d8c70(
    Seg31_32Host& host,
    void* self_descriptor,
    void* owner,
    float x,
    float y,
    float z);
void native_string_destroy_storage_006d8e50(
    Seg31_32Host& host, void* self_string);
void native_string_destroy_storage_006d8e70(
    Seg31_32Host& host, void* self_string);
void* named_value_construct_006d9140(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    const std::uint32_t* value_ptr);
void* named_value_construct_006d9190(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    const std::uint32_t* value_ptr);
void* named_value_copy_construct_006d91e0(
    Seg31_32Host& host, void* self, const void* other);
void* named_value_copy_construct_006d9230(
    Seg31_32Host& host, void* self, const void* other);
void* gui_element_build_path_006d9300(
    Seg31_32Host& host, void* out_string, void* element);
void marker_manager_add_gui_highlight_006dae20(
    Seg31_32Host& host,
    void* self,
    void* gui_element,
    std::uint8_t recursive_flag,
    std::uint8_t color_mode_flag);
void gui_element_collect_descendants_006db3f0(
    Seg31_32Host& host, void* root_element, void* out_vec);
void gui_element_apply_highlight_pulse_006dbac0(
    Seg31_32Host& host,
    void* element,
    std::uint8_t enable,
    std::uint8_t recursive_flag,
    std::uint8_t color_mode_flag,
    float scale);
void marker_manager_remove_gui_highlight_006dc5a0(
    Seg31_32Host& host, void* self, void* gui_element);
void marker_manager_forget_entity_006dda40(
    Seg31_32Host& host, void* self, std::uint32_t entity_id);
void marker_manager_bind_unit_marker_stub_006de3d0(
    void* self, std::uint32_t arg0, std::uint32_t arg1);
void triv_body_006de3e0(
    void* self, std::uint32_t arg0, float x, float y, float z);
void marker_manager_clear_position_marker_006de4c0(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    float x,
    float y,
    float z);
void marker_manager_destruct_006deac0(Seg31_32Host& host, void* self);

}  // namespace bsp::airfield_marker_seg31_32_argon
