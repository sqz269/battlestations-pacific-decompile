#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual hidden-record construction requires MSVC Win32.
#endif

namespace bsp {

struct NativeStringRawPoolContext;

// Stable/disjoint Source binding metadata, not a native owner or argument.
// The native incoming24 cells are bag,class,name,party,matrix,parent-header.
// saved_this4 binds the distinct native private [EBP-10h] cell written by
// PUSH ECX and again at004693E4. Supplied record/payload may alias these cells;
// implicit compiler/native private-state and outgoing-argument aliases are not
// supplied. Only reached actual backing is required; no validation is added.
struct NativeSceneHiddenRecordConstructFrame {
    const void* actual_parameters24;
    void* actual_saved_this4;
};

// Whole178B/56ops004693C0..00469471 code, through actual raw providers.
// Native ECX record; six LIVE stacked words;RET18h/EAX captured record.
// Write borrowed actual vtable word00CE5640 and zero parent-header54/58.
// Preserve late argument reads, captured class before name publication,
// party/frame before class publication, exact raw x87 copy, late parent pointer,
// self-header skip, resize/reread and destlen->srcdata->destdata copy reads.
// Cleanup arms only after bag/name capture and disarms on normal completion.
void* construct_native_scene_hidden_entity_record_004693c0(void* actual_record,
    const NativeSceneHiddenRecordConstructFrame&, NativeStringRawPoolContext&);

// Whole11B/3ops00C61AD0..00C61ADA action mapping only: CURRENT saved_this4,
// add54 modulo32, then actual raw pooled header destruction. Native EBP frame,
// no stacked arguments, tail JMP0041DD20. No raw-name/bag/record release.
// Source current C++ cleanup transport does not reproduce Original FH3/SEH.
void unwind_native_scene_hidden_entity_record_00c61ad0(
    const void* actual_saved_this4, NativeStringRawPoolContext&);

// GENERAL SOURCE-READY IS FALSE. Complete code does not expand the published
// duplicate00438E40 admission: null or live nonwrapping contiguous NUL-terminated
// text; normal canonical allocation and disjoint copy. Allocation failure,
// new-handler/reentry, naked-frame unwind and Original CRT/FH3 equivalence are
// unproved. The code retains current-provider return/throw paths and sole
// cleanup, without claiming admission of those paths or general parent closure.
// Exact103B x87 copying preserves reached overlap/ambient CW/NaN operations;
// current SDK memmove admits zero count and overlap through a Source-only raw
// call adapter. No std::string, Host dispatch, extra free, snapshot, finite guard,
// object lifetime/vtable callable binding, binary ABI or game proof is supplied.

} // namespace bsp
