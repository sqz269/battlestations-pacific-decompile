#include "bsp/ship_ai_search_storage.hpp"
#include <cassert>
#include <cstring>
#include <type_traits>
#include <utility>

namespace bsp {
namespace {
// Layout check only: never construct this aggregate, because its existing
// members have semantic default initializers that overwrite native raw bounds.
struct SearchRecord {
    ShipAiAvoidZoneSearcher cache;
    ShipAiAvoidZoneSegmentList list;
};
static_assert(sizeof(void*) == 4 && sizeof(bool) == 1);
static_assert(std::is_standard_layout_v<SearchRecord>);
static_assert(std::is_trivially_copyable_v<SearchRecord>);
static_assert(sizeof(ShipAiAvoidZoneSearcher) == 0x18);
static_assert(sizeof(ShipAiAvoidZoneSegmentList) == 8);
static_assert(alignof(SearchRecord) == 4);
static_assert(sizeof(SearchRecord) == kShipAiAvoidZoneSearcherStride);
static_assert(offsetof(SearchRecord, cache) == 0);
static_assert(offsetof(SearchRecord, list) == kShipAiAvoidZoneSearcherOffList);
static_assert(offsetof(ShipAiAvoidZoneSearcher, enabled) == 0);
static_assert(offsetof(ShipAiAvoidZoneSearcher, min_x) == 4);
static_assert(offsetof(ShipAiAvoidZoneSearcher, min_z) == 8);
static_assert(offsetof(ShipAiAvoidZoneSearcher, max_x) == 0x0c);
static_assert(offsetof(ShipAiAvoidZoneSearcher, max_z) == 0x10);
static_assert(offsetof(ShipAiAvoidZoneSearcher, layer_key) == 0x14);
static_assert(offsetof(ShipAiAvoidZoneSegmentList, head) == 0);
static_assert(offsetof(ShipAiAvoidZoneSegmentList, layer) == 4);
constexpr std::size_t first_record = ShipAiSearchStorage::navigation_offset
    + kShipAiBlkOffAvoidZoneSearchers;
static_assert(first_record == 0xa84);
static_assert(first_record % alignof(SearchRecord) == 0);
static_assert(first_record + 3 * sizeof(SearchRecord) <= ShipAiSearchStorage::storage_bytes);

SearchRecord& record(unsigned char* backing, std::size_t index) noexcept {
    assert(backing && index < kShipAiAvoidZoneSearcherCount);
    // Deliberate MSVC Win32 raw-object overlay, as in the other native storage
    // adapters. No placement construction, full-record copy or FP load here.
    return *reinterpret_cast<SearchRecord*>(backing + first_record
        + index * sizeof(SearchRecord));
}
} // namespace

ShipAiSearchStorage::ShipAiSearchStorage(const AvoidZoneAllocationAccess& allocation)
    : allocation_(allocation) {
    // Required native allocation contract is nonnull-or-throw, with matching
    // free callbacks; no allocation fallback, zero fill or exception swallowing.
    backing_ = static_cast<unsigned char*>(allocation_.allocate_record_00bf681b(
        allocation_.context, static_cast<std::uint32_t>(storage_bytes)));
    for (std::size_t i = 0; i < kShipAiAvoidZoneSearcherCount; ++i) {
        std::memcpy(initial_bounds_[i].data(), backing_ + first_record
            + i * sizeof(SearchRecord) + 4, sizeof(initial_bounds_[i]));
    }
    for (std::size_t i = 0; i < kShipAiAvoidZoneSearcherCount; ++i) {
        auto& current = record(backing_, i);
        // Native order at 009E4401/07/11/17, repeated at 441E and 4437.
        current.list.head = nullptr;
        current.list.layer = 0;
        current.cache.layer_key = -1;
        current.cache.enabled = true;
    }
}

ShipAiSearchStorage::~ShipAiSearchStorage() { release(); }
ShipAiSearchStorage::ShipAiSearchStorage(ShipAiSearchStorage&& other) noexcept
    : allocation_(other.allocation_), backing_(std::exchange(other.backing_, nullptr)),
      initial_bounds_(other.initial_bounds_) {}

ShipAiSearchStorage& ShipAiSearchStorage::operator=(ShipAiSearchStorage&& other) noexcept {
    if (this != &other) {
        release();
        allocation_ = other.allocation_;
        backing_ = std::exchange(other.backing_, nullptr);
        initial_bounds_ = other.initial_bounds_;
    }
    return *this;
}

void ShipAiSearchStorage::release() noexcept {
    if (!backing_) return;
    for (std::size_t i = 0; i < kShipAiAvoidZoneSearcherCount; ++i)
        avoid_zone_selected_segments_clear_004158a0(record(backing_, i).list.head, allocation_);
    allocation_.free_record_00bf65ac(allocation_.context, backing_);
    backing_ = nullptr;
}

ShipAiAvoidZoneSearcher& ShipAiSearchStorage::cache(std::size_t index) noexcept {
    return record(backing_, index).cache;
}
const ShipAiAvoidZoneSearcher& ShipAiSearchStorage::cache(std::size_t index) const noexcept {
    return record(backing_, index).cache;
}
ShipAiAvoidZoneSegmentList& ShipAiSearchStorage::list(std::size_t index) noexcept {
    return record(backing_, index).list;
}
const ShipAiAvoidZoneSegmentList& ShipAiSearchStorage::list(std::size_t index) const noexcept {
    return record(backing_, index).list;
}
ShipAiSearchStorage::CacheViews ShipAiSearchStorage::cache_views() noexcept {
    CacheViews result{};
    if (backing_)
        for (std::size_t i = 0; i < result.size(); ++i) result[i] = &cache(i);
    return result;
}
} // namespace bsp
