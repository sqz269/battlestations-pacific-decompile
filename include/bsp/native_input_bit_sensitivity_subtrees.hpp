#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input bit/sensitivity subtrees require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole00558420..0055848F, ECX tree forwarded only, EDX unconsumed,
// stacked node, RET4, no semantic result. Actual2Ch nodes: left/right0/8,
// pooled8B key+C, bit count+14, backing+1C/end+20/cap+24, nil byte29.
// Right recursion; capture left; clear count; capture/conditionally free
// backing; clear1C/20/24; CURRENT key data then conditional length+1 and
// actual pool getter/return; free node; read captured left nil; iterate left.
void __fastcall destroy_native_input_bit_vector_subtree_00558420(void* tree,
    std::uint32_t unused_edx, void* node, NativeStringRawPoolContext& strings);

// Whole00558ED0..00558F3C, same original physical ABI and nil29. Unlike the
// bit body, capture backing BEFORE left; no count+14 clear. Retains opaque
// fields14/18. Other ownership/key/free/nil steps are in the same order.
void __fastcall destroy_native_input_sensitivity_subtree_00558ed0(void* tree,
    std::uint32_t unused_edx, void* node, NativeStringRawPoolContext& strings);

// Complete ordinary subtree paths: no local EH, extra checks, callbacks,
// host containers or rollback. Nil nodes are not freed; tree/head/count
// metadata is untouched. Valid reached native storage, ownership and pool/
// CRT contracts apply. Raw pool getter failures propagate after prior effects;
// no noexcept NativeStringStorage bridge. Source adds context and normalizes
// dead recursive EDX. These interfaces do not prove native stack/register,
// SEH/fault/exception/CRT identity, parent destruction or live gameplay.
} // namespace bsp
