#pragma once
#include <cstdint>

namespace bsp {
struct NativeNodeStorage;

// Complete 00B6DA70..00B6DAAB over the existing NativeNodeStorage hierarchy.
// Native ECX node, stack(float bits, recurse DWORD), RET8; EDX is unused.
// Copies initial float bits with MOVSS. Only recurse's low byte is tested;
// each recursive value crosses the original FLD/FSTP float32 boundary.
// This raw operation borrows actual+34/+3C links, without a GUI registry.
void __fastcall set_native_node_visibility_factor_00b6da70(NativeNodeStorage*,
    void* unused_edx, float factor, std::uint32_t recurse);
} // namespace bsp
