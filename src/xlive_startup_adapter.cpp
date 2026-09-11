#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>

#include "bsp/xlive_startup_adapter.hpp"
#include "bsp/xlive_library.hpp"

#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

namespace bsp {
namespace {
template<class Result, class... Args>
Result call_startup_ordinal(void* module, std::uint16_t ordinal, Args... args) {
    static_assert(sizeof(void*) == 4, "The original XLive startup ABI is Win32.");
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(ordinal));
    if (address == nullptr)
        throw std::runtime_error("Missing XLive startup ordinal " + std::to_string(ordinal));
    using Function = Result (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}

static_assert(sizeof(XLiveInitializeInfo) == 0x1c);
static_assert(offsetof(XLiveInitializeInfo, d3d_device) == 8);
static_assert(offsetof(XLiveInitializeInfo, d3d_present_parameters) == 0xc);
static_assert(offsetof(XLiveInitializeInfo, language_id) == 0x10);
static_assert(sizeof(WSADATA) == 0x190);
static_assert(offsetof(WSADATA, wVersion) == 0);
static_assert(offsetof(WSADATA, wHighVersion) == 2);
static_assert(offsetof(WSADATA, szDescription) == 4);
static_assert(offsetof(WSADATA, szSystemStatus) == 0x105);
static_assert(offsetof(WSADATA, iMaxSockets) == 0x186);
static_assert(offsetof(WSADATA, iMaxUdpDg) == 0x188);
static_assert(offsetof(WSADATA, lpVendorInfo) == 0x18c);
static_assert(std::is_abstract_v<XLiveStartupAdapter>);
} // namespace

XLiveWsaStartupOutputError::XLiveWsaStartupOutputError(std::int32_t result)
    : std::runtime_error("XWSAStartup returned " + std::to_string(result) +
        "; this adapter cannot establish valid WSADATA output on the error path"),
      result_(result) {}

XLiveStartupAdapter::XLiveStartupAdapter(const XLiveLibrary& library)
    : module_(library.module_handle()) {
    if (module_ == nullptr) throw std::invalid_argument("XLive startup requires a live module");
}

std::uint16_t XLiveStartupAdapter::user_default_lang_id() { return GetUserDefaultLangID(); }

std::int32_t XLiveStartupAdapter::xlive_initialize_ex(const XLiveInitializeInfo& info,
    std::uint32_t version) {
    return call_startup_ordinal<std::int32_t>(module_, 5297, &info, version);
}

void XLiveStartupAdapter::x_online_startup() {
    static_cast<void>(call_startup_ordinal<std::int32_t>(module_, 5310));
}

std::int32_t XLiveStartupAdapter::x_wsa_startup(std::uint16_t requested_version,
    std::uint16_t* out_version) {
    if (out_version == nullptr)
        throw std::invalid_argument("XLive startup version output must be nonnull");
    WSADATA data; // Actual 400-byte SDK output, not a two-byte surrogate or zeroed success.
    const auto result = call_startup_ordinal<std::int32_t>(module_, 1, requested_version, &data);
    if (result != 0) throw XLiveWsaStartupOutputError(result);
    *out_version = data.wVersion;
    return result;
}

void XLiveStartupAdapter::x_wsa_cleanup() {
    static_cast<void>(call_startup_ordinal<std::int32_t>(module_, 2));
}

std::uint16_t XLiveStartupAdapter::x_socket_ntohs(std::uint16_t value) {
    return call_startup_ordinal<std::uint16_t>(module_, 38, value);
}

void XLiveStartupAdapter::x_net_set_system_link_port(std::uint16_t port) {
    static_cast<void>(call_startup_ordinal<std::int32_t>(module_, 84, port));
}

void* XLiveStartupAdapter::x_notify_create_listener(std::uint64_t areas) {
    return call_startup_ordinal<void*>(module_, 5270, areas);
}

void XLiveStartupAdapter::log(const char*) {}
void XLiveStartupAdapter::log_state_change(std::uint32_t) {}
} // namespace bsp
