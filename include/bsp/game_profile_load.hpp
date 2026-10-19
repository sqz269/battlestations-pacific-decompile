#pragma once

#include "bsp/lua_state_owner.hpp"

#include <cstddef>
#include <string>
#include <string_view>

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
inline constexpr bool kProfileLoadCc15Bound = false;

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

// 0067CC60's profile half for a sign-in with save id `save_name` ("%llx" of
// 00A3E5D0): 007FDB20 then 007FF100(save_name, 0067CA40). Only reads are
// issued; the storage operation is the immediate 00BD4380 read in place of the
// queued 00BD3D70 one, whose completion would stamp <name>\valid.
GameProfileLoadReport load_signed_in_profile_read_only(GameProfileOwner& owner,
    GameHostLog& log, const std::string& save_root, std::string_view save_name,
    LuaStateOwnerEnvironment lua);

} // namespace bsp::game
