#pragma once

namespace bsp {
struct SystemFogState;
struct SystemFogOwner;

// Borrow the actual pointer word, not a snapshot of its value. Native camera
// +184 stores the D63180 owner; legacy diagnostics store its +08 field view.
// Both forms remain live and all captured values are converted only AFTER the
// caller's required load/store sequence. The pointed-to owners remain borrowed.
class SystemFogSlotView {
public:
    explicit SystemFogSlotView(const SystemFogState* const&) noexcept;
    explicit SystemFogSlotView(SystemFogOwner* const&) noexcept;
    const void* address() const noexcept { return address_; }
    bool stores_owner() const noexcept { return stores_owner_; }
    const void* load_raw() const noexcept;
    const SystemFogState* state_from_raw(const void*) const noexcept;
    SystemFogOwner* owner_from_raw(const void*) const noexcept;
    const SystemFogState* state() const noexcept;
    SystemFogOwner* owner() const noexcept;
    operator const SystemFogState*() const noexcept { return state(); }
    const SystemFogState* operator->() const noexcept { return state(); }
protected:
    const void* const address_;
    const bool stores_owner_;
};

// Mutable companion for one live slot. Copying this descriptor keeps the same
// slot; it does not retain its owner. Native form accepts only the proven owner
// profile. Legacy nonnull state views passed to ownership APIs must likewise
// originate from SystemFogOwner::fields_08, never standalone diagnostic fields.
class SystemFogSlotRef final : public SystemFogSlotView {
public:
    explicit SystemFogSlotRef(const SystemFogState*&) noexcept;
    explicit SystemFogSlotRef(SystemFogOwner*&) noexcept;
    void store_owner(SystemFogOwner*) const noexcept;
    void clear() const noexcept { store_owner(nullptr); }
private:
    void* const writable_address_;
};
} // namespace bsp
