#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

// GOAL16AZ repairs the unaccepted AT layout using accepted AX evidence.
// The Win32 layout is grounded in the frozen bytes and stock trees.c/trees.h;
// these descriptive types do not recover an Original public type or ABI.
namespace bsp {

struct NativeZlibStaticCodeWords {
    std::uint16_t code;
    std::uint16_t length;
};

struct NativeZlibStaticTreeDescriptor32 {
    const NativeZlibStaticCodeWords* static_tree;
    const std::int32_t* extra_bits;
    std::int32_t extra_base;
    std::int32_t elements;
    std::int32_t max_length;
};

// One authentic 2616-byte RO owner preserves cross-member raw read values.
// The three spacing fields contain observed static bytes, not implicit padding.
struct NativeZlibTreeReadOnlyRegion {
    std::int32_t extra_lbits[29];
    std::uint8_t bl_order[19];
    std::uint8_t spacing_after_bl_order[1];
    std::int32_t extra_dbits[30];
    std::int32_t extra_blbits[19];
    std::uint8_t spacing_before_static_ltree[4];
    NativeZlibStaticCodeWords static_ltree[288];
    NativeZlibStaticCodeWords static_dtree[30];
    std::uint8_t dist_code[512];
    std::uint8_t length_code[256];
    std::int32_t base_length[29];
    std::uint8_t spacing_before_base_dist[4];
    std::int32_t base_dist[30];
};

static_assert(sizeof(void*) == 4, "This DATA interface targets MSVC Win32.");
static_assert(std::is_standard_layout<NativeZlibStaticCodeWords>::value);
static_assert(sizeof(NativeZlibStaticCodeWords) == 4);
static_assert(alignof(NativeZlibStaticCodeWords) == 2);
static_assert(offsetof(NativeZlibStaticCodeWords, code) == 0);
static_assert(offsetof(NativeZlibStaticCodeWords, length) == 2);
static_assert(std::is_standard_layout<NativeZlibStaticTreeDescriptor32>::value);
static_assert(sizeof(NativeZlibStaticTreeDescriptor32) == 20);
static_assert(alignof(NativeZlibStaticTreeDescriptor32) == 4);
static_assert(offsetof(NativeZlibStaticTreeDescriptor32, static_tree) == 0);
static_assert(offsetof(NativeZlibStaticTreeDescriptor32, extra_bits) == 4);
static_assert(offsetof(NativeZlibStaticTreeDescriptor32, extra_base) == 8);
static_assert(offsetof(NativeZlibStaticTreeDescriptor32, elements) == 12);
static_assert(offsetof(NativeZlibStaticTreeDescriptor32, max_length) == 16);

static_assert(std::is_standard_layout<NativeZlibTreeReadOnlyRegion>::value);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion) == 2616);
static_assert(alignof(NativeZlibTreeReadOnlyRegion) == 4);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, extra_lbits) == 0);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, bl_order) == 116);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, spacing_after_bl_order) == 135);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, extra_dbits) == 136);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, extra_blbits) == 256);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, spacing_before_static_ltree) == 332);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, static_ltree) == 336);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, static_dtree) == 1488);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, dist_code) == 1608);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, length_code) == 2120);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, base_length) == 2376);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, spacing_before_base_dist) == 2492);
static_assert(offsetof(NativeZlibTreeReadOnlyRegion, base_dist) == 2496);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::extra_lbits) == 116);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::bl_order) == 19);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::spacing_after_bl_order) == 1);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::extra_dbits) == 120);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::extra_blbits) == 76);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::spacing_before_static_ltree) == 4);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::static_ltree) == 1152);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::static_dtree) == 120);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::dist_code) == 512);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::length_code) == 256);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::base_length) == 116);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::spacing_before_base_dist) == 4);
static_assert(sizeof(NativeZlibTreeReadOnlyRegion::base_dist) == 120);

}  // namespace bsp

extern "C" {

extern const bsp::NativeZlibTreeReadOnlyRegion
    bsp_native_zlib_tree_ro_region_00d677c0;

// Typed subobjects do not make out-of-array C++ indexing valid. Raw consumers
// need their own physical contract; Original placement/fault/ABI/game are open.

// All three descriptor objects, including their pointer fields, are mutable.
// Separate external objects have no C++ adjacency or Original address promise.
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_l_desc_00e1470c;
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_d_desc_00e14720;
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_bl_desc_00e14734;

}  // extern "C"
