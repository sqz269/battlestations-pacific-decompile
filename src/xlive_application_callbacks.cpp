#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "bsp/xlive_application_callbacks.hpp"
#include "bsp/xlive_library.hpp"

#include <cstring>
#include <stdexcept>
#include <string_view>

namespace bsp {
namespace {
std::uint8_t known_byte(const XLiveUserName128& name, std::uint32_t index) {
    if (index >= name.bytes.size() || !name.defined.test(index))
        throw std::logic_error("Native selected username byte is unspecified");
    return name.bytes[index];
}

class TemporaryName {
public:
    explicit TemporaryName(NativeStringStorage& storage) noexcept : storage_(storage) {}
    ~TemporaryName() { name.release_to(storage_); }
    NativeString name;
private:
    NativeStringStorage& storage_;
};

void construct_known_name(NativeString& output, const XLiveUserName128& source,
    NativeStringStorage& storage) {
    // The fresh 0041E870 constructor scans first, allocates, then copies the
    // original source pointer's current length+1 bytes. Keep that order even
    // if storage allocation reenters and changes the manager's name bytes.
    std::uint32_t length = 0;
    while (known_byte(source, length) != 0u) ++length;
    output.resize_0041dd40(storage, length, true);
    if (output.data() != nullptr) {
        for (std::uint32_t index = 0; index <= length; ++index)
            static_cast<void>(known_byte(source, index));
        std::memcpy(output.data(), source.bytes.data(), length + 1u);
    }
}
} // namespace

XLiveApplicationContextAdapter::XLiveApplicationContextAdapter(const XLiveLibrary& library)
    : module_(library.module_handle()) {
    if (module_ == nullptr) throw std::invalid_argument("XLive context requires a live module");
}

void XLiveApplicationContextAdapter::user_set_context(std::uint32_t user_index,
    std::uint32_t context_id, std::uint32_t value) {
    static_assert(sizeof(void*) == 4, "The original XLive context ABI is Win32.");
    const auto address = GetProcAddress(static_cast<HMODULE>(module_), MAKEINTRESOURCEA(5277));
    if (address == nullptr) throw std::runtime_error("Missing XLive ordinal 5277 (XUserSetContext)");
    using Function = void (__stdcall*)(std::uint32_t, std::uint32_t, std::uint32_t);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    function(user_index, context_id, value);
}

void set_online_context_six_00735510(std::uint32_t incoming_ecx,
    XLiveApplicationContextHost& sdk) {
    sdk.user_set_context(incoming_ecx, 0x8001u, 6u);
}

void set_online_context_four_00735520(std::uint32_t incoming_ecx,
    XLiveApplicationContextHost& sdk) {
    sdk.user_set_context(incoming_ecx, 0x8001u, 4u);
}

std::uint32_t selected_username_offset_00a3eae0(const OnlineSystemState& manager) noexcept {
    return (manager.signin_state_11c << 7u) + 0x90u;
}

void apply_online_profile_name_00737d60(XLiveApplicationGlobals& globals,
    NativeStringStorage& storage) {
    const auto manager = globals.current_manager_00f8abe8();
    if (selected_username_offset_00a3eae0(manager.online) != 0x90u)
        throw std::logic_error("Native selected username lies outside the canonical name cache");

    TemporaryName temporary(storage);
    construct_known_name(temporary.name, manager.signin.cached_username_90, storage);
    const auto name = temporary.name.data() == nullptr ? std::string_view{} :
        std::string_view(temporary.name.data(), temporary.name.length());
    const auto display_target = globals.current_game_profile_00e188a8();
    set_profile_display_name_007f9340(display_target.profile, display_target.game_name, name);
    const auto player_target = globals.current_game_profile_00e188a8();
    set_profile_name_007f9290(player_target.profile, player_target.game_name, name);
}

XLiveApplicationCallbackInvoker::XLiveApplicationCallbackInvoker(XLiveApplicationGlobals& globals,
    XLiveApplicationContextHost& sdk, NativeStringStorage& storage) noexcept
    : globals_(globals), sdk_(sdk), storage_(storage) {}

void XLiveApplicationCallbackInvoker::invoke_state_callback(const void* callback) {
    invoke_callback(callback, 0u);
}

void XLiveApplicationCallbackInvoker::invoke_callback(const void* callback,
    std::uint32_t incoming_ecx) {
    switch (reinterpret_cast<std::uintptr_t>(callback)) {
    case kOnlineCallbackStubA:
        set_online_context_six_00735510(incoming_ecx, sdk_);
        return;
    case kOnlineCallbackStubB:
        set_online_context_four_00735520(incoming_ecx, sdk_);
        return;
    case kOnlineProfileNameCallback:
        apply_online_profile_name_00737d60(globals_, storage_);
        return;
    default:
        throw std::invalid_argument("Unreconstructed XLive application callback identity");
    }
}
} // namespace bsp
