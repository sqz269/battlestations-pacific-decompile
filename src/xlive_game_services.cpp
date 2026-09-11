#include "bsp/xlive_game_services.hpp"

#include <stdexcept>

namespace bsp {

RecoveredXLiveGameServices::RecoveredXLiveGameServices(const LocaleTextResolver& locale,
    XLiveUpdateInformationHost& sdk, XLiveUpdatePlatformHost& platform,
    NativeStringStorage& strings) noexcept
    : locale_(locale), sdk_(sdk), platform_(platform), strings_(strings) {}

std::wstring RecoveredXLiveGameServices::resolve_localization_00a9fad0(
    const NativeString& key) {
    if (key.length() && !key.data())
        throw std::invalid_argument("localization key has length but no data");
    const auto text = locale_.resolve(key.length()
        ? std::string(key.data(), key.length()) : std::string{});
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "The native text ABI is UTF16.");
    return std::wstring(text.begin(), text.end());
}
void RecoveredXLiveGameServices::title_update_path_00a3ff20(NativeString& output) {
    (void)bsp::title_update_path_00a3ff20(output, sdk_, strings_);
}
void RecoveredXLiveGameServices::system_update_path_00a3fde0(NativeString& output) {
    (void)bsp::system_update_path_00a3fde0(output, platform_, strings_);
}
std::wstring RecoveredXLiveGameServices::widen_update_path_004c5e60(const char* path) {
    return bsp::widen_update_path_004c5e60(path);
}
void RecoveredXLiveGameServices::launch_update_00a3e560(const char* exe, const char* path) {
    (void)bsp::launch_update_00a3e560(exe, path, platform_);
}

} // namespace bsp
