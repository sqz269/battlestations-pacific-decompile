#pragma once

#include "bsp/native_string.hpp"
#include "bsp/platform_loop.hpp"

#include <shellapi.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

// Original XLiveGetUpdateInformation output, not an invented SDK definition.
// Only cbSize, type and the 260-unit UTF16 path are consumed by 00A3FF20.
struct XLiveUpdateInformation {
    std::uint32_t cb_size_00;
    std::uint32_t type_04;
    std::array<std::uint32_t, 2> opaque_08;
    std::array<wchar_t, 260> path_10;
};
static_assert(sizeof(XLiveUpdateInformation) == 0x218);
static_assert(offsetof(XLiveUpdateInformation, path_10) == 0x10);

class XLiveUpdateInformationHost {
public:
    virtual ~XLiveUpdateInformationHost() = default;
    // Game thunk A4D59C -> IAT CE26E4 -> ordinal5022. Win32 stdcall,
    // one pointer argument; signed HRESULT. Caller zeroes the whole object.
    virtual std::int32_t x_live_get_update_information(XLiveUpdateInformation&) = 0;
};

class XLiveUpdatePlatformHost {
public:
    virtual ~XLiveUpdatePlatformHost() = default;
    virtual LSTATUS reg_open_key(HKEY root, const char* subkey, DWORD options,
        REGSAM access, HKEY& result) = 0;
    virtual LSTATUS reg_query_value(HKEY key, const char* name, DWORD* reserved,
        DWORD& type, BYTE* data, DWORD& bytes) = 0;
    virtual LSTATUS reg_close_key(HKEY key) = 0;
    virtual BOOL shell_execute(SHELLEXECUTEINFOA& request) = 0;
};

// Calls the actual Win32 APIs. Merely constructing this host has no effects.
class Win32XLiveUpdatePlatform final : public XLiveUpdatePlatformHost {
public:
    LSTATUS reg_open_key(HKEY, const char*, DWORD, REGSAM, HKEY&) override;
    LSTATUS reg_query_value(HKEY, const char*, DWORD*, DWORD&, BYTE*, DWORD&) override;
    LSTATUS reg_close_key(HKEY) override;
    BOOL shell_execute(SHELLEXECUTEINFOA&) override;
};

// Full 00436630; ECX destination8h, wide pointer stack, EAX destination, RET4.
// Constructor semantics: overwrite header, without releasing its old buffer.
// Allocate full wide length; copy low bytes through the first low-byte zero.
// Tail allocation bytes remain untouched; they are NOT zero-filled Unicode.
NativeString& construct_narrow_from_wide_00436630(NativeString& destination,
    const wchar_t* source, NativeStringStorage& storage = crt_string_storage());

// Full 0041E350; ECX destination8h, C-string pointer stack, EAX destination,
// RET4. Null assigns length0; resize(preserve=false), then copy current length.
NativeString& assign_native_cstring_0041e350(NativeString& destination,
    const char* source, NativeStringStorage& storage = crt_string_storage());

// 004C5E60 value projection through existing startup_widen_path. Native original:
// ECX fresh wide header8h, C-string pointer stack, EAX header, RET4. Source must
// be nonnull. Byte widening is exact; native wide allocation/header ABI is not.
std::wstring widen_update_path_004c5e60(const char* source);

// Full normal bodies. Original A3FF20/A3FDE0 take output-header pointer on the
// stack, RET4, AL bool; incoming ECX is unused. The game-service layer may
// discard these booleans, as notification dispatch does.
bool title_update_path_00a3ff20(NativeString& output, XLiveUpdateInformationHost& sdk,
    NativeStringStorage& storage = crt_string_storage());
bool system_update_path_00a3fde0(NativeString& output, XLiveUpdatePlatformHost& platform,
    NativeStringStorage& storage = crt_string_storage());

// Full A3E560; two pointers on stack, RET8; incoming ECX unused, EAX bool.
// Null executable returns false without calling the host. This helper launches
// only; notification dispatch owns its later sleep/exit behavior.
bool launch_update_00a3e560(const char* executable, const char* parameters,
    XLiveUpdatePlatformHost& platform);

} // namespace bsp
