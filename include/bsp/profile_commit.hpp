#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/profile_reset.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// Host projections, not original layouts or binary replacement interfaces.
// Address, ABI, assembly evidence and unresolved backends: docs/PROFILE_COMMIT.md.
struct ProfileSettingsRestoreState {
    ProfileCompletion completion; // 00f88958, distinct from profile I/O's slot
};

struct ProfileSettingsRestoreHost {
    virtual ~ProfileSettingsRestoreHost() = default;
    virtual void reset_storage_operation_00bd3450() = 0;
    virtual bool storage_available_21() = 0;
    virtual bool game_present() = 0;
    virtual bool profile_has_save_name_007f8ca0() = 0;
    virtual std::string copy_profile_save_name_00425f40() = 0;
    virtual bool storage_query_1c(std::string_view name, bool flag) = 0;
    virtual void request_read_00bd3d70(std::string_view name, bool flag) = 0;
    // The operation may invoke or retain this callback. Its referenced state,
    // settings and host must outlive deferred completion, as with native globals.
    virtual void run_storage_operation_006adb50(ProfileCompletion continuation) = 0;
    virtual void immediate_read_00bd4380(std::string_view name, bool flag) = 0;

    virtual int storage_state_08() = 0;
    // Gets storage owner+38h's globals before constructing its Lua reader.
    virtual void begin_settings_reader_004425c0() = 0;
    virtual void deserialize_settings_008d6dc0(GameSettingsBlock& settings) = 0;
    virtual bool has_storage_buffer_30() = 0;
    virtual void free_and_clear_storage_buffer_30() = 0;
    virtual void close_storage_archive_00b65e80() = 0;
    virtual void destroy_settings_reader_00441a20() = 0;
};

// 008d7a50: one stack callback, RET 4; ECX is not consumed. The immediate
// rejection path invokes its argument without changing the global pending slot.
void restore_profile_settings_008d7a50(GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsRestoreHost& host,
    ProfileCompletion completion);

// 008d79a0: no arguments, RET. Only storage state exactly zero is deserialized.
void complete_profile_settings_restore_008d79a0(GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsRestoreHost& host);

// 008d5030's content manager/platform operations remain required call sites.
// Presence must be queried again after construction: allocation or publication
// can fail. The manager's vector is separate from settings.downloaded_content.
struct SettingsDownloadedContentHost {
    virtual ~SettingsDownloadedContentHost() = default;
    virtual bool has_download_manager_00f8a304() = 0;
    virtual bool has_xenon_manager_00f8abe8() = 0;
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    virtual void try_construct_download_manager_009955f0(std::uint32_t bytes) = 0;
    virtual bool download_manager_ready_1c() = 0;
    virtual void refresh_download_manager_virtual_24() = 0;
    virtual void filter_downloaded_content_0043ea80(std::vector<std::string>& names) = 0;
    virtual void apply_downloaded_content_virtual_14(std::vector<std::string>& names) = 0;
    virtual void finish_download_manager_virtual_20() = 0;
    virtual void register_localization_table_00aa0d30(std::string_view name) = 0;
    virtual void reload_localization_tables_00aa06d0(bool flag) = 0;
};

// 008d5030: ECX=settings, no stack arguments, RET. Does not call the general
// settings apply routine; its only settings input/output is the +98h vector.
void refresh_settings_downloaded_content_008d5030(
    GameSettingsBlock& settings, SettingsDownloadedContentHost& host);

struct ProfileCommitState {
    // Native profile+CC list, header+D0/signed count+D4. This collection is
    // distinct from profile bonus+AC and settings' downloaded-content vector.
    std::vector<std::string> downloaded_content_names_cc;
};

struct ProfileCommitHost : SettingsDownloadedContentHost {
    virtual void construct_dlc_lua_owner_00b66bd0() = 0;
    virtual void open_dlc_lua_owner_00b6a020(std::uint32_t mask) = 0;
    virtual void run_dlc_script_with_overrides_00b69d40(
        std::string_view path, std::uint32_t flags) = 0;
    virtual GuiLuaHost& dlc_lua() = 0;
    virtual void select_scene_record_004c6890(std::int32_t index) = 0;
    virtual void close_dlc_lua_owner_00b669a0() = 0;
};

inline constexpr std::string_view kProfileDlcScript = "SCRIPTS/datatables/DLC.lua";
inline constexpr char kProfileDlcTable[] = "DLCTable";

// The native caller reads exactly 64 bytes, even beyond NUL; only '1' sets a
// bit. The host projection rejects shorter spans instead of reading outside
// storage. Extra bytes are ignored. First character selects bit 63.
std::uint64_t profile_content_mask_007fae70(std::string_view digits);

// 007fae70: ECX=profile, no stack arguments, RET. Commit refers to rebuilding
// content/scene state; it does not write a profile archive. The Lua environment
// must expose DLCTable with string keys and strings at least 64 bytes long.
void commit_profile_007fae70(ProfileResetState& profile, GameSettingsBlock& settings,
    ProfileCommitState& state, ProfileCommitHost& host);

} // namespace bsp
