#pragma once

#include "bsp/native_online_notifications.hpp"
#include "bsp/native_online_signin.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {
class LocaleTextResolver;
class XLiveLibrary;

// The original stack output is an eight-byte Win32 wide-string header, with
// no inline buffer or owner. These four headers are local to a state-1 call.
struct NativeOnlineUiWide8 {
    std::uint32_t length;
    wchar_t* data;
};
static_assert(sizeof(NativeOnlineUiWide8) == 8);

// Each virtual call is a real, observable native boundary. A test double may
// mutate the same manager during any call; the pump rereads fields afterward.
class NativeOnlineSigninUiCalls {
public:
    virtual ~NativeOnlineSigninUiCalls() = default;
    virtual void append_localization_00a9fad0(NativeOnlineUiWide8& output,
        const NativeString& key, NativeStringStorage& storage) = 0;
    virtual std::uint32_t show_message_box_00a4d5c0(std::uint32_t user,
        const wchar_t* title, const wchar_t* text, std::uint32_t button_count,
        const wchar_t* const* buttons, std::uint32_t focus, std::uint32_t flags,
        std::uint32_t* choice, void* overlapped) = 0;
    virtual std::uint32_t get_overlapped_result_00a4d42e(
        void* overlapped, std::uint32_t* result, std::uint32_t wait) = 0;
    virtual std::uint32_t show_signin_00a4d5ba(
        std::uint32_t users, std::uint32_t flags) = 0;
    // The caller owns an uninitialized 28h-byte stack output. The SDK may
    // write only part of it; byte +8 is read only after a zero return.
    virtual std::uint32_t user_get_signin_info_00a4d560(
        std::uint32_t user, std::uint32_t flags, void* output_28h) = 0;
    virtual void build_storage_path_00a3ed10(NativeOnlineManagerStorage&) = 0;
};

// Borrows the already-loaded XLive module and the caller's current locale
// tables/Lua-context resolver. No manager, pool, locale table or DLL is made.
class NativeOnlineSigninUiRuntime final : public NativeOnlineSigninUiCalls {
public:
    NativeOnlineSigninUiRuntime(const XLiveLibrary&, const LocaleTextResolver&);
    void append_localization_00a9fad0(NativeOnlineUiWide8&,
        const NativeString&, NativeStringStorage&) override;
    std::uint32_t show_message_box_00a4d5c0(std::uint32_t, const wchar_t*,
        const wchar_t*, std::uint32_t, const wchar_t* const*, std::uint32_t,
        std::uint32_t, std::uint32_t*, void*) override;
    std::uint32_t get_overlapped_result_00a4d42e(
        void*, std::uint32_t*, std::uint32_t) override;
    std::uint32_t show_signin_00a4d5ba(std::uint32_t, std::uint32_t) override;
    std::uint32_t user_get_signin_info_00a4d560(
        std::uint32_t, std::uint32_t, void*) override;
    void build_storage_path_00a3ed10(NativeOnlineManagerStorage&) override;
private:
    void* module_;
    const LocaleTextResolver& locale_;
};

struct NativeOnlineSigninUiContext {
    NativeOnlineManagerStorage& manager;
    NativeOnlineSigninUiCalls& calls;
    NativeOnlineSigninCalls& signin_calls;
    NativeStringRawPoolContext& strings;
};

// Complete normal bodies, as semantic C++ interfaces over the captured actual
// 3F0h manager. Native entry for each is ECX=manager and RET. These interfaces
// do not claim binary ABI substitution, SEH parity or game validation.
void reset_native_online_ui_slots_00a3e700(
    NativeOnlineManagerStorage& manager) noexcept;
void reset_native_online_signin_state_00a40020(
    NativeOnlineManagerStorage& manager, NativeOnlineSigninCalls& calls);
void pump_native_online_signin_ui_00a40510(NativeOnlineSigninUiContext& context);
} // namespace bsp
