#pragma once

#include "bsp/platform_loop.hpp"
#include "bsp/xlive_notifications.hpp"

#include <memory>
#include <string>

namespace bsp {

// Loads a caller-selected Win32 DLL and calls the original imported ordinals.
// No SDK implementation, identity substitution, initialization, or account work
// is hidden in this adapter. All objects must belong to this library instance.
class XLiveLibrary final : public XLiveNotificationLibrary {
public:
    explicit XLiveLibrary(const std::wstring& absolute_dll_path);
    ~XLiveLibrary() override;
    XLiveLibrary(const XLiveLibrary&) = delete;
    XLiveLibrary& operator=(const XLiveLibrary&) = delete;

    // IAT00CE25DC, thunk00C2F1D2, ordinal5030; BOOL __stdcall(MSG*).
    bool pretranslate(MSG& message);
    // Borrowed HMODULE for the separately audited SDK forwarding adapter.
    // The library must outlive that adapter and every pending SDK operation.
    void* module_handle() const noexcept;
    void* notify_create_listener(std::uint64_t areas) override;
    bool notify_get_next(void*, std::uint32_t, std::uint32_t&, std::uint32_t&) override;
    XLiveAcceptedInvite invite_get_accepted_info(std::uint32_t) override;
    std::int32_t update_system(const wchar_t*) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp
