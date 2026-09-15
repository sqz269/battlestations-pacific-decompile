#pragma once

#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {

// Borrowed native 74h game resource and 54h Points items. The resource's
// classified vector is +64h, name storage is item+8, Identifier index is +24h,
// and the vector of three-float points is +44h. No ownership is acquired.
// Original entries use ECX resource, stacked (name pointer, index), RET8.
// Names are counted, case-sensitive bytes. Duplicate matches are all visited;
// find returns the last one. Validation may return and mutate current storage.
std::uint8_t contains_native_game_group_00717f20(
    const void* resource, const NativeLegacySboStringStorage& name,
    std::uint32_t index, const SingletonLifetimeCallbacks& callbacks);
void* find_native_game_group_00718000(
    const void* resource, const NativeLegacySboStringStorage& name,
    std::uint32_t index, const SingletonLifetimeCallbacks& callbacks);
// Calls contains, then performs a fresh find scan with the same arguments.
void* lookup_native_game_group_00718870(
    const void* resource, const NativeLegacySboStringStorage& name,
    std::uint32_t index, const SingletonLifetimeCallbacks& callbacks);

// Original count: ECX item, EAX signed wrapped byte-distance / 12, RET.
std::int32_t count_native_named_group_points_004fba10(const void* item) noexcept;
// Original STL adapter: ECX item, stacked (destination, index), RET8/EAX dest.
// Preserve the ordered FLD32/FSTP32 pairs, including overlap and x87 effects.
// A returning invalid-parameter handler is followed by a fresh begin load.
void* copy_native_named_group_point_00484270(
    const void* item, void* destination, std::uint32_t index,
    const SingletonLifetimeCallbacks& callbacks);

} // namespace bsp
