#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual opaque vector-storage tidy alias requires MSVC Win32.
#endif

namespace bsp {

// Complete00468020..00468049 [42B/13ops]. Native ECX opaque16B storage,
// incoming EDX unused, ESI preserved, no stack arguments, plain RET; no semantic
// EAX result. Full original bytes match00468840 except its free CALL rel32,
// which independently resolves to the same actual00BF65AC target.
// Source reuses the complete accepted00468840 implementation: capture+4 ONCE,
// conditionally free that captured pointer, then zero+4,+8,+Ch in order on
// both returning paths. Native00468020 does not call00468840.
void __fastcall tidy_native_vector_storage_00468020(void* actual_storage16);

// +0 is not directly accessed; element type and all field roles remain opaque.
// No destructor, diagnostic, copied owner, callback or extra guard is invented.
// Correct STL_Vector_Tidy native name is retained. Valid taken backing/current-
// CRT releases are required. Source fastcall retains the receiver shape but
// does not prove original private-frame/register/flags/mangling/heap/CRT/FH3/
// SEH/fault/binary identity, direct native differential or game behavior.

} // namespace bsp
