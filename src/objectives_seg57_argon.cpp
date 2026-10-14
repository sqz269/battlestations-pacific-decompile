#include "bsp/objectives_seg57_argon.hpp"

#include <cstring>

namespace bsp::objectives_seg57_argon {

namespace {

inline Raw raw_of(void* p) noexcept {
  return Raw{static_cast<std::uint8_t*>(p)};
}

inline Raw raw_of(const void* p) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(p))};
}

inline void copy_native_string_if_not_alias(
    Seg57Host& host, void* dst_str, const void* src_str) {
  if (dst_str == src_str) {
    return;
  }
  const Raw dst = raw_of(dst_str);
  const Raw src = raw_of(src_str);
  host.native_string_resize_0041dd40(dst_str, src.at<std::uint32_t>(0x00), 1u);
  if (src.at<std::uint32_t>(0x00) != 0u) {
    std::memcpy(
        dst.ptr(0x04),
        src.ptr(0x04),
        static_cast<std::size_t>(dst.at<std::uint32_t>(0x00)));
  }
}

inline void release_temp_native_string(
    Seg57Host& host, const NativeStringView& str) {
  if (str.data != nullptr) {
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(
        pool, str.data, str.length + 1u, 1);
  }
}

inline bool native_string_equals_case_insensitive(
    Seg57Host& host, const void* lhs_str, const void* rhs_str) {
  const Raw lhs = raw_of(lhs_str);
  const Raw rhs = raw_of(rhs_str);
  const std::uint32_t lhs_len = lhs.at<std::uint32_t>(0x00);
  const std::uint32_t rhs_len = rhs.at<std::uint32_t>(0x00);
  if (lhs_len != rhs_len) {
    return false;
  }
  std::int32_t cmp = 0;
  if (lhs_len == 0u) {
    cmp = (rhs_len != 0u) ? -1 : 0;
  } else if (rhs_len == 0u) {
    cmp = 1;
  } else {
    cmp = host.stricmp_00bf7fbf(
        static_cast<const char*>(lhs.ptr(0x04)),
        static_cast<const char*>(rhs.ptr(0x04)));
  }
  return cmp == 0;
}

struct TempPtrVector {
  std::uint32_t reserved{0};
  void** begin{nullptr};
  void** end{nullptr};
  void** capacity{nullptr};
};

}  // namespace

// 008dd380: BSP_NativeString_ReleaseStorage_008dd380
void native_string_release_storage_008dd380(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  void* const data = s.ptr(0x04);
  if (data != nullptr) {
    const std::uint32_t bytes = s.at<std::uint32_t>(0x00) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// 008dd460: BSP_ObjectiveSounds_PlayThrottled
void objective_sounds_play_throttled_008dd460(
    Seg57Host& host, void* self, std::int32_t sample_index) {
  const Raw s = raw_of(self);
  const double next_allowed =
      static_cast<double>(s.at<float>(0x18)) + host.global_f64(0x00d7a210u);
  const double now = static_cast<double>(host.global_f32(0x00f876a4u));
  if (!(now > next_allowed)) {
    return;
  }

  if ((host.global_u32(0x00f88c1cu) & 1u) == 0u) {
    host.global_u32(0x00f88c1cu) |= 1u;
    NativeStringView class_name{};
    host.native_string_assign_0041e870(
        &class_name,
        static_cast<const char*>(host.image_address(0x00ce7854u)));
    void* const sound_mgr =
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            host.global_u32(0x00f8bbd8u)));
    const std::int32_t class_idx =
        host.sound_manager_find_class_by_name_00a7acf0(sound_mgr, &class_name);
    host.global_u32(0x00f88c18u) = static_cast<std::uint32_t>(class_idx);
    release_temp_native_string(host, class_name);
  }

  NativeStringView type_name{};
  host.native_string_assign_0041e870(
      &type_name, static_cast<const char*>(host.image_address(0x00cecd20u)));
  void* const sample =
      s.at<void*>(static_cast<std::size_t>(sample_index) * 4u);
  void* const sound_mgr = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00f8bbd8u)));
  const std::int32_t type_idx =
      host.sound_manager_find_type_by_name_00a7b0a0(sound_mgr, &type_name);
  void* tracked_sound = nullptr;
  host.sound_manager_create_tracked_sound_00a7e490(
      sound_mgr,
      &tracked_sound,
      sample,
      static_cast<std::int32_t>(host.global_u32(0x00f88c18u)),
      type_idx,
      1u);
  if (tracked_sound != nullptr) {
    std::int32_t* const ref_count =
        &raw_of(tracked_sound).at<std::int32_t>(0x04);
    if (host.interlocked_decrement_00ce2220(ref_count) == 0) {
      host.vcall_tracked_sound_destroy_00(tracked_sound);
    }
    tracked_sound = nullptr;
  }
  release_temp_native_string(host, type_name);
  s.at<float>(0x18) = host.global_f32(0x00f876a4u);
}

// 008dd5c0: BSP_Objective_Construct
void* objective_construct_008dd5c0(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* text_str,
    std::int32_t kind,
    std::uint8_t quiet_flag) {
  const Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00d16038u);
  s.at<std::uint32_t>(0x04) = 0u;
  s.ptr(0x08) = nullptr;
  s.at<std::uint32_t>(0x0c) = 0u;
  s.ptr(0x10) = nullptr;
  s.ptr(0x24) = host.stl_pointer_list_allocate_sentinel_008db4f0();
  s.at<std::uint32_t>(0x28) = 0u;
  copy_native_string_if_not_alias(host, s.p + 0x04, id_str);
  copy_native_string_if_not_alias(host, s.p + 0x0c, text_str);
  s.at<std::uint8_t>(0x14) = quiet_flag;
  s.at<std::int32_t>(0x18) = kind;
  s.at<std::int32_t>(0x1c) = 0;
  return self;
}

// 008dd6a0: BSP_SessionMessage56_ConstructObjAdd
void* session_message56_construct_obj_add_008dd6a0(
    Seg57Host& host, void* self, const void* objective) {
  const Raw s = raw_of(self);
  const Raw obj = raw_of(objective);
  host.session_message_construct_base_0075b430(self, 0x38u);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d16050u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  s.at<std::uint32_t>(0x20) = 0u;
  s.ptr(0x24) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, obj.p + 0x04);
  copy_native_string_if_not_alias(host, s.p + 0x20, obj.p + 0x0c);
  s.at<std::int32_t>(0x28) = obj.at<std::int32_t>(0x18);
  s.at<std::uint8_t>(0x2c) = obj.at<std::uint8_t>(0x14);
  return self;
}

// 008dd780: BSP_SessionMessage57_ConstructObjAddEntity
void* session_message57_construct_obj_add_entity_008dd780(
    Seg57Host& host, void* self, const void* name_str, const void* entity) {
  const Raw s = raw_of(self);
  const Raw ent = raw_of(entity);
  host.session_message_construct_base_0075b430(self, 0x39u);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d16064u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, name_str);
  s.at<std::uint16_t>(0x20) = ent.at<std::uint16_t>(0x174);
  return self;
}

// 008dd820: BSP_SessionMessage58_ConstructObjRemoveEntity
void* session_message58_construct_obj_remove_entity_008dd820(
    Seg57Host& host, void* self, const void* name_str, const void* entity) {
  const Raw s = raw_of(self);
  const Raw ent = raw_of(entity);
  host.session_message_construct_base_0075b430(self, 0x3au);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d16078u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, name_str);
  s.at<std::uint16_t>(0x20) = ent.at<std::uint16_t>(0x174);
  return self;
}

// 008dd8c0: BSP_SessionMessage59_ConstructObjAddPosition
void* session_message59_construct_obj_add_position_008dd8c0(
    Seg57Host& host, void* self, const void* name_str, const float* pos3) {
  const Raw s = raw_of(self);
  host.session_message_construct_base_0075b430(self, 0x3bu);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d1608cu);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, name_str);
  s.at<float>(0x20) = pos3[0];
  s.at<float>(0x24) = pos3[1];
  s.at<float>(0x28) = pos3[2];
  return self;
}

// 008dd970: BSP_SessionMessage60_ConstructObjRemovePosition
void* session_message60_construct_obj_remove_position_008dd970(
    Seg57Host& host, void* self, const void* name_str, const float* pos3) {
  const Raw s = raw_of(self);
  host.session_message_construct_base_0075b430(self, 0x3cu);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d160a0u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, name_str);
  s.at<float>(0x20) = pos3[0];
  s.at<float>(0x24) = pos3[1];
  s.at<float>(0x28) = pos3[2];
  return self;
}

// 008dda20: BSP_SessionMessage61_ConstructObjCompleted
void* session_message61_construct_obj_completed_008dda20(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* comment_str,
    std::uint8_t silent_flag) {
  const Raw s = raw_of(self);
  const Raw obj = raw_of(objective);
  host.session_message_construct_base_0075b430(self, 0x3du);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d160b4u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  s.at<std::uint32_t>(0x20) = 0u;
  s.ptr(0x24) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, obj.p + 0x04);
  copy_native_string_if_not_alias(host, s.p + 0x20, comment_str);
  s.at<std::uint8_t>(0x28) = silent_flag;
  return self;
}

// 008ddb00: BSP_SessionMessage62_ConstructObjFailed
void* session_message62_construct_obj_failed_008ddb00(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* comment_str,
    std::uint8_t silent_flag) {
  const Raw s = raw_of(self);
  const Raw obj = raw_of(objective);
  host.session_message_construct_base_0075b430(self, 0x3eu);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d160c8u);
  s.at<std::uint32_t>(0x18) = 0u;
  s.ptr(0x1c) = nullptr;
  s.at<std::uint32_t>(0x20) = 0u;
  s.ptr(0x24) = nullptr;
  copy_native_string_if_not_alias(host, s.p + 0x18, obj.p + 0x04);
  copy_native_string_if_not_alias(host, s.p + 0x20, comment_str);
  s.at<std::uint8_t>(0x28) = silent_flag;
  return self;
}

// 008ddbe0: BSP_ObjectiveSync_DescribeObjectives
void objective_sync_describe_objectives_008ddbe0(
    Seg57Host& host, void* self, void* archive_writer) {
  const Raw s = raw_of(self);
  host.vcall_archive_enter_block_04(
      archive_writer, 0u, host.image_address(0x00d160ecu));

  void* iter[2]{s.p + 0x18, raw_of(s.ptr(0x1c)).ptr(0x00)};
  std::int32_t index = 0;
  for (;;) {
    void* const end_node = s.ptr(0x1c);
    void* const expected_tree = s.p + 0x18;
    if (iter[0] == nullptr || iter[0] != expected_tree) {
      host.invalid_parameter_00bf6713();
    }
    if (iter[1] == end_node) {
      break;
    }
    if (iter[0] == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (iter[1] == raw_of(iter[0]).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* const entity_val = raw_of(iter[1]).ptr(0x0c);
    host.vcall_archive_write_property_0c(
        archive_writer, 1u, index, 4u, entity_val);
    host.stl_tree_iter_inc_008da530(iter);
    ++index;
  }
  host.vcall_archive_leave_block_08(archive_writer);

  host.vcall_archive_enter_block_04(
      archive_writer, 0u, host.image_address(0x00d160dcu));
  void* const list_container = s.p + 0x24;
  void* node = raw_of(s.ptr(0x28)).ptr(0x00);
  std::int32_t obj_index = 0;
  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      break;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* const entry = raw_of(node).ptr(0x08);
    host.vcall_archive_enter_block_04(
        archive_writer,
        1u,
        reinterpret_cast<const void*>(static_cast<std::intptr_t>(obj_index)));
    host.map_ping_entry_describe_properties_008dc210(entry, archive_writer);
    host.vcall_archive_leave_block_08(archive_writer);
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
    ++obj_index;
  }
  host.vcall_archive_leave_block_08(archive_writer);
}

// 008ddd30: BSP_ObjectiveSet_RemovePositionFromNamedObjective
void objective_set_remove_position_from_named_objective_008ddd30(
    Seg57Host& host,
    void* self,
    const void* name_str,
    float x,
    float y,
    float z) {
  const Raw s = raw_of(self);
  const float pos[3]{x, y, z};

  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    const std::int32_t slot = s.at<std::int32_t>(0x14);
    void* const player = raw_of(game).at<void*>(
        0x18ccu + static_cast<std::size_t>(slot) * 4u);
    if (raw_of(player).ptr(0x50) != nullptr) {
      std::uint8_t msg60[0x2c]{};
      void* const built =
          session_message60_construct_obj_remove_position_008dd970(
              host, msg60, name_str, pos);
      void* const game_now = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      const std::int32_t slot_now = s.at<std::int32_t>(0x14);
      void* const player_now = raw_of(game_now).at<void*>(
          0x18ccu + static_cast<std::size_t>(slot_now) * 4u);
      void* const peer = raw_of(player_now).ptr(0x50);
      host.session_send_message_to_nonlocal_peer_00770b50(
          raw_of(game_now).p + 0x1ef0, peer, built);
      host.session_message60_destroy_008dce40(msg60);
    }
  }

  void* const list_container = s.p + 0x24;
  void* node = raw_of(s.ptr(0x28)).ptr(0x00);
  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      return;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* const obj = raw_of(node).ptr(0x08);
    if (native_string_equals_case_insensitive(host, raw_of(obj).p + 0x04, name_str)) {
      if (node == raw_of(list_container).ptr(0x04)) {
        host.invalid_parameter_00bf6713();
      }
      host.objective_erase_position_entries_008dc500(raw_of(node).ptr(0x08), pos);
      NativeStringView marker_name{};
      host.native_string_resize_0041dd40(&marker_name, 10u, 1u);
      if (marker_name.data != nullptr) {
        std::memcpy(
            marker_name.data,
            host.image_address(0x00d16100u),
            static_cast<std::size_t>(marker_name.length + 1u));
      }
      void* const game_mgr = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      void* const marker_mgr = raw_of(game_mgr).ptr(0x21d4);
      host.marker_manager_clear_position_marker_006de4c0(
          marker_mgr, &marker_name, pos[0], pos[1], pos[2]);
      release_temp_native_string(host, marker_name);
      return;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }
}

// 008ddf40: BSP_ObjectiveSync_ApplyObjRemovePosition
void objective_sync_apply_obj_remove_position_008ddf40(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  objective_set_remove_position_from_named_objective_008ddd30(
      host,
      self,
      m.p + 0x18,
      m.at<float>(0x20),
      m.at<float>(0x24),
      m.at<float>(0x28));
}

// 008deb40: BSP_ObjectiveSet_EraseNamedEntries
std::uint32_t objective_set_erase_named_entries_008deb40(
    Seg57Host& host, void* tree_self, const void* key_ptr) {
  void* range[4]{};
  host.stl_tree_equal_range_008dbba0(tree_self, range, key_ptr);
  std::uint32_t count = 0u;
  host.stl_tree_distance_008db950(
      range[0], range[1], range[2], range[3], &count, 0u);
  void* out_iter[2]{};
  host.stl_tree_erase_range_008de7e0(
      tree_self, out_iter, range[0], range[1], range[2], range[3]);
  return count;
}

// 008debc0: BSP_Receive28Range_Destroy_Forward
void receive28_range_destroy_forward_008debc0(
    Seg57Host& host, void* self, void* first, void* last) {
  host.stl_receive28_range_destroy_008de660(first, last, self, last);
}

// 008dec10: BSP_SessionMessage55Entry_ReadProperties
void* session_message55_entry_read_properties_008dec10(
    Seg57Host& host, void* self, void* archive_reader) {
  const Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00d16038u);
  s.at<std::uint32_t>(0x04) = 0u;
  s.ptr(0x08) = nullptr;
  s.at<std::uint32_t>(0x0c) = 0u;
  s.ptr(0x10) = nullptr;
  void* const list_self = s.p + 0x20;
  s.ptr(0x24) = host.stl_pointer_list_allocate_sentinel_008db4f0();
  s.at<std::uint32_t>(0x28) = 0u;

  host.vcall_archive_read_property_10(
      archive_reader, 0u, host.image_address(0x00ce59b4u), 0u, s.p + 0x04);
  host.vcall_archive_read_property_10(
      archive_reader, 0u, host.image_address(0x00cf1010u), 0u, s.p + 0x0c);
  host.vcall_archive_read_property_10(
      archive_reader, 0u, host.image_address(0x00d16048u), 3u, s.p + 0x14);

  std::int32_t temp_type = 0;
  host.vcall_archive_read_property_10(
      archive_reader, 0u, host.image_address(0x00ce4ad0u), 1u, &temp_type);
  s.at<std::int32_t>(0x18) = temp_type;

  std::int32_t temp_state = 0;
  host.vcall_archive_read_property_10(
      archive_reader, 0u, host.image_address(0x00ce7324u), 1u, &temp_state);
  s.at<std::int32_t>(0x1c) = temp_state;

  host.vcall_archive_enter_block_04(
      archive_reader, 0u, host.image_address(0x00d1603cu));
  for (std::int32_t idx = 0;; ++idx) {
    if (host.vcall_archive_has_entry_14(archive_reader, 1u, idx) == 0u) {
      break;
    }
    host.vcall_archive_enter_block_04(
        archive_reader,
        1u,
        reinterpret_cast<const void*>(static_cast<std::intptr_t>(idx)));
    void* const raw_entry = host.operator_new_00bf681b(0x10u);
    void* entry = nullptr;
    if (raw_entry != nullptr) {
      entry = host.objective_entry_select_entity_or_pos_key_008db990(
          raw_entry, archive_reader);
    }
    void* const head = s.ptr(0x24);
    void* const prev = raw_of(head).ptr(0x04);
    void* const new_node =
        host.stl_list_buy_node4_008db650(list_self, head, prev, &entry);
    host.stl_xlen_throw_008de200(list_self, 1u);
    raw_of(head).ptr(0x04) = new_node;
    raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;
    host.vcall_archive_leave_block_08(archive_reader);
  }
  host.vcall_archive_leave_block_08(archive_reader);
  return self;
}

// 008def60: BSP_ObjectiveSync_Destruct
void objective_sync_destruct_008def60(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00d1610cu);
  host.objective_set_destroy_objectives_008dd1c0(self);
  host.stl_pointer_list_destroy_008db580(s.p + 0x24);

  void* const tree = s.p + 0x18;
  void* const head = s.ptr(0x1c);
  void* const first = raw_of(head).ptr(0x00);
  void* out_iter[2]{};
  host.stl_tree_erase_range_008de7e0(tree, out_iter, tree, first, tree, head);
  host.free_00bf65ac(s.ptr(0x1c));
  s.ptr(0x1c) = nullptr;
  s.at<std::uint32_t>(0x20) = 0u;
  host.callback_owner_destroy_00695870(self);
}

// 008df000: BSP_ObjectiveSync_ReadObjectives
void objective_sync_read_objectives_008df000(
    Seg57Host& host, void* self, void* archive_reader) {
  const Raw s = raw_of(self);
  host.vcall_archive_enter_block_04(
      archive_reader, 0u, host.image_address(0x00d160ecu));
  for (std::int32_t idx = 0;; ++idx) {
    if (host.vcall_archive_has_entry_14(archive_reader, 1u, idx) == 0u) {
      break;
    }
    void* entity = nullptr;
    host.vcall_archive_read_property_10(
        archive_reader,
        1u,
        reinterpret_cast<const void*>(static_cast<std::intptr_t>(idx)),
        4u,
        &entity);
    void* insert_res[3]{};
    host.stl_set_ptr_insert_008de6a0(s.p + 0x18, insert_res, &entity);
    if (host.observer_is_pair_registered_00694af0(entity, self) == 0u) {
      host.observer_register_pair_00694a60(entity, self);
    }
  }
  host.vcall_archive_leave_block_08(archive_reader);

  host.vcall_archive_enter_block_04(
      archive_reader, 0u, host.image_address(0x00d160dcu));
  for (std::int32_t idx = 0;; ++idx) {
    if (host.vcall_archive_has_entry_14(archive_reader, 1u, idx) == 0u) {
      break;
    }
    host.vcall_archive_enter_block_04(
        archive_reader,
        1u,
        reinterpret_cast<const void*>(static_cast<std::intptr_t>(idx)));
    void* const raw_obj = host.operator_new_00bf681b(0x2cu);
    void* obj = nullptr;
    if (raw_obj != nullptr) {
      obj = session_message55_entry_read_properties_008dec10(
          host, raw_obj, archive_reader);
    }
    void* const list_self = s.p + 0x24;
    void* const head = s.ptr(0x28);
    void* const prev = raw_of(head).ptr(0x04);
    void* const new_node =
        host.stl_list_buy_node4_008db6a0(list_self, head, prev, &obj);
    host.stl_list_checked_size_add_008de2a0(list_self, 1u);
    raw_of(head).ptr(0x04) = new_node;
    raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;
    host.vcall_archive_leave_block_08(archive_reader);
  }
  host.vcall_archive_leave_block_08(archive_reader);
}

// 008df1e0: BSP_ObjectiveSync_DetachObservers
void objective_sync_detach_observers_008df1e0(
    Seg57Host& host, void* self, void* entity) {
  const Raw s = raw_of(self);
  if (host.observer_is_pair_registered_00694af0(entity, self) != 0u) {
    host.observer_unregister_pair_006952a0(entity, self);
  }

  void* const list_container = s.p + 0x24;
  void* node = raw_of(s.ptr(0x28)).ptr(0x00);
  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      break;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    host.objective_erase_unit_entries_008dc3e0(raw_of(node).ptr(0x08), entity);
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }

  void* const tree_self = s.p + 0x18;
  void* range[4]{};
  host.stl_tree_equal_range_008dbba0(tree_self, range, &entity);
  std::uint32_t count = 0u;
  host.stl_tree_distance_008db950(
      range[0], range[1], range[2], range[3], &count, 0u);
  void* out_iter[2]{};
  host.stl_tree_erase_range_008de7e0(
      tree_self, out_iter, range[0], range[1], range[2], range[3]);
}

// 008df400: BSP_ObjectiveSet_RemoveUnitFromNamedObjective
void objective_set_remove_unit_from_named_objective_008df400(
    Seg57Host& host, void* self, const void* name_str, void* entity) {
  const Raw s = raw_of(self);
  void* const list_container = s.p + 0x24;
  void* node = raw_of(s.ptr(0x28)).ptr(0x00);
  std::uint8_t still_referenced = 0u;

  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      break;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* const obj = raw_of(node).ptr(0x08);
    if (native_string_equals_case_insensitive(host, raw_of(obj).p + 0x04, name_str)) {
      if (node == raw_of(list_container).ptr(0x04)) {
        host.invalid_parameter_00bf6713();
      }
      host.objective_erase_unit_entries_008dc3e0(raw_of(node).ptr(0x08), entity);
    } else {
      if (node == raw_of(list_container).ptr(0x04)) {
        host.invalid_parameter_00bf6713();
      }
      void* const other_obj = raw_of(node).ptr(0x08);
      if (raw_of(other_obj).at<std::int32_t>(0x1c) == 0) {
        if (node == raw_of(list_container).ptr(0x04)) {
          host.invalid_parameter_00bf6713();
        }
        void* const check_obj = raw_of(node).ptr(0x08);
        if (raw_of(check_obj).at<std::int32_t>(0x18) != 2 ||
            raw_of(check_obj).at<std::int32_t>(0x1c) == 1) {
          if (node == raw_of(list_container).ptr(0x04)) {
            host.invalid_parameter_00bf6713();
          }
          still_referenced |= host.objective_contains_unit_008dc4a0(
              raw_of(node).ptr(0x08), entity);
        }
      }
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }

  if (still_referenced == 0u) {
    if (host.observer_is_pair_registered_00694af0(entity, self) != 0u) {
      host.observer_unregister_pair_006952a0(entity, self);
    }
    objective_set_erase_named_entries_008deb40(host, s.p + 0x18, &entity);
    NativeStringView marker_name{};
    host.native_string_resize_0041dd40(&marker_name, 10u, 1u);
    if (marker_name.data != nullptr) {
      std::memcpy(
          marker_name.data,
          host.image_address(0x00d16100u),
          static_cast<std::size_t>(marker_name.length + 1u));
    }
    void* const game = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
    void* const marker_mgr = raw_of(game).ptr(0x21d4);
    host.marker_manager_drop_unit_marker_006de3f0(
        marker_mgr, &marker_name, entity);
    release_temp_native_string(host, marker_name);
  }
}

// 008df800: BSP_ObjectiveSync_ApplyObjAddPosition
void objective_sync_apply_obj_add_position_008df800(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  host.objective_set_add_position_to_named_objective_008df5d0(
      self,
      m.p + 0x18,
      m.at<float>(0x20),
      m.at<float>(0x24),
      m.at<float>(0x28));
}

// 008df860: BSP_Receive28Vector_Tidy_008df860
void receive28_vector_tidy_008df860(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  if (s.ptr(0x04) != nullptr) {
    host.stl_receive28_range_destroy_008de660(
        s.ptr(0x04), s.ptr(0x08), self, self);
    host.free_00bf65ac(s.ptr(0x04));
  }
  s.ptr(0x04) = nullptr;
  s.ptr(0x08) = nullptr;
  s.ptr(0x0c) = nullptr;
}

// 008df900: BSP_ObjectiveSet_Construct
void* objective_set_construct_008df900(
    Seg57Host& host, void* self, std::int32_t slot_index) {
  const Raw s = raw_of(self);
  s.ptr(0x04) = nullptr;
  s.ptr(0x08) = nullptr;
  s.ptr(0x0c) = nullptr;
  s.at<std::uint8_t>(0x10) = 0u;
  s.ptr(0x00) = host.image_address(0x00d1610cu);
  s.at<std::int32_t>(0x14) = slot_index;

  void* const tree_head = host.stl_tree_buy_head_node_008db7b0();
  s.ptr(0x1c) = tree_head;
  raw_of(tree_head).at<std::uint8_t>(0x11) = 1u;
  raw_of(s.ptr(0x1c)).ptr(0x04) = s.ptr(0x1c);
  raw_of(s.ptr(0x1c)).ptr(0x00) = s.ptr(0x1c);
  raw_of(s.ptr(0x1c)).ptr(0x08) = s.ptr(0x1c);
  s.at<std::uint32_t>(0x20) = 0u;

  s.ptr(0x28) = host.stl_pointer_list_allocate_sentinel_008db560();
  s.at<std::uint32_t>(0x2c) = 0u;
  return self;
}

// 008df9b0: BSP_ObjectiveSet_AddUnitExpandingGroups
void objective_set_add_unit_expanding_groups_008df9b0(
    Seg57Host& host, void* self, const void* name_str, void* unit) {
  const Raw s = raw_of(self);
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    const std::int32_t slot = s.at<std::int32_t>(0x14);
    void* const player = raw_of(game).at<void*>(
        0x18ccu + static_cast<std::size_t>(slot) * 4u);
    if (raw_of(player).ptr(0x50) != nullptr) {
      std::uint8_t msg57[0x24]{};
      void* const built = session_message57_construct_obj_add_entity_008dd780(
          host, msg57, name_str, unit);
      void* const game_now = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      const std::int32_t slot_now = s.at<std::int32_t>(0x14);
      void* const player_now = raw_of(game_now).at<void*>(
          0x18ccu + static_cast<std::size_t>(slot_now) * 4u);
      void* const peer = raw_of(player_now).ptr(0x50);
      host.session_send_message_to_nonlocal_peer_00770b50(
          raw_of(game_now).p + 0x1ef0, peer, built);
      host.session_message57_destroy_008dc980(msg57);
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 5) != 0u) {
    host.objective_set_add_unit_to_named_objective_008df2b0(
        self, name_str, unit);
    return;
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x18) != 0u) {
    const Raw u = raw_of(unit);
    const std::int32_t count = u.at<std::int32_t>(0x3cc);
    for (std::int32_t i = 0; i < count; ++i) {
      void* const member =
          (static_cast<std::uint32_t>(i) <= 4u)
              ? u.at<void*>(0x3d0u + static_cast<std::size_t>(i) * 4u)
              : nullptr;
      host.objective_set_add_unit_to_named_objective_008df2b0(
          self, name_str, member);
    }
    return;
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x1a) != 0u) {
    const Raw vec = raw_of(raw_of(unit).p + 0x394);
    void** it = vec.at<void**>(0x04);
    if (it > vec.at<void**>(0x08)) {
      host.invalid_parameter_00bf6713();
    }
    for (;;) {
      void** const end_it = vec.at<void**>(0x08);
      if (vec.at<void**>(0x04) > end_it) {
        host.invalid_parameter_00bf6713();
      }
      if (vec.p == nullptr) {
        host.invalid_parameter_00bf6713();
      }
      if (it == end_it) {
        return;
      }
      if (it >= vec.at<void**>(0x08)) {
        host.invalid_parameter_00bf6713();
      }
      host.objective_set_add_unit_to_named_objective_008df2b0(
          self, name_str, *it);
      if (it >= vec.at<void**>(0x08)) {
        host.invalid_parameter_00bf6713();
      }
      ++it;
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x45) != 0u) {
    TempPtrVector local_vec{};
    host.marker_group_vector_clear_006d6010(unit, &local_vec);
    for (std::size_t idx = 0u;; ++idx) {
      void** const begin = local_vec.begin;
      if (begin == nullptr) {
        return;
      }
      const std::size_t len = static_cast<std::size_t>(local_vec.end - begin);
      if (idx >= len) {
        host.free_00bf65ac(begin);
        return;
      }
      host.objective_set_add_unit_to_named_objective_008df2b0(
          self, name_str, begin[idx]);
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x46) != 0u) {
    TempPtrVector local_vec{};
    host.shipyard_collect_live_hangar_objects_00848410(unit, &local_vec);
    for (std::size_t idx = 0u;; ++idx) {
      void** const begin = local_vec.begin;
      if (begin == nullptr) {
        return;
      }
      const std::size_t len = static_cast<std::size_t>(local_vec.end - begin);
      if (idx >= len) {
        host.free_00bf65ac(begin);
        return;
      }
      host.objective_set_add_unit_to_named_objective_008df2b0(
          self, name_str, begin[idx]);
    }
  }
}

// 008dfc00: BSP_ObjectiveSet_RemoveUnitExpandingGroups
void objective_set_remove_unit_expanding_groups_008dfc00(
    Seg57Host& host, void* self, const void* name_str, void* unit) {
  const Raw s = raw_of(self);
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    const std::int32_t slot = s.at<std::int32_t>(0x14);
    void* const player = raw_of(game).at<void*>(
        0x18ccu + static_cast<std::size_t>(slot) * 4u);
    if (raw_of(player).ptr(0x50) != nullptr) {
      std::uint8_t msg58[0x24]{};
      void* const built =
          session_message58_construct_obj_remove_entity_008dd820(
              host, msg58, name_str, unit);
      void* const game_now = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      const std::int32_t slot_now = s.at<std::int32_t>(0x14);
      void* const player_now = raw_of(game_now).at<void*>(
          0x18ccu + static_cast<std::size_t>(slot_now) * 4u);
      void* const peer = raw_of(player_now).ptr(0x50);
      host.session_send_message_to_nonlocal_peer_00770b50(
          raw_of(game_now).p + 0x1ef0, peer, built);
      host.session_message58_destroy_008dcb40(msg58);
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 5) != 0u) {
    objective_set_remove_unit_from_named_objective_008df400(
        host, self, name_str, unit);
    return;
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x18) != 0u) {
    const Raw u = raw_of(unit);
    const std::int32_t count = u.at<std::int32_t>(0x3cc);
    for (std::int32_t i = 0; i < count; ++i) {
      void* const member =
          (static_cast<std::uint32_t>(i) <= 4u)
              ? u.at<void*>(0x3d0u + static_cast<std::size_t>(i) * 4u)
              : nullptr;
      objective_set_remove_unit_from_named_objective_008df400(
          host, self, name_str, member);
    }
    return;
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x1a) != 0u) {
    const Raw vec = raw_of(raw_of(unit).p + 0x394);
    void** it = vec.at<void**>(0x04);
    if (it > vec.at<void**>(0x08)) {
      host.invalid_parameter_00bf6713();
    }
    for (;;) {
      void** const end_it = vec.at<void**>(0x08);
      if (vec.at<void**>(0x04) > end_it) {
        host.invalid_parameter_00bf6713();
      }
      if (vec.p == nullptr) {
        host.invalid_parameter_00bf6713();
      }
      if (it == end_it) {
        return;
      }
      if (it >= vec.at<void**>(0x08)) {
        host.invalid_parameter_00bf6713();
      }
      objective_set_remove_unit_from_named_objective_008df400(
          host, self, name_str, *it);
      if (it >= vec.at<void**>(0x08)) {
        host.invalid_parameter_00bf6713();
      }
      ++it;
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x45) != 0u) {
    TempPtrVector local_vec{};
    host.marker_group_vector_clear_006d6010(unit, &local_vec);
    for (std::size_t idx = 0u;; ++idx) {
      void** const begin = local_vec.begin;
      if (begin == nullptr) {
        return;
      }
      const std::size_t len = static_cast<std::size_t>(local_vec.end - begin);
      if (idx >= len) {
        host.free_00bf65ac(begin);
        return;
      }
      objective_set_remove_unit_from_named_objective_008df400(
          host, self, name_str, begin[idx]);
    }
  }

  if (host.vcall_entity_is_kind_of_5c(unit, 0x46) != 0u) {
    TempPtrVector local_vec{};
    host.shipyard_collect_live_hangar_objects_00848410(unit, &local_vec);
    for (std::size_t idx = 0u;; ++idx) {
      void** const begin = local_vec.begin;
      if (begin == nullptr) {
        return;
      }
      const std::size_t len = static_cast<std::size_t>(local_vec.end - begin);
      if (idx >= len) {
        host.free_00bf65ac(begin);
        return;
      }
      objective_set_remove_unit_from_named_objective_008df400(
          host, self, name_str, begin[idx]);
    }
  }
}

// 008e0060: BSP_ObjectiveSync_ApplyObjAddEntity
void objective_sync_apply_obj_add_entity_008e0060(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  const std::int32_t handle =
      static_cast<std::int32_t>(m.at<std::uint16_t>(0x20));
  std::int32_t base_id = 0;
  std::uint32_t table_addr = 0u;
  if (handle < static_cast<std::int32_t>(host.global_u32(0x00f89a10u))) {
    base_id = static_cast<std::int32_t>(host.global_u32(0x00f89a0cu));
    table_addr = host.global_u32(0x00f89a54u);
  } else {
    base_id = static_cast<std::int32_t>(host.global_u32(0x00f89a60u));
    table_addr = host.global_u32(0x00f89aa8u);
  }
  const Raw table =
      raw_of(reinterpret_cast<void*>(static_cast<std::uintptr_t>(table_addr)));
  void* const unit = table.at<void*>(
      static_cast<std::size_t>(handle - base_id) * 0x10u + 0x0cu);
  objective_set_add_unit_expanding_groups_008df9b0(
      host, self, m.p + 0x18, unit);
}

// 008e00b0: BSP_ObjectiveSync_ApplyObjRemoveEntity
void objective_sync_apply_obj_remove_entity_008e00b0(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  const std::int32_t handle =
      static_cast<std::int32_t>(m.at<std::uint16_t>(0x20));
  std::int32_t base_id = 0;
  std::uint32_t table_addr = 0u;
  if (handle < static_cast<std::int32_t>(host.global_u32(0x00f89a10u))) {
    base_id = static_cast<std::int32_t>(host.global_u32(0x00f89a0cu));
    table_addr = host.global_u32(0x00f89a54u);
  } else {
    base_id = static_cast<std::int32_t>(host.global_u32(0x00f89a60u));
    table_addr = host.global_u32(0x00f89aa8u);
  }
  const Raw table =
      raw_of(reinterpret_cast<void*>(static_cast<std::uintptr_t>(table_addr)));
  void* const unit = table.at<void*>(
      static_cast<std::size_t>(handle - base_id) * 0x10u + 0x0cu);
  objective_set_remove_unit_expanding_groups_008dfc00(
      host, self, m.p + 0x18, unit);
}

// 008e0100: BSP_Receive28Vector_Tidy_008e0100
void receive28_vector_tidy_008e0100(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  if (s.ptr(0x04) != nullptr) {
    host.stl_receive28_range_destroy_008de660(
        s.ptr(0x04), s.ptr(0x08), self, self);
    host.free_00bf65ac(s.ptr(0x04));
  }
  s.ptr(0x04) = nullptr;
  s.ptr(0x08) = nullptr;
  s.ptr(0x0c) = nullptr;
}

// 008e1380: BSP_SessionMessage55_ConstructObjCreateSnapshot
void* session_message55_construct_obj_create_snapshot_008e1380(
    Seg57Host& host, void* self, const void* objective_set) {
  const Raw s = raw_of(self);
  const Raw set = raw_of(objective_set);
  host.session_message_construct_base_0075b430(self, 0x37u);
  s.at<std::uint32_t>(0x04) = 1u;
  s.ptr(0x00) = host.image_address(0x00d16124u);
  void* const dst_list = s.p + 0x18;
  s.ptr(0x1c) = host.stl_pointer_list_allocate_sentinel_008db560();
  s.at<std::uint32_t>(0x20) = 0u;
  s.ptr(0x28) = nullptr;
  s.ptr(0x2c) = nullptr;
  s.ptr(0x30) = nullptr;

  void* const src_list = set.p + 0x24;
  void* node = raw_of(set.ptr(0x28)).ptr(0x00);
  for (;;) {
    if (src_list == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(src_list).ptr(0x04)) {
      break;
    }
    if (node == raw_of(src_list).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* obj = raw_of(node).ptr(0x08);
    if (raw_of(obj).at<std::int32_t>(0x1c) == 0) {
      void* const dst_head = s.ptr(0x1c);
      void* const prev = raw_of(dst_head).ptr(0x04);
      void* const new_node =
          host.stl_list_buy_node4_008db6a0(dst_list, dst_head, prev, &obj);
      host.stl_list_checked_size_add_008de2a0(dst_list, 1u);
      raw_of(dst_head).ptr(0x04) = new_node;
      raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;
    }
    if (node == raw_of(src_list).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }
  return self;
}

// 008e1720: BSP_ObjectiveSet_SendCreateSnapshotToPeer
void objective_set_send_create_snapshot_to_peer_008e1720(
    Seg57Host& host, void* self, const void* peer_owner) {
  std::uint8_t msg55[0x34]{};
  void* const built = session_message55_construct_obj_create_snapshot_008e1380(
      host, msg55, self);
  void* const peer = raw_of(peer_owner).ptr(0x50);
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  host.session_send_message_to_nonlocal_peer_00770b50(
      raw_of(game).p + 0x1ef0, peer, built);
  host.session_message55_destroy_008e1480(msg55);
}

// 008e1b90: BSP_ObjectiveSet_AnnounceAddedToLocalParty
void objective_set_announce_added_to_local_party_008e1b90(
    Seg57Host& host, void* self, void* objective) {
  const Raw s = raw_of(self);
  const Raw obj = raw_of(objective);
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  const std::int32_t local_party = raw_of(game).at<std::int32_t>(0x18ec);
  if (local_party < 0 || local_party > 7 ||
      s.at<std::int32_t>(0x14) != local_party) {
    return;
  }
  const std::int32_t kind = obj.at<std::int32_t>(0x18);
  if (kind != 0 && kind != 1) {
    return;
  }

  void* const cfg = host.global_config_get_singleton_00432650();
  objective_sounds_play_throttled_008dd460(host, raw_of(cfg).p + 0x2c0, 0);

  NativeStringView empty_str{};
  host.native_string_assign_0041e870(
      &empty_str, static_cast<const char*>(host.image_address(0x00ce3a0cu)));
  void* const obj_text = obj.p + 0x0c;
  const bool not_empty =
      host.native_string_not_equal_case_insensitive_00449af0(
          obj_text, &empty_str) != 0u;
  release_temp_native_string(host, empty_str);

  if (not_empty) {
    NativeStringView empty_sub{};
    host.native_string_assign_0041e870(
        &empty_sub, static_cast<const char*>(host.image_address(0x00ce3a0cu)));
    NativeStringView prefix_str{};
    NativeStringView concat_str{};
    std::uint32_t cleanup_flags = 0u;
    const void* display_text = obj_text;
    if (obj.at<std::int32_t>(0x18) == 1) {
      void* const lhs = host.native_string_assign_0041e870(
          &prefix_str,
          static_cast<const char*>(host.image_address(0x00d16138u)));
      display_text =
          host.native_string_concat_004261a0(lhs, &concat_str, obj_text);
      cleanup_flags = 3u;
    }
    void* const game_now = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
    void* const hud_mgr = raw_of(game_now).ptr(0x21e8);
    host.hud_announce_objective_text_00734870(
        hud_mgr, display_text, &empty_sub, 0);
    if ((cleanup_flags & 2u) != 0u) {
      host.native_string_destroy_storage_0041dd20(&concat_str);
    }
    if ((cleanup_flags & 1u) != 0u) {
      host.native_string_destroy_storage_0041dd20(&prefix_str);
    }
    release_temp_native_string(host, empty_sub);
  }
}

// 008e1d30: BSP_ObjectiveSet_AnnounceStateChange
void objective_set_announce_state_change_008e1d30(
    Seg57Host& host,
    void* self,
    const void* objective,
    const void* text_str,
    std::uint8_t completed_flag,
    std::uint8_t silent_flag) {
  const Raw s = raw_of(self);
  const Raw obj = raw_of(objective);
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  const std::int32_t local_party = raw_of(game).at<std::int32_t>(0x18ec);
  if (local_party < 0 || local_party > 7 ||
      s.at<std::int32_t>(0x14) != local_party) {
    return;
  }
  const std::int32_t kind = obj.at<std::int32_t>(0x18);
  if (kind != 0 && kind != 1) {
    return;
  }

  if (silent_flag == 0u) {
    const std::int32_t sound_idx = (completed_flag != 0u) ? 1 : 2;
    void* const cfg = host.global_config_get_singleton_00432650();
    objective_sounds_play_throttled_008dd460(
        host, raw_of(cfg).p + 0x2c0, sound_idx);
  }

  NativeStringView empty_str{};
  host.native_string_assign_0041e870(
      &empty_str, static_cast<const char*>(host.image_address(0x00ce3a0cu)));
  const bool not_empty =
      host.native_string_not_equal_case_insensitive_00449af0(
          text_str, &empty_str) != 0u;
  release_temp_native_string(host, empty_str);

  if (not_empty) {
    NativeStringView empty_sub{};
    host.native_string_assign_0041e870(
        &empty_sub, static_cast<const char*>(host.image_address(0x00ce3a0cu)));
    void* const game_now = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
    void* const hud_mgr = raw_of(game_now).ptr(0x21e8);
    host.hud_announce_objective_text_00734870(
        hud_mgr, text_str, &empty_sub, 0);
    release_temp_native_string(host, empty_sub);
  }
}

// 008e1e60: BSP_ObjectiveSync_ApplyObjCreate
void objective_sync_apply_obj_create_008e1e60(
    Seg57Host& host, void* self, void* msg) {
  const Raw s = raw_of(self);
  const Raw m = raw_of(msg);
  void* const dst_list = s.p + 0x24;
  void* const src_list = m.p + 0x18;
  if (dst_list != src_list) {
    void* const src_head = raw_of(src_list).ptr(0x04);
    host.stl_list_assign_range_008df8a0(
        dst_list,
        src_list,
        raw_of(src_head).ptr(0x00),
        src_list,
        src_head,
        msg);
  }

  void* node = raw_of(raw_of(dst_list).ptr(0x04)).ptr(0x00);
  for (;;) {
    if (dst_list == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(dst_list).ptr(0x04)) {
      break;
    }
    if (node == raw_of(dst_list).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    objective_set_announce_added_to_local_party_008e1b90(
        host, self, raw_of(node).ptr(0x08));
    if (node == raw_of(dst_list).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }

  std::uint32_t idx = 0u;
  std::size_t byte_off = 0u;
  for (;;) {
    std::uint8_t* const begin = m.at<std::uint8_t*>(0x28);
    if (begin == nullptr) {
      break;
    }
    const std::uint32_t count = static_cast<std::uint32_t>(
        (m.at<std::uint8_t*>(0x2c) - begin) / 0x1c);
    if (idx >= count) {
      break;
    }
    std::uint8_t* const begin_check = m.at<std::uint8_t*>(0x28);
    if (begin_check == nullptr ||
        idx >= static_cast<std::uint32_t>(
                   (m.at<std::uint8_t*>(0x2c) - begin_check) / 0x1c)) {
      host.invalid_parameter_00bf6713();
    }
    const Raw rec{m.at<std::uint8_t*>(0x28) + byte_off};
    if (rec.at<std::uint8_t>(0x08) != 0u) {
      objective_set_add_unit_expanding_groups_008df9b0(
          host, self, rec.p, rec.ptr(0x0c));
    } else {
      host.objective_set_add_position_to_named_objective_008df5d0(
          self,
          rec.p,
          rec.at<float>(0x10),
          rec.at<float>(0x14),
          rec.at<float>(0x18));
    }
    ++idx;
    byte_off += 0x1cu;
  }
}

// 008e1f80: BSP_ObjectiveSet_Add
void objective_set_add_008e1f80(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* text_str,
    std::int32_t kind,
    std::uint8_t quiet_flag) {
  const Raw s = raw_of(self);
  void* const raw_obj = host.operator_new_00bf681b(0x2cu);
  void* obj = nullptr;
  if (raw_obj != nullptr) {
    obj = objective_construct_008dd5c0(
        host, raw_obj, id_str, text_str, kind, quiet_flag);
  }

  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    const std::int32_t slot = s.at<std::int32_t>(0x14);
    void* const player = raw_of(game).at<void*>(
        0x18ccu + static_cast<std::size_t>(slot) * 4u);
    if (raw_of(player).ptr(0x50) != nullptr) {
      std::uint8_t msg56[0x30]{};
      void* const built =
          session_message56_construct_obj_add_008dd6a0(host, msg56, obj);
      void* const game_now = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      const std::int32_t slot_now = s.at<std::int32_t>(0x14);
      void* const player_now = raw_of(game_now).at<void*>(
          0x18ccu + static_cast<std::size_t>(slot_now) * 4u);
      void* const peer = raw_of(player_now).ptr(0x50);
      host.session_send_message_to_nonlocal_peer_00770b50(
          raw_of(game_now).p + 0x1ef0, peer, built);
      host.session_message56_destroy_008dc7a0(msg56);
    }
  }

  void* const list_self = s.p + 0x24;
  void* const head = s.ptr(0x28);
  void* const prev = raw_of(head).ptr(0x04);
  void* const new_node =
      host.stl_list_buy_node4_008db6a0(list_self, head, prev, &obj);
  host.stl_list_checked_size_add_008de2a0(list_self, 1u);
  raw_of(head).ptr(0x04) = new_node;
  raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;

  if (quiet_flag == 0u) {
    objective_set_announce_added_to_local_party_008e1b90(host, self, obj);
  }
  void* const hud_root = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e198c4u)));
  raw_of(raw_of(hud_root).ptr(0x60)).at<std::uint8_t>(0x65) = 0u;
}

// 008e2200: BSP_ObjectiveSet_SetFailed
void objective_set_set_failed_008e2200(
    Seg57Host& host,
    void* self,
    const void* id_str,
    const void* comment_str,
    std::uint8_t silent_flag) {
  const Raw s = raw_of(self);
  void* const list_container = s.p + 0x24;
  void* node = raw_of(s.ptr(0x28)).ptr(0x00);
  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      return;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    void* const obj = raw_of(node).ptr(0x08);
    if (native_string_equals_case_insensitive(
            host, raw_of(obj).p + 0x04, id_str)) {
      objective_set_announce_state_change_008e1d30(
          host, self, obj, comment_str, 0u, silent_flag);
      host.objective_clear_unit_markers_and_empty_008dfe50(self, obj);
      raw_of(obj).at<std::int32_t>(0x1c) = 2;

      void* const game = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
      if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
        const std::int32_t slot = s.at<std::int32_t>(0x14);
        void* const player = raw_of(game).at<void*>(
            0x18ccu + static_cast<std::size_t>(slot) * 4u);
        if (raw_of(player).ptr(0x50) != nullptr) {
          std::uint8_t msg62[0x2c]{};
          void* const built = session_message62_construct_obj_failed_008ddb00(
              host, msg62, obj, comment_str, silent_flag);
          void* const game_now = reinterpret_cast<void*>(
              static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
          const std::int32_t slot_now = s.at<std::int32_t>(0x14);
          void* const player_now = raw_of(game_now).at<void*>(
              0x18ccu + static_cast<std::size_t>(slot_now) * 4u);
          void* const peer = raw_of(player_now).ptr(0x50);
          host.session_send_message_to_nonlocal_peer_00770b50(
              raw_of(game_now).p + 0x1ef0, peer, built);
          host.session_message62_destroy_008dd120(msg62);
        }
      }
      return;
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
  }
}

// 008e2350: BSP_ObjectiveSync_ApplyObjCompleted
void objective_sync_apply_obj_completed_008e2350(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  host.objective_set_set_completed_008e20d0(
      self, m.p + 0x18, m.p + 0x20, m.at<std::uint8_t>(0x28));
}

// 008e2370: BSP_ObjectiveSync_ApplyObjFailed
void objective_sync_apply_obj_failed_008e2370(
    Seg57Host& host, void* self, const void* msg) {
  const Raw m = raw_of(msg);
  objective_set_set_failed_008e2200(
      host, self, m.p + 0x18, m.p + 0x20, m.at<std::uint8_t>(0x28));
}

// 008e2390: BSP_ObjectiveSync_DispatchReceived
void objective_sync_dispatch_received_008e2390(Seg57Host& host, void* msg) {
  const Raw m = raw_of(msg);
  auto local_objective_set = [&]() -> void* {
    void* const game = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
    const std::int32_t party = raw_of(game).at<std::int32_t>(0x18ec);
    return raw_of(game).at<void*>(
        0x21a4u + static_cast<std::size_t>(party) * 4u);
  };

  if (host.vcall_session_message_is_type_0c(msg, 0x37) != 0u) {
    objective_sync_apply_obj_create_008e1e60(host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x38) != 0u) {
    objective_set_add_008e1f80(
        host,
        local_objective_set(),
        m.p + 0x18,
        m.p + 0x20,
        m.at<std::int32_t>(0x28),
        m.at<std::uint8_t>(0x2c));
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x39) != 0u) {
    objective_sync_apply_obj_add_entity_008e0060(
        host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x3a) != 0u) {
    objective_sync_apply_obj_remove_entity_008e00b0(
        host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x3b) != 0u) {
    objective_sync_apply_obj_add_position_008df800(
        host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x3c) != 0u) {
    objective_sync_apply_obj_remove_position_008ddf40(
        host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x3d) != 0u) {
    objective_sync_apply_obj_completed_008e2350(
        host, local_objective_set(), msg);
    return;
  }
  if (host.vcall_session_message_is_type_0c(msg, 0x3e) != 0u) {
    objective_sync_apply_obj_failed_008e2370(host, local_objective_set(), msg);
  }
}

// 008e24f0: BSP_Thread_SetDebugName
void thread_set_debug_name_008e24f0(
    Seg57Host& host, std::uint32_t thread_id, const char* name) {
  const std::uintptr_t info[4]{
      0x1000u,
      reinterpret_cast<std::uintptr_t>(name),
      static_cast<std::uintptr_t>(thread_id),
      0u};
  host.raise_exception_00ce2280(0x406d1388u, 0u, 4u, info);
}

// 008e2560: BSP_FlagCloth_ProcessFrame
void flag_cloth_process_frame_008e2560(Seg57Host& host, float dt) {
  host.flag_model_update_wave_0070aa70(
      reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e19b08u))),
      dt);
  host.flag_model_update_wave_0070aa70(
      reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e19b04u))),
      dt);
  host.flag_model_update_wave_0070aa70(
      reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e19b0cu))),
      dt);
  host.flag_cloth_lock_vertex_buffer_0070a710(reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e19b08u))));
  host.flag_cloth_lock_vertex_buffer_0070a710(reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e19b04u))));
  host.flag_cloth_lock_vertex_buffer_0070a710(reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e19b0cu))));
}

// 008e25f0: BSP_MapPing_ResetCooldown
void map_ping_reset_cooldown_008e25f0(Seg57Host& host) {
  host.global_f32(0x00f88c28u) = 0.0f;
}

// 008e2b90: BSP_MapPingList_Construct
void* map_ping_list_construct_008e2b90(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00d16168u);
  s.ptr(0x08) = host.stl_list_buy_head_node_008e28f0();
  s.at<std::uint32_t>(0x0c) = 0u;
  return self;
}

// 008e2bf0: BSP_MapPingList_PruneExpired
void map_ping_list_prune_expired_008e2bf0(Seg57Host& host, void* self) {
  const Raw s = raw_of(self);
  void* const list_container = s.p + 0x04;
  void* node = raw_of(s.ptr(0x08)).ptr(0x00);
  for (;;) {
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (node == raw_of(list_container).ptr(0x04)) {
      break;
    }
    void* const cur = node;
    if (node == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    node = raw_of(node).ptr(0x00);
    if (list_container == nullptr) {
      host.invalid_parameter_00bf6713();
    }
    if (cur == raw_of(list_container).ptr(0x04)) {
      host.invalid_parameter_00bf6713();
    }
    if (raw_of(cur).at<float>(0x18) <= host.global_f32(0x00f876a4u)) {
      if (cur == raw_of(list_container).ptr(0x04)) {
        host.invalid_parameter_00bf6713();
        if (cur == raw_of(list_container).ptr(0x04)) {
          continue;
        }
      }
      void* const prev = raw_of(cur).ptr(0x04);
      void* const next = raw_of(cur).ptr(0x00);
      raw_of(prev).ptr(0x00) = next;
      raw_of(next).ptr(0x04) = prev;
      host.free_00bf65ac(cur);
      raw_of(list_container).at<std::uint32_t>(0x08) -= 1u;
    }
  }
}

// 008e2da0: BSP_MapPing_HandleReceived
void map_ping_handle_received_008e2da0(Seg57Host& host, void* msg) {
  const Raw m = raw_of(msg);
  void* game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    host.session_broadcast_except_thunk_0076a9f0(
        raw_of(game).p + 0x1ef0, msg, nullptr);
    game = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  }

  void* const ping_list = raw_of(game).ptr(0x21c8);
  if (ping_list == nullptr) {
    return;
  }
  const std::int32_t local_party = raw_of(game).at<std::int32_t>(0x18ec);
  if (local_party < 0 || local_party > 7) {
    return;
  }
  const std::int32_t sender_party = m.at<std::int32_t>(0x18);
  void* const local_player = raw_of(game).at<void*>(
      0x18ccu + static_cast<std::size_t>(local_party) * 4u);
  void* const sender_player = raw_of(game).at<void*>(
      0x18ccu + static_cast<std::size_t>(sender_party) * 4u);
  if (raw_of(local_player).at<std::int32_t>(0x28) !=
      raw_of(sender_player).at<std::int32_t>(0x28)) {
    return;
  }

  struct PingNodePayload {
    std::int32_t party;
    std::uint32_t field_04;
    float x;
    float y;
    float expiry;
  };
  PingNodePayload payload{
      sender_party,
      m.at<std::uint32_t>(0x1c),
      m.at<float>(0x20),
      m.at<float>(0x24),
      static_cast<float>(
          static_cast<double>(host.global_f32(0x00f876a4u)) +
          host.global_f64(0x00ce3db0u))};
  host.stl_list_push_back_008e2d60(raw_of(ping_list).p + 0x04, &payload);
  void* const sound_q = host.sound_request_queue_get_singleton_004c1b90();
  raw_of(sound_q).at<std::uint8_t>(0x1a) = 1u;
}

// 008e2e50: BSP_MapPing_AddLocal
void map_ping_add_local_008e2e50(Seg57Host& host, float x, float y) {
  void* const game = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e188a8u)));
  const std::int32_t local_party = raw_of(game).at<std::int32_t>(0x18ec);
  const float expiry = static_cast<float>(
      static_cast<double>(host.global_f32(0x00f876a4u)) +
      host.global_f64(0x00ce3db0u));
  void* const ping_list = raw_of(game).ptr(0x21c8);
  void* const list_self = raw_of(ping_list).p + 0x04;
  void* const head = raw_of(ping_list).ptr(0x08);
  void* const prev = raw_of(head).ptr(0x04);

  struct PingNodePayload {
    std::int32_t party;
    std::uint32_t field_04;
    float x;
    float y;
    float expiry;
  };
  PingNodePayload payload{local_party, 0u, x, y, expiry};
  void* const new_node =
      host.stl_list_buy_node_008e2b40(list_self, head, prev, &payload);
  host.stl_xlen_throw_008e2c80(list_self, 1u);
  raw_of(head).ptr(0x04) = new_node;
  raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;
  void* const sound_q = host.sound_request_queue_get_singleton_004c1b90();
  raw_of(sound_q).at<std::uint8_t>(0x1a) = 1u;
}

// 008e34f0: BSP_PowerupClass_ResolveAirSupportVehicleClasses
void powerup_class_resolve_air_support_vehicle_classes_008e34f0(
    Seg57Host& host, void* powerup_class) {
  const Raw s = raw_of(powerup_class);
  if (s.at<std::int32_t>(0x08) != 1) {
    return;
  }

  if (s.ptr(0x2c) == nullptr && s.at<std::int32_t>(0x24) != 0) {
    host.vehicle_class_mark_party_required_0095ba60(
        s.at<std::int32_t>(0x24), 0);
    void* const scene_mgr = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e18680u)));
    host.scene_entity_register_hidden_stock_unit_0046be90(
        scene_mgr, s.at<std::int32_t>(0x24));
    s.ptr(0x2c) = host.vehicle_class_get_or_create_00964790(
        s.at<std::int32_t>(0x24), 1u);
  }

  if (s.ptr(0x64) == nullptr && s.at<std::int32_t>(0x5c) != 0) {
    host.vehicle_class_mark_party_required_0095ba60(
        s.at<std::int32_t>(0x5c), 1);
    void* const scene_mgr = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e18680u)));
    host.scene_entity_register_hidden_stock_unit_0046be90(
        scene_mgr, s.at<std::int32_t>(0x5c));
    s.ptr(0x64) = host.vehicle_class_get_or_create_00964790(
        s.at<std::int32_t>(0x5c), 1u);
  }
}

// 008e3570: BSP_PowerupModifier_MatchesMessage68
bool powerup_modifier_matches_message68_008e3570(
    const void* modifier, const void* msg68) {
  const Raw m = raw_of(modifier);
  const Raw msg = raw_of(msg68);
  return m.at<std::int32_t>(0x04) == msg.at<std::int32_t>(0x18) &&
         m.at<float>(0x14) == msg.at<float>(0x20) &&
         m.at<std::int16_t>(0x0c) == msg.at<std::int16_t>(0x24) &&
         m.at<std::int16_t>(0x0e) == msg.at<std::int16_t>(0x26) &&
         m.at<std::int32_t>(0x10) == msg.at<std::int32_t>(0x28) &&
         m.at<std::int32_t>(0x08) == msg.at<std::int32_t>(0x30) &&
         m.at<float>(0x18) == msg.at<float>(0x34);
}

// 008e35d0: BSP_CommandBuildingPowerupBinding_Construct
void* command_building_powerup_binding_construct_008e35d0(
    Seg57Host& host, void* self, void* owner) {
  const Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00d16194u);
  s.ptr(0x04) = owner;
  s.ptr(0x08) = nullptr;
  return self;
}

// 008e35f0: BSP_PowerupClass_ScopeAcceptsUnit
bool powerup_class_scope_accepts_unit_008e35f0(
    Seg57Host& host, void* unit, const void* powerup_class) {
  const Raw cls = raw_of(powerup_class);
  const std::int32_t scope = cls.at<std::int32_t>(0x84);
  if (scope != 1 && scope != 2) {
    return true;
  }
  if (unit == nullptr) {
    return false;
  }
  const std::uint32_t mask = cls.at<std::uint32_t>(0x88);
  if (mask == 0x0fu) {
    return true;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 8) != 0u) {
    return ((mask >> 1u) & 1u) != 0u;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 6) != 0u) {
    return (mask & 1u) != 0u;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 0x18) != 0u) {
    return ((mask >> 2u) & 1u) != 0u;
  }
  if (host.entity_is_static_installation_009fe0b0(unit) != 0u) {
    return ((mask >> 3u) & 1u) != 0u;
  }
  return false;
}

// 008e3680: BSP_PowerupClass_UnitKindMaskAccepts
bool powerup_class_unit_kind_mask_accepts_008e3680(
    Seg57Host& host, void* unit, const void* powerup_class) {
  if (unit == nullptr) {
    return false;
  }
  const Raw cls = raw_of(powerup_class);
  const std::uint32_t mask = cls.at<std::uint32_t>(0x88);
  if (mask == 0x0fu) {
    return true;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 8) != 0u) {
    return ((mask >> 1u) & 1u) != 0u;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 6) != 0u) {
    return (mask & 1u) != 0u;
  }
  if (host.vcall_entity_is_kind_of_5c(unit, 0x18) != 0u ||
      host.vcall_entity_is_kind_of_5c(unit, 0x0f) != 0u) {
    return ((mask >> 2u) & 1u) != 0u;
  }
  if (host.entity_is_static_installation_009fe0b0(unit) != 0u) {
    return ((mask >> 3u) & 1u) != 0u;
  }
  return false;
}

// 008e3710: BSP_PowerupSlotRecord_Reset
void powerup_slot_record_reset_008e3710(void* self) {
  const Raw s = raw_of(self);
  s.at<std::uint32_t>(0x08) = 0u;
  s.at<std::uint8_t>(0x14) = 0u;
  s.at<float>(0x0c) = 0.0f;
  s.at<float>(0x10) = 0.0f;
  s.at<std::uint32_t>(0x04) = 0u;
  s.at<std::uint8_t>(0x00) = 0u;
}

}  // namespace bsp::objectives_seg57_argon
