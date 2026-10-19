#pragma once
// XLive online system, segment 72 (00A46B80-00A625D0), packet cc13-gui2
// (docs/CC13_GUI_2.md, Part B). Layout of the 2218h-byte XLiveSystem object
// (vtable 00D24878, singleton [00F8A2FC]) is in docs/CC13_BAND_00A1_00A4.md;
// the voice and session helpers already reconstructed by cc13-sweepA1 live in
// include/bsp/network_online_cc13.hpp (bsp::xlive_cc13).
//
// Every routine takes the native object as void* and reads native offsets.
// XLive, XHV and CRT calls go through host interfaces. Build-tested only: not
// ABI-compatible, not game-validated (online play was not exercised).
#include "bsp/native_legacy_sbo_string.hpp"

#include <cstdint>
#include <string>

namespace bsp::online_seg71_cc13 {
struct XLiveVoiceCalls;
struct XenonCalls;
}

namespace bsp::online_cc13 {

// ---------------------------------------------------------------------------
// XLiveSystem vtable slots that only read a field or return a constant.
// All __thiscall(ECX = XLiveSystem); the RET size is the slot's stack-argument
// count, which the C++ signatures below ignore except where noted.
// ---------------------------------------------------------------------------

// 00A47890: AL = byte +ECh (online started).
bool is_online_started_00a47890(const void* system) noexcept;
// 00A478A0: AL = byte +124h.
bool flag_124_00a478a0(const void* system) noexcept;
// 00A478B0: AL = byte +1BD5h (last-error flag).
bool has_last_error_00a478b0(const void* system) noexcept;
// 00A478C0: AL = byte +1BDCh.
bool flag_1bdc_00a478c0(const void* system) noexcept;
// 00A478D0: EAX = system + 128h.
const void* block_128_00a478d0(const void* system) noexcept;
// 00A478E0: AL = byte +EDh (session search active).
bool is_search_active_00a478e0(const void* system) noexcept;
// 00A47920: AL = dword +1D28h != 0 (voice active).
bool is_voice_active_00a47920(const void* system) noexcept;
// 00A479F0: ST0 = float +1D40h.
float float_1d40_00a479f0(const void* system) noexcept;
// 00A47A40: EAX = system + 8Ch.
const void* block_8c_00a47a40(const void* system) noexcept;
// Constant slots: 00A478F0 (RET 4), 00A479D0 (RET 0), 00A47A00 (RET 8),
// 00A47A10, 00A47A20, 00A47A50 (RET 4) return AL = 1; 00A47970, 00A47980
// (RET 4), 00A479A0, 00A479B0 (RET 0Ch) return AL = 0; 00A48F00 (TRIV body)
// returns AL = 1; 00A48EA0 returns EAX = 0; 00A47990 (RET 4) returns ST0 =
// +0.0 (FLDZ); 00A47930, 00A47940, 00A47960, 00A479E0 (RET 4), 00A47950,
// 00A479C0 (RET 0Ch), 00A47A30 (RET 8) do nothing.
bool return_true_00a478f0() noexcept;
bool return_true_00a479d0() noexcept;
bool return_true_00a47a00() noexcept;
bool return_true_00a47a10() noexcept;
bool return_true_00a47a20() noexcept;
bool return_true_00a47a50() noexcept;
bool return_false_00a47970() noexcept;
bool return_false_00a47980() noexcept;
bool return_false_00a479a0() noexcept;
bool return_false_00a479b0() noexcept;
bool return_true_00a48f00() noexcept;
std::uint32_t return_zero_00a48ea0() noexcept;
float return_zero_float_00a47990() noexcept;
void nop_00a47930() noexcept;
void nop_00a47940() noexcept;
void nop_00a47950() noexcept;
void nop_00a47960() noexcept;
void nop_00a479c0() noexcept;
void nop_00a479e0() noexcept;
void nop_00a47a30() noexcept;

// 00A48E90 (slot +1B4h): clear byte +4h, then tail-jump to 00A48F00 (AL = 1).
bool clear_byte4_00a48e90(void* object) noexcept;

// What the system-level routines call back into.
struct XLiveSystemHost {
    virtual ~XLiveSystemHost() = default;
    // System vtable +40h / +44h (two session-state predicates, read by 00A46F30).
    virtual bool virtual_40(void* system) = 0;
    virtual bool virtual_44(void* system) = 0;
    // System vtable +80h: the active player list (its +8h is the count).
    virtual const void* player_list_80(void* system) = 0;
    // 00A46D60 BSP_XLiveSystem_LeaveSession.
    virtual void leave_session_00a46d60(void* system) = 0;
    // XOnlineCleanup (import thunk 00A4D5EA): HRESULT.
    virtual std::int32_t xonline_cleanup_00a4d5ea() = 0;
};

// 00A47A60 and its byte-identical copy 00A47A80: AL = vtable +80h()'s +8h != 0.
bool has_players_00a47a60(void* system, XLiveSystemHost& host);
bool has_players_00a47a80(void* system, XLiveSystemHost& host);

// 00A46EE0 BSP_XLiveSystem_Shutdown, __fastcall(ECX = system), AL. When +ECh
// is set: 00A46D60, XOnlineCleanup, game pointer +2210h = 0; a failing HRESULT
// returns false with +ECh still set, success clears +ECh and returns true.
// When +ECh is clear: +2210h = 0 and return FALSE (not true: the AL = BL = 1
// set at 00A46EEB is only returned on the success path).
bool shutdown_00a46ee0(void* system, XLiveSystemHost& host);

// 00A46F30, __fastcall(ECX = system), AL = 1 always: when vtable +40h or
// vtable +44h answers true, 00A46D60 runs.
bool leave_session_if_players_listed_00a46f30(void* system, XLiveSystemHost& host);

// ---------------------------------------------------------------------------
// Sub-object constructors (all __fastcall(ECX = this), EAX = this)
// ---------------------------------------------------------------------------

// 00A48F10 BSP_XLiveVoice_Construct (system +1C24h): zeroes +70h, +74h, +78h,
// +7Ch, +80h, +84h, +C4h, +104h, +108h, +10Ch, +118h; +114h = -100 (FFFFFF9Ch).
void* voice_construct_00a48f10(void* voice) noexcept;
// 00A499D0 BSP_XLiveSession_Construct (system +2098h): zeroes +C0h, +130h,
// +158h, +15Ch, +160h.
void* session_construct_00a499d0(void* session) noexcept;
// 00A49CF0 BSP_XLiveMatchmaking_Construct (system +184h): zeroes +1808h,
// +1938h, +193Ch, +1960h; +1940h = 1.
void* matchmaking_construct_00a49cf0(void* matchmaking) noexcept;
// 00A4A380 BSP_XLiveHostQos_Construct (system +1AE8h): zeroes +78h, +88h,
// +8Ch; +90h = 1.
void* host_qos_construct_00a4a380(void* qos) noexcept;

struct XLiveQosHost {
    virtual ~XLiveQosHost() = default;
    // XNetQosRelease (import thunk 00A4D45E).
    virtual std::int32_t xnet_qos_release_00a4d45e(void* qos) = 0;
};

// 00A4A3A0 BSP_XLiveHostQos_ReleaseLookup, __fastcall: when +88h is set,
// XNetQosRelease(+88h) and +88h = 0; +78h = 0 in either case.
void host_qos_release_lookup_00a4a3a0(void* qos, XLiveQosHost& host);
// 00A4A3D0 BSP_XLiveHostQos_FinishIfComplete, __thiscall(two unused stack
// arguments), RET 8, HRESULT. State +8Ch == 4: release as 00A4A3A0 (inlined)
// and state = 0. Returns 0 when the state is 0 afterwards, else E_UNEXPECTED
// 8000FFFFh (NEG/SBB/AND).
std::int32_t host_qos_finish_if_complete_00a4a3d0(void* qos, XLiveQosHost& host);

struct XLiveCrtHost {
    virtual ~XLiveCrtHost() = default;
    virtual void free_00bf65ac(void* block) = 0;                     // _free
    virtual void release_string_buffer(void* buffer, std::uint32_t bytes) = 0; // 00419CC0 + 00BD1510
    virtual void close_handle(void* handle) = 0;                     // CloseHandle (IAT 00CE225C)
};

// 00A4A750 BSP_HeapPointer_FreeAndClear, __fastcall(ECX = void**): _free and null.
void heap_pointer_free_and_clear_00a4a750(void** slot, XLiveCrtHost& host);
// 00A4B470 BSP_PooledString_ReleaseStorage, __fastcall(ECX = native string
// {length, buffer}): returns length+1 bytes of a non-null buffer to the pool.
// Neither field is cleared.
void pooled_string_release_storage_00a4b470(void* string, XLiveCrtHost& host);

struct XLiveVoiceDestroyHost {
    virtual ~XLiveVoiceDestroyHost() = default;
    // 00A49270 (bsp::xlive_cc13::shutdown_00a49270 over the voice projection).
    virtual void shutdown_00a49270(void* voice) = 0;
};
// 00A493A0 BSP_XLiveVoice_Destroy, __fastcall: 00A49270, then CloseHandle(+10Ch).
void voice_destroy_00a493a0(void* voice, XLiveVoiceDestroyHost& voice_host, XLiveCrtHost& host);

// ---------------------------------------------------------------------------
// Voice (system +1C24h; the XHV2 engine pointer at +78h)
// ---------------------------------------------------------------------------

// The XHV2 engine methods these routines call, by vtable offset. The native
// engine is a COM-style object called __stdcall with an explicit this.
struct Xhv2EngineCalls {
    virtual ~Xhv2EngineCalls() = default;
    virtual std::int32_t start_local_processing_0c(void* engine, std::uint32_t user,
        const void* modes, std::uint32_t count) = 0;                                   // +0Ch
    virtual std::int32_t stop_local_processing_10(void* engine, std::uint32_t user,
        const void* modes, std::uint32_t count) = 0;                                   // +10h
    virtual std::int32_t register_local_talker_20(void* engine, std::uint32_t user) = 0; // +20h
    virtual std::int32_t start_remote_processing_14(void* engine, std::uint64_t xuid,
        const void* modes, std::uint32_t count) = 0;                                   // +14h
    virtual std::int32_t register_remote_talker_28(void* engine, std::uint64_t xuid) = 0; // +28h(xuid, 0, 0, 0)
    virtual std::int32_t get_remote_talkers_30(void* engine, std::uint32_t* count, std::uint64_t* xuids) = 0; // +30h
    virtual std::uint32_t is_remote_talking_3c(void* engine, std::uint64_t xuid) = 0;    // +3Ch
    virtual std::int32_t set_playback_priority_48(void* engine, std::uint64_t xuid, std::uint32_t user,
        std::int32_t priority) = 0;                                                    // +48h
    virtual std::uint32_t is_local_talking_38(void* engine, std::uint32_t user) = 0;      // +38h
    virtual std::uint32_t is_headset_present_34(void* engine, std::uint32_t arg) = 0;       // +34h
    virtual std::int32_t get_local_chat_data_44(void* engine, std::uint32_t user, void* data,
        std::uint32_t* bytes, std::uint32_t* packets) = 0;                             // +44h
    virtual std::int32_t submit_incoming_chat_data_4c(void* engine, std::uint64_t xuid,
        const void* data, std::uint32_t* bytes) = 0;                                   // +4Ch
};

// 00A49000 BSP_XLiveVoice_GetLocalChatData, __thiscall(voice, data, unused,
// DWORD* packets), RET 0Ch. *packets = 0, then engine +44h(user +118h, data,
// &bytes = 0C8h, packets) with `bytes` living in the third argument's stack
// slot; returns the byte count the engine wrote back on success, 0 on failure
// (NEG/SBB/NOT/AND). The second argument is never read.
std::uint32_t voice_get_local_chat_data_00a49000(void* voice, void* data, std::uint32_t* packets,
    Xhv2EngineCalls& xhv);
// 00A49040 BSP_XLiveVoice_SubmitIncomingChatData, __thiscall(voice, data,
// WORD bytes, const XUID* talker), RET 0Ch: engine +4Ch(xuid, data, &bytes
// zero-extended); returns 0 whatever the engine says.
std::uint32_t voice_submit_incoming_chat_data_00a49040(void* voice, const void* data, std::uint16_t bytes,
    const std::uint64_t* talker, Xhv2EngineCalls& xhv);
// 00A49220 BSP_XLiveVoice_IsLocalTalking, __fastcall: EAX = engine +38h(user +118h).
std::uint32_t voice_is_local_talking_00a49220(void* voice, Xhv2EngineCalls& xhv);

struct XenonSystemCalls {
    virtual ~XenonSystemCalls() = default;
    // BSP_XenonSystemManager_GetActiveUserSlot / _GetActiveXuid / _GetSlotState8C(0)
    // ([00F8ABE8] 00A3E530).
    virtual std::uint32_t active_user_slot() = 0;
    virtual std::uint64_t active_xuid() = 0;
    virtual std::int32_t slot_state_8c(std::uint32_t slot) = 0;
};

// 00A492D0 BSP_XLiveVoice_RegisterActiveLocalTalker, __fastcall: +114h = old
// +118h, +118h = active user slot; unless +108h is set, engine +20h(0); then
// StartLocalProcessingModes +0Ch(0, {2 = voice chat, 00D24A34}, 1) and
// StopLocalProcessingModes +10h(0, {1 = loopback, 00D24A30}, 1), +108h = 1,
// word +6Ch = 0. (Method names from the IXHV2Engine vtable order, which the
// sweepA1 slots +24h/+2Ch/+48h already match.) Both engine calls pass user 0, not
// the slot just stored.
void voice_register_active_local_talker_00a492d0(void* voice, Xhv2EngineCalls& xhv, XenonSystemCalls& xenon);

struct VoiceUpdateCalls {
    virtual ~VoiceUpdateCalls() = default;
    // XNotifyGetNext(+10Ch, 0, &id, &param) (import thunk 00A4D5AE): BOOL.
    virtual bool xnotify_get_next(void* listener, std::uint32_t& id, std::uint32_t& param) = 0;
    // 00A48F60 BSP_XLiveVoice_ApplyMuteListToRemoteTalkers.
    virtual void apply_mute_list_00a48f60(void* voice) = 0;
    // [00F8A2FC] vtable +28h(), the XLiveSystem singleton.
    virtual void system_virtual_28() = 0;
};

// 00A49530 BSP_XLiveVoice_Update, __fastcall, EAX = 1. Frame counter +74h:
// at exactly 14h (20) 00A492D0 runs; the counter increments either way. A
// notification with id 11h (XN_SYS_MUTELISTCHANGED) reapplies the mute list
// and calls the system's vtable +28h. When the Xenon slot state is 0, +4h and
// +0h are cleared; otherwise IsHeadsetPresent +34h(0) is compared with +4h and,
// when it changed, stored there with +0h = 1 (a headset-changed flag).
std::uint32_t voice_update_00a49530(void* voice, Xhv2EngineCalls& xhv, XenonSystemCalls& xenon,
    VoiceUpdateCalls& host);

// ---------------------------------------------------------------------------
// XSession wrapper (system +2098h; the session handle at +12Ch)
// ---------------------------------------------------------------------------

struct XSessionCalls {
    virtual ~XSessionCalls() = default;
    // XSessionArbitrationRegister(handle, 0, nonce, DWORD* bytes, results, 0).
    virtual std::int32_t arbitration_register(void* session, std::uint32_t flags, std::uint64_t nonce,
        std::uint32_t* bytes, void* results) = 0;
    virtual std::int32_t join_local(void* session, std::uint32_t count, const std::uint32_t* users,
        const std::uint32_t* private_slots) = 0;
    virtual std::int32_t leave_local(void* session, std::uint32_t count, const std::uint32_t* users) = 0;
    virtual std::int32_t join_remote(void* session, std::uint32_t count, const std::uint64_t* xuids,
        const std::uint32_t* private_slots) = 0;
    virtual std::int32_t leave_remote(void* session, std::uint32_t count, const std::uint64_t* xuids) = 0;
    // XGetOverlappedExtendedError(0).
    virtual std::int32_t overlapped_extended_error() = 0;
    // XNetQosListen(key, data, bytes, bits_per_second, flags).
    virtual std::int32_t qos_listen(const void* key, const void* data, std::uint32_t bytes,
        std::uint32_t bits_per_second, std::uint32_t flags) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
};

// 00A49A50 BSP_XLiveSession_ArbitrationRegister, __fastcall, HRESULT. Zeroes
// +110h..+128h, asks XSessionArbitrationRegister for the result size (nonce
// = qword +108h, null buffer), allocates that many bytes (00BF55BE),
// registers again with the buffer, maps a nonzero return through
// XGetOverlappedExtendedError(0), frees the buffer (00BF6989) and returns.
// The first call's own return value is ignored.
std::int32_t session_arbitration_register_00a49a50(void* session, XSessionCalls& x);
// 00A4AE70 BSP_XLiveSession_JoinLocalPlayer, __fastcall: XSessionJoinLocal(
// +12Ch, 1, {active user}, {FALSE}, 0); nonzero -> extended error, else 0.
std::int32_t session_join_local_player_00a4ae70(void* session, XSessionCalls& x, XenonSystemCalls& xenon);
// 00A4AED0 BSP_XLiveSession_LeaveLocalPlayer, __fastcall: XSessionLeaveLocal(
// +12Ch, 1, {active user}, 0).
std::int32_t session_leave_local_player_00a4aed0(void* session, XSessionCalls& x, XenonSystemCalls& xenon);
// 00A49B90 BSP_XLiveSession_JoinRemotePlayer, __thiscall(session, const XUID*,
// BOOL invited (low byte)), RET 8: XSessionJoinRemote(+12Ch, 1, xuid,
// {invited != 0}, 0).
std::int32_t session_join_remote_player_00a49b90(void* session, const std::uint64_t* xuid, bool invited,
    XSessionCalls& x);
// 00A4B180 BSP_XLiveSession_RemoveRemotePlayer, __thiscall(session, const
// XUID*), RET 4: the active user's own XUID goes to 00A4AED0 instead;
// otherwise XSessionLeaveRemote(+12Ch, 1, xuid, 0).
std::int32_t session_remove_remote_player_00a4b180(void* session, const std::uint64_t* xuid,
    XSessionCalls& x, XenonSystemCalls& xenon);
// 00A49C10 BSP_XLiveSession_StartQosListen, __thiscall(session, data, bytes,
// bits per second), RET 0Ch. Flags 4 (XNET_QOS_LISTEN_SET_DATA), 0Ch when a
// bit rate is given (| SET_BITSPERSEC), | 1 (ENABLE) when +130h is clear;
// the key is session +C7h. Success sets +130h = 1 and returns 0; a failure
// returns the error and leaves +130h.
std::int32_t session_start_qos_listen_00a49c10(void* session, const void* data, std::uint32_t bytes,
    std::uint32_t bits_per_second, XSessionCalls& x);

// ---------------------------------------------------------------------------
// Matchmaking (system +184h) and host QoS (system +1AE8h)
// ---------------------------------------------------------------------------

struct XLiveSearchCalls {
    virtual ~XLiveSearchCalls() = default;
    virtual std::int32_t xnet_qos_release(void* qos) = 0;          // 00A4D45E
    virtual std::int32_t x_cancel_overlapped(void* overlapped) = 0; // XCancelOverlapped
    virtual void free_00bf65ac(void* block) = 0;
};

// 00A49920 BSP_XLiveMatchmaking_RemoveResultAt, __thiscall(index), RET 4: in
// range, count +1808h decrements and the F6h-byte records after the index
// move down one (_memcpy over an overlapping range; VS2008's memcpy is
// memmove, which the C++ uses).
void matchmaking_remove_result_at_00a49920(void* matchmaking, std::uint32_t index) noexcept;
// 00A49990 BSP_XLiveHostQos_RemoveEntryAt, __thiscall(index), RET 4: the same
// over the 76h-byte entries and count +78h.
void host_qos_remove_entry_at_00a49990(void* qos, std::uint32_t index) noexcept;
// 00A4AF70 BSP_XLiveMatchmaking_CancelSearch, __fastcall: release QoS +1938h,
// cancel the overlapped +1944h when its status is ERROR_IO_PENDING (3E5h),
// results count +1808h = 0, free +1960h.
void matchmaking_cancel_search_00a4af70(void* matchmaking, XLiveSearchCalls& x);
// 00A4AFD0 BSP_XLiveHostQos_Cancel and its byte-identical copy 00A4B410,
// __fastcall: state +8Ch in 1..4: release +88h, +78h = 0, state 0, +90h = 1.
void host_qos_cancel_00a4afd0(void* qos, XLiveSearchCalls& x);
void host_qos_cancel_00a4b410(void* qos, XLiveSearchCalls& x);
// 00A4B250 BSP_XLiveMatchmaking_Reset, __fastcall, and its JMP thunk 00A4B460:
// state +193Ch in {1, 2, 4, 0Ch}: 00A4AF70, state 0, +1940h = 1.
void matchmaking_reset_00a4b250(void* matchmaking, XLiveSearchCalls& x);
void matchmaking_reset_00a4b460(void* matchmaking, XLiveSearchCalls& x);

// 00A47B50 BSP_XLiveSystem_MakeEmptyPlayerList, __thiscall(list), EAX = list:
// head = 00A43D40 (sentinel node), size +8h = 0. 00A474E0 (renamed
// STL_PlayerList_CopyConstruct_00a474e0) is the copy constructor: the same,
// then 00A46800 inserts source [begin, end) at the head through checked
// iterators.
struct PlayerListCalls {
    virtual ~PlayerListCalls() = default;
    virtual void* buy_sentinel_00a43d40() = 0;
    virtual void insert_range_00a46800(void* list, void* where, const void* source) = 0;
};
void* player_list_construct_00a47b50(void* list, PlayerListCalls& host);
void* player_list_copy_construct_00a474e0(void* list, const void* source, PlayerListCalls& host);

// 00A4A850 BSP_XLiveSystem_PublishUserProperty10000030, cdecl: when
// [00E188A8]+5FCh is set, XUserSetProperty(active user, 10000030h, 4,
// that +1098h).
struct UserPropertyCalls {
    virtual ~UserPropertyCalls() = default;
    virtual void* game_setup_5fc() = 0; // [00E188A8] + 5FCh
    virtual void xuser_set_property(std::uint32_t user, std::uint32_t id, std::uint32_t bytes, const void* value) = 0;
};
void publish_user_property_10000030_00a4a850(UserPropertyCalls& host, XenonSystemCalls& xenon);
struct VoiceTalkerCalls {
    virtual ~VoiceTalkerCalls() = default;
    // XUserMuteListQuery(user, xuid, BOOL* muted) (import thunk 00A4D5E4).
    virtual std::int32_t xuser_mute_list_query(std::uint32_t user, std::uint64_t xuid, std::uint32_t* muted) = 0;
    // 00A49340 (bsp::xlive_cc13::find_remote_talker_00a49340): HRESULT, index out.
    virtual std::int32_t find_remote_talker_00a49340(void* voice, std::uint64_t xuid, std::uint32_t* index) = 0;
};

// 00A48F60 BSP_XLiveVoice_ApplyMuteListToRemoteTalkers, __fastcall. Engine
// GetRemoteTalkers +30h(&count = 10h, xuids[16]); for user slot 0 only (the
// loop bound is 1) and only when +108h is set: for each talker,
// XUserMuteListQuery(0, xuid, &muted) and SetPlaybackPriority +48h(xuid, 0,
// muted ? -1 : 0). The query's return value is ignored, so a failed query
// applies whatever `muted` held from the previous talker (or the count word
// the first time: the slot [ESP+14h] is shared with the initial count 10h).
void voice_apply_mute_list_00a48f60(void* voice, Xhv2EngineCalls& xhv, VoiceTalkerCalls& host);

// 00A495E0 BSP_XLiveVoice_AddRemoteTalker, __thiscall(voice, xuid lo, hi),
// RET 8, HRESULT. A talker already in the 38h-byte array (+80h, count +7Ch)
// returns 0. Otherwise RegisterRemoteTalker +28h(xuid, 0, 0, 0); on success
// StartRemoteProcessingModes +14h(xuid, {2}, 1) and the record at index count
// gets xuid, priority[0] = -1, mute_list[0] = game_mute[0] = 0, count + 1. No
// capacity test against the 16 records 00A493C0 allocates.
std::int32_t voice_add_remote_talker_00a495e0(void* voice, std::uint64_t xuid, Xhv2EngineCalls& xhv);

// 00A49840 BSP_XLiveVoice_IsRemoteTalking, __thiscall(voice, xuid lo, hi,
// slot), RET 0Ch, EAX. 0 unless the slot's local talker is registered
// (+84h[slot]). talking = IsRemoteTalking +3Ch(xuid); headset =
// IsHeadsetPresent +34h(slot), always called. When talking: a talker missing
// from the array returns 0; either mute word set or priority -1 clears it;
// and no headset with mute-all (+C4h[slot]) clear also clears it.
std::uint32_t voice_is_remote_talking_00a49840(void* voice, std::uint64_t xuid, std::uint32_t slot,
    Xhv2EngineCalls& xhv, VoiceTalkerCalls& host);
struct XNetQosLookupCalls {
    virtual ~XNetQosLookupCalls() = default;
    virtual std::int32_t xnet_qos_release(void* qos) = 0;
    // XNetQosLookup(count, xnaddr*[], xnkid*[], xnkey*[], 0, 0, 0, probes,
    // bits per second, flags, event, XNQOS** out).
    virtual std::int32_t xnet_qos_lookup(std::uint32_t count, const void* const* addresses,
        const void* const* ids, const void* const* keys, std::uint32_t probes,
        std::uint32_t bits_per_second, void** qos) = 0;
};

// 00A49D20 BSP_XLiveMatchmaking_ReleaseQosAndRelinkResults, __fastcall,
// HRESULT. Only in state 4 (otherwise 8000FFFFh): release QoS +1938h; for
// each of the +1808h results (F6h bytes each) clear its dword +F2h and point
// the three pointer arrays at it: +180Ch[i] = result+BEh (XNADDR), +1870h[i]
// = result+A6h (XNKID), +18D4h[i] = result+AEh (XNKEY) (each array is 19h
// entries); then XNetQosLookup(count, ..., 8 probes, 64000 bps) into +1938h.
// Failure returns E_FAIL; success sets state 3. No results: return 0 and the
// state stays 4.
std::int32_t matchmaking_relink_and_lookup_00a49d20(void* matchmaking, XNetQosLookupCalls& x);
// 00A4A420 BSP_XLiveHostQos_ReleaseAndRelinkEntries, __fastcall, HRESULT: the
// same over the 76h-byte entries (count +78h, QoS +88h, state +8Ch; pointers
// entry+3Eh, +26h, +2Eh; flag dword +72h). Its three arrays at +7Ch, +80h,
// +84h are ONE dword apart, so with two or more entries each iteration
// overwrites the previous iteration's XNKID/XNKEY slots (listing 00A4A470..
// 00A4A48E: [EAX-4], [EAX], [EAX+4] with EAX += 4). Correct only for a single
// entry.
std::int32_t host_qos_relink_and_lookup_00a4a420(void* qos, XNetQosLookupCalls& x);

// 00A4BDC0 BSP_HeapObject_ScalarDelete_00a4bdc0, __thiscall(this, flags), RET
// 4, EAX = this: _free(this) when bit 0 is set; no destructor body.
void* heap_object_scalar_delete_00a4bdc0(void* self, std::uint8_t flags, XLiveCrtHost& host);

// ===========================================================================
// Packet cc13-online2 (docs/CC13_ONLINE_2.md)
// ===========================================================================

// The online player record: the 40h-byte payload of the two std::list nodes
// (48h with next/prev) in system +100h (Live players, head +104h, size
// +108h) and +10Ch (LAN players, head +110h, size +114h), and the single
// record at system +128h (the local player; 00A478D0 returns its address).
// Layout from the stack frames of 00A47130 (base ESP+0Ch) and 00A47220 (base
// ESP+34h), and from the copy constructor 00588640, which copies every field
// below. The qword at +20h aligns the struct to 8, so +1Ch and +3Ch are
// padding.
struct OnlinePlayerRecord {
    bsp::NativeLegacySboStringStorage name; // +00h gamertag / profile name
    std::uint32_t pad_1c;                   // +1Ch
    // +20h: the XUID for a Live player (00A3EB00); a LAN player stores the
    // six abEnet bytes of its XNADDR at +20h..+25h and its user slot at +26h.
    union Identity {
        std::uint64_t xuid;
        struct Lan {
            std::uint8_t enet[6];
            std::uint8_t user_slot;
            std::uint8_t unused;
        } lan;
    } id;
    std::uint32_t field_28;   // +28h copied; neither builder writes it
    std::uint32_t field_2c;   // +2Ch zero in both builders
    std::uint8_t user_slot;   // +30h
    std::uint32_t field_34;   // +34h zero in 00A47130, untouched by 00A47220
    std::uint8_t flag_38;     // +38h 0
    std::uint8_t flag_39;     // +39h 0
    std::uint8_t flag_3a;     // +3Ah 1 (00A47220 also copies it to system +1D4Ch[slot])
    std::uint8_t flag_3b;     // +3Bh 0
    std::uint32_t pad_3c;     // +3Ch
};

// Everything the cc13-online2 system routines call. Each method names the
// native routine or import it stands for.
struct OnlineSystemCalls {
    virtual ~OnlineSystemCalls() = default;
    // [00F8ABE8] XenonSystemManager 00A3EAC0 (AL), 00A3EAE0 (char*), 00A3EB00 (EDX:EAX).
    virtual std::uint8_t active_user_slot_00a3eac0() = 0;
    virtual const char* selected_username_00a3eae0() = 0;
    virtual std::uint64_t active_xuid_00a3eb00() = 0;
    // 00A45F70 (buy a 48h node {next, prev} and copy the record through
    // 00588640), 00A450B0(1) (length check, size + 1), then head->prev and
    // prev->next are linked: std::list::push_back on the list object.
    virtual void player_list_push_back(void* list, const OnlinePlayerRecord& record) = 0;
    // 00688780 STL_List_ClearStrings (ECX = list object).
    virtual void player_list_clear_00688780(void* list) = 0;
    // 006883B0 BSP_OnlinePlayerInfo_Assign (ECX = destination): record operator=.
    virtual void player_record_assign_006883b0(void* destination, const OnlinePlayerRecord& source) = 0;
    // [00E188A8] game setup: the native string {length, char*} at +6A0h, or +68Ch.
    virtual const void* game_setup_string_6a0() = 0;
    virtual const void* game_setup_string_68c() = 0;
    // XNetGetTitleXnAddr (import thunk 00A4D4D0): XNET_GET_XNADDR_* flags; 0 = pending.
    virtual std::uint32_t xnet_get_title_xnaddr(void* xnaddr_36_bytes) = 0;
    // XLiveSystem virtual slots on ECX = system.
    virtual void system_virtual_8c(void* system) = 0;
    virtual void system_virtual_170(void* system, std::uint32_t argument) = 0;
    virtual bool system_virtual_19c(void* system) = 0;
    virtual bool system_virtual_40(void* system) = 0;
    virtual const void* system_player_list_80(void* system) = 0;
    // The same slots called on the singleton [00F8A2FC] instead of ECX.
    virtual const void* singleton_player_list_80() = 0;
    virtual void singleton_virtual_1a0(std::uint32_t argument) = 0;
    virtual void singleton_virtual_d8(std::uint32_t argument) = 0;
    // Session result A0h-record vector at system +174h: the object at a
    // record's +0 is deleted through its vtable +0 with flags 1.
    virtual void delete_result_object(void* object) = 0;
    // 00573490 STL_Vector_EraseRange (ECX = vector, whole range).
    virtual void result_vector_erase_all_00573490(void* vector) = 0;
    // 00575150 STL_Vector_PushBack (ECX = vector, A0h record).
    virtual void result_vector_push_back_00575150(void* vector, const void* record) = 0;
    // [00E198B4]+4Ch, the multiplayer session browser screen, and its
    // 0057A9C0 BSP_MultiSessionBrowserScreen_JoinSession(record, 1).
    virtual void* session_browser_screen() = 0;
    virtual void browser_join_session_0057a9c0(void* screen, const void* record, std::uint32_t argument) = 0;
    // 0076A6F0 on ECX = [system +2210h] + 1EF0h with (0, 0).
    virtual void game_1ef0_0076a6f0(void* system, std::uint32_t a, std::uint32_t b) = 0;
    // 00A4B2A0 / 00A4B7F0 matchmaking searches (system +184h).
    virtual std::int32_t matchmaking_begin_search20_00a4b2a0(void* matchmaking, std::uint32_t a,
        std::uint32_t b) = 0;
    virtual std::int32_t matchmaking_begin_search_00a4b7f0(void* matchmaking, const std::uint64_t values[6]) = 0;
};

// 00A47130 BSP_XLiveSystem_AddLocalPlayerRecord, __fastcall(ECX = system).
// Record defaults (above) with user slot = 00A3EAC0, name = 00A3EAE0, XUID
// = 00A3EB00, +34h = 0; push_back into the Live list +100h; the temporary's
// string is released (cap >= 10h -> _free).
void add_local_player_record_00a47130(void* system, OnlineSystemCalls& host);
// 00A47220 BSP_XLiveSystem_AddLanLocalPlayer, __thiscall(system, BYTE slot),
// RET 4. Only when the LAN list size +114h is 0: name = game setup string
// +6A0h when its length is nonzero, else +68Ch (a null buffer reads as ""
// 00F8AC01); XNetGetTitleXnAddr is retried until it stops returning 0
// (pending); id = abEnet (XNADDR +0Ah..+0Fh) + slot; +3Ah (1) is copied to
// system +1D4Ch[slot]; push_back into +10Ch.
void add_lan_local_player_00a47220(void* system, std::uint8_t slot, OnlineSystemCalls& host);
// 00A47AA0 BSP_XLiveSystem_ResetSessionKeepLocalPlayer, __fastcall, AL = 1:
// 00A46D60; a default record (id left uninitialised) is assigned into +128h
// through 006883B0; then 00A47130.
bool reset_session_keep_local_player_00a47aa0(void* system, OnlineSystemCalls& host,
    XLiveSearchCalls& search);
// 00A48330 BSP_XLiveSystem_JoinSessionRecord (was EnsureLocalPlayerListed),
// __thiscall(system, const A0h record*), RET 4. Only while the session
// browser screen exists: an empty Live list gets 00A47130; singleton vtable
// +1A0h(0) and +D8h(1); the result vector is cleared (objects deleted),
// the record pushed, 0057A9C0(record, 1) on the screen, byte +4Ch = 1.
void join_session_record_00a48330(void* system, const void* record, OnlineSystemCalls& host);
// 00A46D60 BSP_XLiveSystem_LeaveSession, __fastcall. While searching (+EDh):
// vtable +8Ch; matchmaking state +1AC0h in {1, 2} -> 00A4B250 (+184h); host
// QoS in use (+EFh) with state +1B74h in {1, 2} -> 00A4AFD0 (+1AE8h). Then
// session state +21F0h in {4, 2} -> vtable +170h(0); both player lists are
// cleared; every result object is deleted and the results erased.
void leave_session_00a46d60(void* system, OnlineSystemCalls& host, XLiveSearchCalls& search);

// The search request 00A46F60 reads (layout from its listing 00A4703C..00A470E0).
struct SessionSearchRequest {
    std::uint8_t field_00;
    std::uint8_t use_host_qos;     // +01h
    std::uint8_t search_by_id;     // +02h
    std::uint8_t pad_03[5];
    std::uint32_t qos_arg_08;      // +08h \ forwarded to 00A4A3D0, which ignores them
    std::uint32_t qos_arg_0c;      // +0Ch /
    std::uint32_t search20_arg_10; // +10h \ forwarded to 00A4B2A0
    std::uint32_t search20_arg_14; // +14h /
    std::uint8_t has_value_1c;     // +18h
    std::uint8_t pad_19[3];
    std::uint32_t value_1c;        // +1Ch
    std::uint8_t has_value_24;     // +20h
    std::uint8_t pad_21[3];
    std::uint32_t value_24;        // +24h
    std::uint32_t value_28;        // +28h
    std::uint32_t value_2c;        // +2Ch
    std::uint32_t value_30;        // +30h
};

// 00A46F60 BSP_XLiveSystem_BeginSessionSearch, __thiscall(system, const
// request*), RET 4, AL. Already searching (+EDh): AL = 0, nothing else.
// Otherwise +EEh = 0 and the results are cleared. When vtable +40h is false
// or vtable +19Ch true: 0076A6F0(0, 0) on the game, +EDh = 1, AL = 1.
// Otherwise the request picks one of: host QoS (+EFh = 1, +F0h = 0,
// 00A4A3D0), search by id (+EFh = 0, +F0h = 1, 00A4B2A0(+10h, +14h)), or
// the filtered search (+EFh = +F0h = 0, 00A4B7F0 with six qwords: +30h,
// +2Ch, +28h zero-extended, then +24h and +1Ch when their flags are set or
// INT64_MAX (7FFFFFFF:FFFFFFFF) when not, and INT64_MAX). A negative HRESULT
// is stored at +1BD8h with +1BD5h = 0 and AL = 0; success sets +EDh = 1, AL = 1.
bool begin_session_search_00a46f60(void* system, const SessionSearchRequest& request,
    OnlineSystemCalls& host, XLiveQosHost& qos);

// XSession wrapper (system +2098h). Fields: +A8h..+B7h the key-exchange key
// copy, +B8h the XNKID copy, +C7h an XSESSION_INFO (XNKID +C7h, XNADDR +CFh,
// XNKEY +F3h; the odd offset is what the listing addresses), +108h nonce,
// +110h XOVERLAPPED (status 3E5h = ERROR_IO_PENDING), +12Ch session handle,
// +130h QoS listening, +158h state, +15Ch key registered, +160h restart flag.
struct XSessionLifetimeCalls {
    virtual ~XSessionLifetimeCalls() = default;
    virtual std::int32_t xnet_qos_listen(const void* key, const void* data, std::uint32_t bytes,
        std::uint32_t bits_per_second, std::uint32_t flags) = 0;               // 00A4D5F6
    virtual std::int32_t xnet_unregister_key(const void* xnkid) = 0;         // XNetUnregisterKey
    virtual std::uint32_t x_close_handle(void* handle) = 0;                  // 00A4D4A6
    virtual std::uint32_t x_get_overlapped_extended_error(void* overlapped) = 0; // 00A4D422
    virtual std::uint32_t x_get_overlapped_result(void* overlapped, std::uint32_t* result,
        std::uint32_t wait) = 0;                                             // 00A4D42E
    virtual void log(const char* format, std::int32_t value) = 0;            // 004254B0
    // 004254B0 with the eight XNKID bytes.
    virtual void log_key(const char* format, const std::uint8_t key[8]) = 0;
    virtual void* game_setup() = 0;                                          // [00E188A8]
    virtual const std::uint8_t* qos_title_data() = 0;                        // [00E0E3F0] (00D24A38)
};

// 00A49C80 BSP_XLiveSession_StopQosListen, __fastcall, HRESULT: when +130h is
// set, XNetQosListen(+C7h, 0, 0, 0, 10h = RELEASE); failure logs "Warning:
// Failed to stop QoS listener, error 0x%08x\n" and returns the error with
// +130h kept; success clears +130h. Returns 0 otherwise.
std::int32_t session_stop_qos_listen_00a49c80(void* session, XSessionLifetimeCalls& x);
// 00A4A790 BSP_XLiveSession_TerminateXNet, __fastcall, and its JMP thunk
// 00A4B020: +158h = +160h = 0; the QoS release as 00A49C80 (inlined, error
// only logged); when +15Ch is set, log the XNKID (+B8h..+BFh) with "%x%x%x%x
// %x%x%x%x UnregisterKey cNetworkConsole::TerminateXNet() ",
// XNetUnregisterKey(+B8h) and +15Ch = 0.
void session_terminate_xnet_00a4a790(void* session, XSessionLifetimeCalls& x);
void session_terminate_xnet_00a4b020(void* session, XSessionLifetimeCalls& x);
// 00A4AF20 BSP_XLiveSession_CompleteDelete, __fastcall, EAX: 0 while the
// overlapped is pending; else XCloseHandle(+12Ch), +12Ch = -1, and the
// overlapped extended error is returned; 0 also clears +158h and +160h.
std::int32_t session_complete_delete_00a4af20(void* session, XSessionLifetimeCalls& x);
// 00A49A00 BSP_XLiveSession_FlagRestart_Provisional (called, not in this
// packet): state 1/2 -> +160h = 1, state 4 -> state 2, else 8000FFFFh.
struct XSessionCreateCalls {
    virtual ~XSessionCreateCalls() = default;
    virtual std::int32_t join_local_player_00a4ae70(void* session) = 0;
    virtual std::int32_t flag_restart_00a49a00(void* session) = 0;
};
// 00A4B030 BSP_XLiveSession_CompleteCreate, __fastcall, HRESULT. Pending: 0.
// A nonzero extended error: XGetOverlappedResult(ovl, &dummy, FALSE), the
// error is read again and discarded, return 8000FFFFh. Success: XNKID +C7h
// -> +B8h, XNKEY +F3h -> +A8h, nonce +108h -> game setup +2158h, state 4,
// XNetQosListen(+C7h, title data + 1, 2 * wcslen(title data + 2), 0, 4 or 5
// when not yet listening): the data starts one byte before the wide string
// L"Battlestations..." at 00D24A3A, so it is misaligned by a byte. Success
// sets +130h = 1, failure logs "Failed to start QoS listener, error
// 0x%08x\n". Then 00A4AE70; when +160h is set, return 00A49A00, else 0.
std::int32_t session_complete_create_00a4b030(void* session, XSessionLifetimeCalls& x,
    XSessionCreateCalls& create);

// ---------------------------------------------------------------------------
// Voice initialisation
// ---------------------------------------------------------------------------

// XHV_INIT_PARAMS as 00A493C0 builds it on the stack (ESP+28h..+4Ch).
struct XhvInitParams {
    std::uint32_t max_remote_talkers;        // 10h
    std::uint32_t max_local_talkers;         // 1
    const std::uint32_t* local_modes;        // {2 voice chat, 1 loopback}
    std::uint32_t local_mode_count;          // 2
    const std::uint32_t* remote_modes;       // {2}
    std::uint32_t remote_mode_count;         // 1
    std::uint32_t custom_vad;                // 0
    std::uint32_t relax_privileges;          // 0
    void* microphone_callback;               // 0
    void* focus_window;                      // 00BEC230([0109CF04])
};

struct VoiceInitCalls {
    virtual ~VoiceInitCalls() = default;
    // [00F8ABE8] 00A3E510 (AL), 00A3EAD0 (sign-in state), 00A3EB30(BYTE* out) (AL).
    virtual bool has_selected_user_00a3e510() = 0;
    virtual std::int32_t signin_state_00a3ead0() = 0;
    virtual bool communication_privilege_00a3eb30(std::uint8_t* out) = 0;
    virtual void* focus_window_00bec230() = 0;
    virtual std::int32_t xhv_create_engine(const XhvInitParams& params, void** engine) = 0; // 00A4D5F0
    virtual std::uint32_t get_tick_count() = 0;
    virtual void* xnotify_create_listener(std::uint64_t areas) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void shutdown_00a49270(void* voice) = 0;
};

// 00A493C0 BSP_XLiveVoice_Initialize, __fastcall(ECX = voice), HRESULT. No
// selected user: 0. Sign-in state 2 (Live): 0 when 00A3EB30 fails (its
// output byte is not read). Then +84h, +C4h, +108h, +4h, +0h = 0 and
// XHVCreateEngine(params, 0, &engine +78h); a failure HRESULT is returned
// as is. Success: +70h = GetTickCount(); XNotifyCreateListener(1 =
// XNOTIFY_SYSTEM) -> +10Ch; a valid listener gets the 380h-byte talker array
// (16 x 38h) at +80h, +74h = 0, +104h = 1 and, when Xenon slot state(0) is
// nonzero, +4h = IsHeadsetPresent +34h(0) with +0h = 1; return 0. A null or
// INVALID_HANDLE_VALUE listener, or a failed allocation, runs 00A49270 and
// returns E_OUTOFMEMORY (8007000Eh).
std::int32_t voice_initialize_00a493c0(void* voice, VoiceInitCalls& host, XenonSystemCalls& xenon,
    Xhv2EngineCalls& xhv);

// ---------------------------------------------------------------------------
// Session creation and user properties (ECX = session, system +2098h)
// ---------------------------------------------------------------------------

// 00A4A890 BSP_XLiveSession_SetUserProperties, __thiscall(session, const
// DWORD settings[0Eh], int p3, int p4, int64 p5 (two dwords), int p7, int
// p8, BYTE force), RET 20h. Each XUserSetProperty(active user, id, size, &v)
// runs when its value differs from the stored copy or `force` is set:
//   mode block (stored +58h vs settings[2]): 100000CDh = (old mode <= 3),
//     100000C9h = (old == 4), 100000CAh (5), 100000CBh (6), 100000CCh (7).
//     The values come from the OLD +58h; settings are stored only at the end,
//     so the mode properties lag one call behind (listing 00A4A8A6..00A4A96C).
//   10000048h p3 (+44h), 10000047h p4 (+40h), 100000D9h p7 (+48h), 100000DAh
//   p8 (+4Ch), 200000DBh qword +A0h (vs +98h), 200000D8h qword +B8h (the
//   XNKID, vs +90h), 200000D7h p5 (vs +88h), then settings[3..0Ch] vs
//   +5Ch..+80h with ids D0, D6, D3, D5, D5, D1, D4, CF, CE, D2 (all 1000xxxxh).
//   Settings[6] and [7] both publish 100000D5h (00A4AAF8, 00A4AB18), so [7]
//   overwrites [6].
// Afterwards +44h = p3, +40h = p4, +4Ch = p8, +48h = p7, +50h..+87h =
// settings, +90h = +B8h, +98h = +A0h (qwords), +88h = p5.
struct SessionPropertyCalls {
    virtual ~SessionPropertyCalls() = default;
    virtual std::uint32_t active_user_slot_00a3eac0() = 0;
    virtual void xuser_set_property(std::uint32_t user, std::uint32_t id, std::uint32_t bytes, const void* value) = 0;
};
void session_set_user_properties_00a4a890(void* session, const std::uint32_t settings[14], std::int32_t p3,
    std::int32_t p4, std::uint64_t p5, std::int32_t p7, std::int32_t p8, bool force, SessionPropertyCalls& host);

struct SessionCreateCalls {
    virtual ~SessionCreateCalls() = default;
    virtual std::uint32_t active_user_slot_00a3eac0() = 0;
    virtual void xuser_set_context(std::uint32_t user, std::uint32_t id, std::uint32_t value) = 0; // 00A4D3F2
    virtual void xuser_set_property(std::uint32_t user, std::uint32_t id, std::uint32_t bytes, const void* value) = 0;
    // XSessionCreate (00A4D524)(flags, user, public slots, private slots,
    // &nonce, &session info, overlapped, &handle).
    virtual std::uint32_t xsession_create(std::uint32_t flags, std::uint32_t user, std::uint32_t public_slots,
        std::uint32_t private_slots, void* nonce, void* info, void* overlapped, void* handle) = 0;
    virtual void log(const char* text) = 0;                // 004254B0
    virtual std::uint8_t system_byte_4d() = 0;             // [00F8A2FC] + 4Dh
    virtual const void* xenon_system() = 0;                // [00F8ABE8] (+358h, +35Ch)
    virtual const void* game_setup() = 0;                  // [00E188A8]
};
// 00A4AC40 BSP_XLiveSession_Create, __fastcall, EAX = 0 always. A nonzero
// state +158h is cleared with +160h; the XOVERLAPPED +110h..+12Bh is zeroed;
// XUserSetContext(user, 800Bh, 0); with system +4Dh set: flags 43Fh and
// context 800Ah = 0, else flags 42Fh and 800Ah = 1; log "CSession::Create()
// -- >  XSessionCreate"; XSessionCreate(flags, user, +34h, +3Ch, &+108h,
// &+C7h, &+110h, &+12Ch); the 10000030h property from setup +5FCh +1098h
// when +5FCh is set (00A4A850 inlined); 00A4A890(setup +2020h, (char) setup
// +201Ch, (char) +201Dh, qword setup +2060h, max(xenon +358h, setup +73Ch),
// max(xenon +35Ch, setup +740h), force = 1) (unsigned maxima). Only
// ERROR_IO_PENDING sets +C4h..+C6h = 0 and state 1; any other result leaves
// the state alone.
std::uint32_t session_create_00a4ac40(void* session, SessionCreateCalls& host, SessionPropertyCalls& properties);

// 00A4BB00 BSP_XLiveSessionInfo_FillFromGameSetup, __thiscall(session, const
// block*), RET 4. The block is game setup +1FF0h in the callers that pass
// it (its +2Ch/+2Dh/+30h/+70h line up with the setup +201Ch/+201Dh/+2020h/
// +2060h that 00A4AC40 reads). Name: the C string at +0 widened (004C5E60),
// cut to 16 characters when longer (00770E50 + 004C5E20) and copied with its
// terminator to +0. Then +28h = (int64)(char)+27h, +30h = c28 - c2A, +34h =
// c2A - c28 - c29 + c27, +38h = c2A, +3Ch = c29 - c2A (chars sign-extended),
// +88h = qword +70h, +40h = (char)+2Dh, +44h = (char)+2Ch, +48h = qword
// +78h, +A0h = qword +80h, +50h..+87h = the 0Eh dwords at +30h.
struct SessionInfoCalls {
    virtual ~SessionInfoCalls() = default;
    // 004C5E60: the native wide string built from a C string (UTF-16 units).
    virtual std::u16string widen_004c5e60(const char* text) = 0;
};
void session_info_fill_from_game_setup_00a4bb00(void* session, const void* block, SessionInfoCalls& host);

// ---------------------------------------------------------------------------
// Matchmaking search (ECX = matchmaking, system +184h)
// ---------------------------------------------------------------------------

// XUSER_DATA / XUSER_PROPERTY / XUSER_CONTEXT as the image lays them out.
struct XUserData {
    std::uint8_t type;      // 1 = int32, 2 = int64
    std::uint8_t pad[7];
    std::int64_t value;     // +8h
};
struct XUserProperty {
    std::uint32_t id;
    std::uint32_t pad;
    XUserData data;         // +8h
};
struct XUserContext {
    std::uint32_t id;
    std::uint32_t value;
};
// The search arguments BeginSearch20 and BeginSearch fill in, BSS 00F8AC08..
// 00F8ACF7, followed by the query table 00F8ACF8..00F8AEE7 (31 rows of 10h:
// a pooled native string {length, char*} then the query id). The image fills
// the table at static initialisation; the C++ takes it from the host.
struct MatchmakingSearchGlobals {
    XUserContext contexts20[2];   // 00F8AC08
    std::uint8_t gap_18[8];
    XUserProperty property20;     // 00F8AC20
    XUserContext contexts[2];     // 00F8AC38
    std::uint8_t gap_48[8];
    XUserProperty properties[7];  // 00F8AC50
    struct QueryRow {
        std::uint32_t key_length;
        const char* key;
        std::uint32_t query_id;
        std::uint32_t pad;
    } query_table[31];            // 00F8ACF8
};

struct MatchmakingSearchCalls {
    virtual ~MatchmakingSearchCalls() = default;
    virtual std::uint32_t active_user_slot_00a3eac0() = 0;
    // XSessionSearchEx (00A4D608)(procedure, user, results, users, property
    // count, context count, properties, contexts, DWORD* bytes, results, overlapped).
    virtual std::uint32_t xsession_search_ex(std::uint32_t procedure, std::uint32_t user, std::uint32_t results,
        std::uint16_t users, std::uint16_t property_count, std::uint16_t context_count,
        const XUserProperty* properties, const XUserContext* contexts, std::uint32_t* bytes,
        void* buffer, void* overlapped) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void log(const char* format, const char* text) = 0; // 004254B0
    virtual void log_text(const char* text) = 0;                // 004254B0, no argument
    virtual std::uint8_t system_byte_4d() = 0;                  // [00F8A2FC] + 4Dh
    virtual const char* query_name_00f8aefc() = 0;              // [00F8AEFC]
};

// 00A4B490 BSP_XLiveMatchmaking_SelectQueryId, __stdcall(uint64 mode mask),
// RET 8, EAX. Mode letters: 80h C (competitive), 10h D (duel), 20h E
// (escort), 40h S (siege), any of 0Fh I (island capture). Mask 0: 0. One
// letter: C 3, D 1, E 2, S 4, I 5. Two to four letters: the letters in the
// order C D E S I form a pooled string, and the first table row 5..30 whose
// key starts with it (0043E9A0 is a prefix test) gives the id; no row: 0.
// All five letters: 0.
std::uint32_t matchmaking_select_query_id_00a4b490(std::uint64_t mask, const MatchmakingSearchGlobals& globals);

// 00A4B2A0 BSP_XLiveMatchmaking_BeginSearch20, __thiscall(matchmaking, uint64
// session id), RET 8, HRESULT. Logs "MatchMaking Query ID %s" with
// [00F8AEFC] (or ""). A busy state +193Ch runs 00A4AF70 and resets it (the
// re-test that follows can never fail). XOVERLAPPED +1944h zeroed; procedure
// 20h, 19h results, 1 user: a size probe (null buffer) must return
// ERROR_INSUFFICIENT_BUFFER (7Ah) with a nonzero size, the buffer goes to
// +1960h, then property 200000D8h (int64) = the id and contexts {800Ah,
// system +4Dh == 0} {800Bh, 0}; ERROR_IO_PENDING sets state 1 and returns
// 0. Every other path returns 8000FFFFh (a buffer already bought stays at
// +1960h).
std::int32_t matchmaking_begin_search20_00a4b2a0(void* matchmaking, std::uint64_t session_id,
    MatchmakingSearchGlobals& globals, MatchmakingSearchCalls& host, XLiveSearchCalls& search);

// 00A4B7F0 BSP_XLiveMatchmaking_BeginSearch, __thiscall(matchmaking, six
// uint64), RET 30h, HRESULT. Only q0 (the mode mask), the low dword of q1
// and the low dword of q2 are read; q3..q5 are ignored. Procedure = 00A4B490
// (q0); log "MatchMaking Query ID %s" with that table row's key. Same
// cancel / probe / allocate sequence as 00A4B2A0, then properties 10000047h
// = q2 and 10000048h = q1 (int32), and for each mode a 0 property (C
// 100000CCh, D C9h, E CAh, S CBh, I CDh, each logged "MatchMaking Query Set
// Mode ...") when (mode bit set) == single, where single = (id in 1..5): a
// single-mode procedure adds no mode property, a combined one adds a 0 for
// every mode left out. Contexts as in 00A4B2A0.
std::int32_t matchmaking_begin_search_00a4b7f0(void* matchmaking, const std::uint64_t q[6],
    MatchmakingSearchGlobals& globals, MatchmakingSearchCalls& host, XLiveSearchCalls& search);

// ---------------------------------------------------------------------------
// Search and QoS polling
// ---------------------------------------------------------------------------

struct SearchPollCalls {
    virtual ~SearchPollCalls() = default;
    virtual std::uint32_t x_get_overlapped_extended_error(void* overlapped) = 0;     // 00A4D422
    virtual std::uint32_t x_get_overlapped_result(void* overlapped, std::uint32_t* result,
        std::uint32_t wait) = 0;                                                    // 00A4D42E
    // XNetGetXnAddrPlatform (00A4D434)(const XNADDR*, DWORD* platform).
    virtual std::uint32_t xnet_get_xnaddr_platform(const void* xnaddr, std::uint32_t* platform) = 0;
    // XNetQosLookup (00A4D5FC)(count, xnaddr*[], xnkid*[], xnkey*[], 0, 0, 0,
    // probes, bits per second, 0, 0, XNQOS** out).
    virtual std::int32_t xnet_qos_lookup(std::uint32_t count, void* addresses, void* ids, void* keys,
        std::uint32_t probes, std::uint32_t bits_per_second, void** qos) = 0;
};

// 00A49DF0 BSP_XLiveMatchmaking_PollSearch, __fastcall(ECX = matchmaking),
// HRESULT. State +193Ch: 0 -> 0; 4 -> +1940h; 3 -> attach every completed
// XNQOSINFO (bFlags & 1) to a result whose +F2h is still null, then with no
// QoS lookups pending (XNQOS +4h) state 0Ch and +1940h, else 3E5h; 2 -> once
// nothing is pending, results whose QoS lacks bFlags 2 (contacted) or has 4
// (disabled) are removed (00A49920) and the rest get their info pointer,
// state 0Ch, +1940h; still pending: 3E5h. State 1 (the search): pending
// overlapped -> 0; an extended error -> XGetOverlappedResult, the error read
// again, 8000FFFFh. Completion: state 4, and each XSESSION_SEARCHRESULT (5Ch)
// in the +1960h buffer whose host XNADDR platform is not 1 becomes an F6h
// record (below); +1808h = the kept count, the pointer arrays +180Ch/+1870h/
// +18D4h are filled and XNetQosLookup(kept, ..., 8 probes, 64000 bps,
// &+1938h) moves to state 3 on success. Returns 0.
// Record (F6h): +0..+0Fh = 0, +10h = open + filled slots (public + private),
// +14h = 0, +18h = open public, +1Ch = 0, +20h = wcscpy of any UNICODE (type
// 4) property, unbounded; property values: 10000030h -> +42h and +5Ah,
// 10000048h +46h, 10000047h +4Ah, 100000D9h +4Eh, 100000DAh +52h, D0 +62h,
// D6 +66h, D3 +6Ah, D5 +6Eh AND +72h (both tests use D5), D1 +76h, D4 +7Ah,
// CF +7Eh, CE +82h, D2 +86h, qwords 200000D7h +8Eh, 200000D8h +96h,
// 200000DBh +9Eh; mode +5Eh = 4/6/5/7/0 for C9/CB/CA/CC/CD == 1 (later tests
// win), never initialised first, and 0 is replaced by the D0 value; XNKID
// +A6h, XNKEY +AEh, XNADDR +BEh, the four slot counts +E2h..+EEh, QoS info
// pointer +F2h = 0. This is the session-side layout of 00A4A890 shifted by 6.
std::int32_t matchmaking_poll_search_00a49df0(void* matchmaking, SearchPollCalls& x);

// 00A4A4E0 BSP_XLiveHostQos_Poll, __fastcall(ECX = host QoS), HRESULT. The
// same machine over the 76h-byte entries at +0 (count +78h, XNQOS +88h, state
// +8Ch, result +90h, XOVERLAPPED +94h, search header pointer +B0h) with
// these differences: completion states go to 4, not 0Ch; state 1 sets state 4
// before copying, takes every result (no platform filter), copies XNKID +26h,
// XNKEY +2Eh, XNADDR +3Eh, the slot counts +62h..+6Eh, clears +72h, and runs
// XNetQosLookup with 0 probes (state 2 on success). The entry array has room
// for one entry before the count at +78h, and the three pointer arrays
// overlap (see 00A4A420), so a second result overwrites live fields.
std::int32_t host_qos_poll_00a4a4e0(void* qos, SearchPollCalls& x);

// ---------------------------------------------------------------------------
// Session browser records and the per-frame update
// ---------------------------------------------------------------------------

// The A0h-byte session record of the result vector at system +174h, as
// 00A48450 (host and matchmaking results) and 00A488D0 (LAN servers) fill
// it. Offsets not listed stay as the record constructor left them.
//   +00h object*: new(88h) with vtable 00CEED04: +4h XNKID, +0Ch XNKEY,
//        +1Ch XNADDR (null for LAN records)
//   +04h BYTE  index: 0FFh for the hosted entry, the server index for LAN
//   +08h std::wstring name (MSVC9: buffer +0Ch, size +1Ch, capacity +20h)
//   +24h int   property 10000030h
//   +28h int   filled slots (public + private)
//   +2Ch int   total slots
//   +30h int   median RTT in ms (XNQOSINFO +0Eh), LAN: 0
//   +38h DWORD up bits per second (XNQOSINFO +10h), LAN: 7D0000h
//   +3Ch DWORD down bits per second (XNQOSINFO +14h), LAN: 7D0000h
//   +48h BYTE  property 10000047h, +49h BYTE property 10000048h
//   +4Ch DWORD settings[0Eh] (the 100000xxh block of 00A4A890)
//   +88h qword property 200000D7h, +90h 100000D9h, +94h 100000DAh,
//   +98h qword 200000DBh
struct SessionRecordCalls {
    virtual ~SessionRecordCalls() = default;
    // 005707A0 (00A48450) and 00571E50 (00A488D0) on a stack record.
    virtual void session_record_init_005707a0(void* record) = 0;
    virtual void session_record_construct_00571e50(void* record) = 0;
    // 00BF681B operator new(88h); the C++ writes vtable 00CEED04 itself.
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 0067D5A0 (assign count units) / 0067D6B0 (assign NUL-terminated) on
    // the record's wide string at +8h.
    virtual void wstring_assign_0067d5a0(void* string, const char16_t* text, std::uint32_t count) = 0;
    virtual void wstring_assign_z_0067d6b0(void* string, const char16_t* text) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void result_vector_push_back_00575150(void* vector, const void* record) = 0;
    virtual void result_vector_erase_all_00573490(void* vector) = 0;
    virtual void delete_result_object(void* object) = 0;
    // MultiByteToWideChar(CP_ACP, 0, text, length, out, 32h) (IAT 00CE2268).
    virtual void multi_byte_to_wide(const char* text, std::int32_t length, char16_t* out, std::int32_t units) = 0;
};

// 00A48450 BSP_XLiveSystem_RefreshHostQosUi, __fastcall(ECX = system). The
// result objects are deleted and the vector erased. Host QoS done (state
// +1B74h == 4) with an entry (+1B60h): one record with index 0FFh from the
// entry at +1AE8h (XNKID +26h, XNKEY +2Eh, XNADDR +3Eh, the wide name at +0,
// property 10000030h from +22h, QoS figures from the info at +72h when set),
// pushed; then 00A4AFD0 and +EDh = 0. Otherwise one record per matchmaking
// result (+198Ch of them, F6h each from +184h): XNKID/XNKEY/XNADDR, the name
// at +20h, filled = +EAh + +EEh, total +10h, 10000030h +42h, bytes +4Ah and
// +46h, settings +56h..+8Bh, D7 +8Eh, D9 +4Eh, DA +52h, DB +9Eh, QoS figures
// from +F2h; then 00A4B250 and +EDh = 0. Index +4h is 0FFh for every record.
void refresh_session_records_00a48450(void* system, SessionRecordCalls& host, XLiveSearchCalls& search);

struct SystemUpdateCalls {
    virtual ~SystemUpdateCalls() = default;
    virtual bool system_virtual_19c(void* system) = 0;
    virtual bool system_virtual_40(void* system) = 0;
    virtual void system_virtual_44(void* system) = 0;
    virtual bool singleton_virtual_198() = 0;                    // [00F8A2FC] vtable +198h
    virtual void session_delete_00a4adc0(void* session) = 0;     // BSP_XLiveSession_Delete
    virtual void session_poll_overlapped_00a4b1f0(void* session) = 0;
    // [00E198B4] and its screen +4Ch: 005789C0(6) BuildRows, 00688AD0(1Ah).
    virtual void* multi_menu() = 0;
    virtual void browser_build_rows_005789c0(void* screen, std::uint32_t mode) = 0;
    virtual void multi_menu_push_request_00688ad0(void* menu, std::uint32_t request) = 0;
    // [00E188A8] game setup; its +207Ch network client (+9Ch state, +C8h
    // 00A39750 PollCreateResult, +A0h LAN server vector).
    virtual void* game_setup() = 0;
    virtual std::int32_t client_poll_create_result_00a39750(void* client_c8) = 0;
    virtual void game_1ef0_0076a7a0(void* game_1ef0, std::uint32_t argument) = 0;
    // The LAN server vector at client +A0h: element count and the server
    // info (element->+20h, a game-setup block like 00A4BB00's).
    virtual std::uint32_t lan_server_count(void* client) = 0;
    virtual const void* lan_server_info(void* client, std::uint32_t index) = 0;
    // 00566310 / 005663B0: size / at() of the result vector (system +174h).
    virtual std::uint32_t result_vector_size_00566310(void* vector) = 0;
    virtual void* result_vector_at_005663b0(void* vector, std::uint32_t index) = 0;
    virtual void result_vector_clear_00573c90(void* vector) = 0;
    virtual void* xenon_system() = 0;                            // [00F8ABE8]
    virtual void xenon_request_upload_00a3f4a0(void* xenon, std::uint32_t a, std::uint32_t b) = 0;
    virtual void update_friends_and_voice_00a46480(void* system, float dt) = 0;
    virtual void refresh_session_records_00a48450(void* system) = 0;
    virtual std::int32_t matchmaking_poll_00a49df0(void* matchmaking) = 0;
    virtual std::int32_t host_qos_poll_00a4a4e0(void* qos) = 0;
    virtual std::int32_t matchmaking_relink_00a49d20(void* matchmaking) = 0;
    virtual std::int32_t host_qos_relink_00a4a420(void* qos) = 0;
    virtual void matchmaking_reset_00a4b250(void* matchmaking) = 0;
    virtual void host_qos_cancel_00a4afd0(void* qos) = 0;
};

// 00A488D0 BSP_XLiveSystem_Update, __thiscall(system, float dt), RET 4.
//  1. lan = vtable +19Ch. A pending delete (+220Eh) with session state
//     +21F0h != 2 runs 00A4ADC0 and clears +220Eh.
//  2. Searching (+EDh), not LAN: done when (+EFh ? host state +1B74h :
//     matchmaking state +1AC0h) == 4 -> +EDh = 0.
//  3. Not by id (+F0h), not searching, not LAN, records stale (+EEh): when
//     (+EFh ? +1B74h == 4 : +1AC0h == 0Ch): +EEh = 0, 00A48450, and the
//     browser screen rebuilds its rows (005789C0(6)).
//  4. A search state in 1..3 (either object): poll the active one (+EFh
//     picks host QoS). 0: when +EEh is clear and +EDh set, matchmaking
//     without +F0h relinks (00A49D20), host QoS relinks (00A4A420), and +EEh
//     = 1; 00A48450 in every case. Negative: cancel/reset, +EDh = 0.
//     Otherwise, searching in LAN mode with setup +2210h->+1FE4h == 2: the
//     result objects are deleted and the vector cleared, and every LAN
//     server with open slots (c2A - c29 - c28 + c27 > 0) becomes a record
//     (name through MultiByteToWideChar, at most 32h units).
//  5. Byte +4Ch with client state 6 and 00A39750 == 217h: 0076A7A0 on the
//     game with the screen's +24h, menu request 1Ah, +4Ch = 0, return.
//  6. vtable +40h false -> vtable +44h; 00A4B1F0; session state 4 or 2
//     sets +220Dh once.
//  7. Singleton +198h true: Xenon +12Ch == 4 -> setup +73Ch/+740h = the
//     unsigned maxima with Xenon +358h/+35Ch, Xenon +12Ch = 1, and
//     00A3F4A0(a, b) unless Xenon already holds both; == 0Ah -> setup
//     values 0 and 00A3F4A0(0, 0).
//  8. Voice active (+1D28h): 00A46480(dt).
void system_update_00a488d0(void* system, float dt, SystemUpdateCalls& host, SessionRecordCalls& records);

// ---------------------------------------------------------------------------
// End-of-session stats
// ---------------------------------------------------------------------------

struct SessionStatsCalls {
    virtual ~SessionStatsCalls() = default;
    virtual bool system_virtual_19c(void* system) = 0;
    virtual void* game_setup() = 0;                                  // [00E188A8]
    virtual void* singleton() = 0;                                   // [00F8A2FC]
    virtual void stats_fill_from_game_00993220(void* singleton) = 0;
    // A native std::vector of 50h-byte stat records: {alloc, first, last, end}.
    virtual void stat_vector_assign_00a47350(void* destination, const void* source) = 0;
    virtual void stat_vector_sort_00a47560(void* vector) = 0;        // checked [first, last)
    virtual void stat_record_assign_00992f60(void* destination, const void* source) = 0;
    virtual void stat_vector_destroy_00993990(void* first, void* last) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 004C6090 + 004CF800: push_back of the 38h-byte value into the std::list
    // at system +2200h (head +2204h).
    virtual void stats_list_push_back(void* list, const void* value_38) = 0;
    // 00A46150 builds a session message from the list and 0076A9F0(message,
    // 0) sends it through setup +1EF0h; the message is then destroyed
    // inline (vtable 00CE7608, list clear when its +18h byte is 3, tidy,
    // vtable 00CE4974).
    virtual void send_stats_message(void* game_1ef0, void* list) = 0;
    // 004C04C0 BSP_OnlineStats_WriteSessionStats on the game setup, 12 dwords.
    virtual void write_session_stats_004c04c0(void* setup, const std::uint32_t arguments[12]) = 0;
};

// 00A47B80 BSP_XLiveSystem_WriteSessionStats, __fastcall(ECX = system).
// +88h = -1.0 (00D7A260); LAN (vtable +19Ch) stops there. Otherwise +88h =
// 60.0 (00CEB4B0), and the stats session id = setup +207Ch ? client +9CCh :
// system +21C4h; -1 or 0 stops. The singleton's stats block is refreshed
// (00993220) unless its +74h is set, copied into a local vector (00A47350)
// and sorted (00A47560). top = record[0] +10h. Each record i is then copied
// INTO record[0] (00992F60; record[0] is the scratch slot, so the vector ends
// with record[0] = the last record) and record[0] gets +18h = id, +20h/+24h
// = its own +0h/+4h, +28h = system +48h, +30h = system +4Dh, +34h = (+4Dh ?
// count - i - 1 : 0), +38h = (+10h != top) + 1; the 38h bytes from +18h are
// appended to the list at +2200h. When setup +1FE4h == 1 the list goes out
// as a session message. Then the FIRST list node's value C[0..0Dh] (the
// list is never cleared here, so an older node wins) gives
// 004C04C0(id, C2, C3, system +48h, C5, C6, C10, C8, C7, 0, C11, C12), and
// the local vector is destroyed. (Argument order from the pushes at
// 00A47E25..00A47E5D.)
void write_session_stats_00a47b80(void* system, SessionStatsCalls& host);

// 00A47EE0 BSP_XLiveSystem_Construct. Native __thiscall (ECX = 2218h-byte
// system), no stack arguments, RET, EAX = system. The export's __fastcall
// spelling does not imply an EDX input (00A47EFB..00A47F02, 00A4831D..2C).
// New C++ interface; build-only. See docs/OCT18B_MPA.md for listing evidence,
// exact array strides and the constructor-unwind adapter contract.
struct XLiveSystemConstructCalls : PlayerListCalls {
    // STL allocator calls: ECX identifies the owning native list.
    virtual void* participant_head_004c3180(void* list) = 0;
    virtual void construct_mask_list_00a46b80(void* list, std::uint32_t zero) = 0;
    virtual void* stats_head_004c1f50(void* list) = 0;
    // Original CRT EH iterator: completes each element or destroys the
    // completed prefix before propagating. Callback values are native code
    // addresses, not callable pointers in this reconstructed executable.
    virtual void construct_array_00bf7cd1(void* first, std::uint32_t stride,
        std::uint32_t count, std::uint32_t constructor, std::uint32_t destructor) = 0;
    virtual void push_mask_name_00849170(void* vector,
        const NativeLegacySboStringStorage& text) = 0;
    virtual const void* xenon_system_manager_00f8abe8() = 0;
    // Adapter for the outer object's native EH cleanup graph (00CB49F6).
    // This is not the handler's callable ABI.
    // state 18h includes all members; the temporary string is already gone.
    // Must destroy completed members in reverse order and must not throw.
    virtual void unwind_failed_construction_00cb49f6(void* system,
        std::int32_t completed_state) noexcept = 0;
};
void* system_construct_00a47ee0(void* system, XLiveSystemConstructCalls& host,
    online_seg71_cc13::XLiveVoiceCalls& voice_calls,
    online_seg71_cc13::XenonCalls& xenon_calls);
} // namespace bsp::online_cc13
