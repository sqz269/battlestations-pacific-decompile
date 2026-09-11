#include "bsp/profile_write.hpp"

#include <utility>

namespace bsp {
namespace {
void deliver_completion(ProfileIoState& io)
{
    auto completion = std::exchange(io.completion, {});
    if (completion) completion();
}
}

void clear_profile_save_name_007f8f00(ProfileResetState& profile)
{
    profile.save_name_34.clear();
}

void complete_profile_storage_write_007fa670(ProfileResetState& profile,
    const GameSettingsBlock& settings, ProfileIoState& io, ProfileWriteHost& host)
{
    if (host.storage_state_08() == 1) {
        clear_profile_save_name_007f8f00(profile);
        deliver_completion(io);
        return;
    }
    {
        SettingsWriter& writer = host.begin_settings_writer_00ce4104();
        struct Retire {
            ProfileWriteHost& host;
            SettingsWriter& writer;
            ~Retire() { host.end_settings_writer_00ce3784(writer); }
        } retire{host, writer};
        write_settings_008d64a0(settings, writer);
    }
    host.register_task_006adb50(ProfileIoTask::SettingsWriteCompleted);
}

void complete_profile_settings_write_007fa220(ProfileResetState& profile,
    ProfileIoState& io, ProfileWriteHost& host)
{
    if (host.storage_state_08() == 1) {
        clear_profile_save_name_007f8f00(profile);
        deliver_completion(io);
        return;
    }
    {
        ProfileArchiveWriter& writer = host.begin_profile_writer_00ce4104();
        struct Retire {
            ProfileWriteHost& host;
            ProfileArchiveWriter& writer;
            ~Retire() { host.end_profile_writer_00ce3784(writer); }
        } retire{host, writer};
        write_profile_archive_007f9540(profile, writer, host.archive_services());
    }
    host.register_task_006adb50(ProfileIoTask::ProfileWriteCompleted);
}

void complete_profile_archive_write_007f9500(ProfileResetState& profile,
    ProfileIoState& io, ProfileWriteHost& host)
{
    if (host.storage_state_08() == 1) clear_profile_save_name_007f8f00(profile);
    deliver_completion(io);
}

void dispatch_profile_io_task(ProfileIoTask task, ProfileResetState& profile,
    GameSettingsBlock& settings, ProfileIoState& io, ProfileIoHost& read_host,
    ProfileWriteHost& write_host)
{
    switch (task) {
    case ProfileIoTask::ReadCompleted:
        complete_profile_read_007fefe0(profile, settings, io, read_host);
        break;
    case ProfileIoTask::WriteCompleted:
        complete_profile_storage_write_007fa670(profile, settings, io, write_host);
        break;
    case ProfileIoTask::SettingsWriteCompleted:
        complete_profile_settings_write_007fa220(profile, io, write_host);
        break;
    case ProfileIoTask::ProfileWriteCompleted:
        complete_profile_archive_write_007f9500(profile, io, write_host);
        break;
    }
}

} // namespace bsp
