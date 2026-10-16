#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole AD0C60, actual2Ch record copy: native ECX destination, stacked source,
// EAX same destination, RET4. This explicit Source API is not that binary ABI.
// Caller owns fresh DWORD-aligned destination storage and a valid live source.
// Distinct records must not partially overlap. The observed same-header path
// is retained for a borrowed construction preimage, not two owning lifetimes.
// All actual resource/count and name-buffer accesses must satisfy the genuine
// raw helper domain; no owned destination buffer may be abandoned by clearing.
// Name-buffer alias/copy/zero-count rules are those of the existing raw string
// Source boundary. Resource counts are aligned live actual DWORDs, disjoint
// from destination. The current x87 environment/stack must admit four paired
// FLD/FSTP transfers; no FP control/status defaults or conversions are added.
//
// Borrow the SAME real NativeStringRawPoolContext/publication/gate/manager.
// Context metadata, cells, resources and all services remain live/stable and
// disjoint from destination/callee scratch; access is externally serialized.
// Caller retains the SAME partial record/source/context graph BEFORE entry.
// Raw resize/getter failures propagate without catch, rollback or cleanup.
// The copied resource pointer has NOT been acquired if string creation fails:
// do not apply AD0860 to the failed record or replay an interrupted operation.
// No internal retention owner, fail-closed destructor or successful retirement
// is supplied here; those remain the caller's explicit lifetime obligations.
void* copy_construct_native_gui_frame_box_record_00ad0c60(
    void* actual_destination, const void* actual_source,
    NativeStringRawPoolContext& actual_strings);
} // namespace bsp
