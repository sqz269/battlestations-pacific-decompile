#include "bsp/native_resource_support.hpp"

#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource support reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSectionGuard final {
public:
    explicit CapturedSectionGuard(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSectionGuard() { release(); }
    CapturedSectionGuard(const CapturedSectionGuard&) = delete;
    CapturedSectionGuard& operator=(const CapturedSectionGuard&) = delete;
    void release() noexcept {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
            section_ = nullptr;
        }
    }
private:
    SystemSingletonCriticalSection* section_;
};
} // namespace

NativeResourceSupportStorage* construct_native_resource_support_00b61d50(
    void* actual_allocation) noexcept {
    // Default initialization of this trivial aggregate leaves its bytes intact.
    auto* owner = ::new (actual_allocation) NativeResourceSupportStorage;
    owner->native_vtable_00 = 0x00d62b64u;
    return owner;
}

NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportStorage* volatile& actual_published_0108fedc,
    SingletonLifetimeDomain& actual_lifetime) {
    if (auto* owner = actual_published_0108fedc) return owner;

    auto* const captured_section = actual_lifetime.get_manager_00415350()
        ->system_owner().section_10;
    CapturedSectionGuard guard(captured_section);
    if (!actual_published_0108fedc) {
        void* allocation = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 8, sizeof(NativeResourceSupportStorage)});
        NativeResourceSupportStorage* owner;
        try {
            owner = allocation
                ? construct_native_resource_support_00b61d50(allocation) : nullptr;
        } catch (...) {
            // Native state 1 frees the raw allocation, then state 0 unlocks.
            singleton_lifetime_free(allocation);
            throw;
        }
        // Native switches back to state 0 BEFORE publication/registration.
        // A throwing registration leaves the owner published and only unlocks.
        actual_published_0108fedc = owner;
        auto* const registration_manager = actual_lifetime.get_manager_00415350();
        registration_manager->register_object(actual_published_0108fedc);
    }
    guard.release(); // Native final global reload occurs AFTER LeaveCriticalSection.
    return actual_published_0108fedc;
}

NativeResourceSupportStorage* delete_native_resource_support_00b61d60(
    NativeResourceSupportStorage& owner, std::uint32_t flags,
    NativeResourceSupportStorage* volatile& actual_published_0108fedc) noexcept {
    auto* const original_address = &owner;
    actual_published_0108fedc = nullptr;
    owner.native_vtable_00 = 0x00ce3818u;
    if ((flags & 1u) != 0) {
        owner.~NativeResourceSupportStorage();
        singleton_lifetime_free(original_address);
    }
    return original_address;
}

} // namespace bsp
