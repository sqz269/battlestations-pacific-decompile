#include "bsp/native_render_preparation_job_owner.hpp"
#include <cstddef>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t construction_secondary_table = 0x00d5e154u;
constexpr std::uint32_t final_secondary_table = 0x00d5e15cu;
constexpr std::uint32_t primary_table = 0x00d5e160u;
constexpr std::uint32_t base_table = 0x00ce3818u;
constexpr std::uint32_t secondary_deleting_thunk = 0x00b0f1c0u;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderPreparationJobSecondary) == 4);
static_assert(sizeof(NativeRenderPreparationJobStorage) == 8);
static_assert(offsetof(NativeRenderPreparationJobStorage, secondary_04) == 4);

class CapturedManagerGuard final {
public:
    explicit CapturedManagerGuard(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
        // Completion arms cleanup only after native Enter and the increment.
    }
    ~CapturedManagerGuard() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
private:
    SystemSingletonCriticalSection* section_;
};
} // namespace

NativeRenderPreparationJobLifetime::NativeRenderPreparationJobLifetime(
    SingletonLifetimeDomain& domain,
    NativeRenderPreparationJobStorage* volatile& published,
    const volatile std::uint32_t* secondary_profile)
    : domain_(domain), global_00f8d444_(published),
      final_secondary_table_00d5e15c_(secondary_profile) {
    if (!secondary_profile)
        throw std::invalid_argument("Native preparation job requires the actual D5E15C table");
}

NativeRenderPreparationJobStorage*
NativeRenderPreparationJobLifetime::get_singleton_00b0ffb0() {
    if (auto* current = global_00f8d444_) return current;
    {
        CapturedManagerGuard guard(domain_.get_manager_00415350()->system_owner().section_10);
        if (!global_00f8d444_) {
            void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
                8, sizeof(NativeRenderPreparationJobStorage)});
            NativeRenderPreparationJobStorage* owner = nullptr;
            if (storage) {
                owner = ::new (storage) NativeRenderPreparationJobStorage;
                owner->secondary_04.native_vtable_00 = construction_secondary_table;
                owner->native_vtable_00 = primary_table;
                owner->secondary_04.native_vtable_00 = final_secondary_table;
            }
            global_00f8d444_ = owner;
            // B1003D..B1004D capture the secondary argument BEFORE B1004E's
            // second getter. Registration must not reload publication after it.
            auto* current = global_00f8d444_;
            auto* secondary = current ? &current->secondary_04 : nullptr;
            auto* manager = domain_.get_manager_00415350();
            manager->register_object(secondary);
        }
    }
    // Native final publication load is after leaving the captured section.
    return global_00f8d444_;
}

void NativeRenderPreparationJobLifetime::destroy_body_00b0f1d0(
    NativeRenderPreparationJobStorage& owner) noexcept {
    global_00f8d444_ = nullptr;
    owner.secondary_04.native_vtable_00 = base_table;
}

NativeRenderPreparationJobStorage*
NativeRenderPreparationJobLifetime::delete_primary_00b0f210(
    NativeRenderPreparationJobStorage* owner, std::uint32_t flags) noexcept {
    destroy_body_00b0f1d0(*owner);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

NativeRenderPreparationJobStorage*
NativeRenderPreparationJobLifetime::delete_secondary_00b0f1c0(
    NativeRenderPreparationJobSecondary* secondary, std::uint32_t flags) noexcept {
    auto* primary = reinterpret_cast<NativeRenderPreparationJobStorage*>(
        reinterpret_cast<std::uintptr_t>(secondary) - 4u);
    return delete_primary_00b0f210(primary, flags);
}

NativeRenderPreparationJobSecondary*
NativeRenderPreparationJobLifetime::delete_construction_base_00b0d930(
    NativeRenderPreparationJobSecondary* base, std::uint32_t flags) noexcept {
    global_00f8d444_ = nullptr;
    base->native_vtable_00 = base_table;
    if (flags & 1u) singleton_lifetime_free(base);
    return base;
}

bool NativeRenderPreparationJobLifetime::try_delete_registered(
    void* registered, std::uint32_t flags) noexcept {
    if (!registered) return false;
    auto* secondary = static_cast<NativeRenderPreparationJobSecondary*>(registered);
    if (secondary->native_vtable_00 != final_secondary_table) return false;
    if (final_secondary_table_00d5e15c_[0] != secondary_deleting_thunk) return false;
    delete_secondary_00b0f1c0(secondary, flags);
    return true;
}
} // namespace bsp
