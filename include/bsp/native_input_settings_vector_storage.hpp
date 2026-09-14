#pragma once
#include "bsp/native_string.hpp"
#include <array>

namespace bsp {
// Concrete storage contracts over actual checked vector headers. Opaque header
// words are retained. Descriptor values copy all five DWORDs. Ordinary vectors
// use count-based 1.5x growth. Copied nested values allocate their exact size;
// outer growth transfers existing nested buffers and preserves their capacities.
// These are the resize/end-insertion uses, not general STL insertion APIs.
void resize_native_input_settings_descriptors_006a0db0(void*, std::uint32_t,
    std::array<std::uint32_t,5>);
void resize_native_input_settings_words_00492210(void*, std::uint32_t, std::uint32_t);
// Shared DWORD storage mechanics. Pointer specializations publish begin last;
// the existing00492210 DWORD specialization publishes begin first. Other
// contracts and source allocation/exception boundaries below are unchanged.
enum class NativeCheckedDwordPublication { begin_capacity_end, capacity_end_begin };
void resize_native_checked_dword_storage(void*, std::uint32_t, std::uint32_t,
    NativeCheckedDwordPublication);
// Non-owning two-DWORD specialization of the same checked storage mechanics.
// Actual10h header, eight-byte records, 1.5x element-count growth; replacement
// publication is capacity/end/begin. Only end insertion is supplied, with valid
// consistent storage and a source pair independent of invalidated old backing.
// This is a source storage contract, not a general original STL/iterator ABI.
void append_native_checked_pair_storage(void*, const void* source_pair);
// Actual14h packed-bit storage: bit count0, opaque4, word begin8/endC/capacity10.
// Only the low byte of the fill value is boolean. Shrink masks unused high bits.
void resize_native_input_settings_bits_0049df50(void*, std::uint32_t, std::uint32_t);

// These original by-value arguments own their nested/string allocations and
// are consumed on return or supported C++ unwind. A caller must provide an
// independent value, as it would after a native by-value copy construction.
void resize_native_input_settings_strings_0049e050(void*, std::uint32_t,
    std::array<std::uint32_t,2>, NativeStringStorage&);
void resize_native_input_settings_name_pairs_006a6350(void*, std::uint32_t,
    std::array<std::uint32_t,4>, NativeStringStorage&);
void resize_native_input_settings_groups_006a79a0(void*, std::uint32_t,
    std::array<std::uint32_t,4>, NativeStringStorage&);
void resize_native_input_settings_conflict_pairs_006a4710(void*, std::uint32_t,
    std::array<std::uint32_t,4>);

// Complete normal nested-range cleanup contracts. Capture first/end; destroy
// each element forward, free its CURRENT backing, clear pointer words4/8/C.
// Element opaque0 is retained. Caller releases the outer backing separately.
void destroy_native_input_settings_conflict_pair_range_0069eea0(void*, void*);
void destroy_native_input_settings_group_range_006a6ee0(void*, void*, NativeStringStorage&);

// Valid consistent owned storage is required. Allocation/string callbacks may
// throw but must not structurally mutate these vectors or source values.
// Source allocation, string and exception services retain their documented
// boundaries; original STL/CRT/FH3 ABI, private-stack aliasing, malformed ranges,
// hardware-fault behavior and complete production settings trees are not supplied.
} // namespace bsp
