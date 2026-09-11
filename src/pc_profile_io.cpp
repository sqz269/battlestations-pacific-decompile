#include "bsp/pc_profile_io.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include <stdexcept>
#include <utility>

namespace bsp {
struct PcProfileIoHost::ReaderFrame {
    GuiLua51Host lua;
    GuiLuaReader reader;
    ReaderFrame(lua_State* state, const bool& crt_sse2_conversion)
        : lua(require(state)), reader(lua, lua.globals(), crt_sse2_conversion) {}
    static lua_State& require(lua_State* state) {
        if (!state) throw std::logic_error("Native archive reader requires an open Lua owner");
        return *state;
    }
};
PcProfileIoHost::PcProfileIoHost(PcProfileIoContext context, PcStorageBackend& backend,
    StorageOperationState& operation, StorageOperationHost& ui, PcProfileIoServices services,
    ArchiveTextOutput& output, ArchiveSettingsServices& settings_services)
    : context_(context), backend_(backend), operation_(operation), storage_host_(backend, ui),
      services_(services), writer_(context.profile, context.settings, context.io, *this,
        operation, storage_host_, context.archive, output, settings_services) {
    if (operation_.manager_0109cecc != &backend_.operation())
        throw std::invalid_argument("Profile operation must use the concrete PC backend");
}
PcProfileIoHost::~PcProfileIoHost() = default;
void PcProfileIoHost::read(std::string_view name, ProfileCompletion completion) {
    request_profile_read_007ff100(context_.profile, context_.settings, context_.io,
        *this, name, std::move(completion));
}
void PcProfileIoHost::write(std::string_view name, ProfileCompletion completion, bool force) {
    request_profile_write_007fa710(context_.profile, context_.io, *this, name,
        std::move(completion), force);
}
void PcProfileIoHost::destroy_mission_progress_007fd780() { context_.scores.destroy_mission_progress_007fd780(); }
bool PcProfileIoHost::construct_mission_progress_00920e10() { return context_.scores.construct_mission_progress_00920e10(); }
bool PcProfileIoHost::xenon_state_is_two() { return services_.game.xenon_state_is_two(); }
bool PcProfileIoHost::xenon_user_selected() { return services_.game.xenon_user_selected(); }
void PcProfileIoHost::restore_selected_user_controls_008d45d0() { services_.game.restore_selected_user_controls_008d45d0(); }
bool PcProfileIoHost::storage_query_1c(std::string_view name, bool kind) { return backend_.storage_query_1c(name, kind ? 1u : 0u); }
void PcProfileIoHost::request_read_00bd3d70(std::string_view name, bool kind) { backend_.request_read_00bd3d70(name, kind ? 1u : 0u); }
void PcProfileIoHost::request_write_00bd3dc0(std::string_view name) { backend_.request_write_00bd3dc0(name); }
void PcProfileIoHost::run_storage_operation_006adb50(ProfileIoTask task) {
    drive_profile_storage_task(task, context_.profile, context_.settings, context_.io,
        *this, writer_, operation_, storage_host_);
}
void PcProfileIoHost::run_storage_operation_006adb50(ProfileCompletion completion) {
    bsp::run_storage_operation_006adb50(operation_, storage_host_, std::move(completion));
}
int PcProfileIoHost::storage_state_08() { return backend_.operation().state_08; }
void PcProfileIoHost::begin_profile_reader_004425c0() {
    profile_readers_.push_back(std::make_unique<ReaderFrame>(backend_.storage_lua_38(),
        context_.crt_sse2_conversion));
}
void PcProfileIoHost::deserialize_profile_007fdf00(ProfileResetState& profile) {
    read_profile_archive_007fdf00(profile, profile_readers_.back()->reader, context_.archive);
}
bool PcProfileIoHost::has_storage_buffer_30() { return backend_.has_storage_buffer_30(); }
void PcProfileIoHost::free_and_clear_storage_buffer_30() { backend_.free_and_clear_storage_buffer_30(); }
void PcProfileIoHost::close_storage_archive_00b65e80() { backend_.close_storage_archive_00b65e80(); }
bool PcProfileIoHost::has_manager_00f8a2fc() { return services_.game.has_manager_00f8a2fc(); }
void PcProfileIoHost::notify_manager_virtual_a0() { services_.game.notify_manager_virtual_a0(); }
void PcProfileIoHost::restore_settings_008d7a50(ProfileCompletion completion) {
    restore_profile_settings_008d7a50(context_.settings, restore_state_, *this, std::move(completion));
}
void PcProfileIoHost::apply_settings_008d5b50() {
    apply_all_settings_008d5b50(context_.settings, services_.languages,
        services_.apply_environment, services_.apply_latches, services_.apply);
}
void PcProfileIoHost::commit_profile_007fae70(ProfileResetState& profile) {
    bsp::commit_profile_007fae70(profile, context_.settings, commit_state_, services_.commit);
}
void PcProfileIoHost::destroy_profile_reader_00441a20() { profile_readers_.pop_back(); }
void PcProfileIoHost::reset_storage_operation_00bd3450() { backend_.reset_storage_operation_00bd3450(); }
bool PcProfileIoHost::storage_available_21() { return backend_.storage_available_21(); }
bool PcProfileIoHost::game_present() { return services_.game.game_present(); }
bool PcProfileIoHost::profile_has_save_name_007f8ca0() { return !context_.profile.save_name_34.empty(); }
std::string PcProfileIoHost::copy_profile_save_name_00425f40() { return context_.profile.save_name_34; }
void PcProfileIoHost::immediate_read_00bd4380(std::string_view name, bool kind) { backend_.immediate_read_00bd4380(name, kind ? 1u : 0u); }
void PcProfileIoHost::begin_settings_reader_004425c0() {
    settings_readers_.push_back(std::make_unique<ReaderFrame>(backend_.storage_lua_38(),
        context_.crt_sse2_conversion));
}
void PcProfileIoHost::deserialize_settings_008d6dc0(GameSettingsBlock& settings) {
    read_settings_archive_008d6dc0(settings, settings_readers_.back()->reader,
        context_.input, services_.keyboard, services_.archive_read);
}
void PcProfileIoHost::destroy_settings_reader_00441a20() { settings_readers_.pop_back(); }
}
