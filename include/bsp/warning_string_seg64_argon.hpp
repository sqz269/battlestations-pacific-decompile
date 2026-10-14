#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::warning_string_seg64_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct TreeIteratorPair {
  void* container;
  void* node;
};

struct ShipNoiseDefaultValue {
  float disable_time_0;
  float disable_time_4;
  void* effect_ref;
};

struct ShipNoiseMapValue {
  std::uint32_t key;
  float disable_time_0;
  float disable_time_4;
  void* effect_ref;
};

struct MessageRef12 {
  void* vtable;
  void* record;
  std::uint32_t kind;
};

struct Seg64Host {
  virtual ~Seg64Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // External direct callees (verified against assembly/RET in bundle_argon-v2-seg64.txt)
  virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual void* stl_tree_erase_range_00492a00(
      void* tree,
      void* out_iter,
      void* first_container,
      void* first_node,
      void* last_container,
      void* last_node) = 0;
  virtual void stl_vector_push_back_005bb020(void* vec, const void* value) = 0;
  virtual void voice_manager_play_line_005bbc10(
      void* voice_manager,
      void* clip_vector,
      std::uint32_t target_word,
      void* speaker_entity) = 0;
  virtual void voice_manager_play_positional_line_005bbdc0(
      void* voice_manager,
      const void* vtable_00cf0dd0,
      void* clip_record,
      std::uint32_t clip_kind,
      void* speaker_entity) = 0;
  virtual void* message_record_resolve_named_00705e00(
      void* resolver,
      const void* native_string,
      std::uint32_t flags) = 0;
  virtual std::uint32_t recon_relation_if_identified_00803ce0(
      std::int32_t party,
      void* unit) = 0;
  virtual void point_effect_stop_children_00867b10(void* point_effect) = 0;
  virtual void** point_effect_create_with_parent_matrix_008687c0(
      void** output_slot,
      void* parent,
      void* consumed_definition,
      const float* matrix,
      std::uint32_t transform_word,
      std::uint32_t option_word,
      std::uint32_t tail_word) = 0;
  virtual void* stl_list_buy_node_0096b510(
      void* list,
      void* next_node,
      void* prev_node,
      const void* value_ptr) = 0;
  virtual void* stl_tree_buy_head_node_0096ba00(void* tree) = 0;
  virtual void ship_noise_info_start_air_raid_effect_0096c070(
      void* warning_manager,
      void* record,
      void* ship_entity) = 0;
  virtual void warning_save_archive_0096cf10(void* warning, void* archive) = 0;
  virtual void warning_event_record_destruct_0096dcb0(void* record) = 0;
  virtual void warning_destruct_0096f170(void* warning) = 0;
  virtual void entity_warning_destruct_0096f270(void* warning) = 0;
  virtual void stl_list_incsize_0096f960(void* list, std::uint32_t count) = 0;
  virtual void* stl_tree_insert_hint_00973410(
      void* tree,
      void* out_iter,
      void* hint_container,
      void* hint_node,
      const void* value_ptr) = 0;
  virtual void* message_warning_construct_with_id_00975ac0(
      void* warning,
      const void* id_native_string) = 0;
  virtual float* stl_map_subscript_u32_f32_00975c40(
      void* map,
      const std::uint32_t* key_ptr) = 0;
  virtual void string_list_warning_destroy_009762a0(void* warning) = 0;
  virtual void warning_manager_report_009763e0(
      void* warning_manager,
      void* warning) = 0;
  virtual void* stl_tree_find_u16_009790d0(
      void* tree,
      void* out_iter,
      const void* key_u16_ptr) = 0;
  virtual void sized_storage_pool_return_block_00bd1510(
      void* pool,
      void* block,
      std::uint32_t bytes,
      std::int32_t count) = 0;
  virtual void free_00bf65ac(void* block) = 0;
  virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;

  // Import table and virtual slot hooks
  virtual void leave_critical_section_00ce2210(void* critical_section) = 0;
  virtual void enter_critical_section_00ce2218(void* critical_section) = 0;
  virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* addend) = 0;
  virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* addend) = 0;
  virtual void ref_counted_vslot00_destroy(void* ref_object) = 0;
  virtual void* warning_vslot00_deleting_dtor(void* warning, std::uint8_t flags) = 0;
  virtual std::uint32_t warning_vslot10_get_kind(void* warning) = 0;
  virtual void archive_vslot04_begin_table(
      void* archive,
      std::uint32_t kind,
      const void* name) = 0;
  virtual void archive_vslot08_end_table(void* archive) = 0;
  virtual void archive_vslot0c_write_entry(
      void* archive,
      std::uint32_t key_kind,
      std::uint32_t key_index,
      std::uint32_t val_kind,
      const void* val_str) = 0;
};

// 47 reconstructed functions in Segment 64 (0096aa00 through 0097aaf0)
void warning_activate_0096aa00(Seg64Host& host, Raw warning);
void warning_list_remove_by_value_0096c280(
    Seg64Host& host,
    Raw list,
    void* const* value_ptr);
void vtable_native_string_release_storage_0096cd90(Seg64Host& host, Raw obj);
void* vtable_native_string_scalar_deleting_dtor_0096cdc0(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags);
void string_list_warning_save_archive_0096d330(
    Seg64Host& host,
    Raw warning,
    void* archive);
void native_string_release_storage_0096d7b0(Seg64Host& host, Raw str);
void* ref_holder_scalar_deleting_dtor_0096db10(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags);
std::uint32_t warning_event_record_get_kind_0096dca0();
void* warning_event_record_scalar_deleting_dtor_0096dd10(
    Seg64Host& host,
    Raw record,
    std::uint8_t flags);
void* offset_native_string_scalar_deleting_dtor_0096ecc0(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags);
float warning_get_zero_lifetime_0096f220();
float warning_get_default_cooldown_0096f240(Seg64Host& host);
void* warning_base_scalar_deleting_dtor_0096f250(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
void* entity_warning_kind0_scalar_deleting_dtor_00973b80(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t entity_warning_get_kind1_00973c50();
void* entity_warning_kind1_scalar_deleting_dtor_00973c80(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t entity_warning_get_kind2_00973cd0();
float entity_warning_kind2_get_cooldown_00973ce0(Seg64Host& host);
void* entity_warning_kind2_scalar_deleting_dtor_00973d10(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t warning_get_kind3_00973d50();
void* warning_kind3_scalar_deleting_dtor_00973d70(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t entity_warning_get_kind4_00973dc0();
void* entity_warning_kind4_scalar_deleting_dtor_00973df0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t entity_warning_get_kind5_00973e40();
void* entity_warning_kind5_scalar_deleting_dtor_00973e70(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t entity_warning_get_kind6_00973ec0();
void* entity_warning_kind6_scalar_deleting_dtor_00973f00(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
void warning_manager_apply_warning_00974070(
    Seg64Host& host,
    Raw manager,
    void* warning_ptr);
std::uint32_t warning_get_kind7_00975710();
void* warning_kind7_scalar_deleting_dtor_00975730(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t warning_get_kind9_00975770();
void* warning_kind9_scalar_deleting_dtor_00975790(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t warning_get_kind8_009758a0();
void* warning_kind8_scalar_deleting_dtor_009758c0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t warning_get_kind10_00975a80();
void* warning_kind10_scalar_deleting_dtor_00975aa0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
float* ship_noise_map_subscript_00975d00(
    Seg64Host& host,
    Raw map,
    const std::uint32_t* key_ptr);
std::uint32_t string_list_warning_get_kind0_00976280();
void warning_manager_report_first_string_00976cb0(
    Seg64Host& host,
    Raw manager,
    Raw string_vector,
    std::uint32_t unused_arg2 = 0,
    std::uint32_t unused_arg3 = 0);
void string_list_warning_activate_00976db0(Seg64Host& host, Raw warning);
void warning_manager_cancel_by_target_00976f10(
    Seg64Host& host,
    Raw manager,
    std::uint32_t target_id);
void warning_manager_play_collision_sound_00977820(
    Seg64Host& host,
    Raw manager,
    Raw ship_entity);
void warning_manager_scan_proximity_00977990(Seg64Host& host, Raw manager);
void* string_list_warning_scalar_deleting_dtor_00978fe0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags);
std::uint32_t subscription_set_matches_or_empty_00979140(
    Seg64Host& host,
    Raw subscription_set,
    Raw candidate);
void* subscription_tree_owner_scalar_deleting_dtor_0097a130(
    Seg64Host& host,
    Raw owner,
    std::uint8_t flags);
void* fire_subscription_construct_0097aaf0(Seg64Host& host, Raw subscription);

}  // namespace bsp::warning_string_seg64_argon
