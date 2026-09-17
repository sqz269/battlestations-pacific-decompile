#pragma once

#include "bsp/native_online_achievements.hpp"
#include "bsp/native_online_notification_leaves.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
class XLiveLibrary;

class NativeOnlineNotificationCalls {
public:
    virtual ~NativeOnlineNotificationCalls() = default;
    virtual void* create_listener(std::uint64_t areas) = 0;
    virtual bool get_next(void* listener, std::uint32_t filter,
        std::uint32_t& id, std::uint32_t& parameter) = 0;
    // A40328 ignores the DWORD result. The SDK may leave any payload bytes
    // untouched. This is a pointer to the caller's explicit native stack image,
    // not a returned, value-initialized XLiveAcceptedInvite projection.
    virtual std::uint32_t accepted_invite(std::uint32_t user, void* output) = 0;
    virtual std::int32_t update_system(const wchar_t* path) = 0;
    virtual void sleep_milliseconds(std::uint32_t) = 0;
    // Genuine _exit: must not return or throw. Native exit does not unwind.
    [[noreturn]] virtual void exit_process(int code) = 0;
};

// Borrows an already loaded DLL. Resolves/calls actual Win32 SDK ordinals;
// no module loading, manager construction or requests occur in its constructor.
class NativeOnlineNotificationRuntime final : public NativeOnlineNotificationCalls {
public:
    explicit NativeOnlineNotificationRuntime(const XLiveLibrary&);
    void* create_listener(std::uint64_t) override;
    bool get_next(void*, std::uint32_t, std::uint32_t&, std::uint32_t&) override;
    std::uint32_t accepted_invite(std::uint32_t, void*) override;
    std::int32_t update_system(const wchar_t*) override;
    void sleep_milliseconds(std::uint32_t) override;
    [[noreturn]] void exit_process(int) override;
private:
    void* module_;
};

// No image writer for F8ABF0 was found. Null is the established image domain.
// A nonnull source hook must be a caller-supplied no-argument cdecl thunk;
// this does not recover an unknown native hook's register-input ABI.
using NativeOnlineNotification11Hook = void (__cdecl*)();
struct NativeOnlineNotificationPublications {
    NativeOnlineNotification9Hook volatile& hook_00f8abec;
    NativeOnlineNotification11Hook volatile& hook_00f8abf0;
    // Actual borrowed object and its CURRENT vtable. Slot+28 has native
    // thiscall/RET ABI. No session projection or cached virtual target is used.
    void* volatile& client_00f8a2fc;
};

// Caller-owned stack preimage. Initialize id, parameter and invite_stack before
// entry; native XNotifyGetNext and XInviteGetAcceptedInfo can partially write
// them. 5Ch is the local window before the EH record, NOT an SDK structure size.
// Exactly the first 54h bytes are copied to manager+32h, even on SDK failure.
// The operation and manager must occupy separate storage and remain alive.
struct NativeOnlineNotificationOperation final {
    std::uint32_t id;
    std::uint32_t parameter;
    alignas(4) std::array<std::byte, 0x5c> invite_stack;
    NativeString header_20;
    NativeString header_28;
    bool active{false};
    int cleanup_state{-1};
    bool cleanup_failed{false};
};

struct NativeOnlineNotificationContext final {
    NativeOnlineManagerStorage& manager;
    NativeOnlineNotificationOperation& operation;
    NativeOnlineNotificationPublications publications;
    NativeOnlineNotificationCalls& notifications;
    NativeOnlineSigninCalls& signin;
    NativeOnlineProfileNameCalls& profile;
    NativeOnlineUpdateCalls& updates;
    NativeStringRawPoolContext& strings;
    const NativeOnlineAchievementsSdk& achievements;
    const NativeOnlineStorageMemory& memory;
    const NativeOnlineAchievementsCrt& crt;
};

// Complete normal A40110..A404F3 schedule over one captured actual 3F0h owner.
// Explicit semantic C++ API, not native ECX/RET or FH3/SEH substitution.
// A throwing call retains active=true and surviving raw headers for diagnosis;
// replay is refused. The original FH3 states select string returns only. In
// state0 the wide header20 is deliberately NOT destroyed during unwind.
void drain_native_online_notifications_00a40110(NativeOnlineNotificationContext&);
} // namespace bsp
