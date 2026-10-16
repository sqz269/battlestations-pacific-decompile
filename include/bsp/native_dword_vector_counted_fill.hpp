#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual checked DWORD-vector counted fill requires MSVC Win32.
#endif

namespace bsp {

// Whole00698040 normal body plus lexical0069809B catch. Native ECX output,
// EDX unsignedcount; four physical stackwords source,ownerbits,valuebits,tag;
// RET10h. Sourcevoid excludes incidental nativeEAX, including oldFS0 onempty.
// Capture initial/current/completed output before the count test. Count0 never
// loads source. Each nonzero current rereads the source ARGUMENT before actual
// 557590 construction, which consumes the CURRENT source header/backing.
// Null current skips construction only; count decrements, cursor advances10h,
// and completed end publishes only after that successful/skip iteration.
void __fastcall fill_native_dword_vector_counted_00698040(
    void* output, std::uint32_t count, const void* source,
    std::uint32_t unused_owner_bits, std::uint32_t unused_value_bits,
    std::uint32_t unused_tag);

// Whole00698310: native three stacked output/count/source words, RET0Ch.
// Incoming ECX contributes only ignored owner/tagbits; incomingEDX is unused.
// Source is stdcall3inputs, with canonical private owner/tag0 and duplicate
// sourcebits in the four core stackslots. No ambient/native owner is invented.
// Ignore coreEAX; independently return originaloutput+(count<<4), DWORDwrap.
void* __stdcall fill_native_dword_vector_counted_dispatch_00698310(
    void* output, std::uint32_t count, const void* source);

// On a CurrentCpp exception, lexical catch captures [initial,completedend),
// captures ownerbits only if nonempty, then FORWARD tidies each10h header via
// actual004D29D0 and rethrows. No nullskip, rollback or failing-element cleanup
// beyond the actual constructor's own scope. Native state1 actionC7EC10 reads
// two saved private addresses, calls literal00401130 RET and balances8bytes;
// effect-elided ONLY within valid Source private-frame/currentCPP limits. No
// public EBP/action helper, dummy destructor, callback or provider is added.
//
// All reached accesses/releases/child equality loops must be valid and finite,
// in accepted currentCRT allocation domain. Public aliases retain child/current
// source and prefix order; original private argument/local stack aliases and
// asynchronous mutation are excluded. Source EH/privateframes/CRT differ:
// original FH3/RTTI/nested cleanup failure/fault/SEH/heap/ambient registers,
// binaryABI and gameplay equivalence are unproved. No noexcept promise or new
// guard/count bridge/typed header model. Correct library names and existing
// actual narrower/Hook interfaces stay separate; Source names are hypotheses.

} // namespace bsp
