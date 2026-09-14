#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource cache nodes require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete B7F220..B7F28F[112]. Original ECX actual1Ch node; stack
// (left,parent,right,actual0Ch source pair,color DWORD low byte); EAX node,
// RET14h. Actual node: links+0/+4/+8, key length/data+C/+10, raw resource+14,
// color+18,nil+19; final two allocation bytes are untouched.
// Compare node+C/source identity BEFORE link stores and key clear. Unless
// identical, resize using current source length through actual raw pool;
// reload the source guard and all copy fields afterward. Finally reload the
// source+8 resource value, write it, then color and nil0. No resource AddRef,
// old-key release at the initial clear, or partial-key unwind cleanup.
void* construct_native_resource_cache_node_00b7f220(void* actual_node,
    void* left, void* parent, void* right, const void* actual_source_pair,
    std::uint32_t color_word, NativeStringRawPoolContext& strings);

// Complete B7F6A0..B7F711[114] plus catch B7F712..B7F726[21]. Original
// incoming ECX unused; same five stack arguments, EAX captured allocation,
// RET14h. Allocate exactly1Ch through canonical BF681B service; construct only
// when nonnull. Construction failure frees the captured OUTER allocation via
// BF65AC and rethrows. CC20D0's placement-delete401130 action is RET-only;
// no partial-key release is added. Allocation failure precedes the try state.
void* allocate_native_resource_cache_node_00b7f6a0(void* left, void* parent,
    void* right, const void* actual_source_pair, std::uint32_t color_word,
    NativeStringRawPoolContext& strings);

// Explicit raw pool publication/lifetime references and host CRT exception
// transport are new source interfaces, not original register/stack/FH3/SEH
// ABI bridges. Returning null preserves the native branch even though the
// existing canonical allocation service ordinarily returns nonnull or throws.
// Valid native storage/address arithmetic is required; no null-owner/pair or
// resource-pointer validation is introduced. See NATIVE_RESOURCE_CACHE_NODE_BN.md.
} // namespace bsp
