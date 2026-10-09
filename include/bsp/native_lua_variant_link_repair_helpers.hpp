#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant link repair helpers require MSVC Win32.
#endif

namespace bsp {

// Provisional descriptive Source names for the complete 006EDC80..006EDCCD
// and 006EDCD0..006EDD21 schedules. No recovered tree type or semantic API.
// Naked fastcall places actual_receiver in ECX and the unused DWORD in EDX;
// actual_pivot is the first stack word. Each body captures that word in EDX
// and its selected neighbor in EAX BEFORE saving ESI, then uses RET4 on all
// three exits. The Source void* result exposes the physical surviving EAX;
// it does not establish a recovered Native return-value contract.
//
// Raw backing, with X=pivot, Q=selected neighbor, H=current receiver+4:
// - Receiver and H: readable DWORD+4; H+4 writable on the root branch (8-byte
//   address span). The receiver's +0 and +8 are not accessed directly.
// - 006EDC80: X has readable/writable DWORDs+4/+8 (12-byte span); Q has
//   readable/writable DWORD+0 and writable DWORD+4 (8-byte span).
// - 006EDCD0: X has readable/writable DWORDs+0/+4 (8-byte span); Q has
//   readable/writable DWORD+8 and writable DWORD+4 (12-byte span).
// - The reloaded moved node has readable byte+31h and, only when it is zero,
//   writable DWORD+4. Its 50-byte highest-address span is not an object size.
// - A nonroot parent supplies the tested +0/+8 DWORD and selected writable
//   +0/+8 DWORD (up to 12 bytes, depending on the body and branch).
// No null, alignment, membership, capacity, lifetime or ownership guard.
//
// Repeated reads and ordered stores are retained for raw aliases: reload the
// moved link after the pivot-link store, read pivot.parent after the optional
// moved-parent store, load receiver+4 after the Q-parent store, and reload
// pivot.parent again on the nonroot branch. The current ESI save word is
// popped BEFORE root/parent attachment stores. EBX/BL, EDI and EBP are not
// touched; EDX returns captured X, EAX returns captured Q, and ECX returns H
// or the freshly loaded nonroot parent. Final CMP arithmetic flags survive.
//
// Caller supplies usable stack, saved-ESI and return backing. Data aliases
// can corrupt the save/return slots; PUSH ESI can affect a later aliased read.
// Preserving the incoming ESI value requires its save slot to survive until
// POP. Completed stores are not rolled back if a later access faults.
// No raw noexcept promise, allocation, callback, owner, default storage,
// callable profile, vtable or Native type is introduced. Original placement,
// caller ABI, flags/fault/runtime/EH and gameplay parity remain unproved.
void* __fastcall repair_native_lua_variant_links_006edc80(
    void* actual_receiver,
    std::uint32_t unused_edx,
    void* actual_pivot);

void* __fastcall repair_native_lua_variant_links_006edcd0(
    void* actual_receiver,
    std::uint32_t unused_edx,
    void* actual_pivot);

} // namespace bsp
