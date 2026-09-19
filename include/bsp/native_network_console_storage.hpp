#pragma once
#include "bsp/frame_clock.hpp"
#include "bsp/native_frame_clock_publication.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual array element extents from A3CE60/A3D060. Default initialization
// deliberately preserves every byte the native constructors leave unwritten.
struct alignas(4) NativeNetworkConsoleSlotStorage { std::byte bytes[0x10]; };
struct alignas(4) NativeNetworkConsoleChannelStorage { std::byte bytes[0x49c]; };
struct alignas(4) NativeNetworkConsoleQueue82cStorage { std::byte bytes[0x82c]; };
struct alignas(4) NativeNetworkConsoleQueue820Storage { std::byte bytes[0x820]; };

struct NativeNetworkConsoleChannelContext {
    const NativeFrameClockPublicationContext& clock;
    const ClockTimestamp& sample_stack_preimage;
    const volatile std::uint32_t& reset_interval_bits_00d7a260;
};
// Original ECX=actual element, EAX=same element, RET. Slots write only byte0
// and DWORD4. Queue constructors store their one-past-element pointer at+0;
// the 820h instantiation also clears DWORD1C. Queue direction is not inferred.
NativeNetworkConsoleSlotStorage* construct_native_network_console_slot_00a3a330(
    NativeNetworkConsoleSlotStorage&) noexcept;
NativeNetworkConsoleQueue82cStorage* construct_native_network_console_queue_00a3a3d0(
    NativeNetworkConsoleQueue82cStorage&) noexcept;
NativeNetworkConsoleQueue820Storage* construct_native_network_console_queue_00a3a500(
    NativeNetworkConsoleQueue820Storage&) noexcept;
// Distinct original RET-only element destructors. No fields are read or cleared.
void destroy_native_network_console_slot_00a3a340(NativeNetworkConsoleSlotStorage&) noexcept;
void destroy_native_network_console_queue_00a3a3e0(NativeNetworkConsoleQueue82cStorage&) noexcept;
void destroy_native_network_console_queue_00a3a520(NativeNetworkConsoleQueue820Storage&) noexcept;

// Complete A3AD10: ECX=actual49Ch channel, RET. Two initial byte stores precede
// the CURRENT AB0+20 clock call. Keep FILD/FILD/FDIVP and the delayed float
// store at+20 in x87, preserve scalar/64-slot write order, then read D7A260
// only after the slot loop. No whole-object clear or invented padding writes.
void reset_native_network_console_channel_00a3ad10(NativeNetworkConsoleChannelStorage&,
    const NativeNetworkConsoleChannelContext&);
// Complete A3CE60/A3A770: construct64 actual10h slots at+8C before reset;
// destroy them in reverse order. The source C++ failure path invokes the same
// no-op member cleanup. Native private EH/SEH/ABI identity is not established.
NativeNetworkConsoleChannelStorage* construct_native_network_console_channel_00a3ce60(
    NativeNetworkConsoleChannelStorage&, const NativeNetworkConsoleChannelContext&);
void destroy_native_network_console_channel_00a3a770(NativeNetworkConsoleChannelStorage&) noexcept;
} // namespace bsp
