#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// Host projection of the profile-side unlock predicate.
//
// Addresses: 007FC4C0 (one requirement string against the profile), 007FC820
// (a whole requirement vector). Evidence and native ABI:
// docs/PROFILE_UNLOCK_PREDICATE.md.
//
// The profile block lives at `game+650h` (00E188A8 + 650h). 007FC4C0 takes it
// as ECX and reads five keyed containers inside it; nothing else in the block
// takes part. All five compare through BSP_NativeString_LessCaseInsensitive
// (00443D00) or __stricmp, so every lookup here is case-insensitive.
//
// Names below are hypotheses. `Unlocks` and `SeenUnlocks` are read off the save
// keys the profile serializer 007FDF00 writes; the rest are named from their
// writers.
namespace bsp {

// The token separators 007FC4C0 hands to `strtok`: the literal at 00D08D18 is
// " ," (space, comma).
inline constexpr std::string_view kUnlockTokenSeparators = " ,";

// 007FC4C0 copies the requirement text into `char[256]` at ESP+64h with an
// unbounded `do { *dst++ = *src++; } while (c)` loop, so a requirement longer
// than 255 characters smashes the native frame. This projection does not
// reproduce that; the constant is recorded because it bounds what shipped data
// may legally carry.
inline constexpr std::size_t kUnlockExpressionBufferSize = 256;

// Offsets of the five sources inside the profile block, for cross-reading the
// listing. Add `game+650h` to reach the absolute address.
inline constexpr std::size_t kProfileMissionCompletionPtrOffset = 0x64;
inline constexpr std::size_t kProfileUnlocksSetOffset = 0x70;
inline constexpr std::size_t kProfilePendingUnlocksSetOffset = 0x7C;
inline constexpr std::size_t kProfileSeenUnlocksSetOffset = 0x88;
inline constexpr std::size_t kProfileNamedCounterMapOffset = 0xA0;
inline constexpr std::size_t kProfileContentListOffset = 0xD0;

// Ordering that stands in for BSP_NativeString_LessCaseInsensitive 00443D00.
// Only the equivalence it induces is load bearing: two keys collide exactly
// when they match ignoring ASCII case.
struct NativeStringCaseInsensitiveLess {
    using is_transparent = void;
    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept;
};

using UnlockNameSet = std::set<std::string, NativeStringCaseInsensitiveLess>;
using UnlockCounterMap = std::map<std::string, int, NativeStringCaseInsensitiveLess>;

// Which of the five sources answered for a token. Declared in the order
// 007FC4C0 tries them, because the native short-circuits on the first hit.
enum class UnlockSource : std::uint32_t {
    kNone = 0,
    // 0090C560 on the map behind `[profile+64h]`: present and non-zero.
    kMissionCompleted,
    // 004C7F10 find on the set at `profile+70h`, the saved "Unlocks" key.
    kUnlocks,
    // 004C7F10 find on the set at `profile+7Ch`, granted this session and not
    // yet folded into `Unlocks` by the debriefing screen.
    kPendingUnlocks,
    // 005097D0 on the map at `profile+A0h`: strictly greater than zero.
    kNamedCounter,
    // 007F8890 linear scan of the list at `profile+D0h`.
    kDownloadableContent,
};

// The part of the profile block at `game+650h` the predicate reads.
struct ProfileUnlockState {
    // `[profile+64h]` -> a 24h-byte object whose map is keyed by mission id.
    // 005C4080 walks every 434h mission record and asks this map about
    // `record+00h`, the mission name, so a non-zero value means completed.
    UnlockCounterMap mission_completion;

    // profile+70h. Serialized by 007FDF00 under the key "Unlocks". The
    // debriefing screen 0062C2C0 lists `pending_unlocks` and then 007FB990
    // moves every entry into this set and empties the pending one.
    UnlockNameSet unlocks;

    // profile+7Ch. Written by the Lua binding luaMW_Scoring_GrantUnlock
    // (008CB920, ECX = game+6CCh). 007FDF00 does not serialize it.
    UnlockNameSet pending_unlocks;

    // profile+A0h. 007FDB20 seeds it with `map["RANK"] = 1`; the award
    // trackers raise entries through BSP_AwardTracker_RecordAtLeast 007FBE20.
    UnlockCounterMap named_counters;

    // profile+D0h, a std::list of owned downloadable-content ids such as
    // "DL_Content_0000062" (008D57A0, 00706760).
    std::vector<std::string> content_ids;
};

// The strtok split 007FC4C0 performs. A null or empty expression yields no
// tokens, which is why the callers that guard on an empty UnlockID never reach
// the predicate.
std::vector<std::string> parse_unlock_expression_007fc4c0(std::string_view expression);

// The per-token decision, in native order. Returns kNone when no source
// answers.
UnlockSource classify_unlock_token_007fc4c0(
    const ProfileUnlockState& profile, std::string_view token);

// 007FC4C0, __thiscall(this = game+650h, const NativeString* requirement)
// returning char, RET 4. Any-of over the tokens: the first token that any
// source answers wins and the walk stops. An expression with no tokens is
// false, so an empty requirement locks its subject.
bool is_unlock_expression_satisfied_007fc4c0(
    const ProfileUnlockState& profile, std::string_view expression);

// 007FC820, __thiscall(this = game+650h, vector<NativeString>* requirements)
// returning char, RET 4. All-of over the 8-byte elements of the vector at
// `record+78h`; an empty vector is true, which is how the 131 missions with
// `prerequisites = {}` stay open.
bool are_unlock_requirements_met_007fc820(
    const ProfileUnlockState& profile, const std::vector<std::string>& requirements);

} // namespace bsp
