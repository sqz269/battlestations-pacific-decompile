#pragma once

#include "bsp/ship_ai_avoid_zone_search.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
// Process storage adaptation, not the 009F3F20/009E4330 composite constructor.
// Owns its exact 2268h allocation; only 009E4401..009E444F searcher stores run.
// The MSVC Win32 views overlay the allocator's bytes without invoking the
// semantic cache defaults. Bounds/padding retain those bytes, including NaNs.
// No original allocator-history or complete native object-lifetime claim.
class ShipAiSearchStorage final {
public:
    static constexpr std::size_t storage_bytes = 0x2268;
    static constexpr std::size_t navigation_offset = 0x60;
    using BoundsBits = std::array<std::array<std::uint32_t, 4>,
        kShipAiAvoidZoneSearcherCount>;
    using CacheViews = std::array<ShipAiAvoidZoneSearcher*,
        kShipAiAvoidZoneSearcherCount>;

    // Copies the existing callback table and borrows its context. Record
    // allocation must return aligned storage or throw, as native 00BF681B does.
    // The matching free service/context must outlive this owner. Every selected
    // node added through a list view must use that same deallocation contract.
    explicit ShipAiSearchStorage(const AvoidZoneAllocationAccess&);
    ~ShipAiSearchStorage();
    ShipAiSearchStorage(const ShipAiSearchStorage&) = delete;
    ShipAiSearchStorage& operator=(const ShipAiSearchStorage&) = delete;
    ShipAiSearchStorage(ShipAiSearchStorage&&) noexcept;
    ShipAiSearchStorage& operator=(ShipAiSearchStorage&&) noexcept;

    // Preconditions: a nonempty owner and index < 3. These are live aliases to
    // the same 20h records, not cache/list copies. Their nodes must be disjoint
    // across lists and obey the recovered next/closes_run/next_run topology.
    ShipAiAvoidZoneSearcher& cache(std::size_t index) noexcept;
    const ShipAiAvoidZoneSearcher& cache(std::size_t index) const noexcept;
    ShipAiAvoidZoneSegmentList& list(std::size_t index) noexcept;
    const ShipAiAvoidZoneSegmentList& list(std::size_t index) const noexcept;
    CacheViews cache_views() noexcept; // All null for a moved-from owner.

    // Snapshot taken with byte copies before any floating-point access. This
    // remains the original allocation input when live queries change bounds.
    const BoundsBits& initial_bounds_bits() const noexcept { return initial_bounds_; }
    const unsigned char* data() const noexcept { return backing_; }

    // Move transfers backing without relocating records. Existing aliases to
    // the transferred allocation remain valid until the receiving owner dies.
    // Move assignment first clears/releases its old allocation, invalidating
    // aliases into that allocation. Moved-from data() is null.
private:
    void release() noexcept;
    AvoidZoneAllocationAccess allocation_;
    unsigned char* backing_{};
    BoundsBits initial_bounds_;
};
} // namespace bsp
