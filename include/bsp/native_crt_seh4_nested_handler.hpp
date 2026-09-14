#pragma once

#include <cstdint>
#include <excpt.h>

namespace bsp {
// Complete C0DBC4[144] __local_unwind4 and C0DC54[70] nested handler.
// MSVC Win32 naked instruction entries. These declarations do not construct
// the required inherited EBP, OS exception state, FS chain or native funclets.
// The already-published canonical owner supplies actual E15590 and E16830;
// the real canonical checker/reporter, NLG notifier and cleanup entry are
// separate providers. No passed binding, portable exception shim or frame
// owner substitutes for those domains.
//
// Entry ESP=S, inherited EBP=H, P=[S+4], F=[S+8], T=[S+C]. Caller cleans
// 12 bytes. P is the actual scope-decoding cookie pointer; the registration
// cookie separately uses fixed E15590. The nested registration N=S-28h owns
// previousFS/handler/cookie/T/F/P/H at +0/+4/+8/+C/+10/+14/+18. Publication
// follows the cookie store. The loop reloads P/F/current level and actual
// table fields, uses unsigned stopping comparisons and wrapping x86 LEAs,
// publishes enclosing level before cleanup, and reloads the cleanup target
// after actual NLG. Native cleanup targets and their frame/register/stack
// disciplines must be valid; no callback wrapper or argument validation exists.
// Normal exit unlinks FS first and skips saved H rather than restoring EBP.
// EBX/ESI/EDI restore; raw EAX/ECX/EDX and flags retain native exit effects.
void __cdecl unwind_native_crt_local_scopes_00c0dbc4(
    const volatile std::uint32_t* scope_cookie, void* registration,
    std::uint32_t target_level);

// Original four cdecl OS words; caller cleans 16 bytes. ContextRecord is
// unread. Flags&6==0 returns ExceptionContinueSearch (1), preserving TEST
// flags. Otherwise ECX=N.cookie XOR N reaches the no-argument checker;
// equality must preserve EAX=N for the following inherited-EBP load. The
// handler recursively calls the complete loop, restores incoming EBP,
// reloads both N and dispatcher_output, writes N through that output, then
// returns ExceptionCollidedUnwind (3) with ADD ESP,C arithmetic flags.
EXCEPTION_DISPOSITION __cdecl handle_native_crt_seh4_nested_unwind_00c0dc54(
    _EXCEPTION_RECORD* exception_record, void* nested_registration,
    _CONTEXT* unused_context_record, void* dispatcher_output);

// DF is neither changed nor normalized. Reached providers keep their own
// domain requirements, including DK's DF=0 requirement on its failure path.
// Faults/nonlocal exit may leave published level/FS/shared state; no rollback
// or catch is supplied. An unexpectedly returning failure reporter follows
// the original continuation with its actual returned registers.
//
// Exact code and these register/stack interfaces do not by themselves admit
// the handler to the rebuilt module's SafeSEH table. Actual OS admission,
// exception-chain/frame validity, debugger behavior and gameplay require
// separate evidence. The source is not installed at original code addresses.
// See docs/NATIVE_CRT_SEH4_NESTED_HANDLER_DP.md.
} // namespace bsp
