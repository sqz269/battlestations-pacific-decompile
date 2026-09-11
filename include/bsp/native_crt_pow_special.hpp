#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pow special handling requires MSVC Win32.
#endif

namespace bsp {

// Borrowed actual immutable cells, not copied/synthesized literal values.
// Assembly caller loads EBX=half_00d7a280 and EDI=literals_00e165a0 before
// either entry. Original relative layout in the second region is required:
// +00h infinity (8 bytes), +08h negative qNaN (8), +20h negative zero (8).
// Bytes +10h..+1Fh are never read by these entries. Half occupies eight bytes
// at EBX+0. Expected cell bits, little endian: half000000000000E03F,
// infinity000000000000F07F, qNaN000000000000F8FF, zero0000000000000080.
// Both registers remain bound across all child calls and survive the entries.
// Pointed-to storage must remain valid for every reached read; no entry-time
// read/validation, new global, or constant snapshot is added. This aggregate
// is a binding description; the bodies never read the aggregate itself.
struct NativeCrtPowSpecialContext {
    const void* half_00d7a280;
    const void* literals_00e165a0;
};
static_assert(sizeof(NativeCrtPowSpecialContext) == 8);

// ASSEMBLY CALLERS ONLY. In addition to the actual raw stack words below,
// callers must establish EBX/EDI as above; ordinary C++ calls are unsafe.
// Source interfaces add those explicit register/data bindings and make no
// original-address/drop-in or general original-caller ABI claim. All returns
// use plain RET; the assembly caller removes its original argument bytes.
// Current x87 rounding, precision, status, tags and exception behavior apply.
// These entries neither save/reset the environment nor translate faults.

// Complete original __d_inttype C19DC0[100]. Two actual argument DWORDs occupy
// the incoming binary64 slot; repeated reads remain at their original sites.
// Requires one free x87 stack slot. No x87 value remains on normal return.
// EAX is 0 when classification/equality rejects, 1 when the half-product
// equality rejects, 2 when it accepts. This is integer parity only where the
// current rounding/precision keeps half-product classification exact.
// Internal calls to full __fpclass and __frnd pass ONLY the original eight
// bytes. __fpclass's extra readable tail bytes are within allocated caller
// locals; its public third-word declaration must not add a third push here.
// __frnd's ST0 output is consumed by the original comparison, not C++ code.
std::int32_t __cdecl native_crt_d_inttype_00c19dc0(
    std::uint32_t low_word, std::uint32_t high_word);

// Complete C19E24[318], descriptive source name. Four actual words occupy x/y
// binary64 slots; output is the current fifth stack argument (8 writable
// bytes when a store is reached). No double snapshots or argument adapters.
// Requires three free x87 slots; normal paths balance their own temporaries.
// EAX=0/1 from original ESI status. Unsupported paths can return0 without
// writing output; no default is synthesized. Exact literal and branch logic,
// including unordered comparisons and negative-zero/NaN behavior, remains.
// EBX/EDI/EBP survive; ESI is saved/restored. EAX/ECX/EDX and flags retain
// original instruction/complete child effects; no C++ numerical abstraction.
std::int32_t __cdecl evaluate_native_crt_pow_special_00c19e24(
    std::uint32_t x_low, std::uint32_t x_high,
    std::uint32_t y_low, std::uint32_t y_high, void* actual_output);

} // namespace bsp
