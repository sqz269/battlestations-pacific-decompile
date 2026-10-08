#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Physical reference payload replacement requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_reference_payload_replace_bytes = 8;

// Whole 008F0310..008F0340: 48 bytes / 16 instructions. ECX is actual writable
// 8-byte payload storage; incoming EDX is unused. The two native stack DWORDs
// are borrowed text, then raw scalar bits. Full EAX returns those scalar bits
// on every normal path; the actual new owned pointer is stored at payload +4.
// Old +4 must be null or a sole-owned actual matching current duplicate result.
// A nonnull old allocation is freed, then +4 is cleared before text duplication.
// The text argument is loaded after old free; scalar is loaded after duplication.
// Successful writes store the new pointer/null at +4, then scalar bits at +0.
// Payload, borrowed text and old allocation must be disjoint; none may overlap
// active frames/argument slots. Text inside the old allocation would be UAF.
// Text must remain readable through NUL; length+1 must not wrap. CRT requires DF0.
// Caller frees nonnull payload +4 once with singleton_lifetime_free before
// payload reuse/disposal. Never free scalar EAX or read retired old text.
// Old/new allocation addresses may be reused; no inequality is promised.
// RET8 and normal nonvolatiles survive. Null-text ECX is zero; incoming EDX
// survives only when old +4 was null. Null XOR AF is undefined and excluded.
// Nonnull arithmetic flags derive from ADD32(T-40,16), where T is the actual
// first argument-slot ESP immediately before outer CALL. No blanket FP/flags
// or original private-heap/class/destructor/caller/OOM/EH admission is made.
std::uint32_t __fastcall replace_native_reference_payload_008f0310(
    void* actual_destination_ecx, void* unused_edx,
    const char* borrowed_text_stack, std::uint32_t raw_scalar_stack);

}  // namespace bsp
