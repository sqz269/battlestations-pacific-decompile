#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record type-5 storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF2B0,008EF2EF): 63 bytes, 19 instructions, one genuine CALL.
// The descriptive name is a hypothesis; no native class/refcount/World or
// owning property-object lifetime is established by this raw operation.
inline constexpr std::size_t native_scene_property_record_type5_storage_bytes = 0x38;

// Supply actual fresh/unowned writable 56-byte destination storage and either
// nullptr or actual stable readable NUL-terminated text. Text through its first
// NUL, destination and active target call frame must be disjoint; length+1 and
// address ranges must not wrap. Do not overwrite a live owning property object.
// Require DF clear for the genuine current CRT providers. Only normal/null
// execution is admitted: no failure, reentry or naked-frame unwind promise.
// ECX=destination; incoming EDX unused; stack DWORDs at entry +4/+8/+C are
// actual_text, word_18_bits, word_08_bits; RET0C. EAX returns destination.
// The actual admitted ECX/RET0 duplicate entry supplies the owned child at+1C.
// Input remains borrowed; after observing each nonnull child while live, free
// it exactly once with singleton_lifetime_free before releasing its root.
// ESI is saved/restored; EBX/EDI/EBP follow the genuine provider's normal ABI.
// ECX/EDX retain provider residuals (nonnull values are unconstrained).
// Final XOR flags: CF=OF=SF=0, ZF=PF=1; AF undefined. No blanket DF/ES/FPU/
// MXCSR preservation claim covers the provider call. No noexcept promise.
// Writes 33 bytes: [00,0C), [18,28), [2C,2D), [34,38).
// Preserves 23 bytes: [0C,18), [28,2C), [2D,34).
// Literal opaque phase00CE89D4, tag5 and both raw words are stored before the
// duplicate CALL. Its full EAX is stored at+1C, then +20/+24/+34 are zeroed
// and byte1 is stored at+2C. Never dispatch the phase word as a Source vtable.
// Native private CRT/EH, parsing, class/owner/destructor and gameplay are outside
// this qualified current allocation/copy/free domain.
void* __fastcall construct_native_scene_property_record_type5_storage_008ef2b0(
    void* actual_record_ecx, void* unused_edx, const char* actual_text,
    std::uint32_t word_18_bits, std::uint32_t word_08_bits);

} // namespace bsp
