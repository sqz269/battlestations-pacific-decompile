#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <cstring>

namespace bsp::entity_mission_seg60_argon {

namespace {
template<class T> T& nb_high_tree_field(void* owner, std::size_t offset) {
  return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(owner) + offset);
}
}
bool entity_visit_tree_009040d0(void* visitor, void* entity, std::uint32_t a, std::uint32_t b,
                              void* excluded, EntityTreeWalkCalls& calls) {
  if (entity == excluded || nb_high_tree_field<std::uint32_t>(entity, 0xb8) == 0) return false;
  bool result = calls.visit_slote4(entity, a, b);
  for (void* child = nb_high_tree_field<void*>(entity, 0x48); child; child = nb_high_tree_field<void*>(child, 0x44)) {
    const bool child_result = entity_visit_tree_009040d0(visitor, child, a, b, excluded, calls);
    result = result | child_result;
  }
  return result;
}
void entity_set_float_recursive_00922ee0(void* entity, float value, EntityTreeWalkCalls& calls) {
  calls.set_slotdc(entity, value);
  for (void* child = nb_high_tree_field<void*>(entity, 0x48); child; child = nb_high_tree_field<void*>(child, 0x44))
    entity_set_float_recursive_00922ee0(child, value, calls);
}
void entity_format_tree_00923af0(void* entity, EntityTreeWalkCalls& calls) {
  if (!calls.is_kind_slot5c(entity, 1)) return;
  char buffer[1024];
  calls.format_0040cfc0(buffer, "id=%d type=%s childnum=%d", nb_high_tree_field<std::uint16_t>(entity, 0x174),
    calls.type_name_00e0cd80(nb_high_tree_field<std::uint32_t>(entity, 0xc4)),
    nb_high_tree_field<std::uint32_t>(entity, 0x50));
  void* selected = nb_high_tree_field<void*>(entity, 0x48);
  while (selected && !calls.is_kind_slot5c(selected, 1)) selected = nb_high_tree_field<void*>(selected, 0x44);
  if (!selected) return;
  for (void* child = nb_high_tree_field<void*>(entity, 0x48); child; child = nb_high_tree_field<void*>(child, 0x44))
    entity_format_tree_00923af0(child, calls);
}
bool entity_predicate_true_008fb780() noexcept { return true; }

// 0x00922b10: BSP_Entity_IsAirborneTarget
// Original ABI: int __fastcall(ECX = entity), RET 0.
std::int32_t entity_is_airborne_target_00922b10(Seg60Host& host, void* entity) {
  if (entity == nullptr) {
    return 0;
  }
  Raw r{static_cast<std::uint8_t*>(entity)};
  if (r.at<std::uint8_t>(0x5d) != 0) {
    return 0;
  }
  if (!host.vcall_is_kind_of_slot5c(entity, 0x0f) &&
      !host.vcall_is_kind_of_slot5c(entity, 0x18)) {
    return 0;
  }
  return 1;
}

// 0x00922b70: BSP_Entity_CastToSubmarine
// Original ABI: void* __fastcall(ECX = entity), RET 0.
void* entity_cast_to_submarine_00922b70(Seg60Host& host, void* entity) {
  if (entity != nullptr && host.vcall_is_kind_of_slot5c(entity, 8)) {
    return entity;
  }
  return nullptr;
}

// 0x00922b90: BSP_Entity_CastToLandFort
// Original ABI: void* __fastcall(ECX = entity), RET 0.
void* entity_cast_to_land_fort_00922b90(Seg60Host& host, void* entity) {
  if (entity != nullptr && host.vcall_is_kind_of_slot5c(entity, 0x1b)) {
    return entity;
  }
  return nullptr;
}

// 0x00922c80: BSP_Entity_IsSurfaceTarget
// Original ABI: uint8_t __fastcall(ECX = entity, DL = allow_far), RET 0.
std::uint8_t entity_is_surface_target_00922c80(
    Seg60Host& host,
    void* entity,
    std::uint8_t allow_far
) {
  if (entity == nullptr) {
    return 0;
  }
  Raw r{static_cast<std::uint8_t*>(entity)};
  if (r.at<std::uint8_t>(0x5d) != 0 ||
      host.vcall_is_kind_of_slot5c(entity, 0x0f) ||
      host.vcall_is_kind_of_slot5c(entity, 0x18)) {
    return 0;
  }
  if (host.vcall_is_kind_of_slot5c(entity, 6)) {
    void* sub = entity_cast_to_submarine_00922b70(host, entity);
    if (sub != nullptr) {
      void* settings = host.game_settings_get_singleton_00424c40();
      float* pos = host.entity_pose_get_world_position_refreshed_00427eb0(sub);
      Raw s_raw{static_cast<std::uint8_t*>(settings)};
      const float threshold = static_cast<float>(
          -static_cast<double>(s_raw.at<float>(0x49c)) * host.global_f64(0x00d7a348)
      );
      if (!(pos[1] > threshold)) {  // 00922D08 JBE also rejects unordered.
        return 0;
      }
    }
    return 1;
  }

  if (host.vcall_is_kind_of_slot5c(entity, 0x45) ||
      host.vcall_is_kind_of_slot5c(entity, 0x46) ||
      host.vcall_is_kind_of_slot5c(entity, 0x1c)) {
    return 1;
  }

  Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
  const std::int32_t slot = game.at<std::int32_t>(0x18ec);
  void* szurke_nyil = game.ptr(0x21a4 + static_cast<std::size_t>(slot) * 4u);
  if (host.szurke_nyil_contains_unit_008ddf90(szurke_nyil, entity)) {
    return 1;
  }

  if (allow_far == 0) {
    return 0;
  }

  if (host.vcall_is_kind_of_slot5c(entity, 0x35)) {
    float* pos = host.entity_pose_get_world_position_refreshed_00427eb0(entity);
    if (!(static_cast<double>(pos[1]) > host.global_f64(0x00ce3938))) {  // 00922D84 JBE.
      return 0;
    }
    return 1;
  }

  void* fort = entity_cast_to_land_fort_00922b90(host, entity);
  if (fort == nullptr) {
    return 0;
  }
  void* fort_cls = Raw{static_cast<std::uint8_t*>(fort)}.ptr(0x538);
  if (Raw{static_cast<std::uint8_t*>(fort_cls)}.at<std::int32_t>(0x178) != 0x1b) {
    return 0;
  }
  return 1;
}

// 0x00922e20: BSP_ScenePropertyBagRef_Construct
// Original ABI: void* __thiscall(ECX = self, [ESP+4] = src_bag), RET 4.
void* scene_property_bag_ref_construct_00922e20(
    Seg60Host& host,
    void* self,
    void* src_bag
) {
  Raw r{static_cast<std::uint8_t*>(self)};
  r.ptr(0x00) = host.image_address(0x00d03d94);
  r.ptr(0x08) = host.scene_property_bag_clone_008f41f0(src_bag);
  r.at<std::int32_t>(0x04) = 1;
  return self;
}

// 0x00922e90: BSP_Entity_FindAncestorOfKind
// Original ABI: void* __thiscall(ECX = entity, [ESP+4] = kind), RET 4.
void* entity_find_ancestor_of_kind_00922e90(
    Seg60Host& host,
    void* entity,
    std::int32_t kind
) {
  void* cur = Raw{static_cast<std::uint8_t*>(entity)}.ptr(0x3c);
  while (cur != nullptr && !host.vcall_is_kind_of_slot5c(cur, kind)) {
    cur = Raw{static_cast<std::uint8_t*>(cur)}.ptr(0x3c);
  }
  return cur;
}

// 0x00923020: TRIV_body_00923020 (RET 4)
void triv_body_00923020(void* /*self*/, std::uint32_t /*unused*/) noexcept {}

// 0x00923030: TRIV_body_00923030 (RET 4)
void triv_body_00923030(void* /*self*/, std::uint32_t /*unused*/) noexcept {}

// 0x00923040: TRIV_body_00923040 (RET)
void triv_body_00923040(void* /*self*/) noexcept {}

// 0x00923050: TRIV_body_00923050 (RET)
void triv_body_00923050(void* /*self*/) noexcept {}

// 0x00923060: TRIV_body_00923060 (RET)
void triv_body_00923060(void* /*self*/) noexcept {}

// 0x00923080: TRIV_body_00923080
// Original ABI: const char* __fastcall(ECX = race_index), RET 0.
const char* race_name_from_index_00923080(Seg60Host& host, std::int32_t race_index) {
  const std::uint32_t addr = 0x00e0cf24u + static_cast<std::uint32_t>(race_index) * 4u;
  return reinterpret_cast<const char*>(host.global_u32(addr));
}

// 0x00923290: TMPL_OwnedPtr_DeleteReset_00923290
// Original ABI: void __fastcall(ECX = slot_ptr), RET 0.
void owned_ptr_delete_reset_00923290(Seg60Host& host, void* slot_ptr) {
  Raw slot{static_cast<std::uint8_t*>(slot_ptr)};
  void* target = slot.ptr(0x00);
  if (target != nullptr) {
    host.vcall_scalar_deleting_dtor_slot00(target, 1);
    slot.ptr(0x00) = nullptr;
  }
}

// 0x00923840: BSP_Entity_InheritRaceAndActivate
// Original ABI: void __fastcall(ECX = entity), RET 0.
void entity_inherit_race_and_activate_00923840(void* entity) noexcept {
  Raw r{static_cast<std::uint8_t*>(entity)};
  if (r.at<std::int32_t>(0x58) < 0 && r.ptr(0x3c) != nullptr) {
    Raw parent{static_cast<std::uint8_t*>(r.ptr(0x3c))};
    r.at<std::int32_t>(0x58) = parent.at<std::int32_t>(0x58);
  }
  r.at<std::uint8_t>(0x5c) = 1;
  r.at<std::uint8_t>(0xbd) = 1;
}

// 0x00923860: TRIV_body_00923860 (RET)
void triv_body_00923860(void* /*self*/) noexcept {}

// 0x009238a0: SEntity_GetSavedEntityLuaData
// Original ABI: LuaObject* __thiscall(ECX = entity, [ESP+4] = out_lua_obj), RET 4.
void* sentity_get_saved_entity_lua_data_009238a0(
    Seg60Host& host,
    void* entity,
    void* out_lua_obj
) {
  std::uint8_t local_globals[20]{};
  std::uint8_t local_savedata[20]{};
  std::uint8_t local_entities[20]{};

  std::uint8_t* game = reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8));
  void* globals_obj = host.lua_state_owner_get_globals_00b67980(game + 0x1a0c, local_globals);
  void* savedata_obj = host.lua_object_get_by_name_00b67800(
      globals_obj,
      local_savedata,
      static_cast<const char*>(host.image_address(0x00cf8250))
  );
  void* entities_obj = host.lua_object_get_by_name_00b67800(
      savedata_obj,
      local_entities,
      static_cast<const char*>(host.image_address(0x00cf825c))
  );
  Raw r{static_cast<std::uint8_t*>(entity)};
  Raw holder{static_cast<std::uint8_t*>(r.ptr(0xc0))};
  host.lua_object_get_by_index_00b67720(entities_obj, out_lua_obj, holder.at<std::int32_t>(0x08));

  host.lua_object_destruct_00b67700(local_entities);
  host.lua_object_destruct_00b67700(local_savedata);
  host.lua_object_destruct_00b67700(local_globals);
  return out_lua_obj;
}

// 0x00924280: BSP_SceneProperty_CopyFloat16
// Original ABI: float* __thiscall(ECX = prop_record, [ESP+4] = dst16), RET 4.
float* scene_property_copy_float16_00924280(
    void* prop_record,
    float* dst16
) noexcept {
  const float* src = Raw{static_cast<std::uint8_t*>(prop_record)}.at<const float*>(0x20);
  // Sixteen FLD/FSTP DWORD copies, which quiet a signalling NaN (cc13-fixture10).
  for (std::size_t i = 0; i < 16; ++i) {
    std::uint32_t bits;
    std::memcpy(&bits, src + i, 4);
    bits = cc13x87::x87_fld_fstp_bits(bits);
    std::memcpy(dst16 + i, &bits, 4);
  }
  return dst16;
}

// 0x00924990: BSP_DeferredEntityEventLock_Get
// Original ABI: void* __cdecl(), RET 0.
void* deferred_entity_event_lock_get_00924990(Seg60Host& host) {
  if (host.global_u32(0x00f899ec) == 0) {
    void* mgr = host.singleton_lifetime_get_manager_00415350();
    void* cs = Raw{static_cast<std::uint8_t*>(mgr)}.ptr(0x10);
    if (cs != nullptr) {
      host.enter_critical_section_00ce2218(cs);
      Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) += 1;
    }
    if (host.global_u32(0x00f899ec) == 0) {
      void* mem = host.operator_new_00bf681b(8);
      void* constructed = nullptr;
      if (mem != nullptr) {
        constructed = host.deferred_entity_event_lock_construct_00924200(mem);
      }
      host.global_u32(0x00f899ec) = reinterpret_cast<std::uint32_t>(constructed);
      void* reg_mgr = host.singleton_lifetime_get_manager_00415350();
      host.singleton_lifetime_register_00bd0c30(
          reg_mgr,
          reinterpret_cast<void*>(host.global_u32(0x00f899ec))
      );
    }
    if (cs != nullptr) {
      Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) -= 1;
      host.leave_critical_section_00ce2210(cs);
    }
  }
  return reinterpret_cast<void*>(host.global_u32(0x00f899ec));
}

// 0x00925050: BSP_HitRecord_Assign
// Original ABI: void* __thiscall(ECX = dst_record, [ESP+4] = src_record), RET 4.
void* hit_record_assign_00925050(Seg60Host& host, void* dst_record, const void* src_record) {
  Raw dst{static_cast<std::uint8_t*>(dst_record)};
  Raw src{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(src_record))};

  dst.at<std::uint32_t>(0x00) = src.at<std::uint32_t>(0x00);
  dst.at<std::uint32_t>(0x04) = src.at<std::uint32_t>(0x04);
  dst.at<float>(0x08) = src.at<float>(0x08);
  dst.at<float>(0x0c) = src.at<float>(0x0c);
  dst.at<float>(0x10) = src.at<float>(0x10);
  dst.at<float>(0x14) = src.at<float>(0x14);
  dst.at<float>(0x18) = src.at<float>(0x18);
  dst.at<float>(0x1c) = src.at<float>(0x1c);
  dst.at<float>(0x20) = src.at<float>(0x20);
  dst.at<float>(0x24) = src.at<float>(0x24);
  dst.at<float>(0x28) = src.at<float>(0x28);
  dst.at<std::uint8_t>(0x2c) = src.at<std::uint8_t>(0x2c);
  dst.at<std::uint32_t>(0x30) = src.at<std::uint32_t>(0x30);
  dst.at<std::uint32_t>(0x34) = src.at<std::uint32_t>(0x34);
  dst.at<std::uint32_t>(0x38) = src.at<std::uint32_t>(0x38);

  if (dst.at<std::uint32_t>(0x40) != 0) {
    dst.at<std::uint32_t>(0x40) = 0;
  }

  const std::uint32_t src_count = src.at<std::uint32_t>(0x40);
  if (dst.at<std::uint32_t>(0x44) < src_count) {
    dst.at<std::uint32_t>(0x44) = src_count;
    auto* new_arr = static_cast<std::uint32_t*>(host.operator_new_00bf55be(src_count << 4u));
    const std::uint32_t cur_count = dst.at<std::uint32_t>(0x40);
    if (cur_count != 0) {
      const auto* old_arr = static_cast<const std::uint32_t*>(dst.ptr(0x3c));
      for (std::uint32_t i = 0; i < cur_count; ++i) {
        std::uint32_t* d_elem = new_arr + static_cast<std::size_t>(i) * 4u;
        if (d_elem != nullptr) {
          const std::uint32_t* s_elem = old_arr + static_cast<std::size_t>(i) * 4u;
          d_elem[0] = s_elem[0];
          d_elem[1] = s_elem[1];
          d_elem[2] = s_elem[2];
          d_elem[3] = s_elem[3];
        }
      }
    }
    if (dst.ptr(0x3c) != nullptr) {
      host.free_00bf6989(dst.ptr(0x3c));
    }
    dst.ptr(0x3c) = new_arr;
  }

  dst.at<std::uint32_t>(0x40) = src_count;
  if (src_count != 0) {
    auto* dst_arr = static_cast<std::uint32_t*>(dst.ptr(0x3c));
    const auto* src_arr = static_cast<const std::uint32_t*>(src.ptr(0x3c));
    for (std::uint32_t i = 0; i < src_count; ++i) {
      std::uint32_t* d_elem = dst_arr + static_cast<std::size_t>(i) * 4u;
      if (d_elem != nullptr) {
        const std::uint32_t* s_elem = src_arr + static_cast<std::size_t>(i) * 4u;
        d_elem[0] = s_elem[0];
        d_elem[1] = s_elem[1];
        d_elem[2] = s_elem[2];
        d_elem[3] = s_elem[3];
      }
    }
  }

  dst.at<float>(0x48) = src.at<float>(0x48);
  dst.at<float>(0x4c) = src.at<float>(0x4c);
  dst.at<float>(0x50) = src.at<float>(0x50);
  return dst_record;
}

// 0x00925780: BSP_Entity_DestructBase
// Original ABI: void __fastcall(ECX = entity), RET 0.
void entity_destruct_base_00925780(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  r.ptr(0x00) = host.image_address(0x00d19120);
  r.ptr(0x10) = host.image_address(0x00d19104);
  r.ptr(0x24) = host.image_address(0x00d190fc);

  host.entity_life_ref_release_00740270(
      reinterpret_cast<void*>(host.global_u32(0x00e1aea0)),
      r.ptr(0x168)
  );

  while (r.at<std::int32_t>(0x50) != 0) {
    void* head_child = r.ptr(0x48);
    if (head_child != nullptr) {
      host.vcall_scalar_deleting_dtor_slot00(head_child, 1);
    }
  }

  Raw world_node{static_cast<std::uint8_t*>(r.ptr(0x30))};
  host.world_unlink_from_update_chain_00903f30(world_node.ptr(0x04), entity);

  void* parent = r.ptr(0x3c);
  if (parent != nullptr) {
    host.entity_unlink_from_sibling_chain_00924710(
        static_cast<std::uint8_t*>(parent) + 0x48,
        entity
    );
    r.ptr(0x3c) = nullptr;
  } else {
    Raw world_node_reloaded{static_cast<std::uint8_t*>(r.ptr(0x30))};
    host.entity_unlink_from_sibling_chain_00924710(world_node_reloaded.ptr(0x08), entity);
    if (r.at<std::int32_t>(0xb8) != 0) {
      host.unit_list_remove_004845a0(
           static_cast<std::uint8_t*>(r.ptr(0x30)) + 0x0c,
          entity
      );
    }
  }

  void* str_160 = r.ptr(0x160);
  if (str_160 != nullptr) {
    const std::uint32_t bytes = r.at<std::uint32_t>(0x15c) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, str_160, bytes, 1);
  }

  void* str_158 = r.ptr(0x158);
  if (str_158 != nullptr) {
    const std::uint32_t bytes = r.at<std::uint32_t>(0x154) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, str_158, bytes, 1);
  }

  host.weak_owner_destroy_00925540(r.p + 0x24);
  host.callback_owner_destroy_00695870(r.p + 0x10);
  host.observer_endpoint_destruct_00695760(entity);
}

// 0x009258d0: CG_adjustor_thunk_009258d0
// SUB ECX, 0x24; JMP 0x00925f00
void* cg_adjustor_thunk_009258d0(Seg60Host& host, void* subobj_at_24, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_24) - 0x24;
  return host.entity_scalar_deleting_dtor_00925f00(base, flags);
}

// 0x009258e0: CG_adjustor_thunk_009258e0
// SUB ECX, 0x10; JMP 0x00925f00
void* cg_adjustor_thunk_009258e0(Seg60Host& host, void* subobj_at_10, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_10) - 0x10;
  return host.entity_scalar_deleting_dtor_00925f00(base, flags);
}

// 0x00925a90: BSP_EntityRegistry_FindEntityByName
// Original ABI: void* __thiscall(ECX = registry, [ESP+4] = name), RET 4.
void* entity_registry_find_entity_by_name_00925a90(
    Seg60Host& host,
    void* registry,
    const char* name
) {
  Raw reg{static_cast<std::uint8_t*>(registry)};
  Raw party_list{static_cast<std::uint8_t*>(reg.ptr(0x08))};
  std::int32_t index = 0;
  if (party_list.at<std::int32_t>(0x08) > 0) {
    do {
      Raw cur_list{static_cast<std::uint8_t*>(reg.ptr(0x08))};
      void* party_node = nullptr;
      if (index >= 0 && index < cur_list.at<std::int32_t>(0x08)) {
        party_node = cur_list.ptr(0x00);
        for (std::int32_t step = index; party_node != nullptr && step != 0; --step) {
          party_node = Raw{static_cast<std::uint8_t*>(party_node)}.ptr(0x44);
        }
      }
      void* found = host.scene_node_find_by_qualified_name_009251f0(registry, party_node, name);
      if (found != nullptr) {
        return found;
      }
      ++index;
    } while (index < Raw{static_cast<std::uint8_t*>(reg.ptr(0x08))}.at<std::int32_t>(0x08));
  }
  return nullptr;
}

// 0x00925ce0: BSP_SceneNode_Construct
// Original ABI: void* __fastcall(ECX = self), RET 0.
void* scene_node_construct_00925ce0(Seg60Host& host, void* self) {
  Raw r{static_cast<std::uint8_t*>(self)};
  r.ptr(0x00) = host.image_address(0x00ceccc8);
  r.at<std::uint32_t>(0x04) = 0;
  r.at<std::uint32_t>(0x08) = 0;
  r.at<std::uint32_t>(0x0c) = 0;
  r.ptr(0x10) = host.image_address(0x00ce3cd4);
  r.at<std::uint32_t>(0x14) = 0;
  r.at<std::uint32_t>(0x18) = 0;
  r.at<std::uint32_t>(0x1c) = 0;
  r.at<std::uint8_t>(0x20) = 0;

  host.weak_owner_construct_00925490(r.p + 0x24);

  const float one = host.global_f32(0x00d7a24c);
  r.ptr(0x00) = host.image_address(0x00d19120);
  r.ptr(0x10) = host.image_address(0x00d19104);
  r.ptr(0x24) = host.image_address(0x00d190fc);
  r.at<std::uint32_t>(0x4c) = 0;
  r.at<std::uint32_t>(0x48) = 0;
  r.at<std::uint32_t>(0x50) = 0;

  float ident_a[16] = {
      one,  0.0f, 0.0f, 0.0f,
      0.0f, one,  0.0f, 0.0f,
      0.0f, 0.0f, one,  0.0f,
      0.0f, 0.0f, 0.0f, one,
  };
  host.matrix_copy_4x4_x87_004134f0(r.p + 0x74, ident_a);

  r.at<std::uint32_t>(0x154) = 0;
  r.at<std::uint32_t>(0x158) = 0;
  r.at<std::uint32_t>(0x15c) = 0;
  r.at<std::uint32_t>(0x160) = 0;

  const float one_b = host.global_f32(0x00d7a24c);
  r.at<std::uint32_t>(0xc0) = 0;
  r.at<std::uint32_t>(0xc4) = 0;
  r.at<std::uint8_t>(0xbd) = 0;
  r.at<std::uint8_t>(0x60) = 0;
  r.at<std::uint8_t>(0x5f) = 0;
  r.at<std::uint8_t>(0x5c) = 0;
  r.at<std::uint8_t>(0x5e) = 0;
  r.at<std::uint8_t>(0x5d) = 0;
  r.at<std::uint32_t>(0x6c) = 0;
  r.at<std::uint32_t>(0x70) = 0;
  r.at<std::uint32_t>(0x54) = 2;
  r.at<std::uint32_t>(0x58) = 0xffffffffu;
  r.at<std::uint8_t>(0xbc) = 0;
  r.at<float>(0x150) = 0.0f;

  float ident_b[16] = {
      one_b, 0.0f,  0.0f,  0.0f,
      0.0f,  one_b, 0.0f,  0.0f,
      0.0f,  0.0f,  one_b, 0.0f,
      0.0f,  0.0f,  0.0f,  one_b,
  };
  host.matrix_copy_4x4_x87_004134f0(r.p + 0x74, ident_b);

  host.native_string_resize_0041dd40(r.p + 0x154, 0, 0);
  if (r.ptr(0x158) != nullptr) {
    std::memcpy(r.ptr(0x158), nullptr, r.at<std::uint32_t>(0x154));
  }

  r.at<std::uint32_t>(0x16c) = 0;
  r.at<std::uint32_t>(0x168) = 0;
  r.at<std::uint8_t>(0x61) = 0;
  r.at<std::uint8_t>(0x62) = 0;
  r.at<std::uint8_t>(0x68) = 0;
  r.at<float>(0x64) = 0.0f;
  return self;
}

// 0x00926e80: BSP_DeferredEntityEventQueue_Push
// Original ABI: void __fastcall(ECX = hit_record, EDX = direction_xyz), RET 0.
void deferred_entity_event_queue_push_00926e80(
    Seg60Host& host,
    void* hit_record,
    const float* direction_xyz
) {
  void* lock_owner = deferred_entity_event_lock_get_00924990(host);
  void* cs = Raw{static_cast<std::uint8_t*>(lock_owner)}.ptr(0x04);
  if (cs != nullptr) {
    host.enter_critical_section_00ce2218(cs);
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) += 1;
  }

  host.global_u8(0x00e18684) = 1;
  std::uint8_t temp_record[0x60]{};
  host.hit_record_reset_00470470(temp_record);

  void* list_this = host.image_address(0x00f899c0);
  Raw head{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00f899c4))};
  void* prev_node = head.ptr(0x04);
  void* new_node = host.stl_list_buy_node_hit_record_00926670(
      list_this,
      head.p,
      prev_node,
      temp_record
  );
  host.stl_xlen_throw_00926890(list_this, 1);
  head.ptr(0x04) = new_node;
  Raw new_node_raw{static_cast<std::uint8_t*>(new_node)};
  Raw{static_cast<std::uint8_t*>(new_node_raw.ptr(0x04))}.ptr(0x00) = new_node;

  std::uint8_t* cur_head = reinterpret_cast<std::uint8_t*>(host.global_u32(0x00f899c4));
  std::uint8_t* back_node = static_cast<std::uint8_t*>(Raw{cur_head}.ptr(0x04));
  if (back_node == cur_head) {
    host.invalid_parameter_noinfo_00bf6713();
    if (back_node == reinterpret_cast<std::uint8_t*>(host.global_u32(0x00f899c4))) {
      host.invalid_parameter_noinfo_00bf6713();
    }
  }

  std::uint8_t* payload = back_node + 0x08;
  hit_record_assign_00925050(host, payload, hit_record);
  Raw payload_raw{payload};
  payload_raw.at<float>(0x54) = direction_xyz[0];
  payload_raw.at<float>(0x58) = direction_xyz[1];
  payload_raw.at<float>(0x5c) = direction_xyz[2];

  void* shot_entity = Raw{static_cast<std::uint8_t*>(hit_record)}.ptr(0x04);
  if (shot_entity != nullptr) {
    host.vcall_is_kind_of_slot5c(shot_entity, 0x29);
  }

  host.hit_record_destroy_004704b0(temp_record);
  if (cs != nullptr) {
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(cs);
  }
}

// 0x00926fa0: BSP_PendingEntityList_Copy
// Original ABI: void* __thiscall(ECX = dst_list, [ESP+4] = src_list), RET 4.
void* pending_entity_list_copy_00926fa0(Seg60Host& host, void* dst_list, void* src_list) {
  Raw dst{static_cast<std::uint8_t*>(dst_list)};
  Raw src{static_cast<std::uint8_t*>(src_list)};
  void* sentinel = host.stl_list_buy_sentinel_node_00924a50(dst_list);
  dst.ptr(0x04) = sentinel;
  dst.at<std::uint32_t>(0x08) = 0;

  void* src_sentinel = src.ptr(0x04);
  void* src_first = Raw{static_cast<std::uint8_t*>(src_sentinel)}.ptr(0x00);
  void* dst_first = Raw{static_cast<std::uint8_t*>(sentinel)}.ptr(0x00);
  host.stl_list_insert_range_009269b0(
      dst_list,
      dst_list,
      dst_first,
      src_list,
      src_first,
      src_list,
      src_sentinel
  );
  return dst_list;
}

// 0x00927050: BSP_MissionEntity_ApplySpawnDescriptor
// Original ABI: void __fastcall(ECX = entity), RET 0.
void mission_entity_apply_spawn_descriptor_00927050(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  void* holder = r.ptr(0xc0);
  if (holder == nullptr) {
    return;
  }
  const std::int32_t kind = Raw{static_cast<std::uint8_t*>(holder)}.at<std::int32_t>(0x04);
  if (kind == 1) {
    void* bag = Raw{static_cast<std::uint8_t*>(holder)}.ptr(0x08);
    void* race_prop = host.scene_property_bag_find_008f2260(
        bag,
        static_cast<const char*>(host.image_address(0x00ce8ee0))
    );
    r.at<std::int32_t>(0x58) = Raw{static_cast<std::uint8_t*>(race_prop)}.at<std::int32_t>(0x0c);

    void* party_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
    void* party_prop = host.scene_property_bag_find_008f2260(
        party_bag,
        static_cast<const char*>(host.image_address(0x00ce5804))
    );
    r.at<std::int32_t>(0x54) = Raw{static_cast<std::uint8_t*>(party_prop)}.at<std::int32_t>(0x0c);

    void* gui_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
    void* gui_check = host.scene_property_bag_find_008f2260(
        gui_bag,
        static_cast<const char*>(host.image_address(0x00d19258))
    );
    if (gui_check != nullptr) {
      void* gui_bag2 = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
      void* gui_prop = host.scene_property_bag_find_008f2260(
          gui_bag2,
          static_cast<const char*>(host.image_address(0x00d19258))
      );
      const char* gui_str = host.scene_property_get_string_value_008f0df0(gui_prop);
      struct {
        std::uint32_t len;
        void* data;
      } temp_str{0, nullptr};
      host.native_string_assign_0041e870(&temp_str, gui_str);
      host.vcall_entity_set_gui_name_slot0c(entity, &temp_str, 0);
      if (temp_str.data != nullptr) {
        const std::uint32_t bytes = temp_str.len + 1u;
        void* pool = host.sized_storage_pool_get_singleton_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, temp_str.data, bytes, 1);
      }
    }

    void* set_hier_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
    void* set_hier_prop = host.scene_property_bag_find_008f2260(
        set_hier_bag,
        static_cast<const char*>(host.image_address(0x00cea028))
    );
    if (set_hier_prop != nullptr) {
      void* rm_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
      host.scene_property_bag_remove_record_004f01f0(
          static_cast<std::uint8_t*>(rm_bag) + 0x04,
          set_hier_prop
      );

      void* hierarchy_parent = nullptr;
      void* parent_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
      void* parent_prop = host.scene_property_bag_find_008f2260(
          parent_bag,
          static_cast<const char*>(host.image_address(0x00cea018))
      );
      if (parent_prop != nullptr &&
          Raw{static_cast<std::uint8_t*>(parent_prop)}.at<std::int32_t>(0x04) == 5) {
        const char* text = host.scene_property_get_value_text_008f0e20(parent_prop);
        if (text != nullptr && std::strlen(text) != 0) {
          const char* parent_name = host.scene_property_get_value_text_008f0e20(parent_prop);
          Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
          hierarchy_parent = entity_registry_find_entity_by_name_00925a90(
              host,
              game.ptr(0x19cc),
              parent_name
          );
        }
      }

      float local_matrix[16]{};
      float copied_matrix[16]{};
      void* mat_bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
      void* mat_prop = host.scene_property_bag_find_008f2260(
          mat_bag,
          static_cast<const char*>(host.image_address(0x00cea008))
      );
      if (mat_prop != nullptr) {
        float* src16 = scene_property_copy_float16_00924280(mat_prop, copied_matrix);
        host.matrix_copy_4x4_x87_004134f0(local_matrix, src16);
      }
      Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
      host.vcall_place_in_world_slot98(
          entity,
          hierarchy_parent,
          game.ptr(0x19cc),
          local_matrix
      );
    }
  } else if (kind == 3) {
    std::uint8_t saved_lua_obj[20]{};
    sentity_get_saved_entity_lua_data_009238a0(host, entity, saved_lua_obj);
    std::uint8_t reader[20]{};
    host.lua_reader_construct_004425c0(reader, saved_lua_obj);

    host.lua_reader_enter_key_00bd8e20(reader, 0, host.image_address(0x00d190e4));

    std::uint8_t active_flag = 0;
    host.lua_reader_read_field_00bd6830(
        reader,
        0,
        host.image_address(0x00ce4a08),
        3,
        &active_flag
    );
    if (active_flag == 0) {
      host.scene_node_disable_00922f80(entity, 0);
    }

    std::uint8_t dead_meat = 0;
    host.lua_reader_read_field_or_default_00bd68d0(
        reader,
        0,
        host.image_address(0x00d190d8),
        3,
        &dead_meat,
        3,
        nullptr
    );
    if (dead_meat != 0) {
      r.at<std::uint8_t>(0x5d) = 1;
      r.at<std::int32_t>(0x70) = 1;
    }

    float invincible = 0.0f;
    host.lua_reader_read_field_or_default_00bd68d0(
        reader,
        0,
        host.image_address(0x00d190cc),
        2,
        &invincible,
        2,
        nullptr
    );
    if (invincible != host.global_f32(0x00d7a218)) {
      host.vcall_set_invincible_slotf4(entity, invincible);
    }

    host.lua_reader_leave_key_thunk_00bd7a20(reader);
    host.lua_reader_destroy_stack_instance_00441a20(reader);
  }
}

// 0x00927800: TRIV_body_00927800 (XOR AL,AL; RET)
std::uint8_t triv_body_00927800(void* /*self*/) noexcept {
  return 0;
}

// 0x00927a80: BSP_PropertyBag_GetSkill
// Original ABI: int __fastcall(ECX = entity), RET 0.
std::int32_t property_bag_get_skill_00927a80(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  void* bag = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
  void* skill_prop = host.scene_property_bag_find_008f2260(
      bag,
      static_cast<const char*>(host.image_address(0x00cf8838))
  );
  if (skill_prop != nullptr) {
    return Raw{static_cast<std::uint8_t*>(skill_prop)}.at<std::int32_t>(0x0c);
  }
  void* bag2 = Raw{static_cast<std::uint8_t*>(r.ptr(0xc0))}.ptr(0x08);
  void* crew_prop = host.scene_property_bag_find_008f2260(
      bag2,
      static_cast<const char*>(host.image_address(0x00d19264))
  );
  if (crew_prop != nullptr) {
    const std::int32_t crew_level =
        Raw{static_cast<std::uint8_t*>(crew_prop)}.at<std::int32_t>(0x0c);
    return host.skill_from_crew_level_006e6210(crew_level);
  }
  return 1;
}

// 0x00927ad0: BSP_Entity_GetPropertyUnitCommand
// Original ABI: void __thiscall(ECX = entity, [ESP+4] = out_lua_obj, [ESP+8] = key_native_str), RET 8.
void entity_get_property_unit_command_00927ad0(
    Seg60Host& host,
    void* entity,
    void* out_lua_obj,
    const void* key_native_str
) {
  Raw key{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(key_native_str))};
  const char* key_cstr = static_cast<const char*>(key.ptr(0x04));
  if (key_cstr == nullptr) {
    return;
  }
  if (host.stricmp_00bf7fbf(
          key_cstr,
          static_cast<const char*>(host.image_address(0x00d19278))
      ) == 0) {
    void* director = host.vcall_get_weapon_director_slot114(entity);
    if (director != nullptr) {
      void* cmd = host.weapon_director_current_command_0071be40(director);
      if (cmd != nullptr) {
        const char* cmd_name = host.vcall_command_name_slot04(cmd);
        host.lua_object_push_cstring_00b66710(out_lua_obj, cmd_name);
        return;
      }
      host.lua_object_push_cstring_00b66710(
          out_lua_obj,
          static_cast<const char*>(host.image_address(0x00d1926c))
      );
    }
  }
}

// 0x00927bf0: BSP_Unit_PushLuaSelfSlot
// Original ABI: void __thiscall(ECX = unit, [ESP+4] = lua_machine), RET 4.
void unit_push_lua_self_slot_00927bf0(Seg60Host& host, void* unit, void* lua_machine) {
  host.lua_machine_get_global_006b8460(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00ce7494))
  );
  char num_buf[16]{};
  const std::uint16_t entity_id = Raw{static_cast<std::uint8_t*>(unit)}.at<std::uint16_t>(0x174);
  char* text = host.itoa_00bf9010(static_cast<std::int32_t>(entity_id), num_buf, 10);
  host.lua_machine_push_string_006b8120(lua_machine, text);
  host.lua_machine_get_table_006b8470(lua_machine, -2);
  host.lua_machine_remove_006b7ea0(lua_machine, -2);
}

// 0x00927d20: BSP_Unit_SetRoleAvailability
// Original ABI: void __thiscall(ECX = unit, [ESP+4] = role_mask, [ESP+8] = state), RET 8.
void unit_set_role_availability_00927d20(
    Seg60Host& host,
    void* unit,
    std::uint32_t role_mask,
    std::int32_t state
) {
  Raw r{static_cast<std::uint8_t*>(unit)};
  std::uint32_t bit = 1u;
  for (std::int32_t i = 0; i < 9; ++i, bit <<= 1u) {
    if ((role_mask & bit) != 0u) {
      const std::size_t avail_off = 0x188u + static_cast<std::size_t>(i) * 4u;
      const std::size_t slot_off = 0x1acu + static_cast<std::size_t>(i) * 4u;
      r.at<std::int32_t>(avail_off) = state;
      if (host.vcall_is_kind_of_slot5c(unit, 2)) {
        const std::int32_t cur_slot = r.at<std::int32_t>(slot_off);
        if (cur_slot != 8 && r.at<std::int32_t>(avail_off) == 8) {
          std::uint8_t msg[0x34]{};
          host.session_message_construct_base_0075b430(msg, 0x4b);
          Raw msg_raw{msg};
          msg_raw.at<std::uint16_t>(0x18) = 0;
          msg_raw.at<std::uint8_t>(0x1a) = 0;
          msg_raw.at<std::uint32_t>(0x04) = 1;
          msg_raw.at<std::uint8_t>(0x1c) = 0;
          msg_raw.ptr(0x00) = host.image_address(0x00d02c90);
          msg_raw.at<std::uint8_t>(0x20) = 0;
          msg_raw.at<std::uint32_t>(0x24) = bit;
          msg_raw.at<std::int32_t>(0x28) = cur_slot;
          msg_raw.at<std::uint32_t>(0x2c) = 0;
          msg_raw.at<std::uint8_t>(0x30) = 0;
          host.session_route_message_0077c2a0(unit, msg, 0, 0);
        }
      }
    }
  }

  if (state != 8 && state != 9) {
    std::int32_t zero_out = 0;
    Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
    void* peer_entry = game.ptr(0x18ccu + static_cast<std::size_t>(state) * 4u);
    const std::uint32_t peer_party =
        Raw{static_cast<std::uint8_t*>(peer_entry)}.at<std::uint32_t>(0x28);
    host.vcall_entity_slot2c(unit, peer_party, r.at<std::uint32_t>(0x58), &zero_out);
  }
  host.observer_notify_slot0c_00696350(unit, 5, 0);
}

// 0x00927f30: BSP_UnitInstance_IsLocalPlayerRole
// Original ABI: int __thiscall(ECX = unit, [ESP+4] = role_index), RET 4.
std::int32_t unit_instance_is_local_player_role_00927f30(
    Seg60Host& host,
    const void* unit,
    std::int32_t role_index
) {
  Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
  const std::uint32_t local_slot = game.at<std::uint32_t>(0x18ec);
  if (local_slot <= 7u) {
    Raw u{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(unit))};
    if (u.at<std::uint32_t>(0x1acu + static_cast<std::size_t>(role_index) * 4u) == local_slot) {
      return 1;
    }
  }
  return 0;
}

// 0x00928030: BSP_EntityList_EraseNode
// Original ABI: void* __thiscall(ECX = list, [ESP+4] = node), RET 4.
void* entity_list_erase_node_00928030(Seg60Host& host, void* list, void* node) {
  Raw lst{static_cast<std::uint8_t*>(list)};
  Raw nd{static_cast<std::uint8_t*>(node)};
  void* prev = nd.ptr(0x00);
  if (prev != nullptr) {
    Raw{static_cast<std::uint8_t*>(prev)}.ptr(0x04) = nd.ptr(0x04);
  } else {
    lst.ptr(0x04) = nd.ptr(0x04);
  }

  prev = nd.ptr(0x00);
  void* next = nd.ptr(0x04);
  if (next != nullptr) {
    Raw{static_cast<std::uint8_t*>(next)}.ptr(0x00) = prev;
    lst.at<std::int32_t>(0x00) -= 1;
    host.free_00bf65ac(node);
    return next;
  }

  lst.at<std::int32_t>(0x00) -= 1;
  lst.ptr(0x08) = prev;
  host.free_00bf65ac(node);
  return nullptr;
}

// 0x00928100: SEntity_MirrorIdentityToThisTable
// Original ABI: void __fastcall(ECX = entity), RET 0.
void sentity_mirror_identity_to_this_table_00928100(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  if (r.at<std::int32_t>(0x178) == 0) {
    return;
  }
  Raw game{reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8))};
  Raw script_mgr{static_cast<std::uint8_t*>(game.ptr(0x1a08))};
  void* lua_machine = script_mgr.ptr(0x04);

  host.lua_machine_get_global_006b8460(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00ce7494))
  );
  const char* key_str = static_cast<const char*>(r.ptr(0x17c));
  if (key_str == nullptr) {
    key_str = static_cast<const char*>(host.image_address(0x00f89ac8));
  }
  host.lua_machine_push_string_006b8120(lua_machine, key_str);
  host.lua_machine_get_table_006b8470(lua_machine, -2);
  host.lua_table_set_string_number_006b8260(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00ce8ee0)),
      static_cast<float>(r.at<std::int32_t>(0x58))
  );
  host.lua_table_set_string_number_006b8260(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00ce5804)),
      static_cast<float>(r.at<std::int32_t>(0x54))
  );
  if (r.at<std::int32_t>(0x154) != 0) {
    const char* ent_name = host.vcall_entity_get_name_slot10(entity);
    host.lua_table_set_string_string_006b8360(
        lua_machine,
        static_cast<const char*>(host.image_address(0x00ce8ed0)),
        ent_name
    );
  }
  const std::uint32_t type_idx = r.at<std::uint32_t>(0xc4);
  const char* type_name =
      reinterpret_cast<const char*>(host.global_u32(0x00e0cd80u + type_idx * 4u));
  host.lua_table_set_string_string_006b8360(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00ce4780)),
      type_name
  );
  host.lua_machine_pop_006b85f0(lua_machine, 2);
}

// 0x009281c0: BSP_Unit_SetRoleSlot_Provisional
// Original ABI: void __thiscall(ECX = unit, [ESP+4] = role_index, [ESP+8] = slot_val), RET 8.
void unit_set_role_slot_provisional_009281c0(
    Seg60Host& host,
    void* unit,
    std::int32_t role_index,
    std::int32_t slot_val
) {
  Raw r{static_cast<std::uint8_t*>(unit)};
  if (role_index == 0) {
    if (slot_val == 8) {
      r.at<std::uint8_t>(0x184) = 0;
    }
    host.mission_scoring_notify_entity_00927f60(unit);
    r.at<std::int32_t>(0x1ac) = slot_val;
    return;
  }
  r.at<std::int32_t>(0x1acu + static_cast<std::size_t>(role_index) * 4u) = slot_val;
}

// 0x00928630: BSP_GameEntity_Construct
// Original ABI: void* __thiscall(ECX = self, [ESP+4] = registry_select, [ESP+8] = requested_id), RET 8.
void* game_entity_construct_00928630(
    Seg60Host& host,
    void* self,
    std::uint8_t registry_select,
    std::uint16_t requested_id
) {
  scene_node_construct_00925ce0(host, self);
  Raw r{static_cast<std::uint8_t*>(self)};
  r.ptr(0x170) = host.image_address(0x00cff378);
  r.ptr(0x00) = host.image_address(0x00d192e0);
  r.ptr(0x10) = host.image_address(0x00d192c8);
  r.ptr(0x24) = host.image_address(0x00d192c0);
  r.ptr(0x170) = host.image_address(0x00d192bc);
  r.at<std::uint32_t>(0x178) = 0;
  r.at<std::uint32_t>(0x17c) = 0;

  void* lock_owner = host.entity_registry_get_scene_lock_00928240();
  void* cs = Raw{static_cast<std::uint8_t*>(lock_owner)}.ptr(0x04);
  if (cs != nullptr) {
    host.enter_critical_section_00ce2218(cs);
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) += 1;
  }

  r.at<std::int32_t>(0xc4) = 1;
  void* id_table = (registry_select != 0)
      ? host.image_address(0x00f89a08)
      : host.image_address(0x00f89a5c);
  r.at<std::uint16_t>(0x174) =
      host.entity_id_table_allocate_009517c0(id_table, requested_id, self);

  r.at<std::uint8_t>(0x184) = 0;
  r.at<std::int32_t>(0x180) = 9;
  host.vcall_set_role_availability_slot148(self, 0x1ffu, 9);

  for (std::size_t i = 0; i < 9; ++i) {
    r.at<std::int32_t>(0x1acu + i * 4u) = 8;
  }
  r.at<float>(0x1d0) = host.global_f32(0x00f876a4);
  host.sentity_push_pending_00926be0(self);
  r.at<std::uint8_t>(0x1d4) = 0;

  if (cs != nullptr) {
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(cs);
  }
  return self;
}

// 0x00928790: CG_adjustor_thunk_00928790
// SUB ECX, 0x24; JMP 0x009289e0
void* cg_adjustor_thunk_00928790(Seg60Host& host, void* subobj_at_24, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_24) - 0x24;
  return host.game_entity_scalar_deleting_dtor_009289e0(base, flags);
}

// 0x009287a0: CG_adjustor_thunk_009287a0
// SUB ECX, 0x10; JMP 0x009289e0
void* cg_adjustor_thunk_009287a0(Seg60Host& host, void* subobj_at_10, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_10) - 0x10;
  return host.game_entity_scalar_deleting_dtor_009289e0(base, flags);
}

// 0x009287b0: BSP_GameEntity_Destruct
// Original ABI: void __fastcall(ECX = entity), RET 0.
void game_entity_destruct_009287b0(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  r.ptr(0x00) = host.image_address(0x00d192e0);
  r.ptr(0x10) = host.image_address(0x00d192c8);
  r.ptr(0x24) = host.image_address(0x00d192c0);
  r.ptr(0x170) = host.image_address(0x00d192bc);

  const std::uint16_t entity_id = r.at<std::uint16_t>(0x174);
  void* id_table = (static_cast<std::int32_t>(entity_id) <
                    static_cast<std::int32_t>(host.global_u32(0x00f89a10)))
      ? host.image_address(0x00f89a08)
      : host.image_address(0x00f89a5c);
  host.entity_id_table_release_009516d0(id_table, entity_id);

  void* str_17c = r.ptr(0x17c);
  if (str_17c != nullptr) {
    const std::uint32_t bytes = r.at<std::uint32_t>(0x178) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, str_17c, bytes, 1);
  }

  entity_destruct_base_00925780(host, entity);
}

// 0x00928860: BSP_GameEntity_PlaceInWorld
// Original ABI: void __thiscall(ECX = entity, [ESP+4] = hierarchy_parent, [ESP+8] = world_node, [ESP+0Ch] = matrix), RET 0Ch.
void game_entity_place_in_world_00928860(
    Seg60Host& host,
    void* entity,
    void* hierarchy_parent,
    void* world_node,
    const void* matrix
) {
  void* lock_owner = host.entity_registry_get_scene_lock_00928240();
  void* cs = Raw{static_cast<std::uint8_t*>(lock_owner)}.ptr(0x04);
  if (cs != nullptr) {
    host.enter_critical_section_00ce2218(cs);
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) += 1;
  }

  Raw r{static_cast<std::uint8_t*>(entity)};
  void* old_world = r.ptr(0x30);
  const bool world_changed = (world_node != old_world);
  if (world_changed && old_world != nullptr) {
    host.vcall_on_leave_world_slot134(entity);
  }
  host.scene_node_attach_to_parents_009258f0(entity, hierarchy_parent, world_node, matrix);
  if (world_changed && r.ptr(0x30) != nullptr) {
    host.vcall_on_enter_world_slot130(entity);
  }

  if (cs != nullptr) {
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(cs);
  }
}

// 0x00928920: BSP_ScriptEntity_Destruct
// Original ABI: void __fastcall(ECX = entity), RET 0.
void script_entity_destruct_00928920(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  r.ptr(0x00) = host.image_address(0x00d11138);
  r.ptr(0x10) = host.image_address(0x00d11120);
  r.ptr(0x24) = host.image_address(0x00d11118);
  r.ptr(0x170) = host.image_address(0x00d11114);

  std::uint8_t local_globals[20]{};
  std::uint8_t local_this_table[20]{};
  std::uint8_t* game = reinterpret_cast<std::uint8_t*>(host.global_u32(0x00e188a8));
  void* globals_obj = host.lua_state_owner_get_globals_00b67980(game + 0x1a0c, local_globals);
  void* this_table_obj = host.lua_object_get_by_name_00b67800(
      globals_obj,
      local_this_table,
      static_cast<const char*>(host.image_address(0x00ce7494))
  );
  host.lua_object_set_nil_00b67350(this_table_obj, r.p + 0x178);
  host.lua_object_destruct_00b67700(local_this_table);
  host.lua_object_destruct_00b67700(local_globals);
  game_entity_destruct_009287b0(host, entity);
}

// 0x009295b0: SEntity_DefaultInitSlotA4
// Original ABI: void __fastcall(ECX = entity), RET 0.
void sentity_default_init_slot_a4_009295b0(Seg60Host& host, void* entity) {
  triv_body_00923860(entity);
  Raw r{static_cast<std::uint8_t*>(entity)};
  void* holder = r.ptr(0xc0);
  if (holder != nullptr &&
      Raw{static_cast<std::uint8_t*>(holder)}.at<std::int32_t>(0x04) == 3) {
    std::uint8_t saved_lua_obj[20]{};
    sentity_get_saved_entity_lua_data_009238a0(host, entity, saved_lua_obj);
    std::uint8_t reader[20]{};
    host.lua_reader_construct_004425c0(reader, saved_lua_obj);
    host.lua_reader_enter_key_00bd8e20(reader, 0, host.image_address(0x00d192b0));

    struct {
      std::uint32_t len;
      char* data;
    } think_fn{0, nullptr};
    host.lua_reader_read_field_or_default_00bd68d0(
        reader,
        0,
        host.image_address(0x00d192a0),
        0,
        &think_fn,
        0,
        host.image_address(0x00ce3a0c)
    );
    if (think_fn.len != 0) {
      const char* script_name = (think_fn.data != nullptr)
          ? think_fn.data
          : static_cast<const char*>(host.image_address(0x00f89ac8));
      host.entity_set_think_script_name_0088a330(entity, script_name);
    }

    float timing = 0.0f;
    host.lua_reader_read_field_or_default_00bd68d0(
        reader,
        0,
        host.image_address(0x00d19298),
        2,
        &timing,
        2,
        nullptr
    );
    if (timing != host.global_f32(0x00d7a218)) {
      r.at<float>(0x1e0) = timing;
      r.at<std::uint8_t>(0x1dc) = 1;
    }

    host.vcall_set_role_availability_slot148(entity, 0x1ffu, 9);
    host.lua_reader_enter_key_00bd8e20(reader, 0, host.image_address(0x00d19288));

    for (std::int32_t i = 0; i < 9; ++i) {
      if (host.lua_reader_has_key_00bd5eb0(
              reader,
              1,
              reinterpret_cast<const void*>(static_cast<std::uintptr_t>(i))
          )) {
        std::int32_t role_val = 0;
        host.vcall_lua_reader_read_field_slot10(reader, 1, i, 1, &role_val);
        r.at<std::int32_t>(0x188u + static_cast<std::size_t>(i) * 4u) = role_val;
      }
    }

    host.lua_reader_leave_key_thunk_00bd7a20(reader);
    host.lua_reader_leave_key_thunk_00bd7a20(reader);

    if (think_fn.data != nullptr) {
      const std::uint32_t bytes = think_fn.len + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, think_fn.data, bytes, 1);
    }

    Raw{reader}.ptr(0x00) = host.image_address(0x00ce44fc);
    host.lua_object_vector_tidy_00441210(reader + 0x04);
    return;
  }

  sentity_mirror_identity_to_this_table_00928100(host, entity);
}

// 0x00929800: BSP_MissionEntity_PublishLuaDead
// Original ABI: void __fastcall(ECX = entity), RET 0.
void mission_entity_publish_lua_dead_00929800(Seg60Host& host, void* entity) {
  Raw r{static_cast<std::uint8_t*>(entity)};
  if (r.at<std::int32_t>(0x178) == 0) {
    return;
  }

  void* lock_owner = host.mission_entity_lock_get_004c1570();
  void* cs = Raw{static_cast<std::uint8_t*>(lock_owner)}.ptr(0x04);
  if (cs != nullptr) {
    host.enter_critical_section_00ce2218(cs);
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) += 1;
  }

  std::uint8_t lua_self[20]{};
  host.mission_entity_lua_self_object_00927b40(entity, lua_self);

  struct {
    std::uint32_t len;
    void* data;
  } dead_key{0, nullptr};
  host.native_string_assign_0041e870(
      &dead_key,
      static_cast<const char*>(host.image_address(0x00cf829c))
  );
  host.lua_object_set_boolean_00b673a0(lua_self, &dead_key, 1);
  if (dead_key.data != nullptr) {
    const std::uint32_t bytes = dead_key.len + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, dead_key.data, bytes, 1);
  }

  const std::uint32_t reason_idx = r.at<std::uint32_t>(0x70);
  const char* reason_str =
      reinterpret_cast<const char*>(host.global_u32(0x00e0cf04u + reason_idx * 4u));
  host.lua_object_set_cstring_00b66790(
      lua_self,
      static_cast<const char*>(host.image_address(0x00d19494)),
      reason_str
  );

  if (r.ptr(0x1d8) != nullptr) {
    struct {
      void* begin;
      std::uint32_t size;
      std::uint32_t cap;
    } arg_vec{nullptr, 0, 0};

    struct {
      std::uint32_t proxy;
      void* head;
      std::uint32_t count;
    } var_map{0, nullptr, 0};

    void* head_node = host.mission_lua_variant_allocate_link_record_006edea0(&var_map);
    var_map.head = head_node;
    Raw head_raw{static_cast<std::uint8_t*>(head_node)};
    head_raw.at<std::uint8_t>(0x31) = 1;
    head_raw.ptr(0x04) = head_node;
    head_raw.ptr(0x00) = head_node;
    head_raw.ptr(0x08) = head_node;
    var_map.count = 0;

    std::uint8_t key_buf[20]{};
    std::uint8_t val_buf[20]{};
    std::uint8_t pair_buf[56]{};
    std::uint8_t iter_pair[12]{};

    host.lua_variant_key_construct_00884d90(
        key_buf,
        static_cast<const char*>(host.image_address(0x00cfacfc))
    );
    void* msg_val = host.lua_variant_construct_int_00884a00(val_buf, 0);
    void* msg_pair = host.lua_variant_pair_construct_006ee0d0(pair_buf, key_buf, msg_val);
    host.lua_variant_map_insert_unique_006ee4e0(&var_map, iter_pair, msg_pair);
    host.mission_lua_variant_map_destroy_node_006ee040(pair_buf);
    host.lua_variant_destruct_00884af0(val_buf);
    if (Raw{key_buf}.ptr(0x08) != nullptr) {
      const std::uint32_t bytes = Raw{key_buf}.at<std::uint32_t>(0x04) + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, Raw{key_buf}.ptr(0x08), bytes, 1);
    }

    host.lua_variant_key_construct_00884d90(
        key_buf,
        static_cast<const char*>(host.image_address(0x00ce59b4))
    );
    void* id_val = host.lua_variant_construct_string_00884fc0(val_buf, r.p + 0x178);
    void* id_pair = host.lua_variant_pair_construct_006ee0d0(pair_buf, key_buf, id_val);
    host.lua_variant_map_insert_unique_006ee4e0(&var_map, iter_pair, id_pair);
    host.mission_lua_variant_map_destroy_node_006ee040(pair_buf);
    host.lua_variant_destruct_00884af0(val_buf);
    if (Raw{key_buf}.ptr(0x08) != nullptr) {
      const std::uint32_t bytes = Raw{key_buf}.at<std::uint32_t>(0x04) + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, Raw{key_buf}.ptr(0x08), bytes, 1);
    }

    host.lua_variant_construct_table_00886840(key_buf, &var_map);
    host.lua_variant_vector_push_back_006edf00(&arg_vec, key_buf);
    host.lua_variant_destruct_00884af0(key_buf);

    host.entity_invoke_think_once_009291d0(entity, &arg_vec);
    host.entity_list_erase_by_entity_00928300(host.image_address(0x00f89ab0), entity);
    host.entity_list_erase_by_entity_00928300(host.image_address(0x00f89abc), entity);

    void* first_node = Raw{static_cast<std::uint8_t*>(var_map.head)}.ptr(0x00);
    host.stl_tree_erase_range_006ee960(
        &var_map,
        iter_pair,
        &var_map,
        first_node,
        &var_map,
        var_map.head
    );
    host.free_00bf65ac(var_map.head);
    var_map.head = nullptr;
    var_map.count = 0;

    host.lua_variant_vector_resize_006b88e0(&arg_vec, 0);
    host.free_00bf6989(arg_vec.begin);
  }

  if (r.ptr(0x1d8) != nullptr) {
    host.free_00bf6989(r.ptr(0x1d8));
    r.ptr(0x1d8) = nullptr;
  }

  host.lua_object_destruct_00b67700(lua_self);
  if (cs != nullptr) {
    Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(cs);
  }
}

// 0x00929b60: BSP_MissionEntity_OnKilledLua
// Original ABI: void __fastcall(ECX = entity), RET 0.
void mission_entity_on_killed_lua_00929b60(Seg60Host& host, void* entity) {
  host.mission_scoring_notify_entity_00927f60(entity);
  sentity_mirror_identity_to_this_table_00928100(host, entity);
  mission_entity_publish_lua_dead_00929800(host, entity);
  triv_body_00923040(entity);
}

// 0x00929b80: BSP_CollisionShape_TraceSegment_TransformedBox
// Original ABI: uint8_t __thiscall(ECX = shape_at_1a4, [ESP+4] = p0_xyz, [ESP+8] = p1_xyz, [ESP+0Ch] = hit_record), RET 0Ch.
std::uint8_t collision_shape_trace_segment_transformed_box_00929b80(
    Seg60Host& host,
    void* shape_at_1a4,
    const void* p0_xyz,
    const void* p1_xyz,
    void* hit_record
) {
  Raw s{static_cast<std::uint8_t*>(shape_at_1a4)};
  float raw_max[3]{};
  float raw_min[3]{};
  // 00929B87..00929B97: the first output of 00C31F90 is the box MIN (it reaches 0085CDB0 in EDX).
  host.dyn_physics_get_box_extents_00c31f90(s.ptr(0x1b0), raw_min, raw_max);

  const float box_max[3] = {raw_max[0], raw_max[1], raw_max[2]};
  const float box_min[3] = {raw_min[0], raw_min[1], raw_min[2]};

  float local_p0[3]{};
  float local_p1[3]{};
  host.vector3f_transform_affine_point_004142e0(p0_xyz, local_p0, s.p + 0xb0);
  host.vector3f_transform_affine_point_004142e0(p1_xyz, local_p1, s.p + 0xb0);

  float local_hit[3]{};
  if (host.geometry_segment_box_hit_0085cdb0(local_hit, box_min, box_max, local_p0, local_p1)) {
    float world_hit_storage[3]{};
    float* world_hit = host.vector3f_transform_affine_point_004142e0(
        local_hit,
        world_hit_storage,
        s.p + 0x70
    );
    Raw rec{static_cast<std::uint8_t*>(hit_record)};
    rec.at<float>(0x08) = world_hit[0];
    rec.at<float>(0x0c) = world_hit[1];
    rec.at<float>(0x10) = world_hit[2];
    host.hit_record_set_entity_00470370(hit_record, s.p - 0x1a4);
    rec.at<std::uint32_t>(0x30) = 10u;
    rec.at<std::uint32_t>(0x34) = 0xffffffffu;
    return 1;
  }
  return 0;
}

// 0x00929e50: BSP_TickableGameEntity_Construct
// Original ABI: void* __fastcall(ECX = self), RET 0.
void* tickable_game_entity_construct_00929e50(Seg60Host& host, void* self) {
  scene_node_construct_00925ce0(host, self);
  Raw r{static_cast<std::uint8_t*>(self)};
  host.tick_registration_construct_00875890(r.p + 0x170, self, 0);

  void* col_node = (self != nullptr) ? (r.p + 0x1c4) : nullptr;
  r.ptr(0x1a4) = host.image_address(0x00ce89dc);
  r.ptr(0x1c0) = col_node;

  r.at<float>(0x1b4) = host.global_f32(0x00f87574);
  r.at<float>(0x1b8) = host.global_f32(0x00f87578);
  r.at<float>(0x1bc) = host.global_f32(0x00f8757c);
  r.at<float>(0x1a8) = r.at<float>(0x1b4);
  r.at<float>(0x1ac) = r.at<float>(0x1b8);
  r.ptr(0x1c4) = host.image_address(0x00ce89e8);
  r.at<float>(0x1b0) = r.at<float>(0x1bc);

  host.collision_node_init_fields_004e6480(r.p + 0x1c4);

  const float val_388 = host.global_f32(0x00ceb4b0);
  r.ptr(0x210) = self;
  r.ptr(0x00) = host.image_address(0x00d19500);
  r.ptr(0x10) = host.image_address(0x00d194e4);
  r.ptr(0x24) = host.image_address(0x00d194dc);
  r.ptr(0x170) = host.image_address(0x00d194c4);
  r.ptr(0x1a4) = host.image_address(0x00d194b8);
  r.ptr(0x1c4) = host.image_address(0x00d194a8);

  r.at<std::uint32_t>(0x324) = 0;
  r.at<std::uint32_t>(0x328) = 0;
  r.at<std::uint32_t>(0x32c) = 0;
  r.at<std::uint32_t>(0x330) = 0;
  r.at<std::uint32_t>(0x334) = 0;
  r.at<std::uint32_t>(0x338) = 0;
  r.at<float>(0x388) = val_388;
  r.at<std::uint32_t>(0xc4) = 0x58;
  r.at<std::uint32_t>(0x354) = 0;
  r.at<std::uint32_t>(0x350) = 0;
  r.at<std::uint32_t>(0x34c) = 0;
  r.at<std::uint8_t>(0x384) = 0;
  r.at<std::uint8_t>(0x385) = 0;
  r.at<float>(0x38c) = 0.0f;

  void* head = reinterpret_cast<void*>(host.global_u32(0x00f89ad8));
  r.ptr(0x340) = head;
  r.ptr(0x344) = nullptr;
  if (head != nullptr) {
    Raw{static_cast<std::uint8_t*>(head)}.ptr(0x344) = self;
  }
  host.global_u32(0x00f89ad8) = reinterpret_cast<std::uint32_t>(self);
  return self;
}

// 0x00929ff0: TRIV_body_00929ff0 (XOR AL,AL; RET 0Ch)
std::uint8_t triv_body_00929ff0(
    void* /*self*/,
    std::uint32_t /*a0*/,
    std::uint32_t /*a1*/,
    std::uint32_t /*a2*/
) noexcept {
  return 0;
}

// 0x0092a000: CG_adjustor_thunk_0092a000
// SUB ECX, 0x24; JMP 0x0092a220
void* cg_adjustor_thunk_0092a000(Seg60Host& host, void* subobj_at_24, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_24) - 0x24;
  return host.tickable_game_entity_scalar_deleting_dtor_0092a220(base, flags);
}

// 0x0092a020: CG_adjustor_thunk_0092a020
// SUB ECX, 0x1c4; JMP 0x0092a220
void* cg_adjustor_thunk_0092a020(Seg60Host& host, void* subobj_at_1c4, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_1c4) - 0x1c4;
  return host.tickable_game_entity_scalar_deleting_dtor_0092a220(base, flags);
}

// 0x0092a030: CG_adjustor_thunk_0092a030
// SUB ECX, 0x170; JMP 0x0092a220
void* cg_adjustor_thunk_0092a030(Seg60Host& host, void* subobj_at_170, std::int32_t flags) {
  void* base = static_cast<std::uint8_t*>(subobj_at_170) - 0x170;
  return host.tickable_game_entity_scalar_deleting_dtor_0092a220(base, flags);
}

// 0x0092b350: BSP_TickableGameEntity_TickAdvance
// Original ABI: void __thiscall(ECX = tick_node_at_170, [ESP+4] = step), RET 4.
// Note: x87 DC-form instructions verified at 0092b5bf (dc c9 fmul st(1),st(0) with st(0)=0.0)
// and 0092b82f (dc e9 fsub st(1),st(0) in the fyl2x/frndint/f2xm1/fscale pow sequence).
void tickable_game_entity_tick_advance_0092b350(
    Seg60Host& host,
    void* tick_node_at_170,
    float step
) {
  Raw r{static_cast<std::uint8_t*>(tick_node_at_170)};
  void* body = r.ptr(0x1e4);
  if (body == nullptr) {
    return;
  }

  void* settings = host.game_settings_get_singleton_00424c40();
  Raw set_raw{static_cast<std::uint8_t*>(settings)};
  const float mass = host.dyn_body_get_mass_00c31fc0(body);

  r.at<float>(0x20c) = mass * step * set_raw.at<float>(0x10) + r.at<float>(0x20c);

  float delta[3] = {
      r.at<float>(0x1f4) - r.at<float>(0x200),
      r.at<float>(0x1f8) - r.at<float>(0x204),
      r.at<float>(0x1fc) - r.at<float>(0x208),
  };
  host.geometry_normalize_vector_with_floor_0042b260(delta);

  Raw cfg{static_cast<std::uint8_t*>(r.ptr(0x1d8))};
  const float move_step = static_cast<float>(
                              static_cast<double>(cfg.at<float>(0xa0)) /
                              host.global_f64(0x00ce3ca8)
                          ) *
                          step;

  r.at<float>(0x200) += move_step * delta[0];
  r.at<float>(0x204) += move_step * delta[1];
  r.at<float>(0x208) += move_step * delta[2];

  std::uint8_t* owner_entity = r.p - 0x170;
  if (Raw{owner_entity}.at<std::uint8_t>(0xc8) == 0) {
    host.entity_pose_refresh_world_00414db0(owner_entity);
  }

  float world_pt[3]{};
  host.vector3f_transform_affine_point_004142e0(r.p + 0x200, world_pt, owner_entity + 0xcc);
  const float copy_pt[3] = {world_pt[0], world_pt[1], world_pt[2]};
  const float grav_force[3] = {
      0.0f,
      host.global_f32(0x00d7a208) - r.at<float>(0x20c),
      0.0f,
  };
  host.dyn_physics_apply_force_at_point_00c37f60(r.ptr(0x1e4), grav_force, copy_pt);

  const std::int32_t sample_count = set_raw.at<std::int32_t>(0x1c);
  const float exp_power = set_raw.at<float>(0x14);
  const float drag_tan = set_raw.at<float>(0x08);
  const float drag_norm = set_raw.at<float>(0x0c);
  const float length_ext = r.at<float>(0x1f0);

  float lin_vel[3]{};
  host.dyn_physics_get_linear_velocity_00c31f40(r.ptr(0x1e4), lin_vel);
  float ang_vel[3]{};
  host.dyn_physics_get_angular_velocity_00c31f20(r.ptr(0x1e4), ang_vel);

  if (sample_count > 0) {
    const std::int32_t half_count = sample_count / 2;
    const float half_count_f = static_cast<float>(half_count);
    const double z_base =
        static_cast<double>(length_ext / static_cast<float>(half_count * 2)) -
        static_cast<double>(length_ext) * host.global_f64(0x00d7a280);

    for (std::uint32_t u = 0; static_cast<std::int32_t>(u) < sample_count; ++u) {
      const std::int32_t side_sign = static_cast<std::int32_t>((u & 1u) * 2u) - 1;
      const float local_x = static_cast<float>(
          static_cast<double>(r.at<float>(0x1e8)) *
          host.global_f64(0x00d7a348) *
          static_cast<double>(side_sign)
      );
      const float local_z = static_cast<float>(
          static_cast<double>((length_ext * static_cast<float>(u >> 1u)) / half_count_f) +
          z_base
      );

      float* mat = host.object_field_at8_accessor_00c32000(r.ptr(0x1e4));
      float sample_pos[3];
      sample_pos[1] = local_x * mat[1] + mat[4] * 0.0f + local_z * mat[7] + mat[10];
      sample_pos[2] = local_z * mat[8] + mat[5] * 0.0f + local_x * mat[2] + mat[11];
      sample_pos[0] = local_z * mat[6] + mat[3] * 0.0f + local_x * mat[0] + mat[9];

      const float height_ext = r.at<float>(0x1ec);
      float depth_diff = static_cast<float>(
          (static_cast<double>(sample_pos[1]) -
           host.global_f64(0x00d7a280) * static_cast<double>(height_ext)) -
          0.0
      );
      if (depth_diff < 0.0f) {
        if (depth_diff < -height_ext) {
          depth_diff = -height_ext;
        }
        const float submersion = -depth_diff / height_ext;
        const float buoy_force[3] = {
            0.0f,
            static_cast<float>(
                (static_cast<double>(submersion) *
                 host.global_f64(0x00d7a328) *
                 static_cast<double>(mass) *
                 host.global_f64(0x00ce3dc0)) /
                static_cast<double>(sample_count)
            ),
            0.0f,
        };
        host.dyn_physics_apply_force_at_point_00c37f60(r.ptr(0x1e4), buoy_force, sample_pos);

        const float arm_x = sample_pos[0] - mat[9];
        const float arm_y = sample_pos[1] - mat[10];
        const float arm_z = sample_pos[2] - mat[11];

        const float cross_x = arm_z * ang_vel[1] - arm_y * ang_vel[2];
        const float cross_y = arm_x * ang_vel[2] - ang_vel[0] * arm_z;
        const float cross_z = ang_vel[0] * arm_y - arm_x * ang_vel[1];

        const float pt_vel_x = cross_x + lin_vel[0];
        const float pt_vel_y = lin_vel[1] + cross_y;
        const float pt_vel_z = lin_vel[2] + cross_z;

        const float axial_dot = pt_vel_y * mat[7] + pt_vel_x * mat[6] + pt_vel_z * mat[8];

        float pow_sub = 0.0f;
        if (submersion != 0.0f) {
          float abs_sub = submersion;
          if (submersion < 0.0f) {
            abs_sub = host.global_f32(0x00d7a208) - submersion;
          }
          pow_sub = std::pow(abs_sub, exp_power);
        }

        const float axial_scale = -(pow_sub * mass) * drag_norm;
        const float axial_fx = axial_scale * axial_dot * mat[6];
        const float axial_fy = mat[7] * axial_dot * axial_scale;
        const float axial_fz = axial_scale * axial_dot * mat[8];

        const float tan_scale = -(pow_sub * mass) * drag_tan;
        const float damp_force[3] = {
            tan_scale * (pt_vel_x - axial_dot * mat[6]) + axial_fx,
            (pt_vel_y - mat[7] * axial_dot) * tan_scale + axial_fy,
            tan_scale * (pt_vel_z - axial_dot * mat[8]) + axial_fz,
        };
        host.dyn_physics_apply_force_at_point_00c37f60(r.ptr(0x1e4), damp_force, sample_pos);
      }
    }
  }
}

// 0x0092bd30: BSP_UnitController_ClearHullShapeFields
// Original ABI: void __fastcall(ECX = controller), RET 0.
void unit_controller_clear_hull_shape_fields_0092bd30(Seg60Host& host, void* controller) {
  void* hull_body = Raw{static_cast<std::uint8_t*>(controller)}.ptr(0x2c);
  if (hull_body != nullptr) {
    for (void* shape = host.dyn_physics_first_shape_00c31dc0(hull_body);
         shape != nullptr;
         shape = Raw{static_cast<std::uint8_t*>(shape)}.ptr(0x208)) {
      host.dyn_physics_set_shape_field30_00c48020(shape, 0);
      host.dyn_physics_set_shape_field2c_00c48050(shape, 0);
    }
  }
}

// 0x0092bd70: BSP_UnitController_SetCollisionGroup
// Original ABI: void __thiscall(ECX = controller, [ESP+4] = enabled), RET 4.
void unit_controller_set_collision_group_0092bd70(
    Seg60Host& host,
    void* controller,
    std::uint8_t enabled
) {
  Raw ctrl{static_cast<std::uint8_t*>(controller)};
  ctrl.at<std::uint8_t>(0x64) = enabled;

  void* unit = ctrl.ptr(0x1c);
  void* unit_class = Raw{static_cast<std::uint8_t*>(unit)}.ptr(0x538);
  const std::int32_t kind = host.vcall_unit_class_kind_slot1c(unit_class);

  std::uint32_t mask = 0u;
  switch (kind) {
    case 7:
      mask = 0x40u;
      break;
    case 8:
      mask = 0x2000u;
      break;
    case 9:
      mask = 0x20u;
      break;
    case 10: {
      void* u_reloaded = ctrl.ptr(0x1c);
      void* cls_reloaded = Raw{static_cast<std::uint8_t*>(u_reloaded)}.ptr(0x538);
      const bool flag = Raw{static_cast<std::uint8_t*>(cls_reloaded)}.at<std::uint8_t>(0x808) != 0;
      mask = flag ? 0x1000u : 0x800u;
      break;
    }
    case 11:
      mask = 0x400u;
      break;
    case 12: {
      void* u_reloaded = ctrl.ptr(0x1c);
      void* cls_reloaded = Raw{static_cast<std::uint8_t*>(u_reloaded)}.ptr(0x538);
      const bool flag = Raw{static_cast<std::uint8_t*>(cls_reloaded)}.at<std::uint8_t>(0x808) != 0;
      mask = flag ? 0x200u : 0x100u;
      break;
    }
    case 13:
      mask = 0x10u;
      break;
    case 14:
      mask = 0x80u;
      break;
    default:
      break;
  }

  void* shape = host.dyn_physics_first_shape_00c31dc0(ctrl.ptr(0x2c));
  if (enabled != 0) {
    for (; shape != nullptr; shape = Raw{static_cast<std::uint8_t*>(shape)}.ptr(0x208)) {
      host.dyn_physics_set_collision_mask_bits_00c47f90(shape, mask);
    }
  } else {
    for (; shape != nullptr; shape = Raw{static_cast<std::uint8_t*>(shape)}.ptr(0x208)) {
      host.dyn_physics_clear_collision_mask_bits_00c47f60(shape, mask);
    }
  }
}

// 0x0092be90: TRIV_body_0092be90
// MOV EAX, [ECX + 0x374]; RET
std::uint32_t triv_body_0092be90(const void* self) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(self))}
      .at<std::uint32_t>(0x374);
}

// 0x0092bea0: TRIV_body_0092bea0
// MOV EAX, [ECX + 0x370]; RET
std::uint32_t triv_body_0092bea0(const void* self) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(self))}
      .at<std::uint32_t>(0x370);
}

// 0x0092beb0: TRIV_body_0092beb0
// FLD float ptr [ECX + 0x84]; RET
float triv_body_0092beb0(const void* self) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(self))}
      .at<float>(0x84);
}

// 0x0092ced0: BSP_UnitMessage_PartDestroyed_Construct
// Original ABI: void* __thiscall(ECX = msg, [ESP+4] = part_index), RET 4.
void* unit_message_part_destroyed_construct_0092ced0(
    Seg60Host& host,
    void* msg,
    std::int32_t part_index
) {
  host.session_message_construct_base_0075b430(msg, 0x99);
  Raw r{static_cast<std::uint8_t*>(msg)};
  r.at<std::uint16_t>(0x18) = 0;
  r.at<std::uint8_t>(0x1a) = 0;
  r.at<std::uint8_t>(0x1c) = 0;
  r.at<std::int32_t>(0x20) = part_index;
  r.at<std::int32_t>(0x04) = 1;
  r.ptr(0x00) = host.image_address(0x00d033ec);
  return msg;
}

}  // namespace bsp::entity_mission_seg60_argon
