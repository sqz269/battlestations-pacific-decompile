#include "bsp/native_gui_frame_box_vector_copy.hpp"

#include "bsp/native_gui_frame_box_vector_allocation.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"

#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Address = std::uintptr_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(std::size_t) == 4);

struct Region { Address first, last; };
struct KnownRegions { Region destination, source, operation, records; };

Region region(const volatile void* pointer, std::size_t bytes) {
    const auto first = reinterpret_cast<Address>(pointer);
    if ((bytes != 0 && first == 0) || bytes > (std::numeric_limits<Address>::max)() - first)
        throw std::invalid_argument("raw vector copy known region is null or wraps");
    return {first, first + bytes};
}

bool overlaps(Region a, Region b) noexcept {
    return a.first != a.last && b.first != b.last && a.first < b.last && b.first < a.last;
}

void separate(Region a, Region b) {
    if (overlaps(a, b))
        throw std::invalid_argument("raw vector copy known regions overlap");
}

KnownRegions validate_preparation(const void* operation, void* destination, const void* source,
    NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t capacity) {
    if ((reinterpret_cast<Address>(destination) & 3u) != 0 ||
        (reinterpret_cast<Address>(source) & 3u) != 0 ||
        (reinterpret_cast<Address>(operation) % alignof(NativeGuiFrameBoxVectorCopyOperation)) != 0 ||
        (capacity != 0 && reinterpret_cast<Address>(records) %
            alignof(NativeGuiFrameBoxRecordReleaseOperation) != 0) ||
        capacity > (std::numeric_limits<std::size_t>::max)() /
            sizeof(NativeGuiFrameBoxRecordReleaseOperation))
        throw std::invalid_argument("raw vector copy metadata alignment or capacity invalid");
    const KnownRegions known{region(destination, 0x10), region(source, 0x10),
        region(operation, sizeof(NativeGuiFrameBoxVectorCopyOperation)),
        region(records, capacity * sizeof(NativeGuiFrameBoxRecordReleaseOperation))};
    separate(known.destination, known.source);
    separate(known.destination, known.operation);
    separate(known.destination, known.records);
    separate(known.source, known.operation);
    separate(known.source, known.records);
    separate(known.operation, known.records);
    for (std::size_t i = 0; i != capacity; ++i) {
        if (records[i].phase() != NativeGuiFrameBoxRecordReleasePhase::fresh)
            throw std::invalid_argument("raw vector copy requires fresh independent111 metadata");
    }
    return known;
}

void validate_service(const KnownRegions& known, const volatile void* service, std::size_t bytes) {
    const auto actual = region(service, bytes);
    separate(actual, known.destination);
    separate(actual, known.source);
    separate(actual, known.operation);
    separate(actual, known.records);
}

Word load(const void* header, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const std::byte*>(header) + offset);
}

void store(void* header, std::size_t offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(header) + offset) = value;
}

void* pointer(Word address) noexcept { return reinterpret_cast<void*>(static_cast<Address>(address)); }

std::size_t validate_current_copy(const KnownRegions& known, const void* first, const void* last,
    void* destination, void* allocation, Word allocated_bytes, std::size_t capacity) {
    const auto begin = reinterpret_cast<Address>(first);
    const auto end = reinterpret_cast<Address>(last);
    if (destination != allocation || end < begin || ((end - begin) % 0x2cu) != 0 ||
        (begin & 3u) != 0 || (end & 3u) != 0 || end - begin > allocated_bytes ||
        (end - begin) / 0x2cu > capacity)
        throw std::invalid_argument("raw vector copy current range outside prepared allocation domain");
    const auto source = region(first, end - begin);
    const auto target = region(destination, end - begin);
    separate(source, target);
    for (const auto span : {source, target}) {
        separate(span, known.destination);
        separate(span, known.source);
        separate(span, known.operation);
        separate(span, known.records);
    }
    return (end - begin) / 0x2cu;
}
} // namespace

NativeGuiFrameBoxVectorCopyOperation::NativeGuiFrameBoxVectorCopyOperation(void* destination,
    const void* source, NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t capacity)
    : destination_((validate_preparation(this, destination, source, records, capacity), destination)),
      source_(source), records_(records), capacity_(capacity) {}

NativeGuiFrameBoxRecordCopyOperation* NativeGuiFrameBoxVectorCopyOperation::mutable_copy() noexcept {
    return std::launder(reinterpret_cast<NativeGuiFrameBoxRecordCopyOperation*>(copy_storage_));
}

NativeGuiFrameBoxVectorTidyOperation* NativeGuiFrameBoxVectorCopyOperation::mutable_tidy() noexcept {
    return std::launder(reinterpret_cast<NativeGuiFrameBoxVectorTidyOperation*>(tidy_storage_));
}

const NativeGuiFrameBoxRecordCopyOperation*
NativeGuiFrameBoxVectorCopyOperation::copy_operation() const noexcept {
    return copy_constructed_ ? std::launder(
        reinterpret_cast<const NativeGuiFrameBoxRecordCopyOperation*>(copy_storage_)) : nullptr;
}

const NativeGuiFrameBoxVectorTidyOperation*
NativeGuiFrameBoxVectorCopyOperation::tidy_operation() const noexcept {
    return tidy_constructed_ ? std::launder(
        reinterpret_cast<const NativeGuiFrameBoxVectorTidyOperation*>(tidy_storage_)) : nullptr;
}

NativeGuiFrameBoxVectorCopyOperation::~NativeGuiFrameBoxVectorCopyOperation() noexcept {
    if (phase_ == NativeGuiFrameBoxVectorCopyPhase::entered ||
        phase_ == NativeGuiFrameBoxVectorCopyPhase::failed)
        std::terminate();
    if (copy_constructed_)
        mutable_copy()->~NativeGuiFrameBoxRecordCopyOperation();
    if (tidy_constructed_)
        mutable_tidy()->~NativeGuiFrameBoxVectorTidyOperation();
}

void NativeGuiFrameBoxVectorCopyOperation::complete_parent_catch() noexcept {
    if (copy_constructed_) {
        auto* const copy = mutable_copy();
        prior_copy_phase_ = copy->phase_;
        if (copy->phase_ == NativeGuiFrameBoxRecordCopyPhase::failed) {
            auto* const child = const_cast<NativeGuiFrameBoxRecordReleaseOperation*>(
                copy->last_release_operation_);
            if (child != nullptr && (child->phase_ == NativeGuiFrameBoxRecordReleasePhase::entered ||
                                    child->phase_ == NativeGuiFrameBoxRecordReleasePhase::failed)) {
                terminal_release_ = child;
                prior_release_phase_ = child->phase_;
                child->phase_ = NativeGuiFrameBoxRecordReleasePhase::parent_catch_completed;
            }
            copy->phase_ = NativeGuiFrameBoxRecordCopyPhase::parent_catch_completed;
        }
    }
    phase_ = NativeGuiFrameBoxVectorCopyPhase::parent_catch_completed;
}

void* construct_native_gui_frame_box_vector_copy_00ad2230(void* destination, const void* source,
    NativeStringRawPoolContext& strings, NativeRenderActualOwners& owners,
    NativeGuiFrameBoxVectorCopyOperation& operation) {
    if (operation.phase_ != NativeGuiFrameBoxVectorCopyPhase::fresh ||
        destination != operation.destination_ || source != operation.source_)
        throw std::logic_error("raw vector copy requires its fresh exact-header operation");
    const auto known = validate_preparation(&operation, destination, source,
        operation.records_, operation.capacity_);
    validate_service(known, &strings, sizeof(strings));
    validate_service(known, &owners, sizeof(owners));
    validate_service(known, &strings.actual_published_01090aa8, sizeof(void*));
    validate_service(known, &strings.actual_small_returns_disabled_01090aa4, sizeof(Word));
    validate_service(known, &strings.actual_manager_publication_01090aa0, sizeof(void*));
    operation.phase_ = NativeGuiFrameBoxVectorCopyPhase::entered;
    try {
        const Word initial_first = load(source, 4);
        Word count = 0;
        if (initial_first != 0) {
            const Word delta = load(source, 8) - initial_first;
            std::int32_t signed_delta;
            std::memcpy(&signed_delta, &delta, sizeof(delta));
            count = static_cast<Word>(signed_delta / 0x2c);
        }
        operation.initial_count_ = count;
        store(destination, 4, 0);
        store(destination, 8, 0);
        store(destination, 0xc, 0);
        if (count == 0) {
            operation.phase_ = NativeGuiFrameBoxVectorCopyPhase::returned;
            return destination;
        }
        if (count > 0x05d1745du)
            throw_native_gui_frame_box_vector_length_error_00ad1810();
        auto* const allocation = allocate_native_gui_frame_box_records_00acef10(count, nullptr);
        operation.allocation_returned_ = true;
        operation.allocation_ = allocation;
        const Word first = static_cast<Word>(reinterpret_cast<Address>(allocation));
        const Word bytes = count * 0x2cu;
        operation.allocated_bytes_ = bytes;
        store(destination, 4, first);
        store(destination, 8, first);
        store(destination, 0xc, first + bytes);
        try {
            ::new (operation.tidy_storage_) NativeGuiFrameBoxVectorTidyOperation(
                destination, allocation, allocation, nullptr, 0);
            operation.tidy_constructed_ = true;
        } catch (...) {
            operation.preparation_failed_ = true;
            throw;
        }
        const Word cached_last = load(source, 8);
        operation.cached_last_ = pointer(cached_last);
        const bool first_invalid = load(source, 4) > cached_last;
        operation.catch_armed_ = true;
        if (first_invalid)
            invoke_native_invalid_parameter_00bf6713();
        const Word current_first = load(source, 4);
        operation.captured_first_ = pointer(current_first);
        if (current_first > load(source, 8))
            invoke_native_invalid_parameter_00bf6713();
        auto* const current_destination = pointer(load(destination, 4));
        operation.captured_destination_ = current_destination;
        try {
            const auto records = validate_current_copy(known, pointer(current_first),
                pointer(cached_last), current_destination, allocation, bytes, operation.capacity_);
            ::new (operation.copy_storage_) NativeGuiFrameBoxRecordCopyOperation(
                pointer(current_first), pointer(cached_last), current_destination,
                operation.records_, records);
            operation.copy_constructed_ = true;
        } catch (...) {
            operation.preparation_failed_ = true;
            throw;
        }
        void* copied_end;
        try {
            copied_end = copy_native_gui_frame_box_uninitialized_range_00ad1910(
                pointer(current_first), pointer(cached_last), current_destination,
                strings, owners, *operation.mutable_copy());
        } catch (...) {
            if (operation.mutable_copy()->phase() == NativeGuiFrameBoxRecordCopyPhase::fresh)
                operation.preparation_failed_ = true;
            throw;
        }
        store(destination, 8, static_cast<Word>(reinterpret_cast<Address>(copied_end)));
        operation.phase_ = NativeGuiFrameBoxVectorCopyPhase::returned;
        return destination;
    } catch (...) {
        if (!operation.allocation_returned_ && !operation.copy_constructed_ &&
            !operation.tidy_constructed_) {
            operation.phase_ = NativeGuiFrameBoxVectorCopyPhase::failed_before_allocation;
            throw;
        }
        operation.phase_ = NativeGuiFrameBoxVectorCopyPhase::failed;
        if (!operation.catch_armed_ || operation.preparation_failed_)
            throw;
        operation.catch_entered_ = true;
        tidy_native_gui_frame_box_vector_00ad21b0(destination, owners, strings,
            *operation.mutable_tidy());
        operation.catch_returned_ = true;
        operation.complete_parent_catch();
        throw;
    }
}
} // namespace bsp
