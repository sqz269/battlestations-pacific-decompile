#pragma once

#include "bsp/cc13_avoid1.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoidance group factory requires MSVC Win32.
#endif

namespace bsp {
// Complete 00417CA0..00417D58 over existing manager/group storage. Capture
// signed layer from the CURRENT input slot at entry, before search/allocation.
// Return the first exact-key group (reloading its manager slot on that path).
// Otherwise genuinely allocate14h, install the matched Source group vtable,
// zero zones/count/capacity, write key-1 then captured key, and perform native
// signed sorted insertion. The twenty-slot array receives no added bound.
// Current-count loads, captured insertion count, reverse shifts, wrapped slot
// arithmetic and post-store CURRENT-count increment retain the native schedule.
//
// Native ECX manager, one stacked signed key, EAX group, RET4. Source naked
// fastcall adds an ignored EDX placeholder; all owned instructions are retained
// except allocator call target and relocated Source vtable identity. Real current
// allocator/new-handler/bad_alloc semantics and matched group deletion providers
// are used. No blanket null-return, rollback, alternate group class or constructor
// callback is added. Reached spans, argument/control slots and provider lifetimes
// must be valid. Current CRT/exception/provider frames and original global/game
// binding remain qualified. The older index-returning semantic API is separate.
// Evidence: docs/GOAL15E_AVOID_GROUP_FACTORY.md.
Cc13AvoidZoneNativeGroup* __fastcall find_or_create_native_avoid_group_00417ca0(
    Cc13AvoidZoneManagerNative* manager, void* unused_source_edx,
    std::int32_t layer_key);
} // namespace bsp
