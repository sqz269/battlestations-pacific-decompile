#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string vector operations require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 00B6C670..00B6C6AC, native/source cdecl with three stacked pointer
// inputs, plain RET, AL Boolean only (upper native EAX unspecified). Call the
// existing exact native8h-string mismatch helper with all six physical words:
// private output, first, last, second_first, unused last, unused dispatch tag.
// Compare its first result with captured last. No range validation or ownership.
// Source initializes the dead tag instead of reading native local high bytes;
// original private-frame aliasing and current CRT locale identity are qualified.
bool __cdecl equal_native_string_range_00b6c670(const void* first,
    const void* last, const void* second_first);

// Complete 00432360..00432375. Original ECX is an unused opaque owner;
// stack first/last, RET8, no semantic result. The child receives ECX=first,
// EDX=last and two unused physical words (owner,last), then destroys ascending
// actual8h string headers using the current actual419CC0/BD1510 pool domain.
// This new Source interface carries that domain explicitly and delegates to
// the existing complete raw-header destruction body. It does not free backing,
// clear headers, validate ranges, or assume ownership of owner/context. Getter
// exceptions propagate; no noexcept, rollback or local cleanup is introduced.
void destroy_native_string_range_dispatch_00432360(void* unused_owner,
    void* first, void* last, NativeStringRawPoolContext& strings);

// These Source interfaces are not original stack/FH3/CRT ABI replacements.
// The complete00506C80 assignment composed from these accepted services is
// declared separately in native_string_vector_assignment.hpp.

} // namespace bsp
