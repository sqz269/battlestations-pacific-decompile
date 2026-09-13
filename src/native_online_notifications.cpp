#include "bsp/native_online_notifications.hpp"

namespace bsp {
void apply_native_online_notification9_fragment_00a401a9(
    NativeOnlineManagerStorage& captured_manager,
    volatile std::uint32_t& parameter_esp14,
    NativeOnlineNotification9Hook volatile& hook_f8abec) {
    const auto captured_hook = hook_f8abec; // A401A9; one current-slot read.
    if (captured_hook != nullptr) {
        captured_hook(static_cast<std::uint8_t>(parameter_esp14 != 0));
    }
    // A401BB reads the caller's cell again after any callback mutation.
    captured_manager.notification9_3e8 =
        static_cast<std::uint8_t>(parameter_esp14 != 0);
}
} // namespace bsp
