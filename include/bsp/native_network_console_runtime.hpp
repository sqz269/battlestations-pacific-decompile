#pragma once
#include "bsp/native_network_console_owner.hpp"
#include "bsp/native_network_console_storage.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {
// Explicit SDK boundaries: ordinals38/40 in the already loaded xlive module. Resolving
// it does not load a DLL or provide a replacement implementation.
struct NativeNetworkConsoleSocketImports {
    using ShortConversion = std::uint16_t (WINAPI*)(std::uint16_t);
    ShortConversion network_to_host_short;
    ShortConversion host_to_network_short;
};
NativeNetworkConsoleSocketImports resolve_native_network_console_socket_imports(HMODULE);

struct NativeNetworkConsoleThreadImports {
    using SleepFunction = void (WINAPI*)(DWORD);
    using CloseFunction = BOOL (WINAPI*)(HANDLE);
    SleepFunction volatile sleep;
    CloseFunction volatile close_handle;
};
const NativeNetworkConsoleThreadImports& native_network_console_thread_imports() noexcept;
struct NativeNetworkConsoleStopContext {
    volatile std::uint8_t& quit_00f8abe0;
    const volatile std::uint8_t& send_active_00f8abe1;
    const volatile std::uint8_t& receive_active_00f8abe2;
    const NativeNetworkConsoleThreadImports& imports;
};

// ECX actual49Ch channel, stack output header, RET4. Header covers at least8
// bytes. Rebuild the24-bit selective-ACK cache only when byte29 is zero;
// preserve existing header byte2 bits with OR. Valid finite native node lists.
void write_native_network_console_ack_00a3aab0(
    NativeNetworkConsoleChannelStorage&, std::byte* header) noexcept;
// ECX channel, stack node820h/index, RET8. Valid stream indices0..2. Append
// without clearing the incoming next link; repeatedly scan the captured head
// to advance the contiguous sequence. No sorting, freeing or cache invalidation.
void append_native_network_console_received_00a3ab30(
    NativeNetworkConsoleChannelStorage&, NativeNetworkConsoleQueue820Storage&,
    std::uint32_t stream_index) noexcept;
// ECX actual4003F0h owner, stack sockaddr16h, RET4/EAX index orFFFFFFFF.
// First active channel matching only the IPv4 DWORD. Capture lock+148 once;
// preserve the miss-only ordinal38 call under that lock (return discarded).
std::uint32_t find_native_network_console_channel_00a3bb30(
    NativeNetworkConsoleStorage&, const std::byte* sockaddr16,
    const NativeNetworkConsoleSocketImports&);
// ECX owner, RET. Set quit under captured+14C, always Sleep(10) at least once,
// reload+14C per poll, then close the two handles. Capture Sleep once and
// CloseHandle once; reload the second handle after the first close. This polls
// active flags, not thread-handle completion; there is no wait or timeout.
void stop_native_network_console_threads_00a3b6e0(
    NativeNetworkConsoleStorage&, const NativeNetworkConsoleStopContext&);
// Complete normal-path A3D1A0: profile, two actual owned1Ch locks, reverse
// arrays(1000x820h,1000x82Ch,16x49Ch), then base. Requires constructed members
// and quiescent workers. Does not stop workers or close handles. Original
// FH3/SEH cleanup and drop-in binary ABI are not established by this interface.
void destroy_native_network_console_00a3d1a0(
    NativeNetworkConsoleStorage&, NativeNetworkConsoleBaseContext&);
// ECX owner, stack flags, RET4/EAX captured identity. Only flags bit0 frees;
// it requires an allocation from new NativeNetworkConsoleStorage.
NativeNetworkConsoleStorage* delete_native_network_console_00a3d260(
    NativeNetworkConsoleStorage&, std::uint32_t flags, NativeNetworkConsoleBaseContext&);
} // namespace bsp
