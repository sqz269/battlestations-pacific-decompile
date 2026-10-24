#pragma once

#include "bsp/lua_state_owner.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
struct LanguageEntry;
struct NativeGameSettingsStorage;
}

// cc15-prof: the signed-in profile READ, 0067CC60 -> 007FF100 -> 007FEFE0 ->
// 007FDF00 -> 00920000, run read-only into the lane-L2 profile owner
// (game+650h). Evidence, the no-write proof and the pairs:
// docs/CC15_PROFILE_LOAD.md. A host projection over the existing typed bodies,
// not a native layout.
namespace bsp::game {

class GameHostLog;
class GameProfileOwner;

// Off: the profile owner holds a fresh profile only (lane L2). On, and only
// together with kProfileOwnerCc15Bound: where the host passes the press-start
// screen, it applies the profile half of the sign-in 0067CC60 and reads the
// save through 007FF100 without any write to the save directory.
inline constexpr bool kProfileLoadCc15Bound = true;

// cc15-prof2. Off: 007FEFE0's 008D7A50 and 008D5B50 log as withheld and
// 007FDB20's settings half runs on a throwaway block, as above. On, with
// kProfileLoadCc15Bound: the sign-in's 007FDB20 settings half, then 008D7A50
// (the save's <name>\quick read through 008D6DC0) and 008D5B50 run on the live
// settings 00F88980, and 008D5B50's sinks reach the host state that exists.
// Nothing is written to the save. docs/CC15_PROFILE_LOAD.md section 6.
inline constexpr bool kQuickSettingsApplyCc15Bound = false;

// cc15-prof2. game+6B0h is profile+60h (SelectedDifficulty) of the game+650h
// profile, which 0058BF52/0058BF58 copy into game+6ACh. Off: chosen_difficulty()
// answers --difficulty, else 1. On: --difficulty, else the profile owner's +60h.
// The user and cc9 schedule this switch; docs/CC15_PROFILE_LOAD.md section 7.
// ON (cc15-prof2 flip): the harness launchers pin --difficulty 1 (docs/TOOLING.md).
inline constexpr bool kProfileDifficultyCc15Bound = true;

struct GameProfileLoadReport {
    bool queried{};            // 0109CECC vtable +1Ch(name, 1): valid and player exist
    bool read{};               // 00BD4380 read, decoded and executed the archive
    std::size_t seen_hints{};  // profile+08h (AllSeenHints), game+65Ch's list
    std::size_t mission_records{};
    std::size_t completed_records{};
    int score_total{};         // profile+30h after 0090BF50
};

// 00BEB2C0's save directory, <CSIDL_PERSONAL>\Battlestations-Pacific\save,
// built without either CreateDirectoryA. BSP_PROFILE_LOAD_ROOT replaces it
// (a copy under local\ for parser checks).
std::string read_only_save_root();

// The live host state 008D7A50 / 008D5B50 reach under kQuickSettingsApplyCc15Bound,
// bound by the startup host. Borrowed; every member must outlive the menu host.
struct GameProfileSettingsSinks {
    NativeGameSettingsStorage* settings_00f88980{};   // the raw BCh owner
    const std::vector<LanguageEntry>* languages{};    // 00F88974's catalog
    std::uint32_t pixel_shader_version_28{};          // 008D7671: renderer +104h -> +28h
    void* volatile* sound_alternate_00f8bbcc{};        // 008D5483/5490: +218h, +21Ch
    std::function<void(bool)> set_rumble_enabled_00a94c50;  // 008D5DE2
};

// 0067CC60's profile half for a sign-in with save id `save_name` ("%llx" of
// 00A3E5D0): 007FDB20 then 007FF100(save_name, 0067CA40). Only reads are
// issued; the storage operation is the immediate 00BD4380 read in place of the
// queued 00BD3D70 one, whose completion would stamp <name>\valid. `sinks` is
// used only under kQuickSettingsApplyCc15Bound.
GameProfileLoadReport load_signed_in_profile_read_only(GameProfileOwner& owner,
    GameHostLog& log, const std::string& save_root, std::string_view save_name,
    LuaStateOwnerEnvironment lua, const GameProfileSettingsSinks* sinks = nullptr);

} // namespace bsp::game
