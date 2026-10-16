#pragma once

#include "bsp/cc13_avoid1.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoid-manager group lookup requires MSVC Win32.
#endif

namespace bsp {

// Complete 004120D0..00412111 (66 bytes). Original: ECX actual manager,
// one signed layer DWORD on the stack, EAX borrowed group pointer, RET 4.
// Source-only unused_edx supplies the physical fastcall shape; Original has
// no EDX input. Capture signed count +58h once. A nonpositive count returns
// the current slot0 (+08h) without reading the layer word. Positive counts
// scan actual group pointers and signed +10h keys, return the first equality,
// or the last physical index with key < layer (fallback0). Return reloads
// the chosen slot; there is no null, count-capacity or sorted-order repair.
// Actual required receiver/slot/key cells must remain readable. No ownership,
// lock, allocation, global or callback is introduced. The older typed index
// projection and the enclosing caller's mandatory bindings remain separate.
// Exact leaf instruction evidence is not whole-game or deployment validation.
Cc13AvoidZoneNativeGroup* __fastcall lookup_native_avoid_manager_group_004120d0(
    Cc13AvoidZoneManagerNative* actual_manager, void* unused_edx,
    std::int32_t layer);

} // namespace bsp
