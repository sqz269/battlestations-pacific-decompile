#pragma once

// cc13 pool16 (segments 2 and 5): the 1Ch-slot node pool lifecycle (00436230 / 004360E0 /
// 00435A50) and the engine-movie constructor 00460B80. Build only: no host path calls these.
// Descriptive names are hypotheses.

#include "bsp/allocator_list.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------------------------
// Node pool of 1Ch-byte slots, 400h per slab (vtable 00CE4120, slot 0 = 004360E0). One body
// serves every instance: 00436230 has 20 CRT-initializer callers and 00435A50 20 atexit callers;
// the locale map node pool at 00E1768C is one of them (native_locale_map_node_recycling.hpp).
// Slab: slot i at i*1Ch with its slab index at +18h, free-index WORD stack at +7000h, free
// count WORD at +7800h (400h = slab empty).
// ---------------------------------------------------------------------------------------------
struct NodePool1CStorage {
    AllocatorListElement allocator_00;          // vtable, prev +4h, next +8h (list head 00E188B4)
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// Library calls the three bodies make, bound to the host (no credit). The Win32 critical-section
// imports (00CE220C Initialize, 00CE2210 Leave, 00CE2214 Delete) are called directly.
struct NodePool1CHost {
    virtual ~NodePool1CHost() = default;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;  // throws on failure
    virtual void free_00bf6989(void* block) = 0;                    // the slab table
    virtual void free_00bf65ac(void* block) = 0;                    // the slabs
};

inline constexpr std::uint32_t kNodePool1CVtable = 0x00CE4120u;

// 00436230, __thiscall(ECX = pool), plain RET, EAX = pool.
NodePool1CStorage& node_pool_1c_construct_00436230(NodePool1CStorage& pool, AllocatorListDomain& list,
                                                   NodePool1CHost& host);
// 004360E0, slot 0 of 00CE4120, __thiscall(ECX = pool), plain RET.
void node_pool_1c_trim_empty_slabs_004360e0(NodePool1CStorage& pool, NodePool1CHost& host);
// 00435A50, __fastcall(ECX = pool), plain RET.
void node_pool_1c_destroy_00435a50(NodePool1CStorage& pool, AllocatorListDomain& list,
                                   NodePool1CHost& host);

// ---------------------------------------------------------------------------------------------
// 00460B80, the engine-movie object constructor (74h block, kept at game+38h by 004CCE50).
// ---------------------------------------------------------------------------------------------
// The 8-byte NativeString {length +0, data +4} that 0045D480 takes by value.
struct MovieNativeString {
    std::uint32_t length;
    char* data;
};

// One 0045D480 call: its eight strings in argument order (name, refer, num1, num2, num3, the
// string stored at record +38h, the string stored at +40h, offset), its six flags in argument
// order (refer +10h, num1 +1Ch, num2 +28h, num3 +34h, enum +48h, offset +54h), and which strings
// the image builds through the 0041E870 constructor (bit k = string k) rather than the inlined
// 0041DD40 + memcpy sequence.
struct MovieActionTypeCall {
    const char* strings[8];
    std::uint8_t flags[6];
    std::uint8_t constructor_mask;
};
// The 32 calls, in call order (00460E24 .. 0046426F), read from the image's pushes.
extern const MovieActionTypeCall kMovieActionTypeCalls_00460b80[32];

struct EngineMovieConstructHost {
    virtual ~EngineMovieConstructHost() = default;
    // 0041DD40 __thiscall(ECX = string, length, preserve), RET 8.
    virtual void native_string_resize_0041dd40(MovieNativeString& string, std::uint32_t length,
                                               std::uint8_t preserve) = 0;
    // 00BF7680 memcpy (CRT).
    virtual void memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) = 0;
    // 0041E870 __thiscall(ECX = string, const char*), RET 4: construct from a C string.
    virtual void construct_native_string_0041e870(MovieNativeString& string, const char* text) = 0;
    // 0041DD20 __thiscall(ECX = string): release (the unwind funclets).
    virtual void native_string_destroy_0041dd20(MovieNativeString& string) = 0;
    // 0045D480 __thiscall(ECX = 58h record, eight strings by value, six flags), RET 58h, EAX =
    // record. The callee owns and releases the eight strings.
    virtual void* movie_action_type_construct_0045d480(void* record, const MovieNativeString (&strings)[8],
                                                       const std::uint8_t (&flags)[6]) = 0;
    // 00460AD0 __thiscall(ECX = vector 00E18550, const record*), RET 4: push_back.
    virtual void movie_action_types_push_back_00460ad0(std::byte* vector_00e18550, const void* record) = 0;
    // 0045CFC0 __thiscall(ECX = record): release the record's eight strings.
    virtual void movie_action_type_destroy_0045cfc0(void* record) = 0;
    // 0045CDD0 __thiscall(ECX = vector): tidy (the state-1 unwind of block +64h).
    virtual void vector_tidy_0045cdd0(std::byte* vector) = 0;
};

// 00460B80, __thiscall(ECX = 74h block), plain RET, EAX = block. `vector_00e18550` is the global
// action-type vector (+4h first, +8h last). `clock_bits_00ce5380` is the dword at 00CE5380
// (41700000h, 15.0f) stored to block +0.
std::byte* engine_movie_construct_00460b80(std::byte* block, std::byte* vector_00e18550,
                                           std::uint32_t clock_bits_00ce5380,
                                           EngineMovieConstructHost& host);

}  // namespace bsp
