#include "bsp/hardware_options_seg66_argon.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::hardware_options_seg66_argon {

namespace {

inline Raw raw_mut(void* p) noexcept {
  return Raw{static_cast<std::uint8_t*>(p)};
}

inline Raw raw_const(const void* p) noexcept {
  return Raw{const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(p))};
}

}  // namespace

// 0x0098c7f0: BSP_HardwareInfo_CpuSpeed
std::uint32_t hardware_info_cpu_speed_0098c7f0(const void* self) {
  return raw_const(self).at<std::uint32_t>(0x30);
}

// 0x0098c800: BSP_HardwareInfo_MemorySize
std::uint32_t hardware_info_memory_size_0098c800(const void* self) {
  return raw_const(self).at<std::uint32_t>(0x2c);
}

// 0x0098c810: TRIV_body_0098c810
void* hardware_info_display_options_ptr_0098c810(void* self) {
  return raw_mut(self).p + 0x234;
}

// 0x0098d3d0: BSP_HardwareInfo_SoundDeviceName
const char* hardware_info_sound_device_name_0098d3d0(Seg66Host& host) {
  return static_cast<const char*>(host.image_address(0x00f8a0e8u));
}

// 0x0098da40: TRIV_body_0098da40
void std_locale_facet_dtor_0098da40(Seg66Host& host, void* self) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d692b8u);
}

// 0x0098db80: CG_scalar_deleting_dtor_0098db80
void* cg_scalar_deleting_dtor_0098db80(Seg66Host& host, void* self, std::uint8_t flags) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d692b8u);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098dbd0: TRIV_body_0098dbd0
void std_locale_facet_ctor_0098dbd0(Seg66Host& host, void* self) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d692b8u);
}

// 0x0098dc50: TRIV_body_0098dc50
void std_codecvt_base_dtor_0098dc50(Seg66Host& host, void* self) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d692b8u);
}

// 0x0098dc60: CG_scalar_deleting_dtor_0098dc60
void* cg_scalar_deleting_dtor_0098dc60(Seg66Host& host, void* self, std::uint8_t flags) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d692b8u);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098dde0: CG_scalar_deleting_dtor_0098dde0
void* cg_scalar_deleting_dtor_0098dde0(Seg66Host& host, void* self, std::uint8_t flags) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d1d208u);
  host.crt_ios_base_dtor_00bf5ec3(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098de50: FUN_0098de50
void* std_locale_scalar_deleting_dtor_0098de50(Seg66Host& host, void* self, std::uint8_t flags) {
  Raw s = raw_mut(self);
  void* ptr = s.ptr(0x00);
  if (ptr != nullptr) {
    std::uint32_t lockit = 0;
    host.crt_lockit_ctor_00bf5993(&lockit, 0);
    Raw p = raw_mut(ptr);
    std::uint32_t refs = p.at<std::uint32_t>(0x04);
    if (refs != 0u && refs != 0xffffffffu) {
      p.at<std::uint32_t>(0x04) = refs - 1u;
    }
    void* to_delete = (p.at<std::uint32_t>(0x04) != 0u) ? nullptr : ptr;
    host.crt_lockit_dtor_00bf59b4(&lockit);
    if (to_delete != nullptr) {
      host.invoke_vtable_00_scalar_deleting_dtor(to_delete, 1u);
    }
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098e0e0: BSP_HardwareInfo_SupportsDisplayMode
bool hardware_info_supports_display_mode_0098e0e0(
    Seg66Host& host, const void* self, std::int32_t width, std::int32_t height) {
  Raw s = raw_const(self);
  std::uint32_t index = 0u;
  std::uint32_t count = host.stl_vector_size_0098d610(self);
  if (count != 0u) {
    do {
      std::uint8_t* first = s.at<std::uint8_t*>(0x04);
      std::uint8_t* last = s.at<std::uint8_t*>(0x08);
      if (first == nullptr ||
          static_cast<std::uint32_t>((last - first) >> 3) <= index) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      first = s.at<std::uint8_t*>(0x04);
      if (Raw{first}.at<std::int32_t>(index * 8u) == width) {
        last = s.at<std::uint8_t*>(0x08);
        if (first == nullptr ||
            static_cast<std::uint32_t>((last - first) >> 3) <= index) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
        first = s.at<std::uint8_t*>(0x04);
        if (Raw{first}.at<std::int32_t>(index * 8u + 4u) == height) {
          return true;
        }
      }
      ++index;
      count = host.stl_vector_size_0098d610(self);
    } while (index < count);
  }
  return false;
}

// 0x0098e170: CG_scalar_deleting_dtor_0098e170
void* cg_scalar_deleting_dtor_0098e170(Seg66Host& host, void* self, std::uint8_t flags) {
  raw_mut(self).ptr(0x00) = host.image_address(0x00d1d208u);
  host.crt_ios_base_dtor_00bf5ec3(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098e4e0: FUN_0098e4e0
void std_basic_streambuf_dtor_0098e4e0(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  void* ploc = s.ptr(0x38);
  s.ptr(0x00) = host.image_address(0x00d1d214u);
  if (ploc != nullptr) {
    void* locimp = raw_mut(ploc).ptr(0x00);
    if (locimp != nullptr) {
      std::uint32_t lockit = 0;
      host.crt_lockit_ctor_00bf5993(&lockit, 0);
      Raw imp = raw_mut(locimp);
      std::uint32_t refs = imp.at<std::uint32_t>(0x04);
      if (refs != 0u && refs != 0xffffffffu) {
        imp.at<std::uint32_t>(0x04) = refs - 1u;
      }
      void* to_delete = (imp.at<std::uint32_t>(0x04) != 0u) ? nullptr : locimp;
      host.crt_lockit_dtor_00bf59b4(&lockit);
      if (to_delete != nullptr) {
        host.invoke_vtable_00_scalar_deleting_dtor(to_delete, 1u);
      }
    }
    host.free_00bf65ac(ploc);
  }
  host.crt_mutex_dtor_00bf5f19(s.p + 0x04);
}

// 0x0098e890: CG_scalar_deleting_dtor_0098e890
void* cg_scalar_deleting_dtor_0098e890(Seg66Host& host, void* self, std::uint8_t flags) {
  std_basic_streambuf_dtor_0098e4e0(host, self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098f180: FUN_0098f180
void* std_basic_streambuf_ctor_0098f180(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d214u);
  host.crt_mutex_ctor_00bf5f01(s.p + 0x04);
  void* ploc = host.operator_new_00bf681b(4u);
  if (ploc != nullptr) {
    raw_mut(ploc).ptr(0x00) = host.crt_locale_init_00bf5cc2();
    void* glob_loc = host.crt_get_global_locale_00bf5acc();
    std::uint32_t lockit = 0;
    host.crt_lockit_ctor_00bf5993(&lockit, 0);
    Raw g = raw_mut(glob_loc);
    if (g.at<std::uint32_t>(0x04) != 0xffffffffu) {
      g.at<std::uint32_t>(0x04) = g.at<std::uint32_t>(0x04) + 1u;
    }
    host.crt_lockit_dtor_00bf59b4(&lockit);
  } else {
    ploc = nullptr;
  }
  s.ptr(0x38) = ploc;
  host.stl_basic_streambuf_init_0098e250(self);
  return self;
}

// 0x0098f950: CG_scalar_deleting_dtor_0098f950
void* cg_scalar_deleting_dtor_0098f950(Seg66Host& host, void* self, std::uint8_t flags) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d46cu);
  if (s.at<std::uint32_t>(0x24) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x10));
  }
  s.at<std::uint32_t>(0x24) = 0xfu;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint8_t>(0x10) = 0u;
  host.allocator_exception_base_cleanup_00bf6454(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x0098f9a0: FUN_0098f9a0
void* std_string_append_chars_0098f9a0(
    Seg66Host& host, void* self, std::uint32_t count, char ch) {
  Raw s = raw_mut(self);
  std::uint32_t cur_size = s.at<std::uint32_t>(0x14);
  if (0xffffffffu - cur_size <= count) {
    host.crt_xlen_00bf5695();
  }
  if (count != 0u) {
    std::uint32_t new_size = s.at<std::uint32_t>(0x14) + count;
    if (new_size > 0xfffffffeu) {
      host.crt_xlen_00bf5695();
    }
    std::uint32_t cap = s.at<std::uint32_t>(0x18);
    if (cap < new_size) {
      host.legacy_sbo_string_grow_allocation_004089e0(
          self, new_size, s.at<std::uint32_t>(0x14));
    } else if (new_size == 0u) {
      s.at<std::uint32_t>(0x14) = 0u;
      if (cap >= 0x10u) {
        *static_cast<char*>(s.ptr(0x04)) = '\0';
        return self;
      }
      s.at<char>(0x04) = '\0';
      return self;
    }
    if (new_size != 0u) {
      host.stl_string_chassign_0098e1b0(self, s.at<std::uint32_t>(0x14), count, ch);
      s.at<std::uint32_t>(0x14) = new_size;
      if (s.at<std::uint32_t>(0x18) >= 0x10u) {
        static_cast<char*>(s.ptr(0x04))[new_size] = '\0';
        return self;
      }
      s.at<char>(0x04u + new_size) = '\0';
    }
  }
  return self;
}

// 0x0098fa90: FUN_0098fa90
void* std_string_assign_chars_0098fa90(
    Seg66Host& host, void* self, std::uint32_t count, char ch) {
  Raw s = raw_mut(self);
  if (count == 0xffffffffu) {
    host.crt_xlen_00bf5695();
    host.crt_xlen_00bf5695();
  } else if (count > 0xfffffffeu) {
    host.crt_xlen_00bf5695();
  }
  std::uint32_t cap = s.at<std::uint32_t>(0x18);
  if (cap < count) {
    host.legacy_sbo_string_grow_allocation_004089e0(self, count, s.at<std::uint32_t>(0x14));
  } else if (count == 0u) {
    s.at<std::uint32_t>(0x14) = 0u;
    if (cap >= 0x10u) {
      *static_cast<char*>(s.ptr(0x04)) = '\0';
      return self;
    }
    s.at<char>(0x04) = '\0';
    return self;
  }
  if (count != 0u) {
    host.stl_string_chassign_0098e1b0(self, 0u, count, ch);
    s.at<std::uint32_t>(0x14) = count;
    if (s.at<std::uint32_t>(0x18) >= 0x10u) {
      static_cast<char*>(s.ptr(0x04))[count] = '\0';
      return self;
    }
    s.at<char>(0x04u + count) = '\0';
  }
  return self;
}

// 0x0098fb50: FUN_0098fb50
void std_locinfo_dtor_wrapper_0098fb50(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  host.crt_locinfo_dtor_00bf5bfd(self);
  if (s.at<std::uint32_t>(0x70) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x5c));
  }
  s.at<std::uint32_t>(0x70) = 0xfu;
  s.at<std::uint32_t>(0x6c) = 0u;
  s.at<std::uint8_t>(0x5c) = 0u;
  if (s.at<std::uint32_t>(0x54) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x40));
  }
  s.at<std::uint32_t>(0x54) = 0xfu;
  s.at<std::uint32_t>(0x50) = 0u;
  s.at<std::uint8_t>(0x40) = 0u;
  if (s.at<std::uint32_t>(0x38) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x24));
  }
  s.at<std::uint32_t>(0x38) = 0xfu;
  s.at<std::uint32_t>(0x34) = 0u;
  s.at<std::uint8_t>(0x24) = 0u;
  if (s.at<std::uint32_t>(0x1c) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x08));
  }
  s.at<std::uint32_t>(0x1c) = 0xfu;
  s.at<std::uint32_t>(0x18) = 0u;
  s.at<std::uint8_t>(0x08) = 0u;
  host.crt_lockit_dtor_00bf59b4(self);
}

// 0x0098fc20: FUN_0098fc20
std::uint32_t std_basic_filebuf_uflow_0098fc20(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  std::uint8_t** igfirst = s.at<std::uint8_t**>(0x20);
  if (*igfirst != nullptr) {
    std::int32_t* igcount = s.at<std::int32_t*>(0x30);
    std::uint8_t* cur = *igfirst;
    std::uint8_t* end = cur + *igcount;
    if (cur < end) {
      *igcount = *igcount - 1;
      std::uint8_t* p = *igfirst;
      *igfirst = p + 1;
      return static_cast<std::uint32_t>(*p);
    }
  }
  void* file = s.ptr(0x4c);
  if (file == nullptr) {
    return 0xffffffffu;
  }
  if (s.ptr(0x3c) == nullptr) {
    std::int32_t ch = host.crt_getc_00bfc5f7(file);
    if (ch == -1) {
      return 0xffffffffu;
    }
    return static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch));
  }

  alignas(4) std::uint8_t str_buf[28] = {};
  Raw str = raw_mut(str_buf);
  str.at<std::uint32_t>(0x18) = 0xfu;
  str.at<std::uint32_t>(0x14) = 0u;
  str.at<std::uint8_t>(0x04) = 0u;

  std::int32_t ch = host.crt_getc_00bfc5f7(file);
  while (ch != -1) {
    std_string_append_chars_0098f9a0(host, str_buf, 1u, static_cast<char>(ch));

    // Checked-iterator validation 1 & 2 (0098fd2b, 0098fd56)
    char* p_end_base = nullptr;
    std::uint32_t cap1 = str.at<std::uint32_t>(0x18);
    if (cap1 < 0x10u) {
      p_end_base = reinterpret_cast<char*>(str.p + 0x04);
    } else {
      p_end_base = static_cast<char*>(str.ptr(0x04));
      if (p_end_base == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }
    char* base1 = (str.at<std::uint32_t>(0x18) < 0x10u)
                      ? reinterpret_cast<char*>(str.p + 0x04)
                      : static_cast<char*>(str.ptr(0x04));
    if (p_end_base < base1 || p_end_base > base1 + str.at<std::uint32_t>(0x14)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
      char* base1b = (str.at<std::uint32_t>(0x18) < 0x10u)
                         ? reinterpret_cast<char*>(str.p + 0x04)
                         : static_cast<char*>(str.ptr(0x04));
      if (p_end_base >= base1b + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }

    // Checked-iterator validation 3 & 4 (0098fd98, 0098fdc1)
    std::uint32_t str_len = str.at<std::uint32_t>(0x14);
    char* p_begin = nullptr;
    if (str.at<std::uint32_t>(0x18) < 0x10u) {
      p_begin = reinterpret_cast<char*>(str.p + 0x04);
    } else {
      p_begin = static_cast<char*>(str.ptr(0x04));
      if (p_begin == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }
    char* base2 = (str.at<std::uint32_t>(0x18) < 0x10u)
                      ? reinterpret_cast<char*>(str.p + 0x04)
                      : static_cast<char*>(str.ptr(0x04));
    if (p_begin < base2 || p_begin > base2 + str.at<std::uint32_t>(0x14)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
      char* base2b = (str.at<std::uint32_t>(0x18) < 0x10u)
                         ? reinterpret_cast<char*>(str.p + 0x04)
                         : static_cast<char*>(str.ptr(0x04));
      if (p_begin >= base2b + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }

    char out_ch = '\0';
    const char* mid1 = nullptr;
    char* mid2 = nullptr;
    std::int32_t cvt_res = host.invoke_vtable_10_codecvt_in(
        s.ptr(0x3c),
        s.p + 0x44,
        p_begin,
        p_end_base + str_len,
        &mid1,
        &out_ch,
        &out_ch + 1,
        &mid2);
    if (cvt_res < 0) {
      break;
    }
    if (cvt_res <= 1) {
      if (mid2 != &out_ch) {
        std::uint8_t iter_buf[12] = {};
        void* it = host.stl_string_begin_0098f490(str_buf, iter_buf);
        const char* deref_p = host.stl_string_const_iterator_deref_0098f0c0(it);
        std::ptrdiff_t n =
            (deref_p + static_cast<std::ptrdiff_t>(str.at<std::uint32_t>(0x14))) - mid1;
        while (n > 0) {
          std::int32_t unget_ch = static_cast<std::int8_t>(mid1[n - 1]);
          --n;
          host.crt_ungetc_00bfcc1b(unget_ch, s.ptr(0x4c));
        }
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return static_cast<std::uint32_t>(static_cast<std::uint8_t>(out_ch));
      }

      // Checked-iterator validation 5 & 6 (0098feb6, 0098fedf)
      char* p_erase_base = nullptr;
      if (str.at<std::uint32_t>(0x18) < 0x10u) {
        p_erase_base = reinterpret_cast<char*>(str.p + 0x04);
      } else {
        p_erase_base = static_cast<char*>(str.ptr(0x04));
        if (p_erase_base == nullptr) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      char* base3 = (str.at<std::uint32_t>(0x18) < 0x10u)
                        ? reinterpret_cast<char*>(str.p + 0x04)
                        : static_cast<char*>(str.ptr(0x04));
      if (p_erase_base < base3 || p_erase_base > base3 + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
        char* base3b = (str.at<std::uint32_t>(0x18) < 0x10u)
                           ? reinterpret_cast<char*>(str.p + 0x04)
                           : static_cast<char*>(str.ptr(0x04));
        if (p_erase_base >= base3b + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      host.legacy_sbo_string_erase_004087f0(
          str_buf, 0u, static_cast<std::uint32_t>(mid1 - p_erase_base));
    } else {
      if (cvt_res != 3) {
        break;
      }
      if (str.at<std::uint32_t>(0x14) >= 1u) {
        std::uint8_t iter_buf[8] = {};
        void* it = host.stl_string_begin_0098f490(str_buf, iter_buf);
        const char* deref_p = host.stl_string_const_iterator_deref_0098f0c0(it);
        host.crt_memcpy_s_00bf672c(&out_ch, 1u, deref_p, 1u);
        std::uint32_t ret_val = static_cast<std::uint8_t>(out_ch);
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return ret_val;
      }
    }
    ch = host.crt_getc_00bfc5f7(s.ptr(0x4c));
  }
  host.legacy_sbo_string_destroy_004072d0(str_buf);
  return 0xffffffffu;
}

// 0x0098ffe0: FUN_0098ffe0
bool std_basic_filebuf_endwrite_0098ffe0(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  if (s.ptr(0x3c) == nullptr || s.at<std::uint8_t>(0x41) == 0u) {
    return true;
  }
  if (host.invoke_vtable_04_overflow(self, -1) == -1) {
    return false;
  }

  alignas(4) std::uint8_t str_buf[28] = {};
  Raw str = raw_mut(str_buf);
  str.at<std::uint32_t>(0x18) = 0xfu;
  str.at<std::uint32_t>(0x04) = 0u;
  str.at<std::uint32_t>(0x08) = 0u;
  str.at<std::uint32_t>(0x14) = 8u;
  str.at<std::uint8_t>(0x0c) = 0u;

  for (;;) {
    // Checked-iterator validation 1 & 2 (009900a6, 009900d1)
    char* p_end_base = nullptr;
    if (str.at<std::uint32_t>(0x18) < 0x10u) {
      p_end_base = reinterpret_cast<char*>(str.p + 0x04);
    } else {
      p_end_base = static_cast<char*>(str.ptr(0x04));
      if (p_end_base == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }
    char* base1 = (str.at<std::uint32_t>(0x18) < 0x10u)
                      ? reinterpret_cast<char*>(str.p + 0x04)
                      : static_cast<char*>(str.ptr(0x04));
    if (p_end_base < base1 || p_end_base > base1 + str.at<std::uint32_t>(0x14)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
      char* base1b = (str.at<std::uint32_t>(0x18) < 0x10u)
                         ? reinterpret_cast<char*>(str.p + 0x04)
                         : static_cast<char*>(str.ptr(0x04));
      if (p_end_base >= base1b + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }

    // Checked-iterator validation 3 & 4 (00990119, 00990142)
    std::uint32_t str_len = str.at<std::uint32_t>(0x14);
    char* p_begin = nullptr;
    if (str.at<std::uint32_t>(0x18) < 0x10u) {
      p_begin = reinterpret_cast<char*>(str.p + 0x04);
    } else {
      p_begin = static_cast<char*>(str.ptr(0x04));
      if (p_begin == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }
    char* base2 = (str.at<std::uint32_t>(0x18) < 0x10u)
                      ? reinterpret_cast<char*>(str.p + 0x04)
                      : static_cast<char*>(str.ptr(0x04));
    if (p_begin < base2 || p_begin > base2 + str.at<std::uint32_t>(0x14)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
      char* base2b = (str.at<std::uint32_t>(0x18) < 0x10u)
                         ? reinterpret_cast<char*>(str.p + 0x04)
                         : static_cast<char*>(str.ptr(0x04));
      if (p_begin >= base2b + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }

    char* mid2 = nullptr;
    std::int32_t res = host.invoke_vtable_18_codecvt_unshift(
        s.ptr(0x3c), s.p + 0x44, p_begin, p_end_base + str_len, &mid2);
    if (res == 0) {
      s.at<std::uint8_t>(0x41) = 0u;
    } else if (res != 1) {
      if (res == 3) {
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return true;
      }
      host.legacy_sbo_string_destroy_004072d0(str_buf);
      return false;
    }

    // Checked-iterator validation 5 & 6 (009901e0, 0099020b)
    char* p_count_base = nullptr;
    if (str.at<std::uint32_t>(0x18) < 0x10u) {
      p_count_base = reinterpret_cast<char*>(str.p + 0x04);
    } else {
      p_count_base = static_cast<char*>(str.ptr(0x04));
      if (p_count_base == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }
    char* base3 = (str.at<std::uint32_t>(0x18) < 0x10u)
                      ? reinterpret_cast<char*>(str.p + 0x04)
                      : static_cast<char*>(str.ptr(0x04));
    if (p_count_base < base3 || p_count_base > base3 + str.at<std::uint32_t>(0x14)) {
      host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
      char* base3b = (str.at<std::uint32_t>(0x18) < 0x10u)
                         ? reinterpret_cast<char*>(str.p + 0x04)
                         : static_cast<char*>(str.ptr(0x04));
      if (p_count_base >= base3b + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
    }

    std::uint32_t n = static_cast<std::uint32_t>(mid2 - p_count_base);
    if (n != 0u) {
      // Checked-iterator validation 7 & 8 (00990253, 0099027c)
      char* p_write_base = nullptr;
      if (str.at<std::uint32_t>(0x18) < 0x10u) {
        p_write_base = reinterpret_cast<char*>(str.p + 0x04);
      } else {
        p_write_base = static_cast<char*>(str.ptr(0x04));
        if (p_write_base == nullptr) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      char* base4 = (str.at<std::uint32_t>(0x18) < 0x10u)
                        ? reinterpret_cast<char*>(str.p + 0x04)
                        : static_cast<char*>(str.ptr(0x04));
      if (p_write_base < base4 || p_write_base > base4 + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
        char* base4b = (str.at<std::uint32_t>(0x18) < 0x10u)
                           ? reinterpret_cast<char*>(str.p + 0x04)
                           : static_cast<char*>(str.ptr(0x04));
        if (p_write_base >= base4b + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }

      std::uint32_t written = host.crt_fwrite_00bfb08b(p_write_base, 1u, n, s.ptr(0x4c));
      if (written != n) {
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return false;
      }
    }

    if (s.at<std::uint8_t>(0x41) == 0u) {
      host.legacy_sbo_string_destroy_004072d0(str_buf);
      return true;
    }
    if (n == 0u) {
      std_string_append_chars_0098f9a0(host, str_buf, 8u, '\0');
    }
  }
}

// 0x00990340: FUN_00990340
std::int32_t std_basic_filebuf_overflow_00990340(
    Seg66Host& host, void* self, std::int32_t meta) {
  if (meta == -1) {
    return 0;
  }
  Raw s = raw_mut(self);
  char ch = static_cast<char>(meta);
  char** ipnext = s.at<char**>(0x24);
  if (*ipnext != nullptr) {
    std::int32_t* ipcount = s.at<std::int32_t*>(0x34);
    char* cur = *ipnext;
    if (cur < cur + *ipcount) {
      *ipcount = *ipcount - 1;
      char* p = *ipnext;
      *ipnext = p + 1;
      *p = ch;
      return meta;
    }
  }
  void* file = s.ptr(0x4c);
  if (file == nullptr) {
    return -1;
  }
  if (s.ptr(0x3c) == nullptr) {
    if (host.crt_fputc_00bfc87f(static_cast<std::int8_t>(ch), file) != -1) {
      return meta;
    }
    return -1;
  }

  alignas(4) std::uint8_t str_buf[28] = {};
  Raw str = raw_mut(str_buf);
  str.at<std::uint32_t>(0x18) = 0xfu;
  str.at<std::uint32_t>(0x04) = 0u;
  str.at<std::uint32_t>(0x08) = 0u;
  str.at<std::uint32_t>(0x14) = 8u;
  str.at<std::uint8_t>(0x0c) = 0u;

  for (;;) {
    std::uint32_t n = 0u;
    do {
      // Checked-iterator validation 1 & 2 (00990460, 0099048b)
      char* p_end_base = nullptr;
      if (str.at<std::uint32_t>(0x18) < 0x10u) {
        p_end_base = reinterpret_cast<char*>(str.p + 0x04);
      } else {
        p_end_base = static_cast<char*>(str.ptr(0x04));
        if (p_end_base == nullptr) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      char* base1 = (str.at<std::uint32_t>(0x18) < 0x10u)
                        ? reinterpret_cast<char*>(str.p + 0x04)
                        : static_cast<char*>(str.ptr(0x04));
      if (p_end_base < base1 || p_end_base > base1 + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
        char* base1b = (str.at<std::uint32_t>(0x18) < 0x10u)
                           ? reinterpret_cast<char*>(str.p + 0x04)
                           : static_cast<char*>(str.ptr(0x04));
        if (p_end_base >= base1b + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }

      // Checked-iterator validation 3 & 4 (009904cd, 009904f6)
      std::uint32_t str_len = str.at<std::uint32_t>(0x14);
      char* p_begin = nullptr;
      if (str.at<std::uint32_t>(0x18) < 0x10u) {
        p_begin = reinterpret_cast<char*>(str.p + 0x04);
      } else {
        p_begin = static_cast<char*>(str.ptr(0x04));
        if (p_begin == nullptr) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      char* base2 = (str.at<std::uint32_t>(0x18) < 0x10u)
                        ? reinterpret_cast<char*>(str.p + 0x04)
                        : static_cast<char*>(str.ptr(0x04));
      if (p_begin < base2 || p_begin > base2 + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
        char* base2b = (str.at<std::uint32_t>(0x18) < 0x10u)
                           ? reinterpret_cast<char*>(str.p + 0x04)
                           : static_cast<char*>(str.ptr(0x04));
        if (p_begin >= base2b + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }

      const char* mid1 = nullptr;
      char* mid2 = nullptr;
      std::int32_t res = host.invoke_vtable_14_codecvt_out(
          s.ptr(0x3c),
          s.p + 0x44,
          &ch,
          &ch + 1,
          &mid1,
          p_begin,
          p_end_base + str_len,
          &mid2);
      if (res < 0) {
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return -1;
      }
      if (res > 1) {
        if (res == 3) {
          if (!host.stl_fputc_0098f580(ch, s.ptr(0x4c))) {
            host.legacy_sbo_string_destroy_004072d0(str_buf);
            return -1;
          }
          host.legacy_sbo_string_destroy_004072d0(str_buf);
          return meta;
        }
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return -1;
      }

      // Checked-iterator validation 5 & 6 (00990571, 0099059c)
      char* p_count_base = nullptr;
      if (str.at<std::uint32_t>(0x18) < 0x10u) {
        p_count_base = reinterpret_cast<char*>(str.p + 0x04);
      } else {
        p_count_base = static_cast<char*>(str.ptr(0x04));
        if (p_count_base == nullptr) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }
      char* base3 = (str.at<std::uint32_t>(0x18) < 0x10u)
                        ? reinterpret_cast<char*>(str.p + 0x04)
                        : static_cast<char*>(str.ptr(0x04));
      if (p_count_base < base3 || p_count_base > base3 + str.at<std::uint32_t>(0x14)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
      }
      if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
        char* base3b = (str.at<std::uint32_t>(0x18) < 0x10u)
                           ? reinterpret_cast<char*>(str.p + 0x04)
                           : static_cast<char*>(str.ptr(0x04));
        if (p_count_base >= base3b + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
      }

      n = static_cast<std::uint32_t>(mid2 - p_count_base);
      if (n != 0u) {
        // Checked-iterator validation 7 & 8 (009905e8, 00990611)
        char* p_write_base = nullptr;
        if (str.at<std::uint32_t>(0x18) < 0x10u) {
          p_write_base = reinterpret_cast<char*>(str.p + 0x04);
        } else {
          p_write_base = static_cast<char*>(str.ptr(0x04));
          if (p_write_base == nullptr) {
            host.crt_invalid_parameter_noinfo_00bf6713();
          }
        }
        char* base4 = (str.at<std::uint32_t>(0x18) < 0x10u)
                          ? reinterpret_cast<char*>(str.p + 0x04)
                          : static_cast<char*>(str.ptr(0x04));
        if (p_write_base < base4 || p_write_base > base4 + str.at<std::uint32_t>(0x14)) {
          host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (reinterpret_cast<std::uintptr_t>(str.p) != static_cast<std::uintptr_t>(-2)) {
          char* base4b = (str.at<std::uint32_t>(0x18) < 0x10u)
                             ? reinterpret_cast<char*>(str.p + 0x04)
                             : static_cast<char*>(str.ptr(0x04));
          if (p_write_base >= base4b + str.at<std::uint32_t>(0x14)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
          }
        }

        std::uint32_t written = host.crt_fwrite_00bfb08b(p_write_base, 1u, n, s.ptr(0x4c));
        if (written != n) {
          host.legacy_sbo_string_destroy_004072d0(str_buf);
          return -1;
        }
      }

      s.at<std::uint8_t>(0x41) = 1u;
      if (mid1 != &ch) {
        host.legacy_sbo_string_destroy_004072d0(str_buf);
        return meta;
      }
    } while (n != 0u);

    if (str.at<std::uint32_t>(0x14) >= 0x20u) {
      host.legacy_sbo_string_destroy_004072d0(str_buf);
      return -1;
    }
    std_string_append_chars_0098f9a0(host, str_buf, 8u, '\0');
  }
}

// 0x00990740: std_basic_filebuf_seekoff_00990740
void* std_basic_filebuf_seekoff_00990740(
    Seg66Host& host,
    void* self,
    void* out_fpos,
    std::int32_t off_lo,
    std::int32_t off_hi,
    std::int32_t way,
    std::int32_t /*mode*/) {
  (void)way;
  Raw s = raw_mut(self);
  Raw out = raw_mut(out_fpos);
  void* pback_slot = s.p + 0x40;
  std::int32_t seek_off = off_hi;
  std::int32_t seek_way = off_lo;
  if (*s.at<void**>(0x20) == pback_slot && seek_way == 1 && s.ptr(0x3c) == nullptr) {
    seek_off -= 1;
  }
  std::uint32_t fpos_words[2] = {0u, 0u};
  if (s.ptr(0x4c) != nullptr && std_basic_filebuf_endwrite_0098ffe0(host, self) &&
      ((seek_off == 0 && seek_way == 1) ||
       host.crt_fseek_00bfb72b(s.ptr(0x4c), seek_off, seek_way) == 0) &&
      host.crt_fgetpos_00bfcf65(s.ptr(0x4c), fpos_words) == 0) {
    if (*s.at<void**>(0x20) == pback_slot) {
      *s.at<void**>(0x10) = pback_slot;
      *s.at<void**>(0x20) = s.p + 0x41;
      *s.at<std::int32_t*>(0x30) = 0;
    }
    out.at<std::uint32_t>(0x00) = 0u;
    out.at<std::uint32_t>(0x08) = fpos_words[0];
    out.at<std::uint32_t>(0x0c) = fpos_words[1];
    out.at<std::uint32_t>(0x10) = s.at<std::uint32_t>(0x44);
    return out_fpos;
  }
  out.at<std::uint32_t>(0x00) = host.global_u32(0x00d692c4u);
  out.at<std::uint32_t>(0x08) = 0u;
  out.at<std::uint32_t>(0x0c) = 0u;
  out.at<std::uint32_t>(0x10) = 0u;
  return out_fpos;
}

// 0x00990820: std_basic_filebuf_seekpos_00990820
void* std_basic_filebuf_seekpos_00990820(
    Seg66Host& host,
    void* self,
    void* out_fpos,
    std::int32_t pos_off_lo,
    std::int32_t /*pos_off_hi*/,
    std::uint32_t fpos_lo,
    std::uint32_t fpos_hi,
    std::uint32_t state_val,
    std::int32_t /*mode*/) {
  Raw s = raw_mut(self);
  Raw out = raw_mut(out_fpos);
  std::uint32_t fpos_words[2] = {fpos_lo, fpos_hi};
  if (s.ptr(0x4c) != nullptr && std_basic_filebuf_endwrite_0098ffe0(host, self) &&
      host.crt_fsetpos_00bfcfd0(s.ptr(0x4c), fpos_words) == 0 &&
      (pos_off_lo == 0 || host.crt_fseek_00bfb72b(s.ptr(0x4c), pos_off_lo, 1) == 0) &&
      host.crt_fgetpos_00bfcf65(s.ptr(0x4c), fpos_words) == 0) {
    s.at<std::uint32_t>(0x44) = state_val;
    void* pback_slot = s.p + 0x40;
    if (*s.at<void**>(0x20) == pback_slot) {
      *s.at<void**>(0x10) = pback_slot;
      *s.at<void**>(0x20) = s.p + 0x41;
      *s.at<std::int32_t*>(0x30) = 0;
    }
    out.at<std::uint32_t>(0x00) = 0u;
    out.at<std::uint32_t>(0x08) = fpos_words[0];
    out.at<std::uint32_t>(0x0c) = fpos_words[1];
    out.at<std::uint32_t>(0x10) = s.at<std::uint32_t>(0x44);
    return out_fpos;
  }
  out.at<std::uint32_t>(0x00) = host.global_u32(0x00d692c4u);
  out.at<std::uint32_t>(0x08) = 0u;
  out.at<std::uint32_t>(0x0c) = 0u;
  out.at<std::uint32_t>(0x10) = 0u;
  return out_fpos;
}

// 0x00990910: std_runtime_error_ctor_00990910
void* std_runtime_error_ctor_00990910(Seg66Host& host, void* self, const void* msg_str) {
  Raw s = raw_mut(self);
  host.allocator_exception_base_default_construct_00bf632f(self);
  s.ptr(0x00) = host.image_address(0x00d1d46cu);
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint32_t>(0x24) = 0xfu;
  s.at<std::uint8_t>(0x10) = 0u;
  host.legacy_sbo_string_assign_substring_00408120(s.p + 0x0c, msg_str, 0u, 0xffffffffu);
  return self;
}

// 0x00990970: std_basic_filebuf_close_00990970
void* std_basic_filebuf_close_00990970(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  void* result = self;
  if (s.ptr(0x4c) != nullptr) {
    if (!std_basic_filebuf_endwrite_0098ffe0(host, self)) {
      result = nullptr;
    }
    if (host.crt_fclose_00bf8081(s.ptr(0x4c)) != 0) {
      result = nullptr;
    }
  } else {
    result = nullptr;
  }
  s.at<std::uint8_t>(0x48) = 0u;
  s.at<std::uint8_t>(0x41) = 0u;
  host.stl_basic_streambuf_init_0098e250(self);
  s.ptr(0x4c) = nullptr;
  s.at<std::uint32_t>(0x44) = host.global_u32(0x00f8a2e8u);
  s.ptr(0x3c) = nullptr;
  return result;
}

// 0x009909e0: std_runtime_error_dtor_009909e0
void std_runtime_error_dtor_009909e0(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d46cu);
  if (s.at<std::uint32_t>(0x24) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x10));
  }
  s.at<std::uint32_t>(0x24) = 0xfu;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint8_t>(0x10) = 0u;
  host.allocator_exception_base_cleanup_00bf6454(self);
}

// 0x00990a20: CG_scalar_deleting_dtor_00990a20
void* cg_scalar_deleting_dtor_00990a20(Seg66Host& host, void* self, std::uint8_t flags) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d46cu);
  if (s.at<std::uint32_t>(0x24) >= 0x10u) {
    host.free_00bf65ac(s.ptr(0x10));
  }
  s.at<std::uint32_t>(0x24) = 0xfu;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint8_t>(0x10) = 0u;
  host.allocator_exception_base_cleanup_00bf6454(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00990a70: std_basic_filebuf_dtor_00990a70
void std_basic_filebuf_dtor_00990a70(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d434u);
  if (s.at<std::uint8_t>(0x48) != 0u) {
    std_basic_filebuf_close_00990970(host, self);
  }
  std_basic_streambuf_dtor_0098e4e0(host, self);
}

// 0x00990ad0: CG_scalar_deleting_dtor_00990ad0
void* cg_scalar_deleting_dtor_00990ad0(Seg66Host& host, void* self, std::uint8_t flags) {
  std_basic_filebuf_dtor_00990a70(host, self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00990b60: std_Locinfo_ctor_00990b60
void* std_locinfo_ctor_00990b60(Seg66Host& host, void* self, const char* locname) {
  Raw s = raw_mut(self);
  host.crt_lockit_ctor_00bf5993(self, 0);
  s.at<std::uint32_t>(0x1c) = 0xfu;
  s.at<std::uint32_t>(0x18) = 0u;
  s.at<std::uint8_t>(0x08) = 0u;
  s.at<std::uint32_t>(0x38) = 0xfu;
  s.at<std::uint32_t>(0x34) = 0u;
  s.at<std::uint8_t>(0x24) = 0u;
  s.at<std::uint32_t>(0x54) = 0xfu;
  s.at<std::uint32_t>(0x50) = 0u;
  s.at<std::uint8_t>(0x40) = 0u;
  s.at<std::uint32_t>(0x70) = 0xfu;
  s.at<std::uint32_t>(0x6c) = 0u;
  s.at<std::uint8_t>(0x5c) = 0u;
  if (locname == nullptr) {
    alignas(4) std::uint8_t msg_str[28] = {};
    Raw m = raw_mut(msg_str);
    m.at<std::uint32_t>(0x18) = 0xfu;
    m.at<std::uint32_t>(0x14) = 0u;
    m.at<std::uint8_t>(0x04) = 0u;
    host.character_string_assign_counted_00408720(
        msg_str, static_cast<const char*>(host.image_address(0x00d1d47cu)), 0xfu);
    alignas(4) std::uint8_t exc_buf[40] = {};
    std_runtime_error_ctor_00990910(host, exc_buf, msg_str);
    host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de16a4u));
  }
  host.crt_locinfo_ctor_00bf5d5c(self, locname);
  return self;
}

// 0x00990c20: std_runtime_error_copy_ctor_00990c20
void* std_runtime_error_copy_ctor_00990c20(Seg66Host& host, void* self, const void* other) {
  Raw s = raw_mut(self);
  Raw o = raw_const(other);
  host.crt_exception_copy_ctor_00bf63a6(self, other);
  s.ptr(0x00) = host.image_address(0x00d1d46cu);
  s.at<std::uint32_t>(0x24) = 0xfu;
  s.at<std::uint32_t>(0x20) = 0u;
  s.at<std::uint8_t>(0x10) = 0u;
  host.legacy_sbo_string_assign_substring_00408120(s.p + 0x0c, o.p + 0x0c, 0u, 0xffffffffu);
  return self;
}

// 0x00990c90: std_ctype_char_ctor_00990c90
void* std_ctype_char_ctor_00990c90(
    Seg66Host& host, void* self, const void* table, bool del_flag, std::uint32_t refs) {
  Raw s = raw_mut(self);
  s.at<std::uint32_t>(0x04) = refs;
  s.ptr(0x00) = host.image_address(0x00d1d48cu);
  alignas(4) std::uint8_t locinfo_buf[120] = {};
  std_locinfo_ctor_00990b60(
      host, locinfo_buf, static_cast<const char*>(host.image_address(0x00d08204u)));
  alignas(4) std::uint32_t ctype_vec[4] = {};
  void* cv = host.crt_getctype_00bf5879(ctype_vec);
  Raw cv_raw = raw_const(cv);
  s.at<std::uint32_t>(0x08) = cv_raw.at<std::uint32_t>(0x00);
  s.at<std::uint32_t>(0x0c) = cv_raw.at<std::uint32_t>(0x04);
  s.at<std::uint32_t>(0x10) = cv_raw.at<std::uint32_t>(0x08);
  s.at<std::uint32_t>(0x14) = cv_raw.at<std::uint32_t>(0x0c);
  std_locinfo_dtor_wrapper_0098fb50(host, locinfo_buf);
  if (table != nullptr) {
    std::int32_t delfl = s.at<std::int32_t>(0x14);
    if (delfl > 0) {
      host.free_00bf9dc8(s.ptr(0x10));
    } else if (delfl < 0) {
      host.free_00bf6989(s.ptr(0x10));
    }
    s.ptr(0x10) = const_cast<void*>(table);
    s.at<std::int32_t>(0x14) = del_flag ? -1 : 0;
  }
  return self;
}

// 0x00990d60: std_ctype_char_do_tolower_00990d60
char std_ctype_char_do_tolower_00990d60(Seg66Host& host, const void* self, char ch) {
  return static_cast<char>(host.crt_tolower_00bf576a(
      static_cast<std::uint8_t>(ch), raw_const(self).p + 0x08));
}

// 0x00990d80: std_ctype_char_do_tolower_range_00990d80
const char* std_ctype_char_do_tolower_range_00990d80(
    Seg66Host& host, const void* self, char* first, const char* last) {
  const void* ctype_vec = raw_const(self).p + 0x08;
  while (first != last) {
    *first = static_cast<char>(
        host.crt_tolower_00bf576a(static_cast<std::uint8_t>(*first), ctype_vec));
    ++first;
  }
  return first;
}

// 0x00990db0: std_ctype_char_do_toupper_00990db0
char std_ctype_char_do_toupper_00990db0(Seg66Host& host, const void* self, char ch) {
  return static_cast<char>(host.crt_toupper_00bf5f87(
      static_cast<std::uint8_t>(ch), raw_const(self).p + 0x08));
}

// 0x00990dd0: std_ctype_char_do_toupper_range_00990dd0
const char* std_ctype_char_do_toupper_range_00990dd0(
    Seg66Host& host, const void* self, char* first, const char* last) {
  const void* ctype_vec = raw_const(self).p + 0x08;
  while (first != last) {
    *first = static_cast<char>(
        host.crt_toupper_00bf5f87(static_cast<std::uint8_t>(*first), ctype_vec));
    ++first;
  }
  return first;
}

// 0x00990e00: TRIV_body_00990e00
char std_ctype_char_do_widen_00990e00(char ch) {
  return ch;
}

// 0x00990e80: TRIV_body_00990e80
char std_ctype_char_do_narrow_00990e80(char ch, char /*dflt*/) {
  return ch;
}

// 0x00990f00: std_ctype_char_dtor_00990f00
void std_ctype_char_dtor_00990f00(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d48cu);
  std::int32_t delfl = s.at<std::int32_t>(0x14);
  if (delfl > 0) {
    host.free_00bf9dc8(s.ptr(0x10));
    s.ptr(0x00) = host.image_address(0x00d692b8u);
    return;
  }
  if (delfl < 0) {
    host.free_00bf6989(s.ptr(0x10));
  }
  s.ptr(0x00) = host.image_address(0x00d692b8u);
}

// 0x00990f40: CG_scalar_deleting_dtor_00990f40
void* cg_scalar_deleting_dtor_00990f40(Seg66Host& host, void* self, std::uint8_t flags) {
  Raw s = raw_mut(self);
  s.ptr(0x00) = host.image_address(0x00d1d48cu);
  std::int32_t delfl = s.at<std::int32_t>(0x14);
  if (delfl > 0) {
    host.free_00bf9dc8(s.ptr(0x10));
  } else if (delfl < 0) {
    host.free_00bf6989(s.ptr(0x10));
  }
  s.ptr(0x00) = host.image_address(0x00d692b8u);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00990f90: std_ctype_char_Getcat_00990f90
std::uint32_t std_ctype_char_getcat_00990f90(Seg66Host& host, void** out_facet) {
  if (out_facet != nullptr && *out_facet == nullptr) {
    void* mem = host.operator_new_00bf681b(0x18u);
    if (mem != nullptr) {
      *out_facet = std_ctype_char_ctor_00990c90(host, mem, nullptr, false, 0u);
    } else {
      *out_facet = nullptr;
    }
  }
  return 2u;
}

// 0x00991000: std_ios_base_clear_00991000
void std_ios_base_clear_00991000(
    Seg66Host& host, void* self, std::uint32_t state, bool reraise) {
  Raw s = raw_mut(self);
  std::uint32_t masked = state & 0x17u;
  s.at<std::uint32_t>(0x08) = masked;
  std::uint32_t active = s.at<std::uint32_t>(0x0c) & masked;
  if (active == 0u) {
    return;
  }
  if (reraise) {
    host.cxx_throw_exception_00bf6885(nullptr, nullptr);
    return;
  }
  if ((active & 4u) != 0u) {
    alignas(4) std::uint8_t msg_str[28] = {};
    host.legacy_sbo_string_construct_from_cstring_00407290(
        msg_str, static_cast<const char*>(host.image_address(0x00d1d4e8u)));
    alignas(4) std::uint8_t exc_buf[40] = {};
    std_runtime_error_ctor_00990910(host, exc_buf, msg_str);
    raw_mut(exc_buf).ptr(0x00) = host.image_address(0x00d1d474u);
    host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de17b4u));
    return;
  }
  if ((active & 2u) != 0u) {
    alignas(4) std::uint8_t msg_str[28] = {};
    host.legacy_sbo_string_construct_from_cstring_00407290(
        msg_str, static_cast<const char*>(host.image_address(0x00d1d4d0u)));
    alignas(4) std::uint8_t exc_buf[40] = {};
    std_runtime_error_ctor_00990910(host, exc_buf, msg_str);
    raw_mut(exc_buf).ptr(0x00) = host.image_address(0x00d1d474u);
    host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de17b4u));
    return;
  }
  alignas(4) std::uint8_t msg_str[28] = {};
  host.legacy_sbo_string_construct_from_cstring_00407290(
      msg_str, static_cast<const char*>(host.image_address(0x00d1d4b8u)));
  alignas(4) std::uint8_t exc_buf[40] = {};
  std_runtime_error_ctor_00990910(host, exc_buf, msg_str);
  raw_mut(exc_buf).ptr(0x00) = host.image_address(0x00d1d474u);
  host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de17b4u));
}

// 0x00991150: FUN_00991150
void std_ios_base_init_00991150(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  s.ptr(0x24) = nullptr;
  s.at<std::uint32_t>(0x0c) = 0u;
  s.at<std::uint32_t>(0x10) = 0x201u;
  s.at<std::uint32_t>(0x14) = 6u;
  s.at<std::uint32_t>(0x18) = 0u;
  s.at<std::uint32_t>(0x1c) = 0u;
  s.at<std::uint32_t>(0x20) = 0u;
  std_ios_base_clear_00991000(host, self, 0u, false);
  void* ploc = host.operator_new_00bf681b(4u);
  if (ploc != nullptr) {
    raw_mut(ploc).ptr(0x00) = host.crt_locale_init_00bf5cc2();
    void* glob_loc = host.crt_get_global_locale_00bf5acc();
    std::uint32_t lockit = 0u;
    host.crt_lockit_ctor_00bf5993(&lockit, 0);
    Raw g = raw_mut(glob_loc);
    if (g.at<std::uint32_t>(0x04) != 0xffffffffu) {
      g.at<std::uint32_t>(0x04) = g.at<std::uint32_t>(0x04) + 1u;
    }
    host.crt_lockit_dtor_00bf59b4(&lockit);
    s.ptr(0x24) = ploc;
    return;
  }
  s.ptr(0x24) = nullptr;
}

// 0x009911f0: std_use_facet_009911f0
void* std_use_facet_ctype_char_009911f0(Seg66Host& host, const void* locale_obj) {
  std::uint32_t lockit_outer = 0u;
  host.crt_lockit_ctor_00bf5993(&lockit_outer, 0);
  void* cached_facet = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00f8a2ecu)));
  void* local_facet = cached_facet;
  if (host.global_u32(0x0109dcbcu) == 0u) {
    std::uint32_t lockit_inner = 0u;
    host.crt_lockit_ctor_00bf5993(&lockit_inner, 0);
    if (host.global_u32(0x0109dcbcu) == 0u) {
      std::uint32_t next_id = host.global_u32(0x0109dcb8u) + 1u;
      host.global_u32(0x0109dcb8u) = next_id;
      host.global_u32(0x0109dcbcu) = next_id;
    }
    host.crt_lockit_dtor_00bf59b4(&lockit_inner);
  }
  std::uint32_t id = host.global_u32(0x0109dcbcu);
  Raw loc_imp = raw_const(raw_const(locale_obj).ptr(0x00));
  void* found = nullptr;
  if (id < loc_imp.at<std::uint32_t>(0x0c)) {
    found = raw_const(loc_imp.ptr(0x08)).at<void*>(id * 4u);
    if (found != nullptr) {
      host.crt_lockit_dtor_00bf59b4(&lockit_outer);
      return found;
    }
  }
  if (loc_imp.at<std::uint8_t>(0x14) != 0u) {
    Raw glob_imp = raw_const(host.crt_get_global_locale_00bf5acc());
    if (id < glob_imp.at<std::uint32_t>(0x0c)) {
      found = raw_const(glob_imp.ptr(0x08)).at<void*>(id * 4u);
    }
  }
  if (found != nullptr) {
    host.crt_lockit_dtor_00bf59b4(&lockit_outer);
    return found;
  }
  if (cached_facet != nullptr) {
    host.crt_lockit_dtor_00bf59b4(&lockit_outer);
    return cached_facet;
  }
  if (std_ctype_char_getcat_00990f90(host, &local_facet) == 0xffffffffu) {
    alignas(4) std::uint8_t exc_buf[12] = {};
    host.crt_bad_cast_ctor_00bf6477(
        exc_buf, static_cast<const char*>(host.image_address(0x00d1d500u)));
    host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de182cu));
  }
  found = local_facet;
  host.global_u32(0x00f8a2ecu) =
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(found));
  host.stl_facet_incref_0098d9c0(found);
  host.crt_facet_register_00bf5bc4(found);
  host.crt_lockit_dtor_00bf59b4(&lockit_outer);
  return found;
}

// 0x00991310: std_basic_ifstream_dtor_00991310
void std_basic_ifstream_dtor_00991310(Seg66Host& host, void* vbase_self) {
  Raw vb = raw_mut(vbase_self);
  std::uint8_t* obj_base = vb.p - 0x58;
  std::int32_t vbase_off1 = raw_const(Raw{obj_base}.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{obj_base + vbase_off1}.ptr(0x00) = host.image_address(0x00d1d50cu);
  std::uint8_t* filebuf_ptr = vb.p - 0x50;
  std_basic_filebuf_dtor_00990a70(host, filebuf_ptr);
  std::int32_t vbase_off2 = raw_const(Raw{filebuf_ptr - 0x08}.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{filebuf_ptr - 0x08 + vbase_off2}.ptr(0x00) = host.image_address(0x00d1d20cu);
}

// 0x00991610: FUN_00991610
char std_basic_ios_widen_00991610(Seg66Host& host, void* self, char ch) {
  void* loc_imp = nullptr;
  void* loc_ptr = host.stl_ios_base_getloc_0098dd80(self, &loc_imp);
  void* facet = std_use_facet_ctype_char_009911f0(host, loc_ptr);
  if (loc_imp != nullptr) {
    std::uint32_t lockit = 0u;
    host.crt_lockit_ctor_00bf5993(&lockit, 0);
    Raw imp = raw_mut(loc_imp);
    std::uint32_t refs = imp.at<std::uint32_t>(0x04);
    if (refs != 0u && refs != 0xffffffffu) {
      imp.at<std::uint32_t>(0x04) = refs - 1u;
    }
    void* to_delete = (imp.at<std::uint32_t>(0x04) != 0u) ? nullptr : loc_imp;
    host.crt_lockit_dtor_00bf59b4(&lockit);
    if (to_delete != nullptr) {
      host.invoke_vtable_00_scalar_deleting_dtor(to_delete, 1u);
    }
  }
  return host.invoke_vtable_18_ctype_do_widen(facet, ch);
}

// 0x009916c0: std_facet_ctor_009916c0
void* std_facet_ctor_009916c0(Seg66Host& host, void* self, std::uint32_t refs) {
  Raw s = raw_mut(self);
  s.at<std::uint32_t>(0x04) = refs;
  s.ptr(0x00) = host.image_address(0x00d1d510u);
  alignas(4) std::uint8_t locinfo_buf[120] = {};
  std_locinfo_ctor_00990b60(
      host, locinfo_buf, static_cast<const char*>(host.image_address(0x00d08204u)));
  std_locinfo_dtor_wrapper_0098fb50(host, locinfo_buf);
  return self;
}

// 0x00991970: std_basic_ios_init_00991970
void std_basic_ios_init_00991970(
    Seg66Host& host, void* self, void* streambuf, bool is_std) {
  Raw s = raw_mut(self);
  std_ios_base_init_00991150(host, self);
  s.ptr(0x28) = streambuf;
  s.ptr(0x2c) = nullptr;
  s.at<char>(0x30) = std_basic_ios_widen_00991610(host, self, ' ');
  if (s.ptr(0x28) == nullptr) {
    std_ios_base_clear_00991000(host, self, s.at<std::uint32_t>(0x08) | 4u, false);
  }
  if (is_std) {
    host.crt_ios_base_addstd_00bf5e73(self);
    return;
  }
  s.at<std::uint32_t>(0x04) = 0u;
}

// 0x009919d0: std_facet_Getcat_009919d0
std::uint32_t std_facet_getcat_009919d0(Seg66Host& host, void** out_facet) {
  if (out_facet != nullptr && *out_facet == nullptr) {
    void* mem = host.operator_new_00bf681b(8u);
    if (mem != nullptr) {
      *out_facet = std_facet_ctor_009916c0(host, mem, 0u);
    } else {
      *out_facet = nullptr;
    }
  }
  return 2u;
}

// 0x00991a40: std_basic_ostream_flush_00991a40
void* std_basic_ostream_flush_00991a40(Seg66Host& host, void* self) {
  Raw s = raw_mut(self);
  std::int32_t vbase_off = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw ios_sub{s.p + vbase_off};
  std::uint32_t err = 0u;
  if ((ios_sub.at<std::uint8_t>(0x08) & 6u) == 0u) {
    if (host.invoke_vtable_30_streambuf_sync(ios_sub.ptr(0x28)) == -1) {
      err = 4u;
    }
  }
  if (err != 0u) {
    std::int32_t vbase_off2 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
    Raw ios_sub2{s.p + vbase_off2};
    std::uint32_t new_state = ios_sub2.at<std::uint32_t>(0x08) | err;
    if (ios_sub2.ptr(0x28) == nullptr) {
      new_state |= 4u;
    }
    std_ios_base_clear_00991000(host, ios_sub2.p, new_state, false);
  }
  return self;
}

// 0x00991a90: std_basic_istream_Ipfx_00991a90
bool std_basic_istream_ipfx_00991a90(Seg66Host& host, void* self, bool noskip) {
  Raw s = raw_mut(self);
  std::int32_t vbase_off = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw ios_sub{s.p + vbase_off};
  if (ios_sub.at<std::uint32_t>(0x08) == 0u) {
    void* tied = ios_sub.ptr(0x2c);
    if (tied != nullptr) {
      std_basic_ostream_flush_00991a40(host, tied);
    }
    std::int32_t vbase_off2 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
    Raw ios_sub2{s.p + vbase_off2};
    if (!noskip && (ios_sub2.at<std::uint32_t>(0x10) & 1u) != 0u) {
      void* loc_obj = nullptr;
      void* loc_ptr = host.stl_ios_base_getloc_0098dd80(ios_sub2.p, &loc_obj);
      void* facet = std_use_facet_ctype_char_009911f0(host, loc_ptr);
      host.stl_locale_dtor_0098daf0(&loc_obj);

      try {
        std::int32_t vbase_off3 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
        Raw sb = raw_mut(Raw{s.p + vbase_off3}.ptr(0x28));
        std::int32_t ch = 0;
        std::uint8_t** igfirst = sb.at<std::uint8_t**>(0x20);
        std::int32_t* igcount = sb.at<std::int32_t*>(0x30);
        if (*igfirst != nullptr && *igcount > 0) {
          ch = static_cast<std::int32_t>(**igfirst);
        } else {
          ch = host.invoke_vtable_10_uflow(sb.p);
        }
        while (ch != -1) {
          const std::uint16_t* table = raw_const(facet).at<const std::uint16_t*>(0x10);
          if ((table[static_cast<std::uint8_t>(ch)] & 0x48u) == 0u) {
            goto after_ws;
          }
          std::int32_t vbase_off_loop = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
          ch = host.stl_basic_streambuf_snextc_0098f3b0(Raw{s.p + vbase_off_loop}.ptr(0x28));
        }
        {
          std::int32_t vbase_off_eof = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
          Raw ios_eof{s.p + vbase_off_eof};
          std::uint32_t st = ios_eof.at<std::uint32_t>(0x08) | 1u;
          if (ios_eof.ptr(0x28) == nullptr) {
            st |= 4u;
          }
          std_ios_base_clear_00991000(host, ios_eof.p, st, false);
        }
      after_ws:;
      } catch (...) {
        std::int32_t vbase_off_catch = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
        Raw ios_catch{s.p + vbase_off_catch};
        std::uint32_t catch_st = (ios_catch.at<std::uint32_t>(0x08) | 4u) & 0x17u;
        ios_catch.at<std::uint32_t>(0x08) = catch_st;
        if ((ios_catch.at<std::uint32_t>(0x0c) & catch_st) != 0u) {
          host.cxx_throw_exception_00bf6885(nullptr, nullptr);
        }
      }
    }
    std::int32_t vbase_off_chk = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
    Raw ios_chk{s.p + vbase_off_chk};
    if (ios_chk.at<std::uint32_t>(0x08) == 0u) {
      return true;
    }
    ios_sub = ios_chk;
  }
  std::uint32_t fail_st = ios_sub.at<std::uint32_t>(0x08) | 2u;
  if (ios_sub.ptr(0x28) == nullptr) {
    fail_st |= 4u;
  }
  std_ios_base_clear_00991000(host, ios_sub.p, fail_st, false);
  return false;
}

// 0x00991ce0: std_basic_istream_ctor_00991ce0
void* std_basic_istream_ctor_00991ce0(
    Seg66Host& host, void* self, void* streambuf, bool is_std, std::int32_t init_vbtable) {
  Raw s = raw_mut(self);
  if (init_vbtable != 0) {
    s.ptr(0x00) = host.image_address(0x00d1d530u);
    s.ptr(0x08) = host.image_address(0x00d1d210u);
  }
  std::int32_t vbase_off = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{s.p + vbase_off}.ptr(0x00) = host.image_address(0x00d1d20cu);
  s.at<std::uint32_t>(0x04) = 0u;
  std::int32_t vbase_off2 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  std_basic_ios_init_00991970(host, s.p + vbase_off2, streambuf, is_std);
  return self;
}

// 0x00991d60: std_use_facet_00991d60
void* std_use_facet_codecvt_char_00991d60(Seg66Host& host, const void* locale_obj) {
  std::uint32_t lockit_outer = 0u;
  host.crt_lockit_ctor_00bf5993(&lockit_outer, 0);
  void* cached_facet = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(host.global_u32(0x00f8a2f0u)));
  void* local_facet = cached_facet;
  if (host.global_u32(0x00f8a2f4u) == 0u) {
    std::uint32_t lockit_inner = 0u;
    host.crt_lockit_ctor_00bf5993(&lockit_inner, 0);
    if (host.global_u32(0x00f8a2f4u) == 0u) {
      std::uint32_t next_id = host.global_u32(0x0109dcb8u) + 1u;
      host.global_u32(0x0109dcb8u) = next_id;
      host.global_u32(0x00f8a2f4u) = next_id;
    }
    host.crt_lockit_dtor_00bf59b4(&lockit_inner);
  }
  std::uint32_t id = host.global_u32(0x00f8a2f4u);
  Raw loc_imp = raw_const(raw_const(locale_obj).ptr(0x00));
  void* found = nullptr;
  if (id < loc_imp.at<std::uint32_t>(0x0c)) {
    found = raw_const(loc_imp.ptr(0x08)).at<void*>(id * 4u);
    if (found != nullptr) {
      host.crt_lockit_dtor_00bf59b4(&lockit_outer);
      return found;
    }
  }
  if (loc_imp.at<std::uint8_t>(0x14) != 0u) {
    Raw glob_imp = raw_const(host.crt_get_global_locale_00bf5acc());
    if (id < glob_imp.at<std::uint32_t>(0x0c)) {
      found = raw_const(glob_imp.ptr(0x08)).at<void*>(id * 4u);
    }
  }
  if (found != nullptr) {
    host.crt_lockit_dtor_00bf59b4(&lockit_outer);
    return found;
  }
  if (cached_facet != nullptr) {
    host.crt_lockit_dtor_00bf59b4(&lockit_outer);
    return cached_facet;
  }
  if (std_facet_getcat_009919d0(host, &local_facet) == 0xffffffffu) {
    alignas(4) std::uint8_t exc_buf[12] = {};
    host.crt_bad_cast_ctor_00bf6477(
        exc_buf, static_cast<const char*>(host.image_address(0x00d1d500u)));
    host.cxx_throw_exception_00bf6885(exc_buf, host.image_address(0x00de182cu));
  }
  found = local_facet;
  host.global_u32(0x00f8a2f0u) =
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(found));
  host.stl_facet_incref_0098d9c0(found);
  host.crt_facet_register_00bf5bc4(found);
  host.crt_lockit_dtor_00bf59b4(&lockit_outer);
  return found;
}

// 0x00991e80: FUN_00991e80
void* std_basic_istream_sentry_ctor_00991e80(
    Seg66Host& host, void* self, void* istream_obj, bool noskip) {
  Raw s = raw_mut(self);
  Raw is = raw_mut(istream_obj);
  s.ptr(0x00) = istream_obj;
  std::int32_t vbase_off = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
  void* sb = Raw{is.p + vbase_off}.ptr(0x28);
  if (sb != nullptr) {
    host.crt_mutex_lock_00bf5f2e(raw_mut(sb).p + 0x04);
  }
  bool ok = std_basic_istream_ipfx_00991a90(host, s.ptr(0x00), noskip);
  s.at<std::uint8_t>(0x04) = ok ? 1u : 0u;
  return self;
}

// 0x00992370: std_basic_filebuf_open_00992370
void* std_basic_filebuf_open_00992370(
    Seg66Host& host, void* self, const char* filename, std::int32_t mode, std::int32_t prot) {
  Raw s = raw_mut(self);
  if (s.ptr(0x4c) != nullptr) {
    return nullptr;
  }
  void* file = host.crt_fiopen_00bf622c(filename, mode, prot);
  if (file == nullptr) {
    return nullptr;
  }
  s.at<std::uint8_t>(0x48) = 1u;
  s.at<std::uint8_t>(0x41) = 0u;
  host.stl_basic_streambuf_init_0098e250(self);
  Raw f = raw_mut(file);
  s.ptr(0x30) = f.p + 0x04;
  s.ptr(0x34) = f.p + 0x04;
  s.ptr(0x10) = f.p + 0x08;
  s.ptr(0x14) = f.p + 0x08;
  s.ptr(0x20) = file;
  s.ptr(0x24) = file;
  s.ptr(0x4c) = file;
  s.at<std::uint32_t>(0x44) = host.global_u32(0x00f8a2e8u);
  s.ptr(0x3c) = nullptr;

  void* loc_obj = nullptr;
  void* loc_ptr = host.stl_streambuf_getloc_0098e210(self, &loc_obj);
  void* facet = std_use_facet_codecvt_char_00991d60(host, loc_ptr);
  if (host.invoke_vtable_04_always_noconv(facet)) {
    s.ptr(0x3c) = nullptr;
    host.stl_locale_dtor_0098daf0(&loc_obj);
    return self;
  }
  s.ptr(0x3c) = facet;
  host.stl_basic_streambuf_init_0098e250(self);
  host.stl_locale_dtor_0098daf0(&loc_obj);
  return self;
}

// 0x00992480: FUN_00992480
void* std_operator_gt_gt_string_00992480(
    Seg66Host& host, void* istream_obj, void* str_obj) {
  Raw is = raw_mut(istream_obj);
  std::uint32_t state = 0u;
  bool changed = false;
  alignas(4) std::uint8_t sentry_buf[8] = {};
  std_basic_istream_sentry_ctor_00991e80(host, sentry_buf, istream_obj, false);
  if (raw_const(sentry_buf).at<std::uint8_t>(0x04) != 0u) {
    std::int32_t vbase_off = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
    void* loc_obj = nullptr;
    void* loc_ptr = host.stl_ios_base_getloc_0098dd80(is.p + vbase_off, &loc_obj);
    void* facet = std_use_facet_ctype_char_009911f0(host, loc_ptr);
    host.stl_locale_dtor_0098daf0(&loc_obj);
    host.legacy_sbo_string_erase_004087f0(str_obj, 0u, 0xffffffffu);

    std::int32_t vbase_off2 = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
    Raw ios_sub{is.p + vbase_off2};
    std::int32_t width = ios_sub.at<std::int32_t>(0x18);
    std::uint32_t max_chars = 0xfffffffeu;
    if (width > 0 && static_cast<std::uint32_t>(width) < 0xfffffffeu) {
      max_chars = static_cast<std::uint32_t>(width);
    }

    Raw sb = raw_mut(ios_sub.ptr(0x28));
    std::int32_t ch = 0;
    std::uint8_t** igfirst = sb.at<std::uint8_t**>(0x20);
    std::int32_t* igcount = sb.at<std::int32_t*>(0x30);
    if (*igfirst != nullptr && *igcount > 0) {
      ch = static_cast<std::int32_t>(**igfirst);
    } else {
      ch = host.invoke_vtable_10_uflow(sb.p);
    }

    while (max_chars > 0u) {
      if (ch == -1) {
        state |= 1u;
        break;
      }
      const std::uint16_t* table = raw_const(facet).at<const std::uint16_t*>(0x10);
      if ((table[static_cast<std::uint8_t>(ch)] & 0x48u) != 0u) {
        break;
      }
      std_string_append_chars_0098f9a0(host, str_obj, 1u, static_cast<char>(ch));
      changed = true;
      --max_chars;
      std::int32_t vbase_off_loop = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
      ch = host.stl_basic_streambuf_snextc_0098f3b0(Raw{is.p + vbase_off_loop}.ptr(0x28));
    }
  }

  std::int32_t vbase_off_end = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{is.p + vbase_off_end}.at<std::uint32_t>(0x18) = 0u;
  if (!changed) {
    state |= 2u;
  }
  std::int32_t vbase_off_clr = raw_const(is.ptr(0x00)).at<std::int32_t>(0x04);
  Raw ios_clr{is.p + vbase_off_clr};
  if (state != 0u) {
    std::uint32_t new_st = ios_clr.at<std::uint32_t>(0x08) | state;
    if (ios_clr.ptr(0x28) == nullptr) {
      new_st |= 4u;
    }
    std_ios_base_clear_00991000(host, ios_clr.p, new_st, false);
  }
  Raw sentry_stream = raw_mut(raw_const(sentry_buf).ptr(0x00));
  std::int32_t vbase_off_unlk = raw_const(sentry_stream.ptr(0x00)).at<std::int32_t>(0x04);
  void* sb_unlk = Raw{sentry_stream.p + vbase_off_unlk}.ptr(0x28);
  if (sb_unlk != nullptr) {
    host.crt_mutex_unlock_00bf5f37(raw_mut(sb_unlk).p + 0x04);
  }
  return istream_obj;
}

// 0x009926a0: std_basic_ifstream_ctor_009926a0
void* std_basic_ifstream_ctor_009926a0(
    Seg66Host& host,
    void* self,
    const char* filename,
    std::uint32_t mode,
    std::int32_t prot,
    std::int32_t init_vbtable) {
  Raw s = raw_mut(self);
  if (init_vbtable != 0) {
    s.ptr(0x00) = host.image_address(0x00d1d5a4u);
    s.ptr(0x58) = host.image_address(0x00d1d210u);
  }
  std::int32_t vbase_off = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{s.p + vbase_off}.ptr(0x00) = host.image_address(0x00d1d20cu);
  s.at<std::uint32_t>(0x04) = 0u;

  std::int32_t vbase_off2 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw ios_sub{s.p + vbase_off2};
  std_ios_base_init_00991150(host, ios_sub.p);
  void* filebuf_ptr = s.p + 0x08;
  ios_sub.ptr(0x28) = filebuf_ptr;
  ios_sub.ptr(0x2c) = nullptr;
  ios_sub.at<char>(0x30) = std_basic_ios_widen_00991610(host, ios_sub.p, ' ');
  if (ios_sub.ptr(0x28) == nullptr) {
    std_ios_base_clear_00991000(host, ios_sub.p, ios_sub.at<std::uint32_t>(0x08) | 4u, false);
  }
  ios_sub.at<std::uint32_t>(0x04) = 0u;

  std::int32_t vbase_off3 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
  Raw{s.p + vbase_off3}.ptr(0x00) = host.image_address(0x00d1d50cu);

  std_basic_streambuf_ctor_0098f180(host, filebuf_ptr);
  Raw fb = raw_mut(filebuf_ptr);
  fb.ptr(0x00) = host.image_address(0x00d1d434u);
  fb.at<std::uint8_t>(0x48) = 0u;
  fb.at<std::uint8_t>(0x41) = 0u;
  host.stl_basic_streambuf_init_0098e250(filebuf_ptr);
  fb.ptr(0x4c) = nullptr;
  fb.at<std::uint32_t>(0x44) = host.global_u32(0x00f8a2e8u);
  fb.ptr(0x3c) = nullptr;

  if (std_basic_filebuf_open_00992370(
          host, filebuf_ptr, filename, static_cast<std::int32_t>(mode | 1u), prot) == nullptr) {
    std::int32_t vbase_off4 = raw_const(s.ptr(0x00)).at<std::int32_t>(0x04);
    Raw ios_fail{s.p + vbase_off4};
    std::uint32_t st = ios_fail.at<std::uint32_t>(0x08) | 2u;
    if (ios_fail.ptr(0x28) == nullptr) {
      st |= 4u;
    }
    std_ios_base_clear_00991000(host, ios_fail.p, st, false);
  }
  return self;
}

// 0x00993060: CG_scalar_deleting_dtor_00993060
void* cg_scalar_deleting_dtor_00993060(Seg66Host& host, void* self, std::uint8_t flags) {
  Raw s = raw_mut(self);
  void* block = s.ptr(0x0c);
  if (block != nullptr) {
    std::uint32_t bytes = s.at<std::uint32_t>(0x08) + 1u;
    void* pool = host.sized_storage_pool_get_singleton_00419cc0();
    host.sized_storage_pool_return_block_00bd1510(pool, block, bytes, 1);
  }
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00993b70: FUN_00993b70
void* stl_xlive_stat_record_vector_erase_range_00993b70(
    Seg66Host& host,
    void* self,
    void* out_iter,
    void* first_owner,
    void* first_ptr,
    void* last_owner,
    void* last_ptr) {
  Raw s = raw_mut(self);
  Raw out = raw_mut(out_iter);
  if (first_owner == nullptr || first_owner != last_owner) {
    host.crt_invalid_parameter_noinfo_00bf6713();
  }
  if (first_ptr != last_ptr) {
    std::uint32_t flag_word =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(first_owner)) & 0xffffff00u;
    void* new_end = host.stl_xlive_stat_record_range_copy_dispatch_00993540(
        last_ptr, s.ptr(0x08), first_ptr, out_iter, out_iter, flag_word);
    host.stl_xlive_stat_record_range_destroy_00993990(
        new_end, s.ptr(0x08), self, out_iter);
    s.ptr(0x08) = new_end;
  }
  out.ptr(0x00) = first_owner;
  out.ptr(0x04) = first_ptr;
  return out_iter;
}

// 0x009942f0: CG_scalar_deleting_dtor_009942f0
void* cg_scalar_deleting_dtor_009942f0(Seg66Host& host, void* self, std::uint8_t flags) {
  host.xlive_stats_block_destruct_00994180(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00994500: TRIV_body_00994500
bool return_true_stub_00994500(std::uint32_t /*unused_arg*/) {
  return true;
}

// 0x00994680: CG_scalar_deleting_dtor_00994680
void* cg_scalar_deleting_dtor_00994680(Seg66Host& host, void* self, std::uint8_t flags) {
  host.download_manager_base_destruct_009945e0(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00994750: CG_scalar_deleting_dtor_00994750
void* cg_scalar_deleting_dtor_00994750(Seg66Host& host, void* self, std::uint8_t flags) {
  host.content_package_record_destruct_009946d0(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00994e90: TRIV_body_00994e90
bool return_false_stub_ret4_00994e90(std::uint32_t /*unused_arg*/) {
  return false;
}

// 0x00994ea0: TRIV_body_00994ea0
bool return_false_stub_00994ea0() {
  return false;
}

// 0x00994ef0: CG_scalar_deleting_dtor_00994ef0
void* cg_scalar_deleting_dtor_00994ef0(Seg66Host& host, void* self, std::uint8_t flags) {
  host.content_package_list_destruct_00994e10(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00995670: CG_scalar_deleting_dtor_00995670
void* cg_scalar_deleting_dtor_00995670(Seg66Host& host, void* self, std::uint8_t flags) {
  host.download_manager_destruct_00995150(self);
  if ((flags & 1u) != 0u) {
    host.free_00bf65ac(self);
  }
  return self;
}

// 0x00996060: BSP_Localization_AlreadyRunningMessage
const wchar_t* localization_already_running_message_00996060(
    Seg66Host& host, std::uint32_t lcid, std::int32_t message_index) {
  std::int32_t row = host.localization_language_row_for_lcid_0098d400(lcid);
  std::int32_t idx = row + message_index * 6;
  const void* table = host.image_address(0x00d1e918u);
  return raw_const(table).at<const wchar_t*>(static_cast<std::size_t>(idx) * 4u);
}

}  // namespace bsp::hardware_options_seg66_argon
