#include "bsp/xlive_owner_lifetime.hpp"

#include <cstddef>
#include <exception>
#include <type_traits>

namespace bsp {

XLiveOwnerAllocation::XLiveOwnerAllocation(const XLiveManagerOwnerStorage& preimage,
    XLiveSystemPumpContext& context, XLiveSigninStorage& signin,
    XLiveManagerLifetimeAccess& lifetime, XLiveManagerOwnerHost& host)
    : storage_(preimage),
      owner_(new XLiveManagerOwner{context, signin, storage_, lifetime, host, this}) {
    static_assert(std::is_standard_layout_v<XLiveOwnerAllocation>);
    static_assert(offsetof(XLiveOwnerAllocation, storage_) == 0);
    static_assert(offsetof(XLiveManagerOwnerStorage, vtable_00) == 0);
}

XLiveOwnerAllocation::~XLiveOwnerAllocation() { delete owner_; }

void XLiveOwnerAllocation::free_owner_storage(XLiveManagerOwner* owner) noexcept {
    // Validate the projection/allocation relationship before touching the
    // allocation's ownership. Do not follow the CURRENT global publication:
    // native deleting wrappers free their captured incoming owner.
    if (!owner || !owner->allocation_identity ||
        &owner->allocation_identity->owner() != owner ||
        &owner->allocation_identity->storage() != &owner->storage)
        std::terminate();
    delete owner->allocation_identity;
}

void* delete_xlive_manager_base_00a3f670(XLiveOwnerAllocation& allocation,
    std::uint8_t flags) {
    void* const captured = allocation.identity();
    delete_xlive_manager_base_00a3f670(allocation.owner(), flags);
    return captured;
}

void* delete_xlive_manager_00a3fdc0(XLiveOwnerAllocation& allocation,
    std::uint8_t flags) {
    void* const captured = allocation.identity();
    delete_xlive_manager_00a3fdc0(allocation.owner(), flags);
    return captured;
}

} // namespace bsp
