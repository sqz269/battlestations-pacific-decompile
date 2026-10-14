#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::hardware_options_seg66_argon {

struct Raw {
  std::uint8_t* p;
  template <class T>
  T& at(std::size_t off) const noexcept {
    return *reinterpret_cast<T*>(p + off);
  }
  void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct Seg66Host {
  virtual ~Seg66Host() = default;

  virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
  virtual std::uint8_t& global_u8(std::uint32_t address) = 0;
  virtual float& global_f32(std::uint32_t address) = 0;
  virtual double& global_f64(std::uint32_t address) = 0;
  virtual void* image_address(std::uint32_t address) = 0;

  // External direct callees
  virtual void* legacy_sbo_string_construct_from_cstring_00407290(void* str, const char* cstr) = 0;
  virtual void legacy_sbo_string_destroy_004072d0(void* str) = 0;
  virtual void* legacy_sbo_string_assign_substring_00408120(
      void* dst_str, const void* src_str, std::uint32_t offset, std::uint32_t count) = 0;
  virtual void* character_string_assign_counted_00408720(
      void* dst_str, const char* ptr, std::uint32_t count) = 0;
  virtual void* legacy_sbo_string_erase_004087f0(
      void* str, std::uint32_t offset, std::uint32_t count) = 0;
  virtual void legacy_sbo_string_grow_allocation_004089e0(
      void* str, std::uint32_t new_size, std::uint32_t old_size) = 0;
  virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
  virtual std::int32_t localization_language_row_for_lcid_0098d400(std::uint32_t lcid) = 0;
  virtual std::uint32_t stl_vector_size_0098d610(const void* vec) = 0;
  virtual void stl_facet_incref_0098d9c0(void* facet) = 0;
  virtual void stl_locale_dtor_0098daf0(void* locale_obj) = 0;
  virtual void* stl_ios_base_getloc_0098dd80(void* ios_base_self, void* out_locale) = 0;
  virtual void stl_string_chassign_0098e1b0(
      void* str, std::uint32_t offset, std::uint32_t count, char ch) = 0;
  virtual void* stl_streambuf_getloc_0098e210(void* streambuf_self, void* out_locale) = 0;
  virtual void stl_basic_streambuf_init_0098e250(void* streambuf_self) = 0;
  virtual const char* stl_string_const_iterator_deref_0098f0c0(const void* iter) = 0;
  virtual std::int32_t stl_basic_streambuf_snextc_0098f3b0(void* streambuf_self) = 0;
  virtual void* stl_string_begin_0098f490(const void* str, void* out_iter) = 0;
  virtual bool stl_fputc_0098f580(char ch, void* file) = 0;
  virtual void* stl_xlive_stat_record_range_copy_dispatch_00993540(
      void* first,
      void* last,
      void* dst,
      void* tag1,
      void* tag2,
      std::uint32_t flag_word) = 0;
  virtual void stl_xlive_stat_record_range_destroy_00993990(
      void* first, void* last, void* owner, void* arg) = 0;
  virtual void xlive_stats_block_destruct_00994180(void* self) = 0;
  virtual void download_manager_base_destruct_009945e0(void* self) = 0;
  virtual void content_package_record_destruct_009946d0(void* self) = 0;
  virtual void content_package_list_destruct_00994e10(void* self) = 0;
  virtual void download_manager_destruct_00995150(void* self) = 0;
  virtual void sized_storage_pool_return_block_00bd1510(
      void* pool, void* block, std::uint32_t bytes, std::int32_t count) = 0;
  virtual void crt_xlen_00bf5695() = 0;
  virtual std::int32_t crt_tolower_00bf576a(std::int32_t ch, const void* ctypevec) = 0;
  virtual void* crt_getctype_00bf5879(void* out_ctypevec) = 0;
  virtual void* crt_lockit_ctor_00bf5993(void* lockit_self, std::int32_t kind) = 0;
  virtual void crt_lockit_dtor_00bf59b4(void* lockit_self) = 0;
  virtual void* crt_get_global_locale_00bf5acc() = 0;
  virtual void crt_facet_register_00bf5bc4(void* facet) = 0;
  virtual void crt_locinfo_dtor_00bf5bfd(void* locinfo) = 0;
  virtual void* crt_locale_init_00bf5cc2() = 0;
  virtual void crt_locinfo_ctor_00bf5d5c(void* locinfo, const char* name) = 0;
  virtual void crt_ios_base_addstd_00bf5e73(void* ios_base_self) = 0;
  virtual void crt_ios_base_dtor_00bf5ec3(void* ios_base_self) = 0;
  virtual void* crt_mutex_ctor_00bf5f01(void* mutex_self) = 0;
  virtual void crt_mutex_dtor_00bf5f19(void* mutex_self) = 0;
  virtual void crt_mutex_lock_00bf5f2e(void* mutex_self) = 0;
  virtual void crt_mutex_unlock_00bf5f37(void* mutex_self) = 0;
  virtual std::int32_t crt_toupper_00bf5f87(std::int32_t ch, const void* ctypevec) = 0;
  virtual void* crt_fiopen_00bf622c(const char* filename, std::int32_t mode, std::int32_t prot) = 0;
  virtual void* allocator_exception_base_default_construct_00bf632f(void* exc_self) = 0;
  virtual void* crt_exception_copy_ctor_00bf63a6(void* exc_self, const void* other) = 0;
  virtual void allocator_exception_base_cleanup_00bf6454(void* exc_self) = 0;
  virtual void* crt_bad_cast_ctor_00bf6477(void* exc_self, const char* msg) = 0;
  virtual void free_00bf65ac(void* ptr) = 0;
  virtual void crt_invalid_parameter_noinfo_00bf6713() = 0;
  virtual std::int32_t crt_memcpy_s_00bf672c(
      void* dst, std::uint32_t dst_size, const void* src, std::uint32_t count) = 0;
  virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
  virtual void cxx_throw_exception_00bf6885(void* exc_obj, const void* throw_info) = 0;
  virtual void free_00bf6989(void* ptr) = 0;
  virtual std::int32_t crt_fclose_00bf8081(void* file) = 0;
  virtual void free_00bf9dc8(void* ptr) = 0;
  virtual std::uint32_t crt_fwrite_00bfb08b(
      const void* buf, std::uint32_t size, std::uint32_t count, void* file) = 0;
  virtual std::int32_t crt_fseek_00bfb72b(void* file, std::int32_t offset, std::int32_t origin) = 0;
  virtual std::int32_t crt_getc_00bfc5f7(void* file) = 0;
  virtual std::int32_t crt_fputc_00bfc87f(std::int32_t ch, void* file) = 0;
  virtual std::int32_t crt_ungetc_00bfcc1b(std::int32_t ch, void* file) = 0;
  virtual std::int32_t crt_fgetpos_00bfcf65(void* file, void* pos) = 0;
  virtual std::int32_t crt_fsetpos_00bfcfd0(void* file, const void* pos) = 0;

  // Virtual table slot dispatches
  virtual void* invoke_vtable_00_scalar_deleting_dtor(void* self, std::uint8_t flags) = 0;
  virtual bool invoke_vtable_04_always_noconv(void* facet) = 0;
  virtual std::int32_t invoke_vtable_04_overflow(void* streambuf, std::int32_t meta) = 0;
  virtual std::int32_t invoke_vtable_10_uflow(void* streambuf) = 0;
  virtual std::int32_t invoke_vtable_10_codecvt_in(
      void* codecvt_facet,
      void* state,
      const char* first1,
      const char* last1,
      const char** mid1,
      char* first2,
      char* last2,
      char** mid2) = 0;
  virtual std::int32_t invoke_vtable_14_codecvt_out(
      void* codecvt_facet,
      void* state,
      const char* first1,
      const char* last1,
      const char** mid1,
      char* first2,
      char* last2,
      char** mid2) = 0;
  virtual std::int32_t invoke_vtable_18_codecvt_unshift(
      void* codecvt_facet, void* state, char* first2, char* last2, char** mid2) = 0;
  virtual char invoke_vtable_18_ctype_do_widen(void* ctype_facet, char ch) = 0;
  virtual std::int32_t invoke_vtable_30_streambuf_sync(void* streambuf) = 0;
};

// Reconstructed Segment 66 functions (0098c7f0 through 00996060)
std::uint32_t hardware_info_cpu_speed_0098c7f0(const void* self);
std::uint32_t hardware_info_memory_size_0098c800(const void* self);
void* hardware_info_display_options_ptr_0098c810(void* self);
const char* hardware_info_sound_device_name_0098d3d0(Seg66Host& host);
void std_locale_facet_dtor_0098da40(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_0098db80(Seg66Host& host, void* self, std::uint8_t flags);
void std_locale_facet_ctor_0098dbd0(Seg66Host& host, void* self);
void std_codecvt_base_dtor_0098dc50(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_0098dc60(Seg66Host& host, void* self, std::uint8_t flags);
void* cg_scalar_deleting_dtor_0098dde0(Seg66Host& host, void* self, std::uint8_t flags);
void* std_locale_scalar_deleting_dtor_0098de50(Seg66Host& host, void* self, std::uint8_t flags);
bool hardware_info_supports_display_mode_0098e0e0(
    Seg66Host& host, const void* self, std::int32_t width, std::int32_t height);
void* cg_scalar_deleting_dtor_0098e170(Seg66Host& host, void* self, std::uint8_t flags);
void std_basic_streambuf_dtor_0098e4e0(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_0098e890(Seg66Host& host, void* self, std::uint8_t flags);
void* std_basic_streambuf_ctor_0098f180(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_0098f950(Seg66Host& host, void* self, std::uint8_t flags);
void* std_string_append_chars_0098f9a0(
    Seg66Host& host, void* self, std::uint32_t count, char ch);
void* std_string_assign_chars_0098fa90(
    Seg66Host& host, void* self, std::uint32_t count, char ch);
void std_locinfo_dtor_wrapper_0098fb50(Seg66Host& host, void* self);
std::uint32_t std_basic_filebuf_uflow_0098fc20(Seg66Host& host, void* self);
bool std_basic_filebuf_endwrite_0098ffe0(Seg66Host& host, void* self);
std::int32_t std_basic_filebuf_overflow_00990340(Seg66Host& host, void* self, std::int32_t meta);
void* std_basic_filebuf_seekoff_00990740(
    Seg66Host& host,
    void* self,
    void* out_fpos,
    std::int32_t off_lo,
    std::int32_t off_hi,
    std::int32_t way,
    std::int32_t mode);
void* std_basic_filebuf_seekpos_00990820(
    Seg66Host& host,
    void* self,
    void* out_fpos,
    std::int32_t pos_off_lo,
    std::int32_t pos_off_hi,
    std::uint32_t fpos_lo,
    std::uint32_t fpos_hi,
    std::uint32_t state_val,
    std::int32_t mode);
void* std_runtime_error_ctor_00990910(Seg66Host& host, void* self, const void* msg_str);
void* std_basic_filebuf_close_00990970(Seg66Host& host, void* self);
void std_runtime_error_dtor_009909e0(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_00990a20(Seg66Host& host, void* self, std::uint8_t flags);
void std_basic_filebuf_dtor_00990a70(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_00990ad0(Seg66Host& host, void* self, std::uint8_t flags);
void* std_locinfo_ctor_00990b60(Seg66Host& host, void* self, const char* locname);
void* std_runtime_error_copy_ctor_00990c20(Seg66Host& host, void* self, const void* other);
void* std_ctype_char_ctor_00990c90(
    Seg66Host& host, void* self, const void* table, bool del_flag, std::uint32_t refs);
char std_ctype_char_do_tolower_00990d60(Seg66Host& host, const void* self, char ch);
const char* std_ctype_char_do_tolower_range_00990d80(
    Seg66Host& host, const void* self, char* first, const char* last);
char std_ctype_char_do_toupper_00990db0(Seg66Host& host, const void* self, char ch);
const char* std_ctype_char_do_toupper_range_00990dd0(
    Seg66Host& host, const void* self, char* first, const char* last);
char std_ctype_char_do_widen_00990e00(char ch);
char std_ctype_char_do_narrow_00990e80(char ch, char dflt);
void std_ctype_char_dtor_00990f00(Seg66Host& host, void* self);
void* cg_scalar_deleting_dtor_00990f40(Seg66Host& host, void* self, std::uint8_t flags);
std::uint32_t std_ctype_char_getcat_00990f90(Seg66Host& host, void** out_facet);
void std_ios_base_clear_00991000(
    Seg66Host& host, void* self, std::uint32_t state, bool reraise);
void std_ios_base_init_00991150(Seg66Host& host, void* self);
void* std_use_facet_ctype_char_009911f0(Seg66Host& host, const void* locale_obj);
void std_basic_ifstream_dtor_00991310(Seg66Host& host, void* vbase_self);
char std_basic_ios_widen_00991610(Seg66Host& host, void* self, char ch);
void* std_facet_ctor_009916c0(Seg66Host& host, void* self, std::uint32_t refs);
void std_basic_ios_init_00991970(
    Seg66Host& host, void* self, void* streambuf, bool is_std);
std::uint32_t std_facet_getcat_009919d0(Seg66Host& host, void** out_facet);
void* std_basic_ostream_flush_00991a40(Seg66Host& host, void* self);
bool std_basic_istream_ipfx_00991a90(Seg66Host& host, void* self, bool noskip);
void* std_basic_istream_ctor_00991ce0(
    Seg66Host& host, void* self, void* streambuf, bool is_std, std::int32_t init_vbtable);
void* std_use_facet_codecvt_char_00991d60(Seg66Host& host, const void* locale_obj);
void* std_basic_istream_sentry_ctor_00991e80(
    Seg66Host& host, void* self, void* istream_obj, bool noskip);
void* std_basic_filebuf_open_00992370(
    Seg66Host& host, void* self, const char* filename, std::int32_t mode, std::int32_t prot);
void* std_operator_gt_gt_string_00992480(
    Seg66Host& host, void* istream_obj, void* str_obj);
void* std_basic_ifstream_ctor_009926a0(
    Seg66Host& host,
    void* self,
    const char* filename,
    std::uint32_t mode,
    std::int32_t prot,
    std::int32_t init_vbtable);
void* cg_scalar_deleting_dtor_00993060(Seg66Host& host, void* self, std::uint8_t flags);
void* stl_xlive_stat_record_vector_erase_range_00993b70(
    Seg66Host& host,
    void* self,
    void* out_iter,
    void* first_owner,
    void* first_ptr,
    void* last_owner,
    void* last_ptr);
void* cg_scalar_deleting_dtor_009942f0(Seg66Host& host, void* self, std::uint8_t flags);
bool return_true_stub_00994500(std::uint32_t unused_arg);
void* cg_scalar_deleting_dtor_00994680(Seg66Host& host, void* self, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00994750(Seg66Host& host, void* self, std::uint8_t flags);
bool return_false_stub_ret4_00994e90(std::uint32_t unused_arg);
bool return_false_stub_00994ea0();
void* cg_scalar_deleting_dtor_00994ef0(Seg66Host& host, void* self, std::uint8_t flags);
void* cg_scalar_deleting_dtor_00995670(Seg66Host& host, void* self, std::uint8_t flags);
const wchar_t* localization_already_running_message_00996060(
    Seg66Host& host, std::uint32_t lcid, std::int32_t message_index);

}  // namespace bsp::hardware_options_seg66_argon
