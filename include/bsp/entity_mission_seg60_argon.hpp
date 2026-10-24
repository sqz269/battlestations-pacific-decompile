#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::entity_mission_seg60_argon {

// nB-high hierarchy leaves. Actual Win32 links are child+48h / sibling+44h.
struct EntityTreeWalkCalls {
  virtual ~EntityTreeWalkCalls() = default;
  virtual bool visit_slote4(void* entity, std::uint32_t a, std::uint32_t b) = 0;
  virtual void set_slotdc(void* entity, float value) = 0;
  virtual bool is_kind_slot5c(void* entity, int kind) = 0;
  virtual const char* type_name_00e0cd80(std::uint32_t index) = 0;
  virtual void format_0040cfc0(char* out1024, const char* format, unsigned id,
                              const char* type_name, std::uint32_t child_count) = 0;
};
// 009040D0 ECX opaque visitor, four stack words node,a,b,excluded; RET10h.
// The same exclusion pointer is forwarded recursively (pseudocode got this wrong).
bool entity_visit_tree_009040d0(void* visitor, void* entity, std::uint32_t a, std::uint32_t b,
                              void* excluded, EntityTreeWalkCalls&);
// 00922EE0 ECX entity, stack float; RET4. Parent callback before reading child link.
void entity_set_float_recursive_00922ee0(void*, float value, EntityTreeWalkCalls&);
// 00923AF0 ECX entity, RET. Format into a discarded local 400h buffer only
// for kind1 nodes. Descend all siblings iff at least one child is kind1.
void entity_format_tree_00923af0(void*, EntityTreeWalkCalls&);
// 008FB780 MOV AL,1 / RET. Unknown predicate semantics are intentionally unnamed.
bool entity_predicate_true_008fb780() noexcept;

struct Raw {
  std::uint8_t* p;

  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }

  void*& ptr(std::size_t off) const noexcept {
    return at<void*>(off);
  }
};

struct Seg60Host {
  virtual ~Seg60Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // Win32 CRITICAL_SECTION import thunks ([0x00ce2218], [0x00ce2210])
  virtual void enter_critical_section_00ce2218(void* cs) = 0;
  virtual void leave_critical_section_00ce2210(void* cs) = 0;

  // Virtual table dispatch hooks
  virtual void vcall_scalar_deleting_dtor_slot00(void* obj, std::int32_t flags) = 0;
  virtual const char* vcall_command_name_slot04(void* command) = 0;
  virtual void vcall_entity_set_gui_name_slot0c(
      void* entity,
      void* native_str,
      std::int32_t flag
  ) = 0;
  virtual const char* vcall_entity_get_name_slot10(void* entity) = 0;
  virtual void vcall_lua_reader_read_field_slot10(
      void* reader,
      std::int32_t key_kind,
      std::int32_t key_val,
      std::int32_t field_type,
      void* dest
  ) = 0;
  virtual std::int32_t vcall_unit_class_kind_slot1c(void* unit_class) = 0;
  virtual void vcall_entity_slot2c(
      void* entity,
      std::uint32_t party_info,
      std::uint32_t race,
      std::int32_t* out_flag
  ) = 0;
  virtual bool vcall_is_kind_of_slot5c(void* entity, std::int32_t kind) = 0;
  virtual void vcall_place_in_world_slot98(
      void* entity,
      void* hierarchy_parent,
      void* world_node,
      const float* matrix16
  ) = 0;
  virtual void vcall_set_invincible_slotf4(void* entity, float value) = 0;
  virtual void* vcall_get_weapon_director_slot114(void* entity) = 0;
  virtual void vcall_on_enter_world_slot130(void* entity) = 0;
  virtual void vcall_on_leave_world_slot134(void* entity) = 0;
  virtual void vcall_set_role_availability_slot148(
      void* entity,
      std::uint32_t mask,
      std::int32_t state
  ) = 0;

  // External direct/tail callees
  virtual float* matrix_copy_4x4_x87_004134f0(void* dst, const void* src) = 0;
  virtual float* vector3f_transform_affine_point_004142e0(
      const void* src_xyz,
      void* dst_xyz,
      const void* matrix
  ) = 0;
  virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
  virtual void* singleton_lifetime_get_manager_00415350() = 0;
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual void native_string_resize_0041dd40(
      void* native_str,
      std::uint32_t new_len,
      std::uint8_t preserve
  ) = 0;
  virtual void* native_string_assign_0041e870(void* native_str, const char* text) = 0;
  virtual void* game_settings_get_singleton_00424c40() = 0;
  virtual float* entity_pose_get_world_position_refreshed_00427eb0(void* entity) = 0;
  virtual void geometry_normalize_vector_with_floor_0042b260(float* vec3) = 0;
  virtual void lua_object_vector_tidy_00441210(void* vec_proxy_base) = 0;
  virtual void lua_reader_destroy_stack_instance_00441a20(void* reader) = 0;
  virtual void* lua_reader_construct_004425c0(void* reader, const void* root_lua_obj_20b) = 0;
  virtual void hit_record_set_entity_00470370(void* record, void* entity) = 0;
  virtual void hit_record_reset_00470470(void* record) = 0;
  virtual void hit_record_destroy_004704b0(void* record) = 0;
  virtual void unit_list_remove_004845a0(void* unit_list, void* entity) = 0;
  virtual void* mission_entity_lock_get_004c1570() = 0;
  virtual void collision_node_init_fields_004e6480(void* collision_node) = 0;
  virtual void scene_property_bag_remove_record_004f01f0(void* bag_map_at_4, void* prop_record) = 0;
  virtual void observer_endpoint_destruct_00695760(void* endpoint) = 0;
  virtual void callback_owner_destroy_00695870(void* callback_owner) = 0;
  virtual void observer_notify_slot0c_00696350(
      void* entity,
      std::int32_t event_id,
      std::uint32_t value
  ) = 0;
  virtual void lua_machine_remove_006b7ea0(void* lua_machine, std::int32_t index) = 0;
  virtual void lua_machine_push_string_006b8120(void* lua_machine, const char* text) = 0;
  virtual void lua_table_set_string_number_006b8260(
      void* lua_machine,
      const char* key,
      float value
  ) = 0;
  virtual void lua_table_set_string_string_006b8360(
      void* lua_machine,
      const char* key,
      const char* value
  ) = 0;
  virtual void lua_machine_get_global_006b8460(void* lua_machine, const char* name) = 0;
  virtual void lua_machine_get_table_006b8470(void* lua_machine, std::int32_t index) = 0;
  virtual void lua_machine_pop_006b85f0(void* lua_machine, std::int32_t count) = 0;
  virtual void lua_variant_vector_resize_006b88e0(void* vec, std::int32_t new_size) = 0;
  virtual std::int32_t skill_from_crew_level_006e6210(std::int32_t crew_level) = 0;
  virtual void* mission_lua_variant_allocate_link_record_006edea0(void* tree_owner) = 0;
  virtual void lua_variant_vector_push_back_006edf00(void* vec, const void* variant) = 0;
  virtual void mission_lua_variant_map_destroy_node_006ee040(void* pair_buf) = 0;
  virtual void* lua_variant_pair_construct_006ee0d0(
      void* pair_buf,
      const void* key_str,
      const void* val_variant
  ) = 0;
  virtual void lua_variant_map_insert_unique_006ee4e0(
      void* map_tree,
      void* out_iter_pair,
      const void* kv_pair
  ) = 0;
  virtual void stl_tree_erase_range_006ee960(
      void* map_tree,
      void* out_iter,
      void* first_proxy,
      void* first_node,
      void* last_proxy,
      void* last_node
  ) = 0;
  virtual void* weapon_director_current_command_0071be40(void* director) = 0;
  virtual void entity_life_ref_release_00740270(void* global_array, void* life_ref) = 0;
  virtual void* session_message_construct_base_0075b430(void* msg, std::uint8_t msg_type) = 0;
  virtual void session_route_message_0077c2a0(
      void* sender_entity,
      void* msg,
      std::uint32_t a1,
      std::uint32_t a2
  ) = 0;
  virtual bool geometry_segment_box_hit_0085cdb0(
      float* out_hit_xyz,
      const float* box_min_xyz,
      const float* box_max_xyz,
      const float* p0_xyz,
      const float* p1_xyz
  ) = 0;
  virtual void* tick_registration_construct_00875890(
      void* tick_node,
      void* payload,
      std::int32_t group
  ) = 0;
  virtual void* lua_variant_construct_int_00884a00(void* variant, std::int32_t val) = 0;
  virtual void lua_variant_destruct_00884af0(void* variant) = 0;
  virtual void* lua_variant_key_construct_00884d90(void* key_obj, const char* key_text) = 0;
  virtual void* lua_variant_construct_string_00884fc0(
      void* variant,
      const void* native_str
  ) = 0;
  virtual void* lua_variant_construct_table_00886840(
      void* variant,
      const void* variant_map
  ) = 0;
  virtual void entity_set_think_script_name_0088a330(void* entity, const char* script_name) = 0;
  virtual bool szurke_nyil_contains_unit_008ddf90(void* szurke_nyil, void* entity) = 0;
  virtual const char* scene_property_get_string_value_008f0df0(void* prop_record) = 0;
  virtual const char* scene_property_get_value_text_008f0e20(void* prop_record) = 0;
  virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
  virtual void* scene_property_bag_clone_008f41f0(void* src_bag) = 0;
  virtual void world_unlink_from_update_chain_00903f30(void* chain_head, void* entity) = 0;
  virtual void scene_node_disable_00922f80(void* entity, std::int32_t flag) = 0;
  virtual void* deferred_entity_event_lock_construct_00924200(void* mem) = 0;
  virtual void entity_unlink_from_sibling_chain_00924710(void* chain_head, void* entity) = 0;
  virtual void* stl_list_buy_sentinel_node_00924a50(void* list_this) = 0;
  virtual void* scene_node_find_by_qualified_name_009251f0(
      void* registry_this,
      void* party_node,
      const char* name
  ) = 0;
  virtual void* weak_owner_construct_00925490(void* weak_owner) = 0;
  virtual void weak_owner_destroy_00925540(void* weak_owner) = 0;
  virtual void scene_node_attach_to_parents_009258f0(
      void* entity,
      void* hierarchy_parent,
      void* world_node,
      const void* matrix
  ) = 0;
  virtual void* entity_scalar_deleting_dtor_00925f00(void* entity, std::int32_t flags) = 0;
  virtual void* stl_list_buy_node_hit_record_00926670(
      void* list_this,
      void* next_node,
      void* prev_node,
      const void* hit_record_60h
  ) = 0;
  virtual void stl_xlen_throw_00926890(void* list_this, std::uint32_t inc) = 0;
  virtual void stl_list_insert_range_009269b0(
      void* dest_list,
      void* where_proxy,
      void* where_node,
      void* first_proxy,
      void* first_node,
      void* last_proxy,
      void* last_node
  ) = 0;
  virtual void sentity_push_pending_00926be0(void* entity) = 0;
  virtual void* mission_entity_lua_self_object_00927b40(void* entity, void* out_lua_obj) = 0;
  virtual void mission_scoring_notify_entity_00927f60(void* entity) = 0;
  virtual void* entity_registry_get_scene_lock_00928240() = 0;
  virtual std::int32_t entity_list_erase_by_entity_00928300(void* list, void* entity) = 0;
  virtual void* game_entity_scalar_deleting_dtor_009289e0(void* entity, std::int32_t flags) = 0;
  virtual void entity_invoke_think_once_009291d0(void* entity, void* arg_vec) = 0;
  virtual void* tickable_game_entity_scalar_deleting_dtor_0092a220(
      void* entity,
      std::int32_t flags
  ) = 0;
  virtual void entity_id_table_release_009516d0(void* table, std::uint16_t id) = 0;
  virtual std::uint16_t entity_id_table_allocate_009517c0(
      void* table,
      std::uint16_t requested_id,
      void* entity
  ) = 0;
  virtual void lua_object_push_cstring_00b66710(void* lua_obj, const char* text) = 0;
  virtual void lua_object_set_cstring_00b66790(
      void* lua_obj,
      const char* key,
      const char* value
  ) = 0;
  virtual void lua_object_set_nil_00b67350(void* lua_obj, const void* native_str) = 0;
  virtual void lua_object_set_boolean_00b673a0(
      void* lua_obj,
      const void* native_str,
      std::uint8_t value
  ) = 0;
  virtual void lua_object_destruct_00b67700(void* lua_obj) = 0;
  virtual void* lua_object_get_by_index_00b67720(
      void* table_lua_obj,
      void* out_lua_obj,
      std::int32_t index
  ) = 0;
  virtual void* lua_object_get_by_name_00b67800(
      void* table_lua_obj,
      void* out_lua_obj,
      const char* name
  ) = 0;
  virtual void* lua_state_owner_get_globals_00b67980(void* state_owner, void* out_lua_obj) = 0;
  virtual void singleton_lifetime_register_00bd0c30(void* manager, void* instance) = 0;
  virtual void sized_storage_pool_return_block_00bd1510(
      void* pool,
      void* block,
      std::uint32_t bytes,
      std::int32_t count
  ) = 0;
  virtual bool lua_reader_has_key_00bd5eb0(
      void* reader,
      std::int32_t key_kind,
      const void* key_val
  ) = 0;
  virtual void lua_reader_read_field_00bd6830(
      void* reader,
      std::int32_t key_kind,
      const void* key_val,
      std::int32_t field_type,
      void* dest
  ) = 0;
  virtual void lua_reader_read_field_or_default_00bd68d0(
      void* reader,
      std::int32_t key_kind,
      const void* key_val,
      std::int32_t field_type,
      void* dest,
      std::int32_t default_type,
      const void* default_val
  ) = 0;
  virtual void lua_reader_leave_key_thunk_00bd7a20(void* reader) = 0;
  virtual void lua_reader_enter_key_00bd8e20(
      void* reader,
      std::int32_t key_kind,
      const void* key_val
  ) = 0;
  virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
  virtual void free_00bf6989(void* ptr) = 0;
  virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
  virtual char* itoa_00bf9010(std::int32_t value, char* buf, std::int32_t radix) = 0;
  virtual void* dyn_physics_first_shape_00c31dc0(void* body) = 0;
  virtual void dyn_physics_get_angular_velocity_00c31f20(void* body, float* out_xyz) = 0;
  virtual void dyn_physics_get_linear_velocity_00c31f40(void* body, float* out_xyz) = 0;
  virtual void dyn_physics_get_box_extents_00c31f90(
      void* shape_box,
      float* out_min_xyz,   // first output: 00929B80 passes it to 0085CDB0 in EDX, the min slot
      float* out_max_xyz
  ) = 0;
  virtual float dyn_body_get_mass_00c31fc0(void* body) = 0;
  virtual float* object_field_at8_accessor_00c32000(void* body) = 0;
  virtual void dyn_physics_apply_force_at_point_00c37f60(
      void* body,
      const float* force_xyz,
      const float* point_xyz
  ) = 0;
  virtual void dyn_physics_clear_collision_mask_bits_00c47f60(
      void* shape,
      std::uint32_t mask
  ) = 0;
  virtual void dyn_physics_set_collision_mask_bits_00c47f90(
      void* shape,
      std::uint32_t mask
  ) = 0;
  virtual void dyn_physics_set_shape_field30_00c48020(void* shape, std::uint32_t val) = 0;
  virtual void dyn_physics_set_shape_field2c_00c48050(void* shape, std::uint32_t val) = 0;
};

// Segment 60 reconstructed functions (58 functions: 00922b10..0092ced0)
std::int32_t entity_is_airborne_target_00922b10(Seg60Host& host, void* entity);
void* entity_cast_to_submarine_00922b70(Seg60Host& host, void* entity);
void* entity_cast_to_land_fort_00922b90(Seg60Host& host, void* entity);
std::uint8_t entity_is_surface_target_00922c80(
    Seg60Host& host,
    void* entity,
    std::uint8_t allow_far
);
// 00922DD0..00922DD6 live listing: XOR DL,DL; JMP00922C80. Original
// ECX=entity, no stack args, AL result. Callee model supplies the whole body;
// this explicit interface is not the original register ABI.
inline std::uint8_t entity_is_surface_target_00922dd0(Seg60Host& host, void* entity) {
    return entity_is_surface_target_00922c80(host, entity, 0);
}
void* scene_property_bag_ref_construct_00922e20(
    Seg60Host& host,
    void* self,
    void* src_bag
);
void* entity_find_ancestor_of_kind_00922e90(
    Seg60Host& host,
    void* entity,
    std::int32_t kind
);
void triv_body_00923020(void* self, std::uint32_t unused) noexcept;
void triv_body_00923030(void* self, std::uint32_t unused) noexcept;
void triv_body_00923040(void* self) noexcept;
void triv_body_00923050(void* self) noexcept;
void triv_body_00923060(void* self) noexcept;
const char* race_name_from_index_00923080(Seg60Host& host, std::int32_t race_index);
void owned_ptr_delete_reset_00923290(Seg60Host& host, void* slot_ptr);
void entity_inherit_race_and_activate_00923840(void* entity) noexcept;
void triv_body_00923860(void* self) noexcept;
void* sentity_get_saved_entity_lua_data_009238a0(
    Seg60Host& host,
    void* entity,
    void* out_lua_obj
);
float* scene_property_copy_float16_00924280(
    void* prop_record,
    float* dst16
) noexcept;
void* deferred_entity_event_lock_get_00924990(Seg60Host& host);
void* hit_record_assign_00925050(Seg60Host& host, void* dst_record, const void* src_record);
void entity_destruct_base_00925780(Seg60Host& host, void* entity);
void* cg_adjustor_thunk_009258d0(Seg60Host& host, void* subobj_at_24, std::int32_t flags);
void* cg_adjustor_thunk_009258e0(Seg60Host& host, void* subobj_at_10, std::int32_t flags);
void* entity_registry_find_entity_by_name_00925a90(
    Seg60Host& host,
    void* registry,
    const char* name
);
void* scene_node_construct_00925ce0(Seg60Host& host, void* self);
void deferred_entity_event_queue_push_00926e80(
    Seg60Host& host,
    void* hit_record,
    const float* direction_xyz
);
void* pending_entity_list_copy_00926fa0(Seg60Host& host, void* dst_list, void* src_list);
void mission_entity_apply_spawn_descriptor_00927050(Seg60Host& host, void* entity);
std::uint8_t triv_body_00927800(void* self) noexcept;
std::int32_t property_bag_get_skill_00927a80(Seg60Host& host, void* entity);
void entity_get_property_unit_command_00927ad0(
    Seg60Host& host,
    void* entity,
    void* out_lua_obj,
    const void* key_native_str
);
void unit_push_lua_self_slot_00927bf0(Seg60Host& host, void* unit, void* lua_machine);
void unit_set_role_availability_00927d20(
    Seg60Host& host,
    void* unit,
    std::uint32_t role_mask,
    std::int32_t state
);
std::int32_t unit_instance_is_local_player_role_00927f30(
    Seg60Host& host,
    const void* unit,
    std::int32_t role_index
);
void* entity_list_erase_node_00928030(Seg60Host& host, void* list, void* node);
void sentity_mirror_identity_to_this_table_00928100(Seg60Host& host, void* entity);
void unit_set_role_slot_provisional_009281c0(
    Seg60Host& host,
    void* unit,
    std::int32_t role_index,
    std::int32_t slot_val
);
void* game_entity_construct_00928630(
    Seg60Host& host,
    void* self,
    std::uint8_t registry_select,
    std::uint16_t requested_id
);
void* cg_adjustor_thunk_00928790(Seg60Host& host, void* subobj_at_24, std::int32_t flags);
void* cg_adjustor_thunk_009287a0(Seg60Host& host, void* subobj_at_10, std::int32_t flags);
void game_entity_destruct_009287b0(Seg60Host& host, void* entity);
void game_entity_place_in_world_00928860(
    Seg60Host& host,
    void* entity,
    void* hierarchy_parent,
    void* world_node,
    const void* matrix
);
void script_entity_destruct_00928920(Seg60Host& host, void* entity);
void sentity_default_init_slot_a4_009295b0(Seg60Host& host, void* entity);
void mission_entity_publish_lua_dead_00929800(Seg60Host& host, void* entity);
void mission_entity_on_killed_lua_00929b60(Seg60Host& host, void* entity);
std::uint8_t collision_shape_trace_segment_transformed_box_00929b80(
    Seg60Host& host,
    void* shape_at_1a4,
    const void* p0_xyz,
    const void* p1_xyz,
    void* hit_record
);
void* tickable_game_entity_construct_00929e50(Seg60Host& host, void* self);
std::uint8_t triv_body_00929ff0(
    void* self,
    std::uint32_t a0,
    std::uint32_t a1,
    std::uint32_t a2
) noexcept;
void* cg_adjustor_thunk_0092a000(Seg60Host& host, void* subobj_at_24, std::int32_t flags);
void* cg_adjustor_thunk_0092a020(Seg60Host& host, void* subobj_at_1c4, std::int32_t flags);
void* cg_adjustor_thunk_0092a030(Seg60Host& host, void* subobj_at_170, std::int32_t flags);
void tickable_game_entity_tick_advance_0092b350(
    Seg60Host& host,
    void* tick_node_at_170,
    float step
);
void unit_controller_clear_hull_shape_fields_0092bd30(Seg60Host& host, void* controller);
void unit_controller_set_collision_group_0092bd70(
    Seg60Host& host,
    void* controller,
    std::uint8_t enabled
);
std::uint32_t triv_body_0092be90(const void* self) noexcept;
std::uint32_t triv_body_0092bea0(const void* self) noexcept;
float triv_body_0092beb0(const void* self) noexcept;
void* unit_message_part_destroyed_construct_0092ced0(
    Seg60Host& host,
    void* msg,
    std::int32_t part_index
);

}  // namespace bsp::entity_mission_seg60_argon
