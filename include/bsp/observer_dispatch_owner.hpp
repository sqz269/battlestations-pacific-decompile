#pragma once

#include "bsp/observer_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"

namespace bsp {

// Actual owning 14h allocation published at 00E198DC. The separate 00E198E4
// cell aliases dispatch_04; it does not own another allocation. Construction
// leaves dispatch_04.unconsumed_00 (owner+4) untouched.
struct NativeObserverDispatchOwner {
    volatile std::uint32_t native_vtable_00;
    NativeObserverDispatchStorage dispatch_04;
};
static_assert(sizeof(NativeObserverDispatchOwner) == 0x14);
static_assert(offsetof(NativeObserverDispatchOwner, dispatch_04) == 4);
inline constexpr std::uint32_t kObserverDispatchOwnerVtable = 0x00cf7e74u;

// Complete 00695ED0..00695F07 normal stores. Native ECX=raw14h, EAX=owner,
// RET. No allocation or vector reserve. Preserves owner+4, sets profile and
// zeros only owner+8/+C/+10. Original FH3/hardware-fault cleanup is external.
NativeObserverDispatchOwner* construct_observer_dispatch_owner_00695ed0(
    void* storage) noexcept;

// Complete 00696360..0069641C normal getter. Native no inputs, EAX=owner,
// RET. Borrow the same existing manager access and actual owning publication.
// Fast return is captured; slow path captures the first manager section,
// rechecks, allocates14h, constructs, publishes, then resolves the manager
// again before reading/registering the current owner. Release precedes the
// final publication reload. Registration failure retains the published owner.
NativeObserverDispatchOwner* get_observer_dispatch_owner_00696360(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc);

// Complete 00696420..00696424: native tail JMP to the same getter.
NativeObserverDispatchOwner* observer_dispatch_owner_thunk_00696420(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc);

// Complete 00CCD6A0..00CCD6AD. CRT initializer table slot00CE2BAC calls this
// once during process initialization: getter, ADD EAX,4, publish00E198E4.
// Preserve native32 addition even for a null getter result (alias becomes4).
// Getter calls alone never republish the alias. Caller owns startup ordering;
// this source function does not install a second CRT/atexit registration.
void publish_observer_dispatch_storage_00ccd6a0(
    SoundLifetimeAccess lifetime,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc,
    NativeObserverDispatchStorage* volatile& actual_dispatch_00e198e4);

// Complete 00695F10..00695F3D nondeleting destructor. Native ECX=owner, RET.
// Free captured begin if nonnull, zero slots, unconditionally clear owning
// publication, then write base profileCE3818. No edge deletion/unregister.
void destroy_observer_dispatch_owner_00695f10(
    NativeObserverDispatchOwner&,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept;

// Complete existing CG_scalar_deleting_dtor_00695f40 body..00695F81.
// Native ECX=owner, flags on stack, EAX=original address, RET4. Inline the
// same destruction then free owner iff flags&1. Bind produced profileCF7E74
// to this entry in the actual manager's registered-delete dispatch.
NativeObserverDispatchOwner* delete_observer_dispatch_owner_00695f40(
    NativeObserverDispatchOwner*, std::uint32_t flags,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept;

// Complete base cleanup00693C70..00693C80, also the constructor unwind target
// via00C7EAF0. Native ECX=owner, RET. Only clear owning publication/base profile.
void clear_observer_dispatch_owner_base_00693c70(
    NativeObserverDispatchOwner&,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept;
// Complete base scalar deletion00693E10..00693E38; tableCF7E6C, ECX=owner,
// flags stack, EAX=original, RET4. It does not destroy the derived vector.
NativeObserverDispatchOwner* delete_observer_dispatch_owner_base_00693e10(
    NativeObserverDispatchOwner*, std::uint32_t flags,
    NativeObserverDispatchOwner* volatile& actual_owner_00e198dc) noexcept;

// None of these destructors clears00E198E4. After manager destruction the
// alias remains dangling; callers must stop observer dispatch/cleanup first.
// Recreating only the getter leaves that alias unchanged. Native identity
// words are not callable source vtables. Source interfaces are not original
// ABI/FH3/SEH replacements; storage must remain valid through each operation.
} // namespace bsp
