#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native MLandFort kind query requires MSVC Win32.
#endif

namespace bsp {

// Complete 006F5890..006F58C5: ECX=actual receiver, stack=raw tag DWORD,
// full EAX=0/1, RET4. The unused EDX formal only gives the tag its actual
// stack placement; the Native body neither reads nor writes EDX.
// Fixed matches 1Bh/5/4/2/1/0 precede the sole fresh DWORD[ECX+C4] read.
// A fixed match needs no receiver data backing. Ordinary fallback callers
// supply a live actual receiver with readable aligned DWORD+C4 (span>=C8),
// plus valid argument/return backing. There is no added null or range check.
// No copied id, recovered class layout, ownership or lifetime is supplied.
// This leaf does not bind 008761E0's receiver/profile/current target or
// numbering services; query1Bh does not select a profile. No noexcept,
// Original whole-class ABI, runtime, startup or gameplay claim is made.
std::uint32_t __fastcall native_mlandfort_is_kind_006f5890(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

} // namespace bsp
