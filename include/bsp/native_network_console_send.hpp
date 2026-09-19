#pragma once
#include "bsp/native_network_console_runtime.hpp"

namespace bsp {
struct NativeNetworkConsoleTimingContext {
    const NativeFrameClockPublicationContext& clock;
    const ClockTimestamp& sample_stack_preimage;
    const volatile float& no_samples_00d7a2f0;
    const volatile double& half_00d7a280;
    const volatile double& three_00d7a2b0;
    const volatile double& quarter_00d7a348;
};
struct NativeNetworkConsoleSendContext {
    const NativeNetworkConsoleTimingContext& timing;
    const ClockTimestamp& enqueue_stack_preimage;
    NativeNetworkConsoleStorage* volatile& current_owner_00f8abdc;
    const volatile std::uint8_t* sequenced_types_00e0e360;
    const volatile std::uint32_t* stream_indices_00e0e370;
    const volatile double& initial_packet_age_00ce47a0;
    const volatile double& packet_drop_unit_00d7a210;
    const NativeNetworkConsoleChannelContext& reset;
    const NativeNetworkConsoleSocketImports& sockets;
};
// ECX owner, stack index/max/min/drop/weight, RET14. Actual16x14h profile array
// starts+8. Producer8D31ED supplies Lua index-1, Latency_Max/Min, Packet_Drop,
// Sync_Send_Weight. Preserve x87 (max+min)*half before the delayed mean store.
void set_native_network_console_latency_profile_00a3a890(NativeNetworkConsoleStorage&,
    std::uint32_t index, float maximum, float minimum, float drop, float send_weight,
    const volatile double& half_00d7a280);
// ECX channel, stack level-out, RET4/ST0. Write current494 before reading70;
// zero sent returns the actual default constant without sampling. Otherwise
// inspect min(sent,max(4,sent-ack60)) ring entries, wrapping backwards over32,
// and average the largest up-to-four values (initial top values are zero).
// Keep exact x87/SSE moves, float spills, ordered/unordered comparisons.
float estimate_native_network_console_latency_00a3aef0(NativeNetworkConsoleChannelStorage&,
    std::uint32_t& level, const NativeNetworkConsoleTimingContext&);
// Original ECX=channel, stack removed-byte/stale-byte/now/interval, RET10/EAX
// record. EDX is unused and explicit here to retain the original stack layout.
// Pure native body: retain two x87 values across scans; rotate due reliable
// records to the tail, remove stream2/closing/stale records, preserve flags.
NativeNetworkConsoleQueue82cStorage* __fastcall select_native_network_console_packet_00a3a8f0(
    NativeNetworkConsoleChannelStorage*, void*, std::uint8_t* removed,
    std::uint8_t* stale, float now, float interval) noexcept;
// ECX channel, stack two actual four-byte pool slots, RET8. Prepend complete
// send lists0..2, then receive lists0..2, clear their head/tail fields, then
// invoke full R172 reset. No element free; finite valid native lists required.
void return_native_network_console_channel_queues_00a3b150(NativeNetworkConsoleChannelStorage&,
    std::byte* send_pool_slot, std::byte* receive_pool_slot,
    const NativeNetworkConsoleChannelContext&);
// ECX owner, stack sockaddr16, RET4. Captured tracked148; first active IPv4
// match returns its queues and resets it. Miss has no SDK conversion call.
void remove_native_network_console_channel_00a3bc60(NativeNetworkConsoleStorage&,
    const std::byte* sockaddr16, const NativeNetworkConsoleChannelContext&);
// ECX channel, stack header/type/record, RET0C/AL success (upper EAX unspecified).
// Complete profile adaptation, timestamp, sequence/header/slot bookkeeping or
// unsequenced drop accumulator and stream2 append. Current owner is reloaded
// at each native publication read. Type0..15, sequenced stream0..1 and populated profile rows required;
// native index arithmetic is unchecked. Record/header are actual writable storage.
bool queue_native_network_console_packet_00a3b210(NativeNetworkConsoleChannelStorage&,
    std::byte* header, std::uint32_t type, NativeNetworkConsoleQueue82cStorage&,
    const NativeNetworkConsoleSendContext&);
// ECX owner, stack socket/address/type/payload/length/direct, RET18/AL.
// False only for an unregistered socket or empty pool. After taking a record,
// missing channel, disabled direct queue or an intentional drop still returns
// true and returns the record to the pool. Payload must fit native800h packet
// storage (normal header8/direct header3); no invented size clamp or allocation.
bool enqueue_native_network_console_packet_00a3c1a0(NativeNetworkConsoleStorage&,
    std::uint32_t socket, const std::byte* sockaddr16, std::uint32_t type,
    const void* payload, std::uint32_t length, std::uint8_t direct,
    const NativeNetworkConsoleSendContext&);
// Except the explicit selector bridge, these are new source interfaces.
// Original private EH/SEH, fault paths and full binary ABI are not established.
} // namespace bsp
