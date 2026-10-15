#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Scene property record assignment requires MSVC Win32.
#endif

namespace bsp {

// Whole [008F0700,008F07C1),193 bytes/69 instructions; original twelve-entry
// table at008F07C4 is48 bytes. Descriptive name is a hypothesis. This new
// ordinary C++ interface does not supply Original ECX/stack/RET/flags transport,
// private CRT, failure/SEH/EH/OOM, class/vtable dispatch or game behavior.
//
// Dispatch reads DESTINATION tag+4 only. Unsigned tag>11 and tag6 return the
// actual destination without reading the source. All active arms require actual
// stable distinct raw38h records with compatible selected payload storage; the
// admitted whole owning-graph use has matching tags. No source-tag guard or
// self-assignment shortcut is added. Owner+30, ordinal+34, phase/tag, Type4
// identity+28, Type5 word+08 and array ownership+2C remain destination values.
//
// Type2 frees/clears destination+C BEFORE physically rereading source+C and
// calling actual duplicate00438E40. Type5 calls actual reference-copy008F0340
// on+18. Nullable borrowed text follows those providers' admitted contracts.
// Source text/payloads, both records and active frames must be disjoint from
// destination's old owned allocations. Type5/array owning self/shared aliases
// would be UAF; do not infer blanket self-safety from Type2's late reread.
// Type7 performs three ordered DWORD load/store pairs; no FP or bulk snapshot.
// Array arms8..11 use the full byte-copy helper on+20: positive nonwrapping
// source byte span, sole-owned current canonical old pointer, existing target
// ownership flag1, no stride conversion/flag repair/zero-count shortcut.
//
// Type1 performs actual FLD m32[source+C] then FSTP m32[destination+C]. Require
// an empty x87 stack, all six exception masks already1, valid PC24/53/64 and any
// RC, and no pending unmasked exception. All binary32 inputs are admitted;
// preserve hardware NaN quieting/sticky status, CW and net depth. No C++ float
// conversion or production state reset. Undefined condition bits, popped
// register contents and FIP/FDP/FOP address identity are not oracles.
//
// Require DF0, sufficient stack, successful same-current-canonical providers,
// stable live storage and no concurrency, reentry, invalid alias/access or
// overflow. Existing owning lifetime services retire replaced/current payloads;
// allocation may reuse an old address. No failure cleanup is invented here.
void* assign_native_scene_property_record_008f0700(
    void* actual_destination, const void* actual_source);

} // namespace bsp
