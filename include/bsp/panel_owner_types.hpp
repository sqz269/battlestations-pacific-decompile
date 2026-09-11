#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual Win32 palette header. This owns native 24h nodes, not a std::map.
// The constructor at00452660 does not initialize allocator_00.
struct PanelPaletteTreeStorage {
    std::uint32_t allocator_00;
    void* head_04;
    std::uint32_t count_08;
};

// Character and sequence containers use the existing standard-map projections.
// Their unused native allocator words remain explicit state; no shadow native
// tree is created for those projections. This is not a native38h owner layout.
struct PanelOwnerNativeStorage {
    std::uint32_t vtable_00;
    std::uint32_t character_allocator_04;
    PanelPaletteTreeStorage palette_10;
    std::uint32_t queue_allocator_1c;
};

// Required captured words from the allocation, BEFORE00452660. Native leaves
// all five untouched. No default pause, state, or allocator values are inferred.
struct PanelOwnerAllocationWords {
    std::uint32_t character_allocator_04;
    std::uint32_t palette_allocator_10;
    std::uint32_t queue_allocator_1c;
    std::uint32_t pause_bits_30;
    std::uint32_t state_34;
};

} // namespace bsp
