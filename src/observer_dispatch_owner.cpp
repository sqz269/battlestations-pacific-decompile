#include "bsp/observer_dispatch_owner.hpp"

#include <cstdint>
#include <new>

namespace bsp {
namespace {
constexpr std::uint32_t base_profile = 0x00ce3818u;

void free_dispatch_buffer(NativeObserverDispatchOwner& owner) noexcept {
    void** const begin = owner.dispatch_04.slots_04.begin;
    if (begin) singleton_lifetime_free(begin); // actual CRT free boundary
}
void clear_dispatch_slots(NativeObserverDispatchOwner& owner) noexcept {
    owner.dispatch_04.slots_04.begin = nullptr;
    owner.dispatch_04.slots_04.end = nullptr;
    owner.dispatch_04.slots_04.capacity_end = nullptr;
}
} // namespace

NativeObserverDispatchOwner* construct_observer_dispatch_owner_00695ed0(
    void* storage) noexcept {
    auto* const owner = ::new (storage) NativeObserverDispatchOwner;
    owner->native_vtable_00 = kObserverDispatchOwnerVtable;
    owner->dispatch_04.slots_04.begin = nullptr;
    owner->dispatch_04.slots_04.end = nullptr;
    owner->dispatch_04.slots_04.capacity_end = nullptr;
    return owner;
}

NativeObserverDispatchOwner* get_observer_dispatch_owner_00696360(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) {
    if (auto* const initial = actual_owner_00e198dc) return initial;
    {
        CapturedSoundLifetimeSection guard(lifetime);
        if (!actual_owner_00e198dc) {
            void* const storage = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x14, sizeof(NativeObserverDispatchOwner)});
            // The native state1 cleanup frees this allocation if construction
            // unwinds. Its constructor has only field stores: no source throw
            // site; native hardware exceptions/FH3 are not emulated here.
            actual_owner_00e198dc = storage
                ? construct_observer_dispatch_owner_00695ed0(storage) : nullptr;
            auto manager = lifetime.get_manager_00415350();
            manager->register_object(actual_owner_00e198dc);
        }
    }
    return actual_owner_00e198dc;
}

NativeObserverDispatchOwner* observer_dispatch_owner_thunk_00696420(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) {
    return get_observer_dispatch_owner_00696360(lifetime, actual_owner_00e198dc);
}

void publish_observer_dispatch_storage_00ccd6a0(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc,
    NativeObserverDispatchStorage* volatile& actual_dispatch_00e198e4) {
    auto* const owner = get_observer_dispatch_owner_00696360(lifetime, actual_owner_00e198dc);
    const auto address = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(owner));
    actual_dispatch_00e198e4 = reinterpret_cast<NativeObserverDispatchStorage*>(address + 4u);
}

void clear_observer_dispatch_owner_base_00693c70(
    NativeObserverDispatchOwner& owner,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept {
    actual_owner_00e198dc = nullptr;
    owner.native_vtable_00 = base_profile;
}

void destroy_observer_dispatch_owner_00695f10(
    NativeObserverDispatchOwner& owner,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept {
    free_dispatch_buffer(owner);
    clear_dispatch_slots(owner);
    clear_observer_dispatch_owner_base_00693c70(owner, actual_owner_00e198dc);
}

NativeObserverDispatchOwner* delete_observer_dispatch_owner_00695f40(
    NativeObserverDispatchOwner* owner, std::uint32_t flags,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept {
    free_dispatch_buffer(*owner);
    const bool release_owner = (flags & 1u) != 0;
    clear_dispatch_slots(*owner);
    actual_owner_00e198dc = nullptr;
    owner->native_vtable_00 = base_profile;
    if (release_owner) singleton_lifetime_free(owner);
    return owner;
}

NativeObserverDispatchOwner* delete_observer_dispatch_owner_base_00693e10(
    NativeObserverDispatchOwner* owner, std::uint32_t flags,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept {
    const bool release_owner = (flags & 1u) != 0;
    clear_observer_dispatch_owner_base_00693c70(*owner, actual_owner_00e198dc);
    if (release_owner) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
