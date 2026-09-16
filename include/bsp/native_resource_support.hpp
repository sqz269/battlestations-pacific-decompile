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

// Borrow the application's actual publication cells through the shared raw
// manager drain. The D62B64 deletion binding must use this same context.
struct NativeResourceSupportRawContext {
    void* volatile& actual_manager_publication_01090aa0;
    NativeResourceSupportStorage* volatile& actual_published_0108fedc;
};

NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportRawContext&);

// Borrow one lifetime domain. Existing semantic callers retain their interface;
// render contexts can instead select the application's actual raw manager.
// This adapter owns no manager, publication, lock or native allocation.
class NativeResourceSupportLifetime final {
public:
    NativeResourceSupportLifetime(SingletonLifetimeDomain& lifetime) noexcept
        : semantic_(&lifetime) {}
    NativeResourceSupportLifetime(NativeResourceSupportRawContext& context) noexcept
        : raw_(&context) {}
    NativeResourceSupportStorage* get(NativeResourceSupportStorage* volatile&) const;
    // Compare borrowed identities, never current pointer values. A raw route
    // cannot match a semantic manager, even when both publications are null.
    bool borrows_same_domain(const SingletonLifetimeDomain& lifetime) const noexcept {
        return semantic_ == &lifetime;
    }
    bool borrows_same_domain(void* volatile& publication) const noexcept {
        return raw_ && &raw_->actual_manager_publication_01090aa0 == &publication;
    }
private:
    SingletonLifetimeDomain* semantic_{};
    NativeResourceSupportRawContext* raw_{};
};

NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportStorage* volatile&, const NativeResourceSupportLifetime&);

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
