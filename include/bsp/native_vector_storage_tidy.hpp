#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual opaque vector-storage tidy requires MSVC Win32.
#endif

namespace bsp {

// Complete00468840..00468869 [42B/13ops]. Native ECX opaque16B storage,
// incoming EDX unused, ESI preserved, no stack arguments, plain RET; no
// semantic result. Source fastcall keeps the receiver slot without an original
// private-frame/register/CRT/mangling/binary ABI claim.
// Capture DWORD+4 ONCE. If nonzero, pass that captured pointer to the actual
// current-CRT free service. On either returning path store zero at+4, then+8,
// then+0Ch, in that order. No reload of+4 after free or early null-path return.
void __fastcall tidy_native_vector_storage_00468840(void* actual_storage16);

// Actual+0 is not directly accessed. No producer/caller establishes element
// type or begin/end/capacity/allocator field meanings; all fields remain opaque.
// No element destructor, diagnostic, callback, copied owner or validation is
// invented. Correct native STL_Vector_Tidy name is retained as a library name;
// Source naming describes this observed storage schedule, not a recovered type.
// Every taken raw access/release must remain valid. Current CRT policy does
// not prove original heap/CRT/FH3/SEH/fault identity, direct native differential,
// application admission or game behavior. No noexcept promise is added.

} // namespace bsp
