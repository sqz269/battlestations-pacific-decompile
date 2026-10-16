#pragma once

#include "bsp/native_gui_frame_box_records.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw FrameBox vector copy requires MSVC Win32.
#endif

namespace bsp {

enum class NativeGuiFrameBoxVectorCopyPhase : std::uint8_t {
    fresh, entered, returned, failed, parent_catch_completed, failed_before_allocation
};

// Source orchestration only: this owns scheduling metadata, never a graph or
// allocation. Construct persistently outside the failing scope BEFORE entry,
// with distinct fresh111 metadata up to the caller-provided capacity. The first
// initializer validates known regions before member stores; no raw header or
// record payload is read. No default/copy/move/reset or public abandonment API.
// Header IDs, operation and complete111 array stay live/stable/disjoint; access
// is externally serialized and callbacks cannot mutate scheduling metadata.
// Full actual source/creator/resource/name/pool/renderer/manager/backing graphs
// are independently retained before callbacks, outside all retired storage.
// Metadata owns none of that graph and cannot discover it after failure.
//
// After Native allocation/publication, prepare an empty119 for THAT allocation
// before validation/copy callbacks. Only that empty unchanged current catch
// range is supported. Changed/null/nonempty current headers make real119 fail
// closed; no failed103 or already-rolled-back record is inspected or released.
// The copied current range after allocation/validation callbacks must be valid,
// fit the allocated byte extent and prepared111 capacity, and use the SAME
// current destination allocation. Other changes are retained Source rejection,
// not a promise that callbacks cannot change headers. No raw cap reread/repair.
//
// Inline128 preparation follows the Native cached-last/current-first/current-
// destination reads, before103 callbacks. Added preparation and still-fresh128
// entry-preflight failures retain the graph and skip invented Native cleanup.
// These software flags describe Source scheduling, never Native EH origins.
// Entered/failed destruction terminates reading only metadata. Fresh/returned/
// parent_catch_completed destruction touches child metadata only, never graph.
// Genuine119 must return before parent-only terminal transitions of failed128
// and its reached failed/entered111. Preserve diagnostic identities and prior
// phases; no reset/replay/dereference/automatic cleanup or fresh-metadata retry.
// That terminal state accepts orphaned names/resource credits as Native can;
// it does not recover them or discharge independently partial service lifetimes.
// Failure before the allocation service returns, with neither inline child
// constructed, instead becomes failed_before_allocation: metadata-only
// destruction is allowed without reset/reentry or any claim that callbacks'
// independently partial graphs were recovered. Caller lifetimes still apply.
class NativeGuiFrameBoxVectorCopyOperation final {
public:
    NativeGuiFrameBoxVectorCopyOperation(void* destination_header, const void* source_header,
        NativeGuiFrameBoxRecordReleaseOperation* fresh_records, std::size_t capacity);
    ~NativeGuiFrameBoxVectorCopyOperation() noexcept;
    NativeGuiFrameBoxVectorCopyOperation(const NativeGuiFrameBoxVectorCopyOperation&) = delete;
    NativeGuiFrameBoxVectorCopyOperation& operator=(const NativeGuiFrameBoxVectorCopyOperation&) = delete;
    NativeGuiFrameBoxVectorCopyOperation(NativeGuiFrameBoxVectorCopyOperation&&) = delete;
    NativeGuiFrameBoxVectorCopyOperation& operator=(NativeGuiFrameBoxVectorCopyOperation&&) = delete;

    NativeGuiFrameBoxVectorCopyPhase phase() const noexcept { return phase_; }
    void* destination_header() const noexcept { return destination_; }
    const void* source_header() const noexcept { return source_; }
    std::size_t prepared_capacity() const noexcept { return capacity_; }
    std::uint32_t initial_count() const noexcept { return initial_count_; }
    void* allocation_identity() const noexcept { return allocation_; }
    bool allocation_returned() const noexcept { return allocation_returned_; }
    std::uint32_t allocated_bytes() const noexcept { return allocated_bytes_; }
    const void* cached_source_last() const noexcept { return cached_last_; }
    const void* captured_source_first() const noexcept { return captured_first_; }
    void* captured_destination() const noexcept { return captured_destination_; }
    bool source_preparation_failed() const noexcept { return preparation_failed_; }
    bool native_catch_armed() const noexcept { return catch_armed_; }
    bool parent_catch_entered() const noexcept { return catch_entered_; }
    bool parent_catch_returned() const noexcept { return catch_returned_; }
    const NativeGuiFrameBoxRecordCopyOperation* copy_operation() const noexcept;
    const NativeGuiFrameBoxVectorTidyOperation* tidy_operation() const noexcept;
    NativeGuiFrameBoxRecordCopyPhase copy_phase_before_terminal() const noexcept { return prior_copy_phase_; }
    const NativeGuiFrameBoxRecordReleaseOperation* terminal_release_identity() const noexcept { return terminal_release_; }
    NativeGuiFrameBoxRecordReleasePhase release_phase_before_terminal() const noexcept { return prior_release_phase_; }
private:
    friend void* construct_native_gui_frame_box_vector_copy_00ad2230(void*, const void*,
        NativeStringRawPoolContext&, NativeRenderActualOwners&,
        NativeGuiFrameBoxVectorCopyOperation&);
    NativeGuiFrameBoxRecordCopyOperation* mutable_copy() noexcept;
    NativeGuiFrameBoxVectorTidyOperation* mutable_tidy() noexcept;
    void complete_parent_catch() noexcept;

    void* const destination_;
    const void* const source_;
    NativeGuiFrameBoxRecordReleaseOperation* const records_;
    const std::size_t capacity_;
    NativeGuiFrameBoxVectorCopyPhase phase_{};
    std::uint32_t initial_count_{}, allocated_bytes_{};
    void* allocation_{};
    const void* cached_last_{};
    const void* captured_first_{};
    void* captured_destination_{};
    bool allocation_returned_{}, tidy_constructed_{}, copy_constructed_{}, preparation_failed_{};
    bool catch_armed_{}, catch_entered_{}, catch_returned_{};
    NativeGuiFrameBoxRecordCopyPhase prior_copy_phase_{};
    NativeGuiFrameBoxRecordReleasePhase prior_release_phase_{};
    const NativeGuiFrameBoxRecordReleaseOperation* terminal_release_{};
    alignas(NativeGuiFrameBoxRecordCopyOperation)
        std::byte copy_storage_[sizeof(NativeGuiFrameBoxRecordCopyOperation)];
    alignas(NativeGuiFrameBoxVectorTidyOperation)
        std::byte tidy_storage_[sizeof(NativeGuiFrameBoxVectorTidyOperation)];
};

// Whole AD2230[AD2230,AD2306),214B, plus physical AD2306[AD2306,AD2317),17B.
// Native ECX=actual16B destination, stack source header, EAX=same destination,
// RET4. Source explicitly supplies services/metadata; no Original stack/FH3 ABI.
// Destination is fresh aligned16B storage with no existing owned allocation;
// distinct Source header/backing and all known metadata/service regions must
// be valid/disjoint. Source fields remain current through real callbacks.
// Initial signed wrapped DWORD delta/44 and real133/124/invalid-parameter calls
// precede genuine128. Only successful128 publishes end. Actual catch uses119
// and rethrows, subject to the narrow supported empty-range/preparation domain.
// SDK allocator/free, raw-name memmove/CRT and Win32 atomic/noexcept terminals
// retain their existing qualified Source boundaries. Ordinary111 escape can
// replace the first exception; noexcept terminal/nested cleanup can terminate.
// No private Native SEH/FH3/double-exception equivalence or normal host activation.
void* construct_native_gui_frame_box_vector_copy_00ad2230(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& actual_strings,
    NativeRenderActualOwners& actual_owners, NativeGuiFrameBoxVectorCopyOperation& operation);

} // namespace bsp
