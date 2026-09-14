#pragma once

#include "bsp/native_online_notifications.hpp"
#include "bsp/native_online_signin.hpp"
#include "bsp/native_string.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <windows.h>
#include <shellapi.h>

namespace bsp {
class XLiveLibrary;

// The SDK writes this exact 218h Win32 image. In particular path_10 is 260
// 16-bit units, regardless of the host C++ standard library's wide-string type.
struct NativeOnlineUpdateInformation218 final {
    std::uint32_t cb_size_00;
    std::uint32_t type_04;
    std::array<std::byte, 8> opaque_08;
    std::array<wchar_t, 260> path_10;
};
static_assert(sizeof(wchar_t) == 2);
static_assert(sizeof(NativeOnlineUpdateInformation218) == 0x218);
static_assert(offsetof(NativeOnlineUpdateInformation218, path_10) == 0x10);

class NativeOnlineProfileNameCalls {
public:
    virtual ~NativeOnlineProfileNameCalls() = default;
    // A4D566 -> XUserGetName ordinal 5263, Win32 stdcall, DWORD result.
    // The supplied image has unspecified preimage; SDK writes survive success.
    virtual std::uint32_t user_get_name_00a4d566(std::uint32_t user,
        NativeOnlineName128& output, std::uint32_t capacity) = 0;
    // Captured manager+18 target is reloaded after the 128-byte name copy.
    // A3E67D supplies ECX=0 and no stack arguments.
    virtual void call_callback18(std::uint32_t target, std::uint32_t incoming_ecx) = 0;
};

class NativeOnlineUpdateCalls {
public:
    virtual ~NativeOnlineUpdateCalls() = default;
    // A4D59C -> XLiveGetUpdateInformation ordinal 5022, signed HRESULT.
    virtual std::int32_t get_update_information_00a4d59c(
        NativeOnlineUpdateInformation218& output) = 0;
    virtual LSTATUS reg_open_key(HKEY root, const char* subkey, DWORD options,
        REGSAM access, HKEY& result) = 0;
    virtual LSTATUS reg_query_value(HKEY key, const char* name, DWORD* reserved,
        DWORD& type, BYTE* data, DWORD& bytes) = 0;
    virtual LSTATUS reg_close_key(HKEY key) = 0;
    virtual BOOL shell_execute(SHELLEXECUTEINFOA& request) = 0;
};

// Production boundary borrows one already loaded XLive module. It makes actual
// SDK ordinal and Win32 calls; constructing it does not make a request. The
// installed callback18 identity 00735520 is the XUserSetContext(0,8001h,4)
// stub. A different target needs a caller-supplied Calls binding.
class NativeOnlineNotificationLeafRuntime final : public NativeOnlineProfileNameCalls,
                                                  public NativeOnlineUpdateCalls {
public:
    explicit NativeOnlineNotificationLeafRuntime(const XLiveLibrary&);
    std::uint32_t user_get_name_00a4d566(std::uint32_t,
        NativeOnlineName128&, std::uint32_t) override;
    void call_callback18(std::uint32_t, std::uint32_t) override;
    std::int32_t get_update_information_00a4d59c(
        NativeOnlineUpdateInformation218&) override;
    LSTATUS reg_open_key(HKEY, const char*, DWORD, REGSAM, HKEY&) override;
    LSTATUS reg_query_value(HKEY, const char*, DWORD*, DWORD&, BYTE*, DWORD&) override;
    LSTATUS reg_close_key(HKEY) override;
    BOOL shell_execute(SHELLEXECUTEINFOA&) override;
private:
    void* module_;
};

// Complete normal A3E600. Original ECX=actual 3F0h manager, mask byte in one
// stack DWORD, RET4. Only bit0 gates a name query. Successful different name
// copies all 128 bytes, then tests the current +11C and +18 for callback.
void refresh_native_online_profile_name_00a3e600(NativeOnlineManagerStorage&,
    std::uint8_t mask, NativeOnlineProfileNameCalls&);

// Complete normal A3FF20/A3FDE0. Original output is a pointer to the actual
// eight-byte NativeString header on the stack; incoming ECX unused, RET4, AL
// boolean. Supply the live ActualNativeStringPoolStorage as storage in the
// raw dispatcher. Failure and partial SDK/registry writes are not sanitized.
bool title_native_online_update_path_00a3ff20(NativeString& output,
    NativeStringStorage& storage, NativeOnlineUpdateCalls&);
bool system_native_online_update_path_00a3fde0(NativeString& output,
    NativeStringStorage& storage, NativeOnlineUpdateCalls&);

// Complete normal A3E560. Original two C-string pointers on stack, RET8;
// incoming ECX unused, EAX boolean. Nonnull executable genuinely calls
// ShellExecuteExA in the production runtime. The caller owns later sleep/exit.
bool launch_native_online_update_00a3e560(const char* executable,
    const char* parameters, NativeOnlineUpdateCalls&);
} // namespace bsp
