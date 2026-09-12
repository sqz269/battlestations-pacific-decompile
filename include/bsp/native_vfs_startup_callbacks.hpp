#pragma once

#include "bsp/native_vfs_mount_registration.hpp"

namespace bsp {

// Complete one-byte RET bodies. Neither consumes an argument nor supplies a
// semantic return value. These are confirmed no-op application callbacks.
void ignore_native_vfs_mount_failure_00530620() noexcept;
void ignore_native_vfs_callback_00735b30() noexcept;

// Only the 32-byte Init fragment 73D63C..73D65B, after manager construction:
// load current publication/store +90=530620, reload/store +8C=735B30.
// The enclosing first-time gate and allocation belong to the application.
// As in the native fragment, each publication read must yield a valid manager.
void install_native_vfs_startup_callbacks_0073d63c(
    void* volatile& actual_manager_publication_0109ceec) noexcept;

// Finite source binding for BE18B8's captured callback entry and publication.
// Numeric native code addresses are identities; only the verified startup
// mount callback is supported here. No default callback is substituted.
class NativeVfsStartupCallbacks final : public NativeVfsMountFailureDispatch {
public:
    void mount_failure_00be18b8(std::uintptr_t captured_target,
        void* captured_current_manager) override;
};

} // namespace bsp
