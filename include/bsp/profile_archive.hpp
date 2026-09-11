#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/profile_reset.hpp"

namespace bsp {

// 007fdf00 receives the same 004425c0 Lua reader already reconstructed for
// GUI loading. Its virtual +0Ch and +10h consume destinations, whereas the
// separate writer at 007f9540 passes values. Neither is a bidirectional visitor.
using ProfileArchiveReader = GuiLuaReader;

// Native writer virtuals +4h, +8h, +0Ch. Keys reuse the reader's name/index
// pair and values reuse the settings writer's tags 0=string, 1=int, 3=bool.
// The backend must actually write these values to its selected archive.
struct ProfileArchiveWriter {
    virtual ~ProfileArchiveWriter() = default;
    virtual void begin_section(const char* name) = 0;
    virtual void end_section() = 0;
    virtual void write_field(const GuiLuaVariant& key, const SettingsValue& value) = 0;
};

// Specific calls outside the two profile traversal bodies. The +64h native
// score object contains more than the existing mission_completion projection;
// its real storage, archive format and allocation cannot be synthesized here.
struct ProfileArchiveHost {
    virtual ~ProfileArchiveHost() = default;
    virtual int hints_owner_field_08_004c1e90() = 0;
    // Includes the caller's free (00bf65ac) of the destroyed 24h allocation.
    virtual void destroy_mission_progress_007fd780() = 0;
    virtual bool construct_mission_progress_00920e10() = 0;
    // Keep unlock_state.mission_completion synchronized with the chosen score
    // storage. Called even after allocation failure, just like the native body.
    virtual void read_mission_progress_00920000(
        ProfileResetState& profile, ProfileArchiveReader& reader) = 0;
    virtual void write_mission_progress_0090cb40(
        const ProfileResetState& profile, ProfileArchiveWriter& writer) = 0;
    // Sum the distinct +18h counter tree, not the projected completion map.
    virtual int sum_mission_progress_0090bf50() = 0;
    // Singleton 00425c20 followed by 004374f0. That manager can repopulate the
    // +94h records cleared immediately before this call; its body is external.
    virtual void refresh_profile_manager_004374f0(ProfileResetState& profile) = 0;
};

// Both original routines: __thiscall(ECX=game+650h, archive*), RET 4.
// Full normal-flow field/section traversal; new C++ interfaces, not native ABI.
// See docs/GAME_PROFILE_ARCHIVE.md for compatibility rules and boundaries.
void read_profile_archive_007fdf00(
    ProfileResetState& profile, ProfileArchiveReader& reader, ProfileArchiveHost& host);
void write_profile_archive_007f9540(
    ProfileResetState& profile, ProfileArchiveWriter& writer, ProfileArchiveHost& host);

} // namespace bsp
