#include "bsp/native_squadron_launch_task_scalar_deletion.hpp"

#include "bsp/native_squadron_launch_task_cleanup.hpp"

#include <cstdlib>

namespace bsp {

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
    volatile std::uint8_t& actual_deletion_flags_low_byte) {
    void* const captured_task = actual_task_base;
    cleanup_native_squadron_launch_task_007f1e70(captured_task,
        actual_task_profile_00, actual_member_owner_20, actual_endpoint_cell_34,
        retained_lifetime, actual_pending_owners, actual_pending_access,
        pending_flag_publication, pending_manager_publication);
    const std::uint8_t late_flags = actual_deletion_flags_low_byte;
    if ((late_flags & 0x01u) != 0u) std::free(captured_task);
    return captured_task;
}

} // namespace bsp
