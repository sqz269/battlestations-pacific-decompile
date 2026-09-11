#pragma once
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/profile_commit.hpp"
#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {
struct ProfileSceneSelectionHost {
    virtual ~ProfileSceneSelectionHost() = default;
    virtual void select_scene_record_004c6890(std::int32_t index) = 0;
};
// Connects profile content commit to actual VFS scripts, override execution and
// Lua ownership. Platform DLC management and scene selection retain their real
// required hosts. All referenced services outlive this object and its Lua state.
class VfsProfileDlcHost final : public ProfileCommitHost {
public:
    VfsProfileDlcHost(VfsLuaScriptFiles&, LuaScriptRuntime&, const LuaRuntimeGlobals&,
        SettingsDownloadedContentHost&, ProfileSceneSelectionHost&) noexcept;
    bool has_download_manager_00f8a304() override;
    bool has_xenon_manager_00f8abe8() override;
    bool xenon_has_selected_user_00a3e510() override;
    void try_construct_download_manager_009955f0(std::uint32_t) override;
    bool download_manager_ready_1c() override;
    void refresh_download_manager_virtual_24() override;
    void filter_downloaded_content_0043ea80(std::vector<std::string>&) override;
    void apply_downloaded_content_virtual_14(std::vector<std::string>&) override;
    void finish_download_manager_virtual_20() override;
    void register_localization_table_00aa0d30(std::string_view) override;
    void reload_localization_tables_00aa06d0(bool) override;
    void construct_dlc_lua_owner_00b66bd0() override;
    void open_dlc_lua_owner_00b6a020(std::uint32_t mask) override;
    void run_dlc_script_with_overrides_00b69d40(std::string_view, std::uint32_t flags) override;
    GuiLuaHost& dlc_lua() override;
    void select_scene_record_004c6890(std::int32_t index) override;
    void close_dlc_lua_owner_00b669a0() override;
private:
    VfsLuaScriptFiles& files_;
    LuaScriptRuntime& runtime_;
    const LuaRuntimeGlobals& globals_;
    SettingsDownloadedContentHost& content_;
    ProfileSceneSelectionHost& scene_;
    bool constructed_{};
    std::unique_ptr<PcStorageLuaOwner> owner_;
    std::unique_ptr<GuiLua51Host> reader_host_;
};
}
