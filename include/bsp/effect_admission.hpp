#pragma once

#include "bsp/camera_transform.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <array>
#include <cstdint>

namespace bsp {

// Borrow the actual template's native pointer-slot storage. The array owner and
// count references are live fields, not a copied list of mapped row objects.
// The captured array extent must remain allocated throughout the call, even if
// a virtual callback changes this owner/count or individual pointer slots.
struct EffectAdmissionTemplateView {
    void* const* const& rows_08;
    const std::uint32_t& count_0c;
};
struct EffectAdmissionRowView {
    const std::uint8_t& gated_10;
    const float& threshold_18;
    std::uint8_t& admitted_1c;
};
class EffectAdmissionDispatch {
public:
    virtual ~EffectAdmissionDispatch() = default;
    // Pure, nonthrowing projection of this exact owner. Borrow actual fields;
    // do not run native code, snapshot state, or substitute another row.
    virtual EffectAdmissionRowView project_row(void* actual_owner) noexcept = 0;
    // Dispatch this owner's CURRENT virtual+1C with the original input objects.
    // The low byte is significant; every nonzero byte means admission.
    virtual std::uint8_t virtual_1c(void* actual_owner,
        const std::array<float, 3>& point, CameraTransform& reference) = 0;
};

// Complete 0086A650 behavior: native ECX template; stack XYZ, reference;
// AL result, RET8. Upper EAX is stale and is not an additional return value.
// Reference must be the canonical transform borrowing the actual owner fields.
// Computes float32 deltas and one float32 squared-distance spill using native
// x87 order/environment; each threshold is squared without that float32 spill.
// A zero gate admits without writing +1C; other rows write normalized 0/1 to
// the CURRENT pointer in the captured slot after any virtual callback.
bool admit_point_effect_0086a650(EffectAdmissionTemplateView,
    const std::array<float, 3>& point, CameraTransform& reference,
    EffectAdmissionDispatch&);

// New C++ owner, NOT an executable native vtable or binary replacement.
// The integer preserves original-image identity only. section_04 points to the
// canonical projection of this owner's real Win32 lock and its actual +18 count.
// This exact owner pointer is published and registered with the lifetime domain.
struct EffectManager {
    std::uint32_t original_vtable_identity_00;
    SystemSingletonCriticalSection* section_04;
};

class EffectManagerLifetimeAccess {
public:
    virtual ~EffectManagerLifetimeAccess() = default;
    virtual SystemSingletonLifetimeOwner& manager_00415350() = 0;
    virtual void* allocate_00bf681b(const SingletonAllocationRequest&) = 0;
    virtual void free_00bf65ac(void*) noexcept = 0;
    virtual SystemSingletonCriticalSection* create_section_00bd1860() = 0;
    virtual void destroy_section_0041cc80(SystemSingletonCriticalSection*&) noexcept = 0;
    // Same pointer as publication, including null. The concrete implementation
    // invokes the canonical manager and never creates a second lifetime domain.
    virtual void register_00bd0c30(SystemSingletonLifetimeOwner&, EffectManager*) = 0;
};

// 00865FB0: ECX owner, RET. Constructor unwind clears the global unconditionally
// before restoring original-image base identity CE3818; it does not free +04.
void unwind_effect_manager_base_00865fb0(EffectManager&,
    EffectManager* volatile& global_00f87650) noexcept;
// 00866230: ECX raw owner, EAX same owner, RET. Only writes D0D3CC identity and
// +04 created lock. On a throwing lock creation, run the actual base unwind.
EffectManager& construct_effect_manager_00866230(EffectManager&,
    EffectManager* volatile& global_00f87650, EffectManagerLifetimeAccess&);
// 00866440: no inputs, EAX published owner, RET. Captures the lifetime manager's
// +10 lock; rechecks global under it; allocates 8 bytes; constructs, publishes,
// gets manager AGAIN, reloads global for registration, unlocks, reloads result.
EffectManager* effect_manager_singleton_00866440(
    EffectManager* volatile& global_00f87650, EffectManagerLifetimeAccess&);
// 008669D0: ECX owner; stack flags; EAX original pointer even after free; RET4.
// Does NOT unregister. Domain shutdown already pops the pointer before dispatch.
// Registering owners requires the application's actual shared lifetime dispatch
// to call this function for this owner; there is no default/no-op callback.
EffectManager* delete_effect_manager_008669d0(EffectManager*, std::uint32_t flags,
    EffectManager* volatile& global_00f87650, EffectManagerLifetimeAccess&) noexcept;

class ConcreteEffectManagerLifetimeAccess final : public EffectManagerLifetimeAccess {
public:
    explicit ConcreteEffectManagerLifetimeAccess(SingletonLifetimeDomain&) noexcept;
    SystemSingletonLifetimeOwner& manager_00415350() override;
    void* allocate_00bf681b(const SingletonAllocationRequest&) override;
    void free_00bf65ac(void*) noexcept override;
    SystemSingletonCriticalSection* create_section_00bd1860() override;
    void destroy_section_0041cc80(SystemSingletonCriticalSection*&) noexcept override;
    void register_00bd0c30(SystemSingletonLifetimeOwner&, EffectManager*) override;
private:
    SingletonLifetimeDomain& domain_;
};

} // namespace bsp
