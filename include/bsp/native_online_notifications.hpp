#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// Actual 3F0h allocation: 73DC50 allocates it, A3F530 publishes that same
// identity at F8ABE8, and A40EB5 initially writes zero at +3E8. This type
// supplies storage, not a constructor, registration, or ownership adapter.
// Default initialization deliberately leaves every byte indeterminate.
struct alignas(4) NativeOnlineManagerStorage final {
    std::byte opaque_000[0x3e8];
    std::uint8_t notification9_3e8;
    std::byte opaque_3e9[7];
};
static_assert(sizeof(NativeOnlineManagerStorage) == 0x3f0);
static_assert(alignof(NativeOnlineManagerStorage) == 4);
static_assert(offsetof(NativeOnlineManagerStorage, notification9_3e8) == 0x3e8);
static_assert(std::is_standard_layout_v<NativeOnlineManagerStorage>);

// Startup 4E555E installs 4CEB40 in F8ABEC. That target consumes CL and RETs
// without stack arguments; high ECX bits and EDX are not interface inputs.
// The actual callback remains an external native interface, not a fake UI
// implementation. A callable supplied slot must have this observed Win32 ABI.
using NativeOnlineNotification9Hook = void (__fastcall*)(std::uint8_t);
static_assert(sizeof(NativeOnlineNotification9Hook) == 4);

// Partial A40110: only A401A9..A401CC, the already-selected notification-9
// branch, ending at its jump to A402C6. This is NOT the notification drain.
// Entry: ESI=captured live manager, EBX=0, [ESP+14]=DWORD parameter; the C++
// references represent those exact storage identities. Capture current hook,
// call it with SETNZ(parameter) in CL, then reread/normalize the parameter and
// store the resulting byte to captured manager+3E8. No global-manager reload.
// The store overwrites any raw +3E8 preimage; all other bytes are unchanged.
// Keep manager, parameter cell, and slot alive across the callback, excluding
// concurrent mutation/retirement. A C++ callback exception propagates before
// the final store; callback effects remain. No FH3 equivalence is claimed.
void apply_native_online_notification9_fragment_00a401a9(
    NativeOnlineManagerStorage& captured_manager,
    volatile std::uint32_t& parameter_esp14,
    NativeOnlineNotification9Hook volatile& hook_f8abec);
} // namespace bsp
