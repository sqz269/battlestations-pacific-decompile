#include "bsp/native_vfs_derived_manager.hpp"

#include "bsp/native_physical_factory.hpp"
#include "bsp/native_vfs_factory_registration.hpp"
#include "bsp/native_vfs_manager_lifetime.hpp"

namespace bsp {
namespace {
struct DerivedManagerUnwind {
    void* owner;
    NativeVfsManagerLifetimeContext& manager;
    bool armed = false;
    ~DerivedManagerUnwind() noexcept(false) {
        if (armed) destroy_native_vfs_manager_00be1f60(owner, manager);
    }
};
} // namespace

void* construct_native_vfs_derived_manager_00beda60(void* owner,
    NativeVfsManagerLifetimeContext& manager, NativePhysicalFactoryContext& factory) {
    DerivedManagerUnwind unwind{owner, manager};
    construct_native_vfs_manager_00be1dc0(owner, manager);
    // E0203C state0 -> -1, CC74E0 -> BE1F60 with captured owner EBP-10h.
    // No owner free, publication rollback or factory-registration cleanup.
    unwind.armed = true;
    *static_cast<volatile std::uint32_t*>(owner) = 0x00d68d04u;
    void* const current_factory = get_native_physical_factory_00bed990(factory);
    register_native_vfs_provider_factory_00be0660(owner, nullptr, current_factory);
    unwind.armed = false;
    return owner;
}

void* delete_native_vfs_derived_manager_00bedac0(void* owner, std::uint32_t flags,
    NativeVfsManagerLifetimeContext& manager) {
    destroy_native_vfs_manager_00be1f60(owner, manager);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

} // namespace bsp
