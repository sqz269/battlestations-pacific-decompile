#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native gameplay effect destruction requires MSVC Win32.
#endif

namespace bsp {
// Full86FE20[159]: original ECX raw10h owner, RET. Source fastcall adds EDX
// stable reference to actual mutable F87664. Publish D0DA64, capture initial
// root, arm tree/base cleanup, erase weak subtree, reset current sentinel and
// count, end tree cleanup, call full range erase, free current sentinel, clear
// current head/count, then unconditionally clear publication and storeCE3818.
// Source C++ failure during subtree/reset invokes full86FDE0 then869C50;
// failure during range/free tail invokes869C50. No payload release/unregister.
void __fastcall destroy_native_gameplay_effect_manager_0086fe20(
    void* owner, void* volatile& actual_publication_00f87664);

// Full8703E0[30]: ECX actual owner, stack flags, EAX captured owner, RET4.
// Added EDX publication reference reaches the completed destructor. Read bit0
// of the current flags low byte only after it returns; free captured owner
// if set, then return its original address bits (possibly freed).
void* __fastcall delete_native_gameplay_effect_manager_008703e0(
    void* owner, void* volatile& actual_publication_00f87664, std::uint32_t flags);

// Source CRT/C++ EH contracts and raw ownership apply. Native FH3/SEH metadata,
// private binding/EH-spill aliases, hardware-fault and cleanup-provider exception
// identity are not reproduced. Native profile D0DA64 remains identity data;
// these functions do not turn it into a callable C++ vtable or supply the raw
// lifetime manager's complete mixed-owner dispatch. No original caller ABI or
// full gameplay compatibility is implied.
} // namespace bsp
