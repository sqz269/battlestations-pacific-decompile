#include "bsp/profile_dlc_runtime.hpp"
#include <stdexcept>

namespace bsp {
VfsProfileDlcHost::VfsProfileDlcHost(VfsLuaScriptFiles& files, LuaScriptRuntime& runtime,
    const LuaRuntimeGlobals& globals, SettingsDownloadedContentHost& content,
    ProfileSceneSelectionHost& scene) noexcept
    : files_(files), runtime_(runtime), globals_(globals), content_(content), scene_(scene) {}
bool VfsProfileDlcHost::has_download_manager_00f8a304() { return content_.has_download_manager_00f8a304(); }
bool VfsProfileDlcHost::has_xenon_manager_00f8abe8() { return content_.has_xenon_manager_00f8abe8(); }
bool VfsProfileDlcHost::xenon_has_selected_user_00a3e510() { return content_.xenon_has_selected_user_00a3e510(); }
void VfsProfileDlcHost::try_construct_download_manager_009955f0(std::uint32_t bytes) { content_.try_construct_download_manager_009955f0(bytes); }
bool VfsProfileDlcHost::download_manager_ready_1c() { return content_.download_manager_ready_1c(); }
void VfsProfileDlcHost::refresh_download_manager_virtual_24() { content_.refresh_download_manager_virtual_24(); }
void VfsProfileDlcHost::filter_downloaded_content_0043ea80(std::vector<std::string>& names) { content_.filter_downloaded_content_0043ea80(names); }
void VfsProfileDlcHost::apply_downloaded_content_virtual_14(std::vector<std::string>& names) { content_.apply_downloaded_content_virtual_14(names); }
void VfsProfileDlcHost::finish_download_manager_virtual_20() { content_.finish_download_manager_virtual_20(); }
void VfsProfileDlcHost::register_localization_table_00aa0d30(std::string_view name) { content_.register_localization_table_00aa0d30(name); }
void VfsProfileDlcHost::reload_localization_tables_00aa06d0(bool flag) { content_.reload_localization_tables_00aa06d0(flag); }
void VfsProfileDlcHost::construct_dlc_lua_owner_00b66bd0() {
    if (constructed_) throw std::logic_error("DLC Lua owner is already constructed");
    constructed_ = true;
}
void VfsProfileDlcHost::open_dlc_lua_owner_00b6a020(std::uint32_t mask) {
    if (!constructed_ || owner_) throw std::logic_error("DLC Lua owner is not ready for open");
    owner_ = std::make_unique<PcStorageLuaOwner>(files_.owner_environment(runtime_, globals_));
    owner_->open_storage_archive_00b6a020(mask);
    reader_host_ = std::make_unique<GuiLua51Host>(*owner_->storage_lua_38());
}
void VfsProfileDlcHost::run_dlc_script_with_overrides_00b69d40(std::string_view path, std::uint32_t flags) {
    if (!owner_) throw std::logic_error("DLC Lua owner is not open");
    runtime_.run_file(owner_->storage_lua_38(), std::string(path), static_cast<std::uint8_t>(flags) != 0);
}
GuiLuaHost& VfsProfileDlcHost::dlc_lua() {
    if (!reader_host_) throw std::logic_error("DLC reader host is not open");
    return *reader_host_;
}
void VfsProfileDlcHost::select_scene_record_004c6890(std::int32_t index) { scene_.select_scene_record_004c6890(index); }
void VfsProfileDlcHost::close_dlc_lua_owner_00b669a0() {
    reader_host_.reset();
    owner_.reset();
    constructed_ = false;
}
}
