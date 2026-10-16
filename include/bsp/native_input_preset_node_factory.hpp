#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input preset node construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 0069F5F0: 126B/44 instructions. Native ECX/EDX are unconsumed;
// left,parent,right,pair,color occupy five stack words, RET14h/EAX saved node.
// Source adds actual RawCtx/0109EEA4 references: seven stack words/RET1Ch.
// Allocate raw 2Ch bytes through the accepted current-CRT service OUTSIDE the
// try. Nonnull: ordered link stores +0/+4/+8, published pair construction +C,
// then CURRENT private color low byte -> +28 and nil byte +29 -> 0. Other
// bytes, including +2A/+2B padding, retain their allocation representations.
// Lexical catch 0069F66E frees ONLY saved raw allocation and bare-rethrows;
// no key/payload cleanup or rollback. C7EF80's placement effect is elided only
// by the proved two-saved-allocation/three-state map and literal 401130 RET.
// Neither scope exposes a callable Original EH/private-EBP interface.
// Requires valid reached storage and actual borrowed providers excluding
// active private frames. Allocation/exception/heap policies are current C++/
// CRT; Original ABI, FH3/SEH/RTTI, fault/flags/runtime/game identity is unproved.
// Existing STL_Tree_Buynode_StrKeyCI_N2C_0069f5f0 identity is preserved;
// this Source name is provisional. No node overlay, guard or provider added.
void* __fastcall buy_native_input_preset_node_0069f5f0(
    void* unused_ecx, std::uint32_t unused_edx,
    std::uint32_t left_bits, std::uint32_t parent_bits,
    std::uint32_t right_bits, const void* actual_pair,
    std::uint32_t color_word, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
