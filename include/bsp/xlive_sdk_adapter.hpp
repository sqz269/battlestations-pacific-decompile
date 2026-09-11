#pragma once

#include "bsp/xlive_signin.hpp"
#include "bsp/xlive_system_pump.hpp"

namespace bsp {
class XLiveLibrary;

// Required SDK forwarding only. The game-facing clock, notification drain,
// localization, cached-state reads and callback operations remain abstract.
// The supplied library owns its module: it must outlive this adapter and every
// listener, pending overlap, buffer and SDK object passed through the adapter.
// Construction does not load, initialize, unload or substitute any DLL.
class XLiveSdkAdapter : public XLiveSystemPumpHost, public XLiveSigninHost {
public:
    explicit XLiveSdkAdapter(const XLiveLibrary& library);
    ~XLiveSdkAdapter() override = default;
    XLiveSdkAdapter(const XLiveSdkAdapter&) = delete;
    XLiveSdkAdapter& operator=(const XLiveSdkAdapter&) = delete;

    std::uint32_t x_storage_build_server_path(std::uint32_t user,
        std::uint32_t facility, const void* item_info, std::uint32_t item_info_bytes,
        const wchar_t* item, wchar_t* path, std::uint32_t& path_bytes) override;
    std::uint32_t x_storage_download_to_memory(std::uint32_t user,
        const wchar_t* path, std::uint32_t bytes, void* buffer,
        std::uint32_t result_bytes, XLiveStorageDownloadResults& results,
        XLiveOverlapped& overlapped) override;
    std::uint32_t x_storage_upload_from_memory(std::uint32_t user,
        const wchar_t* path, std::uint32_t bytes, const void* buffer,
        XLiveOverlapped& overlapped) override;
    std::uint32_t x_storage_download_progress(XLiveOverlapped& overlapped,
        std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) override;
    std::uint32_t x_storage_upload_progress(XLiveOverlapped& overlapped,
        std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) override;
    std::uint32_t x_get_overlapped_result(XLiveOverlapped& overlapped,
        std::uint32_t* result, bool wait) override;
    std::uint32_t x_get_overlapped_extended_error(XLiveOverlapped& overlapped) override;
    std::uint32_t x_user_write_achievements(std::uint32_t count,
        const XLiveAchievement* achievements, XLiveOverlapped& overlapped) override;
    std::uint32_t x_show_message_box_ui(std::uint32_t user,
        const wchar_t* title, const wchar_t* text, std::uint32_t button_count,
        const wchar_t* const* buttons, std::uint32_t focus, std::uint32_t flags,
        std::uint32_t& choice, XLiveOverlapped& overlapped) override;
    std::uint32_t x_show_signin_ui(std::uint32_t users, std::uint32_t flags) override;
    std::uint32_t x_user_get_signin_info_flags(std::uint32_t user,
        std::uint32_t flags, std::uint8_t& flags_byte_08) override;

    std::uint32_t user_get_signin_state(std::uint32_t user_index) override;
    std::uint32_t user_get_name(std::uint32_t user_index,
        XLiveUserName128& output, std::uint32_t capacity) override;
    std::uint32_t user_get_xuid(std::uint32_t user_index, std::uint64_t& output) override;
    std::uint32_t user_check_privilege(std::uint32_t user_index,
        std::uint32_t privilege, std::int32_t& output_bool) override;

private:
    void* module_; // borrowed HMODULE from XLiveLibrary::module_handle()
};
} // namespace bsp
