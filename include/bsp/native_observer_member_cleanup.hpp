#pragma once

namespace bsp {

struct NativeObserverOwnerStorage;
class NativeObserverLifetime;

// Qualified Source composition of 00653390..006533E7[88]. Borrow the actual
// 10h callback-owner prefix, its separate actual volatile endpoint-pointer
// cell, and the retained observer lifetime serving their existing domain.
// The endpoint cell is not part of NativeObserverOwnerStorage. Its identity,
// backing and association with this owner are caller conditions; this API
// derives no enclosing member/task address or invented 18h object layout.
//
// First stamp opaque profile DWORD 00CF6494, then capture the endpoint cell
// once. A nonnull captured endpoint is unregistered against this actual owner.
// The private armed guard destroys this owner if unregister throws. Disarm
// before normal owner destruction, so a failure in that call is never retried.
// No endpoint/array clearing or owner allocation/deletion is added here.
//
// Keep the actual owner, selected endpoint, cell and retained lifetime/domain
// usable through reached child calls and failure cleanup. Reuse the existing
// observer storage/child contracts, including live dispatch and valid arrays;
// copies, proxy identities and a temporary replacement lifetime are not bindings.
//
// This public function may throw. The guard destructor is explicitly noexcept:
// if destruction throws during unregister unwinding, current C++ termination
// policy applies. Native handler bytes motivate the cleanup action, but their
// framework's selection, EBP basis and double-exception policy are unproved.
// No Native FS/frame/register ABI, source-size, callable-profile, raw-task
// production binding or game-equivalence claim accompanies this C++ interface.
void cleanup_native_observer_member_00653390(
    NativeObserverOwnerStorage& actual_member_owner,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell,
    NativeObserverLifetime& retained_lifetime);

} // namespace bsp
