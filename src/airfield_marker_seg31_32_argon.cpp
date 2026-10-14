#include "bsp/airfield_marker_seg31_32_argon.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace bsp::airfield_marker_seg31_32_argon {

namespace {

inline Raw raw_from_u32(std::uint32_t addr) noexcept {
  return Raw{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(addr))};
}

inline Raw raw_from_ptr(void* ptr) noexcept {
  return Raw{static_cast<std::uint8_t*>(ptr)};
}

inline Raw raw_from_cptr(const void* ptr) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(ptr))};
}

struct NativeStringLocal {
  std::uint32_t length{0};
  void* data{nullptr};
};

}  // namespace

// 006d4370 | BSP_AirOpsStockSnapshot_Destruct
// Original ABI: __thiscall(void* self) -> void, RET.
void air_ops_stock_snapshot_destruct_006d4370(Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x00) = host.image_address(0x00cf8eb8u);
  void* vec2_first = s.ptr(0x1c);
  if (vec2_first != nullptr) {
    host.free_00bf65ac(vec2_first);
  }
  s.at<std::uint32_t>(0x1c) = 0u;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint32_t>(0x24) = 0u;
  void* vec1_first = s.ptr(0x0c);
  if (vec1_first != nullptr) {
    host.free_00bf65ac(vec1_first);
  }
  s.at<std::uint32_t>(0x0c) = 0u;
  s.at<std::uint32_t>(0x10) = 0u;
  s.at<std::uint32_t>(0x14) = 0u;
}

// 006d4420 | BSP_SessionMessage_AIRFIELD_CREATE_Destruct
// Original ABI: __thiscall(void* self) -> void, tail JMP 007668e0.
void session_message_airfield_create_destruct_006d4420(
    Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x00) = host.image_address(0x00cf8ebcu);
  void* hangars_buf = s.ptr(0x13c);
  if (hangars_buf != nullptr) {
    host.free_00bf6989(hangars_buf);
    s.ptr(0x13c) = nullptr;
  }
  s.ptr(0x110) = host.image_address(0x00cf8eb8u);
  void* vec2_first = s.ptr(0x12c);
  if (vec2_first != nullptr) {
    host.free_00bf65ac(vec2_first);
  }
  s.at<std::uint32_t>(0x12c) = 0u;
  s.at<std::uint32_t>(0x130) = 0u;
  s.at<std::uint32_t>(0x134) = 0u;
  void* vec1_first = s.ptr(0x11c);
  if (vec1_first != nullptr) {
    host.free_00bf65ac(vec1_first);
  }
  s.at<std::uint32_t>(0x11c) = 0u;
  s.at<std::uint32_t>(0x120) = 0u;
  s.at<std::uint32_t>(0x124) = 0u;
  host.net_entity_create_message_destruct_007668e0(self);
}

// 006d44b0 | BSP_SessionMessage_AIRFIELD_CREATE_StockSnapshot
// Original ABI: __thiscall(void* self) -> void*, RET.
void* session_message_airfield_create_stock_snapshot_006d44b0(void* self) {
  return static_cast<std::uint8_t*>(self) + 0x110;
}

// 006d44c0 | BSP_NetEntityCreateMessage_VSlot18_ReturnZero
// Original ABI: __thiscall(void* self) -> uint32_t, RET.
std::uint32_t net_entity_create_message_vslot18_return_zero_006d44c0(
    void* /*self*/) {
  return 0u;
}

// 006d44f0 | BSP_AirOpsStockSnapshot_Write
// Original ABI: __thiscall(void* self, void* cursor) -> void, RET 0x4.
void air_ops_stock_snapshot_write_006d44f0(
    Seg31_32Host& host, void* self, void* cursor) {
  Raw s = raw_from_ptr(self);
  host.bit_cursor_write_unsigned_dword_bits_00429070(
      cursor, s.at<std::uint32_t>(0x04), 6u);

  std::uint32_t count1 = 0u;
  if (s.ptr(0x0c) != nullptr) {
    count1 = static_cast<std::uint32_t>(
        (static_cast<std::uint8_t*>(s.ptr(0x10)) -
         static_cast<std::uint8_t*>(s.ptr(0x0c))) /
        12);
  }
  host.bit_cursor_write_unsigned_dword_bits_00429070(cursor, count1, 6u);

  if (count1 > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
              .at<std::uint32_t>(0x00),
          10u);

      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
              .at<std::uint32_t>(0x04),
          8u);

      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
              .at<std::uint32_t>(0x08),
          3u);

      ++idx;
      off += 0x0cu;
    } while (idx < count1);
  }

  std::uint32_t count2 = 0u;
  if (s.ptr(0x1c) != nullptr) {
    count2 = static_cast<std::uint32_t>(
        (static_cast<std::uint8_t*>(s.ptr(0x20)) -
         static_cast<std::uint8_t*>(s.ptr(0x1c))) /
        20);
  }
  host.bit_cursor_write_unsigned_dword_bits_00429070(cursor, count2, 6u);

  if (count2 > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
              .at<std::uint32_t>(0x00),
          10u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
              .at<std::uint32_t>(0x04),
          6u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
              .at<std::uint32_t>(0x08),
          4u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_word_bits_00429120(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
              .at<std::uint16_t>(0x0c),
          12u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_write_unsigned_dword_bits_00429070(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
              .at<std::uint32_t>(0x10),
          3u);

      ++idx;
      off += 0x14u;
    } while (idx < count2);
  }
}

// 006d47a0 | BSP_SessionMessage_AIRFIELD_CREATE_Write
// Original ABI: __thiscall(void* self, void* cursor) -> void, RET 0x4.
void session_message_airfield_create_write_006d47a0(
    Seg31_32Host& host, void* self, void* cursor) {
  Raw s = raw_from_ptr(self);
  host.net_entity_create_message_write_00765000(self, cursor);
  air_ops_stock_snapshot_write_006d44f0(host, s.p + 0x110, cursor);
  host.bit_cursor_write_numeric_float_004295c0(
      cursor,
      s.at<float>(0x140),
      0u,
      1u,
      host.global_f32(0x00d7a248u),
      0x20);
  host.bit_cursor_write_numeric_float_004295c0(
      cursor,
      s.at<float>(0x144),
      0u,
      1u,
      host.global_f32(0x00d7a248u),
      0x20);
  host.bit_cursor_write_unsigned_dword_bits_00429070(
      cursor, s.at<std::uint32_t>(0x138), 3u);

  if (s.at<std::uint32_t>(0x138) > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      host.bit_cursor_write_word_bits_00429120(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
              .at<std::uint16_t>(0x00),
          0x0cu);
      host.bit_cursor_write_word_bits_00429120(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
              .at<std::uint16_t>(0x02),
          0x0du);
      host.bit_cursor_write_word_bits_00429120(
          cursor,
          Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
              .at<std::uint16_t>(0x04),
          0x0du);
      ++idx;
      off += 6u;
    } while (idx < s.at<std::uint32_t>(0x138));
  }
}

// 006d4c20 | BSP_AirOpsStockSnapshot_Construct
// Original ABI: __thiscall(void* self) -> void*, RET.
void* air_ops_stock_snapshot_construct_006d4c20(
    Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x00) = host.image_address(0x00cf8eb8u);
  s.at<std::uint32_t>(0x0c) = 0u;
  s.at<std::uint32_t>(0x10) = 0u;
  s.at<std::uint32_t>(0x14) = 0u;
  s.at<std::uint32_t>(0x1c) = 0u;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint32_t>(0x24) = 0u;
  return self;
}

// 006d4c70 | BSP_SessionMessage_AIRFIELD_CREATE_Construct
// Original ABI: __thiscall(void* self, void* airfield, float field_140, float field_144) -> void*, RET 0xc.
void* session_message_airfield_create_construct_006d4c70(
    Seg31_32Host& host,
    void* self,
    void* airfield,
    float field_140,
    float field_144) {
  Raw s = raw_from_ptr(self);
  Raw af = raw_from_ptr(airfield);
  std::uint32_t class_id =
      raw_from_ptr(af.ptr(0x538)).at<std::uint32_t>(0x70);
  host.net_entity_create_message_construct_00766fc0(
      self, 0xa3u, class_id, airfield, 1u);

  s.ptr(0x00) = host.image_address(0x00cf8ebcu);
  s.ptr(0x110) = host.image_address(0x00cf8eb8u);
  s.at<std::uint32_t>(0x11c) = 0u;
  s.at<std::uint32_t>(0x120) = 0u;
  s.at<std::uint32_t>(0x124) = 0u;
  s.at<std::uint32_t>(0x12c) = 0u;
  s.at<std::uint32_t>(0x130) = 0u;
  s.at<std::uint32_t>(0x134) = 0u;

  host.air_ops_stock_snapshot_capture_006cc800(s.p + 0x110, af.p + 0x72c);
  s.at<float>(0x140) = field_140;
  s.at<float>(0x144) = field_144;

  std::uint32_t count = af.at<std::uint32_t>(0x834);
  s.at<std::uint32_t>(0x138) = count;
  std::uint64_t prod = static_cast<std::uint64_t>(count) * 6ull;
  std::uint32_t alloc_bytes =
      ((prod >> 32) != 0ull) ? 0xffffffffu : static_cast<std::uint32_t>(prod);
  s.ptr(0x13c) = host.operator_new_00bf55be(alloc_bytes);

  if (s.at<std::uint32_t>(0x138) > 0u) {
    std::uint32_t idx = 0u;
    std::size_t src_off = 0u;
    std::size_t dst_off = 0u;
    do {
      Raw src_rec{static_cast<std::uint8_t*>(af.ptr(0x830)) + src_off};
      std::uint16_t entity_id =
          raw_from_ptr(src_rec.ptr(0x00)).at<std::uint16_t>(0x174);
      Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + dst_off}
          .at<std::uint16_t>(0x00) = entity_id;

      void* entry_path =
          Raw{static_cast<std::uint8_t*>(af.ptr(0x830)) + src_off}.ptr(0x04);
      std::uint16_t entry_id = host.vcall_path_get_id_vslot28(entry_path);
      Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + dst_off}
          .at<std::uint16_t>(0x02) = entry_id;

      void* exit_path =
          Raw{static_cast<std::uint8_t*>(af.ptr(0x830)) + src_off}.ptr(0x08);
      std::uint16_t exit_id = host.vcall_path_get_id_vslot28(exit_path);
      Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + dst_off}
          .at<std::uint16_t>(0x04) = exit_id;

      ++idx;
      src_off += 0x0cu;
      dst_off += 6u;
    } while (idx < s.at<std::uint32_t>(0x138));
  }
  return self;
}

// 006d4dd0 | BSP_AirField_AppendIntactHangarsAsSubEntities
// Original ABI: __thiscall(void* self, void* out_pointer_vec) -> void, RET 0x4.
void air_field_append_intact_hangars_as_sub_entities_006d4dd0(
    Seg31_32Host& host, void* self, void* out_pointer_vec) {
  Raw s = raw_from_ptr(self);
  std::uint8_t* cur = static_cast<std::uint8_t*>(s.ptr(0x830));
  std::uint8_t* end = cur + s.at<std::uint32_t>(0x834) * 12u;
  if (cur != end) {
    do {
      void* hangar = Raw{cur}.ptr(0x00);
      if (hangar != nullptr &&
          raw_from_ptr(hangar).at<float>(0x370) >
              host.global_f32(0x00d7a218u)) {
        void* elem = hangar;
        host.pointer_vector_push_back_004323d0(out_pointer_vec, &elem);
      }
      cur += 12u;
    } while (cur != static_cast<std::uint8_t*>(s.ptr(0x830)) +
                        s.at<std::uint32_t>(0x834) * 12u);
  }
}

// 006d4eb0 | BSP_AirOpsStockSnapshot_Read
// Original ABI: __thiscall(void* self, void* stream) -> void, RET 0x4.
void air_ops_stock_snapshot_read_006d4eb0(
    Seg31_32Host& host, void* self, void* stream) {
  Raw s = raw_from_ptr(self);
  void* cursor = static_cast<std::uint8_t*>(stream) + 0x04;
  host.bit_cursor_read_unsigned_dword_00428d10(
      cursor, &s.at<std::uint32_t>(0x04), 6u);

  std::uint32_t count = 0u;
  host.bit_cursor_read_unsigned_dword_00428d10(cursor, &count, 6u);
  host.stl_vector_resize_006ca010(s.p + 0x08, count, 0u, 0u, 0u);

  if (count > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
               .at<std::uint32_t>(0x00),
          10u);

      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
               .at<std::uint32_t>(0x04),
          8u);

      if (s.ptr(0x0c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x10)) -
                      static_cast<std::uint8_t*>(s.ptr(0x0c))) /
                     12)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x0c)) + off}
               .at<std::uint32_t>(0x08),
          3u);

      ++idx;
      off += 0x0cu;
    } while (idx < count);
  }

  host.bit_cursor_read_unsigned_dword_00428d10(cursor, &count, 6u);
  host.stl_vector_resize_006ca0f0(s.p + 0x18, count, 0u, 0u, 0u, 0u, 0u);

  if (count > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
               .at<std::uint32_t>(0x00),
          10u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
               .at<std::uint32_t>(0x04),
          6u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
               .at<std::uint32_t>(0x08),
          4u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_word_bits_00428e30(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
               .at<std::uint16_t>(0x0c),
          12u);

      if (s.ptr(0x1c) == nullptr ||
          idx >= static_cast<std::uint32_t>(
                     (static_cast<std::uint8_t*>(s.ptr(0x20)) -
                      static_cast<std::uint8_t*>(s.ptr(0x1c))) /
                     20)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      host.bit_cursor_read_unsigned_dword_00428d10(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x1c)) + off}
               .at<std::uint32_t>(0x10),
          3u);

      ++idx;
      off += 0x14u;
    } while (idx < count);
  }
}

// 006d5130 | BSP_SessionMessage_AIRFIELD_CREATE_Read
// Original ABI: __thiscall(void* self, void* stream) -> void, RET 0x4.
void session_message_airfield_create_read_006d5130(
    Seg31_32Host& host, void* self, void* stream) {
  Raw s = raw_from_ptr(self);
  host.net_entity_create_message_read_00767e30(self, stream);
  air_ops_stock_snapshot_read_006d4eb0(host, s.p + 0x110, stream);

  void* cursor = static_cast<std::uint8_t*>(stream) + 0x04;
  host.bit_cursor_read_numeric_float_004293f0(
      cursor,
      &s.at<float>(0x140),
      0u,
      1u,
      host.global_f32(0x00d7a248u),
      0x20);
  host.bit_cursor_read_numeric_float_004293f0(
      cursor,
      &s.at<float>(0x144),
      0u,
      1u,
      host.global_f32(0x00d7a248u),
      0x20);
  host.bit_cursor_read_unsigned_dword_00428d10(
      cursor, &s.at<std::uint32_t>(0x138), 3u);

  std::uint32_t count = s.at<std::uint32_t>(0x138);
  std::uint64_t prod = static_cast<std::uint64_t>(count) * 6ull;
  std::uint32_t alloc_bytes =
      ((prod >> 32) != 0ull) ? 0xffffffffu : static_cast<std::uint32_t>(prod);
  s.ptr(0x13c) = host.operator_new_00bf55be(alloc_bytes);

  if (s.at<std::uint32_t>(0x138) > 0u) {
    std::uint32_t idx = 0u;
    std::size_t off = 0u;
    do {
      host.bit_cursor_read_word_bits_00428e30(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
               .at<std::uint16_t>(0x00),
          0x0cu);
      host.bit_cursor_read_word_bits_00428e30(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
               .at<std::uint16_t>(0x02),
          0x0du);
      host.bit_cursor_read_word_bits_00428e30(
          cursor,
          &Raw{static_cast<std::uint8_t*>(s.ptr(0x13c)) + off}
               .at<std::uint16_t>(0x04),
          0x0du);
      ++idx;
      off += 6u;
    } while (idx < s.at<std::uint32_t>(0x138));
  }
}

// 006d6010 | BSP_MarkerGroupVector_Clear
// Original ABI: __thiscall(void* self, void* out_vec) -> void, RET 0x4.
void marker_group_vector_clear_006d6010(
    Seg31_32Host& host, void* self, void* out_vec) {
  Raw s = raw_from_ptr(self);
  Raw v = raw_from_ptr(out_vec);
  void* last = v.ptr(0x08);
  if (reinterpret_cast<std::uintptr_t>(v.ptr(0x04)) >
      reinterpret_cast<std::uintptr_t>(last)) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  void* first = v.ptr(0x04);
  if (reinterpret_cast<std::uintptr_t>(first) >
      reinterpret_cast<std::uintptr_t>(v.ptr(0x08))) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  std::uint32_t out_iter[2] = {0u, 0u};
  host.stl_vector_erase_range_006d3a50(
      out_vec, out_iter, out_vec, first, out_vec, last);

  std::uint8_t* cur = static_cast<std::uint8_t*>(s.ptr(0x830));
  std::uint8_t* end = cur + s.at<std::uint32_t>(0x834) * 12u;
  if (cur != end) {
    do {
      Raw item = raw_from_ptr(Raw{cur}.ptr(0x00));
      if (item.at<std::uint8_t>(0x5c) != 0u &&
          item.at<std::uint8_t>(0x5d) == 0u &&
          item.at<std::uint8_t>(0x60) == 0u &&
          item.at<std::uint8_t>(0x5e) == 0u) {
        host.stl_vector_push_back_006d4e40(out_vec, cur);
      }
      cur += 12u;
    } while (cur != static_cast<std::uint8_t*>(s.ptr(0x830)) +
                        s.at<std::uint32_t>(0x834) * 12u);
  }
}

// 006d6170 | BSP_Vec3_LexicographicLess
// Original ABI: __stdcall/__thiscall(const float* lhs, const float* rhs) -> bool, RET 0x8.
bool vec3_lexicographic_less_006d6170(const float* lhs, const float* rhs) {
  if (lhs[0] < rhs[0]) {
    return true;
  }
  if (lhs[0] == rhs[0]) {
    if (lhs[1] < rhs[1]) {
      return true;
    }
    if (lhs[1] == rhs[1] && lhs[2] < rhs[2]) {
      return true;
    }
  }
  return false;
}

// 006d61f0 | BSP_MarkerInstance_Tick
// Original ABI: __thiscall(void* self, float delta) -> void, RET 0x4.
void marker_instance_tick_006d61f0(
    Seg31_32Host& host, void* self, float /*delta*/) {
  host.vcall_marker_instance_vslot08(self);
}

// 006d6200 | TRIV_body_006d6200
// Original ABI: void(void), RET.
void triv_body_006d6200() {}

// 006d7100 | TMPL_OwnedPtr_DeleteReset_006d7100
// Original ABI: __thiscall(void** self_slot) -> void, RET.
void tmpl_owned_ptr_delete_reset_006d7100(
    Seg31_32Host& host, void** self_slot) {
  if (*self_slot != nullptr) {
    host.vcall_scalar_deleting_dtor_vslot00(*self_slot, 1u);
    *self_slot = nullptr;
  }
}

// 006d7120 | BSP_OwnedObjectPtr_Delete_006d7120
// Original ABI: __thiscall(void** self_slot) -> void, RET.
void owned_object_ptr_delete_006d7120(Seg31_32Host& host, void** self_slot) {
  if (*self_slot != nullptr) {
    host.vcall_scalar_deleting_dtor_vslot04(*self_slot, 1u);
    *self_slot = nullptr;
  }
}

// 006d7140 | TMPL_OwnedPtr_DeleteReset_006d7140
// Original ABI: __thiscall(void** self_slot) -> void, RET.
void tmpl_owned_ptr_delete_reset_006d7140(
    Seg31_32Host& host, void** self_slot) {
  if (*self_slot != nullptr) {
    host.vcall_scalar_deleting_dtor_vslot00(*self_slot, 1u);
    *self_slot = nullptr;
  }
}

// 006d74b0 | BSP_MarkerClasses_ResetLuaTable
// Original ABI: __cdecl(void) -> void, RET.
void marker_classes_reset_lua_table_006d74b0(Seg31_32Host& host) {
  Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
  Raw lua_holder = raw_from_ptr(app.ptr(0x1a08));
  void* lua_machine = lua_holder.ptr(0x04);
  host.lua_machine_run_string_006b8ad0(
      lua_machine,
      static_cast<const char*>(host.image_address(0x00cf8f54u)),
      nullptr,
      nullptr,
      2);
}

// 006d74d0 | BSP_MarkerInstance_Destruct
// Original ABI: __thiscall(void* self) -> void, RET.
void marker_instance_destruct_006d74d0(Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x00) = host.image_address(0x00cf8f68u);
  void* scene_inst = s.ptr(0x18);
  if (scene_inst != nullptr) {
    host.vcall_scalar_deleting_dtor_vslot04(scene_inst, 1u);
    s.ptr(0x18) = nullptr;
  }
}

// 006d7fc0 | BSP_MarkerInstance_UpdatePose
// Original ABI: __thiscall(void* self) -> void, RET.
void marker_instance_update_pose_006d7fc0(Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  Raw desc = raw_from_ptr(s.ptr(0x14));
  if (desc.at<std::int32_t>(0x18) == 0) {
    return;
  }

  void* owner = s.ptr(0x04);
  const float* pos_src = nullptr;
  if (owner != nullptr) {
    Raw own = raw_from_ptr(owner);
    if (own.at<std::uint8_t>(0xc8) == 0u) {
      host.entity_pose_refresh_world_00414db0(owner);
    }
    pos_src = &own.at<float>(0xfc);
  } else {
    pos_src = &s.at<float>(0x08);
  }

  float pos_x = pos_src[0];
  float pos_y = pos_src[1];
  float pos_z = pos_src[2];

  float one = host.global_f32(0x00d7a24cu);
  float mat[16] = {
      one, 0.0f, 0.0f, 0.0f,
      0.0f, one, 0.0f, 0.0f,
      0.0f, 0.0f, one, 0.0f,
      0.0f, 0.0f, 0.0f, one};

  Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
  std::int32_t mode = raw_from_ptr(s.ptr(0x14)).at<std::int32_t>(0x18);
  float t = app.at<float>(0x64c);
  float phase = (t + t) + s.at<float>(0x1c);

  float scratch_rot[16]{};
  float scratch_copy[16]{};

  if (mode == 1) {
    float neg_phase = host.global_f32(0x00d7a208u) - phase;
    void* rot = host.matrix_build_rotation_x_00b64640(scratch_rot, &neg_phase);
    host.matrix_copy_4x4_x87_004134f0(scratch_copy, rot);
    host.matrix_copy_4x4_x87_004134f0(mat, scratch_copy);
  } else if (mode == 2) {
    float neg_phase = host.global_f32(0x00d7a208u) - phase;
    void* rot = host.matrix_build_rotation_y_00b646e0(scratch_rot, &neg_phase);
    host.matrix_copy_4x4_x87_004134f0(scratch_copy, rot);
    host.matrix_copy_4x4_x87_004134f0(mat, scratch_copy);
  } else if (mode == 3) {
    void* rot = host.matrix_set_rotation_z_negated_0042b4d0(scratch_rot, phase);
    host.matrix_copy_4x4_x87_004134f0(mat, rot);
  } else if (mode == 4) {
    float abs_sin = std::fabs(std::sin(phase));
    pos_y = static_cast<float>(
        static_cast<double>(abs_sin) * host.global_f64(0x00ce3880u) +
        static_cast<double>(pos_y));
  }

  mat[12] = pos_x;
  mat[13] = pos_y;
  mat[14] = pos_z;

  Raw scene_inst = raw_from_ptr(s.ptr(0x18));
  host.vcall_node_set_transform_vslot34(scene_inst.ptr(0x0c), mat);
}

// 006d8750 | BSP_MarkerInstance_Construct
// Original ABI: __thiscall(void* self, void* descriptor, void* owner, float x, float y, float z) -> void*, RET 0x14.
void* marker_instance_construct_006d8750(
    Seg31_32Host& host,
    void* self,
    void* descriptor,
    void* owner,
    float x,
    float y,
    float z) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x04) = owner;
  s.ptr(0x00) = host.image_address(0x00cf8f68u);
  s.at<float>(0x08) = x;
  s.at<float>(0x0c) = y;
  s.at<float>(0x10) = z;
  s.ptr(0x14) = descriptor;

  void* resource = raw_from_ptr(descriptor).ptr(0x14);
  void* scene_inst = host.vcall_resource_instantiate_vslot08(resource, 0, 1.0f);
  s.ptr(0x18) = scene_inst;

  Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
  host.node_propagate_root_registration_00b6d890(
      raw_from_ptr(scene_inst).ptr(0x0c), app.ptr(0x19ec));

  if (raw_from_ptr(s.ptr(0x14)).at<std::uint8_t>(0x1c) != 0u) {
    s.at<float>(0x1c) = 0.0f;
    marker_instance_update_pose_006d7fc0(host, self);
    return self;
  }

  s.at<float>(0x1c) = host.random_uniform_float_range_00bd2f10(
      1u, 0.0f, host.global_f32(0x00cf8f74u));
  marker_instance_update_pose_006d7fc0(host, self);
  return self;
}

// 006d8c70 | BSP_MarkerClass_CreateInstance
// Original ABI: __thiscall(void* self_descriptor, void* owner, float x, float y, float z) -> void*, RET 0x10.
void* marker_class_create_instance_006d8c70(
    Seg31_32Host& host,
    void* self_descriptor,
    void* owner,
    float x,
    float y,
    float z) {
  void* mem = host.operator_new_00bf681b(0x20u);
  if (mem != nullptr) {
    return marker_instance_construct_006d8750(
        host, mem, self_descriptor, owner, x, y, z);
  }
  return nullptr;
}

// 006d8e50 | BSP_NativeString_DestroyStorage_006d8e50
// Original ABI: __thiscall(void* self_string) -> void, RET.
void native_string_destroy_storage_006d8e50(
    Seg31_32Host& host, void* self_string) {
  Raw s = raw_from_ptr(self_string);
  void* data = s.ptr(0x04);
  if (data != nullptr) {
    std::uint32_t bytes = s.at<std::uint32_t>(0x00) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// 006d8e70 | BSP_NativeString_DestroyStorage_006d8e70
// Original ABI: __thiscall(void* self_string) -> void, RET.
void native_string_destroy_storage_006d8e70(
    Seg31_32Host& host, void* self_string) {
  Raw s = raw_from_ptr(self_string);
  void* data = s.ptr(0x04);
  if (data != nullptr) {
    std::uint32_t bytes = s.at<std::uint32_t>(0x00) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, data, bytes, 1);
  }
}

// 006d9140 | BSP_NamedValue_Construct_006d9140
// Original ABI: __thiscall(void* self, const void* name_str, const uint32_t* value_ptr) -> void*, RET 0x8.
void* named_value_construct_006d9140(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    const std::uint32_t* value_ptr) {
  Raw s = raw_from_ptr(self);
  Raw n = raw_from_cptr(name_str);
  s.at<std::uint32_t>(0x00) = 0u;
  s.ptr(0x04) = nullptr;
  if (self != name_str) {
    host.native_string_resize_0041dd40(self, n.at<std::uint32_t>(0x00), 1u);
    if (n.at<std::uint32_t>(0x00) != 0u) {
      std::memcpy(s.ptr(0x04), n.ptr(0x04), s.at<std::uint32_t>(0x00));
    }
  }
  s.at<std::uint32_t>(0x08) = *value_ptr;
  return self;
}

// 006d9190 | BSP_NamedValue_Construct_006d9190
// Original ABI: __thiscall(void* self, const void* name_str, const uint32_t* value_ptr) -> void*, RET 0x8.
void* named_value_construct_006d9190(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    const std::uint32_t* value_ptr) {
  Raw s = raw_from_ptr(self);
  Raw n = raw_from_cptr(name_str);
  s.at<std::uint32_t>(0x00) = 0u;
  s.ptr(0x04) = nullptr;
  if (self != name_str) {
    host.native_string_resize_0041dd40(self, n.at<std::uint32_t>(0x00), 1u);
    if (n.at<std::uint32_t>(0x00) != 0u) {
      std::memcpy(s.ptr(0x04), n.ptr(0x04), s.at<std::uint32_t>(0x00));
    }
  }
  s.at<std::uint32_t>(0x08) = *value_ptr;
  return self;
}

// 006d91e0 | BSP_NamedValue_CopyConstruct_006d91e0
// Original ABI: __thiscall(void* self, const void* other) -> void*, RET 0x4.
void* named_value_copy_construct_006d91e0(
    Seg31_32Host& host, void* self, const void* other) {
  Raw s = raw_from_ptr(self);
  Raw o = raw_from_cptr(other);
  s.at<std::uint32_t>(0x00) = 0u;
  s.ptr(0x04) = nullptr;
  if (self != other) {
    host.native_string_resize_0041dd40(self, o.at<std::uint32_t>(0x00), 1u);
    if (o.at<std::uint32_t>(0x00) != 0u) {
      std::memcpy(s.ptr(0x04), o.ptr(0x04), s.at<std::uint32_t>(0x00));
    }
  }
  s.at<std::uint32_t>(0x08) = o.at<std::uint32_t>(0x08);
  return self;
}

// 006d9230 | BSP_NamedValue_CopyConstruct_006d9230
// Original ABI: __thiscall(void* self, const void* other) -> void*, RET 0x4.
void* named_value_copy_construct_006d9230(
    Seg31_32Host& host, void* self, const void* other) {
  Raw s = raw_from_ptr(self);
  Raw o = raw_from_cptr(other);
  s.at<std::uint32_t>(0x00) = 0u;
  s.ptr(0x04) = nullptr;
  if (self != other) {
    host.native_string_resize_0041dd40(self, o.at<std::uint32_t>(0x00), 1u);
    if (o.at<std::uint32_t>(0x00) != 0u) {
      std::memcpy(s.ptr(0x04), o.ptr(0x04), s.at<std::uint32_t>(0x00));
    }
  }
  s.at<std::uint32_t>(0x08) = o.at<std::uint32_t>(0x08);
  return self;
}

// 006d9300 | BSP_GuiElement_BuildPath
// Original ABI: __fastcall(void* out_string, void* element) -> void*, RET.
void* gui_element_build_path_006d9300(
    Seg31_32Host& host, void* out_string, void* element) {
  NativeStringLocal accum{};
  for (void* cur = element; cur != nullptr;
       cur = raw_from_ptr(cur).ptr(0x70)) {
    NativeStringLocal sep{};
    host.native_string_resize_0041dd40(&sep, 1u, 1u);
    if (sep.data != nullptr) {
      std::memcpy(
          sep.data, host.image_address(0x00ce3a70u), sep.length + 1u);
    }

    const void* elem_name = host.vcall_gui_element_get_name_vslot28(cur);
    NativeStringLocal temp1{};
    void* cat1 = host.native_string_concat_004261a0(&sep, &temp1, elem_name);
    NativeStringLocal temp2{};
    void* cat2 = host.native_string_concat_004261a0(cat1, &temp2, &accum);

    if (static_cast<void*>(&accum) != cat2) {
      Raw c2 = raw_from_ptr(cat2);
      host.native_string_resize_0041dd40(&accum, c2.at<std::uint32_t>(0x00), 1u);
      if (c2.at<std::uint32_t>(0x00) != 0u) {
        std::memcpy(accum.data, c2.ptr(0x04), accum.length);
      }
    }

    if (temp2.data != nullptr) {
      std::uint32_t bytes = temp2.length + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, temp2.data, bytes, 1);
    }
    if (temp1.data != nullptr) {
      std::uint32_t bytes = temp1.length + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, temp1.data, bytes, 1);
    }
    if (sep.data != nullptr) {
      std::uint32_t bytes = sep.length + 1u;
      void* pool = host.sized_storage_pool_get_singleton_00419cc0();
      host.sized_storage_pool_return_block_00bd1510(pool, sep.data, bytes, 1);
    }
  }

  host.native_string_substring_00469840(&accum, out_string, 1, 0x7fffffff);
  if (accum.data != nullptr) {
    std::uint32_t bytes = accum.length + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, accum.data, bytes, 1);
  }
  return out_string;
}

// 006dae20 | BSP_MarkerManager_AddGuiHighlight
// Original ABI: __thiscall(void* self, void* gui_element, uint8_t recursive_flag, uint8_t color_mode_flag) -> void, RET 0xc.
void marker_manager_add_gui_highlight_006dae20(
    Seg31_32Host& host,
    void* self,
    void* gui_element,
    std::uint8_t recursive_flag,
    std::uint8_t color_mode_flag) {
  Raw s = raw_from_ptr(self);
  std::uint32_t out_result[3] = {0u, 0u, 0u};
  if (recursive_flag != 0u) {
    host.stl_tree_insert_006da5b0(s.p + 0x38, out_result, &gui_element);
    Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
    host.global_f32(0x00e19970u) = app.at<float>(0x64c);
    return;
  }

  void* target_tree = (color_mode_flag != 0u) ? (s.p + 0x44) : (s.p + 0x2c);
  host.stl_tree_insert_006da5b0(target_tree, out_result, &gui_element);
  Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
  host.global_f32(0x00e19970u) = app.at<float>(0x64c);
}

// 006db3f0 | BSP_GuiElement_CollectDescendants
// Original ABI: __fastcall(void* root_element, void* out_vec) -> void, RET.
void gui_element_collect_descendants_006db3f0(
    Seg31_32Host& host, void* root_element, void* out_vec) {
  void* seed = root_element;
  host.credits_screen_push_widget_0052ed20(out_vec, &seed);

  std::uint32_t idx = 0u;
  if (host.stl_vector_size_00519d50(out_vec) == 0u) {
    return;
  }

  Raw v = raw_from_ptr(out_vec);
  do {
    std::uint8_t* vec_first = static_cast<std::uint8_t*>(v.ptr(0x04));
    if (vec_first == nullptr ||
        idx >= static_cast<std::uint32_t>(
                   (static_cast<std::uint8_t*>(v.ptr(0x08)) - vec_first) >>
                   2)) {
      host.invalid_parameter_noinfo_00bf6713();
    }

    Raw elem = raw_from_ptr(
        Raw{static_cast<std::uint8_t*>(v.ptr(0x04)) + idx * 4u}.ptr(0x00));
    Raw list_container{elem.p + 0x64};
    void* head = elem.ptr(0x68);
    void* node = raw_from_ptr(head).ptr(0x00);

    while (true) {
      void* cur_head = list_container.ptr(0x04);
      if (node == cur_head) {
        break;
      }
      if (node == list_container.ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }

      void* value_ptr = static_cast<std::uint8_t*>(node) + 0x08;
      std::uint8_t* cur_first = static_cast<std::uint8_t*>(v.ptr(0x04));
      std::uint32_t cur_size =
          (cur_first == nullptr)
              ? 0u
              : static_cast<std::uint32_t>(
                    (static_cast<std::uint8_t*>(v.ptr(0x08)) - cur_first) >> 2);
      if (cur_first != nullptr &&
          cur_size < static_cast<std::uint32_t>(
                         (static_cast<std::uint8_t*>(v.ptr(0x0c)) - cur_first) >>
                         2)) {
        raw_from_ptr(v.ptr(0x08)).ptr(0x00) = raw_from_ptr(value_ptr).ptr(0x00);
        v.ptr(0x08) = static_cast<std::uint8_t*>(v.ptr(0x08)) + 4u;
      } else {
        void* cur_last = v.ptr(0x08);
        if (reinterpret_cast<std::uintptr_t>(cur_first) >
            reinterpret_cast<std::uintptr_t>(cur_last)) {
          host.invalid_parameter_noinfo_00bf6713();
        }
        std::uint32_t out_iter[2] = {0u, 0u};
        host.stl_vector_insert_one_0052eb90(
            out_vec, out_iter, out_vec, cur_last, value_ptr);
      }

      if (node == list_container.ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      node = raw_from_ptr(node).ptr(0x00);
    }

    ++idx;
  } while (idx < host.stl_vector_size_00519d50(out_vec));
}

// 006dbac0 | BSP_GuiElement_ApplyHighlightPulse
// Original ABI: register ECX=element, DL=enable; stack(uint8_t recursive_flag, uint8_t color_mode_flag, float scale), RET 0xc.
void gui_element_apply_highlight_pulse_006dbac0(
    Seg31_32Host& host,
    void* element,
    std::uint8_t enable,
    std::uint8_t recursive_flag,
    std::uint8_t color_mode_flag,
    float scale) {
  float pulse = 0.0f;
  if (enable != 0u) {
    Raw app = raw_from_u32(host.global_u32(0x00e188a8u));
    float dt = app.at<float>(0x64c) - host.global_f32(0x00e19970u);
    pulse = std::fabs(std::cos(dt + dt));
  }

  if (color_mode_flag != 0u) {
    float rgba[4]{};
    host.vcall_gui_element_get_color_vslot54(element, rgba);
    double factor =
        (enable != 0u)
            ? (static_cast<double>(pulse) * host.global_f64(0x00cec9d8u) +
               host.global_f64(0x00d7a348u))
            : 1.0;
    rgba[3] = static_cast<float>(factor * static_cast<double>(scale));
    host.vcall_gui_element_set_color_vslot50(element, rgba);
    return;
  }

  float scaled_pulse =
      static_cast<float>(static_cast<double>(pulse) * host.global_f64(0x00cf8fc8u));
  raw_from_ptr(element).at<float>(0x94) = scaled_pulse;

  if (recursive_flag != 0u) {
    void* desc_vec[4] = {nullptr, nullptr, nullptr, nullptr};
    gui_element_collect_descendants_006db3f0(host, element, desc_vec);

    std::uint32_t idx = 1u;
    while (true) {
      std::uint8_t* first = static_cast<std::uint8_t*>(desc_vec[1]);
      if (first == nullptr) {
        break;
      }
      std::uint8_t* last = static_cast<std::uint8_t*>(desc_vec[2]);
      std::uint32_t count = static_cast<std::uint32_t>((last - first) >> 2);
      if (idx >= count) {
        host.free_00bf65ac(first);
        break;
      }
      void* child = Raw{first + idx * 4u}.ptr(0x00);
      raw_from_ptr(child).at<float>(0x94) = scaled_pulse;
      ++idx;
    }
  }
}

// 006dc5a0 | BSP_MarkerManager_RemoveGuiHighlight
// Original ABI: __thiscall(void* self, void* gui_element) -> void, RET 0x4.
void marker_manager_remove_gui_highlight_006dc5a0(
    Seg31_32Host& host, void* self, void* gui_element) {
  Raw s = raw_from_ptr(self);

  void* tree2c = s.p + 0x2c;
  void* iter2c[2] = {nullptr, nullptr};
  host.stl_tree_find_006d8870(tree2c, iter2c, &gui_element);
  void* head2c = s.ptr(0x30);
  if (iter2c[0] == nullptr || iter2c[0] != tree2c) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (iter2c[1] != head2c) {
    raw_from_ptr(gui_element).at<float>(0x94) =
        static_cast<float>(0.0 * host.global_f64(0x00cf8fc8u));
    void* out_iter[2] = {nullptr, nullptr};
    host.stl_tree_erase_006d9bf0(tree2c, out_iter, iter2c[0], iter2c[1]);
  }

  void* tree38 = s.p + 0x38;
  void* iter38_buf[2] = {nullptr, nullptr};
  auto* found38 = static_cast<void**>(
      host.stl_tree_find_006d8870(tree38, iter38_buf, &gui_element));
  void* owner38 = found38[0];
  void* node38 = found38[1];
  void* head38 = s.ptr(0x3c);
  if (owner38 == nullptr || owner38 != tree38) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (node38 != head38) {
    void* out_iter[2] = {nullptr, nullptr};
    host.stl_tree_erase_006d9bf0(tree38, out_iter, owner38, node38);
    gui_element_apply_highlight_pulse_006dbac0(
        host, gui_element, 0u, 1u, 0u, 1.0f);
  }

  void* tree44 = s.p + 0x44;
  void* iter44_buf[2] = {nullptr, nullptr};
  auto* found44 = static_cast<void**>(
      host.stl_tree_find_006d8870(tree44, iter44_buf, &gui_element));
  void* owner44 = found44[0];
  void* node44 = found44[1];
  void* head44 = s.ptr(0x48);
  if (owner44 == nullptr || owner44 != tree44) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  if (node44 != head44) {
    void* out_iter[2] = {nullptr, nullptr};
    host.stl_tree_erase_006d9bf0(tree44, out_iter, owner44, node44);
    float rgba[4]{};
    host.vcall_gui_element_get_color_vslot54(gui_element, rgba);
    rgba[3] = host.global_f32(0x00d7a24cu);
    host.vcall_gui_element_set_color_vslot50(gui_element, rgba);
  }
}

// 006dda40 | BSP_MarkerManager_ForgetEntity
// Original ABI: __thiscall(void* self, uint32_t entity_id) -> void, RET 0x4.
void marker_manager_forget_entity_006dda40(
    Seg31_32Host& host, void* self, std::uint32_t entity_id) {
  Raw s = raw_from_ptr(self);
  void* outer_tree = s.p + 0x14;
  void* outer_iter[2] = {outer_tree, nullptr};
  host.stl_tree_find_006d8d10(outer_tree, outer_iter, &entity_id);

  void* outer_owner = outer_iter[0];
  void* outer_head = s.ptr(0x18);
  if (outer_owner == nullptr || outer_owner != outer_tree) {
    host.invalid_parameter_noinfo_00bf6713();
  }
  void* outer_node = outer_iter[1];
  if (outer_node != outer_head) {
    if (outer_owner == nullptr) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    if (outer_node == raw_from_ptr(outer_owner).ptr(0x04)) {
      host.invalid_parameter_noinfo_00bf6713();
    }

    std::uint8_t* inner_tree = static_cast<std::uint8_t*>(outer_node) + 0x10;
    void* inner_head = raw_from_ptr(outer_node).ptr(0x14);
    void* inner_first = raw_from_ptr(inner_head).ptr(0x00);
    void* inner_iter[2] = {inner_tree, inner_first};

    while (true) {
      if (outer_iter[1] == raw_from_ptr(outer_owner).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      void* cur_inner_head = Raw{inner_tree}.ptr(0x04);
      if (inner_iter[0] == nullptr || inner_iter[0] != inner_tree) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      if (inner_iter[1] == cur_inner_head) {
        break;
      }
      if (inner_iter[0] == nullptr) {
        host.invalid_parameter_noinfo_00bf6713();
      }
      if (inner_iter[1] == raw_from_ptr(inner_iter[0]).ptr(0x04)) {
        host.invalid_parameter_noinfo_00bf6713();
      }

      Raw inner_node = raw_from_ptr(inner_iter[1]);
      void* marker_inst = inner_node.ptr(0x14);
      if (marker_inst != nullptr) {
        host.vcall_scalar_deleting_dtor_vslot00(marker_inst, 1u);
        inner_node.ptr(0x14) = nullptr;
      }

      host.stl_tree_iter_inc_006d7d20(inner_iter);
      outer_owner = outer_iter[0];
    }

    void* out_erase_iter[2] = {nullptr, nullptr};
    host.stl_tree_erase_006dd1f0(
        outer_tree, out_erase_iter, outer_iter[0], outer_iter[1]);
  }
}

// 006de3d0 | BSP_MarkerManager_BindUnitMarkerStub
// Original ABI: __thiscall(void* self, uint32_t arg0, uint32_t arg1) -> void, RET 0x8.
void marker_manager_bind_unit_marker_stub_006de3d0(
    void* /*self*/, std::uint32_t /*arg0*/, std::uint32_t /*arg1*/) {}

// 006de3e0 | TRIV_body_006de3e0
// Original ABI: __thiscall(void* self, uint32_t arg0, float x, float y, float z) -> void, RET 0x10.
void triv_body_006de3e0(
    void* /*self*/,
    std::uint32_t /*arg0*/,
    float /*x*/,
    float /*y*/,
    float /*z*/) {}

// 006de4c0 | BSP_MarkerManager_ClearPositionMarker
// Original ABI: __thiscall(void* self, const void* name_str, float x, float y, float z) -> void, RET 0x10.
void marker_manager_clear_position_marker_006de4c0(
    Seg31_32Host& host,
    void* self,
    const void* name_str,
    float x,
    float y,
    float z) {
  Raw s = raw_from_ptr(self);
  float point[3] = {x, y, z};
  void* inner_map = host.stl_map_subscript_006de150(s.p + 0x20, point);
  void** slot =
      static_cast<void**>(host.stl_map_subscript_006dbd40(inner_map, name_str));
  if (*slot != nullptr) {
    host.vcall_scalar_deleting_dtor_vslot00(*slot, 1u);
    *slot = nullptr;
  }
}

// 006deac0 | BSP_MarkerManager_Destruct
// Original ABI: __thiscall(void* self) -> void, RET.
void marker_manager_destruct_006deac0(Seg31_32Host& host, void* self) {
  Raw s = raw_from_ptr(self);
  s.ptr(0x00) = host.image_address(0x00cf8ff8u);

  while (s.at<std::uint32_t>(0x34) != 0u) {
    void* head = s.ptr(0x30);
    void* first = raw_from_ptr(head).ptr(0x00);
    if (first == head) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    marker_manager_remove_gui_highlight_006dc5a0(
        host, self, raw_from_ptr(first).ptr(0x0c));
  }

  while (s.at<std::uint32_t>(0x40) != 0u) {
    void* head = s.ptr(0x3c);
    void* first = raw_from_ptr(head).ptr(0x00);
    if (first == head) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    marker_manager_remove_gui_highlight_006dc5a0(
        host, self, raw_from_ptr(first).ptr(0x0c));
  }

  while (s.at<std::uint32_t>(0x4c) != 0u) {
    void* head = s.ptr(0x48);
    void* first = raw_from_ptr(head).ptr(0x00);
    if (first == head) {
      host.invalid_parameter_noinfo_00bf6713();
    }
    marker_manager_remove_gui_highlight_006dc5a0(
        host, self, raw_from_ptr(first).ptr(0x0c));
  }

  void* out_iter[2] = {nullptr, nullptr};

  void* tree44 = s.p + 0x44;
  void* head44 = s.ptr(0x48);
  void* first44 = raw_from_ptr(head44).ptr(0x00);
  host.stl_tree_erase_range_006da090(
      tree44, out_iter, tree44, first44, tree44, head44);
  host.free_00bf65ac(s.ptr(0x48));
  s.ptr(0x48) = nullptr;
  s.at<std::uint32_t>(0x4c) = 0u;

  void* tree38 = s.p + 0x38;
  void* head38 = s.ptr(0x3c);
  void* first38 = raw_from_ptr(head38).ptr(0x00);
  host.stl_tree_erase_range_006da090(
      tree38, out_iter, tree38, first38, tree38, head38);
  host.free_00bf65ac(s.ptr(0x3c));
  s.ptr(0x3c) = nullptr;
  s.at<std::uint32_t>(0x40) = 0u;

  void* tree2c = s.p + 0x2c;
  void* head2c = s.ptr(0x30);
  void* first2c = raw_from_ptr(head2c).ptr(0x00);
  host.stl_tree_erase_range_006da090(
      tree2c, out_iter, tree2c, first2c, tree2c, head2c);
  host.free_00bf65ac(s.ptr(0x30));
  s.ptr(0x30) = nullptr;
  s.at<std::uint32_t>(0x34) = 0u;

  void* tree20 = s.p + 0x20;
  void* head20 = s.ptr(0x24);
  void* first20 = raw_from_ptr(head20).ptr(0x00);
  host.stl_tree_erase_range_006de300(
      tree20, out_iter, tree20, first20, tree20, head20);
  host.free_00bf65ac(s.ptr(0x24));
  s.ptr(0x24) = nullptr;
  s.at<std::uint32_t>(0x28) = 0u;

  void* tree14 = s.p + 0x14;
  void* head14 = s.ptr(0x18);
  void* first14 = raw_from_ptr(head14).ptr(0x00);
  host.stl_tree_erase_range_006ddf00(
      tree14, out_iter, tree14, first14, tree14, head14);
  host.free_00bf65ac(s.ptr(0x18));
  s.ptr(0x18) = nullptr;
  s.at<std::uint32_t>(0x1c) = 0u;

  host.callback_owner_destroy_00695870(self);
}

}  // namespace bsp::airfield_marker_seg31_32_argon
