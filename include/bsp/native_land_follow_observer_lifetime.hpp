#pragma once

#include "bsp/native_land_state_entries.hpp"

namespace bsp {

// Borrow the SAME actual component: callback prefix at component+0 and its
// first-endpoint identity cell at +14h. In Follow these are state+18h/+2Ch.
// No translated endpoint, separate pointer cache or default storage. Require
// live coherent >=18h component storage; neither field is owned by this view.
struct NativeLandFollowObserverCleanupView {
    NativeObserverOwnerStorage& callback_00;
    const void* const volatile& first_endpoint_14;
};

// Pure reference projection of the completed entry's actual member fields.
// The entry view must satisfy the actual same-component offset contract above.
NativeLandFollowObserverCleanupView native_land_follow_observer_cleanup_view(
    const NativeLandFollowEntryView&) noexcept;

// Complete ordinary006CDD70..006CDDC8: original ECX=component, RET. Raw CF8900
// stamp -> one fresh +14 capture -> optional real6952A0 -> complete695870.
// Nonnull +14 MUST already be the actual6952A0 FIRST-endpoint prefix identity
// at the SAME address, not a semantic unit or an offset-translated pointer.
// Borrow the mandatory existing lifetime with actual manager/publications/
// recursive locks/dispatch/current allocation and deletion-provider context.
// Its resulting base profile and fields are retained; no +14 clear, receiver
// free, header reset, callback/profile substitute or default observer world.
// The raw numeric CF8900 table remains UNCALLABLE in this new SOURCE API.
void destroy_native_land_follow_observer_006cdd70(
    const NativeLandFollowObserverCleanupView&, NativeObserverLifetime&);

// Complete scalar006CDDF0..006CDE0E: original stack DWORD flags, low BYTE bit0,
// EAX=original component identity, RET4. Test AFTER ordinary destruction.
// Also the complete normal SOURCE provider for active CF89B4 scalar006CEEB0
// ..006CEECE: its 30 bytes match after ONLY the two rel32 CALL operands are
// normalized, and both actual targets remain 006CDD70/BF65AC. No new wrapper
// or ABI bridge: incoming COMPONENT identity is preserved, with no root+18
// adjustment or routing to the distinct whole-root scalar009C2A60.
// Flag1 requires a separate actual CRT allocation of the component; NEVER
// self-free the interior callback+18 in an allocated/embedded Follow state.
// Return after flag1 is dangling; do not dereference or destroy again.
NativeObserverOwnerStorage* scalar_delete_native_land_follow_observer_006cddf0(
    const NativeLandFollowObserverCleanupView&, std::uint32_t flags,
    NativeObserverLifetime&);

// Both are new C++ interfaces, not original ABI bridges. Stable live actual
// fields/endpoints and ordinary successful returning providers are required.
// Structural reentry, concurrency, faults/private EH, full Follow constructor,
// image class/profile/arena/observer lifetime binding, whole approach/task
// teardown and gameplay are excluded; borrowed pointee lifetime is not proved.

} // namespace bsp
