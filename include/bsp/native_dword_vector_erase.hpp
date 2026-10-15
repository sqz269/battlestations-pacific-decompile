#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DWORD vector erasure requires MSVC Win32.
#endif

namespace bsp {
// Whole006977F0..00697852,99B/40instructions. NativeECX receiver, unusedEDX;
// stack output,first_owner,first,last_owner,last; RET14h/EAX original output.
// Validate nonnull first_owner==last_owner only, with returning/throwing current
// SDK diagnostic policy. There is no comparison of either owner with receiver.
// Capture positions AFTER the report. Equal positions skip header end accesses.
// Otherwise capture end, precompute first+SAR2(end-last)*4, progressively copy
// current source DWORDs until captured end, then publish precomputed receiver+8.
// If copying ran, reload the first_owner input word before publication; an empty
// tail retains the original owner. Output node is stored BEFORE output owner.
void* __fastcall erase_native_checked_dword_vector_range_006977f0(
    void* receiver, std::uint32_t unused_edx, void* output,
    const void* first_owner, void* first, const void* last_owner, void* last);

// Whole00697890..006978C8,57B/26instructions. NativeECX receiver,unusedEDX,
// no stack inputs,RET; ambient/private-output EAX has no public semantic result.
// Capture original end before first begin>end report; then current begin and
// fresh end for a second report. Call the actual erase entry with captured begin
// and ORIGINAL end, even after returning handlers mutate the header. No shortcut
// end=begin, additional bounds/null/owner guard or rollback is introduced.
void __fastcall clear_native_checked_dword_vector_00697890(
    void* receiver, std::uint32_t unused_edx);

// Require accessible raw reached storage and finite equality termination.
// Progressive aliases and DWORD wrapping are retained; no memmove_s/typedarray
// substitute or invented iterator/frame is used. Existing narrower Source and
// fixtures remain intact. New C++ private inputs/output cells do not reproduce
// original private-stack aliases, register/flag residue, CRT/fault/SEH/binaryABI
// or game identity. The owned native routines have no local EH or allocation.
} // namespace bsp
