#pragma once

#include "bsp/cc13_avoid1.hpp"

#include <cstdint>

namespace bsp {

// Complete raw-storage clear at 0041D0C0. ECX is the existing 14h-byte group;
// the retained slot array/capacity/layer key are not reset. Actual current-CRT
// frees replace the original CRT entries. This Source service owns no callbacks.
void __fastcall clear_native_avoid_group_0041d0c0(Cc13AvoidZoneNativeGroup* group);

// Complete normal operation and the recovered array-free unwind action of
// 0041DE70. Current C++ EH replaces Native FH3 registration/funclet/frame ABI.
// Writes the actual Source table below, clears zones, then frees/nulls +04h.
void __fastcall destroy_native_avoid_group_0041de70(Cc13AvoidZoneNativeGroup* group);

// Native ECX receiver + stacked flags, RET 4; EDX is an ignored Source slot so
// actual indirect CALL EDX manager dispatch is usable. Bit 0 alone selects the
// object free; the original receiver is returned, including after that free.
Cc13AvoidZoneNativeGroup* __fastcall scalar_delete_native_avoid_group_0041e940(
    Cc13AvoidZoneNativeGroup* group, void* unused_source_edx, std::uint32_t flags);

using NativeAvoidGroupScalarDeleteEntry = Cc13AvoidZoneNativeGroup* (__fastcall*)(
    Cc13AvoidZoneNativeGroup*, void*, std::uint32_t);

// A real Source table for the recovered/consumed virtual slot. Its relocated
// pointer replaces the Native identity 00CE3858 in the existing uint32 vtable
// word. No contract for additional Native virtual slots is asserted/provided.
struct NativeAvoidGroupLifetimeSourceVtable {
    NativeAvoidGroupScalarDeleteEntry scalar_delete;
};
static_assert(sizeof(NativeAvoidGroupLifetimeSourceVtable) == 4);
extern const NativeAvoidGroupLifetimeSourceVtable kNativeAvoidGroupLifetimeSourceVtable;

} // namespace bsp
