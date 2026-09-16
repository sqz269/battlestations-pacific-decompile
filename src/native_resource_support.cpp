#include "bsp/native_resource_support.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/sound_lifetime_access.hpp"

#include <new>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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

NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportRawContext& context) {
    if (auto* captured = context.actual_published_0108fedc) return captured;
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    const auto field = [](void* object, std::size_t offset) -> volatile std::uint32_t& {
        return *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::byte*>(object) + offset);
    };
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(field(first_manager, 0x10));
    alignas(4) std::uint32_t guard[2]{0x00ce37fcu, reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        field(section, 0x18) = field(section, 0x18) + 1u;
    }
    try {
        if (!context.actual_published_0108fedc) {
            void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 8, 8});
            // B61D50 is a noexcept single profile store; its +04 bytes survive.
            // Native state1 surrounds this constructor, then disarms before
            // publication. Hardware-fault/FH3 cleanup is outside this source ABI.
            context.actual_published_0108fedc = allocation
                ? construct_native_resource_support_00b61d50(allocation) : nullptr;
            void* const registration_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            // Reload the publication AFTER the second manager getter.
            auto* const current = context.actual_published_0108fedc;
            register_native_singleton_object_00bd0c30(registration_manager, nullptr, current);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    if (section) {
        field(section, 0x18) = field(section, 0x18) - 1u;
        LeaveCriticalSection(section);
    }
    return context.actual_published_0108fedc;
}

NativeResourceSupportStorage* NativeResourceSupportLifetime::get(
    NativeResourceSupportStorage* volatile& publication) const {
    if (raw_) {
        if (&publication != &raw_->actual_published_0108fedc)
            throw std::invalid_argument("resource support requires the same actual publication cell");
        return resource_support_singleton_00b3e730(*raw_);
    }
    return resource_support_singleton_00b3e730(publication, *semantic_);
}

bool NativeResourceSupportLifetime::borrows_same_domain(
    const SoundLifetimeAccess& lifetime) const noexcept {
    if (semantic_) return lifetime.borrows_same_domain(*semantic_);
    return raw_ && lifetime.borrows_same_domain(
        raw_->actual_manager_publication_01090aa0);
}

NativeResourceSupportStorage* resource_support_singleton_00b3e730(
    NativeResourceSupportStorage* volatile& publication, const NativeResourceSupportLifetime& lifetime) {
    return lifetime.get(publication);
}

} // namespace bsp
