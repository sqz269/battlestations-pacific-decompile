#include "bsp/warning_string_seg64_argon.hpp"

namespace bsp::warning_string_seg64_argon {

// 0x0096aa00
void warning_activate_0096aa00(Seg64Host& host, Raw warning) {
  warning.at<std::uint32_t>(0x0c) = 1u;
  warning.at<float>(0x10) = host.global_f32(0x00f876a4u);
}

// 0x0096c280
void warning_list_remove_by_value_0096c280(
    Seg64Host& host,
    Raw list,
    void* const* value_ptr) {
  std::uint8_t* const list_owner = list.p;
  Raw head{ static_cast<std::uint8_t*>(list.ptr(0x04)) };
  void* const target = *value_ptr;
  Raw node{ static_cast<std::uint8_t*>(head.ptr(0x00)) };
  std::uint8_t* iter_owner = list_owner;

  for (;;) {
    if (iter_owner == nullptr || iter_owner != list_owner) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node.p == head.p) {
      break;
    }
    if (iter_owner == nullptr) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node.p == Raw{ iter_owner }.ptr(0x04)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node.ptr(0x08) == target) {
      if (node.p == Raw{ iter_owner }.ptr(0x04)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      Raw next{ static_cast<std::uint8_t*>(node.ptr(0x00)) };
      if (node.p != list.ptr(0x04)) {
        Raw prev{ static_cast<std::uint8_t*>(node.ptr(0x04)) };
        prev.ptr(0x00) = next.p;
        Raw next_reloaded{ static_cast<std::uint8_t*>(node.ptr(0x00)) };
        next_reloaded.ptr(0x04) = prev.p;
        host.free_00bf65ac(node.p);
        list.at<std::int32_t>(0x08) -= 1;
      }
      node = next;
    } else {
      if (node.p == Raw{ iter_owner }.ptr(0x04)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      node = Raw{ static_cast<std::uint8_t*>(node.ptr(0x00)) };
    }
  }
}

// 0x0096cd90
void vtable_native_string_release_storage_0096cd90(Seg64Host& host, Raw obj) {
  obj.ptr(0x00) = host.image_address(0x00d1af88u);
  void* const data = obj.ptr(0x08);
  if (data != nullptr) {
    const std::uint32_t bytes = obj.at<std::uint32_t>(0x04) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// 0x0096cdc0
void* vtable_native_string_scalar_deleting_dtor_0096cdc0(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags) {
  obj.ptr(0x00) = host.image_address(0x00d1af88u);
  void* const data = obj.ptr(0x08);
  if (data != nullptr) {
    const std::uint32_t bytes = obj.at<std::uint32_t>(0x04) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(obj.p);
  }
  return obj.p;
}

// 0x0096d330
void string_list_warning_save_archive_0096d330(
    Seg64Host& host,
    Raw warning,
    void* archive) {
  host.warning_save_archive_0096cf10(warning.p, archive);
  host.archive_vslot04_begin_table(archive, 0u, host.image_address(0x00ce4ad8u));

  std::uint32_t index = 0u;
  for (;;) {
    std::uint8_t* const first = static_cast<std::uint8_t*>(warning.ptr(0x70));
    if (first == nullptr) {
      break;
    }
    std::uint8_t* const last = static_cast<std::uint8_t*>(warning.ptr(0x74));
    const std::uint32_t count =
        static_cast<std::uint32_t>((last - first) >> 3);
    if (index >= count) {
      break;
    }
    if (first == nullptr ||
        index >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(warning.ptr(0x74)) - first) >> 3)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Raw elem{ static_cast<std::uint8_t*>(warning.ptr(0x70)) + index * 8u };
    const void* str_ptr = elem.ptr(0x04);
    if (str_ptr == nullptr) {
      str_ptr = host.image_address(0x00f8a0c8u);
    }
    host.archive_vslot0c_write_entry(archive, 1u, index, 0u, str_ptr);
    index += 1u;
  }

  host.archive_vslot08_end_table(archive);
}

// 0x0096d7b0
void native_string_release_storage_0096d7b0(Seg64Host& host, Raw str) {
  void* const data = str.ptr(0x04);
  if (data != nullptr) {
    const std::uint32_t bytes = str.at<std::uint32_t>(0x00) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// 0x0096db10
void* ref_holder_scalar_deleting_dtor_0096db10(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags) {
  Raw ref{ static_cast<std::uint8_t*>(obj.ptr(0x18)) };
  if (ref.p != nullptr) {
    if (host.interlocked_decrement_00ce2220(&ref.at<std::int32_t>(0x04)) == 0) {
      host.ref_counted_vslot00_destroy(ref.p);
    }
    obj.ptr(0x18) = nullptr;
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(obj.p);
  }
  return obj.p;
}

// 0x0096dca0
std::uint32_t warning_event_record_get_kind_0096dca0() {
  return 3u;
}

// 0x0096dd10
void* warning_event_record_scalar_deleting_dtor_0096dd10(
    Seg64Host& host,
    Raw record,
    std::uint8_t flags) {
  host.warning_event_record_destruct_0096dcb0(record.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(record.p);
  }
  return record.p;
}

// 0x0096ecc0
void* offset_native_string_scalar_deleting_dtor_0096ecc0(
    Seg64Host& host,
    Raw obj,
    std::uint8_t flags) {
  void* const data = obj.ptr(0x10);
  if (data != nullptr) {
    const std::uint32_t bytes = obj.at<std::uint32_t>(0x0c) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(obj.p);
  }
  return obj.p;
}

// 0x0096f220
float warning_get_zero_lifetime_0096f220() {
  return 0.0f;
}

// 0x0096f240
float warning_get_default_cooldown_0096f240(Seg64Host& host) {
  return host.global_f32(0x00ce5380u);
}

// 0x0096f250
void* warning_base_scalar_deleting_dtor_0096f250(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973b80
void* entity_warning_kind0_scalar_deleting_dtor_00973b80(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973c50
std::uint32_t entity_warning_get_kind1_00973c50() {
  return 1u;
}

// 0x00973c80
void* entity_warning_kind1_scalar_deleting_dtor_00973c80(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973cd0
std::uint32_t entity_warning_get_kind2_00973cd0() {
  return 2u;
}

// 0x00973ce0
float entity_warning_kind2_get_cooldown_00973ce0(Seg64Host& host) {
  return host.global_f32(0x00ce38c8u);
}

// 0x00973d10
void* entity_warning_kind2_scalar_deleting_dtor_00973d10(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973d50
std::uint32_t warning_get_kind3_00973d50() {
  return 3u;
}

// 0x00973d70
void* warning_kind3_scalar_deleting_dtor_00973d70(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973dc0
std::uint32_t entity_warning_get_kind4_00973dc0() {
  return 4u;
}

// 0x00973df0
void* entity_warning_kind4_scalar_deleting_dtor_00973df0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973e40
std::uint32_t entity_warning_get_kind5_00973e40() {
  return 5u;
}

// 0x00973e70
void* entity_warning_kind5_scalar_deleting_dtor_00973e70(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00973ec0
std::uint32_t entity_warning_get_kind6_00973ec0() {
  return 6u;
}

// 0x00973f00
void* entity_warning_kind6_scalar_deleting_dtor_00973f00(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.entity_warning_destruct_0096f270(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00974070
void warning_manager_apply_warning_00974070(
    Seg64Host& host,
    Raw manager,
    void* warning_ptr) {
  void* warning_arg = warning_ptr;
  Raw warning{ static_cast<std::uint8_t*>(warning_ptr) };
  void* const positional_entity = warning.ptr(0x50);
  if (positional_entity != nullptr) {
    std::uint8_t* const clip_begin = static_cast<std::uint8_t*>(warning.ptr(0x18));
    std::uint8_t* const clip_end = static_cast<std::uint8_t*>(warning.ptr(0x1c));
    if (clip_begin == nullptr || ((clip_end - clip_begin) / 12) == 0) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Raw first_clip{ static_cast<std::uint8_t*>(warning.ptr(0x18)) };
    Raw sound_root{
      reinterpret_cast<std::uint8_t*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e198c4u)))
    };
    void* const voice_mgr = sound_root.ptr(0xa4);
    host.voice_manager_play_positional_line_005bbdc0(
        voice_mgr,
        host.image_address(0x00cf0dd0u),
        first_clip.ptr(0x04),
        first_clip.at<std::uint32_t>(0x08),
        positional_entity);
  } else {
    Raw sound_root{
      reinterpret_cast<std::uint8_t*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e198c4u)))
    };
    void* const voice_mgr = sound_root.ptr(0xa4);
    host.voice_manager_play_line_005bbc10(
        voice_mgr,
        warning.p + 0x14,
        warning.at<std::uint32_t>(0x38),
        warning.ptr(0x68));
  }

  warning.at<std::uint32_t>(0x0c) = 2u;
  warning.at<float>(0x10) = host.global_f32(0x00f876a4u);

  warning_list_remove_by_value_0096c280(
      host,
      Raw{ manager.p + 0xe0 },
      &warning_arg);

  Raw active_list{ manager.p + 0xec };
  Raw head{ static_cast<std::uint8_t*>(manager.ptr(0xf0)) };
  Raw new_node{
    static_cast<std::uint8_t*>(host.stl_list_buy_node_0096b510(
        active_list.p,
        head.p,
        head.ptr(0x04),
        &warning_arg))
  };
  host.stl_list_incsize_0096f960(active_list.p, 1u);
  head.ptr(0x04) = new_node.p;
  Raw prev{ static_cast<std::uint8_t*>(new_node.ptr(0x04)) };
  prev.ptr(0x00) = new_node.p;
}

// 0x00975710
std::uint32_t warning_get_kind7_00975710() {
  return 7u;
}

// 0x00975730
void* warning_kind7_scalar_deleting_dtor_00975730(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00975770
std::uint32_t warning_get_kind9_00975770() {
  return 9u;
}

// 0x00975790
void* warning_kind9_scalar_deleting_dtor_00975790(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x009758a0
std::uint32_t warning_get_kind8_009758a0() {
  return 8u;
}

// 0x009758c0
void* warning_kind8_scalar_deleting_dtor_009758c0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00975a80
std::uint32_t warning_get_kind10_00975a80() {
  return 10u;
}

// 0x00975aa0
void* warning_kind10_scalar_deleting_dtor_00975aa0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.warning_destruct_0096f170(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00975d00
float* ship_noise_map_subscript_00975d00(
    Seg64Host& host,
    Raw map,
    const std::uint32_t* key_ptr) {
  Raw head{ static_cast<std::uint8_t*>(map.ptr(0x04)) };
  Raw candidate = head;
  Raw cur{ static_cast<std::uint8_t*>(head.ptr(0x04)) };
  if (cur.at<std::uint8_t>(0x1d) == 0u) {
    const std::uint32_t key = *key_ptr;
    do {
      if (cur.at<std::uint32_t>(0x0c) < key) {
        cur = Raw{ static_cast<std::uint8_t*>(cur.ptr(0x08)) };
      } else {
        candidate = cur;
        cur = Raw{ static_cast<std::uint8_t*>(cur.ptr(0x00)) };
      }
    } while (cur.at<std::uint8_t>(0x1d) == 0u);
  }

  std::uint8_t* container = map.p;
  if (candidate.p == static_cast<std::uint8_t*>(map.ptr(0x04)) ||
      *key_ptr < candidate.at<std::uint32_t>(0x0c)) {
    const float default_time = host.global_f32(0x00ce4adcu);
    ShipNoiseDefaultValue default_info{ default_time, default_time, nullptr };
    (void)default_info;
    ShipNoiseMapValue value_pair{
      *key_ptr,
      default_time,
      default_time,
      nullptr
    };
    TreeIteratorPair inserted{ nullptr, nullptr };
    auto* const iter_res = static_cast<TreeIteratorPair*>(
        host.stl_tree_insert_hint_00973410(
            map.p,
            &inserted,
            container,
            candidate.p,
            &value_pair));
    container = static_cast<std::uint8_t*>(iter_res->container);
    candidate = Raw{ static_cast<std::uint8_t*>(iter_res->node) };
    if (value_pair.effect_ref != nullptr) {
      Raw ref{ static_cast<std::uint8_t*>(value_pair.effect_ref) };
      if (host.interlocked_decrement_00ce2220(&ref.at<std::int32_t>(0x04)) == 0) {
        host.ref_counted_vslot00_destroy(ref.p);
      }
    }
  }

  if (container == nullptr) {
    host.crt_invalid_parameter_noinfo_00bf6713();
  }
  if (candidate.p == Raw{ container }.ptr(0x04)) {
    host.crt_invalid_parameter_noinfo_00bf6713();
  }
  return &candidate.at<float>(0x10);
}

// 0x00976280
std::uint32_t string_list_warning_get_kind0_00976280() {
  return 0u;
}

// 0x00976cb0
void warning_manager_report_first_string_00976cb0(
    Seg64Host& host,
    Raw manager,
    Raw string_vector,
    std::uint32_t unused_arg2,
    std::uint32_t unused_arg3) {
  (void)unused_arg2;
  (void)unused_arg3;
  if (manager.at<std::uint8_t>(0xd0) != 0u) {
    return;
  }
  Raw game{
    reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)))
  };
  const std::int32_t slot = game.at<std::int32_t>(0x18ec);
  if (slot < 0 || slot > 7) {
    return;
  }
  if (host.global_f32(0x00f876a4u) <= host.global_f32(0x00ce3d34u)) {
    return;
  }

  Raw lock{ static_cast<std::uint8_t*>(manager.ptr(0x24)) };
  if (lock.p != nullptr) {
    host.enter_critical_section_00ce2218(lock.p);
    lock.at<std::int32_t>(0x18) += 1;
  }

  void* const raw_mem = host.operator_new_00bf681b(0x6cu);
  void* constructed = nullptr;
  if (raw_mem != nullptr) {
    std::uint8_t* const first = static_cast<std::uint8_t*>(string_vector.ptr(0x04));
    std::uint8_t* const last = static_cast<std::uint8_t*>(string_vector.ptr(0x08));
    if (first == nullptr || ((last - first) >> 3) == 0) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    constructed = host.message_warning_construct_with_id_00975ac0(
        raw_mem,
        string_vector.ptr(0x04));
  }

  host.warning_manager_report_009763e0(manager.p, constructed);

  if (lock.p != nullptr) {
    lock.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(lock.p);
  }
}

// 0x00976db0
void string_list_warning_activate_00976db0(Seg64Host& host, Raw warning) {
  warning.at<std::uint32_t>(0x0c) = 1u;
  warning.at<float>(0x10) = host.global_f32(0x00f876a4u);

  std::uint32_t index = 0u;
  for (;;) {
    std::uint8_t* const first = static_cast<std::uint8_t*>(warning.ptr(0x70));
    if (first == nullptr) {
      break;
    }
    std::uint8_t* const last = static_cast<std::uint8_t*>(warning.ptr(0x74));
    const std::uint32_t count =
        static_cast<std::uint32_t>((last - first) >> 3);
    if (index >= count) {
      break;
    }
    if (first == nullptr ||
        index >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(warning.ptr(0x74)) - first) >> 3)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Raw game{
      reinterpret_cast<std::uint8_t*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)))
    };
    void* const resolver = game.ptr(0x21dc);
    std::uint8_t* const elem_str =
        static_cast<std::uint8_t*>(warning.ptr(0x70)) + index * 8u;
    void* const resolved =
        host.message_record_resolve_named_00705e00(resolver, elem_str, 0u);
    MessageRef12 msg_ref{ host.image_address(0x00cf0dd0u), resolved, 3u };
    host.stl_vector_push_back_005bb020(warning.p + 0x14, &msg_ref);
    index += 1u;
  }
}

// 0x00976f10
void warning_manager_cancel_by_target_00976f10(
    Seg64Host& host,
    Raw manager,
    std::uint32_t target_id) {
  std::uint32_t id_key = target_id;
  Raw list{ manager.p + 0xe0 };
  Raw head_init{ static_cast<std::uint8_t*>(manager.ptr(0xe4)) };
  Raw node{ static_cast<std::uint8_t*>(head_init.ptr(0x00)) };
  std::uint8_t* const list_self = list.p;

  for (;;) {
    if (list_self != list.p) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    std::uint8_t* const head_cur = static_cast<std::uint8_t*>(list.ptr(0x04));
    if (node.p == head_cur) {
      break;
    }
    if (node.p == static_cast<std::uint8_t*>(list.ptr(0x04))) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Raw next{ static_cast<std::uint8_t*>(node.ptr(0x00)) };
    if (list.p == nullptr) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node.p == static_cast<std::uint8_t*>(list.ptr(0x04))) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Raw warning{ static_cast<std::uint8_t*>(node.ptr(0x08)) };
    if (host.warning_vslot10_get_kind(warning.p) == 4u &&
        warning.at<std::uint32_t>(0x80) == id_key) {
      host.warning_vslot00_deleting_dtor(warning.p, 1u);
      if (node.p == static_cast<std::uint8_t*>(list.ptr(0x04))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (node.p != static_cast<std::uint8_t*>(list.ptr(0x04))) {
        Raw prev{ static_cast<std::uint8_t*>(node.ptr(0x04)) };
        prev.ptr(0x00) = node.ptr(0x00);
        Raw next_link{ static_cast<std::uint8_t*>(node.ptr(0x00)) };
        next_link.ptr(0x04) = node.ptr(0x04);
        host.free_00bf65ac(node.p);
        list.at<std::int32_t>(0x08) -= 1;
      }
    }
    node = next;
  }

  const float deadline = static_cast<float>(
      static_cast<double>(host.global_f32(0x00f876a4u)) +
      host.global_f64(0x00d7a308u));
  float* const slot =
      host.stl_map_subscript_u32_f32_00975c40(manager.p + 0x15c, &id_key);
  *slot = deadline;
}

// 0x00977820
void warning_manager_play_collision_sound_00977820(
    Seg64Host& host,
    Raw manager,
    Raw ship_entity) {
  Raw lock{ static_cast<std::uint8_t*>(manager.ptr(0x24)) };
  if (lock.p != nullptr) {
    host.enter_critical_section_00ce2218(lock.p);
    lock.at<std::int32_t>(0x18) += 1;
  }

  std::uint32_t ship_key =
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(ship_entity.p));
  float* const record =
      ship_noise_map_subscript_00975d00(host, Raw{ manager.p + 0x184 }, &ship_key);
  const float clock = host.global_f32(0x00f876a4u);
  if (clock > record[0]) {
    record[0] = static_cast<float>(
        static_cast<double>(clock) + host.global_f64(0x00ce7630u));
    const float diag = host.global_f32(0x00d7a24cu);
    const float matrix[16] = {
      diag, 0.0f, 0.0f, 0.0f,
      0.0f, diag, 0.0f, 0.0f,
      0.0f, 0.0f, diag, 0.0f,
      0.0f, 0.0f, 0.0f, diag
    };
    void* const parent = ship_entity.ptr(0x4a4);
    void* consumed_def = nullptr;
    Raw def_ref{ static_cast<std::uint8_t*>(manager.ptr(0x190)) };
    if (def_ref.p != nullptr) {
      consumed_def = def_ref.p;
      host.interlocked_increment_00ce221c(&def_ref.at<std::int32_t>(0x04));
    }
    void* out_effect = nullptr;
    void** const result_slot = host.point_effect_create_with_parent_matrix_008687c0(
        &out_effect,
        parent,
        consumed_def,
        matrix,
        1u,
        0u,
        0u);
    Raw created{ static_cast<std::uint8_t*>(*result_slot) };
    if (out_effect != nullptr) {
      Raw out_ref{ static_cast<std::uint8_t*>(out_effect) };
      if (host.interlocked_decrement_00ce2220(&out_ref.at<std::int32_t>(0x04)) == 0) {
        host.ref_counted_vslot00_destroy(out_ref.p);
      }
    }
    created.at<std::uint8_t>(0x09) = 1u;
  }

  if (lock.p != nullptr) {
    lock.at<std::int32_t>(0x18) -= 1;
    host.leave_critical_section_00ce2210(lock.p);
  }
}

// 0x00977990
void warning_manager_scan_proximity_00977990(Seg64Host& host, Raw manager) {
  Raw game{
    reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)))
  };
  Raw world{ static_cast<std::uint8_t*>(game.ptr(0x19cc)) };
  Raw outer_node{ static_cast<std::uint8_t*>(world.ptr(0x64)) };

  while (outer_node.p != nullptr) {
    Raw ship{ static_cast<std::uint8_t*>(outer_node.ptr(0x08)) };
    if (ship.at<std::uint8_t>(0x5c) != 0u &&
        ship.at<std::uint8_t>(0x5d) == 0u &&
        ship.at<std::uint8_t>(0x60) == 0u &&
        ship.at<std::uint8_t>(0x5e) == 0u) {
      std::uint8_t* const arr_begin = static_cast<std::uint8_t*>(ship.ptr(0x348));
      std::uint8_t* const arr_end = static_cast<std::uint8_t*>(ship.ptr(0x34c));
      if (arr_begin != nullptr &&
          static_cast<std::uint32_t>((arr_end - arr_begin) >> 2) >= 2u) {
        bool nearby_hostile = false;
        if (ship.at<std::uint8_t>(0xc8) == 0u) {
          host.entity_pose_refresh_world_00414db0(ship.p);
        }

        Raw game_inner{
          reinterpret_cast<std::uint8_t*>(
              static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)))
        };
        Raw world_inner{ static_cast<std::uint8_t*>(game_inner.ptr(0x19cc)) };
        Raw inner_node{ static_cast<std::uint8_t*>(world_inner.ptr(0x13c)) };
        while (inner_node.p != nullptr) {
          Raw other{ static_cast<std::uint8_t*>(inner_node.ptr(0x08)) };
          if (other.at<std::uint8_t>(0x5c) != 0u &&
              other.at<std::uint8_t>(0x5d) == 0u &&
              other.at<std::uint8_t>(0x60) == 0u &&
              other.at<std::uint8_t>(0x5e) == 0u &&
              ship.at<std::int32_t>(0x54) != other.at<std::int32_t>(0x54) &&
              host.recon_relation_if_identified_00803ce0(
                  ship.at<std::int32_t>(0x54),
                  other.p) == 1u) {
            Raw other_pose{ static_cast<std::uint8_t*>(other.ptr(0x3d0)) };
            if (other_pose.at<std::uint8_t>(0xc8) == 0u) {
              host.entity_pose_refresh_world_00414db0(other_pose.p);
            }
            const float dx = ship.at<float>(0xfc) - other_pose.at<float>(0xfc);
            const float dy = ship.at<float>(0x100) - other_pose.at<float>(0x100);
            const float dz = ship.at<float>(0x104) - other_pose.at<float>(0x104);
            const float dist_sq = dx * dx + dy * dy + dz * dz;
            if (host.global_f64(0x00d09fe8u) > static_cast<double>(dist_sq)) {
              nearby_hostile = true;
              break;
            }
          }
          inner_node = Raw{ static_cast<std::uint8_t*>(inner_node.ptr(0x04)) };
        }

        std::uint32_t ship_key =
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(ship.p));
        Raw record{
          reinterpret_cast<std::uint8_t*>(ship_noise_map_subscript_00975d00(
              host,
              Raw{ manager.p + 0x184 },
              &ship_key))
        };
        if (nearby_hostile) {
          host.ship_noise_info_start_air_raid_effect_0096c070(
              manager.p,
              record.p,
              ship.p);
          record.at<float>(0x04) = static_cast<float>(
              static_cast<double>(host.global_f32(0x00f876a4u)) +
              host.global_f64(0x00d7a308u));
        } else if (host.global_f32(0x00f876a4u) > record.at<float>(0x04)) {
          Raw effect{ static_cast<std::uint8_t*>(record.ptr(0x08)) };
          if (effect.p != nullptr) {
            host.point_effect_stop_children_00867b10(effect.p);
            effect.at<std::uint8_t>(0x09) = 1u;
            Raw ref{ static_cast<std::uint8_t*>(record.ptr(0x08)) };
            if (ref.p != nullptr) {
              if (host.interlocked_decrement_00ce2220(&ref.at<std::int32_t>(0x04)) == 0) {
                host.ref_counted_vslot00_destroy(ref.p);
              }
              record.ptr(0x08) = nullptr;
            }
            record.ptr(0x08) = nullptr;
          }
        }
      }
    }
    outer_node = Raw{ static_cast<std::uint8_t*>(outer_node.ptr(0x04)) };
  }
}

// 0x00978fe0
void* string_list_warning_scalar_deleting_dtor_00978fe0(
    Seg64Host& host,
    Raw warning,
    std::uint8_t flags) {
  host.string_list_warning_destroy_009762a0(warning.p);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(warning.p);
  }
  return warning.p;
}

// 0x00979140
std::uint32_t subscription_set_matches_or_empty_00979140(
    Seg64Host& host,
    Raw subscription_set,
    Raw candidate) {
  if (subscription_set.at<std::uint32_t>(0x0c) != 0u) {
    void* const head = subscription_set.ptr(0x08);
    std::uint8_t* const tree = subscription_set.p + 0x04;
    TreeIteratorPair found{ nullptr, nullptr };
    auto* const res = static_cast<TreeIteratorPair*>(
        host.stl_tree_find_u16_009790d0(tree, &found, candidate.p + 0x04));
    if (res->container == nullptr || res->container != tree) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (res->node == head) {
      return 0u;
    }
  }
  return 1u;
}

// 0x0097a130
void* subscription_tree_owner_scalar_deleting_dtor_0097a130(
    Seg64Host& host,
    Raw owner,
    std::uint8_t flags) {
  Raw tree{ owner.p + 0x10 };
  Raw head{ static_cast<std::uint8_t*>(owner.ptr(0x14)) };
  void* const first = head.ptr(0x00);
  TreeIteratorPair erased{ nullptr, nullptr };
  host.stl_tree_erase_range_00492a00(
      tree.p,
      &erased,
      tree.p,
      first,
      tree.p,
      head.p);
  host.free_00bf65ac(tree.ptr(0x04));
  tree.ptr(0x04) = nullptr;
  tree.at<std::uint32_t>(0x08) = 0u;
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(owner.p);
  }
  return owner.p;
}

// 0x0097aaf0
void* fire_subscription_construct_0097aaf0(Seg64Host& host, Raw subscription) {
  subscription.at<std::uint32_t>(0x04) = 0u;
  subscription.at<std::uint32_t>(0x08) = 0u;
  subscription.ptr(0x00) = host.image_address(0x00d1b6dcu);
  subscription.ptr(0x0c) = host.image_address(0x00d1b670u);
  Raw tree{ subscription.p + 0x10 };
  Raw head{
    static_cast<std::uint8_t*>(host.stl_tree_buy_head_node_0096ba00(tree.p))
  };
  tree.ptr(0x04) = head.p;
  head.at<std::uint8_t>(0x0f) = 1u;
  Raw head_parent{ static_cast<std::uint8_t*>(tree.ptr(0x04)) };
  head_parent.ptr(0x04) = head_parent.p;
  Raw head_left{ static_cast<std::uint8_t*>(tree.ptr(0x04)) };
  head_left.ptr(0x00) = head_left.p;
  Raw head_right{ static_cast<std::uint8_t*>(tree.ptr(0x04)) };
  head_right.ptr(0x08) = head_right.p;
  tree.at<std::uint32_t>(0x08) = 0u;
  subscription.ptr(0x1c) = host.image_address(0x00d1af44u);
  return subscription.p;
}

}  // namespace bsp::warning_string_seg64_argon
