#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::director_geommesh_seg38_39_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct Seg38_39Host {
  virtual ~Seg38_39Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // Virtual table dispatch slots
  virtual void vcall_00(void* obj) = 0;
  virtual void vcall_00_deleting(void* obj, std::uint32_t flags) = 0;
  virtual void vcall_08(void* obj) = 0;
  virtual bool vcall_0c_u32_bool(void* obj, std::uint32_t arg) = 0;
  virtual void vcall_18_u32(void* obj, std::uint32_t arg) = 0;
  virtual void* vcall_24(void* obj) = 0;
  virtual void vcall_30(void* obj) = 0;
  virtual void vcall_38_ptr(void* obj, void* arg) = 0;
  virtual void vcall_3c_u32(void* obj, std::uint32_t arg) = 0;
  virtual void vcall_4c(void* obj, void* arg) = 0;
  virtual bool vcall_5c_is_kind_of(void* obj, std::uint32_t kind) = 0;
  virtual void* vcall_114(void* obj) = 0;
  virtual void vcall_1b4_u32(void* obj, std::uint32_t arg) = 0;

  // Win32 import slots
  virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* addend) = 0;
  virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* addend) = 0;

  // External callees
  virtual float* vector3f_transform_affine_point_004142e0(const float* src, float* dst, const void* matrix) = 0;
  virtual std::int32_t stl_checked_vector12_size_00415290(const void* vec) = 0;
  virtual void* stl_vector12_move_forward_00419330(
      const void* first,
      const void* last,
      void* dest,
      std::uint32_t tag0,
      std::uint32_t tag1,
      std::uint32_t tag2) = 0;
  virtual void* stl_vector12_move_forward_dispatch_0041c4d0(const void* first, const void* last, void* dest) = 0;
  virtual bool stl_vector12_buy_00420540(void* vec, std::uint32_t capacity) = 0;
  virtual void hit_record_reset_00470470(void* record) = 0;
  virtual void* command_target_resolve_object_00521ea0(void* target_descriptor) = 0;
  virtual void stl_inst_00585e70(void* vec) = 0;
  virtual void* stl_vector_u16_copy_construct_0058aac0(void* dst_vec, const void* src_vec) = 0;
  virtual void hit_record_append_part_hit_006d2e30(void* record, const void* entry) = 0;
  virtual void* scalar_deleting_dtor_0071e410(void* self, std::uint8_t flags) = 0;
  virtual void* weapon_director_newest_command_target_0071ebf0(void* director) = 0;
  virtual void* stl_inst_0071f0b0(void* vec, const void* first, const void* last, void* dest) = 0;
  virtual void geom_mesh_radix_sort_key4_00722400(std::uint32_t* records, std::int32_t count, std::uint32_t* scratch) = 0;
  virtual void geom_mesh_radix_sort_key8_007225f0(std::uint32_t* records, std::int32_t count, std::uint32_t* scratch) = 0;
  virtual void* stl_allocator_allocate_007227e0(std::uint32_t count) = 0;
  virtual void vector3f_min_max_00722b20(const float* a, const float* b, float* out_min, float* out_max) = 0;
  virtual void stl_destroy_range_geom_mesh_element_00725670(void* vec, void* first, void* last) = 0;
  virtual void stl_vector_tidy_geom_mesh_element_007256a0(void* vec) = 0;
  virtual void stl_xlen_throw_00725c50() = 0;
  virtual void* stl_uninit_copy_geom_mesh_elements_00726a80(
      const void* first,
      const void* last,
      void* dst,
      void* vec,
      std::uint32_t cap,
      std::uint32_t tag) = 0;
  virtual void geom_mesh_resource_parser_parse_payload_00727310(void* item, void* node_handle) = 0;
  virtual bool unit_instance_is_occupant_owner_00778890(void* unit) = 0;
  virtual void path_cursor_set_index_from_message_007ae060(void* cursor, std::int32_t index) = 0;
  virtual void path_source_ref_copy_from_007b1bf0(void* dst_ref, const void* src_ref) = 0;
  virtual void** entity_path_source_create_for_slot_007b2250(void** out_slot, void* path_interface) = 0;
  virtual float aabb_point_distance_squared_0085bf90(const float* point, const float* box_min, const float* box_max) = 0;
  virtual bool geometry_segment_overlaps_aabb_0085cad0(
      const float* box_min,
      const float* box_max,
      const float* from,
      const float* to) = 0;
  virtual bool geometry_segment_intersect_triangle_0085d020(
      const float* p0,
      const float* p1,
      const float* a,
      const float* b,
      const float* c,
      float* out_hit) = 0;
  virtual float geometry_sphere_triangle_distance_0085df60(
      const float* centre,
      const float* a,
      const float* b,
      const float* c) = 0;
  virtual void debug_draw_line_rgba_floats_0085f990(
      const float* p0,
      const float* p1,
      float r0,
      float g0,
      float b0,
      float a0,
      float r1,
      float g1,
      float b1,
      float a1) = 0;
  virtual void tick_subnode_reparent_00876020(void* owner, void* subnode) = 0;
  virtual void tick_subnode_requeue_00876120(void* parent, void* subnode) = 0;
  virtual void unit_tickable_entity_set_scene_nodes_visible_008781f0(void* entity, std::uint32_t visible) = 0;
  virtual void* unit_tickable_entity_construct_0087b670(void* self, std::uint32_t flag) = 0;
  virtual void mission_entity_cancel_pending_destroy_00925a00(void* entity) = 0;
  virtual void unit_rebuild_weapon_category_index_00956c20(void* unit) = 0;
  virtual void transform_refresh_world_matrix_00b6db70(void* transform) = 0;
  virtual void resource_item_destroy_reference_base_00b86890(void* self) = 0;
  virtual void* resource_item_construct_reference_base_00b868b0(void* self) = 0;
  virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
  virtual void free_00bf6989(void* ptr) = 0;
  virtual double libcrt_sqrt_00bf7030(double value) = 0;
  virtual void eh_vector_constructor_iterator_00bf7cd1(
      void* ptr,
      std::uint32_t size,
      std::int32_t count,
      void* ctor,
      void* dtor) = 0;
};

// Reconstructed functions (77 total: 0071bc30 through 00728960)
void triv_body_0071bc30(Seg38_39Host& host, void* self);
void* triv_body_0071bdb0(void* self);
void triv_body_0071bdc0(void* self, std::uint32_t arg1);
void triv_body_0071bdd0(void* self, std::uint32_t arg1);
void* bsp_weapon_director_slot_path_object_0071bfc0(const void* self, std::int32_t index);
void cg_adjustor_thunk_0071c0b0(Seg38_39Host& host, void* self, std::int32_t index);
void bsp_weapon_director_class_test_5c_0071c150(Seg38_39Host& host, void* self, void* other);
void bsp_weapon_director_observer_forward_slot8_0071c1a0(Seg38_39Host& host, void* self);
void bsp_weapon_director_vslot_3c_noop_0071c280(void* self, std::uint32_t arg1);
void tmpl_owned_ptr_delete_reset_0071c370(Seg38_39Host& host, void* self);
void tmpl_owned_ptr_delete_reset_0071c390(Seg38_39Host& host, void* self);
void bsp_director_attach_tick_subnode_0071c470(Seg38_39Host& host, void* self, void* owner);
void triv_body_0071c490(void* self, std::uint32_t arg1);
void triv_body_0071c5d0(Seg38_39Host& host, void* self);
std::uint8_t triv_body_0071d540(void* self, std::uint32_t arg1, std::uint32_t arg2);
std::uint32_t triv_body_0071d5b0(void* self);
void triv_body_0071d5c0(void* self, std::uint32_t arg1);
void* cg_adjustor_thunk_0071d5f0(Seg38_39Host& host, void* self, std::uint8_t flags);
void bsp_weapon_director_assign_slot_target_0071db50(void* self, const void* src_target);
void* bsp_ref_ptr_release_0071ea60(Seg38_39Host& host, void* self);
void bsp_ref_ptr_release_0071f0e0(Seg38_39Host& host, void* self);
void bsp_ref_ptr_assign_0071f110(Seg38_39Host& host, void* self, void* const* src_slot);
void* bsp_weapon_director_fire_target_0071f150(Seg38_39Host& host, void* self);
void bsp_ref_counted_ptr_release_0071f3b0(Seg38_39Host& host, void* self);
void bsp_ref_holder_db04_destruct_0071f460(Seg38_39Host& host, void* self);
void bsp_ref_holder_db04_destruct_0071f530(Seg38_39Host& host, void* self);
void* bsp_vec3_vector_assign_0071f9f0(Seg38_39Host& host, void* self, const void* other);
void* triv_body_0071fc20(void* self);
std::uint8_t triv_body_0071fc30(void* self);
void bsp_director_path_object_destruct_0071fc70(Seg38_39Host& host, void* self);
void bsp_director_path_object_copy_from_0071fd10(Seg38_39Host& host, void* self, const void* other);
void* bsp_command_controller_base_construct_00720180(Seg38_39Host& host, void* self, void* owner);
std::uint8_t triv_body_00720d80(const void* self);
std::uint8_t triv_body_00720d90(const void* self);
std::uint8_t triv_body_00720da0(const void* self);
std::uint8_t triv_body_00720db0(const void* self);
std::uint8_t triv_body_00720dc0(const void* self);
std::uint8_t triv_body_00720dd0(const void* self);
void triv_body_00720f50(void* self, std::uint32_t arg1);
void* bsp_game_unit_message_read_target_descriptor_00721030(const void* self, void* out_target);
std::uint32_t bsp_geom_mesh_resource_item_get_runtime_type_token_00721cb0(Seg38_39Host& host);
std::uint32_t bsp_geom_mesh_resource_item_get_runtime_type_name_address_00721cc0(Seg38_39Host& host);
void bsp_geom_mesh_radix_sort_key0_00722210(std::uint32_t* records, std::int32_t count, std::uint32_t* scratch);
void bsp_geom_mesh_element_root_box_centre_00723030(Seg38_39Host& host, const void* self, float* out_centre);
void bsp_geom_mesh_radix_sort_by_axis_007230f0(
    Seg38_39Host& host,
    std::int32_t axis,
    std::int32_t start,
    std::int32_t count);
std::uint8_t bsp_geom_mesh_element_trace_segment_node_007238e0(
    Seg38_39Host& host,
    const void* self,
    void* query_record,
    std::int32_t node_index,
    std::uint32_t first_ordinal,
    std::int32_t ordinal_count);
std::uint8_t bsp_geom_mesh_element_trace_segment_00723aa0(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    float* out_hit);
std::uint8_t bsp_geom_mesh_element_test_sphere_00723b70(
    Seg38_39Host& host,
    const void* self,
    const float* centre,
    const float* radius,
    float* out_distance);
const void* bsp_geom_mesh_trace_segment_closest_element_00723d60(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    float* out_hit);
std::uint8_t bsp_geom_mesh_trace_segment_into_hit_record_00723e90(
    Seg38_39Host& host,
    const void* self,
    const void* to_world,
    const void* to_local,
    const float* from,
    const float* to,
    void* record);
std::uint8_t bsp_geom_mesh_collect_sphere_hits_00723f80(
    Seg38_39Host& host,
    const void* self,
    const void* body_f0,
    const void* to_local,
    const float* centre,
    float radius,
    void* record);
bool bsp_unit_part_collision_shape_trace_segment_00724510(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    void* record);
void bsp_geom_mesh_debug_draw_wireframe_00724a20(
    Seg38_39Host& host,
    const void* self,
    const void* matrix,
    const float* rgba);
void bsp_geom_mesh_component_debug_draw_00725550(Seg38_39Host& host, void* self);
void bsp_geom_mesh_element_destruct_00725830(Seg38_39Host& host, void* self);
void* bsp_geom_mesh_element_copy_construct_00726710(Seg38_39Host& host, void* self, const void* other);
void* bsp_geom_mesh_resource_item_construct_007268d0(Seg38_39Host& host, void* self);
bool bsp_geom_mesh_resource_item_accepts_runtime_type_token_00726930(Seg38_39Host& host, std::uint32_t token);
void bsp_geom_mesh_resource_item_destruct_00726960(Seg38_39Host& host, void* self);
void bsp_geom_mesh_element_vector_reserve_00726d80(Seg38_39Host& host, void* self, std::uint32_t new_capacity);
void* bsp_geom_mesh_resource_parser_parse_00727a90(Seg38_39Host& host, void* self, void* node_handle);
void* bsp_hit_record_dbfc_construct_00727b20(Seg38_39Host& host, void* self, const void* src_record);
void* bsp_hit_record_dc04_construct_00727b90(Seg38_39Host& host, void* self, const void* src_record);
std::int32_t bsp_gun_index_of_readiest_barrel_00727c30(const void* self);
std::int32_t bsp_gun_count_ready_barrels_00727d70(const void* self, float horizon);
void bsp_gun_set_pair_43c_00727e10(void* self, std::uint32_t val_43c, std::uint32_t val_440);
void bsp_gun_set_bot_skill_00727e70(Seg38_39Host& host, void* self, std::uint32_t skill);
void bsp_gun_set_bot_fire_target_00727f10(Seg38_39Host& host, void* self, void* target, std::uint32_t flag);
void bsp_gun_clear_bot_fire_target_00728000(Seg38_39Host& host, void* self);
void bsp_gun_apply_clear_destroyed_failure_007280a0(Seg38_39Host& host, void* self, void* message);
void bsp_scene_node_clear_flags_recursive_00728340(void* self, std::uint32_t mask);
void bsp_free_owned_block_00728480(Seg38_39Host& host, void* self);
void bsp_owned_ptr_delete_007284b0(Seg38_39Host& host, void* self);
void bsp_free_owned_block_007284d0(Seg38_39Host& host, void* self);
void bsp_owned_ptr_delete_00728520(Seg38_39Host& host, void* self);
void bsp_native_vector12_grow_00728610(Seg38_39Host& host, void* self);
void* bsp_gun_base_construct_00728960(Seg38_39Host& host, void* self, std::uint32_t flag);

}  // namespace bsp::director_geommesh_seg38_39_argon

namespace bsp::cc13_sweepA { struct SweepAHost; }
namespace bsp::director_geommesh_seg38_39_argon {
// oct18b-nB: original ECX=gun, float stack argument, RET 4. Calls 00442AE0
// only while byte +3B8h is zero. Source adds the existing damage host explicitly.
void gun_apply_damage_if_enabled_00728080(void* gun, float amount, cc13_sweepA::SweepAHost& host);
// GunBase failure slots 101..103 in 00CFDC58 / 00D0DF70. XOR AL,AL;
// RET 8 / RET 0Ch / RET 8. Upper EAX and ignored argument types are unknown.
std::uint8_t gun_failure_slot101_007289b0(void*, std::uint32_t, std::uint32_t) noexcept;
std::uint8_t gun_failure_slot102_007289c0(void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
std::uint8_t gun_failure_slot103_007289d0(void*, std::uint32_t, std::uint32_t) noexcept;
}
