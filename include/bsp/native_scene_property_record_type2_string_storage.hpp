#pragma once

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record type-2 string storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type2_string_storage_bytes = 0x38;

// Whole 008EF1B0..008EF1EC: 60 bytes / 20 instructions. Actual ECX is fresh,
// unowned writable 56-byte storage disjoint from the borrowed text and frame.
// The sole stack DWORD is nullable NUL-terminated text; incoming EDX is unused.
// Return full EAX=receiver, RET4. Do not overwrite a live owning record.
// Writes 37 bytes and preserves [08,0C), [10,18), [28,2C), [2D,30).
// The literal +00 word 00CE89D4 is identity only, never a Source vtable.
// Tag +04 is 2; +0C receives the genuine current-owned duplicate or null;
// byte +2C becomes 1 even for null. Free each nonnull copy exactly once through
// singleton_lifetime_free before disposing/reusing caller-owned receiver
// storage. No native class, destructor or original private CRT/EH is admitted.
// Final flags come from the actual duplicate: null XOR has undefined AF;
// nonnull ADD flags depend on the nested stack. Nonnull ECX/EDX are volatile.
// Text must stay readable through NUL, with representable length+1/no wrap.
void* __fastcall construct_native_scene_property_record_type2_string_storage_008ef1b0(
    void* actual_record_ecx, void* unused_edx, const char* actual_text_stack);

}  // namespace bsp
