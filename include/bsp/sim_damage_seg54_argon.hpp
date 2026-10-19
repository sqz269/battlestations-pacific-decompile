#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::sim_damage_seg54_argon {

// 00875220 ECX singleton owner, RET. Clear F878DC then store CE3818 at
// owner+4; null owner is the original invalid address-zero store path.
void damage_singleton_reset_00875220(void*, std::uint32_t& singleton_f878dc) noexcept;
// 00876BA0 ECX packed DWORD -> AL=bits24..31, RET.
std::uint8_t packed_word_high_byte_00876ba0(std::uint32_t) noexcept;
struct DamageLastVariantCalls {
  virtual ~DamageLastVariantCalls() = default;
  virtual void set_variant_slot1b4(void* owner, std::int32_t index) = 0;
};
// 00877A50 ECX damageable, RET. current+358h, class+360h, 10h rows at
// class+16Ch..170h. Empty/null row arrays select zero; nonempty select last.
void damage_select_last_variant_00877a50(void*, DamageLastVariantCalls&);

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

struct Seg54Host {
  virtual ~Seg54Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // Win32 / CRT / core lifetime services
  virtual void enter_critical_section_00ce2218(void* crit_sec) = 0;
  virtual void leave_critical_section_00ce2210(void* crit_sec) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void invalid_parameter_noinfo_00bf6713() = 0;
  virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
  virtual void* singleton_lifetime_get_manager_00415350() = 0;
  virtual void singleton_lifetime_register_00bd0c30(void* manager, void* object) = 0;
  virtual void* critical_section_create_00bd1860() = 0;
  virtual void critical_section_destroy_owned_0041cc80(void* lock_slot_addr) = 0;

  // External engine callees
  virtual void model_instance_set_numbering_00711be0(void* model_instance, std::int32_t number) = 0;
  virtual void* session_message_construct_base_0075b430(void* message_storage, std::uint8_t msg_type) = 0;
  virtual void session_flush_outbound_step_0076ffc0(void* session, float step_dt, std::int32_t mode) = 0;
  virtual void session_pump_step_00778450(void* session, float step_dt) = 0;
  virtual void net_entity_write_replication_state_0077c160(void* entity, void* writer) = 0;
  virtual void session_owner_broadcast_message_0077c7b0(void* entity, void* message, void* exclude_peer) = 0;
  virtual void replication_apply_pending_entity_creates_0077ec20() = 0;
  virtual void tick_registry_flush_pending_groups_00874c90() = 0;
  virtual void unit_instance_set_model_group_state_00876ec0(void* entity, std::int32_t level) = 0;
  virtual void unit_instance_set_health_00877b90(void* unit, float health) = 0;
  virtual void mission_lua_host_drain_queued_calls_00888230(void* lua_host) = 0;
  virtual void world_release_expired_objects_00903610(void* world) = 0;
  virtual void sentity_init_all_00925f20(std::uint8_t flag) = 0;
  virtual void deferred_entity_event_queue_drain_00926700() = 0;
  virtual void entity_event_queues_flush_pending_009273a0() = 0;
  virtual void entity_think_list_run_due_00929460(float step_dt) = 0;
  virtual void scene_node_set_visibility_factor_00b6da70(void* scene_node, float factor, std::uint8_t recurse) = 0;
  virtual std::uint8_t tagged_value_equals_00bd5680(const void* lhs_tagged, const void* rhs_tagged) = 0;

  // Virtual table dispatchers
  virtual void* vtable_00_deleting_dtor(void* obj, std::uint8_t flags) = 0;
  virtual void vtable_04_step_pre(void* sim_obj, float dt) = 0;
  virtual void vtable_08_step_post(void* sim_obj, float dt) = 0;
  virtual void vtable_0c_step_mid(void* sim_obj) = 0;
  virtual std::uint8_t vtable_5c_is_kind_of(void* entity, std::int32_t class_id) = 0;
  virtual void writer_vtable_04_begin_table(void* writer, std::uint32_t tag_kind, const char* name) = 0;
  virtual void writer_vtable_08_end_table(void* writer) = 0;
  virtual void writer_vtable_0c_write_tagged(
    void* writer,
    std::uint32_t key_kind,
    const char* key_name,
    std::uint32_t val_kind,
    std::uint32_t val_payload
  ) = 0;
};

void sim_object_run_fixed_steps_provisional_008746a0(Seg54Host& host, void* this_ptr, std::int32_t count);
void triv_body_008746f0(Seg54Host& host, void* this_ptr, std::uint32_t arg0);
void triv_body_00874700(Seg54Host& host, void* this_ptr, std::uint32_t arg0);
void fixed_step_callback_registry_clear_singleton_unwind_00874740(Seg54Host& host, void* this_ptr);
void* cg_scalar_deleting_dtor_00874930(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00874a80(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* singleton_holder_f878d4_deleting_dtor_00874ac0(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* singleton_holder_f878d8_deleting_dtor_00874b00(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00874b40(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00874c10(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* fixed_step_callback_registry_construct_00874c40(Seg54Host& host, void* this_ptr);
void game_run_extra_fixed_step_00874d00(Seg54Host& host, std::uint8_t mode);
void unit_tick_element_clear_sub_list_00874f00(Seg54Host& host, void* this_ptr);
void* cg_adjustor_thunk_00874fb0(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00875010(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_adjustor_thunk_00875070(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00875100(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_adjustor_thunk_00875180(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_008751b0(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_adjustor_thunk_00875210(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00875240(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void* fixed_step_callback_registry_get_singleton_00875340(Seg54Host& host);
void fixed_step_callback_registry_singleton_holder_dtor_00875420(Seg54Host& host, void* this_ptr);
void* cg_scalar_deleting_dtor_00875450(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void unit_tick_element_destruct_00875490(Seg54Host& host, void* this_ptr);
void* step_job_wave3_get_descriptor_008754d0(Seg54Host& host);
void* step_job_wave1_get_descriptor_008755a0(Seg54Host& host);
void* step_job_wave2_get_descriptor_00875750(Seg54Host& host);
void pending_registry_singleton_holder_dtor_00875820(Seg54Host& host, void* this_ptr);
void* tick_element_scalar_deleting_dtor_00875920(Seg54Host& host, void* this_ptr, std::uint8_t flags);
void fixed_step_callback_unregister_00875ae0(Seg54Host& host, void* this_ptr);
void unit_instance_set_numbering_008761e0(Seg54Host& host, void* this_ptr, std::int32_t number);
void* unit_model_group_find_entry_00876e20(Seg54Host& host, void* this_ptr, std::int32_t key_a, std::int32_t key_b);
void unit_tickable_entity_write_state_008776e0(Seg54Host& host, void* this_ptr, void* writer);
void unit_tickable_entity_set_damaged_gfx_level_00877b00(Seg54Host& host, void* this_ptr, std::int32_t level);
void unit_instance_set_health_from_scale255_00877cd0(Seg54Host& host, void* this_ptr, std::int32_t scale_val);
void unit_tickable_entity_set_scene_nodes_visible_008781f0(Seg54Host& host, void* this_ptr, std::uint8_t visible);
void** unit_tickable_entity_find_damage_section_008782a0(
  Seg54Host& host,
  void* this_ptr,
  std::int32_t key_a,
  std::int32_t key_b
);

}  // namespace bsp::sim_damage_seg54_argon
