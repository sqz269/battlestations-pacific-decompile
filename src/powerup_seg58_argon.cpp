#include "bsp/powerup_seg58_argon.hpp"

#include <cstring>

namespace bsp::powerup_seg58_argon {

namespace {

inline void* resolve_unit_by_id(Seg58Host& host, std::uint16_t raw_id) {
  const std::uint32_t unit_id = static_cast<std::uint32_t>(raw_id);
  if (static_cast<std::int32_t>(unit_id) <
      static_cast<std::int32_t>(host.global_u32(0x00f89a10))) {
    const std::uint32_t idx = unit_id - host.global_u32(0x00f89a0c);
    auto* const table = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00f89a54)));
    return Raw{table}.ptr(static_cast<std::size_t>(idx) * 0x10u + 0x0cu);
  }
  const std::uint32_t idx = unit_id - host.global_u32(0x00f89a60);
  auto* const table = reinterpret_cast<std::uint8_t*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00f89aa8)));
  return Raw{table}.ptr(static_cast<std::size_t>(idx) * 0x10u + 0x0cu);
}

}  // namespace

// [0x008e45d0] BSP_PowerupClass_ResolveLocalUseTarget
void* powerup_class_resolve_local_use_target_008e45d0(
    Seg58Host& host, const void* powerup_class) {
  const Raw cls{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(powerup_class))};
  const std::int32_t scope = cls.at<std::int32_t>(0x84);
  if (scope == 1 || scope == 2) {
    auto* const hud_root = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00e198c4)));
    auto* const sub_b4 = Raw{hud_root}.at<std::uint8_t*>(0xb4);
    void* const selected_holder = Raw{sub_b4}.ptr(0x08);
    void* selected_unit = nullptr;
    bool is_local_party = false;
    void* friend_target = nullptr;
    if (selected_holder != nullptr) {
      selected_unit = host.vcall_0140_get_unit(selected_holder);
      if (selected_unit != nullptr) {
        auto* const session = reinterpret_cast<std::uint8_t*>(
            static_cast<std::uintptr_t>(host.global_u32(0x00e188a8)));
        const std::int32_t local_slot = Raw{session}.at<std::int32_t>(0x18ec);
        auto* const player_rec = Raw{session}.at<std::uint8_t*>(
            0x18ccu + static_cast<std::size_t>(local_slot) * 4u);
        if (Raw{static_cast<std::uint8_t*>(selected_unit)}.at<std::int32_t>(0x54) ==
            Raw{player_rec}.at<std::int32_t>(0x28)) {
          is_local_party = true;
          friend_target = selected_unit;
        } else {
          friend_target = reinterpret_cast<void*>(
              static_cast<std::uintptr_t>(host.global_u32(0x00e188d8)));
        }
      } else {
        friend_target = reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(host.global_u32(0x00e188d8)));
      }
    } else {
      friend_target = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00e188d8)));
    }
    const std::int32_t current_scope = cls.at<std::int32_t>(0x84);
    if (current_scope == 1) {
      return friend_target;
    }
    if (current_scope == 2) {
      return is_local_party ? nullptr : selected_unit;
    }
  }
  return nullptr;
}

// [0x008e4660] BSP_PowerUpManager_RefreshHudIfShown
void power_up_manager_refresh_hud_if_shown_008e4660(Seg58Host& host) {
  const std::uint32_t hud_addr = host.global_u32(0x00e198c4);
  if (hud_addr != 0u) {
    auto* const hud_root =
        reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(hud_addr));
    void* const screen = Raw{hud_root}.ptr(0xc0);
    if (Raw{static_cast<std::uint8_t*>(screen)}.at<std::uint8_t>(0x05) != 0u) {
      host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
    }
  }
}

// [0x008e4680] BSP_PowerupTarget_AppliesToUnit
bool powerup_target_applies_to_unit_008e4680(
    Seg58Host& host, const void* target_record, void* unit) {
  const Raw rec{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(target_record))};
  const Raw u{static_cast<std::uint8_t*>(unit)};

  std::uint8_t matched = 0u;
  if (rec.at<std::uint16_t>(0x0c) != 0u && unit != nullptr &&
      u.at<std::uint8_t>(0x5c) != 0u && u.at<std::uint8_t>(0x5d) == 0u &&
      u.at<std::uint8_t>(0x60) == 0u && u.at<std::uint8_t>(0x5e) == 0u) {
    void* const resolved = host.vcall_0140_get_unit(unit);
    if (rec.at<std::uint16_t>(0x0c) ==
        Raw{static_cast<std::uint8_t*>(resolved)}.at<std::uint16_t>(0x174)) {
      matched = 1u;
    }
  }

  const std::int32_t party = rec.at<std::int32_t>(0x08);
  if (party < 2 && party == u.at<std::int32_t>(0x54)) {
    matched |= 1u;
  }

  if (rec.at<std::int32_t>(0x10) != -1) {
    const std::int32_t slot = host.unit_loss_counting_slot_009ffd20(unit);
    if (rec.at<std::int32_t>(0x10) == slot) {
      matched |= 1u;
    }
  }

  if (matched == 0u) {
    const std::uint16_t centre_id = rec.at<std::uint16_t>(0x0e);
    if (centre_id != 0u) {
      void* const centre_unit = resolve_unit_by_id(host, centre_id);
      const Raw cu{static_cast<std::uint8_t*>(centre_unit)};
      if (centre_unit != nullptr && cu.at<std::uint8_t>(0x5c) != 0u &&
          cu.at<std::uint8_t>(0x5d) == 0u && cu.at<std::uint8_t>(0x60) == 0u &&
          cu.at<std::uint8_t>(0x5e) == 0u &&
          cu.at<std::int32_t>(0x54) == u.at<std::int32_t>(0x54)) {
        void* const sub_64 = host.vcall_0064_get_subobject(centre_unit);
        const double radius_sq = host.vcall_0000_f64(sub_64);
        const float* const unit_pos =
            host.entity_pose_get_world_position_refreshed_00427eb0(unit);
        const float* const centre_pos =
            host.entity_pose_get_world_position_refreshed_00427eb0(centre_unit);
        float diff[3] = {0.0f, 0.0f, 0.0f};
        const float* const diff_ptr =
            host.vector3f_subtract_to_00427c90(centre_pos, diff, unit_pos);
        const float dist_sq = host.vector3f_length_squared_00427e30(diff_ptr);
        if (static_cast<double>(dist_sq) < radius_sq) {
          return true;
        }
      }
    }
  }
  return matched != 0u;
}

// [0x008e4b40] BSP_PowerupSlotRecord_IsCooldownExpired
std::uint32_t powerup_slot_record_is_cooldown_expired_008e4b40(
    Seg58Host& host, const void* record) {
  const Raw rec{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(record))};
  if (rec.at<std::uint8_t>(0x14) == 0u &&
      host.global_f32(0x00f876a4) > rec.at<float>(0x10)) {
    return 1u;
  }
  return 0u;
}

// [0x008e4b60] BSP_PowerupSlotRecord_GetReadyTime
float powerup_slot_record_get_ready_time_008e4b60(Seg58Host& host, const void* record) {
  const Raw rec{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(record))};
  if (rec.at<std::uint8_t>(0x14) != 0u) {
    auto* const cls = rec.at<std::uint8_t*>(0x08);
    return Raw{cls}.at<float>(0x7c) + host.global_f32(0x00f876a4);
  }
  return rec.at<float>(0x0c);
}

// [0x008e4b80] BSP_PowerupSlotRecord_GetEndTime
float powerup_slot_record_get_end_time_008e4b80(Seg58Host& host, const void* record) {
  const Raw rec{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(record))};
  if (rec.at<std::uint8_t>(0x14) != 0u) {
    auto* const cls = rec.at<std::uint8_t*>(0x08);
    return Raw{cls}.at<float>(0x80) + host.global_f32(0x00f876a4);
  }
  return rec.at<float>(0x10);
}

// [0x008e4e10] TMPL_Array12_Reserve_008E4E10
void tmpl_array12_reserve_008e4e10(Seg58Host& host, void* array, std::int32_t new_cap) {
  const Raw arr{static_cast<std::uint8_t*>(array)};
  if (new_cap < 1) {
    new_cap = 1;
  }
  if (arr.at<std::int32_t>(0x08) < new_cap) {
    auto* const new_data = static_cast<std::uint8_t*>(
        host.operator_new_00bf55be(static_cast<std::uint32_t>(new_cap * 12)));
    const std::int32_t size = arr.at<std::int32_t>(0x04);
    for (std::int32_t i = 0, byte_off = 0; i < size; ++i, byte_off += 12) {
      std::uint8_t* const dst =
          (new_data != nullptr) ? (new_data + byte_off) : nullptr;
      if (dst != nullptr) {
        auto* const src = arr.at<std::uint8_t*>(0x00) + byte_off;
        Raw{dst}.at<std::uint32_t>(0x00) = Raw{src}.at<std::uint32_t>(0x00);
        Raw{dst}.at<std::uint32_t>(0x04) = Raw{src}.at<std::uint32_t>(0x04);
        Raw{dst}.at<std::uint32_t>(0x08) = Raw{src}.at<std::uint32_t>(0x08);
      }
    }
    host.free_00bf6989(arr.ptr(0x00));
    arr.ptr(0x00) = new_data;
    arr.at<std::int32_t>(0x08) = new_cap;
  }
}

// [0x008e5170] TMPL_Array12_Resize_008E5170
void tmpl_array12_resize_008e5170(Seg58Host& host, void* array, std::int32_t new_size) {
  const Raw arr{static_cast<std::uint8_t*>(array)};
  if (arr.at<std::int32_t>(0x08) < new_size) {
    tmpl_array12_reserve_008e4e10(host, array, new_size);
  }
  const std::int32_t old_size = arr.at<std::int32_t>(0x04);
  if (old_size < new_size) {
    std::int32_t byte_off = old_size * 12;
    for (std::int32_t rem = new_size - old_size; rem != 0; --rem, byte_off += 12) {
      auto* const base = arr.at<std::uint8_t*>(0x00);
      std::uint8_t* const elem = base + byte_off;
      if (elem != nullptr) {
        Raw{elem}.at<std::uint32_t>(0x00) = 0xffffffffu;
        Raw{elem}.at<std::uint32_t>(0x04) = 0u;
        Raw{elem}.at<std::uint16_t>(0x08) = 0u;
      }
    }
  }
  while (new_size < arr.at<std::int32_t>(0x04)) {
    arr.at<std::int32_t>(0x04) -= 1;
  }
  arr.at<std::int32_t>(0x04) = new_size;
}

// [0x008e5670] TMPL_Array12_PushBack_008E5670
void tmpl_array12_push_back_008e5670(
    Seg58Host& host, void* array, const void* elem) {
  const Raw arr{static_cast<std::uint8_t*>(array)};
  if (arr.at<std::int32_t>(0x04) == arr.at<std::int32_t>(0x08)) {
    std::int32_t new_cap = arr.at<std::int32_t>(0x08) * 2;
    if (new_cap < 2) {
      new_cap = 1;
    }
    tmpl_array12_reserve_008e4e10(host, array, new_cap);
  }
  auto* const base = arr.at<std::uint8_t*>(0x00);
  std::uint8_t* const dst = base + arr.at<std::int32_t>(0x04) * 12;
  if (dst != nullptr) {
    const Raw src{
        const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(elem))};
    Raw{dst}.at<std::uint32_t>(0x00) = src.at<std::uint32_t>(0x00);
    Raw{dst}.at<std::uint32_t>(0x04) = src.at<std::uint32_t>(0x04);
    Raw{dst}.at<std::uint32_t>(0x08) = src.at<std::uint32_t>(0x08);
  }
  arr.at<std::int32_t>(0x04) += 1;
}

// [0x008e5f60] TMPL_Array12_Destroy_008E5F60
void tmpl_array12_destroy_008e5f60(Seg58Host& host, void* array) {
  const Raw arr{static_cast<std::uint8_t*>(array)};
  tmpl_array12_resize_008e5170(host, array, 0);
  host.free_00bf6989(arr.ptr(0x00));
}

// [0x008e6220] CG_adjustor_thunk_008e6220
void cg_adjustor_thunk_008e6220(Seg58Host& host, void* obj) {
  host.stl_list_clear_008e58e0(static_cast<std::uint8_t*>(obj) + 0x10);
}

// [0x008e6230] BSP_PowerupItem_ReleaseModifiers
void powerup_item_release_modifiers_008e6230(Seg58Host& host, void* item) {
  if (host.global_u32(0x00f88c30) == 0u) {
    return;
  }
  const Raw it{static_cast<std::uint8_t*>(item)};
  std::uint8_t* const list_container = it.p + 0x10;
  auto* const head_init = it.at<std::uint8_t*>(0x14);
  std::uint8_t* node = Raw{head_init}.at<std::uint8_t*>(0x00);
  while (true) {
    if (list_container == nullptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    auto* const sentinel = Raw{list_container}.at<std::uint8_t*>(0x04);
    if (node == sentinel) {
      break;
    }
    if (node == Raw{list_container}.at<std::uint8_t*>(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    auto* const modifier = Raw{node}.at<std::uint8_t*>(0x08);
    if (node == Raw{list_container}.at<std::uint8_t*>(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    auto* const cls = it.at<std::uint8_t*>(0x04);
    const std::int32_t class_type = Raw{cls}.at<std::int32_t>(0x08);
    node = Raw{node}.at<std::uint8_t*>(0x00);
    if (class_type == 3) {
      void* const manager = reinterpret_cast<void*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00f88c30)));
      host.power_up_manager_expire_modifier_008e8c30(manager, modifier);
    } else {
      Raw{modifier}.at<std::uint32_t>(0x00) = 0u;
    }
  }
}

// [0x008e62a0] BSP_PowerUp_AppliesToUnit
bool power_up_applies_to_unit_008e62a0(Seg58Host& host, void* entry, void* unit) {
  const Raw ent{static_cast<std::uint8_t*>(entry)};
  const Raw u{static_cast<std::uint8_t*>(unit)};
  if (!host.powerup_class_unit_kind_mask_accepts_008e3680(unit, ent.ptr(0x04))) {
    return false;
  }
  auto* const cls = ent.at<std::uint8_t*>(0x04);
  const std::int32_t type = Raw{cls}.at<std::int32_t>(0x08);
  if (type == 3 && ent.ptr(0x08) != nullptr) {
    const std::int32_t scope = Raw{cls}.at<std::int32_t>(0x84);
    auto* const binding = ent.at<std::uint8_t*>(0x08);
    auto* const bound_unit = Raw{binding}.at<std::uint8_t*>(0x04);
    if ((scope == 4 &&
         u.at<std::int32_t>(0x180) == Raw{bound_unit}.at<std::int32_t>(0x180)) ||
        (scope == 5 &&
         u.at<std::int32_t>(0x54) == Raw{bound_unit}.at<std::int32_t>(0x54))) {
      return true;
    }
    return false;
  }
  if (type == 2) {
    for (std::size_t off = 0; off < 0xc0u; off += 0x0cu) {
      auto* const mgr = reinterpret_cast<std::uint8_t*>(
          static_cast<std::uintptr_t>(host.global_u32(0x00f88c30)));
      std::uint8_t* const head_slot = mgr + 0x84u + off;
      auto* const head = Raw{head_slot}.at<std::uint8_t*>(0x00);
      for (std::uint8_t* node = Raw{head}.at<std::uint8_t*>(0x00);
           node != Raw{head_slot}.at<std::uint8_t*>(0x00);) {
        if (Raw{node}.ptr(0x08) == entry &&
            powerup_target_applies_to_unit_008e4680(host, node + 0x08, unit)) {
          return true;
        }
        if (node == Raw{head_slot}.at<std::uint8_t*>(0x00)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        node = Raw{node}.at<std::uint8_t*>(0x00);
      }
    }
  }
  return false;
}

// [0x008e6400] BSP_NativeString_ReleaseStorage_008e6400
void native_string_release_storage_008e6400(Seg58Host& host, void* modifier) {
  const Raw mod{static_cast<std::uint8_t*>(modifier)};
  mod.at<std::uint32_t>(0x00) = 0u;
  void* const data = mod.ptr(0x24);
  if (data != nullptr) {
    const std::uint32_t bytes = mod.at<std::uint32_t>(0x20) + 1u;
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// [0x008e64a0] BSP_NativeString_InsertAt_008E64A0
void native_string_insert_at_008e64a0(
    Seg58Host& host, void* dst_str, const void* src_str, std::uint32_t pos) {
  const Raw dst{static_cast<std::uint8_t*>(dst_str)};
  const Raw src{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(src_str))};
  const std::uint32_t src_len = src.at<std::uint32_t>(0x00);
  const std::uint32_t old_len = dst.at<std::uint32_t>(0x00);
  if (pos > old_len) {
    pos = old_len;
  }
  host.native_string_resize_0041dd40(dst_str, src_len + old_len, 1);
  auto* const dst_buf = dst.at<std::uint8_t*>(0x04);
  host.memmove_00bf87e0(
      dst_buf + src_len + pos, dst_buf + pos, old_len - pos);
  const void* src_buf = src.ptr(0x04);
  if (src_buf == nullptr) {
    src_buf = host.image_address(0x00f88c34);
  }
  host.memcpy_00bf7680(dst.at<std::uint8_t*>(0x04) + pos, src_buf, src_len);
}

// [0x008e6a80] BSP_PowerupItem_Construct
void* powerup_item_construct_008e6a80(Seg58Host& host, void* item) {
  const Raw it{static_cast<std::uint8_t*>(item)};
  it.at<std::uint32_t>(0x00) = 0u;
  it.at<std::uint32_t>(0x04) = 0u;
  it.at<std::uint32_t>(0x08) = 0u;
  it.at<std::uint32_t>(0x0c) = 0xffffffffu;
  it.ptr(0x14) = host.stl_list_buy_head_node_008e58c0();
  it.at<std::uint32_t>(0x18) = 0u;
  return item;
}

// [0x008e6ac0] BSP_PowerupModifier_Construct
void* powerup_modifier_construct_008e6ac0(Seg58Host& host, void* modifier) {
  const Raw mod{static_cast<std::uint8_t*>(modifier)};
  const float default_mult = host.global_f32(0x00d7a24c);
  mod.at<std::uint32_t>(0x00) = 0u;
  mod.at<std::uint16_t>(0x0c) = 0u;
  mod.at<std::uint16_t>(0x0e) = 0u;
  mod.at<float>(0x14) = default_mult;
  const float default_end = host.global_f32(0x00ce4970);
  mod.at<std::uint32_t>(0x04) = 0x10u;
  mod.at<std::uint32_t>(0x08) = 3u;
  mod.at<std::uint32_t>(0x10) = 0xffffffffu;
  mod.at<float>(0x18) = default_end;
  mod.at<std::uint32_t>(0x1c) = 0xffffffffu;
  host.native_string_from_int_004260b0(mod.p + 0x20, 0);
  return modifier;
}

// [0x008e6b10] BSP_PowerupModifier_InitFromMessage68
void powerup_modifier_init_from_message68_008e6b10(
    Seg58Host& host, void* modifier, const void* msg68) {
  const Raw mod{static_cast<std::uint8_t*>(modifier)};
  const Raw msg{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(msg68))};

  mod.at<std::uint32_t>(0x04) = msg.at<std::uint32_t>(0x18);
  mod.at<float>(0x14) = msg.at<float>(0x20);
  mod.at<std::uint16_t>(0x0c) = msg.at<std::uint16_t>(0x24);
  mod.at<std::uint16_t>(0x0e) = msg.at<std::uint16_t>(0x26);
  mod.at<std::uint32_t>(0x10) = msg.at<std::uint32_t>(0x28);
  mod.at<std::uint32_t>(0x08) = msg.at<std::uint32_t>(0x30);
  mod.at<float>(0x18) = msg.at<float>(0x34);

  std::uint8_t* const dst_str = mod.p + 0x20;
  std::uint8_t* const src_str = msg.p + 0x3c;
  if (dst_str != src_str) {
    const std::uint32_t src_len = Raw{src_str}.at<std::uint32_t>(0x00);
    host.native_string_resize_0041dd40(dst_str, src_len, 1);
    if (Raw{src_str}.at<std::uint32_t>(0x00) != 0u) {
      host.memcpy_00bf7680(
          Raw{dst_str}.ptr(0x04),
          Raw{src_str}.ptr(0x04),
          Raw{dst_str}.at<std::uint32_t>(0x00));
    }
  }

  mod.at<std::int32_t>(0x1c) = msg.at<std::int32_t>(0x2c);
  mod.ptr(0x00) = nullptr;

  void* const unit = resolve_unit_by_id(host, msg.at<std::uint16_t>(0x1c));
  if (unit != nullptr && host.vcall_005c_is_kind_of(unit, 0x1c)) {
    const std::int32_t player_slot = msg.at<std::int32_t>(0x2c);
    auto* const mgr = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(host.global_u32(0x00f88c30)));
    std::uint8_t* const list_base =
        mgr + static_cast<std::size_t>(player_slot) * 0x0cu + 0x20u;
    auto* const head = Raw{list_base}.at<std::uint8_t*>(0x04);
    for (std::uint8_t* node = Raw{head}.at<std::uint8_t*>(0x00);
         node != Raw{list_base}.at<std::uint8_t*>(0x04);) {
      std::uint8_t* const item = node + 0x08;
      auto* const binding = Raw{node}.at<std::uint8_t*>(0x10);
      if (binding != nullptr && Raw{binding}.at<std::uint32_t>(0x04) != 0u &&
          !host.native_string_not_equal_case_insensitive_00449af0(
              dst_str, Raw{item}.ptr(0x04)) &&
          binding == static_cast<std::uint8_t*>(unit) + 0x758) {
        mod.ptr(0x00) = item;
        return;
      }
      if (node == Raw{list_base}.at<std::uint8_t*>(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      node = Raw{node}.at<std::uint8_t*>(0x00);
    }
    return;
  }

  const std::int32_t player_slot = msg.at<std::int32_t>(0x2c);
  auto* const mgr = reinterpret_cast<std::uint8_t*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00f88c30)));
  std::uint8_t* const list_base =
      mgr + static_cast<std::size_t>(player_slot) * 0x0cu + 0x20u;
  auto* const head = Raw{list_base}.at<std::uint8_t*>(0x04);
  for (std::uint8_t* node = Raw{head}.at<std::uint8_t*>(0x00);
       node != Raw{list_base}.at<std::uint8_t*>(0x04);) {
    std::uint8_t* const item = node + 0x08;
    auto* const cls = Raw{node}.at<std::uint8_t*>(0x0c);
    if (Raw{dst_str}.at<std::uint32_t>(0x00) == 0u) {
      const std::int32_t cmp =
          (Raw{cls}.at<std::uint32_t>(0x00) != 0u) ? -1 : 0;
      if (cmp == 0) {
        mod.ptr(0x00) = item;
      }
    } else if (Raw{cls}.at<std::uint32_t>(0x00) != 0u) {
      const std::int32_t cmp = host.stricmp_00bf7fbf(
          Raw{dst_str}.at<const char*>(0x04), Raw{cls}.at<const char*>(0x04));
      if (cmp == 0) {
        mod.ptr(0x00) = item;
      }
    }
    if (node == Raw{list_base}.at<std::uint8_t*>(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    node = Raw{node}.at<std::uint8_t*>(0x00);
  }
}

// [0x008e6cb0] FUN_008e6cb0
void* fun_008e6cb0(Seg58Host& host, void* manager, const void* name_str) {
  std::uint8_t* const tree = static_cast<std::uint8_t*>(manager) + 0x14;
  void* iter[2] = {nullptr, nullptr};
  host.stl_tree_find_008e6530(tree, iter, name_str);
  void* const head = Raw{tree}.ptr(0x04);
  if (iter[0] == nullptr || iter[0] != tree) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (iter[1] != head) {
    if (iter[0] == nullptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (iter[1] == Raw{static_cast<std::uint8_t*>(iter[0])}.ptr(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    std::uint8_t* const cls = static_cast<std::uint8_t*>(iter[1]) + 0x14;
    host.powerup_class_resolve_air_support_vehicle_classes_008e34f0(cls);
    if (iter[1] == Raw{static_cast<std::uint8_t*>(iter[0])}.ptr(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    return cls;
  }
  return nullptr;
}

// [0x008e6d30] BSP_PowerUpManager_OnPowerupBindingMessage
void power_up_manager_on_powerup_binding_message_008e6d30(
    Seg58Host& host, void* manager, const void* msg) {
  const Raw m{
      const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(msg))};
  for (std::uint32_t i = 0u;; ++i) {
    auto* const ids_begin = m.at<std::uint8_t*>(0x1c);
    if (ids_begin == nullptr) {
      break;
    }
    const std::uint32_t ids_count = static_cast<std::uint32_t>(
        (m.at<std::int32_t>(0x20) -
         static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(ids_begin))) >>
        1);
    if (i >= ids_count) {
      break;
    }
    if (ids_begin == nullptr || i >= ids_count) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::uint16_t raw_id =
        Raw{m.at<std::uint8_t*>(0x1c)}.at<std::uint16_t>(
            static_cast<std::size_t>(i) * 2u);
    void* const unit = resolve_unit_by_id(host, raw_id);

    auto* const names_begin = m.at<std::uint8_t*>(0x2c);
    const std::uint32_t names_count =
        (names_begin != nullptr)
            ? static_cast<std::uint32_t>(
                  (m.at<std::int32_t>(0x30) -
                   static_cast<std::int32_t>(
                       reinterpret_cast<std::uintptr_t>(names_begin))) >>
                  3)
            : 0u;
    if (names_begin == nullptr || i >= names_count) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    void* const cls = fun_008e6cb0(
        host,
        manager,
        m.at<std::uint8_t*>(0x2c) + static_cast<std::size_t>(i) * 8u);
    if (unit != nullptr && host.vcall_005c_is_kind_of(unit, 0x1c) &&
        cls != nullptr) {
      Raw{static_cast<std::uint8_t*>(unit)}.ptr(0x760) = cls;
    }
  }
}

// [008e6e00] BSP_SessionMessage67_ConstructFromClass
// Original ABI: __thiscall(void* self, uint32_t player_slot, const void* item,
//                          uint32_t op, const void* unit_a, const void* unit_b) -> void* (RET 0x14)
void* session_message67_construct_from_class_008e6e00(
    Seg58Host& host, void* self, std::uint32_t player_slot, const void* item,
    std::uint32_t op, const void* unit_a, const void* unit_b) {
  host.session_message_construct_base_0075b430(self, 0x43u);
  raw(self).at<std::uint32_t>(0x04) = 1u;
  void* dst_name = add_bytes(self, 0x1c);
  raw(self).ptr(0x00) = host.image_address(0x00d0366cu);
  raw(dst_name).at<std::uint32_t>(0x00) = 0u;
  raw(dst_name).ptr(0x04) = nullptr;
  raw(self).at<std::uint32_t>(0x18) = player_slot;

  void* class_ptr = raw(item).ptr(0x04);
  if (dst_name != class_ptr) {
    host.native_string_resize_0041dd40(
        dst_name, raw(class_ptr).at<std::uint32_t>(0x00), 1u);
    if (raw(class_ptr).at<std::uint32_t>(0x00) != 0u) {
      host.memcpy_00bf7680(raw(dst_name).ptr(0x04), raw(class_ptr).ptr(0x04),
                           raw(dst_name).at<std::uint32_t>(0x00));
    }
  }

  raw(self).at<std::uint32_t>(0x24) = raw(item).at<std::uint32_t>(0x0c);
  raw(self).at<std::uint32_t>(0x28) = op;
  raw(self).at<std::uint16_t>(0x2c) =
      (unit_b != nullptr) ? raw(unit_b).at<std::uint16_t>(0x174) : 0u;
  raw(self).at<std::uint16_t>(0x2e) =
      (unit_a != nullptr) ? raw(unit_a).at<std::uint16_t>(0x174) : 0u;
  return self;
}

// [008e6ed0] BSP_SessionMessage67_ConstructFromName
// Original ABI: __thiscall(void* self, uint32_t player_slot, const void* name_str,
//                          uint32_t use_limit, uint32_t op, const void* unit_a, const void* unit_b) -> void* (RET 0x18)
void* session_message67_construct_from_name_008e6ed0(
    Seg58Host& host, void* self, std::uint32_t player_slot,
    const void* name_str, std::uint32_t use_limit, std::uint32_t op,
    const void* unit_a, const void* unit_b) {
  host.session_message_construct_base_0075b430(self, 0x43u);
  raw(self).at<std::uint32_t>(0x04) = 1u;
  void* dst_name = add_bytes(self, 0x1c);
  raw(self).ptr(0x00) = host.image_address(0x00d0366cu);
  raw(dst_name).at<std::uint32_t>(0x00) = 0u;
  raw(dst_name).ptr(0x04) = nullptr;
  raw(self).at<std::uint32_t>(0x18) = player_slot;

  if (dst_name != name_str) {
    host.native_string_resize_0041dd40(
        dst_name, raw(name_str).at<std::uint32_t>(0x00), 1u);
    if (raw(name_str).at<std::uint32_t>(0x00) != 0u) {
      host.memcpy_00bf7680(raw(dst_name).ptr(0x04), raw(name_str).ptr(0x04),
                           raw(dst_name).at<std::uint32_t>(0x00));
    }
  }

  raw(self).at<std::uint32_t>(0x24) = use_limit;
  raw(self).at<std::uint32_t>(0x28) = op;
  raw(self).at<std::uint16_t>(0x2c) =
      (unit_b != nullptr) ? raw(unit_b).at<std::uint16_t>(0x174) : 0u;
  raw(self).at<std::uint16_t>(0x2e) =
      (unit_a != nullptr) ? raw(unit_a).at<std::uint16_t>(0x174) : 0u;
  return self;
}

// [008e6fb0] BSP_SessionMessage68_ConstructFromModifier
// Original ABI: __thiscall(void* self, const void* modifier, uint32_t op) -> void* (RET 0x8)
void* session_message68_construct_from_modifier_008e6fb0(Seg58Host& host,
                                                         void* self,
                                                         const void* modifier,
                                                         std::uint32_t op) {
  host.session_message_construct_base_0075b430(self, 0x44u);
  raw(self).at<std::uint32_t>(0x04) = 1u;
  void* dst_name = add_bytes(self, 0x3c);
  raw(self).ptr(0x00) = host.image_address(0x00d03680u);
  raw(dst_name).at<std::uint32_t>(0x00) = 0u;
  raw(dst_name).ptr(0x04) = nullptr;

  raw(self).at<std::uint32_t>(0x18) = raw(modifier).at<std::uint32_t>(0x04);
  raw(self).at<float>(0x20) = raw(modifier).at<float>(0x14);
  raw(self).at<std::uint16_t>(0x24) = raw(modifier).at<std::uint16_t>(0x0c);
  raw(self).at<std::uint16_t>(0x26) = raw(modifier).at<std::uint16_t>(0x0e);
  raw(self).at<std::uint32_t>(0x28) = raw(modifier).at<std::uint32_t>(0x10);
  raw(self).at<std::uint32_t>(0x30) = raw(modifier).at<std::uint32_t>(0x08);
  raw(self).at<float>(0x34) = raw(modifier).at<float>(0x18);
  raw(self).at<std::uint32_t>(0x2c) = raw(modifier).at<std::uint32_t>(0x1c);

  void* src_name = add_bytes(modifier, 0x20);
  if (dst_name != src_name) {
    host.native_string_resize_0041dd40(
        dst_name, raw(src_name).at<std::uint32_t>(0x00), 1u);
    if (raw(src_name).at<std::uint32_t>(0x00) != 0u) {
      host.memcpy_00bf7680(raw(dst_name).ptr(0x04), raw(src_name).ptr(0x04),
                           raw(dst_name).at<std::uint32_t>(0x00));
    }
  }

  std::uint16_t bound_unit_id = 0u;
  if (op == 0u) {
    void* item = raw(modifier).ptr(0x00);
    if (item != nullptr) {
      void* binding = raw(item).ptr(0x08);
      if (binding != nullptr) {
        void* unit = raw(binding).ptr(0x04);
        if (unit != nullptr) {
          bound_unit_id = raw(unit).at<std::uint16_t>(0x174);
        }
      }
    }
  }
  raw(self).at<std::uint16_t>(0x1c) = bound_unit_id;
  raw(self).at<std::uint32_t>(0x38) = op;
  return self;
}

// [008e7240] CG_scalar_deleting_dtor_008e7240
// Original ABI: __thiscall(void* self, uint8_t flags) -> void* (RET 0x4)
void* cg_scalar_deleting_dtor_008e7240(Seg58Host& host, void* self,
                                       std::uint8_t flags) {
  host.stl_list_clear_008e58e0(add_bytes(self, 0x18));
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// [008e7270] CG_scalar_deleting_dtor_008e7270
// Original ABI: __thiscall(void* self, uint8_t flags) -> void* (RET 0x4)
void* cg_scalar_deleting_dtor_008e7270(Seg58Host& host, void* self,
                                       std::uint8_t flags) {
  raw(self).ptr(0x08) = nullptr;
  void* str_data = raw(self).ptr(0x2c);
  if (str_data != nullptr) {
    std::uint32_t bytes = raw(self).at<std::uint32_t>(0x28) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, str_data, bytes, 1);
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// [008e72b0] BSP_PowerupModifier_CopyConstruct
// Original ABI: __thiscall(void* self, const void* src) -> void* (RET 0x4)
void* powerup_modifier_copy_construct_008e72b0(Seg58Host& host, void* self,
                                               const void* src) {
  raw(self).at<std::uint32_t>(0x00) = raw(src).at<std::uint32_t>(0x00);
  raw(self).at<std::uint32_t>(0x04) = raw(src).at<std::uint32_t>(0x04);
  raw(self).at<std::uint32_t>(0x08) = raw(src).at<std::uint32_t>(0x08);
  raw(self).at<std::uint16_t>(0x0c) = raw(src).at<std::uint16_t>(0x0c);
  raw(self).at<std::uint16_t>(0x0e) = raw(src).at<std::uint16_t>(0x0e);
  raw(self).at<std::uint32_t>(0x10) = raw(src).at<std::uint32_t>(0x10);
  raw(self).at<float>(0x14) = raw(src).at<float>(0x14);
  raw(self).at<float>(0x18) = raw(src).at<float>(0x18);
  raw(self).at<std::uint32_t>(0x1c) = raw(src).at<std::uint32_t>(0x1c);

  void* dst_name = add_bytes(self, 0x20);
  void* src_name = add_bytes(src, 0x20);
  raw(dst_name).at<std::uint32_t>(0x00) = 0u;
  raw(dst_name).ptr(0x04) = nullptr;
  if (dst_name != src_name) {
    host.native_string_resize_0041dd40(
        dst_name, raw(src_name).at<std::uint32_t>(0x00), 1u);
    if (raw(src_name).at<std::uint32_t>(0x00) != 0u) {
      host.memcpy_00bf7680(raw(dst_name).ptr(0x04), raw(src_name).ptr(0x04),
                           raw(dst_name).at<std::uint32_t>(0x00));
    }
  }
  return self;
}

// [008e7820] BSP_PowerupItem_LoadLua
// Original ABI: __thiscall(void* self, void* reader) -> void (RET 0x4)
void powerup_item_load_lua_008e7820(Seg58Host& host, void* self, void* reader) {
  std::uint8_t class_id_str[8]{};
  host.vcall_lua_reader_read_string_slot10(
      reader, 0u, host.image_address(0x00cf86e0u), 0u, class_id_str);

  void* manager = reinterpret_cast<void*>(host.global_u32(0x00f88c30u));
  raw(self).ptr(0x04) =
      host.power_up_manager_find_class_008e6cb0(manager, class_id_str);

  host.vcall_lua_reader_read_int_slot0c(reader, 0u,
                                        host.image_address(0x00d16210u), 1u,
                                        add_bytes(self, 0x0c), 1u, 0xffffffffu);

  void* str_data = raw(class_id_str).ptr(0x04);
  if (str_data != nullptr) {
    std::uint32_t bytes = raw(class_id_str).at<std::uint32_t>(0x00) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, str_data, bytes, 1);
  }
}

// [008e7930] BSP_PowerupItem_InitFromMessage67
// Original ABI: __thiscall(void* self, const void* msg67) -> void (RET 0x4)
void powerup_item_init_from_message67_008e7930(Seg58Host& host, void* self,
                                               const void* msg67) {
  void* manager = reinterpret_cast<void*>(host.global_u32(0x00f88c30u));
  raw(self).ptr(0x04) = host.power_up_manager_find_class_008e6cb0(
      manager, add_bytes(msg67, 0x1c));
  raw(self).at<std::uint32_t>(0x0c) = raw(msg67).at<std::uint32_t>(0x24);

  std::uint32_t unit_id = raw(msg67).at<std::uint16_t>(0x2e);
  void* unit = nullptr;
  if (static_cast<std::int32_t>(unit_id) <
      static_cast<std::int32_t>(host.global_u32(0x00f89a10u))) {
    std::uint32_t idx = unit_id - host.global_u32(0x00f89a0cu);
    void* table = reinterpret_cast<void*>(host.global_u32(0x00f89a54u));
    unit = raw(table).ptr(idx * 0x10u + 0x0cu);
  } else {
    std::uint32_t idx = unit_id - host.global_u32(0x00f89a60u);
    void* table = reinterpret_cast<void*>(host.global_u32(0x00f89aa8u));
    unit = raw(table).ptr(idx * 0x10u + 0x0cu);
  }

  if (unit != nullptr && host.vcall_entity_is_kind_of_slot5c(unit, 0x1cu) != 0u) {
    raw(self).ptr(0x08) = host.vcall_entity_get_powerup_binding_slot64(unit);
    void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
    if (raw(game).at<std::uint32_t>(0x1fe4) == 2u) {
      raw(unit).ptr(0x760) = raw(self).ptr(0x04);
    }
    return;
  }
  raw(self).ptr(0x08) = nullptr;
}

// [008e7c10] CG_scalar_deleting_dtor_008e7c10
// Original ABI: __thiscall(void* self, uint8_t flags) -> void* (RET 0x4)
void* cg_scalar_deleting_dtor_008e7c10(Seg58Host& host, void* self,
                                       std::uint8_t flags) {
  host.stl_powerup_class_pair_destroy_008e70b0(add_bytes(self, 0x0c));
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// [008e8370] FUN_008e8370 (STL list erase for PowerupModifier nodes)
// Original ABI: __thiscall(void* self, void* out_iter, void* iter_owner, void* node) -> void* (RET 0xC)
void* stl_list_erase_modifier_008e8370(Seg58Host& host, void* self,
                                       void* out_iter, void* iter_owner,
                                       void* node) {
  if (iter_owner == nullptr) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (node == raw(iter_owner).ptr(0x04)) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  void* next_node = raw(node).ptr(0x00);
  if (node != raw(self).ptr(0x04)) {
    void* prev_node = raw(node).ptr(0x04);
    raw(prev_node).ptr(0x00) = next_node;
    raw(next_node).ptr(0x04) = prev_node;
    raw(node).ptr(0x08) = nullptr;
    void* str_data = raw(node).ptr(0x2c);
    if (str_data != nullptr) {
      std::uint32_t bytes = raw(node).at<std::uint32_t>(0x28) + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, str_data, bytes, 1);
    }
    host.free_00bf65ac(node);
    raw(self).at<std::uint32_t>(0x08) -= 1u;
  }
  raw(out_iter).ptr(0x04) = next_node;
  raw(out_iter).ptr(0x00) = iter_owner;
  return out_iter;
}

// [008e87c0] BSP_PowerUpManager_RemoveItem
// Original ABI: __thiscall(void* self, void* item) -> void (RET 0x4)
void power_up_manager_remove_item_008e87c0(Seg58Host& host, void* self,
                                           void* item) {
  void* list_cursor = add_bytes(self, 0x20);
  for (std::int32_t player_slot = 0; player_slot < 8;
       ++player_slot, list_cursor = add_bytes(list_cursor, 0x0c)) {
    for (void* node = raw(raw(list_cursor).ptr(0x04)).ptr(0x00);
         node != raw(list_cursor).ptr(0x04); node = raw(node).ptr(0x00)) {
      void* node_item = add_bytes(node, 0x08);
      if (node_item == item) {
        host.powerup_item_release_modifiers_008e6230(node_item);
        void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
        if (raw(game).at<std::uint32_t>(0x1fe4) == 1u) {
          void* binding = raw(node_item).ptr(0x08);
          if (binding != nullptr) {
            std::uint8_t msg[48]{};
            session_message67_construct_from_class_008e6e00(
                host, msg, static_cast<std::uint32_t>(player_slot), node_item,
                2u, raw(binding).ptr(0x04), nullptr);
            host.session_transport_broadcast_except_0076a9f0(
                add_bytes(game, 0x1ef0), msg, nullptr);
            raw(msg).ptr(0x00) = host.image_address(0x00d0366cu);
            void* str_data = raw(msg).ptr(0x20);
            if (str_data != nullptr) {
              std::uint32_t bytes = raw(msg).at<std::uint32_t>(0x1c) + 1u;
              void* pool = host.sized_storage_pool_get_singleton_00419cc0();
              host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                            bytes, 1);
            }
          } else {
            std::uint8_t msg[48]{};
            host.session_message_construct_base_0075b430(msg, 0x43u);
            raw(msg).at<std::uint32_t>(0x04) = 1u;
            raw(msg).ptr(0x00) = host.image_address(0x00d0366cu);
            raw(msg).at<std::uint32_t>(0x1c) = 0u;
            raw(msg).ptr(0x20) = nullptr;
            raw(msg).at<std::uint32_t>(0x18) =
                static_cast<std::uint32_t>(player_slot);
            void* class_ptr = raw(node_item).ptr(0x04);
            void* msg_name = add_bytes(msg, 0x1c);
            if (msg_name != class_ptr) {
              host.native_string_resize_0041dd40(
                  msg_name, raw(class_ptr).at<std::uint32_t>(0x00), 1u);
              if (raw(class_ptr).at<std::uint32_t>(0x00) != 0u) {
                host.memcpy_00bf7680(raw(msg_name).ptr(0x04),
                                     raw(class_ptr).ptr(0x04),
                                     raw(msg_name).at<std::uint32_t>(0x00));
              }
            }
            raw(msg).at<std::uint32_t>(0x24) =
                raw(node_item).at<std::uint32_t>(0x0c);
            raw(msg).at<std::uint32_t>(0x28) = 2u;
            raw(msg).at<std::uint16_t>(0x2c) = 0u;
            raw(msg).at<std::uint16_t>(0x2e) = 0u;
            host.session_transport_broadcast_except_0076a9f0(
                add_bytes(game, 0x1ef0), msg, nullptr);
            raw(msg).ptr(0x00) = host.image_address(0x00d0366cu);
            void* str_data = raw(msg).ptr(0x20);
            if (str_data != nullptr) {
              std::uint32_t bytes = raw(msg).at<std::uint32_t>(0x1c) + 1u;
              void* pool = host.sized_storage_pool_get_singleton_00419cc0();
              host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                            bytes, 1);
            }
            raw(msg).ptr(0x00) = host.image_address(0x00ce4974u);
          }
        }

        if (node == raw(list_cursor).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        if (node != raw(list_cursor).ptr(0x04)) {
          void* prev = raw(node).ptr(0x04);
          void* next = raw(node).ptr(0x00);
          raw(prev).ptr(0x00) = next;
          raw(next).ptr(0x04) = prev;

          void* sub_head = raw(node).ptr(0x1c);
          void* sub_cur = raw(sub_head).ptr(0x00);
          raw(sub_head).ptr(0x00) = sub_head;
          raw(sub_head).ptr(0x04) = sub_head;
          raw(node).at<std::uint32_t>(0x20) = 0u;
          while (sub_cur != raw(node).ptr(0x1c)) {
            void* sub_next = raw(sub_cur).ptr(0x00);
            host.free_00bf65ac(sub_cur);
            sub_cur = sub_next;
          }
          host.free_00bf65ac(raw(node).ptr(0x1c));
          raw(node).ptr(0x1c) = nullptr;
          host.free_00bf65ac(node);
          raw(list_cursor).at<std::uint32_t>(0x08) -= 1u;
        }

        void* hud_root = reinterpret_cast<void*>(host.global_u32(0x00e198c4u));
        if (hud_root != nullptr) {
          void* screen = raw(hud_root).ptr(0xc0);
          if (raw(screen).at<std::uint8_t>(0x05) != 0u) {
            host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
          }
        }
        return;
      }
      if (node == raw(list_cursor).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
    }
  }
}

// [008e8a30] BSP_PowerUpManager_RemoveItemsBoundTo
// Original ABI: __thiscall(void* self, void* binding) -> void (RET 0x4)
void power_up_manager_remove_items_bound_to_008e8a30(Seg58Host& host,
                                                     void* self,
                                                     void* binding) {
  void* list_cursor = add_bytes(self, 0x20);
  for (std::int32_t player_slot = 0; player_slot < 8;
       ++player_slot, list_cursor = add_bytes(list_cursor, 0x0c)) {
    for (void* node = raw(raw(list_cursor).ptr(0x04)).ptr(0x00);
         node != raw(list_cursor).ptr(0x04); node = raw(node).ptr(0x00)) {
      if (raw(node).ptr(0x10) == binding) {
        void* node_item = add_bytes(node, 0x08);
        host.powerup_item_release_modifiers_008e6230(node_item);
        void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
        if (raw(game).at<std::uint32_t>(0x1fe4) == 1u) {
          std::uint8_t msg[48]{};
          void* item_binding = raw(node_item).ptr(0x08);
          session_message67_construct_from_class_008e6e00(
              host, msg, static_cast<std::uint32_t>(player_slot), node_item, 2u,
              raw(item_binding).ptr(0x04), nullptr);
          host.session_transport_broadcast_except_0076a9f0(
              add_bytes(game, 0x1ef0), msg, nullptr);
          raw(msg).ptr(0x00) = host.image_address(0x00d0366cu);
          void* str_data = raw(msg).ptr(0x20);
          if (str_data != nullptr) {
            std::uint32_t bytes = raw(msg).at<std::uint32_t>(0x1c) + 1u;
            void* pool = host.sized_storage_pool_get_singleton_00419cc0();
            host.sized_storage_pool_return_block_00bd1510(pool, str_data, bytes,
                                                          1);
          }
          raw(msg).ptr(0x00) = host.image_address(0x00ce4974u);
        }

        void* class_ptr = raw(node_item).ptr(0x04);
        if (raw(class_ptr).at<std::uint32_t>(0x08) == 2u) {
          void* cat_head_ptr = add_bytes(self, 0x84);
          for (std::int32_t cat = 0; cat < 16;
               ++cat, cat_head_ptr = add_bytes(cat_head_ptr, 0x0c)) {
            void* cat_head = raw(cat_head_ptr).ptr(0x00);
            for (void* mod_node = raw(cat_head).ptr(0x00); mod_node != cat_head;
                 mod_node = raw(mod_node).ptr(0x00)) {
              if (raw(mod_node).ptr(0x08) == node_item) {
                if (mod_node == raw(cat_head_ptr).ptr(0x00)) {
                  host.invalid_parameter_noinfo_00bf6713();
                }
                raw(mod_node).ptr(0x08) = nullptr;
              }
              if (mod_node == raw(cat_head_ptr).ptr(0x00)) {
                host.invalid_parameter_noinfo_00bf6713();
              }
            }
          }
        }

        if (node == raw(list_cursor).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        if (node != raw(list_cursor).ptr(0x04)) {
          void* prev = raw(node).ptr(0x04);
          void* next = raw(node).ptr(0x00);
          raw(prev).ptr(0x00) = next;
          raw(next).ptr(0x04) = prev;

          void* sub_head = raw(node).ptr(0x1c);
          void* sub_cur = raw(sub_head).ptr(0x00);
          raw(sub_head).ptr(0x00) = sub_head;
          raw(sub_head).ptr(0x04) = sub_head;
          raw(node).at<std::uint32_t>(0x20) = 0u;
          while (sub_cur != raw(node).ptr(0x1c)) {
            void* sub_next = raw(sub_cur).ptr(0x00);
            host.free_00bf65ac(sub_cur);
            sub_cur = sub_next;
          }
          host.free_00bf65ac(raw(node).ptr(0x1c));
          raw(node).ptr(0x1c) = nullptr;
          host.free_00bf65ac(node);
          raw(list_cursor).at<std::uint32_t>(0x08) -= 1u;
        }

        void* hud_root = reinterpret_cast<void*>(host.global_u32(0x00e198c4u));
        if (hud_root != nullptr) {
          void* screen = raw(hud_root).ptr(0xc0);
          if (raw(screen).at<std::uint8_t>(0x05) != 0u) {
            host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
          }
        }
        return;
      }
      if (node == raw(list_cursor).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
    }
  }
}

// [008e8c30] BSP_PowerUpManager_ExpireModifier
// Original ABI: __thiscall(void* self, void* modifier) -> void (RET 0x4)
void power_up_manager_expire_modifier_008e8c30(Seg58Host& host, void* self,
                                               void* modifier) {
  void* item = raw(modifier).ptr(0x00);
  if (item != nullptr) {
    void* class_ptr = raw(item).ptr(0x04);
    if (raw(class_ptr).at<std::uint32_t>(0x08) == 2u) {
      raw(item).at<std::int32_t>(0x00) -= 1;
    } else {
      void* item_mod_list = add_bytes(item, 0x10);
      for (void* node = raw(raw(item).ptr(0x14)).ptr(0x00);
           node != raw(item_mod_list).ptr(0x04); node = raw(node).ptr(0x00)) {
        if (raw(node).ptr(0x08) == modifier) {
          raw(modifier).ptr(0x00) = nullptr;
          std::uint8_t iter_out[8]{};
          host.stl_list_erase_008e5610(item_mod_list, iter_out, item_mod_list,
                                       node);
          break;
        }
        if (node == raw(item_mod_list).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
      }
    }
  }

  std::uint32_t category = raw(modifier).at<std::uint32_t>(0x04);
  void* cat_list = add_bytes(self, 0x80u + category * 0x0cu);
  for (void* node = raw(raw(cat_list).ptr(0x04)).ptr(0x00);
       node != raw(cat_list).ptr(0x04); node = raw(node).ptr(0x00)) {
    if (add_bytes(node, 0x08) == modifier) {
      void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
      if (raw(game).at<std::uint32_t>(0x1fe4) == 1u) {
        std::uint8_t msg[72]{};
        session_message68_construct_from_modifier_008e6fb0(host, msg, modifier,
                                                           2u);
        host.session_transport_broadcast_except_0076a9f0(
            add_bytes(game, 0x1ef0), msg, nullptr);
        raw(msg).ptr(0x00) = host.image_address(0x00d03680u);
        void* str_data = raw(msg).ptr(0x40);
        if (str_data != nullptr) {
          std::uint32_t bytes = raw(msg).at<std::uint32_t>(0x3c) + 1u;
          void* pool = host.sized_storage_pool_get_singleton_00419cc0();
          host.sized_storage_pool_return_block_00bd1510(pool, str_data, bytes,
                                                        1);
        }
      }
      std::uint8_t iter_out[8]{};
      stl_list_erase_modifier_008e8370(host, cat_list, iter_out, cat_list,
                                       node);
      break;
    }
    if (node == raw(cat_list).ptr(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
  }

  if (item != nullptr) {
    void* class_ptr = raw(item).ptr(0x04);
    if (raw(class_ptr).at<std::uint32_t>(0x08) == 2u &&
        raw(item).at<std::int32_t>(0x00) < 1 &&
        raw(item).at<std::int32_t>(0x0c) == 0) {
      power_up_manager_remove_item_008e87c0(host, self, item);
    }
  }
}

// [008e94d0] BSP_CommandBuildingPowerupBinding_Release
// Original ABI: __fastcall(void* self) -> void (RET)
void command_building_powerup_binding_release_008e94d0(Seg58Host& host,
                                                       void* self) {
  if (raw(self).ptr(0x08) != nullptr && host.global_u32(0x00f88c30u) != 0u) {
    raw(self).ptr(0x08) = nullptr;
    void* manager = reinterpret_cast<void*>(host.global_u32(0x00f88c30u));
    power_up_manager_remove_items_bound_to_008e8a30(host, manager, self);
    host.observer_unregister_pair_if_registered_006956a0(
        raw(self).ptr(0x04),
        reinterpret_cast<void*>(host.global_u32(0x00f88c30u)));
  }
}

// [008e9510] BSP_PowerUpManager_OnObservedDestroyed
// Original ABI: __thiscall(void* self, void* event) -> void (RET 0x4)
void power_up_manager_on_observed_destroyed_008e9510(Seg58Host& host,
                                                     void* self, void* event) {
  void* subject = host.vcall_observer_event_get_subject_slot04(event);
  if (host.vcall_observer_event_get_subject_slot04(event) != nullptr) {
    void* subj_check = host.vcall_observer_event_get_subject_slot04(event);
    if (host.vcall_entity_get_powerup_binding_slot64(subj_check) != nullptr) {
      void* subj_bind = host.vcall_observer_event_get_subject_slot04(event);
      void* binding = host.vcall_entity_get_powerup_binding_slot64(subj_bind);
      power_up_manager_remove_items_bound_to_008e8a30(host, self, binding);
      return;
    }
  }

  if (host.vcall_entity_is_kind_of_slot5c(subject, 2u) != 0u) {
    std::int32_t party = raw(subject).at<std::int32_t>(0x30c);
    void* tree = add_bytes(self, 0x14c + party * 0x0c);
    void* head = raw(tree).ptr(0x04);
    void* iter[2] = {tree, raw(head).ptr(0x00)};
    for (;;) {
      if (iter[0] == nullptr || iter[0] != tree) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      if (iter[1] == head) {
        break;
      }
      if (iter[0] == nullptr) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      if (iter[1] == raw(iter[0]).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      if (raw(iter[1]).ptr(0x14) == subject) {
        if (iter[1] == raw(iter[0]).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        if (raw(iter[1]).at<std::uint8_t>(0x24) != 0u) {
          void* class_ptr = raw(iter[1]).ptr(0x18);
          raw(iter[1]).at<std::uint8_t>(0x24) = 0u;
          float now = host.global_f32(0x00f876a4u);
          raw(iter[1]).at<float>(0x20) = raw(class_ptr).at<float>(0x80) + now;
          raw(iter[1]).at<float>(0x1c) = raw(class_ptr).at<float>(0x7c) + now;
        }
      }
      host.stl_tree_iter_inc_00613760(iter);
    }
  }
}

// [008e9630] BSP_PowerUpManager_OnObservedReleased
// Original ABI: __thiscall(void* self, void* event) -> void (RET 0x4)
void power_up_manager_on_observed_released_008e9630(Seg58Host& host, void* self,
                                                    void* event) {
  if (host.vcall_observer_event_get_subject_slot04(event) != nullptr) {
    void* subj_check = host.vcall_observer_event_get_subject_slot04(event);
    if (host.vcall_entity_get_powerup_binding_slot64(subj_check) != nullptr) {
      void* subj_bind = host.vcall_observer_event_get_subject_slot04(event);
      void* binding = host.vcall_entity_get_powerup_binding_slot64(subj_bind);
      power_up_manager_remove_items_bound_to_008e8a30(host, self, binding);
    }
  }
}

// [008e9a70] BSP_CommandBuildingPowerupBinding_Destruct
// Original ABI: __fastcall(void* self) -> void (RET)
void command_building_powerup_binding_destruct_008e9a70(Seg58Host& host,
                                                        void* self) {
  raw(self).ptr(0x00) = host.image_address(0x00d16194u);
  if (raw(self).ptr(0x08) != nullptr && host.global_u32(0x00f88c30u) != 0u) {
    raw(self).ptr(0x08) = nullptr;
    void* manager = reinterpret_cast<void*>(host.global_u32(0x00f88c30u));
    power_up_manager_remove_items_bound_to_008e8a30(host, manager, self);
    host.observer_unregister_pair_if_registered_006956a0(
        raw(self).ptr(0x04),
        reinterpret_cast<void*>(host.global_u32(0x00f88c30u)));
  }
}

// [008e9af0] BSP_PowerUpManager_CollectActive
// Original ABI: __thiscall(void* self, void* out_vec_type3, void* out_vec_items,
//                          void* out_vec_counts, void* out_vec_end_times, void* out_vec_start_times,
//                          uint8_t include_all_players, uint8_t include_zero_limit) -> void (RET 0x1C)
void power_up_manager_collect_active_008e9af0(
    Seg58Host& host, void* self, void* out_vec_type3, void* out_vec_items,
    void* out_vec_counts, void* out_vec_end_times, void* out_vec_start_times,
    std::uint8_t include_all_players, std::uint8_t include_zero_limit) {
  std::uint8_t iter_tmp[8]{};

  auto check_and_clear = [&](void* vec, int kind) {
    void* last = raw(vec).ptr(0x08);
    if (raw(vec).ptr(0x04) > last) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    void* first = raw(vec).ptr(0x04);
    if (first > raw(vec).ptr(0x08)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (kind == 0) {
      host.stl_vector_erase_range_005a0be0(vec, iter_tmp, vec, first, vec,
                                           last);
    } else if (kind == 1) {
      host.stl_vector_erase_range_004685a0(vec, iter_tmp, vec, first, vec,
                                           last);
    } else {
      host.stl_vector_erase_range_00488bf0(vec, iter_tmp, vec, first, vec,
                                           last);
    }
  };

  check_and_clear(out_vec_items, 0);
  check_and_clear(out_vec_counts, 1);
  check_and_clear(out_vec_end_times, 2);
  check_and_clear(out_vec_start_times, 2);
  check_and_clear(out_vec_type3, 0);

  if (host.global_u8(0x00e0c978u) == 0u) {
    return;
  }

  void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
  std::int32_t local_slot = raw(game).at<std::int32_t>(0x18ec);
  void* local_player = raw(game).ptr(0x18cc + local_slot * 4);
  std::int32_t local_team = raw(local_player).at<std::int32_t>(0x28);

  for (std::int32_t slot = 0; slot < 8; ++slot) {
    void* slot_player = raw(game).ptr(0x18cc + slot * 4);
    if (raw(slot_player).at<std::int32_t>(0x28) == local_team ||
        include_all_players != 0u) {
      void* slot_base = add_bytes(self, slot * 0x0c);
      for (void* node = raw(raw(slot_base).ptr(0x24)).ptr(0x00);
           node != raw(slot_base).ptr(0x24);) {
        void* item = add_bytes(node, 0x08);
        void* item_class_field = add_bytes(item, 0x04);
        void* class_ptr = raw(item).ptr(0x04);

        if (raw(class_ptr).at<std::uint32_t>(0x08) == 3u) {
          void* first = raw(out_vec_type3).ptr(0x04);
          void* last = raw(out_vec_type3).ptr(0x08);
          void* cap = raw(out_vec_type3).ptr(0x0c);
          std::uint32_t size_elems =
              (first != nullptr)
                  ? static_cast<std::uint32_t>(
                        (static_cast<std::uint8_t*>(last) -
                         static_cast<std::uint8_t*>(first)) >>
                        2)
                  : 0u;
          std::uint32_t cap_elems =
              (first != nullptr)
                  ? static_cast<std::uint32_t>(
                        (static_cast<std::uint8_t*>(cap) -
                         static_cast<std::uint8_t*>(first)) >>
                        2)
                  : 0u;
          if (first != nullptr && size_elems < cap_elems) {
            raw(last).ptr(0x00) = item;
            raw(out_vec_type3).ptr(0x08) = add_bytes(last, 4);
          } else {
            if (first > last) {
              host.invalid_parameter_noinfo_00bf6713();
            }
            host.stl_vector_insert_one_005a6580(out_vec_type3, iter_tmp,
                                                out_vec_type3, last, &item);
          }
        } else if (slot == local_slot || include_all_players != 0u) {
          std::int32_t use_limit = raw(item).at<std::int32_t>(0x0c);
          if (use_limit > 0) {
            void* item_val = item;
            std::int32_t one_val = 1;
            for (std::int32_t rep = 0; rep < raw(item).at<std::int32_t>(0x0c);
                 ++rep) {
              {
                void* first = raw(out_vec_items).ptr(0x04);
                void* last = raw(out_vec_items).ptr(0x08);
                void* cap = raw(out_vec_items).ptr(0x0c);
                std::uint32_t sz =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(last) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                std::uint32_t cp =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(cap) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                if (first != nullptr && sz < cp) {
                  raw(last).ptr(0x00) = item;
                  raw(out_vec_items).ptr(0x08) = add_bytes(last, 4);
                } else {
                  if (first > last) {
                    host.invalid_parameter_noinfo_00bf6713();
                  }
                  host.stl_vector_insert_one_005a6580(
                      out_vec_items, iter_tmp, out_vec_items, last, &item_val);
                }
              }

              {
                void* first = raw(out_vec_counts).ptr(0x04);
                void* last = raw(out_vec_counts).ptr(0x08);
                void* cap = raw(out_vec_counts).ptr(0x0c);
                std::uint32_t sz =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(last) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                std::uint32_t cp =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(cap) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                if (first != nullptr && sz < cp) {
                  raw(last).at<std::int32_t>(0x00) = 1;
                  raw(out_vec_counts).ptr(0x08) = add_bytes(last, 4);
                } else {
                  if (first > last) {
                    host.invalid_parameter_noinfo_00bf6713();
                  }
                  host.stl_vector_insert_one_00441f30(
                      out_vec_counts, iter_tmp, out_vec_counts, last, &one_val);
                }
              }

              void* cd = host.stl_map_subscript_00617030(
                  add_bytes(slot_base, 0x14c), add_bytes(item, 0x04));
              float end_time =
                  (raw(cd).at<std::uint8_t>(0x14) != 0u)
                      ? (raw(raw(cd).ptr(0x08)).at<float>(0x80) +
                         host.global_f32(0x00f876a4u))
                      : raw(cd).at<float>(0x10);
              {
                void* first = raw(out_vec_end_times).ptr(0x04);
                void* last = raw(out_vec_end_times).ptr(0x08);
                void* cap = raw(out_vec_end_times).ptr(0x0c);
                std::uint32_t sz =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(last) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                std::uint32_t cp =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(cap) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                if (first != nullptr && sz < cp) {
                  raw(last).at<float>(0x00) = end_time;
                  raw(out_vec_end_times).ptr(0x08) = add_bytes(last, 4);
                } else {
                  if (first > last) {
                    host.invalid_parameter_noinfo_00bf6713();
                  }
                  host.stl_checked_float_vector_insert_one_0045a120(
                      out_vec_end_times, iter_tmp, out_vec_end_times, last,
                      &end_time);
                }
              }

              float start_time =
                  (raw(cd).at<std::uint8_t>(0x14) != 0u)
                      ? (raw(raw(cd).ptr(0x08)).at<float>(0x7c) +
                         host.global_f32(0x00f876a4u))
                      : raw(cd).at<float>(0x0c);
              {
                void* first = raw(out_vec_start_times).ptr(0x04);
                void* last = raw(out_vec_start_times).ptr(0x08);
                void* cap = raw(out_vec_start_times).ptr(0x0c);
                std::uint32_t sz =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(last) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                std::uint32_t cp =
                    (first != nullptr)
                        ? static_cast<std::uint32_t>(
                              (static_cast<std::uint8_t*>(cap) -
                               static_cast<std::uint8_t*>(first)) >>
                              2)
                        : 0u;
                if (first != nullptr && sz < cp) {
                  raw(last).at<float>(0x00) = start_time;
                  raw(out_vec_start_times).ptr(0x08) = add_bytes(last, 4);
                } else {
                  if (first > last) {
                    host.invalid_parameter_noinfo_00bf6713();
                  }
                  host.stl_checked_float_vector_insert_one_0045a120(
                      out_vec_start_times, iter_tmp, out_vec_start_times, last,
                      &start_time);
                }
              }
            }
          } else if (use_limit != 0 || include_zero_limit != 0u) {
            void* item_val = item;
            host.stl_vector_push_back_005a7960(out_vec_items, &item_val);
            std::int32_t count_val = raw(item).at<std::int32_t>(0x0c);
            host.stl_vector_push_back_00442190(out_vec_counts, &count_val);
            void* cd = host.stl_map_subscript_00617030(
                add_bytes(slot_base, 0x14c), item_class_field);
            float end_time =
                (raw(cd).at<std::uint8_t>(0x14) != 0u)
                    ? (raw(raw(cd).ptr(0x08)).at<float>(0x80) +
                       host.global_f32(0x00f876a4u))
                    : raw(cd).at<float>(0x10);
            host.stl_vector_push_back_0045afd0(out_vec_end_times, &end_time);
            float start_time =
                (raw(cd).at<std::uint8_t>(0x14) != 0u)
                    ? (raw(raw(cd).ptr(0x08)).at<float>(0x7c) +
                       host.global_f32(0x00f876a4u))
                    : raw(cd).at<float>(0x0c);
            host.stl_vector_push_back_0045afd0(out_vec_start_times,
                                               &start_time);
          }
        }

        if (node == raw(slot_base).ptr(0x24)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        node = raw(node).ptr(0x00);
      }
      game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
    }
  }
}

// [008e9fd0] BSP_PowerUpManager_PrepareFromLua
// Original ABI: __stdcall(const void* lua_table) -> void (RET 0x4)
void power_up_manager_prepare_from_lua_008e9fd0(Seg58Host& host,
                                                const void* lua_table) {
  std::uint8_t temp_item[28]{};
  raw(temp_item).at<std::uint32_t>(0x00) = 0u;
  raw(temp_item).ptr(0x04) = nullptr;
  raw(temp_item).ptr(0x08) = nullptr;
  raw(temp_item).at<std::int32_t>(0x0c) = -1;
  raw(temp_item).ptr(0x14) = host.stl_list_buy_head_node_008e58c0();
  raw(temp_item).at<std::uint32_t>(0x18) = 0u;

  std::uint8_t lua_obj_copy[20]{};
  host.lua_object_copy_construct_00b66fa0(lua_obj_copy, lua_table);

  std::uint8_t reader[20]{};
  void* reader_ptr = host.lua_reader_construct_004425c0(reader, lua_obj_copy);
  powerup_item_load_lua_008e7820(host, temp_item, reader_ptr);

  raw(reader).ptr(0x00) = host.image_address(0x00ce44fcu);
  host.lua_object_vector_tidy_00441210(add_bytes(reader, 0x04));

  void* game = reinterpret_cast<void*>(host.global_u32(0x00e188a8u));
  if (raw(game).at<std::uint32_t>(0x1fe4) == 1u) {
    std::uint8_t msg[48]{};
    void* msg_ptr = session_message67_construct_from_class_008e6e00(
        host, msg, 0u, temp_item, 3u, nullptr, nullptr);
    host.session_transport_broadcast_except_0076a9f0(add_bytes(game, 0x1ef0),
                                                     msg_ptr, nullptr);
    host.session_message67_destroy_007657d0(msg);
  }

  host.stl_list_clear_008e58e0(add_bytes(temp_item, 0x10));
}

// [008ea0c0] BSP_PowerUpManager_ListReadyItems
// Original ABI: __thiscall(void* self, int32_t slot, void* out_vec) -> void (RET 0x8)
void power_up_manager_list_ready_items_008ea0c0(Seg58Host& host, void* self,
                                                std::int32_t slot,
                                                void* out_vec) {
  void* last = raw(out_vec).ptr(0x08);
  if (raw(out_vec).ptr(0x04) > last) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  void* first = raw(out_vec).ptr(0x04);
  if (first > raw(out_vec).ptr(0x08)) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  std::uint8_t iter_tmp[8]{};
  host.stl_vector_erase_range_005a0be0(out_vec, iter_tmp, out_vec, first,
                                       out_vec, last);

  if (host.global_u8(0x00e0c978u) == 0u) {
    return;
  }

  void* slot_base = add_bytes(self, slot * 0x0c);
  for (void* node = raw(raw(slot_base).ptr(0x24)).ptr(0x00);
       node != raw(slot_base).ptr(0x24);) {
    void* item_class_ptr_addr = add_bytes(node, 0x0c);
    void* cd = host.stl_map_subscript_00617030(add_bytes(slot_base, 0x14c),
                                               item_class_ptr_addr);
    if (raw(cd).at<std::uint8_t>(0x14) == 0u &&
        host.global_f32(0x00f876a4u) > raw(cd).at<float>(0x10)) {
      std::uint32_t match_idx = 0xffffffffu;
      std::uint32_t idx = 0u;
      if (host.stl_vector_size_0059ae20(out_vec) > 0u) {
        do {
          void* vec_first = raw(out_vec).ptr(0x04);
          void* vec_last = raw(out_vec).ptr(0x08);
          if (vec_first == nullptr ||
              idx >= static_cast<std::uint32_t>(
                         (static_cast<std::uint8_t*>(vec_last) -
                          static_cast<std::uint8_t*>(vec_first)) >>
                         2)) {
            host.invalid_parameter_noinfo_00bf6713();
          }
          void* existing_item = raw(vec_first).ptr(idx * 4u);
          if (raw(existing_item).ptr(0x04) ==
              raw(item_class_ptr_addr).ptr(0x00)) {
            match_idx = idx;
          }
          ++idx;
        } while (idx < host.stl_vector_size_0059ae20(out_vec));
      }

      void* item = add_bytes(node, 0x08);
      if (match_idx == 0xffffffffu) {
        host.stl_vector_push_back_005a7960(out_vec, &item);
      } else {
        void* vec_first = raw(out_vec).ptr(0x04);
        void* vec_last = raw(out_vec).ptr(0x08);
        if (vec_first == nullptr ||
            match_idx >= static_cast<std::uint32_t>(
                             (static_cast<std::uint8_t*>(vec_last) -
                              static_cast<std::uint8_t*>(vec_first)) >>
                             2)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        raw(vec_first).ptr(match_idx * 4u) = item;
      }
    }

    if (node == raw(slot_base).ptr(0x24)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    node = raw(node).ptr(0x00);
  }
}

namespace {
inline Raw raw_of(void* ptr) noexcept {
  return Raw{static_cast<std::uint8_t*>(ptr)};
}
inline Raw raw_of(const void* ptr) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(ptr))};
}
inline void* ptr_from_u32(std::uint32_t addr) noexcept {
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(addr));
}
}  // namespace

// [BSP reconstruction 008ea250]
// Original ABI: __thiscall(void* manager /*ECX*/, int category, float multiplier,
//   void* powerup_class, void* item, int player_slot, void* target_unit), RET 0x18.
// Hypothesis, not a recovered symbol. Inserts a modifier into category list
// manager + 0x80 + category * 0xc, initializes its fields, links it to the item
// or increments the item active counter, populates scope target fields, and on
// host broadcasts SessionMessage68 (MT_POWERUP_EFFECT).
void power_up_manager_insert_modifier_008ea250(
    Seg58Host& host, void* manager, std::int32_t category, float multiplier,
    void* powerup_class, void* item, std::int32_t player_slot,
    void* target_unit) {
  std::uint8_t* const category_list =
      raw_of(manager).p + 0x80 + category * 0xc;

  std::uint8_t temp_mod[0x28]{};
  raw_of(temp_mod).at<std::uint32_t>(0x00) = 0;
  raw_of(temp_mod).at<std::uint32_t>(0x04) = 0x10;
  raw_of(temp_mod).at<std::uint32_t>(0x08) = 3;
  raw_of(temp_mod).at<std::uint16_t>(0x0c) = 0;
  raw_of(temp_mod).at<std::uint16_t>(0x0e) = 0;
  raw_of(temp_mod).at<std::uint32_t>(0x10) = 0xffffffffu;
  raw_of(temp_mod).at<float>(0x14) = host.global_f32(0x00d7a24c);
  raw_of(temp_mod).at<float>(0x18) = host.global_f32(0x00ce4970);
  raw_of(temp_mod).at<std::uint32_t>(0x1c) = 0xffffffffu;
  host.native_string_from_int_004260b0(temp_mod + 0x20, 0);

  void* const head = raw_of(category_list).ptr(0x04);
  void* const new_node = host.stl_list_buy_node_008e8480(
      head, raw_of(head).ptr(0x04), temp_mod);
  host.stl_xlen_throw_008e7f10(category_list, 1);
  raw_of(head).ptr(0x04) = new_node;
  raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;

  void* const temp_str_data = raw_of(temp_mod).ptr(0x24);
  if (temp_str_data != nullptr) {
    const std::uint32_t temp_str_len = raw_of(temp_mod).at<std::uint32_t>(0x20);
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, temp_str_data,
                                                  temp_str_len + 1, 1);
  }

  void* const head_after = raw_of(category_list).ptr(0x04);
  void* const back_node = raw_of(head_after).ptr(0x04);
  if (back_node == head_after) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (back_node == raw_of(category_list).ptr(0x04)) {
    host.invalid_parameter_noinfo_00bf6713();
  }

  std::uint8_t* const mod = raw_of(back_node).p + 0x08;
  raw_of(mod).at<std::int32_t>(0x04) = category;
  raw_of(mod).at<float>(0x14) = multiplier;
  raw_of(mod).at<std::int32_t>(0x1c) = player_slot;

  std::uint8_t* const mod_str = mod + 0x20;
  if (mod_str != raw_of(powerup_class).p) {
    host.native_string_resize_0041dd40(
        mod_str, raw_of(powerup_class).at<std::uint32_t>(0x00), 1);
    if (raw_of(powerup_class).at<std::uint32_t>(0x00) != 0) {
      host.memcpy_00bf7680(raw_of(mod_str).ptr(0x04),
                           raw_of(powerup_class).ptr(0x04),
                           raw_of(mod_str).at<std::uint32_t>(0x00));
    }
  }

  if (raw_of(powerup_class).at<std::int32_t>(0x08) == 2) {
    raw_of(mod).at<float>(0x18) =
        raw_of(powerup_class).at<float>(0x7c) + host.global_f32(0x00f876a4);
  } else {
    raw_of(mod).at<float>(0x18) =
        static_cast<float>(host.global_f64(0x00d16220));
  }

  if (item != nullptr) {
    if (raw_of(powerup_class).at<std::int32_t>(0x08) == 3) {
      std::uint8_t* const item_mod_list = raw_of(item).p + 0x10;
      void* const item_mod_head = raw_of(item).ptr(0x14);
      void* mod_ptr_val = mod;
      void* const item_node = host.stl_list_buy_node_008e5aa0(
          item_mod_head, raw_of(item_mod_head).ptr(0x04), &mod_ptr_val);
      host.stl_xlen_throw_008e7d00(item_mod_list, 1);
      raw_of(item_mod_head).ptr(0x04) = item_node;
      raw_of(raw_of(item_node).ptr(0x04)).ptr(0x00) = item_node;
    } else {
      raw_of(item).at<std::int32_t>(0x00) += 1;
    }
    raw_of(mod).ptr(0x00) = item;
  }

  switch (raw_of(powerup_class).at<std::int32_t>(0x84)) {
    case 1:
    case 2:
      raw_of(mod).at<std::uint16_t>(0x0c) =
          raw_of(target_unit).at<std::uint16_t>(0x174);
      break;
    case 3: {
      void* const bound_item = raw_of(mod).ptr(0x00);
      void* const binding = raw_of(bound_item).ptr(0x08);
      void* const building = raw_of(binding).ptr(0x04);
      raw_of(mod).at<std::uint16_t>(0x0e) =
          raw_of(building).at<std::uint16_t>(0x174);
      break;
    }
    case 4:
      raw_of(mod).at<std::int32_t>(0x10) = player_slot;
      break;
    case 5: {
      void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
      void* const slot_obj =
          raw_of(game).ptr(0x18cc + static_cast<std::size_t>(player_slot) * 4);
      raw_of(mod).at<std::uint32_t>(0x08) =
          raw_of(slot_obj).at<std::uint32_t>(0x28);
      break;
    }
    default:
      break;
  }

  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg68[0x44]{};
    void* const msg_ptr =
        host.session_message68_construct_from_modifier_008e6fb0(msg68, mod, 0);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg_ptr, 0);
    host.session_message68_destroy_007658c0(msg68);
  }
}

// [BSP reconstruction 008ea4e0]
// Original ABI: __thiscall(void* manager /*ECX*/, void* msg68), RET 0x4.
// Hypothesis, not a recovered symbol. Handles MT_POWERUP_EFFECT (SessionMessage68)
// on multiplayer clients (game + 0x1fe4 == 2): adds a new modifier on op 0 or
// finds and expires the matching modifier on op 2.
void power_up_manager_on_powerup_effect_message_008ea4e0(Seg58Host& host,
                                                         void* manager,
                                                         void* msg68) {
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) != 2) {
    return;
  }

  const std::int32_t category = raw_of(msg68).at<std::int32_t>(0x18);
  std::uint8_t* const category_list =
      raw_of(manager).p + 0x80 + category * 0xc;
  const std::int32_t op = raw_of(msg68).at<std::int32_t>(0x38);

  if (op == 0) {
    std::uint8_t temp_mod[0x28]{};
    void* const constructed = host.powerup_modifier_construct_008e6ac0(temp_mod);
    host.stl_list_push_back_008e9680(category_list, constructed);
    void* const str_data = raw_of(temp_mod).ptr(0x24);
    raw_of(temp_mod).at<std::uint32_t>(0x00) = 0;
    if (str_data != nullptr) {
      const std::uint32_t str_len = raw_of(temp_mod).at<std::uint32_t>(0x20);
      void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                    str_len + 1, 1);
    }
    void* const head = raw_of(category_list).ptr(0x04);
    void* const back_node = raw_of(head).ptr(0x04);
    if (back_node == head) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (back_node == raw_of(category_list).ptr(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    host.powerup_modifier_init_from_message68_008e6b10(
        raw_of(back_node).p + 0x08, msg68);
  } else if (op == 2) {
    for (void* node = raw_of(raw_of(category_list).ptr(0x04)).ptr(0x00);
         node != raw_of(category_list).ptr(0x04);
         node = raw_of(node).ptr(0x00)) {
      if (host.powerup_modifier_matches_message68_008e3570(
              raw_of(node).p + 0x08, msg68) != 0) {
        if (node == raw_of(category_list).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        host.power_up_manager_expire_modifier_008e8c30(manager,
                                                       raw_of(node).p + 0x08);
        return;
      }
      if (node == raw_of(category_list).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
    }
  }
}

// [BSP reconstruction 008ea610]
// Original ABI: __fastcall(void* lua_state /*ECX*/) -> int, RET.
// Hypothesis, not a recovered symbol; Lua global name `PreparePowerup` is genuine.
std::int32_t lua_binding_prepare_powerup_008ea610(Seg58Host& host,
                                                  void* lua_state) {
  if ((host.global_u32(0x00f88c3c) & 1u) == 0) {
    host.global_u32(0x00f88c3c) |= 1u;
    std::uint8_t temp_str[8]{};
    host.native_string_assign_0041e870(
        temp_str,
        reinterpret_cast<const char*>(host.image_address(0x00d11294)));
    host.global_u32(0x00f88c38) = 0;
    void* const str_data = raw_of(temp_str).ptr(0x04);
    if (str_data != nullptr) {
      const std::uint32_t str_len = raw_of(temp_str).at<std::uint32_t>(0x00);
      void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                    str_len + 1, 1);
    }
  }

  std::uint8_t state_owner[0x4c4]{};
  std::uint8_t call_frame[0x14]{};
  std::uint8_t arg0[0x14]{};

  host.lua_state_owner_construct_borrowed_00b66c00(state_owner, lua_state);
  host.lua_object_open_call_frame_00b679b0(state_owner, call_frame);
  if (host.triv_body_00b663f0(call_frame) == 1) {
    void* const arg_obj = host.lua_object_argument_at_00b677e0(call_frame, arg0, 0);
    void* const manager = ptr_from_u32(host.global_u32(0x00f88c30));
    host.power_up_manager_prepare_from_lua_008e9fd0(manager, arg_obj);
    host.lua_object_destruct_00b67700(arg0);
  }
  const std::int32_t result_count =
      host.lua_object_result_count_00b66400(call_frame);
  host.lua_object_destruct_00b67700(call_frame);
  host.lua_state_owner_close_00b669a0(state_owner);
  return result_count;
}

// [BSP reconstruction 008eaad0]
// Original ABI: __thiscall(void* self /*ECX*/, uint8_t flags), RET 0x4.
// Hypothesis, not a recovered symbol. Destroys random-name map node payload at
// self + 0xc and conditionally frees self.
void* cg_scalar_deleting_dtor_008eaad0(Seg58Host& host, void* self,
                                       std::uint8_t flags) {
  host.stl_powerup_random_name_map_value_destroy_008e9700(raw_of(self).p + 0x0c);
  if ((flags & 1u) != 0) {
    host.free_00bf65ac(self);
  }
  return self;
}

// [BSP reconstruction 008eab00]
// Original ABI: __thiscall(void* manager /*ECX*/, int player_slot, void* call_frame), RET 0x8.
// Hypothesis, not a recovered symbol. Collects ready powerup classes for
// player_slot and populates a 1-indexed Lua table with their names.
void power_up_manager_build_available_names_table_008eab00(
    Seg58Host& host, void* manager, std::int32_t player_slot,
    void* call_frame) {
  std::uint8_t ready_vec[0x10]{};
  host.power_up_manager_list_ready_items_008ea0c0(manager, player_slot,
                                                  ready_vec);

  std::uint8_t table_obj[0x14]{};
  host.lua_object_new_table_00b67930(call_frame, table_obj);

  std::uint32_t idx = 0;
  while (true) {
    std::uint8_t* const first = raw_of(ready_vec).at<std::uint8_t*>(0x04);
    std::uint8_t* const last = raw_of(ready_vec).at<std::uint8_t*>(0x08);
    if (first == nullptr ||
        idx >= static_cast<std::uint32_t>((last - first) >> 2)) {
      break;
    }
    void* const item_class = reinterpret_cast<void**>(first)[idx];
    const char* const name = raw_of(item_class).at<const char*>(0x04);
    ++idx;
    host.lua_object_set_lstring_at_index_00b672f0(
        table_obj, static_cast<std::int32_t>(idx), name);
  }

  host.lua_object_destruct_00b67700(table_obj);
  void* const vec_buf = raw_of(ready_vec).ptr(0x04);
  if (vec_buf != nullptr) {
    host.free_00bf65ac(vec_buf);
  }
}

// [BSP reconstruction 008eabb0]
// Original ABI: __fastcall(int player_slot /*ECX*/, void* flight_unit /*EDX*/,
//   void* pending_pair /*stack*/), RET 0x4.
// Hypothesis, not a recovered symbol. Callback invoked when an air-support flight
// spawns; broadcasts op 4 on host, registers observer pair, records slot +0x30c
// and active subject on cooldown record, and frees pending_pair.
void power_up_manager_on_air_support_flight_spawned_008eabb0(
    Seg58Host& host, std::int32_t player_slot, void* flight_unit,
    void* pending_pair) {
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg67[0x34]{};
    void* const msg_ptr = host.session_message67_construct_from_name_008e6ed0(
        msg67, player_slot, raw_of(pending_pair).ptr(0x00),
        raw_of(pending_pair).at<std::int32_t>(0x04), 4, flight_unit, nullptr);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg_ptr, 0);
    host.session_message67_destroy_007657d0(msg67);
  }

  void* class_key = raw_of(pending_pair).ptr(0x00);
  void* const manager = ptr_from_u32(host.global_u32(0x00f88c30));
  host.observer_register_pair_00694a60(flight_unit, manager);
  raw_of(flight_unit).at<std::int32_t>(0x30c) = player_slot;
  void* const slot_map = raw_of(manager).p + 0x14c + player_slot * 0xc;
  void* const record = host.stl_map_subscript_00617030(slot_map, &class_key);
  raw_of(record).ptr(0x04) = flight_unit;
  host.free_00bf65ac(pending_pair);
}

// [BSP reconstruction 008eac80]
// Original ABI: __fastcall(void* manager /*ECX*/), RET.
// Hypothesis, not a recovered symbol. Resolves pending air-support flight unit
// ids in manager + 0x140 (count +0x144), binds resolved units, shifts remaining
// entries, and refreshes HUD if visible.
void power_up_manager_bind_pending_flights_008eac80(Seg58Host& host,
                                                    void* manager) {
  std::int32_t idx = 0;
  if (raw_of(manager).at<std::int32_t>(0x144) <= 0) {
    return;
  }

  std::size_t byte_off = 0;
  do {
    std::uint8_t* const base = raw_of(manager).at<std::uint8_t*>(0x140);
    const std::uint32_t unit_id =
        raw_of(base + byte_off).at<std::uint16_t>(0x08);

    std::uint32_t table_addr = host.global_u32(0x00f89aa8);
    std::int32_t id_base =
        static_cast<std::int32_t>(host.global_u32(0x00f89a60));
    if (static_cast<std::int32_t>(unit_id) <
        static_cast<std::int32_t>(host.global_u32(0x00f89a10))) {
      table_addr = host.global_u32(0x00f89a54);
      id_base = static_cast<std::int32_t>(host.global_u32(0x00f89a0c));
    }

    void* const resolved_unit =
        raw_of(ptr_from_u32(table_addr))
            .ptr(static_cast<std::size_t>(
                     static_cast<std::int32_t>(unit_id) - id_base) *
                     0x10 +
                 0x0c);

    if (resolved_unit == nullptr) {
      ++idx;
      byte_off += 0xc;
    } else {
      std::uint8_t* const entry =
          raw_of(manager).at<std::uint8_t*>(0x140) + byte_off;
      const std::int32_t slot = raw_of(entry).at<std::int32_t>(0x00);
      void* class_key = raw_of(entry).ptr(0x04);

      host.observer_register_pair_00694a60(resolved_unit, manager);
      raw_of(resolved_unit).at<std::int32_t>(0x30c) = slot;
      void* const slot_map = raw_of(manager).p + 0x14c + slot * 0xc;
      void* const record =
          host.stl_map_subscript_00617030(slot_map, &class_key);
      raw_of(record).ptr(0x04) = resolved_unit;

      std::int32_t shift_idx = idx;
      std::size_t shift_off = byte_off;
      while (shift_idx < raw_of(manager).at<std::int32_t>(0x144) - 1) {
        std::uint8_t* const dst =
            raw_of(manager).at<std::uint8_t*>(0x140) + shift_off;
        raw_of(dst).at<std::uint32_t>(0x00) =
            raw_of(dst).at<std::uint32_t>(0x0c);
        raw_of(dst).at<std::uint32_t>(0x04) =
            raw_of(dst).at<std::uint32_t>(0x10);
        raw_of(dst).at<std::uint32_t>(0x08) =
            raw_of(dst).at<std::uint32_t>(0x14);
        ++shift_idx;
        shift_off += 0xc;
      }

      raw_of(manager).at<std::int32_t>(0x144) -= 1;
      void* const hud = ptr_from_u32(host.global_u32(0x00e198c4));
      if (hud != nullptr) {
        void* const screen = raw_of(hud).ptr(0xc0);
        if (raw_of(screen).at<std::uint8_t>(0x05) != 0) {
          host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
        }
      }
    }
  } while (idx < raw_of(manager).at<std::int32_t>(0x144));
}

// [BSP reconstruction 008eada0]
// Original ABI: __thiscall(void* manager /*ECX*/, void* item, int player_slot,
//   void* target_unit), RET 0xc.
// Hypothesis, not a recovered symbol. Activates a powerup item: sends client
// request in MP client mode, checks cooldown, spawns air-support flight or
// inserts category modifiers, starts cooldown, decrements uses, and refreshes HUD.
void power_up_manager_use_item_008eada0(Seg58Host& host, void* manager,
                                        void* item, std::int32_t player_slot,
                                        void* target_unit) {
  if (host.global_u8(0x00e0c978) == 0) {
    return;
  }

  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 2) {
    std::uint8_t msg67[0x34]{};
    void* const msg_ptr = host.session_message67_construct_from_class_008e6e00(
        msg67, player_slot, item, 1, nullptr, target_unit);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.session_send_message_to_secondary_first_peer_00770af0(
        raw_of(game_send).p + 0x1ef0, msg_ptr);
    host.session_message67_destroy_007657d0(msg67);
    return;
  }

  void* const powerup_class = raw_of(item).ptr(0x04);
  void* const slot_map = raw_of(manager).p + 0x14c + player_slot * 0xc;
  void* class_key = powerup_class;
  void* const record = host.stl_map_subscript_00617030(slot_map, &class_key);

  const float now = host.global_f32(0x00f876a4);
  float ready_time = 0.0f;
  if (raw_of(record).at<std::uint8_t>(0x14) != 0) {
    void* const rec_class = raw_of(record).ptr(0x08);
    ready_time = raw_of(rec_class).at<float>(0x80) + now;
  } else {
    ready_time = raw_of(record).at<float>(0x10);
  }

  if (ready_time > now) {
    return;
  }

  if (raw_of(powerup_class).at<std::int32_t>(0x08) == 1) {
    if (target_unit == nullptr) {
      return;
    }
    if (host.powerup_class_scope_accepts_unit_008e35f0(target_unit,
                                                       powerup_class) == 0) {
      return;
    }

    void* const game_slot_owner = ptr_from_u32(host.global_u32(0x00e188a8));
    void* const slot_entry = raw_of(game_slot_owner)
                                 .ptr(0x18cc + static_cast<std::size_t>(player_slot) * 4);
    const std::int32_t party = raw_of(slot_entry).at<std::int32_t>(0x28);
    std::uint8_t* const party_cfg =
        raw_of(powerup_class).p + 0x0c + party * 0x38;

    void* const pending_pair = host.operator_new_00bf681b(8);
    raw_of(pending_pair).ptr(0x00) = powerup_class;
    raw_of(pending_pair).at<std::int32_t>(0x04) =
        raw_of(item).at<std::int32_t>(0x0c);

    std::uint8_t empty_str[8]{};
    host.native_string_assign_0041e870(
        empty_str,
        reinterpret_cast<const char*>(host.image_address(0x00ce3a0c)));
    void* const spawner = ptr_from_u32(host.global_u32(0x00f89b3c));
    host.fun_0094bff0(spawner, player_slot,
                      raw_of(party_cfg).at<std::uint32_t>(0x20), target_unit,
                      raw_of(party_cfg).at<std::uint32_t>(0x1c), 0, target_unit,
                      empty_str, host.image_address(0x008eabb0), pending_pair);
    host.native_string_destroy_storage_0041dd20(empty_str);
  } else {
    if (host.powerup_class_scope_accepts_unit_008e35f0(target_unit,
                                                       powerup_class) == 0) {
      return;
    }

    const bool is_type_2 =
        (raw_of(powerup_class).at<std::int32_t>(0x08) == 2);
    bool keep_continuing = false;
    raw_of(item).at<std::int32_t>(0x00) = 0;

    for (std::int32_t cat = 0; cat < 0x10; ++cat) {
      const float mult =
          raw_of(powerup_class)
              .at<float>(0x8c + static_cast<std::size_t>(cat) * 4);
      if (mult != host.global_f32(0x00d7a24c)) {
        power_up_manager_insert_modifier_008ea250(
            host, manager, cat, mult, powerup_class, item, player_slot,
            target_unit);
        keep_continuing = is_type_2;
      }
    }

    if (!keep_continuing) {
      return;
    }
  }

  void* const game_after = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game_after).at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg67[0x34]{};
    void* const msg_ptr = host.session_message67_construct_from_class_008e6e00(
        msg67, player_slot, item, 1, target_unit, nullptr);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg_ptr, 0);
    host.session_message67_destroy_007657d0(msg67);
  }

  void* const game_mode_obj = ptr_from_u32(host.global_u32(0x00e188a8));
  const std::int32_t mode = raw_of(game_mode_obj).at<std::int32_t>(0x1fe4);
  if (mode == 0 || mode == 1) {
    host.fun_0090f5a0(raw_of(game_mode_obj).ptr(0x21a0), player_slot);
  }

  std::uint32_t temp_record[6]{};
  host.powerup_slot_record_start_cooldown_008e4b00(temp_record, powerup_class);
  std::uint32_t* const dst_record = static_cast<std::uint32_t*>(
      host.stl_map_subscript_00617030(slot_map, &class_key));
  for (int i = 0; i < 6; ++i) {
    dst_record[i] = temp_record[i];
  }

  std::int32_t uses_left = raw_of(item).at<std::int32_t>(0x0c);
  if (uses_left > 0) {
    uses_left -= 1;
    raw_of(item).at<std::int32_t>(0x0c) = uses_left;
    if (uses_left <= 0 && raw_of(powerup_class).at<std::int32_t>(0x08) != 2) {
      host.power_up_manager_remove_item_008e87c0(manager, item);
    }
  }

  void* const hud = ptr_from_u32(host.global_u32(0x00e198c4));
  if (hud != nullptr) {
    void* const screen = raw_of(hud).ptr(0xc0);
    if (raw_of(screen).at<std::uint8_t>(0x05) != 0) {
      host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
    }
  }
}

// [BSP reconstruction 008eb350]
// Original ABI: __fastcall(void* lua_state /*ECX*/) -> int, RET.
// Hypothesis, not a recovered symbol; Lua global name `GetAvailablePowerups` is genuine.
std::int32_t lua_binding_get_available_powerups_008eb350(Seg58Host& host,
                                                         void* lua_state) {
  if ((host.global_u32(0x00f88c44) & 1u) == 0) {
    host.global_u32(0x00f88c44) |= 1u;
    std::uint8_t temp_str[8]{};
    host.native_string_assign_0041e870(
        temp_str,
        reinterpret_cast<const char*>(host.image_address(0x00d11294)));
    host.global_u32(0x00f88c40) = 0;
    void* const str_data = raw_of(temp_str).ptr(0x04);
    if (str_data != nullptr) {
      const std::uint32_t str_len = raw_of(temp_str).at<std::uint32_t>(0x00);
      void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                    str_len + 1, 1);
    }
  }

  std::uint8_t state_owner[0x4c4]{};
  std::uint8_t call_frame[0x14]{};
  std::uint8_t arg0[0x14]{};

  host.lua_state_owner_construct_borrowed_00b66c00(state_owner, lua_state);
  host.lua_object_open_call_frame_00b679b0(state_owner, call_frame);

  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  std::int32_t player_slot = raw_of(game).at<std::int32_t>(0x18ec);

  if (host.triv_body_00b663f0(call_frame) >= 1) {
    void* const arg_obj =
        host.lua_object_argument_at_00b677e0(call_frame, arg0, 0);
    player_slot = host.lua_object_get_integer_00b66290(arg_obj);
    host.lua_object_destruct_00b67700(arg0);
  }

  void* const manager = ptr_from_u32(host.global_u32(0x00f88c30));
  power_up_manager_build_available_names_table_008eab00(
      host, manager, player_slot, call_frame);

  const std::int32_t result_count =
      host.lua_object_result_count_00b66400(call_frame);
  host.lua_object_destruct_00b67700(call_frame);
  host.lua_state_owner_close_00b669a0(state_owner);
  return result_count;
}

// [BSP reconstruction 008eb6e0]
// Original ABI: __thiscall(void* manager /*ECX*/, void* item), RET 0x4.
// Hypothesis, not a recovered symbol. Resolves local use target for item's
// class and activates the item for the local player slot (game + 0x18ec).
void power_up_manager_use_item_local_008eb6e0(Seg58Host& host, void* manager,
                                              void* item) {
  void* const powerup_class = raw_of(item).ptr(0x04);
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  const std::int32_t local_slot = raw_of(game).at<std::int32_t>(0x18ec);
  void* const target_unit =
      host.powerup_class_resolve_local_use_target_008e45d0(manager,
                                                           powerup_class);
  power_up_manager_use_item_008eada0(host, manager, item, local_slot,
                                     target_unit);
}

// [BSP reconstruction 008eb710]
// Original ABI: __cdecl(), RET.
// Hypothesis, not a recovered symbol. On host (game + 0x1fe4 == 1), constructs
// SessionMessage69, appends unit id (+0x174) and powerup class name (+0x760) for
// every bound unit in the world list, and broadcasts it.
void power_up_manager_broadcast_command_building_bindings_008eb710(
    Seg58Host& host) {
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) != 1) {
    return;
  }

  std::uint8_t msg69[0x38]{};
  host.session_message69_construct_00767570(msg69);

  void* const game_world_owner = ptr_from_u32(host.global_u32(0x00e188a8));
  void* const world = raw_of(game_world_owner).ptr(0x19cc);
  for (void* node = raw_of(world).ptr(0x16c); node != nullptr;
       node = raw_of(node).ptr(0x04)) {
    void* const unit = raw_of(node).ptr(0x08);
    void* const powerup_class = raw_of(unit).ptr(0x760);
    if (powerup_class != nullptr) {
      std::uint32_t unit_id = raw_of(unit).at<std::uint16_t>(0x174);
      host.stl_vector_push_back_007265e0(
          msg69 + 0x18, reinterpret_cast<const std::uint16_t*>(&unit_id));
      host.stl_checked_native_string_vector_append_00450540(msg69 + 0x28,
                                                            powerup_class);
    }
  }

  void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
  host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg69, 0);
  host.session_message69_destroy_00767610(msg69);
}

// [BSP reconstruction 008ed4b0]
// Original ABI: __thiscall(void* manager /*ECX*/, void* binding), RET 0x4.
// Hypothesis, not a recovered symbol. Grants a command-building bound powerup
// item to the building owner's slot, reports "pup_lost" or "pup_gain", refreshes
// HUD, broadcasts SessionMessage67 on host, and immediately uses passive type-3 items.
void command_building_powerup_binding_grant_to_owner_008ed4b0(Seg58Host& host,
                                                              void* manager,
                                                              void* binding) {
  if (host.global_u8(0x00e0c978) == 0) {
    return;
  }
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 2) {
    return;
  }
  if (raw_of(binding).ptr(0x08) == nullptr) {
    return;
  }

  void* const owner_unit = raw_of(binding).ptr(0x04);
  const std::uint32_t slot =
      static_cast<std::uint32_t>(host.unit_loss_counting_slot_009ffd20(owner_unit));
  if (slot > 7u) {
    return;
  }

  std::uint8_t temp_item[0x1c]{};
  void* const constructed_item = host.powerup_item_construct_008e6a80(temp_item);
  std::uint8_t* const slot_list =
      raw_of(manager).p + 0x20 + slot * 0xc;
  host.stl_inst_008ec420(slot_list, constructed_item);
  host.stl_list_clear_008e58e0(temp_item + 0x10);

  void* const back_item = host.stl_inst_008e5ed0(slot_list);
  raw_of(back_item).ptr(0x04) = raw_of(binding).ptr(0x08);
  raw_of(back_item).ptr(0x08) = binding;

  const std::uint32_t sound_addr =
      (raw_of(owner_unit).at<std::int32_t>(0x54) == 2) ? 0x00d1627cu
                                                       : 0x00d16270u;
  std::uint8_t sound_str[8]{};
  host.native_string_assign_0041e870(
      sound_str, reinterpret_cast<const char*>(host.image_address(sound_addr)));
  void* const warning_mgr = ptr_from_u32(host.global_u32(0x00f8a0c4));
  host.warning_manager_report_powerup_009789a0(
      warning_mgr, static_cast<std::int32_t>(slot), back_item, sound_str);
  host.native_string_destroy_storage_0041dd20(sound_str);

  host.power_up_manager_refresh_hud_if_shown_008e4660(manager);

  void* const game_host_check = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game_host_check).at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg67[0x34]{};
    void* const msg_ptr = host.session_message67_construct_from_class_008e6e00(
        msg67, static_cast<std::int32_t>(slot), back_item, 0, owner_unit,
        nullptr);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg_ptr, 0);
    host.session_message67_destroy_007657d0(msg67);
  }

  void* const powerup_class = raw_of(binding).ptr(0x08);
  if (raw_of(powerup_class).at<std::int32_t>(0x08) == 3) {
    power_up_manager_use_item_008eada0(
        host, manager, back_item, static_cast<std::int32_t>(slot), owner_unit);
  }
}

// [BSP reconstruction 008ed620]
// Original ABI: __thiscall(void* manager /*ECX*/, void* event_obj, int event_code,
//   void* unused_arg3), RET 0xc.
// Hypothesis, not a recovered symbol. Observer callback on BSP_PowerUpManager:
// on event code 5, queries event subject (vslot +0x4) and command-building
// binding (vslot +0x64), removes previous bound items, and grants to new owner.
void power_up_manager_on_observed_event_008ed620(Seg58Host& host, void* manager,
                                                 void* event_obj,
                                                 std::int32_t event_code,
                                                 void* unused_arg3) {
  (void)unused_arg3;
  if (event_code == 5) {
    void* const subject = host.observed_event_get_subject_vslot_04(event_obj);
    void* const binding = host.unit_get_powerup_binding_vslot_64(subject);
    host.power_up_manager_remove_items_bound_to_008e8a30(manager, binding);
    command_building_powerup_binding_grant_to_owner_008ed4b0(host, manager,
                                                             binding);
  }
}

// [BSP reconstruction 008ed720]
// Original ABI: __thiscall(void* binding /*ECX*/, void* prop_container), RET 0x4.
// Hypothesis, not a recovered symbol. Reads "PowerupClassID" from the scene
// property bag on SP/host, resolves bracketed random lists via 008ec650, binds
// the class at binding + 0x8, grants to owner, and registers observer pair.
void command_building_powerup_binding_init_from_properties_008ed720(
    Seg58Host& host, void* binding, void* prop_container) {
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  const std::int32_t mode = raw_of(game).at<std::int32_t>(0x1fe4);
  if (mode != 0 && mode != 1) {
    raw_of(binding).ptr(0x08) = nullptr;
    return;
  }

  if (raw_of(prop_container).at<std::int32_t>(0x04) != 1) {
    return;
  }

  void* const bag = raw_of(prop_container).ptr(0x08);
  void* const prop = host.scene_property_bag_find_008f2260(
      bag, reinterpret_cast<const char*>(host.image_address(0x00d1628c)));
  const char* const raw_str = host.scene_property_get_string_value_008f0df0(prop);

  std::uint8_t prop_str[8]{};
  host.native_string_assign_0041e870(prop_str, raw_str);

  std::uint8_t picked_str[8]{};
  const char* const prop_data = raw_of(prop_str).at<const char*>(0x04);
  const std::uint32_t prop_len = raw_of(prop_str).at<std::uint32_t>(0x00);

  const char* bracket = nullptr;
  if (prop_data != nullptr) {
    bracket = host.strstr_00bf9440(
        prop_data,
        reinterpret_cast<const char*>(host.image_address(0x00cf5980)));
  }

  if (prop_data != nullptr && bracket != nullptr && (bracket - prop_data) > 0) {
    std::uint32_t pos = 0;
    if (prop_len > 0) {
      do {
        std::uint32_t end_pos = 0xffffffffu;
        if (prop_len != 0) {
          std::uint32_t search_pos =
              (static_cast<std::int32_t>(pos) < 0) ? 0u : pos;
          if (static_cast<std::int32_t>(pos) < 0 || pos <= prop_len) {
            const char* const cur_data = raw_of(prop_str).at<const char*>(0x04);
            const std::uint32_t span = host.strcspn_00bf9a50(
                cur_data + search_pos,
                reinterpret_cast<const char*>(host.image_address(0x00d16288)));
            const std::uint32_t candidate = search_pos + span;
            if (candidate != prop_len) {
              end_pos = candidate;
            }
          }
        }

        if (static_cast<std::int32_t>(pos) <
                static_cast<std::int32_t>(end_pos) &&
            end_pos != 0xffffffffu) {
          std::uint8_t sub_str[8]{};
          host.native_string_substring_00469840(prop_str, sub_str, pos,
                                                end_pos - pos);
          const char* const trim_chars =
              reinterpret_cast<const char*>(host.image_address(0x00cf58fc));
          host.native_string_trim_left_set_00585eb0(sub_str, trim_chars);
          host.native_string_trim_right_set_00584110(sub_str, trim_chars);

          std::uint8_t arg_str[8]{};
          host.native_string_copy_construct_00426060(arg_str, sub_str);

          std::uint8_t picked_tmp[8]{};
          void* const manager_pick = ptr_from_u32(host.global_u32(0x00f88c30));
          void* const picked_res =
              host.powerup_pick_from_bracket_list_provisional_008ec650(
                  manager_pick, picked_tmp, arg_str);
          host.native_string_assign_header_00425f40(picked_str, picked_res);

          void* const tmp_data = raw_of(picked_tmp).ptr(0x04);
          if (tmp_data != nullptr) {
            const std::uint32_t tmp_len =
                raw_of(picked_tmp).at<std::uint32_t>(0x00);
            void* const pool =
                host.sized_storage_pool_get_singleton_00419cc0();
            host.sized_storage_pool_return_block_00bd1510(pool, tmp_data,
                                                          tmp_len + 1, 1);
          }

          void* const manager_find = ptr_from_u32(host.global_u32(0x00f88c30));
          void* const cls = host.fun_008e6cb0(manager_find, picked_str);
          raw_of(binding).ptr(0x08) = cls;
          if (cls != nullptr) {
            void* const manager_grant =
                ptr_from_u32(host.global_u32(0x00f88c30));
            command_building_powerup_binding_grant_to_owner_008ed4b0(
                host, manager_grant, binding);
            void* const manager_obs =
                ptr_from_u32(host.global_u32(0x00f88c30));
            host.observer_register_pair_00694a60(raw_of(binding).ptr(0x04),
                                                 manager_obs);
          }

          void* const sub_data = raw_of(sub_str).ptr(0x04);
          if (sub_data != nullptr) {
            const std::uint32_t sub_len =
                raw_of(sub_str).at<std::uint32_t>(0x00);
            void* const pool =
                host.sized_storage_pool_get_singleton_00419cc0();
            host.sized_storage_pool_return_block_00bd1510(pool, sub_data,
                                                          sub_len + 1, 1);
          }
        }

        pos = end_pos + 1;
      } while (pos < prop_len);
    }
  } else {
    void* const manager_find = ptr_from_u32(host.global_u32(0x00f88c30));
    void* const cls = host.fun_008e6cb0(manager_find, prop_str);
    raw_of(binding).ptr(0x08) = cls;
    if (cls != nullptr) {
      void* const manager_grant = ptr_from_u32(host.global_u32(0x00f88c30));
      command_building_powerup_binding_grant_to_owner_008ed4b0(
          host, manager_grant, binding);
      void* const manager_obs = ptr_from_u32(host.global_u32(0x00f88c30));
      host.observer_register_pair_00694a60(raw_of(binding).ptr(0x04),
                                           manager_obs);
    }
  }

  void* const picked_data = raw_of(picked_str).ptr(0x04);
  if (picked_data != nullptr) {
    const std::uint32_t picked_len =
        raw_of(picked_str).at<std::uint32_t>(0x00);
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, picked_data,
                                                  picked_len + 1, 1);
  }

  void* const final_prop_data = raw_of(prop_str).ptr(0x04);
  if (final_prop_data != nullptr) {
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, final_prop_data,
                                                  prop_len + 1, 1);
  }
}

// [BSP reconstruction 008ed9c0]
// Original ABI: __fastcall(void* manager /*ECX*/) -> uint8_t, RET.
// Hypothesis, not a recovered symbol. Copies the local player's item list and
// returns 1 if any item has class type 3 or nonzero remaining uses (+0x14 on node).
std::uint8_t power_up_manager_local_player_has_usable_item_008ed9c0(
    Seg58Host& host, void* manager) {
  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  const std::int32_t local_slot = raw_of(game).at<std::int32_t>(0x18ec);
  void* const src_list = raw_of(manager).p + 0x20 + local_slot * 0xc;

  std::uint8_t copy_list[0xc]{};
  host.stl_list_copy_construct_008ed660(copy_list, src_list);

  void* head = raw_of(copy_list).ptr(0x04);
  for (void* node = raw_of(head).ptr(0x00); node != head;
       node = raw_of(node).ptr(0x00)) {
    void* const item_class = raw_of(node).ptr(0x0c);
    if (raw_of(item_class).at<std::int32_t>(0x08) == 3 ||
        raw_of(node).at<std::int32_t>(0x14) != 0) {
      host.stl_inst_008e7da0(copy_list);
      host.free_00bf65ac(raw_of(copy_list).ptr(0x04));
      return 1;
    }
    if (node == head) {
      host.invalid_parameter_noinfo_00bf6713();
      head = raw_of(copy_list).ptr(0x04);
    }
  }

  host.stl_inst_008e7da0(copy_list);
  host.free_00bf65ac(raw_of(copy_list).ptr(0x04));
  return 0;
}

// [BSP reconstruction 008edad0]
// Original ABI: __fastcall(void* manager /*ECX*/), RET.
// Hypothesis, not a recovered symbol. Destructor body of BSP_PowerUpManager:
// clears global singleton 00f88c30, destroys internal maps/arrays/lists, and
// destroys base CallbackOwner.
void cg_vector_deleting_dtor_008edad0(Seg58Host& host, void* manager) {
  raw_of(manager).ptr(0x00) = host.image_address(0x00d1629c);
  host.global_u32(0x00f88c30) = 0;

  std::uint8_t iter_out[8]{};

  std::uint8_t* const tree_1b8 = raw_of(manager).p + 0x1b8;
  void* const head_1b8 = raw_of(tree_1b8).ptr(0x04);
  host.stl_tree_erase_range_008ec550(tree_1b8, iter_out, tree_1b8,
                                     raw_of(head_1b8).ptr(0x00), tree_1b8,
                                     head_1b8);
  host.free_00bf65ac(raw_of(tree_1b8).ptr(0x04));
  raw_of(tree_1b8).ptr(0x04) = nullptr;
  raw_of(tree_1b8).at<std::uint32_t>(0x08) = 0;

  std::uint8_t* const tree_1ac = raw_of(manager).p + 0x1ac;
  void* const head_1ac = raw_of(tree_1ac).ptr(0x04);
  host.stl_tree_erase_range_008ec550(tree_1ac, iter_out, tree_1ac,
                                     raw_of(head_1ac).ptr(0x00), tree_1ac,
                                     head_1ac);
  host.free_00bf65ac(raw_of(tree_1ac).ptr(0x04));
  raw_of(tree_1ac).ptr(0x04) = nullptr;
  raw_of(tree_1ac).at<std::uint32_t>(0x08) = 0;

  host.eh_vector_destructor_iterator_00bf7c6e(
      raw_of(manager).p + 0x14c, 0xc, 8, host.image_address(0x008e9ab0));

  std::uint8_t* const arr_140 = raw_of(manager).p + 0x140;
  host.tmpl_array12_resize_008e5170(arr_140, 0);
  host.free_00bf6989(raw_of(arr_140).ptr(0x00));

  host.eh_vector_destructor_iterator_00bf7c6e(
      raw_of(manager).p + 0x80, 0xc, 0x10, host.image_address(0x008e8e00));
  host.eh_vector_destructor_iterator_00bf7c6e(
      raw_of(manager).p + 0x20, 0xc, 8, host.image_address(0x008e8de0));

  std::uint8_t* const tree_14 = raw_of(manager).p + 0x14;
  void* const head_14 = raw_of(tree_14).ptr(0x04);
  host.stl_tree_erase_range_008ea990(tree_14, iter_out, tree_14,
                                     raw_of(head_14).ptr(0x00), tree_14,
                                     head_14);
  host.free_00bf65ac(raw_of(tree_14).ptr(0x04));
  raw_of(tree_14).ptr(0x04) = nullptr;
  raw_of(tree_14).at<std::uint32_t>(0x08) = 0;

  host.callback_owner_destroy_00695870(manager);
}

// [BSP reconstruction 008edda0]
// Original ABI: __thiscall(void* self /*ECX*/, uint8_t flags), RET 0x4.
// Hypothesis, not a recovered symbol. Scalar deleting destructor for
// BSP_PowerUpManager.
void* cg_scalar_deleting_dtor_008edda0(Seg58Host& host, void* self,
                                       std::uint8_t flags) {
  cg_vector_deleting_dtor_008edad0(host, self);
  if ((flags & 1u) != 0) {
    host.free_00bf65ac(self);
  }
  return self;
}

// [BSP reconstruction 008eddc0]
// Original ABI: __thiscall(void* manager /*ECX*/, int player_slot, void* lua_obj), RET 0x8.
// Hypothesis, not a recovered symbol. Appends a new item to manager + 0x20 +
// player_slot * 0xc, loads it from lua_obj, reports "pup_gain", triggers
// "PUM1STGET" hint if not yet shown, and broadcasts SessionMessage67 on host.
void power_up_manager_grant_item_008eddc0(Seg58Host& host, void* manager,
                                          std::int32_t player_slot,
                                          void* lua_obj) {
  if (host.global_u8(0x00e0c978) == 0) {
    return;
  }

  std::uint8_t temp_item[0x1c]{};
  raw_of(temp_item).at<std::uint32_t>(0x00) = 0;
  raw_of(temp_item).at<std::uint32_t>(0x04) = 0;
  raw_of(temp_item).at<std::uint32_t>(0x08) = 0;
  raw_of(temp_item).at<std::uint32_t>(0x0c) = 0xffffffffu;
  raw_of(temp_item).ptr(0x14) = host.stl_list_buy_head_node_008e58c0();
  raw_of(temp_item).at<std::uint32_t>(0x18) = 0;

  std::uint8_t* const slot_list =
      raw_of(manager).p + 0x20 + player_slot * 0xc;
  void* const head = raw_of(slot_list).ptr(0x04);
  void* const new_node =
      host.stl_inst_008ebbe0(head, raw_of(head).ptr(0x04), temp_item);
  host.stl_xlen_throw_008e7e10(slot_list, 1);
  raw_of(head).ptr(0x04) = new_node;
  raw_of(raw_of(new_node).ptr(0x04)).ptr(0x00) = new_node;

  host.stl_list_clear_008e58e0(temp_item + 0x10);

  void* const head_after = raw_of(slot_list).ptr(0x04);
  void* const back_node = raw_of(head_after).ptr(0x04);
  if (back_node == head_after) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (back_node == raw_of(slot_list).ptr(0x04)) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  std::uint8_t* const back_item = raw_of(back_node).p + 0x08;

  std::uint8_t copied_lua[0x14]{};
  host.lua_object_copy_construct_00b66fa0(copied_lua, lua_obj);
  std::uint8_t reader[0x14]{};
  void* const reader_ptr =
      host.lua_reader_construct_004425c0(reader, copied_lua);
  host.powerup_item_load_lua_008e7820(back_item, reader_ptr);
  host.lua_reader_destroy_stack_instance_00441a20(reader);

  std::uint8_t gain_str[8]{};
  host.native_string_assign_0041e870(
      gain_str,
      reinterpret_cast<const char*>(host.image_address(0x00d16270)));
  void* const warning_mgr = ptr_from_u32(host.global_u32(0x00f8a0c4));
  host.warning_manager_report_powerup_009789a0(warning_mgr, player_slot,
                                               back_item, gain_str);
  void* const gain_data = raw_of(gain_str).ptr(0x04);
  if (gain_data != nullptr) {
    const std::uint32_t gain_len = raw_of(gain_str).at<std::uint32_t>(0x00);
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, gain_data,
                                                  gain_len + 1, 1);
  }

  void* const hud = ptr_from_u32(host.global_u32(0x00e198c4));
  if (hud != nullptr) {
    void* const screen = raw_of(hud).ptr(0xc0);
    if (raw_of(screen).at<std::uint8_t>(0x05) != 0) {
      host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
    }
  }

  std::uint8_t hint_str1[8]{};
  host.native_string_assign_0041e870(
      hint_str1,
      reinterpret_cast<const char*>(host.image_address(0x00d162b4)));
  void* const tracker1 = host.award_tracker_get_singleton_004e1ca0();
  const bool not_shown =
      (host.award_tracker_has_hint_been_shown_0068e600(tracker1, hint_str1) ==
       0);
  void* const hint_data1 = raw_of(hint_str1).ptr(0x04);
  if (hint_data1 != nullptr) {
    const std::uint32_t hint_len1 = raw_of(hint_str1).at<std::uint32_t>(0x00);
    void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, hint_data1,
                                                  hint_len1 + 1, 1);
  }

  if (not_shown) {
    std::uint8_t hint_str2[8]{};
    host.native_string_assign_0041e870(
        hint_str2,
        reinterpret_cast<const char*>(host.image_address(0x00d162b4)));
    void* const tracker2 = host.award_tracker_get_singleton_004e1ca0();
    host.award_tracker_trigger_hint_00690fd0(tracker2, hint_str2, 0);
    void* const hint_data2 = raw_of(hint_str2).ptr(0x04);
    if (hint_data2 != nullptr) {
      const std::uint32_t hint_len2 = raw_of(hint_str2).at<std::uint32_t>(0x00);
      void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, hint_data2,
                                                    hint_len2 + 1, 1);
    }
  }

  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  if (raw_of(game).at<std::int32_t>(0x1fe4) == 1) {
    std::uint8_t msg67[0x34]{};
    void* const msg_ptr = host.session_message67_construct_from_class_008e6e00(
        msg67, player_slot, back_item, 0, nullptr, nullptr);
    void* const game_send = ptr_from_u32(host.global_u32(0x00e188a8));
    host.cg_adjustor_thunk_0076a9f0(raw_of(game_send).p + 0x1ef0, msg_ptr, 0);
    host.session_message67_destroy_007657d0(msg67);
  }
}

// [BSP reconstruction 008ee020]
// Original ABI: __thiscall(void* manager /*ECX*/, void* msg67), RET 0x4.
// Hypothesis, not a recovered symbol. Handles MT_POWERUP (SessionMessage67)
// across multiplayer clients (op 0/1/2/3/4) and multiplayer host (client use
// request op 1).
void power_up_manager_on_powerup_message_008ee020(Seg58Host& host,
                                                  void* manager, void* msg67) {
  const std::int32_t player_slot = raw_of(msg67).at<std::int32_t>(0x18);
  std::uint8_t* const slot_base = raw_of(manager).p + player_slot * 0xc;
  std::uint8_t* const slot_list = slot_base + 0x20;

  void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
  const std::int32_t mode = raw_of(game).at<std::int32_t>(0x1fe4);

  if (mode == 2) {
    const std::uint32_t op = raw_of(msg67).at<std::uint32_t>(0x28);
    switch (op) {
      case 3: {
        void* const mgr_singleton = ptr_from_u32(host.global_u32(0x00f88c30));
        host.fun_008e6cb0(mgr_singleton, raw_of(msg67).p + 0x1c);
        [[fallthrough]];
      }
      case 0: {
        std::uint8_t temp_item[0x1c]{};
        void* const constructed_item =
            host.powerup_item_construct_008e6a80(temp_item);
        host.stl_inst_008ec420(slot_list, constructed_item);
        host.stl_list_clear_008e58e0(temp_item + 0x10);

        void* const head1 = raw_of(slot_list).ptr(0x04);
        void* const back_node1 = raw_of(head1).ptr(0x04);
        if (back_node1 == head1) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        if (back_node1 == raw_of(slot_list).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        host.powerup_item_init_from_message67_008e7930(
            raw_of(back_node1).p + 0x08, msg67);

        std::uint8_t gain_str[8]{};
        host.native_string_assign_0041e870(
            gain_str,
            reinterpret_cast<const char*>(host.image_address(0x00d16270)));
        void* const head2 = raw_of(slot_list).ptr(0x04);
        void* const back_node2 = raw_of(head2).ptr(0x04);
        if (back_node2 == head2) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        if (back_node2 == raw_of(slot_list).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        void* const warning_mgr = ptr_from_u32(host.global_u32(0x00f8a0c4));
        host.warning_manager_report_powerup_009789a0(
            warning_mgr, player_slot, raw_of(back_node2).p + 0x08, gain_str);
        void* const gain_data = raw_of(gain_str).ptr(0x04);
        if (gain_data != nullptr) {
          const std::uint32_t gain_len =
              raw_of(gain_str).at<std::uint32_t>(0x00);
          void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
          host.sized_storage_pool_return_block_00bd1510(pool, gain_data,
                                                        gain_len + 1, 1);
        }

        std::uint8_t hint_str1[8]{};
        host.native_string_assign_0041e870(
            hint_str1,
            reinterpret_cast<const char*>(host.image_address(0x00d162b4)));
        void* const tracker1 = host.award_tracker_get_singleton_004e1ca0();
        const bool not_shown =
            (host.award_tracker_has_hint_been_shown_0068e600(tracker1,
                                                             hint_str1) == 0);
        void* const hint_data1 = raw_of(hint_str1).ptr(0x04);
        if (hint_data1 != nullptr) {
          const std::uint32_t hint_len1 =
              raw_of(hint_str1).at<std::uint32_t>(0x00);
          void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
          host.sized_storage_pool_return_block_00bd1510(pool, hint_data1,
                                                        hint_len1 + 1, 1);
        }

        if (not_shown) {
          std::uint8_t hint_str2[8]{};
          host.native_string_assign_0041e870(
              hint_str2,
              reinterpret_cast<const char*>(host.image_address(0x00d162b4)));
          void* const tracker2 = host.award_tracker_get_singleton_004e1ca0();
          host.award_tracker_trigger_hint_00690fd0(tracker2, hint_str2, 0);
          host.native_string_destroy_storage_0041dd20(hint_str2);
        }

        void* const hud = ptr_from_u32(host.global_u32(0x00e198c4));
        if (hud != nullptr) {
          void* const screen = raw_of(hud).ptr(0xc0);
          if (raw_of(screen).at<std::uint8_t>(0x05) != 0) {
            host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
          }
        }
        return;
      }
      case 1:
      case 2:
      case 4: {
        void* iter_pair[2]{slot_list,
                           raw_of(raw_of(slot_list).ptr(0x04)).ptr(0x00)};
        for (void* node = iter_pair[1]; node != raw_of(slot_list).ptr(0x04);
             node = raw_of(node).ptr(0x00)) {
          if (raw_of(node).at<std::int32_t>(0x14) ==
                  raw_of(msg67).at<std::int32_t>(0x24) &&
              host.native_string_equals_insensitive_00435c40(
                  raw_of(node).ptr(0x0c), raw_of(msg67).p + 0x1c) != 0) {
            iter_pair[1] = node;
            const std::uint32_t cur_op = raw_of(msg67).at<std::uint32_t>(0x28);
            if (cur_op == 1) {
              if (node == raw_of(slot_list).ptr(0x04)) {
                host.invalid_parameter_noinfo_00bf6713();
              }
              void* item_class = raw_of(node).ptr(0x0c);
              std::uint32_t temp_record[6]{};
              host.powerup_slot_record_start_cooldown_008e4b00(temp_record,
                                                               item_class);
              std::uint32_t* const dst_record = static_cast<std::uint32_t*>(
                  host.stl_map_subscript_00617030(slot_base + 0x14c,
                                                  &item_class));
              for (int i = 0; i < 6; ++i) {
                dst_record[i] = temp_record[i];
              }
              if (node == raw_of(slot_list).ptr(0x04)) {
                host.invalid_parameter_noinfo_00bf6713();
              }
              if (raw_of(node).at<std::int32_t>(0x14) > 0) {
                void* const item_ptr = host.stl_inst_008e57f0(iter_pair);
                raw_of(item_ptr).at<std::int32_t>(0x0c) -= 1;
              }
            } else if (cur_op == 2) {
              void* const item_ptr = host.stl_inst_008e57c0(iter_pair);
              host.power_up_manager_remove_item_008e87c0(manager, item_ptr);
            } else if (cur_op == 4) {
              void* const item_ptr = host.stl_inst_008e57f0(iter_pair);
              std::uint8_t pending_entry[0xc]{};
              raw_of(pending_entry).at<std::uint32_t>(0x00) =
                  raw_of(msg67).at<std::uint32_t>(0x18);
              raw_of(pending_entry).ptr(0x04) = raw_of(item_ptr).ptr(0x04);
              raw_of(pending_entry).at<std::uint16_t>(0x08) =
                  raw_of(msg67).at<std::uint16_t>(0x2c);
              host.tmpl_array12_push_back_008e5670(raw_of(manager).p + 0x140,
                                                   pending_entry);
            }

            void* const hud = ptr_from_u32(host.global_u32(0x00e198c4));
            if (hud != nullptr) {
              void* const screen = raw_of(hud).ptr(0xc0);
              if (raw_of(screen).at<std::uint8_t>(0x05) != 0) {
                host.hud_powerup_manager_screen_rebuild_lists_0061b0f0(screen);
              }
            }
            return;
          }
          if (node == raw_of(slot_list).ptr(0x04)) {
            host.invalid_parameter_noinfo_00bf6713();
          }
        }
        break;
      }
      default:
        break;
    }
  } else if (mode == 1 && raw_of(msg67).at<std::int32_t>(0x28) == 1) {
    for (void* node = raw_of(raw_of(slot_list).ptr(0x04)).ptr(0x00);
         node != raw_of(slot_list).ptr(0x04);
         node = raw_of(node).ptr(0x00)) {
      if (raw_of(node).at<std::int32_t>(0x14) ==
              raw_of(msg67).at<std::int32_t>(0x24) &&
          host.native_string_equals_insensitive_00435c40(
              raw_of(node).ptr(0x0c), raw_of(msg67).p + 0x1c) != 0) {
        const std::uint32_t unit_id = raw_of(msg67).at<std::uint16_t>(0x2c);
        std::uint32_t table_addr = host.global_u32(0x00f89aa8);
        std::int32_t id_base =
            static_cast<std::int32_t>(host.global_u32(0x00f89a60));
        if (static_cast<std::int32_t>(unit_id) <
            static_cast<std::int32_t>(host.global_u32(0x00f89a10))) {
          table_addr = host.global_u32(0x00f89a54);
          id_base = static_cast<std::int32_t>(host.global_u32(0x00f89a0c));
        }
        void* const target_unit =
            raw_of(ptr_from_u32(table_addr))
                .ptr(static_cast<std::size_t>(
                         static_cast<std::int32_t>(unit_id) - id_base) *
                         0x10 +
                     0x0c);
        if (node == raw_of(slot_list).ptr(0x04)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        power_up_manager_use_item_008eada0(
            host, manager, raw_of(node).p + 0x08, player_slot, target_unit);
        return;
      }
      if (node == raw_of(slot_list).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
    }
  }
}

// [BSP reconstruction 008ee410]
// Original ABI: __fastcall(void* lua_state /*ECX*/) -> int, RET.
// Hypothesis, not a recovered symbol; Lua global name `AddPowerup` is genuine.
std::int32_t lua_binding_add_powerup_008ee410(Seg58Host& host,
                                              void* lua_state) {
  if ((host.global_u32(0x00f88c4c) & 1u) == 0) {
    host.global_u32(0x00f88c4c) |= 1u;
    std::uint8_t temp_str[8]{};
    host.native_string_assign_0041e870(
        temp_str,
        reinterpret_cast<const char*>(host.image_address(0x00d11294)));
    host.global_u32(0x00f88c48) = 0;
    void* const str_data = raw_of(temp_str).ptr(0x04);
    if (str_data != nullptr) {
      const std::uint32_t str_len = raw_of(temp_str).at<std::uint32_t>(0x00);
      void* const pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, str_data,
                                                    str_len + 1, 1);
    }
  }

  std::uint8_t state_owner[0x4c4]{};
  std::uint8_t call_frame[0x14]{};
  std::uint8_t arg_first[0x14]{};
  std::uint8_t arg_second[0x14]{};

  host.lua_state_owner_construct_borrowed_00b66c00(state_owner, lua_state);
  host.lua_object_open_call_frame_00b679b0(state_owner, call_frame);

  if (host.triv_body_00b663f0(call_frame) == 1) {
    void* const item_obj =
        host.lua_object_argument_at_00b677e0(call_frame, arg_first, 0);
    void* const game = ptr_from_u32(host.global_u32(0x00e188a8));
    const std::int32_t local_slot = raw_of(game).at<std::int32_t>(0x18ec);
    void* const manager = ptr_from_u32(host.global_u32(0x00f88c30));
    power_up_manager_grant_item_008eddc0(host, manager, local_slot, item_obj);
    host.lua_object_destruct_00b67700(arg_first);
  } else {
    void* const item_obj =
        host.lua_object_argument_at_00b677e0(call_frame, arg_second, 1);
    void* const slot_obj =
        host.lua_object_argument_at_00b677e0(call_frame, arg_first, 0);
    const std::int32_t slot = host.lua_object_get_integer_00b66290(slot_obj);
    void* const manager = ptr_from_u32(host.global_u32(0x00f88c30));
    power_up_manager_grant_item_008eddc0(host, manager, slot, item_obj);
    host.lua_object_destruct_00b67700(arg_first);
    host.lua_object_destruct_00b67700(arg_second);
  }

  const std::int32_t result_count =
      host.lua_object_result_count_00b66400(call_frame);
  host.lua_object_destruct_00b67700(call_frame);
  host.lua_state_owner_close_00b669a0(state_owner);
  return result_count;
}

}  // namespace bsp::powerup_seg58_argon
