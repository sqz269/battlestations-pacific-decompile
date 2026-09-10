#include "bsp/system_fog_slot.hpp"
#include "bsp/system_fog_owner.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Fog slot access requires the MSVC Win32 pointer ABI.
#endif

namespace bsp {
SystemFogSlotView::SystemFogSlotView(const SystemFogState* const& slot) noexcept
    : address_(&slot), stores_owner_(false) {}
SystemFogSlotView::SystemFogSlotView(SystemFogOwner* const& slot) noexcept
    : address_(&slot), stores_owner_(true) {}
const void* SystemFogSlotView::load_raw() const noexcept {
    const auto* slot = address_;
    const void* value;
    __asm {
        mov eax, slot
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
const SystemFogState* SystemFogSlotView::state_from_raw(const void* value) const noexcept {
    if (!value) return nullptr;
    return stores_owner_ ? &static_cast<const SystemFogOwner*>(value)->fields_08
                         : static_cast<const SystemFogState*>(value);
}
SystemFogOwner* SystemFogSlotView::owner_from_raw(const void* value) const noexcept {
    return stores_owner_ ? const_cast<SystemFogOwner*>(static_cast<const SystemFogOwner*>(value))
                         : system_fog_owner_from_state(static_cast<const SystemFogState*>(value));
}
const SystemFogState* SystemFogSlotView::state() const noexcept {
    return state_from_raw(load_raw());
}
SystemFogOwner* SystemFogSlotView::owner() const noexcept {
    return owner_from_raw(load_raw());
}
SystemFogSlotRef::SystemFogSlotRef(const SystemFogState*& slot) noexcept
    : SystemFogSlotView(slot), writable_address_(&slot) {}
SystemFogSlotRef::SystemFogSlotRef(SystemFogOwner*& slot) noexcept
    : SystemFogSlotView(slot), writable_address_(&slot) {}
void SystemFogSlotRef::store_owner(SystemFogOwner* value) const noexcept {
    if (stores_owner_)
        *static_cast<SystemFogOwner**>(writable_address_) = value;
    else
        *static_cast<const SystemFogState**>(writable_address_) = value ? &value->fields_08 : nullptr;
}
} // namespace bsp
