#pragma once
// XLive system, network console/client/server and Xenon system manager,
// segment 71 (00A371C0-00A46930), packet cc13-online3 (docs/CC13_ONLINE_3.md).
// Complements include/bsp/online_cc13.hpp (segment 72) and
// include/bsp/network_online_cc13.hpp (cc13-sweepA1).
//
// Every routine takes the native object as void* and reads native offsets.
// Calls go through host interfaces whose methods name the native routine.
// Build-tested only: not ABI-compatible, not game-validated.
#include "bsp/online_cc13.hpp"

#include <cstdint>

namespace bsp::online_seg71_cc13 {

// ---------------------------------------------------------------------------
// XLiveSystem vtable slots 00A42690..00A429D0 (ECX = system)
// ---------------------------------------------------------------------------

struct XLiveSlotCalls {
    virtual ~XLiveSlotCalls() = default;
    virtual std::uint32_t xnet_get_ethernet_link_status() = 0;   // 00A4D4CA
    virtual bool system_virtual_19c(void* system) = 0;
    virtual bool system_virtual_180(void* system) = 0;
    virtual bool system_virtual_40(void* system) = 0;
    virtual void system_virtual_44(void* system) = 0;
    virtual void system_virtual_54(void* system) = 0;
    virtual void system_virtual_58(void* system) = 0;
    virtual const std::uint8_t* qos_title_data() = 0;            // [00E0E3F0]
    virtual std::int32_t session_start_qos_listen_00a49c10(void* session, const void* data,
        std::uint32_t bytes, std::uint32_t bits_per_second) = 0;
    virtual std::int32_t session_stop_qos_listen_00a49c80(void* session) = 0;
    virtual void session_delete_00a4adc0(void* session) = 0;
    virtual void voice_remove_remote_talker_00a49680(void* voice, std::uint32_t xuid_lo, std::uint32_t xuid_hi) = 0;
    virtual void voice_set_talker_mute_list_flag_00a49780(void* voice, std::uint32_t a, std::uint32_t b,
        std::uint8_t c, std::uint8_t d) = 0;
    virtual std::uint32_t voice_is_remote_talking_00a49840(void* voice, std::uint32_t xuid_lo,
        std::uint32_t xuid_hi, std::uint8_t slot) = 0;
    virtual void publish_user_property_00a4a850() = 0;
};

// 00A42690: AL = XNetGetEthernetLinkStatus() != 0 (no ECX use).
bool is_ethernet_link_up_00a42690(XLiveSlotCalls& host);
// 00A426A0: EAX = vtable +19Ch (LAN) ? system +10Ch : system +100h.
void* player_list_00a426a0(void* system, XLiveSlotCalls& host);
// 00A426C0 EAX = +10Ch; 00A426D0 EAX = +174h; 00A428A0 EAX = +1F1Ch;
// 00A429A0 EAX = +168h.
void* lan_player_list_00a426c0(void* system) noexcept;
void* result_vector_00a426d0(void* system) noexcept;
void* block_1f1c_00a428a0(void* system) noexcept;
void* list_168_00a429a0(void* system) noexcept;
// Trivial slots: 00A426E0, 00A426F0 RET; 00A42980 RET 10h; 00A42700 and
// 00A42930 AL = 0, RET 0Ch; 00A42860 AL = 0, RET 4; 00A42990 AL = 0, RET.
void nop_00a426e0() noexcept;
void nop_00a426f0() noexcept;
void nop_ret10_00a42980() noexcept;
bool return_false_00a42700() noexcept;
bool return_false_00a42930() noexcept;
bool return_false_00a42860() noexcept;
bool return_false_00a42990() noexcept;
// 00A42710 BSP_XLiveSystem_SetQosListen, __thiscall(system, BYTE stop), RET
// 4, AL. vtable +180h false: 0. stop == 0: +21C8h zero -> 0; else vtable
// +54h, 00A49C10(+2098h, [00E0E3F0]+1, 2*wcslen([00E0E3F0]+2), 0) (the
// misaligned title data of 00A4B030 again) and AL = 1. stop != 0: +21C8h
// zero -> 00A49C80(+2098h) and AL = 1; else 0.
bool set_qos_listen_00a42710(void* system, std::uint8_t stop, XLiveSlotCalls& host);
// 00A427A0 BSP_XLiveSystem_DeleteSession, __thiscall(system, ignored), RET 4,
// AL = 1: byte +85h -> vtable +58h; 00A4ADC0(+2098h); +220Eh = 0.
bool delete_session_00a427a0(void* system, XLiveSlotCalls& host);
// 00A427D0 (RET 8), 00A42800 (RET 10h), 00A42830 (RET 0Ch, AL = result != 0):
// forward to voice +1C24h (00A49680, 00A49780, 00A49840) only while +1D28h
// is set; otherwise nothing / AL = 0.
void remove_remote_talker_00a427d0(void* system, std::uint32_t xuid_lo, std::uint32_t xuid_hi, XLiveSlotCalls& host);
void set_talker_mute_list_flag_00a42800(void* system, std::uint32_t a, std::uint32_t b, std::uint8_t c,
    std::uint8_t d, XLiveSlotCalls& host);
bool is_remote_talking_00a42830(void* system, std::uint32_t xuid_lo, std::uint32_t xuid_hi, std::uint8_t slot,
    XLiveSlotCalls& host);
// 00A42870, RET 4: slot < 4 and byte +1D4Ch[slot] set -> AL = +1C02h[slot],
// else 0.
std::uint8_t slot_flag_1c02_00a42870(const void* system, std::uint8_t slot) noexcept;
// 00A428B0, RET 4: EAX = system + 1D5Ch + index * 1Ch (voice mask record).
void* voice_mask_record_00a428b0(void* system, std::uint8_t index) noexcept;
// 00A428D0: vtable +40h; when it is false, tail-jump to vtable +44h.
void players_dispatch_00a428d0(void* system, XLiveSlotCalls& host);
// 00A428F0 / 00A42900 AL = +2060h[i] / +205Ch[i]; 00A42910 / 00A42920 clear
// them (all RET 4, byte index, no bound).
std::uint8_t flag_2060_00a428f0(const void* system, std::uint8_t i) noexcept;
std::uint8_t flag_205c_00a42900(const void* system, std::uint8_t i) noexcept;
void clear_flag_2060_00a42910(void* system, std::uint8_t i) noexcept;
void clear_flag_205c_00a42920(void* system, std::uint8_t i) noexcept;
// 00A42940 (RET 4) AL = i < 4 ? +1C12h[i] : 0; 00A42960 (dword i, byte v,
// RET 8) stores +1C12h[i] = v when i < 4.
std::uint8_t slot_flag_1c12_00a42940(const void* system, std::uint8_t i) noexcept;
void set_slot_flag_1c12_00a42960(void* system, std::uint32_t i, std::uint8_t value) noexcept;
// 00A429D0: ECX += 2098h, JMP 00A4A850 (a cdecl routine: the adjustment is
// unused).
void publish_user_property_00a429d0(XLiveSlotCalls& host);

// ---------------------------------------------------------------------------
// XLiveSystem session control 00A42A30..00A43650 (ECX = system)
// ---------------------------------------------------------------------------

struct XLiveSessionControlCalls {
    virtual ~XLiveSessionControlCalls() = default;
    virtual bool system_virtual_19c(void* system) = 0;
    virtual bool system_virtual_170(void* system, std::uint32_t argument) = 0;
    virtual bool system_virtual_198(void* system) = 0;
    virtual void system_virtual_58(void* system) = 0;
    virtual void* game_setup() = 0;                                       // [00E188A8]
    virtual void voice_shutdown_00a49270(void* voice) = 0;
    virtual std::int32_t voice_initialize_00a493c0(void* voice) = 0;
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    virtual std::uint32_t xenon_active_user_slot_00a3eac0() = 0;
    virtual std::int32_t xuser_mute_list_query(std::uint32_t user, std::uint32_t xuid_lo, std::uint32_t xuid_hi,
        std::uint32_t* muted) = 0;                                        // 00A4D5E4
    virtual void multiplayer_set_muted_00771630(void* game_1ef0, std::uint32_t xuid_lo, std::uint32_t xuid_hi,
        bool muted) = 0;
    virtual void game_1ef0_0076a700(void* game_1ef0) = 0;
    virtual std::int32_t client_arbitration_register_00a395e0(void* client_c8) = 0;
    virtual void client_start_session_00a39690(void* client_c8) = 0;
    virtual void client_end_session_00a396b0(void* client_c8) = 0;
    virtual std::int32_t client_virtual_38(void* client) = 0;             // returns a pointer value
    virtual std::int32_t session_arbitration_register_00a49a50(void* session) = 0;
    virtual void session_start_00a49b00(void* session) = 0;
    virtual void session_end_00a49b30(void* session) = 0;
    virtual void session_poll_overlapped_00a4b1f0(void* session) = 0;
    virtual void session_info_fill_00a4bb00(void* session, const void* block) = 0;
    virtual std::int32_t session_create_00a4ac40(void* session) = 0;
    virtual void session_terminate_xnet_00a4a790(void* session) = 0;
    virtual std::int32_t session_flag_restart_00a49a00(void* session) = 0;
    virtual void host_qos_cancel_00a4afd0(void* qos) = 0;
    virtual void matchmaking_reset_00a4b250(void* matchmaking) = 0;
    virtual void stats_fill_from_game_00993220(void* stats) = 0;
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t id) = 0;
};

// 00A42A30 BSP_XLiveSystem_ShutdownVoice: voice active (+1D28h) -> tail
// 00A49270(+1C24h).
void shutdown_voice_00a42a30(void* system, XLiveSessionControlCalls& host);
// 00A42A50 BSP_XLiveSystem_StepCountdown, __thiscall(system, float dt), RET 4:
// with the countdown armed (+85h) and +88h >= 0.0 (00D7A218; NaN fails),
// +88h -= dt and, when 0 > the new value (FLDZ/FCOMIP, NaN fails), vtable
// +58h runs.
void step_countdown_00a42a50(void* system, float dt, XLiveSessionControlCalls& host);
// 00A42AA0: AL = 1.
bool return_true_00a42aa0() noexcept;
// 00A42F90 BSP_XLiveStatsBlock_EnsureLoaded, __fastcall(ECX = stats owner):
// 00993220 unless +74h is set; EAX = +64h.
void* stats_block_ensure_loaded_00a42f90(void* owner, XLiveSessionControlCalls& host);
// 00A42FB0 BSP_VoiceChangeMessage_Construct, __thiscall(message, a, b, BYTE
// c), RET 0Ch, EAX = message: 0075B430(message, 0E8h), +18h = c, +4h = 1,
// vtable 00D03278, +20h = a, +24h = b.
void* voice_change_message_construct_00a42fb0(void* message, std::uint32_t a, std::uint32_t b, std::uint8_t c,
    XLiveSessionControlCalls& host);
// 00A43070 BSP_XLiveTaskStubPair_Construct: +0h = +4h = vtable 00D24764.
// 00A43080 BSP_XLiveTaskStubPair_Destruct: the same stores, each followed by
// the stub destructor 00A48F00 (an AL = 1 body), +4h first.
void* task_stub_pair_construct_00a43070(void* pair) noexcept;
void task_stub_pair_destruct_00a43080(void* pair) noexcept;
// 00A43130 BSP_XLiveSystem_RefreshMuteForPlayer, __thiscall(system, XUID lo,
// hi), RET 8: XUserMuteListQuery(active user, xuid, &muted) with `muted` in
// the low-dword argument slot (read before the call), then 00771630 on
// setup +1EF0h (xuid, muted != 0). The query's result is ignored.
void refresh_mute_for_player_00a43130(void* system, std::uint32_t xuid_lo, std::uint32_t xuid_hi,
    XLiveSessionControlCalls& host);
// 00A43220 BSP_XLiveSystem_ArbitrationRegister, HRESULT: LAN or +4Dh clear ->
// 0; client setup +207Ch -> 00A395E0(client +C8h); setup +2078h ->
// 00A49A50(+2098h); else 8000FFFFh.
std::int32_t arbitration_register_00a43220(void* system, XLiveSessionControlCalls& host);
// 00A43280 BSP_XLiveSystem_CancelSessionSearch, AL: not searching -> 0. LAN:
// 0076A700 on [+2210h]+1EF0h. Otherwise host QoS state 1/2 -> 00A4AFD0 and
// matchmaking state 1/2 -> 00A4B250. Then +EDh = 0, AL = 1.
bool cancel_session_search_00a43280(void* system, XLiveSessionControlCalls& host);
// 00A43300: RET.
void nop_00a43300() noexcept;
// 00A43310 BSP_XLiveSystem_ClearLastError, __thiscall(system, BYTE keep), RET
// 4: keep == 0 clears +1BD5h and +1BD8h.
void clear_last_error_00a43310(void* system, std::uint8_t keep) noexcept;
// 00A43330 BSP_XLiveSystem_HostCreateSession, __thiscall(system, const block*),
// RET 4, AL. Session state 4 or 2 and vtable +170h(0) false: 00A4B1F0 until
// the state leaves 2, then vtable +170h(0) again. +2150h = +2154h = 0;
// 00A4BB00(+2098h, block); +220Ch = (c27 == c29); r = 00A4AC40; +220Dh = 0;
// r < 0 -> 00A4A790, +1BD8h = r, +1BD5h = 0. +1C24h = 1 always. AL = r >= 0.
bool host_create_session_00a43330(void* system, const void* block, XLiveSessionControlCalls& host);
// 00A433F0 BSP_XLiveSystem_HostRefreshSessionInfo and its byte-identical copy
// 00A43460, __thiscall(system, ignored), RET 4, AL: game +1FE4h == 1,
// vtable +198h, session state 4 or 2 -> 00A4BB00(+2098h, game +1FF0h) and AL
// = 00A49A00 >= 0; otherwise 0.
bool host_refresh_session_info_00a433f0(void* system, XLiveSessionControlCalls& host);
bool host_refresh_session_info_00a43460(void* system, XLiveSessionControlCalls& host);
// 00A434D0 BSP_XLiveSystem_HostFlagSession: the same gate, AL = 00A49A00 >= 0.
bool host_flag_session_00a434d0(void* system, XLiveSessionControlCalls& host);
// 00A43520 (EAX = 0) and 00A435B0 (tail, EAX = 00A493C0's HRESULT): voice
// active -> 00A49270; 00A3E510 (result ignored); 00A493C0(+1C24h).
std::int32_t restart_voice_engine_00a43520(void* system, XLiveSessionControlCalls& host);
std::int32_t restart_voice_engine_00a435b0(void* system, XLiveSessionControlCalls& host);
// 00A43560 BSP_XLiveSystem_GetCurrentSessionInfo, EAX: session state 4 or 2
// -> system +2150h; game +1FE4h == 2 with client state +9Ch >= 7 -> client
// vtable +38h() + 4; else system +8Ch.
std::uintptr_t current_session_info_00a43560(void* system, XLiveSessionControlCalls& host);
// 00A435E0 BSP_XLiveSystem_OnCountdownExpired: armed (+85h) -> +85h = 0; then
// setup +1FE4h nonzero and not LAN: 1 -> 00A49B30 (session end), else a
// client with state >= 7 -> 00A396B0(client +C8h).
void on_countdown_expired_00a435e0(void* system, XLiveSessionControlCalls& host);
// 00A43650 BSP_XLiveSystem_ArmCountdown: armed -> vtable +58h. Setup +1FE4h
// nonzero and not LAN: 1 -> 00A49B00 (session start), else 00A39690(client
// +C8h) with no null test; then +85h = 1, +88h = -1.0 (00D7A260). A failed
// gate leaves the countdown alone.
void arm_countdown_00a43650(void* system, XLiveSessionControlCalls& host);

// ---------------------------------------------------------------------------
// MNetworkServerXLive / MNetworkClientXLive wrappers 00A410F0..00A41660.
// Both derive from the MNetwork base (00784xxx/00785xxx) and embed the
// cNetworkServer (+A4h) or cNetworkClient (+C8h) console object.
// ---------------------------------------------------------------------------

struct MNetworkCalls {
    virtual ~MNetworkCalls() = default;
    virtual void mnetwork_queue_incoming_peer_00785790(void* base) = 0;  // JMP target, stack args forwarded
    virtual void mnetwork_receive_packet_007858c0(void* base) = 0;       // JMP target, stack args forwarded
    virtual void server_update_00a3d6f0(void* self, float dt) = 0;
    virtual void client_update_00a39d10(void* self, float dt) = 0;
    virtual void object_virtual(void* object, std::uint32_t slot_offset) = 0; // vtable call, no stack args
    virtual void object_virtual_arg(void* object, std::uint32_t slot_offset, std::uintptr_t argument) = 0;
    virtual std::uint8_t object_virtual_al(void* object, std::uint32_t slot_offset) = 0;
    virtual void mnetwork_destruct_007844e0(void* self) = 0;
    virtual void server_destruct_00a3dcf0(void* server) = 0;
    // [00F8ABDC] the network console sender.
    virtual void request_disconnect_00a3bbb0(void* address, float seconds) = 0;
    virtual float estimate_latency_00a3bda0(void* address, void* out_slot) = 0;
    virtual float profile_value_00a3be60(void* address) = 0;
    virtual std::uint32_t pending_count_00a3bee0(void* address) = 0;
    virtual void mnetwork_client_construct_00785b80(void* self, std::uint32_t argument) = 0;
    virtual void client_construct_00a39570(void* client) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void session_target_construct_007839d0(void* target, std::uint32_t a, std::uint32_t b) = 0;
    virtual void mnetwork_append_peer_00785c10(void* self, void* peer) = 0;
    virtual void mnetwork_client_clear_host_lists_00785170(void* self) = 0;
    virtual void xnet_unregister_key(const void* xnkid) = 0;             // 00A4D488
    virtual void client_destruct_00a39e30(void* client) = 0;
    virtual void mnetwork_client_destruct_00784d60(void* self) = 0;
    virtual void client_open_00a39f70(void* client, std::uint32_t timeout_ms) = 0;
    virtual void* xlive_singleton() = 0;                                 // [00F8A2FC]
    virtual void* game_setup() = 0;                                      // [00E188A8]
};

// Adjustor thunks: 00A410F0 (ECX -= 0A4h, JMP 00785790), 00A41210 (ECX -=
// 0C8h, JMP 007858C0), 00A41250 (ECX -= 0A4h, JMP 007858C0).
void adjustor_00a410f0(void* self, MNetworkCalls& host);
void adjustor_00a41210(void* self, MNetworkCalls& host);
void adjustor_00a41250(void* self, MNetworkCalls& host);
// 00A41100 BeginLanSearch (RET 8, arguments unused): +941h = 1, +944h = 0.
// 00A41120 EndLanSearch: +941h = 0. 00A41130 (not a target) is a bare RET.
void begin_lan_search_00a41100(void* client) noexcept;
void end_lan_search_00a41120(void* client) noexcept;
// 00A41140 BSP_cNetworkClient_WriteLocalAddress, __thiscall(client, out),
// RET 4: out +5Ah = XNADDR +81Ch (24h bytes), out +7Eh = XNKID +840h, out
// +86h = XNKEY +848h (10h bytes).
void client_write_local_address_00a41140(const void* client, void* out) noexcept;
// 00A411A0 BSP_XnAddr_Equals, __stdcall(a, b), RET 8: AL = the eight bytes at
// +24h are equal (byte by byte).
bool xnaddr_equals_00a411a0(const void* a, const void* b) noexcept;
// 00A41220 OnDisconnected, __thiscall(sub, ignored), RET 4: [ECX-2Ch] = 4,
// +81Ah = 1, +880h = 0.
void client_on_disconnected_00a41220(void* sub) noexcept;
// 00A41260 / 00A41270 (RET 4): 00A3D6F0 / 00A39D10 with the same ECX and dt.
void server_update_00a41260(void* self, float dt, MNetworkCalls& host);
void client_update_00a41270(void* self, float dt, MNetworkCalls& host);
// 00A41280 StopReceive: the console object at +C8h, its vtable +0Ch(0).
void client_stop_receive_00a41280(void* self, MNetworkCalls& host);
// 00A41300: vtable +28h, then +9Ch = 2. 00A41320: AL = 1.
void start_hosting_and_set_state2_00a41300(void* self, MNetworkCalls& host);
bool return_true_00a41320() noexcept;
// 00A41360 BSP_MNetworkServerXLive_Destruct: vtables 00D243EC / +0A4h
// 00D243C0, 00A3DCF0(+0A4h), vtable 00D24390, 007844E0.
void server_xlive_destruct_00a41360(void* self, MNetworkCalls& host);
// 00A413D0 / 00A413E0 / 00A413F0: ECX = [ECX-0A0h], JMP its vtable +10h /
// +14h / +8h (stack arguments forwarded).
void forward_slot10_00a413d0(void* sub, MNetworkCalls& host);
void forward_slot14_00a413e0(void* sub, MNetworkCalls& host);
void forward_slot08_00a413f0(void* sub, MNetworkCalls& host);
// 00A41400 DisconnectPeer, __thiscall(self, peer, two unused), RET 0Ch:
// 00A3BBB0(peer +0D74h, 2.0 (00CE3958)) on the sender, then vtable +8(peer).
void server_disconnect_peer_00a41400(void* self, void* peer, MNetworkCalls& host);
// 00A41440 / 00A41BA0-style peer queries, RET 4: a null peer answers 1.0 /
// 1.0 / 100; otherwise 00A3BDA0 (with the argument slot as its output),
// 00A3BE60, 00A3BEE0 on peer +0D74h.
float server_peer_latency_00a41440(void* peer, MNetworkCalls& host);
float server_peer_profile_value_00a41470(void* peer, MNetworkCalls& host);
std::uint32_t server_peer_pending_count_00a414a0(void* peer, MNetworkCalls& host);
// 00A414D0 BSP_MNetworkClientXLive_Construct, __thiscall(self, argument), RET
// 4, EAX = self: 00785B80(self, argument), 00A39570(+0C8h), vtables 00D24448 /
// +0C8h 00D2441C, +9D8h = 00CEED04 (the session-info holder vtable), a new
// 0DC0h target through 007839D0(0, 0) with vtable 00D03860 (null when the
// allocation fails) into +93Ch, and 00785C10(self, target).
void* client_xlive_construct_00a414d0(void* self, std::uint32_t argument, MNetworkCalls& host);
// 00A41590 EAX = +9D8h (the host info). 00A415A0: [ECX-0B4h] = 2.
void* client_host_info_00a41590(void* self) noexcept;
void client_set_state2_00a415a0(void* sub) noexcept;
// 00A415B0 BSP_MNetworkClientXLive_Destruct: vtables, 00785170, +948h = 0,
// a registered key (+961h) is unregistered (XNetUnregisterKey(+959h)) and
// cleared, +9D8h = 00CEED00, 00A39E30(+0C8h), 00784D60.
void client_xlive_destruct_00a415b0(void* self, MNetworkCalls& host);
// 00A41660 BSP_MNetworkClientXLive_Open: 00A39F70(+0C8h, 1000), +14h = 1;
// with [00F8A2FC] present and its vtable +198h true, setup +6A9h = its
// vtable +30h() (AL).
void client_xlive_open_00a41660(void* self, MNetworkCalls& host);

struct MNetworkLifetimeCalls {
    virtual ~MNetworkLifetimeCalls() = default;
    virtual void log(const char* text) = 0;                               // 004254B0
    virtual void log_key(const char* format, const std::uint8_t key[8]) = 0;
    virtual void client_disconnect_from_server_00a39dc0(void* client) = 0;
    virtual void xnet_unregister_key(const void* xnkid) = 0;             // 00A4D488
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void client_xlive_construct_00a414d0(void* self, std::uint32_t argument) = 0;
    virtual void server_xlive_construct_00a41ce0(void* self, std::uint32_t argument) = 0;
    virtual void object_virtual(void* object, std::uint32_t slot_offset) = 0;
    virtual void mnetwork_construct_00784f90(void* self, std::uint32_t argument) = 0;
    virtual void server_construct_00a3dc80(void* server) = 0;
    virtual void int_list_remove_value_00a421e0(void* list, const std::uint32_t* value) = 0;
    virtual void mnetwork_queue_peer_removal_007857f0(void* self, std::uint32_t peer) = 0;
    virtual void mnetwork_client_push_pending_host_00785ee0(void* self, void* message) = 0;
    virtual void lan_host_record_set_blob_00783540(void* message, std::uint32_t a, std::uint32_t b) = 0;
};

// 00A41880 BSP_XLiveHostInfo_Assign, __thiscall(holder, source), RET 4, EAX
// = holder: everything but the vtable word: +4h..+1Bh (XNKID, XNKEY),
// +1Ch..+3Fh (XNADDR), the 14h-unit wide string +40h and the 10h-unit wide
// string +68h.
void* host_info_assign_00a41880(void* holder, const void* source) noexcept;
// 00A41900 BSP_MNetworkClientXLive_Disconnect: logs
// "MNetworkClientXLive::DisconnectFromServerInner()" and "Client send
// MquiteMsg and Flush queue, Disconnecet low level", 00A39DC0(+0C8h), +14h =
// 1; with +948h and +961h set the key +959h is logged ("%x%x%x%x%x%x%x%x
// Unregister Key by MNetworkClientXLive::DisconnectFromServerInner()"),
// unregistered and +961h cleared (+948h stays).
void client_xlive_disconnect_00a41900(void* self, MNetworkLifetimeCalls& host);
// 00A41C70 BSP_MNetworkClientXLive_Create and 00A42170
// BSP_MNetworkServerXLive_Create, __fastcall(ECX = argument, EDX = out),
// RET 8 / RET 0Ch (stack arguments unused): new(0A60h / 920h), construct
// (00A414D0 / 00A41CE0) with the argument, *out = object, object vtable
// +4(). A failed allocation stores null and the vtable call faults.
void client_xlive_create_00a41c70(std::uint32_t argument, void** out, MNetworkLifetimeCalls& host);
void server_xlive_create_00a42170(std::uint32_t argument, void** out, MNetworkLifetimeCalls& host);
// 00A41CE0 BSP_MNetworkServerXLive_Construct, __thiscall(self, argument),
// RET 4: 00784F90, vtable 00D24390, +9Ch = 1 (stored only when different),
// +0A0h = 0, 00A3DC80(+0A4h), vtables +0A4h 00D243C0 and 00D243EC.
void* server_xlive_construct_00a41ce0(void* self, std::uint32_t argument, MNetworkLifetimeCalls& host);
// 00A422C0 BSP_MNetworkServerXLive_RemovePeer, __thiscall(self, peer), RET
// 4: the peer id leaves the int list +90Ch (00A421E0), then 007857F0.
void server_xlive_remove_peer_00a422c0(void* self, std::uint32_t peer, MNetworkLifetimeCalls& host);
// 00A42480 BSP_MNetworkClientXLive_PostPeerAddressMessage, __thiscall(ECX =
// console subobject +0C8h, a, b, const XNKID*, const XNKEY*, const
// XNADDR*), RET 14h. A new 64h message (vtable 00D24674, empty std::string
// at +4h, +20h = 0) gets the XNKID at +24h, XNKEY +2Ch, XNADDR +3Ch; it is
// pushed through 00785EE0 on the owner (ECX - 0C8h) and then 00783540(a, b)
// runs on it. No null test after the allocation.
void client_post_peer_address_message_00a42480(void* console, std::uint32_t a, std::uint32_t b,
    const void* xnkid, const void* xnkey, const void* xnaddr, MNetworkLifetimeCalls& host);
// 00A42510 BSP_XLiveTaskStub_Construct: vtable 00D24764.
void* task_stub_construct_00a42510(void* stub) noexcept;
// Peer queries on the client, RET 4: 00A41BA0 latency, 00A41BD0 profile
// value, 00A41C00 pending count; the same bodies as 00A41440/70/A0.
float client_peer_latency_00a41ba0(void* peer, MNetworkCalls& host);
float client_peer_profile_value_00a41bd0(void* peer, MNetworkCalls& host);
std::uint32_t client_peer_pending_count_00a41c00(void* peer, MNetworkCalls& host);

struct MNetworkConnectCalls {
    virtual ~MNetworkConnectCalls() = default;
    virtual void log(const char* text) = 0;
    virtual void log_pointer(const char* format, const void* pointer) = 0;     // 004254B0, one pointer argument
    virtual void log_key(const char* format, const std::uint8_t key[8]) = 0;
    virtual void log_address(const char* format, const std::uint8_t ina[4], const std::uint8_t enet[6]) = 0;
    virtual void xnet_unregister_key(const void* xnkid) = 0;                  // 00A4D488
    virtual std::int32_t xnet_register_key(const void* xnkid, const void* xnkey) = 0; // 00A4D4F4
    // XNetXnAddrToInAddr (00A4D48E)(xnaddr, xnkid, IN_ADDR* out).
    virtual std::int32_t xnet_xnaddr_to_inaddr(const void* xnaddr, const void* xnkid, std::uint32_t* ina) = 0;
    virtual std::uint16_t xsocket_htons(std::uint16_t value) = 0;             // 00A4D494
    virtual void client_set_server_address_00a39d40(void* client, const void* sockaddr_in) = 0;
    // [self+4] vtable +0Ch(&buffer, &length): the serialised connect message.
    virtual void build_connect_message(void* serializer, void** buffer, std::uint32_t* length) = 0;
    virtual void client_send_connect_00a39870(void* client, void* buffer, std::uint32_t length) = 0;
    virtual void object_virtual(void* object, std::uint32_t slot_offset) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void* game_setup() = 0;
};

// 00A416B0 BSP_MNetworkClientXLive_ConnectToServerInner, __thiscall(self,
// const host message*), RET 4, AL. With a key registered before (+948h):
// when +961h is set, log "%x%x%x%x%x%x%x%x Unreg key ..." with only the
// key's ADDRESS as argument (00A416CB pushes one pointer), unregister and
// clear +961h; +948h = 0. Then XNetRegisterKey(msg +24h, msg +2Ch), result
// ignored, and XNetXnAddrToInAddr(msg +3Ch, msg +24h, &ina). Failure logs
// "NETWORK ConnectToServerInner To %d.%d.%d.%d   XnAddr: ..." and AL = 0.
// Success: +948h = 1, key copy +959h, +961h = 1, the key logged, sockaddr
// {AF_INET, htons(1000), ina} to 00A39D40(+0C8h), the connect message built
// by [self+4] vtable +0Ch and sent (00A39870), vtable +30h, the buffer freed
// (00BF6989), client state setup +207Ch->+9Ch = 6, AL = 1.
bool client_connect_to_server_inner_00a416b0(void* self, const void* message, MNetworkConnectCalls& host);
// 00A42010 BSP_MNetworkClientXLive_ConnectToServer, __thiscall(self, const
// host info*), RET 4, AL: log "MNetworkClientXLive::ConnectToServerInner";
// 00A41880 into +9D8h; +961h = 0; XNetXnAddrToInAddr(+9F4h, +9DCh, &ina);
// +908h/+90Ch = the XNKID; success: +948h = 1, server address, connect
// message sent and freed, AL = 1 (no vtable +30h, no state change); else 0.
bool client_connect_to_server_00a42010(void* self, const void* host_info, MNetworkConnectCalls& host);
// 00A42520 BSP_XLiveTaskStub_ReturnZero: EAX = 0.
std::uint32_t task_stub_return_zero_00a42520() noexcept;

struct MNetworkHostingCalls {
    virtual ~MNetworkHostingCalls() = default;
    virtual void server_open_00a3d480(void* server, std::uint32_t max_peers, std::uint32_t timeout_ms) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void* game_setup() = 0;                        // [00E188A8]
    virtual void* xenon_system() = 0;                      // [00F8ABE8]
    virtual void* xlive_singleton() = 0;                   // [00F8A2FC]
    virtual std::uint8_t object_virtual_al(void* object, std::uint32_t slot_offset) = 0;
    virtual const char* version_text_00f88a38(std::uint32_t* length_00f88a34) = 0;
    virtual const char* language_name_008d4870() = 0;      // on 00F88980
    virtual void session_target_construct_007839d0(void* target, const void* info, std::uint32_t bytes) = 0;
    virtual std::int32_t xnet_xnaddr_to_inaddr(const void* xnaddr, const void* xnkid, std::uint32_t* ina) = 0;
    virtual std::uint16_t xsocket_htons(std::uint16_t value) = 0;
    virtual void mnetwork_queue_incoming_peer_00785790(void* self, void* target) = 0;
    // Virtual calls on the server subobject (vtable +4h, returns a value) and
    // on the outer object (vtable +8h with one argument).
    virtual std::uintptr_t server_virtual_04(void* server, std::uintptr_t peer) = 0;
    virtual void outer_virtual_08(void* outer, std::uintptr_t value) = 0;
    virtual void free_00bf65ac(void* block) = 0;
};

// 00A41D80 BSP_MNetworkServerXLive_StartHosting, __fastcall(ECX = self).
// 00A3D480(+0A4h, 8, 1000). A new 98h host info: +0h strncpy(setup string
// +6A0h, or +68Ch when empty, 20h) with +1Fh = 0; +20h strncpy([00F88A38],
// min([00F88A34], 5)) and its terminator; +28h/+2Ch = setup +698h/+69Ch;
// +59h = setup +6A9h, both replaced by singleton vtable +30h() when the
// singleton exists and vtable +198h is true (vtable +30h is called twice);
// +58h = 0; +50h/+54h = the unsigned maxima of setup +73Ch/+740h and Xenon
// +358h/+35Ch; +32h strncpy(language name 008D4870, 10h); +48h/+4Ch = setup
// +728h/+72Ch. A new 0DC0h target (007839D0(info, 98h), vtable 00D03860; no
// null test afterwards) gets the sockaddr {AF_INET, htons(1000), ina of
// XNetXnAddrToInAddr(+8C0h, +8E4h)} at +0D74h..+0D83h and is queued
// (00785790). The info buffer is not freed here.
void server_xlive_start_hosting_00a41d80(void* self, MNetworkHostingCalls& host);
// 00A422F0 BSP_MNetworkServerXLive_ResolvePacketSource, __thiscall(ECX =
// server subobject +0A4h, peer), RET 4. A peer: outer vtable +8(server
// vtable +4(peer)) (tail call). No peer: every value of the list at +868h
// (head +86Ch) goes to outer vtable +8, iterating through the member
// iterator +874h/+878h; the list is then emptied (nodes _free'd, size +870h
// = 0) and +81Ah = 1. It disconnects peers; it resolves nothing.
void server_xlive_resolve_packet_source_00a422f0(void* server, std::uintptr_t peer, MNetworkHostingCalls& host);

// ---------------------------------------------------------------------------
// Network console base / cNetworkConsole 00A39060..00A39500 (ECX = console).
// Layout: +4h receive buffer (800h), +804h/+818h..+81Ah flags, +808h..+817h
// last unresolved sender (sockaddr), +840h XNKID, +848h XNKEY, +858h/+85Ch
// the two sockets, +860h key valid, +861h key foreign (not ours to register).
// ---------------------------------------------------------------------------

struct NetworkConsoleCalls {
    virtual ~NetworkConsoleCalls() = default;
    // Console vtable: +0h deliver(peer, from, data, bytes), +4h resolve(sockaddr*), +8h(a, c, d).
    virtual void console_deliver_00(void* console, std::uint32_t peer, std::uint32_t from, const void* data,
        std::int32_t bytes) = 0;
    virtual std::uint32_t console_resolve_04(void* console, const void* sockaddr_in) = 0;
    virtual void console_virtual_08(void* console, std::uint32_t a, std::uint32_t c, std::uint32_t d) = 0;
    virtual std::uint32_t xnet_get_ethernet_link_status() = 0;                       // 00A4D4CA
    // 00A3C3C0 on [00F8ABDC](socket, int* from, sockaddr* out, buffer, 800h, channel).
    virtual std::int32_t receive_packet_00a3c3c0(std::uint32_t socket, std::uint32_t* from, void* sockaddr_in,
        void* buffer, std::uint32_t bytes, std::uint32_t channel) = 0;
    virtual std::int32_t xnet_create_key(void* xnkid, void* xnkey) = 0;               // 00A4D4FA
    virtual std::int32_t xnet_register_key(const void* xnkid, const void* xnkey) = 0; // 00A4D4F4
    virtual std::int32_t xnet_unregister_key(const void* xnkid) = 0;                  // 00A4D488
    virtual void reset_for_socket_00a3b870(std::uint32_t socket, std::uint32_t argument) = 0; // on [00F8ABDC]
    virtual std::int32_t xsocket_close(std::uint32_t socket) = 0;                     // 00A4D500
};

// 00A39060 BSP_NetworkConsoleBase_HandlePacket, __thiscall(console, a, type,
// c, d), RET 10h: type 5 or 6 -> vtable +8(a, c, d).
void console_base_handle_packet_00a39060(void* console, std::uint32_t a, std::int32_t type, std::uint32_t c,
    std::uint32_t d, NetworkConsoleCalls& host);
// 00A390B0 BSP_NetworkConsoleBase_Construct: vtable 00D23B88, bytes +804h,
// +818h, +819h, +81Ah = 0. 00A390E0 BSP_cNetworkConsole_Construct: that,
// +860h = +861h = 0, sockets +858h = +85Ch = -1, vtable 00D23BA4.
void* console_base_construct_00a390b0(void* console) noexcept;
void* console_construct_00a390e0(void* console) noexcept;
// 00A39110: EAX = XNetGetEthernetLinkStatus() != 0 (NEG/SBB/NEG).
std::uint32_t console_is_ethernet_link_up_00a39110(NetworkConsoleCalls& host);
// 00A391A0 BSP_cNetworkConsole_PumpReceive. With +818h set and +861h clear:
// channel 0, then channel 1, each read with 00A3C3C0(socket +858h, &from,
// &sockaddr, +4h, 800h, channel) until it returns a negative count. Channel
// 0 clears +819h before each resolve. Each packet is resolved through
// vtable +4(&sockaddr); a known peer is delivered as vtable +0(peer, from,
// +4h, count); an unknown sender is stored at +808h..+817h and delivered
// with peer 0. (The three pointers come from the stack layout at
// 00A391D6..00A391EF.)
void console_pump_receive_00a391a0(void* console, NetworkConsoleCalls& host);
// 00A39480 BSP_cNetworkConsole_RegisterKey: key valid and not foreign ->
// XNetCreateKey(+840h, +848h) and, when that returns 0, XNetRegisterKey.
void console_register_key_00a39480(void* console, NetworkConsoleCalls& host);
// 00A394C0 BSP_cNetworkConsole_UnregisterKey: key valid and not foreign ->
// XNetUnregisterKey(+840h); +860h = +861h = 0 in every case.
void console_unregister_key_00a394c0(void* console, NetworkConsoleCalls& host);
// 00A39500 BSP_cNetworkConsole_CloseSockets: socket +858h -> 00A3B870(socket,
// 0) on the sender, XSocketClose, -1; socket +85Ch -> XSocketClose, -1.
void console_close_sockets_00a39500(void* console, NetworkConsoleCalls& host);

// ---------------------------------------------------------------------------
// cNetworkClient session wrapper 00A39570..00A397D0 (ECX = client console,
// MNetworkClientXLive +0C8h). Fields: +8E0h nonce, +8E8h XOVERLAPPED,
// +904h session handle, +908h create pending, +90Ch started, +90Dh ended,
// +90Eh deleted.
// ---------------------------------------------------------------------------

struct ClientSessionCalls {
    virtual ~ClientSessionCalls() = default;
    virtual void log(const char* text) = 0;
    virtual void log_join(const char* format, std::uint64_t xuid, const char* kind) = 0;
    virtual void log_hex3(const char* format, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual std::uint32_t xsession_arbitration_register(void* session, std::uint32_t flags, std::uint64_t nonce,
        std::uint32_t* bytes, void* results, void* overlapped) = 0;   // 00A4D506
    virtual std::uint32_t xsession_start(void* session, std::uint32_t flags, void* overlapped) = 0;      // 00A4D50C
    virtual std::uint32_t xsession_end(void* session, void* overlapped) = 0;                            // 00A4D512
    virtual std::uint32_t xsession_join_remote(void* session, std::uint32_t count, const std::uint64_t* xuids,
        const std::uint32_t* private_slots, void* overlapped) = 0;                                      // 00A4D518
    virtual std::uint32_t xsession_delete(void* session, void* overlapped) = 0;                         // 00A4D51E
    virtual std::uint32_t x_close_handle(void* handle) = 0;                                             // 00A4D4A6
    virtual std::uint32_t x_get_overlapped_extended_error(void* overlapped) = 0;                        // 00A4D422
    virtual std::uint32_t x_get_overlapped_result(void* overlapped, std::uint32_t* result, std::uint32_t wait) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void console_construct_00a390e0(void* console) = 0;
};

// 00A39570 BSP_cNetworkClient_Construct: 00A390E0, then +878h, +879h,
// +87Ch, +819h, +880h, +90Ch, +908h, +8A0h, +904h, +899h = 0, vtable
// 00D23BC0, +89Ch = 7. +90Dh and +90Eh are not initialised.
void* client_construct_00a39570(void* client, ClientSessionCalls& host);
// 00A395E0 BSP_cNetworkClient_ArbitrationRegister, HRESULT: XOVERLAPPED
// +8E8h..+903h zeroed; a size probe XSessionArbitrationRegister(+904h, 0,
// nonce +8E0h, &bytes, 0, 0) (result ignored), a 00BF55BE buffer, the real
// call; nonzero -> XGetOverlappedExtendedError(0); buffer freed (00BF6989).
// The overlapped it zeroes is never passed. The twin of 00A49A50.
std::int32_t client_arbitration_register_00a395e0(void* client, ClientSessionCalls& host);
// 00A39690 BSP_cNetworkClient_StartSession: +90Ch = 1, EAX = XSessionStart(
// +904h, 0, 0).
std::uint32_t client_start_session_00a39690(void* client, ClientSessionCalls& host);
// 00A396B0 BSP_cNetworkClient_EndSession: handle, not ended, started ->
// +90Dh = 1 and EAX = XSessionEnd(+904h, 0); else 0.
std::uint32_t client_end_session_00a396b0(void* client, ClientSessionCalls& host);
// 00A396E0 BSP_cNetworkClient_JoinRemotePlayer, __thiscall(client, const
// XUID*, BYTE invited), RET 8: log "  --  ONLINE  --  Client Join Remote
// Player %llx   %s" ("Invite" or " "), XSessionJoinRemote(+904h, 1, xuid,
// {invited != 0}, 0); nonzero -> XGetOverlappedExtendedError(0), else 0.
std::int32_t client_join_remote_player_00a396e0(void* client, const std::uint64_t* xuid, std::uint8_t invited,
    ClientSessionCalls& host);
// 00A39750 BSP_cNetworkClient_PollCreateResult: create pending (+908h == 1)
// and the overlapped finished: an extended error -> XGetOverlappedResult,
// error read again, log "__ ONLINE __ Client Session hiba %x %x %x" (result
// word, GetOverlappedResult's return, error), 8000FFFFh; success -> +908h =
// 0 and EAX = 217h. Otherwise 0.
std::int32_t client_poll_create_result_00a39750(void* client, ClientSessionCalls& host);
// 00A397D0 BSP_cNetworkClient_DeleteSession: log "NETWORK: CLIENT ---
// SessionDelete"; with a handle: started and not ended -> log "  --  ONLINE
// --  cNetworkClient  SessionEnd", +90Dh = 1, XSessionEnd(handle, 0); not
// deleted -> XSessionDelete(+904h, 0), log "NETWORK: CLIENT ---
// SessionDeleted", +90Eh = 1, XCloseHandle(+904h); +904h = 0.
void client_delete_session_00a397d0(void* client, ClientSessionCalls& host);

// ---------------------------------------------------------------------------
// XenonSystemManager ([00F8ABE8]) accessors 00A3E420..00A3EB30. Fields:
// +2Fh storage removed, +31h invite accepted, +32h invitee XUID, +3Ah
// inviter XUID, +8Ch slot states (one slot), +110h slot XUIDs, +118h slot
// bytes, +119h signed in, +11Ch active slot, +3B8h sign-in user index.
// ---------------------------------------------------------------------------

struct XenonCalls {
    virtual ~XenonCalls() = default;
    // XUserGetSigninInfo (00A4D560)(user, flags, XUSER_SIGNIN_INFO* (28h)).
    virtual std::uint32_t xuser_get_signin_info(std::uint32_t user, std::uint32_t flags, void* info) = 0;
    // XUserCheckPrivilege (00A4D482)(user, privilege, BOOL* result).
    virtual std::uint32_t xuser_check_privilege(std::uint32_t user, std::uint32_t privilege, std::uint32_t* result) = 0;
};

// 00A3E420 / 00A3E440, __thiscall(xenon, BYTE take), RET 4: AL = +2Fh / +31h,
// cleared when `take` is set.
std::uint8_t take_storage_removed_00a3e420(void* xenon, std::uint8_t take) noexcept;
std::uint8_t take_invite_accepted_00a3e440(void* xenon, std::uint8_t take) noexcept;
// 00A3E470 / 00A3E4B0, EAX: the first slot (the loop bound is 1) with a
// nonzero state whose XUID equals the invitee (+32h) / inviter (+3Ah) XUID;
// 1 when none matches.
std::uint32_t find_slot_for_invitee_00a3e470(const void* xenon) noexcept;
std::uint32_t find_slot_for_inviter_00a3e4b0(const void* xenon) noexcept;
// 00A3E530, RET 4: EAX = +8Ch[slot] (no bound).
std::uint32_t slot_state_00a3e530(const void* xenon, std::uint32_t slot) noexcept;
// 00A3E5D0 BSP_XenonSystemManager_GetOnlineXuid, EDX:EAX: user +3B8h == 1 ->
// 0; else XUserGetSigninInfo(user, 2 = ONLINE_XUID_ONLY, &info) and the
// info's first qword (the call's result is not checked).
std::uint64_t online_xuid_00a3e5d0(const void* xenon, XenonCalls& host);
// 00A3EB00 EDX:EAX = +110h[active slot +11Ch]; 00A3EB20 AL = +118h[active].
std::uint64_t active_xuid_00a3eb00(const void* xenon) noexcept;
std::uint8_t active_slot_byte_00a3eb20(const void* xenon) noexcept;
// 00A3EB30 BSP_XenonSystemManager_GetCommunicationPrivilege, __thiscall(xenon,
// BYTE* friends_only), RET 4, AL. Not signed in (+119h): *out = 0, AL = 0.
// With slot state nonzero or active slot 0: user 0 (hard-coded) is checked
// for XPRIVILEGE_COMMUNICATIONS (0FCh) -> level 2, else
// COMMUNICATIONS_FRIENDS_ONLY (0FBh) -> level 1, else 0; otherwise level 0.
// *out = (level == 1); AL = (level != 0).
bool communication_privilege_00a3eb30(const void* xenon, std::uint8_t* friends_only, XenonCalls& host);

// ---------------------------------------------------------------------------
// Small client/server/thread/system routines
// ---------------------------------------------------------------------------

struct MiscNetworkCalls {
    virtual ~MiscNetworkCalls() = default;
    virtual void client_enumerate_lan_servers_00a39900(void* client) = 0;
    virtual void console_virtual_14(void* console) = 0;
    virtual void activate_first_free_channel_00a3baa0(void* address) = 0;   // on [00F8ABDC]
    virtual void console_unregister_key_00a394c0(void* console) = 0;
    virtual void console_close_sockets_00a39500(void* console) = 0;
    virtual void client_delete_session_00a397d0(void* client) = 0;
    virtual void client_handle_packet_00a3a160(void* client) = 0;           // JMP target, stack args forwarded
    virtual void server_handle_packet_00a3e000(void* server) = 0;           // JMP target, stack args forwarded
    virtual void thread_set_debug_name_008e24f0(std::int32_t thread, const char* name) = 0;
    virtual std::uint32_t send_worker_00a3c5e0(void* sender) = 0;
    virtual std::uint32_t receive_worker_00a3caf0(void* sender) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void player_record_assign_006883b0(void* destination, const void* source) = 0;
    virtual void system_virtual_20(void* system) = 0;
    virtual void list_clear_00a45510(void* list) = 0;
    virtual void voice_shutdown_00a49270(void* voice) = 0;
    virtual void player_list_clear_00688780(void* list) = 0;
};

// 00A39D10 BSP_cNetworkClient_Update, __thiscall(client, float dt), RET 4
// (dt unused): sockets open (+818h): LAN search (+879h) -> 00A39900, else
// connected (+880h) -> vtable +14h.
void client_update_00a39d10(void* client, MiscNetworkCalls& host);
// 00A39D40 BSP_cNetworkClient_SetServerAddress, __thiscall(client, const
// sockaddr_in*), RET 4: the target [+874h] gets the 16 bytes at +0D74h,
// +878h = 1, then tail 00A3BAA0(target +0D74h) on the sender.
void client_set_server_address_00a39d40(void* client, const void* sockaddr_in, MiscNetworkCalls& host);
// 00A39D90 BSP_cNetworkClient_Close: 00A394C0, 00A39500, 00A397D0 when no
// key of ours is valid (+860h clear), +818h = 0.
void client_close_00a39d90(void* client, MiscNetworkCalls& host);
// 00A3A320 / 00A3E3A0: JMP 00A3A160 / 00A3E000 (the HandlePacket bodies).
void client_handle_packet_thunk_00a3a320(void* client, MiscNetworkCalls& host);
void server_handle_packet_thunk_00a3e3a0(void* server, MiscNetworkCalls& host);
// 00A3D460: RET.
void nop_00a3d460() noexcept;
// 00A3CEC0 / 00A3CEE0, __stdcall thread procedures (parameter = sender),
// RET 4: 008E24F0(-1, "cNetworkConsoleSend::sSendThread" /
// "cNetworkConsoleSend::sRecvThread"), then EAX = 00A3C5E0 / 00A3CAF0(sender).
std::uint32_t send_thread_entry_00a3cec0(void* sender, MiscNetworkCalls& host);
std::uint32_t receive_thread_entry_00a3cee0(void* sender, MiscNetworkCalls& host);
// 00A44800 / 00A44820, __thiscall(system, OnlinePlayerRecord by value), RET
// 40h: the record is only destroyed (its string buffer is freed when the
// capacity at record +18h is 10h or more).
void ignore_player_record_00a44800(bsp::online_cc13::OnlinePlayerRecord record, MiscNetworkCalls& host);
void ignore_player_record_00a44820(bsp::online_cc13::OnlinePlayerRecord record, MiscNetworkCalls& host);
// 00A454F0, __thiscall(system, const record*), RET 4: 006883B0 into system
// +8h, then vtable +20h.
void set_record_and_refresh_00a454f0(void* system, const void* record, MiscNetworkCalls& host);
// 00A45D90: ECX += 168h, JMP 00A45510.
void clear_list_168_00a45d90(void* system, MiscNetworkCalls& host);
// 00A46240 BSP_XLiveSystem_StopVoice, AL = 1: voice active -> 00A49270
// (+1C24h); the LAN player list +10Ch is cleared (00688780).
bool stop_voice_00a46240(void* system, MiscNetworkCalls& host);

} // namespace bsp::online_seg71_cc13
