#pragma once
#include "bsp/frame_clock.hpp"
#include "bsp/native_online_notifications.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
class XLiveLibrary;
using NativeOnlineName128 = std::array<std::byte, 128>;

// One method per native external call. Name output is the actual 128-byte
// scratch image, including SDK writes on failure and unspecified untouched
// tail bytes. Never bind this to the projected XLiveUserName128 adapter.
class NativeOnlineSigninCalls {
public:
    virtual ~NativeOnlineSigninCalls() = default;
    virtual std::uint32_t user_get_signin_state_00a4d572(std::uint32_t user) = 0;
    virtual std::uint32_t user_get_name_00a4d566(std::uint32_t user,
        NativeOnlineName128& output, std::uint32_t capacity) = 0;
    virtual std::uint32_t user_get_xuid_00a4d56c(std::uint32_t user,
        std::uint64_t& output) = 0;
    virtual std::uint32_t user_check_privilege_00a4d482(std::uint32_t user,
        std::uint32_t privilege, std::int32_t& output) = 0;
    // A3ECBF: captured manager+20 identity, ECX=0, no stack arguments.
    virtual void call_callback20(std::uint32_t target, std::uint32_t incoming_ecx) = 0;
    // Reload the canonical 1090AB0 slot each call. Native ECX=clock, one
    // output pointer on stack, EAX=that pointer, RET4. Output does not alias
    // manager; it may mutate manager while sampling. No cached timestamp.
    virtual void sample_clock_vslot20(ClockTimestamp& output) = 0;
};

// Genuine ordinal forwarding through the SAME borrowed library and canonical
// FrameClock slot used by frame/system-time services. Slot and library outlive
// this object. No account, DLL, manager or clock is constructed or substituted.
// Installed callback 00735510 executes real XUserSetContext(ECX,8001h,6).
// Other identities throw; custom callbacks require another Calls binding.
// The existing clock sampler reports QPC failure: this binding throws rather
// than inventing the native failed-QPC uninitialized local output.
class NativeOnlineSigninRuntime final : public NativeOnlineSigninCalls {
public:
    NativeOnlineSigninRuntime(const XLiveLibrary&, FrameClock* volatile& clock_slot);
    std::uint32_t user_get_signin_state_00a4d572(std::uint32_t) override;
    std::uint32_t user_get_name_00a4d566(std::uint32_t,
        NativeOnlineName128&, std::uint32_t) override;
    std::uint32_t user_get_xuid_00a4d56c(std::uint32_t, std::uint64_t&) override;
    std::uint32_t user_check_privilege_00a4d482(std::uint32_t,
        std::uint32_t, std::int32_t&) override;
    void call_callback20(std::uint32_t, std::uint32_t) override;
    void sample_clock_vslot20(ClockTimestamp&) override;
private:
    void* module_;
    FrameClock* volatile& clock_slot_;
};

// Full normal bodies over the actual 3F0h byte storage, not the projected
// OnlineSystemState/XLiveSigninState. Caller supplies one live manager identity
// for the entire call; external calls may mutate it, but may not retire it.
// These semantic C++ entry points do not promise native entry/return ABI or
// arbitrary hardware fault equivalence. Native ECX=manager; EBD0 has one
// forced-byte DWORD stack slot and RET4; the other routines have plain RET.
// Name comparison requires native NUL-readable input; the full 128-byte copy
// retains unspecified name-tail representations without defining their value.
void restart_native_online_signin_00a3e6a0(NativeOnlineManagerStorage&) noexcept;
void refresh_native_online_user_00a3ebd0(NativeOnlineManagerStorage&,
    std::uint8_t forced, NativeOnlineSigninCalls&);
void poll_native_online_signin_00a3f3e0(NativeOnlineManagerStorage&,
    NativeOnlineSigninCalls&);
void toggle_native_online_signin_00a3f440(NativeOnlineManagerStorage&,
    NativeOnlineSigninCalls&);
} // namespace bsp
