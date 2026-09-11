#pragma once

#include "bsp/locale_text_lookup.hpp"
#include "bsp/xlive_manager_runtime.hpp"
#include "bsp/xlive_updates.hpp"

namespace bsp {

// Binds all recovered game helpers needed by the online manager. Tables, Lua
// context conversion and CRT casing remain the LocaleTextResolver's real
// dependencies. Win32XLiveUpdatePlatform and XLiveLibrary supply the real APIs.
// Construction has no effects; launch is reached only by an actual update call.
class RecoveredXLiveGameServices final : public XLiveGameServices {
public:
    RecoveredXLiveGameServices(const LocaleTextResolver&,
        XLiveUpdateInformationHost&, XLiveUpdatePlatformHost&,
        NativeStringStorage&) noexcept;
    std::wstring resolve_localization_00a9fad0(const NativeString&) override;
    void title_update_path_00a3ff20(NativeString&) override;
    void system_update_path_00a3fde0(NativeString&) override;
    std::wstring widen_update_path_004c5e60(const char*) override;
    void launch_update_00a3e560(const char*, const char*) override;
private:
    const LocaleTextResolver& locale_;
    XLiveUpdateInformationHost& sdk_;
    XLiveUpdatePlatformHost& platform_;
    NativeStringStorage& strings_;
};

} // namespace bsp
