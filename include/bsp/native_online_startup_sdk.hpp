#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <windows.h>

namespace bsp {
class XLiveLibrary;

// A40DF0's actual 1Ch stack image. The parent zeroes it, sets size=1Ch, then
// writes the current renderer device, the SAME mutable F8D394+1A28 pointer,
// and the Win32 LANGID. This adapter neither constructs nor copies the image.
struct NativeOnlineInitializeInfo1c final {
    std::uint32_t size_00;
    std::uint32_t reserved_04;
    void* d3d_device_08;
    void* present_parameters_0c;
    std::uint16_t language_id_10;
    std::uint16_t reserved_12;
    std::uint32_t reserved_14;
    std::uint32_t reserved_18;
};
static_assert(sizeof(NativeOnlineInitializeInfo1c) == 0x1c);
static_assert(std::is_standard_layout_v<NativeOnlineInitializeInfo1c>);
static_assert(offsetof(NativeOnlineInitializeInfo1c, d3d_device_08) == 8);
static_assert(offsetof(NativeOnlineInitializeInfo1c, present_parameters_0c) == 0xc);
static_assert(offsetof(NativeOnlineInitializeInfo1c, language_id_10) == 0x10);

// Exact Win32 WSADATA output image. A40DF0's local has an unspecified preimage;
// it reads version_00 even when XWSAStartup returns failure. SDK partial writes
// remain in this caller-owned buffer, and no result is converted to an exception.
struct NativeOnlineWsadata400 final {
    std::uint16_t version_00;
    std::uint16_t high_version_02;
    std::byte remaining_04[0x18c];
};
static_assert(sizeof(NativeOnlineWsadata400) == 0x190);
static_assert(std::is_standard_layout_v<NativeOnlineWsadata400>);
static_assert(offsetof(NativeOnlineWsadata400, version_00) == 0);

// Typed already-resolved imports. Function pointers borrow their loaded
// modules; callers keep those modules alive. Each SDK member is Win32 stdcall
// with the original DWORD stack slots and raw return bits.
struct NativeOnlineStartupSdkImports final {
    LANGID (WINAPI* get_user_default_lang_id)();
    std::int32_t (__stdcall* xlive_initialize_ex)(
        NativeOnlineInitializeInfo1c*, std::uint32_t);
    std::int32_t (__stdcall* x_online_startup)();
    std::int32_t (__stdcall* x_wsa_startup)(
        std::uint32_t, NativeOnlineWsadata400*);
    std::int32_t (__stdcall* x_wsa_cleanup)();
    std::uint32_t (__stdcall* x_socket_ntohs)(std::uint32_t);
    std::int32_t (__stdcall* x_net_set_system_link_port)(std::uint32_t);
    void* (__stdcall* x_notify_create_listener)(std::uint32_t, std::uint32_t);
};
static_assert(std::is_standard_layout_v<NativeOnlineStartupSdkImports>);

class NativeOnlineStartupSdkCalls {
public:
    virtual ~NativeOnlineStartupSdkCalls() = default;
    virtual std::uint16_t user_default_lang_id() = 0;
    virtual std::int32_t xlive_initialize_ex(
        NativeOnlineInitializeInfo1c*, std::uint32_t) = 0;
    virtual std::int32_t x_online_startup() = 0;
    virtual std::int32_t x_wsa_startup(
        std::uint32_t, NativeOnlineWsadata400*) = 0;
    virtual std::int32_t x_wsa_cleanup() = 0;
    virtual std::uint32_t x_socket_ntohs(std::uint32_t) = 0;
    virtual std::int32_t x_net_set_system_link_port(std::uint32_t) = 0;
    virtual void* x_notify_create_listener(std::uint32_t, std::uint32_t) = 0;
};

// Borrow one already loaded XLiveLibrary or an equivalent already-resolved
// import table. Construction resolves/checks pointers but calls no SDK method.
// Unlike the projected XLiveStartupAdapter, the raw XWSA method passes the
// full caller's 400-byte image and returns the SDK result without translation.
class NativeOnlineStartupSdkRuntime final : public NativeOnlineStartupSdkCalls {
public:
    explicit NativeOnlineStartupSdkRuntime(const XLiveLibrary&);
    explicit NativeOnlineStartupSdkRuntime(NativeOnlineStartupSdkImports);
    std::uint16_t user_default_lang_id() override;
    std::int32_t xlive_initialize_ex(
        NativeOnlineInitializeInfo1c*, std::uint32_t) override;
    std::int32_t x_online_startup() override;
    std::int32_t x_wsa_startup(
        std::uint32_t, NativeOnlineWsadata400*) override;
    std::int32_t x_wsa_cleanup() override;
    std::uint32_t x_socket_ntohs(std::uint32_t) override;
    std::int32_t x_net_set_system_link_port(std::uint32_t) override;
    void* x_notify_create_listener(std::uint32_t, std::uint32_t) override;
private:
    NativeOnlineStartupSdkImports imports_;
};
} // namespace bsp
