#pragma once
// Packet cc13-seg61: segment 61 (0092E250..00951940) leaves. The scene text
// writer (indentation, number and string output, the in-memory chunk list and
// the Win32 file sink), the pooled string list at 0094EFB0..0094F540, the block
// buffer at 0094F560..0094F770 and two singleton-base destructors.
// docs/CC13_SEG61.md lists every function with its ABI and uncertainty.
//
// Objects are reached through Raw (byte offsets into the Win32 layout); every
// call the image makes goes through a Seg61Host method named for its target,
// except the pure CRT routines (memcpy, memset, strlen, strcmp, qsort), which are
// called directly. Descriptive names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>

namespace bsp::support_seg61_cc13 {

static_assert(sizeof(void*) == 4, "segment 61 objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

struct Seg61Host {
    virtual ~Seg61Host() = default;
    // Image globals (dword storage) and image data addresses.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
    virtual void* image_address(std::uint32_t address) = 0;
    // Allocation: two operator new entry points, operator delete and _free.
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void operator_delete_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // Sized string pool: 00419CC0 returns the pool, 00BD1510 (ECX = pool) takes the block back.
    virtual void* string_pool_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                             std::int32_t count) = 0;
    // 00426060, __thiscall(ECX = memory, const string* source), RET 4: copy-construct, EAX = memory.
    virtual void* native_string_copy_construct_00426060(void* memory, const void* source) = 0;
    // Writer vtable slot 0 (__thiscall(writer, const void* data, length), RET 8).
    virtual void writer_write_slot0(Raw writer, const void* data, std::uint32_t length) = 0;
    // Vtable slot 0 with argument 1 (the scalar deleting destructor).
    virtual void object_delete_slot0(void* object) = 0;
    // 00BF9010 _itoa(value, buffer, radix).
    virtual void itoa_00bf9010(std::int32_t value, char* buffer, std::int32_t radix) = 0;
    // 00439AC0, __cdecl(format, ...): formats into a static buffer, returns it.
    virtual const char* format_static_00439ac0(const char* format, std::uint32_t value) = 0;
    // 004B8490, __cdecl(buffer, format, ...): 256-byte bounded format.
    virtual void format_string256_004b8490(char* buffer, const char* format, double value) = 0;
    // kernel32 imports through 00CE2260 / 00CE2290.
    virtual std::uint32_t create_file_a_ce2260(const char* name, std::uint32_t access, std::uint32_t share,
                                               void* security, std::uint32_t disposition,
                                               std::uint32_t attributes, void* template_file) = 0;
    virtual std::int32_t write_file_ce2290(std::uint32_t handle, const void* data, std::uint32_t length,
                                           std::uint32_t* written, void* overlapped) = 0;
    // 00414DB0, ECX = entity: refresh the pose.
    virtual void entity_pose_refresh_00414db0(Raw entity) = 0;
    // 006FA6D0, __fastcall(ECX = world, EDX = const float matrix[16]).
    virtual void dyn_world_for_each_body_transform_006fa6d0(void* world, const void* matrix) = 0;
    // 004531F0, __fastcall(ECX = name): the last-error message box.
    virtual void show_last_system_error_box_004531f0(const char* name) = 0;
};

// --- scene text writer ---------------------------------------------------
// Layout: +0 vtable (slot 0 = write(data, length)), byte +4 = indent already
// written on this line, +8 indent depth, +0Ch indent width, char +10h indent
// fill. The chunk-list writer keeps head +14h, tail +18h, total length +1Ch;
// the file writer keeps its HANDLE at +14h.
void scene_text_writer_write_indent_0094fdc0(Raw writer, Seg61Host& host);
Raw scene_text_writer_write_string_0094ff10(Raw writer, const char* text, Seg61Host& host);
Raw scene_text_writer_write_line_0094ff50(Raw writer, const char* text, Seg61Host& host);
Raw scene_text_writer_manipulator_0094ffa0(Raw writer, std::int32_t code, Seg61Host& host);
Raw scene_text_writer_write_int_00950000(Raw writer, std::int32_t value, Seg61Host& host);
Raw scene_text_writer_write_unsigned_00950050(Raw writer, std::uint32_t value, Seg61Host& host);
Raw scene_text_writer_write_float_00950130(Raw writer, float value, Seg61Host& host);
char* scene_text_chunk_list_join_009500a0(Raw writer, Seg61Host& host);
void win32_file_writer_write_00950190(Raw writer, const void* data, std::uint32_t length, Seg61Host& host);
bool win32_file_writer_open_009501b0(Raw writer, const char* name, Seg61Host& host);
void scene_text_chunk_list_append_chunk_009501f0(Raw writer, Seg61Host& host);
void scene_text_chunk_list_write_00950240(Raw writer, const void* data, std::uint32_t length, Seg61Host& host);
void scene_text_chunk_list_reset_009502e0(Raw writer, Seg61Host& host);
void scene_text_chunk_list_clear_00950320(Raw writer, Seg61Host& host);
void scene_text_chunk_destruct_00950350(Raw chunk, Seg61Host& host);

// --- pooled string list --------------------------------------------------
// Layout: +0 array of string pointers, +4 count, +8 capacity. Each string is
// an 8-byte {length, data} record whose data comes from the sized pool.
Raw string_list_construct_0094efb0(Raw list, Seg61Host& host);
void string_list_grow_0094f010(Raw list, Seg61Host& host);
std::int32_t string_list_find_0094f090(Raw list, const char* needle, bool case_insensitive);
int string_list_compare_0094f130(const void* left, const void* right);
int string_list_compare_insensitive_0094f180(const void* left, const void* right);
void string_list_sort_0094f1b0(Raw list, bool case_sensitive);
char* string_list_join_lines_0094f1e0(Raw list, Seg61Host& host);
std::int32_t string_list_append_0094f270(Raw list, void* string, bool adopt, Seg61Host& host);
void string_list_insert_0094f2f0(Raw list, void* string, std::int32_t index, bool adopt, Seg61Host& host);
void string_list_delete_string_0094f3a0(Raw slot, Seg61Host& host);
void string_list_clear_0094f3e0(Raw list, Seg61Host& host);
void string_list_destroy_0094f540(Raw list, Seg61Host& host);

// --- block buffer --------------------------------------------------------
// Layout: +0 vtable 00D19B64, +4 block size, +8 block pointer array, +0Ch
// current block index, +10h last allocated index, +14h bytes used in the
// current block, +18h flattened copy, +1Ch (compared, never written here).
Raw block_buffer_construct_0094f560(Raw buffer, std::uint32_t block_size, Seg61Host& host);
void block_buffer_destruct_0094f5e0(Raw buffer, Seg61Host& host);
void block_buffer_reset_0094f650(Raw buffer, Seg61Host& host);
void block_buffer_next_block_0094f680(Raw buffer, Seg61Host& host);
char* block_buffer_flatten_0094f770(Raw buffer, Seg61Host& host);

// --- singleton bases and small helpers -----------------------------------
void singleton_base_destruct_0093f520(Raw object, Seg61Host& host);
void singleton_base_destruct_00940d80(Raw object, Seg61Host& host);
void array12_reserve_0093f590(Raw array, std::int32_t n, Seg61Host& host);
void array4_reserve_0093f610(Raw array, std::int32_t n, Seg61Host& host);
void array4_resize_0093f670(Raw array, std::int32_t n, Seg61Host& host);
void array12_resize_0093f840(Raw array, std::int32_t n, Seg61Host& host);
void array12_push_0093f7f0(Raw array, const void* element, Seg61Host& host);
void array4_push_0093f8a0(Raw array, const std::uint32_t* element, Seg61Host& host);
void array16_push_0093f900(Raw array, const void* element);
void array12_find_pair_0093f790(Raw owner, float out[2], std::int32_t key) noexcept;
void singleton_owner_destruct_0093fc80(Raw owner, Seg61Host& host);
void vector_destroy_0093fc10(Raw vector, Seg61Host& host);
void vector_destroy_0093fc30(Raw vector, Seg61Host& host);
void two_vectors_clear_009318d0(Raw owner) noexcept;
void dyn_world_apply_entity_transform_0092e580(Raw owner, Seg61Host& host);

}  // namespace bsp::support_seg61_cc13
