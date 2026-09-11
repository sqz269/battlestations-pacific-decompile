#include "bsp/xlive_sdk_adapter.hpp"
#include "bsp/xlive_library.hpp"

#include <array>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace bsp {
namespace {
template<class... Args>
std::uint32_t call_ordinal(void* module, std::uint16_t ordinal, Args... args) {
    static_assert(sizeof(void*) == 4, "The recovered XLive imports use Win32 stdcall.");
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(ordinal));
    if (address == nullptr)
        throw std::runtime_error("Missing XLive ordinal " + std::to_string(ordinal));
    using Function = std::uint32_t (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}
static_assert(sizeof(wchar_t) == 2);
static_assert(sizeof(XLiveStorageDownloadResults) == 0x14);
static_assert(sizeof(XLiveOverlapped) == 0x1c);
static_assert(sizeof(XLiveAchievement) == 8);
static_assert(std::is_abstract_v<XLiveSdkAdapter>);
} // namespace

XLiveSdkAdapter::XLiveSdkAdapter(const XLiveLibrary& library) : module_(library.module_handle()) {
    if (module_ == nullptr) throw std::invalid_argument("XLive SDK adapter requires a live module");
}

std::uint32_t XLiveSdkAdapter::x_storage_build_server_path(std::uint32_t user,
    std::uint32_t facility, const void* item_info, std::uint32_t item_info_bytes,
    const wchar_t* item, wchar_t* path, std::uint32_t& path_bytes) {
    return call_ordinal(module_, 5344, user, facility, item_info, item_info_bytes,
        item, path, &path_bytes);
}

std::uint32_t XLiveSdkAdapter::x_storage_download_to_memory(std::uint32_t user,
    const wchar_t* path, std::uint32_t bytes, void* buffer, std::uint32_t result_bytes,
    XLiveStorageDownloadResults& results, XLiveOverlapped& overlapped) {
    return call_ordinal(module_, 5345, user, path, bytes, buffer, result_bytes,
        results.data(), &overlapped);
}

std::uint32_t XLiveSdkAdapter::x_storage_upload_from_memory(std::uint32_t user,
    const wchar_t* path, std::uint32_t bytes, const void* buffer, XLiveOverlapped& overlapped) {
    return call_ordinal(module_, 5305, user, path, bytes, buffer, &overlapped);
}

std::uint32_t XLiveSdkAdapter::x_storage_download_progress(XLiveOverlapped& overlapped,
    std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) {
    return call_ordinal(module_, 5307, &overlapped, &progress, total, transferred);
}

std::uint32_t XLiveSdkAdapter::x_storage_upload_progress(XLiveOverlapped& overlapped,
    std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) {
    return call_ordinal(module_, 5304, &overlapped, &progress, total, transferred);
}

std::uint32_t XLiveSdkAdapter::x_get_overlapped_result(XLiveOverlapped& overlapped,
    std::uint32_t* result, bool wait) {
    const std::int32_t wait_bool = wait ? 1 : 0; // BOOL occupies a DWORD stack slot
    return call_ordinal(module_, 1083, &overlapped, result, wait_bool);
}

std::uint32_t XLiveSdkAdapter::x_get_overlapped_extended_error(XLiveOverlapped& overlapped) {
    return call_ordinal(module_, 1082, &overlapped);
}

std::uint32_t XLiveSdkAdapter::x_user_write_achievements(std::uint32_t count,
    const XLiveAchievement* achievements, XLiveOverlapped& overlapped) {
    return call_ordinal(module_, 5278, count, achievements, &overlapped);
}

std::uint32_t XLiveSdkAdapter::x_show_message_box_ui(std::uint32_t user,
    const wchar_t* title, const wchar_t* text, std::uint32_t button_count,
    const wchar_t* const* buttons, std::uint32_t focus, std::uint32_t flags,
    std::uint32_t& choice, XLiveOverlapped& overlapped) {
    return call_ordinal(module_, 5266, user, title, text, button_count, buttons,
        focus, flags, &choice, &overlapped);
}

std::uint32_t XLiveSdkAdapter::x_show_signin_ui(std::uint32_t users, std::uint32_t flags) {
    return call_ordinal(module_, 5260, users, flags);
}

std::uint32_t XLiveSdkAdapter::x_user_get_signin_info_flags(std::uint32_t user,
    std::uint32_t flags, std::uint8_t& flags_byte_08) {
    // The installed shim allocates/copies 28h bytes for XUSER_SIGNIN_INFO.
    // Only byte8 is consumed, and only when the actual SDK reports success.
    alignas(8) std::array<std::uint8_t, 0x28> info;
    const auto result = call_ordinal(module_, 5267, user, flags, info.data());
    if (result == 0) flags_byte_08 = info[8];
    return result;
}

std::uint32_t XLiveSdkAdapter::user_get_signin_state(std::uint32_t user_index) {
    return call_ordinal(module_, 5262, user_index);
}

std::uint32_t XLiveSdkAdapter::user_get_name(std::uint32_t user_index,
    XLiveUserName128& output, std::uint32_t capacity) {
    if (capacity > output.bytes.size())
        throw std::invalid_argument("XLive name capacity exceeds the native 128-byte buffer");
    output.defined.reset();
    // Nonzero poison makes a missing SDK terminator detectable without treating
    // backing zeros as a returned name. No poison/tail bytes become defined.
    std::array<char, 128> name;
    name.fill(static_cast<char>(0xff));
    const auto result = call_ordinal(module_, 5263, user_index, name.data(), capacity);
    if (result != 0) return result;
    std::uint32_t count = 0;
    while (count < capacity && name[count] != '\0') ++count;
    if (count == capacity)
        throw std::runtime_error("XUserGetName succeeded without a bounded terminator");
    ++count;
    std::memcpy(output.bytes.data(), name.data(), count);
    for (std::uint32_t i = 0; i < count; ++i) output.defined.set(i);
    return result;
}

std::uint32_t XLiveSdkAdapter::user_get_xuid(std::uint32_t user_index, std::uint64_t& output) {
    return call_ordinal(module_, 5261, user_index, &output);
}

std::uint32_t XLiveSdkAdapter::user_check_privilege(std::uint32_t user_index,
    std::uint32_t privilege, std::int32_t& output_bool) {
    return call_ordinal(module_, 5265, user_index, privilege, &output_bool);
}
} // namespace bsp
