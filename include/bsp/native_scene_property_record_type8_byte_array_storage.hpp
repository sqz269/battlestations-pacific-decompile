#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native type-8 byte-array storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type8_byte_array_storage_bytes = 0x38;

// Whole [008EF2F0,008EF360): 112 bytes, 34 instructions, both branches.
// Descriptive names are hypotheses; this is raw storage, not a native class.
// Supply actual fresh/unowned writable 56-byte root storage and a stable,
// readable positive-byte_count input span. Root, input and active call frame
// must be disjoint; ranges must not wrap. Require DF clear for current CRT.
// ECX=root; incoming EDX unused; stack DWORDs +4/+8/+C are input/count/flags.
// Only the low byte of copy_flag_bits selects the branch; high bits are ignored.
// Low byte zero retains the exact input pointer at +20. Input stays caller-
// managed while that pointer is used; no new child is allocated or freed.
// Nonzero low byte allocates and copies exactly byte_count bytes with genuine
// current providers. Observe that owned child while live and free it once with
// singleton_lifetime_free before disposing the root. Never free a retained
// input as a newly allocated child. Byte +2C is one in both branches and does
// not establish class ownership. No phase/vtable/destructor dispatch is valid.
// Writes 29 bytes: [00,08), [18,28), [2C,2D), [34,38). Preserves 27 bytes:
// [08,18), [28,2C), [2D,34), including the DWORD at +30.
// EAX returns root, RET0C removes three DWORDs, ESI/EDI are saved/restored.
// Retaining branch: ECX=root, EDX=input; final CMP flags mask8D5=44, AF=0.
// Copying branch: ECX/EDX are unconstrained actual provider residuals; final
// ADD ESP,10h flags derive from entry T: (T-24)+16=T-8, all six arithmetic
// flags defined. EBX/EBP follow the genuine providers' normal ABI. No blanket
// DF/ES/FPU/MXCSR preservation through providers and no noexcept promise.
// Initial flag-byte read precedes phase/tag stores; byte-count load follows
// them. Copying stores the actual allocation at +20 before memcpy. Failed
// providers may leave partial writes. Zero-size, invalid/overlapping spans,
// failure/reentry/unwind, private native CRT/EH, class/World and game are out
// of this qualified successful current allocation/copy/free domain.
void* __fastcall construct_native_scene_property_record_type8_byte_array_storage_008ef2f0(
    void* actual_root_ecx, void* unused_edx, const void* actual_bytes,
    std::uint32_t byte_count, std::uint32_t copy_flag_bits);

} // namespace bsp
