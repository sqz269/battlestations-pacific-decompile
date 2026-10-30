#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual MenuPrompt scalar lifetime composition requires MSVC Win32.
#endif

namespace bsp::frontend_seg15_22_cc13 { struct Seg15Host; }
namespace bsp {

// Source composition of whole00532A00..00532A1D [30B/11ops]. Native ECX is
// the captured actual290h receiver; one fullDWORD flags argument; RET4/EAX
// original receiver identity. Call WHOLE existing raw5321F0 through the SAME
// genuine required Seg15Host. ONLY after normal return read byte0 of the live
// flags home; bit0 requests realBF65AC on the captured allocation. Upper bits
// are not normalized, copied into a bool, read early or written. The returned
// address is identity only and may already be freed: it is no liveness proof.
void* delete_native_menu_prompt_screen_00532a00(void* actual_receiver,
    volatile std::uint32_t& actual_flags_dword_home,
    frontend_seg15_22_cc13::Seg15Host& actual_host);

// Whole00532340..00532347 [8B/2ops]: native SUB ECX,0Ch; JMP00532A00.
// Apply DWORD address subtraction to the captured actual singleton+0C
// subobject, forwarding the SAME full flags HOME and SAME host unchanged.
// No publication reload, ID, current-profile substitute or codeword cast.
void* delete_native_menu_prompt_secondary_00532340(void* actual_secondary_receiver,
    volatile std::uint32_t& actual_flags_dword_home,
    frontend_seg15_22_cc13::Seg15Host& actual_host);

// These new ordinary C++ interfaces neither reproduce the original physical
// thiscall/tail-JMP/RET4 ABI nor add an exception/ownership policy. A throwing
// full destructor skips flag read/free/return; a throwing free propagates.
// No noexcept, catch, termination, retain/count, retry, fallback or rollback
// is added. Caller retains genuine actual backing/flag home through their
// reached accesses and the real host/member providers through the whole call.
// No receiver bytes are read after possible free; only captured identity returns.
//
// Future current dispatch must resolve captured CED3AC/+0->532340 at its actual
// invocation epoch, then use these operations over the genuine member graph.
// Base/current-profile variants need their own real route. No singleton-map,
// application, global-view or activation binding is installed here. CG wrappers
// remain compiler_generated_out_of_scope; zero extra Original/vendor/CG or
// duplicate-destructor credit. Native ABI/FH3/SEH/runtime/game proof is absent.
} // namespace bsp
