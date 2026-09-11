#include "bsp/live_effect_manager_lifetime.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Live-effect manager lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
class MemberUnwind final {
public:
    MemberUnwind(NativeLiveEffectManagerStorage& owner,
        NativeLiveEffectManagerStorage* volatile& global,
        NativeRenderActualOwners& references) noexcept
        : owner_(owner), global_(global), references_(references) {}
    ~MemberUnwind() noexcept {
        if (state >= 3) destroy_live_effect_auxiliary_array_004ba0f0(owner_.references_1c);
        if (state >= 2) destroy_live_effect_reference_array_004cdea0(owner_.effects_10, references_);
        if (state >= 1) destroy_live_effect_pending_list_004c8c20(owner_.pending_04);
        if (state >= 0) unwind_native_live_effect_manager_base_004b7f50(owner_, global_);
    }
    int state{3};
private:
    NativeLiveEffectManagerStorage& owner_;
    NativeLiveEffectManagerStorage* volatile& global_;
    NativeRenderActualOwners& references_;
};
}

void destroy_live_effect_auxiliary_array_004ba0f0(NativeRenderPointerArrayStorage& array) noexcept {
    volatile auto& actual = array;
    if (void* const captured = actual.data_00) {
        singleton_lifetime_free(captured);
        actual.data_00 = nullptr;
    }
}

void destroy_live_effect_reference_array_004cdea0(NativeRenderPointerArrayStorage& array,
    NativeRenderActualOwners& owners) {
    resize_live_effect_references_004c9550(array, 0, owners);
    volatile auto& actual = array;
    singleton_lifetime_free(actual.data_00);
}

void destroy_live_effect_pending_list_004c8c20(NativeEffectDeletionListStorage& list) noexcept {
    destroy_native_effect_deletion_list_004c5940(list);
}

void destroy_live_effect_manager_00867c00(NativeLiveEffectManagerStorage& owner,
    NativeLiveEffectManagerStorage* volatile& global,
    NativeRenderActualOwners& references, EffectDeletionDispatch& pending) {
    owner.native_vtable_00 = 0x00ce789cu;
    MemberUnwind unwind(owner, global, references);
    resize_live_effect_references_004c9550(owner.effects_10, 0, references);
    flush_effect_deletions_008671a0(owner, pending);
    destroy_live_effect_auxiliary_array_004ba0f0(owner.references_1c);
    unwind.state = 1;
    destroy_live_effect_reference_array_004cdea0(owner.effects_10, references);
    destroy_live_effect_pending_list_004c8c20(owner.pending_04);
    unwind.state = -1;
    unwind_native_live_effect_manager_base_004b7f50(owner, global);
}

NativeLiveEffectManagerStorage* delete_live_effect_manager_004cf770(
    NativeLiveEffectManagerStorage* owner, std::uint32_t flags,
    NativeLiveEffectManagerStorage* volatile& global,
    NativeRenderActualOwners& references, EffectDeletionDispatch& pending) {
    auto* const original = owner;
    destroy_live_effect_manager_00867c00(*owner, global, references, pending);
    if (flags & 1u) {
        owner->~NativeLiveEffectManagerStorage();
        singleton_lifetime_free(owner);
    }
    return original;
}

NativeLiveEffectManagerStorage* live_effect_manager_singleton_004d1100(
    NativeLiveEffectManagerStorage* volatile& global, SingletonLifetimeDomain& domain) {
    if (auto* existing = global) return existing;
    {
        CapturedSection section(domain.get_manager_00415350()->system_owner().section_10);
        if (!global) {
            void* const raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x28, sizeof(NativeLiveEffectManagerStorage)});
            NativeLiveEffectManagerStorage* created = nullptr;
            if (raw) {
                created = ::new (raw) NativeLiveEffectManagerStorage;
                try {
                    construct_native_live_effect_manager_004cf700(*created, global);
                } catch (...) {
                    created->~NativeLiveEffectManagerStorage();
                    singleton_lifetime_free(raw);
                    throw;
                }
            }
            global = created;
            auto* const manager = domain.get_manager_00415350();
            manager->register_object(global);
        }
    }
    return global;
}

} // namespace bsp
