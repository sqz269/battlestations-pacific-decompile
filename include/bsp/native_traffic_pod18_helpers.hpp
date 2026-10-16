#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw traffic POD18 helpers require MSVC Win32.
#endif

namespace bsp {

// Complete 0047C4D0..0047C4EE: ECX actual header, no stacked words, plain RET.
// EAX is zero for null first(+4), otherwise the raw signed /24 magic result
// from wrapped last(+8)-first; no size validation or C++ pointer subtraction.
std::uint32_t __fastcall size_native_traffic_pod18_0047c4d0(const void* actual_header);

// Complete 0047FC30..0047FC7C. Every verified direct caller pushes SIX physical
// words: first,last,destination,duplicate-destination,tag0,tag1; plain RET and
// caller cleanup18h. Only the first three are read by this body. Exposing the
// other actual words does not assign them recovered semantic C++ types/defaults.
// EAX is advanced destination. Equal endpoints skip pointed accesses; otherwise
// x87 reads/stores each of six float words in order, WITHOUT a null-dest guard.
void* __cdecl copy_native_traffic_pod18_0047fc30(
    const void* first, const void* last, void* destination,
    std::uint32_t physical_duplicate_destination,
    std::uint32_t physical_tag0, std::uint32_t physical_tag1);

// Complete 004841A0..004841E8 and 00486BE0..00486C25. Verified physical binding:
// ECX/EDX as named, FOUR stacked words {destination-or-value,opaque0,opaque1,tag},
// RET10h. Only the first stacked word is read. All actual caller words remain
// in their original incoming cells; there is no fabricated default/tail copy.
// The CURRENT computed destination is tested each iteration. Zero skips all
// six pointed x87 reads/stores, then cursors still advance24 modulo2^32.
// Each x87 word is read then written before the next source read, retaining
// forward overlap, value alias, NaN quieting and inherited FP status/control.
// Copy returns advanced destination; equality still reads the destination word.
// Fill count0 does not read that word or pointed storage and leaves EAX intact;
// its incidental EAX has no established semantic return, hence Source void.
void* __fastcall copy_uninitialized_native_traffic_pod18_004841a0(
    const void* first, const void* last, void* destination,
    std::uint32_t physical_opaque0, std::uint32_t physical_opaque1,
    std::uint32_t physical_tag);

void __fastcall fill_n_uninitialized_native_traffic_pod18_00486be0(
    void* destination, std::uint32_t count, const void* value,
    std::uint32_t physical_opaque0, std::uint32_t physical_opaque1,
    std::uint32_t physical_tag);

// Complete 0048D2F0..0048D324. Original ECX metadata preimage +three actual
// stack words {destination,count,value}, RET0Ch/EAX end. Source's unused EDX
// placeholder only binds that physical register/stack shape; it is not an
// Original input. Local tag copies ECX and clears ONLY its low byte. The full
// four-word call to the real fill kernel remains; returned ECX is masked too.
void* __fastcall fill_n_native_traffic_pod18_dispatch_0048d2f0(
    std::uint32_t ecx_preimage, std::uint32_t unused_edx,
    void* destination, std::uint32_t count, const void* value);

// Complete 00486850..004868A5. Three actual incoming stack words, plain RET.
// Reserves two UNINITIALIZED local tag words, clears ONLY each low byte, then
// pushes all six physical words to the real copy child. EAX end is recomputed
// with Native signed /24 magic and wrapped24 multiplication. No snapshots,
// whole-tag zero initialization or substitute integer/memmove copy is used.
void* __cdecl copy_native_traffic_pod18_dispatch_00486850(
    const void* first, const void* last, void* destination);

// These complete pure hardware/library bodies have no CRT/callback/EH child.
// The explicit Win32 Source interfaces are not recovered template symbols or
// production/drop-in/gameplay proof. Actual reached storage and live stack
// cells must be backed; no noexcept/SEH/private-frame identity is promised.
// Full traffic/layout/resize/settings/raw-global producers remain incomplete.
} // namespace bsp
