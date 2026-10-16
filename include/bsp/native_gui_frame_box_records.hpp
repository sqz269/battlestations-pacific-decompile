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


#include <cstddef>

namespace bsp {
enum class NativeGuiFrameBoxVectorTidyPhase : std::uint8_t {
    fresh, entered, returned, failed
};

// Required preparation owns ONLY Source scheduling metadata, never a vector,
// allocation, resource, registry, pool or retained graph. No default/copy/move.
// Construct outside the failing scope, BEFORE entry/callbacks, over one stable
// array of distinct fresh111 operations for the exact constructed2Ch range.
// A null expected first uses null expected end and an empty span WITHOUT any
// raw-header snapshot; actual null begin later skips the actual end read.
// Known header/constructed-range/metadata regions and fresh phases are checked.
// Constructor validation precedes its first metadata field store; this does
// not restore object lifetimes or legalize invalid C++ placement.
// Full backing extents, live storage, matching SDK heap, resource/provider/
// scratch disjointness and independent ownership remain caller proofs.
// Providers/external callbacks cannot mutate this operation or its prepared
//111 metadata; only this engine and its direct111 calls advance that metadata.
// Access is externally serialized. Captured pointers are diagnostics, possibly
// stale after release/free. No reset, retry, rollback or automatic graph cleanup.
// Entered/failed destruction terminates using metadata only; fresh/returned
// destruction reads metadata only. These are explicit NEW Source rules, not
// Original object/destructor/FH3 behavior. A different operation cannot replay
// the same released/interrupted vector lifetime.
class NativeGuiFrameBoxVectorTidyOperation final {
public:
    NativeGuiFrameBoxVectorTidyOperation(void* actual_header, void* expected_first,
        void* expected_end, NativeGuiFrameBoxRecordReleaseOperation* fresh_records,
        std::size_t record_count);
    ~NativeGuiFrameBoxVectorTidyOperation() noexcept;
    NativeGuiFrameBoxVectorTidyOperation(const NativeGuiFrameBoxVectorTidyOperation&) = delete;
    NativeGuiFrameBoxVectorTidyOperation& operator=(const NativeGuiFrameBoxVectorTidyOperation&) = delete;
    NativeGuiFrameBoxVectorTidyOperation(NativeGuiFrameBoxVectorTidyOperation&&) = delete;
    NativeGuiFrameBoxVectorTidyOperation& operator=(NativeGuiFrameBoxVectorTidyOperation&&) = delete;

    NativeGuiFrameBoxVectorTidyPhase phase() const noexcept { return phase_; }
    void* header_identity() const noexcept { return header_; }
    void* expected_first_identity() const noexcept { return expected_first_; }
    void* expected_end_identity() const noexcept { return expected_end_; }
    std::size_t prepared_record_count() const noexcept { return record_count_; }
    void* captured_first_identity() const noexcept { return captured_first_; }
    void* captured_end_identity() const noexcept { return captured_end_; }
    bool end_captured() const noexcept { return end_captured_; }
    void* cursor_identity() const noexcept { return cursor_; }
    std::size_t completed_records() const noexcept { return completed_; }
    void* free_identity() const noexcept { return free_identity_; }
    bool free_entered() const noexcept { return free_entered_; }
    bool free_returned() const noexcept { return free_returned_; }
    bool header_cleared() const noexcept { return header_cleared_; }
private:
    friend void tidy_native_gui_frame_box_vector_00ad21b0(void*, NativeRenderActualOwners&,
        NativeStringRawPoolContext&, NativeGuiFrameBoxVectorTidyOperation&);
    void* const header_;
    void* const expected_first_;
    void* const expected_end_;
    NativeGuiFrameBoxRecordReleaseOperation* const records_;
    const std::size_t record_count_;
    NativeGuiFrameBoxVectorTidyPhase phase_{};
    void* captured_first_{};
    void* captured_end_{};
    void* cursor_{};
    std::size_t completed_{};
    void* free_identity_{};
    bool end_captured_{}, free_entered_{}, free_returned_{}, header_cleared_{};
};

// Whole AD21B0 [AD21B0,AD21F6), ECX actual16B header, RET. AD2320 is the
// existing undefined five-byte tail to it. These Source APIs add explicit
// services/preparation; neither reproduces Original binary/private-EH ABI.
// Header+0 is untouched; +4/+8/+C are actual raw DWORD cells, not std::vector.
// Borrow SAME actual owners/strings and ONLY111's genuine registered2D family.
// All records are successfully constructed owners; a failed103 record is NOT
// eligible. AD1910 committed-prefix rollback is a separate unprovided caller.
//
// Caller independently retains the full graph, captured record backing, any
// current matching SDK-free allocation, header and prepared metadata BEFORE
// entry and through all applicable actions. Header/metadata cannot be inside
// freed backing. Later header changes do not revoke captured range lifetime.
// CURRENT header+4 after callbacks must be null or a valid std::free-compatible
// Source allocation base; no Original/private/foreign heap or new/delete/pool
// compatibility is inferred. Do not discover retention from container links.
//
// Fresh/identity/known-region/metadata preflight rejections precede raw effects
// and leave the operation fresh. Then mark entered and capture begin once;
// capture end ONLY for nonnull begin. Prepared-range mismatch is sticky entered
// Source failure before any record callback, not an Original validation rule.
// Forward111, advance ONLY after return; current begin reload then concrete
// matching SDK free (even null), then +4/+8/+C zeros. No cached-begin extra free.
// On any escaping C++ exception retain the failed operation/graph; do not retry
// a record, free the buffer, or zero the header. Scheduling flags are not a
// Native instruction/exception-origin claim. Inherit111's atomic/noexcept/
// compiler-EH limits; SDK free and /EHsc do not supply Native CRT/FH3/SEH proof.
void tidy_native_gui_frame_box_vector_00ad21b0(void* actual_header,
    NativeRenderActualOwners& actual_owners, NativeStringRawPoolContext& actual_strings,
    NativeGuiFrameBoxVectorTidyOperation& operation);
void cleanup_native_gui_frame_box_vector_00ad2320(void* actual_header,
    NativeRenderActualOwners& actual_owners, NativeStringRawPoolContext& actual_strings,
    NativeGuiFrameBoxVectorTidyOperation& operation); // SAME operation, one attempt.
} // namespace bsp


namespace bsp {
enum class NativeGuiFrameBoxRecordCopyPhase : std::uint8_t {
    fresh, entered, returned, failed
};

// Explicit Source orchestration, not a Native object, owner or EH frame.
// Required preparation supplies an exact aligned 2Ch source span, a disjoint
// equally sized fresh destination span, and one live fresh111 operation per
// record. Empty spans may use null source/destination; nonempty null backing
// is rejected. The Native null-destination skip is outside this admitted domain.
// Known regions are validated in the FIRST initializer before member stores;
// this does not legalize invalid C++ placement or restore overwritten lifetimes.
// Full allocation membership, live backing, all wider aliases and canonical
// resource/pool services remain caller proof. No record payload is pre-read.
//
// Access is externally serialized. Callbacks must not mutate this operation
// or the prepared111 array. They must retain the SAME source/destination and
// complete independent graph/services BEFORE entry, outside failing/retired
// storage. Metadata owns no graph. Never discover ownership from container
// links after failure. Failure is sticky even after prefix rollback returns;
// another operation does not authorize replay of the same interrupted graph.
//
// Destroying entered/failed metadata terminates, reading only its phase.
// Fresh/returned destruction also reads metadata only. This fail-closed rule
// is a new Source lifetime obligation, not Native cleanup/FH3 behavior; it does
// not release, count-gate or repair any graph. Diagnostics may hold stale
// identities and must not be used to inspect already retired payloads.
class NativeGuiFrameBoxRecordCopyOperation final {
public:
    NativeGuiFrameBoxRecordCopyOperation(const void* first, const void* last,
        void* destination, NativeGuiFrameBoxRecordReleaseOperation* fresh_records,
        std::size_t record_count);
    ~NativeGuiFrameBoxRecordCopyOperation() noexcept;
    NativeGuiFrameBoxRecordCopyOperation(const NativeGuiFrameBoxRecordCopyOperation&) = delete;
    NativeGuiFrameBoxRecordCopyOperation& operator=(const NativeGuiFrameBoxRecordCopyOperation&) = delete;
    NativeGuiFrameBoxRecordCopyOperation(NativeGuiFrameBoxRecordCopyOperation&&) = delete;
    NativeGuiFrameBoxRecordCopyOperation& operator=(NativeGuiFrameBoxRecordCopyOperation&&) = delete;

    NativeGuiFrameBoxRecordCopyPhase phase() const noexcept { return phase_; }
    const void* source_first() const noexcept { return first_; }
    const void* source_last() const noexcept { return last_; }
    void* destination_first() const noexcept { return destination_; }
    std::size_t record_count() const noexcept { return record_count_; }
    const NativeGuiFrameBoxRecordReleaseOperation* release_operations() const noexcept { return records_; }
    const void* source_cursor() const noexcept { return source_cursor_; }
    void* committed_end() const noexcept { return committed_end_; }
    std::size_t committed_records() const noexcept { return committed_records_; }
    const void* attempted_source() const noexcept { return attempted_source_; }
    void* attempted_record() const noexcept { return attempted_record_; }
    bool record_copy_entered() const noexcept { return record_copy_entered_; }
    void* failed_record() const noexcept { return failed_record_; }
    bool rollback_entered() const noexcept { return rollback_entered_; }
    void* rollback_cursor() const noexcept { return rollback_cursor_; }
    std::size_t rolled_back_records() const noexcept { return rolled_back_records_; }
    const NativeGuiFrameBoxRecordReleaseOperation* last_release_operation() const noexcept { return last_release_operation_; }
    bool rollback_release_entered() const noexcept { return rollback_release_entered_; }
    bool rollback_completed() const noexcept { return rollback_completed_; }
    const NativeRenderActualOwners* actual_owners_identity() const noexcept { return actual_owners_; }
    const NativeStringRawPoolContext* actual_strings_identity() const noexcept { return actual_strings_; }
private:
    friend void* copy_native_gui_frame_box_uninitialized_range_00ad1910(
        const void*, const void*, void*, NativeStringRawPoolContext&,
        NativeRenderActualOwners&, NativeGuiFrameBoxRecordCopyOperation&);
    const void* const first_;
    const void* const last_;
    void* const destination_;
    NativeGuiFrameBoxRecordReleaseOperation* const records_;
    const std::size_t record_count_;
    NativeGuiFrameBoxRecordCopyPhase phase_{};
    const void* source_cursor_{};
    void* committed_end_{};
    std::size_t committed_records_{};
    const void* attempted_source_{};
    void* attempted_record_{};
    bool record_copy_entered_{};
    void* failed_record_{};
    bool rollback_entered_{};
    void* rollback_cursor_{};
    std::size_t rolled_back_records_{};
    const NativeGuiFrameBoxRecordReleaseOperation* last_release_operation_{};
    bool rollback_release_entered_{};
    bool rollback_completed_{};
    const NativeRenderActualOwners* actual_owners_{};
    const NativeStringRawPoolContext* actual_strings_{};
};

// Whole AD1910 normal body and separately identified AD196A prefix catch.
// Native ECX=first, EDX=cached last; four stack words=(destination,unused
// owner/header,two opaque words), EAX=advanced destination, RET10h. Source
// explicitly adds SAME raw-string/render services and persistent metadata;
// this ordinary C++ API does not reproduce that register/stack/private EH ABI.
// No eager payload snapshot: each genuine103 reads current source fields.
// Commit only after103 returns. On failure attempt forward111 rollback only
// over the completed prefix; the failed103 record NEVER reaches111. Advance
// rollback only after111 returns. Cleanup escape stops the loop and replaces
// the original exception under C++ rules; Native FH3/double-exception/SEH is
// not claimed. All phase flags describe Source scheduling, not fault origin.
// No allocation,119 tidy/free, parent end publication or automatic retirement.
void* copy_native_gui_frame_box_uninitialized_range_00ad1910(
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings, NativeRenderActualOwners& owners,
    NativeGuiFrameBoxRecordCopyOperation& operation);
} // namespace bsp
