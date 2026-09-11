#pragma once

#include "bsp/profile_archive.hpp"

namespace bsp {

// The native callbacks construct a temporary8h store with vtable00ce4104
// and word+4=0, then replace its vtable with00ce3784 before queuing the next
// callback. This service provides real archive backends with that lifetime;
// it must not synthesize successful persistence. See docs/GAME_PROFILE_WRITE.md.
struct ProfileWriteHost {
    virtual ~ProfileWriteHost() = default;
    virtual int storage_state_08() = 0;
    virtual void register_task_006adb50(ProfileIoTask task) = 0;
    virtual SettingsWriter& begin_settings_writer_00ce4104() = 0;
    virtual void end_settings_writer_00ce3784(SettingsWriter&) noexcept = 0;
    virtual ProfileArchiveWriter& begin_profile_writer_00ce4104() = 0;
    virtual void end_profile_writer_00ce3784(ProfileArchiveWriter&) noexcept = 0;
    virtual ProfileArchiveHost& archive_services() = 0;
};

//007f8f00: ECX=profile, RET. Assign an empty native string to+34h only.
void clear_profile_save_name_007f8f00(ProfileResetState& profile);
//007fa670: no arguments, RET. State1 clears name and delivers completion;
// other states write settings and queue007fa220, retaining the callback slot.
void complete_profile_storage_write_007fa670(ProfileResetState& profile,
    const GameSettingsBlock& settings, ProfileIoState& io, ProfileWriteHost& host);
//007fa220: no arguments, RET. Same state1 rule; otherwise write profile and
// queue007f9500. The backend writer is retired before the scheduling call.
void complete_profile_settings_write_007fa220(ProfileResetState& profile,
    ProfileIoState& io, ProfileWriteHost& host);
//007f9500: no arguments, RET or tail JMP callback. State1 clears name;
// all states take and clear the shared callback slot before optional delivery.
void complete_profile_archive_write_007f9500(ProfileResetState& profile,
    ProfileIoState& io, ProfileWriteHost& host);

// Host dispatch for the four recovered queued addresses. This is composition,
// not the native scheduler006adb50, whose implementation remains required.
void dispatch_profile_io_task(ProfileIoTask task, ProfileResetState& profile,
    GameSettingsBlock& settings, ProfileIoState& io, ProfileIoHost& read_host,
    ProfileWriteHost& write_host);

} // namespace bsp
