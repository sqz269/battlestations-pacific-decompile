#pragma once

#include "bsp/xlive_manager_owner.hpp"

namespace bsp {

// Source allocation boundary, not the native 3F0h manager layout. The first
// word is the SAME storage.vtable_00 written by the recovered constructors
// and destructors. The allocation owns that storage and its typed projection;
// all runtime, sign-in, lifetime and service objects remain borrowed.
//
// Supply defined scalar preimages. Construction copies only this storage and
// binds the projection: it does not publish/register, invoke native startup,
// initialize borrowed state, or create a private singleton domain.
class XLiveOwnerAllocation final {
public:
    XLiveOwnerAllocation(const XLiveManagerOwnerStorage& preimage,
        XLiveSystemPumpContext&, XLiveSigninStorage&, XLiveManagerLifetimeAccess&,
        XLiveManagerOwnerHost&);
    // Storage release only. Native teardown/unregistration must already have
    // completed (or failed-construction host recovery must have removed any
    // surviving registration/publication). No implicit native destructor.
    ~XLiveOwnerAllocation();
    XLiveOwnerAllocation(const XLiveOwnerAllocation&) = delete;
    XLiveOwnerAllocation& operator=(const XLiveOwnerAllocation&) = delete;

    void* identity() noexcept { return this; }
    const void* identity() const noexcept { return this; }
    bool owns_identity(const void* candidate) const noexcept { return candidate == this; }
    XLiveManagerOwner& owner() noexcept { return *owner_; }
    const XLiveManagerOwner& owner() const noexcept { return *owner_; }
    XLiveManagerOwnerStorage& storage() noexcept { return storage_; }
    const XLiveManagerOwnerStorage& storage() const noexcept { return storage_; }

    // Plug into RecoveredXLiveManagerOwnerServices::FreeOwner for this
    // allocation. Requires allocation by new XLiveOwnerAllocation. Frees only
    // the captured allocation and projection; borrowed runtime survives. The
    // caller must relinquish its owning pointer exactly once after flags1.
    static void free_owner_storage(XLiveManagerOwner*) noexcept;

private:
    XLiveManagerOwnerStorage storage_; // canonical vtable at allocation+0
    XLiveManagerOwner* owner_;
};

// A3F670/A3FDC0 allocation overloads. Native ECX owner, flags DWORD stack,
// RET4/EAX captured owner. New source ABI returns the allocation identity,
// including after its flag-bit0 free; legacy projection overloads remain.
// These invoke only the existing recovered scalar bodies. Their owner's
// required free service must release this allocation through the callback
// above, or an equivalent actual deleter; no second free is added here.
void* delete_xlive_manager_base_00a3f670(XLiveOwnerAllocation&, std::uint8_t flags);
void* delete_xlive_manager_00a3fdc0(XLiveOwnerAllocation&, std::uint8_t flags);

} // namespace bsp
