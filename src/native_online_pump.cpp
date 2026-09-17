#include "bsp/native_online_pump.hpp"

#include <cstring>

namespace bsp {
namespace {
template<class T> T read(const NativeOnlineManagerStorage& manager, std::size_t offset) {
    T result;
    std::memcpy(&result, reinterpret_cast<const std::byte*>(&manager) + offset, sizeof(T));
    return result;
}
float sample_seconds_and_first_flag(const ClockTimestamp& sample,
    volatile std::uint8_t& first, bool& was_first) noexcept {
    const auto* pair = &sample;
    auto* const flag = &first;
    float seconds;
    unsigned char captured;
    __asm {
        mov eax, pair
        mov ecx, flag
        fild qword ptr[eax]
        cmp byte ptr[ecx], 0
        fild qword ptr[eax + 8]
        fdivp st(1), st(0)
        fstp seconds
        setne captured
    }
    was_first = captured != 0;
    return seconds;
}
bool greater_than_two_x87(float current, volatile float& saved) noexcept {
    static const double two = 2.0; // D7A308: 0000000000000040 little-endian.
    auto* const last = &saved;
    unsigned char ordered_greater;
    __asm {
        mov eax, last
        fld current
        fsub dword ptr[eax]
        fld two
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta ordered_greater
    }
    return ordered_greater != 0;
}
} // namespace
void pump_native_online_00a409f0(NativeOnlinePumpContext& context) {
    auto& notifications = context.notifications;
    auto& manager = notifications.manager;
    drain_native_online_notifications_00a40110(notifications);
    NativeOnlineSigninUiContext ui{manager, context.signin_ui, notifications.signin,
        notifications.strings, context.signin_info_preimage, context.ui_crt};
    pump_native_online_signin_ui_00a40510(ui);
    notifications.signin.sample_clock_vslot20(context.clock_output);
    if (read<std::uint32_t>(manager, 0x12c) == 7)
        upload_native_online_storage_00a3ef20(manager, context.storage, notifications.memory);
    if (read<std::uint32_t>(manager, 0x12c) == 3)
        download_native_online_storage_00a3ed60(manager, context.storage, notifications.memory);
    pump_native_online_achievements_00a3fa70(manager, 0,
        notifications.achievements, notifications.memory, notifications.crt);
    notifications.signin.sample_clock_vslot20(context.clock_output);
    bool was_first;
    const float current = sample_seconds_and_first_flag(context.clock_output,
        context.first_sample_00e0e3ec, was_first);
    if (was_first) context.last_callback_00f8abfc = current;
    context.first_sample_00e0e3ec = 0;
    if (!greater_than_two_x87(current, context.last_callback_00f8abfc)) return;
    bool enabled = read<std::uint32_t>(manager, 0x8c) != 0;
    if (read<std::uint8_t>(manager, 0x119) != 0 && read<std::uint32_t>(manager, 0x11c) == 0)
        enabled = false;
    const auto callback = read<std::uint32_t>(manager, 0x20);
    if (callback != 0 && enabled) notifications.signin.call_callback20(callback, 0);
    context.last_callback_00f8abfc = current; // Overwrites callback mutation.
}
} // namespace bsp
