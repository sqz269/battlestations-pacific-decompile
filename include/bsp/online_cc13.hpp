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
#include <cstdint>

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
    virtual std::int32_t start_remote_processing_10(void* engine, std::uint32_t user,
        const void* modes, std::uint32_t count) = 0;                                   // +10h
    virtual std::int32_t register_local_talker_20(void* engine, std::uint32_t user) = 0; // +20h
    virtual std::uint32_t is_local_talking_38(void* engine, std::uint32_t user) = 0;      // +38h
    virtual std::uint32_t data_ready_flags_34(void* engine, std::uint32_t arg) = 0;       // +34h
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
// engine +0Ch(0, 00D24A34, 1) and +10h(0, 00D24A30, 1) (one processing-mode
// dword each), +108h = 1, word +6Ch = 0. Both engine calls pass user 0, not
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
// +0h are cleared; otherwise engine +34h(0) is compared with +4h and, when it
// changed, stored there with +0h = 1.
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
} // namespace bsp::online_cc13
