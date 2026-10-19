#include "bsp/director_geommesh_seg38_39_argon.hpp"
#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_sweepA.hpp"

#include <cstring>

namespace bsp::director_geommesh_seg38_39_argon {

namespace {

inline Raw raw_of(void* p) noexcept {
  return Raw{static_cast<std::uint8_t*>(p)};
}

inline Raw raw_of(const void* p) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(p))};
}

}  // namespace

// 0071bc30: TRIV_body_0071bc30
// Original ABI: __fastcall(ECX=self), RET 0. Installs vtable 00cfd93c at +0x00.
void triv_body_0071bc30(Seg38_39Host& host, void* self) {
  raw_of(self).ptr(0x00) = host.image_address(0x00cfd93c);
}

// 0071bdb0: TRIV_body_0071bdb0
// Original ABI: __fastcall(ECX=self) -> EAX=self+0x10, RET 0.
void* triv_body_0071bdb0(void* self) {
  return raw_of(self).p + 0x10;
}

// 0071bdc0: TRIV_body_0071bdc0
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void triv_body_0071bdc0(void* /*self*/, std::uint32_t /*arg1*/) {}

// 0071bdd0: TRIV_body_0071bdd0
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void triv_body_0071bdd0(void* /*self*/, std::uint32_t /*arg1*/) {}

// 0071bfc0: BSP_WeaponDirector_SlotPathObject
// Original ABI: __thiscall(ECX=self, stack index) -> EAX=[self + 0x1a4 + index*4], RET 4.
void* bsp_weapon_director_slot_path_object_0071bfc0(const void* self, std::int32_t index) {
  return raw_of(self).ptr(0x1a4 + static_cast<std::size_t>(index) * 4);
}

// 0071c0b0: CG_adjustor_thunk_0071c0b0
// Original ABI: __thiscall(ECX=self, stack index), tail JMP 007ae060 with ECX = [self+0x1a4] + 0x10.
void cg_adjustor_thunk_0071c0b0(Seg38_39Host& host, void* self, std::int32_t index) {
  void* slot0 = raw_of(self).ptr(0x1a4);
  void* cursor = raw_of(slot0).p + 0x10;
  host.path_cursor_set_index_from_message_007ae060(cursor, index);
}

// 0071c150: BSP_WeaponDirector_ClassTest5C
// Original ABI: __thiscall(ECX=self, stack other), RET 4.
void bsp_weapon_director_class_test_5c_0071c150(Seg38_39Host& host, void* self, void* other) {
  Raw s = raw_of(self);
  if (host.unit_instance_is_occupant_owner_00778890(s.ptr(0x34)) && other != nullptr) {
    void* other_sub = host.vcall_114(other);
    void* own_sub = host.vcall_114(s.ptr(0x34));
    host.vcall_4c(other_sub, own_sub);
  }
}

// 0071c1a0: BSP_WeaponDirectorObserver_ForwardSlot8
// Original ABI: __fastcall(ECX=self), tail JMP vtable[+0x08].
void bsp_weapon_director_observer_forward_slot8_0071c1a0(Seg38_39Host& host, void* self) {
  host.vcall_08(self);
}

// 0071c280: BSP_WeaponDirector_VSlot3C_Noop
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void bsp_weapon_director_vslot_3c_noop_0071c280(void* /*self*/, std::uint32_t /*arg1*/) {}

// 0071c370: TMPL_OwnedPtr_DeleteReset_0071c370
// Original ABI: __fastcall(ECX=slot), RET 0.
void tmpl_owned_ptr_delete_reset_0071c370(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    host.vcall_00_deleting(held, 1);
    s.ptr(0x00) = nullptr;
  }
}

// 0071c390: TMPL_OwnedPtr_DeleteReset_0071c390
// Original ABI: __fastcall(ECX=slot), RET 0.
void tmpl_owned_ptr_delete_reset_0071c390(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    host.vcall_00_deleting(held, 1);
    s.ptr(0x00) = nullptr;
  }
}

// 0071c470: BSP_Director_AttachTickSubnode
// Original ABI: __thiscall(ECX=self, stack owner), RET 4.
// Assembly: PUSH ECX; MOV ECX,[ESP+8]; CALL 00876020; RET 4.
void bsp_director_attach_tick_subnode_0071c470(Seg38_39Host& host, void* self, void* owner) {
  host.tick_subnode_reparent_00876020(owner, self);
}

// 0071c490: TRIV_body_0071c490
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void triv_body_0071c490(void* /*self*/, std::uint32_t /*arg1*/) {}

// 0071c5d0: TRIV_body_0071c5d0
// Original ABI: __fastcall(ECX=self), RET 0. Installs vtable 00ce4974 at +0x00.
void triv_body_0071c5d0(Seg38_39Host& host, void* self) {
  raw_of(self).ptr(0x00) = host.image_address(0x00ce4974);
}

// 0071d540: TRIV_body_0071d540
// Original ABI: __thiscall(ECX=self, stack arg1, stack arg2) -> AL=0, RET 8.
std::uint8_t triv_body_0071d540(void* /*self*/, std::uint32_t /*arg1*/, std::uint32_t /*arg2*/) {
  return 0;
}

// 0071d5b0: TRIV_body_0071d5b0
// Original ABI: __fastcall(ECX=self) -> EAX=1, RET 0.
std::uint32_t triv_body_0071d5b0(void* /*self*/) {
  return 1;
}

// 0071d5c0: TRIV_body_0071d5c0
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void triv_body_0071d5c0(void* /*self*/, std::uint32_t /*arg1*/) {}

// 0071d5f0: CG_adjustor_thunk_0071d5f0
// Original ABI: __thiscall(ECX=self, stack flags), SUB ECX,0x1c; JMP 0071e410.
void* cg_adjustor_thunk_0071d5f0(Seg38_39Host& host, void* self, std::uint8_t flags) {
  return host.scalar_deleting_dtor_0071e410(raw_of(self).p - 0x1c, flags);
}

// 0071db50: BSP_WeaponDirector_AssignSlotTarget
// Original ABI: __thiscall(ECX=self, stack src_target), RET 4. Copies 0x18-byte descriptor.
void bsp_weapon_director_assign_slot_target_0071db50(void* self, const void* src_target) {
  Raw dst = raw_of(self);
  Raw src = raw_of(src_target);
  dst.at<std::uint8_t>(0x00) = src.at<std::uint8_t>(0x00);
  dst.at<std::uint8_t>(0x01) = src.at<std::uint8_t>(0x01);
  dst.at<std::uint16_t>(0x02) = src.at<std::uint16_t>(0x02);
  dst.at<std::uint32_t>(0x04) = src.at<std::uint32_t>(0x04);
  dst.at<float>(0x08) = src.at<float>(0x08);
  dst.at<float>(0x0c) = src.at<float>(0x0c);
  dst.at<float>(0x10) = src.at<float>(0x10);
  dst.at<float>(0x14) = src.at<float>(0x14);
}

// 0071ea60: BSP_RefPtr_Release_0071ea60
// Original ABI: __fastcall(ECX=self) -> EAX=self, RET 0.
void* bsp_ref_ptr_release_0071ea60(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x00) = nullptr;
  }
  return self;
}

// 0071f0e0: BSP_RefPtr_Release_0071f0e0
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_ref_ptr_release_0071f0e0(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x00) = nullptr;
  }
}

// 0071f110: BSP_RefPtr_Assign_0071f110
// Original ABI: __thiscall(ECX=self, stack src_slot), RET 4.
void bsp_ref_ptr_assign_0071f110(Seg38_39Host& host, void* self, void* const* src_slot) {
  Raw s = raw_of(self);
  void* next = *src_slot;
  void* prev = s.ptr(0x04);
  if (prev != next) {
    s.ptr(0x04) = next;
    if (next != nullptr) {
      host.interlocked_increment_00ce221c(&raw_of(next).at<std::int32_t>(0x04));
    }
    if (prev != nullptr) {
      if (host.interlocked_decrement_00ce2220(&raw_of(prev).at<std::int32_t>(0x04)) == 0) {
        host.vcall_00(prev);
      }
    }
  }
}

// 0071f150: BSP_WeaponDirector_FireTarget
// Original ABI: __thiscall(ECX=self) -> EAX=object, RET 0.
void* bsp_weapon_director_fire_target_0071f150(Seg38_39Host& host, void* self) {
  void* target_desc = host.weapon_director_newest_command_target_0071ebf0(self);
  return host.command_target_resolve_object_00521ea0(target_desc);
}

// 0071f3b0: BSP_RefCountedPtr_Release
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_ref_counted_ptr_release_0071f3b0(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x00) = nullptr;
  }
}

// 0071f460: BSP_RefHolderDB04_Destruct
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_ref_holder_db04_destruct_0071f460(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00cfdb04);
  void* held = s.ptr(0x04);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x04) = nullptr;
  }
}

// 0071f530: BSP_RefHolderDB04_Destruct_0071f530
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_ref_holder_db04_destruct_0071f530(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00cfdb04);
  void* held = s.ptr(0x04);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x04) = nullptr;
  }
}

// 0071f9f0: BSP_Vec3Vector_Assign
// Original ABI: __thiscall(ECX=self, stack other) -> EAX=self, RET 4.
void* bsp_vec3_vector_assign_0071f9f0(Seg38_39Host& host, void* self, const void* other) {
  if (self == other) {
    return self;
  }
  Raw dst = raw_of(self);
  Raw src = raw_of(other);
  std::uint8_t* src_first = static_cast<std::uint8_t*>(src.ptr(0x04));
  if (src_first != nullptr) {
    std::ptrdiff_t src_bytes = static_cast<std::uint8_t*>(src.ptr(0x08)) - src_first;
    std::uint32_t src_len = static_cast<std::uint32_t>(src_bytes / 12);
    if (src_len != 0) {
      std::uint8_t* dst_first = static_cast<std::uint8_t*>(dst.ptr(0x04));
      std::uint32_t dst_len = 0;
      if (dst_first != nullptr) {
        dst_len = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(dst.ptr(0x08)) - dst_first) / 12);
      }
      if (src_len <= dst_len) {
        host.stl_vector12_move_forward_00419330(src_first, src.ptr(0x08), dst_first, 0, 0, 0);
        std::uint8_t* reloaded_src_first = static_cast<std::uint8_t*>(src.ptr(0x04));
        if (reloaded_src_first == nullptr) {
          dst.ptr(0x08) = dst.ptr(0x04);
          return self;
        }
        std::ptrdiff_t reloaded_count =
            (static_cast<std::uint8_t*>(src.ptr(0x08)) - reloaded_src_first) / 12;
        dst.ptr(0x08) = static_cast<std::uint8_t*>(dst.ptr(0x04)) + reloaded_count * 12;
        return self;
      }
      std::uint32_t dst_cap = 0;
      if (dst_first != nullptr) {
        dst_cap = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(dst.ptr(0x0c)) - dst_first) / 12);
      }
      const void* uninit_first = nullptr;
      void* uninit_dst = nullptr;
      if (dst_cap < src_len) {
        if (dst_first != nullptr) {
          host.free_00bf65ac(dst_first);
        }
        std::uint32_t buy_count =
            static_cast<std::uint32_t>(host.stl_checked_vector12_size_00415290(other));
        if (!host.stl_vector12_buy_00420540(self, buy_count)) {
          return self;
        }
        uninit_dst = dst.ptr(0x04);
        uninit_first = src.ptr(0x04);
      } else {
        std::int32_t cur_size = host.stl_checked_vector12_size_00415290(self);
        const std::uint8_t* mid = src_first + cur_size * 12;
        host.stl_vector12_move_forward_dispatch_0041c4d0(src_first, mid, dst_first);
        uninit_dst = dst.ptr(0x08);
        uninit_first = mid;
      }
      dst.ptr(0x08) = host.stl_inst_0071f0b0(self, uninit_first, src.ptr(0x08), uninit_dst);
      return self;
    }
  }
  host.stl_inst_00585e70(self);
  return self;
}

// 0071fc20: TRIV_body_0071fc20
// Original ABI: __fastcall(ECX=self) -> EAX=self+0x40, RET 0.
void* triv_body_0071fc20(void* self) {
  return raw_of(self).p + 0x40;
}

// 0071fc30: TRIV_body_0071fc30
// Original ABI: __fastcall(ECX=self) -> AL=1, RET 0.
std::uint8_t triv_body_0071fc30(void* /*self*/) {
  return 1;
}

// 0071fc70: BSP_DirectorPathObject_Destruct
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_director_path_object_destruct_0071fc70(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* pts = s.ptr(0x44);
  if (pts != nullptr) {
    host.free_00bf65ac(pts);
  }
  s.ptr(0x44) = nullptr;
  s.ptr(0x48) = nullptr;
  s.ptr(0x4c) = nullptr;
  s.ptr(0x10) = host.image_address(0x00cfdb04);
  void* held = s.ptr(0x14);
  if (held != nullptr) {
    if (host.interlocked_decrement_00ce2220(&raw_of(held).at<std::int32_t>(0x04)) == 0) {
      host.vcall_00(held);
    }
    s.ptr(0x14) = nullptr;
  }
  s.ptr(0x00) = host.image_address(0x00cfd93c);
}

// 0071fd10: BSP_DirectorPathObject_CopyFrom
// Original ABI: __thiscall(ECX=self, stack other), RET 4.
void bsp_director_path_object_copy_from_0071fd10(Seg38_39Host& host, void* self, const void* other) {
  if (other == self) {
    return;
  }
  Raw dst = raw_of(self);
  Raw src = raw_of(other);
  dst.at<std::uint32_t>(0x08) = src.at<std::uint32_t>(0x08);
  dst.at<std::uint32_t>(0x0c) = src.at<std::uint32_t>(0x0c);
  host.path_source_ref_copy_from_007b1bf0(dst.p + 0x10, src.p + 0x10);
  bsp_vec3_vector_assign_0071f9f0(host, dst.p + 0x40, src.p + 0x40);

  void* src_source = src.ptr(0x14);
  void* resolved_iface = nullptr;
  if (src_source != nullptr) {
    resolved_iface = host.vcall_24(src_source);
  }
  if (resolved_iface == src.p + 0x40) {
    void* temp_source = nullptr;
    void** created_slot = host.entity_path_source_create_for_slot_007b2250(&temp_source, dst.p + 0x40);
    bsp_ref_ptr_assign_0071f110(host, dst.p + 0x10, created_slot);
    if (temp_source != nullptr) {
      if (host.interlocked_decrement_00ce2220(&raw_of(temp_source).at<std::int32_t>(0x04)) == 0) {
        host.vcall_00(temp_source);
      }
    }
  }
}

// 00720180: BSP_CommandControllerBase_Construct
// Original ABI: __thiscall(ECX=self, stack owner) -> EAX=self, RET 4.
void* bsp_command_controller_base_construct_00720180(Seg38_39Host& host, void* self, void* owner) {
  Raw s = raw_of(self);
  Raw own = raw_of(owner);
  s.ptr(0x00) = host.image_address(0x00cfd99c);
  s.ptr(0x04) = nullptr;
  s.ptr(0x08) = nullptr;
  s.ptr(0x0c) = nullptr;
  s.at<std::uint8_t>(0x10) = 0;
  s.at<std::uint8_t>(0x11) = 1;
  s.at<std::uint32_t>(0x14) = 0;
  s.at<std::uint32_t>(0x18) = 2;
  host.tick_subnode_reparent_00876020(owner, self);

  s.ptr(0x1c) = host.image_address(0x00ce3cd4);
  s.ptr(0x20) = nullptr;
  s.ptr(0x24) = nullptr;
  s.ptr(0x28) = nullptr;
  s.at<std::uint8_t>(0x2c) = 0;

  const float neg_one = host.global_f32(0x00d7a260);
  s.ptr(0x00) = host.image_address(0x00cfda40);
  s.ptr(0x1c) = host.image_address(0x00cfda28);
  s.at<std::uint32_t>(0x30) = 0;
  s.ptr(0x34) = own.ptr(0x28);
  s.at<std::uint32_t>(0x38) = 0;
  s.at<std::uint8_t>(0x3c) = 0;
  s.at<std::uint8_t>(0x3d) = 0;
  s.at<float>(0x40) = neg_one;
  s.at<std::uint8_t>(0x44) = 0;
  s.at<std::uint32_t>(0x48) = 0;
  s.at<std::uint8_t>(0x4c) = 0;
  s.at<std::uint32_t>(0x50) = 0;

  host.eh_vector_constructor_iterator_00bf7cd1(
      s.p + 0x54,
      0x1c,
      10,
      host.image_address(0x005f9a50),
      host.image_address(0x005f9640));

  s.at<std::uint32_t>(0x16c) = 0;
  s.at<float>(0x178) = host.global_f32(0x00f87574);
  s.at<float>(0x17c) = host.global_f32(0x00f87578);
  s.at<float>(0x180) = host.global_f32(0x00f8757c);
  s.at<std::uint16_t>(0x170) = 0;
  s.at<std::uint32_t>(0x174) = 0;
  s.at<std::uint8_t>(0x170) = 0;
  s.at<std::uint16_t>(0x172) = 0;
  s.at<float>(0x184) = 0.0f;

  s.at<std::uint32_t>(0x188) = 0;
  s.at<float>(0x194) = host.global_f32(0x00f87574);
  s.at<float>(0x198) = host.global_f32(0x00f87578);
  s.at<float>(0x19c) = host.global_f32(0x00f8757c);
  s.at<std::uint16_t>(0x18c) = 0;
  s.at<std::uint32_t>(0x190) = 0;
  s.at<std::uint8_t>(0x18c) = 0;
  s.at<std::uint16_t>(0x18e) = 0;
  s.at<float>(0x1a0) = 0.0f;

  s.at<std::uint32_t>(0x1cc) = 0x1ff;
  s.at<std::uint8_t>(0x220) = 1;
  s.at<std::uint8_t>(0x221) = 1;
  s.at<std::uint8_t>(0x222) = 1;
  s.at<std::uint8_t>(0x223) = 1;

  void* tick_parent = s.ptr(0x04);
  s.at<std::uint32_t>(0x18) = 1;
  if (tick_parent != nullptr) {
    host.tick_subnode_requeue_00876120(tick_parent, self);
  }

  for (std::size_t i = 0; i < 20; ++i) {
    s.at<std::uint32_t>(0x1d0 + i * 4) = 0xd01502f9u;
  }

  for (std::size_t slot = 0; slot < 10; ++slot) {
    void* mem = host.operator_new_00bf681b(0x50);
    if (mem != nullptr) {
      Raw po = raw_of(mem);
      const float path_neg_one = host.global_f32(0x00d7a260);
      po.ptr(0x00) = host.image_address(0x00cfdb24);
      po.ptr(0x04) = self;
      po.at<std::uint32_t>(0x08) = 1;
      po.at<std::uint32_t>(0x0c) = 5;
      po.ptr(0x14) = nullptr;
      po.at<std::uint32_t>(0x18) = 0;
      po.at<std::uint32_t>(0x1c) = 1;
      po.at<std::uint8_t>(0x20) = 1;
      po.ptr(0x10) = host.image_address(0x00cfdb10);
      po.ptr(0x24) = self;
      po.at<std::uint8_t>(0x28) = 0;
      po.at<std::uint32_t>(0x2c) = 0xffffffffu;
      po.at<float>(0x3c) = path_neg_one;
      po.ptr(0x44) = nullptr;
      po.ptr(0x48) = nullptr;
      po.ptr(0x4c) = nullptr;
    }
    s.ptr(0x1a4 + slot * 4) = mem;
  }
  return self;
}

// 00720d80: TRIV_body_00720d80
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x3c], RET 0.
std::uint8_t triv_body_00720d80(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x3c);
}

// 00720d90: TRIV_body_00720d90
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x3d], RET 0.
std::uint8_t triv_body_00720d90(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x3d);
}

// 00720da0: TRIV_body_00720da0
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x220], RET 0.
std::uint8_t triv_body_00720da0(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x220);
}

// 00720db0: TRIV_body_00720db0
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x221], RET 0.
std::uint8_t triv_body_00720db0(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x221);
}

// 00720dc0: TRIV_body_00720dc0
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x222], RET 0.
std::uint8_t triv_body_00720dc0(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x222);
}

// 00720dd0: TRIV_body_00720dd0
// Original ABI: __fastcall(ECX=self) -> AL=[self+0x223], RET 0.
std::uint8_t triv_body_00720dd0(const void* self) {
  return raw_of(self).at<std::uint8_t>(0x223);
}

// 00720f50: TRIV_body_00720f50
// Original ABI: __thiscall(ECX=self, stack arg1), RET 4.
void triv_body_00720f50(void* /*self*/, std::uint32_t /*arg1*/) {}

// 00721030: BSP_GameUnitMessage_ReadTargetDescriptor
// Original ABI: __thiscall(ECX=self, stack out_target) -> EAX=out_target, RET 4.
void* bsp_game_unit_message_read_target_descriptor_00721030(const void* self, void* out_target) {
  Raw src = raw_of(self);
  Raw dst = raw_of(out_target);
  dst.at<std::uint16_t>(0x00) = src.at<std::uint16_t>(0x24);
  dst.at<std::uint16_t>(0x02) = src.at<std::uint16_t>(0x26);
  dst.at<std::uint32_t>(0x04) = src.at<std::uint32_t>(0x28);
  dst.at<float>(0x08) = src.at<float>(0x2c);
  dst.at<float>(0x0c) = src.at<float>(0x30);
  dst.at<float>(0x10) = src.at<float>(0x34);
  dst.at<float>(0x14) = src.at<float>(0x38);
  return out_target;
}

// 00721cb0: BSP_GeomMeshResourceItem_GetRuntimeTypeToken
// Original ABI: __thiscall(ECX=self) -> EAX=[00e19be4], RET 0.
std::uint32_t bsp_geom_mesh_resource_item_get_runtime_type_token_00721cb0(Seg38_39Host& host) {
  return host.global_u32(0x00e19be4);
}

// 00721cc0: BSP_GeomMeshResourceItem_GetRuntimeTypeNameAddress
// Original ABI: __thiscall(ECX=self) -> EAX=[00e19bf0], RET 0.
std::uint32_t bsp_geom_mesh_resource_item_get_runtime_type_name_address_00721cc0(Seg38_39Host& host) {
  return host.global_u32(0x00e19bf0);
}

// 00722210: BSP_GeomMesh_RadixSortKey0
// Original ABI: __fastcall(ECX=records, EDX=count, stack scratch), RET 4.
// Four-pass byte radix sort of 16-byte records keyed by the uint32 at offset +0x00.
void bsp_geom_mesh_radix_sort_key0_00722210(
    std::uint32_t* records,
    std::int32_t count,
    std::uint32_t* scratch) {
  std::int32_t hist[1024];
  std::memset(hist, 0, sizeof(hist));

  for (std::int32_t i = 0; i < count; ++i) {
    const std::uint32_t key = records[i * 4];
    hist[(key & 0xffu) * 4 + 0] += 1;
    hist[((key >> 8) & 0xffu) * 4 + 1] += 1;
    hist[((key >> 16) & 0xffu) * 4 + 2] += 1;
    hist[(key >> 24) * 4 + 3] += 1;
  }

  std::int32_t sum0 = 0;
  std::int32_t sum1 = 0;
  std::int32_t sum2 = 0;
  std::int32_t sum3 = 0;
  for (std::int32_t b = 0; b < 256; ++b) {
    const std::int32_t c0 = hist[b * 4 + 0];
    hist[b * 4 + 0] = sum0;
    sum0 += c0;

    const std::int32_t c1 = hist[b * 4 + 1];
    hist[b * 4 + 1] = sum1;
    sum1 += c1;

    const std::int32_t c2 = hist[b * 4 + 2];
    hist[b * 4 + 2] = sum2;
    sum2 += c2;

    const std::int32_t c3 = hist[b * 4 + 3];
    hist[b * 4 + 3] = sum3;
    sum3 += c3;
  }

  if (count > 0) {
    for (std::int32_t i = 0; i < count; ++i) {
      const std::uint32_t* src = records + i * 4;
      const std::uint32_t bucket = (src[0] & 0xffu) * 4 + 0;
      const std::int32_t dst_idx = hist[bucket];
      hist[bucket] = dst_idx + 1;
      std::uint32_t* dst = scratch + dst_idx * 4;
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
    for (std::int32_t i = 0; i < count; ++i) {
      const std::uint32_t* src = scratch + i * 4;
      const std::uint32_t bucket = ((src[0] >> 8) & 0xffu) * 4 + 1;
      const std::int32_t dst_idx = hist[bucket];
      hist[bucket] = dst_idx + 1;
      std::uint32_t* dst = records + dst_idx * 4;
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
    for (std::int32_t i = 0; i < count; ++i) {
      const std::uint32_t* src = records + i * 4;
      const std::uint32_t bucket = ((src[0] >> 16) & 0xffu) * 4 + 2;
      const std::int32_t dst_idx = hist[bucket];
      hist[bucket] = dst_idx + 1;
      std::uint32_t* dst = scratch + dst_idx * 4;
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
    for (std::int32_t i = 0; i < count; ++i) {
      const std::uint32_t* src = scratch + i * 4;
      const std::uint32_t bucket = (src[0] >> 24) * 4 + 3;
      const std::int32_t dst_idx = hist[bucket];
      hist[bucket] = dst_idx + 1;
      std::uint32_t* dst = records + dst_idx * 4;
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
  }
}

// 00723030: BSP_GeomMeshElement_RootBoxCentre
// Original ABI: __thiscall(ECX=self, stack out_centre), RET 4.
// Note: 0072309b is `dc c9 fmul st(1), st(0)`, multiplying the X sum in st(1) by 0.5 ([00d7a280]
// in st(0)) while keeping 0.5 in st(0) for the Y and Z multiplies at 007230a5 and 007230aa.
void bsp_geom_mesh_element_root_box_centre_00723030(
    Seg38_39Host& host,
    const void* self,
    float* out_centre) {
  Raw s = raw_of(self);
  if (s.at<std::int32_t>(0x28) > 0) {
    const float* min_box = static_cast<const float*>(s.ptr(0x20));
    const float min_x = min_box[0];
    const float min_z = min_box[2];
    const float min_y = min_box[1];
    const float* max_box = static_cast<const float*>(s.ptr(0x24));
    const float max_x = max_box[0];
    const float max_y = max_box[1];
    const float max_z = max_box[2];
    const float sum_x = max_x + min_x;
    const float sum_y = max_y + min_y;
    const float sum_z = max_z + min_z;
    const double half = host.global_f64(0x00d7a280);
    out_centre[0] = static_cast<float>(static_cast<double>(sum_x) * half);
    out_centre[1] = static_cast<float>(static_cast<double>(sum_y) * half);
    out_centre[2] = static_cast<float>(half * static_cast<double>(sum_z));
    return;
  }
  out_centre[0] = host.global_f32(0x00f87574);
  out_centre[1] = host.global_f32(0x00f87578);
  out_centre[2] = host.global_f32(0x00f8757c);
}

// 007230f0: BSP_GeomMesh_RadixSortByAxis
// Original ABI: __stdcall(stack axis, stack start, stack count), RET 0Ch.
void bsp_geom_mesh_radix_sort_by_axis_007230f0(
    Seg38_39Host& host,
    std::int32_t axis,
    std::int32_t start,
    std::int32_t count) {
  std::uint32_t* base = reinterpret_cast<std::uint32_t*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e19bd8)));
  std::uint32_t* scratch = reinterpret_cast<std::uint32_t*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00e19bdc)));
  std::uint32_t* records = base + static_cast<std::ptrdiff_t>(start) * 4;
  if (axis == 0) {
    bsp_geom_mesh_radix_sort_key0_00722210(records, count, scratch);
  } else if (axis == 1) {
    host.geom_mesh_radix_sort_key4_00722400(records, count, scratch);
  } else if (axis == 2) {
    host.geom_mesh_radix_sort_key8_007225f0(records, count, scratch);
  }
}

// 007238e0: BSP_GeomMeshElement_TraceSegmentNode
// Original ABI: __thiscall(ECX=self, stack query_record, stack node_index, stack first_ordinal,
// stack ordinal_count) -> AL=bool, RET 10h.
std::uint8_t bsp_geom_mesh_element_trace_segment_node_007238e0(
    Seg38_39Host& host,
    const void* self,
    void* query_record,
    std::int32_t node_index,
    std::uint32_t first_ordinal,
    std::int32_t ordinal_count) {
  Raw s = raw_of(self);
  Raw q = raw_of(query_record);
  const float* box_min =
      reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(s.ptr(0x20)) + node_index * 12);
  const float* box_max =
      reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(s.ptr(0x24)) + node_index * 12);
  const float* seg_from = &q.at<float>(0x18);
  float* seg_to = &q.at<float>(0x24);
  if (!host.geometry_segment_overlaps_aabb_0085cad0(box_min, box_max, seg_from, seg_to)) {
    return 0;
  }

  if (ordinal_count > 0x14) {
    const std::int32_t half = ordinal_count >> 1;
    const std::uint8_t left_hit = bsp_geom_mesh_element_trace_segment_node_007238e0(
        host, self, query_record, node_index * 2 + 1, first_ordinal, half);
    const std::uint8_t right_hit = bsp_geom_mesh_element_trace_segment_node_007238e0(
        host,
        self,
        query_record,
        node_index * 2 + 2,
        first_ordinal + static_cast<std::uint32_t>(half),
        ordinal_count - half);
    return (left_hit != 0 || right_hit != 0) ? 1 : 0;
  }

  Raw mesh = raw_of(s.ptr(0x0c));
  const std::uint8_t* tris_begin = static_cast<const std::uint8_t*>(mesh.ptr(0x2c));
  const std::uint8_t* tris_end = static_cast<const std::uint8_t*>(mesh.ptr(0x30));
  if (tris_begin == nullptr || (tris_end - tris_begin) / 6 == 0) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  tris_begin = static_cast<const std::uint8_t*>(mesh.ptr(0x2c));

  const std::uint8_t* ord_begin = static_cast<const std::uint8_t*>(s.ptr(0x14));
  const std::uint8_t* ord_end = static_cast<const std::uint8_t*>(s.ptr(0x18));
  if (ord_begin == nullptr ||
      static_cast<std::uint32_t>((ord_end - ord_begin) >> 1) <= first_ordinal) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  const std::uint16_t* ordinals =
      reinterpret_cast<const std::uint16_t*>(s.ptr(0x14)) + first_ordinal;

  Raw mesh_v = raw_of(s.ptr(0x0c));
  const std::uint8_t* verts_begin = static_cast<const std::uint8_t*>(mesh_v.ptr(0x1c));
  const std::uint8_t* verts_end = static_cast<const std::uint8_t*>(mesh_v.ptr(0x20));
  if (verts_begin == nullptr || (verts_end - verts_begin) / 12 == 0) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  verts_begin = static_cast<const std::uint8_t*>(mesh_v.ptr(0x1c));

  std::uint8_t any_hit = 0;
  float* out_hit = &q.at<float>(0x30);
  for (std::int32_t i = 0; i < ordinal_count; ++i) {
    const std::uint16_t tri_idx = ordinals[i];
    const std::int16_t* tri =
        reinterpret_cast<const std::int16_t*>(tris_begin + static_cast<std::size_t>(tri_idx) * 6);
    const float* v0 = reinterpret_cast<const float*>(verts_begin + static_cast<std::ptrdiff_t>(tri[0]) * 12);
    const float* v1 = reinterpret_cast<const float*>(verts_begin + static_cast<std::ptrdiff_t>(tri[1]) * 12);
    const float* v2 = reinterpret_cast<const float*>(verts_begin + static_cast<std::ptrdiff_t>(tri[2]) * 12);
    if (host.geometry_segment_intersect_triangle_0085d020(seg_from, seg_to, v0, v1, v2, out_hit)) {
      any_hit = 1;
      seg_to[0] = out_hit[0];
      seg_to[1] = out_hit[1];
      seg_to[2] = out_hit[2];
    }
  }
  return any_hit;
}

// 00723aa0: BSP_GeomMeshElement_TraceSegment
// Original ABI: __thiscall(ECX=self, stack from, stack to, stack out_hit) -> AL=bool, RET 0Ch.
std::uint8_t bsp_geom_mesh_element_trace_segment_00723aa0(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    float* out_hit) {
  alignas(4) std::uint8_t query_buf[0x3c];
  Raw q{query_buf};
  q.at<float>(0x18) = from[0];
  q.at<float>(0x1c) = from[1];
  q.at<float>(0x20) = from[2];
  q.at<float>(0x24) = to[0];
  q.at<float>(0x28) = to[1];
  q.at<float>(0x2c) = to[2];
  host.vector3f_min_max_00722b20(&q.at<float>(0x18), &q.at<float>(0x24), &q.at<float>(0x00), &q.at<float>(0x0c));

  Raw s = raw_of(self);
  std::int32_t ordinal_count = 0;
  const std::uint8_t* ord_begin = static_cast<const std::uint8_t*>(s.ptr(0x14));
  if (ord_begin != nullptr) {
    const std::uint8_t* ord_end = static_cast<const std::uint8_t*>(s.ptr(0x18));
    ordinal_count = static_cast<std::int32_t>((ord_end - ord_begin) >> 1);
  }
  if (bsp_geom_mesh_element_trace_segment_node_007238e0(host, self, query_buf, 0, 0, ordinal_count) != 0) {
    out_hit[0] = q.at<float>(0x30);
    out_hit[1] = q.at<float>(0x34);
    out_hit[2] = q.at<float>(0x38);
    return 1;
  }
  return 0;
}

// 00723b70: BSP_GeomMeshElement_TestSphere
// Original ABI: __thiscall(ECX=self, stack centre, stack radius, stack out_distance) -> AL=bool, RET 0Ch.
std::uint8_t bsp_geom_mesh_element_test_sphere_00723b70(
    Seg38_39Host& host,
    const void* self,
    const float* centre,
    const float* radius,
    float* out_distance) {
  Raw s = raw_of(self);
  float best_dist_sq = (*radius) * (*radius);
  const float aabb_dist_sq = host.aabb_point_distance_squared_0085bf90(
      centre,
      static_cast<const float*>(s.ptr(0x20)),
      static_cast<const float*>(s.ptr(0x24)));
  if (aabb_dist_sq > best_dist_sq) {
    return 0;
  }

  std::int32_t ordinal_count = 0;
  if (s.ptr(0x14) != nullptr) {
    ordinal_count = static_cast<std::int32_t>(
        (static_cast<const std::uint8_t*>(s.ptr(0x18)) - static_cast<const std::uint8_t*>(s.ptr(0x14))) >> 1);
  }
  if (ordinal_count <= 0) {
    return 0;
  }

  bool any_hit = false;
  for (std::int32_t i = 0; i < ordinal_count; ++i) {
    const std::uint8_t* ord_begin = static_cast<const std::uint8_t*>(s.ptr(0x14));
    const std::uint8_t* ord_end = static_cast<const std::uint8_t*>(s.ptr(0x18));
    if (ord_begin == nullptr ||
        static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((ord_end - ord_begin) >> 1)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::uint16_t tri_idx = reinterpret_cast<const std::uint16_t*>(s.ptr(0x14))[i];

    Raw mesh0 = raw_of(s.ptr(0x0c));
    const std::uint8_t* tris_begin = static_cast<const std::uint8_t*>(mesh0.ptr(0x2c));
    const std::uint8_t* tris_end = static_cast<const std::uint8_t*>(mesh0.ptr(0x30));
    if (tris_begin == nullptr ||
        static_cast<std::uint32_t>(tri_idx) >= static_cast<std::uint32_t>((tris_end - tris_begin) / 6)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::int16_t* tri =
        reinterpret_cast<const std::int16_t*>(static_cast<const std::uint8_t*>(mesh0.ptr(0x2c)) +
                                              static_cast<std::size_t>(tri_idx) * 6);

    const std::int32_t v0_idx = tri[0];
    Raw mesh1 = raw_of(s.ptr(0x0c));
    const std::uint8_t* v_begin1 = static_cast<const std::uint8_t*>(mesh1.ptr(0x1c));
    const std::uint8_t* v_end1 = static_cast<const std::uint8_t*>(mesh1.ptr(0x20));
    if (v_begin1 == nullptr ||
        static_cast<std::uint32_t>(v0_idx) >= static_cast<std::uint32_t>((v_end1 - v_begin1) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* v0 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(mesh1.ptr(0x1c)) + static_cast<std::ptrdiff_t>(v0_idx) * 12);

    const std::int32_t v1_idx = tri[1];
    Raw mesh2 = raw_of(s.ptr(0x0c));
    const std::uint8_t* v_begin2 = static_cast<const std::uint8_t*>(mesh2.ptr(0x1c));
    const std::uint8_t* v_end2 = static_cast<const std::uint8_t*>(mesh2.ptr(0x20));
    if (v_begin2 == nullptr ||
        static_cast<std::uint32_t>(v1_idx) >= static_cast<std::uint32_t>((v_end2 - v_begin2) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* v1 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(mesh2.ptr(0x1c)) + static_cast<std::ptrdiff_t>(v1_idx) * 12);

    const std::int32_t v2_idx = tri[2];
    Raw mesh3 = raw_of(s.ptr(0x0c));
    const std::uint8_t* v_begin3 = static_cast<const std::uint8_t*>(mesh3.ptr(0x1c));
    const std::uint8_t* v_end3 = static_cast<const std::uint8_t*>(mesh3.ptr(0x20));
    if (v_begin3 == nullptr ||
        static_cast<std::uint32_t>(v2_idx) >= static_cast<std::uint32_t>((v_end3 - v_begin3) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* v2 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(mesh3.ptr(0x1c)) + static_cast<std::ptrdiff_t>(v2_idx) * 12);

    const float dist_sq = host.geometry_sphere_triangle_distance_0085df60(centre, v0, v1, v2);
    if (dist_sq < best_dist_sq) {
      any_hit = true;
      best_dist_sq = dist_sq;
    }
  }

  if (any_hit) {
    *out_distance = static_cast<float>(host.libcrt_sqrt_00bf7030(best_dist_sq));
    return 1;
  }
  return 0;
}

// 00723d60: BSP_GeomMesh_TraceSegmentClosestElement
// Original ABI: __thiscall(ECX=self, stack from, stack to, stack out_hit) -> EAX=closest_elem, RET 0Ch.
const void* bsp_geom_mesh_trace_segment_closest_element_00723d60(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    float* out_hit) {
  float current_to[3] = {to[0], to[1], to[2]};
  const void* closest_elem = nullptr;
  Raw s = raw_of(self);
  std::int32_t elem_count = 0;
  if (s.ptr(0x0c) != nullptr) {
    elem_count = static_cast<std::int32_t>(
        (static_cast<const std::uint8_t*>(s.ptr(0x10)) - static_cast<const std::uint8_t*>(s.ptr(0x0c))) / 0x2c);
  }
  if (elem_count > 0) {
    std::ptrdiff_t byte_off = 0;
    for (std::int32_t i = 0; i < elem_count; ++i, byte_off += 0x2c) {
      const std::uint8_t* begin0 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
      const std::uint8_t* end0 = static_cast<const std::uint8_t*>(s.ptr(0x10));
      if (begin0 == nullptr ||
          static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((end0 - begin0) / 0x2c)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      const std::uint8_t* elem_ptr = static_cast<const std::uint8_t*>(s.ptr(0x0c)) + byte_off;
      if (bsp_geom_mesh_element_trace_segment_00723aa0(host, elem_ptr, from, current_to, out_hit) != 0) {
        const std::uint8_t* begin1 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
        const std::uint8_t* end1 = static_cast<const std::uint8_t*>(s.ptr(0x10));
        if (begin1 == nullptr ||
            static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((end1 - begin1) / 0x2c)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        current_to[0] = out_hit[0];
        current_to[1] = out_hit[1];
        closest_elem = static_cast<const std::uint8_t*>(s.ptr(0x0c)) + byte_off;
        current_to[2] = out_hit[2];
      }
    }
    return closest_elem;
  }
  return nullptr;
}

// 00723e90: BSP_GeomMesh_TraceSegmentIntoHitRecord
// Original ABI: __thiscall(ECX=self, stack to_world, stack to_local, stack from, stack to, stack record)
// -> AL=bool, RET 14h.
std::uint8_t bsp_geom_mesh_trace_segment_into_hit_record_00723e90(
    Seg38_39Host& host,
    const void* self,
    const void* to_world,
    const void* to_local,
    const float* from,
    const float* to,
    void* record) {
  float scratch[3];
  host.vector3f_transform_affine_point_004142e0(to, scratch, to_local);
  float local_to[3] = {scratch[0], scratch[1], scratch[2]};
  host.vector3f_transform_affine_point_004142e0(from, scratch, to_local);
  float local_from[3] = {scratch[0], scratch[1], scratch[2]};

  const void* hit_elem =
      bsp_geom_mesh_trace_segment_closest_element_00723d60(host, self, local_from, local_to, scratch);
  if (hit_elem == nullptr) {
    return 0;
  }

  float world_hit[3];
  host.vector3f_transform_affine_point_004142e0(scratch, world_hit, to_world);
  Raw rec = raw_of(record);
  Raw elem = raw_of(hit_elem);
  rec.at<float>(0x08) = world_hit[0];
  rec.at<float>(0x0c) = world_hit[1];
  rec.at<float>(0x10) = world_hit[2];
  rec.at<std::uint32_t>(0x30) = elem.at<std::uint32_t>(0x04);
  const std::uint32_t hull_seg = elem.at<std::uint32_t>(0x08);
  rec.ptr(0x38) = const_cast<void*>(hit_elem);
  rec.at<std::uint32_t>(0x34) = hull_seg;
  return 1;
}

// 00723f80: BSP_GeomMesh_CollectSphereHits
// Original ABI: __thiscall(ECX=self, stack body_f0, stack to_local, stack centre, stack radius,
// stack record) -> AL=bool, RET 14h.
std::uint8_t bsp_geom_mesh_collect_sphere_hits_00723f80(
    Seg38_39Host& host,
    const void* self,
    const void* /*body_f0*/,
    const void* to_local,
    const float* centre,
    float radius,
    void* record) {
  std::uint8_t any_hit = 0;
  float transformed[3];
  host.vector3f_transform_affine_point_004142e0(centre, transformed, to_local);
  float local_centre[3] = {transformed[0], transformed[1], transformed[2]};

  Raw s = raw_of(self);
  std::int32_t elem_count = 0;
  if (s.ptr(0x0c) != nullptr) {
    elem_count = static_cast<std::int32_t>(
        (static_cast<const std::uint8_t*>(s.ptr(0x10)) - static_cast<const std::uint8_t*>(s.ptr(0x0c))) / 0x2c);
  }
  if (elem_count > 0) {
    std::ptrdiff_t byte_off = 0;
    for (std::int32_t i = 0; i < elem_count; ++i, byte_off += 0x2c) {
      const std::uint8_t* begin0 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
      const std::uint8_t* end0 = static_cast<const std::uint8_t*>(s.ptr(0x10));
      if (begin0 == nullptr ||
          static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((end0 - begin0) / 0x2c)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      const std::uint8_t* elem_ptr = static_cast<const std::uint8_t*>(s.ptr(0x0c)) + byte_off;
      float out_dist = 0.0f;
      if (bsp_geom_mesh_element_test_sphere_00723b70(host, elem_ptr, local_centre, &radius, &out_dist) != 0) {
        alignas(4) std::uint8_t entry_buf[0x10];
        Raw entry{entry_buf};
        entry.at<std::uint32_t>(0x08) = 0;
        entry.at<float>(0x0c) = out_dist;

        const std::uint8_t* begin1 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
        const std::uint8_t* end1 = static_cast<const std::uint8_t*>(s.ptr(0x10));
        if (begin1 == nullptr ||
            static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((end1 - begin1) / 0x2c)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        const std::uint8_t* base1 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
        entry.at<std::uint32_t>(0x00) = raw_of(base1 + byte_off).at<std::uint32_t>(0x04);

        const std::uint8_t* end2 = static_cast<const std::uint8_t*>(s.ptr(0x10));
        if (base1 == nullptr ||
            static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>((end2 - base1) / 0x2c)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        const std::uint8_t* base2 = static_cast<const std::uint8_t*>(s.ptr(0x0c));
        entry.at<std::uint32_t>(0x04) = raw_of(base2 + byte_off).at<std::uint32_t>(0x08);

        host.hit_record_append_part_hit_006d2e30(record, entry_buf);
        any_hit = 1;
      }
    }
  }
  return any_hit;
}

// 00724510: BSP_UnitPartCollisionShape_TraceSegment
// Original ABI: __thiscall(ECX=self, stack from, stack to, stack record) -> AL=bool, RET 0Ch.
bool bsp_unit_part_collision_shape_trace_segment_00724510(
    Seg38_39Host& host,
    const void* self,
    const float* from,
    const float* to,
    void* record) {
  Raw s = raw_of(self);
  const std::uint8_t* node = static_cast<const std::uint8_t*>(s.ptr(0x1c));
  const void* mesh = s.ptr(0x24);
  return bsp_geom_mesh_trace_segment_into_hit_record_00723e90(
             host, mesh, node + 0x50, node + 0x90, from, to, record) != 0;
}

// 00724a20: BSP_GeomMesh_DebugDrawWireframe
// Original ABI: __thiscall(ECX=self, stack matrix, stack rgba), RET 8.
void bsp_geom_mesh_debug_draw_wireframe_00724a20(
    Seg38_39Host& host,
    const void* self,
    const void* matrix,
    const float* rgba) {
  Raw s = raw_of(self);
  const std::int16_t* tri_cur = static_cast<const std::int16_t*>(s.ptr(0x2c));
  if (static_cast<const std::int16_t*>(s.ptr(0x30)) < tri_cur) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  for (;;) {
    const std::int16_t* tri_end = static_cast<const std::int16_t*>(s.ptr(0x30));
    if (tri_end < static_cast<const std::int16_t*>(s.ptr(0x2c))) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (tri_cur == tri_end) {
      break;
    }
    if (static_cast<const std::int16_t*>(s.ptr(0x30)) <= tri_cur) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::int32_t idx0 = tri_cur[0];
    const std::uint8_t* v_begin0 = static_cast<const std::uint8_t*>(s.ptr(0x1c));
    const std::uint8_t* v_end0 = static_cast<const std::uint8_t*>(s.ptr(0x20));
    if (v_begin0 == nullptr ||
        static_cast<std::uint32_t>(idx0) >= static_cast<std::uint32_t>((v_end0 - v_begin0) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* src0 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(s.ptr(0x1c)) + static_cast<std::ptrdiff_t>(idx0) * 12);
    float v0[3] = {src0[0], src0[1], src0[2]};

    if (static_cast<const std::int16_t*>(s.ptr(0x30)) <= tri_cur) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::int32_t idx1 = tri_cur[1];
    const std::uint8_t* v_begin1 = static_cast<const std::uint8_t*>(s.ptr(0x1c));
    const std::uint8_t* v_end1 = static_cast<const std::uint8_t*>(s.ptr(0x20));
    if (v_begin1 == nullptr ||
        static_cast<std::uint32_t>(idx1) >= static_cast<std::uint32_t>((v_end1 - v_begin1) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* src1 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(s.ptr(0x1c)) + static_cast<std::ptrdiff_t>(idx1) * 12);
    float v1[3] = {src1[0], src1[1], src1[2]};

    if (static_cast<const std::int16_t*>(s.ptr(0x30)) <= tri_cur) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const std::int32_t idx2 = tri_cur[2];
    const std::uint8_t* v_begin2 = static_cast<const std::uint8_t*>(s.ptr(0x1c));
    const std::uint8_t* v_end2 = static_cast<const std::uint8_t*>(s.ptr(0x20));
    if (v_begin2 == nullptr ||
        static_cast<std::uint32_t>(idx2) >= static_cast<std::uint32_t>((v_end2 - v_begin2) / 12)) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    const float* src2 = reinterpret_cast<const float*>(
        static_cast<const std::uint8_t*>(s.ptr(0x1c)) + static_cast<std::ptrdiff_t>(idx2) * 12);
    float v2[3] = {src2[0], src2[1], src2[2]};

    float out_a[3];
    float out_b[3];
    float out_c[3];
    float out_d[3];
    float out_e[3];
    float out_f[3];

    const float* p1_e1 = host.vector3f_transform_affine_point_004142e0(v1, out_a, matrix);
    const float* p0_e1 = host.vector3f_transform_affine_point_004142e0(v0, out_b, matrix);
    host.debug_draw_line_rgba_floats_0085f990(
        p0_e1, p1_e1, rgba[0], rgba[1], rgba[2], rgba[3], rgba[0], rgba[1], rgba[2], rgba[3]);

    const float* p1_e2 = host.vector3f_transform_affine_point_004142e0(v2, out_c, matrix);
    const float* p0_e2 = host.vector3f_transform_affine_point_004142e0(v1, out_d, matrix);
    host.debug_draw_line_rgba_floats_0085f990(
        p0_e2, p1_e2, rgba[0], rgba[1], rgba[2], rgba[3], rgba[0], rgba[1], rgba[2], rgba[3]);

    const float* p1_e3 = host.vector3f_transform_affine_point_004142e0(v2, out_e, matrix);
    const float* p0_e3 = host.vector3f_transform_affine_point_004142e0(v0, out_f, matrix);
    host.debug_draw_line_rgba_floats_0085f990(
        p0_e3, p1_e3, rgba[0], rgba[1], rgba[2], rgba[3], rgba[0], rgba[1], rgba[2], rgba[3]);

    if (static_cast<const std::int16_t*>(s.ptr(0x30)) <= tri_cur) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    tri_cur += 3;
  }
}

// 00725550: BSP_GeomMeshComponent_DebugDraw
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_geom_mesh_component_debug_draw_00725550(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* owner = s.ptr(0x04);
  Raw own = raw_of(owner);
  if ((own.at<std::uint8_t>(0x5c) & 0x02u) == 0) {
    host.transform_refresh_world_matrix_00b6db70(owner);
  }
  bsp_geom_mesh_debug_draw_wireframe_00724a20(
      host, s.ptr(0x08), own.p + 0xf0, reinterpret_cast<const float*>(s.p + 0x0c));
}

// 00725830: BSP_GeomMeshElement_Destruct
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_geom_mesh_element_destruct_00725830(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00cfdbc8);
  if (s.ptr(0x20) != nullptr) {
    host.free_00bf6989(s.ptr(0x20));
  }
  if (s.ptr(0x24) != nullptr) {
    host.free_00bf6989(s.ptr(0x24));
  }
  if (s.ptr(0x14) != nullptr) {
    host.free_00bf65ac(s.ptr(0x14));
  }
  s.ptr(0x14) = nullptr;
  s.ptr(0x18) = nullptr;
  s.ptr(0x1c) = nullptr;
}

// 00726710: BSP_GeomMeshElement_CopyConstruct
// Original ABI: __thiscall(ECX=self, stack other) -> EAX=self, RET 4.
void* bsp_geom_mesh_element_copy_construct_00726710(Seg38_39Host& host, void* self, const void* other) {
  Raw dst = raw_of(self);
  Raw src = raw_of(other);
  dst.ptr(0x00) = host.image_address(0x00cfdbc8);
  dst.at<std::uint32_t>(0x04) = src.at<std::uint32_t>(0x04);
  dst.at<std::uint32_t>(0x08) = src.at<std::uint32_t>(0x08);
  dst.ptr(0x0c) = src.ptr(0x0c);
  host.stl_vector_u16_copy_construct_0058aac0(dst.p + 0x10, src.p + 0x10);
  dst.ptr(0x20) = src.ptr(0x20);
  dst.ptr(0x24) = src.ptr(0x24);
  dst.at<std::uint32_t>(0x28) = src.at<std::uint32_t>(0x28);
  return self;
}

// 007268d0: BSP_GeomMeshResourceItem_Construct
// Original ABI: __fastcall(ECX=self) -> EAX=self, RET 0.
void* bsp_geom_mesh_resource_item_construct_007268d0(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  host.resource_item_construct_reference_base_00b868b0(self);
  s.ptr(0x00) = host.image_address(0x00cfdbd8);
  s.ptr(0x0c) = nullptr;
  s.ptr(0x10) = nullptr;
  s.ptr(0x14) = nullptr;
  s.ptr(0x1c) = nullptr;
  s.ptr(0x20) = nullptr;
  s.ptr(0x24) = nullptr;
  s.ptr(0x2c) = nullptr;
  s.ptr(0x30) = nullptr;
  s.ptr(0x34) = nullptr;
  return self;
}

// 00726930: BSP_GeomMeshResourceItem_AcceptsRuntimeTypeToken
// Original ABI: __thiscall(ECX=self, stack token) -> AL=bool, RET 4.
// Walks the 3-dword token chain [00e19be4, 00e19bf0).
bool bsp_geom_mesh_resource_item_accepts_runtime_type_token_00726930(
    Seg38_39Host& host,
    std::uint32_t token) {
  for (std::uint32_t addr = 0x00e19be4u; addr != 0x00e19bf0u; addr += 4u) {
    if (host.global_u32(addr) == token) {
      return true;
    }
  }
  return false;
}

// 00726960: BSP_GeomMeshResourceItem_Destruct
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_geom_mesh_resource_item_destruct_00726960(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  if (s.ptr(0x2c) != nullptr) {
    host.free_00bf65ac(s.ptr(0x2c));
  }
  s.ptr(0x2c) = nullptr;
  s.ptr(0x30) = nullptr;
  s.ptr(0x34) = nullptr;
  if (s.ptr(0x1c) != nullptr) {
    host.free_00bf65ac(s.ptr(0x1c));
  }
  s.ptr(0x1c) = nullptr;
  s.ptr(0x20) = nullptr;
  s.ptr(0x24) = nullptr;
  host.stl_vector_tidy_geom_mesh_element_007256a0(s.p + 0x08);
  host.resource_item_destroy_reference_base_00b86890(self);
}

// 00726d80: BSP_GeomMeshElementVector_Reserve
// Original ABI: __thiscall(ECX=self, stack new_capacity), RET 4.
void bsp_geom_mesh_element_vector_reserve_00726d80(
    Seg38_39Host& host,
    void* self,
    std::uint32_t new_capacity) {
  if (new_capacity > 0x05d1745du) {
    host.stl_xlen_throw_00725c50();
  }
  Raw s = raw_of(self);
  std::uint32_t cur_cap = 0;
  if (s.ptr(0x04) != nullptr) {
    cur_cap = static_cast<std::uint32_t>(
        (static_cast<std::uint8_t*>(s.ptr(0x0c)) - static_cast<std::uint8_t*>(s.ptr(0x04))) / 0x2c);
  }
  if (cur_cap < new_capacity) {
    std::uint8_t* new_buf = static_cast<std::uint8_t*>(host.stl_allocator_allocate_007227e0(new_capacity));
    std::uint8_t* old_end = static_cast<std::uint8_t*>(s.ptr(0x08));
    if (old_end < static_cast<std::uint8_t*>(s.ptr(0x04))) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    std::uint8_t* old_begin = static_cast<std::uint8_t*>(s.ptr(0x04));
    if (static_cast<std::uint8_t*>(s.ptr(0x08)) < old_begin) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    host.stl_uninit_copy_geom_mesh_elements_00726a80(old_begin, old_end, new_buf, self, new_capacity, 0);

    std::uint8_t* cur_begin = static_cast<std::uint8_t*>(s.ptr(0x04));
    std::ptrdiff_t cur_size = 0;
    if (cur_begin != nullptr) {
      cur_size = (static_cast<std::uint8_t*>(s.ptr(0x08)) - cur_begin) / 0x2c;
      host.stl_destroy_range_geom_mesh_element_00725670(self, cur_begin, s.ptr(0x08));
      host.free_00bf65ac(s.ptr(0x04));
    }
    s.ptr(0x0c) = new_buf + static_cast<std::size_t>(new_capacity) * 0x2c;
    s.ptr(0x08) = new_buf + cur_size * 0x2c;
    s.ptr(0x04) = new_buf;
  }
}

// 00727a90: BSP_GeomMeshResourceParser_Parse
// Original ABI: __thiscall(ECX=self, stack node_handle) -> EAX=mesh_item, RET 4.
void* bsp_geom_mesh_resource_parser_parse_00727a90(Seg38_39Host& host, void* /*self*/, void* node_handle) {
  void* raw_mem = host.operator_new_00bf681b(0x50);
  void* item = nullptr;
  if (raw_mem != nullptr) {
    item = bsp_geom_mesh_resource_item_construct_007268d0(host, raw_mem);
  }
  host.geom_mesh_resource_parser_parse_payload_00727310(item, node_handle);
  return item;
}

// 00727b20: BSP_HitRecordDBFC_Construct
// Original ABI: __thiscall(ECX=self, stack src_record) -> EAX=self, RET 4.
void* bsp_hit_record_dbfc_construct_00727b20(Seg38_39Host& host, void* self, const void* src_record) {
  Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00cfdbfc);
  host.hit_record_reset_00470470(s.p + 0x04);
  s.at<std::uint32_t>(0x58) = raw_of(src_record).at<std::uint32_t>(0x3c);
  return self;
}

// 00727b90: BSP_HitRecordDC04_Construct
// Original ABI: __thiscall(ECX=self, stack src_record) -> EAX=self, RET 4.
void* bsp_hit_record_dc04_construct_00727b90(Seg38_39Host& host, void* self, const void* src_record) {
  Raw s = raw_of(self);
  s.ptr(0x00) = host.image_address(0x00cfdbfc);
  host.hit_record_reset_00470470(s.p + 0x04);
  s.at<std::uint32_t>(0x58) = raw_of(src_record).at<std::uint32_t>(0x3c);
  s.ptr(0x00) = host.image_address(0x00cfdc04);
  return self;
}

// 00727c30: BSP_Gun_IndexOfReadiestBarrel
// Original ABI: __fastcall(ECX=self) -> EAX=index, RET 0.
std::int32_t bsp_gun_index_of_readiest_barrel_00727c30(const void* self) {
  Raw s = raw_of(self);
  std::int32_t best_idx = 0;
  std::int32_t i = 0;
  const std::int32_t barrel_count = s.at<std::int32_t>(0x448);
  if (barrel_count >= 4) {
    const float* timers = static_cast<const float*>(s.ptr(0x414));
    for (; i < barrel_count - 3; i += 4) {
      if (timers[i] < timers[best_idx]) {
        best_idx = i;
      }
      if (timers[i + 1] < timers[best_idx]) {
        best_idx = i + 1;
      }
      if (timers[i + 2] < timers[best_idx]) {
        best_idx = i + 2;
      }
      if (timers[i + 3] < timers[best_idx]) {
        best_idx = i + 3;
      }
    }
  }
  if (i < barrel_count) {
    const float* timers = static_cast<const float*>(s.ptr(0x414));
    for (; i < barrel_count; ++i) {
      if (timers[i] < timers[best_idx]) {
        best_idx = i;
      }
    }
  }
  return best_idx;
}

// 00727d70: BSP_Gun_CountReadyBarrels
// Original ABI: __thiscall(ECX=self, stack horizon) -> EAX=count, RET 4.
std::int32_t bsp_gun_count_ready_barrels_00727d70(const void* self, float horizon) {
  Raw s = raw_of(self);
  const std::int32_t barrel_count = s.at<std::int32_t>(0x448);
  std::int32_t ready_count = 0;
  std::int32_t i = 0;
  if (barrel_count >= 4) {
    const float* timers = static_cast<const float*>(s.ptr(0x414));
    const std::uint32_t blocks = (static_cast<std::uint32_t>(barrel_count - 4) >> 2) + 1u;
    i = static_cast<std::int32_t>(blocks * 4u);
    for (std::uint32_t b = 0; b < blocks; ++b) {
      const float* p = timers + b * 4u;
      if (p[0] <= horizon) {
        ready_count += 1;
      }
      if (p[1] <= horizon) {
        ready_count += 1;
      }
      if (p[2] <= horizon) {
        ready_count += 1;
      }
      if (p[3] <= horizon) {
        ready_count += 1;
      }
    }
  }
  if (i < barrel_count) {
    const float* timers = static_cast<const float*>(s.ptr(0x414));
    for (; i < barrel_count; ++i) {
      if (timers[i] <= horizon) {
        ready_count += 1;
      }
    }
  }
  return ready_count;
}

// 00727e10: BSP_Gun_SetPair43C
// Original ABI: __thiscall(ECX=self, stack val_43c, stack val_440), RET 8.
void bsp_gun_set_pair_43c_00727e10(void* self, std::uint32_t val_43c, std::uint32_t val_440) {
  Raw s = raw_of(self);
  s.at<std::uint32_t>(0x43c) = val_43c;
  s.at<std::uint32_t>(0x440) = val_440;
}

// 00727e70: BSP_Gun_SetBotSkill
// Original ABI: __thiscall(ECX=self, stack skill), RET 4.
void bsp_gun_set_bot_skill_00727e70(Seg38_39Host& host, void* self, std::uint32_t skill) {
  Raw s = raw_of(self);
  if (s.ptr(0x394) != nullptr) {
    host.vcall_18_u32(s.ptr(0x394), skill);
  }
  if (s.ptr(0x390) != nullptr) {
    host.vcall_18_u32(s.ptr(0x390), skill);
  }
  if (s.ptr(0x398) != nullptr) {
    host.vcall_18_u32(s.ptr(0x398), skill);
  }
  if (s.ptr(0x3a0) != nullptr) {
    host.vcall_18_u32(s.ptr(0x3a0), skill);
  }
  if (s.ptr(0x3a4) != nullptr) {
    host.vcall_18_u32(s.ptr(0x3a4), skill);
  }
  if (s.ptr(0x39c) != nullptr) {
    host.vcall_18_u32(s.ptr(0x39c), skill);
  }
}

// 00727f10: BSP_Gun_SetBotFireTarget
// Original ABI: __thiscall(ECX=self, stack target, stack flag), RET 8.
// Note: 00727f4e passes stack arg 2 (`flag`, at [ESP+0x10]) to vtable[0x3c] on bot +0x39c.
void bsp_gun_set_bot_fire_target_00727f10(
    Seg38_39Host& host,
    void* self,
    void* target,
    std::uint32_t flag) {
  Raw s = raw_of(self);
  if (s.ptr(0x390) != nullptr) {
    host.vcall_38_ptr(s.ptr(0x390), target);
  }
  if (s.ptr(0x39c) != nullptr) {
    host.vcall_38_ptr(s.ptr(0x39c), target);
    host.vcall_3c_u32(s.ptr(0x39c), flag);
  }
  if (s.ptr(0x3a0) != nullptr) {
    host.vcall_38_ptr(s.ptr(0x3a0), target);
  }
  if (s.ptr(0x3a4) != nullptr) {
    host.vcall_38_ptr(s.ptr(0x3a4), target);
  }
  if (s.ptr(0x398) != nullptr) {
    if (!host.vcall_5c_is_kind_of(target, 0x0f)) {
      host.vcall_38_ptr(s.ptr(0x398), target);
    } else {
      host.vcall_30(s.ptr(0x398));
    }
  }
  if (s.ptr(0x394) != nullptr) {
    if (!host.vcall_5c_is_kind_of(target, 0x0f) && !host.vcall_5c_is_kind_of(target, 0x0e)) {
      host.vcall_30(s.ptr(0x394));
      return;
    }
    host.vcall_38_ptr(s.ptr(0x394), target);
  }
}

// 00728000: BSP_Gun_ClearBotFireTarget
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_gun_clear_bot_fire_target_00728000(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  if (s.ptr(0x398) != nullptr) {
    host.vcall_30(s.ptr(0x398));
  }
  if (s.ptr(0x394) != nullptr) {
    host.vcall_30(s.ptr(0x394));
  }
  if (s.ptr(0x390) != nullptr) {
    host.vcall_30(s.ptr(0x390));
  }
  if (s.ptr(0x39c) != nullptr) {
    host.vcall_30(s.ptr(0x39c));
  }
  if (s.ptr(0x3a0) != nullptr) {
    host.vcall_30(s.ptr(0x3a0));
  }
}

// 007280a0: BSP_Gun_ApplyClearDestroyedFailure
// Original ABI: __thiscall(ECX=self, stack message), RET 4.
void bsp_gun_apply_clear_destroyed_failure_007280a0(Seg38_39Host& host, void* self, void* message) {
  Raw s = raw_of(self);
  if (host.vcall_0c_u32_bool(message, 0x69)) {
    s.at<std::uint8_t>(0x3b8) = 0;
    host.vcall_1b4_u32(self, 0);
    host.unit_tickable_entity_set_scene_nodes_visible_008781f0(self, 1);
    host.mission_entity_cancel_pending_destroy_00925a00(self);
  }
  host.unit_rebuild_weapon_category_index_00956c20(s.ptr(0x3f0));
}

// 00728340: BSP_SceneNode_ClearFlagsRecursive
// Original ABI: __thiscall(ECX=self, stack mask), RET 4.
void bsp_scene_node_clear_flags_recursive_00728340(void* self, std::uint32_t mask) {
  Raw s = raw_of(self);
  void* child = s.ptr(0x34);
  s.at<std::uint32_t>(0x48) &= ~mask;
  while (child != nullptr) {
    bsp_scene_node_clear_flags_recursive_00728340(child, mask);
    child = raw_of(child).ptr(0x3c);
  }
}

// 00728480: BSP_FreeOwnedBlock_00728480
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_free_owned_block_00728480(Seg38_39Host& host, void* self) {
  void* block = raw_of(self).ptr(0x00);
  if (block != nullptr) {
    host.free_00bf6989(block);
  }
}

// 007284b0: BSP_OwnedPtr_Delete_007284b0
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_owned_ptr_delete_007284b0(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    host.vcall_00_deleting(held, 1);
    s.ptr(0x00) = nullptr;
  }
}

// 007284d0: BSP_FreeOwnedBlock_007284d0
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_free_owned_block_007284d0(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* block = s.ptr(0x00);
  if (block != nullptr) {
    host.free_00bf6989(block);
    s.ptr(0x00) = nullptr;
  }
}

// 00728520: BSP_OwnedPtr_Delete_00728520
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_owned_ptr_delete_00728520(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  void* held = s.ptr(0x00);
  if (held != nullptr) {
    host.vcall_00_deleting(held, 1);
    s.ptr(0x00) = nullptr;
  }
}

// 00728610: BSP_NativeVector12_Grow_00728610
// Original ABI: __fastcall(ECX=self), RET 0.
void bsp_native_vector12_grow_00728610(Seg38_39Host& host, void* self) {
  Raw s = raw_of(self);
  const std::int32_t new_cap = s.at<std::int32_t>(0x08) * 2 + 2;
  s.at<std::int32_t>(0x08) = new_cap;
  std::uint8_t* new_buf =
      static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(new_cap * 12)));
  const std::uint32_t count = s.at<std::uint32_t>(0x04);
  for (std::uint32_t i = 0; i < count; ++i) {
    std::uint8_t* dst_elem = new_buf + static_cast<std::size_t>(i) * 12;
    if (dst_elem != nullptr) {
      const std::uint8_t* src_elem =
          static_cast<const std::uint8_t*>(s.ptr(0x00)) + static_cast<std::size_t>(i) * 12;
      Raw dst_r{dst_elem};
      Raw src_r = raw_of(src_elem);
      dst_r.at<std::uint32_t>(0x00) = src_r.at<std::uint32_t>(0x00);
      dst_r.at<std::uint32_t>(0x04) = src_r.at<std::uint32_t>(0x04);
      dst_r.at<std::uint32_t>(0x08) = src_r.at<std::uint32_t>(0x08);
    }
  }
  if (s.ptr(0x00) != nullptr) {
    host.free_00bf6989(s.ptr(0x00));
  }
  s.ptr(0x00) = new_buf;
}

// 00728960: BSP_GunBase_Construct
// Original ABI: __thiscall(ECX=self, stack flag) -> EAX=self, RET 4.
void* bsp_gun_base_construct_00728960(Seg38_39Host& host, void* self, std::uint32_t flag) {
  Raw s = raw_of(self);
  host.unit_tickable_entity_construct_0087b670(self, flag);
  s.ptr(0x00) = host.image_address(0x00cfdc58);
  s.ptr(0x10) = host.image_address(0x00cfdc40);
  s.ptr(0x24) = host.image_address(0x00cfdc38);
  s.ptr(0x170) = host.image_address(0x00cfdc34);
  s.ptr(0x1e4) = host.image_address(0x00cfdc2c);
  s.ptr(0x310) = host.image_address(0x00cfdc14);
  s.at<std::uint32_t>(0xc4) = 0x1eu;
  return self;
}

// oct18b-nB-low additions. All return-only bodies have the original ECX
// receiver, no used register inputs, and RET unless an argument is listed.
std::uint8_t __fastcall DirectorPathBase_VSlot04_False_0071bc40(const void*) { return 0; }
std::uint32_t __fastcall DirectorPathBase_VSlot08_Zero_0071bc50(const void*) { return 0; }
std::uint8_t __fastcall DirectorPathBase_VSlot0C_False_0071bc60(const void*) { return 0; }
std::int32_t __fastcall DirectorPathBase_VSlot10_MinusOne_0071bc70(const void*) { return -1; }
std::int32_t __fastcall DirectorPathBase_VSlot14_MinusOne_0071bc80(const void*) { return -1; }
// 0071BC90 RET 4, 0071BCA0 RET; neither establishes a return value.
void __fastcall DirectorPathBase_VSlot18_NoOp_0071bc90(void*, void*, std::uint32_t) {}
void __fastcall DirectorPathBase_VSlot1C_NoOp_0071bca0(void*) {}

// 0071D550..0071D556: an ordinary vtable tail dispatch, not a no-return
// function and not a deleting thunk. Preserve stack and result registers for
// the unknown signature of the virtual target at +18h.
__declspec(naked) void __fastcall WeaponDirector_ForwardSlot18_0071d550(void*) {
    __asm {
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 18h]
        jmp edx
    }
}

// 0071EA90..0071EAF5: __thiscall(params)(archive), RET 4. The decompiler
// mistakes the two output addresses for saved pointers. The listing actually
// passes two eight-byte tagged arguments to archive->vtable[10h]: key kind 0,
// output kind 1 (u32*). Reuse ArchiveArg rather than introducing another tag.
void __fastcall PathFollowParams_Read_0071ea90(void* self, void*, void* archive) {
    using cc13_seg33a::ArchiveArg;
    using ReadField = void(__thiscall*)(void*, ArchiveArg, ArchiveArg);
    auto** table = *reinterpret_cast<void***>(archive);
    auto read_field = reinterpret_cast<ReadField>(table[0x10 / 4]);
    // These initial words are the saved ECX and original argument slots used
    // as scratch by the original; a valid archive read replaces each one.
    std::uint32_t follow = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(self));
    read_field(archive, ArchiveArg{0, 0x00cfdacc},
               ArchiveArg{1, reinterpret_cast<std::uintptr_t>(&follow)});
    raw_of(self).at<std::uint32_t>(0) = follow;
    std::uint32_t start = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
    table = *reinterpret_cast<void***>(archive);
    read_field = reinterpret_cast<ReadField>(table[0x10 / 4]);
    read_field(archive, ArchiveArg{0, 0x00cfdac0},
               ArchiveArg{1, reinterpret_cast<std::uintptr_t>(&start)});
    raw_of(self).at<std::uint32_t>(4) = start;
}

// 00720F40 is a literal RET 4. 00721970 is a tail JMP 00721890 with no
// receiver adjustment: ECX=message, stack=controller, target cleanup RET 4.
void __fastcall CommandController_VSlot_NoOp_00720f40(void*, void*, std::uint32_t) {}
void __fastcall CommandControllerStateMessage_Apply_00721970(void* self, void*, void* controller) {
    reinterpret_cast<void(__thiscall*)(void*, void*)>(0x00721890)(self, controller);
}

}  // namespace bsp::director_geommesh_seg38_39_argon

namespace bsp::director_geommesh_seg38_39_argon {
void gun_apply_damage_if_enabled_00728080(void* gun, float amount, cc13_sweepA::SweepAHost& host) {
    auto* bytes = static_cast<std::uint8_t*>(gun);
    if (bytes[0x3b8] == 0) {
        // 00728089..00728091 quiets/forwards the stack float through x87.
        float forwarded;
        __asm { fld amount }
        __asm { fstp forwarded }
        cc13_sweepA::device_apply_scaled_damage_00442ae0({bytes}, forwarded, host);
    }
}
std::uint8_t gun_failure_slot101_007289b0(void*, std::uint32_t, std::uint32_t) noexcept { return 0; }
std::uint8_t gun_failure_slot102_007289c0(void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept { return 0; }
std::uint8_t gun_failure_slot103_007289d0(void*, std::uint32_t, std::uint32_t) noexcept { return 0; }
}
