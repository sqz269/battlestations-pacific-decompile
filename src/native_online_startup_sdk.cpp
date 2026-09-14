#include "bsp/native_online_startup_sdk.hpp"
#include "bsp/xlive_library.hpp"

#include <cstring>
#include <stdexcept>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native online startup SDK boundary requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

template<class Function>
Function ordinal(HMODULE module, std::uint16_t number) {
    FARPROC address = GetProcAddress(module, MAKEINTRESOURCEA(number));
    if (!address)
        throw std::runtime_error("Missing XLive startup ordinal " +
                                 std::to_string(number));
    static_assert(sizeof(Function) == sizeof(address));
    Function result;
    std::memcpy(&result, &address, sizeof(result));
    return result;
}

NativeOnlineStartupSdkImports resolve(const XLiveLibrary& library) {
    auto* const module = static_cast<HMODULE>(library.module_handle());
    if (!module) throw std::invalid_argument("Native online startup needs loaded XLive");
    NativeOnlineStartupSdkImports imports{};
    imports.get_user_default_lang_id = &GetUserDefaultLangID;
    imports.xlive_initialize_ex =
        ordinal<decltype(imports.xlive_initialize_ex)>(module, 5297);
    imports.x_online_startup =
        ordinal<decltype(imports.x_online_startup)>(module, 5310);
    imports.x_wsa_startup =
        ordinal<decltype(imports.x_wsa_startup)>(module, 1);
    imports.x_wsa_cleanup =
        ordinal<decltype(imports.x_wsa_cleanup)>(module, 2);
    imports.x_socket_ntohs =
        ordinal<decltype(imports.x_socket_ntohs)>(module, 38);
    imports.x_net_set_system_link_port =
        ordinal<decltype(imports.x_net_set_system_link_port)>(module, 84);
    imports.x_notify_create_listener =
        ordinal<decltype(imports.x_notify_create_listener)>(module, 5270);
    return imports;
}
} // namespace

NativeOnlineStartupSdkRuntime::NativeOnlineStartupSdkRuntime(const XLiveLibrary& library)
    : NativeOnlineStartupSdkRuntime(resolve(library)) {}

NativeOnlineStartupSdkRuntime::NativeOnlineStartupSdkRuntime(
    NativeOnlineStartupSdkImports imports) : imports_(imports) {
    if (!imports_.get_user_default_lang_id || !imports_.xlive_initialize_ex ||
        !imports_.x_online_startup || !imports_.x_wsa_startup ||
        !imports_.x_wsa_cleanup || !imports_.x_socket_ntohs ||
        !imports_.x_net_set_system_link_port || !imports_.x_notify_create_listener)
        throw std::invalid_argument("Native online startup import table is incomplete");
}

std::uint16_t NativeOnlineStartupSdkRuntime::user_default_lang_id() {
    return imports_.get_user_default_lang_id();
}
std::int32_t NativeOnlineStartupSdkRuntime::xlive_initialize_ex(
    NativeOnlineInitializeInfo1c* info, std::uint32_t version) {
    return imports_.xlive_initialize_ex(info, version);
}
std::int32_t NativeOnlineStartupSdkRuntime::x_online_startup() {
    return imports_.x_online_startup();
}
std::int32_t NativeOnlineStartupSdkRuntime::x_wsa_startup(
    std::uint32_t version, NativeOnlineWsadata400* output) {
    return imports_.x_wsa_startup(version, output);
}
std::int32_t NativeOnlineStartupSdkRuntime::x_wsa_cleanup() {
    return imports_.x_wsa_cleanup();
}
std::uint32_t NativeOnlineStartupSdkRuntime::x_socket_ntohs(std::uint32_t value) {
    return imports_.x_socket_ntohs(value);
}
std::int32_t NativeOnlineStartupSdkRuntime::x_net_set_system_link_port(
    std::uint32_t port) {
    return imports_.x_net_set_system_link_port(port);
}
void* NativeOnlineStartupSdkRuntime::x_notify_create_listener(
    std::uint32_t areas, std::uint32_t flags) {
    return imports_.x_notify_create_listener(areas, flags);
}
} // namespace bsp
