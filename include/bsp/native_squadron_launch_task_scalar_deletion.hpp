#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native squadron launch task scalar deletion requires MSVC Win32.
#endif

namespace bsp {

struct NativeObserverOwnerStorage;
class NativeObserverLifetime;
struct NativePendingEntityOwners;
class NativePendingEntityProducerAccess;

// Qualified cdecl Source composition of 007F1EE0..007F1EFD[30]. Capture the
// actual Task pointer and invoke the admitted nine-input task cleanup with
// the same actual profile/member/endpoint/service/publication identities.
// Only after normal cleanup return, read the caller's actual volatile flags
// low-byte cell once. Mask 01h selects current Source std::free on that Task.
// Return the retained opaque pointer value without dereferencing it; a freed
// result is not a live object. Cleanup failure performs no flags read or free.
//
// Preserve the task cleanup's actual storage and lifetime conditions. The
// flags cell is real caller-supplied backing, kept live through its late read;
// it may change during cleanup. It is not a copied flag, fabricated receiver,
// new heap/domain or recovered Original stack word. If deletion is selected,
// the captured Task must still be eligible for the current Source CRT free.
// No new allocation, flags write, owner copy, validation, callback or production
// binding is supplied. The public function may throw; it is not noexcept.
//
// Native raw30 has 11 operations; its saved listing has 10 starts and omits
// ADD ESP,4 at 007F1EF5. This interface neither repairs that listing nor proves
// a NoReturn/flow property. Native ECX/flags-word/RET4/register/frame aliases,
// original allocator/free policy and ABI/gameplay equivalence remain separate.
void* __cdecl scalar_delete_native_squadron_launch_task_007f1ee0(
    void* actual_task_base,
    volatile std::uint32_t& actual_task_profile_00,
    NativeObserverOwnerStorage& actual_member_owner_20,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_34,
    NativeObserverLifetime& retained_lifetime,
    NativePendingEntityOwners& actual_pending_owners,
    NativePendingEntityProducerAccess& actual_pending_access,
    void* volatile& pending_flag_publication,
    void* volatile& pending_manager_publication,
    volatile std::uint8_t& actual_deletion_flags_low_byte);

} // namespace bsp
