#include "bsp/profile_commit.hpp"

#include <stdexcept>
#include <utility>

namespace bsp {
namespace {

// Native compiler-generated unwinding destroys the Lua objects before their
// state owner. This guard also covers host exceptions; cleanup must not throw.
class DlcOwnerLifetime {
public:
    explicit DlcOwnerLifetime(ProfileCommitHost& host) : host_(host) {
        host_.construct_dlc_lua_owner_00b66bd0();
    }
    ~DlcOwnerLifetime() { host_.close_dlc_lua_owner_00b669a0(); }
private:
    ProfileCommitHost& host_;
};

struct LuaRefLifetime {
    GuiLuaHost& host;
    GuiLuaRef ref;
    ~LuaRefLifetime() { if (ref.valid()) host.release(ref); }
};

} // namespace

void restore_profile_settings_008d7a50(GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsRestoreHost& host,
    ProfileCompletion completion)
{
    host.reset_storage_operation_00bd3450();
    if (host.storage_available_21() && host.game_present() &&
        host.profile_has_save_name_007f8ca0()) {
        const std::string name = host.copy_profile_save_name_00425f40();
        // 00436710 constructs the empty string; 00449af0 compares its header
        // with the copied name. No case comparison is needed against empty.
        if (!name.empty() && host.storage_query_1c(name, false)) {
            state.completion = std::move(completion);
            // The native body reloads the game global after the virtual query.
            if (host.game_present()) {
                host.request_read_00bd3d70(name, false);
                host.run_storage_operation_006adb50([&settings, &state, &host] {
                    complete_profile_settings_restore_008d79a0(settings, state, host);
                });
            } else {
                host.immediate_read_00bd4380(name, false);
                complete_profile_settings_restore_008d79a0(settings, state, host);
            }
            return;
        }
    }
    if (completion) completion();
}

void complete_profile_settings_restore_008d79a0(GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsRestoreHost& host)
{
    if (host.storage_state_08() == 0) {
        host.begin_settings_reader_004425c0();
        host.deserialize_settings_008d6dc0(settings);
        if (host.has_storage_buffer_30()) host.free_and_clear_storage_buffer_30();
        host.close_storage_archive_00b65e80();
        host.destroy_settings_reader_00441a20();
    }
    auto completion = std::exchange(state.completion, {});
    if (completion) completion();
}

void refresh_settings_downloaded_content_008d5030(
    GameSettingsBlock& settings, SettingsDownloadedContentHost& host)
{
    if (!host.has_download_manager_00f8a304()) {
        if (!host.has_xenon_manager_00f8abe8()) return;
        if (host.xenon_has_selected_user_00a3e510()) {
            host.try_construct_download_manager_009955f0(0x844);
        }
        if (!host.has_download_manager_00f8a304()) return;
    }
    if (host.download_manager_ready_1c()) {
        host.refresh_download_manager_virtual_24();
        host.filter_downloaded_content_0043ea80(settings.downloaded_content);
        host.apply_downloaded_content_virtual_14(settings.downloaded_content);
        host.finish_download_manager_virtual_20();
    }
    host.register_localization_table_00aa0d30("globals");
    host.reload_localization_tables_00aa06d0(true);
}

std::uint64_t profile_content_mask_007fae70(std::string_view digits)
{
    if (digits.size() < 64) {
        throw std::invalid_argument("DLCTable values require at least 64 readable characters");
    }
    std::uint64_t mask = 0;
    for (std::size_t i = 0; i < 64; ++i) {
        if (digits[i] == '1') mask |= std::uint64_t{1} << (63 - i);
    }
    return mask;
}

void commit_profile_007fae70(ProfileResetState& profile, GameSettingsBlock& settings,
    ProfileCommitState& state, ProfileCommitHost& host)
{
    refresh_settings_downloaded_content_008d5030(settings, host);
    DlcOwnerLifetime owner(host);
    host.open_dlc_lua_owner_00b6a020(4);
    host.run_dlc_script_with_overrides_00b69d40(kProfileDlcScript, 0);
    auto& lua = host.dlc_lua();
    // Native globals temporary dies immediately after obtaining DLCTable.
    LuaRefLifetime table{lua, {}};
    {
        LuaRefLifetime globals{lua, lua.globals()};
        table.ref = lua.get_by_name(globals.ref, kProfileDlcTable);
    }
    LuaRefLifetime key{lua, {}};
    LuaRefLifetime value{lua, {}};
    state.downloaded_content_names_cc.clear();
    profile.content_mask_d8 = 0;
    for (bool more = lua.next(table.ref, key.ref, value.ref, true); more;
        more = lua.next(table.ref, key.ref, value.ref, false)) {
        const char* digits = lua.to_string(value.ref);
        if (!digits) throw std::invalid_argument("DLCTable value is not a string");
        profile.content_mask_d8 |= profile_content_mask_007fae70(digits);
        const char* name = lua.to_string(key.ref);
        if (!name) throw std::invalid_argument("DLCTable key is not a string");
        state.downloaded_content_names_cc.emplace_back(name);
        // GuiLuaHost retains its own iteration cursor, as in GuiLuaReader.
        // It does not release the caller-owned handles when next replaces them.
        lua.release(value.ref);
        value.ref = {};
        lua.release(key.ref);
        key.ref = {};
    }
    host.select_scene_record_004c6890(0);
    // Reverse local destruction: value, key, table, then the Lua state owner.
}

} // namespace bsp
