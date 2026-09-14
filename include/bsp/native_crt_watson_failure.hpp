#pragma once

#include <cstdint>

struct _EXCEPTION_POINTERS;

namespace bsp {
// Eight stable borrowed reference identities from one actual CRT domain.
// The caller owns the cookie/complement, hook/feature words, shared records,
// debugger word and the actual pair object. No canonical storage is created.
// Arrays retain actual current bytes; neither shared record is initialized or
// cleared here. Their alignment must support the original DWORD/WORD accesses.
struct NativeCrtWatsonBindings {
    const volatile std::uint32_t& cookie_00e15590;
    const volatile std::uint32_t& complement_00e15594;
    volatile std::uint32_t& hook_word_0109eea8;
    const volatile std::uint32_t& feature_word_0109eea4;
    volatile std::uint8_t (&exception_record_0109e568)[0x50];
    volatile std::uint8_t (&context_record_0109e5c0)[0x2cc];
    volatile std::uint32_t& debugger_word_0109e5b8;
    volatile ::_EXCEPTION_POINTERS& exception_pointers_00d6e1cc;
};

// Complete BF65BB[252], new naked cdecl interface: the original five ignored
// diagnostic words are followed by bindings at entryESP+18h. Caller cleans24.
// With S=entryESP, biased EBP=S-2ACh, capture records remain at original offsets:
// exception record S-32Ch[50h], pair S-2DCh[8], CONTEXT S-2D4h[2CCh], guard S-8.
// Captures cookie XOR biased EBP as EAX and the XOR flags, original other integer
// registers, low16 segment words, source return PC, saved EBP and ESP=S.
// CONTEXT is not zeroed. Real full CL memset clears only the50h record: DF=0 is
// required, no CLD is added, and neither feature-cell read nor SSE2 executes.
// The real APIs clear the unhandled filter without restoring it and request
// termination. If they return, the real cookie check and original return tail
// remain. No noreturn contract, exception translation or caller-frame owner.
void __cdecl invoke_native_crt_watson_00bf65bb(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file,
    std::uint32_t line, std::uintptr_t reserved,
    const NativeCrtWatsonBindings& bindings);

// Complete BFE120[15] through a NEW register/stack interface. Actual ECX must
// already hold the cookie; this ordinary-looking declaration does not arrange
// ECX for a C++ caller. A naked assembly caller supplies bindings at entryESP+4
// and cleans4. Preserves EAX and all registers on equality, retains CMP flags
// and F3C3 return; mismatch directly tail-jumps the complete reporter with the
// existing return word, raw registers and added binding slot. This cannot serve
// unchanged C0DC54 or any native/compiler no-extra-argument cookie-check edge.
void __cdecl check_native_crt_cookie_00bfe120(
    const NativeCrtWatsonBindings& bindings);

// Complete C185A4[260], new naked cdecl binding at entryESP+4, caller cleans4.
// Original PUSH EBP/MOV/SUB ESP,328h precedes capture. One added PUSH ECX uses
// [EBP-32Ch], below the original allocation, and is restored after PUSHFD/POP.
// MOV-only capture plumbing preserves incoming integer/segment snapshots and
// SUB flags. Shared EBP/EIP/ESP describe the source caller, ESP=entryESP+4.
// Retains all original partial widths, current EIP/debugger reloads, the unused
// [EBP-320h] read, and ordered cookie/complement reads into original stack slots.
// Passes the address of the actual pair object whose current fields point to
// these same actual record/context objects. It never substitutes a local pair.
// If TerminateProcess returns, LEAVE/RET preserves its EAX result.
void __cdecl report_native_crt_gsfailure_00c185a4(
    const NativeCrtWatsonBindings& bindings);

// Bindings, reference identities, their extra argument words, actual pair and
// all original-width memory must remain valid/stable throughout reached calls.
// Native record/context/pair identities are distinct, and must not alias active
// source/provider frames, binding storage, extra arguments or capture scratch.
// Current referenced contents remain mutable; ordered reloads remain visible.
// DF=0, real writable nonwrapping stack/extents and the real Win32 API domain
// are caller requirements. Untouched local CONTEXT bytes stay uninitialized;
// untouched shared bytes retain prior state, including segment high16 halves.
// Added binding/scratch/call accesses and incidental volatile-register changes
// after capture have distinct fault and asynchronous-observation sites. No
// original code/return-address/native SEH/FS-frame/exception identity, startup,
// canonical cookie/failure owner, runtime or gameplay validation is supplied.
// No synthetic service, validation, catch, RtlCaptureContext or host CRT cookie.
} // namespace bsp
