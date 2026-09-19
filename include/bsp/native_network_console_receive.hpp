#pragma once
#include "bsp/native_network_console_runtime.hpp"

namespace bsp {
// Borrow actual process cells/constants. The two tables cover16 entries; values
// used as stream indices must be0..2. Native image E0E360/E0E370 currently uses
// only stream1 for sequenced packet types. Keep reads after ACK processing.
struct NativeNetworkConsoleReceiveContext {
    const NativeFrameClockPublicationContext& clock;
    const ClockTimestamp& classify_stack_preimage;
    const ClockTimestamp& ack_stack_preimage;
    const volatile std::uint8_t* sequenced_types_00e0e360;
    const volatile std::uint32_t* stream_indices_00e0e370;
    const volatile double& maximum_sample_00d049a8;
    const volatile float& capped_sample_00cf4848;
};
// Stack header/reference, RET8/EAX extended sequence; ECX is not an input.
// Reconstruct10 bits around a DWORD reference using unsigned wrapping tests.
std::uint32_t decode_native_network_console_sequence_00a3a6b0(
    const std::byte* header, std::uint32_t reference) noexcept;
// ECX actual49Ch channel, stack header, RET4/EAX reversed list of released
// 82Ch send records. Reject older ACKs and, for equal counters, masks containing
// any previously unseen bit. Sample current AB0 only after admission. Preserve
// x87 float rounding, unordered comparisons, slot statistics and exact links.
// Valid finite native lists and matching head/tail ownership are required.
NativeNetworkConsoleQueue82cStorage* process_native_network_console_ack_00a3b420(
    NativeNetworkConsoleChannelStorage&, const std::byte* header,
    const NativeNetworkConsoleReceiveContext&);
// ECX channel, stack header/stream-out/sequence-out, RET0C/EAX released send list.
// Sample timestamp20, run complete ACK processing, then classify. Outputs -1
// for rejected/duplicate/ACK-only type8,2 for unsequenced packets, or the table's
// stream index for a new sequence; clear byte28+index only on that last path.
NativeNetworkConsoleQueue82cStorage* classify_native_network_console_received_00a3b5f0(
    NativeNetworkConsoleChannelStorage&, const std::byte* header,
    std::int32_t& stream, std::uint32_t& sequence, const NativeNetworkConsoleReceiveContext&);
// ECX owner, stack sockaddr16, RET4. Under captured148 activate the first free
// channel, copy four address DWORDs in order, then call ordinal40 (discard result).
// No channel reset or duplicate lookup. Full table is a no-op except the lock.
void activate_native_network_console_channel_00a3baa0(NativeNetworkConsoleStorage&,
    const std::byte* sockaddr16, const NativeNetworkConsoleSocketImports&);
// ECX channel, stack float delay, RET4. On first request set byte18 before the
// CURRENT AB0 sample. Otherwise retain the earlier deadline with exact x87
// float spills and FCOMIP/JBE behavior (unordered selects the new candidate).
// Explicit caller stack preimage; source contract errors are not native faults.
void request_native_network_console_disconnect_00a3adf0(NativeNetworkConsoleChannelStorage&,
    float delay, const NativeFrameClockPublicationContext&, const ClockTimestamp& stack_preimage);
} // namespace bsp
