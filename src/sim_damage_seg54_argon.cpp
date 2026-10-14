#include "bsp/sim_damage_seg54_argon.hpp"

namespace bsp::sim_damage_seg54_argon {

// 0x008746a0 — BSP_SimObject_RunFixedSteps_Provisional
void sim_object_run_fixed_steps_provisional_008746a0(Seg54Host& host, void* this_ptr, std::int32_t count) {
  for (; count != 0; --count) {
    host.vtable_04_step_pre(this_ptr, host.global_f32(0x00d0de84));
    host.vtable_0c_step_mid(this_ptr);
    host.vtable_08_step_post(this_ptr, host.global_f32(0x00d0de84));
  }
}

// 0x008746f0 — TRIV_body_008746f0
void triv_body_008746f0(Seg54Host&, void*, std::uint32_t) {}

// 0x00874700 — TRIV_body_00874700
void triv_body_00874700(Seg54Host&, void*, std::uint32_t) {}

// 0x00874740 — BSP_FixedStepCallbackRegistry_ClearSingleton_Unwind
void fixed_step_callback_registry_clear_singleton_unwind_00874740(Seg54Host& host, void* this_ptr) {
  host.global_u32(0x00f878c8) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
}

// 0x00874930 — CG_scalar_deleting_dtor_00874930
void* cg_scalar_deleting_dtor_00874930(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878c8) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874a80 — CG_scalar_deleting_dtor_00874a80
void* cg_scalar_deleting_dtor_00874a80(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878d0) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874ac0 — BSP_SingletonHolder_F878D4_DeletingDtor
void* singleton_holder_f878d4_deleting_dtor_00874ac0(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878d4) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874b00 — BSP_SingletonHolder_F878D8_DeletingDtor
void* singleton_holder_f878d8_deleting_dtor_00874b00(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878d8) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874b40 — CG_scalar_deleting_dtor_00874b40
void* cg_scalar_deleting_dtor_00874b40(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878dc) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874c10 — CG_scalar_deleting_dtor_00874c10
void* cg_scalar_deleting_dtor_00874c10(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  host.global_u32(0x00f878cc) = 0;
  Raw{static_cast<std::uint8_t*>(this_ptr)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00874c40 — BSP_FixedStepCallbackRegistry_Construct
void* fixed_step_callback_registry_construct_00874c40(Seg54Host& host, void* this_ptr) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  self.ptr(0x00) = host.image_address(0x00d0dea4);
  self.ptr(0x04) = host.critical_section_create_00bd1860();
  return this_ptr;
}

// 0x00874d00 — BSP_Game_RunExtraFixedStep
void game_run_extra_fixed_step_00874d00(Seg54Host& host, std::uint8_t mode) {
  void* game0 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
  void* lua_host = Raw{static_cast<std::uint8_t*>(game0)}.ptr(0x1a08);
  host.mission_lua_host_drain_queued_calls_00888230(lua_host);

  if (mode == 0u) {
    host.entity_think_list_run_due_00929460(host.global_f32(0x00d0de84));
  }

  void* game1 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
  if (game1 != nullptr) {
    void* world1 = Raw{static_cast<std::uint8_t*>(game1)}.ptr(0x19cc);
    if (Raw{static_cast<std::uint8_t*>(world1)}.at<std::uint8_t>(0x4ac) != 0u) {
      const float pump_dt = (mode != 0u) ? 0.0f : host.global_f32(0x00d0de84);
      host.session_pump_step_00778450(static_cast<std::uint8_t*>(game1) + 0x1ef0, pump_dt);
      host.replication_apply_pending_entity_creates_0077ec20();
      host.tick_registry_flush_pending_groups_00874c90();
      host.sentity_init_all_00925f20(0u);

      const float flush_dt = (mode != 0u) ? 0.0f : host.global_f32(0x00d0de84);
      const std::int32_t flush_mode = (mode == 0u) ? 1 : 0;
      void* game2 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
      host.session_flush_outbound_step_0076ffc0(static_cast<std::uint8_t*>(game2) + 0x1ef0, flush_dt, flush_mode);
    }
  }

  if (mode != 0u) {
    return;
  }

  host.deferred_entity_event_queue_drain_00926700();
  host.entity_event_queues_flush_pending_009273a0();
  void* game3 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
  void* world3 = Raw{static_cast<std::uint8_t*>(game3)}.ptr(0x19cc);
  host.world_release_expired_objects_00903610(world3);
}

// 0x00874f00 — BSP_UnitTickElement_ClearSubList
void unit_tick_element_clear_sub_list_00874f00(Seg54Host& host, void* this_ptr) {
  Raw list{static_cast<std::uint8_t*>(this_ptr)};
  if (list.at<std::int32_t>(0x08) == 0) {
    return;
  }

  do {
    void* head = list.ptr(0x00);
    Raw node{static_cast<std::uint8_t*>(head)};
    void* prev = node.ptr(0x08);
    void* next = node.ptr(0x0c);

    bool unlink = true;
    if (prev != nullptr) {
      Raw{static_cast<std::uint8_t*>(prev)}.ptr(0x0c) = next;
    } else if (next != nullptr || list.at<std::int32_t>(0x08) <= 1) {
      list.ptr(0x00) = next;
    } else {
      unlink = false;
    }

    if (unlink) {
      void* next_now = node.ptr(0x0c);
      void* prev_now = node.ptr(0x08);
      if (next_now != nullptr) {
        Raw{static_cast<std::uint8_t*>(next_now)}.ptr(0x08) = prev_now;
      } else {
        list.ptr(0x04) = prev_now;
      }
      node.ptr(0x0c) = nullptr;
      node.ptr(0x08) = nullptr;
      list.at<std::int32_t>(0x08) -= 1;
    }

    host.vtable_00_deleting_dtor(head, 1u);
  } while (list.at<std::int32_t>(0x08) != 0);
}

// 0x00874fb0 — CG_adjustor_thunk_00874fb0
void* cg_adjustor_thunk_00874fb0(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  return cg_scalar_deleting_dtor_00875010(host, static_cast<std::uint8_t*>(this_ptr) - 0x04, flags);
}

// 0x00875010 — CG_scalar_deleting_dtor_00875010
void* cg_scalar_deleting_dtor_00875010(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  void* base_subobj = (this_ptr != nullptr) ? (static_cast<std::uint8_t*>(this_ptr) + 0x04) : nullptr;
  host.global_u32(0x00f878d0) = 0;
  Raw{static_cast<std::uint8_t*>(base_subobj)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875070 — CG_adjustor_thunk_00875070
void* cg_adjustor_thunk_00875070(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  return cg_scalar_deleting_dtor_00875100(host, static_cast<std::uint8_t*>(this_ptr) - 0x04, flags);
}

// 0x00875100 — CG_scalar_deleting_dtor_00875100
void* cg_scalar_deleting_dtor_00875100(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  void* base_subobj = (this_ptr != nullptr) ? (static_cast<std::uint8_t*>(this_ptr) + 0x04) : nullptr;
  host.global_u32(0x00f878d4) = 0;
  Raw{static_cast<std::uint8_t*>(base_subobj)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875180 — CG_adjustor_thunk_00875180
void* cg_adjustor_thunk_00875180(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  return cg_scalar_deleting_dtor_008751b0(host, static_cast<std::uint8_t*>(this_ptr) - 0x04, flags);
}

// 0x008751b0 — CG_scalar_deleting_dtor_008751b0
void* cg_scalar_deleting_dtor_008751b0(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  void* base_subobj = (this_ptr != nullptr) ? (static_cast<std::uint8_t*>(this_ptr) + 0x04) : nullptr;
  host.global_u32(0x00f878d8) = 0;
  Raw{static_cast<std::uint8_t*>(base_subobj)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875210 — CG_adjustor_thunk_00875210
void* cg_adjustor_thunk_00875210(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  return cg_scalar_deleting_dtor_00875240(host, static_cast<std::uint8_t*>(this_ptr) - 0x04, flags);
}

// 0x00875240 — CG_scalar_deleting_dtor_00875240
void* cg_scalar_deleting_dtor_00875240(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  void* base_subobj = (this_ptr != nullptr) ? (static_cast<std::uint8_t*>(this_ptr) + 0x04) : nullptr;
  host.global_u32(0x00f878dc) = 0;
  Raw{static_cast<std::uint8_t*>(base_subobj)}.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875340 — BSP_FixedStepCallbackRegistry_GetSingleton
void* fixed_step_callback_registry_get_singleton_00875340(Seg54Host& host) {
  if (host.global_u32(0x00f878c8) == 0u) {
    void* mgr = host.singleton_lifetime_get_manager_00415350();
    void* lock = Raw{static_cast<std::uint8_t*>(mgr)}.ptr(0x10);
    if (lock != nullptr) {
      host.enter_critical_section_00ce2218(lock);
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) += 1;
    }

    if (host.global_u32(0x00f878c8) == 0u) {
      void* mem = host.operator_new_00bf681b(0x08u);
      void* created = (mem != nullptr) ? fixed_step_callback_registry_construct_00874c40(host, mem) : nullptr;
      host.global_u32(0x00f878c8) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(created));

      void* mgr2 = host.singleton_lifetime_get_manager_00415350();
      void* reg_obj = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878c8)));
      host.singleton_lifetime_register_00bd0c30(mgr2, reg_obj);
    }

    if (lock != nullptr) {
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) -= 1;
      host.leave_critical_section_00ce2210(lock);
    }
  }
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878c8)));
}

// 0x00875420 — BSP_FixedStepCallbackRegistry_SingletonHolderDtor
void fixed_step_callback_registry_singleton_holder_dtor_00875420(Seg54Host& host, void* this_ptr) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  self.ptr(0x00) = host.image_address(0x00d0dea4);
  host.critical_section_destroy_owned_0041cc80(self.p + 0x04);
  host.global_u32(0x00f878c8) = 0;
  self.ptr(0x00) = host.image_address(0x00ce3818);
}

// 0x00875450 — CG_scalar_deleting_dtor_00875450
void* cg_scalar_deleting_dtor_00875450(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  self.ptr(0x00) = host.image_address(0x00d0dea4);
  host.critical_section_destroy_owned_0041cc80(self.p + 0x04);
  host.global_u32(0x00f878c8) = 0;
  self.ptr(0x00) = host.image_address(0x00ce3818);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875490 — BSP_UnitTickElement_Destruct
void unit_tick_element_destruct_00875490(Seg54Host& host, void* this_ptr) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  void* prev1 = self.ptr(0x04);
  void* next1 = self.ptr(0x08);
  self.ptr(0x00) = host.image_address(0x00d0dec8);
  Raw{static_cast<std::uint8_t*>(prev1)}.ptr(0x08) = next1;

  void* next2 = self.ptr(0x08);
  void* prev2 = self.ptr(0x04);
  Raw{static_cast<std::uint8_t*>(next2)}.ptr(0x04) = prev2;
  unit_tick_element_clear_sub_list_00874f00(host, self.p + 0x1c);
}

// 0x008754d0 — BSP_StepJobWave3_GetDescriptor
void* step_job_wave3_get_descriptor_008754d0(Seg54Host& host) {
  if (host.global_u32(0x00f878d0) == 0u) {
    void* mgr = host.singleton_lifetime_get_manager_00415350();
    void* lock = Raw{static_cast<std::uint8_t*>(mgr)}.ptr(0x10);
    if (lock != nullptr) {
      host.enter_critical_section_00ce2218(lock);
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) += 1;
    }

    if (host.global_u32(0x00f878d0) == 0u) {
      void* mem = host.operator_new_00bf681b(0x08u);
      if (mem != nullptr) {
        Raw obj{static_cast<std::uint8_t*>(mem)};
        obj.ptr(0x04) = host.image_address(0x00d0de90);
        obj.ptr(0x00) = host.image_address(0x00d0deac);
        obj.ptr(0x04) = host.image_address(0x00d0dea8);
        host.global_u32(0x00f878d0) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(mem));
      } else {
        host.global_u32(0x00f878d0) = 0u;
      }

      void* cur = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878d0)));
      void* reg_subobj = (cur != nullptr) ? (static_cast<std::uint8_t*>(cur) + 0x04) : nullptr;
      void* mgr2 = host.singleton_lifetime_get_manager_00415350();
      host.singleton_lifetime_register_00bd0c30(mgr2, reg_subobj);
    }

    if (lock != nullptr) {
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) -= 1;
      host.leave_critical_section_00ce2210(lock);
    }
  }
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878d0)));
}

// 0x008755a0 — BSP_StepJobWave1_GetDescriptor
void* step_job_wave1_get_descriptor_008755a0(Seg54Host& host) {
  if (host.global_u32(0x00f878d4) == 0u) {
    void* mgr = host.singleton_lifetime_get_manager_00415350();
    void* lock = Raw{static_cast<std::uint8_t*>(mgr)}.ptr(0x10);
    if (lock != nullptr) {
      host.enter_critical_section_00ce2218(lock);
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) += 1;
    }

    if (host.global_u32(0x00f878d4) == 0u) {
      void* mem = host.operator_new_00bf681b(0x08u);
      if (mem != nullptr) {
        Raw obj{static_cast<std::uint8_t*>(mem)};
        obj.ptr(0x04) = host.image_address(0x00d0de94);
        obj.ptr(0x00) = host.image_address(0x00d0deb4);
        obj.ptr(0x04) = host.image_address(0x00d0deb0);
        host.global_u32(0x00f878d4) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(mem));
      } else {
        host.global_u32(0x00f878d4) = 0u;
      }

      void* cur = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878d4)));
      void* reg_subobj = (cur != nullptr) ? (static_cast<std::uint8_t*>(cur) + 0x04) : nullptr;
      void* mgr2 = host.singleton_lifetime_get_manager_00415350();
      host.singleton_lifetime_register_00bd0c30(mgr2, reg_subobj);
    }

    if (lock != nullptr) {
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) -= 1;
      host.leave_critical_section_00ce2210(lock);
    }
  }
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878d4)));
}

// 0x00875750 — BSP_StepJobWave2_GetDescriptor
void* step_job_wave2_get_descriptor_00875750(Seg54Host& host) {
  if (host.global_u32(0x00f878dc) == 0u) {
    void* mgr = host.singleton_lifetime_get_manager_00415350();
    void* lock = Raw{static_cast<std::uint8_t*>(mgr)}.ptr(0x10);
    if (lock != nullptr) {
      host.enter_critical_section_00ce2218(lock);
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) += 1;
    }

    if (host.global_u32(0x00f878dc) == 0u) {
      void* mem = host.operator_new_00bf681b(0x08u);
      if (mem != nullptr) {
        Raw obj{static_cast<std::uint8_t*>(mem)};
        obj.ptr(0x04) = host.image_address(0x00d0de9c);
        obj.ptr(0x00) = host.image_address(0x00d0dec4);
        obj.ptr(0x04) = host.image_address(0x00d0dec0);
        host.global_u32(0x00f878dc) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(mem));
      } else {
        host.global_u32(0x00f878dc) = 0u;
      }

      void* cur = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878dc)));
      void* reg_subobj = (cur != nullptr) ? (static_cast<std::uint8_t*>(cur) + 0x04) : nullptr;
      void* mgr2 = host.singleton_lifetime_get_manager_00415350();
      host.singleton_lifetime_register_00bd0c30(mgr2, reg_subobj);
    }

    if (lock != nullptr) {
      Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) -= 1;
      host.leave_critical_section_00ce2210(lock);
    }
  }
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00f878dc)));
}

// 0x00875820 — BSP_PendingRegistry_SingletonHolderDtor
void pending_registry_singleton_holder_dtor_00875820(Seg54Host& host, void* this_ptr) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  self.ptr(0x00) = host.image_address(0x00d0dea0);
  host.critical_section_destroy_owned_0041cc80(self.p + 0x04);
  host.global_u32(0x00f878cc) = 0;
  self.ptr(0x00) = host.image_address(0x00ce3818);
}

// 0x00875920 — BSP_TickElement_ScalarDeletingDtor
void* tick_element_scalar_deleting_dtor_00875920(Seg54Host& host, void* this_ptr, std::uint8_t flags) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  void* prev1 = self.ptr(0x04);
  void* next1 = self.ptr(0x08);
  self.ptr(0x00) = host.image_address(0x00d0dec8);
  Raw{static_cast<std::uint8_t*>(prev1)}.ptr(0x08) = next1;

  void* next2 = self.ptr(0x08);
  void* prev2 = self.ptr(0x04);
  Raw{static_cast<std::uint8_t*>(next2)}.ptr(0x04) = prev2;
  unit_tick_element_clear_sub_list_00874f00(host, self.p + 0x1c);

  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(this_ptr);
  }
  return this_ptr;
}

// 0x00875ae0 — BSP_FixedStepCallback_Unregister
void fixed_step_callback_unregister_00875ae0(Seg54Host& host, void* this_ptr) {
  void* registry = fixed_step_callback_registry_get_singleton_00875340(host);
  void* lock = Raw{static_cast<std::uint8_t*>(registry)}.ptr(0x04);
  if (lock != nullptr) {
    host.enter_critical_section_00ce2218(lock);
    Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) += 1;
  }

  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  void* prev1 = self.ptr(0x0c);
  void* next1 = self.ptr(0x10);
  Raw{static_cast<std::uint8_t*>(prev1)}.ptr(0x10) = next1;

  void* next2 = self.ptr(0x10);
  void* prev2 = self.ptr(0x0c);
  Raw{static_cast<std::uint8_t*>(next2)}.ptr(0x0c) = prev2;
  self.at<std::uint8_t>(0x18) = 0u;
  self.at<std::uint8_t>(0x19) = 0u;

  if (lock != nullptr) {
    Raw{static_cast<std::uint8_t*>(lock)}.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(lock);
  }
}

// 0x008761e0 — BSP_UnitInstance_SetNumbering
void unit_instance_set_numbering_008761e0(Seg54Host& host, void* this_ptr, std::int32_t number) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  if (self.at<std::int32_t>(0x35c) == number) {
    return;
  }

  const bool has_model = (self.ptr(0x360) != nullptr);
  self.at<std::int32_t>(0x35c) = number;
  if (!has_model) {
    return;
  }

  if (host.vtable_5c_is_kind_of(this_ptr, 6) == 0u && host.vtable_5c_is_kind_of(this_ptr, 0x1b) == 0u) {
    return;
  }

  host.model_instance_set_numbering_00711be0(self.ptr(0x360), number);
}

// 0x00876e20 — FUN_00876e20
void* unit_model_group_find_entry_00876e20(
  Seg54Host& host,
  void* this_ptr,
  std::int32_t key_a,
  std::int32_t key_b
) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  std::uint32_t index = 0u;
  std::size_t byte_offset = 0u;

  for (;;) {
    std::uint8_t* begin = static_cast<std::uint8_t*>(self.ptr(0x1c));
    std::uint8_t* end = static_cast<std::uint8_t*>(self.ptr(0x20));
    const std::uint32_t count =
      (begin != nullptr) ? static_cast<std::uint32_t>((end - begin) / 0x30) : 0u;

    if (begin == nullptr || index >= count) {
      std::uint8_t* begin_fallback = static_cast<std::uint8_t*>(self.ptr(0x1c));
      std::uint8_t* end_fallback = static_cast<std::uint8_t*>(self.ptr(0x20));
      if (begin_fallback == nullptr || ((end_fallback - begin_fallback) / 0x30) == 0) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      return self.ptr(0x1c);
    }

    std::uint8_t* begin_chk = static_cast<std::uint8_t*>(self.ptr(0x1c));
    std::uint8_t* end_chk = static_cast<std::uint8_t*>(self.ptr(0x20));
    if (begin_chk == nullptr || index >= static_cast<std::uint32_t>((end_chk - begin_chk) / 0x30)) {
      host.invalid_parameter_noinfo_00bf6713();
    }

    std::uint8_t* entry = static_cast<std::uint8_t*>(self.ptr(0x1c)) + byte_offset;
    if (Raw{entry}.at<std::int32_t>(0x04) == key_a && Raw{entry}.at<std::int32_t>(0x08) == key_b) {
      return entry;
    }

    index += 1u;
    byte_offset += 0x30u;
  }
}

// 0x008776e0 — BSP_UnitTickableEntity_WriteState
void unit_tickable_entity_write_state_008776e0(Seg54Host& host, void* this_ptr, void* writer) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  host.net_entity_write_replication_state_0077c160(this_ptr, writer);

  host.writer_vtable_04_begin_table(
    writer,
    0u,
    static_cast<const char*>(host.image_address(0x00d0def8))
  );

  const std::int32_t gfx_level = self.at<std::int32_t>(0x358);
  if (gfx_level != 0) {
    const std::uint32_t current_tagged[2] = {1u, static_cast<std::uint32_t>(gfx_level)};
    const std::uint32_t default_tagged[2] = {1u, 0u};
    if (host.tagged_value_equals_00bd5680(current_tagged, default_tagged) == 0u) {
      host.writer_vtable_0c_write_tagged(
        writer,
        0u,
        static_cast<const char*>(host.image_address(0x00d0dee8)),
        current_tagged[0],
        current_tagged[1]
      );
    }
  }

  host.writer_vtable_04_begin_table(
    writer,
    0u,
    static_cast<const char*>(host.image_address(0x00cfd7c8))
  );

  for (std::uint32_t i = 0u;; ++i) {
    std::uint8_t* begin = static_cast<std::uint8_t*>(self.ptr(0x348));
    std::uint8_t* end = static_cast<std::uint8_t*>(self.ptr(0x34c));
    if (begin == nullptr || i >= static_cast<std::uint32_t>((end - begin) >> 2)) {
      break;
    }
    std::uint8_t* end_chk = static_cast<std::uint8_t*>(self.ptr(0x34c));
    if (begin == nullptr || i >= static_cast<std::uint32_t>((end_chk - begin) >> 2)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
  }

  host.writer_vtable_08_end_table(writer);
  host.writer_vtable_08_end_table(writer);
}

// 0x00877b00 — BSP_UnitTickableEntity_SetDamagedGfxLevel
void unit_tickable_entity_set_damaged_gfx_level_00877b00(
  Seg54Host& host,
  void* this_ptr,
  std::int32_t level
) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  if (self.at<std::int32_t>(0x358) == level) {
    return;
  }

  host.unit_instance_set_model_group_state_00876ec0(this_ptr, level);

  void* game = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
  if (Raw{static_cast<std::uint8_t*>(game)}.at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg_storage[0x24]{};
    host.session_message_construct_base_0075b430(msg_storage, 0x56u);
    Raw msg{msg_storage};
    msg.at<std::uint16_t>(0x18) = 0u;
    msg.at<std::uint8_t>(0x1a) = 0u;
    msg.at<std::uint32_t>(0x04) = 1u;
    msg.at<std::uint8_t>(0x1c) = 0u;
    msg.ptr(0x00) = host.image_address(0x00d02d6c);
    msg.at<std::int32_t>(0x20) = level;
    host.session_owner_broadcast_message_0077c7b0(this_ptr, msg_storage, nullptr);
  }
}

// 0x00877cd0 — BSP_UnitInstance_SetHealthFromScale255
void unit_instance_set_health_from_scale255_00877cd0(
  Seg54Host& host,
  void* this_ptr,
  std::int32_t scale_val
) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  double val = static_cast<double>(scale_val);
  if (scale_val < 0) {
    val += static_cast<double>(host.global_f32(0x00ce3978));
  }
  const float scaled_health = static_cast<float>(
    (val / host.global_f64(0x00ce4b48)) * static_cast<double>(self.at<float>(0x36c))
  );
  host.unit_instance_set_health_00877b90(this_ptr, scaled_health);
}

// 0x008781f0 — BSP_UnitTickableEntity_SetSceneNodesVisible
void unit_tickable_entity_set_scene_nodes_visible_008781f0(
  Seg54Host& host,
  void* this_ptr,
  std::uint8_t visible
) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  for (std::uint32_t i = 0u;; ++i) {
    void** begin = static_cast<void**>(self.ptr(0x380));
    void** end = static_cast<void**>(self.ptr(0x384));
    if (begin == nullptr || i >= static_cast<std::uint32_t>(end - begin)) {
      break;
    }

    const float factor = (visible != 0u) ? host.global_f32(0x00d7a24c) : 0.0f;
    void** end_chk = static_cast<void**>(self.ptr(0x384));
    if (begin == nullptr || i >= static_cast<std::uint32_t>(end_chk - begin)) {
      host.invalid_parameter_noinfo_00bf6713();
    }

    void** cur_begin = static_cast<void**>(self.ptr(0x380));
    void* entry = cur_begin[i];
    void* scene_node = Raw{static_cast<std::uint8_t*>(entry)}.ptr(0x0c);
    host.scene_node_set_visibility_factor_00b6da70(scene_node, factor, 0u);
  }

  for (void* child = self.ptr(0x48); child != nullptr;
       child = Raw{static_cast<std::uint8_t*>(child)}.ptr(0x44)) {
    if (host.vtable_5c_is_kind_of(child, 4) != 0u) {
      unit_tickable_entity_set_scene_nodes_visible_008781f0(host, child, visible);
    }
  }
}

// 0x008782a0 — BSP_UnitTickableEntity_FindDamageSection
void** unit_tickable_entity_find_damage_section_008782a0(
  Seg54Host& host,
  void* this_ptr,
  std::int32_t key_a,
  std::int32_t key_b
) {
  Raw self{static_cast<std::uint8_t*>(this_ptr)};
  std::uint8_t* vec_obj = self.p + 0x344;
  void** cur = static_cast<void**>(self.ptr(0x348));
  if (cur > static_cast<void**>(self.ptr(0x34c))) {
    host.invalid_parameter_noinfo_00bf6713();
  }

  for (;;) {
    void** end_ptr = static_cast<void**>(self.ptr(0x34c));
    if (static_cast<void**>(self.ptr(0x348)) > end_ptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (vec_obj == nullptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (cur == end_ptr) {
      return nullptr;
    }
    if (vec_obj == nullptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (cur >= static_cast<void**>(self.ptr(0x34c))) {
      host.invalid_parameter_noinfo_00bf6713();
    }

    void* elem = *cur;
    if (Raw{static_cast<std::uint8_t*>(elem)}.at<std::int32_t>(0x04) == key_a) {
      if (cur >= static_cast<void**>(self.ptr(0x34c))) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      void* elem2 = *cur;
      if (Raw{static_cast<std::uint8_t*>(elem2)}.at<std::int32_t>(0x08) == key_b) {
        if (cur >= static_cast<void**>(self.ptr(0x34c))) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        return cur;
      }
    }

    if (cur >= static_cast<void**>(self.ptr(0x34c))) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    ++cur;
  }
}

}  // namespace bsp::sim_damage_seg54_argon
