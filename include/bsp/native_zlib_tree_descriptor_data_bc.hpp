#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

// GOAL16AT: new public Source DATA names for eight complete Native ranges.
// The Win32 layout is grounded in the frozen bytes and stock trees.c/deflate.h;
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

}  // namespace bsp

extern "C" {

extern const std::int32_t bsp_native_zlib_extra_lbits_00d677c0[29];
extern const std::int32_t bsp_native_zlib_extra_dbits_00d67848[30];
extern const std::int32_t bsp_native_zlib_extra_blbits_00d678c0[19];
extern const bsp::NativeZlibStaticCodeWords
    bsp_native_zlib_static_ltree_00d67910[288];
extern const bsp::NativeZlibStaticCodeWords
    bsp_native_zlib_static_dtree_00d67d90[30];

// All three descriptor objects, including their pointer fields, are mutable.
// Separate external objects have no C++ adjacency or Original address promise.
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_l_desc_00e1470c;
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_d_desc_00e14720;
extern bsp::NativeZlibStaticTreeDescriptor32
    bsp_native_zlib_static_bl_desc_00e14734;

}  // extern "C"
