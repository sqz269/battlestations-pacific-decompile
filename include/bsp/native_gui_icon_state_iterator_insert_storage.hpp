#pragma once
#include "bsp/native_gui_icon_state_growth_storage.hpp"

namespace bsp {
// Whole139B/57op AB6070 schedule over actual16B vector backing. Original ECX,
// four stackDWORD homes and RET10h become this ordinary five-argument API.
// Incoming homes are [output8B pointer, iterator owner, element, prototype].
// Capture owner first, then actual receiver/current first. Empty/null or the
// actual wrapped SUB/SAR6 size0 skips initial validation and ordinal-home read.
// Otherwise keep captured first/owner across real returning invalid-parameter
// calls, then read CURRENT element home. No noreturn/NoThrow is inferred.
//
// Real growth arguments are separate four mutable DWORD homes: capture current
// prototype THEN current element, stage prototype/count1/element/capturedowner,
// and directly call actual AB59E0 with SAME context/frame. Never copy mutated
// growth homes back to the incoming caller homes. No new frame, owner, callback,
// allocation, state constructor/destructor or implicit cleanup is introduced.
//
// After growth capture CURRENT first before its validation call. Keep that
// capture across returning callbacks; form ordinalSHL6+first with DWORD wrap.
// Capture CURRENT last, overwrite incoming element HOME with captured first,
// then decide the already-captured comparison. Only its non-above path reads
// CURRENT first again. The pure compare may occur after the HOME store in this
// Source expression; operands are already captured and private controls are
// disjoint. Original FLAGS/private-stack/fault timing is not claimed.
// Read output HOME last, AFTER any returning handler. Store output+4 then+0,
// return captured output identity, and never reread a possibly overwritten HOME.
//
// Caller supplies genuine DWORD/subobject/backing/iterator/output lifetimes and
// retains both home arrays, growth frame, current/partial/completed graph and
// providers across entered escapes. The two home arrays have separate backing;
// the growth frame is distinct from incoming/output/vector backing. Private
// Source controls/spills are disjoint from operand/provider writes. Genuine
// writes to declared callee homes/frame (including its temp) and current fields
// remain live under the actual growth supplier's contract.
// No typed vector/T[n] family or custody follows from an index/profile/count.
// Existing raw59E0 allocation/copy/dtor/exception and genuine current SDK
// _invalid_parameter_noinfo policy are inherited. Source90's normal supplier is
// a separately pinned compiled private dependency; future Source91 catches do
// not change this API but must be qualified at actual caller admission.
// Native BF66EF handler/CRT selection, Origin, FH3/SEH/ABI/FLAGS/fault/CPU/DF/FP,
// actual caller/disposition/runtime/activation remain unproved. No extra
// Original/vendor/helper/CG/duplicate body credit; preserve existing STL name.
void* insert_native_gui_icon_state_iterator_storage_00ab6070(void* actual_vector_f0,
    volatile std::uint32_t (&actual_incoming_homes)[4],
    volatile std::uint32_t (&actual_growth_homes)[4],
    NativeGuiIconStateGrowthStorageFrame&, NativeGuiIconStateGrowthStorageContext&);
} // namespace bsp
