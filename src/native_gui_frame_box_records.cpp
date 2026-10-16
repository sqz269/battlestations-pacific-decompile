#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox record copy requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
Word word(const void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(raw) + offset);
}
} // namespace

void* copy_construct_native_gui_frame_box_record_00ad0c60(
    void* destination, const void* source, NativeStringRawPoolContext& strings) {
    const Word resource = word(source, 0);
    auto* const destination_name = static_cast<std::byte*>(destination) + 4;
    const auto* const source_name = static_cast<const std::byte*>(source) + 4;
    const bool same_header = destination_name == source_name;
    word(destination, 0) = resource;
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    if (!same_header) {
        const Word requested = word(source, 4);
        resize_native_string_header_0041dd40(destination_name, strings, requested, true);
        if (word(source, 4) != 0) {
            const Word count = word(destination, 4);
            const auto* const source_data = reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(word(source, 8)));
            auto* const destination_data = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(word(destination, 8)));
            // Existing Source byte-copy boundary: overlap is admitted and a
            // zero count does not invoke a standard copy with null pointers.
            if (count != 0) std::memmove(destination_data, source_data, count);
        }
    }
    __asm {
        mov edi, source
        mov esi, destination
        fld dword ptr [edi+0ch]
        fstp dword ptr [esi+0ch]
        fld dword ptr [edi+10h]
        fstp dword ptr [esi+10h]
        fld dword ptr [edi+14h]
        fstp dword ptr [esi+14h]
        fld dword ptr [edi+18h]
        fstp dword ptr [esi+18h]
    }
    word(destination, 0x1c) = word(source, 0x1c);
    word(destination, 0x20) = word(source, 0x20);
    word(destination, 0x24) = word(source, 0x24);
    word(destination, 0x28) = word(source, 0x28);
    auto* const current_resource = reinterpret_cast<std::byte*>(
        static_cast<std::uintptr_t>(word(destination, 0)));
    if (current_resource != nullptr)
        ::InterlockedIncrement(reinterpret_cast<volatile LONG*>(current_resource + 4));
    return destination;
}
} // namespace bsp


#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
NativeGuiFrameBoxRecordReleaseOperation::~NativeGuiFrameBoxRecordReleaseOperation() noexcept {
    if (phase_ == NativeGuiFrameBoxRecordReleasePhase::entered ||
        phase_ == NativeGuiFrameBoxRecordReleasePhase::failed)
        std::terminate();
}

void release_native_gui_frame_box_record_00ad0860(void* record,
    NativeRenderActualOwners& owners, NativeStringRawPoolContext& strings,
    NativeGuiFrameBoxRecordReleaseOperation& operation) {
    if (operation.phase_ != NativeGuiFrameBoxRecordReleasePhase::fresh)
        throw std::logic_error("raw FrameBox record release is one-shot");
    operation.record_ = record;
    operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::entered;
    auto* const resource = reinterpret_cast<std::byte*>(
        static_cast<std::uintptr_t>(word(record, 0)));
    operation.resource_ = resource;
    operation.name_action_armed_ = true;
    try {
        if (resource != nullptr) {
            operation.count_after_decrement_ = ::InterlockedDecrement(
                reinterpret_cast<volatile LONG*>(resource + 4));
            operation.resource_decremented_ = true;
            if (operation.count_after_decrement_ == 0) {
                operation.zero_dispatch_entered_ = true;
                dispatch_native_render_actual_owner_zero(owners, resource);
                operation.zero_dispatch_returned_ = true;
            }
            word(record, 0) = 0;
            operation.resource_cleared_ = true;
        }
        auto* const data = reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(word(record, 8)));
        operation.name_data_ = data;
        operation.name_action_armed_ = false;
        operation.name_action_consumed_ = true;
        if (data != nullptr) {
            const Word size = word(record, 4) + 1u;
            operation.name_size_ = size;
            operation.normal_name_return_entered_ = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size,
                strings.actual_small_returns_disabled_01090aa4);
            operation.normal_name_returned_ = true;
        }
        operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::returned;
    } catch (...) {
        operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::failed;
        if (operation.name_action_armed_) {
            operation.name_action_armed_ = false;
            operation.name_action_consumed_ = true;
            operation.failure_name_cleanup_entered_ = true;
            try {
                destroy_native_string_header_0041dd20(static_cast<std::byte*>(record) + 4, strings);
                operation.failure_name_cleanup_returned_ = true;
            } catch (...) {
                std::terminate();
            }
        }
        throw;
    }
}
} // namespace bsp


#include <limits>

namespace bsp {
namespace {
struct TidyKnownRegion {
    std::uintptr_t first, last;
};
TidyKnownRegion tidy_known_region(const volatile void* identity, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    if ((bytes != 0 && first == 0) || bytes > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("raw vector tidy region is null or wraps");
    return {first, first + bytes};
}
bool tidy_overlap(TidyKnownRegion left, TidyKnownRegion right) noexcept {
    return left.first != left.last && right.first != right.last &&
        left.first < right.last && right.first < left.last;
}
bool tidy_contains(TidyKnownRegion region, std::uintptr_t point) noexcept {
    return region.first <= point && point < region.last;
}
struct TidyKnownRegions {
    TidyKnownRegion header, records, metadata, operation;
};
TidyKnownRegions tidy_validate_preparation(const NativeGuiFrameBoxVectorTidyOperation& operation,
    void* header, void* first, void* last,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t record_count) {
    const auto begin = reinterpret_cast<std::uintptr_t>(first);
    const auto end = reinterpret_cast<std::uintptr_t>(last);
    if ((reinterpret_cast<std::uintptr_t>(header) & 3u) != 0 || (begin & 3u) != 0 ||
        (end & 3u) != 0 || end < begin || (begin == 0 && end != 0) ||
        (end - begin) % 0x2cu != 0 || (end - begin) / 0x2cu != record_count ||
        record_count > (std::numeric_limits<std::size_t>::max)() / sizeof(records[0]) ||
        (reinterpret_cast<std::uintptr_t>(records) % alignof(NativeGuiFrameBoxRecordReleaseOperation)) != 0)
        throw std::invalid_argument("raw vector tidy requires an exact aligned constructed span");
    const TidyKnownRegions known{tidy_known_region(header, 0x10),
        tidy_known_region(first, end - begin),
        tidy_known_region(records, record_count * sizeof(records[0])),
        tidy_known_region(&operation, sizeof(operation))};
    const TidyKnownRegion regions[]{known.header, known.records, known.metadata, known.operation};
    for (std::size_t i = 0; i != 4; ++i) {
        for (std::size_t j = i + 1; j != 4; ++j) {
            if (tidy_overlap(regions[i], regions[j]))
                throw std::invalid_argument("raw vector tidy known regions overlap");
        }
    }
    if (begin != 0 && (tidy_contains(known.header, begin) ||
        tidy_contains(known.metadata, begin) || tidy_contains(known.operation, begin)))
        throw std::invalid_argument("raw vector tidy backing aliases prepared metadata/header");
    for (std::size_t i = 0; i != record_count; ++i) {
        if (records[i].phase() != NativeGuiFrameBoxRecordReleasePhase::fresh)
            throw std::logic_error("raw vector tidy requires fresh per-record operations");
    }
    return known;
}
void tidy_validate_service_region(TidyKnownRegions known, const volatile void* identity,
    std::size_t bytes) {
    const auto service = tidy_known_region(identity, bytes);
    if (tidy_overlap(service, known.header) || tidy_overlap(service, known.records) ||
        tidy_overlap(service, known.metadata) || tidy_overlap(service, known.operation))
        throw std::invalid_argument("raw vector tidy service metadata overlaps known regions");
}
} // namespace

NativeGuiFrameBoxVectorTidyOperation::NativeGuiFrameBoxVectorTidyOperation(void* header,
    void* first, void* last, NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t record_count)
    : header_((tidy_validate_preparation(*this, header, first, last, records, record_count), header)),
      expected_first_(first), expected_end_(last), records_(records), record_count_(record_count) {}
NativeGuiFrameBoxVectorTidyOperation::~NativeGuiFrameBoxVectorTidyOperation() noexcept {
    if (phase_ == NativeGuiFrameBoxVectorTidyPhase::entered ||
        phase_ == NativeGuiFrameBoxVectorTidyPhase::failed)
        std::terminate();
}

void tidy_native_gui_frame_box_vector_00ad21b0(void* header,
    NativeRenderActualOwners& owners, NativeStringRawPoolContext& strings,
    NativeGuiFrameBoxVectorTidyOperation& operation) {
    if (operation.phase_ != NativeGuiFrameBoxVectorTidyPhase::fresh || header != operation.header_)
        throw std::logic_error("raw vector tidy requires its fresh exact-header operation");
    const auto known = tidy_validate_preparation(operation, header,
        operation.expected_first_, operation.expected_end_, operation.records_, operation.record_count_);
    tidy_validate_service_region(known, &owners, sizeof(NativeRenderActualOwners));
    tidy_validate_service_region(known, &strings, sizeof(strings));
    tidy_validate_service_region(known, &strings.actual_published_01090aa8, sizeof(void*));
    tidy_validate_service_region(known, &strings.actual_small_returns_disabled_01090aa4, sizeof(Word));
    tidy_validate_service_region(known, &strings.actual_manager_publication_01090aa0, sizeof(void*));
    operation.phase_ = NativeGuiFrameBoxVectorTidyPhase::entered;
    try {
        auto* const first = reinterpret_cast<std::byte*>(
            static_cast<std::uintptr_t>(word(header, 4)));
        operation.captured_first_ = first;
        operation.cursor_ = first;
        if (first != operation.expected_first_)
            throw std::invalid_argument("raw vector tidy captured begin differs from preparation");
        if (first != nullptr) {
            auto* const last = reinterpret_cast<std::byte*>(
                static_cast<std::uintptr_t>(word(header, 8)));
            operation.captured_end_ = last;
            operation.end_captured_ = true;
            if (last != operation.expected_end_)
                throw std::invalid_argument("raw vector tidy captured end differs from preparation");
            auto* cursor = first;
            while (cursor != last) {
                release_native_gui_frame_box_record_00ad0860(cursor, owners, strings,
                    operation.records_[operation.completed_]);
                cursor += 0x2c;
                operation.cursor_ = cursor;
                ++operation.completed_;
            }
            auto* const current = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(word(header, 4)));
            operation.free_identity_ = current;
            operation.free_entered_ = true;
            singleton_lifetime_free(current);
            operation.free_returned_ = true;
        }
        word(header, 4) = 0;
        word(header, 8) = 0;
        word(header, 0xc) = 0;
        operation.header_cleared_ = true;
        operation.phase_ = NativeGuiFrameBoxVectorTidyPhase::returned;
    } catch (...) {
        operation.phase_ = NativeGuiFrameBoxVectorTidyPhase::failed;
        throw;
    }
}
void cleanup_native_gui_frame_box_vector_00ad2320(void* header,
    NativeRenderActualOwners& owners, NativeStringRawPoolContext& strings,
    NativeGuiFrameBoxVectorTidyOperation& operation) {
    tidy_native_gui_frame_box_vector_00ad21b0(header, owners, strings, operation);
}
} // namespace bsp


namespace bsp {
namespace {
struct CopyKnownRegion {
    std::uintptr_t first, last;
};
CopyKnownRegion copy_known_region(const volatile void* identity, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    if ((bytes != 0 && first == 0) || bytes > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("raw record copy region is null or wraps");
    return {first, first + bytes};
}
bool copy_overlap(CopyKnownRegion left, CopyKnownRegion right) noexcept {
    return left.first != left.last && right.first != right.last &&
        left.first < right.last && right.first < left.last;
}
struct CopyKnownRegions {
    CopyKnownRegion source, destination, metadata, operation;
};
CopyKnownRegions copy_validate_preparation(const NativeGuiFrameBoxRecordCopyOperation& operation,
    const void* first, const void* last, void* destination,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t record_count) {
    const auto begin = reinterpret_cast<std::uintptr_t>(first);
    const auto end = reinterpret_cast<std::uintptr_t>(last);
    if ((begin & 3u) != 0 || (end & 3u) != 0 ||
        (reinterpret_cast<std::uintptr_t>(destination) & 3u) != 0 ||
        end < begin || (begin == 0 && end != 0) ||
        (end - begin) % 0x2cu != 0 || (end - begin) / 0x2cu != record_count ||
        record_count > (std::numeric_limits<std::size_t>::max)() / sizeof(records[0]) ||
        (reinterpret_cast<std::uintptr_t>(records) % alignof(NativeGuiFrameBoxRecordReleaseOperation)) != 0)
        throw std::invalid_argument("raw record copy requires exact aligned prepared spans");
    const CopyKnownRegions known{copy_known_region(first, end - begin),
        copy_known_region(destination, end - begin),
        copy_known_region(records, record_count * sizeof(records[0])),
        copy_known_region(&operation, sizeof(operation))};
    const CopyKnownRegion regions[]{known.source, known.destination, known.metadata, known.operation};
    for (std::size_t i = 0; i != 4; ++i) {
        for (std::size_t j = i + 1; j != 4; ++j) {
            if (copy_overlap(regions[i], regions[j]))
                throw std::invalid_argument("raw record copy known regions overlap");
        }
    }
    for (std::size_t i = 0; i != record_count; ++i) {
        if (records[i].phase() != NativeGuiFrameBoxRecordReleasePhase::fresh)
            throw std::logic_error("raw record copy requires fresh per-record release operations");
    }
    return known;
}
void copy_validate_service_region(CopyKnownRegions known, const volatile void* identity,
    std::size_t bytes) {
    const auto service = copy_known_region(identity, bytes);
    if (copy_overlap(service, known.source) || copy_overlap(service, known.destination) ||
        copy_overlap(service, known.metadata) || copy_overlap(service, known.operation))
        throw std::invalid_argument("raw record copy service metadata overlaps known regions");
}
} // namespace

NativeGuiFrameBoxRecordCopyOperation::NativeGuiFrameBoxRecordCopyOperation(
    const void* first, const void* last, void* destination,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t record_count)
    : first_((copy_validate_preparation(*this, first, last, destination, records, record_count), first)),
      last_(last), destination_(destination), records_(records), record_count_(record_count),
      source_cursor_(first), committed_end_(destination), rollback_cursor_(destination) {}

NativeGuiFrameBoxRecordCopyOperation::~NativeGuiFrameBoxRecordCopyOperation() noexcept {
    if (phase_ == NativeGuiFrameBoxRecordCopyPhase::entered ||
        phase_ == NativeGuiFrameBoxRecordCopyPhase::failed)
        std::terminate();
}

void* copy_native_gui_frame_box_uninitialized_range_00ad1910(
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings, NativeRenderActualOwners& owners,
    NativeGuiFrameBoxRecordCopyOperation& operation) {
    if (operation.phase_ != NativeGuiFrameBoxRecordCopyPhase::fresh ||
        first != operation.first_ || last != operation.last_ || destination != operation.destination_)
        throw std::logic_error("raw record copy requires its fresh exact-range operation");
    const auto known = copy_validate_preparation(operation, first, last, destination,
        operation.records_, operation.record_count_);
    copy_validate_service_region(known, &owners, sizeof(owners));
    copy_validate_service_region(known, &strings, sizeof(strings));
    copy_validate_service_region(known, &strings.actual_published_01090aa8, sizeof(void*));
    copy_validate_service_region(known, &strings.actual_small_returns_disabled_01090aa4, sizeof(Word));
    copy_validate_service_region(known, &strings.actual_manager_publication_01090aa0, sizeof(void*));
    operation.actual_owners_ = &owners;
    operation.actual_strings_ = &strings;
    operation.phase_ = NativeGuiFrameBoxRecordCopyPhase::entered;
    try {
        auto source = reinterpret_cast<std::uintptr_t>(first);
        const auto end = reinterpret_cast<std::uintptr_t>(last);
        auto target = reinterpret_cast<std::uintptr_t>(destination);
        while (source != end) {
            operation.attempted_source_ = reinterpret_cast<const void*>(source);
            operation.attempted_record_ = reinterpret_cast<void*>(target);
            operation.record_copy_entered_ = true;
            // Native's null skip is retained, but preparation excludes null
            // destination for every nonempty valid-buffer operation.
            if (target != 0)
                copy_construct_native_gui_frame_box_record_00ad0c60(
                    reinterpret_cast<void*>(target), reinterpret_cast<const void*>(source), strings);
            operation.record_copy_entered_ = false;
            target += 0x2cu;
            operation.committed_end_ = reinterpret_cast<void*>(target);
            ++operation.committed_records_;
            source += 0x2cu;
            operation.source_cursor_ = reinterpret_cast<const void*>(source);
        }
        operation.phase_ = NativeGuiFrameBoxRecordCopyPhase::returned;
        return reinterpret_cast<void*>(target);
    } catch (...) {
        operation.phase_ = NativeGuiFrameBoxRecordCopyPhase::failed;
        operation.failed_record_ = operation.attempted_record_;
        operation.rollback_entered_ = true;
        auto cursor = reinterpret_cast<std::uintptr_t>(destination);
        const auto committed = reinterpret_cast<std::uintptr_t>(operation.committed_end_);
        while (cursor != committed) {
            operation.last_release_operation_ = operation.records_ + operation.rolled_back_records_;
            operation.rollback_release_entered_ = true;
            release_native_gui_frame_box_record_00ad0860(reinterpret_cast<void*>(cursor), owners,
                strings, operation.records_[operation.rolled_back_records_]);
            operation.rollback_release_entered_ = false;
            cursor += 0x2cu;
            operation.rollback_cursor_ = reinterpret_cast<void*>(cursor);
            ++operation.rolled_back_records_;
        }
        operation.rollback_completed_ = true;
        throw;
    }
}
} // namespace bsp
