#pragma once

#include <cstdint>

namespace bsp {
// Complete native entries, requiring the already-published DD canonical owner:
// actual E15590/E15594 cookie pair, 109E568[324h] shared failure/context block,
// 109EEA8 hook word and actual RO D6E1CC pair. No binding argument, shadow,
// startup, validation, exception translation or frame owner is supplied here.
// Existing NativeCrtWatsonBindings interfaces remain separate and unchanged.
// These are MSVC Win32 naked native entries, not ordinary C++ service APIs.

// BF65BB[252]: original five cdecl diagnostic words, all unread; caller cleans
// 20 bytes. Captures cookie XOR biased EBP and XOR flags, the remaining native
// registers, partial segment words and native source caller EBP/EIP/ESP. Only
// the 50h exception record is cleared; local CONTEXT remains partly unwritten.
// The actual complete CL memset receives the original three words in assembly:
// count=50h is below its 100h gate, so no fourth feature-binding word is read.
// This is a bounded reached-path compatibility claim, not general memset ABI.
// DF=0 and a writable nonwrapping native stack disjoint from owner cells are
// required. Real APIs can return; the real cookie-check and original tail stay.
void __cdecl invoke_native_crt_canonical_watson_00bf65bb(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file,
    std::uint32_t line, std::uintptr_t reserved);

// BFE120[15]: raw cookie in ECX, NO stacked arguments. Ordinary C++ calls do
// not establish ECX. Equality preserves every register and the actual CMP
// flags through F3 C3; inequality tail-jumps the complete canonical reporter
// with unchanged raw registers, return word and stack. No EAX scratch is used.
void __cdecl check_native_crt_canonical_cookie_00bfe120();

// C185A4[260]: NO stacked arguments. Original PUSH EBP/MOV/SUB ESP,328h
// precedes the ordered shared capture; stores incoming EAX/ECX and SUB flags
// without added scratch. Shared EBP/EIP/ESP describe its source caller.
// Partial widths, unused stack read and ordered current-state reloads remain.
// Passes actual D6E1CC to the real filter API. If termination returns, LEAVE/RET
// preserves its EAX result. No noreturn contract is invented.
void __cdecl report_native_crt_canonical_gsfailure_00c185a4();

// C04EF3[8]: original ignored reason word, cdecl caller cleanup. Exact AND0
// RMW at actual109EEA8; RET. All registers survive; CF/OF/SF=0, ZF/PF=1,
// AF undefined, other flags unchanged. No extra hook binding or shadow cell.
void __cdecl clear_native_crt_canonical_debugger_hook_00c04ef3(
    std::uint32_t ignored_reason);

// Captured code PCs belong to this rebuilt source. Fixed data identity and
// static native/emitted ABI audits do not establish original code addresses,
// native SEH/unwind closure, asynchronous/debugger integration or gameplay.
// Owner storage, source stack/arguments and reached APIs must remain valid.
// Faults, partial stores, ignored API failures and returned-termination tails
// retain their native behavior. No original/owned failure function is run by
// this packet. See docs/NATIVE_CRT_CANONICAL_FAILURE_DK.md.
} // namespace bsp
