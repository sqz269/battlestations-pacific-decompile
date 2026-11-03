#pragma once

// Goal 3 chain 234 (kWireGoal3Chain234Bound). The descriptor half of the
// VehicleClass registry singleton 00437F50 (cache data +4h, size +8h,
// capacity +0Ch; docs/VEHICLE_CLASS_DESCRIPTORS.md) and the image descriptors
// 00964790 BSP_VehicleClass_GetOrCreate builds into it.
//
// Producer: 00964790 itself (bsp::resolve_vehicle_class_00964790), reached in
// this host through GameMissionLuaHost::mark_vehicle_class_row_00964790, the
// host's existing 00964790 projection. Each descriptor is the zeroed storage
// 00964790 allocates at its branch head (00470B80 for twenty kinds, 00BF55BE +
// 00BF79F0 for Destroyer/Cruiser) followed by that branch's constructor:
// the 21 leaf constructors in vehicle_class.cpp / cc13_seg41_land.cpp, the
// plane base 007D1E50, the ship base 00963380, the vehicle base 00749050 and
// the damageable base 0087C640 under them.
//
// Lifetime: one cache per GameMissionLuaHost, created by the first ON resolve
// and released with it. The image clears the cache at world teardown
// (0095BA30 -> 00437BD0 shrink -> InterlockedDecrement(+4h) -> vtable slot 0).
// DIVERGENCE: no native destructor runs here (0095F1C0 and the damageable
// destructor sit behind Seg62Host, which nothing implements); the image bytes
// are freed, and the 58h damageable tree head 00877FA0 allocated inside
// 0087C640 is not.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "bsp/vehicle_class.hpp"

namespace bsp {
struct NativeStringRawPoolContext;
}

namespace bsp::game {

class GameVehicleClassDescriptorCache final {
public:
    // `strings` is the native string pool the damageable base 0087C640
    // borrows for its constructor-failure cleanup (GameVfsHost::raw_strings()).
    explicit GameVehicleClassDescriptorCache(bsp::NativeStringRawPoolContext& strings);
    ~GameVehicleClassDescriptorCache();
    GameVehicleClassDescriptorCache(const GameVehicleClassDescriptorCache&) = delete;
    GameVehicleClassDescriptorCache& operator=(const GameVehicleClassDescriptorCache&) = delete;

    // 00964790's cache operations on singleton+4h (bsp::VehicleClassHost).
    int cache_size() const noexcept;
    void resize_cache(int size);
    bsp::VehicleClassDescriptor* cached(int class_index) const noexcept;
    // 00965119..0096513A: release what the slot held, then store.
    void store_cached(int class_index, bsp::VehicleClassDescriptor* descriptor);

    // 00964790 step 5: the kind row's zeroed allocation and constructor.
    // Throws when `size` is not the row's descriptor size.
    bsp::VehicleClassDescriptor* construct_descriptor(int kind_index, std::uint32_t size);
    void destroy_descriptor(bsp::VehicleClassDescriptor* descriptor);

    // vtable slot +18h (IsKindOf) of the image descriptor, dispatched through
    // the primary vtable its constructor stored at +0h.
    bool descriptor_is_kind(const bsp::VehicleClassDescriptor* descriptor, int kind) const;

    // The image bytes behind a descriptor this cache constructed.
    const std::uint8_t* image(const bsp::VehicleClassDescriptor* descriptor) const noexcept;
    std::size_t constructed() const noexcept { return constructed_; }

private:
    struct Entry;
    struct Bindings;
    Entry* find(const bsp::VehicleClassDescriptor* descriptor) const noexcept;
    void release_slot(void* descriptor) noexcept;

    std::unique_ptr<Bindings> bindings_;
    std::vector<std::unique_ptr<Entry>> entries_;
    std::vector<void*> cache_;   // singleton+4h..+0Ch, VehicleClassDescriptor* slots
    std::size_t constructed_{0};
};

} // namespace bsp::game
