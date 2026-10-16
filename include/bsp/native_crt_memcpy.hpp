#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT memcpy requires MSVC Win32.
#endif

namespace bsp {
// Complete00BF7680 _memcpy behavior:711definedB/247instructions/22ranges,
// 869Bphysicalspan includes120Bsixtables and38Bskippedpadding. Correct library
// name retained even though the actual body supports backward overlap.
// Native cdecl(D,S,DWORDcount), EAX originalD, plainRET/caller12cleanup.
// This new Source interface borrows the actual stable canonical0109EEA4 cell
// as its fourth argument; caller removes16bytes. No feature state is created.
// Read the current cell ONLYafterforward-direction and unsignedsize>=256;
// nonzero/matchingpointerlowbits selects restored-frame actualC0C82B tail.
// Scalarzero still follows the native return path without buffer reads;
// equal pointers still copy. Unaligned/unrolled/tail and backwardSTD/REP/CLD
// schedules remain. No stdmemmove/BF87delegate/copycallback/validation/guard.
// Requires DF0, valid readable/writable nonwrapping extents excluding active
// argument/return/provider frames and nativecode/tables. Actual SSE2 CPU/OS
// support must hold when selected; canonical cell truth/lifetime belong to
// the caller. PEvirtualzero/Ghidra0 does not establish runtimeflag truth.
// Preserves EBP/ESI/EDI and untouchedEBX; EAX/ECX/EDX/arithmeticflags volatile.
// Eight proven selector ladders replace16Native indexedjumps, retaining
// material copyaccess order and partial writes. A backward fault between
// STD/CLD can exposeDF1. No fault recovery or exception conversion is added.
// Source extraargument/privateframes, flags, Native table/code/faultPC,
// Original ABI/FH3/SEH/heap/runtime/game identity remain qualified.
void* __cdecl copy_native_crt_bytes_00bf7680(
    void* actual_destination, const void* actual_source, std::uint32_t byte_count,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
