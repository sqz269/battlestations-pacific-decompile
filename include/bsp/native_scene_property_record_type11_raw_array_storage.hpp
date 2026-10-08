#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native type-11 raw-array storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type11_raw_array_storage_bytes = 0x38;

// Whole [008EF460,008EF4D7): 119 bytes, 37 instructions, both branches.
// Reconstructed Source only; primary complete-helper/build qualification is
// pending. Descriptive names are hypotheses; this constructs raw storage.
// Supply actual fresh/unowned writable 56-byte root storage and a stable
// readable span of exactly 12*element_count bytes, count in [1,15555555h].
// Root, input and active frame must be disjoint and nonwrapping. Require DF
// clear for current CRT. Payload bytes have no integer/float/vector semantics.
// ECX=root; incoming EDX unused; stack +4/+8/+C are count/input/full flag bits.
// Load count BEFORE saving ESI/EDI; LEA makes 3*count, ADD makes 6*count; store
// phase and tag 11, then ADD makes 12*count before reading the low flag byte.
// No new count/overflow guard, signed multiply or element conversion is added.
// Low flag byte zero retains the exact input pointer at +20. Input remains
// caller-managed while used; no child allocation/free or ownership transfer.
// A nonzero low byte allocates/copies exactly 12*count bytes through genuine
// current providers. Observe the child while live, then free it once with
// singleton_lifetime_free before root disposal. Never free retained input as
// a new child. Byte +2C equals one in both branches, not an ownership proof.
// Writes 29 bytes: [00,08), [18,28), [2C,2D), [34,38). Preserves 27 bytes:
// [08,18), [28,2C), [2D,34), including owner DWORD +30. No owner is published.
// EAX returns root; RET0C removes three DWORDs; ESI/EDI are saved/restored.
// Retention: ECX=root, EDX=input; final CMP flags mask8D5=44, AF=0.
// Copy: ECX/EDX provider residuals are unconstrained; final ADD ESP,10h defines
// all six arithmetic flags from entry T: (T-24)+16=T-8. EBX/EBP depend on the
// genuine providers' normal ABI. No blanket DF/ES/FPU/MXCSR preservation.
// Copy loads input after allocation and stores actual child+20 before memcpy.
// Failure may leave partial writes; no noexcept or naked-frame unwind promise.
// Zero-size/overflow/alias/failure/reentry, Native private CRT/EH, phase/vtable
// dispatch, class/free-marker/owner lifetime, whole clone and game are outside
// this proposed successful current allocation/copy/free qualification domain.
void* __fastcall construct_native_scene_property_record_type11_raw_array_storage_008ef460(
    void* actual_root_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_bytes, std::uint32_t copy_flag_bits);

} // namespace bsp
