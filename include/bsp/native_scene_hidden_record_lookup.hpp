#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene hidden-record lookup requires MSVC Win32.
#endif

namespace bsp {

// Bind distinct actual native regions: at entry ESP=P, input4=P+4,
// key header8=P-10h and output iterator8=P-8. Caller initialization of the
// header/output is not invented. Keep this binding metadata stable/disjoint
// from reached storage. Payload aliases with those actual regions remain
// admitted wherever all reached accesses and provider domains stay valid.
struct NativeSceneHiddenRecordLookupFrame {
    const void* actual_parameter4;
    void* actual_key_header8;
    void* actual_iterator8;
};

// Whole004691B0..0046923D [142B/54ops]. Original ECX database, unused EDX,
// stacked name pointer, RET4/raw EAX mapped DWORD or0; ESI/EDI/EBX preserved.
// Read input4 BEFORE any backing writes, construct actual header, run real
// Find on database+18h, then release CURRENT key.data/CURRENT key.length.
// After release capture CURRENT output.owner, then CURRENT map.head even
// when owner is null; report bad owner, allowing return. Only then capture
// CURRENT output.node. End returns0. Unequal paths retain owner/node across
// both remaining reports; read current owner.head and finally current node+14.
std::uint32_t find_native_scene_hidden_record_004691b0(
    void* actual_scene_database, const NativeSceneHiddenRecordLookupFrame&,
    NativeStringRawPoolContext&);

// Whole00469480..004694ED [110B/43ops]. Same original inputs/RET4, AL0/1
// only; upper EAX bits are not a canonical boolean contract. Capture head
// AFTER construction but BEFORE real Find, then use its actual returned
// iterator pointer. Capture owner once, report if bad, compare CURRENT node
// after a returning report, read CURRENT key.data, retain the comparison's
// byte result across explicit normal release. No mapped-word test is made.
std::uint8_t has_native_scene_hidden_record_00469480(
    void* actual_scene_database, const NativeSceneHiddenRecordLookupFrame&,
    NativeStringRawPoolContext&);

// Complete ordinary raw Source at the accepted constructor/Find/raw pool and
// returning current-CRT boundaries; no new owner, callback, guard or stub.
// Neither flat native body installs outer FH3 cleanup. No outer Source RAII
// cleanup is added on child/report failure. Raw results imply no nominal
// payload class, CString validity of results, ownership or lifetime.
// These added metadata interfaces do not expose native/compiler private
// frame, child argument-slot, saved-register/return/EH/spill or async aliases.
// Original CRT/fault/SEH/register/binary ABI and game identity are unproved.

} // namespace bsp
