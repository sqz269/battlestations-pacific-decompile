#pragma once

#include "bsp/native_vfs_mount_registration.hpp"

namespace bsp {

// Complete one-byte RET bodies. Neither consumes an argument nor supplies a
// semantic return value. These are confirmed no-op application callbacks.
void ignore_native_vfs_mount_failure_00530620() noexcept;
void ignore_native_vfs_callback_00735b30() noexcept;

// Address of the existing raw C3 leaf in this MSVC Win32 code image. Read and
// qualify that declared symbol's single byte; no callback is invoked here.
// Keep its readable executable image alive through every published use/drain.
// Throws if that trusted entry no longer has the qualified C3 body.
std::uintptr_t qualified_native_vfs_raw_failure_target();

// Finite typed dispatch: Original 00530620 uses the existing typed no-op;
// only the exact raw Source symbol is qualified and called directly by name.
// Unknown values return false without dereference. This typed interface does
// not promise the raw notifier's register/flag ABI or create an owner lifetime.
bool invoke_native_vfs_startup_failure_target(std::uintptr_t captured_target);

// Only the 32-byte Init fragment 73D63C..73D65B, after manager construction:
// load current publication/store +90=530620, reload/store +8C=735B30.
// The enclosing first-time gate and allocation belong to the application.
// As in the native fragment, each publication read must yield a valid manager.
void install_native_vfs_startup_callbacks_0073d63c(
    void* volatile& actual_manager_publication_0109ceec) noexcept;

// Finite source binding for BE18B8's captured callback entry and publication.
// Accept the verified startup identity or qualified exact raw Source symbol.
// No arbitrary indirect call or default callback is substituted.
class NativeVfsStartupCallbacks final : public NativeVfsMountFailureDispatch {
public:
    void mount_failure_00be18b8(std::uintptr_t captured_target,
        void* captured_current_manager) override;
};

} // namespace bsp
