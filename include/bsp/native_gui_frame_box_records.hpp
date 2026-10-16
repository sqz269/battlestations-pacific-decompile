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


#include <cstdint>

namespace bsp {
class NativeRenderActualOwners;

enum class NativeGuiFrameBoxRecordReleasePhase : std::uint8_t {
    fresh, entered, returned, failed
};

// Explicit Source orchestration metadata, not an Original object/stack layout.
// Prepare this SAME persistent object outside the failing scope, with the full
// actual record/resource/name/services graph already retained BEFORE entry.
// It owns none of that graph. Access is externally serialized; metadata stays
// live/stable and disjoint from record/resource/buffer/provider scratch storage.
// Captured pointers are diagnostics only, possibly freed/reused after a terminal.
// Phase flags mark Source scheduling, not Native instruction/exception origin.
// zero_dispatch_entered includes registry lookup; normal_name_return_entered
// covers getter+return, so the getter can fail before BD1510.
// There is no reset, replay, rollback, count gate or graph-cleanup operation.
// Destroying entered/failed metadata terminates without inspecting the graph;
// fresh/returned destruction reads metadata only. This fail-closed rule is NEW
// Source orchestration, not recovered Original destructor or FH3 behavior.
class NativeGuiFrameBoxRecordReleaseOperation final {
public:
    NativeGuiFrameBoxRecordReleaseOperation() noexcept = default;
    ~NativeGuiFrameBoxRecordReleaseOperation() noexcept;
    NativeGuiFrameBoxRecordReleaseOperation(const NativeGuiFrameBoxRecordReleaseOperation&) = delete;
    NativeGuiFrameBoxRecordReleaseOperation& operator=(const NativeGuiFrameBoxRecordReleaseOperation&) = delete;
    NativeGuiFrameBoxRecordReleaseOperation(NativeGuiFrameBoxRecordReleaseOperation&&) = delete;
    NativeGuiFrameBoxRecordReleaseOperation& operator=(NativeGuiFrameBoxRecordReleaseOperation&&) = delete;

    NativeGuiFrameBoxRecordReleasePhase phase() const noexcept { return phase_; }
    void* record_identity() const noexcept { return record_; }
    void* captured_resource_identity() const noexcept { return resource_; }
    bool resource_decremented() const noexcept { return resource_decremented_; }
    std::int32_t count_after_decrement() const noexcept { return count_after_decrement_; }
    bool zero_dispatch_entered() const noexcept { return zero_dispatch_entered_; }
    bool zero_dispatch_returned() const noexcept { return zero_dispatch_returned_; }
    bool resource_cleared() const noexcept { return resource_cleared_; }
    bool name_action_armed() const noexcept { return name_action_armed_; }
    bool name_action_consumed() const noexcept { return name_action_consumed_; }
    void* captured_normal_name_identity() const noexcept { return name_data_; }
    std::uint32_t captured_normal_name_size() const noexcept { return name_size_; }
    bool normal_name_return_entered() const noexcept { return normal_name_return_entered_; }
    bool normal_name_returned() const noexcept { return normal_name_returned_; }
    bool failure_name_cleanup_entered() const noexcept { return failure_name_cleanup_entered_; }
    bool failure_name_cleanup_returned() const noexcept { return failure_name_cleanup_returned_; }
private:
    friend void release_native_gui_frame_box_record_00ad0860(void*, NativeRenderActualOwners&,
        NativeStringRawPoolContext&, NativeGuiFrameBoxRecordReleaseOperation&);
    NativeGuiFrameBoxRecordReleasePhase phase_{};
    void* record_{};
    void* resource_{};
    void* name_data_{};
    std::int32_t count_after_decrement_{};
    std::uint32_t name_size_{};
    bool resource_decremented_{}, zero_dispatch_entered_{}, zero_dispatch_returned_{};
    bool resource_cleared_{}, name_action_armed_{}, name_action_consumed_{};
    bool normal_name_return_entered_{}, normal_name_returned_{};
    bool failure_name_cleanup_entered_{}, failure_name_cleanup_returned_{};
};

// Whole AD0860 [AD0860,AD08DE): native ECX actual2Ch record, no stacked args,
// RET. This explicit Source interface is NOT that private binary/exception ABI.
// Actual storage/counts are live DWORD-aligned raw objects in the genuine helper
// domain, not typed widget/projection owners. One operation per owning lifetime;
// even a different fresh operation must not replay an interrupted/released record.
//
// Borrow the SAME renderer actual registry (via NativeRenderActualOwners) and
// SAME NativeStringRawPoolContext/publication/gate/raw-manager cells. The ONLY
// established zero family is a genuine completed canonical registered2D creator:
// current D61948, actual slot00 BD30E0/slot04 B3F590, same texture context/pool.
// This provenance is a caller domain precondition, not an arbitrary companion
// adapter. Positive-count return performs no registry lookup. No new owner,
// fallback terminal, fake profile, implicit retain or pool is supplied here.
// All graph/services/cells and metadata outlive the applicable terminal/name
// actions. Caller retains entered failure outside the unwinding scope; stale
// record links/captured pointers are NOT ownership or healthy retry capability.
//
// Capture resource before arming name cleanup; decrement once, dispatch only
// observed zero, clear record+0 ONLY in the captured-nonnull branch on success.
// Capture normal name DATA before consuming its cleanup action, then CURRENT
// length+1 before the real pool getter/return. The name header remains unchanged.
// A resource-service escape consumes the action BEFORE genuine41DD20; cleanup
// escape terminates. A normal getter escape never retries name cleanup.
// Win32 atomic service/IAT identity and private Native FH3/double-exception ABI
// are not claimed. Existing canonical Entry zero terminals are noexcept and
// terminate on failure: those failures cannot unwind into the name action.
void release_native_gui_frame_box_record_00ad0860(void* actual_record,
    NativeRenderActualOwners& actual_owners, NativeStringRawPoolContext& actual_strings,
    NativeGuiFrameBoxRecordReleaseOperation& operation);
} // namespace bsp
