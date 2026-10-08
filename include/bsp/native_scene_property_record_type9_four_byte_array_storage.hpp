#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native type-9 four-byte-array storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type9_four_byte_array_storage_bytes = 0x38;

// Whole [008EF360,008EF3D4): 116 bytes, 36 instructions, both branches.
// Descriptive names are hypotheses. This API constructs raw storage.
// Supply fresh/unowned writable 56-byte root storage and a stable readable
// span of exactly 4*element_count bytes, with element_count in [1,3FFFFFFF].
// Root, input and active call frame must be disjoint and nonwrapping. Require
// DF clear for current CRT. Elements are opaque bits with no numeric semantics.
// ECX=root; incoming EDX unused; stack +4/+8/+C contain count/data/full flags.
// Native DWORD additions double count twice: the first precedes phase/tag
// stores, the second follows them, and then CMP reads only the low flag byte.
// No new overflow guard or signed multiplication is inserted in the body.
// Low flag byte zero retains the exact input pointer at +20. Input remains
// caller-managed while used; no child allocation/free or ownership transfer.
// A nonzero low byte allocates and copies exactly 4*element_count bytes with
// genuine current providers. Observe the actual child while live, then free
// it once with singleton_lifetime_free before disposing the root. Never free
// a retained input as a new child. Byte +2C equals one in both branches and
// does not establish class ownership. No phase/vtable/destructor dispatch.
// Writes 29 bytes: [00,08), [18,28), [2C,2D), [34,38). Preserves 27 bytes:
// [08,18), [28,2C), [2D,34), including the DWORD at +30.
// EAX returns root; RET0C removes three DWORDs; ESI/EDI are saved/restored.
// Retaining branch: ECX=root, EDX=input; CMP defines flags mask8D5=44, AF=0.
// Copying branch: actual provider ECX/EDX residuals are unconstrained; final
// ADD ESP,10h defines all six arithmetic flags from entry T: T-24+16=T-8.
// EBX/EBP follow the current providers' normal ABI. No blanket DF/ES/FPU/MXCSR
// preservation through providers and no noexcept promise. Copy reads data
// after allocation and stores the actual child at +20 before memcpy.
// Failure can leave partial writes. Zero-size/overflow/alias/reentry/unwind,
// private Original CRT/EH, class ownership, full clone, World and game behavior
// remain outside this qualified successful current allocation/copy/free API.
void* __fastcall construct_native_scene_property_record_type9_four_byte_array_storage_008ef360(
    void* actual_root_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_bytes, std::uint32_t copy_flag_bits);

} // namespace bsp
