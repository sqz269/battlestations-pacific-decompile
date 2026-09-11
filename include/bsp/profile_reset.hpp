#pragma once

#include "bsp/game_settings.hpp"
#include "bsp/profile_unlock.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// Addresses and original ABI: docs/GAME_PROFILE_RESET.md. These are host
// projections, not native layouts. Names are hypotheses unless tied to save keys.
struct ProfileLobbyFilters {
    std::vector<std::uint8_t> entries_14;
    int player_count_20{};
    int free_slots_24{};
    int latency_28{};
    int array_size_2c{};
};

// The +94h tree's values have three scalar words and one owned string after
// the key (007f89f0). Only clearing this collection is recovered here; vector
// order does not assert a native comparator or interpret the scalar words.
struct ProfileTransientRecord94 {
    std::string key;
    std::array<std::uint32_t, 3> words{};
    std::string text;
};

struct ProfileResetState {
    ProfileUnlockState unlock_state;
    std::vector<std::string> string_list_08;
    ProfileLobbyFilters lobby_filters;
    int score_total_30{}; // sum of mission-progress +18h counters after load
    std::string save_name_34;
    std::string player_name_3c;
    std::uint64_t xuid_48{}; // reset does not write this field
    std::string display_name_50;
    bool voice_59{};
    int difficulty_5c{};
    int selected_difficulty_60{};
    bool mission_progress_present{}; // native +64h pointer; allocation may fail
    std::string selected_mission_id_68;
    UnlockNameSet seen_unlocks_88;
    std::vector<ProfileTransientRecord94> transient_records_94;
    std::vector<std::string> bonus_ac;
    std::uint64_t content_mask_d8{};
    int japan_nose_art_e0{};
    int allied_nose_art_e4{};
    std::uint32_t version_e8{}; // reset does not write this field
    int drop_rate_tc_ec{};
    int drop_rate_dc_f0{};
};

// A +64h object owns a large score-record tree and two counter trees. The
// existing unlock projection carries its completion values; the other score
// data, native allocation and native destructors remain an explicit boundary.
// Implementations must manage the real chosen score storage, with no invented
// successful allocation or synthesized save contents.
struct ProfileResetHost {
    virtual ~ProfileResetHost() = default;
    virtual void destroy_mission_progress_007fd780() = 0;
    virtual bool construct_mission_progress_00920e10() = 0;
    virtual bool xenon_state_is_two() = 0;
    virtual bool xenon_user_selected() = 0;
    virtual void restore_selected_user_controls_008d45d0() = 0;
};

// 007fdb20: ECX=game+650h, no stack arguments, RET. Only the listed fields
// are projected; notably XUID, version, byte+58h and vector+BCh are untouched.
void reset_profile_007fdb20(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileResetHost& host);

// Both setters mirror the display name when its header is nonempty, otherwise
// the player name, into game+1FF0h (at most 31 bytes). Reset itself deliberately
// does not update that buffer. An embedded NUL ends the copied C-string prefix.
void set_profile_name_007f9290(
    ProfileResetState& profile, std::array<char, 32>& game_name, std::string_view name);
void set_profile_display_name_007f9340(
    ProfileResetState& profile, std::array<char, 32>& game_name, std::string_view name);

using ProfileCompletion = std::function<void()>;
struct ProfileIoState {
    ProfileCompletion completion; // 00f87458; one shared callback slot
};

enum class ProfileIoTask : std::uint32_t {
    ReadCompleted = 0x007fefe0,
    WriteCompleted = 0x007fa670,
    SettingsWriteCompleted = 0x007fa220,
    ProfileWriteCompleted = 0x007f9500,
};

struct ProfileIoHost : ProfileResetHost {
    // The raw predicates and manager requests retain their call-site meaning.
    // 007ff100 is the read/restore route, despite older press-start hook names.
    virtual bool storage_query_1c(std::string_view name, bool flag) = 0;
    virtual void request_read_00bd3d70(std::string_view name, bool flag) = 0;
    virtual void request_write_00bd3dc0(std::string_view name) = 0;
    virtual void register_task_006adb50(ProfileIoTask task) = 0;
    virtual int storage_state_08() = 0;

    // 007fefe0's successful arm. Reader/archive, serializer, native buffer
    // ownership and settings backend remain required external implementations.
    virtual void begin_profile_reader_004425c0() = 0;
    virtual void deserialize_profile_007fdf00(ProfileResetState& profile) = 0;
    virtual bool has_storage_buffer_30() = 0;
    virtual void free_and_clear_storage_buffer_30() = 0;
    virtual void close_storage_archive_00b65e80() = 0;
    virtual bool has_manager_00f8a2fc() = 0;
    virtual void notify_manager_virtual_a0() = 0;
    virtual void restore_settings_008d7a50(ProfileCompletion completion) = 0;
    virtual void apply_settings_008d5b50() = 0;
    virtual void commit_profile_007fae70(ProfileResetState& profile) = 0;
    virtual void destroy_profile_reader_00441a20() = 0;
};

// 007ff100: __thiscall(profile, NativeString* name, void (*callback)()), RET 8.
// False query result resets immediately and invokes the argument callback;
// true stores it globally, starts the read, and schedules 007fefe0.
void request_profile_read_007ff100(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileIoState& io,
    ProfileIoHost& host, std::string_view name, ProfileCompletion completion);

// 007fefe0: no arguments, RET. Storage state exactly 1 discards; every other
// state enters the reader. The callback slot is cleared before either delivery.
void complete_profile_read_007fefe0(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileIoState& io,
    ProfileIoHost& host);

// 007fa710: __thiscall(profile, NativeString* name, callback, char force), RET C.
// The native name comparison distinguishes empty/nonempty headers, then compares
// nonempty names case-insensitively only up to NUL. Even when unchanged, it
// assigns the new spelling and callback slot without queuing a write.
void request_profile_write_007fa710(
    ProfileResetState& profile, ProfileIoState& io, ProfileIoHost& host,
    std::string_view name, ProfileCompletion completion, bool force);

} // namespace bsp
