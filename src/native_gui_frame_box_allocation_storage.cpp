#include "bsp/native_gui_frame_box_allocation_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include <array>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual FrameBox factory requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Operation = NativeGuiFrameBoxFactoryOperation;
using Phase = NativeGuiFrameBoxFactoryPhase;
using Pool = NativeGuiWidgetPool<GuiFrameBoxPoolTraits>;
struct Region { std::uintptr_t first, last; };
void require(bool value, const char* message) {
    if (!value) throw std::invalid_argument(message);
}
Region region(const void* identity, std::size_t size) {
    if (size == 0) return {0, 0};
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    require(first != 0 && (first & 3u) == 0 &&
        size <= (std::numeric_limits<std::uintptr_t>::max)() - first,
        "FrameBox factory requires aligned nonwrapping live regions");
    return {first, first + size};
}
bool separate(Region a, Region b) noexcept {
    return a.first == a.last || b.first == b.last || a.last <= b.first || b.last <= a.first;
}
template<std::size_t N> void validate_regions(const std::array<Region, N>& regions) {
    for (std::size_t i = 0; i != N; ++i)
        for (std::size_t j = i + 1; j != N; ++j)
            require(separate(regions[i], regions[j]), "FrameBox factory known regions overlap");
}
Region records_region(NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t count) {
    require(count <= (std::numeric_limits<std::size_t>::max)() / sizeof(*records),
        "FrameBox factory prepared111 extent wraps");
    return region(records, count * sizeof(*records));
}
void require_fresh(NativeGuiFrameBoxRecordReleaseOperation* records, std::size_t count) {
    (void)records_region(records, count);
    for (std::size_t i = 0; i != count; ++i)
        require(records[i].phase() == NativeGuiFrameBoxRecordReleasePhase::fresh,
            "FrameBox factory requires its fresh prepared111 array");
}
const void* cell(const volatile std::uint32_t& value) noexcept {
    return const_cast<const std::uint32_t*>(&value);
}
Pool& validate_default(Operation* operation, NativeGuiFrameBoxFactoryPoolProcess& process,
    NativeGuiFrameBoxDefaultBindings& bindings) {
    validate_regions(std::array<Region, 6>{{region(operation, sizeof(*operation)),
        region(&process, sizeof(process)), region(&bindings, sizeof(bindings)),
        region(cell(bindings.actual_one_00d7a24c), 4),
        region(cell(bindings.actual_frame_x_00d5d11c), 4),
        region(cell(bindings.actual_frame_y_00d5d10c), 4)}});
    // The process has a private constructor/noncopyable mutex; legitimate
    // instances are its canonical instance. This getter requires returned startup.
    return process.pool();
}
Pool& validate_copy(Operation* operation, NativeGuiFrameBoxFactoryPoolProcess& process,
    const void* source, NativeGuiFrameBoxCopyStorageServices& services,
    NativeGuiFrameBoxCopyStorageFrames& frames, NativeGuiFrameBoxFactoryCopyStorage& backing,
    NativeGuiFrameBoxRecordReleaseOperation* rollback, std::size_t rollback_count,
    NativeGuiFrameBoxRecordReleaseOperation* destruction, std::size_t destruction_count) {
    validate_regions(std::array<Region, 8>{{region(operation, sizeof(*operation)),
        region(&process, sizeof(process)), region(source, 0x120),
        region(&services, sizeof(services)), region(&frames, sizeof(frames)),
        region(&backing, sizeof(backing)), records_region(rollback, rollback_count),
        records_region(destruction, destruction_count)}});
    require(!backing.attempted(), "FrameBox factory copy backing was already attempted");
    require_fresh(rollback, rollback_count);
    require_fresh(destruction, destruction_count);
    auto& pool = process.pool();
    require(&pool == &services.matching_pool, "FrameBox factory copy must use its SAME canonical pool");
    return pool;
}
template<class T> T* object(std::byte* storage) noexcept {
    return std::launder(reinterpret_cast<T*>(storage));
}
template<class T> const T* object(const std::byte* storage) noexcept {
    return std::launder(reinterpret_cast<const T*>(storage));
}
void* at(void* p, std::size_t offset) noexcept { return static_cast<std::byte*>(p) + offset; }
const void* at(const void* p, std::size_t offset) noexcept { return static_cast<const std::byte*>(p) + offset; }
} // namespace

const NativeGuiFrameBoxVectorCopyOperation*
NativeGuiFrameBoxFactoryCopyStorage::vector_operation() const noexcept {
    return vector_constructed_ ? object<NativeGuiFrameBoxVectorCopyOperation>(vector_storage_) : nullptr;
}
NativeGuiFrameBoxFactoryCopyStorage::~NativeGuiFrameBoxFactoryCopyStorage() noexcept {
    if (vector_constructed_)
        object<NativeGuiFrameBoxVectorCopyOperation>(vector_storage_)->~NativeGuiFrameBoxVectorCopyOperation();
}

Operation::NativeGuiFrameBoxFactoryOperation(NativeGuiFrameBoxFactoryPoolProcess& process,
    NativeGuiFrameBoxDefaultBindings& bindings)
    : pool_(validate_default(this, process, bindings)), source_(nullptr), default_(&bindings),
      services_(nullptr), frames_(nullptr), backing_(nullptr), rollback_(nullptr), rollback_capacity_(0),
      destruction_(nullptr), destruction_capacity_(0) {}

Operation::NativeGuiFrameBoxFactoryOperation(NativeGuiFrameBoxFactoryPoolProcess& process,
    const void* source, NativeGuiFrameBoxCopyStorageServices& services,
    NativeGuiFrameBoxCopyStorageFrames& frames, NativeGuiFrameBoxFactoryCopyStorage& backing,
    NativeGuiFrameBoxRecordReleaseOperation* rollback, std::size_t rollback_count,
    NativeGuiFrameBoxRecordReleaseOperation* destruction, std::size_t destruction_count)
    : pool_(validate_copy(this, process, source, services, frames, backing,
          rollback, rollback_count, destruction, destruction_count)),
      source_(source), default_(nullptr), services_(&services), frames_(&frames), backing_(&backing),
      rollback_(rollback), rollback_capacity_(rollback_count),
      destruction_(destruction), destruction_capacity_(destruction_count) {}

NativeGuiFrameBoxCopyStorageOperation* Operation::copy_operation() noexcept {
    return copy_constructed_ ? object<NativeGuiFrameBoxCopyStorageOperation>(copy_storage_) : nullptr;
}
const NativeGuiFrameBoxCopyStorageOperation* Operation::copy_operation() const noexcept {
    return copy_constructed_ ? object<NativeGuiFrameBoxCopyStorageOperation>(copy_storage_) : nullptr;
}
Operation::~NativeGuiFrameBoxFactoryOperation() noexcept {
    if (phase_ == Phase::allocating || phase_ == Phase::allocated || phase_ == Phase::preparing_copy ||
        phase_ == Phase::constructing_copy || phase_ == Phase::constructing_default ||
        phase_ == Phase::failed_retained || (pool_return_entered_ && !pool_return_returned_))
        std::terminate();
    if (copy_constructed_) copy_operation()->~NativeGuiFrameBoxCopyStorageOperation();
    // External late138/acquisitions/graphs are NOT destroyed here. Retired184
    // and its borrowed163/default base destroy metadata without a slot/count read.
}

void Operation::begin_allocation() noexcept {
    phase_ = Phase::allocating;
    if (backing_) backing_->attempted_ = true;
}
void Operation::validate_slot() {
    const auto slot = region(slot_, 0x120);
    require(separate(slot, region(this, sizeof(*this))) &&
        separate(slot, region(&pool_, sizeof(pool_))), "FrameBox factory slot overlaps metadata");
    if (source_) {
        const Region held[] = {region(source_, 0x120), region(services_, sizeof(*services_)),
            region(frames_, sizeof(*frames_)), region(backing_, sizeof(*backing_)),
            records_region(rollback_, rollback_capacity_), records_region(destruction_, destruction_capacity_)};
        for (const auto r : held)
            require(separate(slot, r), "FrameBox factory copy slot overlaps its known live graph");
    } else {
        const Region held[] = {region(default_, sizeof(*default_)),
            region(cell(default_->actual_one_00d7a24c), 4),
            region(cell(default_->actual_frame_x_00d5d11c), 4),
            region(cell(default_->actual_frame_y_00d5d10c), 4)};
        for (const auto r : held)
            require(separate(slot, r), "FrameBox factory default slot overlaps its borrowed cells");
    }
    slot_checked_ = true;
}
void Operation::prepare_copy() {
    auto* vector = ::new (static_cast<void*>(backing_->vector_storage_)) NativeGuiFrameBoxVectorCopyOperation(
        at(slot_, 0xf0), at(source_, 0xf0), rollback_, rollback_capacity_);
    backing_->vector_constructed_ = true;
    ::new (static_cast<void*>(copy_storage_)) NativeGuiFrameBoxCopyStorageOperation(
        slot_, source_, *services_, *frames_, *vector, destruction_, destruction_capacity_);
    copy_constructed_ = true;
}
bool Operation::can_return_failed_slot() const noexcept {
    if (!slot_checked_) return false;
    if (!source_ || !copy_constructed_) return true; // No entered copy body.
    const auto* copy = copy_operation();
    return copy->phase() == NativeGuiFrameBoxCopyStoragePhase::fresh ||
        (copy->phase() == NativeGuiFrameBoxCopyStoragePhase::failed_unwind_returned && copy->unwind_returned());
}
void Operation::return_failed_slot() noexcept {
    slot_action_ = -1; // Consume BEFORE genuine same-pool return; no repeated action.
    pool_return_entered_ = true;
    pool_.free(slot_);
    pool_return_returned_ = true;
    phase_ = Phase::failed_slot_returned; // Saved metadata ONLY after slot return.
}

void* allocate_native_gui_frame_box_00aa1a40(const void* source, Operation& operation) {
    if (operation.phase_ != Phase::fresh || source != operation.source_)
        throw std::logic_error("FrameBox factory requires its fresh exact-source operation");
    if (source) {
        require(!operation.backing_->attempted(), "FrameBox factory copy backing was already attempted");
        require_fresh(operation.rollback_, operation.rollback_capacity_);
        require_fresh(operation.destruction_, operation.destruction_capacity_);
    }
    const void* const captured_source = source; // Native ESI captured BEFORE allocator callbacks.
    operation.begin_allocation();
    try {
        void* const slot = operation.pool_.allocate();
        operation.slot_ = slot;
        operation.slot_action_ = captured_source ? 0 : 1;
        operation.phase_ = Phase::allocated;
        if (!slot) {
            operation.phase_ = Phase::returned_null;
            return nullptr;
        }
        operation.validate_slot(); // Added Source admission; no raw header/payload read.
        void* result;
        if (captured_source) {
            operation.phase_ = Phase::preparing_copy;
            operation.prepare_copy();
            operation.phase_ = Phase::constructing_copy;
            operation.constructor_entered_ = true;
            result = copy_native_gui_frame_box_storage_00ad27a0(slot, captured_source, *operation.copy_operation());
        } else {
            operation.phase_ = Phase::constructing_default;
            operation.constructor_entered_ = true;
            result = construct_native_gui_frame_box_default_00ad2600(slot, *operation.default_);
        }
        operation.result_ = result;
        operation.phase_ = Phase::returned;
        return result; // Actual constructor result, no replacement identity/extra hook.
    } catch (...) {
        if (!operation.slot_) {
            operation.phase_ = Phase::failed_before_slot;
        } else if (operation.can_return_failed_slot()) {
            operation.return_failed_slot();
        } else {
            operation.phase_ = Phase::failed_retained;
        }
        throw;
    }
}
} // namespace bsp
