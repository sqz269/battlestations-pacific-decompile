#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/frontend_entry.hpp"

// Host projection of the award grant path: how an award name becomes an Xbox
// LIVE achievement id, what has to be true before the id is queued for upload,
// and the one concrete grant 004e4000 performs when the front-end shell opens.
//
// The path has three stages. A name is resolved to an id through the registry
// built by 006b9450 and held in 00E19900; the id is range checked; the online
// manager at 00F8ABE8 is asked whether the signed-in profile may receive it.
// Only then does 00a410a0 append the id to the manager's upload queue, and the
// local tracker at game+650h records the same name (docs/GAME_AWARD_TRACKERS.md
// owns that side).
//
// The manager object at 00F8ABE8 is the same one docs/GAME_SESSION_POLLS.md
// projects as bsp::PlatformManagerFlags; the fields below are disjoint from it
// and are not repeated there.
//
// Evidence, native ABI and uncertainty: docs/AWARD_GRANT.md.
namespace bsp {

// ---------------------------------------------------------------------------
// The award registry at 00E19900
// ---------------------------------------------------------------------------

// BSP_Game_OnInit allocates 20h bytes at 004e3d7f and runs 006b9450 on them
// (004e3d98), then stores the pointer in 00E19900. The object is a vector at
// +0h..+0Fh and a std::map at +10h whose head node is +14h and whose size
// counter at +1Ch the constructor increments once per parsed row.
inline constexpr std::size_t kAwardRegistryInstanceSize = 0x20;
inline constexpr std::size_t kAwardRegistryMapOffset = 0x10;

// The registry is data driven: 006b9450 opens "Scripts\datatables\Achievements.lua"
// and walks the table named "Achievements". Each row key becomes the map key and
// each column below becomes one field of the value struct. Offsets are byte
// offsets inside the value, which 0050fc30 returns as node+14h.
enum AwardDefinitionOffset : std::size_t {
    kAwardDefName = 0x00,          // "Name", localisation key, NativeString
    kAwardDefIconId = 0x08,        // "ID", icon index for AchievementsGUI.lua
    kAwardDefDescription = 0x0C,   // "Description", localisation key
    kAwardDefGuiTexture = 0x14,    // "GUITexture"
    kAwardDefSound = 0x1C,         // "Sound"
    kAwardDefMulti = 0x24,         // "Multi", one byte
    kAwardDefUnlock = 0x28,        // "Unlock", NativeString
    kAwardDefIndexList = 0x34,     // "Index", list of integers
    kAwardDefScore = 0x3C,         // "Score", default 0
    kAwardDefXLastAchievementId = 0x40, // "XLastAchievementID", default -1
};

// One row of the shipped table, reduced to what the grant path reads. The names
// are the Lua row keys and the ids are the resolved "XLastAchievementID"
// values; both come from the retail data file, not from the executable, so this
// table mirrors shipped data rather than recovered code.
struct AwardNameId {
    const char* name;
    int achievement_id;
};

// Every row whose id passes the range check below, in ascending id order. Rows
// the retail table leaves without the column (the BW_, BU_ and BO_ badges) and
// the "RANK" row, whose id is 0, are absent because the native drops them at the
// same range check.
extern const AwardNameId kAwardNameIds[52];
inline constexpr std::size_t kAwardNameIdCount = 52;

// 006b8da0, __thiscall (ECX = the registry, one NativeString* argument), RET 4.
// It runs the map find and returns the id at node+54h, or 0 when the name is not
// a row. The native map is ordered by 00443d00, a case-insensitive compare, so
// the lookup here is case-insensitive too. Returns 0 for an unknown name.
int award_id_for_name_006b8da0(const char* name) noexcept;

// 004e437a: LEA EAX,[ESI-1]; CMP EAX,62h; JA. 0090ef86 repeats it as an unsigned
// (id - 1) < 99. Both reject the -1 default and the 0 of the "RANK" row. The
// bounds are bsp::kAwardIdMin and bsp::kAwardIdMax from frontend_entry.hpp.
bool is_grantable_award_id(int achievement_id) noexcept;

// ---------------------------------------------------------------------------
// Sign-in state of the online manager at 00F8ABE8
// ---------------------------------------------------------------------------

// The sign-in state 00a40510 records for one pad slot, read as the dword at
// manager+8Ch+slot*4. 00a3fbb3 prints "LIVE" for value 2 and "Local" otherwise,
// which is what fixes the two named values.
enum AwardSignInState : int {
    kSignedOut = 0,
    kSignedInLocally = 1,
    kSignedInToLive = 2,
};

inline constexpr std::size_t kAwardSignInSlotCount = 4;

// The four manager fields the grant gate reads. 00a40510 writes all of them:
// state 4 sets user_selected and copies user_index from manager+3B4h (00a40822),
// and state 5 stores bit 0 of the dwInfoFlags returned by XUserGetSigninInfo
// into live_enabled (00a40925). 00a3e6a0 clears the pair and resets user_index
// to 1.
struct OnlineSignInState {
    bool user_selected{false};  // +119h
    bool live_enabled{false};   // +11Ah, XUSER_INFO_FLAG_LIVE_ENABLED
    int user_index{0};          // +11Ch, the XUser slot index
    int slot_state[kAwardSignInSlotCount]{}; // +8Ch, AwardSignInState per slot
};

// Sign-in state of the selected slot. The native indexes manager+8Ch with
// manager+11Ch without a bound check; an index outside the array returns
// kSignedOut here instead of reading past it.
int selected_slot_state_00a3ead0(const OnlineSignInState& state) noexcept;

// 00a3e520, __fastcall (ECX = manager), RET, returns AL. A one-instruction
// getter for +11Ah.
bool live_enabled_account_00a3e520(const OnlineSignInState& state) noexcept;

// 004b44f0, __thiscall (ECX = manager), RET, returns EAX as 0 or 1. It is
// 00a3e510 (+119h) and then 00a3ead0 == 2, so: a slot is selected and that slot
// is signed in to LIVE.
bool signed_into_live_004b44f0(const OnlineSignInState& state) noexcept;

// 00a410a0, __thiscall (ECX = manager, int achievement id), RET 4. Its own gate
// before it touches the queue: +119h set, the selected slot is local or LIVE,
// and +11Ah set. It is weaker than the call-site gate at 004e4000, which already
// demands kSignedInToLive.
bool may_queue_award_00a410a0(const OnlineSignInState& state) noexcept;

// ---------------------------------------------------------------------------
// The upload queue at manager+360h
// ---------------------------------------------------------------------------

// 00a410a0 appends the id to a std::vector<int> at manager+360h (allocator pad
// at +360h, _Myfirst +364h, _Mylast +368h, _Myend +36Ch). 00a41030 is that
// vector's push_back and 00a40d60 its insert; both are compiler-emitted STL and
// are not reimplemented here. 00a3fa70 later copies the queue into an
// XUSER_ACHIEVEMENT array at +3A0h, hands it to XUserWriteAchievements with the
// XOVERLAPPED at +384h, and erases each written id once the write completes.
inline constexpr std::size_t kAwardUploadQueueOffset = 0x360;
inline constexpr std::size_t kAwardUploadArrayOffset = 0x3A0;
inline constexpr std::size_t kAwardUploadCountOffset = 0x3A4;
inline constexpr std::size_t kAwardUploadOverlappedOffset = 0x384;

// ---------------------------------------------------------------------------
// The one grant 004e4000 performs
// ---------------------------------------------------------------------------

// The award name the front-end shell entry grants is bsp::kFirstMainMenuAwardKey
// from frontend_entry.hpp, the string at 00CE824C. It is row GA_HM of the
// shipped table, id 76.
//
// 004e43ba pushes 1 as the count for the local tracker record.
inline constexpr int kMidwaySaveAwardCount = 1;

// 0090c5d0, no arguments, RET (the call site loads ECX from *(*(00E188A8)+21A0h)
// but the body never reads it). It builds <CSIDL_PERSONAL>\Battlestations-Midway\save
// and calls CreateDirectoryA on it. It returns true only when the create fails
// with ERROR_ALREADY_EXISTS, so a run that has to create the folder returns
// false: the award is for a player whose machine already carried a
// Battlestations: Midway save directory.
inline constexpr std::uint32_t kErrorAlreadyExists = 0xB7u;
bool midway_save_folder_preexisted_0090c5d0(bool create_directory_succeeded,
    std::uint32_t last_error) noexcept;

// The whole 004e4310..004e43e2 predicate as one pure function. Both online
// checks read the same manager, so they are one input here.
struct AwardGrantInputs {
    bool midway_save_folder_preexisted{false}; // 0090c5d0
    int achievement_id{0};                     // 006b8da0 on the award name
    OnlineSignInState online{};                // 00F8ABE8
};

// True when 004e4000 reaches 00a410a0. The local tracker record at 004e43d5 is
// inside the same branch, so this also decides whether the name is recorded
// locally: the native never records it without granting it.
bool should_grant_award_004e4310(const AwardGrantInputs& in) noexcept;

// Integration boundary for the award grant. Each method is one native call site
// of the block at 004e42e6..004e43e2, in order. There are no default
// implementations: nothing here stands in for unrecovered game behaviour.
struct AwardGrantHost {
    virtual ~AwardGrantHost() = default;
    // 0090c5d0, ECX = *(*(00E188A8)+21A0h).
    virtual bool midway_save_folder_preexisted() = 0;
    // 006b8da0, ECX = *(00E19900), argument the NativeString built at 004e4338.
    virtual int award_id_for_name(const char* name) = 0;
    // 00a3e520, ECX = *(00F8ABE8).
    virtual bool live_enabled_account() = 0;
    // 004b44f0, ECX = *(00F8ABE8).
    virtual bool signed_into_live() = 0;
    // 00a410a0, ECX = *(00F8ABE8), the id on the stack.
    virtual void queue_online_award(int achievement_id) = 0;
    // 007fbe20, ECX = *(00E188A8)+650h, arguments (name, count).
    // docs/GAME_AWARD_TRACKERS.md owns that routine.
    virtual void record_local_award(const char* name, int count) = 0;
};

// Runs the block for one award name in native order: folder probe, name lookup,
// range check, the two online checks, queue, local record. Returns true when the
// award was granted. Short-circuits exactly where the native branches to
// 004e43e7, so a host sees no call the native would not have made.
bool grant_award_if_earned(AwardGrantHost& host, const char* name, int count) noexcept;

// 004e4000's own call, kept as a name because the front-end shell entry has no
// other award.
inline bool grant_midway_save_award(AwardGrantHost& host) noexcept
{
    return grant_award_if_earned(host, kFirstMainMenuAwardKey, kMidwaySaveAwardCount);
}

} // namespace bsp
