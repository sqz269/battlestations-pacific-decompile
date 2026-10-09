#include "bsp/native_squadron_launch_task_cleanup.hpp"

#include "bsp/native_observer_endpoint_live_count_cleanup.hpp"
#include "bsp/native_observer_member_cleanup.hpp"
#include "bsp/native_tick_subnode_base_cleanup.hpp"

namespace bsp {
namespace {

class LaunchTaskBaseCleanupGuard final {
public:
    LaunchTaskBaseCleanupGuard(void* actual_task,
        void* volatile& actual_flag_publication,
        void* volatile& actual_manager_publication) noexcept
        : task_(actual_task), flag_(actual_flag_publication),
          manager_(actual_manager_publication) {}

    LaunchTaskBaseCleanupGuard(const LaunchTaskBaseCleanupGuard&) = delete;
    LaunchTaskBaseCleanupGuard& operator=(const LaunchTaskBaseCleanupGuard&) = delete;

    ~LaunchTaskBaseCleanupGuard() noexcept {
        if (armed_) cleanup_native_tick_subnode_base_00875b30(
            task_, 0u, flag_, manager_);
    }

    void disarm() noexcept { armed_ = false; }

private:
    void* const task_;
    void* volatile& flag_;
    void* volatile& manager_;
    bool armed_{true};
};

class LaunchTaskMemberCleanupGuard final {
public:
    LaunchTaskMemberCleanupGuard(NativeObserverOwnerStorage& actual_owner,
        NativeObserverOwnerStorage* volatile& actual_endpoint_cell,
        NativeObserverLifetime& retained_lifetime) noexcept
        : owner_(actual_owner), endpoint_(actual_endpoint_cell),
          lifetime_(retained_lifetime) {}

    LaunchTaskMemberCleanupGuard(const LaunchTaskMemberCleanupGuard&) = delete;
    LaunchTaskMemberCleanupGuard& operator=(const LaunchTaskMemberCleanupGuard&) = delete;

    ~LaunchTaskMemberCleanupGuard() noexcept {
        if (armed_) cleanup_native_observer_member_00653390(
            owner_, endpoint_, lifetime_);
    }

    void disarm() noexcept { armed_ = false; }

private:
    NativeObserverOwnerStorage& owner_;
    NativeObserverOwnerStorage* volatile& endpoint_;
    NativeObserverLifetime& lifetime_;
    bool armed_{true};
};

} // namespace

void cleanup_native_squadron_launch_task_007f1e70(
    void* actual_task_base,
    volatile std::uint32_t& actual_task_profile_00,
    NativeObserverOwnerStorage& actual_member_owner_20,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_34,
    NativeObserverLifetime& retained_lifetime,
    NativePendingEntityOwners& actual_pending_owners,
    NativePendingEntityProducerAccess& actual_pending_access,
    void* volatile& pending_flag_publication,
    void* volatile& pending_manager_publication) {
    actual_task_profile_00 = 0x00d08ae4u;
    auto* const captured_endpoint = actual_endpoint_cell_34;
    // Reverse destruction order makes member cleanup precede base cleanup.
    LaunchTaskBaseCleanupGuard base_cleanup(actual_task_base,
        pending_flag_publication, pending_manager_publication);
    LaunchTaskMemberCleanupGuard member_cleanup(actual_member_owner_20,
        actual_endpoint_cell_34, retained_lifetime);
    if (captured_endpoint) {
        cleanup_native_observer_endpoint_live_count_007ee620(
            captured_endpoint, 0u, actual_pending_owners, actual_pending_access);
    }
    member_cleanup.disarm();
    cleanup_native_observer_member_00653390(
        actual_member_owner_20, actual_endpoint_cell_34, retained_lifetime);
    base_cleanup.disarm();
    cleanup_native_tick_subnode_base_00875b30(actual_task_base, 0u,
        pending_flag_publication, pending_manager_publication);
}

} // namespace bsp
