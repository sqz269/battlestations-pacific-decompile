#include "bsp/profile_persistence.hpp"
#include <stdexcept>

namespace bsp {
void MissionProgressOwner::destroy_mission_progress_007fd780() noexcept { progress_.reset(); }
bool MissionProgressOwner::construct_mission_progress_00920e10() noexcept {
    try { progress_ = std::make_unique<MissionProgress>(); return true; }
    catch (const std::bad_alloc&) { progress_.reset(); return false; }
}
MissionProgress& MissionProgressOwner::get() {
    if (!progress_) throw std::logic_error("Mission-progress storage is null");
    return *progress_;
}
const MissionProgress& MissionProgressOwner::get() const {
    if (!progress_) throw std::logic_error("Mission-progress storage is null");
    return *progress_;
}
MissionProfileArchiveHost::MissionProfileArchiveHost(MissionProgressOwner& progress,
    ProfileArchiveManagerServices& managers) noexcept : progress_(progress), managers_(managers) {}
int MissionProfileArchiveHost::hints_owner_field_08_004c1e90() {
    return managers_.hints_owner_field_08_004c1e90();
}
void MissionProfileArchiveHost::destroy_mission_progress_007fd780() {
    progress_.destroy_mission_progress_007fd780();
}
bool MissionProfileArchiveHost::construct_mission_progress_00920e10() {
    return progress_.construct_mission_progress_00920e10();
}
void MissionProfileArchiveHost::read_mission_progress_00920000(ProfileResetState& profile,
    ProfileArchiveReader& reader) {
    bsp::read_mission_progress_00920000(progress_.get(), reader, profile.version_e8);
    sync_mission_completion(progress_.get(), profile.unlock_state);
}
void MissionProfileArchiveHost::write_mission_progress_0090cb40(
    const ProfileResetState&, ProfileArchiveWriter& writer) {
    bsp::write_mission_progress_0090cb40(progress_.get(), writer);
}
int MissionProfileArchiveHost::sum_mission_progress_0090bf50() {
    return bsp::sum_mission_progress_0090bf50(progress_.get());
}
void MissionProfileArchiveHost::refresh_profile_manager_004374f0(ProfileResetState& profile) {
    managers_.refresh_profile_manager_004374f0(profile);
}
void drive_profile_storage_task(ProfileIoTask task, ProfileResetState& profile,
    GameSettingsBlock& settings, ProfileIoState& io, ProfileIoHost& read_host,
    ProfileWriteHost& write_host, StorageOperationState& operation, StorageOperationHost& host) {
    bsp::run_storage_operation_006adb50(operation, host,
        [task, &profile, &settings, &io, &read_host, &write_host] {
            dispatch_profile_io_task(task, profile, settings, io, read_host, write_host);
        });
}
TextProfileWriteHost::TextProfileWriteHost(ProfileResetState& profile,
    GameSettingsBlock& settings, ProfileIoState& io, ProfileIoHost& read_host,
    StorageOperationState& operation, StorageOperationHost& storage_host,
    ProfileArchiveHost& archive_host, ArchiveTextOutput& output,
    ArchiveSettingsServices& settings_services) noexcept
    : profile_(profile), settings_(settings), io_(io), read_host_(read_host),
      operation_(operation), storage_host_(storage_host), archive_host_(archive_host),
      output_(output), settings_services_(settings_services) {}
int TextProfileWriteHost::storage_state_08() {
    if (!operation_.manager_0109cecc) throw std::logic_error("Storage manager is null");
    return operation_.manager_0109cecc->state_08;
}
void TextProfileWriteHost::run_storage_operation_006adb50(ProfileIoTask task) {
    drive_profile_storage_task(task, profile_, settings_, io_, read_host_, *this,
        operation_, storage_host_);
}
ArchiveTextWriter& TextProfileWriteHost::begin_writer() {
    if (writer_) throw std::logic_error("Previous profile writer has not been retired");
    writer_ = std::make_unique<ArchiveTextWriter>(output_, &settings_services_);
    return *writer_;
}
SettingsWriter& TextProfileWriteHost::begin_settings_writer_00ce4104() { return begin_writer(); }
void TextProfileWriteHost::end_settings_writer_00ce3784(SettingsWriter&) noexcept { writer_.reset(); }
ProfileArchiveWriter& TextProfileWriteHost::begin_profile_writer_00ce4104() { return begin_writer(); }
void TextProfileWriteHost::end_profile_writer_00ce3784(ProfileArchiveWriter&) noexcept { writer_.reset(); }
ProfileArchiveHost& TextProfileWriteHost::archive_services() { return archive_host_; }
} // namespace bsp
