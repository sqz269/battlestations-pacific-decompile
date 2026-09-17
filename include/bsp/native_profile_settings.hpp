#pragma once
#include "bsp/native_profile_collections.hpp"
#include <cstdint>

namespace bsp {
class XLiveLibrary;
struct NativeOnlineManagerStorage;
// A4D4A0 -> IATCE2624 -> xlive.dll ordinal5331. Seven DWORD stack slots,
// stdcall/RET1Ch; raw status and mutable caller-owned buffers are preserved.
using NativeReadProfileSettings = std::uint32_t (__stdcall*)(std::uint32_t title,
    std::uint32_t user,std::uint32_t count,std::uint32_t* ids,
    std::uint32_t* bytes,void* result,void* overlapped);
struct NativeProfileSettingsSdkCalls {
    virtual ~NativeProfileSettingsSdkCalls()=default;
    virtual std::uint32_t read_profile(std::uint32_t title,std::uint32_t user,
        std::uint32_t count,std::uint32_t* ids,std::uint32_t* bytes,
        void* result,void* overlapped)=0;
};
// Borrows the already loaded library/function. Construction only resolves or
// validates the import; no initialization, account work or replacement SDK.
class NativeProfileSettingsSdkRuntime final:public NativeProfileSettingsSdkCalls {
public:
    explicit NativeProfileSettingsSdkRuntime(const XLiveLibrary&);
    explicit NativeProfileSettingsSdkRuntime(NativeReadProfileSettings);
    std::uint32_t read_profile(std::uint32_t,std::uint32_t,std::uint32_t,
        std::uint32_t*,std::uint32_t*,void*,void*) override;
private:
    NativeReadProfileSettings read_;
};
struct NativeProfileSettingsContext {
    NativeOnlineManagerStorage* volatile& current_manager_00f8abe8;
    void* volatile& current_game_00e188a8;
    NativeProfileSettingsSdkCalls& sdk;
};
// 8D41C0[43]: actual raw settings receiver, ECX/RET. Eight sparse stores only.
void reset_native_profile_game_defaults_008d41c0(void*) noexcept;
// 8D4820[67]: five control-byte defaults, then CURRENT manager state28==2 and
// CURRENT selected byte119!=0 gate the full selected-user import below.
void reset_native_profile_control_defaults_008d4820(void*,NativeProfileSettingsContext&,
    NativeProfileCollectionCalls&);
// 8D45D0[220]: query size using user0, ignore status, allocate via operator-new,
// then read CURRENT manager slot11C and query again using the SAME ids/size cells.
// Success updates CURRENT game+6AC and captured settings fields, then frees.
// Nonzero second status intentionally leaves the allocation unreleased, matching
// the native normal failure branch. No native EH/SDK/ABI/gameplay equivalence.
void import_native_profile_control_settings_008d45d0(void*,NativeProfileSettingsContext&,
    NativeProfileCollectionCalls&);
} // namespace bsp
