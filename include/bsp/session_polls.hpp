#pragma once
#include <cstddef>
#include <cstdint>
#include "bsp/xlive_types.hpp"

// Per-frame session polling reached from BSP_Game_OnMove (004e4a40).
//
//   004e4b91  004db290  BSP_Game_PollPlatformSessionEvents
//   004e4b98  004caa90  BSP_OnlineStats_UpdateWrite
//   004e5036  00778560  multiplayer tick on game+1EF0h
//   004e5434  004d80d0  BSP_Game_UpdateMultiplayerInterface
//   004e5442  006840f0  BSP_MenuInterface_ServicePendingRequests
//   004e5455  the do/while the decompiler removed as unreachable, driving
//             00776230 and 004c40f0 until 006840f0 stops reporting work
//
// Nothing here is ABI-compatible with the game. Structures are projections of
// the native fields the routines read; the offsets are in the comments.
// See docs/GAME_SESSION_POLLS.md for the evidence behind each statement.

namespace bsp {

// ---------------------------------------------------------------------------
// XNotify drain, 00a40110 (reached through 00a409f0 at 004db2bc)
// ---------------------------------------------------------------------------

// Notification ids the drain has a case arm for. XNotifyCreateListener is called
// with qwAreas = 2Fh at 00a40fd4, and again at the head of the drain when the
// listener handle at manager+1Ch is -1. The names follow the public XNOTIFY_*
// set and are provisional; only the numeric ids and the effects are recovered.
enum class PlatformNotification : std::uint32_t {
    kSystemUi = 0x00000009u,
    kSignInChanged = 0x0000000Au,
    kStorageDevicesChanged = 0x0000000Bu,
    kProfileSettingChanged = 0x0000000Eu,
    kTitleUpdate = 0x00000015u,
    kSystemUpdate = 0x00000016u,
    kLiveConnectionChanged = 0x02000001u,
    kLiveInviteAccepted = 0x02000002u,
    kLiveLinkStateChanged = 0x02000003u,
    kLiveContentInstalled = 0x02000007u,
    kFriendsPresenceChanged = 0x04000002u,
    kFriendsListChanged = 0x04000003u,
};

// What one case arm of 00a40110 does. Every arm logs through 004254b0 first
// (" ...   NotifyGet %x %x", id, parameter) which is not modelled here.
enum class PlatformNotificationEffect {
    // Calls the hook in 00f8abec when it is set (00a409f0 installs 004ceb40 at
    // 004e555e) and stores parameter != 0 into manager+3E8h.
    kSetSystemUiVisible,
    // 00a3f440, the sign-in state refresh. No flag byte is written here.
    kRefreshSignInState,
    // Calls the hook in 00f8abf0. Nothing in the image writes that pointer, so
    // on this build the arm is a no-op.
    kStorageHook,
    // 00a3e600(parameter).
    kProfileSettingChanged,
    // 00a40416: a nonempty update path gets "\\setup.exe" appended, then launches.
    kTitleUpdate,
    // 00a404a9: XLiveUpdateSystem then _exit(0). The process does not return.
    kSystemUpdateAndExit,
    // 00a40353: logs "XN_LIVE_CONNECTIONCHANGED"; parameter 80151005h sets
    // manager+128h (00a403b1), parameter 001510F0h calls 00a3fa70(1) (00a40391).
    kLiveConnectionChanged,
    // 00a4031e: XInviteGetAcceptedInfo(parameter, &info) through 00a4d5a8,
    // manager+31h = 1 at 00a4033c, then REP MOVSD of 15h dwords (84 bytes, one
    // XINVITE_INFO) into manager+32h at 00a4034c.
    kAcceptInvite,
    // 00a403bd, log only.
    kLogOnly,
    // 00a40400: manager+30h = 1 and a log line.
    kSetContentInstalled,
    // 00a403e6: 00f8a2fc virtual +28h when that object exists.
    kNotifyOnlineClient,
};

struct PlatformNotificationArm {
    std::uint32_t id;
    PlatformNotificationEffect effect;
};

// The complete case list of 00a40110 in numeric order. Ids outside it fall out
// of every compare and are dropped without a log line beyond the NotifyGet one.
extern const PlatformNotificationArm kPlatformNotificationArms[12];

// Returns the arm for `id`, or nullptr when the drain ignores the notification.
const PlatformNotificationArm* find_platform_notification_arm(std::uint32_t id) noexcept;

// The bytes of the platform manager (00f8abe8) that the drain writes and that
// 004db290 reads back. Field names are the offsets; only these six are used by
// this packet.
struct PlatformManagerFlags {
    bool profile_changed{false}; // +2Ch, read by 00a3e3b0, never cleared there
    bool storage_removed{false}; // +2Fh, read and cleared by 00a3e420
    bool invite_accepted{false}; // +31h, read and cleared by 00a3e440, set by the drain
    bool content_installed{false}; // +30h, set by the drain
    bool link_failure{false}; // +128h, set for connection parameter 80151005h
    bool system_ui_visible{false}; // +3E8h (manager+1000), set from the parameter
    // +3C0h holds the status of an outstanding asynchronous operation. 00a3e3b0
    // suppresses the profile-changed report while it is 3E5h (ERROR_IO_PENDING).
    // Own the entire durable DLL output block, so the profile poll observes
    // words[0] directly rather than a stale copy of an asynchronous result.
    XLiveOverlapped profile_overlapped_3c0{};
};

// Applies one notification to the manager flags. Only the arms that write a flag
// byte change anything; the rest are call-outs the caller has to perform.
void apply_platform_notification_00a40110(
    PlatformManagerFlags& flags, std::uint32_t id, std::uint32_t parameter) noexcept;

// 00a3e3b0: profile-changed is reported only once the pending asynchronous
// operation has left ERROR_IO_PENDING. Does not clear the flag.
bool profile_changed_pending_00a3e3b0(const PlatformManagerFlags& flags) noexcept;

inline constexpr std::uint32_t kAsyncStatusPending = 0x3E5u; // ERROR_IO_PENDING

// ---------------------------------------------------------------------------
// Accepted invite, manager+32h
// ---------------------------------------------------------------------------

// Projection of the 84-byte block copied at 00a4034c. The size (15h
// dwords) and the four fields 004db290 reads pin this to XINVITE_INFO:
//   +00h xuidInvitee, +08h xuidInviter, +10h dwTitleID,
//   +14h hostInfo.sessionID, +1Ch hostInfo.hostAddress, +40h keyExchangeKey,
//   +50h fFromGameInvite.
struct AcceptedInviteInfo {
    std::uint64_t xuid_invitee{0};
    std::uint64_t xuid_inviter{0};
    std::uint32_t title_id{0};
    std::uint64_t session_id{0};
    bool from_game_invite{false};
};

// Compared against info.title_id at 004db574. Battlestations: Pacific.
inline constexpr std::uint32_t kTitleId = 0x534307FAu;

// Value XNetXnAddrToPlatform (00a4d434) has to return for the host address at
// info+1Ch. Anything else raises FE.crossplatform_notsupported (004db59a).
inline constexpr std::uint32_t kSupportedHostPlatform = 2u;

// Returned by 00a3e470 / 00a3e4b0 when no local slot matches. The slot loop runs
// over a single entry (00a3e480..00a3e4a0), so the result is 0 or 1.
inline constexpr int kNoLocalSlot = 1;

// ---------------------------------------------------------------------------
// 004db290, the platform session poll
// ---------------------------------------------------------------------------

// Everything 004db290 needs about the invite arm at 004db4eb. Each field names
// the native query that produces it.
struct InviteDecisionInputs {
    int game_state{0}; // game+5D4h
    bool suppressed{false}; // game+610h, non-zero skips the in-session check
    bool signed_in{false}; // 004b44f0: 00a3e510 and 00a3ead0 == 2
    bool session_active{false}; // game+1FE4h != 0
    bool already_in_invited_session{false}; // info.session_id == 00a43560 result
    int invitee_slot{kNoLocalSlot}; // 00a3e470, matches info+00h
    int inviter_slot{kNoLocalSlot}; // 00a3e4b0, matches info+08h
    int active_slot{kNoLocalSlot}; // 00a3eac0, manager+11Ch
    std::uint32_t title_id{0}; // info+10h
    bool host_platform_known{false}; // XNetXnAddrToPlatform returned 0
    std::uint32_t host_platform{0}; // its out parameter
    bool from_game_invite{false}; // info+50h
};

enum class InviteOutcome {
    // Dropped without a prompt: the title id did not match, or the local user is
    // already in the invited session.
    kIgnored,
    // 004db6a8: front-end state 2 accepts straight through 004d8000.
    kAcceptImmediately,
    // 004db59a, FE.crossplatform_notsupported.
    kCrossPlatformNotSupported,
    // 004db633, FE_xbox.xsm_invitesameconsole. The inviter is signed in locally.
    kInviteSameConsole,
    // 004db6e7, the composed yes/no prompt with callback 004db260.
    kJoinWarningPrompt,
    // 004db819 / 004db888, a yes/no prompt with callback 004db240. The invitee is
    // not the active local profile.
    kOtherPlayerPrompt,
};

// The decision tree of 004db4eb..004db8f0. Assumes the invite flag was taken and
// the state gate (game+5D4h >= 2) already passed.
InviteOutcome classify_accepted_invite_004db4eb(const InviteDecisionInputs& in) noexcept;

// Localisation keys. The prompt title is always the empty string at 00ce3a0c.
const char* invite_outcome_text_key(InviteOutcome outcome, bool from_game_invite) noexcept;

// The message for kJoinWarningPrompt is built by appending up to three keys
// (004db6e7..004db788). The middle part appears only when 004bb8a0 reports that
// leaving would end the session for the other players.
struct InvitePromptText {
    const char* parts[3]{};
    std::size_t count{0};
};
InvitePromptText compose_join_warning_text_004db6e7(bool would_end_session) noexcept;

// The five distinguishing arguments of the eight 00531b00 takes. Argument order
// is (kind, message, style, callback, flag, title, 0.0f, 0); the title is always
// the empty string at 00ce3a0c and the last two arguments are always 0.0f and 0.
struct PromptRequest {
    InvitePromptText text{}; // argument 2
    int kind{2}; // argument 1: 2 everywhere except the storage prompt (004db497)
    int style{2}; // argument 3: 2 without a callback, 1 with one
    std::uint32_t callback{0}; // argument 4
    int flag{0}; // argument 5: 1 only at 004db7cc, meaning unresolved
};

// 004db509..004db55b: true when the poll returns before it dismisses the dialog
// layers, so nothing else in the invite arm runs.
bool invite_already_in_session_004db509(const InviteDecisionInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 004caa90, the online stats write poll
// ---------------------------------------------------------------------------

struct OnlineStatsWriteState {
    bool write_in_flight{false}; // game+22DDh
    std::uint32_t overlapped_internal_low{0}; // game+7038h, 3E5h while pending
};

enum class OnlineStatsWriteStep {
    kSkipped, // no write in flight, or the overlapped is still pending
    kSubmitNext, // the completed entry was popped and the next one submitted
    kRetryFront, // the write failed; the same front entry is submitted again
    kQueueDrained, // nothing left to write, "WriteStats kaput"
};

// 004caa90. `overlapped_result` is the XGetOverlappedResult output at 004caaf8
// and `pending_writes` is the element count at 00f8a2fc+2208h before the step.
OnlineStatsWriteStep online_stats_write_step_004caa90(
    OnlineStatsWriteState& state, std::uint32_t overlapped_result, std::size_t& pending_writes) noexcept;

// ---------------------------------------------------------------------------
// 006840f0 and the drain loop at 004e5455
// ---------------------------------------------------------------------------

// One of the four menu-interface globals 006840f0 walks, in call order:
// 00e198ac, 00e198b4, 00e198b8, 00e198c4. A null global is skipped.
struct MenuRequestChannel {
    bool present{false}; // the global itself is non-null
    bool active{false}; // +3Ch
    std::int32_t current_a{0}; // +04h
    std::int32_t target_a{0}; // +20h
    std::int32_t current_b{0}; // +1Ch
    std::int32_t target_b{0}; // +38h
};

inline constexpr std::size_t kMenuRequestChannelCount = 4;

struct MenuInterfaceState {
    MenuRequestChannel channels[kMenuRequestChannelCount]{};
};

// True when 006840f0 would call the channel's virtual +10h and set its out byte.
bool menu_channel_has_pending_request(const MenuRequestChannel& channel) noexcept;

struct MenuRequestServicer {
    virtual ~MenuRequestServicer() = default;
    // Virtual +10h of the channel object, called with (target_a, target_b).
    virtual void service_menu_channel(
        std::size_t index, std::int32_t target_a, std::int32_t target_b) = 0;
};

// 006840f0. The native signature is __fastcall(bool *out) with ECX holding the
// out pointer; the caller zeroes the byte first and 006840f0 only ever stores 1,
// so returning the flag is equivalent.
bool service_pending_menu_requests_006840f0(
    MenuInterfaceState& menus, MenuRequestServicer& servicer);

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site of the two sequences below. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct SessionPollHost : MenuRequestServicer {
    // --- 004db290, step 8 of the frame ---
    // 00a409f0 with ECX = 00f8abe8. Drains XNotify through 00a40110.
    virtual void pump_platform_manager_00a409f0() = 0;
    // 00e188ae, the byte 004c40f0 also latches.
    virtual bool system_ui_flag_00e188ae() = 0;
    // 004ceb40 with CL = 1.
    virtual void on_system_ui_raised_004ceb40() = 0;
    // 00a3e3b0. Does not clear manager+2Ch.
    virtual bool profile_changed_pending() = 0;
    // 00a3e420(1). Reads and clears manager+2Fh.
    virtual bool take_storage_removed() = 0;
    // 00a3e440(1). Reads and clears manager+31h. Only called in state >= 2, so a
    // notification that arrives earlier stays pending.
    virtual bool take_invite_accepted() = 0;
    virtual int game_state() = 0; // game+5D4h
    virtual bool game_flag_620() = 0;
    virtual void game_620_handler_004d94f0() = 0; // (game, 0)
    virtual bool game_flag_61f() = 0;
    virtual void game_61f_handler_004d95f0() = 0; // (game, 0)
    // 004d7f90: drains the state-request queue, then either sets state 2 or
    // enqueues request 10h. Taken when the profile changed in state 4.
    virtual void profile_lost_in_state4_004d7f90() = 0;
    virtual void teardown_menu_objects_004db190() = 0;
    virtual void teardown_session_004cccc0() = 0;
    virtual void clear_game_flag_2180() = 0;
    virtual void on_init_title_004c9a70() = 0;
    // 0109cecc virtual state at +4h; the storage prompt is skipped unless zero.
    virtual bool storage_owner_idle_0109cecc() = 0;
    virtual void storage_removed_side_effect_00bd3450() = 0;
    // 00530650: 00532a20(0..6) on the dialog manager from 00425d10.
    virtual void dismiss_dialog_layers_00530650() = 0;
    virtual void invite_decision_inputs(InviteDecisionInputs& out) = 0;
    // 004bb8a0: true when leaving would end the session for the other players.
    virtual bool leaving_ends_session_004bb8a0() = 0;
    // 004d8000, reached directly at 004db6a8 and from both dialog callbacks.
    virtual void accept_invite_004d8000() = 0;
    // 00531b00 on the dialog manager from 00425d10. The callback is 0 for the
    // notification-only prompts, 004db240 for the other-player prompt and
    // 004db260 for the join warning.
    virtual void show_prompt_00531b00(const PromptRequest& request) = 0;

    // --- 004caa90, step 8 of the frame ---
    virtual void update_online_stats_write_004caa90() = 0;

    // --- 00778560, step 16 of the frame ---
    virtual void multiplayer_pre_tick_00778450(float seconds) = 0;
    virtual void multiplayer_post_tick_0076ffc0(float seconds) = 0;

    // --- 004e5434..004e548a, the post-simulation tail ---
    virtual void update_multiplayer_interface_004d80d0() = 0;
    virtual void read_menu_interface_state(MenuInterfaceState& out) = 0;
    // 00776230 with ECX = game+1EF0h.
    virtual void pump_peer_messages_00776230() = 0;
    // 004c40f0 with ECX = game.
    virtual void update_interface_only_004c40f0() = 0;
    // 00e18cdc = 0, cleared after every 006840f0 poll.
    virtual void clear_menu_transition_latch_00e18cdc() = 0;
};

// Prompt styles passed as the third argument of 00531b00. 2 for the
// notification-only prompts, 1 for the two that carry a callback.
inline constexpr int kPromptStyleNotify = 2;
inline constexpr int kPromptStyleConfirm = 1;

// First argument of 00531b00. 004db497 is the only site in this routine that
// passes 0; every other prompt passes 2.
inline constexpr int kPromptKindDefault = 2;
inline constexpr int kPromptKindStorage = 0;

// Dialog callbacks, both __fastcall(int result) and both acting only on 1.
inline constexpr std::uint32_t kInviteAcceptCallback = 0x004DB240u;
inline constexpr std::uint32_t kJoinWarningCallback = 0x004DB260u;

// 004db290 in full. Returns what the poll decided about an accepted invite;
// kIgnored covers both "no invite this frame" and the two drop paths.
InviteOutcome poll_platform_session_events_004db290(SessionPollHost& host);

// Step 8 of BSP_Game_OnMove, 004e4b91..004e4b98.
void run_session_polls(SessionPollHost& host);

// Step 16, 004e5036. Native __fastcall(game+1EF0h, float) with RET 4.
void run_multiplayer_tick_00778560(SessionPollHost& host, float seconds);

// 004e5434..004e548a: the multiplayer interface update, the first request poll,
// and the do/while Ghidra removed as unreachable. Returns the number of times
// the loop body ran, which is 0 whenever the first poll finds nothing pending.
// The native loop has no iteration bound; it ends only when a whole pass over
// the four channels finds no active channel with a mismatched index pair.
int run_menu_interface_drain_004e5434(SessionPollHost& host);
}
