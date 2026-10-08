#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native type-10 raw-array storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type10_raw_array_storage_bytes = 0x38;

// Whole [008EF3E0,008EF454): 116 bytes, 36 instructions, both branches.
// Reconstructed Source only; primary complete-helper/build qualification is
// pending. Descriptive names are hypotheses; this constructs raw storage.
// Supply actual fresh/unowned writable 56-byte root storage and a stable
// readable span of exactly 4*element_count bytes, count in [1,3FFFFFFFh].
// Root, input and active frame must be disjoint and nonwrapping. Require DF
// clear for current CRT. Payload bytes have no integer/float/vector semantics.
// ECX=root; incoming EDX unused; stack +4/+8/+C are count/input/full flag bits.
// Save ESI/EDI, load count, double it, store phase and tag 10, double it again,
// then compare only the low flag byte. No count/overflow guard is inserted.
// Low flag byte zero retains the exact input pointer at +20. Input remains
// caller-managed while used; no child allocation/free or ownership transfer.
// A nonzero low byte allocates/copies exactly 4*count bytes through genuine
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
void* __fastcall construct_native_scene_property_record_type10_raw_array_storage_008ef3e0(
    void* actual_root_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_bytes, std::uint32_t copy_flag_bits);

} // namespace bsp
