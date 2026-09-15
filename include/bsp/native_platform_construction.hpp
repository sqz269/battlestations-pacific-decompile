#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native platform construction requires MSVC Win32.
#endif

namespace bsp {

// Borrow the application's actual publication cells. The string context and
// platform registration must use the same raw singleton manager publication.
// The caller supplies valid, writable original-layout storage (184h bytes for
// the derived owner). No semantic platform, private manager or queue is built.
struct NativePlatformConstructionContext {
    void* volatile& actual_platform_0109cf04;
    NativeStringRawPoolContext& strings;
};

// BE2960[145]: native ECX owner, no stack arguments, EAX original owner, RET.
// Publish before registration; the second getter precedes the publication read.
void* publish_native_platform_base_00be2960(
    void* actual_owner, NativePlatformConstructionContext& context);

// BE2A00[153]: native ECX owner, no stack arguments, RET. Remove the CURRENT
// publication and clear it only after removal returns. Reset original owner.
void destroy_native_platform_base_00be2a00(
    void* actual_owner, NativePlatformConstructionContext& context);

// BE2AC0[74]: native ECX owner, EAX original owner, RET. The aspect calculation
// uses SIGNED incoming +24h/+28h through x87 FILD/FIDIV/FSTP. The later stores
// of 640/480 are not its inputs. The current x87 environment is retained.
void* construct_native_window_platform_base_00be2ac0(
    void* actual_owner, NativePlatformConstructionContext& context);

// BE2B10[103]: native ECX owner, RET. Return captured title data with captured
// length+1 through the actual pool getter, leaving the title header untouched.
// A throwing getter triggers base destruction once; normal base destruction
// runs after that cleanup state is disarmed.
void destroy_native_window_platform_base_00be2b10(
    void* actual_owner, NativePlatformConstructionContext& context);

// BEC710[26]: native ECX unused, no arguments, EAX allocated 0Ch node, RET.
// Self-link next/previous only; leave payload +8h untouched. Native allocation
// failure is not translated into an empty queue or a successful null result.
void* allocate_native_text_queue_sentinel_00bec710();

// BECDA0[132]: native ECX actual184h owner, EAX original owner, RET. The queue
// is initialized only after platform publication and window-base construction.
// Leave every unassigned byte intact, including queue +174h and tail +182h.
void* construct_native_win32_platform_00becda0(
    void* actual_owner, NativePlatformConstructionContext& context);

// Context parameters are new source interfaces, not original register/SEH ABI.
// Numeric profiles preserve identity and are not callable host vtables. C++
// cleanup follows reviewed FH3 states; a second escaping cleanup exception
// terminates. Original CRT exception identity, private EH spills, hardware-fault
// unwind, concurrent mutation and invalid storage are outside the proved domain.
// This does not create a window, run a message loop or establish game validation.
} // namespace bsp
