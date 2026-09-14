#include "bsp/native_online_signin.hpp"
#include "bsp/xlive_library.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native online sign-in requires MSVC Win32 x87 behavior.
#endif

namespace bsp {
namespace {
template<class T>
T read(const NativeOnlineManagerStorage& manager, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const std::byte*>(&manager) + offset, sizeof(value));
    return value;
}
template<class T>
void write(NativeOnlineManagerStorage& manager, std::size_t offset, T value) noexcept {
    std::memcpy(reinterpret_cast<std::byte*>(&manager) + offset, &value, sizeof(value));
}
void byte(NativeOnlineManagerStorage& manager, std::size_t offset, std::uint8_t value) noexcept {
    write(manager, offset, value);
}
bool same_name(const NativeOnlineName128& name, const NativeOnlineManagerStorage& manager) {
    // Native unbounded strcmp, read in pairs. The supported SDK contract has
    // a terminator within 128 bytes; reject an invalid provider at the bound.
    for (std::size_t i = 0; i != name.size(); ++i) {
        const auto ch = std::to_integer<std::uint8_t>(name[i]);
        if (ch != read<std::uint8_t>(manager, 0x90 + i)) return false;
        if (ch == 0) return true;
    }
    throw std::runtime_error("Native online username lacks a bounded terminator");
}
bool past_one(const ClockTimestamp& difference) noexcept {
    const float seconds = timestamp_seconds_x87(difference);
    unsigned char result;
    __asm {
        fld1
        fld seconds
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
template<class... Args>
std::uint32_t ordinal(void* module, std::uint16_t number, Args... args) {
    static_assert(sizeof(void*) == 4);
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(number));
    if (!address) throw std::runtime_error("Missing XLive ordinal " + std::to_string(number));
    using Function = std::uint32_t (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}
} // namespace

NativeOnlineSigninRuntime::NativeOnlineSigninRuntime(const XLiveLibrary& library,
    FrameClock* volatile& clock_slot)
    : module_(library.module_handle()), clock_slot_(clock_slot) {
    if (!module_) throw std::invalid_argument("Native sign-in needs the live XLive library");
}
std::uint32_t NativeOnlineSigninRuntime::user_get_signin_state_00a4d572(std::uint32_t user) {
    return ordinal(module_, 5262, user);
}
std::uint32_t NativeOnlineSigninRuntime::user_get_name_00a4d566(std::uint32_t user,
    NativeOnlineName128& output, std::uint32_t capacity) {
    if (capacity > output.size()) throw std::invalid_argument("Native name capacity exceeds 128");
    return ordinal(module_, 5263, user, reinterpret_cast<char*>(output.data()), capacity);
}
std::uint32_t NativeOnlineSigninRuntime::user_get_xuid_00a4d56c(std::uint32_t user,
    std::uint64_t& output) {
    return ordinal(module_, 5261, user, &output);
}
std::uint32_t NativeOnlineSigninRuntime::user_check_privilege_00a4d482(std::uint32_t user,
    std::uint32_t privilege, std::int32_t& output) {
    return ordinal(module_, 5265, user, privilege, &output);
}
void NativeOnlineSigninRuntime::call_callback20(std::uint32_t target, std::uint32_t ecx) {
    if (target != 0x00735510u)
        throw std::runtime_error("Unbound native online manager callback20 identity");
    static_cast<void>(ordinal(module_, 5277, ecx, 0x8001u, 6u));
}
void NativeOnlineSigninRuntime::sample_clock_vslot20(ClockTimestamp& output) {
    FrameClock* const clock = clock_slot_;
    if (!clock) throw std::runtime_error("Native online clock singleton is unavailable");
    if (!sample_frame_clock_00bee080(*clock, output))
        throw std::runtime_error("QPC failed: native unspecified output is not reproduced");
}

void restart_native_online_signin_00a3e6a0(NativeOnlineManagerStorage& manager) noexcept {
    if (read<std::uint8_t>(manager, 0x120) != 0 && read<std::uint8_t>(manager, 0x3bc) != 0)
        byte(manager, 0x3bc, 0);
    byte(manager, 0x119, 0);
    byte(manager, 0x11a, 0);
    write(manager, 0x11c, std::uint32_t{1});
    byte(manager, 0x2f, 0);
    byte(manager, 0x2d, 0);
    byte(manager, 0x2e, 0);
    byte(manager, 0x120, 0);
    write(manager, 0x124, std::uint32_t{0xffffffff});
    write(manager, 0x3b8, std::uint32_t{1});
    byte(manager, 0x2c, 1);
    byte(manager, 0x31, 0);
}

void refresh_native_online_user_00a3ebd0(NativeOnlineManagerStorage& manager,
    std::uint8_t forced, NativeOnlineSigninCalls& calls) {
    const auto state = calls.user_get_signin_state_00a4d572(0);
    std::int32_t privilege = 0;
    NativeOnlineName128 name; // Native scratch preimage is unspecified, not zero-filled.
    if (calls.user_get_name_00a4d566(0, name, 128) != 0) name[0] = std::byte{0};
    std::uint64_t xuid;
    if (calls.user_get_xuid_00a4d56c(0, xuid) != 0) xuid = 0;
    if (forced == 0 && state == 0) return;
    if (state == 2) {
        if (calls.user_check_privilege_00a4d482(0, 0xfe, privilege) != 0) privilege = 0;
        const auto status = read<std::uint32_t>(manager, 0x3b8);
        if (status == 0 || status == 1) byte(manager, 0x128, 0);
    }
    if (read<std::uint32_t>(manager, 0x3b8) == 0) {
        if ((state != 0) != (read<std::uint32_t>(manager, 0x8c) != 0) || !same_name(name, manager))
            restart_native_online_signin_00a3e6a0(manager);
    } else {
        const auto callback = read<std::uint32_t>(manager, 0x20);
        if (callback != 0 && state != 0) calls.call_callback20(callback, 0);
    }
    write(manager, 0x8c, state);
    std::memcpy(reinterpret_cast<std::byte*>(&manager) + 0x90, name.data(), name.size());
    write(manager, 0x110, xuid);
    byte(manager, 0x118, static_cast<std::uint8_t>(privilege != 0));
}

void poll_native_online_signin_00a3f3e0(NativeOnlineManagerStorage& manager,
    NativeOnlineSigninCalls& calls) {
    if (read<std::uint8_t>(manager, 4) == 0) return;
    ClockTimestamp sampled;
    calls.sample_clock_vslot20(sampled);
    // Original pointer manager+8 survives sampling: read it AFTER the call.
    const auto saved = read<ClockTimestamp>(manager, 8);
    ClockTimestamp difference;
    subtract_timestamp_00530890(difference, sampled, saved);
    if (past_one(difference)) {
        byte(manager, 4, 0);
        refresh_native_online_user_00a3ebd0(manager, 1, calls);
    }
}

void toggle_native_online_signin_00a3f440(NativeOnlineManagerStorage& manager,
    NativeOnlineSigninCalls& calls) {
    if (read<std::uint8_t>(manager, 4) != 0) {
        refresh_native_online_user_00a3ebd0(manager, 1, calls);
        byte(manager, 4, 0);
    } else {
        refresh_native_online_user_00a3ebd0(manager, 0, calls);
        byte(manager, 4, 1);
        ClockTimestamp sampled;
        calls.sample_clock_vslot20(sampled);
        write(manager, 8, sampled); // Includes frequency-low alias manager+10.
    }
}
} // namespace bsp
