#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Native allocation size is eight bytes. No observed operation interprets +04.
struct NativeResourceSupportStorage {
    std::uint32_t native_vtable_00;
    std::byte untouched_04[4];
};
static_assert(sizeof(NativeResourceSupportStorage) == 8);
static_assert(offsetof(NativeResourceSupportStorage, untouched_04) == 4);

// 00B61D50: ECX raw allocation; EAX same owner; RET. Writes only +00.
NativeResourceSupportStorage* construct_native_resource_support_00b61d50(
    void* actual_allocation) noexcept;

// 00B3E730: no native arguments; EAX published owner; RET. Both references must
// identify the application's actual 0108FEDC slot and shared 01090AA0 domain.
// Its destroy_registered callback must dispatch this owner's actual deleter.
NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportStorage* volatile& actual_published_0108fedc,
    SingletonLifetimeDomain& actual_lifetime);

// 00B61D60: ECX owner, stack flags, EAX original address, RET 4. Always clears
// the actual slot and writes the base profile; bit zero additionally frees.
// It does not remove an entry from the lifetime manager.
NativeResourceSupportStorage* delete_native_resource_support_00b61d60(
    NativeResourceSupportStorage& owner, std::uint32_t flags,
    NativeResourceSupportStorage* volatile& actual_published_0108fedc) noexcept;

} // namespace bsp
