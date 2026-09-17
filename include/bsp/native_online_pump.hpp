#pragma once

#include "bsp/native_online_notification_dispatch.hpp"
#include "bsp/native_online_signin_ui.hpp"

namespace bsp {
struct NativeOnlinePumpContext final {
    NativeOnlineNotificationContext& notifications;
    NativeOnlineSigninUiCalls& signin_ui;
    const NativeOnlineSigninUiInfo28& signin_info_preimage;
    NativeOnlineSigninUiCrt ui_crt;
    const NativeOnlineStorageSdk& storage;
    // The two virtual20 samples use the SAME output image. Its caller-owned
    // preimage is explicit; a provider must not invent successful QPC output.
    ClockTimestamp& clock_output;
    volatile std::uint8_t& first_sample_00e0e3ec;
    volatile float& last_callback_00f8abfc;
};

// Full A409F0..A40ACF normal body, original ECX=manager, RET. Both samples
// reload the canonical clock through NativeOnlineSigninCalls. Current manager
// storage-state is reloaded between upload/download; all children use the SAME
// captured manager. The saved callback time is written only for first sampling
// or ordered x87 (rounded-float current - saved-float) > double(2.0).
void pump_native_online_00a409f0(NativeOnlinePumpContext&);
} // namespace bsp
