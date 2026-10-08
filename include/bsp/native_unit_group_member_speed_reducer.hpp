#pragma once

namespace bsp {

// Complete raw 0070D140..0070D1A0: 96 bytes, 28 instructions, no calls.
// ECX is the SAME actual group; EDX is unused on entry; plain RET returns
// the reduced record+30h speed in ST0. Count at group+4F8h is captured once.
// Positive count requires valid 34h-byte records at group+18h (ordinary
// bounded admission: aligned 508h-byte group with count1..24). Null member
// DWORDs reset their record+30h to the captured native999 value and skip it.
// Member identities are integer words, never dereferenced as class objects.
// Group+504h is caller-retained and is neither read nor written here.
//
// Requires readable native DATA at absolute00CFD6F4, and for positive count
// at00CF4888. The bounded verified cells are4B18967Fh and4479C000h. A caller
// may explicitly supply readonly PE-snapshot cells at those exact VAs;
// such data is frozen native evidence, not live game/class/world admission.
// This function creates no mapping, provider, fallback or replacement global.
//
// Preserve ambient x87 state: return needs one free slot; a nonnull record
// needs two free slots. The FLD/FSTP-float/FLD scratch sequence, FCOMIP and
// resulting exception state are intentional. Production does not reset CW.
// Invalid backing, unmasked faults, concurrency, original class/lifetime,
// allocation, enclosing throttle caller and gameplay remain unbound.
float __fastcall reduce_native_unit_group_member_speeds_0070d140(
    void* actual_group, void* unused_edx) noexcept;

} // namespace bsp
