#include "bsp/native_event_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <Windows.h>
#include <cstddef>

namespace bsp {
static_assert(sizeof(void*) == 4, "Native event owner storage requires Win32.");
static_assert(sizeof(NativeEventOwnerStorage) == 8);
static_assert(offsetof(NativeEventOwnerStorage, handle_04) == 4);

NativeEventOwnerStorage* create_native_event_owner_00bd1970(std::uint8_t manual_reset) {
    auto* owner = static_cast<NativeEventOwnerStorage*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, 8, sizeof(NativeEventOwnerStorage)}));
    if (owner == nullptr) return nullptr; // Native returning-null allocation branch.
    owner->table_00 = native_event_concrete_table_00d6821c;
    owner->handle_04 = CreateEventA(nullptr, static_cast<BOOL>(manual_reset), FALSE, nullptr);
    return owner;
}

NativeEventOwnerStorage* delete_native_event_owner_00bd19b0(
    NativeEventOwnerStorage* owner, std::uint32_t flags) noexcept {
    const auto handle = owner->handle_04;
    owner->table_00 = native_event_concrete_table_00d6821c;
    CloseHandle(handle);
    const bool free_owner = (flags & 1u) != 0;
    owner->table_00 = native_event_base_table_00d68208;
    if (free_owner) singleton_lifetime_free(owner);
    return owner;
}

std::int32_t signal_native_event_owner_00bd1910(const NativeEventOwnerStorage* owner) noexcept {
    return SetEvent(owner->handle_04);
}

std::uint32_t wait_native_event_owner_00bd17c0(const NativeEventOwnerStorage* owner) noexcept {
    return WaitForSingleObject(owner->handle_04, INFINITE);
}

std::int32_t reset_native_event_owner_00bd1960(const NativeEventOwnerStorage* owner) noexcept {
    return ResetEvent(owner->handle_04);
}

} // namespace bsp
