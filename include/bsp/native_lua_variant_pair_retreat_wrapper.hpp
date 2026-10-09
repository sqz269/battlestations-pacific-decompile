#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant pair retreat wrapper requires MSVC Win32.
#endif

namespace bsp {

// Complete 006EDE90..006EDE9B[12] through a new naked Source interface.
// actual_pair occupies ECX alone; no unused EDX formal or stack argument.
// The body preserves PUSH ESI / MOV ESI,ECX / CALL actual Source137 /
// MOV EAX,ESI / POP ESI / RET. Only the CALL REL32 operand is redirected.
//
// The void* result exposes physical POST-CHILD ESI before the wrapper POP
// reads its current saved word. It equals the original pair only under the
// actual child's ESI-preservation and usable control/save-backing contract.
// A changed child ESI is returned; a changed wrapper save word affects ESI
// after the EAX copy. No child-EAX, pair-field or fresh-ECX result is used.
//
// Borrow the actual pair and the admitted child's selected node/control
// backing. Pair writes can alias current save/return words; there is no
// immutable restoration, repair, extra validation, guard, frame or cleanup.
// The actual child uses the admitted Source17 validation provider and its
// qualified current CRT behavior. No noexcept, owner, type or consumer.
// Exact emitted bytes still need Primary review; Original ABI, flags/fault
// timing, CRT binding, production lifetime, startup and gameplay are unproved.
void* __fastcall retreat_native_lua_variant_pair_wrapper_006ede90(void* actual_pair);

} // namespace bsp
