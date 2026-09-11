#pragma once

#include "bsp/archive_text_writer.hpp"
#include "bsp/mission_progress.hpp"
#include "bsp/profile_write.hpp"
#include "bsp/storage_operation.hpp"
#include <memory>

namespace bsp {

// Host composition of recovered routines; no new native addresses or ABI.
// Keep this storage shared between profile-reset and archive-reader hosts.
class MissionProgressOwner {
public:
    void destroy_mission_progress_007fd780() noexcept;
    bool construct_mission_progress_00920e10() noexcept;
    MissionProgress& get();
    const MissionProgress& get() const;
private:
    std::unique_ptr<MissionProgress> progress_;
};

// The two remaining manager effects require an actual implementation.
struct ProfileArchiveManagerServices {
    virtual ~ProfileArchiveManagerServices() = default;
    virtual int hints_owner_field_08_004c1e90() = 0;
    virtual void refresh_profile_manager_004374f0(ProfileResetState&) = 0;
};

class MissionProfileArchiveHost final : public ProfileArchiveHost {
public:
    MissionProfileArchiveHost(MissionProgressOwner&, ProfileArchiveManagerServices&) noexcept;
    int hints_owner_field_08_004c1e90() override;
    void destroy_mission_progress_007fd780() override;
    bool construct_mission_progress_00920e10() override;
    void read_mission_progress_00920000(ProfileResetState&, ProfileArchiveReader&) override;
    void write_mission_progress_0090cb40(const ProfileResetState&, ProfileArchiveWriter&) override;
    int sum_mission_progress_0090bf50() override;
    void refresh_profile_manager_004374f0(ProfileResetState&) override;
private:
    MissionProgressOwner& progress_;
    ProfileArchiveManagerServices& managers_;
};

// All referenced objects must outlive a retained interactive continuation.
// ProfileIoHost's matching method can delegate here with the same references.
void drive_profile_storage_task(ProfileIoTask, ProfileResetState&, GameSettingsBlock&,
    ProfileIoState&, ProfileIoHost&, ProfileWriteHost&, StorageOperationState&,
    StorageOperationHost&);

// Real archive bytes plus the recovered synchronous/interactive storage driver.
// Storage progress, rendering, keyboard serialization and manager effects remain
// required services. No operation completion or persistence success is fabricated.
class TextProfileWriteHost final : public ProfileWriteHost {
public:
    TextProfileWriteHost(ProfileResetState&, GameSettingsBlock&, ProfileIoState&,
        ProfileIoHost&, StorageOperationState&, StorageOperationHost&,
        ProfileArchiveHost&, ArchiveTextOutput&, ArchiveSettingsServices&) noexcept;
    int storage_state_08() override;
    void run_storage_operation_006adb50(ProfileIoTask) override;
    SettingsWriter& begin_settings_writer_00ce4104() override;
    void end_settings_writer_00ce3784(SettingsWriter&) noexcept override;
    ProfileArchiveWriter& begin_profile_writer_00ce4104() override;
    void end_profile_writer_00ce3784(ProfileArchiveWriter&) noexcept override;
    ProfileArchiveHost& archive_services() override;
private:
    ArchiveTextWriter& begin_writer();
    ProfileResetState& profile_;
    GameSettingsBlock& settings_;
    ProfileIoState& io_;
    ProfileIoHost& read_host_;
    StorageOperationState& operation_;
    StorageOperationHost& storage_host_;
    ProfileArchiveHost& archive_host_;
    ArchiveTextOutput& output_;
    ArchiveSettingsServices& settings_services_;
    std::unique_ptr<ArchiveTextWriter> writer_;
};

} // namespace bsp
