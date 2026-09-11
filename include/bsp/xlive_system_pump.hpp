#pragma once

#include "bsp/audio_online_startup.hpp"
#include "bsp/frame_clock.hpp"
#include "bsp/native_string.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/xlive_types.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>

namespace bsp {

struct XLiveAchievement {
    std::uint32_t user;
    std::uint32_t id;
};
static_assert(sizeof(XLiveAchievement) == 8);
using XLiveStorageDownloadResults = std::array<std::uint32_t, 5>;

// Additional manager fields used by the pump. This is a typed projection, not
// a native-layout manager or a reconstruction of its constructor defaults.
// Keep this object and PlatformManagerFlags at fixed addresses while any DLL
// operation is pending. Buffer replacement follows the native state machine,
// including forced achievement submission replacing a pending batch.
struct XLiveSystemPumpState {
    std::uint32_t field_28{};
    bool signin_flag_120{};
    std::uint32_t storage_state_12c{}; // distinct from OnlineSystemState::state (+3B0)
    XLiveOverlapped storage_overlapped_130;
    std::unique_ptr<std::uint8_t[]> storage_buffer_14c;
    std::uint32_t storage_buffer_bytes_150{};
    std::array<wchar_t, 256> storage_path_154{};
    std::uint32_t storage_result_354{};
    std::uint32_t drop_value_358{};
    std::uint32_t drop_value_35c{};
    XLiveStorageDownloadResults download_results_370{};
    XLiveOverlapped achievements_overlapped_384;
    std::unique_ptr<XLiveAchievement[]> achievements_3a0;
    std::uint32_t achievement_count_3a4{};
    std::uint32_t achievement_result_3a8{};
    bool signin_flag_3bc{};
    std::uint32_t message_choice_3dc{};
    std::uint32_t field_3e4{};
};

// Process-wide native globals, shared between manager instances.
struct XLivePumpHeartbeat {
    bool first_sample_e0e3ec{true}; // image byte = 1
    float last_seconds_f8abfc{};
};

// Required external operations, not successful defaults or SDK emulation.
// uint32_t results preserve all Win32/XLive error bits. DLL methods have the
// native stdcall argument order; references project their pointer arguments.
class XLiveSystemPumpHost {
public:
    virtual ~XLiveSystemPumpHost() = default;
    virtual void drain_notifications_00a40110() = 0;
    // Reload the clock singleton on EACH call, including the discarded sample.
    virtual ClockTimestamp sample_clock_vslot20() = 0;
    // Exact indexed native load, not XUserGetSigninState. Index 0 is +8C;
    // index 1 begins the username at +90 (there is no array of user states).
    virtual std::uint32_t read_manager_dword_8c_indexed(std::uint32_t index) = 0;
    virtual std::wstring resolve_localization_00a9fad0(const NativeString& key) = 0;
    virtual std::uint32_t x_storage_build_server_path(std::uint32_t user,
        std::uint32_t facility, const void* item_info, std::uint32_t item_info_bytes,
        const wchar_t* item, wchar_t* path, std::uint32_t& path_bytes) = 0;
    virtual std::uint32_t x_storage_download_to_memory(std::uint32_t user,
        const wchar_t* path, std::uint32_t bytes, void* buffer,
        std::uint32_t result_bytes, XLiveStorageDownloadResults& results,
        XLiveOverlapped& overlapped) = 0;
    virtual std::uint32_t x_storage_upload_from_memory(std::uint32_t user,
        const wchar_t* path, std::uint32_t bytes, const void* buffer,
        XLiveOverlapped& overlapped) = 0;
    // The progress output is unconsumed four-byte storage. Its numeric type
    // and the types of the two null-only outputs are not established here.
    virtual std::uint32_t x_storage_download_progress(XLiveOverlapped& overlapped,
        std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) = 0;
    virtual std::uint32_t x_storage_upload_progress(XLiveOverlapped& overlapped,
        std::uint32_t& progress, std::uint32_t* total, std::uint32_t* transferred) = 0;
    virtual std::uint32_t x_get_overlapped_result(XLiveOverlapped& overlapped,
        std::uint32_t* result, bool wait) = 0;
    virtual std::uint32_t x_get_overlapped_extended_error(XLiveOverlapped& overlapped) = 0;
    virtual std::uint32_t x_user_write_achievements(std::uint32_t count,
        const XLiveAchievement* achievements, XLiveOverlapped& overlapped) = 0;
    virtual std::uint32_t x_show_message_box_ui(std::uint32_t user,
        const wchar_t* title, const wchar_t* text, std::uint32_t button_count,
        const wchar_t* const* buttons, std::uint32_t focus, std::uint32_t flags,
        std::uint32_t& choice, XLiveOverlapped& overlapped) = 0;
    virtual std::uint32_t x_show_signin_ui(std::uint32_t users, std::uint32_t flags) = 0;
    // Adapter owns a full SDK XUSER_SIGNIN_INFO; only byte +8 is consumed here.
    virtual std::uint32_t x_user_get_signin_info_flags(std::uint32_t user,
        std::uint32_t flags, std::uint8_t& flags_byte_08) = 0;
};

struct XLiveSystemPumpContext {
    OnlineSystemState& online;
    PlatformManagerFlags& flags;
    XLiveSystemPumpState& pump;
    XLivePumpHeartbeat& heartbeat;
    XLiveSystemPumpHost& host;
    XLiveStartupHost& startup_host;
    NativeStringStorage& strings;
};

// Original ABIs: ECX=manager, RET, except A3FA70 takes a byte-sized force
// argument in a DWORD stack slot and RET 4. These are typed C++ interfaces.
void reset_online_ui_slots_00a3e700(OnlineSystemState& online) noexcept;
void build_online_storage_path_00a3ed10(XLiveSystemPumpContext& context);
void download_online_storage_00a3ed60(XLiveSystemPumpContext& context);
void upload_online_storage_00a3ef20(XLiveSystemPumpContext& context);
void pump_online_achievements_00a3fa70(XLiveSystemPumpContext& context, bool force);
void pump_online_signin_ui_00a40510(XLiveSystemPumpContext& context);
void pump_xlive_system_00a409f0(XLiveSystemPumpContext& context);

} // namespace bsp
