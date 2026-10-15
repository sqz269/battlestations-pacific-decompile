#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native record vector tidy requires MSVC Win32.
#endif

namespace bsp {
// Complete STL_Vector_Tidy_0041F950 (0041F950..0041F979). Original ECX is
// the receiver; no stack inputs, plain RET, ESI preserved. Capture its +60h
// pointer, conditionally release it through the genuine CRT free service,
// then zero DWORDs +60h/+64h/+68h. No element destructors or outer free.
//
// The two known unwind callers pass the gameplay-settings receiver+180h,
// making these the parent object's +1E0h/+1E4h/+1E8h words. That evidence
// does not establish the complete embedded class or vector element type.
// Pass that adjusted receiver, not the parent object or the +60h header.
// Its actual byte extent must reach +6Ch and the captured allocation must
// match the Source CRT allocator. This new source entry has the same ECX/RET
// shape, without claiming native binary, EH/SEH, or CRT-heap identity.
// Evidence: docs/GOAL15B_RECORD_VECTOR_TIDY.md.
void __fastcall tidy_native_record_vector_0041f950(void* actual_receiver);
} // namespace bsp
