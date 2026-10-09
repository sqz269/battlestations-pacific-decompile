#pragma once

#include <cstdint>

namespace bsp {

struct NativeObserverOwnerStorage;
class NativeObserverLifetime;

// Qualified Source composition of 007F0F80..007F0FDB[92]. The caller supplies
// one actual member M: its existing 10h owner prefix, its actual BYTE M+10h,
// and its actual DWORD endpoint cell M+14h. The separate references express
// binding obligations; this interface supplies no new 18h member type/backing.
// The incoming reference identifies the actual pointer cell, not an eagerly
// copied argument. It is read once AFTER ordered DWORD zeros at M+4, M+8, M+Ch.
// Valid C++ object lifetimes/types and these actual identities are required.
//
// Arm private owner cleanup after that read; publish opaque profile 00CF6494,
// captured endpoint at M+14h, then BYTE 1 at M+10h. M+11h..13h are untouched.
// If the captured endpoint is nonnull, pass it and actual M to the existing
// register_observer_pair_00694a60 free function with the retained lifetime.
// Do not reload M+14h for this call. Successful return is the actual M reference.
//
// The public function may throw. An armed private destructor calls only
// retained_lifetime.destroy_callback_owner_00695870(actual_owner); it is
// explicitly noexcept, so cleanup failure during C++ unwind terminates.
// Preserve prior stores and partial registration effects; no unregister,
// rollback, allocation repair or retry is performed by this composition.
// Retain the actual owner/endpoint backing, observer domain, publication cells,
// services and callback targets throughout registration and failure cleanup.
//
// Native ECX=M, one incoming stack DWORD, EAX=M, RET 4 under compatible callees.
// This C++ reference interface/guard does not reproduce that ABI or Native
// FS/FH3/frame/current-stack-word aliasing, registers, flags or CRT exceptions.
// Selected production binding, Original ABI, startup and gameplay are unproved.
NativeObserverOwnerStorage& initialize_native_observer_member_007f0f80(
    NativeObserverOwnerStorage& actual_owner,
    volatile std::uint8_t& actual_flag_10,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_14,
    NativeObserverOwnerStorage* const volatile& actual_incoming_endpoint_cell,
    NativeObserverLifetime& retained_lifetime);

} // namespace bsp
