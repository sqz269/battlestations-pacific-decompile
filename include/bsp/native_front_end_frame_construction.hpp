#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame construction requires MSVC Win32.
#endif

namespace bsp {

// Complete004F7180 [17B/6ops]. Native ECX receiver, EAX same receiver, RET.
// Capture the actual raw pointer; write native profile identity CEAE54 at+0,
// then separately clear byte+4 and byte+5. No other byte is touched.
void* construct_native_front_end_screen_004f7180(void* actual_receiver);

// Complete00517D10 [32B/9ops]. Native ECX receiver, EAX captured receiver,
// RET. Call the real raw base above, then write +8=CEC388, +0=CEC39C and
// +8=CEC398 in that order. +C..+160 remain untouched, including Page arrays
// and the current-set word. No allocation, selector cell or registration.
void* construct_native_front_end_frame_00517d10(void* actual_receiver);

// Caller supplies genuine suitably aligned writable raw construction storage
// and its actual allocation/lifetime custody. Base accesses six prefix bytes;
// Frame accesses twelve within the actual164h Frame. Neither API authenticates
// the receiver or casts the semantic two-bool FrontEndScreen into a raw class.
// Native profile DWORDs are evidence identities, never callable host pointers.
// Sequential volatile fields retain overlapping-access order; Source/native
// private-stack identity, asynchronous aliases and invalid/fault behavior are
// not established. A failure leaves completed stores; no rollback is added.
// New ordinary C++ interfaces, unwired; no Native register/stack/flags/FH3/SEH,
// destructor, full initialized-object, caller or game-activation claim.

} // namespace bsp
